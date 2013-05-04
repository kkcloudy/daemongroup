
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include <linux/sockios.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "kernel/if_pppoe.h"

#include "pppoe_log.h"
#include "pppoe_buf.h"
#include "pppoe_util.h"
#include "pppoe_netlink.h"

static int 
__send_message(int sk, struct pppoe_buf *pbuf) {
	struct sockaddr_nl daddr;
	struct nlmsghdr *nlhdr;
	struct msghdr msg;
	struct iovec iov;
	char errbuf[128];
	ssize_t length;

	if (pbuf_headroom(pbuf) < NLMSG_HDRLEN) {
		pppoe_log(LOG_WARNING, "pbuf headroom lack space\n");
		return PPPOEERR_ENOMEM;
	}
	
	/*set nlhdr*/
	nlhdr = (struct nlmsghdr *)pbuf_push(pbuf, NLMSG_HDRLEN);
	memset(nlhdr, 0, NLMSG_HDRLEN);	
	nlhdr->nlmsg_len = pbuf->len;
	nlhdr->nlmsg_pid = getpid();  
	nlhdr->nlmsg_flags = 0;

	/*set daddr*/
	memset(&daddr, 0, sizeof(daddr));
	daddr.nl_family = AF_NETLINK;
	daddr.nl_pid = 0;
	daddr.nl_groups = 0;

	/*set message*/
	memset(&iov, 0, sizeof(iov));
	memset(&msg, 0, sizeof(msg));
	iov.iov_base = (void *)nlhdr;
	iov.iov_len = nlhdr->nlmsg_len;
	msg.msg_name = (void *)&daddr;
	msg.msg_namelen = sizeof(daddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/*send message*/
	do  {
		length = sendmsg(sk, &msg, 0);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "netlink %d sendmsg failed, %s\n", sk,
								strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ESEND;		
		}
	} while (length < 0);

	return PPPOEERR_SUCCESS;
}

static int 
__recv_message(int sk, struct pppoe_buf *pbuf) {
	struct sockaddr_nl snl;	
	struct iovec iov = { pbuf->data, pbuf->end - pbuf->data };
	struct msghdr msg = { (void *)&snl, sizeof(struct sockaddr_nl), &iov, 1, NULL, 0, 0 };
	struct nlmsghdr *nlhdr;
	char errbuf[128];
	ssize_t length;

	do  {
		length = recvmsg(sk, &msg, 0);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;
			
			pppoe_log(LOG_WARNING, "netlink %d recvmsg failed, %s\n", sk,
									strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ERECV;
		}	
	} while (length < 0);
	
	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		pppoe_log(LOG_WARNING, "netlink recv address length error: length %d\n", msg.msg_namelen);
		return PPPOEERR_ELENGTH;
	}

	if (snl.nl_pid) {
		pppoe_log(LOG_WARNING, "netlink is not from kernel, pid is %u\n", snl.nl_pid);
		return PPPOEERR_EADDR;
	}

	if (length < NLMSG_HDRLEN) {
		pppoe_log(LOG_WARNING, "netlink message len is error: "
							"length %d < NLMSG_HDRLEN\n", length);
		return PPPOEERR_ELENGTH;
	}

	nlhdr = (struct nlmsghdr *)pbuf->data;
	if (nlhdr->nlmsg_len > length || nlhdr->nlmsg_len < NLMSG_LENGTH(sizeof(struct pppoe_message))) {
		pppoe_log(LOG_WARNING, "netlink message len is error: "
							"nlmsg_len %d\n", nlhdr->nlmsg_len);
		return PPPOEERR_ELENGTH;
	}

	pbuf->data += NLMSG_HDRLEN;
	pbuf->len = nlhdr->nlmsg_len - NLMSG_HDRLEN;
	pbuf->tail = pbuf->data + pbuf->len;
	return PPPOEERR_SUCCESS;
}

