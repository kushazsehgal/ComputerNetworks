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


int main(){
    char buffer[PACKET_SIZE];
    int sockfd;
    struct sockaddr_in serv_addr,cli_addr;
    
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for server\n");
        exit(0);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUM);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
        perror("Unable to bind to local address");
        exit(0);
    }
    else{
        printf("Server Listening on PORT : %d\n",PORT_NUM);
    }

    listen(sockfd,5);

    while (1) {

		int clilen = sizeof(cli_addr);
		int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
		if (fork() == 0) {

			close(sockfd);	
			strcpy(buffer,"LOGIN:");
			send(newsockfd, buffer, strlen(buffer) + 1, 0);
			for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
			recv(newsockfd, buffer, PACKET_SIZE, 0);
            printf("Name recieved : %s\n",buffer);
            FILE *file = fopen("users.txt", "r");
            char name[30];
            bool matched = false;
            while(fgets(name, sizeof(name), file)){
                if(name[strlen(name) - 1] == '\n')
                    name[strlen(name) - 1] = '\0';
                if(strcmp(name,buffer) == 0){
                    matched = true;
                    break;
                }
            }
            fclose(file);
			if(matched){
                strcpy(buffer,"User Successfully Logged In!\n");
                   
            }else{
                strcpy(buffer,"User Login Unsuccessful!\n");
            }
            send(newsockfd, buffer, strlen(buffer) + 1, 0);
			close(newsockfd);
			exit(0);
		}

		close(newsockfd);
	}
        return 0;
}