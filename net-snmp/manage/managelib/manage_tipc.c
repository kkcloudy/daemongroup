

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/tipc.h>

#include "manage_log.h"
#include "manage_type.h"
#include "manage_transport.h"

#include "manage_tipc.h"

#ifndef _MANAGE_TIPC_RELAY_
#define _MANAGE_TIPC_RELAY_
#endif

manage_tipc_addr tipc_relayaddr = { MANAGE_TIPC_TYPE, 0x1 };

static manage_tdomain tipcDomain;

static void
_tipc_addr_set(struct sockaddr_tipc *sockaddr, manage_tipc_addr *tipc_addr) {
	memset(sockaddr, 0, sizeof(struct sockaddr_tipc));
	sockaddr->family = AF_TIPC;
	sockaddr->addrtype = TIPC_ADDR_NAME;
	sockaddr->addr.name.name.type = tipc_addr->type;
	sockaddr->addr.name.name.instance = tipc_addr->instance;  
	sockaddr->addr.name.domain = 0;
	return ;
}

static void
_tipc_sockopt_set(int fd, int local) {

	return ;	
}

static int
_tipc_recv(manage_transport *t, 
				void *buf, size_t size,
		 		struct sockaddr_tipc *from, size_t *fromlen) {
		 
	int rc = -1;
#if 0
	if( NULL == from ){
		return rc;
	}
#endif	

	while(rc < 0) {
		rc = recvfrom(t->sock, buf, size, 0, (struct sockaddr *)from, (socklen_t *)fromlen);
		if(rc < 0 && errno != EINTR) {
			manage_token_log(LOG_TOKEN_TIPC, "_tipc_recv: recv error, rc %d (errno %d)\n", rc, errno);
			break;
		}
		
		if(from && fromlen) {
			manage_token_log(LOG_TOKEN_TIPC, "_tipc_recv: recv from (0x%x:0x%x) %d byte\n", 
								from->addr.name.name.type, from->addr.name.name.instance, rc);
		}
	}

	return rc;
}

static int
_tipc_send(manage_transport *t, 
				void *buf, size_t size,
		 		struct sockaddr_tipc *to, size_t tolen) {
	int rc = -1;
		
	while(rc < 0) {
		rc = sendto(t->sock, buf, size, 0, (struct sockaddr *)to, tolen);
		if (rc < 0 && errno != EINTR) {
			manage_token_log(LOG_TOKEN_TIPC, "_tipc_send: sendto (0x%x:0x%x) error, rc %d (errno %d)\n", 
							to->addr.name.name.type, to->addr.name.name.instance, rc, errno);
			break;
		}
		manage_token_log(LOG_TOKEN_TIPC, "_tipc_send: sendto (0x%x:0x%x) %d byte\n", 
							to->addr.name.name.type, to->addr.name.name.instance, rc);
	}
	
	return rc;
}

static int
_tipc_proxy(manage_transport *transport, void *data, size_t len) {
	int rc = -1;
	struct sockaddr_tipc destaddr;

	if(transport->flags & MANAGE_FLAGS_RELAY_SOCKET) {
		_tipc_addr_set(&destaddr, (manage_tipc_addr *)data);
		rc = _tipc_send(transport, data, len, &destaddr, sizeof(struct sockaddr));
	} else {
		manage_token_log(LOG_TOKEN_TIPC, "_tipc_proxy, receiver is not tipc relay\n");
		return -1;
	}

	manage_token_log(LOG_TOKEN_TIPC, "_tipc_proxy, rc = %d\n", rc);
	return rc;
}

