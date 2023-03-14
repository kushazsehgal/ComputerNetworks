#include "mysocket.h"

int main(int agrc, char* argv[]){

    MySocket* mysock = my_socket(AF_INET, SOCK_MyTCP, 0);
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atol(argv[1]));
    my_bind(mysock, (struct sockaddr *) &server_addr, sizeof(server_addr));
    my_listen(mysock, 5);
    printf("Server listening on port %ld\n", atol(argv[1]));
    // while(1){
        // int client_len = sizeof(client_addr);
        socklen_t client_len = sizeof(client_addr);
        my_accept(mysock, (struct sockaddr *) &client_addr, &client_len);
        printf("mysock->connected: %d\n", mysock->cli_sockfd);
        printf("Client connected: %s %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        char buff[100];
        memset(buff, 0, 100);
        // strcpy(buff, "Send Load");
        // printf("Enter message to send: ");
        // fgets(buff, 100, stdin);
        // my_send(mysock, buff, strlen(buff)+1);
        // printf("Bhai yeh kya hai\n");
        // memset(buff, 0, 100);
        // my_recv(mysock, buff, 100);
        // printf("received: %s\n", buff);
        strcpy(buff, "Send Load1");
        my_send(mysock, buff, strlen(buff)+1);
        strcpy(buff, "Send Load2");
        my_send(mysock, buff, strlen(buff)+1);
        strcpy(buff, "Send Load3");
        my_send(mysock, buff, strlen(buff)+1);
        
        // sleep(3);
        // close(newsockfd);
    // }
    myclose(mysock);
    return 0;
}

