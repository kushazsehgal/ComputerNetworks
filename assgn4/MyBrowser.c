#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>

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
    serv_addr.sin_family = AF_INET;
    while(1){
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("Socket creation error\n");
            exit(1);
        }
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
            // printf("Host: %s\n", host);
            // printf("Path: %s\n", path);
            // printf("Port: %s\n", port);
            // printf("Version: %s\n", version);
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
            // printf("Request\n%s", request);
            send(sockfd, request, strlen(request)+1, 0);

            // receive response
            char headers[1000];
            memset(headers, 0, 1000);
            recv(sockfd, headers, 1000, 0);
            // printf("HEADERS\n%s\n", headers);
            char* lines[100];
            char* line = strtok(headers, "\n");
            i=0;
            while(line != NULL){
                lines[i] = line;
                i++;
                line = strtok(NULL, "\n");
            }
            version = strtok(lines[0], " ");
            char* status_code = strtok(NULL, " ");
            char* status_message = strtok(NULL, " ");
            int status = atoi(status_code);
            if(status == 200){
                printf("Status: OK\n");
                char filename[1000];
                sprintf(filename, "MyOwnBrowser_%d.%s", getpid(), file_extension);
                FILE* fp = fopen(filename, "wb");
                char content[1000];
                memset(content, 0, 1000);
                int n;
                while((n = recv(sockfd, content, 1000, 0)) > 0){
                    fwrite(content, 1, n, fp);
                    memset(content, 0, 1000);
                }
                fclose(fp);
                pid_t pid = fork();
                if(pid == 0){
                    if(strcmp(file_extension, "html") == 0){
                        execlp("firefox", "firefox", filename, NULL);
                    }else if(strcmp(file_extension, "jpg") == 0){
                        execlp("eog", "eog", filename, NULL);
                    }else if(strcmp(file_extension, "pdf") == 0){
                        execlp("xdg-open", "xdg-open", filename, NULL);
                    }else if(strcmp(file_extension, "txt") == 0){
                        execlp("gedit", "gedit", filename, NULL);
                    }else{
                        execlp("xdg-open", "xdg-open", filename, NULL);
                    }
                    perror("execlp");
                    exit(1);
                }
            }
            else if(status == 304){
                printf("Status: Not Modified\n");
            }
            else if(status == 404){
                printf("Status: Not Found\n");
            }
            else if(status == 500){
                printf("Status: Internal Server Error\n");
            }
            else{
                printf("Status: Unknown\n");
            }
        }else if(strcmp(request_type, "PUT") == 0){
            char* url = http_command + 11;
            char* host, *path, *port, *version, *filename;
            if(strchr(url, ':') != NULL){
                host = strtok(url, "/");
                path = strtok(NULL, ":");
                port = strtok(NULL, " ");
                version = strtok(NULL, " ");
                if(version[0] != 'H'){
                    filename = version;
                    version = NULL;
                }else{
                    filename = strtok(NULL, " ");
                }
            }else{
                host = strtok(url, "/");
                path = strtok(NULL, " ");
                port = NULL;
                version = strtok(NULL, " ");
                if(version[0] != 'H'){
                    filename = version;
                    version = NULL;
                }else{
                    filename = strtok(NULL, " ");
                }
            }
            if(port == NULL){
                port = "80";
            }
            if(version == NULL){
                version = "HTTP/1.1";
            }
            // printf("Host: %s\n", host);
            // printf("Path: %s\n", path);
            // printf("Port: %s\n", port);
            // printf("Version: %s\n", version);
            // printf("Filename: %s\n", filename);
            serv_addr.sin_port = htons(atoi(port));
            if(inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0){
                printf("Invalid address\n");
                exit(1);
            }
            if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
                printf("Connection failed\n");
                exit(1);
            }
            FILE* fp = fopen(filename, "rb");
            if(fp == NULL){
                printf("File not found\n");
                exit(1);
            }
            char* extension = strtok(filename, ".");
            extension = strtok(NULL, ".");
            char accept_type[100];
            if(strcmp(extension, "html") == 0){
                strcpy(accept_type, "text/html");
            }else if(strcmp(extension, "jpg") == 0){
                strcpy(accept_type, "image/jpeg");
            }else if(strcmp(extension, "pdf") == 0){
                strcpy(accept_type, "application/pdf");
            }else{
                strcpy(accept_type, "text/*");
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
            sprintf(request, "PUT /%s/%s.%s %s\nHost: %s\nConnection: close\nDate: %s\nAccept: %s\nAccept-Language: en-us\nIf-Modified-Since: %s\nContent-language: en-us\nContent-length: %d\nContent-type: %s\n", path, filename, extension, version, host, date, accept_type, date2, sizeof(request), accept_type);
            printf("Request\n%s", request);
            
            send(sockfd, request, strlen(request)+1, 0);

            char content[1000];
            memset(content, 0, 1000);
            
            int n;
            printf("Content\n");
            while((n = fread(content, 1, 1000, fp)) > 0){
                printf("%s", content);
                send(sockfd, content, n, 0);
                memset(content, 0, 1000);
            }    
            // printf("\n");
            fclose(fp);

            char response[1000];
            memset(response, 0, 1000);
            recv(sockfd, response, 1000, 0);
            printf("Response\n%s", response);
            version = strtok(response, " ");
            char* status = strtok(NULL, " ");
            if(strcmp(status, "200") == 0){
                printf("Status: OK\n");
            }
            else if(strcmp(status, "304") == 0){
                printf("Status: Not Modified\n");
            }
            else if(strcmp(status, "404") == 0){
                printf("Status: Not Found\n");
            }
            else if(strcmp(status, "500") == 0){
                printf("Status: Internal Server Error\n");
            }
            else{
                printf("Status: Unknown\n");
            }

        }else{  
            printf("Invalid command\n");
        }
        close(sockfd);
    }
}

//PUT http://127.0.0.1/docs:8080 file.txt