static int 
__recv_message_wait(int sk, struct pppoe_buf *pbuf, struct timeval *tvp) {
	fd_set readfds;
	int count;	

	FD_ZERO(&readfds);
	FD_SET(sk, &readfds);

reselect:
	count = select(sk + 1, &readfds, NULL, NULL, tvp);
	pppoe_token_log(TOKEN_NETLINK, "netlink %d select, count = %d", sk, count);
	if (count > 0) {
		if (!FD_ISSET(sk, &readfds)) {
			pppoe_token_log(TOKEN_NETLINK, "netlink %d is not set in readfds\n", sk);
			goto reselect;
		}	
	} else switch(count) {
	case 0:
		pppoe_token_log(TOKEN_NETLINK, "netlink return select EOF\n");
		return PPPOEERR_ETIMEOUT;	
		
	case -1:
		pppoe_token_log(TOKEN_NETLINK, "manage select, errno = %d\n", errno);
		if (EINTR == errno) {
			goto reselect;
		} 
		pppoe_log(LOG_WARNING, "netlink %d return select failed\n", sk);
		return PPPOEERR_ESYSCALL;	
			
	default:
		pppoe_log(LOG_WARNING, "netlink %d return select return %d\n", sk, count);
		return PPPOEERR_EINVAL;	
	}

	return  __recv_message(sk, pbuf);
}


static inline int
__recv_reply(int sk, struct pppoe_buf *pbuf, 
				uint32 checkcode, struct timeval *tvp) {
	struct pppoe_message *reply;	
	int ret;	

rerecv:	
	ret = __recv_message_wait(sk, pbuf, tvp);
	if (ret) 
		goto out;

	reply = (struct pppoe_message *)pbuf->data;
	if (PPPOE_MESSAGE_REPLY != reply->type ||
		reply->checkcode != checkcode)
		goto rerecv;

	switch (reply->errorcode) {
	case 0:
		ret = PPPOEERR_SUCCESS;
		break;

	case -EINVAL:
		ret = PPPOEERR_EINVAL;
		break;

	case -EEXIST:
		ret = PPPOEERR_EEXIST;
		break;

	case -ENODEV:
		ret = PPPOEERR_ENOEXIST;
		break;

	case -ENOMEM:
		ret = PPPOEERR_ENOMEM;
		break;

	default:
		ret = PPPOEERR_ESYSCALL;
		break;
	}

	pppoe_token_log(TOKEN_NETLINK, "message reply errorcode is %d\n", reply->errorcode);		
	
out:
	pppoe_token_log(TOKEN_NETLINK, "message recv, ret %d\n", ret);		
	return ret;	
}

static inline int
_send_with_reply(int sk, struct pppoe_buf *pbuf, 
						uint32 checkcode, int msec) {
	struct timeval wait_time, *tvp;
	int ret;

	ret = __send_message(sk, pbuf);
	if (ret) {
		pppoe_log(LOG_WARNING, "send netlink message failed, ret %d\n", ret);
		return ret;
	}

	if (msec < 0) {
		tvp = &wait_time;
		tvp->tv_sec = 3;
		tvp->tv_usec = 0;
	} else if(msec > 0) {
		tvp = &wait_time;
		tvp->tv_sec = msec / 1000;;
		tvp->tv_usec = (msec - (1000 * tvp->tv_sec)) * 1000;
	} else {
		tvp = NULL;
	}

	return __recv_reply(sk, pbuf_init(pbuf), checkcode, tvp);
}



int 
netlink_init(void) {
	int sk;
	struct sockaddr_nl saddr;

	sk = socket(AF_NETLINK, SOCK_RAW, NETLINK_PPPOE);
	if (unlikely(sk < 0)) {
		pppoe_log(LOG_ERR, "creat netlink socket failed\n");
		goto error;
	}

	set_nonblocking(sk);
	
	memset(&saddr, 0, sizeof(struct sockaddr_nl));
	saddr.nl_family	= AF_NETLINK;
	saddr.nl_pid	= getpid();
	saddr.nl_groups	= 0;

	if (bind(sk, (struct sockaddr*)&saddr, sizeof(saddr))) {
		pppoe_log(LOG_ERR, "netlink bind sockaddr failed\n");
		goto error1;
	}

	return sk;

error1:
	close(sk);
error:
	return -1;
}

