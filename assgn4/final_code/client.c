#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>

#define MAX_BUFFER_SIZE 1024

void get_time(char *time_str, char* modified_time_str){
    time_t ct, ct2;
    struct tm* tm;
    time(&ct);
    tm = localtime(&ct);
    strftime(time_str, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
    time(&ct2);
    ct2 = ct - 2*24*60*60;
    tm = gmtime(&ct2);
    if(modified_time_str != NULL)
        strftime(modified_time_str, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
}
void get_accept_type(char* accept_type, char* path){
    char* ext = strrchr(path, '.');
    if(ext == NULL){
        strcpy(accept_type, "text/*");
        return;
    }
    if(strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0){
        strcpy(accept_type, "text/html");
    }
    else if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0){
        strcpy(accept_type, "image/jpeg");
    }
    else if(strcmp(ext, ".pdf") == 0){
        strcpy(accept_type, "application/pdf");
    }
    else{
        strcpy(accept_type, "text/*");
    }
}
int main(){
    int sockfd;
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    while(1){
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("Socket creation error\n");
            exit(1);
        }
        printf("MyOwnBrowser> ");
        char http_command[MAX_BUFFER_SIZE];
        fgets(http_command, MAX_BUFFER_SIZE, stdin);
        http_command[strlen(http_command)-1] = '\0';
        if(strcmp(http_command, "QUIT") == 0){
            printf("Exiting...\n");
            exit(0);
        }
        char request_type[4];
        strncpy(request_type, http_command, 3);
        request_type[3] = '\0';
        if(strcmp(request_type, "GET") == 0){
            char url[MAX_BUFFER_SIZE];
            sscanf(http_command, "%*s %s", url);
            printf("URL: %s\n", url);
            char protocol[10], host[50], path[100];
            int port = 80; // default port
            if (sscanf(url, "%[^:]://%[^/]/%[^:]:%d", protocol, host, path, &port) != 4) {
                if (sscanf(url, "%[^:]://%[^/]/%[^\n]", protocol, host, path) != 3) {
                    printf("Invalid URL\n");
                    return 1;
                }
            }
            char* file_name = strrchr(path, '/');
            if(file_name){
                file_name++;
            }
            else{
                file_name = path;
            }
            printf("File name: %s\n", file_name);
            printf("Protocol: %s\nHost: %s\nPort: %d\nPath: %s\n", protocol, host, port, path);
            struct hostent *server = gethostbyname(host);
            if (server == NULL) {
                fprintf(stderr, "Unable to resolve host %s\n", host);
                return 1;
            }
            struct sockaddr_in serv_addr;
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
            serv_addr.sin_port = htons(port);
            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("connect");
                return 1;
            }
            char request[MAX_BUFFER_SIZE];
            sprintf(request, "GET /%s HTTP/1.1\r\n", path);
            sprintf(request + strlen(request), "Host: %s\r\n", host);
            sprintf(request + strlen(request), "Connection: close\r\n");
            char time_str[100], modified_time_str[100];
            get_time(time_str, modified_time_str);
            sprintf(request + strlen(request), "Date: %s\r\n", time_str);
            char accept_type[100];
            get_accept_type(accept_type, path);
            sprintf(request + strlen(request), "Accept: %s\r\n", accept_type);
            sprintf(request + strlen(request), "Accept-Language: en-us\r\n");
            sprintf(request + strlen(request), "If-Modified-Since: %s\r\n", modified_time_str);
            sprintf(request + strlen(request), "\r\n");
            printf("Request:\n%s\n", request);
            
            if(send(sockfd, request, strlen(request), 0) < 0){
                printf("Send failed\n");
                exit(1);
            }
            
            char response[MAX_BUFFER_SIZE];
            // receive headers first
            int nbytes = recv(sockfd, response, MAX_BUFFER_SIZE, 0);
            response[nbytes] = '\0';
            printf("Response:\n%s\n", response);

            // get version, status code and status message
            char version[10], status_code[10], status_message[100];
            sscanf(response, "%s %s %s", version, status_code, status_message);
            printf("Version: %s\nStatus Code: %s\nStatus Message: %s\n", version, status_code, status_message);

            if(strcmp(status_code, "200") == 0){
                // get content length
                char* content_length_str = strstr(response, "Content-Length: ");
                int content_length;
                sscanf(content_length_str, "Content-Length: %d", &content_length);
                printf("Content Length: %d\n", content_length);

                // get content type
                char* content_type_str = strstr(response, "Content-Type: ");
                char content_type[100];
                sscanf(content_type_str, "Content-Type: %s", content_type);
                printf("Content Type: %s\n", content_type);

                // receive content
                
                char recv_file_name[100];
                sprintf(recv_file_name, "recv_%s", file_name);
                FILE* fp = fopen(recv_file_name, "wb");
                if(fp == NULL){
                    printf("File open error\n");
                    exit(1);
                }
                char content[MAX_BUFFER_SIZE];
                memset(content, 0, MAX_BUFFER_SIZE);
                // int total_bytes = 0;
                // while(total_bytes < content_length){
                //     nbytes = recv(sockfd, content, MAX_BUFFER_SIZE, 0);
                //     total_bytes += nbytes;
                //     content[nbytes] = '\0';
                //     fwrite(content, 1, nbytes, fp);
                // }
                while((nbytes = recv(sockfd, content, MAX_BUFFER_SIZE, 0)) > 0){
                    fwrite(content, 1, nbytes, fp);
                    memset(content, 0, MAX_BUFFER_SIZE);
                }
                fclose(fp);
                printf("File received\n");
                pid_t pid = fork();
                if(pid == 0){
                    if(strcmp(content_type, "text/html") == 0){
                        execlp("firefox", "firefox", recv_file_name, NULL);
                    }else if(strcmp(content_type, "image/jpeg") == 0){
                        execlp("eog", "eog", recv_file_name, NULL);
                    }else if(strcmp(content_type, "txt") == 0){
                        execlp("gedit", "gedit", recv_file_name, NULL);
                    }else{
                        execlp("xdg-open", "xdg-open", recv_file_name, NULL);
                    }
                    perror("execlp");
                    exit(1);
                }
            }else if(strcmp(status_code, "400")){
                printf("Bad Request\n");
            }else if(strcmp(status_code, "404")){
                printf("Not Found\n");
            }else if(strcmp(status_code, "403")){
                printf("Forbidden\n");
            }else{
                printf("Unknown Error\n");
            }
        }
        else if(strcmp(request_type, "PUT") == 0){
            char url[MAX_BUFFER_SIZE], file_name[100];
            sscanf(http_command, "%*s %s %s", url, file_name);
            printf("URL: %s\n", url);
            printf("File name: %s\n", file_name);
            char protocol[10], host[50], path[100];
            int port = 80; // default port
            if (sscanf(url, "%[^:]://%[^/]/%[^:]:%d", protocol, host, path, &port) != 4) {
                if (sscanf(url, "%[^:]://%[^/]/%[^\n]", protocol, host, path) != 3) {
                    printf("Invalid URL\n");
                    return 1;
                }
            }
            printf("Protocol: %s\n", protocol);
            printf("Host: %s\n", host);
            printf("Path: %s\n", path);
            printf("Port: %d\n", port);
            struct hostent *server = gethostbyname(host);
            if (server == NULL) {
                fprintf(stderr, "Unable to resolve host %s\n", host);
                return 1;
            }
            struct sockaddr_in serv_addr;
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
            serv_addr.sin_port = htons(port);
            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("connect");
                return 1;
            }
            char request[MAX_BUFFER_SIZE];
            sprintf(request, "PUT /%s/%s HTTP/1.1\r\n", path, file_name);
            sprintf(request + strlen(request), "Host: %s\r\n", host);
            sprintf(request + strlen(request), "Connection: close\r\n");
            char time_str[100];
            get_time(time_str, NULL);
            sprintf(request + strlen(request), "Date: %s\r\n", time_str);
            sprintf(request + strlen(request), "Content-language: en-us\r\n");
            FILE* fp = fopen(file_name, "rb");
            if(fp == NULL){
                printf("File open error\n");
                exit(1);
            }
            fseek(fp, 0, SEEK_END);
            int file_size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            sprintf(request + strlen(request), "Content-Length: %d\r\n", file_size);

            char content_type[100];
            get_accept_type(content_type, file_name);
            sprintf(request + strlen(request), "Content-Type: %s\r\n", content_type);
            sprintf(request + strlen(request), "\r\n");
            printf("Request:\n%s\n", request);
            send(sockfd, request, strlen(request), 0);

            char file_content[MAX_BUFFER_SIZE];
            memset(file_content, 0, MAX_BUFFER_SIZE);
            int iters=0;
            while(fread(file_content, 1, MAX_BUFFER_SIZE, fp) > 0){
                printf("Iteration %d\n", ++iters);
                send(sockfd, file_content, strlen(file_content), 0);
                memset(file_content, 0, MAX_BUFFER_SIZE);
            }
            fclose(fp);

            char response[MAX_BUFFER_SIZE];
            memset(response, 0, MAX_BUFFER_SIZE);
            recv(sockfd, response, MAX_BUFFER_SIZE, 0);
            printf("Response:\n%s\n", response);

            char status_code[4];
            sscanf(response, "%*s %s", status_code);
            if(strcmp(status_code, "200") == 0){
                printf("File uploaded\n");
            }else if(strcmp(status_code, "400")){
                printf("Bad Request\n");
            }else if(strcmp(status_code, "404")){
                printf("Not Found\n");
            }else if(strcmp(status_code, "403")){
                printf("Forbidden\n");
            }else{
                printf("Unknown Error\n");
            }
        }else{
            printf("Invalid command\n");
        }
        close(sockfd);
    }
    return 0;
}