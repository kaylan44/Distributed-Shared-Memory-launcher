#include "common_impl.h"

#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>


char **set_data_from_file(char* path, char** machine, int *num){
    // On recupere le nom des machine dans path
	int i = 0;
    int n = 0;
	FILE* fp;
	char buffer[SIZE_MSG];
	memset(buffer,0,SIZE_MSG);

	fp = fopen(path, "r"); // On ouvre le fichier en lecture seulement

    //On compte le nombre de machine dans le fichier
    // Lit SIZE_MSG charatecter au plus.
    // Stop si EOF ou nouvelle ligne avec NULL comme retour
	while(fgets(buffer, SIZE_MSG, fp)) {
		(*num)++;
	}

    //fseek(fp, 0, SEEK_SET); // equivalent
	rewind(fp); // Remet au debut la position courante du fichier
    // on peut desormais allouer la memoire au tableau stockant le nombre de machine de facon exacte
    machine = malloc(*num * sizeof(char*));

	while(fgets(buffer, SIZE_MSG, fp)) {
		*(machine + i) = malloc (SIZE_MSG * sizeof(char));
        //sprintf(*(machine + i), "%s", buffer);
        sprintf(machine[i], "%s", buffer);
        n = strlen(machine[i]);
        // on enleve le saut de ligne récupéré
        if (machine[i][n-1]=='\n'){
            machine[i][n-1] = '\0';
        }
        i++;
	}

	fclose(fp);
	return machine;

}

// Private
struct sockaddr_in init_listenner_addr(char* hostname){

    // ecrit le nom de machine dans hostname
    // Il faut qu'il soit plus petit que SIZE_MSG sinon troncature
    gethostname(hostname, SIZE_MSG);
    struct hostent* h;
    h = gethostbyname(hostname);

    struct sockaddr_in listenner_add;
    listenner_add.sin_family = AF_INET; // Internet Protocol v4 addresses
    // char *h_addr est synonyme de h_addr_list[0]
    memcpy(&listenner_add.sin_addr, h->h_addr_list[0], h->h_length);
    listenner_add.sin_port = 0 ; // attribution du port de facon automatique
    return listenner_add;
}

int initListeningSocket(int* sock, int num_procs, char* hostname){
    int port;

    // creation de la socket, on regarde sa validité
    *sock = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (*sock < 0){
      ERROR_EXIT("erreur socket");
    }

    // initialisation de la structure struct sockaddr_in du listenner
    struct sockaddr_in listenner_add = init_listenner_addr(hostname);
    socklen_t len = sizeof(listenner_add);

    //perform the binding
    //we bind on the tcp port specified
    if(bind(*sock, (struct sockaddr*) & listenner_add, sizeof(listenner_add))<0 ){
      ERROR_EXIT("erreur bind");
    }

    //specify the socket to be a listennerer socket and listen for at most 20 concurrent client
    if(listen(*sock, num_procs) < 0){
      ERROR_EXIT("erreur listen");
    }

    // permet de recuperer les informations associant la socket a la structre listenner_add
    // notamment le port atttribuer aléatoirement
    if (getsockname(*sock, (struct sockaddr *)& listenner_add, &len) == -1){
        ERROR_EXIT("getsockname");
    }
    port = htons(listenner_add.sin_port); // from host byte order to network byte order .

    return port;
}

char ** init_SshArg(char* name,int port_serv,char* addr_ip, int argc, char ** argv){

	int i;
	char ** newargv = NULL;
    // en entrée argv = ./bin/dsmexec machine_file exemple arg1

    // (argc - 2) =  nombre d'arguement utile -> exemple + argv1 argv2 ...
    // 5 = ssh toto dsmwrap port ip
	int len_newargv = 5 + argc - 2;

	newargv = malloc(len_newargv * sizeof(char*));

    // on alloue la memoire pour chacun des éléments du nouveau nawargv
    for (i = 0; i<len_newargv;i++){
        newargv[i] = malloc (SIZE_MSG * sizeof(char));
    }

    sprintf(newargv[0],"%s","ssh");
    sprintf(newargv[1],"%s",name);
    sprintf(newargv[2],"%s","Documents/2A/PR204/PR204/Phase2/bin/dsmwrap");
    //sprintf(newargv[2],"%s","Semestre_7/PR204/Phase2/bin/dsmwrap");
    sprintf(newargv[3],"%d",port_serv);
    sprintf(newargv[4],"%s",addr_ip);


	// "-2" car on a le nom du prog et machinefile en 2 premiers arguments
    // on ajoute ici exemple arg1 argv2 ....
	for (i=0;i<argc-2;i++){
        sprintf(newargv[5+i],"%s",argv[i+2]);
	}
    newargv[len_newargv] = NULL;

	return newargv;
}


void do_connect(int sock, char* hostname, int port, struct sockaddr_in* sock_host ){
    // Avec la socket sock on se connect avec la machine hostname
    memset(sock_host, '\0', sizeof(*sock_host));
    sock_host->sin_family = AF_INET;
    sock_host->sin_port = htons(port); // from host byte order to network byte order .

    struct hostent* h;
    h = gethostbyname(hostname);

    // char *h_addr est synonyme de h_addr_list[0]
    memcpy(&sock_host->sin_addr, h->h_addr_list[0], h->h_length);

    if (connect(sock, (struct sockaddr*)  sock_host, sizeof(*sock_host)) < 0)
       ERROR_EXIT("erreur connect");

}
