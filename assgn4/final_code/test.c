#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char request[1000] = "GET /img.jpg HTTP/1.1\r\n";
    // get file path
    char file_path[1000];
    sscanf(request, "GET %s", file_path);
    printf("File path: %s\n", file_path);
}