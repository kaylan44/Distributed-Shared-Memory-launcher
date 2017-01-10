#include "common_impl.h"

/* processus intermediaire pour "nettoyer" la liste des arguments qu'on va passer */
/* à la commande a executer vraiment */
int main(int argc, char **argv){
    char hostname_dsmexec[SIZE_MSG];
    char machine[SIZE_MSG];
    int len_machine;

    int pid;
    int sock_dsmexec;
    int port_dsmexec = atoi(argv[1]);
    int port_p2p;
    int sock_p2p_listen;
    struct sockaddr_in sock_host;

    int len_newargv=argc-3 + 2;
    // -3 on supprime ce qui ne nous interesse pas
    // +2 on veut rajouter la sock p2p et la sock dsmexec
    char ** newargv=malloc (len_newargv * sizeof(char*));

    /* creation d'une socket pour se connecter au lanceur et envoyer/recevoir les infos */
    /* necessaires pour la phase dsm_init */
    sock_dsmexec = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (sock_dsmexec < 0){
        ERROR_EXIT("erreur socket");
    }
    // on rajoute la socket en avant derniere position
    newargv = argv+3;
    newargv[len_newargv-2] = malloc(sizeof(int));
    sprintf(newargv[len_newargv-2],"%d", sock_dsmexec);
    newargv[len_newargv] = NULL;

    sprintf(hostname_dsmexec,"%s",argv[2]);
    pid = getpid();

    //connection avec dsmexec
    do_connect(sock_dsmexec,hostname_dsmexec, port_dsmexec, &sock_host);
    gethostname(machine, SIZE_MSG);

    fprintf(stdout,"nom de machine: %s \n", machine);
    fprintf(stdout,"pid: %d \n\n", pid);

    /*1- Envoi du nom de machine au lanceur */
    len_machine =  strlen(machine);
    if (write(sock_dsmexec, &len_machine, sizeof(int)) < 0){
        ERROR_EXIT("erreur write");
    }
    if (write(sock_dsmexec, machine, len_machine) < 0){
        ERROR_EXIT("erreur write");
    }

    /*2- Envoi du pid au lanceur */
    if (write(sock_dsmexec, &pid,sizeof(int)) < 0){
        ERROR_EXIT("erreur write");
    }

    /* Creation de la socket d'ecoute pour les connexions avec les autres processus dsm */
    port_p2p =  initListeningSocket(&sock_p2p_listen, 3, machine);
    newargv[len_newargv-1] = malloc(sizeof(int));
    sprintf(newargv[len_newargv-1],"%d",sock_p2p_listen);

    /*3- Envoi du numero de port au lanceur pour qu'il le propage à tous les autres processus dsm */
    if (write(sock_dsmexec, &port_p2p, sizeof(int)) < 0){
        ERROR_EXIT("erreur write");
    }

    /* on execute la bonne commande */
    execvp("Documents/2A/PR204/PR204/Phase2/bin/exemple",newargv);
    //execvp("Semestre_7/PR204/Phase2/bin/exemple",newargv);

    return 0;
}
