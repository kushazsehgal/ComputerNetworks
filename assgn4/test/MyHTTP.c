#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

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
        printf("Client connectted from %s:%d with fd %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), clifd);
        if(fork() == 0){
            close(sockfd);
            char http_request[1000];
            int n = recv(clifd, http_request, 1000, 0);
            printf("received %d bytes request\n", n);
            // printf("Request recvd\n%s\n", http_request);
            // split the request into lines
            char* lines[100];
            char* line = strtok(http_request, "\n");
            int i=0;
            while(line != NULL){
                lines[i] = line;
                i++;
                line = strtok(NULL, "\n");
            }
            // if(i == 0){
            //     close(clifd);
            //     printf("Empty request\n");
            //     exit(1);
            // }
            // request line
            if(strncmp(lines[0], "GET", 3) == 0){
                char* path = strtok(lines[0], " ");
                path = strtok(NULL, " ");
                char* version = strtok(NULL, " ");
                if(version == NULL){
                    version = "HTTP/1.1";
                }
                // printf("Path: %s\n", path);
                // printf("Version: %s\n", version);

                // check if file exists
                int fd = open(path+1, O_RDONLY);
                if(fd < 0){
                    printf("File not found\n");
                    char* response = "HTTP/1.1 404 Not Found\n";
                    send(clifd, response, strlen(response)+1, 0);
                    close(clifd);
                    exit(1);
                }
                // headers
                char content_type[100] = "text/*";
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
                            char* response = "HTTP/1.1 304 Not Modified\n";
                            send(clifd, response, strlen(response)+1, 0);
                            close(clifd);
                            exit(1);
                        }
                    }
                    else if(strcmp(header, "Content-Type") == 0){
                        strcpy(content_type, value);
                    }
                }
                // send response
                char response[10000];
                time_t ct;
                struct tm* tm;
                time(&ct);
                tm = localtime(&ct);
                time_t ct2;
                ct2 = ct + 3*24*60*60;
                tm = localtime(&ct2);
                char expire_date[100];
                struct stat st;
                stat(path+1, &st);
                time_t last_modified = st.st_mtime;
                strftime(expire_date, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
                char last_modified_date[100];
                tm = localtime(&last_modified);
                strftime(last_modified_date, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
                sprintf(response, "%s 200 OK\nExpires: %s\nCache-control: no-store\nContent-Language: en-us\nContent-length: %d\nContent-type: %s\nLast modified: %s\n", version, expire_date, 1000, content_type, last_modified_date);
                // printf("Response:\n%s\n", response);
                send(clifd, response, strlen(response)+1, 0);
                char file_content[1000];
                memset(file_content, 0, 1000);
                close(fd);
                FILE* fp = fopen(path+1, "rb");
                int n;
                while((n = fread(file_content, 1, 1000, fp)) > 0){
                    send(clifd, file_content, n, 0);
                    memset(file_content, 0, 1000);
                }
                fclose(fp);
            }
            else if(strncmp(lines[0], "PUT", 3) == 0){
                char* path = strtok(lines[0], " ");
                path = strtok(NULL, " ");
                char* version = strtok(NULL, " ");
                if(version == NULL){
                    version = "HTTP/1.1";
                }
                printf("Path: %s\n", path);
                printf("Version: %s\n", version);
                FILE* fp = fopen(path+1, "wb");
                char file_content[1000];
                memset(file_content, 0, 1000);
                // int n;
                // printf("Receiving file\n");
                // while((n = recv(clifd, file_content, 1000, 0)) > 0){
                //     printf("%s", file_content);
                //     fwrite(file_content, 1, n, fp);
                //     memset(file_content, 0, 1000);
                // }
                n = recv(clifd, file_content, 1000, 0);
                // printf("n = %d\n", n);
                printf("recvd %d bytes content\n", n);
                printf("%s\n", file_content);
                fwrite(file_content, 1, 1000, fp);
                fclose(fp);
                printf("\nFile received\n");
                // headers
                char content_type[100] = "text/*";
                for(int i=1; i<100; i++){
                    if(lines[i] == NULL) break;
                    char* header = strtok(lines[i], ":");
                    char* value = strtok(NULL, ":");
                    if(header == NULL || value == NULL) break;
                    if(strcmp(header, "Content-Type") == 0){
                        strcpy(content_type, value);
                    }
                }
                // send response
                char response[10000];
                time_t ct;
                struct tm* tm;
                time(&ct);
                tm = localtime(&ct);
                time_t ct2;
                ct2 = ct + 3*24*60*60;
                tm = localtime(&ct2);
                char expire_date[100];
                struct stat st;
                stat(path+1, &st);
                time_t last_modified = st.st_mtime;
                strftime(expire_date, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
                char last_modified_date[100];
                tm = localtime(&last_modified);
                strftime(last_modified_date, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
                sprintf(response, "%s 200 OK\nExpires: %s\nCache-control: no-store\nContent-Language: en-us\nContent-length: %d\nContent-type: %s\nLast modified: %s\n", version, expire_date, 1000, content_type, last_modified_date);
                // printf("Response:\n%s\n", response);
                send(clifd, response, strlen(response)+1, 0);
            }
            close(clifd);
            exit(0);
        }
        close(clifd);
    }
    return 0;
}
// GET http://127.0.0.1/docs/file.txt:8080
// GET http://127.0.0.1/Assgn-4.pdf:8080
// GET http://127.0.0.1/sample.html:8080
// GET http://127.0.0.1/img.jpg:8080