static int
_tipc_matchaddr(manage_transport *transport, 
								void	*opaque, size_t olength,
								void *data, int *len) {
									
	if(olength == sizeof(manage_tipc_addr_group)) {
		if(*len > sizeof(manage_tipc_addr_group)) {
			if(memcmp(transport->addr, data, sizeof(manage_tipc_addr))
				&& memcmp(&tipc_relayaddr, data, sizeof(manage_tipc_addr))) { /*compare tipc pkt dest addr*/
				manage_token_log(LOG_TOKEN_TIPC, "_tipc_matchaddr: this tipc server(0x%x:0x%x) is not dest addr(0x%x:0x%x) which sour addr(0x%x:0x%x)\n",
										((manage_tipc_addr *)transport->addr)->type, ((manage_tipc_addr *)transport->addr)->instance, 
										((manage_tipc_addr_group *)data)->dest.type, ((manage_tipc_addr_group *)data)->dest.instance,
										((manage_tipc_addr_group *)data)->sour.type, ((manage_tipc_addr_group *)data)->sour.instance);
				if(_tipc_proxy(transport, data, *len) <  0) {
					goto BAD_ADDR;
				}
				
				goto PACKET_PROXY;
			} else {
				if(opaque) {
					memcpy(opaque, data + sizeof(manage_tipc_addr), sizeof(manage_tipc_addr));
					memcpy(opaque + sizeof(manage_tipc_addr), data, sizeof(manage_tipc_addr));
				}
				
				*len -= sizeof(manage_tipc_addr_group);
				memmove(data, data + sizeof(manage_tipc_addr_group), *len);
			}
		}else {
			goto BAD_ADDR;
		}
	} else if(opaque && olength == sizeof(struct sockaddr_tipc)){
		struct sockaddr_tipc *from =  (struct sockaddr_tipc *)opaque;
		manage_token_log(LOG_TOKEN_TIPC, "_tipc_matchaddr: this tipc server(0x%x:0x%x) is not dest addr(0x%x:0x%x)\n",
								((struct sockaddr_tipc *)(transport->addr))->addr.name.name.type, 
								((struct sockaddr_tipc *)(transport->addr))->addr.name.name.instance,
								from->addr.name.name.type, from->addr.name.name.instance);
	} else{
		goto BAD_ADDR;
	}

	manage_token_log(LOG_TOKEN_TIPC, "_tipc_matchaddr: success\n");
	return MANAGEERR_SUCCESS;


PACKET_PROXY:
	*len = -1;
	manage_token_log(LOG_TOKEN_TIPC, "_tipc_matchaddr: packet proxy\n");
	return MANAGEERR_PACKET_PROXY;

	
BAD_ADDR:
	*len = -1;
	manage_token_log(LOG_TOKEN_TIPC, "_tipc_matchaddr: packet bad addr\n");
	return MANAGEERR_BAD_INPUT;
}


