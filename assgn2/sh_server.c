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
#define TOT_SIZE 2000

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
    // printf("length : %d\n",j);
}
void output(int sockfd,char buffer[],char total[]){
    int i = 0;
    int len = strlen(total);
    for(int j = 0;j < len + 1;j++){
        buffer[i++] = total[j];
        if(i == 50){
            send(sockfd,buffer,PACKET_SIZE,0);
            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
            i = 0;
        }
    }
    if(i != 0){
        send(sockfd,buffer,strlen(buffer) + 1,0);
    }
    for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
}
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
		int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen) ;
		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
		if (fork() == 0) {
			close(sockfd);
            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';	
			strcpy(buffer,"LOGIN:");
			send(newsockfd, buffer, strlen(buffer) + 1, 0);
			for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
            for(int i=0;i < TOT_SIZE;i++) total[i] = '\0';
            
            input(newsockfd,buffer,total);//total has username 

            FILE *file = fopen("users.txt", "r");
            char name[30];
            bool matched = false;
            while(fgets(name, sizeof(name), file)){
                if(name[strlen(name) - 1] == '\n')
                    name[strlen(name) - 1] = '\0';
                if(strcmp(name,total) == 0){
                    matched = true;
                    break;
                }
            }
            fclose(file);
            for(int i=0; i < PACKET_SIZE; i++) total[i] = '\0';
			if(matched){
                strcpy(buffer,"FOUND");
                   
            }else{
                strcpy(buffer,"NOT-FOUND");
            }
            send(newsockfd, buffer, strlen(buffer) + 1, 0);
            for(int i=0; i < PACKET_SIZE; i++) buffer[i] = '\0';
            if(matched){
                while(1){
                    input(newsockfd,buffer,total);//total has command
                    if(strcmp(total,"exit") == 0){
                        output(newsockfd,buffer,"exit");
                        for(int i = 0;i < TOT_SIZE;i++)
                            total[i] = '\0';
                        break;
                    }
                    char command[10],argument[TOT_SIZE];
                    for(int i = 0;i < 10;i++)
                        command[i] = '\0';
                    for(int i = 0;i < TOT_SIZE;i++)
                        argument[i] = '\0';
                    int j = 0;
                    while (total[j] == ' ')
                        j++;
                    int j_ = 0;
                    while(j_ < 10 && total[j] != ' ' && total[j] != '\0')
                        command[j_++] = total[j++];    
                    
                    while(total[j] == ' ')
                        j++;
                    for(int i = 0;i < TOT_SIZE;i++){
                        argument[i] = total[j++];
                        if(argument[i] == '\0')
                            break;
                    }
                    int lastindex = strlen(argument) - 1;
                    while(lastindex >= 0 && argument[lastindex] == ' '){
                        argument[lastindex] = '\0';
                        lastindex--;
                    }
                    
                    for(int i = 0;i < TOT_SIZE;i++)
                        total[i] = '\0';
                    if(strcmp(command,"cd") == 0){
                        if(chdir(argument) != 0)
                            output(newsockfd,buffer,"####");
                        else output(newsockfd,buffer,"Changed Directory !");
                    }
                    else if(strcmp(command,"dir") == 0){
                        struct dirent *de;  
                        if(argument[0] == '\0')
                            argument[0] = '.';
                        DIR *dr = opendir(argument);
                        if (dr == NULL)
                            output(newsockfd,buffer,"####");
                        else{
                            while ((de = readdir(dr)) != NULL){
                                total[strlen(total)] = ' ';
                                strcat(total,de->d_name);
                            }
                            closedir(dr); 
                            output(newsockfd,buffer,total);
                            for(int i = 0;i < TOT_SIZE;i++)
                                total[i] = '\0';
                        }    
                    }
                    else if(strcmp(command,"pwd") == 0){
                        if(getcwd(total,TOT_SIZE) == NULL)
                            output(newsockfd,buffer,"####");
                        else{
                            output(newsockfd,buffer,total);
                            for(int i = 0;i < TOT_SIZE;i++)
                                total[i] = '\0';
                        }
                    }
                    else
                        output(newsockfd,buffer,"$$$$");
                }
            }
			close(newsockfd);
			exit(0);
		}
		close(newsockfd);
	}
        return 0;
}