#include "common_impl.h"
#include <sys/socket.h>
#include <netinet/in.h>



/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
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

void sigchld_handler(int sig)
{
	/* on traite les fils qui se terminent */
	/* pour eviter les zombies */
	int status;
	waitpid (-1, &status, WNOHANG);
	/* Stocke la statut de sortie du dernier dans une variable globale.  */
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
		char **newargv1=NULL;
		char **newargv2=NULL;

		int tube_stderr[2];
		int tube_stdout[2];

		int sock_serv;
		int port_serv;
		int sock_acc;
		struct sockaddr_in addr_acc;
		socklen_t addr_acc_len = sizeof(addr_acc);

		char buffer[SIZE_MSG];
		char hostname[SIZE_MSG] = {0};

		char sortie_out[500],sortie_err[500];


		struct sigaction sigchld_action;

		char** tab_name = NULL;

		/* Mise en place d'un traitant pour recuperer les fils zombies*/
		memset(&sigchld_action, 0, sizeof (sigchld_action));
		sigchld_action.sa_handler = &sigchld_handler;

		/* Gère SIGCHLD en appelent sigchld_handler. */
		sigaction(SIGCHLD, &sigchld_action, NULL);

		/* lecture du fichier de machines */
		/* 1- on recupere le nombre de processus a lancer */
		/* 2- on recupere les noms des machines : */
		tab_name = set_data_from_file("machine_file", tab_name,&num_procs);
		/* la machine est un des elements d'identification */
		/* creation de la socket d'ecoute */
		port_serv = initListeningSocket(&sock_serv, num_procs, hostname);
		/* + ecoute effective */
		printf("fd sock_serv %d\n", sock_serv);




		/* creation des fils */
		for(i = 0; i < num_procs ; i++) {

			/* creation du tube pour rediriger stdout */
			if (pipe(tube_stdout) == -1) {
				perror("Le pipe stdout n'a pas fonctionné");
			}

			/* creation du tube pour rediriger stdout */
			if (pipe(tube_stderr) == -1) {
				perror("Le pipe stderr n'a pas fonctionné");
			}


			pid = fork();
			if(pid == -1) ERROR_EXIT("fork");

			if (pid == 0) { /* fils */

				close(sock_serv); // le fils n'a pas besoin de la socket d'ecoute du pere serveur
				/* redirection stdout */


				close(tube_stdout[0]);          /* Fermeture du coté lecture non utilisé par le fils */
				dup2(tube_stdout[1],STDOUT_FILENO);     /* On affecte stdout à l'extrémité 1 du tube (écriture)*/

				/* redirection stderr */
				close(tube_stderr[0]);          /* Fermeture du coté lecture non utilisé par le fils */
				dup2(tube_stderr[1],STDERR_FILENO);     /* On affecte stderr à l'extrémité 1 du tube (écriture)*/

				/* Creation du tableau d'arguments pour le ssh */

				newargv1 = argv+2;

				newargv2 = init_SshArg(tab_name[i],port_serv,hostname,argc,argv);


				int j =0;
				for (j= 0; j< 4+argc-2+1; j++){
					fprintf(stdout,"test %s\n", newargv2[j]);
				}
				fprintf(stdout,"bien affiche \n");
				//fprintf(stdout,"len %d\n", 5 + argc-2+1);
				//fprintf(stdout,"test %s\n", newargv2[7]);
				// laisser le temps de tout afficher
				//sleep(2);

				/* jump to new prog : */
				execvp("ssh",newargv2);

			} else  if(pid > 0) { /* pere */

				close(tube_stdout[1]);          /* Fermeture du coté écriture non utilisé par le pere */
				close(tube_stderr[1]);          /* Fermeture du coté écriture non utilisé par le pere */



				num_procs_creat++;
			}
		}


		for(i = 0; i < num_procs ; i++){

			/* on accepte les connexions des processus dsm */

			/* ACCEPT SOCKET */

			//accept connection from client
			sock_acc = accept(sock_serv, (struct sockaddr*) & addr_acc, &addr_acc_len);
			if (sock_acc < 0)
			ERROR_EXIT("Erreur acceptation");

			if (read(sock_acc, buffer, SIZE_MSG) < 0){
				ERROR_EXIT("Erreur read");
			}
			printf("read : %s\n", buffer);
			memset(buffer, '\0', SIZE_MSG);
			if (read(sock_acc, buffer, SIZE_MSG) < 0){
				ERROR_EXIT("Erreur read");
			}
			printf("read : %s\n", buffer);
			/*  On recupere le nom de la machine distante */
			/* 1- d'abord la taille de la chaine */
			/* 2- puis la chaine elle-meme */


			/* On recupere le pid du processus distant  */

			/* On recupere le numero de port de la socket */
			/* d'ecoute des processus distants */
		}

		/* envoi du nombre de processus aux processus dsm*/

		/* envoi des rangs aux processus dsm */

		/* envoi des infos de connexion aux processus */

		/* gestion des E/S : on recupere les caracteres */
		/* sur les tubes de redirection de stdout/stderr */
		/* while(1)
		{
		je recupere les infos sur les tubes de redirection
		jusqu'à ce qu'ils soient inactifs (ie fermes par les
		processus dsm ecrivains de l'autre cote ...)*/

		/* cette boucle while permet de lire les caractères présent sur le coté lecture des tube et les afficher */
		while (1){
			i = 0;
			if(i<num_procs){
				if (read(tube_stdout[0], sortie_out, sizeof(sortie_out)) > 0){
					printf("[Proc %d : %s : stdout]  \n%s\n",i+1,tab_name[i], sortie_out);
					memset(&sortie_out,'\0',strlen(sortie_out));
				}

				if (read(tube_stderr[0], sortie_err, sizeof(sortie_err)) > 0){
					printf("[Proc %d : %s : stderr] \n%s\n",i+1,tab_name[i], sortie_err);
					memset(&sortie_err,'\0',strlen(sortie_err));
				}
			}
			i++;
		}


		close(tube_stdout[0]); // fermeture du coté lecture du tube une fois la lecture une fois que la fonction read a renvoyé 0 : EOF, la lecture terminée

		/*Affichage de la sortie des processus fils*/
		printf("\n\n");
		//Nettoie les chaines de caractères utilisées pour afficher les sortie des processus fils
		memset(sortie_out,'\0',strlen(sortie_err));
		memset(sortie_out,'\0',strlen(sortie_err));
		wait(NULL);

		//};


		/* on attend les processus fils */

		/* on ferme les descripteurs proprement */

		/* on ferme la socket d'ecoute */
	}
	exit(EXIT_SUCCESS);
}
