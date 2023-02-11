#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>

int main(){
    int sockfd, clifd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Socket creation error\n");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);

    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Bind error\n");
        exit(1);
    }

    if(listen(sockfd, 5) < 0){
        printf("Listen error\n");
        exit(1);
    }

    while(1){
        clifd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
        if(clifd < 0){
            printf("Accept error\n");
            exit(1);
        }
        if(fork() == 0){
            close(sockfd);
            char http_request[1000];
            recv(clifd, http_request, 1000, 0);
            printf("Request recvd\n%s\n", http_request);
            // split the request into lines
            char* lines[100];
            char* line = strtok(http_request, "\n");
            int i=0;
            while(line != NULL){
                lines[i] = line;
                i++;
                line = strtok(NULL, "\n");
            }
            if(i == 0){
                close(clifd);
                printf("Empty request\n");
                exit(1);
            }
            // request line
            if(strncmp(lines[0], "GET", 3) == 0){
                char* path = strtok(lines[0], " ");
                path = strtok(NULL, " ");
                char* version = strtok(NULL, " ");
                if(version == NULL){
                    version = "HTTP/1.1";
                }
                printf("Path: %s\n", path);
                printf("Version: %s\n", version);

                // check if file exists
                FILE* fp = fopen(path+1, "r");
                if(fp == NULL){
                    printf("File not found\n");
                    close(clifd);
                    exit(1);
                }
                // headers
                for(int i=1; i<100; i++){
                    if(lines[i] == NULL) break;
                    char* header = strtok(lines[i], ":");
                    char* value = strtok(NULL, ":");
                    if(header == NULL || value == NULL) break;
                    if(strcmp(header, "If-Modified-Since") == 0){
                        struct tm tm;
                        strptime(value, "%a, %d %b %Y %H:%M:%S %Z", &tm);
                        time_t if_modified_since = mktime(&tm);
                        struct stat st;
                        stat(path+1, &st);
                        time_t last_modified = st.st_mtime;
                        if(last_modified <= if_modified_since){
                            printf("File not modified since %s\n", value);
                            close(clifd);
                            exit(1);
                        }
                    }
                }
            }
            else{
                printf("Invalid request\n");
            }
            close(clifd);
            exit(0);
        }
        close(clifd);
    }
    return 0;
}
// GET http://127.0.0.1/file.txt:8080