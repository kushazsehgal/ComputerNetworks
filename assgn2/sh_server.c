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
#define PORT_NUM 1301
#define PACKET_SIZE 50
#define TOT_SIZE 500

int main(){
    char buffer[PACKET_SIZE];
    char total[TOT_SIZE];
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
            for(int i=0;i < TOT_SIZE;i++) total[i] = '\0';
			recv(newsockfd, buffer, PACKET_SIZE, 0);
            // printf("Name recieved : %s\n",buffer);
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
            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
			if(matched){
                strcpy(buffer,"FOUND");
                   
            }else{
                strcpy(buffer,"NOT-FOUND");
            }
            send(newsockfd, buffer, strlen(buffer) + 1, 0);
            bool done = false;
            if(matched){
                while(1){
                    int j = 0;
                    while(1){
                        recv(newsockfd,buffer,PACKET_SIZE,0);
                        for(int i = 0;i < PACKET_SIZE;i++){
                            total[j] = buffer[i];
                            if(buffer[i] == '\n'){
                                total[j] = '\0';
                                done = true;
                                break;
                            }
                            j++;
                        }
                        for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                        if(done)
                            break;
                    }
                    // printf("Total Command Recieved :\n%s\n",total);
                    if(strcmp(total,"exit") == 0){
                        strcpy(buffer,"exit");
                        send(newsockfd,buffer,strlen(buffer) + 1,0);
                        for(int i = 0;i < PACKET_SIZE;i++)
                            buffer[i] = '\0';
                        break;
                    }
                    char command[10],argument[TOT_SIZE];
                    for(int i = 0;i < 10;i++)
                        command[i] = '\0';
                    for(int i = 0;i < TOT_SIZE;i++)
                        argument[i] = '\0';
                    j = 0;
                    while(j < 10 && total[j] != ' ' && total[j] != '\0'){
                        command[j] = total[j];
                        j++;
                    }
                    while(total[j] == ' ')
                        j++;
                    for(int i = 0;i < TOT_SIZE;i++){
                        argument[i] = total[j++];
                        if(argument[i] == '\0')
                            break;
                    }
                    // printf("Command : %s , Argument : %s\n",command,argument);
                    for(int i = 0;i < TOT_SIZE;i++)
                        total[i] = '\0';
                    if(strcmp(command,"cd") == 0){
                        if(chdir(argument) != 0){
                            strcpy(buffer,"####");
                            send(newsockfd,buffer,strlen(buffer) + 1,0);
                            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                        }
                        else{
                            strcpy(buffer,"Changed Directory !");
                            send(newsockfd,buffer,strlen(buffer) + 1,0);
                            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                        }
                    }
                    else if(strcmp(command,"dir") == 0){
                        struct dirent *de;  
                        if(argument[0] == '\0')
                            argument[0] = '.';
                        DIR *dr = opendir(argument);
                        if (dr == NULL){
                            strcpy(buffer,"####");
                            send(newsockfd,buffer,strlen(buffer) + 1,0);
                            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                        }
                        while ((de = readdir(dr)) != NULL){
                            total[strlen(total)] = ' ';
                            strcat(total,de->d_name);
                        }
                        closedir(dr); 
                        int i = 0;
                        for(int j = 0;j < strlen(total) + 1;j++){
                            buffer[i++] = total[j];
                            if(i == 50){
                                send(newsockfd,buffer,PACKET_SIZE,0);
                                for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                                i = 0;
                            }
                        }
                        if(i != 0){
                            send(newsockfd,buffer,strlen(buffer) + 1,0);
                            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                        }
                        for(int i = 0;i < TOT_SIZE;i++)
                            total[i] = '\0';
                    }
                    else if(strcmp(command,"pwd") == 0){
                        if(getcwd(total,TOT_SIZE) == NULL){
                            strcpy(buffer,"####");
                            send(newsockfd,buffer,strlen(buffer) + 1,0);
                            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                        }
                        else{
                            int i = 0;
                            for(int j = 0;j < strlen(total) + 1;j++){
                                buffer[i++] = total[j];
                                if(i == 50){
                                    send(newsockfd,buffer,PACKET_SIZE,0);
                                    for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                                    i = 0;
                                }
                            }
                            if(i != 0){
                                send(newsockfd,buffer,strlen(buffer) + 1,0);
                                for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                            }
                            for(int i = 0;i < TOT_SIZE;i++)
                                total[i] = '\0';
                        }
                        
                    }
                    else{   
                        strcpy(buffer,"$$$$");
                        send(newsockfd,buffer,strlen(buffer) + 1,0);
                        for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
                    }
                }
            }
            

			close(newsockfd);
			exit(0);
		}

		close(newsockfd);
	}
        return 0;
}