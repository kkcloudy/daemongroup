#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>

#include <linux/types.h>
#include <asm/byteorder.h>

#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "md5.h"
#include "pppoe_buf.h"
#include "pppoe_list.h"
#include "pppoe_util.h"
#include "pppoe_thread.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"
#include "pppoe_ppp.h"

enum {
	CLIENT_PADI,
	CLIENT_PADR,
	CLIENT_LCPCONFIG,
	CLIENT_LCPACK,
	CLIENT_CHALLENGE,
	CLIENT_IPCPCONFIG,
	CLIENT_ONLINE,
};

struct ppp_packet {
	unsigned short proto;
	struct pppoe_ctl ctl;
} __attribute ((packed));

struct session_client {
	unsigned char ident;
	unsigned int sid;
	unsigned int ip;
	unsigned int state;

	unsigned char mac[ETH_ALEN];
	unsigned char server[ETH_ALEN];

	unsigned char magic[4];
	unsigned char uniq[8];
	unsigned char cookie[20];
	unsigned char chap_passwd[16];

	char sname[64];	
};

static unsigned int seed;
static int disc_sk, sess_sk;
static struct session_client *sessClients;
static struct discover_tag sname, acCookie;
static unsigned char startMac[ETH_ALEN];
static unsigned int	sess_num;
static char	*ifname;
static char *username = "aaa", *passwd = "aaa";

static inline void
index2mac(unsigned char *mac, unsigned char *startMac, unsigned int index) {
	unsigned int *p;
	
	memcpy(mac, startMac, ETH_ALEN);

	p = (unsigned int *)(mac + 2);
	*p = htonl(ntohl(*p) + index);
}

static inline unsigned int
mac2index(unsigned char *mac, unsigned char *startMac) {
	unsigned int tmp, *p;

	p = (unsigned int *)(mac + 2);
	tmp = htonl(*p);
	
	p = (unsigned int *)(startMac + 2);
	return tmp - htonl(*p);
}

static inline int
str2mac(char *str, unsigned char *mac) {
	char *cursor = str;
	unsigned int len = strlen(str);
	int i;
	
	if (17 != len) {
		return -1;
	}

	for (i = 0; i < 6; i++) {
		if (*cursor >= '0' && *cursor <= '9') {
			mac[i] = (*cursor - '0') << 4;
		} else if (*cursor >= 'a' && *cursor <= 'f') {
			mac[i] = (*cursor - 'a' + 10) << 4;
		} else if (*cursor >= 'A' && *cursor <= 'F') {
			mac[i] = (*cursor - 'A' + 10) << 4;
		} else {
			return -1;
		}
		cursor ++;
		len --;

		if (*cursor >= '0' && *cursor <= '9') {
			mac[i] += *cursor - '0';
		} else if (*cursor >= 'a' && *cursor <= 'f') {
			mac[i] += *cursor - 'a' + 10;
		} else if (*cursor >= 'A' && *cursor <= 'F') {
			mac[i] += *cursor - 'A' + 10;
		} else {
			return -1;
		}
		cursor ++;
		len --;
		
		if (!len) {
			break;
		}
		
		if (*cursor != ':') {
			return -1;
		}
		cursor ++;
		len --;
	}

	return 0;
}

static inline int
_recv_packet(int sk, unsigned char *buf, unsigned int length) {
	return recv(sk, buf, length, 0);
}

static inline int
_send_packet(int sk, unsigned char *buf, unsigned int length) {
	return send(sk, buf, length, 0);
}

static void
discover_tag_process(unsigned char *data, unsigned int length) {
	unsigned char *p = data;
	unsigned int len = length, tlen;
	unsigned short code;

	memset(&sname, 0, 4);
	memset(&acCookie, 0, 4);

	while (len) {
		if (len <  4)
			return;

		code = GETSHORT(p);
		tlen = GETSHORT(p + 2);
		
		if (TAG_SERVICE_NAME == code) {
			sname.type = TAG_SERVICE_NAME;
			sname.length = tlen;
			memcpy(sname.payload, p + 4, tlen);
		} else if (TAG_AC_COOKIE == code) {
			acCookie.type = TAG_AC_COOKIE;
			acCookie.length = tlen;
			memcpy(acCookie.payload, p + 4, tlen);
		}

		p += 4 + tlen;
		len -= 4 + tlen;
	}
}

