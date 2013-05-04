#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "manage_log.h"
#include "manage_type.h"
#include "manage_tipc.h"
#include "manage_transport.h"

#include "manage_netlink.h"


#include "board/netlink.h"
#include "sem/product.h"




static manage_tdomain netlinkDomain;

static int
manage_netlink_send(manage_transport *t, void *buf, size_t size,
		 				void *opaque, size_t olength) {
		 				
	return 0;
}

static int
manage_netlink_recv(manage_transport *t, void *buf, size_t size,
						void **opaque, size_t *olength) {
	
	struct msghdr n_msg;	
	struct iovec n_iov;
	struct sockaddr_nl *snl;
	struct nlmsghdr *n_nlh = NULL;
	int ret;

	if(NULL == t ||  t->sock < 0) {
		manage_log(LOG_WARNING, "manage_netlink_recv: transport is %p, sock is %d\n", t, t ? t->sock : -1);
		return -1;
	}

	if(NULL == buf) {
		manage_log(LOG_WARNING, "manage_netlink_recv: buf is NULL\n");
		return -1;
	}

	if((n_nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD))) == NULL) {
		manage_log(LOG_WARNING,"Failed malloc\n");
		return -1;
	}

	
	snl =(struct sockaddr_nl *)malloc(sizeof(struct sockaddr_nl));
	
	if(NULL == snl){
		manage_log(LOG_ERR, "manage_netlink_snl: malloc snl fail\n");
		return -1;
	}

	memset(&n_msg, 0, sizeof(n_msg));
	memset(&n_iov, 0, sizeof(n_iov));	
	memset(n_nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

	n_iov.iov_base = (void *)n_nlh;
	n_iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	n_msg.msg_name = (void *)snl;
	n_msg.msg_namelen = sizeof(snl);
	n_msg.msg_iov = &n_iov;
	n_msg.msg_iovlen = 1;

	ret = recvmsg(t->sock, &n_msg, 0);
	if(ret <= 0){
			manage_log(LOG_ERR, "__recv_message: netlink recvmsg fail");
			MANAGE_FREE(snl);
			return -1;
	}			
		
	if (n_msg.msg_namelen != sizeof(struct sockaddr_nl)) {
			manage_log(LOG_ERR, "netlink recv address length error: length %d \n", n_msg.msg_namelen);
			MANAGE_FREE(snl);
			return -1;
	}
		
	if(NULL == opaque)	{
		manage_log(LOG_ERR, "netlink snl is faild\n");
		MANAGE_FREE(snl);
		return -1;
	}	
	else{
		*opaque = snl;
	  	 memcpy(buf,n_iov.iov_base,n_iov.iov_len);
		 ret = n_iov.iov_len;
	}
	return ret;
	
	}


static int
manage_netlink_close(manage_transport *t) {
	int rc = -1;
	if (t->sock >= 0) {
		rc = close(t->sock);
		t->sock = -1;
	}
	return rc;
}

static manage_transport *
manage_netlink_transport(manage_netlink_addr_group *addr,u_long flags) {

	int	rc = 0;
	manage_transport *t = NULL;
	manage_netlink_addr_group *netlink=NULL;
	struct sockaddr_nl netlink_addr;
	
	netlink=(manage_netlink_addr_group *)malloc(sizeof(manage_netlink_addr_group));
	if(NULL == netlink) {
		manage_log(LOG_ERR, "manage_netlink_transport: malloc transport fail\n");
		return NULL;
	}
	
	memset(netlink , 0 , sizeof(manage_netlink_addr_group));
	memcpy(netlink , addr,sizeof(manage_netlink_addr_group));
	memset(&netlink_addr, 0, sizeof(struct sockaddr_nl));
	
	
	netlink_addr.nl_family = AF_NETLINK;
	netlink_addr.nl_pid = netlink->src.nl_pid;
	netlink_addr.nl_groups = netlink->src.nl_groups;

	manage_log(LOG_INFO, "netlink->src.protocol = %d\n", netlink->src.protocol);
	manage_log(LOG_INFO, "netlink_addr.nl_pid = %d\n", netlink_addr.nl_pid);
	manage_log(LOG_INFO, "netlink_addr.nl_groups = %d\n", netlink_addr.nl_groups);

	t = (manage_transport *) malloc(sizeof(manage_transport));
	if(NULL == t) {
		manage_log(LOG_ERR, "manage_netlink_transport: malloc transport fail\n");
		MANAGE_FREE(netlink);
		return NULL;
	}
	memset(t, 0, sizeof(manage_transport));
	
	if(-1==(t->sock=socket(PF_NETLINK, SOCK_RAW, netlink->src.protocol))){

		manage_log(LOG_ERR, "manage_netlink_transport: socket fail\n");
		return NULL;
	}
	
	manage_log(LOG_INFO, "manage_netlink_transport: socket=:%d\n",t->sock);
//	_NETLINK_SOCkopt_set(t->sock, flags);
	rc = bind(t->sock, (struct sockaddr *)&netlink_addr, sizeof(struct sockaddr ));
	
	if(NULL != &netlink_addr)
	{
		manage_log(LOG_INFO, "manage_netlink_transport: netlink_addr is %p  \n",&netlink_addr);
	}
	if(0 != rc) {
		manage_log(LOG_ERR, "manage_netlink_transport: bind flled errno is = %d \n",errno);
		manage_netlink_close(t);
		manage_transport_free(t);
		return NULL;
	}
	manage_log(LOG_INFO, "manage_netlink_transport: bind secsess\n");
	t->flags = flags;
	t->addr = (void *)netlink;	
	/*
	 * 16-bit length field, 8 byte UDP header, 20 byte IPv4 header  
	 */
	t->msgMaxSize 	= 0xffff - 8 - 20;
	t->f_recv		= manage_netlink_recv;	
	t->f_send     	= manage_netlink_send;
	t->f_close    	= manage_netlink_close;
	t->f_accept   	= NULL;

	return t;
	
}


static manage_transport *
manage_netlink_create_chunk(const u_char *chunk, size_t chunk_size, u_long flags) {
	manage_log(LOG_INFO, "manage_netlink_create_chunk is called\n");
	if(chunk && sizeof(manage_netlink_addr_group) == chunk_size) {
		manage_netlink_addr_group *addr = (manage_netlink_addr_group *)chunk;//
		return manage_netlink_transport(addr, flags);
	}	
	return NULL;
}


void
manage_netlink_ctor(void) {
	netlinkDomain.name = MANAGE_NETLINK_DOMAIN;	
	
	netlinkDomain.f_create_from_chunk = manage_netlink_create_chunk;
	manage_tdomain_register(&netlinkDomain);	
	
	return ;
}



