/////////////////////////////////
// Networks Lab Assignment 1 : Q2
// Name : Kushaz Sehgal
// Roll Number : 20CS30030
/////////////////////////////////
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
#define PACKET_SIZE 20


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
    while(1){
    
        printf("Enter an expression (enter -1 to exit) : ");
        
        int i = 0;
        char c;
        scanf("%c",&c);
        if(c == '-'){
            send(sockfd,"-\0",strlen("-\0") + 1,0);
            break;
        }
        buffer[i] = c;
        i++;
        while(1){
            char c;
            scanf("%c",&c);
            if(c == '\n'){
                buffer[i] = '\0';
                send(sockfd,buffer,strlen(buffer) + 1,0);
                break;
            }
            buffer[i] = c;
            i++;
            if(i == PACKET_SIZE){
                send(sockfd,buffer,PACKET_SIZE,0);
                i = 0;
            }       
        }
    
        recv(sockfd,recv_buffer,PACKET_SIZE*2,0);
        printf("%s",recv_buffer);      
    }
    close(sockfd);
    printf("Connection with server terminated\n");


    return 0;
}