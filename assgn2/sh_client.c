/////////////////////////////////
// Networks Lab Assignment 2 : Q2
// Name : Kushaz Sehgal
// Roll Number : 20CS30030
/////////////////////////////////
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<time.h>
#include<stdbool.h>
#define PORT_NUM 1301
#define PACKET_SIZE 50
#define TOT_SIZE 500
int main(){
    char buffer[PACKET_SIZE];
    char total[TOT_SIZE];
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
    char c;
    scanf("%c",&c);
    send(sockfd,buffer,strlen(buffer)+1,0);
    for(int i = 0;i < PACKET_SIZE;i++)
        buffer[i] = '\0';
    recv(sockfd,buffer,PACKET_SIZE,0);
    if(strcmp(buffer,"FOUND") == 0){
        printf("User Successfully Logged in !\n");
        for(int i = 0;i < PACKET_SIZE;i++)
            buffer[i] = '\0';
        while(1){
            printf("Enter Command : ");
            bool done = false;
            while(1){
                int i = 0;
                while(i < 50){
                    scanf("%c",&c);
                    buffer[i++] = c;
                    if(c == '\n'){
                        done = true;
                        break;
                    }
                }
                send(sockfd,buffer,PACKET_SIZE,0);
                for(int i = 0;i < PACKET_SIZE;i++)
                    buffer[i] = '\0';
                
                if(done)
                    break;
            }
            done = false;
            int j = 0;
            while(1){
                recv(sockfd,buffer,PACKET_SIZE,0);
                for(int i = 0;i < PACKET_SIZE;i++){
                    total[j++] = buffer[i];
                    if(buffer[i] == '\0'){
                        done = true;
                        break;
                    }
                }
                if(done)
                    break;
            }
            if(strcmp(total,"exit") == 0){
                printf("Goodbye!\n");
                break;
            }
            else{
                if(strcmp(total,"####") == 0){
                    printf("Error On Execution of Command !\n");
                }
                else if(strcmp(total,"$$$$") == 0){
                    printf("Invalid Command !\n");
                }
                else{
                    printf("Recived Output:\n%s\n\n",total);
                }
                
            }
            for(int i = 0;i < TOT_SIZE;i++)
                total[i] = '\0';   
        }
    }
    else if(strcmp(buffer,"NOT-FOUND") == 0){
        printf("Login Unsuccessfull !\n");
    }
    
    close(sockfd);
	return 0;

}