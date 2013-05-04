#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "radius_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"
#include "pppoe_buf.h"
#include "radius_packet.h"
#include "pppoe_thread.h"

#include "radius_coa.h"

#define COA_HASHBITS			8
#define COA_HASHSIZE			(1 << COA_HASHBITS)


struct coa_proto {
	uint8 proto;
	void *proto_ptr;
	coaProcess process;
	struct hlist_node next;
};

struct radius_coa {
	int sk;
	void *sk_ptr;

	struct pppoe_buf *pbuf;
	
	coaSendto sendto;
	coaRecvfrom recvfrom;

	thread_struct_t *thread;
	thread_master_t *master;

	struct hlist_head protoHash[COA_HASHSIZE];
};

static inline struct hlist_head *
proto_hash(radius_coa_t *coa, uint8 proto) {
	return &coa->protoHash[proto & (COA_HASHBITS - 1)];
}

static inline struct coa_proto *
proto_get(radius_coa_t *coa, uint8 proto) {
	struct hlist_node *node;
	hlist_for_each(node, proto_hash(coa, proto)) {
		struct coa_proto *pos
			= hlist_entry(node, struct coa_proto, next);
		if (pos->proto == proto)  
			return pos;
	}
	return NULL;
}


static inline int
coa_proto_process(radius_coa_t *coa, uint8 proto, 
			struct pppoe_buf *pbuf, uint32 radius_ip) {
	struct coa_proto *pos = proto_get(coa, proto);
	if (!pos) 
		return PPPOEERR_ENOEXIST;

	return pos->process(pos->proto_ptr, pbuf, radius_ip);
}


int
coa_proto_register(radius_coa_t *coa, uint8 proto, 
					void *proto_ptr, coaProcess process) {
	struct coa_proto *pos;

	if (unlikely(!coa || !process))
		return PPPOEERR_EINVAL;

	pos = proto_get(coa, proto);
	if (unlikely(pos)) {
		pppoe_log(LOG_ERR, "coa proto %u is exist\n", proto);
		return PPPOEERR_EEXIST;
	}

	pos = (struct coa_proto *)malloc(sizeof(struct coa_proto));
	if (unlikely(!pos)) {
		pppoe_log(LOG_ERR, "coa proto malloc failed\n");
		return PPPOEERR_ENOMEM;
	}
	
	memset(pos, 0, sizeof(*pos));
	pos->proto = proto;
	pos->proto_ptr = proto_ptr;
	pos->process = process;

	hlist_add_head(&pos->next, proto_hash(coa, proto));
	return PPPOEERR_SUCCESS;
}

int
coa_proto_unregister(radius_coa_t *coa, uint8 proto) {
	struct coa_proto *pos;

	if (unlikely(!coa))
		return PPPOEERR_EINVAL;

	pos = proto_get(coa, proto);
	if (unlikely(!pos)) 
		return PPPOEERR_ENOEXIST;

	hlist_del(&pos->next);
	free(pos);
	return PPPOEERR_SUCCESS;
}

static inline void
coa_proto_init(radius_coa_t *coa) {
	int i = 0;
	for (; i < COA_HASHSIZE; i++) {
		INIT_HLIST_HEAD(&coa->protoHash[i]);
	}	
}

static inline void
coa_proto_exit(radius_coa_t *coa) {
	struct coa_proto *proto;
	struct hlist_node *pos;
	int i = 0;

	for (; i < COA_HASHSIZE; i++) {
		hlist_for_each_entry(proto, pos, 
				&coa->protoHash[i], next) {
			free(proto);					
		}
		INIT_HLIST_HEAD(&coa->protoHash[i]);
	}
}


