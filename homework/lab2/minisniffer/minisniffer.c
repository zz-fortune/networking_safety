/**
 * 这是一个简单的网络嗅探工具
 * 
 * ＠author: zhang heng
 */


#include"minisniffer.h"

int flag = 0;

int main(){
    pcap_t *device;
    char fbuf[FILE_NAME_LEN];
    int choice;
    info_pcap info;
    pthread_t thread;
    char is_stop;
    bpf_u_int32 netmask;
    while(1){
        printf("|----------------------------------------------|\n");
        printf("|                    menu                      |\n");
        printf("|   1. start to capture package                |\n");
        printf("|   0. exit                                    |\n");
        printf("|----------------------------------------------|\n");
        printf("enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
            case 1:
                if(!init_pcap(&device, fbuf, &netmask)){
                    break;
                }
                info.device = device;
                strcpy(info.filepath, fbuf);
                filter_pcap(device, netmask);
                if(pthread_create(&thread, NULL, start_pcap, &info)<0){
                    printf("something wrong!\n");
                    break;
                }
                do{
                    setbuf(stdin, NULL);
                    printf("stop?(y/n): ");
                    scanf("%c", &is_stop);
                }while(is_stop!='y' && is_stop!='Y');
                stop_pcap();
                pthread_join(thread, NULL);
                printf("capture stopped...\n");
                break;
            case 0:
                exit(0);
            default:
                printf("please input 0~1!\n");
                break;
        }
    }
    return 0;
}

int init_pcap(pcap_t **device, char *filenamebuf, bpf_u_int32 *netmask){
    char *dev_id, errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *dev_tmp;
    bpf_u_int32 netid;
    uid_t uid = getuid();

    char filename[FILE_NAME_LEN], filepath[FILE_NAME_LEN] = "data/";

    // 读入存储结果的文件
    while(1){
        printf("Input the filename to store the results: ");
        scanf("%s", filename);
        strcat(filepath, filename);
        // printf("%s\n", filepath);
        if(access(filepath, F_OK)==0){
            printf("file already exists!\n");
            strcpy(filepath, "data/");
        }else{
            strcpy(filenamebuf, filepath);
            break;
        }
    }

    //　获取root权限，便于打开设备
    if(setuid(0)){
        printf("setuid error: can't get root pemission!\n");
        return 0;
    }

    // 查找可用的设备
    dev_id = pcap_lookupdev(errbuf);
    if (dev_id == NULL){
        printf("Here is no available device. (%s)\n", errbuf);
        return 0;
    }

    //　打开查找到的设备
    dev_tmp = pcap_open_live(dev_id, PACKAGE_MAX_LEN, 0, DEFAULT_TIME, errbuf);
    pcap_lookupnet(dev_id, &netid, netmask, errbuf);

    // 取消root权限，已不需要
    if(setuid(uid)){
        printf("setuid error: can't cansel root pemission!\n");
        pcap_close(dev_tmp);
        return 0;
    }

    //　判断设备是否打开
    if (dev_tmp == NULL){
        printf("can't open deive %s! (%s)\n", dev_id, errbuf);
        return 0;
    }else{
        printf("open deive %s!\n", dev_id);
        *device = dev_tmp;
        return 1;
    }
}


void filter_pcap(pcap_t *device, bpf_u_int32 netmask){
    char filter[FILTER_RULE_LEN];
    // char f[FILE_NAME_LEN] = "net 166.111.4.100";
    struct bpf_program filter_p;

    // 读入过滤规则
    while(1){
        printf("Input the filt rule (input [ENTER] to stop): \n");
        // scanf("%s", filter);
        setbuf(stdin, NULL);
        gets(filter);
        // printf("%s\n", filter);
        // fflush(stdin);
        if (strlen(filter)==0){
            break;
        }

        //　编译过滤规则
        if(pcap_compile(device, &filter_p, filter, 0, netmask)<0){
            printf("compile rule failed, check your input.\n");
            continue;
        }

        //　设置过滤规则
        if(pcap_setfilter(device, &filter_p)<0){
            printf("set filter failed. \n");
            continue;
        }

        break;
    }
}

void* start_pcap(void *param){
    info_pcap info = *(info_pcap *)param;
    pcap_t *device = info.device;
    const u_char* pktstr;
    struct pcap_pkthdr pkthdr;
    FILE *file;
    
    address_pcap addr;

    // 打开存储结果的文件
    if((file=fopen(info.filepath, "w"))==NULL){
        printf("can't create file to store results!\n");
        exit(1);
    }

    // 设置flag为１，表示嗅探器正在工作
    flag = 1;

    printf("start to capture packages...\n");
    // int index = 0;
    // 不断抓包
    while(flag){
        pktstr = pcap_next(device, &pkthdr);
        // printf("pkt %d, length: %d. \n", index++, pkthdr.len);
        if(!pktstr){
            continue;
        }

        //　解析抓到的包
        int offset = 0;
        ether_pcap *ether_head=(ether_pcap *)pktstr;
        if(ntohs(ether_head->prev_prot)!=0x0800){
            continue;
        }
        
        offset += sizeof(ether_pcap);
        ip_pcap *ip_head = (ip_pcap *)(pktstr+offset);
        for(int i = 0; i <4; i++){
            addr.src_ip[i] = ip_head->ip_src[i];
            addr.dst_ip[i] = ip_head->ip_dst[i];
            // printf("%d.%d.%d.%d\n", addr.dst_ip[0], addr.dst_ip[1], addr.dst_ip[2], addr.dst_ip[3]);
        }
        offset += (ip_head->header_len*4);
        // printf("%d\n", ip_head->version);
        if(ip_head->protocol==17){
            udp_pcap *udp_header = (udp_pcap *)(pktstr+offset);
            addr.src_port = ntohs(udp_header->port_src);
            addr.dst_port = ntohs(udp_header->port_dst);
        }else if(ip_head->protocol==6){
            // printf("pkt %d, length: %d. (tcp)\n", index++, pkthdr.len);
            tcp_pcap *tcp_header = (tcp_pcap *)(pktstr+offset);
            addr.src_port = ntohs(tcp_header->port_src);
            addr.dst_port = ntohs(tcp_header->port_dst);
        }else{
            continue;
        }
        

        fprintf(file, "(%d.%d.%d.%d, %d, %d.%d.%d.%d, %d)\n", addr.src_ip[0], 
                addr.src_ip[1],addr.src_ip[2],addr.src_ip[3], addr.src_port, 
                addr.dst_ip[0], addr.dst_ip[1], addr.dst_ip[2], addr.dst_ip[3], 
                addr.dst_port);
    }
    fclose(file);
    pcap_close(device);
}

void stop_pcap(){
    flag = 0;
}