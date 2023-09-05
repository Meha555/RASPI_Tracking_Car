#include "com-headers/net-types.h"
#include "com-headers/utils.hpp"

int main() {
    pcap_if_t* alldevs;
    pcap_if_t* d;
    int i = 0;
    int inum;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        printf("获取列表失败！\n");
        exit(1);
    }
    RecieveMenu(alldevs);
    return 0;
}