static int
manage_tipc_recv(manage_transport *t, void *buf, size_t size,
						void **opaque, size_t *olength) {
	int rc = -1;
	char errbuf[128];

	if(NULL == t ||  t->sock < 0) {
		manage_log(LOG_WARNING, "manage_tipc_recv: transport is %p, sock is %d\n", t, t ? t->sock : -1);
		goto BAD_RECEIVE;
	}

	if(NULL == buf) {
		manage_log(LOG_WARNING, "manage_tipc_recv: buf is NULL\n");
		goto BAD_RECEIVE;
	};

	
	if(opaque && olength) {
	#ifdef _MANAGE_TIPC_RELAY_
		manage_tipc_addr_group *addr_group = (manage_tipc_addr_group *) calloc(1, sizeof(manage_tipc_addr_group));
		if(NULL == addr_group) {
			manage_log(LOG_ERR, "manage_tipc_recv:malloc addr_group fail\n");
			goto BAD_RECEIVE;
		}
		
		rc = _tipc_recv(t, buf, size, NULL, NULL);
		if(rc >= 0) {
			int match_ret = _tipc_matchaddr(t, addr_group, sizeof(manage_tipc_addr_group), buf, &rc);
			if(MANAGEERR_SUCCESS == match_ret) {
				manage_token_log(LOG_TOKEN_TIPC, "manage_tipc_recv: recvfrom fd %d got %d bytes from (0x%x:0x%x) to (0x%x:0x%x)\n", t->sock, rc, 
											addr_group->sour.type, addr_group->sour.instance, addr_group->dest.type, addr_group->dest.instance);
			}	
		} else {
			manage_log(LOG_WARNING, "manage_tipc_recv: recvfrom fd %d err %d (\"%s\")\n", 
									t->sock, errno, strerror_r(errno, errbuf, sizeof(errbuf)));
		}
		
		*opaque = (void *)addr_group;
		*olength = sizeof(manage_tipc_addr_group);
		
	#else
		size_t from_len = sizeof(struct sockaddr);
		struct sockaddr_tipc *from = (struct sockaddr_tipc *)calloc(1, sizeof(struct sockaddr_tipc));
		if(NULL == from) {
			manage_log(LOG_ERR, "manage_tipc_recv:malloc from fail\n");
			goto BAD_RECEIVE;
		}

		rc = _tipc_recv(t, buf, size, from, &from_len);
		if(rc >= 0){
			_tipc_matchaddr(t, from, sizeof(struct sockaddr_tipc), NULL, NULL);
		} else {
			manage_log(LOG_WARNING, "manage_tipc_recv: recvfrom fd %d err %d (\"%s\")\n", 
									t->sock, errno, strerror_r(errno, errbuf, sizeof(errbuf)));
		}

		*opaque = (void *)from;
		*olength = sizeof(struct sockaddr_tipc);
	#endif
	}else if(!opaque && !olength) {
	#ifdef _MANAGE_TIPC_RELAY_	
		rc = _tipc_recv(t, buf, size, NULL, NULL);
		if(rc >= 0) {
			_tipc_matchaddr(t, NULL, sizeof(manage_tipc_addr_group), buf, &rc);
		} else {
			manage_log(LOG_WARNING, "manage_tipc_recv: recvfrom fd %d err %d (\"%s\")\n", 
									t->sock, errno, strerror_r(errno, errbuf, sizeof(errbuf)));
		}		
	#else
		rc = _tipc_recv(t, buf, size, NULL, NULL);
		if(rc < 0){
			manage_log(LOG_WARNING, "manage_tipc_recv: recvfrom fd %d err %d (\"%s\")\n", 
									t->sock, errno, strerror_r(errno, errbuf, sizeof(errbuf)));
		}
	#endif
	} else {
		manage_log(LOG_WARNING, "manage_tipc_recv: opaque is %p, olength is %p\n", opaque, olength);
		return -1;
	}

	
	return rc;

BAD_RECEIVE:
	if(opaque) {
		*opaque = NULL;
	}
	if(olength) {
		*olength = 0;
	}
	return -1;
}

static int
manage_tipc_send(manage_transport *t, void *buf, size_t size,
		 				void *opaque, size_t olength) {
	int rc = -1;
	int tolen = sizeof(struct sockaddr);
	struct sockaddr_tipc to;

	if(NULL == t || t->sock < 0) {	
		manage_log(LOG_WARNING, "manage_tipc_send: transport is %p, sock is %d\n", t, t ? t->sock : -1);
		return -1;
	}

	if(NULL == buf || NULL == opaque) {
		manage_log(LOG_WARNING, "manage_tipc_send: buf is %p, opaque is %p\n", buf, opaque);
		return -1;
	}

	if(olength == sizeof(manage_tipc_addr_group)) {
		manage_tipc_addr_group *addr_group = (manage_tipc_addr_group *)opaque;
		int data_size = size + sizeof(manage_tipc_addr_group);

		void *data = (void *)calloc(1, data_size);
		if(NULL == data) {
			manage_log(LOG_ERR, "manage_tipc_send: MALLOC data fail\n");
			return MANAGEERR_MALLOC_FAIL;
		}
		
		memcpy(data, addr_group, sizeof(manage_tipc_addr_group));
		memcpy(data + sizeof(manage_tipc_addr_group), buf, size);		

		if(t->flags & MANAGE_FLAGS_RELAY_SOCKET) {
			_tipc_addr_set(&to, &(addr_group->dest));
		} else {
			_tipc_addr_set(&to, &tipc_relayaddr);
		}
		
		rc = _tipc_send(t, data, data_size, &to, tolen);
		if(rc < sizeof(manage_tipc_addr_group)) {
			manage_log(LOG_WARNING, "manage_tipc_send: _tipc_send send fail, rc = %d\n", rc);
			MANAGE_FREE(data);
			return -1;
		}
		rc -= sizeof(manage_tipc_addr_group);
		MANAGE_FREE(data);
	} else if(olength == sizeof(struct sockaddr_tipc)) {
		rc = _tipc_send(t, buf, size, opaque, tolen);
	} else{
		manage_log(LOG_WARNING, "manage_tipc_send: input olength is unknow\n");
		return -1;
	}
	
	return rc;
}


