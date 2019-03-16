/* itoa example */
#include <stdio.h>
#include <stdlib.h>
#include"FileTransfer.h"

void client_recv(int sock, char *buf){
    
}

int main (){
    FILE *file;
    char buf[1000];
    file = fopen("data/test3.jpg", "r");
    if (file == NULL) {
        printf("error\n");
    }
    printf("%d\n", fread(buf, 1000, 1, file));
    buf[999] = '\0';
    printf("%d\n", strlen(buf));
    
}