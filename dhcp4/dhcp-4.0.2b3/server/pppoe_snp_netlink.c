#ifdef __cplusplus
extern "C"
{
#endif

 #include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/netlink.h>

#include "pppoe_snp_netlink.h"
#include "dhcpd.h"

static int pppoe_snp_netlink_init(void);
static int send_netlink_msg(int sockfd, char *data, int len);
static int pppoe_snp_snd_netlink_msg(char *data, int len);
static int pppoe_snp_netlink_msg(pppoe_msg_t *msg, int len);


static int pppoe_snp_netlink_init(void)
{
	int sock = -1;
	struct sockaddr_nl saddr;

	sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_PPPOE_SNP);
	if(-1 == sock) {
		log_error("creat netlink socket(NETLINK_PPPOE_SNP) failed: %m.\n");
		return -1;
	}

	saddr.nl_family = AF_NETLINK;
	saddr.nl_pid	= getpid();
	saddr.nl_groups	= 0;
	saddr.nl_pad    = 0;

	if(0 != bind(sock,(struct sockaddr*)&saddr, sizeof(saddr))) {
		log_error("bind socket(NETLINK_PPPOE_SNP) failed: %m.\n");
		close(sock);
		return -1;
	}
	
	log_info("pppoe snooping netlink init success. socket fd %d\n", sock);
	
	return sock;
}


static int send_netlink_msg(int sockfd, char *data, int len)
{
	struct nlmsghdr *nlhdr = NULL;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl daddr;
	int ret = 0;

	if ((sockfd < 0) | (len < 0) | (!data)) {
		return -1;
	}

	log_debug("%s: date len %d\n", __func__, len);

	nlhdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(len));
	if(NULL == nlhdr) {
		log_error("pppoe snp message malloc failed\n");
		return -1;
	}
	memcpy(NLMSG_DATA(nlhdr), (char *)data, len);

	/*set nlhdr*/
	nlhdr->nlmsg_len = NLMSG_LENGTH(len);
	nlhdr->nlmsg_pid = getpid();  
	nlhdr->nlmsg_flags = 0;

	/*set daddr*/
	daddr.nl_family = AF_NETLINK;
	daddr.nl_pid = 0;
	daddr.nl_groups = 0;

	/*set message*/
	memset(&msg, 0 ,sizeof(struct msghdr));
	iov.iov_base = (void *)nlhdr;
	iov.iov_len = nlhdr->nlmsg_len;

	msg.msg_name = (void *)&daddr;
	msg.msg_namelen = sizeof(daddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/*send message*/
	if (sendmsg(sockfd, &msg, 0) == -1) {
		log_error("sendmsg netlink failed: %m");
		ret = -1;
	}

	if (nlhdr) {
		free(nlhdr);
		nlhdr = NULL;
	}
	
	log_debug("%s: sendmsg ok.\n", __func__);

	return ret;
}


static int pppoe_snp_snd_netlink_msg(char *data, int len)
{
	static int sock = -1;

	if (sock < 0) {
		sock = pppoe_snp_netlink_init();
		if (sock < 0) {
			return -1;
		}
	}

	return send_netlink_msg(sock, data, len);
}

static int pppoe_snp_netlink_msg(pppoe_msg_t *msg, int len)
{
	if (!msg | (len < 0)) {
		return -1;
	}
	
	msg->type = pppoe_snp_type;

	return pppoe_snp_snd_netlink_msg((char *)msg, len);;
}


int pppoe_snp_server_enable(unsigned int enable)
{
	pppoe_msg_t msg;

	memset((char *)&msg, 0, sizeof(msg));
	msg.subtype = pppoe_snp_subtype_enable;
	msg.datalen = sizeof(enable);
	msg.data.enable_flag = enable;
	
	return pppoe_snp_netlink_msg(&msg, sizeof(msg));
}



int pppoe_snp_netlink_mru_msg(unsigned short mru)
{
	pppoe_msg_t msg;

	memset((char *)&msg, 0, sizeof(msg));
	msg.subtype = pppoe_snp_subtype_setmru;
	msg.datalen = sizeof(mru);
	msg.data.mru = mru;
	
	return pppoe_snp_netlink_msg(&msg, sizeof(msg));
}


int pppoe_snp_netlink_debug_msg(unsigned int set_flag, unsigned int debug_type)
{
	pppoe_msg_t msg;

	memset((char *)&msg, 0, sizeof(msg));
	
	msg.subtype = pppoe_snp_subtype_logswitch;
	msg.datalen = sizeof(msg);
	msg.data.debug.set_flag = set_flag;
	msg.data.debug.log_level = debug_type;

	return pppoe_snp_netlink_msg(&msg, sizeof(msg));
}

static int dba_netlink_msg(dba_msg_t *msg, int len)
{
	if (!msg | (len < 0)) {
		return -1;
	}
	
	msg->type = dba_msg_type;

	return pppoe_snp_snd_netlink_msg((char *)msg, len);;
}


int dba_server_enable(unsigned int enable)
{
	dba_msg_t msg;

	memset((char *)&msg, 0, sizeof(msg));
	msg.subtype = dba_subtype_enable;
	msg.datalen = sizeof(enable);
	msg.data.enable_flag = enable;
	
	return dba_netlink_msg(&msg, sizeof(msg));
}


#ifdef __cplusplus
}
#endif
