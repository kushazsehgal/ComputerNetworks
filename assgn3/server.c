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
#include <dirent.h>
#define PACKET_SIZE 50
#define TOT_SIZE 200

char* ask_load = "Send Load";
char* ask_time = "Send Time";
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
    srand((int)time(NULL));
    for(int i = 0;i < PACKET_SIZE;i++)
        buffer[i] = '\0';
    for(int i = 0;i < TOT_SIZE;i++)
        total[i] = '\0';
    int servport;
    sscanf(argv[1],"%d",&servport);
    struct sockaddr_in serv_addr,lb_addr;
    int servsockfd;
    if((servsockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for server\n");
        exit(0);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(servport);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(servsockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
        perror("Unable to bind to local address");
        exit(0);
    }
    else{
        printf("Server Listening on PORT : %d\n",servport);
    }

    listen(servsockfd,5);
    while(1){
        int lb_len = sizeof(lb_addr);
        int lb_clisockfd = accept(servsockfd, (struct sockaddr *) &lb_addr,&lb_len);
        if (lb_clisockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
        if(fork() == 0){
            close(servsockfd);
            input(lb_clisockfd,buffer,total);
            if(strcmp(buffer,ask_load) == 0){
                int load = 1 + rand()%100;
                for(int i = 0;i < PACKET_SIZE;i++)
                    buffer[i] = '\0';
                sprintf(buffer,"%d",load);
                send(lb_clisockfd,buffer,strlen(buffer) + 1,0);
            }
            else if(strcmp(buffer,ask_time) == 0){
                time_t t;
                time(&t);
                strcpy(buffer,ctime(&t));
                send(lb_clisockfd,buffer,strlen(buffer) + 1,0);
                for(int i = 0;i < PACKET_SIZE;i++)
                    buffer[i] = '\0';
            }
            close(lb_clisockfd);
            exit(0);
        }
        close(lb_clisockfd);
    }
    
    return 0;
}