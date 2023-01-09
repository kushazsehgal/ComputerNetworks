#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include <stdbool.h>
#define PORT_NUM 1301

int main(){
    char buffer[200];
    int sockfd;
    struct sockaddr_in serv_addr,cli_addr;
    
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for server\n");
        exit(0);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUM);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
        perror("Unable to bind to local address");
        exit(0);
    }
    else{
        printf("Server Listening on PORT : %d\n",PORT_NUM);
    }

    listen(sockfd,1);
    int cli_len = sizeof(cli_addr);
    int newsockfd;
    newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&cli_len);
    if(newsockfd < 0){
        perror("Connection(accept) to client failed\n");
        exit(0);
    }
    else{
        printf("Connection with client established\n");
    }
    // while(1){
    //     newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&cli_len);
    //     if(newsockfd < 0){
    //         perror("Connection(accept) to client failed\n");
    //         exit(0);
    //     }
    //     else{
    //         printf("Connection with client established\n");
    //     }
    //     break;
    // }
    
    bool continue_connection = true;
    while(continue_connection){
        recv(newsockfd,buffer,200,0);
        int val = atoi(buffer);
        if(val == -1){
            printf("Connection Terminated\n");
            close(newsockfd);
            continue_connection = false;
        }
        else{
            printf("Recieved the number : %d\n",val);
        }
    }
    close(sockfd);

    return 0;
}