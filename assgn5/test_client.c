#include "mysocket.h"

int main(){
    struct sockaddr_in server_addr;
    MySocket* mysock = my_socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    server_addr.sin_port = htons(8000);
    my_connect(mysock->sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    mysock->connected = mysock->sockfd;
    printf("Connected to server: %s %d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    char buff[100];
    my_recv(mysock, buff, 100);
    printf("received: %s", buff);
    myclose(mysock);
}