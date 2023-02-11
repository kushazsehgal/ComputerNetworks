#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
    // int fd = open("Assgn-4.pdf", O_RDONLY);
    // if(fd < 0){
    //     printf("Error opening file\n");
    //     exit(1);
    // }
    // char buffer[1000];
    // while(read(fd, buffer, 1) > 0){
    //     printf("%c", buffer[0]);
    // }
    // close(fd);
    FILE* fp = fopen("Assgn-4.pdf", "r");
    if(fp == NULL){
        printf("Error opening file\n");
        exit(1);
    }
    char buffer[1000];
    int n;
    while((n = fread(buffer, 1, 1000, fp)) > 0){
        printf("%s", buffer);
    }
    fclose(fp);
    return 0;
}