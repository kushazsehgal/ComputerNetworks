#include "mysocket.h"

int main(){

    MySocket* mysock = my_socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8000);
    my_bind(mysock->sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    my_listen(mysock->sockfd, 5);
    printf("Server listening on port %d\n", 8000);
    // while(1){
        // int client_len = sizeof(client_addr);
        socklen_t client_len = sizeof(client_addr);
        int newsockfd = my_accept(mysock->sockfd, (struct sockaddr *) &client_addr, &client_len);
        mysock->connected = newsockfd;
        printf("mysock->connected: %d\n", mysock->connected);
        printf("Client connected: %s %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        char buff[100];
        strcpy(buff, "Send Load");
        my_send(mysock, buff, strlen(buff)+1);
        memset(buff, 0, 100);
        close(newsockfd);
    // }
    myclose(mysock);
    return 0;
}

