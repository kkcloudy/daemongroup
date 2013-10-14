/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* intf_cap_stat.c
*
*
* CREATOR:
* 		yangxs@autelan.com
*
* DESCRIPTION:
* 		statistics the interface capability.
*
* DATE:
*		21/7/2010
*
* FILE REVISION NUMBER:
*  		$Revision: 1.0 $
*
*******************************************************************************/
#ifdef __cplusplus
		extern "C"
		{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "ws_intf.h"

#include <dbus/dbus.h>
#include "ws_dbus_def.h"
#include "ac_manage_def.h"

struct intflib_handle rth = { .fd = -1 };


/*******************************************************************************
 * intflib_dump_filter
 *
 * DESCRIPTION:
 *   		receive message by netlink.
 *
 * INPUTS:
 * 		rth - netlink handler
 *		filter - handle function
 *		arg1 - store space
 *		junk - NULL
 *		arg2 - NULL
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/

int 
get_if_index(const char *name, unsigned int *ifindex){
    if(NULL == name) {
        return -1;
    }
    
    int fd, rc = 0;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {
        syslog(LOG_WARNING, "couldn't create socket\n");
        return -2;
    }

    struct ifreq ifrq;
    memset(&ifrq, 0, sizeof(struct ifreq));
    strncpy(ifrq.ifr_name, name, sizeof(ifrq.ifr_name));
    ifrq.ifr_name[sizeof(ifrq.ifr_name)-1] = 0;
    
    rc = ioctl(fd, SIOCGIFINDEX, &ifrq);
    if (rc < 0) {
        syslog(LOG_WARNING, "ioctl %s returned %d\n", name, rc);
        rc = -3;
    }
    *ifindex = ifrq.ifr_ifindex;
    
    close(fd);
    return rc;
}

int 
get_if_name(unsigned int ifindex, char **name){

    if(NULL == name) {
        return -1;
    }
    *name = NULL;
    
    int fd, rc = 0;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {
        syslog(LOG_WARNING, "couldn't create socket\n");
        return -2;
    }

    struct ifreq ifrq;
    memset(&ifrq, 0, sizeof(struct ifreq));
    ifrq.ifr_ifindex = ifindex;
    
    rc = ioctl(fd, SIOCGIFNAME, &ifrq);
    if (rc < 0) {
        syslog(LOG_WARNING, "ioctl %d returned %d\n", ifrq.ifr_ifindex, rc);
        rc = -3;
    }
    
    *name = strdup(ifrq.ifr_name);
    
    close(fd);
    return rc;
}




int intflib_dump_filter
(
	struct intflib_handle *rth,
	rtnl_filter_t filter,
	void *arg1,
	rtnl_filter_t junk,
	void *arg2
)
{
	struct sockaddr_nl nladdr;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char buf[16384];

	iov.iov_base = buf;
	while (1) {
		int status;
		struct nlmsghdr *h;

		iov.iov_len = sizeof(buf);
		status = recvmsg(rth->fd, &msg, 0);

		if (status < 0) {
			if (errno == EINTR)
				continue;
			perror("OVERRUN");
			continue;
		}

		if (status == 0) {
			//printf("EOF on netlink\n");
			return -1;
		}

		h = (struct nlmsghdr*)buf;
//		while (NLMSG_OK(h, status)) {
		for( ; NLMSG_OK(h, status); h = NLMSG_NEXT(h, status) )
		{
			int err;

			if (nladdr.nl_pid != 0 ||
			    h->nlmsg_pid != rth->local.nl_pid ||
			    h->nlmsg_seq != rth->dump) {
				if (junk) {
					err = junk(&nladdr, h, arg2);
					if (err < 0)
						return err;
				}
				continue;
			}

			if (h->nlmsg_type == NLMSG_DONE)
				return 0;
			if (h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
				if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
					//printf("ERROR truncated\n");
				} else {
					errno = -err->error;
					perror("RTNETLINK answers");
				}
				return -1;
			}
			err = filter(&nladdr, h, arg1);
			if (err < 0)
				return err;
#if 0
skip_it:
			h = NLMSG_NEXT(h, status);
#endif			
		}
		if (msg.msg_flags & MSG_TRUNC) {
			//printf("Message truncated\n");
			continue;
		}
		if (status) {
			//printf("!!!Remnant of size %d\n", status);
			return -1;
		}
	}
}

/*******************************************************************************
 * intflib_wilddump_request
 *
 * DESCRIPTION:
 *   		message request by netlink.
 *
 * INPUTS:
 * 		rth - netlink handler
 *		family - family cluster
 *		type - netlink type
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int intflib_wilddump_request
(
	struct intflib_handle *rth,
	int family,
	int type
)
{
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;
	struct sockaddr_nl nladdr;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = type;
	req.nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = rth->dump = ++rth->seq;
	req.g.rtgen_family = family;

	return sendto(rth->fd, (void*)&req, sizeof(req), 0,
		      (struct sockaddr*)&nladdr, sizeof(nladdr));
}


/*******************************************************************************
 * intflib_open_byproto
 *
 * DESCRIPTION:
 *   		open the netlink,init the rth.
 *
 * INPUTS:
 * 		subscriptions - group
 *		protocol - NETLINK_ROUTE
 *
 * OUTPUTS:
 *    	rth - netlink handler
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int intflib_open_byproto
(
	struct intflib_handle *rth, 
	unsigned subscriptions,
	int protocol
)
{
	socklen_t addr_len;
	int sndbuf = 32768;
	int rcvbuf = 32768;

	memset(rth, 0, sizeof(struct intflib_handle));

	rth->fd = socket(AF_NETLINK, SOCK_RAW, protocol);
	if (rth->fd < 0) {
		perror("Cannot open netlink socket");
		return -1;
	}

	if (setsockopt(rth->fd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf)) < 0) {
		perror("SO_SNDBUF");
		return -1;
	}

	if (setsockopt(rth->fd,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf)) < 0) {
		perror("SO_RCVBUF");
		return -1;
	}

	memset(&rth->local, 0, sizeof(rth->local));
	rth->local.nl_family = AF_NETLINK;
	rth->local.nl_groups = subscriptions;

	if (bind(rth->fd, (struct sockaddr*)&rth->local, sizeof(rth->local)) < 0) {
		perror("Cannot bind netlink socket");
		return -1;
	}
	addr_len = sizeof(rth->local);
	if (getsockname(rth->fd, (struct sockaddr*)&rth->local, &addr_len) < 0) {
		perror("Cannot getsockname");
		return -1;
	}
	if (addr_len != sizeof(rth->local)) {
		printf("Wrong address length %d\n", addr_len);
		return -1;
	}
	if (rth->local.nl_family != AF_NETLINK) {
		printf("Wrong address family %d\n", rth->local.nl_family);
		return -1;
	}
	rth->seq = time(NULL);
	return 0;
}

/*******************************************************************************
 * intflib_open
 *
 * DESCRIPTION:
 *   	open the netlink,init the rth.
 *
 * INPUTS:
 * 		subscriptions - group  
 *
 * OUTPUTS:
 *    	rth - netlink handler
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int intflib_open
(
	struct intflib_handle *rth,
	unsigned subscriptions
)
{
	return intflib_open_byproto(rth, subscriptions, NETLINK_ROUTE);
}

/*******************************************************************************
 * intflib_close
 *
 * DESCRIPTION:
 *   		close netlink
 *
 * INPUTS:
 * 		rth -  netlink handler
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
void intflib_close
(
	struct intflib_handle *rth
)
{
	if (rth->fd >= 0) {
		close(rth->fd);
		rth->fd = -1;
	}
}

/*******************************************************************************
 * intflib_store_nlmsg
 *
 * DESCRIPTION:
 *   		store message get by netlink
 *
 * INPUTS:
 * 		who -  netlink address
 *		n - netlink message header
 *
 * OUTPUTS:
 *    	arg - store space
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
static int intflib_store_nlmsg
(
	const struct sockaddr_nl *who,
	struct nlmsghdr *n, 
	void *arg
)
{
	//struct nlmsg_list **linfo = (struct nlmsg_list**)arg;
	struct flow_data_list *head = (struct flow_data_list *)arg;
	struct flow_data_list *temp = NULL;
	int ret = -1;

	struct nlmsg_list *h;
	struct nlmsg_list **lp;

	temp = (struct flow_data_list *)malloc(sizeof(struct flow_data_list));
	if (NULL==temp)
	{
		return -1;
	}
	memset(temp, 0, sizeof(struct flow_data_list));

	ret = collect_linkinfo(n, &(temp->ltdata));
	if (ret < 0)
	{
		printf("collect link info error! ret = %d.\n", ret);
		if(NULL!=temp){free(temp);temp=NULL;}
		return -1;
	}

	temp->next = head->next;
	head->next = temp;
	

#if 0
	h = malloc(n->nlmsg_len+sizeof(void*));
	if (h == NULL)
		return -1;

	memcpy(&h->h, n, n->nlmsg_len);
	h->next = NULL;

	for (lp = linfo; *lp; lp = &(*lp)->next) /* NOTHING */;
	*lp = h;

#endif

	return 0;
}


