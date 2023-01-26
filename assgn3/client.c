/////////////////////////////////
// Networks Lab Assignment 3 : client
// Name : Kushaz Sehgal
// Roll Number : 20CS30030
/////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdbool.h>
#define PACKET_SIZE 50
#define TOT_SIZE 2000
char buffer[PACKET_SIZE];
char total[TOT_SIZE];
void input(int sockfd,char buffer[],char total[]){
    bool done = false;
    int j = 0;
    while(!done){
        recv(sockfd,buffer,PACKET_SIZE,0);
        for(int i = 0;i < PACKET_SIZE;i++){
            total[j++] = buffer[i];
            if(buffer[i] == '\0'){
                done = true;
                break;
            } 
        }
        for(int i = 0;i < PACKET_SIZE;i++)
            buffer[i] = '\0';
    }
}
int main(int argc,char** argv){
    int port;
    sscanf(argv[1],"%d",port);
    for(int i = 0;i < 20;i++)
        buffer[i] = '\0';
    for(int i = 0;i < 200;i++)
        total[i] = '\0';
    int sockfd;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for client\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_aton("127.0.0.1",&serv_addr.sin_addr);

    if(connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0){
        perror("Connection to server failed\n");
        exit(0);
    }
    else{
        printf("Connection with sever established\n");
    }

    input(sockfd,buffer,total);
    printf("Current Date and Time:\n\t\t%s",total);

    close(sockfd);
    return 0;
}