#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"
#include "radius_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"
#include "mem_cache.h"
#include "pppoe_buf.h"
#include "pppoe_thread.h"
#include "pppoe_ippool.h"
#include "pppoe_session.h"

#include "pppoe_ppp.h"


#define PROTO_HASHBITS		16
#define PROTO_HASHSIZE		(1 << PROTO_HASHBITS)

struct ppp_proto {
	uint32 proto;
	protoFunc process;
	struct hlist_node next;
};


static struct hlist_head protoHash[PROTO_HASHSIZE];


static inline struct hlist_head *
proto_hash(uint32 proto) {
	return &protoHash[proto & (PROTO_HASHSIZE - 1)];
}

static inline struct ppp_proto *
proto_get(uint32 proto) {
	struct hlist_node *node;
	hlist_for_each(node, proto_hash(proto)) {
		struct ppp_proto *pos
			= hlist_entry(node, struct ppp_proto, next);
		if (pos->proto == proto)  
			return pos;
	}
	return NULL;
}

int
ppp_proto_process(uint32 proto, session_struct_t *sess, struct pppoe_buf *pbuf) {
	struct ppp_proto *pos = proto_get(proto);
	if (!pos) 
		return PPPOEERR_ENOEXIST;

	return pos->process(sess, pbuf);
}


int
ppp_proto_register(uint32 proto, protoFunc process) {
	struct ppp_proto *pos;

	if (unlikely(!process))
		return PPPOEERR_EINVAL;

	pos = proto_get(proto);
	if (unlikely(pos)) 
		return PPPOEERR_EEXIST;

	pos = (struct ppp_proto *)malloc(sizeof(struct ppp_proto));
	if (unlikely(!pos))
		return PPPOEERR_ENOMEM;

	memset(pos, 0, sizeof(*pos));
	pos->proto = proto; 
	pos->process = process;

	hlist_add_head(&pos->next, proto_hash(proto));
	return PPPOEERR_SUCCESS;
}

int
ppp_proto_unregister(uint32 proto) {
	struct ppp_proto *pos = proto_get(proto);
	if (unlikely(!pos)) 
		return PPPOEERR_EEXIST;

	hlist_del(&pos->next);
	free(pos);
	return PPPOEERR_SUCCESS;
}

void
ppp_proto_init(void) {
	memset(protoHash, 0, sizeof(protoHash));
}

void
ppp_proto_exit(void) {
	struct ppp_proto *sproto;
	struct hlist_node *pos;
	int i = 0;
	
	for(; i < PROTO_HASHSIZE; i++) {
		hlist_for_each_entry(sproto, pos, 
						&protoHash[i], next) {
			free(sproto);					
		}
		INIT_HLIST_HEAD(&protoHash[i]);
	}
}


int
ppp_recv_packet(int sk, struct sockaddr_pppoe *addr, struct pppoe_buf *pbuf) {
	struct iovec iov = { pbuf->data, pbuf->end - pbuf->data };
	struct msghdr msg = { (void *)addr, sizeof(struct sockaddr_pppoe), &iov, 1, NULL, 0, 0 };
	char errbuf[128];	
	ssize_t length;

	do {
		length = recvmsg(sk, &msg, 0);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "socket %d recv failed: %s\n", 
							sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ERECV;			
		}
	} while (length < 0);

	pbuf->len = length;
	pbuf->tail = pbuf->data + pbuf->len;
	return PPPOEERR_SUCCESS;
}

int
ppp_send_packet(int sk, struct sockaddr_pppoe *addr, struct pppoe_buf *pbuf) {
	struct iovec iov = { pbuf->data, pbuf->tail - pbuf->data };
	struct msghdr msg= { (void *)addr, sizeof(struct sockaddr_pppoe), &iov, 1, NULL, 0, 0 };
	char errbuf[128];		
	ssize_t length;

	do {
		length = sendmsg(sk, &msg, 0);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "socket %d send failed: %s\n", 
							sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ESEND;			
		}
	} while (length < 0);
		
	return PPPOEERR_SUCCESS;
}
