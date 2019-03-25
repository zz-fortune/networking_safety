#include<pcap/pcap.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<time.h>
#include<pthread.h>

#define PACKAGE_MAX_LEN 65535
#define FILTER_RULE_LEN 100
#define MAX_RULE 10
#define FILE_NAME_LEN 100
#define DEFAULT_TIME 100

typedef unsigned char u_char;

typedef struct{
    char filepath[FILE_NAME_LEN];
    pcap_t *device;
}info_pcap;

typedef struct{
    u_char ther_src[6];
    u_char ther_dst[6];
    uint16_t prev_prot;
}ether_pcap;

typedef struct{
    int header_len:4;
    int version:4;
    u_char tos:8;
    int total_len:16;
    int indentify:16;
    int flags:16;
    u_char ttl:8;
    u_char protocol:8;
    int check_sum:16;
    u_char ip_src[4];
    u_char ip_dst[4];
}ip_pcap;

typedef struct{
    uint16_t port_src;
    uint16_t port_dst;
    uint32_t seqnum;
    uint32_t acknum;
    u_char head_len;
    u_char flags;
    uint16_t window_size;
    uint16_t check_sum;
    uint16_t urg_ptr;
}tcp_pcap;

typedef struct{
    uint16_t port_src;
    uint16_t port_dst;
    uint16_t total_len;
    uint16_t check_sum;
}udp_pcap;

typedef struct{
    u_char src_ip[4];
    u_char dst_ip[4];
    uint16_t src_port;
    uint16_t dst_port;
}address_pcap;

/**
 * 初始化嗅探器。minisniffer会查询存在的设备并打开，并读入存出结果的文件
 */
int init_pcap(pcap_t **device, char *filename, bpf_u_int32* netmask);

/**
 * 设置过滤规则。这里需要用户输入过滤规则。
 */ 
void filter_pcap(pcap_t *device, bpf_u_int32 netmask);

/**
 * 开始抓包分析，并将结果输入文件中存储。
 */ 
void* start_pcap(void *param);

/**
 * 停止抓包分析。
 */ 
void stop_pcap();