static int
manage_tipc_close(manage_transport *t) {
	int rc = -1;
	if (t->sock >= 0) {
		rc = close(t->sock);
		t->sock = -1;
	}
	return rc;
}


static manage_transport *
manage_tipc_transport(manage_tipc_addr *addr, u_long flags) {

	int	rc = 0;
	manage_transport *t = NULL;
	manage_tipc_addr *tipc = NULL;
	struct sockaddr_tipc tipc_addr;

	tipc = (manage_tipc_addr *)malloc(sizeof(manage_tipc_addr));
	if(NULL == tipc) {
		manage_log(LOG_ERR, "manage_tipc_transport: malloc transport fail\n");
		return NULL;
	}
	memcpy(tipc, addr, sizeof(manage_tipc_addr));
	
	memset(&tipc_addr, 0, sizeof(struct sockaddr_tipc));
	tipc_addr.family	= AF_TIPC;
	tipc_addr.addrtype	= TIPC_ADDR_NAMESEQ;
	tipc_addr.scope	= TIPC_CLUSTER_SCOPE;
	
	if(flags & MANAGE_FLAGS_RELAY_SOCKET) {
		tipc_addr.addr.nameseq.type	= tipc_relayaddr.type;
		tipc_addr.addr.nameseq.lower 	= tipc_relayaddr.instance;
		tipc_addr.addr.nameseq.upper 	= tipc_relayaddr.instance;
	} else {
		tipc_addr.addr.nameseq.type	= tipc->type;
		tipc_addr.addr.nameseq.lower 	= tipc->instance;
		tipc_addr.addr.nameseq.upper 	= tipc->instance;	
	}
	
	t = (manage_transport *) malloc(sizeof(manage_transport));
	if(NULL == t) {
		manage_log(LOG_ERR, "manage_tipc_transport: malloc transport fail\n");
		MANAGE_FREE(tipc);
		return NULL;
	}
	memset(t, 0, sizeof(manage_transport));
	
	manage_token_log(LOG_TOKEN_TIPC, "manage_tipc_transport: open 0x%x:0x%x-0x%x\n", tipc_addr.addr.nameseq.type, 
										tipc_addr.addr.nameseq.lower, tipc_addr.addr.nameseq.upper);
		
	t->sock = socket(AF_TIPC, SOCK_DGRAM, 0);
	if(t->sock < 0) {
		MANAGE_FREE(tipc);
		manage_transport_free(t);
		return NULL;
	}

	_tipc_sockopt_set(t->sock, flags);

	rc = bind(t->sock, (struct sockaddr *)&tipc_addr, sizeof(struct sockaddr));
	if(0 != rc) {
		MANAGE_FREE(tipc);
		manage_tipc_close(t);
		manage_transport_free(t);
		return NULL;
	}

	t->flags = flags;
	t->addr = (void *)tipc;	
	/*
	 * 16-bit length field, 8 byte UDP header, 20 byte IPv4 header  
	 */
	t->msgMaxSize 	= 0xffff - 8 - 20;
	t->f_recv		= manage_tipc_recv;	
	t->f_send     		= manage_tipc_send;
	t->f_close    		= manage_tipc_close;
	t->f_accept   	= NULL;

	return t;
}

static manage_transport *
manage_tipc_create_chunk(const u_char *chunk, size_t chunk_size, u_long flags) {
	if(chunk && sizeof(manage_tipc_addr) == chunk_size) {
		manage_tipc_addr *addr = (manage_tipc_addr *)chunk;
		return manage_tipc_transport(addr, flags);
	}	
	return NULL;
}

void
manage_tipc_ctor(void) {
	tipcDomain.name = MANAGE_TIPC_DOMAIN;
	
	tipcDomain.f_create_from_chunk = manage_tipc_create_chunk;

	manage_tdomain_register(&tipcDomain);
	
	return ;
}