int
netlink_recv_message(int sk, struct pppoe_buf *pbuf) {
	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	return __recv_message(sk, pbuf);
}

int
netlink_recv_message_wait(int sk, struct pppoe_buf *pbuf, uint32 msec) {
	struct timeval tvp;
	
	if (unlikely(sk <= 0 || !pbuf))
		return PPPOEERR_EINVAL;

	tvp.tv_sec = msec / 1000;
	tvp.tv_usec = (msec - (1000 * tvp.tv_sec)) * 1000;
	return __recv_message_wait(sk, pbuf, &tvp);
}


int
netlink_register(int sk, struct pppoe_buf *pbuf, 
				uint32 local_id, uint32 instance_id) {
	struct pppoe_message *message;
	struct timeval tv;

	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;

	gettimeofday(&tv, NULL);
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_NETLINK_REGISTER;
	message->checkcode = tv.tv_usec;
	message->data.m_deamon.local_id = local_id;
	message->data.m_deamon.instance_id = instance_id;
	message->datalen = sizeof(struct pppoe_deamon_msg);

	return _send_with_reply(sk, pbuf, tv.tv_usec, -1 /*default timeout*/);
}

int
netlink_unregister(int sk, struct pppoe_buf *pbuf, 
					uint32 local_id, uint32 instance_id) {
	struct pppoe_message *message;
	struct timeval tv;

	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;

	gettimeofday(&tv, NULL);
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_NETLINK_UNREGISTER;
	message->checkcode = tv.tv_usec;	
	message->data.m_deamon.local_id = local_id;
	message->data.m_deamon.instance_id = instance_id;
	message->datalen = sizeof(struct pppoe_deamon_msg);
	
	return _send_with_reply(sk, pbuf, tv.tv_usec, -1 /*default timeout*/);
}

