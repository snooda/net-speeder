#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <libnet.h>

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 65535

#ifdef COOKED
	#define ETHERNET_H_LEN 16
#else
	#define ETHERNET_H_LEN 14
#endif

#define SPECIAL_TTL 88

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
void print_usage(void);

pcap_t *net_speeder_pcap_open_live(const char *device, int snaplen, int promisc, int to_ms, char *errbuf)
{
	pcap_t *p;
	int status;

	p = pcap_create(device, errbuf);
	if (p == NULL)
		return (NULL);
	status = pcap_set_snaplen(p, snaplen);
	if (status < 0)
		goto fail;
	status = pcap_set_promisc(p, promisc);
	if (status < 0)
		goto fail;
	status = pcap_set_timeout(p, to_ms);
	if (status < 0)
		goto fail;
	status = pcap_set_immediate_mode(p, 1); // in net_speeder, we must handle outbound packets immediately
	if (status < 0)
		goto fail;
	/*
	 * Mark this as opened with pcap_open_live(), so that, for
	 * example, we show the full list of DLT_ values, rather
	 * than just the ones that are compatible with capturing
	 * when not in monitor mode.  That allows existing applications
	 * to work the way they used to work, but allows new applications
	 * that know about the new open API to, for example, find out the
	 * DLT_ values that they can select without changing whether
	 * the adapter is in monitor mode or not.
	 */
	
	// p->oldstyle = 1;
	status = pcap_activate(p);
	if (status < 0)
		goto fail;
	return (p);
fail:
	if (status == PCAP_ERROR)
		snprintf(errbuf, PCAP_ERRBUF_SIZE, "%s: %.*s", device,
		    PCAP_ERRBUF_SIZE - 3, pcap_geterr(p));
	else if (status == PCAP_ERROR_NO_SUCH_DEVICE ||
	    status == PCAP_ERROR_PERM_DENIED ||
	    status == PCAP_ERROR_PROMISC_PERM_DENIED)
		snprintf(errbuf, PCAP_ERRBUF_SIZE, "%s: %s (%.*s)", device,
		    pcap_statustostr(status), PCAP_ERRBUF_SIZE - 6, pcap_geterr(p));
	else
		snprintf(errbuf, PCAP_ERRBUF_SIZE, "%s: %s", device,
		    pcap_statustostr(status));
	pcap_close(p);
	return (NULL);
}

/*
 * print help text
 */
void print_usage(void) {
	printf("Usage: %s [interface][\"filter rule\"]\n", "net_speeder");
	printf("\n");
	printf("Options:\n");
	printf("    interface    Listen on <interface> for packets.\n");
	printf("    filter       Rules to filter packets.\n");
	printf("\n");
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	static int count = 1;                  
	struct libnet_ipv4_hdr *ip;              

	libnet_t *libnet_handler = (libnet_t *)args;
	count++;
	
	ip = (struct libnet_ipv4_hdr*)(packet + ETHERNET_H_LEN);

	if(ip->ip_ttl != SPECIAL_TTL) {
		ip->ip_ttl = SPECIAL_TTL;
		ip->ip_sum = 0;
		if(ip->ip_p == IPPROTO_TCP) {
			struct libnet_tcp_hdr *tcp = (struct libnet_tcp_hdr *)((u_int8_t *)ip + ip->ip_hl * 4);
			tcp->th_sum = 0;
			libnet_do_checksum(libnet_handler, (u_int8_t *)ip, IPPROTO_TCP, LIBNET_TCP_H);
		} else if(ip->ip_p == IPPROTO_UDP) {
			struct libnet_udp_hdr *udp = (struct libnet_udp_hdr *)((u_int8_t *)ip + ip->ip_hl * 4);
			udp->uh_sum = 0;
			libnet_do_checksum(libnet_handler, (u_int8_t *)ip, IPPROTO_UDP, LIBNET_UDP_H);
		}
		int len_written = libnet_adv_write_raw_ipv4(libnet_handler, (u_int8_t *)ip, ntohs(ip->ip_len));
		if(len_written < 0) {
			printf("packet len:[%d] actual write:[%d]\n", ntohs(ip->ip_len), len_written);
			printf("err msg:[%s]\n", libnet_geterror(libnet_handler));
		}
	} else {
		//The packet net_speeder sent. nothing todo
	}
	return;
}

libnet_t* start_libnet(char *dev) {
	char errbuf[LIBNET_ERRBUF_SIZE];
	libnet_t *libnet_handler = libnet_init(LIBNET_RAW4_ADV, dev, errbuf);

	if(NULL == libnet_handler) {
		printf("libnet_init: error %s\n", errbuf);
	}
	return libnet_handler;
}

#define ARGC_NUM 3
int main(int argc, char **argv) {
	char *dev = NULL;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle;

	char *filter_rule = NULL;
	struct bpf_program fp;
	bpf_u_int32 net, mask;

	if (argc == ARGC_NUM) {
		dev = argv[1];
		filter_rule = argv[2];
		printf("Device: %s\n", dev);
		printf("Filter rule: %s\n", filter_rule);
	} else {
		print_usage();	
		return -1;
	}
	
	printf("ethernet header len:[%d](14:normal, 16:cooked)\n", ETHERNET_H_LEN);

	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		printf("Couldn't get netmask for device %s: %s\n", dev, errbuf);
		net = 0;
		mask = 0;
	}

	printf("init pcap\n");
	
	handle = net_speeder_pcap_open_live(dev, SNAP_LEN, 1, 0, errbuf);
	if(handle == NULL) {
		printf("net_speeder_pcap_open_live dev:[%s] err:[%s]\n", dev, errbuf);
		printf("init pcap failed\n");
		return -1;
	}

	printf("init libnet\n");
	libnet_t *libnet_handler = start_libnet(dev);
	if(NULL == libnet_handler) {
		printf("init libnet failed\n");
		return -1;
	}

	if (pcap_compile(handle, &fp, filter_rule, 0, net) == -1) {
		printf("filter rule err:[%s][%s]\n", filter_rule, pcap_geterr(handle));
		return -1;
	}

	if (pcap_setfilter(handle, &fp) == -1) {
		printf("set filter failed:[%s][%s]\n", filter_rule, pcap_geterr(handle));
		return -1;
	}

	while(1) {
		pcap_loop(handle, 1, got_packet, (u_char *)libnet_handler);
	}

	/* cleanup */
	pcap_freecode(&fp);
	pcap_close(handle);
	libnet_destroy(libnet_handler);
	return 0;
}
