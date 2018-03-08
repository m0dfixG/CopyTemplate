#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>



int term = 1;
void finish(int i){term = 0;}

int set_promiscuous(int sock, int index){
	struct packet_mreq req;
	
	memset(&req, 0, sizeof(req));
	
	req.mr_type = PACKET_MR_PROMISC;
	req.mr_ifindex = index;
	req.mr_alen = 0;
	req.mr_address[0] ='\0';	
	
	if(setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (char *)&req, sizeof(req)) != 0){
		perror("set_promiscuous");
		return 1;
	}

	return 0;
}

int val = 1;
void set_nonblocking(int sock){
	ioctl(sock, FIONBIO, &val);
}

int get_addr(char *hostname, char *service, struct addrinfo hints, struct addrinfo *res){
	int err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = PF_UNSPEC;
	
	if((err = getaddrinfo(hostname, service, &hints, &res)) != 0){
		printf("error %d\n", err);
		return 1;
	}

	return 0;
}

int main(int c, const char *args[]){

	signal(SIGTERM, finish);
	signal(SIGKILL, finish);
	signal(SIGINT, finish);

	int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	//int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0){
		perror("socket");
		exit(1);
	}

	int index = if_nametoindex("eth0");

	while(term){
		printf("do anything\n");
		sleep(1);
		//usleep(100);	
	}
	
	close(sock);
	
	return 0;

}
