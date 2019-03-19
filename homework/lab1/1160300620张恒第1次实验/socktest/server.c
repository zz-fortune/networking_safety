#include"FileTransfer.h"


int init_server();
void *server_handler(void *arg);
void server_send(int sock_c, char *buf);
int send_filelist(int sock_c, char *buf);
void close_socket(int sock, char *buf);
void server_recv(int sock_c, char *buf);



int main(){
    int sock, sock_c;
    struct sockaddr_in sa, peeraddr;
    socklen_t len = sizeof(sa);
    pthread_t thread_id;


    // initialize the server
    printf("initialize the server...\n");
    sock = init_server();
    // printf("%d\n", sock);
    getsockname(sock, (struct sockaddr *)&sa, &len);
    printf("server listen at %s: %d\n", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));

    while(1){
        if ((sock_c = accept(sock, (struct sockaddr *)&peeraddr, &len))<0){
            printf("accept error!\n");
            continue;
        }
        if (pthread_create(&thread_id, NULL, server_handler, (void *)(&sock_c)) == -1){
            printf("error occurs while creating new thread!\n");
            shutdown(sock_c, SHUT_RDWR);
        }else{
            getpeername(sock_c, (struct sockaddr *)&sa, &len);
            printf("accept a new client %s: %d\n", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
        }

        
    }
    shutdown(sock, SHUT_RDWR);
    return 0;
}

int init_server(){
    int sock; 
    struct sockaddr_in serveraddr;
    int re = 1;
    char host[] = "127.0.0.1";

    int len;
    struct sockaddr_in sa;
    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s (errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    
    // 设置服务器端的端口以及 ip
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    // serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 设置 ip 为本机的 ip
    serveraddr.sin_port = htons(SERVER_PORT);

    // blind the address(port, ip, protocal) to the socket
    if(bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1){
        printf("blind socket error: %s (errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
   
    // start to listen to client's request
    if (listen(sock, MAX_WAIT) == -1) {
        printf("listen socket error: %s (errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    
    
    return sock;
}

void *server_handler(void *arg){
    int sock_c = *(int *)arg;
    char *buf;
    proc_header header;
    buf = (char *)malloc(BUF_SIZE*sizeof(char));
    
    while(1){
        if(recv(sock_c, buf, BUF_SIZE, 0) <= 0){
            break;
        }
        parse_header(buf, &header);
        if (header.code==0x10) {
            printf("code: 0x10\n");
            server_send(sock_c, buf);
        } else if (header.code == 0x20) {
            printf("code: 0x20\n");
            server_recv(sock_c, buf);
        } else if (header.code == 0x30) {
            printf("code: 0x30\n");
            break;
        }else{
            printf("invalid code!\n");
        }
        
    }

    close_socket(sock_c, buf);
    
}


void close_socket(int sock, char *buf){
    int re = 0;
    shutdown(sock, SHUT_RDWR);
    pthread_exit(&re);
    free(buf);
}

void server_send(int sock_c, char *buf){
    int size, dir_len;
    char filename[FILE_NAME_SIZE];
    proc_header header;
    char dir[] = "data/";

    if (send_filelist(sock_c, buf)<0){
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        close_socket(sock_c, buf);
    }

    if (recv(sock_c, buf, BUF_SIZE, 0)<0){
        perror("recv error\n");
        close_socket(sock_c, buf);
    }

    parse_header(buf, &header);
    if (header.code != 0x11){
        printf("invalid code while request file!\n");
        close_socket(sock_c, buf);
    }

    memcpy((void *)filename, (void *)(buf+6), header.length-6);
    filename[header.length-6] = '\0';
    if (send_file(sock_c, buf, filename) == 0){
        close_socket(sock_c, buf);
    }
    
}

void server_recv(int sock_c, char *buf){
    char filename[FILE_NAME_SIZE];
    proc_header header;

    parse_header(buf, &header);
    memcpy(filename, buf+6, header.length-6);
    format_header(buf, 0x3, 0x1, 6);
    send(sock_c, buf, 6, 0);

    filename[header.length-6] = '\0';

    printf("%s\n", filename);
    recv_file(sock_c, buf, filename);
}


int send_filelist(int sock_c, char *buf){
    DIR *dir;
    struct dirent *ptr;
    int offset = 6;

    dir = opendir("data");

    readdir(dir);
    readdir(dir);

    memset(buf, 0, BUF_SIZE);
    while((ptr = readdir(dir)) != NULL){
        int len = strlen(ptr->d_name);
        memcpy((void *)(buf+offset), (void *)ptr->d_name, len);
        buf[offset+len] = '\n';
        offset += (len+1);
    }
    closedir(dir);
    format_header((char *)buf, 0x01, 0x1, offset);
    return send(sock_c, (void *)buf, offset, 0);
}