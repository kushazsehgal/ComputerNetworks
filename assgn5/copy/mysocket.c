#include "mysocket.h"


MySocket* my_socket(int domain,int type,int protocol){
    if(type != SOCK_MyTCP){
        perror("Invalid Socket type : allowed types are : MyTCP\n");
    }
    MySocket *mysock = (MySocket*)malloc(sizeof(MySocket));
    mysock->sockfd = socket(domain,SOCK_STREAM,protocol);
    if(mysock->sockfd < 0){
            perror("Socket creation failed");
            exit(1);
    }
    
    mysock->send_buffer = (char**)malloc(sizeof(char*)*MAX_SEND_NUM);
    mysock->recv_buffer = (char**)malloc(sizeof(char*)*MAX_RECV_NUM);

    for(int i = 0;i < MAX_SEND_NUM;i++){
        *(mysock->send_buffer + i) = (char*)malloc(sizeof(char)*(MAX_MESSAGE_SIZE + HEADER_SIZE));
    }
    for(int i = 0;i < MAX_RECV_NUM;i++){
        *(mysock->recv_buffer + i) = (char*)malloc(sizeof(char)*(MAX_MESSAGE_SIZE + HEADER_SIZE));
    }

    if (pthread_mutex_init(&mysock->send_mutex, NULL) != 0) {
        perror("pthread_mutex_init() error");
        exit(3);
    }
    if (pthread_mutex_init(&mysock->recv_mutex, NULL) != 0) {
        perror("pthread_mutex_init() error");
        exit(3);
    }
    if (pthread_cond_init(&mysock->send_cond_added, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
        exit(1);                                                                    
    }
    if (pthread_cond_init(&mysock->send_cond_removed, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
        exit(1);                                                                    
    }
    if (pthread_cond_init(&mysock->recv_cond_added, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
        exit(1);                                                                    
    }
    if (pthread_cond_init(&mysock->recv_cond_removed, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
        exit(1);                                                                    
    }
    mysock->send_seq = 0;
    mysock->recv_seq = 0;

    mysock->send_head = 0;
    mysock->send_tail = 0;
    mysock->recv_head = 0;
    mysock->recv_tail = 0;

    pthread_create(&mysock->send_thread,NULL,send_runner,(void*)(mysock));
    pthread_create(&mysock->recv_thread,NULL,recv_runner,(void*)(mysock));
    
    return mysock;
}

int myclose(MySocket* mysock){
    while(mysock->send_seq != 0 || mysock->recv_seq != 0){
        sleep(0.01);
    }
    printf("Closing socket\n");
    pthread_cancel(mysock->send_thread);
    pthread_cancel(mysock->recv_thread);
    pthread_mutex_destroy(&mysock->send_mutex);
    pthread_mutex_destroy(&mysock->recv_mutex);
    pthread_cond_destroy(&mysock->send_cond_added);
    pthread_cond_destroy(&mysock->send_cond_removed);
    pthread_cond_destroy(&mysock->recv_cond_added);
    pthread_cond_destroy(&mysock->recv_cond_removed);

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
    close(mysock->cli_sockfd);
    free(mysock);
    return 0;
}
int my_connect(MySocket* mysock, const struct sockaddr *addr, socklen_t addrlen){
    int ret = connect(mysock->sockfd, addr, addrlen);
    if(ret < 0)
    {
        perror("Connect failed");
        exit(1);
    }
    mysock->cli_sockfd = mysock->sockfd;
    printf("thread mysock->cli_sockfd: %d\n", mysock->cli_sockfd);
    return ret;
}

int my_accept(MySocket* mysock, struct sockaddr *addr, socklen_t *addrlen){
    int ret = accept(mysock->sockfd, addr, addrlen);
    if(ret < 0)
    {
        perror("Accept failed");
        exit(1);
    }
    mysock->cli_sockfd = ret;
    return ret;
}
int my_listen(MySocket* mysock, int backlog){
    int ret = listen(mysock->sockfd, backlog);
    if(ret < 0)
    {
        perror("Listen failed");
        exit(1);
    }
    return ret;
}
int my_bind(MySocket* mysock, const struct sockaddr *addr, socklen_t addrlen){
    int ret = bind(mysock->sockfd, addr, addrlen);
    if(ret < 0)
    {
        perror("Bind failed");
        exit(1);
    }
    return ret;
}

int my_send(MySocket* mysock, char* buffer, int size){
    if(size > MAX_MESSAGE_SIZE)
    {
        printf("Send size too large");
        return -1;
    }
    pthread_mutex_lock(&mysock->send_mutex);
    while(mysock->send_seq == MAX_SEND_SIZE){
        pthread_cond_wait(&mysock->send_cond_removed,&mysock->send_mutex);
    }
    int size_copy = size;
    // mysock->send_buffer[mysock->send_seq][HEADER_SIZE - 1] = '\0';
    // for(int i = HEADER_SIZE - 2;i >= 0;i--){
    //     mysock->send_buffer[mysock->send_seq][i] = '0' + (size_copy%10);
    //     size_copy /= 10;
    // }
    // for(int i = HEADER_SIZE;i < HEADER_SIZE + size;i++)
    //     mysock->send_buffer[mysock->send_seq][i] = buffer[i - HEADER_SIZE];
    // mysock->send_seq++;
    // printf("Send Seq is non zero : %d\n",mysock->send_seq);
    mysock->send_buffer[mysock->send_head][HEADER_SIZE - 1] = '\0';
    for(int i = HEADER_SIZE - 2;i >= 0;i--){
        mysock->send_buffer[mysock->send_head][i] = '0' + (size_copy%10);
        size_copy /= 10;
    }
    for(int i = HEADER_SIZE;i < HEADER_SIZE + size;i++)
        mysock->send_buffer[mysock->send_head][i] = buffer[i - HEADER_SIZE];
    mysock->send_head = (mysock->send_head + 1) % MAX_SEND_NUM;
    mysock->send_seq++;

    pthread_mutex_unlock(&mysock->send_mutex);
    pthread_cond_signal(&mysock->send_cond_added);
    return size;
}
int my_recv(MySocket* mysock, char* buffer, int size){
    if(size > MAX_MESSAGE_SIZE){
        printf("Recv size too large");
        return -1;
    }
    printf("Mutex is Locked\n");
    pthread_mutex_lock(&mysock->recv_mutex);
    printf("Mutex Unlocked\n");
    // printf("Before Recv Seq is zero : %d\n",mysock->recv_seq);
    while(mysock->recv_seq == 0){
        pthread_cond_wait(&mysock->recv_cond_added,&mysock->recv_mutex);
    }
    // printf("After Recv Seq is zero : %d\n",mysock->recv_seq);
    // printf("Recv Seq is non zero : %d\n",mysock->recv_seq);
    // int idx = mysock->recv_seq - 1;
    // int i = 0;
    // int buffer_size = atoi(mysock->recv_buffer[idx]);
    // printf("Buffer Size : %d\n",buffer_size);
    // for(int j = HEADER_SIZE;j < HEADER_SIZE + min(buffer_size,size);j++)
    //     buffer[i++] = mysock->recv_buffer[idx][j];
    // printf("Copied into buffer : %d\n",i);
    // mysock->recv_seq--;
    // memset(mysock->recv_buffer[idx],0,MAX_MESSAGE_SIZE + HEADER_SIZE);
    int idx = mysock->recv_tail;
    int i = 0;
    int buffer_size = atoi(mysock->recv_buffer[idx]);
    printf("Buffer Size : %d\n",buffer_size);
    for(int j = HEADER_SIZE;j < HEADER_SIZE + min(buffer_size,size);j++)
        buffer[i++] = mysock->recv_buffer[idx][j];
    printf("Copied into buffer : %d\n",i);
    mysock->recv_tail = (mysock->recv_tail + 1) % MAX_RECV_NUM; 
    mysock->recv_seq--;
    memset(mysock->recv_buffer[idx],0,MAX_MESSAGE_SIZE + HEADER_SIZE);

    pthread_mutex_unlock(&mysock->recv_mutex);
    pthread_cond_signal(&mysock->recv_cond_removed);
    return min(buffer_size,size);
}

void* send_runner(void *arg){
    MySocket* mysock = (MySocket*) arg;
    while(mysock->cli_sockfd == 0);
    while(1){
        pthread_mutex_lock(&mysock->send_mutex);
        printf("Send Seq Before : %d\n",mysock->send_seq);
        while(mysock->send_seq == 0){
            pthread_cond_wait(&mysock->send_cond_added,&mysock->send_mutex);
        }
        // int idx = mysock->send_seq - 1;
        // int left = atoi(mysock->send_buffer[idx]) + HEADER_SIZE;
        // int sent = 0;
        // while(left > 0){
        //     int ret = send(mysock->cli_sockfd,mysock->send_buffer[idx] + sent,min((int)MAX_SEND_SIZE,left),0);
        //     if(ret < 0){
        //         perror("Send failed");
        //         exit(1);
        //     }
        //     printf("sent [thread]: %d\n", ret);
        //     left -= ret;
        //     sent += ret;
        // }
        // printf("Send Completed --> bytes : %s, content : %s\n", mysock->send_buffer[idx], mysock->send_buffer[idx] + HEADER_SIZE);
        // // printf("Send Completed!\n");
        // mysock->send_seq--;
        // printf("Send Seq After : %d\n",mysock->send_seq);
        // memset(mysock->send_buffer[idx],0,MAX_MESSAGE_SIZE + HEADER_SIZE);
        int idx = mysock->send_tail;
        int left = atoi(mysock->send_buffer[idx]) + HEADER_SIZE;
        int sent = 0;
        while(left > 0){
            int ret = send(mysock->cli_sockfd,mysock->send_buffer[idx] + sent,min((int)MAX_SEND_SIZE,left),0);
            if(ret < 0){
                perror("Send failed");
                exit(1);
            }
            printf("sent [thread]: %d\n", ret);
            left -= ret;
            sent += ret;
        }
        printf("Send Completed --> bytes : %s, content : %s\n", mysock->send_buffer[idx], mysock->send_buffer[idx] + HEADER_SIZE);
        mysock->send_tail = (mysock->send_tail + 1) % MAX_SEND_NUM;
        mysock->send_seq--;
        printf("Send Seq After : %d\n",mysock->send_seq);
        memset(mysock->send_buffer[idx],0,MAX_MESSAGE_SIZE + HEADER_SIZE);
        pthread_mutex_unlock(&mysock->send_mutex);
        pthread_cond_signal(&mysock->send_cond_removed);
        sleep(SEND_THREAD_SLEEP_TIME);
    }
}
void* recv_runner(void* arg){
    MySocket* mysock = (MySocket*)arg;
    char BUFFER[MAX_MESSAGE_SIZE + HEADER_SIZE];
    while(mysock->cli_sockfd == 0);
    while(1){
        memset(BUFFER,0,MAX_MESSAGE_SIZE + HEADER_SIZE);
        // printf("Recv Seq : %d\n",mysock->recv_seq);
        int header_left = HEADER_SIZE;
        int idx = 0;
        while(header_left > 0){
            int ret = recv(mysock->cli_sockfd,BUFFER+idx,header_left,0);
            if(ret < 0){
                perror("Recv Failed");
                exit(1);
            }   
            if(ret == 0){
                printf("Disconnected\n");
                pthread_mutex_unlock(&mysock->recv_mutex);
                pthread_cond_signal(&mysock->recv_cond_added);
                // myclose(mysock);
                pthread_exit(NULL);
            }
            header_left -= ret;
            idx += ret;
            printf("Received [thread] Header : %d\n", ret);
        }
        printf("Header Received idx : %d!\n",idx);
        int left = atoi(BUFFER);
        printf("Left : %d\n",left);
        while(left > 0){
            int ret = recv(mysock->cli_sockfd,BUFFER + idx,min((int)MAX_RECV_SIZE,left),0);
            if(ret < 0){
                perror("Recv failed");
                exit(1);
            }
            left -= ret;
            idx += ret;
            printf("Received [thread]: %d\n", ret);
        }
        printf("Recv Completed!\n");
        printf("size : %s, content : %s\n",BUFFER,BUFFER + HEADER_SIZE);
        pthread_mutex_lock(&mysock->recv_mutex);
        while(mysock->recv_seq == MAX_RECV_NUM){
            pthread_cond_wait(&mysock->recv_cond_removed,&mysock->recv_mutex);
        }
        // for(int i = 0;i < idx;i++)
        //     mysock->recv_buffer[mysock->recv_seq][i] = BUFFER[i];
        // mysock->recv_seq++;
        for(int i=0;i<idx;i++)
            mysock->recv_buffer[mysock->recv_head][i] = BUFFER[i];
        mysock->recv_head = (mysock->recv_head + 1) % MAX_RECV_NUM;
        mysock->recv_seq++;

        pthread_mutex_unlock(&mysock->recv_mutex);
        pthread_cond_signal(&mysock->recv_cond_added);
        // sleep(1);
    }

}
