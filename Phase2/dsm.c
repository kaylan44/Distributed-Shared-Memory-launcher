#include "dsm.h"
#include "common_impl.h"

int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;  /* rang (= numero) du processus */

/* indique l'adresse de debut de la page de numero numpage */
static char *num2address( int numpage )
{
    char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));

    if( pointer >= (char *)TOP_ADDR ){
        fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
        return NULL;
    }
    else return pointer;
}

/* fonctions pouvant etre utiles */
static void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
    if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {
        if (state != NO_CHANGE )
        table_page[numpage].status = state;
        if (owner >= 0 )
        table_page[numpage].owner = owner;
        return;
    }
    else {
        fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
        return;
    }
}

static dsm_page_owner_t get_owner( int numpage)
{
    return table_page[numpage].owner;
}

static dsm_page_state_t get_status( int numpage)
{
    return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
static void dsm_alloc_page( int numpage )
{
    char *page_addr = num2address( numpage );
    mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return ;
}

/* Changement de la protection d'une page */
static void dsm_protect_page( int numpage , int prot)
{
    char *page_addr = num2address( numpage );
    mprotect(page_addr, PAGE_SIZE, prot);
    return;
}

static void dsm_free_page( int numpage )
{
    char *page_addr = num2address( numpage );
    munmap(page_addr, PAGE_SIZE);
    return;
}

static void *dsm_comm_daemon( void *arg)
{
    while(1)
    {
        /* a modifier */
        printf("[%i] Waiting for incoming reqs \n", DSM_NODE_ID);
        sleep(2);
    }
    return;
}

static int dsm_send(int dest,void *buf,size_t size)
{
    /* a completer */
}

static int dsm_recv(int from,void *buf,size_t size)
{

}

static void dsm_handler( void )
{
    /* A modifier */
    printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
    abort();
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context)
{
    /* A completer */
    /* adresse qui a provoque une erreur */
    void  *addr = info->si_addr;
    /* Si ceci ne fonctionne pas, utiliser a la place :*/
    /*
    #ifdef __x86_64__
    void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
    #elif __i386__
    void *addr = (void *)(context->uc_mcontext.cr2);
    #else
    void  addr = info->si_addr;
    #endif
    */
    /*
    pour plus tard (question ++):
    dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;
    */
    /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
    void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

    if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
    {
        dsm_handler();
    }
    else
    {
        /* SIGSEGV normal : ne rien faire*/
    }
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv)
{
    struct sigaction act;
    int index;

    char *proc_array2char = NULL;
    dsm_proc_t *proc_array = NULL;
    int sock_dsmexec;
    int sock_p2p_listen;
    int i;
    int *sock_acc;
    struct sockaddr_in addr_acc;
    socklen_t addr_acc_len = sizeof(addr_acc);
    struct sockaddr_in sock_host;
    int *sock_connect_dsm;

    char msg[SIZE_MSG]; // Test envoi de messages entre processus distants

    sock_dsmexec = atoi(argv[argc-2]);
    sock_p2p_listen = atoi(argv[argc-1]);

    /* reception du nombre de processus dsm envoye */
    /* par le lanceur de programmes (DSM_NODE_NUM)*/
    recv_msg(sock_dsmexec, &DSM_NODE_NUM, sizeof(int));

    proc_array2char = malloc(sizeof(dsm_proc_t) * DSM_NODE_NUM);
    proc_array = malloc(sizeof(dsm_proc_t) * DSM_NODE_NUM);
    sock_acc=malloc(DSM_NODE_NUM*sizeof(int));
    sock_connect_dsm = malloc(DSM_NODE_NUM * sizeof(int));

    /* reception de mon numero de processus dsm envoye */
    /* par le lanceur de programmes (DSM_NODE_ID)*/
    recv_msg(sock_dsmexec, &DSM_NODE_ID, sizeof(int));

    /* reception des informations de connexion des autres */
    /* processus envoyees par le lanceur : */
    /* nom de machine, numero de port, etc. */
    recv_msg(sock_dsmexec, proc_array2char, sizeof(dsm_proc_t) * DSM_NODE_NUM);
    memcpy(proc_array, proc_array2char, sizeof(dsm_proc_t) * DSM_NODE_NUM);

    /* initialisation des connexions */
    /* avec les autres processus : connect/accept */
    for (i = 0; i < DSM_NODE_NUM; i++){
        if (DSM_NODE_ID != i ){// Attention on ne veut pas lire nos propres infos

            sock_connect_dsm[i] = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
            if (sock_connect_dsm[i] < 0){
                ERROR_EXIT("erreur socket")
            }
            /* demande de connection au processus distant */
            do_connect(sock_connect_dsm[i], proc_array[i].connect_info.machine, proc_array[i].connect_info.listenning_port);
        }
    }

    for (i = 0; i < DSM_NODE_NUM; i++){
        if (DSM_NODE_ID != i ){// Attention on ne veut pas lire nos propres infos
            /* acceptation des processus distants */
            do{
                sock_acc[i] = accept(sock_p2p_listen, (struct sockaddr*) & addr_acc, &addr_acc_len);
            }
            while(errno == EINTR );

        }
    }
    // TEST de Connexion
    // Le processus 0 envoie des messages aux restes des processus
    if(DSM_NODE_ID == 0){
        for(i = 0; i < DSM_NODE_NUM; i++){
            if (DSM_NODE_ID != i ){
                sprintf(msg, "%s", "Test d'envoi de message inter processus dsm");
                send_msg(sock_connect_dsm[i], msg,SIZE_MSG);
            }
        }
    }
    else{
        recv_msg(sock_acc[0], msg, SIZE_MSG);
        fprintf(stdout,"sockp2p dsm écrit: %s\n", msg);
    }
    // FIN de test Connexion

    // /* Allocation des pages en tourniquet */
    // for(index = 0; index < PAGE_NUMBER; index ++){
    //   if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
    //     dsm_alloc_page(index);
    //   dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
    // }
    //
    // /* mise en place du traitant de SIGSEGV */
    // act.sa_flags = SA_SIGINFO;
    // act.sa_sigaction = segv_handler;
    // sigaction(SIGSEGV, &act, NULL);
    //
    // /* creation du thread de communication */
    // /* ce thread va attendre et traiter les requetes */
    // /* des autres processus */
    // pthread_create(&comm_daemon, NULL, dsm_comm_daemon, NULL);

    /* Adresse de début de la zone de mémoire partagée */
    return ((char *)BASE_ADDR);
}

void dsm_finalize( void )
{
    /* fermer proprement les connexions avec les autres processus */

    /* terminer correctement le thread de communication */
    /* pour le moment, on peut faire : */
    pthread_cancel(comm_daemon);

    return;
}
