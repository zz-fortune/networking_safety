#include "FileTransfer.h"

int main_menu();
int connect_server(int *sock);
void client_handler();
int get_addr(char *ip, int *port);
int function_menu();
void client_send(int sock, char *buf);
void client_recv(int sock, char *buf);



int main(){
    int choice = -1;
    while(1){
        choice = main_menu();
        switch (choice)
        {
            case 1:
                client_handler();
                break;
            case 0:
                exit(0);
            default:
                printf("invalid input (0~1)!\n");
                break;
        }
    }
    
}

int main_menu(){
    int choice;
    printf("|------------------------------------------|\n");
    printf("|                   menu                   |\n");
    printf("|   1. connect to server                   |\n");
    printf("|   0. exit                                |\n");
    printf("|------------------------------------------|\n");
    printf("enter your choice: ");
    scanf("%d", &choice);
    return choice;
}

void client_handler(){
    int choice, sock;
    char *buf;
    if (!connect_server(&sock)) {
        printf("failed to connect! check the ip and port you input.\n");
        return;
    }
    
    buf = (char *)malloc(BUF_SIZE*sizeof(char));
    while(1){
        choice = function_menu();
        switch (choice)
        {
            case 1:
                client_recv(sock, buf);
                break;
            case 2:
                client_send(sock, buf);
                break;
            case 0:
                shutdown(sock, SHUT_RDWR);
                free(buf);
                return;
            default:
                printf("invalid input (0~2)!\n");
                break;
        }
    }
}

int connect_server(int *sock){
    int port = 2333;
    // char host[IP_LEN] = "192.168.43.73";
    char host[IP_LEN] = "127.0.0.1";
    struct sockaddr_in servaddr;
    // if (!get_addr(host, &port)) {
    //     return 0;
    // }

    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        return 0;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &servaddr.sin_addr)<=0) {
        printf("inet_pton error for %s\n", host);
        return 0;
    }
    
    if (connect(*sock, (struct sockaddr *)&servaddr, sizeof(servaddr))<0) {
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    
    return 1;
}

int get_addr(char *host, int *port){
    char tmp[IP_LEN];
    int start = 0;
    printf("input the server's ip and port: ");
    scanf("%s %d", host, port);
    for(int i = 0; *(host+i) != '\0'; i++) {
        if (*(host+i) == '.') {
            tmp[start++] = '\0';
            start = 0;
            if (atoi(tmp)>255 || atoi(tmp)<0) {
                return 0;
            }
            
        } else{
            tmp[start++] = *(host+i);
        }
        
    }
    tmp[start++] = '\0';
    if (atoi(tmp)>255 || atoi(tmp)<0) {
        return 0;
    }

    if (*port<0 || *port>65535) {
        return 0;
    }
    
    return 1;
}

int function_menu(){
    int choice;
    printf("|------------------------------------------|\n");
    printf("|                   menu                   |\n");
    printf("|   1. download from server                |\n");
    printf("|   2. upload to server                    |\n");
    printf("|   0. exit                                |\n");
    printf("|------------------------------------------|\n");
    printf("enter your choice: ");
    scanf("%d", &choice);
    return choice;
}

void client_send(int sock, char *buf){
    char filepath[FILE_NAME_SIZE], filename[FILE_NAME_SIZE];
    int pos = 0, len;
    proc_header header;

    do{
        printf("input the file path: ");
        scanf("%s", filepath);
    }while(access(filepath, F_OK));

    len = strlen(filepath);
    for (int i = len-1; i>0; i--){
        if (filepath[i] == '/'){
            pos = i+1;
            break;
        }
    }

    // printf("%s\n", filepath);
    
    format_header(buf, 0x20, 0x1, len-pos+6);
    memcpy(buf+6, filepath+pos, len - pos);
    memcpy(filename, filepath+pos, len-pos);
    filename[len-pos] = '\0';

    // for(int i = pos; i < len; i++){
    //     printf("%c", filepath[i]);
    // }
    // printf("\n");

    send(sock, buf, len-pos+6, 0);
    recv(sock, buf, BUF_SIZE, 0);
    parse_header(buf, &header);
    if (header.code == 0x3){
        // printf("test\n");
        send_file(sock, buf, filename);
    }
}

void client_recv(int sock, char *buf){
    proc_header header;
    int index = 0, pos = 6, off = 0;
    char name[MAX_FILE][FILE_NAME_SIZE];
    int choice;

    format_header(buf, 0x10, 0x1, 6);
    send(sock, buf, 6, 0);
    recv(sock, buf, BUF_SIZE, 0);
    parse_header(buf, &header);

    for (int i = 6; i < header.length; i++){
        if (buf[i] == '\n'){
            name[index][off] = '\0';
            printf("%d: %s\n", index+1, name[index]);
            index++;
            off = 0;
        }else{
            name[index][off++] = buf[i];
        }
    }

    // printf("%d\n", strlen(name[1]));
    // printf("%s\n", name[1]);
    // for(int i = 0; i < strlen(name[1]); i++){
    //     printf("%c", name[1][i]);
    // }
    do{
        printf("enter your choice (1~%d): ", index);
        scanf("%d", &choice);
    }while(choice<1 || choice>index);
    
    format_header(buf, 0x11, 0x1, strlen(name[choice-1])+6);
    memcpy(buf+6, name[choice-1], strlen(name[choice-1]));
    send(sock, buf, strlen(name[choice-1])+6, 0);

    // for(int i = 6; i<strlen(name[choice-1])+6; i++){
    //     printf("%c", buf[i]);
    // }
    // printf("\n");

    recv_file(sock, buf, name[choice-1]);
}