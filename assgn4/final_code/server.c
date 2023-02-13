#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>

// PUT http://127.0.0.1/docs:8080 file.txt
#define MAX_BUFFER_SIZE 1024

void get_time(char *time_str, char* expiry_time){
    time_t ct, ct2;
    struct tm* tm;
    time(&ct);
    tm = localtime(&ct);
    strftime(time_str, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
    time(&ct2);
    ct2 = ct + 3*24*60*60;
    tm = gmtime(&ct2);
    strftime(expiry_time, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
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

int main(int argc, char* argv[]){
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

    if(argc == 2){
        serv_addr.sin_port = htons(atoi(argv[1]));
    }

    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Could not bind on port %d\n", ntohs(serv_addr.sin_port));
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
            char http_request[MAX_BUFFER_SIZE];
            int bytes_read = recv(clifd, http_request, MAX_BUFFER_SIZE, 0);
            if(strlen(http_request) == 0){
                close(clifd);
                printf("Empty request\n");
                exit(1);
            }
            // printf("Request:\n");
            // for(int i=0; i<MAX_BUFFER_SIZE; i++){
            //     printf("%c", http_request[i]);
            // }
            // printf("\n");
            // get end of header
            char* end_of_header = strstr(http_request, "\r\n\r\n");
            end_of_header += 4;
            // printf("%s\n",  end_of_header+4);
            // printf("End of header: %s\n", end_of_header);
            // int read_already = strlen(end_of_header);
            // printf("Read already: %d\n", read_already);
            if(strncmp(http_request, "GET", 3) == 0){
                // get file path
                char file_path[MAX_BUFFER_SIZE];
                sscanf(http_request, "GET %s", file_path);
                FILE* fp = fopen(file_path+1, "rb");
                if(fp == NULL){
                    printf("File not found\n");
                    char* not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
                    send(clifd, not_found, strlen(not_found), 0);
                    close(clifd);
                    exit(1);
                }
                // check for If-Modified-Since
                char* if_modified_since = strstr(http_request, "If-Modified-Since");
                char last_modified[100];
                if(if_modified_since != NULL){
                    // get the date
                    char date[100];
                    sscanf(if_modified_since, "If-Modified-Since: %s", date);
                    struct tm tm;
                    strptime(date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
                    time_t if_modified_since_time = mktime(&tm);
                    // get the last modified time of the file
                    struct stat st;
                    stat(file_path+1, &st);
                    time_t last_modified_time = st.st_mtime;
                    strftime(last_modified, 100, "%a, %d %b %Y %H:%M:%S %Z", localtime(&last_modified_time));
                    if(last_modified_time <= if_modified_since_time){
                        // file not modified
                        char* not_modified = "HTTP/1.1 304 Not Modified\r\n\r\n";
                        send(clifd, not_modified, strlen(not_modified), 0);
                        close(clifd);
                        exit(1);
                    }
                }
                char http_response[MAX_BUFFER_SIZE];
                sprintf(http_response, "HTTP/1.1 200 OK\r\n");
                char time_str[100], expiry_time[100];
                get_time(time_str, expiry_time);
                sprintf(http_response+strlen(http_response), "Expires: %s\r\n", expiry_time);
                sprintf(http_response+strlen(http_response), "Cache-Control: no-store\r\n");
                sprintf(http_response+strlen(http_response), "Content-language: en-us\r\n");
                // get file size in bytes
                fseek(fp, 0, SEEK_END);
                int file_size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                sprintf(http_response+strlen(http_response), "Content-Length: %d\r\n", file_size);
                // get accept type
                char accept_type[100];
                get_accept_type(accept_type, file_path);
                sprintf(http_response+strlen(http_response), "Content-Type: %s\r\n", accept_type);
                sprintf(http_response+strlen(http_response), "Last-Modified: %s\r\n", last_modified);
                sprintf(http_response+strlen(http_response), "\r\n");

                int nbytes = send(clifd, http_response, strlen(http_response), 0);
                
                // send file in chunks of 1024 bytes
                char file_content[MAX_BUFFER_SIZE];
                memset(file_content, 0, MAX_BUFFER_SIZE);
                while((nbytes = fread(file_content, 1, MAX_BUFFER_SIZE, fp)) > 0){
                    send(clifd, file_content, nbytes, 0);
                    memset(file_content, 0, MAX_BUFFER_SIZE);
                }
                fclose(fp);
            }
            else if(strncmp(http_request, "PUT", 3) == 0){
                char file_path[MAX_BUFFER_SIZE];
                sscanf(http_request, "PUT %s", file_path);
                FILE* fp = fopen(file_path+1, "wb");
                if(fp == NULL){
                    printf("File not found\n");
                    char* not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
                    send(clifd, not_found, strlen(not_found), 0);
                    close(clifd);
                    exit(1);
                }
                // get content length
                char* content_length = strstr(http_request, "Content-Length");
                int content_length_val;
                sscanf(content_length, "Content-Length: %d", &content_length_val);

                char http_response[MAX_BUFFER_SIZE];
                sprintf(http_response, "HTTP/1.1 200 OK\r\n");
                
                // // receive file in chunks of 1024 bytes
                char file_content[MAX_BUFFER_SIZE];
                memset(file_content, 0, MAX_BUFFER_SIZE);
                int nbytes;
                int iters = 0;

                fwrite(end_of_header, 1, strlen(end_of_header), fp);

                // while((nbytes = recv(clifd, file_content, MAX_BUFFER_SIZE, 0)) > 0){
                //     printf("Iteration %d\n", ++iters);
                //     printf("bytes recvd : %d\n", nbytes);
                //     fwrite(file_content, 1, nbytes, fp);
                //     memset(file_content, 0, MAX_BUFFER_SIZE);
                // }
                // fwrite(http_request, 1, bytes_read, fp);

                if(content_length_val > strlen(end_of_header)){
                    // int remaining = content_length_val - strlen(end_of_header);
                    // while(remaining > 0){
                    //     nbytes = recv(clifd, file_content, MAX_BUFFER_SIZE, 0);
                    //     // check for eof
                    //     if(nbytes == 0){
                    //         break;
                    //     }
                    //     fwrite(file_content, 1, nbytes, fp);
                    //     memset(file_content, 0, MAX_BUFFER_SIZE);
                    //     remaining -= nbytes;
                    // }
                    while((nbytes = recv(clifd, file_content, MAX_BUFFER_SIZE, 0)) > 0){
                        fwrite(file_content, 1, nbytes, fp);
                        memset(file_content, 0, MAX_BUFFER_SIZE);
                    }
                }
                
                fclose(fp);
                send(clifd, http_response, strlen(http_response), 0);
            }
            close(clifd);
            exit(0);
        }
        close(clifd);
    }
}