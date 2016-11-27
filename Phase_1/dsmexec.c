#include "common_impl.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL; 

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
   /* on traite les fils qui se terminent */
   /* pour eviter les zombies */
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
		char **newargv;
		int tube_redirect[2];

		int nb_char=0;
 		char buf, sortie[10000];

		/* Mise en place d'un traitant pour recuperer les fils zombies*/      
		/* XXX.sa_handler = sigchld_handler; */

		/* lecture du fichier de machines */
		/* 1- on recupere le nombre de processus a lancer */
		/* 2- on recupere les noms des machines : le nom de */
		/* la machine est un des elements d'identification */

		/* creation de la socket d'ecoute */
		/* + ecoute effective */ 


		//Test Guillaume
		num_procs=3;



		/* creation des fils */
		for(i = 0; i < num_procs ; i++) {

			/* creation du tube pour rediriger stdout */
			if (pipe(tube_redirect) == -1) {
               perror("Le pipe stdoutn'a pas fonctionné");
           	}


			pid = fork();
			if(pid == -1) ERROR_EXIT("fork");

			
			if (pid == 0) { /* fils */	
			   
			   /* redirection stdout */	      
			   
				close(tube_redirect[0]);          /* Fermeture du coté lecture non utilisé par le fils */
                //dup2(tube_redirect[1],STDOUT_FILENO);     /* On affecte stdout à l'extrémité 1 du tube (écriture)*/

			   /* redirection stderr */

                dup2(tube_redirect[1],STDERR_FILENO);     /* On affecte stderr à l'extrémité 1 du tube (écriture)*/	      	      
			   
			   /* Creation du tableau d'arguments pour le ssh */ 
			   newargv = argv+2;
			   
			   /* jump to new prog : */
			   execvp("bin/truc",newargv);

			} else  if(pid > 0) { /* pere */		      
			   
			    close(tube_redirect[1]);          /* Fermeture du coté écriture non utilisé par le pere */
 				
        		/* cette boucle while permet de lire les caractères présent sur le coté lecture du tube et les stocker dans la chaine sortie */
                while (read(tube_redirect[0], &buf, 1) > 0){
           			sortie[nb_char]=buf; // rempli la chaine sortie caractère par caractère
           			nb_char++;
           		}


       			sortie[nb_char]='\0'; // indique la fin de la chaine de caractère


            	close(tube_redirect[0]); // fermeture du coté lecture du tube une fois la lecture une fois que la fonction read a renvoyé 0 : EOF, la lecture terminée
            	printf("sortie :\n%s \n\n", sortie);

            	//Nettoie les chaines de caractères utilisées pour afficher les sortie des processus fils
            	nb_char=0;
            	memset(&sortie,'\0',strlen(sortie));
            	memset(&buf,'\0',strlen(sortie));

 				wait(NULL);
			   	num_procs_creat++;	      
			}
		}


		for(i = 0; i < num_procs ; i++){

		/* on accepte les connexions des processus dsm */

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
		processus dsm ecrivains de l'autre cote ...)

		};
		*/

		/* on attend les processus fils */

		/* on ferme les descripteurs proprement */

		/* on ferme la socket d'ecoute */
		}   
		exit(EXIT_SUCCESS);  
}

