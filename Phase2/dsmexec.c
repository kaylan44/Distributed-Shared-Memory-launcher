#include "common_impl.h"
#include "dsm.h"
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* variables globales */

/* un tableau gerant les infos d'identification  des processus dsm  */
dsm_proc_t *proc_array = NULL;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

//Variable pour stocker le statut de sortie des fils
sig_atomic_t child_exit_status;

// Renvoie un message indiquant le format d'entrée du programme
void usage(void){
	fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
	fflush(stdout);
	exit(EXIT_FAILURE);
}

void sigchld_handler(int sig){
	/* on traite les fils qui se terminent */
	/* pour eviter les zombies */
	int status;
	waitpid (-1, &status, WNOHANG);
	/* Stock le statut de sortie du dernier dans une variable globale.  */
	child_exit_status = status;
	printf("Processus fils traité\n"); //Indique qu'on libère bien les ressources
}


int main(int argc, char *argv[])
{
	if (argc < 3){
		usage();
	}
	else {
		pid_t pid;
		int num_procs = 0;
		int i;
		char **newargv=NULL;
		struct sigaction sigchld_action;
		char** tab_name = NULL;
		/* Variables redirection E/S */
		int (*tube_stderr)[2];
		int (*tube_stdout)[2];
		fd_set readfs;
		fd_set masterfs;
		char sortie_out[500],sortie_err[500];

		int sock_serv;
		int port_serv;
		int *sock_acc;
		struct sockaddr_in addr_acc;
		socklen_t addr_acc_len = sizeof(addr_acc);

		char buffer[SIZE_MSG];
		int buffer_int;
		char *proc_array2char = NULL;
		char hostname[SIZE_MSG] = {0};

		//Memset \0 pour éviter les probleme d'affichage
		memset(&sortie_out,'\0',500);
		memset(&sortie_err,'\0',500);

		/* Mise en place d'un traitant pour recuperer les fils zombies*/
		memset(&sigchld_action, 0, sizeof (sigchld_action));
		sigchld_action.sa_handler = &sigchld_handler;

		/* Gère SIGCHLD en appelent sigchld_handler. */
		sigaction(SIGCHLD, &sigchld_action, NULL);

		/* lecture du fichier de machines */
		/* 1- on recupere le nombre de processus a lancer */
		/* 2- on recupere les noms des machines : */
		tab_name = set_data_from_file("machine_file", tab_name,&num_procs);

		/* On connait le nb de processus, on alloue la mémoire au tableau qui en dépendent*/
		sock_acc=malloc(num_procs*sizeof(int));
		tube_stderr=malloc(num_procs*sizeof(int));
		tube_stdout=malloc(num_procs*sizeof(int));
		proc_array = malloc(sizeof(proc_array) * num_procs);
		if (proc_array == NULL){
			ERROR_EXIT("Erreur malloc");
		}

		/* la machine est un des elements d'identification */
		/* creation de la socket d'ecoute + ecoute effective*/
		port_serv = initListeningSocket(&sock_serv, num_procs, hostname);

		/* creation des fils */
		for(i = 0; i < num_procs ; i++) {

			/* creation du tube pour rediriger stdout */
			if (pipe(tube_stdout[i]) == -1) {
				perror("Le pipe stdout n'a pas fonctionné");
			}

			/* creation du tube pour rediriger stdout */
			if (pipe(tube_stderr[i]) == -1) {
				perror("Le pipe stderr n'a pas fonctionné");
			}

			pid = fork();
			if(pid == -1) ERROR_EXIT("fork");

			if (pid == 0){ /* fils */

				close(sock_serv); // le fils n'a pas besoin de la socket d'ecoute du pere serveur

				/* redirection stdout */
				close(tube_stdout[i][0]);          /* Fermeture du coté lecture non utilisé par le fils */
				dup2(tube_stdout[i][1],STDOUT_FILENO);     /* On affecte stdout à l'extrémité 1 du tube (écriture)*/

				/* redirection stderr */
				close(tube_stderr[i][0]);          /* Fermeture du coté lecture non utilisé par le fils */
				dup2(tube_stderr[i][1],STDERR_FILENO);     /* On affecte stderr à l'extrémité 1 du tube (écriture)*/

				/* Creation du tableau d'arguments pour le ssh */
				newargv = init_SshArg(tab_name[i],port_serv,hostname,argc,argv);

				/* jump to new prog : */
				execvp("ssh",newargv);
			}
			else  if(pid > 0) { /* pere */

				close(tube_stdout[i][1]);          /* Fermeture du coté écriture non utilisé par le pere */
				close(tube_stderr[i][1]);          /* Fermeture du coté écriture non utilisé par le pere */

				num_procs_creat++;
				proc_array2char = malloc(sizeof(dsm_proc_t) * num_procs_creat);
			}
		}


		for(i = 0; i < num_procs_creat ; i++){

			/* ACCEPT SOCKET */
			do{
				sock_acc[i] = accept(sock_serv, (struct sockaddr*) & addr_acc, &addr_acc_len);
			}
			while(sock_acc[i] <0 );


			/*  On recupere le nom de la machine distante */
			/* 1- d'abord la taille de la chaine */
			if (read(sock_acc[i], &buffer_int, sizeof(int)) < 0){
				ERROR_EXIT("Erreur read");
			}

			/* 2- puis la chaine elle-meme */
			memset(buffer, '\0', SIZE_MSG);
			if (read(sock_acc[i], buffer, buffer_int) < 0){
				ERROR_EXIT("Erreur read");
			}
			sprintf(proc_array[i].connect_info.machine, "%s", buffer);


			/*3- On recupere le pid du processus distant  */
			if (read(sock_acc[i], &buffer_int, sizeof(int)) < 0){
				ERROR_EXIT("Erreur read");
			}
			proc_array[i].pid = buffer_int;

			/*4- On recupere le numero de port de la socket d'ecoute des processus distants */
			memset(buffer, '\0', SIZE_MSG);
			if (read(sock_acc[i], &buffer_int, sizeof(int)) < 0){
				ERROR_EXIT("Erreur read");
			}
			proc_array[i].connect_info.listenning_port = buffer_int;
			proc_array[i].connect_info.rank = i;

			/* Affichage des donées récupérées */
			printf("machine = %s\n", proc_array[i].connect_info.machine);
			printf("pid = %d\n", proc_array[i].pid);
			printf("port = %d\n", proc_array[i].connect_info.listenning_port);
			printf("rank = %d\n\n", proc_array[i].connect_info.rank);
		}

		for(i = 0; i < num_procs_creat ; i++){

			/* envoi du nombre de processus aux processus dsm*/
			if (write(sock_acc[i], &num_procs_creat, sizeof(int)) < 0){
				ERROR_EXIT("erreur write");
			}

			/* envoi du rang du processus*/
			if (write(sock_acc[i], &i, sizeof(int)) < 0){
				ERROR_EXIT("erreur write");
			}

			/* envoi du tableau des processus aux processus dsm*/
			/* envoi des rangs aux processus dsm */
			/* envoi des infos de connexion aux processus */
			memcpy(proc_array2char, proc_array ,sizeof( dsm_proc_t) * num_procs_creat);
			if (write(sock_acc[i], proc_array2char, num_procs_creat*sizeof( dsm_proc_t)) < 0){
				ERROR_EXIT("erreur write");
			}

		}
		/* gestion des E/S : on recupere les caracteres */
		/* sur les tubes de redirection de stdout/stderr */

		//Nettoyage des fd_set
		FD_ZERO(&readfs);
		FD_ZERO(&masterfs);
		// Ajout des tubes au conteneur de socket
		for (i = 0; i < num_procs_creat; i ++){
			FD_SET(tube_stdout[i][0], &masterfs);
			FD_SET(tube_stderr[i][0], &masterfs);
		}

		while (1){
			readfs=masterfs;
			if (select(FD_SETSIZE, & readfs, NULL, NULL, NULL) <= 0) {
				ERROR_EXIT("select");
				break;
			}
			for (i = 0; i < num_procs_creat; i ++){
				/* Redirection STDOUT */
				if (FD_ISSET(tube_stdout[i][0], & readfs)) {
					if (read(tube_stdout[i][0], sortie_out, sizeof(sortie_out)) > 0){
						printf("[Proc %d : %s : stdout]  \n%s\n",i+1,tab_name[i], sortie_out);
						memset(&sortie_out,'\0',strlen(sortie_out));
					}
				}

				/* Redirection STDERR */
				if (FD_ISSET(tube_stderr[i][0], & readfs)) {
					if (read(tube_stderr[i][0], sortie_err, sizeof(sortie_err)) > 0){
						printf("[Proc %d : %s : stderr] \n%s\n",i+1,tab_name[i], sortie_err);
						memset(&sortie_err,'\0',strlen(sortie_err));
					}
				}
			}
		}

		/* on attend les processus fils */
		wait(NULL);

		/* on ferme les descripteurs proprement */
		for (i = 0; i < num_procs_creat; i ++){
			close(tube_stdout[i][0]); // fermeture du coté lecture du tube
			close(tube_stderr[i][0]);
			close(sock_acc[i]);//Fermeture des sock reliés au processus
		}
		/* on ferme la socket d'ecoute */
		close(sock_serv);
	}
	exit(EXIT_SUCCESS);
}
