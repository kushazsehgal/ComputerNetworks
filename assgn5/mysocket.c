#include "mysocket.h"
void* send_runner(void* arg)
{
    MySocket* mysock = (MySocket*)arg;
    while(1)
    {
        // sleep(1);
        pthread_mutex_lock(&mysock->send_mutex);
        if(mysock->send_seq == 0){
            pthread_mutex_unlock(&mysock->send_mutex);
            continue;
        }
        // printf("Sending %d messages\n", mysock->send_seq);
        while(mysock->connected == 0);
        printf("thread mysock->connected: %d\n", mysock->connected);
        for(int i=0; i<mysock->send_seq; i++){
            int ret = send(mysock->connected, mysock->send_buffer[i], strlen(mysock->send_buffer[i])+1, 0);
            printf("sent [thread]: %s\n", mysock->send_buffer[i]);
            if(ret < 0)
            {
                perror("Send failed");
                exit(1);
            }
        }

        mysock->send_seq = 0;
        for(int i=0; i<MAX_SEND_NUM; i++){
            memset(mysock->send_buffer[i], 0, MAX_SEND_SIZE);
        }
        
        pthread_mutex_unlock(&mysock->send_mutex);
        pthread_cond_signal(&mysock->send_cond);
    }
}

void* recv_runner(void* arg)
{
    MySocket* mysock = (MySocket*)arg;
    while(1)
    {   
        pthread_mutex_lock(&mysock->recv_mutex);
        char buffer[MAX_MESSAGE_SIZE];
        while(mysock->connected==0);
        printf("recv thread mysock->connected: %d\n", mysock->connected);
        int ret = recv(mysock->connected, buffer, MAX_MESSAGE_SIZE, 0);
        if(ret < 0)
        {
            perror("Recv failed");
            exit(1);
        }
        printf("Received [thread]: %s\n", buffer);
        int i = 0;
        while(i < strlen(buffer)){
            int j = 0;
            while(j < MAX_RECV_SIZE && i < strlen(buffer)){
                mysock->recv_buffer[mysock->recv_seq][j] = buffer[i];
                j++;
                i++;
            }
            mysock->recv_seq = (mysock->recv_seq + 1);
        }
        
        pthread_mutex_unlock(&mysock->recv_mutex);
        pthread_cond_signal(&mysock->recv_cond);
    }
}

MySocket* my_socket(int domain, int type, int protocol)
{
    MySocket* mysock = (MySocket*)malloc(sizeof(MySocket));
    mysock->sockfd = socket(domain, type, protocol);
    if(mysock->sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    mysock->send_buffer = (char**)malloc(MAX_SEND_NUM*sizeof(char*));
    mysock->recv_buffer = (char**)malloc(MAX_RECV_NUM*sizeof(char*));
    for(int i=0; i<MAX_SEND_NUM; i++)
    {
        mysock->send_buffer[i] = (char*)malloc(MAX_SEND_SIZE*sizeof(char));
    }
    for(int i=0; i<MAX_RECV_NUM; i++)
    {
        mysock->recv_buffer[i] = (char*)malloc(MAX_RECV_SIZE*sizeof(char));
    }

    mysock->send_seq = 0;
    mysock->recv_seq = 0;

    mysock->connected = 0;

    pthread_mutex_init(&mysock->send_mutex, NULL);
    pthread_mutex_init(&mysock->recv_mutex, NULL);
    pthread_cond_init(&mysock->send_cond, NULL);
    pthread_cond_init(&mysock->recv_cond, NULL);

    pthread_create(&mysock->send_thread, NULL, send_runner, (void*)mysock);
    pthread_create(&mysock->recv_thread, NULL, recv_runner, (void*)mysock);

    return mysock;
}

int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int ret = bind(sockfd, addr, addrlen);
    if(ret < 0)
    {
        perror("Bind failed");
        exit(1);
    }
    return ret;
}

int my_listen(int sockfd, int backlog){
    int ret = listen(sockfd, backlog);
    if(ret < 0)
    {
        perror("Listen failed");
        exit(1);
    }
    return ret;
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int ret = accept(sockfd, addr, addrlen);
    if(ret < 0)
    {
        perror("Accept failed");
        exit(1);
    }
    return ret;
}

int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int ret = connect(sockfd, addr, addrlen);
    if(ret < 0)
    {
        perror("Connect failed");
        exit(1);
    }
    return ret;
}

int myclose(MySocket* mysock){
    mysock->connected = 0;
    
    pthread_cancel(mysock->send_thread);
    pthread_cancel(mysock->recv_thread);
    pthread_mutex_destroy(&mysock->send_mutex);
    pthread_mutex_destroy(&mysock->recv_mutex);
    pthread_cond_destroy(&mysock->send_cond);
    pthread_cond_destroy(&mysock->recv_cond);

    for(int i=0; i<MAX_SEND_NUM; i++)
    {
        free(mysock->send_buffer[i]);
    }
    for(int i=0; i<MAX_RECV_NUM; i++)
    {
        free(mysock->recv_buffer[i]);
    }
    free(mysock->send_buffer);
    free(mysock->recv_buffer);
    close(mysock->sockfd);
    
    free(mysock);
    return 0;
}




int my_send(MySocket* mysock, char* buffer, int size)
{
    if(size > MAX_MESSAGE_SIZE)
    {
        printf("Send size too large");
        return -1;
    }
    pthread_mutex_lock(&mysock->send_mutex);
    while(mysock->send_seq == MAX_SEND_NUM){
        pthread_cond_wait(&mysock->send_cond, &mysock->send_mutex);
    }
    int i = 0;
    while(i < size){
        int j = 0;
        while(j < MAX_SEND_SIZE && i < size){
            mysock->send_buffer[mysock->send_seq][j] = buffer[i];
            j++;
            i++;
        }
        mysock->send_seq = (mysock->send_seq + 1);
    }
    pthread_mutex_unlock(&mysock->send_mutex);
    // pthread_cond_signal(&mysock->send_cond);
    return size;
}

int my_recv(MySocket* mysock, char* buffer, int size)
{
    if(size > MAX_MESSAGE_SIZE)
    {
        printf("Recv size too large");
        return -1;
    }
    pthread_mutex_lock(&mysock->recv_mutex);
    while(mysock->recv_seq == 0){
        pthread_cond_wait(&mysock->recv_cond, &mysock->recv_mutex);
    }

    int i = 0;
    int k = 0;
    while(i < size && k < mysock->recv_seq){
        int j = 0;
        while(j < MAX_RECV_SIZE && i < size){
            buffer[i] = mysock->recv_buffer[k][j];
            j++;
            i++;
        }
        k++;
    }

    mysock->recv_seq = 0;

    for(int i=0; i<MAX_RECV_NUM; i++)
    {
        memset(mysock->recv_buffer[i], 0, MAX_RECV_SIZE);
    }

    pthread_mutex_unlock(&mysock->recv_mutex);
    return size;
}







