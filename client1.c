#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#define MAX_NAME 10
#define MSJ_SIZE 100

int main( int argc, char *argv[]){
    char sentmess[MSJ_SIZE];
    char recevmess[MAX_NAME+1];
    int sockt;
    int size_recv;

    // creation of address structure
    for(int i = 0; i < 5; i++){

        int port = *((int*)argv[2]);
        struct sockaddr_in address_socket;
        address_socket.sin_family=AF_INET;
        address_socket.sin_port=htons(port);
        if(inet_aton(argv[1],&address_socket.sin_addr) == -1){
            perror("error creation address");
            return EXIT_FAILURE;
        }

        // creation of the socket  
        sockt = socket(PF_INET,SOCK_STREAM,0);
        if(sockt == -1 ){
            perror("error creation socket");
            return EXIT_FAILURE;
        }
     
        //connection with the server 
        if(connect(sockt,(struct sockaddr *)&address_socket,sizeof( struct sockaddr_in)) == -1){
            perror("error connect");
            return EXIT_FAILURE;
        }

        //initialization and generation of a random number 
        srand(time(NULL));
        uint16_t nbAlea = rand() % 2001;
        printf(" the selected number: %u\n",nbAlea);

        //reading of the pseudo
        char pseudo[MAX_NAME+1];
        int rd = read(STDIN_FILENO,pseudo,MAX_NAME+1);
        if(rd == -1){
            perror("error read pseudo");
            return EXIT_FAILURE;
        }
        
        //send pseudo 
        if(send(sockt,pseudo,MAX_NAME,0) == -1){
            perror("error send pseudo");
            return EXIT_FAILURE;
        }
      
        //collecting  hello from serveur
        size_recv=recv(sockt,recevmess,17*sizeof(char),0);
        if(size_recv == -1){
            perror("erreur recv");
            return EXIT_FAILURE;
        }
    
        recevmess[size_recv]='\0';
        printf("%s\n",recevmess);
        
        sprintf(sentmess,"INT %u\n",htons(nbAlea));
        if(send(sockt,sentmess,strlen(sentmess),0) == -1){
            perror("error send number");
            return EXIT_FAILURE;
        }

        size_recv=recv(sockt,recevmess,6*sizeof(char),0);
        if(size_recv == -1){
            perror("erreur recv");
            return EXIT_FAILURE;
        }
        recevmess[size_recv]='\0';
        printf("%s",recevmess);

        close(sockt); 
    }

    return EXIT_SUCCESS;
}