/*******************************************************************************
 * intflib_parse_rtattr
 *
 * DESCRIPTION:
 *   		store the data in rtattr type, for easy checking all kind of message.
 *
 * INPUTS:
 * 		max - the max type
 *		rta - point to rta data
 *		len - data lenth without netlink header
 *
 * OUTPUTS:
 *    	tb[] - store all type data
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int intflib_parse_rtattr
(
	struct rtattr *tb[],
	int max,
	struct rtattr *rta,
	int len
)
{
	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= max)
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta,len);
	}
	if (len)
		printf("!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
	return 0;
}

/*******************************************************************************
 * collect_linkinfo
 *
 * DESCRIPTION:
 *   		get intface packet info by order
 *
 * INPUTS:
 * 		n -  netlink header
 *
 * OUTPUTS:
 *    	gdata - struct pointer store interface packet info
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
static void get_cheat_data( struct flow_data * gdata )
{
	char file_path[128]={0};
	char line[64]={0};
	FILE *fp=NULL;
	
	snprintf( file_path, sizeof(file_path), "/var/run/%s.status", gdata->ifname );
	fp = fopen( file_path, "r" );
	if( NULL != fp )
	{
		while( !feof(fp) )
		{
			fgets( line, sizeof(line), fp );
			if( strlen(line) <= 0 )
			{
				continue;
			}
			if( strncmp(line,"rx_drop",strlen("rx_drop"))==0 )
			{
				sscanf( line, "rx_drop:%u", &(gdata->rtdata.rx.dropped));
			}
			else if( strncmp(line,"rx_error",strlen("rx_error"))==0 )
			{
				sscanf( line, "rx_error:%u", &(gdata->rtdata.rx.errors) );
			}
			else if( strncmp(line,"tx_drop",strlen("tx_drop"))==0 )
			{
				sscanf( line, "tx_drop:%u", &(gdata->rtdata.tx.dropped) );
			}
			else if( strncmp(line,"tx_error",strlen("tx_error"))==0 )
			{
				sscanf( line, "tx_error:%u", &(gdata->rtdata.tx.errors) );
			}
		}
		fclose(fp);
	}
}
int collect_linkinfo
( 
	struct nlmsghdr *n,
	struct flow_data * gdata
)
{
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr * tb[IFLA_MAX+1];
	int len = n->nlmsg_len;
	unsigned m_flag = 0;

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

	intflib_parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
	if (tb[IFLA_IFNAME] == NULL) {
		printf("BUG: nil ifname\n");
		return -1;
	}

	if(strcmp((char*)RTA_DATA(tb[IFLA_IFNAME]),"radio") == 0){
		strcpy(gdata->ifname,(char *)RTA_DATA(tb[IFLA_IFNAME]));
		return IS_RADIO;
	}

	if (tb[IFLA_STATS]) {
		struct intflib_link_stats slocal;
		struct intflib_link_stats *s = RTA_DATA(tb[IFLA_STATS]);
		if (((unsigned long)s) & (sizeof(unsigned long)-1)) {
			memcpy(&slocal, s, sizeof(slocal));
			s = &slocal;
		}

		strcpy(gdata->ifname,(char *)RTA_DATA(tb[IFLA_IFNAME]));		
		gdata->rtdata.rx.packets = (s->rx_packets) - (s->multicast);
		gdata->rtdata.rx.bytes = s->rx_bytes;
		gdata->rtdata.rx.errors = s->rx_errors;
		gdata->rtdata.rx.dropped = s->rx_dropped;
		gdata->rtdata.rx.multicast = s->multicast;
		gdata->rtdata.tx.packets = s->tx_packets;
		gdata->rtdata.tx.bytes = s->tx_bytes;
		gdata->rtdata.tx.errors = s->tx_errors;
		gdata->rtdata.tx.dropped = s->tx_dropped;
		gdata->rtdata.tx.multicast = 0; /* currently unavailable*/

		gdata->rtdata.index = if_nametoindex(gdata->ifname);

		get_cheat_data(gdata);/*add by shaojunwu for  CMCC test*/
	}
	return 0;
}