static void 
session_echo_reply(int sk, struct session_client *client) {
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;
	struct ppp_packet *p = (struct ppp_packet *)pack->phdr.data;

	memcpy(pack->ethHdr.h_dest, client->server, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, client->mac, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_SES);

	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = 0;
	pack->phdr.sid = htons(client->sid);

	p->proto = htons(PPP_LCP);

	p->ctl.code = ECHOREP;
	p->ctl.ident = 0;
	p->ctl.length = htons(sizeof(struct pppoe_ctl) + 4);
	memcpy(p->ctl.data, client->magic, 4);

	pack->phdr.length = htons(sizeof(struct ppp_packet) + 4);

	_send_packet(sk, buf, sizeof(struct pppoe_packet) + sizeof(struct ppp_packet) + 4);
}

static void
session_discover_PADI(int sk, struct session_client *client) {
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;

	memset(pack->ethHdr.h_dest, 0xff, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, client->mac, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_DISC);

	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = CODE_PADI;
	pack->phdr.sid = 0;
	
	pack->phdr.data[0] = TAG_AC_NAME >> 8;
	pack->phdr.data[1] = TAG_AC_NAME & 0xff;
	pack->phdr.data[2] = 0;
	pack->phdr.data[3] = 0;

	pack->phdr.data[4] = TAG_HOST_UNIQ >> 8;
	pack->phdr.data[5] = TAG_HOST_UNIQ & 0xff;
	pack->phdr.data[6] = 0;
	pack->phdr.data[7] = 8;
	rand_bytes(&seed, client->uniq, 8);
	memcpy(pack->phdr.data + 8, client->uniq, 8);	
	
	pack->phdr.length = htons(16);
	
	_send_packet(sk, buf, sizeof(struct pppoe_packet) + 16);
}

static void
session_discover_PADR(int sk, struct session_client *client) {
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;
	unsigned int slen = strlen(client->sname);

	memcpy(pack->ethHdr.h_dest, client->server, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, client->mac, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_DISC);
	
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = CODE_PADR;
	pack->phdr.sid = 0;

	pack->phdr.data[0] = TAG_SERVICE_NAME >> 8;
	pack->phdr.data[1] = TAG_SERVICE_NAME & 0xff;
	pack->phdr.data[2] = slen >> 8;
	pack->phdr.data[3] = slen & 0xff;
	memcpy(pack->phdr.data + 4, client->sname, slen);

	pack->phdr.data[4 + slen] = TAG_HOST_UNIQ >> 8;
	pack->phdr.data[5 + slen] = TAG_HOST_UNIQ & 0xff;
	pack->phdr.data[6 + slen] = 0;
	pack->phdr.data[7 + slen] = 8;
	memcpy(pack->phdr.data + 8 + slen, client->uniq, 8);	
	
	pack->phdr.data[16 + slen] = TAG_AC_COOKIE >> 8;
	pack->phdr.data[17 + slen] = TAG_AC_COOKIE & 0xff;
	pack->phdr.data[18 + slen] = 0;
	pack->phdr.data[19 + slen] = 0x14;
	memcpy(pack->phdr.data + 20 + slen, client->cookie, 20);

	pack->phdr.length = htons(40 + slen);

	_send_packet(sk, buf, sizeof(struct pppoe_packet) + 40 + slen);
}

static void
session_discover_PADT(int sk, struct session_client *client) {
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;

	memcpy(pack->ethHdr.h_dest, client->server, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, client->mac, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_DISC);
	
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = CODE_PADT;
	pack->phdr.sid = htons(client->sid);
	pack->phdr.length = 0;

	_send_packet(sk, buf, sizeof(struct pppoe_packet));
}


