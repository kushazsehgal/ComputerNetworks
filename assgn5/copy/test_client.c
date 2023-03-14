#include "mysocket.h"

int main(int agrc, char* argv[]){
    struct sockaddr_in server_addr;
    MySocket* mysock = my_socket(AF_INET, SOCK_MyTCP, 0);
    server_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    server_addr.sin_port = htons(atol(argv[1]));
    my_connect(mysock, (struct sockaddr *) &server_addr, sizeof(server_addr));
    printf("Connected to server: %s %d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    char buff[100];
    my_recv(mysock, buff, 100);
    printf("received: %s\n", buff);
    memset(buff, 0, 100);
    my_recv(mysock, buff, 100);
    printf("received: %s\n", buff);
    memset(buff, 0, 100);
    my_recv(mysock, buff, 100);
    printf("received: %s\n", buff);
    // memset(buff, 0, 100);
    // printf("Enter message to send: ");
    // fgets(buff, 100, stdin);
    // my_send(mysock, buff, strlen(buff)+1);

    myclose(mysock);
}