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
// #include <sys/time.h>
#include<time.h>
#define PACKET_SIZE 50
#define TOT_SIZE 200

char* ask_load = "Send Load";
char* ask_time = "Send Time";
char buffer[PACKET_SIZE];
char total[TOT_SIZE];

void input(int sockfd){
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
void make_server_addr(struct entity s[]){
    for(int i = 0;i < 3;i++){
        s[i].serv_addr.sin_family = AF_INET;
        s[i].serv_addr.sin_port = htons(s[i].port);
        inet_aton("127.0.0.1",&s[i].serv_addr.sin_addr);
    }
}
void getdata(struct entity *s,char* text){
    int lb_clisockfd;
    if((lb_clisockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for client\n");
        exit(0);
    }
    if(connect(lb_clisockfd,(struct sockaddr*) &(s->serv_addr),sizeof(s->serv_addr)) < 0){
        perror("Connection to server failed\n");
        exit(0);
    }
    strcpy(buffer,text);
    send(lb_clisockfd,buffer,strlen(buffer)+1,0);
    for(int i = 0;i < PACKET_SIZE;i++)buffer[i] = '\0';
    input(lb_clisockfd);
    close(lb_clisockfd);
}
////////////////////////////////
//s[0] --> load balancer
//s[1] and s[2] --> two loads
////////////////////////////////
int main(int argc, char** argv){
    for(int i = 0;i < PACKET_SIZE;i++)
        buffer[i] = '\0';
    for(int i = 0;i < TOT_SIZE;i++)
        total[i] = '\0';
    entity s[3];
    sscanf(argv[1],"%d",&s[0].port);
    sscanf(argv[2],"%d",&s[1].port);
    sscanf(argv[3],"%d",&s[2].port);
    make_server_addr(s);
    struct sockaddr_in cli_addr;

    if((s[0].servsockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for server\n");
        exit(0);
    }
    if(bind(s[0].servsockfd,(struct sockaddr*)&s[0].serv_addr,sizeof(s[0].serv_addr)) < 0){
        perror("Unable to bind to local address");
        exit(0);
    }
    else{
        printf("Load Balancer Listening on PORT : %d\n",s[0].port);
    }

    listen(s[0].servsockfd,5);  
    while(1){
        
        getdata(&s[1],ask_load);
        int load1 = atoi(total);
        printf("\nLoad 1 : %d from %s %d\n",load1,inet_ntoa(s[1].serv_addr.sin_addr), ntohs(s[1].serv_addr.sin_port));
        for(int i = 0;i < TOT_SIZE;i++)total[i] = '\0';
        getdata(&s[2],ask_load);
        int load2 = atoi(total);
        printf("Load 2 : %d from %s %d\n\n",load2,inet_ntoa(s[2].serv_addr.sin_addr), ntohs(s[2].serv_addr.sin_port));
        for(int i = 0;i < TOT_SIZE;i++)total[i] = '\0';
        int serverindex = (load1 > load2) ? 2 : 1;

        time_t last_load = time(NULL);
        int cli_len = sizeof(cli_addr);
        struct pollfd poll_time;
        memset( &poll_time,0,sizeof(poll_time));
        poll_time.fd = s[0].servsockfd;
        poll_time.events = POLLIN;

        while(time(NULL) - last_load < 5){

            printf("Seconds Past : %f\n",difftime(time(NULL),last_load));
            int pollfor = (5.0 - difftime(time(NULL),last_load))*1000.0;
            printf("Polling for : %d\n",pollfor);
            int ret = poll(&poll_time,1,pollfor);
            if(ret == 1){
                int cli_len = sizeof(cli_addr);
                int clisockfd = accept(s[0].servsockfd,(struct sockaddr*) &cli_addr,&cli_len);
                if(fork() == 0){
                    printf("Recived Client Request\n");
                    printf("Sending request to server no : %d\n",serverindex);
                    close(s[0].servsockfd);
                    getdata(&s[serverindex],ask_time);
                    printf("Got Time from less loaded server: %s %d\n", inet_ntoa(s[serverindex].serv_addr.sin_addr), ntohs(s[serverindex].serv_addr.sin_port));
                    strcpy(buffer,total);
                    send(clisockfd,buffer,strlen(buffer)+1,0);
                    for(int i = 0;i < PACKET_SIZE;i++)buffer[i] = '\0';
                    for(int i = 0;i < TOT_SIZE;i++)total[i] = '\0';
                    close(clisockfd);  
                    exit(0);
                }
                close(clisockfd);
            }
            else if(ret == 0){
                printf("Did not Recieve Client Request\n");
            }
            else{
                printf("Poll error!\n");
            }
        }   
        printf("5 Seconds Past, will get Load again!!\n");
    }
}