static void
session_lcp_request(int sk, struct session_client *client) {
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;
	struct ppp_packet *pctl = (struct ppp_packet *)pack->phdr.data;
	unsigned char *cursor;

	memcpy(pack->ethHdr.h_dest, client->server, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, client->mac, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_SES);
	
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = 0;
	pack->phdr.sid = htons(client->sid);

	pctl->proto = htons(PPP_LCP);
	pctl->ctl.code = CONFREQ;
	pctl->ctl.ident = 0;

	cursor = pctl->ctl.data;
	PUTCHAR(cursor, CI_MRU);
	PUTCHAR(cursor, CILEN_SHORT);
	PUTSHORT(cursor, 1412);

	PUTCHAR(cursor, CI_MAGICNUMBER);
	PUTCHAR(cursor, CILEN_LONG);

	rand_bytes(&seed, client->magic, 4);
	memcpy(cursor, client->magic, 4);
	cursor += 4;
	
	pctl->ctl.length = htons(sizeof(struct pppoe_ctl) + (cursor - pctl->ctl.data));
	pack->phdr.length = htons(sizeof(struct ppp_packet) + (cursor - pctl->ctl.data));

	_send_packet(sk, buf, sizeof(struct pppoe_packet) + sizeof(struct ppp_packet) + (cursor - pctl->ctl.data));
}


static void
session_chap_response(int sk, struct session_client *client) {
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;
	struct ppp_packet *pctl = (struct ppp_packet *)pack->phdr.data;
	unsigned char *cursor;

	memcpy(pack->ethHdr.h_dest, client->server, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, client->mac, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_SES);
	
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = 0;
	pack->phdr.sid = htons(client->sid);

	pctl->proto = htons(PPP_CHAP);
	pctl->ctl.code = CHAP_RESPONSE;
	pctl->ctl.ident = client->ident;

	cursor = pctl->ctl.data;

	PUTCHAR(cursor, 16);
	memcpy(cursor, client->chap_passwd, 16);
	memcpy(cursor + 16, username, strlen(username));
	
	pctl->ctl.length = htons(sizeof(struct pppoe_ctl) + 17 + strlen(username));
	pack->phdr.length = htons(sizeof(struct ppp_packet) + 17 + strlen(username));

	_send_packet(sk, buf, sizeof(struct pppoe_packet) + sizeof(struct ppp_packet) + 17 + strlen(username));
}


static void
session_ipcp_request(int sk, struct session_client *client) {
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;
	struct ppp_packet *pctl = (struct ppp_packet *)pack->phdr.data;
	unsigned char *cursor;

	memcpy(pack->ethHdr.h_dest, client->server, ETH_ALEN);
	memcpy(pack->ethHdr.h_source, client->mac, ETH_ALEN);
	pack->ethHdr.h_proto = htons(ETH_P_PPP_SES);
	
	pack->phdr.ver = 1;
	pack->phdr.type = 1;
	pack->phdr.code = 0;
	pack->phdr.sid = htons(client->sid);

	pctl->proto = htons(PPP_IPCP);
	pctl->ctl.code = CONFREQ;
	pctl->ctl.ident = 0;

	cursor = pctl->ctl.data;
	PUTCHAR(cursor, CI_ADDR);
	PUTCHAR(cursor, CILEN_ADDR);
	PUTLONG(cursor, client->ip);
	
	pctl->ctl.length = htons(sizeof(struct pppoe_ctl) + CILEN_ADDR);
	pack->phdr.length = htons(sizeof(struct ppp_packet) + CILEN_ADDR);

	_send_packet(sk, buf, sizeof(struct pppoe_packet) + sizeof(struct ppp_packet) + CILEN_ADDR);
}


