#include "common_impl.h"

int main(int argc, char **argv)

{
    struct sockaddr_in sock_host;
    char hostname[SIZE_MSG];
    char machine[SIZE_MSG];
    int len_machine;


    int port = atoi(argv[1]);
    int sock;
    int pid;
    int port_p2p;
    int sock_p2p_listen;


    int len_newargv=argc-3 + 2; // -3 on supprime ce qui ne nous interesse pas
                                // +2 on veut rajouter la sock p2p et la cock connectée à dsmexec
    char ** newargv=malloc (len_newargv * sizeof(char*));

    /* processus intermediaire pour "nettoyer" */
    /* la liste des arguments qu'on va passer */
    /* a la commande a executer vraiment */
    newargv = argv+3;

    /* creation d'une socket pour se connecter au */
    /* au lanceur et envoyer/recevoir les infos */

    /* necessaires pour la phase dsm_init */
    sock = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0){
        ERROR_EXIT("erreur socket");
    }
    // on rajoute la socket en dernier position
    newargv[len_newargv-2] = malloc(sizeof(int));
    sprintf(newargv[len_newargv-2],"%d", sock);
    newargv[len_newargv] = NULL;

    fprintf(stdout,"sock1:%s \n", newargv[len_newargv-1]);


    sprintf(hostname,"%s",argv[2]);
    pid = getpid();

    //connect to remote socket
    do_connect(sock,hostname, port, &sock_host);

    gethostname(machine, SIZE_MSG);
    fprintf(stdout,"\n");
    fprintf(stdout,"port:%d de %s\n", port, machine);
    fprintf(stdout,"hostname:%s de %s\n", hostname, machine);
    fprintf(stdout,"pid:%d de %s\n", pid, machine);
    fprintf(stdout,"\n");

    /* Envoi du nom de machine au lanceur */
    len_machine =  strlen(machine);
    if (write(sock, &len_machine, sizeof(int)) < 0){
        ERROR_EXIT("erreur write");
    }
    if (write(sock, machine, len_machine) < 0){
        ERROR_EXIT("erreur write");
    }

    /* Envoi du pid au lanceur */
    if (write(sock, &pid,sizeof(int)) < 0){
        ERROR_EXIT("erreur write");
    }

    /* Creation de la socket d'ecoute pour les */
    /* connexions avec les autres processus dsm */
    port_p2p =  initListeningSocket(&sock_p2p_listen, 3, machine);
    newargv[len_newargv-1] = malloc(sizeof(int));
    sprintf(newargv[len_newargv-1],"%d",sock_p2p_listen);

    /* Envoi du numero de port au lanceur */
    /* pour qu'il le propage à tous les autres */
    /* processus dsm */
    if (write(sock, &port_p2p, sizeof(int)) < 0){
        ERROR_EXIT("erreur write");
    }

    fflush(stdout);
    fflush(stderr);


    FILE* fichier = NULL;
    fichier = fopen("Documents/2A/PR204/PR204/Phase2/titi", "a");
    if (fichier == NULL)
    {
        ERROR_EXIT("erreur write");
    }
    fputs("\n\n", fichier);
    //fputs(buffer, fichier);
    fflush(fichier);
    fclose(fichier);

    /* on execute la bonne commande */
    execvp("Documents/2A/PR204/PR204/Phase2/bin/exemple",newargv);


    return 0;
/*

   execvp("Semestre_7/PR204/Phase_1/bin/truc",newargv);
   return 0;
*/
}
