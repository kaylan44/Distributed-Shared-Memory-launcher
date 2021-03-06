#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SIZE_MSG 70
#define ARGV_SIZE 400
/* autres includes (eventuellement) */

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   int listenning_port;
   char machine[SIZE_MSG];
   /* a completer */
};
typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

int creer_socket(int type, int *port_num);


// Retourne le nouveau vecteur d'argument pour SSH
char ** init_SshArg(char* name,int port_serv,char* addr_ip, int argc, char ** argv);

// 	Récupère les différentes infos du fichier se trouvant a chemin "path"
char **set_data_from_file(char* path, char** machine,int *num);


//Connect the sockeet with the informations in arg
void do_connect(int sock, char* hostname, int port);

int initListeningSocket(int* sock, int num_procs, char* hostname);

void send_msg(int to, void *buf, size_t to_send);

void recv_msg(int from, void *buf, size_t to_recv);
