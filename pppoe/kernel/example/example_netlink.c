#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/if_ether.h>


#include "if_pppoe.h"
		
static int netlink_sk;


static int 
init_netlink(void){
	struct sockaddr_nl saddr;

	netlink_sk = socket(AF_NETLINK, SOCK_RAW, NETLINK_PPPOE);
	if(0 == netlink_sk) {
		printf("creat netlink socket error\n");
		return -1;
	}

	saddr.nl_family	= AF_NETLINK;
	saddr.nl_pid		= getpid();
	saddr.nl_groups	= 0;

	if(0 != bind(netlink_sk,(struct sockaddr*)&saddr, sizeof(saddr))) {
		printf("bind saddr error\n");
		return -1;
	}

	return 0;
}

static void
exit_netlink(void) {
	if(netlink_sk > 0) {
		close(netlink_sk);
	}
}

static int 
send_message(void* data, int len) {
	struct nlmsghdr* 	nlhdr = NULL;
	struct msghdr 	msg;
	struct iovec		iov;
	struct sockaddr_nl	daddr;
	int 				ret;

	nlhdr = (struct nlmsghdr*)malloc(NLMSG_SPACE(len));
	if(NULL == nlhdr) {
		printf("send_message malloc error\n");
		return -1;
	}

	memcpy(NLMSG_DATA(nlhdr), data, len);
	memset(&msg, 0 ,sizeof(struct msghdr));
	
//	show_netlink_transport_info(NLMSG_DATA(nlhdr));

	/*set nlhdr*/
	nlhdr->nlmsg_len = NLMSG_LENGTH(len);
	nlhdr->nlmsg_pid = getpid();  
	nlhdr->nlmsg_flags = 0;

	/*set daddr*/
	
	daddr.nl_family = AF_NETLINK;
	daddr.nl_pid = 0;
	daddr.nl_groups = 0;

	/*set message*/
	iov.iov_base = (void *)nlhdr;
	iov.iov_len = nlhdr->nlmsg_len;
	msg.msg_name = (void *)&daddr;
	msg.msg_namelen = sizeof(daddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/*send message*/
	ret = sendmsg(netlink_sk, &msg, 0);

	printf("sendmsg return is %d\n",ret);
	
	if(-1 == ret) {
		printf("sendmsg error:");
		return -1;
	}
	
	return 0;
}


static int 
recv_errorcode(void *data, int len) {
	int ret;
	struct iovec iov = { data, len };
	struct sockaddr_nl snl;
	struct msghdr msg = { (void *) &snl, sizeof(struct sockaddr_nl), &iov, 1, NULL, 0, 0 };
	
	struct nlmsghdr *nlhdr;
	struct pppoe_message *message;
	
	ret = recvmsg(netlink_sk, &msg, 0);
	if (ret <= 0) {
		printf( "netlink recv fail \n");
		return -1;
	}
	
	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		printf("netlink sender address length error: length %d \n", msg.msg_namelen);
		return -1;
	}

	if (snl.nl_pid) {
		printf("pppoe netlink is not kernel, is %d\n", snl.nl_pid);
		return -1;
	}

	nlhdr = (struct nlmsghdr *)data;
	if (nlhdr->nlmsg_len < NLMSG_LENGTH(sizeof(struct pppoe_message))) {
		printf("pppoe netlink message len is error, is %d\n", nlhdr->nlmsg_len);
		return -1;
	}
	
	return 0;
}