/*******************************************************************************
 * intflib_getdata
 *
 * DESCRIPTION:
 *   		
 *
 * INPUTS:
 * 		alldata - struct flow_data, supply an array to store all interface info.
 *		intfsize - the array size.
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int intflib_getdata
(
	struct flow_data_list ** alldata
)
{
	//struct nlmsg_list *linfo = NULL;
	//struct nlmsg_list *l = NULL, *n = NULL;
	//struct flow_data_list * gdata = NULL;
	struct flow_data_list *fdata = NULL;
	unsigned int ret= 0; //, num = 0;
	
	if (intflib_open(&rth, 0) < 0){
		printf("intflib open error!\n");
		return -1;
		}

	if (intflib_wilddump_request(&rth, AF_PACKET, RTM_GETLINK) < 0) {
		printf("intfidx_wilddump_request: error Cannot send dump request!\n");
		return -1;
	}

	fdata = (struct flow_data_list*)malloc(sizeof(struct flow_data_list));
	if (NULL == fdata)
	{
		printf("intflib_getdata: malloc mem error!\n");
		return -1;
	}
	memset(fdata,0,sizeof(struct flow_data_list));
	*alldata = fdata;
		

	if (intflib_dump_filter(&rth, intflib_store_nlmsg, fdata, NULL, NULL) < 0) {
		printf("intflib Dump terminated.\n");
		return -1;
	}

#if 0
	fdata = (struct flow_data_list*)malloc(sizeof(struct flow_data_list));
	if(NULL == fdata){
		printf("intflib_getdata: malloc mem error!\n");
		return -1;
	}
	memset(fdata,0,sizeof(struct flow_data_list));
	*alldata = fdata;
	
	for (l=linfo; l != NULL; l = n) {
		n = l->next;
		gdata = (struct flow_data_list*)malloc(sizeof(struct flow_data_list));
		if(NULL == gdata){
			printf("intflib_getdata: malloc mem error!\n");
			return -1;
		}
		memset(gdata,0,sizeof(struct flow_data_list));
		fdata->next = gdata;
		fdata = gdata;
		fdata->next = NULL;
		gdata = NULL;
		
		ret = collect_linkinfo(&l->h,&(fdata->ltdata));
		if(ret < 0){
			printf("collect link info error! ret = %d.\n", ret);
			return -1;
		}
		else if(IS_RADIO == ret){
				continue;
		}
	}
#endif

	fdata = NULL;
	fdata = *alldata;
	*alldata = (*alldata)->next;
	free(fdata);
	fdata = NULL;

	intflib_close(&rth);

	
	return 0;
}

#if 0
/*******************************************************************************
 * collect_linkinfo_byname
 *
 * DESCRIPTION:
 *   	get intface packet info by interface name
 *
 * INPUTS:
 * 		n - netlink message header 
 *		
 * OUTPUTS:
 *    	gdata - struct pointer pass the intface info.
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int collect_linkinfo_byname
( 
	struct nlmsghdr *n,
	struct flow_data * gdata
)
{
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr * tb[IFLA_MAX+1];
	int len = n->nlmsg_len;
	unsigned m_flag = 0;

	if(NULL == gdata->ifname){
		printf("collect_linkinfo_byname: nil ifname error.\n");
		return -1;
	}

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

	intflib_parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
	if (tb[IFLA_IFNAME] == NULL) {
		printf("BUG: nil ifname\n");
		return -1;
	}

	if(strcmp((char*)RTA_DATA(tb[IFLA_IFNAME]),"radio") == 0){
		printf("iterface name is radio,neglect!\n");
		return IS_RADIO;
	}

	if (tb[IFLA_STATS] && (strcmp((char*)RTA_DATA(tb[IFLA_IFNAME]),gdata->ifname) == 0)) {
		struct intflib_link_stats slocal;
		struct intflib_link_stats *s = RTA_DATA(tb[IFLA_STATS]);
		if (((unsigned long)s) & (sizeof(unsigned long)-1)) {
			memcpy(&slocal, s, sizeof(slocal));
			s = &slocal;
		}

		gdata->rtdata.rx.packets = (s->rx_packets) - (s->multicast);
		gdata->rtdata.rx.bytes = s->rx_bytes;
		gdata->rtdata.rx.errors = s->rx_errors;
		gdata->rtdata.rx.dropped = s->rx_dropped;
		gdata->rtdata.rx.multicast = s->multicast;
		gdata->rtdata.tx.packets = (s->rx_packets) - (s->multicast);
		gdata->rtdata.tx.bytes = s->tx_bytes;
		gdata->rtdata.tx.errors = s->tx_errors;
		gdata->rtdata.tx.dropped = s->tx_dropped;
		gdata->rtdata.tx.multicast = s->multicast;
		gdata->rtdata.index = if_nametoindex(gdata->ifname);

		return GD_OK;
	}
	return 0;
}


/*******************************************************************************
 * intflib_getdata_byname
 *
 * DESCRIPTION:
 *   	get interface data by interface name.
 *
 * INPUTS:
 * 		null 
 *
 * OUTPUTS:
 *    	namedata - struct flow_data,store intface info.
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int intflib_getdata_byname
(
	char *ifname,
	struct flow_data_list ** namedata
)
{
	struct nlmsg_list *linfo = NULL;
	struct nlmsg_list *l = NULL, *n = NULL;
	unsigned int ret= 0;
	
	if (intflib_open(&rth, 0) < 0){
		printf("intflib open error!\n");
		return -1;
		}

	if (intflib_wilddump_request(&rth, AF_PACKET, RTM_GETLINK) < 0) {
		printf("intfidx_wilddump_request: error Cannot send dump request!\n");
		return -1;
	}

	if (intflib_dump_filter(&rth, intflib_store_nlmsg, &linfo, NULL, NULL) < 0) {
		printf("intflib Dump terminated\n");
		return -1;
	}

	*namedata = (struct flow_data_list *)malloc(sizeof(struct flow_data_list));	
	if(NULL == *namedata){
		printf("intflib_getdata_byname: malloc mem error!\n");
		return -1;
	}
	memset(*namedata,0,sizeof(struct flow_data_list));
	strcpy((*namedata)->ltdata.ifname,ifname);
	for (l=linfo; l; l = n) {
		n = l->next;
		ret = collect_linkinfo_byname(&l->h,&((*namedata)->ltdata));
		if(ret < 0){
			printf("collect link info error! ret = %d.\n", ret);
			break;
		}
		else if(IS_RADIO == ret){ /*when the interface is radio neglect and not store in group.*/
			printf("check out it is radio, skip and continue.\n");
			continue;
		}
		else if(GD_OK == ret)
			break;
	}

	intflib_close(&rth);
	
	return 0;
}
#endif
/*******************************************************************************
 * intflib_memfree
 *
 * DESCRIPTION:
 *   	free the mem.
 *
 * INPUTS:
 * 		freedata - data want to free. 
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int intflib_memfree
(
	struct flow_data_list ** freedata
)
{
	struct flow_data_list * tempin = NULL;

	for(*freedata;*freedata;){
		tempin = *freedata;
		*freedata = (*freedata)->next;
		free(tempin);
	}
	*freedata= NULL;
	tempin = NULL;
	return 0;
}

/*test function*/
#if 0
main(void)
{
	unsigned int ret =0;
	struct flow_data_list * alldata = NULL;
	struct flow_data_list *temp = NULL;	
	struct flow_data_list * namedata = NULL;
	char buff[IFNAME_SIZE] = "eth0";
	
	/*check all interface infomation*/
	printf("check all interface infomation.\n");
	ret = intflib_getdata(&alldata);
	if(ret < 0){
		printf("intflib get data error! ret %d.\n",ret);
	}

	temp = alldata;
	for(temp; temp; temp = temp->next){
		if(strcmp(temp->ltdata.ifname,"radio") == 0){
			continue;
		}
		printf("  ifname %s.\n",temp->ltdata.ifname);
		printf("RX: unic-packets  bytes  errors  dropped  anti-unicast\n");
		printf("    %-7d  %-5d  %-6d  %-7d  %-9d\n",
					temp->ltdata.rtdata.rx.packets,
					temp->ltdata.rtdata.rx.bytes,
					temp->ltdata.rtdata.rx.errors,
					temp->ltdata.rtdata.rx.dropped,
					temp->ltdata.rtdata.rx.multicast
					);
		printf("TX: unic-packets  bytes  errors  dropped  anti-unicast\n");
		printf("    %-7d  %-5d  %-6d  %-7d  %-9d\n",
					temp->ltdata.rtdata.tx.packets,
					temp->ltdata.rtdata.tx.bytes,
					temp->ltdata.rtdata.tx.errors,
					temp->ltdata.rtdata.tx.dropped,
					temp->ltdata.rtdata.tx.multicast
					);
	}
	intflib_memfree(&alldata);

/*check interface infomation by ifterface name*/
	printf("\ncheck interface infomation by ifterface name: %s.\n",buff);
	ret = intflib_getdata_byname(buff,&namedata);
	if(ret < 0){
		printf("intflib getdata by name error! ret %d.\n",ret);
	}

	printf("  ifname %s.\n",namedata->ltdata.ifname);
	printf("RX: unic-packets  bytes  errors  dropped  anti-unicast\n");
	printf("    %-7d  %-5d  %-6d  %-7d  %-9d\n",
				namedata->ltdata.rtdata.rx.packets,
				namedata->ltdata.rtdata.rx.bytes,
				namedata->ltdata.rtdata.rx.errors,
				namedata->ltdata.rtdata.rx.dropped,
				namedata->ltdata.rtdata.rx.multicast
				);
	printf("TX: unic-packets  bytes  errors  dropped  anti-unicast\n");
	printf("    %-7d  %-5d  %-6d  %-7d  %-9d\n",
				namedata->ltdata.rtdata.tx.packets,
				namedata->ltdata.rtdata.tx.bytes,
				namedata->ltdata.rtdata.tx.errors,
				namedata->ltdata.rtdata.tx.dropped,
				namedata->ltdata.rtdata.tx.multicast
				);
	intflib_memfree(&namedata);
	
}
#endif

