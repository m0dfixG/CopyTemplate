#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>

#include <stdbool.h>//boolean, yes i want

int val = 1;
int term = 1;
void finish(int i){term = 0;}

int check(u_int8_t *buf, u_int8_t sender[6], u_int8_t target[6]){
	if(!memcmp(buf+6, target, 6) && memcmp(buf, sender, 6)){
		return 1;
	}
	if(!memcmp(buf, target, 6) && memcmp(buf+6, sender, 6)){
		return 2;
	}
	return 0;
}

void proc_packet(u_int8_t *buf, int flag){
	if(buf[12] == 0x08 && buf[13] == 0x06){
		
		//arppoison

	}else if(buf[12] == 0x86 && buf[13] == 0xDD){
		struct ip6_hdr *i6 = (struct ip6_hdr *)&buf[14];
		if(i6->ip6_nxt == 0x3A){
			struct icmphdr *icmp = (struct icmphdr *)&buf[54];
			if(icmp->type == 136){
			
				//ndppoison
	
			}
		}
	}
}


int main(int c, const char *args[]){
	signal(SIGTERM, finish);
	signal(SIGKILL, finish);
	signal(SIGINT, finish);

	int mirror = if_nametoindex(args[1]);
	int sender = if_nametoindex(args[2]);

	u_int8_t sender_mac[6];
	u_int8_t target_mac[6];
	u_int8_t gateway_mac[6];

	//target_mac init
	sscanf(args[3], "%x:%x:%x:%x:%x:%x", &target_mac[0], &target_mac[1], &target_mac[2], &target_mac[3], &target_mac[4], &target_mac[5]);
	sscanf(args[4], "%x:%x:%x:%x:%x:%x", &target_mac[0], &gateway_mac[1], &gateway_mac[2], &gateway_mac[3], &gateway_mac[4], &gateway_mac[5]);
	
	//sender_mac init
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct ifreq ifr;
	strncpy(ifr.ifr_name, args[2], IFNAMSIZ-1);
	ioctl(sock, SIOCGIFHWADDR, &ifr);
	close(sock);
	memcpy(sender_mac, ifr.ifr_hwaddr.sa_data, 6);

	//test display
	printf("R\\%02x:%02x:%02x:%02x:%02x:%02x -> S\\%02x:%02x:%02x:%02x:%02x:%02x -> T\\%02x:%02x:%02x:%02x:%02x:%02x\n", 
	gateway_mac[0], gateway_mac[1], gateway_mac[2], gateway_mac[3], gateway_mac[4], gateway_mac[5],
	sender_mac[0], sender_mac[1], sender_mac[2], sender_mac[3], sender_mac[4], sender_mac[5],
	target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5]);

	
	printf("mirror -> %d\n", mirror);
	printf("sender -> %d\n", sender);

	int receive_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	int send_arp = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
	int send_ndp = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);

	printf("raw -> %d\n", receive_raw);
	printf("arp -> %d\n", send_arp);
	printf("ndp -> %d\n", send_ndp);

	//setup raw
	ioctl(receive_raw, FIONBIO, &val);
	struct packet_mreq req;
	memset(&req, 0, sizeof(req));
	req.mr_type = PACKET_MR_PROMISC;
	req.mr_ifindex = mirror;
	req.mr_alen = 0;
	req.mr_address[0] ='\0';
	setsockopt(receive_raw, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (char *)&req, sizeof(req));

	//setup raw2
	struct sockaddr_ll m_addr;
	m_addr.sll_family = AF_PACKET;
	m_addr.sll_ifindex = mirror;
	m_addr.sll_protocol = htons(ETH_P_ALL);
	bind(receive_raw, (struct sockaddr *)&m_addr, sizeof(m_addr));

	//select
	fd_set fds, ZERO_F;
	FD_ZERO(&ZERO_F);
	FD_SET(receive_raw, &ZERO_F);
	
	u_int8_t buf[128];
	struct sockaddr_ll recv_addr;
	size_t size_addr = sizeof(struct sockaddr_ll);
	int flag;

	while(term){
		memcpy(&fds, &ZERO_F, sizeof(fd_set));
		select(receive_raw + 1, &fds, NULL, NULL, NULL);
		if(FD_ISSET(receive_raw, &fds)){
			memset(buf, 0, sizeof(buf));
			recvfrom(receive_raw, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &size_addr);
			flag = check(buf, sender_mac, target_mac);
			if(flag > 0){
				proc_packet(buf, flag);
			}
		}
	}
	

	close(receive_raw);
	close(send_arp);
	close(send_ndp);
	
	return 0;
}
