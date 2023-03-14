#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#define SOCK_MyTCP -1
#define MAX_MESSAGE_SIZE 5000
#define MAX_SEND_SIZE 1000
#define MAX_RECV_SIZE 1000
#define HEADER_SIZE 5
#define MAX_SEND_NUM 5
#define MAX_RECV_NUM 5
#define SEND_THREAD_SLEEP_TIME 1

typedef struct 
{
    int sockfd;
    pthread_t send_thread;
    pthread_t recv_thread;
    int cli_sockfd;
    pthread_mutex_t send_mutex;
    pthread_mutex_t recv_mutex;
    pthread_cond_t send_cond_added;
    pthread_cond_t send_cond_removed;
    pthread_cond_t recv_cond_added;
    pthread_cond_t recv_cond_removed;
    char** send_buffer;
    char** recv_buffer;
    int send_seq;
    int recv_seq;
}MySocket;

MySocket* my_socket(int domain, int type, int protocol);
int my_bind(MySocket* mysock , const struct sockaddr *addr, socklen_t addrlen);
int my_listen(MySocket* mysock, int backlog);
int my_accept(MySocket* mysock, struct sockaddr *addr, socklen_t *addrlen);
int my_connect(MySocket* mysock, const struct sockaddr *addr, socklen_t addrlen);
int my_send(MySocket* mysock, char* buffer, int size);
int my_recv(MySocket* mysock, char* buffer, int size);
int myclose(MySocket* mysock);
void* send_runner(void* arg);
void* recv_runner(void* arg);






