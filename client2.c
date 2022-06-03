#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_NAME 10
#define MSJ_SIZE 100

int main( int argc, char *argv[]){
    char sentmess[MSJ_SIZE];
    char recevmess[MSJ_SIZE];
    char answer[4];
    int sockt;
    int size_recv;
   
    // creation of address structure
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

    //reading of the pseudo 
    char pseudo[MAX_NAME+1];
    int rd = read(STDIN_FILENO,pseudo,MAX_NAME);
    if(rd == -1){
        perror("error read pseudo");
        return EXIT_FAILURE;
    }

    //send pseudo
    if(send(sockt,pseudo,rd,0) == -1){
        perror("error send pseudo");
        return EXIT_FAILURE;
    }
   
    //collecting  hello from serveur
    size_recv=recv(sockt,recevmess,17*sizeof(char),0);
    if(size_recv == -1){
        perror("error recv hello");
        return EXIT_FAILURE;
    }
    recevmess[size_recv]='\0';
    printf("%s\n",recevmess);

    // send max
    sprintf(sentmess,"MAX\n");
    if(send(sockt,sentmess,4*sizeof(char),0) == -1){
        perror("error send max");
        return EXIT_FAILURE;
    }
  
    size_recv=recv(sockt,recevmess,(MAX_NAME+3)*sizeof(char),0);
    if(size_recv == -1){
        perror("error recv NOP/REP");
        return EXIT_FAILURE;
    }
    recevmess[size_recv]='\0';

    strncpy(answer,recevmess,3);
    answer[3] = '\0';

    // depending on the message received (NOP or REP) the response is displayed 
    if(strcmp(answer,"NOP") == 0){
        printf("%s\n",answer);
       close(sockt); 
       return EXIT_SUCCESS;

    }else if (strcmp(answer,"REP") == 0){
        char address[32];
        uint32_t ipaddress = 0;
        uint16_t numbermax = 0;

        size_recv=recv(sockt,&ipaddress,sizeof(ipaddress),0);
        if(size_recv == -1){
            perror("error recv ip address");
            return EXIT_FAILURE;
        }
        size_recv=recv(sockt,&numbermax,sizeof(numbermax),0);
        if(size_recv == -1){
            perror("error recv max");
            return EXIT_FAILURE;
        }
        strncpy(pseudo,recevmess+3,MAX_NAME);

        ipaddress = ntohl(ipaddress);
        numbermax = ntohs(numbermax);

        printf("answer from server:\npseudo :%s\n",pseudo);
        printf("ip:%s \n",inet_ntop(AF_INET,&ipaddress,address,INET_ADDRSTRLEN));
        printf("max: %u\n",numbermax);

    }
    
    close(sockt); 

    return EXIT_SUCCESS;
}