static int
packet_process(fd_set *readfds, unsigned int sess_index) {
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;
	unsigned int index = 0xffff + 1;

	if (FD_ISSET(disc_sk, readfds)) {
		if (_recv_packet(disc_sk, buf, sizeof(buf)) < 0) {
			printf("packet_process: discover recv fail\n");
			goto sessrecv;
		}
		
		index = mac2index(pack->ethHdr.h_dest, startMac);
		if (index >= sess_num) {
			printf("packet_process: discover recv mac dest is error\n");
			goto sessrecv;
		}

		if (CODE_PADT == pack->phdr.code) {
			memset(&sessClients[index], 0, sizeof(struct session_client));

			if (index == sess_index)
				goto fail;
						
			goto sessrecv;
		}

		switch (pack->phdr.code) {
			case CODE_PADO:
				discover_tag_process(pack->phdr.data, ntohs(pack->phdr.length));
				if (!sname.type || !acCookie.type) {
					printf("packet_process: discover recv PADO tag error\n");
					break;
				}
			
				memcpy(sessClients[index].server, pack->ethHdr.h_source, ETH_ALEN);	
				memcpy(sessClients[index].sname, sname.payload, sname.length);
				memcpy(sessClients[index].cookie, acCookie.payload, acCookie.length);
				session_discover_PADR(disc_sk, &sessClients[index]);
				sessClients[index].state = CLIENT_PADR;
				break;
	
			case CODE_PADS:
				sessClients[index].sid = ntohs(pack->phdr.sid);
				session_lcp_request(sess_sk, &sessClients[index]);
				sessClients[index].state = CLIENT_LCPCONFIG;
				break;
		}
	} 

sessrecv:
	if(FD_ISSET(sess_sk, readfds)) {
		struct ppp_packet *pctl = (struct ppp_packet *)pack->phdr.data;
		unsigned short proto;

		if (_recv_packet(sess_sk, buf, sizeof(buf)) < 0) {
			printf("packet_process: sess recv fail\n");
			goto out;
		}
			
		index = mac2index(pack->ethHdr.h_dest, startMac);
		if (index >= sess_num) {
			printf("packet_process: sess recv mac dest is error\n");
			goto out;
		}

		proto = ntohs(pctl->proto);	

		if (PPP_LCP == proto && TERMREQ == pctl->ctl.code) {
			pctl->ctl.code = TERMACK;
			memcpy(pack->ethHdr.h_dest, sessClients[index].server, ETH_ALEN);
			memcpy(pack->ethHdr.h_source, sessClients[index].mac, ETH_ALEN);
			_send_packet(sess_sk, buf, sizeof(struct pppoe_packet) + sizeof(struct ppp_packet));

			printf("packet_process: session %u recv LCP Term Request\n", sessClients[index].sid);
			memset(&sessClients[index], 0, sizeof(struct session_client));
			goto term;	
		} else if (PPP_IPCP == proto && TERMREQ == pctl->ctl.code) {
			printf("packet_process: session %u recv IPCP Term Request\n", sessClients[index].sid);
			memset(&sessClients[index], 0, sizeof(struct session_client));
			goto term;
		} 

		if (index != sess_index)
			goto out;
		
		if (PPP_LCP == proto) {
			switch (pctl->ctl.code) {
				case CONFREQ:
					pctl->ctl.code = CONFACK;
					memcpy(pack->ethHdr.h_dest, sessClients[index].server, ETH_ALEN);
					memcpy(pack->ethHdr.h_source, sessClients[index].mac, ETH_ALEN);
					_send_packet(sess_sk, buf, sizeof(struct pppoe_packet) + ntohs(pack->phdr.length));
					break;
			
				case CONFACK:
					sessClients[index].state = CLIENT_LCPACK;
					break;
			}
		} else if (PPP_CHAP == proto) {
			switch (pctl->ctl.code) {
				case CHAP_CHALLENGE: {
						MD5_CTX context;			
						MD5Init(&context);
						MD5Update(&context, &pctl->ctl.ident, 1);
						MD5Update(&context, (uint8 *)passwd, strlen(passwd));
						MD5Update(&context, pctl->ctl.data + 1, 16);
						MD5Final(sessClients[index].chap_passwd, &context);							
						
						sessClients[index].ident = pctl->ctl.ident;
						session_chap_response(sess_sk, &sessClients[index]);
						sessClients[index].state = CLIENT_CHALLENGE;
					}
					break;
						
				case CHAP_SUCCESS:
					session_ipcp_request(sess_sk, &sessClients[index]);
					sessClients[index].state = CLIENT_IPCPCONFIG;
					break;

				case CHAP_FAILURE:
					printf("packet_process: session %u chap auth fail\n", sessClients[index].sid);
					memset(&sessClients[index], 0, sizeof(struct session_client));
					goto fail;
			}
		} else if (PPP_IPCP == proto) {
			switch(pctl->ctl.code) {
				case CONFREQ:
					pctl->ctl.code = CONFACK;
					memcpy(pack->ethHdr.h_dest, sessClients[index].server, ETH_ALEN);
					memcpy(pack->ethHdr.h_source, sessClients[index].mac, ETH_ALEN);
					_send_packet(sess_sk, buf, sizeof(struct pppoe_packet) + ntohs(pack->phdr.length));
					break;
						
				case CONFACK:
					sessClients[index].state = CLIENT_ONLINE;
					goto success;
				
				case CONFNAK:
					if (CI_ADDR == *(pctl->ctl.data) && CILEN_ADDR == *(pctl->ctl.data + 1)) {
						sessClients[index].ip = GETLONG(pctl->ctl.data + 2);
						session_ipcp_request(sess_sk, &sessClients[index]);
					}
					break;		
			}
		}
	}

out:
	return 0;

success:
	return 1;

fail:
	return -1;

term:
	if (index == sess_index)
		goto fail;

	goto out;
}

