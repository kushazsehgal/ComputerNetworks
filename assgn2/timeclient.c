#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<time.h>
#include<poll.h>
#define PORT_NUM 2000
#define BUFFER 200
int main(){
    char buffer[BUFFER]; 
    int sockfd;
    struct sockaddr_in servaddr;
    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
        perror("Cannot create socket for client\n");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr)); 

    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT_NUM); 
    inet_aton("127.0.0.1", &servaddr.sin_addr); 

    char *ask = "CLIENT:Asking for Date and Time"; 
      
    sendto(sockfd, (const char *)ask, strlen(ask) + 1, 0, 
			(const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Asking for time from server\n");

    struct pollfd poll_time;
    memset( &poll_time,0,sizeof(poll_time));
    poll_time.fd = sockfd;
    poll_time.events = POLLIN;
    int count = 0;
    while(count < 5){
        if(poll(&poll_time,1,3000) == 1){
            socklen_t len = sizeof(servaddr);
            recvfrom(sockfd, (char *)buffer, BUFFER, 0, ( struct sockaddr *) &servaddr, &len); 
            printf("Current Date and Time:\n\t\t%s",buffer); 
            break;
        }
        else count++;
    }
    printf("Failed Attempts to recieve : %d\n",count);
    return 0;

}