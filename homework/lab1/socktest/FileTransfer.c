/**
 * 这是一个简单的双向文件传输程序，使用两个线程，
 * 一个线程用以请求文件并接收，另一个线程用以接受
 * 请求并发送文件
 */

#include"FileTransfer.h"

int init_server(){
    int sock; 
    struct sockaddr_in serveraddr;
    int re = 1;

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s (errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    
    // 设置服务器端的端口以及 ip
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 设置 ip 为本机的 ip
    serveraddr.sin_port = htons(SERVER_PORT);

    // blind the address(port, ip, protocal) to the socket
    if(bind(sock, &serveraddr, sizeof(serveraddr)) == -1){
        printf("blind socket error: %s (errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    // start to listen to client's request
    if (listen(sock, MAX_WAIT) == -1) {
        printf("listen socket error: %s (errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    return 1;
}

void *server_handler(void *arg){
    int sock_c = *(int *)arg;
    void *buf;
    int size = 0;
    proc_header header;
    buf = malloc(BUF_SIZE*sizeof(char));
    
    while(1){
        size = recv(sock_c, buf, BUF_SIZE, 0);
        parseHeader(buf, size, &header);
        if (header.code==0x10) {
            server_send(sock_c, buf);
        } else if (header.code == 0x20) {
            server_recv(sock_c, buf);
        } else if (header.code == 0x30) {
            close_socket(sock_c);
        }
        
    }
    
}

void parse_header(void *buf, proc_header *header){

}

void send_file(){

}

void recv_file(){

}

void close_socket(int sock){
    int re = 0;
    shutdown(sock, SHUT_RDWR);
    pthread_exit(&re);
}

void server_send(int sock_c, void *buf){

}

void server_recv(int sock_c, void *buf){

}