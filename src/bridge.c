#include <stdio.h>
#include <stdlib.h>		//close(1)
#include <stdbool.h>		//true false
#include <unistd.h>
#include <string.h>		//memset(3)
#include <signal.h>		//signal(2)
#include <sys/socket.h>		//socket(3)
#include <sys/ioctl.h>		//ioctl(3)
#include <linux/if_packet.h>	//sockaddr_ll
#include <arpa/inet.h>		//htons(1)
#include <net/ethernet.h>	//ETH_P_ALL
#include <net/if.h>		//if_nametoindex(1)

#define max(x, y) ((x < y) ? y : x)

/*
	I NEVER TRY TO EXECUTE THIE PROGRAM YET.
	BECAUSE I COULD NOT MAKE ENVIRONMENT.
*/


int term = 1;
void finish(int i){term = 0;}

int make_socket(const char *if_, struct sockaddr_ll *copy){
	int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	int index = if_nametoindex(if_);	

	struct sockaddr_ll addr;
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = index;
	addr.sll_halen = ETH_ALEN;
	addr.sll_protocol = htons(ETH_P_ALL);
	bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_ll));
	if(copy != NULL)memcpy(copy, &addr, sizeof(struct sockaddr_ll));

	int val = 1;
	ioctl(sock, FIONBIO, &val);  

	struct packet_mreq mreq;
	memset(&mreq, 0, sizeof(mreq));
	mreq.mr_type = PACKET_MR_PROMISC;
	mreq.mr_ifindex = index;
	setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));

	return sock;
}

static u_int8_t ZA[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
bool check(u_int8_t *buf, u_int8_t mac[6]){

	if(!memcmp(mac, ZA, 6))return true;
	if(!memcmp(buf, mac, 6) || !memcmp(buf+6, mac, 6))return false;
	
	return true;
}

void get_mac(char *interface, u_int8_t box[6]){
	int sock = socket(AF_PACKET, SOCK_DGRAM, 0);
	struct ifreq ifr;
	
	ifr.ifr_addr.sa_family = AF_PACKET;
	strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
	ioctl(sock, SIOCGIFHWADDR, &ifr);
	
	close(sock);
	int i;
	for(i = 0;i < 6;i++)box[i] = (unsigned char)ifr.ifr_hwaddr.sa_data[i];
}

void print_mac(u_int8_t mac[6]){
	printf("%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


//1: config file
//2: target mac address
int main(int c, const char *args[]){

	if(c == 0)exit(1);

	//signal method
	signal(SIGTERM, finish);
	signal(SIGKILL, finish);
	signal(SIGINT, finish);

	//reading conf
	FILE *fp = fopen(args[1], "r");
	if(fp == NULL)exit(1);
	
	char if1[128], if2[128];
	memset(if1, 0, sizeof(if1));
	memset(if2, 0, sizeof(if2));
	int re = fscanf(fp, "if_1=%s\nif_2=%s", if1, if2);
	printf("%s %s\n", if1, if2);
	if(re == EOF || !strcmp(if1, if2))exit(1);




	//set up mac
	u_int8_t mac[6], lan1[6], lan2[6];
	memset(mac, 0, sizeof(mac));
	get_mac(if1, lan1);
	get_mac(if2, lan2);
	if(c >= 3)sscanf(args[2], "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

	printf("Target : ");
	print_mac(mac);
	printf("\n");
	printf("Port 1 : ");
	print_mac(lan1);
	printf("\n");
	printf("Port 2 : ");
	print_mac(lan2);
	printf("\n");

	struct sockaddr_ll addr1, addr2;

	int sock1 = make_socket(if1, &addr1);
	int sock2 = make_socket(if2, &addr2);
	int maxfd;	

	fd_set fds, readOnly;
	FD_ZERO(&readOnly);
	FD_SET(sock1, &readOnly);
	FD_SET(sock2, &readOnly);
	maxfd = max(sock1, sock2) + 1;

	u_int8_t buf[1514];
	struct sockaddr_ll recv_addr;
	size_t recv_addr_size = sizeof(recv_addr);

	if(c == 3){
		printf("test mode finish\n");
		exit(1);
	}

	while(term){
		memcpy(&fds, &readOnly, sizeof(fd_set));
		select(maxfd, &fds, NULL, NULL, NULL);
		if(FD_ISSET(sock1, &fds)){
			memset(buf, 0, sizeof(buf));
			recvfrom(sock1, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &recv_addr_size);
			if(check(buf, mac) && check(buf, lan1))sendto(sock2, buf, sizeof(buf), 0, (struct sockaddr *)&addr2, sizeof(addr2));
		}
		if(FD_ISSET(sock2, &fds)){
			memset(buf, 0, sizeof(buf));
			recvfrom(sock2, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &recv_addr_size);
			if(check(buf, mac) && check(buf, lan2))sendto(sock1, buf, sizeof(buf), 0, (struct sockaddr *)&addr1, sizeof(addr1));
		}
	}

	close(sock1);
	close(sock2);
	
	return 0;

}