int
main(int argc, char **argv) {
	struct pppoe_message message, *reply;
	unsigned int message_type;
	int ret, numfds, count;
	fd_set readfds;
	char buf[1024];
	struct timeval tvp = {
		.tv_sec = 3,
		.tv_usec = 0
	};
	
	if(argc < 2) {
		printf("input para is erro!\n");
		return -1;
	}

	if(init_netlink()) {
		printf("init netlink error\n");
		return -1;
	}

	message_type = atoi(argv[1]);
	switch (message_type) {
	case PPPOE_INTERFACE_CREATE:
		{				
			printf("enter PPPOE_INTERFACE_CREATE\n");
			if(3 != argc) {
				printf("input para is erro!\n");
				ret = -1;
				goto out;
			}

			memset(&message, 0, sizeof(struct pppoe_message));
			
			message.type = PPPOE_MESSAGE_REQUEST;
			message.code = PPPOE_INTERFACE_CREATE;
			strncpy(message.data.m_interface.name, argv[2], sizeof(message.data.m_interface.name) - 1);
			message.datalen = sizeof(message.data);

			if(send_message(&message, sizeof(struct pppoe_message))) {
				printf("send PPPOE_CHANNEL_REGISTER message fail\n");
				ret = -1;
				goto out;
			}
		}
		break;
		
	case PPPOE_INTERFACE_DESTROY:
		{				
			printf("enter PPPOE_INTERFACE_DESTROY\n");
			if(3 != argc) {
				printf("input para is erro!\n");
				ret = -1;
				goto out;
			}

			memset(&message, 0, sizeof(struct pppoe_message));
			
			message.type = PPPOE_MESSAGE_REQUEST;
			message.code = PPPOE_INTERFACE_DESTROY;
			message.data.m_interface.ifindex = atoi(argv[2]);
			message.datalen = sizeof(message.data);

			if(send_message(&message, sizeof(struct pppoe_message))) {
				printf("send PPPOE_CHANNEL_REGISTER message fail\n");
				ret = -1;
				goto out;
			}
		}
		break;
		
	case PPPOE_INTERFACE_BASE:
		{				
			printf("enter PPPOE_INTERFACE_BASE\n");
			if(4 != argc) {
				printf("input para is erro!\n");
				ret = -1;
				goto out;
			}

			memset(&message, 0, sizeof(struct pppoe_message));
			
			message.type = PPPOE_MESSAGE_REQUEST;
			message.code = PPPOE_INTERFACE_BASE;
			message.data.m_interface.ifindex = atoi(argv[2]);
			strncpy(message.data.m_interface.name, argv[3], sizeof(message.data.m_interface.name) - 1);
			message.datalen = sizeof(message.data);

			if(send_message(&message, sizeof(struct pppoe_message))) {
				printf("send PPPOE_CHANNEL_REGISTER message fail\n");
				ret = -1;
				goto out;
			}
		}
		break;

	case PPPOE_INTERFACE_UNBASE:
		{				
			printf("enter PPPOE_INTERFACE_UNBASE\n");
			if(4 != argc) {
				printf("input para is erro!\n");
				ret = -1;
				goto out;
			}

			memset(&message, 0, sizeof(struct pppoe_message));
			
			message.type = PPPOE_MESSAGE_REQUEST;
			message.code = PPPOE_INTERFACE_UNBASE;
			message.data.m_interface.ifindex = atoi(argv[2]);
			strncpy(message.data.m_interface.name, argv[3], sizeof(message.data.m_interface.name) - 1);
			message.datalen = sizeof(message.data);

			if(send_message(&message, sizeof(struct pppoe_message))) {
				printf("send PPPOE_CHANNEL_REGISTER message fail\n");
				ret = -1;
				goto out;
			}
		}
		break;
		
	case PPPOE_CHANNEL_REGISTER:
		{
			printf("enter PPPOE_CHANNEL_REGISTER\n");
		#if 0	
			if(4 != argc) {
				printf("input para is erro!\n");
				ret = -1;
				goto out;
			}
		#endif	
			memset(&message, 0, sizeof(struct pppoe_message));

			message.type = PPPOE_MESSAGE_REQUEST;
			message.code = PPPOE_CHANNEL_REGISTER;
			message.data.m_register.ifindex = atoi(argv[2]);
			message.data.m_register.sid = 1;
			message.data.m_register.mac[0] = 0x0;
			message.data.m_register.mac[1] = 0x22;
			message.data.m_register.mac[2] = 0x5f;	
			message.data.m_register.mac[3] = 0xa9;
			message.data.m_register.mac[4] = 0xee;
			message.data.m_register.mac[5] = 0x55;
			message.data.m_register.serverMac[0] = 0x0;
			message.data.m_register.serverMac[1] = 0x1f;
			message.data.m_register.serverMac[2] = 0x64;	
			message.data.m_register.serverMac[3] = 0xe1;
			message.data.m_register.serverMac[4] = 0x01;
			message.data.m_register.serverMac[5] = 0x02;
			message.datalen = sizeof(message.data);

			
			printf("mac is %2x:%2x:%2x:%2x:%2x:%2x\n", message.data.m_register.mac[0],
													message.data.m_register.mac[1], 
													message.data.m_register.mac[2], 
													message.data.m_register.mac[3], 
													message.data.m_register.mac[4], 
													message.data.m_register.mac[5]);				message.datalen = sizeof(message.data);

			if(send_message(&message, sizeof(struct pppoe_message))) {
				printf("send PPPOE_CHANNEL_REGISTER message fail\n");
				ret = -1;
				goto out;
			}
		}		
		break;

	case PPPOE_CHANNEL_UNREGISTER:
		{
			printf("enter PPPOE_CHANNEL_UNREGISTER\n");
		#if 0	
			if(4 != argc) {
				printf("input para is erro!\n");
				ret = -1;
				goto out;
			}
		#endif
			memset(&message, 0, sizeof(struct pppoe_message));

			message.type = PPPOE_MESSAGE_REQUEST;
			message.code = PPPOE_CHANNEL_UNREGISTER;
			message.data.m_register.ifindex = atoi(argv[2]);
			message.data.m_register.sid = 1;
			message.datalen = sizeof(message.data);

			if(send_message(&message, sizeof(struct pppoe_message))) {
				printf("send PPPOE_CHANNEL_REGISTER message fail\n");
				ret = -1;
				goto out;
			}
		}			
		break;

	case PPPOE_CHANNEL_AUTHORIZE:
		{
			printf("enter PPPOE_CHANNEL_AUTHORIZE\n");
		#if 0	
			if(4 != argc) {
				printf("input para is erro!\n");
				ret = -1;
				goto out;
			}
		#endif
			memset(&message, 0, sizeof(struct pppoe_message));

			message.type = PPPOE_MESSAGE_REQUEST;
			message.code = PPPOE_CHANNEL_AUTHORIZE;
			message.data.m_authorize.ifindex = atoi(argv[2]);
			message.data.m_authorize.sid = 1;
			message.data.m_authorize.ip = 0x78010102;
			
			message.datalen = sizeof(message.data);

			if(send_message(&message, sizeof(struct pppoe_message))) {
				printf("send PPPOE_CHANNEL_AUTHORIZE message fail\n");
				ret = -1;
				goto out;
			}
		}			break;

	case PPPOE_CHANNEL_UNAUTHORIZE:
		{
			printf("enter PPPOE_CHANNEL_UNAUTHORIZE\n");
		#if 0	
			if(4 != argc) {
				printf("input para is erro!\n");
				ret = -1;
				goto out;
			}
		#endif
			memset(&message, 0, sizeof(struct pppoe_message));

			message.type = PPPOE_MESSAGE_REQUEST;
			message.code = PPPOE_CHANNEL_UNAUTHORIZE;
			message.data.m_authorize.ifindex = atoi(argv[2]);
			message.data.m_authorize.ip = 0x78010102;
			message.datalen = sizeof(message.data);

			if(send_message(&message, sizeof(struct pppoe_message))) {
				printf("send PPPOE_CHANNEL_UNAUTHORIZE message fail\n");
				ret = -1;
				goto out;
			}
		}			
		break;

	default:
		printf("unknow message type %d\n", message_type);
		ret = -1;
		goto out;
	}
	
	FD_ZERO(&readfds);
	numfds = netlink_sk + 1;
	FD_SET(netlink_sk, &readfds);

reselect:
	count = select(numfds, &readfds, NULL, NULL, &tvp);
	printf("netlink select, count = %d, tvp.tv_sec = %d, tvp.tv_usec = %d\n", count, tvp.tv_sec, tvp.tv_usec);
	if (count > 0) {
		if (FD_ISSET(netlink_sk, &readfds)) {
			if (recv_errorcode(buf, sizeof(buf))) {
				printf("netlink recv errorcode fail\n");
			}

			reply = (struct pppoe_message *)NLMSG_DATA(buf);

			printf("reply->type = %d, reply->checkcode= %d, message.checkcode = %d\n",
					reply->type, reply->checkcode, message.checkcode);
			
			if (PPPOE_MESSAGE_REPLY != reply->type || 
				reply->checkcode != message.checkcode)
				goto reselect;

			if (reply->datalen < sizeof(unsigned int)) {
				ret = -10;
				printf("bad netlink message, ret = %d\n", ret);
			} else {
				ret = reply->errorcode;
			}
			
			printf("netlink message errorcode is %d\n", ret);
			goto out;
		}
	} else switch(count) {
		case 0:
			printf( "netlink select EOF\n");
			ret = -1;
			goto out;
				
		case -1:
			printf("manage select , errno = %d\n", errno);
			if (errno == EINTR) {
				goto reselect;
			} 
			printf("netlink select fail\n");
			ret = -1;
			goto out;
				
		default:
			printf("netlink select returned %d\n", count);
			ret = -1;
			goto out;
	}

out:
	exit_netlink();
	return ret;
}

