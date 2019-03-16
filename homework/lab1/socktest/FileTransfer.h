#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<dirent.h>
#include<unistd.h>
#include<string.h>

#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>


#define SERVER_PORT 2333
#define MAX_WAIT 10
#define BUF_SIZE 1024*1024
#define IP_LEN 20
#define FILE_NAME_SIZE 100
#define MAX_FILE 20


typedef struct
{
    unsigned char code;
    unsigned char is_last;
    uint32_t length;
}proc_header;


void check_filename(char *filename);
void parse_header(char *buf, proc_header *header);
void format_header(char *buf, char code, char is_last, uint32_t len);
void recv_file(int sock, char *buf, char *filename);
int send_file(int sock, char *buf, char *filename);
void strrev(char *src);


void parse_header(char *buf, proc_header *header){
    memcpy(&header->code, buf, 1);
    memcpy(&header->is_last, buf+1, 1);
    memcpy(&header->length, buf+2, 4);
}

void format_header(char *buf, char code, char is_last, uint32_t len){
    memcpy(buf, &code, 1);
    memcpy(buf+1, &is_last, 1);
    memcpy(buf+2, &len, 4);
}

void recv_file(int sock, char *buf, char *filename){
    FILE *file;
    int index;
    char suf[10], filepath[FILE_NAME_SIZE] = "data/";
    proc_header header;

    strcat(filepath, filename);
    check_filename(filepath);

    file = fopen(filepath, "w");
    do{
        memset(buf, 0, BUF_SIZE);
        int status = recv(sock, buf, BUF_SIZE, 0);
        if(status<0){
            printf("error while recieve file!\n");
            fclose(file);
            return;
        }else if (status==0){
            printf("socket closed!\n");
            fclose(file);
            return;
        }
        
        parse_header(buf, &header);

        // printf("%d, %x, %x: %d\n", *(int *)(buf+2), header.code, header.is_last,status);

        // if(header.length==0){
        //     continue;
        // }
        fwrite(buf+6, header.length-6, 1, file);
    }while(!header.is_last);
    fclose(file);
}


int send_file(int sock, char *buf, char *filename){
    FILE *file = NULL;
    char filepath[FILE_NAME_SIZE] = "data/";

    strcat(filepath, filename);
    file = fopen(filepath, "r");

    if (file == NULL){
        printf("error occurs while openning file!\n");
        return 0;
    }

    memset(buf, 0, BUF_SIZE);
    while (fread(buf+6, BUF_SIZE-6, 1, file)>0){
        format_header(buf, 0x02, 0x0, BUF_SIZE);
        if(send(sock, buf, BUF_SIZE, 0)<0){
            printf("error occurs whiles send file!\n");
            fclose(file);
            return 0;
        }

        printf("%d\n", BUF_SIZE);
        memset(buf, 0, BUF_SIZE);
    }
    
    // printf("%d\n", strlen(buf+6)+6);
    format_header(buf, 0x02, 0x1, strlen(buf+6)+6);
    if(send(sock, buf, strlen(buf+6)+6, 0)<0){
        printf("error occurs whiles send file!\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}


void check_filename(char *filename){
    char suf[10];
    int off = 0;
    char index = '1';
    int len = strlen(filename), prefix_len;

    if (access(filename, F_OK)){
        return;
    }

    for (int i = len-1; i>0; i--){
        if (filename[i] == '.'){
            prefix_len = i;
            suf[off++] = filename[i];
            suf[off] = '\0'; 
            break;
        }else{
            suf[off++] = filename[i];
        }
    }
    strrev(suf);
    filename[prefix_len] = index;
    filename[prefix_len+1] = '\0';
    strcat(filename, suf);
    do{
        filename[prefix_len] = index;
        index++;
    }while(!access(filename, F_OK));
}

void strrev(char *src){
    int len = strlen(src);
    char tmp;
    for(int i = 0; i<len/2; i++){
        tmp = src[i];
        src[i] = src[len-1-i];
        src[len-1-i] = tmp;
    }
}