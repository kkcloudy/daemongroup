
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/if_ether.h>

#include "if_pppoe.h"

struct ppp_lcp {
	unsigned char code;
	unsigned char ident;
	unsigned short length;
	unsigned int magic;
};

static int pppoe_sk;

static int 
init_pppoe_socket(void){
	struct sockaddr_pppoe saddr;

	pppoe_sk = socket(AF_PPPOE, SOCK_RAW, 0);
	if(pppoe_sk <= 0) {
		printf("creat pppoe socket error\n");
		return -1;
	}

	printf("create pppoe socket is %d\n", pppoe_sk);

	memset(&saddr, 0, sizeof(struct sockaddr_pppoe));
	saddr.sa_family	= AF_PPPOE;
	strncpy(saddr.addr.dev, "pppoe1-1-1", sizeof(saddr.addr.dev) - 1);
	
	
	if(0 != bind(pppoe_sk,(struct sockaddr*)&saddr, sizeof(saddr))) {
		printf("bind saddr error\n");
		return -1;
	}

	return 0;
}

static int
pppoe_send_message(void *data, int len) {
	struct msghdr 	msg;
	struct iovec		iov;
	struct sockaddr_pppoe	poaddr;
	int 	ret;

	memset(&poaddr, 0, sizeof(struct sockaddr_pppoe));
	memset(&msg, 0 ,sizeof(struct msghdr));

	/*set poaddr*/
	poaddr.sa_family = AF_PPPOE;
	poaddr.addr.sid = htons(1);
	poaddr.addr.mac[0] = 0x00;
	poaddr.addr.mac[1] = 0x22;
	poaddr.addr.mac[2] = 0x5f;
	poaddr.addr.mac[3] = 0xa9;
	poaddr.addr.mac[4] = 0xee;
	poaddr.addr.mac[5] = 0x55;
	
	/*set message*/
	iov.iov_base = (void *)data;
	iov.iov_len = len;
	msg.msg_name = (void *)&poaddr;
	msg.msg_namelen = sizeof(poaddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/*send message*/
	ret = sendmsg(pppoe_sk, &msg, 0);

	printf("sendmsg return is %d\n",ret);
	
	if (ret < 0) {
		printf("sendmsg error:");
		return -1;
	}
	
	return 0;
}
#if 0

int 
main() {
	if(init_pppoe_socket()) {
		printf("init pppoe socket fail\n");
		return -1;
	}
	
	int len;
	char buf[128] = { 0 };

	
	unsigned short proto = htons(0xc021);
	struct ppp_lcp echo = {
		.code = 0x09,
		.ident = 0x00,
		.length = htons(sizeof(struct ppp_lcp)),
		.magic = htonl(0xd5c3a4cb)
	};

	memcpy(buf, &proto, sizeof(unsigned short));
	memcpy(buf + 2, &echo, sizeof(struct ppp_lcp));

	len = sizeof(unsigned short) + sizeof(struct ppp_lcp);

	while(1) {
		if(pppoe_send_message(buf, len)) {
			printf("send pppoe packet error\n");
		}
		sleep(10);
		printf("waitting .............\n");
	}

	return 0;
}

#endif

static int 
pppoe_recv_message(void *data, int len) {
	int ret;
	struct iovec iov = { data, len };
	struct sockaddr_pppoe spo;
	struct msghdr msg = { (void *) &spo, sizeof(struct sockaddr_pppoe), &iov, 1, NULL, 0, 0 };
	
	ret = recvmsg(pppoe_sk, &msg, 0);
	if (ret <= 0) {
		printf( "pppoe recv fail \n");
		return -1;
	}

	printf("pppoe receive sid is %d\n", (unsigned int)ntohs(spo.addr.sid));
	printf("mac is %2x:%2x:%2x:%2x:%2x:%2x\n", spo.addr.mac[0], spo.addr.mac[1], spo.addr.mac[2],
											spo.addr.mac[3], spo.addr.mac[4], spo.addr.mac[5]);
	
	return 0;
}



int 
main() {
	int ret, numfds, count;
	fd_set readfds;
	unsigned char buf[1500];
	unsigned short proto;
	if(init_pppoe_socket()) {
		printf("init pppoe socket fail\n");
		return -1;
	}


	while(1) {
		struct timeval tvp = {
			.tv_sec = 3,
			.tv_usec = 0
		};

		
		FD_ZERO(&readfds);
		numfds = pppoe_sk + 1;
		FD_SET(pppoe_sk, &readfds);
		
	reselect:
		count = select(numfds, &readfds, NULL, NULL, &tvp);
		printf("pppoe select, count = %d, tvp.tv_sec = %d, tvp.tv_usec = %d\n", count, tvp.tv_sec, tvp.tv_usec);
		if (count > 0) {
			if (FD_ISSET(pppoe_sk, &readfds)) {
				if (pppoe_recv_message(buf, sizeof(buf))) {
					printf("pppoe recv message fail\n");
					goto out;
				}
				proto = buf[0] << 8 + buf[1];
				printf("pppoe message proto is 0x%x\n", proto);
			}
		} else switch(count) {
			case 0:
				printf( "pppoe select EOF\n");
				break;
					
			case -1:
				printf("pppoe select , errno = %d\n", errno);
				if (errno == EINTR) {
					goto reselect;
				} 
				printf("pppoe select fail\n");
				ret = -1;
				goto out;
					
			default:
				printf("pppoe select returned %d\n", count);
				ret = -1;
				goto out;
		}
	}	

out:
	return ret;
}