int
netlink_create_interface(int sk, struct pppoe_buf *pbuf, char *ifname) {
	struct pppoe_message *message;
	struct timeval tv;
	int ret;
	
	if (unlikely(sk <= 0 || !pbuf || !ifname)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;

	gettimeofday(&tv, NULL);		
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_INTERFACE_CREATE;
	message->checkcode = tv.tv_usec;	
	strncpy(message->data.m_interface.name, ifname, IFNAMSIZ - 1);
	message->datalen = sizeof(struct pppoe_interface_msg);

	ret = _send_with_reply(sk, pbuf, tv.tv_usec, -1 /*default timeout*/);
	if (!ret) {
		ret = ((struct pppoe_message *)(pbuf->data))->data.m_interface.ifindex;
	}

	return ret;
}

int
netlink_destroy_interface(int sk, struct pppoe_buf *pbuf, int ifindex) {
	struct pppoe_message *message;
	struct timeval tv;

	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;

	gettimeofday(&tv, NULL);				
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_INTERFACE_DESTROY;
	message->checkcode = tv.tv_usec;	
	message->data.m_interface.ifindex = ifindex;
	message->datalen = sizeof(struct pppoe_interface_msg);

	return _send_with_reply(sk, pbuf, tv.tv_usec, -1 /*default timeout*/);
}

int
netlink_base_interface(int sk, struct pppoe_buf *pbuf, int ifindex, char *ifname) {
	struct pppoe_message *message;
	struct timeval tv;
	
	if (unlikely(sk <= 0 || !pbuf || !ifname)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;

	gettimeofday(&tv, NULL);		
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_INTERFACE_BASE;
	message->checkcode = tv.tv_usec;	
	message->data.m_interface.ifindex = ifindex;
	strncpy(message->data.m_interface.name, ifname, IFNAMSIZ - 1);
	message->datalen = sizeof(struct pppoe_interface_msg);

	return _send_with_reply(sk, pbuf, tv.tv_usec, -1 /*default timeout*/);
}

int
netlink_unbase_interface(int sk, struct pppoe_buf *pbuf, int ifindex) {
	struct pppoe_message *message;
	struct timeval tv;
	
	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;

	gettimeofday(&tv, NULL);		
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_INTERFACE_UNBASE;
	message->checkcode = tv.tv_usec;	
	message->data.m_interface.ifindex = ifindex;
	message->datalen = sizeof(struct pppoe_interface_msg);

	return _send_with_reply(sk, pbuf, tv.tv_usec, -1 /*default timeout*/);
}

int
netlink_channel_register(int sk, struct pppoe_buf *pbuf, 
			int ifindex, uint32 sid, uint8 *mac, uint8 *serverMac, uint8 *magic) {
	struct pppoe_message *message;
	
	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;
		
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_CHANNEL_REGISTER;
	message->data.m_register.ifindex = ifindex;
	message->data.m_register.sid = htons(sid);
	memcpy(message->data.m_register.magic, magic, MAGIC_LEN);
	memcpy(message->data.m_register.mac, mac, ETH_ALEN);
	memcpy(message->data.m_register.serverMac, serverMac, ETH_ALEN);
	message->datalen = sizeof(struct pppoe_register_msg);

	return __send_message(sk, pbuf);
}

int
netlink_channel_unregister(int sk, struct pppoe_buf *pbuf, 
						int ifindex, uint32 sid, uint8 *mac) {
	struct pppoe_message *message;
	
	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;
		
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_CHANNEL_UNREGISTER;
	message->data.m_register.ifindex = ifindex;
	message->data.m_register.sid = htons(sid);
	memcpy(message->data.m_register.mac, mac, ETH_ALEN);
	message->datalen = sizeof(struct pppoe_register_msg);

	return __send_message(sk, pbuf);
}

int
netlink_channel_authorize(int sk, struct pppoe_buf *pbuf,
						int ifindex, uint32 sid, uint32 ip) {
	struct pppoe_message *message;
	
	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;
		
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_CHANNEL_AUTHORIZE;
	message->data.m_authorize.ifindex = ifindex;
	message->data.m_authorize.sid = htons(sid);
	message->data.m_authorize.ip = htonl(ip);
	message->datalen = sizeof(struct pppoe_authorize_msg);

	return __send_message(sk, pbuf);
}

int
netlink_channel_unauthorize(int sk, struct pppoe_buf *pbuf, 
						int ifindex, uint32 sid, uint32 ip) {
	struct pppoe_message *message;
	
	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;
		
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_CHANNEL_UNAUTHORIZE;
	message->data.m_authorize.ifindex = ifindex;
	message->data.m_authorize.sid = htons(sid);
	message->data.m_authorize.ip = htonl(ip);
	message->datalen = sizeof(struct pppoe_authorize_msg);

	return __send_message(sk, pbuf);
}

int 
netlink_channel_clear(int sk, struct pppoe_buf *pbuf, int ifindex) {
	struct pppoe_message *message;

	if (unlikely(sk <= 0 || !pbuf)) 
		return PPPOEERR_EINVAL;

	pbuf_reserve(pbuf_init(pbuf), NLMSG_HDRLEN);
	if (pbuf_tailroom(pbuf) < sizeof(struct pppoe_message))
		return PPPOEERR_ENOMEM;
	
	message = (struct pppoe_message *)pbuf_put(pbuf, sizeof(struct pppoe_message));
	memset(message, 0, sizeof(struct pppoe_message));
	message->type = PPPOE_MESSAGE_REQUEST;
	message->code = PPPOE_CHANNEL_CLEAR;
	message->data.m_interface.ifindex = ifindex;
	message->datalen = sizeof(struct pppoe_interface_msg);	

	return __send_message(sk, pbuf);
}