static void
pppoe_session_dispatch(unsigned int sess_index) {
	int count;
	fd_set readfds;
	long timer = time_sysup() + 10;

	if (SESSION_ONLINE == sessClients[sess_index].state)
		return;

	memset(&sessClients[sess_index], 0, sizeof(struct session_client));
	index2mac(sessClients[sess_index].mac, startMac, sess_index);
	session_discover_PADI(disc_sk, &sessClients[sess_index]);

	while (1) {
		struct timeval tvp = {
				.tv_sec = 0,
				.tv_usec = 100000
			};

		if (time_sysup() >= timer) {
			printf("session index %d timeout, so send PADT\n", sess_index);
			session_discover_PADT(disc_sk, &sessClients[sess_index]);
			memset(&sessClients[sess_index], 0, sizeof(struct session_client));
			return;
		}

		FD_ZERO(&readfds);
		FD_SET(disc_sk, &readfds);
		FD_SET(sess_sk, &readfds);

	reselect:
		count = select(sess_sk > disc_sk ? (sess_sk + 1) : (disc_sk + 1), 
					&readfds, NULL, NULL, &tvp);
		if (count > 0) {
			if (packet_process(&readfds, sess_index)) 
				return;
		} else switch (count) {
			case 0:
				/* select EOF */
				break;

			case -1:
				if (EINTR == errno) {
					goto reselect;
				}
				break;
				
			default:
				break;
		}		
			
	}
}

