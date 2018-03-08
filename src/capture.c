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

int term = 1;
void finish(int i){term = 0;}

int main(int c, const char *args[]){
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
	
	while(term){
		memcpy(&fds, &ZERO_F, sizeof(fd_set));
		select(sock + 1, &fds, NULL, NULL, NULL);
		if(FD_ISSET(sock, &fds)){
			memset(buf, 0, sizeof(buf));
			
			recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &size_addr);

			printf(MAC_FORMAT, buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
			printf(" -> ");
			printf(MAC_FORMAT, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);	
			printf("\n");
		}
	}

	close(sock);

	return 0;
}
