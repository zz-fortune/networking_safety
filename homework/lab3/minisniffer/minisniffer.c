#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libnet.h>

#define MSG_LEN 1024
#define UDP_HEADER_LEN 8
#define TCP_HEADER_LEN 20
#define IP_HEADER_LEN 20
#define MAC_ADDR_LEN 6
#define IP_ADDR_LEN 20

int init_libnet(libnet_t **device);
libnet_ptag_t buildtcp_libnet(libnet_t *device, libnet_ptag_t ptag, u_int16_t *len);
libnet_ptag_t buildudp_libnet(libnet_t *device, libnet_ptag_t ptag, u_int16_t *len);
libnet_ptag_t buildip_libnet(libnet_t *device, libnet_ptag_t ptag, u_int16_t *len, int proc);
libnet_ptag_t buildlink_libnet(libnet_t *device, libnet_ptag_t ptag, int proc);


int main(){
    libnet_ptag_t ptag_udp = 0, ptag_tcp = 0, ptag_ip = 0, ptag_ether = 0, pre_ptag_ip=0, pre_ptag_ether=0;
    u_int16_t len = 0, choice, pre_choice=0;
    libnet_t *device;
    int proc_trans, proc_ip = ETHERTYPE_IP;

    // 初始化 libnet
    if(init_libnet(&device)==0){
        printf("error occured while initializing!\n");
        exit(-1);
    }
    while(1){
        
        // 输出菜单，读入用户选择
        printf("|----------------------------------------------|\n");
        printf("|                    menu                      |\n");
        printf("|   1. build tcp package                       |\n");
        printf("|   2. build udp package                       |\n");
        printf("|   0. exit                                    |\n");
        printf("|----------------------------------------------|\n");
        printf("enter your choice: ");
        scanf("%hd", &choice);

        // 当构造第 i+1 个 udp（tcp）包时，libnet_build_udp（libnet_build_tcp）、libnet_build_ip、
        // libnet_build_ether 函数中参数 ptag 需要是构造第 i 个 udp（tcp）时的相应函数返回值。
        // 因此，在从构造 udp（tcp）转换为构造 tcp（udp）时，需要将上一次构造 udp（tcp） 时的返回值缓存下来，
        // 并设置为上一次构造 tcp（udp） 时的返回值
        if(choice!=pre_choice){
            pre_choice = choice;
            int tmp = ptag_ip;
            ptag_ip = pre_ptag_ip;
            pre_ptag_ip = tmp;
            tmp = ptag_ether;
            ptag_ether = pre_ptag_ether;
            pre_ptag_ether = tmp;
        }
        switch(choice){
            case 1:
                proc_trans = 6;
                len = 0;
                ptag_tcp = buildtcp_libnet(device, ptag_tcp, &len);
                break;
            case 2:
                proc_trans = 17;
                len = 0;
                ptag_udp = buildudp_libnet(device, ptag_udp, &len);
                break;
            case 0:
                libnet_destroy(device);
                exit(0);
            default:
                printf("please input 0~2!\n");
                continue;
        }
        
        // 构造 ip 包、链路层包并发送
        ptag_ip = buildip_libnet(device, ptag_ip, &len, proc_trans);
        ptag_ether = buildlink_libnet(device, ptag_ether, proc_ip);
        if(libnet_write(device)==-1){
            printf("error occured while write package!\n");
            libnet_destroy(device);
            exit(-1);
        }
    }
    return 0;
}

/**
 * 初始化环境
 */ 
int init_libnet(libnet_t **device){
    char errbuf[LIBNET_ERRBUF_SIZE];

    *device = NULL;
    if((*device = libnet_init(LIBNET_LINK, "wlp3s0", errbuf)) ==NULL){
        fprintf(stdout, "fail to initialize device: (%s)\n", errbuf);
        return 0;
    }
    return 1;
}

/**
 * 构造tcp包
 */
libnet_ptag_t buildtcp_libnet(libnet_t *device, libnet_ptag_t ptag, u_int16_t *len){
    uint16_t srcport, dstport;
    char msg[MSG_LEN];
    libnet_ptag_t tag = ptag;
    u_int8_t *payload = NULL;
    *len = TCP_HEADER_LEN;

    // 读入源端口、目的端口，并进行简单的输入合法性检查
    do{
        fprintf(stdout, "input the source port and dest port(0~65535): ");
        scanf("%d %d", &srcport, &dstport);
    }while(srcport>65535 || srcport<0 || dstport>65535 || dstport<0);

    // srcport = 8080;
    // dstport = 8080;

    // 读入需要发送的信息，允许信息为空
    printf("input the data in the package(press [ENTER] to skip): ");
    setbuf(stdin, NULL);
    gets(msg);
    *len += strlen(msg);

    // 构造负载
    if(*len == TCP_HEADER_LEN){
        payload = NULL;
    }else{
        payload = (u_int8_t *)msg;
    }

    // 调用 API 构造 tcp 包
    tag = libnet_build_tcp(srcport, dstport, 500, 874, 0, 3000, 0, 0, *len, payload, *len-TCP_HEADER_LEN, device, tag);
    if(tag==-1){
        printf("error occured while building tcp package!\n");
        libnet_destroy(device);
        exit(-1);
    }
    return tag;

}