static int
pppoe_sock_init(char *ifname, unsigned int protocol) {
	struct sockaddr_ll saddr;
	struct ifreq ifr;
	int optval = 1;
	int sk;

	sk = socket(PF_PACKET, SOCK_RAW, htons(protocol));
	if (sk < 0) {
		printf("Cannot create raw socket -- must be run as root.\n");
		goto error;
	}	
	
	if (setsockopt(sk, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
		printf("socket setsockopt fail\n");
		goto error1;
	}

	/* ioctl get interface ifindex */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(sk, SIOCGIFINDEX, &ifr) < 0) {
		printf("ioctl(SIOCFIGINDEX): Could not get interface index\n");
		goto error1;
	}

	/* socket bind  interface */
	memset(&saddr, 0, sizeof(saddr));
	saddr.sll_family = AF_PACKET;
	saddr.sll_protocol = htons(protocol);
	saddr.sll_ifindex = ifr.ifr_ifindex;
	if (bind(sk, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
		printf("socket bind sockaddr fail\n");
		goto error1;
	}

	printf("socket %d bind %s protocol 0x%04x success\n", sk, ifname, protocol);
	return sk;

error1:
	close(sk);
error:	
	return -1;
}


static void *
pppoe_echo_reply_thread_run(void *arg) {
	int sk, count;
	unsigned char buf[1600];
	struct pppoe_packet *pack = (struct pppoe_packet *)buf;		

	sk = pppoe_sock_init(ifname, ETH_P_PPP_SES);
	if (sk < 0) {
		printf("pppoe echo reply thread create socket fail\n");
		return NULL;
	}

	while (1) {
		fd_set readfds;
		unsigned int index;
		struct timeval tvp = {
				.tv_sec = 0,
				.tv_usec = 100000
			};

		FD_ZERO(&readfds);
		FD_SET(sk, &readfds);

	reselect:
		count = select(sk + 1, &readfds, NULL, NULL, &tvp);
		if (count > 0) {
			if(FD_ISSET(sk, &readfds)) {
				struct ppp_packet *pctl = (struct ppp_packet *)pack->phdr.data;
				unsigned short proto;

				if (_recv_packet(sk, buf, sizeof(buf)) < 0) {
					printf("packet_process: sess recv fail\n");
					continue;
				}
					
				index = mac2index(pack->ethHdr.h_dest, startMac);
				if (index >= sess_num || memcmp(sessClients[index].mac, pack->ethHdr.h_dest, ETH_ALEN)) {
					printf("packet_process: sess recv mac dest is error\n");
					continue;
				}

				proto = ntohs(pctl->proto);	
				if (PPP_LCP == proto && ECHOREQ == pctl->ctl.code) {
					session_echo_reply(sk, &sessClients[index]);
				}	
			} 
		} else switch (count) {
			case 0:
				/* select EOF */
				break;

			case -1:
				if (EINTR == errno) {
					goto reselect;
				}
				break;
				
			default:
				break;
		}			
	}
	
	return NULL;
}

int 
main(int argc, char **argv) {
	pthread_t thread_id;
	
	if (argc < 4) {
		printf("input para error\n");
		goto error;
	}

	ifname = argv[1];	
	sess_num = atoi(argv[2]);
	if (!sess_num || sess_num > 0xffff) {
		printf("input para session num error, session num need 1-%u\n", 0xffff);
		goto error;
	}

	if (str2mac(argv[3], startMac)) {
		printf("input mac %s format fail\n", argv[3]);
		goto error;
	}
	
	sessClients = (struct session_client *)calloc(sess_num, sizeof(struct session_client));
	if (!sessClients) {
		printf("alloc session clients fail\n");
		goto error;
	}

	disc_sk = pppoe_sock_init(ifname, ETH_P_PPP_DISC);
	if (disc_sk < 0) {
		printf("discover socket init fail\n");
		goto error1;
	}
	
	sess_sk = pppoe_sock_init(ifname, ETH_P_PPP_SES);
	if (sess_sk < 0) {
		printf("session socket init fail\n");
		goto error2;
	}

	seed = time(NULL);

	if (pthread_create(&thread_id, NULL, pppoe_echo_reply_thread_run, NULL)) {
		printf("create echo reply thread fail\n");
		goto error3;
	}
	
	
	while (1) {
		unsigned int i;
		for (i = 0; i < sess_num; i++) {
			pppoe_session_dispatch(i);		
		}
	}

	return 0;

error3:
	close(sess_sk);
	sess_sk = -1;
error2:
	close(disc_sk);
	disc_sk = -1;
error1:
	PPPOE_FREE(sessClients);
error:
	return -1;	
}

