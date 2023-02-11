#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
void reverse(char str[]){
    int i=0, j=strlen(str)-1;
    while(i < j){
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
int main(){
    int sockfd;
    struct sockaddr_in serv_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Socket creation error\n");
        exit(1);
    }
    serv_addr.sin_family = AF_INET;
    while(1){
        printf("MyOwnBrowser> ");
        char http_command[1000];
        fgets(http_command, 1000, stdin);
        http_command[strlen(http_command)-1] = '\0';
        if(strcmp(http_command, "QUIT") == 0){
            printf("Exiting...\n");
            exit(0);
        }
        char request_type[4];
        strncpy(request_type, http_command, 3);
        request_type[3] = '\0';
        if(strcmp(request_type, "GET") == 0){
            char* url = http_command + 11;
            char* host = strtok(url, "/");
            char* path = strtok(NULL, ":");
            char* temp = path;
            path = (char*)malloc(strlen(temp)+2);
            path[0] = '/';
            strcpy(path+1, temp);
            char* port = strtok(NULL, " ");
            if(port == NULL){
                port = "80";
            }
            char* version = strtok(NULL, " ");
            if(version == NULL){
                version = "HTTP/1.1";
            }
            printf("Host: %s\n", host);
            printf("Path: %s\n", path);
            printf("Port: %s\n", port);
            printf("Version: %s\n", version);
            char file_extension[20];
            int i=strlen(path)-1;
            int j=0;
            while(path[i] != '.'){
                file_extension[j++] = path[i--];
            }
            file_extension[j] = '\0';
            reverse(file_extension);
            printf("File extension: %s\n", file_extension);
            char accept_type[100];
            if(strcmp(file_extension, "html") == 0){
                strcpy(accept_type, "text/html");
            }else if(strcmp(file_extension, "jpg") == 0){
                strcpy(accept_type, "image/jpeg");
            }else if(strcmp(file_extension, "pdf") == 0){
                strcpy(accept_type, "application/pdf");
            }else{
                strcpy(accept_type, "text/*");
            }

            serv_addr.sin_port = htons(atoi(port));
            if(inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0){
                printf("Invalid address\n");
                exit(1);
            }

            if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
                printf("Connection failed\n");
                exit(1);
            }

            char request[1000];
            time_t ct;
            struct tm* tm;
            char date[100];
            time(&ct);
            tm = localtime(&ct);
            strftime(date, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
            time_t ct2;
            ct2 = ct - 2*24*60*60;
            tm = localtime(&ct2);
            char date2[100];
            strftime(date2, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
            sprintf(request, "GET %s %s\nHost: %s\nConnection: close\nDate: %s\nAccept: %s\nAccept-Language: en-us\nIf-Modified-Since: %s\nContent-language: en-us\nContent-length: %d\nContent-type: %s\n", path, version, host, date, accept_type, date2, sizeof(request), accept_type);
            printf("Request\n%s", request);
            send(sockfd, request, strlen(request)+1, 0);
            char response[1000];
            recv(sockfd, response, 1000, 0);
            printf("Response\n%s", response);

        }else if(strcmp(request_type, "PUT") == 0){

        }else{
            printf("Invalid command\n");
        }
    }
}
