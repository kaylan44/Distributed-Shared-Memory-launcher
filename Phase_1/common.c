#include "common_impl.h"

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


struct sockaddr_in init_serv_addr(){
  struct sockaddr_in serv_add;
  serv_add.sin_family = AF_INET;
  serv_add.sin_addr.s_addr = INADDR_ANY;
  serv_add.sin_port = 0 ;
  return serv_add;
}

int initListeningSocket(int* sock, int num_procs){
    int port;
    //create the socket, check for validity!
    *sock = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (*sock < 0)
      ERROR_EXIT("erreur socket");

    //init the serv_add structure
    struct sockaddr_in serv_add = init_serv_addr();

    //perform the binding
    //we bind on the tcp port specified
    if(bind(*sock, (struct sockaddr*) & serv_add, sizeof(serv_add))<0 ){
      ERROR_EXIT("erreur bind");
    }

    //specify the socket to be a server socket and listen for at most 20 concurrent client
    if(listen(*sock, num_procs) < 0){
      ERROR_EXIT("erreur listen");
    }


    if (getsockname(sock, (struct sockaddr *)& serv_add, &len) == -1)
        ERROR_EXIT("getsockname");
    port = ntohs(serv_add.sin_port));


    printf("Père en écoute \n");
    return port;
}
