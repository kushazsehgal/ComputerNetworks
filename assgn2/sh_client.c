#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<time.h>
#define PORT_NUM 1301
#define PACKET_SIZE 50
int main(){
    char buffer[PACKET_SIZE];
    char recv_buffer[PACKET_SIZE];
    for(int i = 0;i < PACKET_SIZE;i++)
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

    recv(sockfd,buffer,PACKET_SIZE,0);
    printf("%s",buffer);
    for(int i = 0;i < PACKET_SIZE;i++)
        buffer[i] = '\0';
    scanf("%s",buffer);
    send(sockfd,buffer,strlen(buffer)+1,0);
    for(int i = 0;i < PACKET_SIZE;i++)
        buffer[i] = '\0';
    recv(sockfd,buffer,PACKET_SIZE,0);
    printf("%s",buffer);
    close(sockfd);
	return 0;

}