static int
coa_process(thread_struct_t *thread) {
	radius_coa_t *coa = thread_get_arg(thread);
	struct pppoe_buf *pbuf = pbuf_init(coa->pbuf);
	struct radius_packet *pack;	
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr_in);
	int ret;

	ret = coa->recvfrom(coa->sk_ptr, coa->sk, pbuf, &addr, &len);
	if (ret) {
		pppoe_log(LOG_WARNING, "recv packet failed, err %d\n", ret);
		goto out;
	}

	if (unlikely(pbuf_may_pull(pbuf, sizeof(struct radius_packet)))) {
		pppoe_log(LOG_WARNING, "pkt length(%d) error\n", pbuf->len);
		ret = PPPOEERR_ELENGTH;
		goto out;
	}

	pack = (struct radius_packet *)pbuf->data;
	if (unlikely(pbuf_trim(pbuf, ntohs(pack->length)))) {
		pppoe_log(LOG_WARNING, "radius packet length %d error\n", ntohs(pack->length));
		ret = PPPOEERR_ELENGTH;
		goto out;
	}

	pppoe_token_log(TOKEN_RADIUS, "coa recv code %u from radius %u.%u.%u.%u:%u\n",
								pack->code, HIPQUAD(ntohl(addr.sin_addr.s_addr)),
								ntohs(addr.sin_port));

	ret = coa_proto_process(coa, pack->code, pbuf, ntohl(addr.sin_addr.s_addr));
	if (ret < 0) {
		pppoe_log(LOG_WARNING, "coa proto %u process failed, err %d\n",
								pack->code, ret);
		goto out;
	}

	ret = coa->sendto(coa->sk_ptr, coa->sk, pbuf, &addr, len);
	if (ret) {
		pppoe_log(LOG_WARNING, "coa send code %u to radius %u.%u.%u.%u:%u failed, ret %d\n",
							pack->code, HIPQUAD(ntohl(addr.sin_addr.s_addr)),
							ntohs(addr.sin_port), ret);
		goto out;
	}

	pppoe_log(LOG_DEBUG, "coa send code %u to radius %u.%u.%u.%u:%u\n",
							pack->code, HIPQUAD(ntohl(addr.sin_addr.s_addr)),
							ntohs(addr.sin_port));
out:
	return ret;
}

static int
coa_socket_init(uint32 ip, uint16 *port) {
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr_in);
	char errbuf[128];	
	int sk;

	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (unlikely(sk < 0)) {
		pppoe_log(LOG_ERR, "create socket(%u.%u.%u.%u:%u) failed, %s\n",
							HIPQUAD(ip), *port, 
							strerror_r(errno, errbuf, sizeof(errbuf)));
		goto error;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(*port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif

	if (bind(sk, (struct sockaddr *)&addr, len) < 0) {
		pppoe_log(LOG_ERR, "socket %d bind %u.%u.%u.%u:%u failed, %s\n",
							sk, HIPQUAD(ip), *port, 
							strerror_r(errno, errbuf, sizeof(errbuf)));
		goto error1;
	}

	if (getsockname(sk, (struct sockaddr*)&addr, &len)) {
		pppoe_log(LOG_ERR, "socket %d getsockname failed, %s\n",
						sk, strerror_r(errno, errbuf, sizeof(errbuf)));
		goto error1;
	}

	*port = ntohs(addr.sin_port);
	pppoe_log(LOG_DEBUG, "socket %d(%u.%u.%u.%u:%u) init success\n",
							sk, HIPQUAD(ip), *port);
	return sk;

error1:
	close(sk);
error:
	return -1;
}

radius_coa_t *
radius_coa_init(thread_master_t *master, 
				void *sk_ptr, uint32 ip, uint16 *port,
				coaSendto sendto, coaRecvfrom recvfrom) {
	radius_coa_t *coa;

	if (unlikely(!master || !sendto || !recvfrom)) 
		goto error;

	coa = (radius_coa_t *)malloc(sizeof(radius_coa_t));
	if (unlikely(!coa)) {
		pppoe_log(LOG_ERR, "radius coa malloc failed\n");
		goto error;
	}

	memset(coa, 0, sizeof(*coa));
	coa->master = master;
	coa->sk_ptr = sk_ptr;
	coa->sendto = sendto;
	coa->recvfrom = recvfrom;
	
	coa->sk = coa_socket_init(ip, port);
	if (unlikely(coa->sk < 0)) {
		pppoe_log(LOG_ERR, "radius coa socket create failed\n");
		goto error1;
	}
	
	coa->pbuf = pbuf_alloc(DEFAULT_BUF_SIZE);
	if (unlikely(!coa->pbuf)) {
		pppoe_log(LOG_ERR, "radius coa mem chache create failed\n");
		goto error2;
	}

	coa->thread = thread_add_read(coa->master, coa_process, coa, coa->sk);
	if (unlikely(!coa->thread)) {
		pppoe_log(LOG_ERR, "radius coa add read thread failed\n");
		goto error3;
	}

	coa_proto_init(coa);
	pppoe_log(LOG_INFO, "radius coa init success\n");
	return coa;

error3:
	PBUF_FREE(coa->pbuf);
error2:	
	PPPOE_CLOSE(coa->sk);
error1:
	PPPOE_FREE(coa);
error:
	return NULL;
}

void
radius_coa_exit(radius_coa_t **coa) {
	if (unlikely(!coa || !*coa))
		return;

	coa_proto_exit(*coa);
	THREAD_CANCEL((*coa)->thread);
	PBUF_FREE((*coa)->pbuf);
	PPPOE_CLOSE((*coa)->sk);
	PPPOE_FREE(*coa);
	pppoe_log(LOG_INFO, "radius coa exit success\n");	
}

