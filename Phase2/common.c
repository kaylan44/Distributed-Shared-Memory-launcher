#include "common_impl.h"

#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int creer_socket(int prop, int *port_num)
{
   int fd = 0;

   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */

   return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */


char **set_data_from_file(char* path, char** machine,int *num){

	int i=0;
    int n = 0;
	FILE* fp;
	char buffer[SIZE_MSG];
	memset(buffer,0,SIZE_MSG);

	fp = fopen(path, "r");

	while(fgets(buffer, SIZE_MSG,fp)) {
		(*num)++;
	}

    //fseek(fp, 0, SEEK_SET);
	rewind(fp);
	machine = malloc(*num * sizeof(char*));

	while(fgets(buffer, SIZE_MSG, fp)) {
		*(machine + i) = malloc (SIZE_MSG * sizeof(char));
        sprintf(*(machine + i), "%s", buffer);
        n = strlen(machine[i]);
        if (machine[i][n-1]=='\n'){
            machine[i][n-1] = '\0';
        }
        i++;
	}

	fclose(fp);
	return machine;

}


struct sockaddr_in init_serv_addr(char* hostname){


    gethostname(hostname, 1023);
    struct hostent* h;
    h = gethostbyname(hostname);

    struct sockaddr_in serv_add;
    serv_add.sin_family = AF_INET;
    //serv_add.sin_addr.s_addr = INADDR_ANY;
    //synonyme char *h_addr of h_addr_list[0
    memcpy(&serv_add.sin_addr, h->h_addr_list[0], h->h_length);
    serv_add.sin_port = 0 ;
    return serv_add;
}

int initListeningSocket(int* sock, int num_procs, char* hostname){
    int port;
    //create the socket, check for validity!
    *sock = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (*sock < 0)
      ERROR_EXIT("erreur socket");

    //init the serv_add structure
    struct sockaddr_in serv_add = init_serv_addr(hostname);
    socklen_t len = sizeof(serv_add);

    //perform the binding
    //we bind on the tcp port specified
    if(bind(*sock, (struct sockaddr*) & serv_add, sizeof(serv_add))<0 ){
      ERROR_EXIT("erreur bind");
    }

    //specify the socket to be a server socket and listen for at most 20 concurrent client
    if(listen(*sock, num_procs) < 0){
      ERROR_EXIT("erreur listen");
    }


    if (getsockname(*sock, (struct sockaddr *)& serv_add, &len) == -1)
        ERROR_EXIT("getsockname");
    port = ntohs(serv_add.sin_port);



    printf("Père en écoute \n");
    return port;
}

char ** init_SshArg(char* name,int port_serv,char* addr_ip, int argc, char ** argv){

	int i;
	char ** newargv = NULL;

    // (argc - 2) = truc + argv1 argv2 ...
    // 5 = ssh toto dsmwrap port ip
    // +1 = NULL
	int len_newargv = 4 + argc - 2 +1;
	newargv = malloc(len_newargv * sizeof(char*));
    for (i = 0; i<len_newargv;i++){
        newargv[i] = malloc (SIZE_MSG * sizeof(char));
    }

    sprintf(newargv[0],"%s","ssh");
    sprintf(newargv[1],"%s",name);
    //sprintf(newargv[2],"%s","Documents/2A/PR204/PR204/Phase2/bin/dsmwrap");
    sprintf(newargv[2],"%s","Semestre_7/PR204/Phase2/bin/dsmwrap");
    sprintf(newargv[3],"%d",port_serv);
    sprintf(newargv[4],"%s",addr_ip);
    //sprintf(newargv[5],"%s","bin/truc");

	// "-2" car on a le nom du prog et machinefile en 2 premiers arguments
	for (i=0;i<argc-2;i++){
        sprintf(newargv[5+i],"%s",argv[i+2]);
	}
    newargv[len_newargv] = NULL;
	return newargv;
}


void do_connect(int sock, char* hostname, int port, struct sockaddr_in* sock_host ){

    memset(sock_host, '\0', sizeof(*sock_host));
    sock_host->sin_family = AF_INET;
    sock_host->sin_port = htons(port);

    struct hostent* h;
    h = gethostbyname(hostname);

    //serv_add.sin_addr.s_addr = INADDR_ANY;
    //synonyme char *h_addr of h_addr_list[0
    memcpy(&sock_host->sin_addr, h->h_addr_list[0], h->h_length);

    //inet_aton(hostname, & sock_host->sin_addr);

    if (connect(sock, (struct sockaddr*)  sock_host, sizeof(*sock_host)) < 0)
       ERROR_EXIT("erreur connect");

}
