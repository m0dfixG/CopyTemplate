#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int c, const char *args[]){
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(56000);
	addr.sin_addr.s_addr = inet_addr(args[1]);

	bind(sock, (struct sockaddr *)&addr, sizeof(addr));

	//ioctl();

	fd_set fds, ZERO_F;
	FD_ZERO(&ZERO_F);
	FD_SET(sock, &ZERO_F);

	/*
	int sock1 = socket(3);
	int sock2 = socket(3);
	int maxfd;
	maxfd = sock1 < sock2 ? sock2 : sock1;
	*/
	
	u_int8_t buf[512];
	struct sockaddr_in recv_addr;
	size_t size_addr = sizeof(recv_addr);

	while(1){
		memcpy(&fds, &ZERO_F, sizeof(fd_set));
		select(sock + 1, &fds, NULL, NULL, NULL);
		if(FD_ISSET(sock, &fds)){
			memset(buf, 0, sizeof(buf));
			recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &size_addr);
		}
	}

	close(sock);
	
	return 0;
}

