#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<time.h>
#include<stdbool.h>
#include <dirent.h>

int main(){
    chdir("..\0");
    char output[500];
    for(int i = 0;i < 500;i++)
        output[i] = '\0';
    getcwd(output,500);
    printf("%s\n",output);
    return 0;
}