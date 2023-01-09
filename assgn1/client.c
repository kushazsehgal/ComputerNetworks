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
    for(int i = 0;i < 200;i++)
        buffer[i] = '\0';
    int sockfd;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for client\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUM);
    //"127.0.0.1" --> special ip corresponding to localhost
    inet_aton("127.0.0.1",&serv_addr.sin_addr);

    if(connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0){
        perror("Connection to server failed\n");
        exit(0);
    }
    else{
        printf("Connection with sever established\n");
    }
    bool continue_connection = true;
    while(continue_connection){
        int num;
        printf("Enter a number : ");
        scanf("%d",&num);
        snprintf(buffer, 10, "%d", num);
        // printf("%s",buffer);
        
        send(sockfd,buffer,strlen(buffer)+1,0);
        if(num == -1){
            continue_connection = false;
        }
    }
    close(sockfd);


    return 0;
}