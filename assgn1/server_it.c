/////////////////////////////////
// Networks Lab Assignment 1 : Q2
// Name : Kushaz Sehgal
// Roll Number : 20CS30030
/////////////////////////////////
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include <stdbool.h>
#define PORT_NUM 1301
#define PACKET_SIZE 40
#define NO_OP '_'
bool isOperator(char c){
    if(c == '-' || c == '+' || c == '*' || c == '/')
        return true;
    else return false;
}
void calc(float present_num,char operator,float* result){
    float prev_result = *result;
    if(operator == '+')
        *result = *result + present_num;
    else if(operator == '*')
        *result = *result * present_num;
    else if(operator == '/')
        *result = *result / present_num;
    else if(operator == '-')
        *result = *result - present_num;
    else if(operator == '=')
        *result = present_num;
    // printf("prev_result : %f presentnum : %f operator : %c result = %f\n",prev_result,present_num,operator,*result);
}
void reset(float* present_num,bool* decimal,float* pow10){
    *present_num = 0;
    *decimal = false;
    *pow10 = 0.1;
}
int main(){
    char buffer[PACKET_SIZE];
    int sockfd;
    struct sockaddr_in serv_addr,cli_addr;
    
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Cannot create socket for server\n");
        exit(0);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUM);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
        perror("Unable to bind to local address");
        exit(0);
    }
    else{
        printf("Server Listening on PORT : %d\n",PORT_NUM);
    }

    listen(sockfd,1);
    while(1){
        int cli_len = sizeof(cli_addr);
        int newsockfd;
        newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&cli_len);
        if(newsockfd < 0){
            perror("Connection(accept) to client failed\n");
            exit(0);
        }
        else{
            printf("Connection with client established\n");
        }
            
        bool done = false;
        while(1){        
            bool first = true;
            bool decimal = false;
            float num[2];
            float present_num = 0;
            float pow10 = 0.1;
            char operator[2];
            operator[0] = '=';
            operator[1] = '=';
            bool bracket = false;
            bool end = false;
            while(1){
                recv(newsockfd,buffer,20,0);
                // printf("Before reading next buffer, present num is: %f\n",present_num);
                if(first){
                    if(buffer[0] == '-'){
                        done = true;
                        break;
                    }
                    first = false;
                }
                for(int i = 0;i < 20;i++){
                    // printf("character recived : %c\n",buffer[i]);
                    if(buffer[i] - '0' >= 0 && buffer[i] - '0' <= 9){
                        
                        if(decimal){
                            present_num += (float)(buffer[i] - '0')*pow10;
                            pow10 /= 10;
                        }
                        else present_num = present_num*10.0 + (float)(buffer[i] - '0');
                        // printf("present_num now : %f\n",present_num);
                        
                    }
                    else if(buffer[i] == '.'){
                        decimal = true;
                    }
                    else if(buffer[i] == '('){
                        bracket = true;
                    }
                    else if(buffer[i] == ')'){
                        if(operator[bracket] != NO_OP){
                            calc(present_num,operator[bracket],&num[bracket]);
                            operator[bracket] = NO_OP;
                        }
                        reset(&present_num,&decimal,&pow10);
                        present_num = num[bracket];
                        operator[bracket] = '=';
                        num[bracket] = 0;
                        bracket = false;
                    }
                    else if(buffer[i] == ' '){
                        continue;
                    }
                    else if(isOperator(buffer[i])){
                        if(operator[bracket] != NO_OP){
                            calc(present_num,operator[bracket],&num[bracket]);
                            operator[bracket] = NO_OP;
                        }
                        reset(&present_num,&decimal,&pow10);
                        operator[bracket] = buffer[i];
                    }
                    else if(buffer[i] == '\0'){
                        if(operator[bracket] != NO_OP){
                            calc(present_num,operator[bracket],&num[bracket]);
                            operator[bracket] = NO_OP;
                        }
                        end = true;
                        break;
                    }
                }
                if(end){
                    char result[40];
                    sprintf(result,"Answer : %f\n",num[bracket]);
                    printf("%s",result);
                    send(newsockfd,result,strlen(result)+1,0);
                    break;
                }   
            }
            if(done)
                    break;

        }
        close(newsockfd);
        printf("Connection with client terminated\n");
    }
    

    close(sockfd);
    return 0;
}