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

	u_int8_t buf[1024];
	struct sockaddr_ll recv_addr;
	size_t size_addr = sizeof(struct sockaddr_ll);
	
	struct ip6_hdr *i6;
	struct icmphdr *icmp;
	int i;
	
	while(term){
		memcpy(&fds, &ZERO_F, sizeof(fd_set));
		select(sock + 1, &fds, NULL, NULL, NULL);
		if(FD_ISSET(sock, &fds)){
			memset(buf, 0, sizeof(buf));
			
			recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &size_addr);
			if(buf[12] == 0x86 && buf[13] == 0xdd){
				i6 = (struct ip6_hdr *)&buf[14];
				if(i6->ip6_nxt == 0x3A){
					printf("--------start--------\n");
					printf("%02x:%02x:%02x:%02x:%02x:%02x -> %02x:%02x:%02x:%02x:%02x:%02x\n", buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
					printf("SRC  :");
                                	for(i = 0;i < 16;i+=2){
                                        	if(i != 0)printf(":");
                                        	printf("%02X%02X", i6->ip6_src.s6_addr[i], i6->ip6_src.s6_addr[i+1]);
                                	}
                                	printf("\nDST  :");
                                	for(i = 0;i < 16;i+=2){
                                		if(i != 0)printf(":");
                                        	printf("%02X%02X", i6->ip6_dst.s6_addr[i], i6->ip6_dst.s6_addr[i+1]);
                                	}

					icmp = (struct icmphdr *)&buf[54];
					printf("\nType :%d\n", icmp->type);

					if(icmp->type == 136 || icmp->type == 135){
						if(icmp->type == 136)printf("Router :%d Res :%d Override :%d\n", buf[58] >> 7 & 1, buf[58] >> 6 & 1, buf[58] >> 5 & 1);
						else if(icmp->type == 135)printf("Request : true\n");
						if(ntohs(i6->ip6_plen) >= 24){
							printf("Target :");
							for(i = 62;i < 78;i+=2){
                                                		if(i != 62)printf(":");
                                                		printf("%02X%02X", i6->ip6_dst.s6_addr[i], i6->ip6_dst.s6_addr[i+1]);
                                        		}
							printf("\n");
							if(ntohs(i6->ip6_plen) >= 32){
								printf("OPT : %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", buf[78], buf[79], buf[80], buf[81], buf[82], buf[83], buf[84], buf[85]);
							}
						}
					}
					printf("--------finish--------\n");
				}
			}
		}
	}

	close(sock);

	return 0;
}
