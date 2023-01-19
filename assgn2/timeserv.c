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
#define PORT_NUM 2000
#define BUFFER 200
#define WAIT_TIME 5


int main(){
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;
    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
        perror("Cannot create socket for server\n");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT_NUM);

    if(bind(sockfd, (struct sockaddr *) &servaddr,sizeof(servaddr)) < 0){
        perror("Unable to bind to local address");
        exit(EXIT_FAILURE);
    }
    else{
        printf("Server Listening on PORT : %d\n",PORT_NUM);
    } 

    int n; 
    socklen_t len;
    char buffer[BUFFER]; 
    len = sizeof(cliaddr);
    n = recvfrom(sockfd, (char *)buffer, BUFFER, 0, 
			( struct sockaddr *) &cliaddr, &len);  
    printf("%s\n",buffer);
    //////////////////
    sleep(WAIT_TIME);
    ///////////////////
    time_t t;
    time(&t);
    strcpy(buffer,ctime(&t));
    sendto(sockfd, (const char *)buffer, strlen(buffer) + 1, 0, 
			(const struct sockaddr *) &cliaddr, sizeof(cliaddr)); 

    close(sockfd);
    return 0;
}