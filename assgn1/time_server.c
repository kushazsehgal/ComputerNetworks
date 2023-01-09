#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include<time.h>
#define PORT_NUM 1301

int main(){
    char buffer[200];
    //socfd --> socket of server
    //newsockfd --> socket for connection with client
    int sockfd;
    struct sockaddr_in cli_addr,serv_addr;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for server\n");
        exit(0);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT_NUM);

    if(bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        perror("Unable to bind to local address");
        exit(0);
    }
    else{
        printf("Server Listening on PORT : %d\n",PORT_NUM);
    }

    listen(sockfd,1);

    while(1){
        int clilen = sizeof(cli_addr);
        //sockfd of connected client
        int newsockfd = accept(sockfd,(struct sockaddr*) &cli_addr,&clilen);

        if(newsockfd < 0){
            perror("Connection(accept) to client failed\n");
            exit(0);
        }
        time_t t;
        time(&t);
        strcpy(buffer,ctime(&t));

        send(newsockfd,buffer,strlen(buffer) + 1,0);
        close(newsockfd);
    }

    close(sockfd);

    return 0;
}