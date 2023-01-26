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
#include<poll.h>
#define PACKET_SIZE 20
#define TOT_SIZE 200

char* ask_load = "Send Load";
char* ask_time = "Send Time";

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
typedef struct entity{
    int port;
    int servsockfd;
    struct sockaddr_in serv_addr;
}entity;
int main(int argc, char** argv){
    entity s[2],lb;
    sscanf(argv[1],"%d",&lb.port);
    sscanf(argv[2],"%d",&s[0].port);
    sscanf(argv[3],"%d",&s[1].port);
    
    struct sockaddr_in cli_addr;

    if((lb.servsockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for server\n");
        exit(0);
    }
    lb.serv_addr.sin_family = AF_INET;
    lb.serv_addr.sin_port = htons(lb.port);
    lb.serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(lb.servsockfd,(struct sockaddr*)&lb.serv_addr,sizeof(lb.serv_addr)) < 0){
        perror("Unable to bind to local address");
        exit(0);
    }
    else{
        printf("Server Listening on PORT : %d\n",lb.port);
    }

    listen(lb.servsockfd,5);  
    
    while(1){

        int cli_len = sizeof(cli_addr);
        int clisockfd = accept(lb.servsockfd, (struct sockaddr *) &cli_addr,&cli_len);
        if (clisockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
        if(fork() == 0){
        
        }
        close(clisockfd);
        
    }
    // struct pollfd poll_time;
    // memset( &poll_time,0,sizeof(poll_time));
    // poll_time.fd = sockfd;
    // poll_time.events = POLLIN;
    
    


}