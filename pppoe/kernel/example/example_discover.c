
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>


#include "if_pppoe.h"

#define NOT_UNICAST(e) ((e[0] & 0x01) != 0)

static int
sock_init(char *ifname) {
	int sk;
	unsigned char hwaddr[ETH_ALEN] = { 0 };
	
	struct ifreq ifr;
	struct sockaddr_ll saddr;
	int optval = 1;
	
	sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_PPP_DISC));	/* init discover socket */
	if (sk < 0) {
		if (EPERM == errno) {
			printf("Cannot create raw socket -- must be run as root.\n");
		}
		printf("creat discover socket fail\n");
		goto error;
	}
	printf("create discover socket is %d\n", sk);
	

	/* set discover socket opt */
	if (setsockopt(sk, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
		printf("discover socket setsockopt fail\n");
		goto error1;
	}

	/* get pppoe base interface hwaddr  */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	printf("listen interface %s\n", ifr.ifr_name);
	if (ioctl(sk, SIOCGIFHWADDR, &ifr) < 0) {
		printf("ioctl(SIOCGIFHWADDR): Could not get interface hwaddr\n");
		goto error1;
	}
	memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	printf("discover hwaddr %02X:%02X:%02X:%02X:%02X:%02X\n", 
					hwaddr[0], hwaddr[1], hwaddr[2], 
					hwaddr[3], hwaddr[4], hwaddr[5]);

	/* base interface is need ether  */
	if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
		printf("Interface %.16s is not Ethernet\n", ifname);
		goto error1;
	}

	/* base interface hwaddr can not be broadcast/multicast MAC address */
	if (NOT_UNICAST(hwaddr)) {
		printf("Interface %.16s has broadcast/multicast MAC address??\n", ifname);
		goto error1;
	}

	/* discover socket bind base interface  */
	memset(&saddr, 0, sizeof(saddr));
	saddr.sll_family = AF_PACKET;
	saddr.sll_protocol = htons(ETH_P_PPP_DISC);
	if (ioctl(sk, SIOCGIFINDEX, &ifr) < 0) {
		printf("ioctl(SIOCFIGINDEX): Could not get interface index\n");
		goto error1;
	}
	saddr.sll_ifindex = ifr.ifr_ifindex;
	printf("bind addr ifindex %d\n", saddr.sll_ifindex);

	if (bind(sk, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		printf("discover socket bind sockaddr fail\n");
		goto error1;
	}

	return sk;
	
error1:
	close(sk);
error:
	return -1;
}

int
main(int argc, char **argv) {
	int sk, numfds, count;
	fd_set readfds;
	char *ifname;
	
	if (argc != 2) {
		printf("please input right para num\n");
		return -1;
	}

	ifname = argv[1];

	sk = sock_init(ifname);
	printf("sock init sk is %d\n", sk);

	FD_ZERO(&readfds);
	numfds = sk + 1;
	FD_SET(sk, &readfds);

reselect:
	count = select(numfds, &readfds, NULL, NULL, NULL);
	printf("discover select, count = %d\n", count);
	if (count > 0) {
		if (FD_ISSET(sk, &readfds)) {
			printf("discover socket recv message\n");
			goto out;
		}
	} else switch(count) {
		case 0:
			printf( "discover select EOF\n");
			goto out;
				
		case -1:
			printf("discover select , errno = %d\n", errno);
			if (errno == EINTR) {
				goto reselect;
			} 
			printf("discover select fail\n");
			goto out;
				
		default:
			printf("discover select returned %d\n", count);
			goto out;
	}
	
out:
	return 0;
}

