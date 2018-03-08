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

#define MAC_FORMAT "%02x:%02x:%02x:%02x:%02x:%02x"
#define IP_FORMAT "%d.%d.%d.%d"

typedef struct arp{
	u_int16_t htype;
	u_int16_t ptype;
	u_int8_t hsize;
	u_int8_t psize;
	u_int16_t op;
	u_int8_t dst_mac[6];
	u_int8_t dst_ip[4];
	u_int8_t src_mac[6];	
	u_int8_t src_ip[4];
} arp_t;

int term = 1;
void finish(int i){term = 0;}

int main(int argc, const char *args[]){
	signal(SIGTERM, finish);
	signal(SIGKILL, finish);
	signal(SIGINT, finish);

	int index = if_nametoindex("eth0");
	int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	struct sockaddr_ll addr;
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = index;
	addr.sll_protocol = htons(ETH_P_ALL);
	bind(sock, (struct sockaddr *)&addr, sizeof(addr));

	int val = 1;
	ioctl(sock, FIONBIO, &val);
	
	struct packet_mreq req;
	memset(&req, 0, sizeof(req));
	req.mr_type = PACKET_MR_PROMISC;
	req.mr_ifindex = index;
	req.mr_alen = 0;
        req.mr_address[0] ='\0';
	setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (char *)&req, sizeof(req));

	fd_set fds, ZERO_F;
	FD_ZERO(&ZERO_F);
	FD_SET(sock, &ZERO_F);

	u_int8_t buf[128];
	struct sockaddr_ll recv_addr;
	size_t size_addr = sizeof(struct sockaddr_ll);
	
	int i;
	arp_t c;

	while(term){
		memcpy(&fds, &ZERO_F, sizeof(fd_set));
		select(sock + 1, &fds, NULL, NULL, NULL);
		if(FD_ISSET(sock, &fds)){
			memset(buf, 0, sizeof(buf));
			
			recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &size_addr);
			//c = (arp_t *)((&buf[0])+14);
			memcpy(&c, &buf[0]+14, sizeof(arp_t));	
				
			if(buf[12] == 0x08 && buf[13] == 0x06){
				printf("DST MAC:");
				printf(MAC_FORMAT, c.dst_mac[0], c.dst_mac[1], c.dst_mac[2], c.dst_mac[3], c.dst_mac[4], c.dst_mac[5]);
				
				printf("\nDST IP :");
				printf(IP_FORMAT, c.dst_ip[0], c.dst_ip[1], c.dst_ip[2], c.dst_ip[3]);	
			
				printf("\n    OP :");
				printf("%x", ntohs(c.op));
				
				printf("\nSRC MAC:");
				printf(MAC_FORMAT, c.src_mac[0], c.src_mac[1], c.src_mac[2], c.src_mac[3], c.src_mac[4], c.src_mac[5]);
				
				printf("\nSRC IP :");
				printf(IP_FORMAT, c.src_ip[0], c.src_ip[1], c.src_ip[2], c.src_ip[3]);
				
				printf("\n\n");
			}
		}
	}

	close(sock);

	return 0;
}
