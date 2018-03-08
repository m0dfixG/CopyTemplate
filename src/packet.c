#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#define ether_header ethhdr;
#define ip_header iphdr;
#define ip6_header ip6_hdr;
#define icmp_header icmp;
#define udp_header udphdr;
#define tcp_header tcphdr;

int main(){
	return 0;
}