int
show_acif_accumulate_stats(DBusConnection *connection, unsigned int slot_id, struct if_stats_list **if_array, unsigned int *if_num) {

    if(NULL == connection || NULL == if_array || NULL == if_num) 
        return AC_MANAGE_INPUT_TYPE_ERROR;

    if(0 == slot_id || slot_id > SLOT_MAX_NUM)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter  iter;
    DBusMessageIter  iter_array;
    DBusMessageIter  iter_struct;

    *if_array = NULL;
    *if_num = 0;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    

    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
				                                         AC_MANAGE_DBUS_OBJPATH,
				                                         AC_MANAGE_DBUS_INTERFACE,
				                                         AC_MANAGE_DBUS_SHOW_MIB_ACCUMULATE_ACIF_STATS);

    dbus_error_init(&err);

    dbus_message_append_args(query,
			                            DBUS_TYPE_UINT32, &slot_id,       
			                            DBUS_TYPE_INVALID);
                            
    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }

    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_iter_next(&iter);  
    dbus_message_iter_get_basic(&iter, &temp_num);

    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   

    if(AC_MANAGE_SUCCESS == ret && temp_num){

        struct if_stats_list *temp_array = (struct if_stats_list *)calloc(temp_num, sizeof(struct if_stats_list));
        if(NULL == temp_array) {
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }
            
        int i = 0;
        for(i = 0; i < temp_num; i++) {

            struct if_stats_list if_stats_node = { 0 };
            
            char *ifname = NULL;
            
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ifname);

            
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_packets));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_packets));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_bytes));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_bytes));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_dropped));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_dropped));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_multicast));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_multicast));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_compressed));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_compressed));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.collisions));



            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_length_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_over_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_crc_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_frame_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_fifo_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.rx_missed_errors));



            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_aborted_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_carrier_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_fifo_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_heartbeat_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].stats.tx_window_errors));


            strncpy(temp_array[i].ifname, ifname, sizeof(temp_array[i].ifname) - 1);
                
            dbus_message_iter_next(&iter_array);
        }

        *if_array = temp_array;
        *if_num = temp_num;
    }

    dbus_message_unref(reply);

    return ret;    
}


#ifdef __cplusplus
}
#endif



