#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#define MAX_NAME 10
#define MSJ_SIZE 100
 
struct data {
    int fdsock;
    uint32_t  address;
};

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct maxint {
    uint16_t max ;
    char pseudo[MAX_NAME+1] ;
    uint32_t ipaddress;
};

struct maxint maxserver = {.max = 0, .ipaddress = 0};
 
//this function allows to update the value of the max with the data of the client
int isMax(struct data *datas, char* pseudo, char * recevmess){
    char sentmess[MSJ_SIZE];
    uint16_t numbersent = 0;
    
    numbersent = (uint16_t)(atoi(recevmess));
    numbersent = ntohs(numbersent);

    pthread_mutex_lock(&lock);
    if(maxserver.max <= numbersent){
        maxserver.max = numbersent;
        strncpy(maxserver.pseudo,pseudo,MAX_NAME+1);
        maxserver.ipaddress = datas->address;   
    }
    pthread_mutex_unlock(&lock);

    sprintf(sentmess,"INTOK\n");
    if(send(datas->fdsock,sentmess,strlen(sentmess),0) == -1){
    perror("error send isMax ");
    return -1;
    }

    return 0;
}

// this function sends the information of the global structure server max
int sendMax(int sock ){
    char sentmess[MSJ_SIZE];
    if(maxserver.max == 0){
       sprintf(sentmess,"NOP\n");
       if(send(sock,sentmess,strlen(sentmess),0) == -1){
            perror("error send sendMax nop");
            return -1;
        } 
    }else{ 
        uint32_t  ipaddrsent = htonl(maxserver.ipaddress);
        uint16_t max = htons(maxserver.max);

        sprintf(sentmess,"REP%s",maxserver.pseudo);
        if(send(sock,sentmess,strlen(sentmess),0) == -1){
            perror("error sendMax rep");
            return -1;
        }

        if(send(sock,&ipaddrsent,sizeof(ipaddrsent),0) == -1){
            perror("erreur sendMax ip address");
            return -1;
        }

        if(send(sock,&max,sizeof(max),0) == -1){
            perror("erreur sendMax max");
            return -1;
        }
    }
     return 0;
}
void * maxint (void * information ){
    int size_recv;
    char sentmess[MSJ_SIZE];
    char recevmess[MSJ_SIZE];
    char *pseudocli = malloc((MAX_NAME+1)*sizeof(char));
    struct data *datas = (struct data *)information; 
    int sockt = datas->fdsock;

    size_recv = recv(sockt,pseudocli,MAX_NAME*sizeof(char),0);
    if(size_recv == -1){
        perror("error recv pseudo");
        exit(EXIT_FAILURE);
    }

    pseudocli[size_recv+1]='\0';
    
    sprintf(sentmess,"HELLO %s",pseudocli);
    if(send(sockt,sentmess,size_recv+7,0) == -1){
        perror("error send hello pseudo");
        exit(EXIT_FAILURE);
    }

     //depending on the string 'MAX' or 'INT' we call the corresponding functions 
    size_recv = recv(sockt,recevmess,3*sizeof(char),0);
    if(size_recv == -1){
        perror("error recv INT/MAX");
        exit(EXIT_FAILURE);
    }

    if(strcmp(recevmess,"INT") == 0){

        // if it's INT we get the number
        size_recv = recv(sockt,recevmess,sizeof(recevmess),0);
        if(size_recv == -1){
            perror("erreur recv INT number");
            exit(EXIT_FAILURE);
        }

        if(isMax(datas, pseudocli, recevmess) == -1){
            perror("erreur isMax");
            exit(EXIT_FAILURE);
        }

    }else if (strcmp(recevmess,"MAX") == 0){

        if(sendMax(sockt) == -1){
            perror("erreur sendMax");
            exit(EXIT_FAILURE);
        }
    }else{
        sprintf(sentmess,"two words accepted: INT or MAX");
        if(send(sockt,sentmess,strlen(sentmess),0) == -1){
            perror("error send error mess");
            exit(EXIT_FAILURE);
        }
    }
    
    free(pseudocli);
    return NULL;
}


int main (int argc, char *argv[]){

    int sock = socket(PF_INET,SOCK_STREAM,0);
    if(sock == -1){
        perror("error creation of socket 1");
        return EXIT_FAILURE;
    }

    int port = *((int *)argv[1]);
    struct sockaddr_in address_sock;
    address_sock.sin_family=AF_INET;
    address_sock.sin_port=htons(port);
    address_sock.sin_addr.s_addr=htonl(INADDR_ANY);

    if (bind(sock,(struct sockaddr *)&address_sock,sizeof(struct sockaddr_in)) == -1){
        perror("error bind");
        return EXIT_FAILURE;
    }

    if(listen(sock,0) == -1){
        perror("error listen");
        return EXIT_FAILURE;
    }
    
    while(1){

        struct sockaddr_in *caller = malloc(sizeof(struct sockaddr_in));
        if(caller == NULL){
            perror("error malloc caller");
            return EXIT_FAILURE;
        }
        socklen_t sizecall=sizeof(caller);

        int *sock2=(int *)malloc(sizeof(int));
        if(sock2 == NULL){
            perror("error malloc sock2");
            return EXIT_FAILURE;
        }
        *sock2=accept(sock,(struct sockaddr *)caller,&sizecall);
        if(*sock2 < 0){
            perror("error accept");
            return EXIT_FAILURE;
        }
    
        struct data * donnee = malloc(sizeof(struct data));
        if(donnee == NULL){
            perror("error malloc struct data ");
            return EXIT_FAILURE;
        }

        donnee->fdsock = *sock2;
        uint32_t *ipadressMain = malloc(sizeof(uint32_t));
        if(ipadressMain  == NULL){
            perror("error malloc donne adress ");
            return EXIT_FAILURE;
        }
        *ipadressMain = caller->sin_addr.s_addr;
        donnee->address = *ipadressMain;
        
        pthread_t th;
        pthread_create(&th,NULL,maxint,donnee);
        free(caller);
        free(sock2);
        free(ipadressMain);
  
    }
    close(sock);
    return EXIT_SUCCESS;
}