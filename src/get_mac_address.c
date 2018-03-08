#include <stdio.h>
#include <unistd.h>//close(1)
#include <string.h> //strncpy(3)
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>//ifreq

int main(int c, const char *args[]){
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	struct ifreq ifr;
	
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, args[1], IFNAMSIZ-1);
	ioctl(sock, SIOCGIFHWADDR, &ifr);

	close(sock);
	int i;
	for(i = 0;i < 6;i++){
		if(i != 0)printf(":");
		printf("%02x", (unsigned char)ifr.ifr_hwaddr.sa_data[i]);
	}
	printf("\n");
	
	return 0;
}
