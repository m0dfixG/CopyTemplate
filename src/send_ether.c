#include <stdio.h>
#include <unistd.h>
#include <string.h> //memset(3);
#include <net/if.h> //if_nametoindex(1)
#include <net/ethernet.h>//ETH_P_ALL
#include <sys/types.h> //u_int8_t
#include <sys/socket.h> //socket(3)
#include <linux/if_packet.h> //sockaddr_ll
#include <arpa/inet.h> //htons(1)

int main(int c, const char *args[]){
	int index = if_nametoindex(args[1]);
	u_int8_t buf[6];
	sscanf(args[2], "%x:%x:%x:%x:%x:%x", &buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5]);
	
	int sock = socket(AF_PACKET, SOCK_DGRAM, 0);

	struct sockaddr_ll addr;
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = index;
	addr.sll_protocol = htons(0x08 << 8 | 0x00);
	addr.sll_halen = 6;
	memset(addr.sll_addr, 0, sizeof(addr.sll_addr));
	memcpy(addr.sll_addr, buf, 6);
	
	sendto(sock, args[3], sizeof(args[3]), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_ll));

	close(sock);

	return 0;
}