/**
 * 构造 udp 包
 */
libnet_ptag_t buildudp_libnet(libnet_t *device, libnet_ptag_t ptag, u_int16_t *len){
    int srcport, dstport;
    char msg[MSG_LEN];
    libnet_ptag_t tag = ptag;
    u_int8_t *payload;
    *len = UDP_HEADER_LEN;

    // 读入源端口、目的端口，并进行简单的输入合法性检查
    do{
        fprintf(stdout, "input the source port and dest port(0~65535): ");
        scanf("%d %d", &srcport, &dstport);
    }while(srcport>65535 || srcport<0 || dstport>65535 || dstport<0);

    // srcport = 8080;
    // dstport = 8080;

    // 读入需要发送的信息，允许信息为空
    printf("input the data in the package(press [ENTER] to skip): ");
    setbuf(stdin, NULL);
    gets(msg);
    *len += strlen(msg);

    // 构造负载
    if(*len == UDP_HEADER_LEN){
        payload = NULL;
    }else{
        payload = (u_int8_t *)msg;
    }

    // 调用 API 构造 udp 包
    tag = libnet_build_udp(srcport, dstport, *len, 0, payload, *len-UDP_HEADER_LEN, device, tag);
    if(tag==-1){
        printf("error occured while building udp package!\n");
        libnet_destroy(device);
        exit(-1);
    }
    return tag;
}

/**
 * 构造 ip 包
 */
libnet_ptag_t buildip_libnet(libnet_t *device, libnet_ptag_t ptag, u_int16_t *len, int proc){
    char srcip_str[IP_ADDR_LEN], dstip_str[IP_ADDR_LEN];
    libnet_ptag_t tag = ptag;
    u_int32_t srcip, dstip;
    *len += IP_HEADER_LEN;

    // 读入源 ip， 并进行简单的合法性检查。允许输入为空
    do{
        printf("input source ip(press [ENTER] to skip): ");
        setbuf(stdin, NULL);
        gets(srcip_str);
        if(strlen(srcip_str)==0){
            srcip = libnet_name2addr4(device, "127.0.0.1", LIBNET_RESOLVE);
        }else{
            srcip = libnet_name2addr4(device, srcip_str, LIBNET_RESOLVE);
        }
    }while(srcip==-1);
    
    // 读入目的 ip， 并进行简单的合法性检查
    do{
        printf("input dest ip: ");
        gets(dstip_str);
        dstip = libnet_name2addr4(device, dstip_str, LIBNET_RESOLVE);
    }while(dstip==-1);

    // char tmp[] = "127.0.0.1";
    // dstip = libnet_name2addr4(device, tmp, LIBNET_RESOLVE);

    // 调用 API 构造 ip 包
    tag = libnet_build_ipv4(*len, 0, 500, 0, 100, proc, 0, srcip, dstip, NULL, 0, device, tag);
    if(tag==-1){
        printf("error occured while building ip package!\n");
        libnet_destroy(device);
        exit(-1);
    }
    return tag;
}

/**
 * 构造 ethernet 包
 */
libnet_ptag_t buildlink_libnet(libnet_t *device, libnet_ptag_t ptag, int proc){
    libnet_ptag_t tag = ptag;
    // u_int8_t srcmac[MAC_ADDR_LEN], dstmac[MAC_ADDR_LEN];
    u_int8_t *srcmac, *dstmac;

    // 读入源 mac 地址和目的 mac 地址
    printf("input source mac: ");
    scanf("%hhx %hhx %hhx %hhx %hhx %hhx", srcmac, srcmac+1, srcmac+2, srcmac+3, srcmac+4, srcmac+5);
    printf("input dest mac: ");
    scanf("%hhx %hhx %hhx %hhx %hhx %hhx", dstmac, dstmac+1, dstmac+2, dstmac+3, dstmac+4, dstmac+5);

    // srcmac = (u_int8_t *)libnet_get_hwaddr(device);
    // dstmac = (u_int8_t *)libnet_get_hwaddr(device);

    // 调用 API 构造链路层包
    tag = libnet_build_ethernet(dstmac, srcmac, proc, NULL, 0, device, tag);
    if(tag==-1){
        printf("error occured while building ethernet package!\n");
        libnet_destroy(device);
        exit(-1);
    }
    return tag;
}