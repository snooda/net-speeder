#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <libnet.h>
#include <arpa/inet.h>

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 65535

#define ETHERNET_H_LEN_COOKED 16
#define ETHERNET_H_LEN_NORMAL 14

static int ethernet_h_len = -1;
static char *localip = NULL;
static int showdebug = 0;
static libnet_t *libnet_handler = NULL;

#define SPECIAL_TTL 88

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
void print_usage(void);


/*
* print help text
*/
void print_usage(void)
{
    printf("Usage: net_speeder mode interface \"filter\" [localip] [debug]\n");
    printf("\n");
    printf("Options:\n");
    printf("    mode         Ethernet header length(auto, normal, cooked)\n");
    printf("    interface    Listen on <interface> for packets.\n");
    printf("    filter       Rules to filter packets.\n");
    printf("    localip      Rules to only rewrite localip send packets.\n");
    printf("    debug        Show debug info.\n");
    printf("\n");
}

int ipcmp(char *ip)
{
    return strncmp(localip, ip, strlen(localip)) == 0;
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    struct libnet_ipv4_hdr *ip = (struct libnet_ipv4_hdr *)(packet + ethernet_h_len);
    if(ip->ip_ttl == SPECIAL_TTL) return;
    switch(ip->ip_p)
    {
    case 6://tcp
    {
        struct libnet_tcp_hdr *tcp = (struct libnet_tcp_hdr *)(ip + ip->ip_hl * 4);
        if(tcp->th_flags & (TH_SYN | TH_FIN | TH_RST)) return;
        int sport = ntohs(tcp->th_sport);
        int dport = ntohs(tcp->th_dport);
        if(sport == 0 || dport == 0) return;
        if(sport == 1723 || dport == 1723) return;
        char *srcip = inet_ntoa(ip->ip_src);
        char *dstip = inet_ntoa(ip->ip_dst);
        if(localip != NULL && !ipcmp(srcip)) return;
        if(showdebug) printf("%s:%d => %s:%d len:%d\n", srcip, sport, dstip, dport, ntohs(ip->ip_len));
        break;
    }
    case 17://udp
    {
        break;
    }
    default:
        return;
    }
    ip->ip_ttl = SPECIAL_TTL;
    int len_written = libnet_adv_write_raw_ipv4(libnet_handler, (uint8_t *)ip, ntohs(ip->ip_len));
    if(len_written < 0)
        printf("error: %s => %s packet len:[%d] message:%s\n", inet_ntoa(ip->ip_src), inet_ntoa(ip->ip_dst), ntohs(ip->ip_len), libnet_geterror(libnet_handler));
}

libnet_t *start_libnet(char *dev)
{
    char errbuf[LIBNET_ERRBUF_SIZE];
    libnet_t *result = libnet_init(LIBNET_RAW4_ADV, dev, errbuf);
    if(result == NULL)
    {
        printf("libnet_init: error %s\n", errbuf);
    }
    return result;
}

int main(int argc, char **argv)
{
    char *dev = NULL, *mode = NULL;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    char *filter_rule = NULL;
    struct bpf_program fp;
    bpf_u_int32 net, mask;

    if (argc >= 4)
    {
        mode = argv[1];
        dev = argv[2];
        filter_rule = argv[3];
        if(strcmp(mode, "auto") == 0)
        {
            if(strncmp(dev, "venet", 5) != 0)
                mode = "normal";
            else
                mode = "cooked";
        }
        if(strcmp(mode, "normal") == 0)
            ethernet_h_len = ETHERNET_H_LEN_NORMAL;
        else if(strcmp(mode, "cooked") == 0)
            ethernet_h_len = ETHERNET_H_LEN_COOKED;
        else
        {
            print_usage();
            return -1;
        }
        printf("Mode: %s\n", mode);
        printf("Device: %s\n", dev);
        printf("Filter rule: %s\n", filter_rule);
        if(argc >= 5)
        {
            if(strcmp(argv[4], "debug") == 0)
                showdebug = 1;
            else
            {
                localip = argv[4];
                printf("LocalIP: %s\n", localip);
                if(argc >= 6 && strcmp(argv[5], "debug") == 0)
                    showdebug = 1;
            }
        }
        printf("Show debug: %s\n", showdebug ? "true" : "false");
    }
    else
    {
        print_usage();
        return -1;
    }

    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1)
    {
        printf("Couldn't get netmask for device %s: %s\n", dev, errbuf);
        net = 0;
        mask = 0;
    }

    printf("init pcap\n");
    handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
    if(handle == NULL)
    {
        printf("pcap_open_live dev:[%s] err:[%s]\n", dev, errbuf);
        printf("init pcap failed\n");
        return -1;
    }

    printf("init libnet\n");
    libnet_handler = start_libnet(dev);
    if(libnet_handler == NULL)
    {
        printf("init libnet failed\n");
        return -1;
    }

    if (pcap_compile(handle, &fp, filter_rule, 0, net) == -1)
    {
        printf("filter rule err:[%s][%s]\n", filter_rule, pcap_geterr(handle));
        return -1;
    }

    if (pcap_setfilter(handle, &fp) == -1)
    {
        printf("set filter failed:[%s][%s]\n", filter_rule, pcap_geterr(handle));
        return -1;
    }

    while(1)
    {
        pcap_loop(handle, 1, got_packet, NULL);
    }

    /* cleanup */
    pcap_freecode(&fp);
    pcap_close(handle);
    libnet_destroy(libnet_handler);
    return 0;
}
