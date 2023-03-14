/////////////////////////////////////////
// Networks Lab Assignment 4 : My HTTP
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
#include <time.h>
#include <arpa/inet.h>
#include<stdbool.h>
#define BUFFER_SIZE 1000000
#define PACKET_SIZE 1000

char BUFFER[PACKET_SIZE];
char TOT_BUFFER[BUFFER_SIZE];

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
void get_time(char *time_str, char* expiry_time){
    time_t ct, ct2;
    struct tm* tm;
    time(&ct);
    tm = localtime(&ct);
    strftime(time_str, 100, "%a, %d %b %Y %H:%M:%S %Z", tm);
    time(&ct2);
    ct2 = ct + 3*24*60*60;
    tm = localtime(&ct2);
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
    FILE* log = fopen("AccessLog.txt","a");
    while(1){
        memset(BUFFER, 0, PACKET_SIZE);
        memset(TOT_BUFFER, 0, BUFFER_SIZE);
        clifd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);
        if(clifd < 0){
            printf("Accept error\n");
            exit(1);
        }
        printf("Client connected from %s:%d with fd %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), clifd);
        if(fork() == 0){
            close(sockfd);
            int bytes_read = 0;
            int total_bytes_read = 0;
            bool header_found = false;
            char* end_of_header;
            int j = 0;
            while((bytes_read = recv(clifd, BUFFER, PACKET_SIZE, 0)) > 0){
                total_bytes_read += bytes_read;
                for(int k=0; k<bytes_read; k++){
                    TOT_BUFFER[j++] = BUFFER[k];
                }
                memset(BUFFER, 0, PACKET_SIZE);
                end_of_header = strstr(TOT_BUFFER, "\r\n\r\n");
                if(end_of_header != NULL){
                    end_of_header += 4;
                    break;
                }
            }
            int file_bytes_read = total_bytes_read - (end_of_header - TOT_BUFFER);
            char http_request[PACKET_SIZE];
            char *temp = TOT_BUFFER;
            while(temp != end_of_header){
                http_request[temp - TOT_BUFFER] = *temp;
                temp++;
            }
            http_request[temp - TOT_BUFFER] = '\0';
            printf("Request:\n%s\n",http_request);
            time_t t;
            t = time(NULL);
            struct tm tm = *localtime(&t);
            if(strncmp(http_request, "GET", 3) == 0){
                // get file path
                char file_path[PACKET_SIZE];
                sscanf(http_request, "GET %s", file_path);
                fprintf(log,"%d/%d/%d::%d:%d:%d::%s::%d::%s::%s\n",tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900,tm.tm_hour, tm.tm_min, tm.tm_sec,inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port),"GET",file_path);
                long int file_size = findSize(file_path+1);
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
                char http_response[PACKET_SIZE];
                sprintf(http_response, "HTTP/1.1 200 OK\r\n");
                char time_str[100], expiry_time[100];
                get_time(time_str, expiry_time);
                sprintf(http_response+strlen(http_response), "Expires: %s\r\n", expiry_time);
                sprintf(http_response+strlen(http_response), "Cache-Control: no-store\r\n");
                sprintf(http_response+strlen(http_response), "Content-language: en-us\r\n");
                // get file size in bytes
                sprintf(http_response+strlen(http_response), "Content-Length: %ld\r\n", file_size);
                // get accept type
                char accept_type[100];
                get_accept_type(accept_type, file_path);
                sprintf(http_response+strlen(http_response), "Content-Type: %s\r\n", accept_type);
                sprintf(http_response+strlen(http_response), "Last-Modified: %s\r\n", last_modified);
                sprintf(http_response+strlen(http_response), "\r\n");

                int nbytes = send(clifd, http_response, strlen(http_response), 0);
                
                // send file in chunks of 1024 bytes
                memset(BUFFER, 0, PACKET_SIZE);
                while((nbytes = fread(BUFFER, 1, PACKET_SIZE, fp)) > 0){
                    send(clifd, BUFFER, nbytes, 0);
                    memset(BUFFER, 0, PACKET_SIZE);
                }
                fclose(fp);
            }
            else if(strncmp(http_request, "PUT", 3) == 0){
                char file_path[PACKET_SIZE];
                sscanf(http_request, "PUT %s", file_path);
                fprintf(log,"%d/%d/%d::%d:%d:%d::%s::%d::%s::%s\n",tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900,tm.tm_hour, tm.tm_min, tm.tm_sec,inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port),"PUT",file_path);
                FILE* fp = fopen(file_path+1, "wb");
                if(fp == NULL){
                    printf("File not found\n");
                    char* not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
                    send(clifd, not_found, strlen(not_found), 0);
                    printf("Request sent\n");
                    close(clifd);
                    exit(1);
                }
                // get content length
                char* content_length = strstr(http_request, "Content-Length");
                int content_length_val;
                sscanf(content_length, "Content-Length: %d", &content_length_val);

                char http_response[PACKET_SIZE];
                sprintf(http_response, "HTTP/1.1 200 OK\r\n");
                
                // // receive file in chunks of 1024 bytes
                char file_content[PACKET_SIZE];
                memset(file_content, 0, PACKET_SIZE);
                int nbytes;
                int iters = 0;
                int total_file_bytes_read = 0;
                if(file_bytes_read > 0){
                    fwrite(end_of_header, 1, file_bytes_read, fp);
                    total_file_bytes_read += file_bytes_read;
                }
                while(total_file_bytes_read < content_length_val){
                    nbytes = recv(clifd, file_content, PACKET_SIZE - 1, 0);
                    total_file_bytes_read += nbytes;
                    fwrite(file_content, 1, nbytes, fp);
                    memset(file_content, 0, PACKET_SIZE);
                }
                fclose(fp);
                send(clifd, http_response, strlen(http_response), 0);
            }
            close(clifd);
            exit(0);
        }
        close(clifd);
    }
    fclose(log);
}