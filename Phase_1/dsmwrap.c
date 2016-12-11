#include "common_impl.h"

int main(int argc, char **argv)

{
   struct sockaddr_in sock_host;
   char hostname[SIZE_MSG];
   char machine[SIZE_MSG];
   char len_machine[SIZE_MSG];

   int port = atoi(argv[1]);
   int sock;
   char pid[SIZE_MSG];

   sprintf(hostname,"%s",argv[2]);
   sprintf(pid,"%d",getpid());



   int len_newargv=argc-3;

   char ** newargv=malloc (len_newargv * sizeof(char*));

   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   newargv = argv+3;
/*
    int j =0;
    for (j= 0; j< len_newargv; j++){
      fprintf(stdout,"dsmwrap %s\n", newargv[j]);
    }
    */

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */

   /* necessaires pour la phase dsm_init */
    sock = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0){
      ERROR_EXIT("erreur socket");
    }
    //connect to remote socket

    //sleep(2);
    do_connect(sock,hostname, port, &sock_host);
    fprintf(stdout,"passe connect\n");

    gethostname(machine, SIZE_MSG);
    fprintf(stdout,"\n");
    fprintf(stdout,"port:%d de %s\n", port, machine);
    fprintf(stdout,"hostname:%s de %s\n", hostname, machine);
    fprintf(stdout,"pid:%s de %s\n", pid, machine);
    fprintf(stdout,"\n");



   /* Envoi du nom de machine au lanceur */
   sprintf(len_machine, "%d", strlen(machine));
   if (write(sock, len_machine, SIZE_MSG) < 0){
      ERROR_EXIT("erreur write");
   }

   if (write(sock, machine, SIZE_MSG) < 0){
      ERROR_EXIT("erreur write");
   }

   /* Envoi du pid au lanceur */
   if (write(sock, pid,strlen(pid)) < 0){
      ERROR_EXIT("erreur write");
  }
   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   fflush(stdout);
   fflush(stderr);

   /* on execute la bonne commande */
   execvp("Semestre_7/PR204/Phase_1/bin/truc",newargv);
   return 0;
}
