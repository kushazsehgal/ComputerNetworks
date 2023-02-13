/////////////////////////////////////////
// Networks Lab Assignment 4 : My Browser
// Name : Kushaz Sehgal
// Roll Number : 20CS30030
// Name : Shivam Raj
// Roll Number : 20CS10056
//////////////////////////////////////////
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
#include <poll.h>

#define BUFFER_SIZE 1000000
#define PACKET_SIZE 1000
char BUFFER[PACKET_SIZE];
void get_time(char *time_str, char* modified_time_str){
    time_t ct, ct2;
    struct tm* tm;
    time(&ct);
    tm = localtime(&ct);
    strftime(time_str, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
    time(&ct2);
    ct2 = ct - 2*24*60*60;
    tm = localtime(&ct2);
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
long int findSize(char file_name[]){
    FILE* fp = fopen(file_name, "r");  
    if (fp == NULL) {
        return -1;
    }
    fseek(fp, 0L, SEEK_END);  
    long int res = ftell(fp);
    fclose(fp);
    return res;
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
        char http_command[PACKET_SIZE];
        fgets(http_command, PACKET_SIZE, stdin);
        http_command[strlen(http_command)-1] = '\0';
        if(strcmp(http_command, "QUIT") == 0){
            printf("Exiting...\n");
            exit(0);
        }
        char request_type[4];
        strncpy(request_type, http_command, 3);
        request_type[3] = '\0';
        if(strcmp(request_type, "GET") == 0){
            char url[PACKET_SIZE];
            sscanf(http_command, "%*s %s", url);
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
            char request[PACKET_SIZE];
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
            
            char total_response[BUFFER_SIZE];

            char buffer[PACKET_SIZE];
            memset(buffer, 0, PACKET_SIZE);
            memset(total_response, 0, BUFFER_SIZE);
            int j=0;
            int bytes_read = 0;
            int total_bytes_read = 0;
            char* end_of_header;
            
            struct pollfd fds[1];
            fds[0].fd = sockfd;
            fds[0].events = POLLIN;
            int timeout = 3000;
            int ret = poll(fds, 1, timeout);
            if(ret < 0){
                printf("Poll error\n");
                exit(1);
            }
            if(ret == 0){
                printf("No response from server\n");
                close(sockfd);
                continue;
            }
            while((bytes_read = recv(sockfd, buffer, PACKET_SIZE, 0))>0){
                total_bytes_read += bytes_read;
                for(int k=0; k<bytes_read; k++){
                    total_response[j] = buffer[k];
                    j++;
                }   
                memset(buffer, 0, PACKET_SIZE);
                end_of_header = strstr(total_response, "\r\n\r\n");
                if(end_of_header){
                    end_of_header += 4;
                    break;
                }
            }

            int file_bytes_read = total_bytes_read - (end_of_header - total_response);

            char response[PACKET_SIZE];
            char* temp = total_response;
            while(temp != end_of_header){
                response[temp - total_response] = *temp;
                temp++;
            }
            response[temp - total_response] = '\0';
            printf("Response:\n%s\n",response);
            // get version, status code and status message
            char version[10], status_code[10], status_message[100];
            sscanf(response, "%s %s %s", version, status_code, status_message);

            if(strcmp(status_code, "200") == 0){
                printf("Recieve Sucessful\n");
                // get content length
                char* content_length_str = strstr(response, "Content-Length: ");
                int content_length;
                sscanf(content_length_str, "Content-Length: %d", &content_length);
                // get content type
                char* content_type_str = strstr(response, "Content-Type: ");
                char content_type[100];
                sscanf(content_type_str, "Content-Type: %s", content_type);
                // receive content
                char recv_file_name[100];
                sprintf(recv_file_name, "recv_%s", file_name);
                FILE* fp = fopen(recv_file_name, "wb");
                if(fp == NULL){
                    printf("File open error\n");
                    exit(1);
                }
                char file_content[PACKET_SIZE];
                memset(file_content, 0, PACKET_SIZE);
                int nbytes = 0;
                int toal_file_bytes_read = 0;
                if(file_bytes_read > 0){
                    fwrite(end_of_header, 1, file_bytes_read, fp);
                    toal_file_bytes_read += file_bytes_read;
                }
                while(total_bytes_read < content_length){
                    nbytes = recv(sockfd, file_content, PACKET_SIZE, 0);
                    total_bytes_read += nbytes;
                    fwrite(file_content, 1, nbytes, fp);
                    memset(file_content, 0, PACKET_SIZE);
                }
                
                fclose(fp);
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
            char url[PACKET_SIZE], file_name[100];
            sscanf(http_command, "%*s %s %s", url, file_name);
            char protocol[10], host[50], path[100];
            int port = 80; // default port
            if (sscanf(url, "%[^:]://%[^/]/%[^:]:%d", protocol, host, path, &port) != 4) {
                if (sscanf(url, "%[^:]://%[^/]/%[^\n]", protocol, host, path) != 3) {
                    printf("Invalid URL\n");
                    return 1;
                }
            }
            
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
            char request[PACKET_SIZE];
            sprintf(request, "PUT /%s/%s HTTP/1.1\r\n", path, file_name);
            sprintf(request + strlen(request), "Host: %s\r\n", host);
            sprintf(request + strlen(request), "Connection: close\r\n");
            char time_str[100];
            get_time(time_str, NULL);
            sprintf(request + strlen(request), "Date: %s\r\n", time_str);
            sprintf(request + strlen(request), "Content-language: en-us\r\n");
            long int file_size = findSize(file_name);
            FILE* fp = fopen(file_name, "rb");
            if(fp == NULL){
                printf("File open error\n");
                exit(1);
            }
            sprintf(request + strlen(request), "Content-Length: %ld\r\n", file_size);
            char content_type[100];
            get_accept_type(content_type, file_name);
            sprintf(request + strlen(request), "Content-Type: %s\r\n", content_type);
            sprintf(request + strlen(request), "\r\n");
            int a = send(sockfd, request, strlen(request), 0);
            char file_content[PACKET_SIZE];
            memset(file_content, 0, PACKET_SIZE);
            int n;
            while((n = fread(file_content, 1, PACKET_SIZE - 1, fp)) > 0){
                send(sockfd, file_content, n, 0);
                memset(file_content, 0, PACKET_SIZE);
            }
            fclose(fp);
            printf("Request sent\n");
            char response[BUFFER_SIZE];
            memset(response, 0, BUFFER_SIZE);
            memset(BUFFER, 0, PACKET_SIZE);
            while(recv(sockfd, BUFFER, PACKET_SIZE, 0) > 0){
                strcat(response, BUFFER);
                memset(BUFFER, 0, PACKET_SIZE);
                char* end = strstr(response, "\r\n\r\n");
                if(end != NULL){
                    break;
                }
            }
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