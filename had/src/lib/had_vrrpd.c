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
* had_vrrpd.c
*
* CREATOR:
*		zhengcs@autelan.com
*
* DESCRIPTION:
*		APIs used in HAD module for vrrp control plane.
*
* DATE:
*		06/16/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.24 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/* system include */
#include <stdio.h>
#include <assert.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/errno.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <pthread.h>
#include <fcntl.h>
#include <dbus/dbus.h>
#include <sys/wait.h>

#if 1
#include <netdb.h>
#include <ifaddrs.h>
#endif

#include "sysdef/npd_sysdef.h"
#include "sysdef/portal_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "dbus/dhcp/dhcp_dbus_def.h"
#include "dbus/asd/ASDDbusDef.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/wcpss/ACDBusPath.h"
#include "dbus/hmd/HmdDbusPath.h"


/* local include */
#include "had_vrrpd.h"
#include "had_ipaddr.h"
#include "had_dbus.h"
#include "had_uid.h"
#include "had_log.h"
#include "had_ndisc.h"
#include "board/netlink.h"

int ip_id = 0;	/* to have my own ip_id creates collision with kernel ip->id
		** but it should be ok because the packets are unlikely to be
		** fragmented (they are non routable and small) */
		/* WORK: this packet isnt routed, i can check the outgoing MTU
		** to warn the user only if the outoing mtu is too small */
//static char vrrp_hwaddr[6];	// WORK: lame hardcoded for ethernet
//static vrrp_rt	glob_vsrv;	/* a global because used in the signal handler*/
/* Scott added 9-4-02 */
//int master_ipaddr = 0;
int global_enable = 0;
int master_ipaddr_uplink[VRRP_MAX_VRID] = {0};
int master_ipaddr_downlink[VRRP_MAX_VRID] = {0};
int uplink_leave_master_timer[VRRP_MAX_VRID] = {0};
int downlink_leave_master_timer[VRRP_MAX_VRID] = {0};
int transfer_timer_start[MAX_HANSI_PROFILE] = {0};
int wid_transfer_state[MAX_HANSI_PROFILE]={0};
int portal_transfer_state[MAX_HANSI_PROFILE]= {0};
#ifndef _VERSION_18SP7_
int pppoe_transfer_state[MAX_HANSI_PROFILE]= {0};
#endif

int service_enable[MAX_HANSI_PROFILE] = {0};
/*
  * heartbeat line interfacec name
  */
char* global_ht_ifname = NULL;
/* 
  *	heartbeat line ip address
  */
int global_ht_ip = 0;
/* 
  *	heartbeat line link state
  */
int global_ht_state = 0;
/* 
  *	heartbeat line peer side ip address
  */
int global_ht_opposite_ip = 0;
/* 
  *	packet count to verify master device is down
  *	This is used by the device in backup state, is also
  *	say as master-fail timer 
  */
int global_ms_down_packet_count = VRRP_DEFAULT_DOWN_PACKET_COUNT;
/* 
  *	time sync reply packet indicator 
  *	master device set the flag, backup device check it
  */
int global_timer_reply = 0;
/* 
  *	time sync request packet indicator 
  *	backup device set the flag, master device check it and reply
  */
int global_timer_request = 0;
/* 
  *	multi-link detection functionality flag, disabled by default
  */
int global_multi_link_detect = 0;

unsigned char vrrp_global_mac[ETH_ALEN] = {0};

/*
  *	vrrp packet vmac 00-00-5E-00-01-{VRID} 
  * 	normally used as source mac of vrrp packet
  */
unsigned char vrrp_vmac[6] = {0x00,0x00,0x5E,0x00,0x01,0x00};

/*
  *	vrrp packet vmac 00-00-5E-00-01-{VRID} 
  * 	normally used as source mac of vrrp packet
  */
unsigned char vrrp_dmac[6] = {0x01,0x00,0x5E,0x00,0x00,0x12};

/*
 * string of dbusname/opjpath/interface
 * use to send dbus message to wid
 */
char global_wid_dbusname[VRRP_DBUSNAME_LEN] = {0};
char global_wid_bak_objpath[VRRP_OBJPATH_LEN] = {0};
char global_wid_bak_interfce[VRRP_DBUSNAME_LEN] = {0};
/*
  * 	global flag indicates whether boot stage check is needed or not ( 1- needed, as default; 0 - no need).
  */
unsigned char g_boot_check_flag = 1;

/* system startup state file descriptor*/
int aw_state_fd = -1;

/* for cancel time synchronize */
int time_synchronize_enable = 0;

/* flag for receive advertisements packet in backloop,
	default value is 0, 
	the value is set 1, if master down timer is timeout or heatbeat link down.
	state machine will goto MASTER and set the value is 0, if do not receive two consecutive advertisements packets.
	If rx thread receive a advertisements packet, the value is set 0.
	*/
int stateFlg = 0;

/* file to save system startup state */
#define HAD_SYSTEM_STARTUP_STATE0_PATH	"/var/run/aw.state"
#define HAD_SYSTEM_STARTUP_STATE0_FLAG	'1'
#define HAD_SYSTEM_STARTUP_STATE1_PATH	"/sys/module/cavium_ethernet/parameters/pend_cvm"
#define HAD_SYSTEM_STARTUP_STATE1_FLAG	'0'
#define HAD_BOOT_CHECK_MSG_HZ	30

#define VRRP_GLOBAL_SOCKET_FD 1
#ifdef VRRP_GLOBAL_SOCKET_FD
 int vrrp_socket_inet_dgram_0_fd = -1;
#endif


pthread_mutex_t PacketMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t IpopMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t StateMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t IoMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t		*dbus_thread;
pthread_attr_t	dbus_thread_attr;
pthread_t		*packet_thread;
pthread_attr_t	packet_thread_attr;
pthread_t		*packet_thread2;
pthread_attr_t	packet_thread_attr2;
pthread_t		*state_thread;
pthread_attr_t	state_thread_attr;	
pthread_t		*link_thread;
pthread_attr_t	link_thread_attr;
pthread_t       *arp_thread;
pthread_attr_t  arp_thread_attr;
pthread_t       *ndisc_thread;
pthread_attr_t  ndisc_thread_attr;

#if 0
pthread_t		*timesyn_thread;
pthread_attr_t	timesyn_thread_attr;
#else
pthread_t       *timer_thread;
pthread_attr_t  timer_thread_attr;
#endif

#if 0
/* delete by jinpc@autelan.com */
static char	PidDir[FILENAME_MAX+1];
#endif

int uplink_ip_id = 0;
int downlink_ip_id = 0;
int global_protal = 1;/*eag is default running in 2.0!!! shaojunwu 2011-09-19*/
#ifndef _VERSION_18SP7_
int global_pppoe = 1;
#endif
int global_hmd = 1;
int global_state_change_bit = 0;
/* while notifying to trap set to 1 ,wid set to 2,portal set to 3,dhcp set to 4 */
int global_notifying_flag = VRRP_NOTIFY_NONE;
char* global_notify_obj [VRRP_NOTIFY_MAX] = {"", "trap", "wid", "portal", "dhcp","pppoe"};
int global_notify_count [VRRP_NOTIFY_MAX] = {0};

int sendsock = 0;
struct sock_list_t* sock_list;
struct state_trace** TRACE_LOG;
extern hansi_s     **g_hansi ;

extern DBusConnection *vrrp_cli_dbus_connection;
extern DBusConnection *vrrp_notify_dbus_connection;
extern int uidSock;

extern int global_current_instance_no;
extern char global_current_objpath[VRRP_OBJPATH_LEN];
extern char global_cli_dbusname[VRRP_DBUSNAME_LEN];
extern char global_notify_dbusname[VRRP_DBUSNAME_LEN];

static void had_get_sys_mac(char *);
long my_pid = 0;

int sock_had_fd = 0;
struct msghdr had_msg;
struct nlmsghdr *had_nlh = NULL;
struct sockaddr_nl had_src_addr, had_dest_addr;
struct iovec had_iov;

int had_netlink_send(char *msgBuf, int len)
{
	memcpy(NLMSG_DATA(had_nlh), msgBuf, len);
	
	nl_msg_head_t *head = (nl_msg_head_t*)NLMSG_DATA(had_nlh);
	
    vrrp_syslog_info("\tNetlink vrrp pid(%d) send to module(%d)\n", head->pid, head->object);
	
	if(sendmsg(sock_had_fd, &had_msg, 0) < 0)
	{
        vrrp_syslog_info("Failed vrrp netlink send : %s\n", strerror(errno));
		return -1;
	}
    vrrp_syslog_info("\thad_netlink_send : vrrp netlink send succeed\n");

	return 0;
}

void had_state_notifier_rtmd(char *ifname,int state)
{
    char chBuf[512];
	//int i;

    nl_msg_head_t *head = (nl_msg_head_t*)chBuf;
    netlink_msg_t *nl_msg= (netlink_msg_t*)(chBuf + sizeof(nl_msg_head_t));			
    head->pid = getpid();		
    head->type = OVERALL_UNIT;
	head->object = COMMON_MODULE;
	head->count = 1;
	
	nl_msg->msgType = VRRP_STATE_NOTIFIER_EVENT;
	memcpy(nl_msg->msgData.vrrpInfo.interface, ifname, strlen(ifname));
    nl_msg->msgData.vrrpInfo.state = state;

	vrrp_syslog_info("\t%s netlink send state to RTMD, vrrp state is %s(%d)\n",__func__,(state == 1)?"MASTER":"OTHER",state);
	
    int len = sizeof(nl_msg_head_t) + head->count*sizeof(netlink_msg_t);
    had_netlink_send(chBuf, len);
	return;
}

int had_netlink_init(void)
{	
	//vrrp_syslog_info("vrrp_netlink_init start ........\n");

	/* Initialize data field */
	memset(&had_src_addr, 0, sizeof(had_src_addr));
	memset(&had_dest_addr, 0, sizeof(had_dest_addr));
	memset(&had_iov, 0, sizeof(had_iov));
	memset(&had_msg, 0, sizeof(had_msg));
	
	/* Create netlink socket use NETLINK_DISTRIBUTED(18) */
	if ((sock_had_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_RTMD)) < 0) {
		vrrp_syslog_error("had_netlink_init:Failed vrrp socket : %s\n", strerror(errno));
		return -1;
	}

	/* Fill in src_addr */
	had_src_addr.nl_family = AF_NETLINK;
	had_src_addr.nl_pid = getpid();
	/* Focus */
	had_src_addr.nl_groups = 1;

	if (bind(sock_had_fd, (struct sockaddr*)&had_src_addr, sizeof(had_src_addr)) < 0) {
		vrrp_syslog_error("had_netlink_init:Failed bind : %s\n", strerror(errno));
		return -1;
	}

	/* Fill in dest_addr */
	had_dest_addr.nl_pid = 0;
	had_dest_addr.nl_family = AF_NETLINK;
	/* Focus */
	had_dest_addr.nl_groups = 1;

	/* Initialize buffer */
	if((had_nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD))) == NULL) {
		vrrp_syslog_error("Failed malloc\n");
		return -1;
	}

	memset(had_nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	had_nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	had_nlh->nlmsg_pid = getpid();
	had_nlh->nlmsg_flags = 0;
	had_iov.iov_base = (void *)had_nlh;
	had_iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	had_msg.msg_name = (void *)&had_dest_addr;
	had_msg.msg_namelen = sizeof(had_dest_addr);
	had_msg.msg_iov = &had_iov;
	had_msg.msg_iovlen = 1;
	//vrrp_syslog_info("vrrp_netlink_init end ........\n");

    return 0;
}

uint32_t had_TIMER_CLK( void )
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL );
	
	return (tv.tv_sec * VRRP_TIMER_HZ + tv.tv_usec);
}

/****************************************************************
 *	DESCRIPTION:
 *		This function get current time in timeval format
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		current  - current time in timeval format
 *
 *	RETURN:
 *		NULL
 *
 *	NOTE:
 *
 ****************************************************************/
void had_current_time(struct timeval *current)
{
	struct timeval tv;

	if(!current) {
		return;
	}
	tv.tv_sec = 0L;
	tv.tv_usec = 0L;
	
	gettimeofday(&tv, NULL);

	current->tv_sec = tv.tv_sec;
	current->tv_usec = tv.tv_usec;

	return;
}

#if 0
/* delete by jinpc@autelan.com
 * for no used
 */
/****************************************************************
 NAME	: get_pid_name				00/10/04 21:06:44
 AIM	: 
 REMARK	:
****************************************************************/
static char *pidfile_get_name( vrrp_rt *vsrv )
{
	static char pidfile[FILENAME_MAX+1];
	snprintf( pidfile, sizeof(pidfile), "%s/" VRRP_PID_FORMAT
					, PidDir
					, vsrv->uplink_vif.ifname 
					, vsrv->vrid );
	return pidfile;
}

/****************************************************************
 NAME	: pidfile_write				00/10/04 21:12:26
 AIM	: 
 REMARK	: write the pid file
****************************************************************/
int pidfile_write( vrrp_rt *vsrv )
{
	char	*name	= pidfile_get_name(vsrv);
	FILE	*fOut	= fopen( name, "w" );
	if( !fOut ){
		fprintf( stderr, "Can't open %s (errno %d %s)\n", name 
						, errno
						, strerror(errno) 
						);
		return -1;
	}
	fprintf( fOut, "%d\n", getpid() );
	fclose( fOut );
	return(0);
}

/****************************************************************
 NAME	: pidfile_rm				00/10/04 21:12:26
 AIM	: 
 REMARK	:
****************************************************************/
static void pidfile_rm( vrrp_rt *vsrv )
{
	unlink( pidfile_get_name(vsrv) );
}

/****************************************************************
 NAME	: pidfile_exist				00/10/04 21:12:26
 AIM	: return 0 if there is no valid pid in the pidfile or no pidfile
 REMARK	: 
****************************************************************/
int kill(pid_t, int);
int pidfile_exist( vrrp_rt *vsrv )
{
	char	*name	= pidfile_get_name(vsrv);
	FILE	*fIn	= fopen( name, "r" );
	pid_t	pid = 0;
	int sig = 0, ret = 0;
	
	/* if there is no file */
	if( !fIn )		return 0;
	fscanf( fIn, "%d", &pid );
	fclose( fIn );
	/* if there is no process, remove the stale file */
	if( (ret = kill( pid, sig )) ){
		fprintf(stderr, "Remove a stale pid file %s\n", name );
		pidfile_rm( vsrv );
		return 0;
	}
	/* if the kill suceed, return an error */
	return -1;
}
#endif

struct sock_list_t* had_LIST_CREATE
(
   void
)
{
    struct sock_list_t* sock_head = NULL;
	
	sock_head = (struct sock_list_t*)malloc(sizeof(struct sock_list_t));
	if(NULL == sock_head){
        vrrp_syslog_error("init socket list header memory failed!\n");
		return NULL;
	}
	sock_head->count = 0;
	sock_head->size = 16;
	sock_head->sock_fd = NULL;
	return sock_head;
}

/*
 *******************************************************************************
 *had_LIST_ADD()
 *
 *  DESCRIPTION:
 *		add socket fd about heartbeat|uplink|downlink to socklist.
 *  INPUTS:
 *		vrrp_rt* vsrv
 *
 *  OUTPUTS:
 *		struct sock_s* sock_fd		- new struct of socket fd
 *
 *  RETURN VALUE:
 *		NULL						- add faild
 *		sock_fd						- add success
 *
 *******************************************************************************
 */
struct sock_s* had_LIST_ADD
(
	vrrp_rt* vsrv
)
{
	struct sock_s* sock_fd = NULL,*tmp = NULL;
	int i = 0;

	if (!vsrv) {
		vrrp_syslog_error("open socket faild, null parameter error\n");
		return NULL;
	}
   
	sock_fd = (struct sock_s*)malloc(sizeof(struct sock_s));
	if (NULL == sock_fd) {
		vrrp_syslog_error("Add vrrp %d sock fd to sock list failed, out of memory!\n", vsrv->vrid);
		return NULL;
	}
	memset(sock_fd, 0, sizeof(struct sock_s));

	/* for heartbeat */
	if (NULL != global_ht_ifname &&
		0 != global_ht_ip)
	{
		sock_fd->sockfd = vsrv->sockfd;
	}else {
		sock_fd->sockfd = 0;
	}

	/* for uplink */
	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
		{
			sock_fd->uplink_fd[i] = vsrv->uplink_fd[i];
		}
	}

	/* for downlink */
	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
		{
			sock_fd->downlink_fd[i] = vsrv->downlink_fd[i];
		}
	}	

	sock_fd->vrid = vsrv->vrid;

	/* check if sock_list created,
	 * if not created, now create.
	 */
	if (!sock_list &&
		NULL == (sock_list = had_LIST_CREATE()))
	{
		if(sock_fd){
			free(sock_fd);
			sock_fd = NULL;
		}
		return NULL;
	}

	/* find the last positon where need add new one. */
	for (tmp = sock_list->sock_fd; tmp && tmp->next; tmp = tmp->next)
	{
		;
	}
	if (!tmp) {
		sock_list->sock_fd = sock_fd;
	}else {
		tmp->next = sock_fd;
	}
	sock_fd->next = NULL;
	sock_list->count++;
	vrrp_syslog_dbg("add vrrp %d sock_fd to sock_list success.\n", vsrv->vrid);

	/* for debug */
	vrrp_syslog_dbg("list sock fds, count %d\n", sock_list->count);
	tmp = sock_list->sock_fd;
	while (tmp)
	{
		/* for heartbeat */
		vrrp_syslog_dbg("vrid %d, heartbeat sockfd %d\n", tmp->vrid, tmp->sockfd);
		
		/* for uplink */
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
			{
				vrrp_syslog_dbg("uplink[%d] fd %d\n", i, tmp->uplink_fd[i]);
			}
		}
		
		/* for downlink */
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
			{
				vrrp_syslog_dbg("downlink[%d] fd %d\n", i, tmp->downlink_fd[i]);
			}
		}

		tmp = tmp->next;
	}

	return sock_fd;
}

/*
 *******************************************************************************
 *had_LIST_UPDATE()
 *
 *  DESCRIPTION:
 *		update socket fd about heartbeat|uplink|downlink to socklist.
 *		find the socket fd which vrid is special vrid.
 *		found it, update. not found, add new one.
 *
 *  INPUTS:
 *		vrrp_rt* vsrv
 *
 *  OUTPUTS:
 *		struct sock_s* sock_fd		- new struct of socket fd
 *
 *  RETURN VALUE:
 *		NULL						- update faild
 *		sock_fd						- update success
 *
 *******************************************************************************
 */
struct sock_s* had_LIST_UPDATE
(
	vrrp_rt* vsrv,
	unsigned int vrid
)
{
	struct sock_s* sock_fd = NULL, *tmp = NULL;
	int i = 0,is_find = 0;

	if (!vsrv) 
	{
		vrrp_syslog_error("update socket list faild, null parameter error\n");
		return NULL;
	}

	/* find the socket fd which vrid is special vrid. */
	for (sock_fd = sock_list->sock_fd; sock_fd; sock_fd = sock_fd->next)
	{
		if (sock_fd->vrid == vsrv->vrid) 
		{
			is_find = 1;
			break;
		}
	}
	/* not found, add new one. */
	#if 0
	if (NULL == sock_fd)
	#endif
	/*zhangcl modified for dead lock*/
	if(is_find == 0)
	{
		return had_LIST_ADD(vsrv);
	}

	/* found, update */
	/* for heartbeat */
	if (NULL != global_ht_ifname &&
		0 != global_ht_ip)
	{
		sock_fd->sockfd = vsrv->sockfd;
	}else {
		sock_fd->sockfd = 0;
	}

	/* for uplink */
	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
		{
			sock_fd->uplink_fd[i] = vsrv->uplink_fd[i];
		}
	}

	/* for downlink */
	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
		{
			sock_fd->downlink_fd[i] = vsrv->downlink_fd[i];
		}
	}   
	sock_fd->vrid = vrid;
	vrrp_syslog_dbg("update vrrp %d sock_fd to sock_list success.\n", vsrv->vrid);

	vrrp_syslog_dbg("list sock update, count %d\n", sock_list->count);
	tmp = sock_list->sock_fd;
	while (tmp)
	{
		/* for heartbeat */
		vrrp_syslog_dbg("vrid %d, heartbeat sockfd %d\n", tmp->vrid, tmp->sockfd);
		
		/* for uplink */
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
			{
				vrrp_syslog_dbg("uplink[%d] fd %d\n", i, tmp->uplink_fd[i]);
			}
		}
		
		/* for downlink */
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
			{
				vrrp_syslog_dbg("downlink[%d] fd %d\n", i, tmp->downlink_fd[i]);
			}
		}

		tmp = tmp->next;
	}

	return sock_fd;
}

/*
 *******************************************************************************
 *had_LIST_DEL()
 *
 *  DESCRIPTION:
 *		delete socket fd about heartbeat|uplink|downlink from socklist.
 *
 *  INPUTS:
 *		vrrp_rt* vsrv
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 * 	 	NULL
 *
 *******************************************************************************
 */
void had_LIST_DEL
(
	vrrp_rt* vsrv
)
{
	struct sock_s* sock_fd = NULL, *tmp = NULL;
	int i = 0;

	if (!vsrv) {
		vrrp_syslog_error("delete socket list faild, null parameter error\n");
		return;
	}

	/* check if sock_list is not created or haven't add sock_fd. */
	if(!sock_list || !sock_list->sock_fd){
		vrrp_syslog_dbg("sock_list is not created or haven't add sock_fd.\n");
		return;
	}

	/* find the sock_fd which vrid is special. */
	sock_fd = sock_list->sock_fd;
	tmp = sock_fd;
	while (sock_fd && sock_fd->vrid != vsrv->vrid) {
		tmp = sock_fd;
		sock_fd = sock_fd->next;
	}

	/* not found */
	if (!sock_fd) {
		vrrp_syslog_error("not found sock_fd(vrid %d) in sock_list.\n", vsrv->vrid);
		return;
	}else {
		/* delete sock_fd from sock_list */
		if (tmp == sock_fd) {//the first delete
			sock_list->sock_fd = tmp->next;
			if (NULL == tmp->next) {
				sock_list->sock_fd = NULL;
			}
		}else {
			tmp->next = sock_fd->next;
		}

		/* close fd and free memory */
		/* for heartbeat */
		if (NULL != global_ht_ifname &&
			0!= global_ht_ip)
		{
			close(sock_fd->sockfd);
		}
		/* for uplink */
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			#if 0
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) {
			#endif
			if (0 != sock_fd->uplink_fd[i]) {
				close(sock_fd->uplink_fd[i]);
			}
		}
		/* for downlink */
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			#if 0
			if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) {
			#endif
			if (0 != sock_fd->downlink_fd[i]) {
				close(sock_fd->downlink_fd[i]);
			}
		}

		free(sock_fd);
		sock_fd = NULL;
		sock_list->count--;
		vrrp_syslog_dbg("delete vrrp %d sock_fd from sock_list success.\n", vsrv->vrid);
	}

	/* for debug */
	vrrp_syslog_dbg("list sock fds, count %d\n", sock_list->count);
	tmp = sock_list->sock_fd;
	while (tmp) {
		/* for heartbeat */
		vrrp_syslog_dbg("vrid %d, heartbeat sockfd %d\n", tmp->vrid, tmp->sockfd);
		
		/* for uplink */
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
			{
				vrrp_syslog_dbg("uplink[%d] fd %d\n", i, tmp->uplink_fd[i]);
			}
		}
		
		/* for downlink */
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
			{
				vrrp_syslog_dbg("downlink[%d] fd %d\n", i, tmp->downlink_fd[i]);
			}
		}

		tmp = tmp->next;
	}

	return;
}


/*
 *******************************************************************************
 *vrrp_clear_link_fd()
 *
 *  DESCRIPTION:
 *		clear socket fd on vrrp_rt (heartbeat & uplink & downlink).
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		NULL
 *
 *******************************************************************************
 */
void vrrp_clear_link_fd
(
	vrrp_rt *vsrv
)
{
	int i = 0;
	
	if (!vsrv)
	{
		vrrp_syslog_error("clear link fd null parameter error\n");
		return;
	}

	vsrv->sockfd = 0;
	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		vsrv->uplink_fd[i] = 0;
		vsrv->downlink_fd[i] = 0;
	}

	return;
}

int vrrp_add_to_multicast
(
    vrrp_rt* vrrp,
    int uplink_ip,
    int downlink_ip
)
{
	   	struct	ip_mreq req_uplink,req_downlink;
		int ret = 0;
		vrrp_syslog_dbg("add uplink ip %d,downlinkip %x to multicast!\n",uplink_ip,downlink_ip);
	   	memset( &req_uplink, 0, sizeof (req_uplink));
		memset( &req_downlink, 0, sizeof (req_downlink));
		if((vrrp->uplink_flag)&&(0 != uplink_ip)){
			req_uplink.imr_multiaddr.s_addr = htonl(INADDR_VRRP_GROUP);
			req_uplink.imr_interface.s_addr = htonl(uplink_ip);
			//req.imr_interface.s_addr = htonl(INADDR_ANY);
			ret = setsockopt (uidSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
						   (char *) &req_uplink, sizeof (struct ip_mreq));
			if( ret < 0 ){
				vrrp_syslog_error("add uplink ip %#x to multicast error %s\n", \
									uplink_ip, strerror(errno));
				return -1;
			}  
			vrrp_syslog_dbg("add ip %#x to multicast success!\n",uplink_ip);
		}
		if((vrrp->downlink_flag)&&(0 != downlink_ip)){
			req_downlink.imr_multiaddr.s_addr = htonl(INADDR_VRRP_GROUP);
			req_downlink.imr_interface.s_addr = htonl(downlink_ip);
			//req.imr_interface.s_addr = htonl(INADDR_ANY);
			ret = setsockopt (uidSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
						   (char *) &req_downlink, sizeof (struct ip_mreq));
			if( ret < 0 ){
				vrrp_syslog_error("add downlink ip %#x to multicast error %s\n", \
								downlink_ip, strerror(errno));
				return -1;
			}  
			vrrp_syslog_dbg("add ip %x to multicast success!\n",downlink_ip);
		}
		return ret;
  
}


/****************************************************************
 NAME	: had_in_csum				00/05/10 20:12:20
 AIM	: compute a IP checksum
 REMARK	: from kuznet's iputils
****************************************************************/
static u_short had_in_csum( u_short *addr, int len, u_short csum)
{
	register int nleft = len;
	const u_short *w = addr;
	register u_short answer;
	register int sum = csum;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
		sum += htons(*(u_char *)w << 8);

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}

/****************************************************************
 NAME	: get_status_from_ifname			00/02/08 06:51:32
 AIM	:
 REMARK	:
****************************************************************/
uint32_t had_ifname_to_status( vrrp_rt* vrrp,char* ifname)
{
#if 0
	struct ifreq	ifr;
	int		fd	= socket(AF_INET, SOCK_DGRAM, 0);
	uint32_t	status	= 0;
	if (fd < 0) 	return (0);
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifr) == 0) {
        if (ifr.ifr_flags & IFF_UP)
        {
         // vrrp_syslog_event("get ifname %s status UP!\n",ifname);
		  status = IFF_UP;
        }
	}
	close(fd);
	return status;
#endif
    int status = INTERFACE_DOWN;
	int i = 0;

	if (!vrrp || !ifname) {
		vrrp_syslog_error("get interface status null paramter error\n");
		return status;
	}

	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		if ((VRRP_LINK_NO_SETTED != vrrp->uplink_flag) &&
			(VRRP_LINK_SETTED == vrrp->uplink_vif[i].set_flg) &&
			(!strncmp(vrrp->uplink_vif[i].ifname, ifname, sizeof(ifname))) &&
			(!strncmp(ifname, vrrp->uplink_vif[i].ifname, sizeof(vrrp->uplink_vif[i].ifname))))
		{
			return vrrp->uplink_vif[i].linkstate;
		}
		
		if ((VRRP_LINK_NO_SETTED != vrrp->downlink_flag) &&
			(VRRP_LINK_SETTED == vrrp->downlink_vif[i].set_flg) &&
			(!strncmp(vrrp->downlink_vif[i].ifname, ifname, sizeof(ifname))) &&
			(!strncmp(ifname, vrrp->downlink_vif[i].ifname, sizeof(vrrp->downlink_vif[i].ifname))))
		{
			return vrrp->downlink_vif[i].linkstate;
		}
		
		#if 1
		/* add by jinpc@autelan.com
		 * for check vgateway link state
		 */
	    if ((0 != vrrp->vgateway_flag) &&
			(!strcmp(vrrp->vgateway_vif[i].ifname, ifname)))
		{
	        return vrrp->vgateway_vif[i].linkstate;
		}
		#endif
	}
	return status;
}

/*
 *******************************************************************************
 *vrrp_get_link_state()
 *
 *  DESCRIPTION:
 *		get link-status of the special link type(uplink or downlink).
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,
 *		int	linkType		- type of the special link
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		unsigned int link_status
 *			 				- INTERFACE_DOWN link-down,
 *							  INTERFACE_UP link-up
 *
 *******************************************************************************
 */
unsigned int vrrp_get_link_state
(
	vrrp_rt *vsrv,
	int linkType
)
{
	unsigned int linkState = INTERFACE_DOWN;
	char ebrState = INTERFACE_DOWN;
	int i = 0;
	unsigned int linkUpNum = 0;
	unsigned int linkSetNum = 0, sensitive = 0;
	int ret = 0;

	if (!vsrv) {
		vrrp_syslog_error("get link state null parameter error\n");
		return linkState;
	}

	switch (linkType)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
					{
						linkSetNum++;
						/*
						  *    by qinhs@autelan.com 2010-4-6
						  *    ~NOTE~ 
						  * 	   non-sensitive interface is ignored when check link status, only care about 
						  *    the state-sensitive interces and take them into account when verify overall status 
						  */
						if(!had_state_sensitive_intf_check(vsrv->uplink_vif[i].ifname, &sensitive)) {
							if(strncmp(vsrv->uplink_vif[i].ifname, "ebr", 3)){
								if (!sensitive || (INTERFACE_UP == vsrv->uplink_vif[i].linkstate))
								{
									linkUpNum++;
								}
								vrrp_syslog_dbg(" interface %s not ebr, sensitive %d \n", 
									vsrv->uplink_vif[i].ifname, sensitive);
							}
							else{
								ret = had_check_ebr_state(vsrv->uplink_vif[i].ifname, &ebrState);
								if((!ret)&&(INTERFACE_UP == ebrState)){
									linkUpNum ++;
								}
								vrrp_syslog_dbg(" interface %s is ebr, ret %d linkState %d\n", 
									vsrv->uplink_vif[i].ifname, ret, ebrState);
								ebrState = INTERFACE_DOWN;
							}
						}
					}
					sensitive = 0;
				}
				
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
					{
						linkSetNum++;
						if(!had_state_sensitive_intf_check(vsrv->downlink_vif[i].ifname, &sensitive)) {  
							if(strncmp(vsrv->downlink_vif[i].ifname, "ebr", 3)){
								if (!sensitive || (INTERFACE_UP == vsrv->downlink_vif[i].linkstate))
								{
									linkUpNum++;
								}
								vrrp_syslog_dbg(" interface %s not ebr, sensitive %d \n", 
									vsrv->downlink_vif[i].ifname, sensitive);
							}
							else{
								ret = had_check_ebr_state(vsrv->downlink_vif[i].ifname, &ebrState);
								if((!ret)&&(INTERFACE_UP == ebrState)){
									linkUpNum ++;
								}
								vrrp_syslog_dbg(" interface %s is ebr, ret %d linkState %d\n", 
									vsrv->downlink_vif[i].ifname, ret, ebrState);
								ebrState = INTERFACE_DOWN;
							}
						}
					}
					sensitive = 0;
				}

				break;
			}		
		case VRRP_LINK_TYPE_VGATEWAY:
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg)
					{
						linkSetNum++;
						if(!had_state_sensitive_intf_check(vsrv->vgateway_vif[i].ifname, &sensitive)) {  
							if(strncmp(vsrv->vgateway_vif[i].ifname, "ebr", 3)){
								if (!sensitive || (INTERFACE_UP == vsrv->vgateway_vif[i].linkstate))
								{
									linkUpNum++;
								}
								vrrp_syslog_dbg(" interface %s not ebr, sensitive %d \n", 
									vsrv->vgateway_vif[i].ifname, sensitive);
							}
							else{
								ret = had_check_ebr_state(vsrv->vgateway_vif[i].ifname, &ebrState);
								if((!ret)&&(INTERFACE_UP == ebrState)){
									linkUpNum ++;
								}
								vrrp_syslog_dbg(" interface %s is ebr, ret %d linkState %d\n", 
									vsrv->vgateway_vif[i].ifname, ret, ebrState);
								ebrState = INTERFACE_DOWN;
							}
						}
					}
					sensitive = 0;
				}

				break;
			}
		case VRRP_LINK_TYPE_L2_UPLINK:
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->l2_uplink_vif[i].set_flg)
					{
						linkSetNum++;
						if(!had_state_sensitive_intf_check(vsrv->l2_uplink_vif[i].ifname, &sensitive)) {
							if(strncmp(vsrv->l2_uplink_vif[i].ifname, "ebr", 3)){
								if (!sensitive || (INTERFACE_UP == vsrv->l2_uplink_vif[i].linkstate))
								{
									linkUpNum++;
								}
								vrrp_syslog_dbg(" interface %s not ebr, sensitive %d \n", 
									vsrv->l2_uplink_vif[i].ifname, sensitive);
							}
							else{
								ret = had_check_l2_ebr_state(vsrv->l2_uplink_vif[i].ifname, &ebrState);
								if((!ret)&&(INTERFACE_UP == ebrState)){
									linkUpNum ++;
								}
								vrrp_syslog_dbg(" interface %s is ebr, ret %d linkState %d\n", 
									vsrv->l2_uplink_vif[i].ifname, ret, ebrState);
								ebrState = INTERFACE_DOWN;
							}
						}
					}
					sensitive = 0;
				}
				
				break;
			}
		default :
			{
				vrrp_syslog_error("not support link type %d.\n",
									linkType);
				linkState = INTERFACE_DOWN;
				return linkState;
			}
	}
	vrrp_syslog_dbg("linkType %d, global multi link detect %s, linkSetNum %d linkUpNum %d\n",
				linkType, global_multi_link_detect ? "ON":"OFF", linkSetNum, linkUpNum);

	/* base on value of global_multi_link_detect, set linkState */
	if (1 == global_multi_link_detect)
	{		
		if ((0 != linkSetNum) &&
			(linkSetNum == linkUpNum))
		{/*turn on multi link detect, only all linkup, return linkup */
			linkState = INTERFACE_UP;
		}
		else {
			linkState = INTERFACE_DOWN;
		}
	}else if (0 == global_multi_link_detect)
	{
		if ((0 != linkSetNum) &&
			(0 != linkUpNum))
		{/*turn off multi link detect, one linkup, return linkup */
			linkState = INTERFACE_UP;
		}
		else {
			linkState = INTERFACE_DOWN;
		}
	}

	vrrp_syslog_dbg("get %s link state: %s.\n",
					VRRP_LINK_TYPE_DESCANT(linkType),
					VRRP_LINK_STATE_DESCANT(linkState));

	return linkState;
}

/*
 *******************************************************************************
 *vrrp_get_link_max_ipaddr()
 *
 *  DESCRIPTION:
 *		get maximal ip address of the special link type(uplink or downlink).
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,
 *		int	linkType		- type of the special link
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		unsigned int max_ip_addr
 *
 *******************************************************************************
 */
unsigned int vrrp_get_link_max_ipaddr
(
	vrrp_rt *vsrv,
	int linkType
)
{
	unsigned int max_ip_addr = 0;
	int i = 0;

	if (!vsrv) {
		vrrp_syslog_error("get maximal ip address null parameter error\n");
		return max_ip_addr;
	}

	switch (linkType)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
					{
						if (max_ip_addr < vsrv->uplink_vif[i].ipaddr)
						{
							max_ip_addr = vsrv->uplink_vif[i].ipaddr;
						}
					}
				}
				
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
					{
						if (max_ip_addr < vsrv->downlink_vif[i].ipaddr)
						{
							max_ip_addr = vsrv->downlink_vif[i].ipaddr;
						}
					}
				}

				break;
			}
		default :
			{
				vrrp_syslog_error("get maximal ip address not support link type %d.\n",
									linkType);
				max_ip_addr = 0;
				return max_ip_addr;
			}
	}

	vrrp_syslog_dbg("get %s maximal ip address: %d.%d.%d.%d.\n",
					VRRP_LINK_TYPE_DESCANT(linkType),
					(max_ip_addr >> 24) & 0xFF,
					(max_ip_addr >> 16) & 0xFF,
					(max_ip_addr >> 8) & 0xFF,
					(max_ip_addr ) & 0xFF);

	return max_ip_addr;
}

/*
 *******************************************************************************
 *vrrp_is_match_ipaddr()
 *
 *  DESCRIPTION:
 *		match ip address of the special link type(uplink or downlink).
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,
 *		int	linkType		- type of the special link
 *		unsigned int ipaddr	- ip address
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		unsigned int is_match	- 1, not matched
 *								- 0, matched
 *
 *******************************************************************************
 */
unsigned int vrrp_is_match_ipaddr
(
	vrrp_rt *vsrv,
	int linkType,
	unsigned int ipaddr	
)
{	/* ismatch: 1, not matched
	 *			0, matched
	 */
	unsigned int is_match = 1;
	int i = 0;

	if (!vsrv) {
		vrrp_syslog_error("match ip address null parameter error\n");
		return is_match;
	}

	switch (linkType)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if ((VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) &&
						(ipaddr == vsrv->uplink_vif[i].ipaddr))
					{
						is_match = 0;
						break;
					}
				}
				
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if ((VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) &&
						(ipaddr == vsrv->downlink_vif[i].ipaddr))
					{
						is_match = 0;
						break;
					}
				}

				break;
			}
		default :
			{
				vrrp_syslog_error("match ip address not support link type %d.\n",
									linkType);
				is_match = 1;
				return is_match;
			}
	}

	vrrp_syslog_dbg("%s ip address %d.%d.%d.%d on %s.\n",
					is_match ? "no matched" : "matched",
					(ipaddr >> 24) & 0xFF,
					(ipaddr >> 16) & 0xFF,
					(ipaddr >> 8) & 0xFF,
					(ipaddr) & 0xFF,
					VRRP_LINK_TYPE_DESCANT(linkType));

	return is_match;
}

/*
 *******************************************************************************
 *had_ifname_to_status_byIoctl()
 *
 *  DESCRIPTION:
 *		get link-status of the special interface
 *  INPUTS:
 *		char *ifname		- name of the special interface
 *
 *  OUTPUTS:
 *		unsigned int *link_status
 *			 				- INTERFACE_DOWN link-down,
 *							  INTERFACE_UP link-up
 *
 *  RETURN VALUE:
 *		0	- success
 *		1	- fail
 *
 *******************************************************************************
 */
uint32_t had_ifname_to_status_byIoctl
(
	char *ifname,
	unsigned int *link_status
)
{
	struct ifreq ifr;
	int fd = -1;
	uint32_t status	= INTERFACE_DOWN;

	if (!ifname) {
		vrrp_syslog_error("get interface link status with null name!\n");
		return (1);
	}

	memset(&ifr, 0, sizeof(struct ifreq));
#ifndef VRRP_GLOBAL_SOCKET_FD
	fd	= socket(AF_INET, SOCK_DGRAM, 0);
#else 
	fd = vrrp_socket_inet_dgram_0_fd;
#endif
	if (fd < 0) {
		vrrp_syslog_error("get interface %s link status init socket error %s!\n",
							ifname, strerror(errno));
		return (1);
	}

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifr) == 0) {
		if(ifr.ifr_flags & IFF_UP){
			if (ifr.ifr_flags & IFF_RUNNING) {
				status = INTERFACE_UP;
				vrrp_syslog_dbg("get interface %s ifr.ifr_flags:%x",ifname, ifr.ifr_flags);
			}
			else{
	            status = INTERFACE_DOWN;
			}
		}
	}else {
		vrrp_syslog_error("get interface %s link status ioctl failed %s!\n",
							ifname, strerror(errno));
#ifndef VRRP_GLOBAL_SOCKET_FD
		close(fd);
#endif
		return (1);
	}

	*link_status = status;
	vrrp_syslog_dbg("get interface %s link status %s.!\n",
						ifname,
						(status == INTERFACE_UP) ? "up" : "down");
	
#ifndef VRRP_GLOBAL_SOCKET_FD
			close(fd);
#endif

	return (0);
}

uint32_t vrrp_get_ifname_linkstate(char* ifname)
{
    unsigned int ret = INTERFACE_DOWN;
	unsigned int ret_val = 0;
	char link_status = INTERFACE_DOWN;
	if(strncmp(ifname,"ebr",3)){
       ret_val = had_ifname_to_status_byIoctl(ifname, &ret);
	   if (ret_val != 0) {
	   		vrrp_syslog_error("get interface %s link status error %d\n", ifname, ret_val);
	   }
	}
	else{
       ret_val = had_check_ebr_state(ifname, &link_status);
	   if (ret_val != 0) {
	   		vrrp_syslog_error("get ebr interface %s link status error %d\n", ifname, ret_val);
	   }
	   ret = link_status;
	}
	return ret;
}

/*
 *******************************************************************************
 *had_ifname_to_ip()
 *
 *  DESCRIPTION:
 *		get interface ip address by ifname.
 *
 *  INPUTS:
 *		char *ifname,			- interface name
 *
 *  OUTPUTS:
 *		unsigned int *addr		- ip address
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR	- faild
 *		VRRP_RETURN_CODE_OK		- success
 *
 *******************************************************************************
 */
unsigned int had_ifname_to_ip
(
	char *ifname,
	unsigned int *addr
)
{
	struct ifreq ifr;
	struct sockaddr_in *sin = NULL;
	int fd = VRRP_FD_INIT;

	if (!ifname || !addr) {
		vrrp_syslog_error("get ip address by ifname null parameter error\n");
		return VRRP_RETURN_CODE_ERR;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
#ifndef VRRP_GLOBAL_SOCKET_FD
	fd = socket(AF_INET, SOCK_DGRAM, 0);
#else
	fd = vrrp_socket_inet_dgram_0_fd;
#endif
	if (fd < 0) {
		vrrp_syslog_error("get socket fd error\n");
		return VRRP_RETURN_CODE_ERR;
	}

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFADDR, (char *)&ifr) == 0) {
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		*addr = ntohl(sin->sin_addr.s_addr);
#ifndef VRRP_GLOBAL_SOCKET_FD
		close(fd);
#endif
		return VRRP_RETURN_CODE_OK;
	}
	
#ifndef VRRP_GLOBAL_SOCKET_FD
	close(fd);
#endif
	vrrp_syslog_error("ioctl to get interface address error\n");
	return VRRP_RETURN_CODE_ERR;
}

/*
 *******************************************************************************
 *had_ifname_to_ipv6()
 *
 *  DESCRIPTION:
 *		get interface ipv6 address by ifname.
 *
 *  INPUTS:
 *		char *ifname,			- interface name
 *
 *  OUTPUTS:
 *		struct in6_addr *addr	- ipv6 address	
                                - link local address
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR	- faild
 *		VRRP_RETURN_CODE_OK		- success
 *
 *******************************************************************************
 */
unsigned int had_ifname_to_ipv6
(
	char *ifname,
	struct in6_addr *addr
)
{
	struct ifaddrs *ifaddr, *ifa;
	int flag = 0;
	
	if (!ifname || !addr) {
		vrrp_syslog_error("get ip address by ifname null parameter error\n");
		return VRRP_RETURN_CODE_ERR;
	}
	
	if( getifaddrs(&ifaddr) == -1 ){
		vrrp_syslog_error("get ipv6 address by getifaddrs() failed!\n");
		return VRRP_RETURN_CODE_ERR;
	}
	
	for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
		if( (ifa->ifa_addr == NULL)
			||(ifa->ifa_addr->sa_family != AF_INET6)
			||(memcmp(ifa->ifa_name,ifname,strlen(ifname)+1)) ){
			continue;
		}
		memcpy(addr,&ifa->ifa_addr->sa_data[6],sizeof(struct in6_addr));
		vrrp_syslog_dbg(" ipv6 address : "NIP6QUAD_FMT"\n",NIP6QUAD(addr->s6_addr));

		flag = 1;
	}
	freeifaddrs(ifaddr);
	
	if( flag == 1 ){
		vrrp_syslog_dbg("ioctl to get interface %s ipv6 address success\n",ifname);
	}
	return VRRP_RETURN_CODE_OK;

}


/****************************************************************
 NAME	: get_dev_from_ip			00/02/08 06:51:32
 AIM	:
 REMARK	:
****************************************************************/
int had_ifname_to_idx( char *ifname )
{
	struct ifreq	ifr;
	
#ifndef VRRP_GLOBAL_SOCKET_FD
	int		fd	= socket(AF_INET, SOCK_DGRAM, 0);
#else 
	int 	fd = vrrp_socket_inet_dgram_0_fd;
#endif
	int		ifindex = -1;
	if (fd < 0) 	return (-1);
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFINDEX, (char *)&ifr) == 0){
		ifindex = ifr.ifr_ifindex;
	}
#ifndef VRRP_GLOBAL_SOCKET_FD
	close(fd);
#endif
	return ifindex;
}

/****************************************************************
 NAME	: had_rcvhwaddr_op				00/02/08 06:51:32
 AIM	:
 REMARK	:
****************************************************************/
static int had_rcvhwaddr_op( char *ifname, char *addr, int addrlen, int addF )
{
	struct ifreq ifr;
	int	fd = -1;
	int	ret = 0;
	
	if (!ifname || !addr) {
		vrrp_syslog_error("%s interface hwaddr null ptr error!\n",
				addF ? "add" : "del");
		return (-1);
	}

	memset(&ifr, 0, sizeof(struct ifreq));
#ifndef VRRP_GLOBAL_SOCKET_FD
	fd = socket(AF_INET, SOCK_DGRAM, 0);
#else
	fd = vrrp_socket_inet_dgram_0_fd;
#endif
	if (fd < 0) {
		vrrp_syslog_error("%s if %s mac %02x:%02x:%02x:%02x:%02x:%02x open socket error %s!\n",
				addF ? "add" : "del", ifname, addr[0],addr[1],addr[2],addr[3],addr[4],addr[5], strerror(errno));
		return (-1);
	}

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	memcpy( ifr.ifr_hwaddr.sa_data, addr, addrlen );
	ifr.ifr_hwaddr.sa_family = AF_UNSPEC;
	ret = ioctl(fd, addF ? SIOCADDMULTI : SIOCDELMULTI, (char *)&ifr);
	if( ret ){
		vrrp_syslog_error("Can't %s on %s error %s\n", 
				addF ? "SIOCADDMULTI" : "SIOCDELMULTI", ifname, strerror(errno));
	}
#ifndef VRRP_GLOBAL_SOCKET_FD
	close(fd);
#endif
	return ret;
}

/****************************************************************
 NAME	: had_hwaddr_set				00/02/08 06:51:32
 AIM	:
 REMARK	: linux refuse to change the hwaddress if the interface is up
****************************************************************/
static int had_hwaddr_set( char *ifname, char *addr, int addrlen )
{
	struct ifreq	ifr;
#ifndef VRRP_GLOBAL_SOCKET_FD
	int		fd	= socket(AF_INET, SOCK_DGRAM, 0);
#else
	int fd = vrrp_socket_inet_dgram_0_fd;
#endif
	int		ret;
	short flags = 0;
	if (fd < 0) 	return (-1);
	vrrp_syslog_dbg("start to change if %s mac to %02x:%02x:%02x:%02x:%02x:%02x\n",\
		ifname,addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	/* get the flags */
	ret = ioctl(fd, SIOCGIFFLAGS, (char *)&ifr);
	if( ret )	goto end2;
	flags = ifr.ifr_flags;
	/* set the interface down */
	ifr.ifr_flags &= ~IFF_UP;
	ret = ioctl(fd, SIOCSIFFLAGS, (char *)&ifr);
	if( ret )	goto end;
	/* change the hwaddr */
	memcpy( ifr.ifr_hwaddr.sa_data, addr, addrlen );
	ifr.ifr_hwaddr.sa_family = AF_UNIX;
	ret = ioctl(fd, SIOCSIFHWADDR, (char *)&ifr);
	if( ret )	goto end;
	/*before up,sleep for zebra action*/
	sleep(1);
	/* set the interface up */
	/*ifr.ifr_flags = flags;
	ret = ioctl(fd, SIOCSIFFLAGS, (char *)&ifr);
	if( ret )	goto end;*/
end:	
	ifr.ifr_flags = flags;
	ret = ioctl(fd, SIOCSIFFLAGS, (char *)&ifr);
end2:
	if( ret )	
		vrrp_syslog_error("set if %s mac to %02x:%02x:%02x:%02x:%02x:%02x error %s\n", \
				ifname, addr[0],addr[1],addr[2],addr[3],addr[4],addr[5], strerror(errno));
#ifndef VRRP_GLOBAL_SOCKET_FD
 	close(fd);
#endif
	return ret;
}

/****************************************************************
 NAME	: had_hwaddr_get				00/02/08 06:51:32
 AIM	:
 REMARK	:
****************************************************************/

/*
 *******************************************************************************
 *had_hwaddr_get()
 *
 *  DESCRIPTION:
 *		get interface mac address by ifname.
 *
 *  INPUTS:
 *		char *ifname,			- interface name
 *		int addrlen				- length of mac address
 *
 *  OUTPUTS:
 *		char *addr				- mac address
 *
 *  RETURN VALUE:
 *		-1				- faild
 *		0				- success
 *
 *******************************************************************************
 */
int had_hwaddr_get
(
	char *ifname,
	char *addr,
	int addrlen
)
{
	struct ifreq ifr;
	int fd = VRRP_FD_INIT;
	int ret = 0;

	if (!ifname || !addr) {
		vrrp_syslog_error("get interface mac, null parameter error\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
#ifndef VRRP_GLOBAL_SOCKET_FD
	fd = socket(AF_INET, SOCK_DGRAM, 0);
#else
	fd = vrrp_socket_inet_dgram_0_fd;
#endif
	if (fd < 0) {
		vrrp_syslog_error("get interface mac, get socket fd error\n");
		return (-1);
	}

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ret = ioctl(fd, SIOCGIFHWADDR, (char *)&ifr);
	if (0 != ret) {
		vrrp_syslog_error("ioctl get interface %s mac error\n");
#ifndef VRRP_GLOBAL_SOCKET_FD 
		close(fd);
#endif
		return -1;
	}
	
	memcpy(addr, ifr.ifr_hwaddr.sa_data, addrlen);
    vrrp_syslog_dbg("interface %s mac: %x:%x:%x:%x:%x:%x\n",
					ifname,
					addr[0], addr[1], addr[2],
					addr[3], addr[4], addr[5]);
#ifndef VRRP_GLOBAL_SOCKET_FD  
	close(fd);
#endif
	return ret;
}


/****************************************************************
 NAME	: had_ipaddr_ops				00/02/08 06:51:32
 AIM	:
 REMARK	:
****************************************************************/
static int had_ipaddr_ops
(
	vrrp_rt *vsrv,
	int addF
)
{
	int	i = 0, err	= 0;
	struct in_addr in;
	int uplink_ifidx = 0,downlink_ifidx = 0;

	if (!vsrv) {
		vrrp_syslog_error("modify(%s) interface ip with null pointer error!\n",  \
						addF ? "ADD":"DEL");
		return VRRP_RETURN_CODE_ERR;
	}

	/* update uplink interface */
	if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{	/* not setted, skip it */
			if (VRRP_LINK_NO_SETTED == vsrv->uplink_vif[i].set_flg) {
				continue;
			}

			/* get interface index */
			uplink_ifidx = had_ifname_to_idx(vsrv->uplink_vif[i].ifname);
			if(uplink_ifidx < 0){
				err = 1;
				vrrp_syslog_error("%s,%d,err uplink_ifidx:%d.\n",__func__,__LINE__,uplink_ifidx);
				continue;
			}
			/* get virtual ip */
			vip_addr *vadd = &vsrv->uplink_vaddr[i];
			if (!addF && !vadd->deletable) {
				continue;
			}

			if (had_ipaddr_op(uplink_ifidx, vadd->addr, vadd->mask, addF))
			{
				err = 1;
				vadd->deletable = 0;
				in.s_addr = htonl(vadd->addr);
				vrrp_syslog_error("cannot %s the address %#x to %s\n"
								, addF ? "set" : "remove"
								, in.s_addr
								, vsrv->uplink_vif[i].ifname ? vsrv->uplink_vif[i].ifname : "nil");
			}else
			{
				vrrp_syslog_dbg("%s uplink ip %#x to ifindex %d success!\n",
								addF ? "add" : "delete",
								vadd->addr,
								uplink_ifidx);
				vadd->deletable = 1;
			}
		}

		/*support ipv6 update uplink interface*/
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{	/* not setted, skip it */
			//if (VRRP_LINK_NO_SETTED == vsrv->uplink_vif[i].set_flg) {
			if( strlen(vsrv->uplink_vif[i].ifname) == 0 ){
				continue;
			}

			/* get interface index */
			uplink_ifidx = had_ifname_to_idx(vsrv->uplink_vif[i].ifname);
			if(uplink_ifidx < 0){
				err = 1;
				vrrp_syslog_error("%s,%d,err uplink_ifidx:%d.\n",__func__,__LINE__,uplink_ifidx);
				continue;
			}
			/* get virtual linklocal ipv6 */
			vipv6_addr *v6add = &vsrv->uplink_local_ipv6_vaddr[i];
			if (!addF && !v6add->deletable) {
				continue;
			}
			if(ipv6_addr_eq_null(&v6add->sin6_addr)){
            	vrrp_syslog_error("%s,%d,error.\n",__func__,__LINE__);
            	continue;
            }
			if (had_ipv6addr_op(uplink_ifidx, &v6add->sin6_addr, v6add->mask, addF))
			{
				err = 1;
				v6add->deletable = 0;
				//in.s_addr = htonl(v6add->sin6_addr);
				vrrp_syslog_error("cannot %s the uplink linklocal ip6 address "NIP6QUAD_FMT" to %s\n",
					            addF ? "set" : "remove",
					            NIP6QUAD(vsrv->uplink_local_ipv6_vaddr[i].sin6_addr.s6_addr),
					            vsrv->uplink_vif[i].ifname ? vsrv->uplink_vif[i].ifname : "nil");

			}else
			{
				vrrp_syslog_info("had_ipaddr_ops: %s uplink linklocal ip6 "NIP6QUAD_FMT" to ifindex %d success!\n",
								addF ? "add" : "delete",
								NIP6QUAD(vsrv->uplink_local_ipv6_vaddr[i].sin6_addr.s6_addr),
					            vsrv->uplink_vif[i].ifname ? vsrv->uplink_vif[i].ifname : "nil");
				v6add->deletable = 1;
				had_state_notifier_rtmd((vsrv->uplink_vif[i].ifname),addF);
			}
			
			/* get virtual ip */
			v6add = &vsrv->uplink_ipv6_vaddr[i];
			if (!addF && !v6add->deletable) {
				continue;
			}
			if(ipv6_addr_eq_null(&v6add->sin6_addr)){
            	vrrp_syslog_error("%s,%d,error.\n",__func__,__LINE__);
            	continue;
            }
			if (had_ipv6addr_op(uplink_ifidx, &v6add->sin6_addr, v6add->mask, addF))
			{
				err = 1;
				v6add->deletable = 0;
				//in.s_addr = htonl(v6add->sin6_addr);
				vrrp_syslog_error("cannot %s the address "NIP6QUAD_FMT" to %s\n",
					            addF ? "set" : "remove",
					            NIP6QUAD(vsrv->uplink_ipv6_vaddr[i].sin6_addr.s6_addr),
					            vsrv->uplink_vif[i].ifname ? vsrv->uplink_vif[i].ifname : "nil");

			}else
			{
				vrrp_syslog_info("had_ipaddr_ops: %s uplink ip6 address "NIP6QUAD_FMT" to %s\n",
					            addF ? "add" : "delete",
					            NIP6QUAD(vsrv->uplink_ipv6_vaddr[i].sin6_addr.s6_addr),
					            vsrv->uplink_vif[i].ifname ? vsrv->uplink_vif[i].ifname : "nil");

				v6add->deletable = 1;

				vrrp_syslog_info(".....%s,%d niehy ...\n",__func__,__LINE__);
				had_state_notifier_rtmd((vsrv->uplink_vif[i].ifname),addF);
				vrrp_syslog_info(".....%s,%d niehy ...\n",__func__,__LINE__);

			}
		}
	}

	/* update downlink interface */
	if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{	/* not setted, skip it */
			if (VRRP_LINK_NO_SETTED == vsrv->downlink_vif[i].set_flg) {
				continue;
			}
		
			/* get interface index */
			downlink_ifidx = had_ifname_to_idx(vsrv->downlink_vif[i].ifname);
			if(downlink_ifidx < 0){
				err = 1;
				vrrp_syslog_error("%s,%d,err downlink_ifidx:%d.\n",__func__,__LINE__,downlink_ifidx);
				continue;
			}
			/* get virtual ip */
			vip_addr *vadd = &vsrv->downlink_vaddr[i];
			if (!addF && !vadd->deletable) {
				continue;
			}
		
			if (had_ipaddr_op(downlink_ifidx, vadd->addr, vadd->mask, addF))
			{
				err = 1;
				vadd->deletable = 0;
				in.s_addr = htonl(vadd->addr);
				vrrp_syslog_error("cannot %s the address %#x to %s\n"
								, addF ? "set" : "remove"
								, in.s_addr
								, vsrv->downlink_vif[i].ifname ? vsrv->downlink_vif[i].ifname : "nil");
			}else
			{
				vrrp_syslog_dbg("%s downlink ip %#x to ifindex %d success!\n",
								addF ? "add" : "delete",
								vadd->addr,
								downlink_ifidx);
				vadd->deletable = 1;
			}
		}

		/*support ipv6 update downlink interface*/
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{	/* not setted, skip it */
			//if (VRRP_LINK_NO_SETTED == vsrv->downlink_vif[i].set_flg) {
			if( strlen(vsrv->downlink_vif[i].ifname) == 0 ){
				continue;
			}
		
			/* get interface index */
			downlink_ifidx = had_ifname_to_idx(vsrv->downlink_vif[i].ifname);
			if(downlink_ifidx < 0){
				err = 1;
				vrrp_syslog_error("%s,%d,err downlink_ifidx:%d.\n",__func__,__LINE__,downlink_ifidx);
				continue;
			}
			/* get virtual link local ip */
			vipv6_addr *v6add = &vsrv->downlink_local_ipv6_vaddr[i];
			if (!addF && !v6add->deletable) {
				continue;
			}
			if(ipv6_addr_eq_null(&v6add->sin6_addr)){
            	vrrp_syslog_error("%s,%d,error.\n",__func__,__LINE__);
            	continue;
            }
			
			if (had_ipv6addr_op(downlink_ifidx, &v6add->sin6_addr, v6add->mask, addF))
			{
				err = 1;
				v6add->deletable = 0;
				//in.s_addr = htonl(v6add->addr);
				vrrp_syslog_error("cannot %s the linklocal ip6 address "NIP6QUAD_FMT" to %s\n",
					            addF ? "set" : "remove",
					            NIP6QUAD(vsrv->downlink_local_ipv6_vaddr[i].sin6_addr.s6_addr),
					            vsrv->downlink_vif[i].ifname ? vsrv->downlink_vif[i].ifname : "nil");
			}else
			{
				vrrp_syslog_info("had_ipaddr_ops: %s downlink linklocal ip6 "NIP6QUAD_FMT" to %s\n",
					            addF ? "add" : "delete",
					            NIP6QUAD(vsrv->downlink_local_ipv6_vaddr[i].sin6_addr.s6_addr),
					            vsrv->downlink_vif[i].ifname ? vsrv->downlink_vif[i].ifname : "nil");

				v6add->deletable = 1;
				had_state_notifier_rtmd((vsrv->downlink_vif[i].ifname),addF);
			}
			
			/* get virtual ip */
			v6add = &vsrv->downlink_ipv6_vaddr[i];
			if (!addF && !v6add->deletable) {
				continue;
			}
			if(ipv6_addr_eq_null(&v6add->sin6_addr)){
            	vrrp_syslog_error("%s,%d,error.\n",__func__,__LINE__);
            	continue;
            }
			if (had_ipv6addr_op(downlink_ifidx, &v6add->sin6_addr, v6add->mask, addF))
			{
				err = 1;
				v6add->deletable = 0;
				//in.s_addr = htonl(v6add->addr);
				vrrp_syslog_error("cannot %s the address "NIP6QUAD_FMT" to %s\n",
					            addF ? "add" : "delete",
					            NIP6QUAD(vsrv->downlink_ipv6_vaddr[i].sin6_addr.s6_addr),
					            vsrv->downlink_vif[i].ifname ? vsrv->downlink_vif[i].ifname : "nil");
			}else
			{
				vrrp_syslog_info("had_ipaddr_ops: %s the downlink ip6 address "NIP6QUAD_FMT" to %s\n",
					            addF ? "add" : "delete",
					            NIP6QUAD(vsrv->downlink_ipv6_vaddr[i].sin6_addr.s6_addr),
					            vsrv->downlink_vif[i].ifname ? vsrv->downlink_vif[i].ifname : "nil");
				v6add->deletable = 1;
				had_state_notifier_rtmd((vsrv->downlink_vif[i].ifname),addF);
			}
		}
		
	}

	/*check if  v gateway set*/
	if(VRRP_LINK_NO_SETTED != vsrv->vgateway_flag){
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {	
			/* not setted, skip it */
			if (VRRP_LINK_NO_SETTED == vsrv->vgateway_vif[i].set_flg) {
				continue;
			}
			if(0 != (err = had_ipaddr_op_withmask(vsrv->vgateway_vif[i].ifindex,
										vsrv->vgateway_vaddr[i].addr,vsrv->vgateway_vaddr[i].mask,addF))){
										in.s_addr = htonl(vsrv->vgateway_vaddr[i].addr);
				vrrp_syslog_error("cannot %s the address %#x to %s\n", addF ? "add" : "delete", 
									in.s_addr, vsrv->vgateway_vif[i].ifname);
			}
	   }
	}
	return err;
}

/****************************************************************
 NAME	: vrrp_dlthd_len			00/02/02 15:16:23
 AIM	: return the vrrp header size in byte
 REMARK	:
****************************************************************/
 int vrrp_dlt_len( vrrp_rt *rt )
{
	return ETHER_HDR_LEN;	/* hardcoded for ethernet */
}

/****************************************************************
 NAME	: vrrp_iphdr_len			00/02/02 15:16:23
 AIM	: return the ip  header size in byte
 REMARK	:
****************************************************************/
static int vrrp_iphdr_len( vrrp_rt *vsrv )
{
	return sizeof( struct iphdr );
}

/****************************************************************
 NAME	: vrrp_hd_len				00/02/02 15:16:23
 AIM	: return the vrrp header size in byte
 REMARK	:
****************************************************************/
static int vrrp_uplink_hd_len( vrrp_rt *vsrv )
{
	return sizeof( vrrp_pkt ) + vsrv->uplink_naddr*sizeof(uint32_t)
						+ VRRP_AUTH_LEN;
}


/****************************************************************
 NAME	: vrrp_hd_len				00/02/02 15:16:23
 AIM	: return the vrrp header size in byte
 REMARK	:
****************************************************************/
static int vrrp_downlink_hd_len( vrrp_rt *vsrv )
{
	return sizeof( vrrp_pkt ) + vsrv->downlink_naddr*sizeof(uint32_t)
						+ VRRP_AUTH_LEN;
}

#if 0
/* delete by jinpc@autelan.com
 * for not in use
 */
/****************************************************************
 NAME	: vrrp_in_chk				00/02/02 12:54:54
 AIM	: check a incoming packet. return 0 if the pkt is valid, != 0 else
 REMARK	: rfc2338.7.1
****************************************************************/
 int vrrp_in_chk_uplink( vrrp_rt *vsrv, struct iphdr *ip )
{
	int		ihl = ip->ihl << 2;
	vrrp_pkt *	hd = (vrrp_pkt *)((char *)ip + ihl);

	/* MUST verify that the IP TTL is 255 */
	if(NULL == vsrv->uplink_vif.ifname){//uplink interface is null or not set in cmd line
        vrrp_syslog_error("vrrp %d check uplink packet but name NULL or not set!\n",vsrv->vrid);
		return 1;
	}
	if( ip->ttl != VRRP_IP_TTL ) {
		vrrp_syslog_error("check uplink packet:invalid ttl, %d and expect %d", ip->ttl,VRRP_IP_TTL);
		return 1;
	}
	/* MUST verify the VRRP version */
	if( (hd->vers_type >> 4) != VRRP_VERSION ){
		vrrp_syslog_error("check uplink packet:invalid version. %d and expect %d",(hd->vers_type >> 4), VRRP_VERSION);
		return 1;
	}
	/* MUST verify that the received packet length is greater than or
	** equal to the VRRP header */
	if( (ntohs(ip->tot_len)-ihl) <= sizeof(vrrp_pkt) ){
		vrrp_syslog_error("check uplink packet:ip payload too short. %d and expect at least %d",ntohs(ip->tot_len)-ihl, sizeof(vrrp_pkt));
		return 1;
	}
	/* WORK: MUST verify the VRRP checksum */
	if( had_in_csum( (u_short*)hd, vrrp_uplink_hd_len(vsrv), 0) ){
		vrrp_syslog_error("check uplink packet:Invalid vrrp checksum" );
		return 1;
	}

	/* MUST verify that the VRID is valid on the receiving interface */
	if( vsrv->vrid != hd->vrid ){
		vrrp_syslog_dbg("vrrp %d receive packet belong to vrrp %d\n",vsrv->vrid,hd->vrid);
		return 1;
	}

	/* MAY verify that the IP address(es) associated with the VRID are
	** valid */
	/* WORK: currently we don't */

	/* MUST verify that the Adver Interval in the packet is the same as
	** the locally configured for this virtual router */
	if( vsrv->adver_int/VRRP_TIMER_HZ != hd->adver_int ){
		vrrp_syslog_error("check uplink packet:advertisement interval mismatch mine=%d rcved=%d",vsrv->adver_int/VRRP_TIMER_HZ ,hd->adver_int);
		return 1;
	}

	/* Scott added 9-4-02 */
	//master_ipaddr_uplink = ip->saddr;
	return 0;
}


/****************************************************************
 NAME	: vrrp_in_chk				00/02/02 12:54:54
 AIM	: check a incoming packet. return 0 if the pkt is valid, != 0 else
 REMARK	: rfc2338.7.1
****************************************************************/
 int vrrp_in_chk_downlink( vrrp_rt *vsrv, struct iphdr *ip )
{
	int		ihl = ip->ihl << 2;
	vrrp_pkt *	hd = (vrrp_pkt *)((char *)ip + ihl);

	if(NULL == vsrv->downlink_vif.ifname){//downlink interface is null or not set in cmd line
        vrrp_syslog_error("vrrp %d check downlink packet but name NULL or not set!\n",vsrv->vrid);
		return 1;
	}
	/* MUST verify that the IP TTL is 255 */
	if( ip->ttl != VRRP_IP_TTL ) {
		vrrp_syslog_error("check downlink packet:invalid ttl. %d and expect %d", ip->ttl,VRRP_IP_TTL);
		return 1;
	}
	/* MUST verify the VRRP version */
	if( (hd->vers_type >> 4) != VRRP_VERSION ){
		vrrp_syslog_error("check downlink packet:invalid version. %d and expect %d",(hd->vers_type >> 4), VRRP_VERSION);
		return 1;
	}
	/* MUST verify that the received packet length is greater than or
	** equal to the VRRP header */
	if( (ntohs(ip->tot_len)-ihl) <= sizeof(vrrp_pkt) ){
		vrrp_syslog_error("check downlink packet:ip payload too short. %d and expect at least %d",ntohs(ip->tot_len)-ihl, sizeof(vrrp_pkt));
		return 1;
	}
	/* WORK: MUST verify the VRRP checksum */
	if( had_in_csum( (u_short*)hd, vrrp_downlink_hd_len(vsrv), 0) ){
		vrrp_syslog_error("check downlink packet:Invalid vrrp checksum" );
		return 1;
	}
/* MUST perform authentication specified by Auth Type */
 	/* check the authentication type */
	/* MUST verify that the VRID is valid on the receiving interface */
	if( vsrv->vrid != hd->vrid ){
		return 1;
	}

	/* MAY verify that the IP address(es) associated with the VRID are
	** valid */
	/* WORK: currently we don't */

	/* MUST verify that the Adver Interval in the packet is the same as
	** the locally configured for this virtual router */
	if( vsrv->adver_int/VRRP_TIMER_HZ != hd->adver_int ){
		vrrp_syslog_error("check downlink packet:advertisement interval mismatch mine=%d rcved=%d",vsrv->adver_int,hd->adver_int);
		return 1;
	}
	return 0;
}
#endif

/****************************************************************
 NAME	: vrrp_build_uplink_dlt			00/02/02 14:39:18
 AIM	:
 REMARK	: rfc2338.7.3
****************************************************************/
static void vrrp_build_uplink_dlt
(
	vrrp_rt *vsrv,
	char *buffer,
	int buflen,
	int index
)
{
	/* hardcoded for ethernet */
	struct ether_header *eth = NULL;
	char vrrp_hwaddr[VRRP_MAC_ADDRESS_LEN] = {0};
	char zeroMac[ETH_ALEN] = {0};
	memset(zeroMac, 0, ETH_ALEN);

	if (!vsrv || !buffer) {
		vrrp_syslog_error("build uplink ether header null parameter error\n");
		return;
	}

	eth = (struct ether_header *)buffer;
	
	if( vsrv->no_vmac ){
		if(!memcmp(vsrv->uplink_vif[index].hwaddr, zeroMac, ETH_ALEN)){
			vrrp_syslog_warning("vgateway interface %s hwaddr is zero mac when send gratuitous arp, rewrite it as vrrp global mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
				vsrv->uplink_vif[index].ifname, vrrp_global_mac[0], vrrp_global_mac[1], vrrp_global_mac[2], vrrp_global_mac[3], vrrp_global_mac[4], vrrp_global_mac[5]);
			memcpy(vsrv->uplink_vif[index].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		}
		memcpy( vrrp_hwaddr, vsrv->uplink_vif[index].hwaddr,sizeof(vsrv->uplink_vif[index].hwaddr));
	}
	else{
		vrrp_hwaddr[0] = vrrp_vmac[0];
		vrrp_hwaddr[1] = vrrp_vmac[1];
		vrrp_hwaddr[2] = vrrp_vmac[2];
		vrrp_hwaddr[3] = vrrp_vmac[3];
		vrrp_hwaddr[4] = vrrp_vmac[4];
		vrrp_hwaddr[5] = vsrv->vrid;
	}
	/* destination address --rfc1122.6.4*/
	eth->ether_dhost[0]	= vrrp_dmac[0];
	eth->ether_dhost[1]	= vrrp_dmac[1];
	eth->ether_dhost[2]	= vrrp_dmac[2];
	eth->ether_dhost[3]	= vrrp_dmac[3];
	eth->ether_dhost[4]	= vrrp_dmac[4];
	eth->ether_dhost[5]	= vrrp_dmac[5];
	/* source address --rfc2338.7.3 */
	memcpy( eth->ether_shost, vrrp_hwaddr, sizeof(vrrp_hwaddr));
	/* type */
	/*
	vrrp_syslog_packet_send("  build uplink dlt: src addr: %02x:%02x:%02x:%02x:%02x:%02x, dest addr %02x:%02x:%02x:%02x:%02x:%02x\n",\
	vrrp_hwaddr[0],vrrp_hwaddr[1],vrrp_hwaddr[2],vrrp_hwaddr[3],vrrp_hwaddr[4],vrrp_hwaddr[5],eth->ether_dhost[0],eth->ether_dhost[1],eth->ether_dhost[2],\
	eth->ether_dhost[3],eth->ether_dhost[4],eth->ether_dhost[5]);
	*/
	eth->ether_type		= htons( ETHERTYPE_IP );

	return;
}


/****************************************************************
 NAME	: vrrp_build_dlt			00/02/02 14:39:18
 AIM	:
 REMARK	: rfc2338.7.3
****************************************************************/
static void vrrp_build_downlink_dlt
(
	vrrp_rt *vsrv,
	char *buffer,
	int buflen,
	int index
)
{
	/* hardcoded for ethernet */
	struct ether_header *eth = NULL;
	char vrrp_hwaddr[VRRP_MAC_ADDRESS_LEN] = {0};
	char zeroMac[ETH_ALEN] = {0};
	memset(zeroMac, 0, ETH_ALEN);

	if (!vsrv || !buffer) {
		vrrp_syslog_error("build downlink ether header null parameter error\n");
		return;
	}

	eth = (struct ether_header *)buffer;

	if (vsrv->no_vmac) {
		if(!memcmp(vsrv->downlink_vif[index].hwaddr, zeroMac, ETH_ALEN)){
			vrrp_syslog_warning("vgateway interface %s hwaddr is zero mac when send gratuitous arp, rewrite it as vrrp global mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
				vsrv->downlink_vif[index].ifname, vrrp_global_mac[0], vrrp_global_mac[1], vrrp_global_mac[2], vrrp_global_mac[3], vrrp_global_mac[4], vrrp_global_mac[5]);
			memcpy(vsrv->downlink_vif[index].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		}
		memcpy( vrrp_hwaddr, vsrv->downlink_vif[index].hwaddr,sizeof(vsrv->downlink_vif[index].hwaddr));
	}
	else{
		vrrp_hwaddr[0] = vrrp_vmac[0];
		vrrp_hwaddr[1] = vrrp_vmac[1];
		vrrp_hwaddr[2] = vrrp_vmac[2];
		vrrp_hwaddr[3] = vrrp_vmac[3];
		vrrp_hwaddr[4] = vrrp_vmac[4];
		vrrp_hwaddr[5] = vsrv->vrid;
	}
	/* destination address --rfc1122.6.4*/
	eth->ether_dhost[0]	= vrrp_dmac[0];
	eth->ether_dhost[1]	= vrrp_dmac[1];
	eth->ether_dhost[2]	= vrrp_dmac[2];
	eth->ether_dhost[3]	= vrrp_dmac[3];
	eth->ether_dhost[4]	= vrrp_dmac[4];
	eth->ether_dhost[5]	= vrrp_dmac[5];
	/* source address --rfc2338.7.3 */
	memcpy( eth->ether_shost, vrrp_hwaddr, sizeof(vrrp_hwaddr));
	/* type */
	eth->ether_type		= htons( ETHERTYPE_IP );

	return;
}

/****************************************************************
 NAME	: vrrp_build_ip				00/02/02 14:39:18
 AIM	: build a ip packet
 REMARK	:
****************************************************************/
static void vrrp_build_uplink_ip
(
	vrrp_rt *vsrv,
	char *buffer,
	int buflen,
	int index
)
{
	struct iphdr *ip = NULL;

	if (!vsrv || !buffer) {
		vrrp_syslog_error("build uplink ip header null parameter error\n");
		return;
	}

	ip = (struct iphdr *)(buffer);
	ip->ihl		= 5;
	ip->version	= 4;
	ip->tos		= 0;
	ip->tot_len	= ip->ihl*4 + vrrp_uplink_hd_len( vsrv );
	ip->tot_len	= htons(ip->tot_len);
	ip->id		= ++uplink_ip_id;
	ip->frag_off	= 0;
	ip->ttl		= VRRP_IP_TTL;
	ip->protocol	= IPPROTO_VRRP;
	/*
	if(NULL != global_ht_ifname && 0 != global_ht_ip){
		ip->saddr	= htonl(global_ht_ip);
	}
	else{
		ip->saddr	= htonl(vsrv->uplink_vif.ipaddr);
	}
	*/
	
	ip->saddr	= htonl(vsrv->uplink_vif[index].ipaddr);
	ip->daddr	= htonl(INADDR_VRRP_GROUP);
	/* checksum must be done last */
	ip->check	= had_in_csum( (u_short*)ip, ip->ihl*4, 0 );

	//vrrp_syslog_packet_send(" uplink src ip %x,dest ip %x\n",ip->saddr,ip->daddr);
	return;
}

/****************************************************************
 NAME	: vrrp_build_ip				00/02/02 14:39:18
 AIM	: build a ip packet
 REMARK	:
****************************************************************/
static void vrrp_build_downlink_ip
(
	vrrp_rt *vsrv,
	char *buffer,
	int buflen,
	int index
)
{
	struct iphdr *ip = NULL;

	if (!vsrv || !buffer) {
		vrrp_syslog_error("build downlink ip header null parameter error\n");
		return;
	}

	
	ip = (struct iphdr *)(buffer);
	ip->ihl		= 5;
	ip->version	= 4;
	ip->tos		= 0;
	ip->tot_len	= ip->ihl*4 + vrrp_downlink_hd_len( vsrv );
	ip->tot_len	= htons(ip->tot_len);
	ip->id		= ++downlink_ip_id;
	ip->frag_off	= 0;
	ip->ttl		= VRRP_IP_TTL;
	ip->protocol	= IPPROTO_VRRP;
	/*
	if(NULL != global_ht_ifname && 0 != global_ht_ip){
		ip->saddr	= htonl(global_ht_ip);
	}
	else{
		ip->saddr	= htonl(vsrv->downlink_vif.ipaddr);
	}
	*/
	ip->saddr	= htonl(vsrv->downlink_vif[index].ipaddr);

	ip->daddr	= htonl(INADDR_VRRP_GROUP);
	/* checksum must be done last */
	ip->check	= had_in_csum( (u_short*)ip, ip->ihl*4, 0 );
	
	//vrrp_syslog_packet_send(" downlink src ip %x,dest ip %x\n",ip->saddr,ip->daddr);
	return;
}


/****************************************************************
 NAME	: vrrp_build_vrrp			00/02/02 14:39:18
 AIM	:
 REMARK	:
****************************************************************/
static int vrrp_build_uplink_vrrp
(
	vrrp_rt *vsrv,
	int prio,
	char *buffer,
	int buflen,
	int index
)
{
	int	i = 0;
	int j = 0;
	vrrp_if	 *vif = NULL;
	vrrp_pkt *hd = NULL;
	uint32_t *iparr = NULL;
	struct timeval tv;

	if (!vsrv || !buffer) {
		vrrp_syslog_error("build uplink vrrp load null parameter error\n");
		return 1;
	}

	vif	= &vsrv->uplink_vif[index];
	hd = (vrrp_pkt *)buffer;
	iparr = (uint32_t *)((char *)hd+sizeof(*hd));
	
	hd->vers_type	= (VRRP_VERSION<<4) | VRRP_PKT_ADVERT;
	hd->vrid	= vsrv->vrid;
	hd->priority	= prio;
	hd->naddr	= vsrv->uplink_naddr;
	hd->auth_type	= vsrv->uplink_vif[index].auth_type;
	hd->adver_int	= vsrv->adver_int/VRRP_TIMER_HZ;
	hd->updown_flag = VRRP_PKT_VIA_UPLINK;/*1 means uplink*/
	hd->state = vsrv->state;

	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag){
		/*
		if((NULL != global_ht_ifname && 0 != global_ht_ip)){
			hd->uplink_ip = htonl(global_ht_ip);
		}
		else{
	        hd->uplink_ip = htonl(vsrv->uplink_vif.ipaddr);
		}
		*/
		hd->uplink_ip = htonl(vsrv->uplink_vif[index].ipaddr);
	}else {
		hd->uplink_ip = htonl(0);
	}
	/*
	if((NULL != global_ht_ifname && 0 != global_ht_ip)){
	    hd->downlink_ip = htonl(global_ht_ip);
      }
	else{
		hd->downlink_ip =htonl(vsrv->downlink_vif.ipaddr);
	}
	*/
	if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag ) {
		hd->downlink_ip = htonl(vrrp_get_link_max_ipaddr(vsrv, VRRP_LINK_TYPE_DOWNLINK));
	}else {
		hd->downlink_ip = htonl(0);
	}
	/*transfer heartbeat link ip */	
	if((NULL != global_ht_ifname) && (0 != global_ht_ip)){
	    hd->heartbeatlinkip = htonl(global_ht_ip);
    }	
	if(VRRP_AUTH_FAIL == hd->auth_type){
	   hd->old_master_ip = master_ipaddr_uplink[vsrv->vrid];
	}
	/* copy the ip addresses */
	j = 0;
	for( i = 0; i < VRRP_LINK_MAX_CNT; i++ ){
		if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) {
			iparr[j] = htonl( vsrv->uplink_vaddr[i].addr );
			j++;
		}
	}
	/* copy the passwd if the authentication is VRRP_AH_PASS */
	if( vif->auth_type == VRRP_AUTH_PASS ){
		char	*pw	= (char *)hd+sizeof(*hd)+vsrv->uplink_naddr*4;
		memcpy( pw, vif->auth_data, sizeof(vif->auth_data));
	}
	/* Must perform the checksum AFTER we copy the password */
	hd->chksum	= had_in_csum( (u_short*)hd, vrrp_uplink_hd_len(vsrv), 0);

	if((time_synchronize_enable)&&(1 == vsrv->que_f)){
	   gettimeofday(&tv,NULL);
	   hd->que_f = 1;
	   hd->tv.tv_sec = tv.tv_sec;
	   hd->tv.tv_usec = tv.tv_usec;
	}
	else{
       hd->que_f = 0;
	   hd->tv.tv_sec = 0;
	   hd->tv.tv_usec = 0;
	}
	//vrrp_syslog_packet_send(" uplink  send packet: vrid %d,priority %d,naddr %d,adver_init %d,old master ip %x,ipaddr %x\n",hd->vrid,hd->priority,hd->naddr,hd->adver_int,hd->old_master_ip,iparr[0]);
    
	return(0);
}



/****************************************************************
 NAME	: vrrp_build_vrrp			00/02/02 14:39:18
 AIM	:
 REMARK	:
****************************************************************/
static int vrrp_build_downlink_vrrp
(
	vrrp_rt *vsrv,
	int prio,
	char *buffer,
	int buflen,
	int index
)
{
	int	i = 0;
	int j = 0;
	vrrp_if	 *vif = NULL;
	vrrp_pkt *hd = NULL;
	uint32_t *iparr = NULL;
	struct timeval tv;

	if (!vsrv || !buffer) {
		vrrp_syslog_error("build uplink vrrp load null parameter error\n");
		return 1;
	}

	vif	= &vsrv->downlink_vif[index];
	hd = (vrrp_pkt *)buffer;
	iparr = (uint32_t *)((char *)hd+sizeof(*hd));
	
	hd->vers_type	= (VRRP_VERSION<<4) | VRRP_PKT_ADVERT;
	hd->vrid	= vsrv->vrid;
	hd->priority	= prio;
	hd->naddr	= vsrv->downlink_naddr;
	hd->auth_type	= vsrv->downlink_vif[index].auth_type;
	hd->adver_int	= vsrv->adver_int/VRRP_TIMER_HZ;	
	hd->updown_flag = VRRP_PKT_VIA_DOWNLINK;/*2 means downlink*/
	hd->state = vsrv->state;
	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag){
		/*
		if((NULL != global_ht_ifname && 0 != global_ht_ip)){
			hd->uplink_ip = htonl(global_ht_ip);
		}
		else{
	        hd->uplink_ip = htonl(vsrv->uplink_vif.ipaddr);
		}
		*/
		#if 0
		/* delete by jinpc@autelan.com */
		hd->uplink_ip = htonl(vsrv->uplink_vif.ipaddr);
		#endif
		hd->uplink_ip = htonl(vrrp_get_link_max_ipaddr(vsrv, VRRP_LINK_TYPE_UPLINK));	
	}else {
		hd->uplink_ip = htonl(0);
	}
	
	/*
	if((NULL != global_ht_ifname && 0 != global_ht_ip)){
	    hd->downlink_ip = htonl(global_ht_ip);
      }
	else{
		hd->downlink_ip =htonl(vsrv->downlink_vif.ipaddr);
	}	
	*/
	if (0 != vsrv->downlink_flag) {
		hd->downlink_ip =htonl(vsrv->downlink_vif[index].ipaddr);
	}else {
		hd->downlink_ip = htonl(0);
	}
	
	/*transfer heartbeat link ip */
	if((NULL != global_ht_ifname) && (0 != global_ht_ip)){
	    hd->heartbeatlinkip = htonl(global_ht_ip);
    }		
	if(VRRP_AUTH_FAIL == hd->auth_type){
	   hd->old_master_ip = master_ipaddr_downlink[vsrv->vrid];
	}
	/* copy the ip addresses */
	j = 0;
	for( i = 0; i < vsrv->downlink_naddr; i++ ){
		if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) {
			iparr[j] = htonl( vsrv->downlink_vaddr[i].addr );
			j++;
		}
	}
	/* copy the passwd if the authentication is VRRP_AH_PASS */
	if( vif->auth_type == VRRP_AUTH_PASS ){
		char	*pw	= (char *)hd+sizeof(*hd)+vsrv->downlink_naddr*4;
		memcpy( pw, vif->auth_data, sizeof(vif->auth_data));
	}
	/* Must perform the checksum AFTER we copy the password */
	hd->chksum	= had_in_csum( (u_short*)hd, vrrp_downlink_hd_len(vsrv), 0);
	if((time_synchronize_enable)&&(1 == vsrv->que_f)){
	   gettimeofday(&tv,NULL);
	   hd->que_f = 1;
	   hd->tv.tv_sec = tv.tv_sec;
	   hd->tv.tv_usec = tv.tv_usec;
	}
	else{
       hd->que_f = 0;
	   hd->tv.tv_sec = 0;
	   hd->tv.tv_usec = 0;
	}
	return(0);
}

/****************************************************************
 NAME	: vrrp_set_uplink_ptk				00/02/02 13:33:32
 AIM	: build a advertissement packet
 REMARK	:
****************************************************************/
static void vrrp_build_uplink_pkt
(
	vrrp_rt *vsrv,
	int prio,
	char *buffer,
	int buflen,
	int index
)
{
	if (!vsrv || !buffer) {
		vrrp_syslog_error("build uplink packet null parameter error\n");
		return;
	}

	//	printf("dltlen=%d iplen=%d", vrrp_dlt_len(vsrv), vrrp_iphdr_len(vsrv) );
	/* build the ethernet header */
	vrrp_build_uplink_dlt( vsrv, buffer, buflen, index);
	buffer += vrrp_dlt_len(vsrv);
	buflen -= vrrp_dlt_len(vsrv);
	
	/* build the ip header */
	vrrp_build_uplink_ip( vsrv, buffer, buflen, index);
	buffer += vrrp_iphdr_len(vsrv);
	buflen -= vrrp_iphdr_len(vsrv);
	
	/* build the vrrp header */
	vrrp_build_uplink_vrrp( vsrv, prio, buffer, buflen, index);

	return;
}


/****************************************************************
 NAME	: vrrp_set_ptk				00/02/02 13:33:32
 AIM	: build a advertissement packet
 REMARK	:
****************************************************************/
static void vrrp_build_downlink_pkt
(
	vrrp_rt *vsrv,
	int prio,
	char *buffer,
	int buflen,
	int index
)
{
	if (!vsrv || !buffer) {
		vrrp_syslog_error("build downlink packet null parameter error\n");
		return;
	}

	//	printf("dltlen=%d iplen=%d", vrrp_dlt_len(vsrv), vrrp_iphdr_len(vsrv) );
	/* build the ethernet header */
	vrrp_build_downlink_dlt( vsrv, buffer, buflen, index);
	buffer += vrrp_dlt_len(vsrv);
	buflen -= vrrp_dlt_len(vsrv);

	/* build the ip header */
	vrrp_build_downlink_ip( vsrv, buffer, buflen, index);
	buffer += vrrp_iphdr_len(vsrv);
	buflen -= vrrp_iphdr_len(vsrv);

	/* build the vrrp header */
	vrrp_build_downlink_vrrp( vsrv, prio, buffer, buflen, index);

	return;
}



/****************************************************************
 NAME	: vrrp_send_pkt				00/02/06 16:37:10
 AIM	:
 REMARK	:
****************************************************************/
int vrrp_send_pkt( vrrp_rt* vsrv,char* ifname, char *buffer, int buflen )
{
	struct sockaddr from;
	int	len;
	if((NULL != global_ht_ifname) && (0 != global_ht_ip)/*&&(global_ht_state)*/){
        if(!global_ht_state){
           vrrp_syslog_dbg("hearbeat link is down,do not send msg!\n");
		   return 0;
	    }
		memset( &from, 0 , sizeof(from));
		strcpy( from.sa_data,global_ht_ifname);
		vrrp_syslog_packet_send("ifname %s send packet\n",global_ht_ifname);		
	}
	else if(had_ifname_to_status(vsrv,ifname)){
		/*
		vrrp_syslog_dbg("create socket fd %d to send packet from intf %s\n",fd,ifname);
	      */
		/* build the address */
		memset( &from, 0 , sizeof(from));
		strcpy( from.sa_data, ifname );
		vrrp_syslog_packet_send("ifname %s send packet\n",ifname);				
	}
	else{
         vrrp_syslog_dbg("interface is down,do not send packets!\n");
		 return 0;
	}
	len = sendto( sendsock, buffer, buflen, 0, &from, sizeof(from) );
	if(len<0){
		vrrp_syslog_error("%s,%d,sendto fail!\n",__func__,__LINE__);
	}
	return len;
}


/****************************************************************
 NAME	: vrrp_send_pkt				00/02/06 16:37:10
 AIM	:
 REMARK	:
****************************************************************/
int vrrp_send_pkt_directly( vrrp_rt* vsrv,char* ifname, char *buffer, int buflen )
{
	struct sockaddr from;
	int	len;
    if(had_ifname_to_status(vsrv,ifname)){
		/*
		vrrp_syslog_dbg("create socket fd %d to send packet from intf %s\n",fd,ifname);
	      */
		/* build the address */
		memset( &from, 0 , sizeof(from));
		strcpy( from.sa_data, ifname );
		vrrp_syslog_packet_send("ifname %s send packet\n",ifname);				
	}
	else{
         vrrp_syslog_dbg("interface is down,do not send packets!\n");
		 return 0;
	}
	len = sendto( sendsock, buffer, buflen, 0, &from, sizeof(from) );
	return len;
}



/****************************************************************
 NAME	: vrrp_send_pkt				00/02/06 16:37:10
 AIM	:
 REMARK	:
****************************************************************/
int vrrp_send_pkt_arp( char* ifname, char *buffer, int buflen )
{
	struct sockaddr from;
	struct timeval tv;
	int	len;
	/*delay timer set*/
	tv.tv_sec = 0;
	tv.tv_usec = 100;
#if 0 
	/*change to global fd*/
	int	fd = socket(PF_PACKET, SOCK_PACKET, 0x300); /* 0x300 is magic */

	if( fd < 0 ){
		vrrp_syslog_error("send arp to interface %s error, create socket failed!\n",
								ifname);
		return -1;
	}
#endif
	if( vrrp_arp_send_fd < 0 ){
		vrrp_syslog_error("send arp to interface %s error, create socket failed!\n",
								ifname);
		return -1;
	}

	/*
	vrrp_syslog_dbg("create socket fd %d to send packet from intf %s\n",fd,ifname);
      */
	/* build the address */
	memset( &from, 0 , sizeof(from));
	strcpy( from.sa_data, ifname );

	/* send the data continuely five */
	/*delay timer*/
    	select(0,NULL,NULL,NULL,&tv);
	len = sendto(vrrp_arp_send_fd, buffer, buflen, 0, &from, sizeof(from) );
    	vrrp_syslog_packet_send("ifname %s send arp return value=%d\n",ifname,len);
#if 0
  	close(fd);
#endif
	return len;
}


/****************************************************************
 NAME	: vrrp_send_uplink_adv				00/02/06 16:31:24
 AIM	:
 REMARK	:
****************************************************************/
static int vrrp_send_uplink_adv
(
	vrrp_rt *vsrv,
	int prio
)
{
	int	buflen = 0;
	int ret = 0;
	int i = 0;
	char *buffer = NULL;
	
	if (!vsrv) {
		vrrp_syslog_error("uplink send adv null parameter error\n");
		return -1;
	}

	/* alloc the memory */
	buflen = vrrp_dlt_len(vsrv) + vrrp_iphdr_len(vsrv) + vrrp_uplink_hd_len(vsrv);
	//  vrrp_syslog_dbg("uplink buflen: vrrp dlt %d,vrrp iphdr %d,vrrp hd len %d\n",
	//	vrrp_dlt_len(vsrv) , vrrp_iphdr_len(vsrv) ,vrrp_uplink_hd_len(vsrv));

	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
		{
			buffer = calloc( buflen, 1 );
			if (NULL == buffer) {
				vrrp_syslog_error("calloc memory for uplink send packets buff failed!\n");
				return -1;
			}
			/* build the packet  */
			vrrp_build_uplink_pkt( vsrv, prio, buffer, buflen, i);

			/* send it */
			if (VRRP_AUTH_FAIL == vsrv->uplink_vif[i].auth_type) {
				had_vrrp_vif_rcv_pck_ON(vsrv);
				ret = vrrp_send_pkt_directly(vsrv,vsrv->uplink_vif[i].ifname, buffer, buflen );
			}
			else {
				ret = vrrp_send_pkt(vsrv,vsrv->uplink_vif[i].ifname, buffer, buflen );
			}

			/* build the memory */
			free( buffer );
		}
	}

	return ret;
}



/****************************************************************
 NAME	: vrrp_send_downlink_adv				00/02/06 16:31:24
 AIM	:
 REMARK	:
****************************************************************/
static int vrrp_send_downlink_adv
(
	vrrp_rt *vsrv,
	int prio
)
{
	int	buflen = 0;
	int ret = 0;
	int i = 0;
	char *buffer = NULL;
	
	if (!vsrv) {
		vrrp_syslog_error("downlink send adv null parameter error\n");
		return -1;
	}

	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
		{
			/* alloc the memory */
			buflen = vrrp_dlt_len(vsrv) + vrrp_iphdr_len(vsrv) + vrrp_downlink_hd_len(vsrv);
			// vrrp_syslog_dbg("downlink buflen: vrrp dlt %d,vrrp iphdr %d,vrrp hd len %d\n",
			//	vrrp_dlt_len(vsrv) , vrrp_iphdr_len(vsrv) ,vrrp_downlink_hd_len(vsrv));

			buffer = calloc( buflen, 1 );
			if(NULL == buffer){
				vrrp_syslog_error("calloc memory for downlink send packets buff failed!\n");
				return -1;
			}

			/* build the packet  */
			vrrp_build_downlink_pkt( vsrv, prio, buffer, buflen, i);
			/* send it */
			if (VRRP_AUTH_FAIL == vsrv->downlink_vif[i].auth_type) {
				had_vrrp_vif_rcv_pck_ON(vsrv);
				ret = vrrp_send_pkt_directly(vsrv,vsrv->downlink_vif[i].ifname, buffer, buflen );
			}
			else {
				ret = vrrp_send_pkt(vsrv,vsrv->downlink_vif[i].ifname, buffer, buflen );
			}

			/* build the memory */
			free( buffer );
		}
	}
	
	return ret;
}

/****************************************************************
 NAME	: vrrp_send_adv				00/02/06 16:31:24
 AIM	:
 REMARK	:
****************************************************************/
int vrrp_send_adv
(
	vrrp_rt *vsrv,
	int prio
)
{
	int	ret = 0;

	if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
		(VRRP_LINK_NO_SETTED == vsrv->downlink_flag))
	{
		vrrp_syslog_dbg("uplink and downlink no interface,return!\n");
		return  ret;
	}
	
	if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
		(vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_UPLINK)))
	{
		vrrp_send_uplink_adv(vsrv,prio);
	}

	if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
		(vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_DOWNLINK)))
	{
		vrrp_send_downlink_adv(vsrv,prio);
	}

	vrrp_syslog_dbg("advertisement send over!\n");
	return ret;
}


/****************************************************************
 NAME	: had_usage					00/02/06 08:50:28
 AIM	: display the usage
 REMARK	:
****************************************************************/
void had_usage( void )
{
	fprintf( stderr, "vrrpd version %s\n", VRRPD_VERSION );
	fprintf( stderr, "Usage: vrrpd -i ifname -v vrid [-f piddir] [-s] [-a auth] [-p prio] [-nh] ipaddr\n" );
	fprintf( stderr, "  -h       : display this short inlined help\n" );
	fprintf( stderr, "  -n       : Dont handle the virtual mac address\n" );
	fprintf( stderr, "  -i ifname: the interface name to run on\n" );
	fprintf( stderr, "  -v vrid  : the id of the virtual server [1-255]\n" );
	fprintf( stderr, "  -s       : Switch the preemption mode (%s by default)\n"
				, VRRP_PREEMPT_DFL? "Enabled" : "Disabled" );
	fprintf( stderr, "  -a auth  : (not yet implemented) set the authentification type\n" );
	fprintf( stderr, "             auth=(none|pass/hexkey|ah/hexkey) hexkey=0x[0-9a-fA-F]+\n");
	fprintf( stderr, "  -p prio  : Set the priority of this host in the virtual server (dfl: %d)\n"
							, VRRP_PRIO_DFL );
	fprintf( stderr, "  -f piddir: specify the directory where the pid file is stored (dfl: %s)\n"
							, VRRP_PIDDIR_DFL );
	fprintf( stderr, "  -d delay : Set the advertisement interval (in sec) (dfl: %d)\n"
							, VRRP_ADVER_DFL );
	fprintf( stderr, "  ipaddr   : the ip address(es) of the virtual server\n" );
}

static void vrrp_set_wid_bathsyn_start
(
   int profile
)
{

   wid_transfer_state[profile] = 1;
}

static void vrrp_set_portal_bathsyn_start
(
   int profile
)
{

   portal_transfer_state[profile] = 1;
}
	
void vrrp_state_trace
(
  int profile,
  int state,
  char* action,
  char* step
)
{
   struct state_trace* log = NULL;
   if((NULL == TRACE_LOG) || (NULL == (log = TRACE_LOG[profile]))){
      return;
   }

   if(!action ||!step) {
		vrrp_syslog_error("hansi %d state trace %s -> %s null pointer error\n",  \
					profile, VRRP_STATE_DESCRIPTION(log->state), VRRP_STATE_DESCRIPTION(state));
		return;
   }
   
   memset(log->action,0,128);
   memset(log->step,0,128);
   strncpy(log->action,action,strlen(action));
   strncpy(log->step,step,strlen(step));
   if(log->state != state) {
   		vrrp_syslog_event("hansi %d state trace %s->%s action %s %s\n",
   					profile, VRRP_STATE_DESCRIPTION(log->state), VRRP_STATE_DESCRIPTION(state),
					log->action, log->step);
   }else{
	   vrrp_syslog_dbg("hansi %d state trace %s->%s action %s %s\n",
				   profile, VRRP_STATE_DESCRIPTION(log->state), VRRP_STATE_DESCRIPTION(state),
				   log->action, log->step);
   }
   log->state = state;

   return;
}
/****************************************************************
 NAME	: had_cfg_add_uplink_ipaddr			00/02/06 09:24:08
 AIM	:
 REMARK	:
****************************************************************/
static void had_cfg_add_uplink_ipaddr
(
	vrrp_rt *vsrv,
	uint32_t ipaddr,
	uint32_t mask,
	int index
)
{
#if 0
	vsrv->uplink_naddr++;
	/* alloc the room */
	if( vsrv->uplink_vaddr ){
		vsrv->uplink_vaddr = realloc( vsrv->uplink_vaddr
					, vsrv->uplink_naddr*sizeof(*vsrv->uplink_vaddr) );
	} else {
		vsrv->uplink_vaddr = malloc( sizeof(*vsrv->uplink_vaddr) );
	}
	//assert( vsrv->vaddr );
	if(NULL == vsrv->uplink_vaddr){
       return;
	}
	/* store the data */
	vsrv->uplink_vaddr[vsrv->uplink_naddr-1].addr		= ipaddr;
	vsrv->uplink_vaddr[vsrv->uplink_naddr-1].mask		= mask;
	vsrv->uplink_vaddr[vsrv->uplink_naddr-1].deletable	= 0;
	return;
#endif
#if 1
	if (!vsrv) {
		vrrp_syslog_error("add uplink ip address null parameter error\n");
		return;
	}

	vsrv->uplink_naddr++;

	/* store the data */
	vsrv->uplink_vaddr[index].addr      = ntohl(ipaddr);
	vsrv->uplink_vaddr[index].mask      = mask;
	vsrv->uplink_vaddr[index].deletable = 0;
	return;
#endif
}

/****************************************************************
 NAME	: had_cfg_add_uplink_ipv6addr			2014-07-03 15:14:00
 AIM	:
 REMARK	:
****************************************************************/
static void had_cfg_add_uplink_ipv6addr
(
	vrrp_rt *vsrv,
	struct iaddr *ipv6addr,
	uint32_t mask,
	int index,
	int islinklocal
)
{
	if (!vsrv) {
		vrrp_syslog_error("add uplink ip address null parameter error\n");
		return;
	}

	/* store the data */
	if( islinklocal ){
		vsrv->uplink_ipv6_naddr++;	/*reacord sum for link local addr*/
		memcpy(vsrv->uplink_local_ipv6_vaddr[index].sin6_addr.s6_addr,ipv6addr->iabuf,sizeof(char)*16);
		vsrv->uplink_local_ipv6_vaddr[index].mask      = mask;
		vsrv->uplink_local_ipv6_vaddr[index].deletable = 0;
	}
	else{
		memcpy(vsrv->uplink_ipv6_vaddr[index].sin6_addr.s6_addr,ipv6addr->iabuf,sizeof(char)*16);
		vsrv->uplink_ipv6_vaddr[index].mask      = mask;
		vsrv->uplink_ipv6_vaddr[index].deletable = 0;
	}
	return;
}

/****************************************************************
 NAME	: had_cfg_add_downlink_ipaddr			00/02/06 09:24:08
 AIM	:
 REMARK	:
****************************************************************/
static void had_cfg_add_downlink_ipaddr
(
	vrrp_rt *vsrv,
	uint32_t ipaddr,
	uint32_t mask,
	int index
)
{
#if 0
	vsrv->downlink_naddr++;
	/* alloc the room */
	if( vsrv->downlink_vaddr ){
		vsrv->downlink_vaddr = realloc( vsrv->downlink_vaddr
					, vsrv->downlink_naddr*sizeof(*vsrv->downlink_vaddr) );
	} else {
		vsrv->downlink_vaddr = malloc( sizeof(*vsrv->downlink_vaddr) );
	}
	//assert( vsrv->vaddr );
	if(NULL == vsrv->downlink_vaddr){
       return;
	}
	/* store the data */
	vsrv->downlink_vaddr[vsrv->downlink_naddr-1].addr		= ipaddr;
	vsrv->downlink_vaddr[vsrv->downlink_naddr-1].mask		= mask;
	vsrv->downlink_vaddr[vsrv->downlink_naddr-1].deletable	= 0;
	return;
#endif
#if 1
	if (!vsrv) {
		vrrp_syslog_error("add downlink ip address null parameter error\n");
		return;
	}

	vsrv->downlink_naddr++;

	/* store the data */
	vsrv->downlink_vaddr[index].addr      = ntohl(ipaddr);
	vsrv->downlink_vaddr[index].mask      = mask;
	vsrv->downlink_vaddr[index].deletable = 0;
	return;
#endif
}

/****************************************************************
 NAME	: had_cfg_add_downlink_ipv6addr			00/02/06 09:24:08
 AIM	:
 REMARK	:
****************************************************************/
static void had_cfg_add_downlink_ipv6addr
(
	vrrp_rt *vsrv,
	struct iaddr *ipv6addr,
	uint32_t mask,
	int index,
	int islinklocal
)
{
	if (!vsrv) {
		vrrp_syslog_error("add downlink ip address null parameter error\n");
		return;
	}


	/* store the data */
	if( islinklocal){
		vsrv->downlink_ipv6_naddr++;	/*record sum for link local addr*/
		memcpy(vsrv->downlink_local_ipv6_vaddr[index].sin6_addr.s6_addr,ipv6addr->iabuf,sizeof(char)*16);
		vsrv->downlink_local_ipv6_vaddr[index].mask = mask;
		vsrv->downlink_local_ipv6_vaddr[index].deletable = 0;
	}
	else{
		memcpy(vsrv->downlink_ipv6_vaddr[index].sin6_addr.s6_addr,ipv6addr->iabuf,sizeof(char)*16);
		vsrv->downlink_ipv6_vaddr[index].mask = mask;
		vsrv->downlink_ipv6_vaddr[index].deletable = 0;
	}
	return;

}

/*
 *******************************************************************************
 *had_cfg_delete_uplink_ipaddr()
 *
 *  DESCRIPTION:
 *		delete virtual ip of uplink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int ipaddr,			- virtual ip
 *		unsigned int mask,				- mask
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		NULL
 *
 *******************************************************************************
 */
void had_cfg_delete_uplink_ipaddr
(
	vrrp_rt *vsrv,
	unsigned int ipaddr,
	unsigned int mask,
	int index
)
{
	#if 0
	unsigned int i = 0;
	unsigned int j = 0;
	vip_addr *addr = NULL;

	if (!vsrv) {
		vrrp_syslog_error("delete uplink ip address null parameter error\n");
		return;
	}

	/* check if the virtual ip exist. */
	if (VRRP_RETURN_CODE_VIP_NOT_EXIST == had_check_link_vip_exist(vsrv, ipaddr, mask, VRRP_LINK_TYPE_UPLINK))
	{
		vrrp_syslog_error("delete uplink ip address null parameter error\n");
		return;
	}

	/* delete last one virtual ip address,
	 * and modify information of uplink_vif
	 */
	if (vsrv->uplink_naddr == 1)
	{
		vrrp_syslog_dbg("last one virtual ip of uplink\n");
		free(vsrv->uplink_vaddr);
		vsrv->uplink_vaddr = NULL;
		vsrv->uplink_naddr = 0;

		vsrv->uplink_flag = VRRP_LINK_NO_SETTED;
		if (0 < vsrv->uplink_fd) {
			close(vsrv->uplink_fd);
		}
		vsrv->uplink_fd = -1;
		vsrv->uplink_adver_timer = 0;
		vsrv->uplink_ms_down_timer = 0;
		free(vsrv->uplink_vif.ifname);
		memset(&(vsrv->uplink_vif), 0, sizeof(vrrp_if));
	}
	else
	{
		/* malloc new mem */
		addr = malloc((vsrv->uplink_naddr - 1) * sizeof(vip_addr));
		if (NULL == addr) {
			vrrp_syslog_error("delete uplink ip address null parameter error\n");
			return;
		}

		/* copy the ip address which not need to delete to new memory */
		j = 0;
		for (i = 0; i < vsrv->uplink_naddr; i++)
		{
			if ((ipaddr == vsrv->uplink_vaddr[i].addr) &&
				(mask == vsrv->uplink_vaddr[i].mask)) {
				/* the ip address need delete */
				continue;
			}
			addr[j].addr = vsrv->uplink_vaddr[i].addr;
			addr[j].mask = vsrv->uplink_vaddr[i].mask;
			addr[j].deletable = vsrv->uplink_vaddr[i].deletable;
			j++;
		}

		/* free old memory, and free */
		free(vsrv->uplink_vaddr);
		vsrv->uplink_vaddr = NULL;

		/* store the data */
		vsrv->uplink_naddr--;
		vsrv->uplink_vaddr = addr;
	}
	#endif
	if (!vsrv) {
		vrrp_syslog_error("delete uplink ip address null parameter error\n");
		return;
	}

	vsrv->uplink_naddr--;

	/* clear the data */
	memset(&(vsrv->uplink_vaddr[index]), 0, sizeof(vip_addr));

	return;
}

/*
 *******************************************************************************
 *had_cfg_delete_downlink_ipaddr()
 *
 *  DESCRIPTION:
 *		delete virtual ip of downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int ipaddr,			- virtual ip
 *		unsigned int mask,				- mask
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		NULL
 *
 *******************************************************************************
 */
void had_cfg_delete_downlink_ipaddr
(
	vrrp_rt *vsrv,
	unsigned int ipaddr,
	unsigned int mask,
	int index
)
{
	#if 0
	unsigned int i = 0;
	unsigned int j = 0;
	vip_addr *addr = NULL;

	if (!vsrv) {
		vrrp_syslog_error("delete downlink ip address null parameter error\n");
		return;
	}

	/* check if the virtual ip exist. */
	if (VRRP_RETURN_CODE_VIP_NOT_EXIST == had_check_link_vip_exist(vsrv, ipaddr, mask, VRRP_LINK_TYPE_DOWNLINK))
	{
		vrrp_syslog_error("delete downlink ip address null parameter error\n");
		return;
	}

	/* delete last one virtual ip address,
	 * and modify information of downlink_vif
	 */
	if (vsrv->downlink_naddr == 1)
	{
		vrrp_syslog_dbg("last one virtual ip of downlink\n");
		free(vsrv->downlink_vaddr);
		vsrv->downlink_vaddr = NULL;
		vsrv->downlink_naddr = 0;

		vsrv->downlink_flag = VRRP_LINK_NO_SETTED;
		if (0 < vsrv->downlink_fd) {
			close(vsrv->downlink_fd);
		}
		vsrv->downlink_fd = -1;
		vsrv->downlink_adver_timer = 0;
		vsrv->downlink_ms_down_timer = 0;
		free(vsrv->downlink_vif.ifname);
		memset(&(vsrv->downlink_vif), 0, sizeof(vrrp_if));
	}
	else
	{
		/* malloc new mem */
		addr = malloc((vsrv->downlink_naddr - 1) * sizeof(vip_addr));
		if (NULL == addr) {
			vrrp_syslog_error("delete downlink ip address null parameter error\n");
			return;
		}

		/* copy the ip address which not need to delete to new memory */
		j = 0;
		for (i = 0; i < vsrv->downlink_naddr; i++)
		{
			if ((ipaddr == vsrv->downlink_vaddr[i].addr) &&
				(mask == vsrv->downlink_vaddr[i].mask)) {
				/* the ip address need delete */
				continue;
			}
			addr[j].addr = vsrv->downlink_vaddr[i].addr;
			addr[j].mask = vsrv->downlink_vaddr[i].mask;
			addr[j].deletable = vsrv->downlink_vaddr[i].deletable;
			j++;
		}

		/* free old memory, and free */
		free(vsrv->downlink_vaddr);
		vsrv->downlink_vaddr = NULL;

		/* store the data */
		vsrv->downlink_naddr--;
		vsrv->downlink_vaddr = addr;
	}
	#endif

	if (!vsrv) {
		vrrp_syslog_error("delete downlink ip address null parameter error\n");
		return;
	}

	vsrv->downlink_naddr--;

	/* clear the data */
	memset(&(vsrv->downlink_vaddr[index]), 0, sizeof(vip_addr));
	
	return;
}

/*
 *******************************************************************************
 * had_cfg_vlink_add_ip6addr_check()
 *
 *  DESCRIPTION:
 *		check hansi instance different link types ip6 address, such as
 *	uplink / downlink /vgateway interface
 *			
 *  INPUTS:
 *		vsrv 	- vrrp instance info,
 *		link_type	- interface link type,
 *		struct iaddr  *ipaddr,
 *	    unsigned int prefix_length,
 *		index 	- index of the ip address' array
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		0					- successful
 *		0					- failure
 *
 *******************************************************************************
 */
static int had_cfg_vlink_add_ip6addr_check
(
	vrrp_rt *vsrv,
	unsigned int link_type,
	struct iaddr  *ip6addr,
	unsigned int prefix_length,
	unsigned int link_local,
	int index
)
{

	vrrp_syslog_info("%s: get ip6 address "NIP6QUAD_FMT"/%d index = %d\n",__func__,NIP6QUAD(ip6addr->iabuf),prefix_length,index);
	if(!vsrv) {
		vrrp_syslog_error("add %s ip6 with null instance info!\n", VRRP_LINK_TYPE_DESCANT(link_type));
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if(index >= VRRP_LINK_MAX_CNT) {
		vrrp_syslog_error("add %s ip6 with index %d out of range!\n", VRRP_LINK_TYPE_DESCANT(link_type), index);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}
	
	switch(link_type) {
		case VRRP_LINK_TYPE_DOWNLINK:
			if(link_local){
                if(!ipv6_addr_eq_null(vsrv->downlink_local_ipv6_vaddr[index].sin6_addr.s6_addr))
                	return 1;
			}
			else{
			    //memcpy(&vip6->sin6_addr.s6_addr, &(vsrv->downlink_ipv6_vaddr[index]),sizeof(vipv6_addr));
			    if(ipv6_addr_eq_null(vsrv->downlink_local_ipv6_vaddr[index].sin6_addr.s6_addr)
			    	|| !ipv6_addr_eq_null(vsrv->downlink_ipv6_vaddr[index].sin6_addr.s6_addr))
			    	return VRRP_RETURN_CODE_VIP_FIRST_LINKLOCAL;
			}
			break;
		case VRRP_LINK_TYPE_UPLINK:
			//vip6 = &(vsrv->uplink_ipv6_vaddr[index]);
			if(link_local){
                 if(!ipv6_addr_eq_null(vsrv->uplink_local_ipv6_vaddr[index].sin6_addr.s6_addr))
                	return 1;
			}
			else{
				 if(ipv6_addr_eq_null(vsrv->uplink_local_ipv6_vaddr[index].sin6_addr.s6_addr)
			    	|| !ipv6_addr_eq_null(vsrv->uplink_ipv6_vaddr[index].sin6_addr.s6_addr))
			    	return VRRP_RETURN_CODE_VIP_FIRST_LINKLOCAL;
            }
			break;
		case VRRP_LINK_TYPE_VGATEWAY:
			//vip6 = &(vsrv->vgateway_ipv6_vaddr[index]);
			if(link_local){
				if(!ipv6_addr_eq_null(vsrv->vgateway_local_ipv6_vaddr[index].sin6_addr.s6_addr))
                	return 1;
			}
			else{
				if(ipv6_addr_eq_null(vsrv->vgateway_local_ipv6_vaddr[index].sin6_addr.s6_addr)
			    	|| !ipv6_addr_eq_null(vsrv->vgateway_ipv6_vaddr[index].sin6_addr.s6_addr))
			    	return VRRP_RETURN_CODE_VIP_FIRST_LINKLOCAL;
			}
			break;
		default:
				vrrp_syslog_error("not such link type %d\n", link_type);
				return VRRP_RETURN_CODE_ERR;

	}
	
	return VRRP_RETURN_CODE_OK;
}

/*
 *******************************************************************************
 * had_cfg_vlink_add_ip6addr()
 *
 *  DESCRIPTION:
 *		add hansi instance different link types ip6 address, such as
 *	uplink / downlink /vgateway interface
 *			
 *  INPUTS:
 *		vsrv 	- vrrp instance info,
 *		link_type	- interface link type,
 *		struct iaddr  *ipaddr,
 *	    unsigned int prefix_length,
 *		index 	- index of the ip address' array
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_BAD_PARAM					- null parameter or error linktype
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip6 has exist
 *		VRRP_RETURN_CODE_IF_EXIST					- the interface has exist
 *		VRRP_RETURN_CODE_OK							- add success
 *
 *******************************************************************************
 */
 static int had_cfg_vlink_add_ip6addr
(
	vrrp_rt *vsrv,
	unsigned int link_type,
	struct iaddr  *ip6addr,
	unsigned int prefix_length,
	unsigned int link_local,
	int index
)
{
	vipv6_addr *vip6 = NULL;
	int *naddr = NULL;

	vrrp_syslog_dbg("%s: get ip6 address "NIP6QUAD_FMT"/%d index = %d\n",__func__,NIP6QUAD(ip6addr->iabuf),prefix_length,index);
	if(!vsrv) {
		vrrp_syslog_error("add %s ip6 with null instance info!\n", VRRP_LINK_TYPE_DESCANT(link_type));
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if(index >= VRRP_LINK_MAX_CNT) {
		vrrp_syslog_error("add %s ip6 with index %d out of range!\n", VRRP_LINK_TYPE_DESCANT(link_type), index);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}
	
	switch(link_type) {
		default:
			return VRRP_RETURN_CODE_BAD_PARAM;
		case VRRP_LINK_TYPE_DOWNLINK:
			if(link_local){
                vip6 = &(vsrv->downlink_local_ipv6_vaddr[index]);
			}
			else{
			    if(ipv6_addr_eq_null(vsrv->downlink_local_ipv6_vaddr[index].sin6_addr.s6_addr))
			    	return VRRP_RETURN_CODE_BAD_PARAM;
			    vip6 = &(vsrv->downlink_ipv6_vaddr[index]);
			}
			naddr = &(vsrv->downlink_ipv6_naddr);
			break;
		case VRRP_LINK_TYPE_UPLINK:
			if(link_local){
                vip6 = &(vsrv->uplink_local_ipv6_vaddr[index]);
			}
			else
			{
			    if(ipv6_addr_eq_null(vsrv->uplink_local_ipv6_vaddr[index].sin6_addr.s6_addr))
			    	return VRRP_RETURN_CODE_BAD_PARAM;
			    vip6 = &(vsrv->uplink_ipv6_vaddr[index]);
			}
			naddr = &(vsrv->uplink_ipv6_naddr);
			break;
		case VRRP_LINK_TYPE_VGATEWAY:
			if(link_local){
                vip6 = &(vsrv->vgateway_local_ipv6_vaddr[index]);
			}
			else
			{
			    if(ipv6_addr_eq_null(vsrv->vgateway_local_ipv6_vaddr[index].sin6_addr.s6_addr))
			    	return VRRP_RETURN_CODE_BAD_PARAM;
			    vip6 = &(vsrv->vgateway_ipv6_vaddr[index]);
			}
			naddr = &(vsrv->vgateway_ipv6_naddr);
			break;
	}

	if(link_local)
		(*naddr)++;
	memcpy(&vip6->sin6_addr,&ip6addr->iabuf,sizeof(struct in6_addr));
	vip6->mask = prefix_length;
	vrrp_syslog_dbg("%s: vip6->sin6_addr "NIP6QUAD_FMT"/%d\n",__func__,NIP6QUAD(vip6->sin6_addr),vip6->mask);
	vrrp_syslog_dbg("%s: ip6addr->iabuf "NIP6QUAD_FMT"/%d\n",__func__,NIP6QUAD(ip6addr->iabuf),vip6->mask);
	vip6->deletable = 0;
	
	return VRRP_RETURN_CODE_OK;
}

/*
 *******************************************************************************
 * had_cfg_vlink_del_ip6addr()
 *
 *  DESCRIPTION:
 *		delete hansi instance different link types ip6 address, such as
 *	uplink / downlink /vgateway interface
 *			
 *  INPUTS:
 *		vsrv 	- vrrp instance info,
 *		link_type	- interface link type,
 *		struct iaddr  *ipaddr,
 *	    unsigned int prefix_length,
 *		index 	- index of the ip address' array
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_BAD_PARAM					- null parameter or error linktype
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip6 has exist
 *		VRRP_RETURN_CODE_IF_EXIST					- the interface has exist
 *		VRRP_RETURN_CODE_OK							- add success
 *
 *******************************************************************************
 */
 static int had_cfg_vlink_del_ip6addr
(
	vrrp_rt *vsrv,
	unsigned int link_type,
	char *ifname,
	struct iaddr  *ip6addr,
	unsigned int prefix_length,
	unsigned int link_local,
	int index
)
{
	vipv6_addr *vip6 = NULL;
	int *naddr = NULL;
	char cmd[1024] = {0};

	//vrrp_syslog_info("had_cfg_vlink_del_ip6addr: get ip6 address "NIP6QUAD_FMT"/%d\n",NIP6QUAD(ip6addr->iabuf),prefix_length);
	if(!vsrv) {
		vrrp_syslog_error("del %s ip6 with null instance info!\n", VRRP_LINK_TYPE_DESCANT(link_type));
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if(index >= VRRP_LINK_MAX_CNT) {
		vrrp_syslog_error("del %s ip6 with index %d out of range!\n", VRRP_LINK_TYPE_DESCANT(link_type), index);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}
	if (!ifname) {
		vrrp_syslog_error("%s: ifname is null, parameter error!\n",__func__);
		return VRRP_RETURN_CODE_ERR;
	}

	switch(link_type) {
		default:
			return VRRP_RETURN_CODE_BAD_PARAM;
		case VRRP_LINK_TYPE_DOWNLINK:
			if(link_local){
				vip6 = &(vsrv->downlink_local_ipv6_vaddr[index]);
			}
			else
				vip6 = &(vsrv->downlink_ipv6_vaddr[index]);
			naddr = &(vsrv->downlink_ipv6_naddr);
			break;
		case VRRP_LINK_TYPE_UPLINK:
			if(link_local){
				vip6 = &(vsrv->uplink_local_ipv6_vaddr[index]);
			}
			else
				vip6 = &(vsrv->uplink_ipv6_vaddr[index]);
			naddr = &(vsrv->uplink_ipv6_naddr);
			break;
		case VRRP_LINK_TYPE_VGATEWAY:
			if(link_local){
    			vip6 = &(vsrv->vgateway_local_ipv6_vaddr[index]);
			}
			else
				vip6 = &(vsrv->vgateway_ipv6_vaddr[index]);
			naddr = &(vsrv->vgateway_ipv6_naddr);
			break;
	}

	if(link_local)
		(*naddr)--;

	/* clear the data */
	memset(vip6, 0, sizeof(vipv6_addr));

#if 1/* notify rtmd */
	/*
		sprintf(name,"config interface %s ipv6_address %s prelen %d link_local %d",argv[0],argv[1],argv[2],link_local);
		vtysh_client_execute(&vtysh_client[0], name, stdout); */
/*
    sprintf(cmd,"sudo /opt/bin/vtysh -c \"en\n configure terminal\n no config interface %s \"",ifname);
    vrrp_syslog_info("cmd : %s\n",cmd);
    int status = system(cmd);
	unsigned int ret = 0;
    ret = WEXITSTATUS(status);
    if (ret != 0)
    {
    	vrrp_syslog_info("cmd : %s execute fail \n",cmd);
    }
	*/
    vrrp_syslog_info("cmd : %s execute fail \n",cmd);

#endif	
	return VRRP_RETURN_CODE_OK;
}

/*
 *******************************************************************************
 * had_cfg_vlink_add_ipaddr()
 *
 *  DESCRIPTION:
 *		add hansi instance different link types ip address, such as
 *	uplink / downlink /vgateway interface
 *			
 *  INPUTS:
 *		vsrv 	- vrrp instance info,
 *		link_type	- interface link type,
 *		ipaddr 	- ip address
 *		mask	- ip mask
 *		index 	- index of the ip address' array
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_BAD_PARAM					- null parameter or error linktype
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip has exist
 *		VRRP_RETURN_CODE_IF_EXIST					- the interface has exist
 *		VRRP_RETURN_CODE_OK							- add success
 *
 *******************************************************************************
 */
 static int had_cfg_vlink_add_ipaddr
(
	vrrp_rt *vsrv,
	unsigned int link_type,
	uint32_t ipaddr,
	uint32_t mask,
	int index
)
{
	vip_addr *vip = NULL;
	int *naddr = NULL;
	
	if(!vsrv) {
		vrrp_syslog_error("add %s ip with null instance info!\n", VRRP_LINK_TYPE_DESCANT(link_type));
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if(index >= VRRP_LINK_MAX_CNT) {
		vrrp_syslog_error("add %s ip with index %d out of range!\n", VRRP_LINK_TYPE_DESCANT(link_type), index);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}
	
	switch(link_type) {
		default:
			return VRRP_RETURN_CODE_BAD_PARAM;
		case VRRP_LINK_TYPE_DOWNLINK:
			vip = &(vsrv->downlink_vaddr[index]);
			naddr = &(vsrv->downlink_naddr);
			break;
		case VRRP_LINK_TYPE_UPLINK:
			vip = &(vsrv->uplink_vaddr[index]);
			naddr = &(vsrv->uplink_naddr);
			break;
		case VRRP_LINK_TYPE_VGATEWAY:
			vip = &(vsrv->vgateway_vaddr[index]);
			naddr = &(vsrv->vgateway_naddr);
			break;
	}
	
	(*naddr)++;
	vip->addr      = ntohl(ipaddr);
	vip->mask      = mask;
	vip->deletable = 0;
	
	return VRRP_RETURN_CODE_OK;
}

/*
 *******************************************************************************
 * had_cfg_vlink_del_ipaddr()
 *
 *  DESCRIPTION:
 *		delete hansi instance different link types ip address, such as
 *	uplink / downlink /vgateway interface
 *			
 *  INPUTS:
 *		vsrv 	- vrrp instance info,
 *		link_type	- interface link type,
 *		ipaddr 	- ip address
 *		mask	- ip mask
 *		index 	- index of the ip address' array
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_BAD_PARAM					- null parameter or error linktype
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip has exist
 *		VRRP_RETURN_CODE_IF_EXIST					- the interface has exist
 *		VRRP_RETURN_CODE_OK							- add success
 *
 *******************************************************************************
 */
 static int had_cfg_vlink_del_ipaddr
(
	vrrp_rt *vsrv,
	unsigned int link_type,
	uint32_t ipaddr,
	uint32_t mask,
	int index
)
{
	vip_addr *vip = NULL;
	int *naddr = NULL;
	
	if(!vsrv) {
		vrrp_syslog_error("del %s ip with null instance info!\n", VRRP_LINK_TYPE_DESCANT(link_type));
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if(index >= VRRP_LINK_MAX_CNT) {
		vrrp_syslog_error("del %s ip with index %d out of range!\n", VRRP_LINK_TYPE_DESCANT(link_type), index);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}
	
	switch(link_type) {
		default:
			return VRRP_RETURN_CODE_BAD_PARAM;
		case VRRP_LINK_TYPE_DOWNLINK:
			vip = &(vsrv->downlink_vaddr[index]);
			naddr = &(vsrv->downlink_naddr);
			break;
		case VRRP_LINK_TYPE_UPLINK:
			vip = &(vsrv->uplink_vaddr[index]);
			naddr = &(vsrv->uplink_naddr);
			break;
		case VRRP_LINK_TYPE_VGATEWAY:
			vip = &(vsrv->vgateway_vaddr[index]);
			naddr = &(vsrv->vgateway_naddr);
			break;
	}

	(*naddr)--;

	/* clear the data */
	memset(vip, 0, sizeof(vip_addr));
	return VRRP_RETURN_CODE_OK;
}

/*
 *******************************************************************************
 * had_cfg_vlink_ip6addr_change()
 *
 *  DESCRIPTION:
 *		add or delete hansi instance different link types ip6 address, such as
 *	uplink / downlink /vgateway interface
 *			
 *  INPUTS:
 *		vsrv 	- vrrp instance info,
 *		link_type	- interface link type,
 *		op 		- add or delete
 *		ipaddr 	- ip6 address
 *		prefix_length	- ip6 prefix length
 *		index 	- index of the ip address' array
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_BAD_PARAM					- null parameter or error linktype
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip has exist
 *		VRRP_RETURN_CODE_IF_EXIST					- the interface has exist
 *		VRRP_RETURN_CODE_OK							- add success
 *
 *******************************************************************************
 */
 static int had_cfg_vlink_ip6addr_change
(
	vrrp_rt *vsrv,
	unsigned int link_type,
	char *ifname,
	unsigned int op,
	struct iaddr  *ipaddr,
	unsigned int prefix_length,
	int index,
	unsigned int link_local
)
{
	int ret = VRRP_RETURN_CODE_OK;

    if(ipaddr ){
	    vrrp_syslog_dbg("%s: %s index = %d ip6 "NIP6QUAD_FMT"/%d !\n", __func__ ,VRRP_LINK_TYPE_DESCANT(link_type),index, NIP6QUAD(ipaddr->iabuf), prefix_length);			
    }
    else{
    	vrrp_syslog_dbg("%s: %s index = %d !\n",__func__,VRRP_LINK_TYPE_DESCANT(link_type),index);			
    }

	if(!vsrv) {
		vrrp_syslog_error("change %s %d null pointer error!\n", VRRP_LINK_TYPE_DESCANT(link_type),index);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}
	if (!ifname) {
		vrrp_syslog_error("%s: ifname is null, parameter error!\n",__func__);
		return VRRP_RETURN_CODE_ERR;
	}

	switch(op) {
		default:
			vrrp_syslog_error("%s: change %s %d ip6 bad op type %d!\n",__func__ ,VRRP_LINK_TYPE_DESCANT(link_type),index,op);			
			return VRRP_RETURN_CODE_BAD_PARAM;
		case VRRP_VIP_OPT_TYPE_ADD:
			ret = had_cfg_vlink_add_ip6addr(vsrv, link_type, ipaddr, prefix_length,link_local,index);
			break;
		case VRRP_VIP_OPT_TYPE_DEL:			
			ret = had_cfg_vlink_del_ip6addr(vsrv, link_type, ifname, ipaddr, prefix_length,link_local, index);
			break;
	}

	return ret;
}

/*
 *******************************************************************************
 * had_cfg_vlink_ipaddr_change()
 *
 *  DESCRIPTION:
 *		add or delete hansi instance different link types ip address, such as
 *	uplink / downlink /vgateway interface
 *			
 *  INPUTS:
 *		vsrv 	- vrrp instance info,
 *		link_type	- interface link type,
 *		op 		- add or delete
 *		ipaddr 	- ip address
 *		mask	- ip mask
 *		index 	- index of the ip address' array
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_BAD_PARAM					- null parameter or error linktype
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip has exist
 *		VRRP_RETURN_CODE_IF_EXIST					- the interface has exist
 *		VRRP_RETURN_CODE_OK							- add success
 *
 *******************************************************************************
 */
 static int had_cfg_vlink_ipaddr_change
(
	vrrp_rt *vsrv,
	unsigned int link_type,
	unsigned int op,
	uint32_t ipaddr,
	uint32_t mask,
	int index
)
{
	int ret = VRRP_RETURN_CODE_OK;
	
	if(!vsrv) {
		vrrp_syslog_error("change %s %d ip %#x mask %#x null pointer error!\n",  \
				VRRP_LINK_TYPE_DESCANT(link_type),index, ipaddr, mask);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	switch(op) {
		default:
			vrrp_syslog_error("change %s %d ip %#x mask %#x bad op type %d!\n",  \
					VRRP_LINK_TYPE_DESCANT(link_type),index, ipaddr, mask, op);			
			return VRRP_RETURN_CODE_BAD_PARAM;
		case VRRP_VIP_OPT_TYPE_ADD:
			ret = had_cfg_vlink_add_ipaddr(vsrv, link_type, ipaddr, mask, index);
			break;
		case VRRP_VIP_OPT_TYPE_DEL:			
			ret = had_cfg_vlink_del_ipaddr(vsrv, link_type, ipaddr, mask, index);
			break;
	}

	return ret;
}

/*
 *******************************************************************************
 *had_get_link_index_by_ifname()
 *
 *  DESCRIPTION:
 *		check if the interface in link,
 *		if exist, output its index.
 *		if not exist, output empty first position index.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		char *ifname,					- interface name
 *		unsigned int up_down_flg,		- link type
 *
 *  OUTPUTS:
 *		int *index						- link index		
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_IF_EXIST		- the interface is exist.
 *		VRRP_RETURN_CODE_IF_NOT_EXIST	- the interface is not exist.
 *
 *******************************************************************************
 */
unsigned int had_get_link_index_by_ifname
(
	vrrp_rt *vsrv,
	char *ifname,
	unsigned int up_down_flg,
	int *index
)
{
	unsigned int ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int empty_index = -1;
	int exist_index = -1;
	unsigned int i = 0;
	vrrp_if *ptrVif = NULL;

	if (!vsrv || !ifname || !index) {
		vrrp_syslog_error("check if virtual ip exist null parameter error\n");
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	switch (up_down_flg)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				ptrVif = &(vsrv->uplink_vif[0]);
				#if 0
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					/* find first empty position index */
					if (VRRP_LINK_NO_SETTED == vsrv->uplink_vif[i].set_flg)
					{
						if (-1 == empty_index) {
							empty_index = i;
							vrrp_syslog_dbg("find uplink first empty position index %d.\n", i);
						}
						continue;
					}
					/* be sure set_flg = 1 and ifname = uplink_vif[i].ifname */
					else if ((VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) &&
							 (!strncmp(ifname, vsrv->uplink_vif[i].ifname, strlen(vsrv->uplink_vif[i].ifname))) &&
							 (!strncmp(ifname, vsrv->uplink_vif[i].ifname, strlen(ifname))))
					{
						exist_index = i;
						ret = VRRP_RETURN_CODE_IF_EXIST;
						break;
					}
				}
				#endif
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				ptrVif = &(vsrv->downlink_vif[0]);
				#if 0
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					/* find first empty position index */
					if (VRRP_LINK_NO_SETTED == vsrv->downlink_vif[i].set_flg)
					{
						if (-1 == empty_index) {
							empty_index = i;
							vrrp_syslog_dbg("find downlink first empty position index %d.\n", i);
						}
						continue;
					}
					/* be sure set_flg = 1 and ifname = downlink_vif[i].ifname */
					else if ((VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) &&
							 (!strncmp(ifname, vsrv->downlink_vif[i].ifname, strlen(vsrv->downlink_vif[i].ifname))) &&
							 (!strncmp(ifname, vsrv->downlink_vif[i].ifname, strlen(ifname))))
					{
						exist_index = i;
						ret = VRRP_RETURN_CODE_IF_EXIST;
						break;
					}
				}
				#endif
				break;
			}
		case VRRP_LINK_TYPE_VGATEWAY:
			{
				ptrVif = &(vsrv->vgateway_vif[0]);
			}
			break;
		case VRRP_LINK_TYPE_L2_UPLINK:
			{
				ptrVif = &(vsrv->l2_uplink_vif[0]);
			}
			break;
		default :
			{
				vrrp_syslog_error("not such link type %d\n", up_down_flg);
				ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
				break;
			}
	}

	for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
	{
		/* find first empty position index */
		if (VRRP_LINK_NO_SETTED == ptrVif[i].set_flg)
		{
			if (-1 == empty_index) {
				empty_index = i;
				vrrp_syslog_dbg("find downlink first empty position index %d.\n", i);
			}
			continue;
		}
		/* be sure set_flg = 1 and ifname = downlink_vif[i].ifname */
		else if ((VRRP_LINK_SETTED == ptrVif[i].set_flg) &&
				 (!strncmp(ifname, ptrVif[i].ifname, strlen(ptrVif[i].ifname))) &&
				 (!strncmp(ifname, ptrVif[i].ifname, strlen(ifname))))
		{
			exist_index = i;
			ret = VRRP_RETURN_CODE_IF_EXIST;
			break;
		}
	}
	/* if exist, output exist index.
	 * if not exist, output first empty position index.
	 */

	if (VRRP_RETURN_CODE_IF_EXIST == ret)
	{
		*index = exist_index;
		vrrp_syslog_dbg("%s exist in %s[%d].\n",
						ifname,
						VRRP_LINK_TYPE_DESCANT(up_down_flg),
						exist_index);

	}else {
		*index = empty_index;
		vrrp_syslog_dbg("%s exist not in %s, first empty position %d.\n",
						ifname,
						VRRP_LINK_TYPE_DESCANT(up_down_flg),
						empty_index);
		if(-1 == empty_index) {
			ret = VRRP_RETURN_CODE_IF_UP_LIMIT;
		}
	}
	
	return ret;
}

/*
 *******************************************************************************
 *had_get_link_realip_index_by_ifname()
 *
 *  DESCRIPTION:
 *		check if the interface in real ip array,
 *		if exist, output its index.
 *		if not exist, output empty first position index.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		char *ifname,					- interface name
 *		unsigned int up_down_flg,		- link type
 *
 *  OUTPUTS:
 *		int *index						- link index		
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_IF_EXIST		- the interface is exist.
 *		VRRP_RETURN_CODE_IF_NOT_EXIST	- the interface is not exist.
 *
 *******************************************************************************
 */
unsigned int had_get_link_realip_index_by_ifname
(
	hansi_s *hansi,
	char *ifname,
	unsigned int up_down_flg,
	int *index
)
{
	unsigned int ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int empty_index = -1;
	int exist_index = -1;
	unsigned int i = 0;

	if (!hansi || !ifname || !index) {
		vrrp_syslog_error("check if real ip exist null parameter error!\n");
		return VRRP_RETURN_CODE_IF_NOT_EXIST;
	}

	switch (up_down_flg)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					/* find first empty position index */
					if (VRRP_LINK_NO_SETTED == hansi->uplink_real_ip[i].set_flg)
					{
						if (-1 == empty_index) {
							empty_index = i;
							vrrp_syslog_dbg("find uplink real-ip array first empty position index %d.\n", i);
						}
						continue;
					}
					/* be sure set_flg = 1 and ifname = uplink_real_ip[i].ifname */
					else if ((VRRP_LINK_SETTED == hansi->uplink_real_ip[i].set_flg) &&
							 (!strncmp(ifname, hansi->uplink_real_ip[i].ifname, strlen(hansi->uplink_real_ip[i].ifname))) &&
							 (!strncmp(ifname, hansi->uplink_real_ip[i].ifname, strlen(ifname))))
					{
						exist_index = i;
						ret = VRRP_RETURN_CODE_IF_EXIST;
						break;
					}
				}
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					/* find first empty position index */
					if (VRRP_LINK_NO_SETTED == hansi->downlink_real_ip[i].set_flg)
					{
						if (-1 == empty_index) {
							empty_index = i;
							vrrp_syslog_dbg("find downlink real-ip array first empty position index %d.\n", i);
						}
						continue;
					}
					/* be sure set_flg = 1 and ifname = downlink_real_ip[i].ifname */
					else if ((VRRP_LINK_SETTED == hansi->downlink_real_ip[i].set_flg) &&
							 (!strncmp(ifname, hansi->downlink_real_ip[i].ifname, strlen(hansi->downlink_real_ip[i].ifname))) &&
							 (!strncmp(ifname, hansi->downlink_real_ip[i].ifname, strlen(ifname))))
					{
						exist_index = i;
						ret = VRRP_RETURN_CODE_IF_EXIST;
						break;
					}
				}
				break;
			}
		case VRRP_LINK_TYPE_VGATEWAY:
			{
				ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
			}
			break;
		default :
			{
				vrrp_syslog_error("not such link type %d\n", up_down_flg);
				ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
				break;
			}
	}

	/* if exist, output exist index.
	 * if not exist, output first empty position index.
	 */

	if (VRRP_RETURN_CODE_IF_EXIST == ret)
	{
		*index = exist_index;
		vrrp_syslog_dbg("%s exist in %s_real_ip[%d].\n",
						ifname,
						VRRP_LINK_TYPE_DESCANT(up_down_flg),
						exist_index);

	}else {
		*index = empty_index;
		vrrp_syslog_dbg("%s exist not in %s_real_ip, first empty position %d.\n",
						ifname,
						VRRP_LINK_TYPE_DESCANT(up_down_flg),
						empty_index);
	}
	
	return ret;
}
/*
 *******************************************************************************
 *had_check_link_vipv6_exist()
 *
 *  DESCRIPTION:
 *		check if the virtual ipv6 exist.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		struct iaddr  *vipv6,   		- virtual ip
 *		unsigned int prefix_length,		- mask
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_VIP_NOT_EXIST		- the virtual ip is not exist.
 *		VRRP_RETURN_CODE_VIP_EXIST			- the virtual ip is exist.
 *
 *******************************************************************************
 */
unsigned int had_check_link_vip6_exist
(
	vrrp_rt *vsrv,
	struct iaddr  *vip6,
	unsigned int prefix_length,
	unsigned int link_local,
	unsigned int up_down_flg
)
{
	unsigned int ret = VRRP_RETURN_CODE_VIP_NOT_EXIST;
	unsigned int i = 0;
	vipv6_addr *ptrVip = NULL;

	if (!vsrv) {
		vrrp_syslog_error("check if virtual ip exist null parameter error\n");
		return VRRP_RETURN_CODE_VIP_NOT_EXIST;
	}

	switch (up_down_flg)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				//ptrVip = &(vsrv->uplink_ipv6_vaddr[0]);
				if(link_local){
    				ptrVip = &(vsrv->uplink_local_ipv6_vaddr[0]);
				}
				else 
					ptrVip = &(vsrv->uplink_ipv6_vaddr[0]);

				#if 0
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
					if ((ntohl(vip) == vsrv->uplink_vaddr[i].addr) &&
						(mask == vsrv->uplink_vaddr[i].mask))
					{
						ret = VRRP_RETURN_CODE_VIP_EXIST;
					}
				}
				#endif
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				//ptrVip = &(vsrv->downlink_ipv6_vaddr[0]);
				if(link_local){
    				ptrVip = &(vsrv->downlink_local_ipv6_vaddr[0]);
				}
				else 
					ptrVip = &(vsrv->downlink_ipv6_vaddr[0]);

				#if 0
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
					if ((ntohl(vip) == vsrv->downlink_vaddr[i].addr) &&
						(mask == vsrv->downlink_vaddr[i].mask))
					{
						ret = VRRP_RETURN_CODE_VIP_EXIST;
					}
				}
				#endif
				break;
			}
		case VRRP_LINK_TYPE_VGATEWAY:
			{
				//ptrVip = &(vsrv->vgateway_ipv6_vaddr[0]);
				if(link_local){
    				ptrVip = &(vsrv->vgateway_local_ipv6_vaddr[0]);
				}
				else 
					ptrVip = &(vsrv->vgateway_ipv6_vaddr[0]);

			}
			break;
		default :
			{
				vrrp_syslog_error("not such link type %d\n", up_down_flg);
				ret = VRRP_RETURN_CODE_VIP_NOT_EXIST;
				break;
			}
	}
	
	for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {

		if (!memcmp(vip6->iabuf,ptrVip[i].sin6_addr.s6_addr,sizeof(char)*16) && (prefix_length == ptrVip[i].mask))
		{
			ret = VRRP_RETURN_CODE_VIP_EXIST;
		}
	}

	if (VRRP_RETURN_CODE_VIP_EXIST == ret)
	{
		vrrp_syslog_info("virtual ipv6 addr : "NIP6QUAD_FMT"/%d exist in %s.\n",
        		NIP6QUAD(vip6->iabuf),
        		prefix_length,
        		VRRP_LINK_TYPE_DESCANT(up_down_flg)
                );

	}
	
	return ret;
}

/*
 *******************************************************************************
 *had_check_link_vip_exist()
 *
 *  DESCRIPTION:
 *		check if the virtual ip exist.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int uplink_ip,			- virtual ip
 *		unsigned int uplink_mask,		- mask
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_VIP_NOT_EXIST		- the virtual ip is not exist.
 *		VRRP_RETURN_CODE_VIP_EXIST			- the virtual ip is exist.
 *
 *******************************************************************************
 */
unsigned int had_check_link_vip_exist
(
	vrrp_rt *vsrv,
	unsigned int vip,
	unsigned int mask,
	unsigned int up_down_flg
)
{
	unsigned int ret = VRRP_RETURN_CODE_VIP_NOT_EXIST;
	unsigned int i = 0;
	vip_addr *ptrVip = NULL;

	if (!vsrv) {
		vrrp_syslog_error("check if virtual ip exist null parameter error\n");
		return VRRP_RETURN_CODE_VIP_NOT_EXIST;
	}

	switch (up_down_flg)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				ptrVip = &(vsrv->uplink_vaddr[0]);
				#if 0
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
					if ((ntohl(vip) == vsrv->uplink_vaddr[i].addr) &&
						(mask == vsrv->uplink_vaddr[i].mask))
					{
						ret = VRRP_RETURN_CODE_VIP_EXIST;
					}
				}
				#endif
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				ptrVip = &(vsrv->downlink_vaddr[0]);
				#if 0
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
					if ((ntohl(vip) == vsrv->downlink_vaddr[i].addr) &&
						(mask == vsrv->downlink_vaddr[i].mask))
					{
						ret = VRRP_RETURN_CODE_VIP_EXIST;
					}
				}
				#endif
				break;
			}
		case VRRP_LINK_TYPE_VGATEWAY:
			{
				ptrVip = &(vsrv->vgateway_vaddr[0]);
			}
			break;
		default :
			{
				vrrp_syslog_error("not such link type %d\n", up_down_flg);
				ret = VRRP_RETURN_CODE_VIP_NOT_EXIST;
				break;
			}
	}
	
	for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
		if ((ntohl(vip) == ptrVip[i].addr) && (mask == ptrVip[i].mask))
		{
			ret = VRRP_RETURN_CODE_VIP_EXIST;
			}
	}

	if (VRRP_RETURN_CODE_VIP_EXIST == ret)
	{
		vrrp_syslog_dbg("virtual ip %d.%d.%d.%d/%d exist in %s.\n",
						(ntohl(vip) >> 24) & 0xFF,
						(ntohl(vip) >> 16) & 0xFF,
						(ntohl(vip) >> 8) & 0xFF,
						(ntohl(vip)) & 0xFF,
						mask,
						VRRP_LINK_TYPE_DESCANT(up_down_flg));
	}
	
	return ret;
}

/*
 *******************************************************************************
 *had_add_vip6_to_linkvif()
 *
 *  DESCRIPTION:
 *		add interface to uplink_vif|downlink_vif array.
 *
 *  INPUTS:
 *		int profile,					- hansi profile
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int up_down_flg		- link type
 *		char * ifname,					- interface name
 *		int index						- add position
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR		- add faild
 *		VRRP_RETURN_CODE_OK			- add success
 *
 *******************************************************************************
 */
unsigned int had_add_vip6_to_linkvif
(
	int profile,
	vrrp_rt *vsrv,
	unsigned int up_down_flg,
	char *ifname,
	int index
)
{
	unsigned int ret = VRRP_RETURN_CODE_OK;
	vipv6_addr real_ip6 ;
	int realip_index = -1;
	hansi_s *hansiNode = NULL;
	vrrp_if *ptrVif = NULL;
	unsigned int *set_flag = NULL;

	if (!vsrv || !ifname) {
		vrrp_syslog_error("vrrp %d add virtual ip to link vif, null parameter error\n", profile);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if (index < 0 ||
		index > VRRP_LINK_MAX_CNT)
	{
		vrrp_syslog_error("vrrp %d add virtual ip to link vif, index %d out of valid range.\n",
							profile, index);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if (MAX_IFNAME_LEN < strlen(ifname)) {
		vrrp_syslog_error("add virtual ip to link vif, length of interface name out of valid range.\n");		
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		vrrp_syslog_error("vrrp %d not create profile node.\n", profile);
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	switch (up_down_flg)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{	/* uplink */
				ptrVif = &(vsrv->uplink_vif[index]);
				set_flag = &(vsrv->uplink_flag);
				#if 0
				memset(vsrv->uplink_vif[index].ifname, 0, MAX_IFNAME_LEN);
				memcpy(vsrv->uplink_vif[index].ifname, ifname, strlen(ifname));

				/*real ip if appointed*/
				if (VRRP_RETURN_CODE_IF_NOT_EXIST ==
					had_get_link_realip_index_by_ifname(hansiNode, ifname, up_down_flg, &realip_index))
				{
					ret = had_ifname_to_ip(ifname, &real_ip);
					if (VRRP_RETURN_CODE_OK != ret) {
						vrrp_syslog_error("get real ip address by ifname %s error\n",
											ifname);
						ret = VRRP_RETURN_CODE_ERR;
						break;
					}
					vsrv->uplink_vif[index].ipaddr = real_ip;
				}
				else{
					vsrv->uplink_vif[index].ipaddr = hansiNode->uplink_real_ip[realip_index].real_ip;
				}
				vrrp_syslog_dbg("interface %s ip addr %x\n",
								ifname, vsrv->uplink_vif[index].ipaddr);

				/* ifindex */
				vsrv->uplink_vif[index].ifindex = had_ifname_to_idx(vsrv->uplink_vif[index].ifname);
				vrrp_syslog_dbg("interface index 0x%x\n",
								vsrv->uplink_vif[index].ifindex);
				
				/* hwaddress */
				if (had_hwaddr_get(vsrv->uplink_vif[index].ifname,
							   vsrv->uplink_vif[index].hwaddr,
							   sizeof(vsrv->uplink_vif[index].hwaddr)))
				{
					vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n",
									vsrv->uplink_vif[index].ifname);
					ret = VRRP_RETURN_CODE_ERR;
					break;
				}

				/*init aute_type*/
				vsrv->uplink_vif[index].auth_type = VRRP_AUTH_PASS;
				if (VRRP_LINK_NO_SETTED == vsrv->uplink_flag) {
					vsrv->uplink_flag = VRRP_LINK_SETTED;
				}
				vsrv->uplink_vif[index].linkstate = vrrp_get_ifname_linkstate(vsrv->uplink_vif[index].ifname);
				vsrv->uplink_vif[index].set_flg = VRRP_LINK_SETTED;

				ret = VRRP_RETURN_CODE_OK;
				#endif
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{	/* downlink */
				ptrVif = &(vsrv->downlink_vif[index]);
				set_flag = &(vsrv->downlink_flag);
				#if 0
				memset(vsrv->downlink_vif[index].ifname, 0, MAX_IFNAME_LEN);
				memcpy(vsrv->downlink_vif[index].ifname, ifname, strlen(ifname));

				/*real ip if appointed*/
				if (VRRP_RETURN_CODE_IF_NOT_EXIST ==
					had_get_link_realip_index_by_ifname(hansiNode, ifname, up_down_flg, &realip_index))
				{
					ret = had_ifname_to_ip(ifname, &real_ip);
					if (VRRP_RETURN_CODE_OK != ret) {
						vrrp_syslog_error("get real ip address by ifname %s error\n",
											ifname);
						ret = VRRP_RETURN_CODE_ERR;
						break;
					}
					vsrv->downlink_vif[index].ipaddr = real_ip;
				}
				else{
					vsrv->downlink_vif[index].ipaddr = hansiNode->downlink_real_ip[realip_index].real_ip;
				}
				vrrp_syslog_dbg("interface %s ip addr %x\n",
								ifname, vsrv->downlink_vif[index].ipaddr);

				/* ifindex */
				vsrv->downlink_vif[index].ifindex = had_ifname_to_idx(vsrv->downlink_vif[index].ifname);
				vrrp_syslog_dbg("interface index 0x%x\n",
								vsrv->downlink_vif[index].ifindex);
				
				/* hwaddress */
				if (had_hwaddr_get(vsrv->downlink_vif[index].ifname,
							   vsrv->downlink_vif[index].hwaddr,
							   sizeof(vsrv->downlink_vif[index].hwaddr)))
				{
					vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n",
									vsrv->downlink_vif[index].ifname);
					ret = VRRP_RETURN_CODE_ERR;
					break;
				}

				/*init aute_type*/
				vsrv->downlink_vif[index].auth_type = VRRP_AUTH_PASS;
				if (VRRP_LINK_NO_SETTED == vsrv->downlink_flag) {
					vsrv->downlink_flag = VRRP_LINK_SETTED;
				}
				vsrv->downlink_vif[index].linkstate = vrrp_get_ifname_linkstate(vsrv->downlink_vif[index].ifname);
				vsrv->downlink_vif[index].set_flg = VRRP_LINK_SETTED;

				ret = VRRP_RETURN_CODE_OK;
				#endif
				break;
			}
		case VRRP_LINK_TYPE_VGATEWAY:
			{	/* vgateway */			
				ptrVif = &(vsrv->vgateway_vif[index]);
				set_flag = &(vsrv->vgateway_flag);
			}
			break;
		case VRRP_LINK_TYPE_L2_UPLINK :
			{	/* l2-uplink */
				ptrVif = &(vsrv->l2_uplink_vif[index]);
				set_flag = &(vsrv->l2_uplink_flag);
			}
			break;//hjw add may be a special logic
		default :
			{
				vrrp_syslog_error("not such link type %d\n", up_down_flg);
				return VRRP_RETURN_CODE_ERR;
			}
	}

	memset(ptrVif->ifname, 0, MAX_IFNAME_LEN);
	memcpy(ptrVif->ifname, ifname, strlen(ifname));
	
	if(VRRP_LINK_TYPE_L2_UPLINK != up_down_flg){
	/*real ip if appointed*/
	if (VRRP_RETURN_CODE_IF_NOT_EXIST ==
		had_get_link_realip_index_by_ifname(hansiNode, ifname, up_down_flg, &realip_index))
	{
		memset(&real_ip6,0,sizeof(real_ip6));
		ret = had_ifname_to_ipv6(ifname, &real_ip6);
		if (VRRP_RETURN_CODE_OK != ret) {
			vrrp_syslog_error("get real ip address by ifname %s error\n",
								ifname);
			return VRRP_RETURN_CODE_ERR;
		}
		memcpy(&ptrVif->ipaddr, &real_ip6,sizeof(real_ip6));
	}
	else{
		memcpy(&ptrVif->ipaddr, &hansiNode->downlink_real_ip[realip_index].real_ip,sizeof(struct in6_addr));
	}
	
	vrrp_syslog_info("interface %s ip addr "NIP6QUAD_FMT"\n", ifname, NIP6QUAD(ptrVif->ipaddr));
	
	}
	/* ifindex */
	ptrVif->ifindex = had_ifname_to_idx(ptrVif->ifname);
	vrrp_syslog_info("interface index 0x%x\n", ptrVif->ifindex);
	
	/* hwaddress */
	/* replace with vrrp_global_mac	  
	if (had_hwaddr_get(ptrVif->ifname, ptrVif->hwaddr, sizeof(ptrVif->hwaddr)))
	{
		vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n", ptrVif->ifname);
		return VRRP_RETURN_CODE_ERR;
	}
	*/
	memcpy(ptrVif->hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
	/*init aute_type*/
	ptrVif->auth_type = VRRP_AUTH_PASS;
	if (VRRP_LINK_NO_SETTED == *set_flag) {
		*set_flag = VRRP_LINK_SETTED;
	}
	ptrVif->linkstate = vrrp_get_ifname_linkstate(ptrVif->ifname);
	ptrVif->set_flg = VRRP_LINK_SETTED;
	if (VRRP_RETURN_CODE_OK == ret) {
		vrrp_syslog_info("add interface %s to %s_vif array success.\n",
						ifname,
						VRRP_LINK_TYPE_DESCANT(up_down_flg));
	}

	return ret;
}
/*
 *******************************************************************************
 *had_add_vip_to_linkvif()
 *
 *  DESCRIPTION:
 *		add interface to uplink_vif|downlink_vif array.
 *
 *  INPUTS:
 *		int profile,					- hansi profile
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int up_down_flg		- link type
 *		char * ifname,					- interface name
 *		int index						- add position
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR		- add faild
 *		VRRP_RETURN_CODE_OK			- add success
 *
 *******************************************************************************
 */
unsigned int had_add_vip_to_linkvif
(
	int profile,
	vrrp_rt *vsrv,
	unsigned int up_down_flg,
	char *ifname,
	int index
)
{
	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int real_ip = 0;
	int realip_index = -1;
	hansi_s *hansiNode = NULL;
	vrrp_if *ptrVif = NULL;
	unsigned int *set_flag = NULL;

	if (!vsrv || !ifname) {
		vrrp_syslog_error("vrrp %d add virtual ip to link vif, null parameter error\n", profile);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if (index < 0 ||
		index > VRRP_LINK_MAX_CNT)
	{
		vrrp_syslog_error("vrrp %d add virtual ip to link vif, index %d out of valid range.\n",
							profile, index);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	if (MAX_IFNAME_LEN < strlen(ifname)) {
		vrrp_syslog_error("add virtual ip to link vif, length of interface name out of valid range.\n");		
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		vrrp_syslog_error("vrrp %d not create profile node.\n", profile);
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	switch (up_down_flg)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{	/* uplink */
				ptrVif = &(vsrv->uplink_vif[index]);
				set_flag = &(vsrv->uplink_flag);
				#if 0
				memset(vsrv->uplink_vif[index].ifname, 0, MAX_IFNAME_LEN);
				memcpy(vsrv->uplink_vif[index].ifname, ifname, strlen(ifname));

				/*real ip if appointed*/
				if (VRRP_RETURN_CODE_IF_NOT_EXIST ==
					had_get_link_realip_index_by_ifname(hansiNode, ifname, up_down_flg, &realip_index))
				{
					ret = had_ifname_to_ip(ifname, &real_ip);
					if (VRRP_RETURN_CODE_OK != ret) {
						vrrp_syslog_error("get real ip address by ifname %s error\n",
											ifname);
						ret = VRRP_RETURN_CODE_ERR;
						break;
					}
					vsrv->uplink_vif[index].ipaddr = real_ip;
				}
				else{
					vsrv->uplink_vif[index].ipaddr = hansiNode->uplink_real_ip[realip_index].real_ip;
				}
				vrrp_syslog_dbg("interface %s ip addr %x\n",
								ifname, vsrv->uplink_vif[index].ipaddr);

				/* ifindex */
				vsrv->uplink_vif[index].ifindex = had_ifname_to_idx(vsrv->uplink_vif[index].ifname);
				vrrp_syslog_dbg("interface index 0x%x\n",
								vsrv->uplink_vif[index].ifindex);
				
				/* hwaddress */
				if (had_hwaddr_get(vsrv->uplink_vif[index].ifname,
							   vsrv->uplink_vif[index].hwaddr,
							   sizeof(vsrv->uplink_vif[index].hwaddr)))
				{
					vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n",
									vsrv->uplink_vif[index].ifname);
					ret = VRRP_RETURN_CODE_ERR;
					break;
				}

				/*init aute_type*/
				vsrv->uplink_vif[index].auth_type = VRRP_AUTH_PASS;
				if (VRRP_LINK_NO_SETTED == vsrv->uplink_flag) {
					vsrv->uplink_flag = VRRP_LINK_SETTED;
				}
				vsrv->uplink_vif[index].linkstate = vrrp_get_ifname_linkstate(vsrv->uplink_vif[index].ifname);
				vsrv->uplink_vif[index].set_flg = VRRP_LINK_SETTED;

				ret = VRRP_RETURN_CODE_OK;
				#endif
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{	/* downlink */
				ptrVif = &(vsrv->downlink_vif[index]);
				set_flag = &(vsrv->downlink_flag);
				#if 0
				memset(vsrv->downlink_vif[index].ifname, 0, MAX_IFNAME_LEN);
				memcpy(vsrv->downlink_vif[index].ifname, ifname, strlen(ifname));

				/*real ip if appointed*/
				if (VRRP_RETURN_CODE_IF_NOT_EXIST ==
					had_get_link_realip_index_by_ifname(hansiNode, ifname, up_down_flg, &realip_index))
				{
					ret = had_ifname_to_ip(ifname, &real_ip);
					if (VRRP_RETURN_CODE_OK != ret) {
						vrrp_syslog_error("get real ip address by ifname %s error\n",
											ifname);
						ret = VRRP_RETURN_CODE_ERR;
						break;
					}
					vsrv->downlink_vif[index].ipaddr = real_ip;
				}
				else{
					vsrv->downlink_vif[index].ipaddr = hansiNode->downlink_real_ip[realip_index].real_ip;
				}
				vrrp_syslog_dbg("interface %s ip addr %x\n",
								ifname, vsrv->downlink_vif[index].ipaddr);

				/* ifindex */
				vsrv->downlink_vif[index].ifindex = had_ifname_to_idx(vsrv->downlink_vif[index].ifname);
				vrrp_syslog_dbg("interface index 0x%x\n",
								vsrv->downlink_vif[index].ifindex);
				
				/* hwaddress */
				if (had_hwaddr_get(vsrv->downlink_vif[index].ifname,
							   vsrv->downlink_vif[index].hwaddr,
							   sizeof(vsrv->downlink_vif[index].hwaddr)))
				{
					vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n",
									vsrv->downlink_vif[index].ifname);
					ret = VRRP_RETURN_CODE_ERR;
					break;
				}

				/*init aute_type*/
				vsrv->downlink_vif[index].auth_type = VRRP_AUTH_PASS;
				if (VRRP_LINK_NO_SETTED == vsrv->downlink_flag) {
					vsrv->downlink_flag = VRRP_LINK_SETTED;
				}
				vsrv->downlink_vif[index].linkstate = vrrp_get_ifname_linkstate(vsrv->downlink_vif[index].ifname);
				vsrv->downlink_vif[index].set_flg = VRRP_LINK_SETTED;

				ret = VRRP_RETURN_CODE_OK;
				#endif
				break;
			}
		case VRRP_LINK_TYPE_VGATEWAY:
			{	/* vgateway */			
				ptrVif = &(vsrv->vgateway_vif[index]);
				set_flag = &(vsrv->vgateway_flag);
			}
			break;
		case VRRP_LINK_TYPE_L2_UPLINK :
			{	/* l2-uplink */
				ptrVif = &(vsrv->l2_uplink_vif[index]);
				set_flag = &(vsrv->l2_uplink_flag);
			}
			break;//hjw add may be a special logic
		default :
			{
				vrrp_syslog_error("not such link type %d\n", up_down_flg);
				return VRRP_RETURN_CODE_ERR;
			}
	}

	memset(ptrVif->ifname, 0, MAX_IFNAME_LEN);
	memcpy(ptrVif->ifname, ifname, strlen(ifname));
	
	if(VRRP_LINK_TYPE_L2_UPLINK != up_down_flg){
	/*real ip if appointed*/
	if (VRRP_RETURN_CODE_IF_NOT_EXIST ==
		had_get_link_realip_index_by_ifname(hansiNode, ifname, up_down_flg, &realip_index))
	{
		ret = had_ifname_to_ip(ifname, &real_ip);
		if (VRRP_RETURN_CODE_OK != ret) {
			vrrp_syslog_error("get real ip address by ifname %s error\n",
								ifname);
			return VRRP_RETURN_CODE_ERR;
		}
		ptrVif->ipaddr = real_ip;
	}
	else{
		ptrVif->ipaddr = hansiNode->downlink_real_ip[realip_index].real_ip;
	}
	
	vrrp_syslog_dbg("interface %s ip addr %x\n", ifname, ptrVif->ipaddr);
	
	}
	/* ifindex */
	ptrVif->ifindex = had_ifname_to_idx(ptrVif->ifname);
	vrrp_syslog_dbg("interface index 0x%x\n", ptrVif->ifindex);
	
	/* hwaddress */
	/* replace with vrrp_global_mac	  
	if (had_hwaddr_get(ptrVif->ifname, ptrVif->hwaddr, sizeof(ptrVif->hwaddr)))
	{
		vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n", ptrVif->ifname);
		return VRRP_RETURN_CODE_ERR;
	}
	*/
	memcpy(ptrVif->hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
	/*init aute_type*/
	ptrVif->auth_type = VRRP_AUTH_PASS;
	if (VRRP_LINK_NO_SETTED == *set_flag) {
		*set_flag = VRRP_LINK_SETTED;
	}
	ptrVif->linkstate = vrrp_get_ifname_linkstate(ptrVif->ifname);
	ptrVif->set_flg = VRRP_LINK_SETTED;
	if (VRRP_RETURN_CODE_OK == ret) {
		vrrp_syslog_dbg("add interface %s to %s_vif array success.\n",
						ifname,
						VRRP_LINK_TYPE_DESCANT(up_down_flg));
	}

	return ret;
}

/*
 *******************************************************************************
 *had_del_vip_from_linkvif()
 *
 *  DESCRIPTION:
 *		delete interface from uplink_vif|downlink_vif array.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int up_down_flg		- link type
 *		char * ifname,					- interface name
 *		int index						- add position
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR		- delete faild
 *		VRRP_RETURN_CODE_OK			- delete success
 *
 *******************************************************************************
 */
unsigned int had_del_vip_from_linkvif
(
	vrrp_rt *vsrv,
	unsigned int up_down_flg,
	char * ifname,
	int index
)
{
	unsigned int ret = VRRP_RETURN_CODE_OK;
	int i = 0;
	int set_cnt = 0;
	vrrp_if *ptrVif = NULL, *ptrTmp = NULL;
	unsigned int *set_flag = NULL;

	if (!vsrv || !ifname) {
		vrrp_syslog_error("delete virtual ip6 from link vif, null parameter error\n");
		return VRRP_RETURN_CODE_ERR;
	}

	if (index < 0 ||
		index > VRRP_LINK_MAX_CNT)
	{
		vrrp_syslog_error("delete virtual ip6 from link vif, index %d out of valid range.\n",
							index);
		return VRRP_RETURN_CODE_ERR;
	}

	if (MAX_IFNAME_LEN < strlen(ifname)) {
		vrrp_syslog_error("delete virtual ip6 from link vif, length of interface name out of valid range.\n");		
		return VRRP_RETURN_CODE_ERR;
	}

	switch (up_down_flg)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{	/* clear uplink_vif[index] */
				ptrTmp = &(vsrv->uplink_vif[0]);
				ptrVif = &(vsrv->uplink_vif[index]);
				set_flag = &(vsrv->uplink_flag);
				#if 0
				memset(&(vsrv->uplink_vif[index]), 0, sizeof(vrrp_if));
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
					if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) {
						set_cnt++;
					}
				}
				vrrp_syslog_dbg("count of %s set_flg: %d\n",
								VRRP_LINK_TYPE_DESCANT(up_down_flg),
								set_cnt);
				if (0 == set_cnt) {
					vsrv->uplink_flag = VRRP_LINK_NO_SETTED;
					vrrp_syslog_dbg("delete all %s_vif array, reset uplink_flg\n",
									VRRP_LINK_TYPE_DESCANT(up_down_flg));
				}
				
				ret = VRRP_RETURN_CODE_OK;
				#endif
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{ /* clear downlink_vif[index] */
				
				ptrTmp = &(vsrv->downlink_vif[0]);
				ptrVif = &(vsrv->downlink_vif[index]);
				set_flag = &(vsrv->downlink_flag);
				#if 0
				memset(&(vsrv->downlink_vif[index]), 0, sizeof(vrrp_if));
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
					if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) {
						set_cnt++;
					}
				}
				vrrp_syslog_dbg("count of %s set_flg: %d\n",
								VRRP_LINK_TYPE_DESCANT(up_down_flg),
								set_cnt);
				if (0 == set_cnt) {
					vsrv->downlink_flag = VRRP_LINK_NO_SETTED;
					vrrp_syslog_dbg("delete all %s_vif array, reset downlink_flg\n",
									VRRP_LINK_TYPE_DESCANT(up_down_flg));
				}
				
				ret = VRRP_RETURN_CODE_OK;
				#endif
				break;
			}
		case VRRP_LINK_TYPE_VGATEWAY:
			{	/* vgateway */			
                ptrTmp = &(vsrv->vgateway_vif[0]);
				ptrVif = &(vsrv->vgateway_vif[index]);
				set_flag = &(vsrv->vgateway_flag);
			    break;
		    }
			
		default :
			{
				vrrp_syslog_error("not such link type %d\n", up_down_flg);
				return VRRP_RETURN_CODE_ERR;
			}
	}
   //delete uplink or downlink or vgateway specific members
	memset(ptrVif, 0, sizeof(vrrp_if));
	for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
		if (VRRP_LINK_SETTED == ptrTmp[i].set_flg) {
			set_cnt++;
		}
	}
	vrrp_syslog_dbg("count of %s set_flg: %d\n",
					VRRP_LINK_TYPE_DESCANT(up_down_flg), set_cnt);
	
	if (0 == set_cnt) {
		*set_flag = VRRP_LINK_NO_SETTED;
		vrrp_syslog_dbg("delete all %s_vif array, reset downlink_flg\n",
						VRRP_LINK_TYPE_DESCANT(up_down_flg));
	}
	
	if (VRRP_RETURN_CODE_OK == ret) {
		vrrp_syslog_dbg("delete interface %s from %s [%d] success.\n",
						ifname, VRRP_LINK_TYPE_DESCANT(up_down_flg), index);
	}

	return ret;
}

/*
 *******************************************************************************
 *had_add_vipv6()
 *
 *  DESCRIPTION:
 *		add link virtual ipv6, and fill uplink_vif|downlink_vif.
 *		before add must setted linkmode(linktype + priority)
 *
 *  INPUTS:
 *		unsigned int profile,			- vrid
 *		char *uplink_ifname,			- uplink interface name
 *		struct iaddr  *uplink_ip,		- virtual ipv6 address
 *		unsigned int uplink_prefix,		- prefix length
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR						- null parameter or error linktype
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST			- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_LINKMODE_NO_SETTED			- not setted linkmode
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip has exist
 *		VRRP_RETURN_CODE_IF_EXIST					- the interface has exist
 *		VRRP_RETURN_CODE_OK							- add success
 *
 *******************************************************************************
 */
 
unsigned int had_add_vipv6
(
	unsigned int profile,
	char *ifname,
	struct iaddr  *vip6,
	unsigned int prefix_length,
	unsigned int link_local,
	unsigned int up_down_flg
)
{
	unsigned int ret = VRRP_RETURN_CODE_OK;
	vrrp_rt *vsrv = NULL;
	hansi_s *hansiNode = NULL;
	int index = -1, i = 0;
	unsigned int set_flag = VRRP_LINK_NO_SETTED;
	vipv6_addr *ipPtr = NULL;
	vrrp_if *linkIf = NULL;

	if (!ifname) {
		vrrp_syslog_error("vrrp %d add %s virtual ip6 null parameter error!\n",
							profile,
							VRRP_LINK_TYPE_DESCANT(up_down_flg));
		return VRRP_RETURN_CODE_ERR;
	}
	
	pthread_mutex_lock(&StateMutex);
	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		vrrp_syslog_error("vrrp %d not create profile node.\n", profile);
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (NULL == (vsrv = hansiNode->vlist)) {
		vrrp_syslog_error("vrrp %d not create vrrp node.\n", profile);
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (VRRP_SERVICE_ENABLE == service_enable[profile]) {
		vrrp_syslog_error("vrrp %d service enabled.\n", profile);
		ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else {
		switch (up_down_flg)
		{
			case VRRP_LINK_TYPE_UPLINK :
				{
					set_flag = vsrv->uplink_flag;
					linkIf = &(vsrv->uplink_vif[0]);
					if(link_local){
    					ipPtr = &(vsrv->uplink_local_ipv6_vaddr[0]);
					}
					else 
						ipPtr = &(vsrv->uplink_ipv6_vaddr[0]);
					break;
				}
			case VRRP_LINK_TYPE_DOWNLINK :
				{
					set_flag = vsrv->downlink_flag;
					linkIf = &(vsrv->downlink_vif[0]);
					if(link_local){
    					ipPtr = &(vsrv->downlink_local_ipv6_vaddr[0]);
					}
					else 
						ipPtr = &(vsrv->downlink_ipv6_vaddr[0]);

					break;
				}
			case VRRP_LINK_TYPE_VGATEWAY:
				{
					set_flag = VRRP_LINK_SETTED;
					linkIf = &(vsrv->vgateway_vif[0]);
					if(link_local){
    					ipPtr = &(vsrv->vgateway_local_ipv6_vaddr[0]);
					}
					else 
						ipPtr = &(vsrv->vgateway_ipv6_vaddr[0]);

				}
				break;
			case VRRP_LINK_TYPE_L2_UPLINK:
				{
					set_flag = VRRP_LINK_SETTED;
					linkIf = &(vsrv->l2_uplink_vif[0]);
				}
				break;
			default :
				{
					vrrp_syslog_error("not such link type %d\n", up_down_flg);
					ret = VRRP_RETURN_CODE_ERR;
					break;
				}
		}
		
		if(VRRP_RETURN_CODE_OK == ret) {
			if (VRRP_LINK_SETTED != set_flag)
			{	/* check if the link setted */
				vrrp_syslog_error("add %s vip6 faild, not setted right (link+priority) mode.\n",
									VRRP_LINK_TYPE_DESCANT(up_down_flg));
				ret = VRRP_RETURN_CODE_LINKMODE_NO_SETTED;
				goto end;
			}
			
			if(VRRP_RETURN_CODE_IF_EXIST == had_get_link_index_by_ifname(vsrv, 
						ifname, up_down_flg, &index))
			{	/* check the link interface name exist. */
                ret = had_cfg_vlink_add_ip6addr_check(vsrv, up_down_flg,vip6, prefix_length ,link_local, index);
				if(VRRP_RETURN_CODE_OK != ret){
					vrrp_syslog_error("add interface %s to %s index %d error!\n",
									ifname,
									VRRP_LINK_TYPE_DESCANT(up_down_flg),
									index);
					goto end;
				}
			}
			
			if((VRRP_LINK_TYPE_L2_UPLINK != up_down_flg)&& (VRRP_RETURN_CODE_VIP_EXIST == had_check_link_vip6_exist(vsrv, &vip6, prefix_length, link_local,up_down_flg)))
			{	/* check if the virtual ip has setted.*/
				ret = VRRP_RETURN_CODE_VIP_EXIST;
			}
			else
			{	/* add uplink to uplink_vif */
				/* index is first empty position in uplink_vif array */
				ret = had_add_vip6_to_linkvif(profile, vsrv, up_down_flg, ifname, index);
				if (VRRP_RETURN_CODE_OK != ret) {
					vrrp_syslog_error("add interface %s to %s index %d error!\n",
									ifname,
									VRRP_LINK_TYPE_DESCANT(up_down_flg),
									index);
				}
				else
				{
add_addr:
					if(VRRP_LINK_TYPE_L2_UPLINK != up_down_flg){
						/* add uplink virtual ip*/
						had_cfg_vlink_ip6addr_change(vsrv, up_down_flg, ifname,VRRP_VIP_OPT_TYPE_ADD, vip6, prefix_length, index,link_local);
					    ipPtr +=index;
					}
					else{
						vsrv->l2_uplink_naddr ++;
					}
					vrrp_syslog_info("%s: add %s ipv6 "NIP6QUAD_FMT"/%d index = %d success.\n", __func__ ,\
							VRRP_LINK_TYPE_DESCANT(up_down_flg), NIP6QUAD(vip6->iabuf), prefix_length, index);
		
				}
			}
			
			/* for debug */
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == linkIf[i].set_flg) {
					vrrp_syslog_info("had_add_vipv6: %s vip6 addr "NIP6QUAD_FMT"/%d\n",
                            		VRRP_LINK_TYPE_DESCANT(up_down_flg),
                            		NIP6QUAD(ipPtr->sin6_addr.s6_addr),
                            		prefix_length);
				}
			}
		}
	}
end:
	pthread_mutex_unlock(&StateMutex);

	return ret;
}

/*
 *******************************************************************************
 *had_delete_vipv6()
 *
 *  DESCRIPTION:
 *		delete link virtual ip, and delete uplink_vif|downlink_vif,
 *		before delete must be sure setted linkmode(linktype + priority),
 *		and virtual ip which configured by "config uplink | downlink priority"
 *		is not allow to delete.
 *
 *  INPUTS:
 *		unsigned int profile,			- vrid
 *		char *uplink_ifname,			- uplink interface name
 *		struct iaddr  *uplink_ip,		- virtual ipv6 address
 *		unsigned int uplink_prefix,		- prefix length
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR						- null parameter or error linktype
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST			- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_LINKMODE_NO_SETTED			- not setted linkmode
 *		VRRP_RETURN_CODE_IFNAME_ERROR				- link interface name error
 *		VRRP_RETURN_CODE_VIP_NOT_EXIST				- the virtual ip has not exist
 *		VRRP_RETURN_CODE_VIP_LAST_ONE				- virtual ip is last one, not allow to deletes
 *
 *******************************************************************************
 */
unsigned int had_delete_vipv6
(
	unsigned int profile,
	char *ifname,
	struct iaddr  *vip6,
	unsigned int prefix_length,
	unsigned int link_local,
	unsigned int up_down_flg
)
{
	unsigned int ret = VRRP_RETURN_CODE_OK;
	vrrp_rt *vsrv = NULL;
	hansi_s *hansiNode = NULL;
	int index = -1;
	unsigned int set_flag = VRRP_LINK_NO_SETTED;
	int naddr = 0;
	vip_addr *ipPtr = NULL;
	vrrp_if *linkIf = NULL;
	int *ptrFd = NULL;

	if (!ifname) {
		vrrp_syslog_error("vrrp %d delete %s virtual ip null parameter error!\n",
							profile,
							VRRP_LINK_TYPE_DESCANT(up_down_flg));
		return VRRP_RETURN_CODE_ERR;
	}
	
	pthread_mutex_lock(&StateMutex);
	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		vrrp_syslog_error("vrrp %d not create profile node.\n", profile);
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (NULL == (vsrv = hansiNode->vlist)) {
		vrrp_syslog_error("vrrp %d not create vrrp node.\n", profile);
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (VRRP_SERVICE_ENABLE == service_enable[profile]) {
		vrrp_syslog_error("vrrp %d service enabled.\n", profile);
		ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else {
		switch (up_down_flg)
		{
			case VRRP_LINK_TYPE_UPLINK :
				{
					set_flag = vsrv->uplink_flag;
					linkIf = &(vsrv->uplink_vif[0]);
					if(link_local){
    					ipPtr = &(vsrv->uplink_local_ipv6_vaddr[0]);
					}
					else 
						ipPtr = &(vsrv->uplink_ipv6_vaddr[0]);

					naddr = vsrv->uplink_naddr;
					break;
				}
			case VRRP_LINK_TYPE_DOWNLINK :
				{
					set_flag = vsrv->downlink_flag;
					linkIf = &(vsrv->downlink_vif[0]);
					naddr = vsrv->downlink_naddr;
					ptrFd = &(vsrv->downlink_fd[index]);
					if(link_local){
    					ipPtr = &(vsrv->downlink_local_ipv6_vaddr[0]);
					}
					else 
						ipPtr = &(vsrv->downlink_ipv6_vaddr[0]);
					break;
				}
			case VRRP_LINK_TYPE_VGATEWAY:
				{
					set_flag = vsrv->vgateway_flag;
					linkIf = &(vsrv->vgateway_vif[0]);
					naddr = vsrv->vgateway_naddr;
					if(link_local){
    					ipPtr = &(vsrv->vgateway_local_ipv6_vaddr[0]);
					}
					else 
						ipPtr = &(vsrv->vgateway_ipv6_vaddr[0]);

					ptrFd = NULL;
				}
				break;
			case VRRP_LINK_TYPE_L2_UPLINK :
				{
					set_flag = vsrv->l2_uplink_flag;
					linkIf = &(vsrv->l2_uplink_vif[0]);
					naddr = vsrv->l2_uplink_naddr;
					ptrFd = NULL;
				}
				break;//hjw add
			default :
				{
					vrrp_syslog_error("not such link type %d\n", up_down_flg);
					ret = VRRP_RETURN_CODE_ERR;
					break;
				}
		}
		if (VRRP_LINK_SETTED != set_flag)
		{	/* check if the link setted */
			vrrp_syslog_error("delete %s vip faild, not setted right (link+priority) mode.\n",
								VRRP_LINK_TYPE_DESCANT(up_down_flg));
			ret = VRRP_RETURN_CODE_LINKMODE_NO_SETTED;
		}
		else if (VRRP_RETURN_CODE_IF_NOT_EXIST == had_get_link_index_by_ifname(vsrv, ifname, up_down_flg, &index))
		{	/* check the link interface name exist. */
			ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
		}
		else if((VRRP_LINK_TYPE_L2_UPLINK == up_down_flg)&& (VRRP_RETURN_CODE_VIP_NOT_EXIST == had_check_link_vip6_exist(vsrv, &vip6, prefix_length,link_local, up_down_flg)))
		{	/* check if the virtual ip has setted.*/
			ret = VRRP_RETURN_CODE_VIP_NOT_EXIST;
		}
		else if ((1 == naddr)&&(VRRP_LINK_TYPE_VGATEWAY != up_down_flg)&&(VRRP_LINK_TYPE_L2_UPLINK != up_down_flg))
		{	/* if the virtual ip is last, not allow to delete.*/
			ret = VRRP_RETURN_CODE_VIP_LAST_ONE;
		}
		else
		{
			if(vsrv){
				switch (up_down_flg)
				{
					case VRRP_LINK_TYPE_UPLINK :
					{
						ptrFd = &(vsrv->uplink_fd[index]);
						break;
					}
					case VRRP_LINK_TYPE_DOWNLINK :
					{
						ptrFd = &(vsrv->downlink_fd[index]);
						break;
					}
					case VRRP_LINK_TYPE_VGATEWAY:
					{
						ptrFd = NULL;
						break;
					}
					case VRRP_LINK_TYPE_L2_UPLINK :
					{
						ptrFd = NULL;
						break;//hjw add
					}
					default :
					{
						break;
					}
				}			/* close socket fd */
			if(ptrFd) {
				close(*ptrFd);
				*ptrFd = 0;
				}
			}
			/* delelte downlink from downlink_vif */
			ret = had_del_vip_from_linkvif(vsrv, up_down_flg, ifname, index);
			if (VRRP_RETURN_CODE_OK != ret) {
				vrrp_syslog_error("delete interface %s from %s_vif[%d] error, ret %x.\n",
									ifname,
									VRRP_LINK_TYPE_DESCANT(up_down_flg),
									index,
									ret);
				ret = VRRP_RETURN_CODE_ERR;
			}
			else
			{
				if(VRRP_LINK_TYPE_L2_UPLINK != up_down_flg){
					/* delete downlink virtual ip*/
					vrrp_syslog_info("had_delete_vipv6: ip6 "NIP6QUAD_FMT"/ !\n",NIP6QUAD(vip6->iabuf), prefix_length);			
					had_cfg_vlink_ip6addr_change(vsrv, up_down_flg, ifname, VRRP_VIP_OPT_TYPE_DEL, vip6, prefix_length, index,link_local);
				}
				else{
					vsrv->l2_uplink_naddr --;
				}
				/*had_cfg_delete_downlink_ipaddr(vsrv, vip, mask, index);*/
				vrrp_syslog_info("delete %s %d vip6 "NIP6QUAD_FMT"/%d success.\n",
								VRRP_LINK_TYPE_DESCANT(up_down_flg),index,NIP6QUAD(vip6->iabuf),prefix_length);
				ret = VRRP_RETURN_CODE_OK;
			}
		}
		
		/* for debug */
		int i = 0;
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == linkIf[i].set_flg) {
					vrrp_syslog_info("had_delete_vipv6 %s vip6 addr "NIP6QUAD_FMT"/%d\n",
                            		VRRP_LINK_TYPE_DESCANT(up_down_flg),
                            		NIP6QUAD(vip6->iabuf),
                            		prefix_length);
			}
		}
	}
	pthread_mutex_unlock(&StateMutex);

	return ret;
}

/*
 *******************************************************************************
 *had_add_vip()
 *
 *  DESCRIPTION:
 *		add link virtual ip, and fill uplink_vif|downlink_vif.
 *		before add must setted linkmode(linktype + priority)
 *
 *  INPUTS:
 *		unsigned int profile,			- vrid
 *		char *uplink_ifname,			- uplink interface name
 *		unsigned long uplink_ip,		- virtual ip
 *		unsigned int uplink_mask,		- mask
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR						- null parameter or error linktype
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST			- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_LINKMODE_NO_SETTED			- not setted linkmode
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip has exist
 *		VRRP_RETURN_CODE_IF_EXIST					- the interface has exist
 *		VRRP_RETURN_CODE_OK							- add success
 *
 *******************************************************************************
 */
unsigned int had_add_vip
(
	unsigned int profile,
	char *ifname,
	unsigned long vip,
	unsigned int mask,
	unsigned int up_down_flg
)
{
	unsigned int ret = VRRP_RETURN_CODE_OK;
	vrrp_rt *vsrv = NULL;
	hansi_s *hansiNode = NULL;
	int index = -1, i = 0;
	unsigned int set_flag = VRRP_LINK_NO_SETTED;
	vip_addr *ipPtr = NULL;
	vrrp_if *linkIf = NULL;

	if (!ifname) {
		vrrp_syslog_error("vrrp %d add %s virtual ip null parameter error!\n",
							profile,
							VRRP_LINK_TYPE_DESCANT(up_down_flg));
		return VRRP_RETURN_CODE_ERR;
	}
	
	pthread_mutex_lock(&StateMutex);
	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		vrrp_syslog_error("vrrp %d not create profile node.\n", profile);
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (NULL == (vsrv = hansiNode->vlist)) {
		vrrp_syslog_error("vrrp %d not create vrrp node.\n", profile);
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (VRRP_SERVICE_ENABLE == service_enable[profile]) {
		vrrp_syslog_error("vrrp %d service enabled.\n", profile);
		ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else {
		switch (up_down_flg)
		{
			case VRRP_LINK_TYPE_UPLINK :
				{
					set_flag = vsrv->uplink_flag;
					ipPtr = &(vsrv->uplink_vaddr[0]);
					linkIf = &(vsrv->uplink_vif[0]);
					#if 0
					if (VRRP_LINK_SETTED != set_flag)
					{	/* check if the link setted */
						vrrp_syslog_error("add %s vip faild, not setted right (link+priority) mode.\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg));
						ret = VRRP_RETURN_CODE_LINKMODE_NO_SETTED;
					}
					else if (VRRP_RETURN_CODE_IF_EXIST == had_get_link_index_by_ifname(vsrv, ifname, up_down_flg, &index))
					{	/* check the link interface name exist. */
						ret = VRRP_RETURN_CODE_IF_EXIST;
					}
					else if (VRRP_RETURN_CODE_VIP_EXIST == had_check_link_vip_exist(vsrv, vip, mask, up_down_flg))
					{	/* check if the virtual ip has setted.*/
						ret = VRRP_RETURN_CODE_VIP_EXIST;
					}
					else
					{	/* add uplink to uplink_vif */
						/* index is first empty position in uplink_vif array */
						ret = had_add_vip_to_linkvif(profile, vsrv, up_down_flg, ifname, index);
						if (VRRP_RETURN_CODE_OK != ret) {
							vrrp_syslog_error("add interface %s to %s index %d error!\n",
											ifname,
											VRRP_LINK_TYPE_DESCANT(up_down_flg),
											index);

							ret = VRRP_RETURN_CODE_ERR;
							break;
						}
						else
						{	/* add uplink virtual ip*/
							had_cfg_add_uplink_ipaddr(vsrv, vip, mask, index);
							vrrp_syslog_dbg("add %s vip success.\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg));
							ret = VRRP_RETURN_CODE_OK;
						}
					}
					
					/* for debug */
					int i = 0;
					for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
					{
						if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) {
							vrrp_syslog_dbg("%s vip %d.%d.%d.%d/%d\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg),
											(vsrv->uplink_vaddr[i].addr >> 24) & 0xFF,
											(vsrv->uplink_vaddr[i].addr >> 16) & 0xFF,
											(vsrv->uplink_vaddr[i].addr >> 8) & 0xFF,
											(vsrv->uplink_vaddr[i].addr) & 0xFF,
											vsrv->uplink_vaddr[i].mask);
						}
					}
					#endif
					break;
				}
			case VRRP_LINK_TYPE_DOWNLINK :
				{
					set_flag = vsrv->downlink_flag;
					ipPtr = &(vsrv->downlink_vaddr[0]);
					linkIf = &(vsrv->downlink_vif[0]);
					#if 0
					if (VRRP_LINK_SETTED != vsrv->downlink_flag)
					{	/* check if the link setted */
						vrrp_syslog_error("add %s vip faild, not setted right (link+priority) mode.\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg));
						ret = VRRP_RETURN_CODE_LINKMODE_NO_SETTED;
					}
					else if (VRRP_RETURN_CODE_IF_EXIST == had_get_link_index_by_ifname(vsrv, ifname, up_down_flg, &index))
					{	/* check the link interface name exist. */
						ret = VRRP_RETURN_CODE_IF_EXIST;
					}
					else if (VRRP_RETURN_CODE_VIP_EXIST == had_check_link_vip_exist(vsrv, vip, mask, up_down_flg))
					{	/* check if the virtual ip has setted.*/
						ret = VRRP_RETURN_CODE_VIP_EXIST;
					}
					else
					{	/* add downlink to downlink_vif */
						/* index is first empty position in downlink_vif array */
						ret = had_add_vip_to_linkvif(profile, vsrv, up_down_flg, ifname, index);
						if (VRRP_RETURN_CODE_OK != ret) {
							vrrp_syslog_error("add interface %s to %s index %d error!\n",
											ifname,
											VRRP_LINK_TYPE_DESCANT(up_down_flg),
											index);
							ret = VRRP_RETURN_CODE_ERR;
						}
						else
						{	/* add downlink virtual ip*/
							had_cfg_add_downlink_ipaddr(vsrv, vip, mask, index);
							vrrp_syslog_dbg("add %s vip success.\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg));
							ret = VRRP_RETURN_CODE_OK;
						}
					}
					
					#if 1
					/* for debug */
					int i = 0;
					for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
					{
						if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) {
							vrrp_syslog_dbg("%s vip %d.%d.%d.%d/%d\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg),
											(vsrv->downlink_vaddr[i].addr >> 24) & 0xFF,
											(vsrv->downlink_vaddr[i].addr >> 16) & 0xFF,
											(vsrv->downlink_vaddr[i].addr >> 8) & 0xFF,
											(vsrv->downlink_vaddr[i].addr) & 0xFF,
											vsrv->downlink_vaddr[i].mask);
						}
					}
					#endif
					#endif
					break;
				}
			case VRRP_LINK_TYPE_VGATEWAY:
				{
					set_flag = VRRP_LINK_SETTED;
					ipPtr = &(vsrv->vgateway_vaddr[0]);
					linkIf = &(vsrv->vgateway_vif[0]);
				}
				break;
			case VRRP_LINK_TYPE_L2_UPLINK:
				{
					set_flag = VRRP_LINK_SETTED;
					linkIf = &(vsrv->l2_uplink_vif[0]);
				}
				break;
			default :
				{
					vrrp_syslog_error("not such link type %d\n", up_down_flg);
					ret = VRRP_RETURN_CODE_ERR;
					break;
				}
		}
		
		if(VRRP_RETURN_CODE_OK == ret) {
			if (VRRP_LINK_SETTED != set_flag)
			{	/* check if the link setted */
				vrrp_syslog_error("add %s vip faild, not setted right (link+priority) mode.\n",
									VRRP_LINK_TYPE_DESCANT(up_down_flg));
				ret = VRRP_RETURN_CODE_LINKMODE_NO_SETTED;
			}
			else if (VRRP_RETURN_CODE_IF_EXIST == had_get_link_index_by_ifname(vsrv, ifname, up_down_flg, &index))
			{	/* check the link interface name exist. */
				ret = VRRP_RETURN_CODE_IF_EXIST;
			}
			else if((VRRP_LINK_TYPE_L2_UPLINK != up_down_flg)&& (VRRP_RETURN_CODE_VIP_EXIST == had_check_link_vip_exist(vsrv, vip, mask, up_down_flg)))
			{	/* check if the virtual ip has setted.*/
				ret = VRRP_RETURN_CODE_VIP_EXIST;
			}
			else
			{	/* add uplink to uplink_vif */
				/* index is first empty position in uplink_vif array */
				ret = had_add_vip_to_linkvif(profile, vsrv, up_down_flg, ifname, index);
				if (VRRP_RETURN_CODE_OK != ret) {
					vrrp_syslog_error("add interface %s to %s index %d error!\n",
									ifname,
									VRRP_LINK_TYPE_DESCANT(up_down_flg),
									index);
				}
				else
				{	
					if(VRRP_LINK_TYPE_L2_UPLINK != up_down_flg){
						/* add uplink virtual ip*/
						had_cfg_vlink_ipaddr_change(vsrv, up_down_flg, VRRP_VIP_OPT_TYPE_ADD, vip, mask, index);
					}
					else{
						vsrv->l2_uplink_naddr ++;
					}
					vrrp_syslog_dbg("add %s %d ip %#x mask %#x success.\n",  \
							VRRP_LINK_TYPE_DESCANT(up_down_flg), index, vip, mask);
				}
			}
			
			/* for debug */
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == linkIf[i].set_flg) {
					vrrp_syslog_dbg("%s vip %d.%d.%d.%d/%d\n",
									VRRP_LINK_TYPE_DESCANT(up_down_flg),
									(ipPtr[i].addr >> 24) & 0xFF,
									(ipPtr[i].addr >> 16) & 0xFF,
									(ipPtr[i].addr >> 8) & 0xFF,
									(ipPtr[i].addr) & 0xFF,
									ipPtr[i].mask);
				}
			}
		}
	}
	pthread_mutex_unlock(&StateMutex);

	return ret;
}

/*
 *******************************************************************************
 *had_delete_vip()
 *
 *  DESCRIPTION:
 *		delete link virtual ip, and delete uplink_vif|downlink_vif,
 *		before delete must be sure setted linkmode(linktype + priority),
 *		and virtual ip which configured by "config uplink | downlink priority"
 *		is not allow to delete.
 *
 *  INPUTS:
 *		unsigned int profile,			- vrid
 *		char *uplink_ifname,			- uplink interface name
 *		unsigned long uplink_ip,		- virtual ip
 *		unsigned int uplink_mask,		- mask
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR						- null parameter or error linktype
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST			- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_LINKMODE_NO_SETTED			- not setted linkmode
 *		VRRP_RETURN_CODE_IFNAME_ERROR				- link interface name error
 *		VRRP_RETURN_CODE_VIP_NOT_EXIST				- the virtual ip has not exist
 *		VRRP_RETURN_CODE_VIP_LAST_ONE				- virtual ip is last one, not allow to deletes
 *
 *******************************************************************************
 */
unsigned int had_delete_vip
(
	unsigned int profile,
	char *ifname,
	unsigned long vip,
	unsigned int mask,
	unsigned int up_down_flg
)
{
	unsigned int ret = VRRP_RETURN_CODE_OK;
	vrrp_rt *vsrv = NULL;
	hansi_s *hansiNode = NULL;
	int index = -1;
	unsigned int set_flag = VRRP_LINK_NO_SETTED;
	int naddr = 0;
	vip_addr *ipPtr = NULL;
	vrrp_if *linkIf = NULL;
	int *ptrFd = NULL;

	if (!ifname) {
		vrrp_syslog_error("vrrp %d delete %s virtual ip null parameter error!\n",
							profile,
							VRRP_LINK_TYPE_DESCANT(up_down_flg));
		return VRRP_RETURN_CODE_ERR;
	}
	
	pthread_mutex_lock(&StateMutex);
	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		vrrp_syslog_error("vrrp %d not create profile node.\n", profile);
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (NULL == (vsrv = hansiNode->vlist)) {
		vrrp_syslog_error("vrrp %d not create vrrp node.\n", profile);
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (VRRP_SERVICE_ENABLE == service_enable[profile]) {
		vrrp_syslog_error("vrrp %d service enabled.\n", profile);
		ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else {
		switch (up_down_flg)
		{
			case VRRP_LINK_TYPE_UPLINK :
				{
					set_flag = vsrv->uplink_flag;
					ipPtr = &(vsrv->uplink_vaddr[0]);
					linkIf = &(vsrv->uplink_vif[0]);
					naddr = vsrv->uplink_naddr;
					//ptrFd = &(vsrv->uplink_fd[index]);
					#if 0
					if (VRRP_LINK_SETTED != vsrv->uplink_flag)
					{	/* check if the link setted */
						vrrp_syslog_error("delete %s vip faild, not setted right (link+priority) mode.\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg));
						ret = VRRP_RETURN_CODE_LINKMODE_NO_SETTED;
					}
					else if (VRRP_RETURN_CODE_IF_NOT_EXIST == had_get_link_index_by_ifname(vsrv, ifname, up_down_flg, &index))
					{	/* check the link interface name exist. */
						ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
					}
					else if (VRRP_RETURN_CODE_VIP_NOT_EXIST == had_check_link_vip_exist(vsrv, vip, mask, up_down_flg))
					{	/* check if the virtual ip has setted.*/
						ret = VRRP_RETURN_CODE_VIP_NOT_EXIST;
					}
					#if 1
					else if (1 == vsrv->uplink_naddr)
					{	/* if the virtual ip is last, not allow to delete.*/
						ret = VRRP_RETURN_CODE_VIP_LAST_ONE;
					}
					#endif
					else
					{
						/* close socket fd */
						close(vsrv->uplink_fd[index]);
						vsrv->uplink_fd[index] = 0;
						
						/* delelte uplink from uplink_vif */
						ret = had_del_vip_from_linkvif(vsrv, up_down_flg, ifname, index);
						if (VRRP_RETURN_CODE_OK != ret) {
							vrrp_syslog_error("delete interface %s from %s_vif[%d] error, ret %x.\n",
												ifname,
												VRRP_LINK_TYPE_DESCANT(up_down_flg),
												index,
												ret);
							ret = VRRP_RETURN_CODE_ERR;
						}
						else
						{
							/* delete uplink virtual ip*/
							had_cfg_delete_uplink_ipaddr(vsrv, vip, mask, index);
							vrrp_syslog_dbg("delete %s vip success.\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg));
							ret = VRRP_RETURN_CODE_OK;
						}
					}
					
					/* for debug */
					int i = 0;
					for (i = 0; i < vsrv->uplink_naddr; i++)
					{
						vrrp_syslog_dbg("%s vip %d.%d.%d.%d/%d\n",
										VRRP_LINK_TYPE_DESCANT(up_down_flg),
										(vsrv->uplink_vaddr[i].addr >> 24) & 0xFF,
										(vsrv->uplink_vaddr[i].addr >> 16) & 0xFF,
										(vsrv->uplink_vaddr[i].addr >> 8) & 0xFF,
										(vsrv->uplink_vaddr[i].addr) & 0xFF,
										vsrv->uplink_vaddr[i].mask);				
					}
					#endif
					break;
				}
			case VRRP_LINK_TYPE_DOWNLINK :
				{
					set_flag = vsrv->downlink_flag;
					ipPtr = &(vsrv->downlink_vaddr[0]);
					linkIf = &(vsrv->downlink_vif[0]);
					naddr = vsrv->downlink_naddr;
					ptrFd = &(vsrv->downlink_fd[index]);
					#if 0
					if (VRRP_LINK_SETTED != vsrv->downlink_flag)
					{	/* check if the link setted */
						vrrp_syslog_error("delete %s vip faild, not setted right (link+priority) mode.\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg));
						ret = VRRP_RETURN_CODE_LINKMODE_NO_SETTED;
					}
					else if (VRRP_RETURN_CODE_IF_NOT_EXIST == had_get_link_index_by_ifname(vsrv, ifname, up_down_flg, &index))
					{	/* check the link interface name exist. */
						ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
					}
					else if (VRRP_RETURN_CODE_VIP_NOT_EXIST == had_check_link_vip_exist(vsrv, vip, mask, up_down_flg))
					{	/* check if the virtual ip has setted.*/
						ret = VRRP_RETURN_CODE_VIP_NOT_EXIST;
					}
					#if 1
					else if (1 == vsrv->downlink_naddr)
					{	/* if the virtual ip is last, not allow to delete.*/
						ret = VRRP_RETURN_CODE_VIP_LAST_ONE;
					}
					#endif
					else
					{
						/* close socket fd */
						close(vsrv->downlink_fd[index]);
						vsrv->downlink_fd[index] = 0;

						/* delelte downlink from downlink_vif */
						ret = had_del_vip_from_linkvif(vsrv, up_down_flg, ifname, index);
						if (VRRP_RETURN_CODE_OK != ret) {
							vrrp_syslog_error("delete interface %s from %s_vif[%d] error, ret %x.\n",
												ifname,
												VRRP_LINK_TYPE_DESCANT(up_down_flg),
												index,
												ret);
							ret = VRRP_RETURN_CODE_ERR;
						}
						else
						{
							/* delete downlink virtual ip*/
							had_cfg_delete_downlink_ipaddr(vsrv, vip, mask, index);
							vrrp_syslog_dbg("delete %s vip success.\n",
											VRRP_LINK_TYPE_DESCANT(up_down_flg));
							ret = VRRP_RETURN_CODE_OK;
						}
					}
					
					/* for debug */
					int i = 0;
					for (i = 0; i < vsrv->downlink_naddr; i++)
					{
						vrrp_syslog_dbg("%s vip %d.%d.%d.%d/%d\n",
										VRRP_LINK_TYPE_DESCANT(up_down_flg),
										(vsrv->downlink_vaddr[i].addr >> 24) & 0xFF,
										(vsrv->downlink_vaddr[i].addr >> 16) & 0xFF,
										(vsrv->downlink_vaddr[i].addr >> 8) & 0xFF,
										(vsrv->downlink_vaddr[i].addr) & 0xFF,
										vsrv->downlink_vaddr[i].mask);				
					}
					#endif
					break;
				}
			case VRRP_LINK_TYPE_VGATEWAY:
				{
					set_flag = vsrv->vgateway_flag;
					ipPtr = &(vsrv->vgateway_vaddr[0]);
					linkIf = &(vsrv->vgateway_vif[0]);
					naddr = vsrv->vgateway_naddr;
					ptrFd = NULL;
				}
				break;
			case VRRP_LINK_TYPE_L2_UPLINK :
				{
					set_flag = vsrv->l2_uplink_flag;
					linkIf = &(vsrv->l2_uplink_vif[0]);
					naddr = vsrv->l2_uplink_naddr;
					ptrFd = NULL;
				}
				break;//hjw add
			default :
				{
					vrrp_syslog_error("not such link type %d\n", up_down_flg);
					ret = VRRP_RETURN_CODE_ERR;
					break;
				}
		}
#if 1
		if (VRRP_LINK_SETTED != set_flag)
		{	/* check if the link setted */
			vrrp_syslog_error("delete %s vip faild, not setted right (link+priority) mode.\n",
								VRRP_LINK_TYPE_DESCANT(up_down_flg));
			ret = VRRP_RETURN_CODE_LINKMODE_NO_SETTED;
		}
		else if (VRRP_RETURN_CODE_IF_NOT_EXIST == had_get_link_index_by_ifname(vsrv, ifname, up_down_flg, &index))
		{	/* check the link interface name exist. */
			ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
		}
		else if((VRRP_LINK_TYPE_L2_UPLINK == up_down_flg)&& (VRRP_RETURN_CODE_VIP_NOT_EXIST == had_check_link_vip_exist(vsrv, vip, mask, up_down_flg)))
		{	/* check if the virtual ip has setted.*/
			ret = VRRP_RETURN_CODE_VIP_NOT_EXIST;
		}
		else if ((1 == naddr)&&(VRRP_LINK_TYPE_VGATEWAY != up_down_flg)&&(VRRP_LINK_TYPE_L2_UPLINK != up_down_flg))
		{	/* if the virtual ip is last, not allow to delete.*/
			ret = VRRP_RETURN_CODE_VIP_LAST_ONE;
		}
		else
		{
			if(vsrv){
				switch (up_down_flg)
				{
					case VRRP_LINK_TYPE_UPLINK :
					{
						ptrFd = &(vsrv->uplink_fd[index]);
						break;
					}
					case VRRP_LINK_TYPE_DOWNLINK :
					{
						ptrFd = &(vsrv->downlink_fd[index]);
						break;
					}
					case VRRP_LINK_TYPE_VGATEWAY:
					{
						ptrFd = NULL;
						break;
					}
					case VRRP_LINK_TYPE_L2_UPLINK :
					{
						ptrFd = NULL;
						break;//hjw add
					}
					default :
					{
						break;
					}
				}			/* close socket fd */
			if(ptrFd) {
				close(*ptrFd);
				*ptrFd = 0;
				}
			}
			/* delelte downlink from downlink_vif */
			ret = had_del_vip_from_linkvif(vsrv, up_down_flg, ifname, index);
			if (VRRP_RETURN_CODE_OK != ret) {
				vrrp_syslog_error("delete interface %s from %s_vif[%d] error, ret %x.\n",
									ifname,
									VRRP_LINK_TYPE_DESCANT(up_down_flg),
									index,
									ret);
				ret = VRRP_RETURN_CODE_ERR;
			}
			else
			{
				if(VRRP_LINK_TYPE_L2_UPLINK != up_down_flg){
					/* delete downlink virtual ip*/
						had_cfg_vlink_ipaddr_change(vsrv, up_down_flg, VRRP_VIP_OPT_TYPE_DEL, vip, mask, index);
                        had_cfg_vlink_ip6addr_change(vsrv, up_down_flg, ifname, VRRP_VIP_OPT_TYPE_DEL, NULL, 0, index,1);
    					had_cfg_vlink_ip6addr_change(vsrv, up_down_flg, ifname, VRRP_VIP_OPT_TYPE_DEL, NULL, 0, index,0);
				}
				else{
					vsrv->l2_uplink_naddr --;
				}
				/*had_cfg_delete_downlink_ipaddr(vsrv, vip, mask, index);*/
				vrrp_syslog_dbg("delete %s vip success.\n",
								VRRP_LINK_TYPE_DESCANT(up_down_flg));
				ret = VRRP_RETURN_CODE_OK;
			}
		}
		
		/* for debug */
		int i = 0;
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == linkIf[i].set_flg) {
				vrrp_syslog_dbg("%s vip %d.%d.%d.%d/%d\n",
								VRRP_LINK_TYPE_DESCANT(up_down_flg),
								(ipPtr[i].addr >> 24) & 0xFF,
								(ipPtr[i].addr >> 16) & 0xFF,
								(ipPtr[i].addr >> 8) & 0xFF,
								(ipPtr[i].addr) & 0xFF,
								ipPtr[i].mask);
			}
		}
#endif
	}
	pthread_mutex_unlock(&StateMutex);

	return ret;
}

/****************************************************************
 NAME	: had_init_virtual_srv			00/02/06 09:18:02
 AIM	:
 REMARK	:
****************************************************************/
static void had_init_virtual_srv( vrrp_rt *vsrv )
{
	memset( vsrv, 0, sizeof(*vsrv) );
	vsrv->state	= VRRP_STATE_INIT;
	vsrv->wantstate = VRRP_STATE_INIT;
	vsrv->priority	= VRRP_PRIO_DFL;
	vsrv->adver_int	= VRRP_ADVER_DFL*VRRP_TIMER_HZ;
	vsrv->smart_vmac_enable = VRRP_SMART_VMAC_ENABLE;
	vsrv->preempt	= VRRP_PREEMPT_DFL;
	/* add by jinpengcheng,
	 * default is open
	 */
	vsrv->notify_flg = VRRP_NOTIFY_OBJ_DFL;
	/* default is open */	
	vsrv->vgateway_tf_flag = VRRP_VGATEWAY_TF_FLG_OFF;
	
	/* init had failover peer and local ip address */
	vsrv->failover.peerip = ~0UI;
	vsrv->failover.localip = ~0UI;
	return;
}

/****************************************************************
 NAME	: had_chk_min_cfg				00/02/06 17:07:45
 AIM	: TRUE if the minimal configuration isnt done
 REMARK	:
****************************************************************/
static int had_chk_min_cfg
(
	vrrp_rt *vsrv
)
{
	/* check if one of (uplink|downlink) or bath virtual ip is setted. */
	if ((vsrv->uplink_naddr == 0) &&
		(vsrv->downlink_naddr == 0))
	{
		vrrp_syslog_error("check configure:neither uplink nor downlink ip for virtual server\n");
		return -1;
	}

#if 0
	/* check vrid if setted */
	if (vsrv->vrid == 0)
	{
		vrrp_syslog_error("check configure:the virtual id must be set!\n");
		return -1;
	}
#endif

	/* check if one of (uplink|downlink) or bath real ip is setted. */
	if ((vsrv->uplink_vif[0].ipaddr == 0) &&
		(vsrv->downlink_vif[0].ipaddr == 0))
	{
		vrrp_syslog_error("check configure:the interface ip must be set uplink or downlink!\n");
		return -1;
	}

	/* make vrrp use the native hwaddr and not the virtual one */
	/*
	if( vsrv->no_vmac ){
		memcpy( vrrp_hwaddr, vsrv->vif.hwaddr,sizeof(vsrv->vif.hwaddr));
	}
	*/
	return(0);
}

/*
 *******************************************************************************
 * had_cfg_dhcp_failover()
 *
 *  DESCRIPTION:
 *		config dhcp failover settings such as peer and local interface's ip address.
 *
 *  INPUTS:
 *		profile	- hansi profile
 *		peerip 	- peer side ip address
 *		localip	- loca ip address
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************/
int had_cfg_dhcp_failover
(
	int profile,
	uint32_t peerip,
	uint32_t localip
)
{
	vrrp_rt* vrrp = NULL;
	hansi_s* hansiNode = had_get_profile_node(profile);
	
	if (NULL == hansiNode) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	vrrp = hansiNode->vlist;
	if (NULL == vrrp) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	/* check if hansi failover is already setted. */
	if ((~0UI == vrrp->failover.peerip) || (~0UI == vrrp->failover.localip)) {
		vrrp_syslog_error("hansi %d failover change peer[%#x -> %#x] local[%#x -> %#x]\n", 
						profile, vrrp->failover.peerip, peerip, vrrp->failover.localip, localip);
	}
	else {
		vrrp_syslog_dbg("set hansi %d dhcp failover peer %#x local %#x\n",
						profile, peerip, localip);
	}
	vrrp->failover.peerip = peerip;
	vrrp->failover.localip = localip;

	
	return VRRP_RETURN_CODE_OK;
}

/****************************************************************
 NAME	: had_send_gratuitous_arp			00/05/11 11:56:30
 AIM	:
 REMARK	: rfc0826
	: WORK: ugly because heavily hardcoded for ethernet
****************************************************************/
int had_send_gratuitous_arp( char* ifname, char* mac,int addr )
{
struct m_arphdr
  {
    unsigned short int ar_hrd;          /* Format of hardware address.  */
    unsigned short int ar_pro;          /* Format of protocol address.  */
    unsigned char ar_hln;               /* Length of hardware address.  */
    unsigned char ar_pln;               /* Length of protocol address.  */
    unsigned short int ar_op;           /* ARP opcode (command).  */
    /* Ethernet looks like this : This bit is variable sized however...  */
    unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
    unsigned char __ar_sip[4];          /* Sender IP address.  */
    unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
    unsigned char __ar_tip[4];          /* Target IP address.  */
  };
    vrrp_rt* vsrv= NULL;
	char	buf[sizeof(struct m_arphdr)+ETHER_HDR_LEN];
	char	buflen	= sizeof(struct m_arphdr)+ETHER_HDR_LEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct m_arphdr	*arph = (struct m_arphdr *)(buf+vrrp_dlt_len(vsrv));
	char vrrp_hwaddr[6];
	/*
	memcpy( vrrp_hwaddr, mac,6);
	*/
	vrrp_hwaddr[0] = mac[0];
	vrrp_hwaddr[1] = mac[1];
	vrrp_hwaddr[2] = mac[2];
	vrrp_hwaddr[3] = mac[3];
	vrrp_hwaddr[4] = mac[4];
	vrrp_hwaddr[5] = mac[5];
	
	
	char	*hwaddr	= vrrp_hwaddr;
	int	hwlen	= ETH_ALEN;

	/* hardcoded for ethernet */
	memset( eth->ether_dhost, 0xFF, ETH_ALEN );
	memcpy( eth->ether_shost, hwaddr, hwlen );
	eth->ether_type	= htons(ETHERTYPE_ARP);

	/* build the arp payload */
	memset( arph, 0, sizeof( *arph ) );
	arph->ar_hrd	= htons(ARPHRD_ETHER);
	arph->ar_pro	= htons(ETHERTYPE_IP);
	arph->ar_hln	= 6;
	arph->ar_pln	= 4;
	arph->ar_op	= htons(ARPOP_REQUEST);
	memcpy( arph->__ar_sha, vrrp_hwaddr, hwlen );
	memset( arph->__ar_tha, 0xFF, hwlen );
	addr = htonl(addr);
	memcpy( arph->__ar_sip, &addr, sizeof(addr) );
	memcpy( arph->__ar_tip, &addr, sizeof(addr) );
	vrrp_syslog_dbg("ifname %s send gratuitous arp,srcMac: %02x:%02x:%02x:%02x:%02x:%02x,ip %d.%d.%d.%d\n",\
		  ifname,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],\
		   (addr & 0xff000000)>>24,(addr & 0xff0000)>>16,(addr & 0xff00)>>8,addr & 0xff);
	return vrrp_send_pkt_arp( ifname, buf, buflen );
}

/****************************************************************
 NAME	: had_send_vgateway_gratuitous_arp			00/05/11 11:56:30
 AIM	:
 REMARK	: rfc0826
	: WORK: ugly because heavily hardcoded for ethernet
****************************************************************/
int had_send_vgateway_gratuitous_arp
(
	vrrp_rt *vsrv, 
	int addr, 
	int index,
	int vAddrF
)
{
struct m_arphdr
  {
    unsigned short int ar_hrd;          /* Format of hardware address.  */
    unsigned short int ar_pro;          /* Format of protocol address.  */
    unsigned char ar_hln;               /* Length of hardware address.  */
    unsigned char ar_pln;               /* Length of protocol address.  */
    unsigned short int ar_op;           /* ARP opcode (command).  */
    /* Ethernet looks like this : This bit is variable sized however...  */
    unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
    unsigned char __ar_sip[4];          /* Sender IP address.  */
    unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
    unsigned char __ar_tip[4];          /* Target IP address.  */
  };
	char	buf[sizeof(struct m_arphdr)+ETHER_HDR_LEN];
	char	buflen	= sizeof(struct m_arphdr)+ETHER_HDR_LEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct m_arphdr	*arph = (struct m_arphdr *)(buf+vrrp_dlt_len(vsrv));
	char vrrp_hwaddr[6], zeroMac[ETH_ALEN] = {0};
	memset(zeroMac, 0, ETH_ALEN);
	if( vsrv->no_vmac ){
		if(!memcmp(vsrv->vgateway_vif[index].hwaddr, zeroMac, ETH_ALEN)){
			vrrp_syslog_warning("vgateway interface %s hwaddr is zero mac when send gratuitous arp, rewrite it as vrrp global mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
				vsrv->vgateway_vif[index].ifname, vrrp_global_mac[0], vrrp_global_mac[1], vrrp_global_mac[2], vrrp_global_mac[3], vrrp_global_mac[4], vrrp_global_mac[5]);
			memcpy(vsrv->vgateway_vif[index].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		}
		memcpy( vrrp_hwaddr, vsrv->vgateway_vif[index].hwaddr,sizeof(vsrv->vgateway_vif[index].hwaddr));
	}
	else{
		vrrp_hwaddr[0] = 0x00;
		vrrp_hwaddr[1] = 0x00;
		vrrp_hwaddr[2] = 0x5E;
		vrrp_hwaddr[3] = 0x00;
		vrrp_hwaddr[4] = 0x01;
		vrrp_hwaddr[5] = vsrv->vrid;
	}
	char	*hwaddr	= vAddrF ? vrrp_hwaddr : vsrv->vgateway_vif[index].hwaddr;
	int	hwlen	= ETH_ALEN;

	/* hardcoded for ethernet */
	memset( eth->ether_dhost, 0xFF, ETH_ALEN );
	memcpy( eth->ether_shost, hwaddr, hwlen );
	eth->ether_type	= htons(ETHERTYPE_ARP);

	/* build the arp payload */
	memset( arph, 0, sizeof( *arph ) );
	arph->ar_hrd	= htons(ARPHRD_ETHER);
	arph->ar_pro	= htons(ETHERTYPE_IP);
	arph->ar_hln	= 6;
	arph->ar_pln	= 4;
	arph->ar_op	= htons(ARPOP_REQUEST);
	memcpy( arph->__ar_sha, hwaddr, hwlen );
	addr = htonl(addr);
	memcpy( arph->__ar_sip, &addr, sizeof(addr) );
	memcpy( arph->__ar_tip, &addr, sizeof(addr) );
	vrrp_syslog_dbg("ifname %s send gratuitous arp,srcMac: %02x:%02x:%02x:%02x:%02x:%02x,ip %d.%d.%d.%d\n",\
		  vsrv->vgateway_vif[index].ifname,
		  hwaddr[0],hwaddr[1],hwaddr[2],
		  hwaddr[3],hwaddr[4],hwaddr[5],\
		   (addr & 0xff000000)>>24,
		   (addr & 0xff0000)>>16,
		   (addr & 0xff00)>>8,
		   addr & 0xff);
	return vrrp_send_pkt_arp( vsrv->vgateway_vif[index].ifname, buf, buflen );
}


/****************************************************************
 NAME	: had_send_uplink_gratuitous_arp			00/05/11 11:56:30
 AIM	:
 REMARK	: rfc0826
	: WORK: ugly because heavily hardcoded for ethernet
****************************************************************/
int had_send_uplink_gratuitous_arp
(
	vrrp_rt *vsrv,
	int addr,
	int index,
	int vAddrF
)
{
	struct m_arphdr
	{
		unsigned short int ar_hrd;          /* Format of hardware address.  */
		unsigned short int ar_pro;          /* Format of protocol address.  */
		unsigned char ar_hln;               /* Length of hardware address.  */
		unsigned char ar_pln;               /* Length of protocol address.  */
		unsigned short int ar_op;           /* ARP opcode (command).  */
		/* Ethernet looks like this : This bit is variable sized however...  */
		unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
		unsigned char __ar_sip[4];          /* Sender IP address.  */
		unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
		unsigned char __ar_tip[4];          /* Target IP address.  */
	};
	char	buf[sizeof(struct m_arphdr)+ETHER_HDR_LEN];
	char	buflen	= sizeof(struct m_arphdr)+ETHER_HDR_LEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct m_arphdr	*arph = (struct m_arphdr *)(buf+vrrp_dlt_len(vsrv));
	char vrrp_hwaddr[6] = {0}, zeroMac[ETH_ALEN] = {0};
	memset(zeroMac, 0, ETH_ALEN);
	if( vsrv->no_vmac ){
		if(!memcmp(vsrv->uplink_vif[index].hwaddr, zeroMac, ETH_ALEN)){
			vrrp_syslog_warning("uplink interface %s hwaddr is zero mac when send gratuitous arp, rewrite it as vrrp global mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
				vsrv->uplink_vif[index].ifname, vrrp_global_mac[0], vrrp_global_mac[1], vrrp_global_mac[2], vrrp_global_mac[3], vrrp_global_mac[4], vrrp_global_mac[5]);
			memcpy(vsrv->uplink_vif[index].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		}
		memcpy( vrrp_hwaddr, vsrv->uplink_vif[index].hwaddr,sizeof(vsrv->uplink_vif[index].hwaddr));
	}
	else{
		vrrp_hwaddr[0] = 0x00;
		vrrp_hwaddr[1] = 0x00;
		vrrp_hwaddr[2] = 0x5E;
		vrrp_hwaddr[3] = 0x00;
		vrrp_hwaddr[4] = 0x01;
		vrrp_hwaddr[5] = vsrv->vrid;
	}
	char *hwaddr = vAddrF ? vrrp_hwaddr : vsrv->uplink_vif[index].hwaddr;
	int	hwlen = ETH_ALEN;

	/* hardcoded for ethernet */
	memset( eth->ether_dhost, 0xFF, ETH_ALEN );
	memcpy( eth->ether_shost, hwaddr, hwlen );
	eth->ether_type	= htons(ETHERTYPE_ARP);

	/* build the arp payload */
	memset( arph, 0, sizeof( *arph ) );
	arph->ar_hrd	= htons(ARPHRD_ETHER);
	arph->ar_pro	= htons(ETHERTYPE_IP);
	arph->ar_hln	= 6;
	arph->ar_pln	= 4;
	arph->ar_op	= htons(ARPOP_REQUEST);
	memcpy( arph->__ar_sha, hwaddr, hwlen );
	addr = htonl(addr);
	memcpy( arph->__ar_sip, &addr, sizeof(addr) );
	memcpy( arph->__ar_tip, &addr, sizeof(addr) );
	
	vrrp_syslog_dbg("ifname %s send gratuitous arp,srcMac: %02x:%02x:%02x:%02x:%02x:%02x,ip %d.%d.%d.%d\n",\
					vsrv->uplink_vif[index].ifname,
					hwaddr[0], hwaddr[1], hwaddr[2],
					hwaddr[3], hwaddr[4], hwaddr[5],	\
					(addr & 0xff000000) >> 24,
					(addr & 0xff0000) >> 16,
					(addr & 0xff00) >> 8,
					addr & 0xff);
	
	return vrrp_send_pkt_arp(vsrv->uplink_vif[index].ifname, buf, buflen);
}



/****************************************************************
 NAME	: had_send_downlink_gratuitous_arp			00/05/11 11:56:30
 AIM	:
 REMARK	: rfc0826
	: WORK: ugly because heavily hardcoded for ethernet
****************************************************************/
int had_send_downlink_gratuitous_arp
(
	vrrp_rt *vsrv,
	int addr,
	int index,
	int vAddrF
)
{
	struct m_arphdr
	{
		unsigned short int ar_hrd;          /* Format of hardware address.  */
		unsigned short int ar_pro;          /* Format of protocol address.  */
		unsigned char ar_hln;               /* Length of hardware address.  */
		unsigned char ar_pln;               /* Length of protocol address.  */
		unsigned short int ar_op;           /* ARP opcode (command).  */
		/* Ethernet looks like this : This bit is variable sized however...  */
		unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
		unsigned char __ar_sip[4];          /* Sender IP address.  */
		unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
		unsigned char __ar_tip[4];          /* Target IP address.  */
	};
	char	buf[sizeof(struct m_arphdr)+ETHER_HDR_LEN];
	char	buflen	= sizeof(struct m_arphdr)+ETHER_HDR_LEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct m_arphdr	*arph = (struct m_arphdr *)(buf+vrrp_dlt_len(vsrv));
	char vrrp_hwaddr[6] = {0}, zeroMac[ETH_ALEN] = {0};
	memset(zeroMac, 0, ETH_ALEN);
	if( vsrv->no_vmac ){
		if(!memcmp(vsrv->downlink_vif[index].hwaddr, zeroMac, ETH_ALEN)){
			vrrp_syslog_warning("downlink interface %s hwaddr is zero mac when send gratuitous arp, rewrite it as vrrp global mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
				vsrv->downlink_vif[index].ifname, vrrp_global_mac[0], vrrp_global_mac[1], vrrp_global_mac[2], vrrp_global_mac[3], vrrp_global_mac[4], vrrp_global_mac[5]);
			memcpy(vsrv->downlink_vif[index].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		}
		memcpy( vrrp_hwaddr, vsrv->downlink_vif[index].hwaddr,sizeof(vsrv->downlink_vif[index].hwaddr));
	}
	else{
		vrrp_hwaddr[0] = 0x00;
		vrrp_hwaddr[1] = 0x00;
		vrrp_hwaddr[2] = 0x5E;
		vrrp_hwaddr[3] = 0x00;
		vrrp_hwaddr[4] = 0x01;
		vrrp_hwaddr[5] = vsrv->vrid;
	}
	char	*hwaddr	= vAddrF ? vrrp_hwaddr : vsrv->downlink_vif[index].hwaddr;
	int	hwlen	= ETH_ALEN;

	/* hardcoded for ethernet */
	memset( eth->ether_dhost, 0xFF, ETH_ALEN );
	memcpy( eth->ether_shost, hwaddr, hwlen );
	eth->ether_type	= htons(ETHERTYPE_ARP);

	/* build the arp payload */
	memset( arph, 0, sizeof( *arph ) );
	arph->ar_hrd	= htons(ARPHRD_ETHER);
	arph->ar_pro	= htons(ETHERTYPE_IP);
	arph->ar_hln	= 6;
	arph->ar_pln	= 4;
	arph->ar_op	= htons(ARPOP_REQUEST);
	memcpy( arph->__ar_sha, hwaddr, hwlen );
	addr = htonl(addr);
	memcpy( arph->__ar_sip, &addr, sizeof(addr) );
	memcpy( arph->__ar_tip, &addr, sizeof(addr) );
	vrrp_syslog_dbg("ifname %s send gratuitous arp,srcMac: %02x:%02x:%02x:%02x:%02x:%02x,ip %d.%d.%d.%d\n",\
		   vsrv->downlink_vif[index].ifname,
		   hwaddr[0], hwaddr[1], hwaddr[2],
		   hwaddr[3], hwaddr[4], hwaddr[5],
		   (addr & 0xff000000) >> 24,
		   (addr & 0xff0000) >> 16,
		   (addr & 0xff00) >> 8,
		   addr & 0xff);
	
	return vrrp_send_pkt_arp( vsrv->downlink_vif[index].ifname, buf, buflen );
}

/****************************************************************
 NAME	: state_gotomaster			00/02/07 00:15:26
 AIM	:
 REMARK	: called when the state is now MASTER
****************************************************************/
char *had_ipaddr_to_str(uint32_t ipaddr)
{
	static char temp_ipaddr[32];
	snprintf(temp_ipaddr, 32, "%d.%d.%d.%d", 
			(unsigned char)(ipaddr & 0xff),
			(unsigned char)((ipaddr >> 8) & 0xff),
			(unsigned char)((ipaddr >> 16) & 0xff),
			(unsigned char)((ipaddr >> 24) & 0xff));
	return temp_ipaddr;
}


vrrp_rt* had_check_if_exist
(
   unsigned int vrid
)
{
   int profile = 1;
   hansi_s *hansiNode = NULL;
   vrrp_rt* vrrp = NULL;
   for (;profile < MAX_HANSI_PROFILE; profile++) {
      hansiNode = had_get_profile_node(profile);
	   if(hansiNode == NULL){
          continue;
	   }
	   else if(NULL == (vrrp = hansiNode->vlist)){
	   	  continue;
	   }
	   else if(vrid == vrrp->vrid){
          return vrrp;
	   }
   }

   return NULL;
   
}
int had_get_profile_by_vrid
(
   int vrid
)
{
   hansi_s* hansi = NULL;
   vrrp_rt* vrrp = NULL;
   int profile = 1;
   for(; profile < MAX_HANSI_PROFILE; profile++) {
      hansi = had_get_profile_node(profile);
	  if(NULL != hansi){
         if((NULL != (vrrp= hansi->vlist))&&(vrid == vrrp->vrid)){
             return profile;
		 }
	  }
	  else{
         continue;
	  }
   }
   return 0;
}

int had_notify_to_npd
(
    char* ifname1,
    char* ifname2
)
{
   	DBusMessage *query, *reply;
	DBusError err;
    	int op_ret = 0;
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_OBJPATH,			\
								NPD_DBUS_INTERFACE,					\
								NPD_DBUS_FDB_METHOD_CREATE_VRRP_BY_IFNAME);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&ifname1,
							 DBUS_TYPE_STRING,&ifname2,
							 DBUS_TYPE_INVALID);

	vrrp_syslog_dbg("vrrp send uplink ifname %s downlink ifname %s and wait reply!\n",ifname1,ifname2);
	reply = dbus_connection_send_with_reply_and_block (vrrp_notify_dbus_connection,query,3000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vrrp_syslog_dbg("Failed to get reply!\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
			vrrp_syslog_dbg("Failed to get reply!\n");
		}
	}
	else{
		vrrp_syslog_dbg("get return value!\n");
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
                vrrp_syslog_dbg("get return value %d\n",op_ret);
			} 
			else {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
					vrrp_syslog_dbg("failed to get return value!\n");
				}
			}
			dbus_message_unref(reply);
	}
	
	return op_ret;

}

/*
 *******************************************************************************
 *had_get_link_first_setted_index()
 *
 *  DESCRIPTION:
 *		check if link(uplink or downlink) is setted,
 *		if setted, output its first setted index.
 *		if not setted, output -1.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int up_down_flg,		- link type
 *
 *  OUTPUTS:
 *		int *index						- link index		
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_IF_EXIST		- the interface is exist.
 *		VRRP_RETURN_CODE_IF_NOT_EXIST	- the interface is not exist.
 *
 *******************************************************************************
 */
unsigned int had_get_link_first_setted_index
(
	vrrp_rt *vsrv,
	unsigned int up_down_flg,
	int *index
)
{
	unsigned int ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int first_index = -1;
	unsigned int i = 0;

	if (!vsrv || !index) {
		vrrp_syslog_error("get first setted index null parameter error\n");
		return VRRP_RETURN_CODE_IF_NOT_EXIST;
	}

	switch (up_down_flg)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{	/* be sure set_flg = 1 */
					if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
					{
						first_index = i;
						ret = VRRP_RETURN_CODE_IF_EXIST;
						break;
					}
				}
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					/* be sure set_flg = 1 */
					if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
					{
						first_index = i;
						ret = VRRP_RETURN_CODE_IF_EXIST;
						break;
					}
				}
				break;
			}
		default :
			{
				vrrp_syslog_error("not such link type %d\n", up_down_flg);
				ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
				break;
			}
	}

	/* if setted, output its first setted index.
	 * if not setted, output -1.
	 */
	if (VRRP_RETURN_CODE_IF_EXIST == ret)
	{
		*index = first_index;
		vrrp_syslog_dbg("first setted index in %s[%d].\n",
						VRRP_LINK_TYPE_DESCANT(up_down_flg),
						first_index);

	}else {
		*index = -1;
		vrrp_syslog_dbg("%s not setted.\n",
						VRRP_LINK_TYPE_DESCANT(up_down_flg));
	}

	return ret;
}

int had_notify_to_protal
(
	vrrp_rt *vrrp,
	int state
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1, iter_array_vgw;

	int op_ret = 0;
	int master_uplinkip = 0;
	int master_downlinkip = 0;
	int back_uplinkip = 0;
	int back_downlinkip = 0;
	char *uplink_ifname = NULL;
	char *downlink_ifname = NULL;
	char *heartbeatlink_if = NULL;
	char *vgateway_if = NULL;
	int vgateway_ip = 0;
    int virtual_uplink_ip = 0;
    int virtual_downlink_ip = 0;
	int uplink_cnt = 0, downlink_cnt = 0, vgateway_cnt = 0;
	int i = 0;
	
	int ret_uplink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int ret_downlink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int uplink_first_index = -1;
	int downlink_first_index = -1;

	if (!vrrp) {
		vrrp_syslog_error("notify to portal null parameter error\n");
		return -1;
	} 
	
	if (0 == global_protal) {
		return 0;
	}

	/* bit[1] is portal */
	if (!(vrrp->notify_flg & VRRP_NOTIFY_BIT_PORTAL)) {
		vrrp_syslog_dbg("notify flg %x, no notify to portal\n",
						vrrp->notify_flg);
		return -1;
	}
	
	global_notifying_flag = VRRP_NOTIFY_PORTAL;
	global_notify_count[global_notifying_flag] = 0;
	/*trace log*/
    vrrp_state_trace(vrrp->profile, state,
    				"NOTIFY PORTAL","start to notify");

/*
DBUS_TYPE_UINT32					// vrid
DBUS_TYPE_UINT32					// state
DBUS_TYPE_UINT32					// count of uplink interfaces
Array of uplink
	DBUS_TYPE_UINT32_AS_STRING		// master uplnik ip address
	DBUS_TYPE_UINT32_AS_STRING		// back uplink ip address
	DBUS_TYPE_UINT32_AS_STRING		// virtual uplink ip address
	DBUS_TYPE_STRING_AS_STRING		// uplink interface name
DBUS_TYPE_UINT32					// count of downlink interfaces
Array of uplink
	DBUS_TYPE_UINT32_AS_STRING		// master downlnik ip address
	DBUS_TYPE_UINT32_AS_STRING		// back downlink ip address
	DBUS_TYPE_UINT32_AS_STRING		// virtual downlink ip address
	DBUS_TYPE_STRING_AS_STRING		// downlink interface name
DBUS_TYPE_STRING					// heartbeat interface name
DBUS_TYPE_UINT32					// heartbeat ip address
DBUS_TYPE_UINT32					// opposite heartbeat ip address
*/
#define EAG_DBUS_NAME_FMT		"aw.eag_%s_%d"
#define EAG_DBUS_OBJPATH_FMT	"/aw/eag_%s_%d"
#define EAG_DBUS_INTERFACE_FMT	"aw.eag_%s_%d"

	char EAG_DBUS_NAME[64];
	char EAG_DBUS_OBJPATH[64];
	char EAG_DBUS_INTERFACE[64];
	snprintf(EAG_DBUS_NAME,sizeof(EAG_DBUS_NAME)-1,     /* should be hansi id, not vrip. 2013-06-28 ZD.*/
				EAG_DBUS_NAME_FMT,"r",vrrp->profile );
	snprintf(EAG_DBUS_OBJPATH,sizeof(EAG_DBUS_NAME)-1,
				EAG_DBUS_OBJPATH_FMT,"r",vrrp->profile );
	snprintf(EAG_DBUS_INTERFACE,sizeof(EAG_DBUS_NAME)-1,
				EAG_DBUS_INTERFACE_FMT,"r",vrrp->profile );
	query = dbus_message_new_method_call(
				                         EAG_DBUS_NAME,
				                         EAG_DBUS_OBJPATH,
										 EAG_DBUS_INTERFACE,
										 EAG_DBUS_SET_STATE);
	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(vrrp->profile));    /* should be hansi id, not vrip. 2013-06-28 ZD.*/   
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &state);
	vrrp_syslog_event("notify to portal,hansi %d, vrid %d, state %d\n",
						vrrp->profile, vrrp->vrid, state);

	uplink_cnt = vrrp->uplink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &uplink_cnt);
	vrrp_syslog_event("uplink count %d\n",
						uplink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	/* get uplink first setted index */
	ret_uplink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_UPLINK, &uplink_first_index);
	/* get downlink first setted index */
	ret_downlink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_DOWNLINK, &downlink_first_index);

#if 1
	/* uplink */
	#if 0
	if (VRRP_LINK_NO_SETTED == vrrp->uplink_flag)
	{
		if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
		{
			uplink_ifname = vrrp->downlink_vif[downlink_first_index].ifname;
		}
		virtual_uplink_ip = 0;
		switch (state)
		{
			case VRRP_STATE_MAST :
			{
				master_uplinkip = 0;
				back_uplinkip = 0
				break;
			}
			case VRRP_STATE_BACK :
			{
				master_uplinkip = 0;
				back_uplinkip = 0;
				break;
			}
			case VRRP_STATE_DISABLE :
			{
				master_uplinkip = 0;
				back_uplinkip = 0;
				break;
			}
		}
		vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
							uplink_ifname,
							master_uplinkip,
							back_uplinkip,
							virtual_uplink_ip);
		
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &master_uplinkip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &back_uplinkip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &virtual_uplink_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING, &uplink_ifname);

		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	else {
	#endif
	if (VRRP_LINK_SETTED == vrrp->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->uplink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			uplink_ifname = NULL;
			virtual_uplink_ip = 0;
			master_uplinkip = 0;
			back_uplinkip = 0;
			
			uplink_ifname = vrrp->uplink_vif[i].ifname;
			virtual_uplink_ip = vrrp->uplink_vaddr[i].addr;
			
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_uplinkip = vrrp->uplink_vif[i].ipaddr;
					back_uplinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = vrrp->uplink_vif[i].ipaddr;
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = 0;
					break;
				}
			}

			vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
								uplink_ifname,
								master_uplinkip,
								back_uplinkip,
								virtual_uplink_ip);
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container(&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &master_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &back_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &virtual_uplink_ip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_STRING, &uplink_ifname);

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array);

	downlink_cnt = vrrp->downlink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &downlink_cnt);
	vrrp_syslog_event("downlink count %d\n",
						downlink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);
	
	/* downlink */
	#if 0
	if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag)
	{
		downlink_cnt = 0;
		if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
		{
			downlink_ifname = vrrp->uplink_vif[uplink_first_index].ifname;
		}
		downlink_ifname = vrrp->uplink_vif.ifname;
		virtual_downlink_ip = 0;
		switch (state)
		{
			case VRRP_STATE_MAST :
			{
				master_downlinkip = 0;
				back_downlinkip = 0;
				break;
			}
			case VRRP_STATE_BACK :
			{
				master_downlinkip = 0;
				back_downlinkip = 0;
				break;
			}
			case VRRP_STATE_DISABLE :
			{
				master_downlinkip = 0;
				back_downlinkip = 0;
				break;
			}
		}
		vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
							downlink_ifname,
							master_downlinkip,
							back_downlinkip,
							virtual_downlink_ip);
		
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &master_downlinkip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &back_downlinkip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &virtual_downlink_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING, &vgateway_if);
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	else {
	#endif
	if (VRRP_LINK_SETTED == vrrp->downlink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->downlink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			downlink_ifname = NULL;
			virtual_downlink_ip = 0;
			master_downlinkip = 0;
			back_downlinkip = 0;
		
			downlink_ifname = vrrp->downlink_vif[i].ifname;
			virtual_downlink_ip = vrrp->downlink_vaddr[i].addr;
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_downlinkip = vrrp->downlink_vif[i].ipaddr;
					back_downlinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = vrrp->downlink_vif[i].ipaddr;	
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = 0;
					break;
				}
			}
			vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
					downlink_ifname,
					master_downlinkip,
					back_downlinkip,
					virtual_downlink_ip);
			
			DBusMessageIter iter_struct1;
			dbus_message_iter_open_container(&iter_array1,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct1);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &master_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &back_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &virtual_downlink_ip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_STRING, &downlink_ifname);
			dbus_message_iter_close_container (&iter_array1, &iter_struct1);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array1);
#endif

	if(NULL != global_ht_ifname){
		heartbeatlink_if = global_ht_ifname;
	}
	else{
		if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
			{
				heartbeatlink_if = vrrp->uplink_vif[uplink_first_index].ifname;
			}
		}else {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
			{
				heartbeatlink_if = vrrp->downlink_vif[downlink_first_index].ifname;
			}
		}
	}
	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &heartbeatlink_if);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_ip);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_opposite_ip);
	vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x\n",
						heartbeatlink_if, global_ht_ip, global_ht_opposite_ip);
#if 0
	/* vgateway interface */
	if(NULL != vrrp->vgateway_vif.ifname){
		vgateway_if = vrrp->vgateway_vif.ifname;
		vgateway_ip = vrrp->vgateway_vif.ipaddr;
	}
	else{
		if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
			{
				vgateway_if = vrrp->uplink_vif[uplink_first_index].ifname;
				vgateway_ip = vrrp->uplink_vif[uplink_first_index].ipaddr;
			}
		}else {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
			{
				vgateway_if = vrrp->downlink_vif[downlink_first_index].ifname;
				vgateway_ip = vrrp->downlink_vif[downlink_first_index].ipaddr;
			}
		}
	}
	dbus_message_iter_append_basic(&iter,
				  				DBUS_TYPE_STRING, &vgateway_if);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32, &vgateway_ip);

	vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x, vgateway %s ip %#x\n",
						heartbeatlink_if, global_ht_ip, global_ht_opposite_ip, vgateway_if, vgateway_ip);
#endif
	/* vgateway interface */
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &vrrp->vgateway_naddr);
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array_vgw);
	if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
	{
		vgateway_cnt = 0;
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED != vrrp->vgateway_vif[i].set_flg && 
					vgateway_cnt < vrrp->vgateway_naddr)
			{
				vgateway_if = vrrp->vgateway_vif[i].ifname;
				vgateway_ip = vrrp->vgateway_vaddr[i].addr; 					
				
				DBusMessageIter iter_struct_vgw;
				dbus_message_iter_open_container(&iter_array_vgw,
												DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct_vgw);
				dbus_message_iter_append_basic(&iter_struct_vgw,
							  DBUS_TYPE_STRING, &vgateway_if);
				dbus_message_iter_append_basic(&iter_struct_vgw,
							  DBUS_TYPE_UINT32, &vgateway_ip);
				dbus_message_iter_close_container (&iter_array_vgw, &iter_struct_vgw);
				vrrp_syslog_event("%-10s %d %s ip %#x\n", vgateway_cnt ? "":"vgateway",
									vgateway_cnt, vgateway_if, vgateway_ip);
				vgateway_cnt++;
				vgateway_if = NULL;
			}
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array_vgw);
	#if 0
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&vrid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_UINT32,&master_uplinkip,
							 DBUS_TYPE_UINT32,&master_downlinkip,
							 DBUS_TYPE_UINT32,&back_uplinkip,
							 DBUS_TYPE_UINT32,&back_downlinkip,	
							 DBUS_TYPE_UINT32,&virtual_uplink_ip,
							 DBUS_TYPE_UINT32,&virtual_downlink_ip,
							 DBUS_TYPE_STRING,&uplink_ifname,
							 DBUS_TYPE_STRING,&vgateway_if,
							 DBUS_TYPE_STRING,&heartbeatlink_if,
							 DBUS_TYPE_UINT32,&global_ht_ip,
							 DBUS_TYPE_UINT32,&global_ht_opposite_ip,								 
							 DBUS_TYPE_INVALID);
	#endif
	
	reply = dbus_connection_send_with_reply_and_block (vrrp_notify_dbus_connection,query,60000, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vrrp_syslog_dbg("had %d state %d notify portal failed to get reply!\n", vrrp->vrid, state);
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else{
		if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&op_ret,
									DBUS_TYPE_INVALID))
		{
			vrrp_syslog_event("Value return from portal %d!\n",op_ret);
		} 
		else {
			if (dbus_error_is_set(&err)) {
				vrrp_syslog_error("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
		}
		dbus_message_unref(reply);
	}

	
	/*trace log*/
	vrrp_state_trace(vrrp->profile,state,"NOTIFY PORTAL","notify over");
	global_notify_count[global_notifying_flag] = 0;
	global_notifying_flag = VRRP_NOTIFY_NONE;
	return op_ret;
}

#ifndef _VERSION_18SP7_
	int had_notify_to_pppoe
(
	vrrp_rt *vrrp,
	int state
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1, iter_array_vgw;

	int op_ret = 0;
	int master_uplinkip = 0;
	int master_downlinkip = 0;
	int back_uplinkip = 0;
	int back_downlinkip = 0;
	char *uplink_ifname = NULL;
	char *downlink_ifname = NULL;
	char *heartbeatlink_if = NULL;
	char *vgateway_if = NULL;
	int vgateway_ip = 0;
    int virtual_uplink_ip = 0;
    int virtual_downlink_ip = 0;
	int uplink_cnt = 0, downlink_cnt = 0, vgateway_cnt = 0;
	int i = 0;
	
	int ret_uplink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int ret_downlink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int uplink_first_index = -1;
	int downlink_first_index = -1;

	if (!vrrp) {
		vrrp_syslog_error("notify to portal null parameter error\n");
		return -1;
	} 
	
	if (0 == global_pppoe) {
		return 0;
	}

	/* bit[1] is portal */
	if (!(vrrp->notify_flg & VRRP_NOTIFY_BIT_PPPOE)) {
		vrrp_syslog_dbg("notify flg %x, no notify to pppoe\n",
						vrrp->notify_flg);
		return -1;
	}
	
	global_notifying_flag = VRRP_NOTIFY_PPPOE;
	global_notify_count[global_notifying_flag] = 0;
	/*trace log*/
    vrrp_state_trace(vrrp->profile, state,
    				"NOTIFY PPPOE","start to notify");

/*
DBUS_TYPE_UINT32					// vrid
DBUS_TYPE_UINT32					// state
DBUS_TYPE_UINT32					// count of uplink interfaces
Array of uplink
	DBUS_TYPE_UINT32_AS_STRING		// master uplnik ip address
	DBUS_TYPE_UINT32_AS_STRING		// back uplink ip address
	DBUS_TYPE_UINT32_AS_STRING		// virtual uplink ip address
	DBUS_TYPE_STRING_AS_STRING		// uplink interface name
DBUS_TYPE_UINT32					// count of downlink interfaces
Array of uplink
	DBUS_TYPE_UINT32_AS_STRING		// master downlnik ip address
	DBUS_TYPE_UINT32_AS_STRING		// back downlink ip address
	DBUS_TYPE_UINT32_AS_STRING		// virtual downlink ip address
	DBUS_TYPE_STRING_AS_STRING		// downlink interface name
DBUS_TYPE_STRING					// heartbeat interface name
DBUS_TYPE_UINT32					// heartbeat ip address
DBUS_TYPE_UINT32					// opposite heartbeat ip address
*/
#define PPPOE_DBUS_NAME_FMT		"aw.pppoe%d_%d"
#define PPPOE_DBUS_OBJPATH_FMT	"/aw/pppoe"
#define PPPOE_DBUS_INTERFACE_FMT	"aw.pppoe"

	char PPPOE_DBUS_NAME[64];

	snprintf(PPPOE_DBUS_NAME,sizeof(PPPOE_DBUS_NAME)-1,
				PPPOE_DBUS_NAME_FMT, 0,vrrp->profile);        /* should be hansi id, not vrip. 2013-06-28 ZD.*/

	query = dbus_message_new_method_call(
				                         PPPOE_DBUS_NAME,
				                         PPPOE_DBUS_OBJPATH_FMT,
										 PPPOE_DBUS_INTERFACE_FMT,
										 PPPOE_DBUS_INSTANCE_VRRP_SWITCH);
	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(vrrp->profile));    /* should be hansi id, not vrip. 2013-06-28 ZD.*/
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &state);
	vrrp_syslog_event("notify to pppoe,hansi %d, state %d\n",
						vrrp->profile, state);

	uplink_cnt = vrrp->uplink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &uplink_cnt);
	vrrp_syslog_event("uplink count %d\n",
						uplink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	/* get uplink first setted index */
	ret_uplink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_UPLINK, &uplink_first_index);
	/* get downlink first setted index */
	ret_downlink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_DOWNLINK, &downlink_first_index);

#if 1
	/* uplink */
	#if 0
	if (VRRP_LINK_NO_SETTED == vrrp->uplink_flag)
	{
		if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
		{
			uplink_ifname = vrrp->downlink_vif[downlink_first_index].ifname;
		}
		virtual_uplink_ip = 0;
		switch (state)
		{
			case VRRP_STATE_MAST :
			{
				master_uplinkip = 0;
				back_uplinkip = 0
				break;
			}
			case VRRP_STATE_BACK :
			{
				master_uplinkip = 0;
				back_uplinkip = 0;
				break;
			}
			case VRRP_STATE_DISABLE :
			{
				master_uplinkip = 0;
				back_uplinkip = 0;
				break;
			}
		}
		vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
							uplink_ifname,
							master_uplinkip,
							back_uplinkip,
							virtual_uplink_ip);
		
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &master_uplinkip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &back_uplinkip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &virtual_uplink_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING, &uplink_ifname);

		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	else {
	#endif
	if (VRRP_LINK_SETTED == vrrp->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->uplink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			uplink_ifname = NULL;
			virtual_uplink_ip = 0;
			master_uplinkip = 0;
			back_uplinkip = 0;
			
			uplink_ifname = vrrp->uplink_vif[i].ifname;
			virtual_uplink_ip = vrrp->uplink_vaddr[i].addr;
			
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_uplinkip = vrrp->uplink_vif[i].ipaddr;
					back_uplinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = vrrp->uplink_vif[i].ipaddr;
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = 0;
					break;
				}
			}

			vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
								uplink_ifname,
								master_uplinkip,
								back_uplinkip,
								virtual_uplink_ip);
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container(&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &master_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &back_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &virtual_uplink_ip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_STRING, &uplink_ifname);

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array);

	downlink_cnt = vrrp->downlink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &downlink_cnt);
	vrrp_syslog_event("downlink count %d\n",
						downlink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);
	
	/* downlink */
	#if 0
	if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag)
	{
		downlink_cnt = 0;
		if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
		{
			downlink_ifname = vrrp->uplink_vif[uplink_first_index].ifname;
		}
		downlink_ifname = vrrp->uplink_vif.ifname;
		virtual_downlink_ip = 0;
		switch (state)
		{
			case VRRP_STATE_MAST :
			{
				master_downlinkip = 0;
				back_downlinkip = 0;
				break;
			}
			case VRRP_STATE_BACK :
			{
				master_downlinkip = 0;
				back_downlinkip = 0;
				break;
			}
			case VRRP_STATE_DISABLE :
			{
				master_downlinkip = 0;
				back_downlinkip = 0;
				break;
			}
		}
		vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
							downlink_ifname,
							master_downlinkip,
							back_downlinkip,
							virtual_downlink_ip);
		
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &master_downlinkip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &back_downlinkip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &virtual_downlink_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING, &vgateway_if);
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	else {
	#endif
	if (VRRP_LINK_SETTED == vrrp->downlink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->downlink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			downlink_ifname = NULL;
			virtual_downlink_ip = 0;
			master_downlinkip = 0;
			back_downlinkip = 0;
		
			downlink_ifname = vrrp->downlink_vif[i].ifname;
			virtual_downlink_ip = vrrp->downlink_vaddr[i].addr;
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_downlinkip = vrrp->downlink_vif[i].ipaddr;
					back_downlinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = vrrp->downlink_vif[i].ipaddr;	
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = 0;
					break;
				}
			}
			vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
					downlink_ifname,
					master_downlinkip,
					back_downlinkip,
					virtual_downlink_ip);
			
			DBusMessageIter iter_struct1;
			dbus_message_iter_open_container(&iter_array1,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct1);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &master_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &back_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &virtual_downlink_ip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_STRING, &downlink_ifname);
			dbus_message_iter_close_container (&iter_array1, &iter_struct1);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array1);
#endif

	if(NULL != global_ht_ifname){
		heartbeatlink_if = global_ht_ifname;
	}
	else{
		if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
			{
				heartbeatlink_if = vrrp->uplink_vif[uplink_first_index].ifname;
			}
		}else {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
			{
				heartbeatlink_if = vrrp->downlink_vif[downlink_first_index].ifname;
			}
		}
	}
	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &heartbeatlink_if);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_ip);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_opposite_ip);
	vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x\n",
						heartbeatlink_if, global_ht_ip, global_ht_opposite_ip);
#if 0
	/* vgateway interface */
	if(NULL != vrrp->vgateway_vif.ifname){
		vgateway_if = vrrp->vgateway_vif.ifname;
		vgateway_ip = vrrp->vgateway_vif.ipaddr;
	}
	else{
		if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
			{
				vgateway_if = vrrp->uplink_vif[uplink_first_index].ifname;
				vgateway_ip = vrrp->uplink_vif[uplink_first_index].ipaddr;
			}
		}else {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
			{
				vgateway_if = vrrp->downlink_vif[downlink_first_index].ifname;
				vgateway_ip = vrrp->downlink_vif[downlink_first_index].ipaddr;
			}
		}
	}
	dbus_message_iter_append_basic(&iter,
				  				DBUS_TYPE_STRING, &vgateway_if);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32, &vgateway_ip);

	vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x, vgateway %s ip %#x\n",
						heartbeatlink_if, global_ht_ip, global_ht_opposite_ip, vgateway_if, vgateway_ip);
#endif
	/* vgateway interface */
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &vrrp->vgateway_naddr);
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array_vgw);
	if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
	{
		vgateway_cnt = 0;
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED != vrrp->vgateway_vif[i].set_flg && 
					vgateway_cnt < vrrp->vgateway_naddr)
			{
				vgateway_if = vrrp->vgateway_vif[i].ifname;
				vgateway_ip = vrrp->vgateway_vaddr[i].addr; 					
				
				DBusMessageIter iter_struct_vgw;
				dbus_message_iter_open_container(&iter_array_vgw,
												DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct_vgw);
				dbus_message_iter_append_basic(&iter_struct_vgw,
							  DBUS_TYPE_STRING, &vgateway_if);
				dbus_message_iter_append_basic(&iter_struct_vgw,
							  DBUS_TYPE_UINT32, &vgateway_ip);
				dbus_message_iter_close_container (&iter_array_vgw, &iter_struct_vgw);
				vrrp_syslog_event("%-10s %d %s ip %#x\n", vgateway_cnt ? "":"vgateway",
									vgateway_cnt, vgateway_if, vgateway_ip);
				vgateway_cnt++;
				vgateway_if = NULL;
			}
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array_vgw);
	#if 0
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&vrid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_UINT32,&master_uplinkip,
							 DBUS_TYPE_UINT32,&master_downlinkip,
							 DBUS_TYPE_UINT32,&back_uplinkip,
							 DBUS_TYPE_UINT32,&back_downlinkip,	
							 DBUS_TYPE_UINT32,&virtual_uplink_ip,
							 DBUS_TYPE_UINT32,&virtual_downlink_ip,
							 DBUS_TYPE_STRING,&uplink_ifname,
							 DBUS_TYPE_STRING,&vgateway_if,
							 DBUS_TYPE_STRING,&heartbeatlink_if,
							 DBUS_TYPE_UINT32,&global_ht_ip,
							 DBUS_TYPE_UINT32,&global_ht_opposite_ip,								 
							 DBUS_TYPE_INVALID);
	#endif
	
	reply = dbus_connection_send_with_reply_and_block (vrrp_notify_dbus_connection,query,60000, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vrrp_syslog_dbg("had %d state %d notify portal failed to get reply!\n", vrrp->vrid, state);
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else{
		if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&op_ret,
									DBUS_TYPE_INVALID))
		{
			vrrp_syslog_event("Value return from portal %d!\n",op_ret);
		} 
		else {
			if (dbus_error_is_set(&err)) {
				vrrp_syslog_error("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
		}
		dbus_message_unref(reply);
	}

	
	/*trace log*/
	vrrp_state_trace(vrrp->profile,state,"NOTIFY PORTAL","notify over");
	global_notify_count[global_notifying_flag] = 0;
	global_notifying_flag = VRRP_NOTIFY_NONE;
	return op_ret;
}
#endif

/* book add for had_notify_hmd, 2011-5-10 */
int had_notify_to_hmd
(
	vrrp_rt *vrrp,
    int state
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1, iter_array_vgw;
    
	int op_ret = 0;
	int master_uplinkip = 0;
	int master_downlinkip = 0;
	int back_uplinkip = 0;
	int back_downlinkip = 0;
	char *uplink_ifname = NULL;
	char *downlink_ifname = NULL;
	char *heartbeatlink_if = NULL;
	char *vgateway_if = NULL;
	int vgateway_ip = 0;
    int virtual_uplink_ip = 0;
    int virtual_downlink_ip = 0;
	int uplink_cnt = 0, downlink_cnt = 0, vgateway_cnt = 0;
	int i = 0;
	
	int ret_uplink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int ret_downlink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int uplink_first_index = -1;
	int downlink_first_index = -1;

	if (!vrrp) {
		vrrp_syslog_error("notify to hmd null parameter error\n");
		return -1;
	} 
	
	global_notifying_flag = VRRP_NOTIFY_HMD;
	global_notify_count[global_notifying_flag] = 0;
	/*trace log*/
    vrrp_state_trace(vrrp->profile, state,
    				"NOTIFY HMD","start to notify");

    
	query = dbus_message_new_method_call(
				                         HMD_DBUS_BUSNAME,
				                         HMD_DBUS_OBJPATH,
										 HMD_DBUS_INTERFACE,
										 HMD_DBUS_METHOD_HAD_NOTIFY_HMD_STATE);
	dbus_error_init(&err);
	dbus_message_iter_init_append(query, &iter);

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(vrrp->profile));    /* should be hansi id, not vrip. 2013-06-28 ZD.*/
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &state);
	vrrp_syslog_event("notify to hmd,hansi %d, vrid %d, state %d\n",
						vrrp->profile, vrrp->vrid, state);

	uplink_cnt = vrrp->uplink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &uplink_cnt);
	vrrp_syslog_event("uplink count %d\n",
						uplink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	/* get uplink first setted index */
	ret_uplink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_UPLINK, &uplink_first_index);
	/* get downlink first setted index */
	ret_downlink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_DOWNLINK, &downlink_first_index);

	/* uplink */
	if (VRRP_LINK_SETTED == vrrp->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->uplink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			uplink_ifname = NULL;
			virtual_uplink_ip = 0;
			master_uplinkip = 0;
			back_uplinkip = 0;
			
			uplink_ifname = vrrp->uplink_vif[i].ifname;
			virtual_uplink_ip = vrrp->uplink_vaddr[i].addr;
			
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_uplinkip = vrrp->uplink_vif[i].ipaddr;
					back_uplinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = vrrp->uplink_vif[i].ipaddr;
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = 0;
					break;
				}
			}

			vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
								uplink_ifname,
								master_uplinkip,
								back_uplinkip,
								virtual_uplink_ip);
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container(&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &master_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &back_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &virtual_uplink_ip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_STRING, &uplink_ifname);

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array);

	/* downlink */
	downlink_cnt = vrrp->downlink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &downlink_cnt);
	vrrp_syslog_event("downlink count %d\n",
						downlink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);

	if (VRRP_LINK_SETTED == vrrp->downlink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->downlink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			downlink_ifname = NULL;
			virtual_downlink_ip = 0;
			master_downlinkip = 0;
			back_downlinkip = 0;
		
			downlink_ifname = vrrp->downlink_vif[i].ifname;
			virtual_downlink_ip = vrrp->downlink_vaddr[i].addr;
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_downlinkip = vrrp->downlink_vif[i].ipaddr;
					back_downlinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = vrrp->downlink_vif[i].ipaddr; 
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = 0;
					break;
				}
			}
			vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
					downlink_ifname,
					master_downlinkip,
					back_downlinkip,
					virtual_downlink_ip);
			
			DBusMessageIter iter_struct1;
			dbus_message_iter_open_container(&iter_array1,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct1);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &master_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &back_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &virtual_downlink_ip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_STRING, &downlink_ifname);
			dbus_message_iter_close_container (&iter_array1, &iter_struct1);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array1);

	if(NULL != global_ht_ifname){
		heartbeatlink_if = global_ht_ifname;
	}
	else{
		if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
			{
				heartbeatlink_if = vrrp->uplink_vif[uplink_first_index].ifname;
			}
		}else {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
			{
				heartbeatlink_if = vrrp->downlink_vif[downlink_first_index].ifname;
			}
		}
	}

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &heartbeatlink_if);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_ip);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_opposite_ip);
	vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x\n",
						heartbeatlink_if, global_ht_ip, global_ht_opposite_ip);
						
	/* vgateway interface */
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &vrrp->vgateway_naddr);
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array_vgw);
	if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
	{
		vgateway_cnt = 0;
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED != vrrp->vgateway_vif[i].set_flg && 
					vgateway_cnt < vrrp->vgateway_naddr)
			{
				vgateway_if = vrrp->vgateway_vif[i].ifname;
				vgateway_ip = vrrp->vgateway_vaddr[i].addr; 					
				
				DBusMessageIter iter_struct_vgw;
				dbus_message_iter_open_container(&iter_array_vgw,
												DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct_vgw);
				dbus_message_iter_append_basic(&iter_struct_vgw,
							  DBUS_TYPE_STRING, &vgateway_if);
				dbus_message_iter_append_basic(&iter_struct_vgw,
							  DBUS_TYPE_UINT32, &vgateway_ip);
				dbus_message_iter_close_container (&iter_array_vgw, &iter_struct_vgw);
				vrrp_syslog_event("%-10s %d %s ip %#x\n", vgateway_cnt ? "":"vgateway",
									vgateway_cnt, vgateway_if, vgateway_ip);
				vgateway_cnt++;
				vgateway_if = NULL;
			}
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array_vgw);
	
	
	reply = dbus_connection_send_with_reply_and_block (vrrp_notify_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vrrp_syslog_dbg("had %d state %d notify hmd failed to get reply!\n", vrrp->vrid, state);
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else{
		if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&op_ret,
									DBUS_TYPE_INVALID))
		{
			vrrp_syslog_event("Value return from hmd %d!\n",op_ret);
		} 
		else {
			if (dbus_error_is_set(&err)) {
				vrrp_syslog_error("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
		}
		dbus_message_unref(reply);
	}

	
	/*trace log*/
	vrrp_state_trace(vrrp->profile,state,"NOTIFY HMD","notify over");
	global_notify_count[global_notifying_flag] = 0;
	global_notifying_flag = VRRP_NOTIFY_NONE;
	return op_ret;
}
/* book add end */

/* temp save for multi link interface by jinpc@autelan */
int had_notify_to_wid
(
	vrrp_rt *vrrp,
    int state
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1;
	
    int op_ret = 0;
	int master_uplinkip = 0;
	int master_downlinkip = 0;
	int back_uplinkip = 0;
	int back_downlinkip = 0;
	char *uplink_ifname = NULL;
	char *downlink_ifname = NULL;
	char *heartbeatlink_if = NULL;
    int virtual_uplink_ip = 0;
    int virtual_downlink_ip = 0;
	int uplink_cnt = 0;
	int downlink_cnt = 0;
	int i = 0;
	
	int ret_uplink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int ret_downlink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int uplink_first_index = -1;
	int downlink_first_index = -1;

	if (!vrrp) {
		vrrp_syslog_error("notify to wid null parameter error\n");
		return -1;
	} 

	/* bit[0] is wid */
	if (!(vrrp->notify_flg & VRRP_NOTIFY_BIT_WID)) {
		vrrp_syslog_dbg("notify flg %x, no notify to wid\n", vrrp->notify_flg);
		return -1;
	}
	global_notifying_flag = VRRP_NOTIFY_WID;
	global_notify_count[global_notifying_flag] = 0;

	/*trace log*/
	vrrp_state_trace(vrrp->profile,state,
					"NOTIFY WID", "start to notify");

/*
DBUS_TYPE_UINT32					// vrid
DBUS_TYPE_UINT32					// state
DBUS_TYPE_UINT32					// count of uplink interfaces
Array of uplink
	DBUS_TYPE_UINT32_AS_STRING		// master uplnik ip address
	DBUS_TYPE_UINT32_AS_STRING		// back uplink ip address
	DBUS_TYPE_UINT32_AS_STRING		// virtual uplink ip address
	DBUS_TYPE_STRING_AS_STRING		// uplink interface name
DBUS_TYPE_UINT32					// count of downlink interfaces
Array of uplink
	DBUS_TYPE_UINT32_AS_STRING		// master downlnik ip address
	DBUS_TYPE_UINT32_AS_STRING		// back downlink ip address
	DBUS_TYPE_UINT32_AS_STRING		// virtual downlink ip address
	DBUS_TYPE_STRING_AS_STRING		// downlink interface name
DBUS_TYPE_STRING					// heartbeat interface name
DBUS_TYPE_UINT32					// heartbeat ip address
DBUS_TYPE_UINT32					// opposite heartbeat ip address
*/
	query = dbus_message_new_method_call(
                         global_wid_dbusname,
                         global_wid_bak_objpath,
						 global_wid_bak_interfce,
						 WID_DBUS_WTP_METHOD_MASTER_BAK_SET);
	dbus_error_init(&err);
	dbus_message_iter_init_append(query, &iter);
/*
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(vrrp->vrid));
*/
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(vrrp->profile));
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32, &state);
	
	vrrp_syslog_event("notify to wid,profile %d,state %d\n",
					vrrp->profile, state);

	uplink_cnt = vrrp->uplink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &uplink_cnt);
	vrrp_syslog_event("uplink count %d\n",
						uplink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	/* get uplink first setted index */
	ret_uplink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_UPLINK, &uplink_first_index);
	/* get downlink first setted index */
	ret_downlink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_DOWNLINK, &downlink_first_index);

	/* uplink */
	if (VRRP_LINK_SETTED == vrrp->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->uplink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			uplink_ifname = NULL;
			virtual_uplink_ip = 0;
			master_uplinkip = 0;
			back_uplinkip = 0;
			
			uplink_ifname = vrrp->uplink_vif[i].ifname;
			virtual_uplink_ip = vrrp->uplink_vaddr[i].addr;
			
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_uplinkip = vrrp->uplink_vif[i].ipaddr;
					back_uplinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = vrrp->uplink_vif[i].ipaddr;
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = 0;
					break;
				}
			}

			vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
								uplink_ifname,
								master_uplinkip,
								back_uplinkip,
								virtual_uplink_ip);
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container(&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &master_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &back_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &virtual_uplink_ip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_STRING, &uplink_ifname);

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array);

	/* downlink */
	downlink_cnt = vrrp->downlink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &downlink_cnt);
	vrrp_syslog_event("downlink count %d\n",
						downlink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);

	if (VRRP_LINK_SETTED == vrrp->downlink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->downlink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			downlink_ifname = NULL;
			virtual_downlink_ip = 0;
			master_downlinkip = 0;
			back_downlinkip = 0;
		
			downlink_ifname = vrrp->downlink_vif[i].ifname;
			virtual_downlink_ip = vrrp->downlink_vaddr[i].addr;
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_downlinkip = vrrp->downlink_vif[i].ipaddr;
					back_downlinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = vrrp->downlink_vif[i].ipaddr; 
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = 0;
					break;
				}
			}
			vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
					downlink_ifname,
					master_downlinkip,
					back_downlinkip,
					virtual_downlink_ip);
			
			DBusMessageIter iter_struct1;
			dbus_message_iter_open_container(&iter_array1,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct1);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &master_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &back_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &virtual_downlink_ip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_STRING, &downlink_ifname);
			dbus_message_iter_close_container (&iter_array1, &iter_struct1);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array1);

	if(NULL != global_ht_ifname){
		heartbeatlink_if = global_ht_ifname;
	}
	else{
		if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
			{
				heartbeatlink_if = vrrp->uplink_vif[uplink_first_index].ifname;
			}
		}else {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
			{
				heartbeatlink_if = vrrp->downlink_vif[downlink_first_index].ifname;
			}
		}
	}

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &heartbeatlink_if);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_ip);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_opposite_ip);

	vrrp_syslog_event("heartbeat %s ip %x, opposite ip %x\n",
						heartbeatlink_if,
						global_ht_ip,
						global_ht_opposite_ip);

	reply = dbus_connection_send_with_reply_and_block (vrrp_notify_dbus_connection,query,60000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vrrp_syslog_dbg("had %d state %d notify wid failed to get reply!\n", vrrp->vrid, state);
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
				vrrp_syslog_event("Success to get return value %d from wid!\n",op_ret);
			} 
			else {
				if (dbus_error_is_set(&err)) {
					vrrp_syslog_error("%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
				}
			}
			dbus_message_unref(reply);
	}

	/*trace log*/
    vrrp_state_trace(vrrp->profile,state,"NOTIFY WID","notify over");
	global_notify_count[global_notifying_flag] = 0;	
	global_notifying_flag = VRRP_NOTIFY_NONE;

	return op_ret; 
}

void had_notify_to_dhcp
(
	vrrp_rt *vsrv,
	int addF
)
{
	char buf[128] = {0};
	vrrp_if *vgateway_vif = NULL;
	vrrp_if *downlink_vif = NULL;
	vrrp_if *uplink_vif = NULL;
	int i = 0;

	if (NULL == vsrv) {
		return;
	}
	
	global_notifying_flag = VRRP_NOTIFY_DHCP;
	global_notify_count[global_notifying_flag] = 0;
	if (VRRP_LINK_SETTED == vsrv->vgateway_flag) {
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
			if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg) {
				vgateway_vif = &(vsrv->vgateway_vif[i]);
			}
		}
		
	}

	if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag) {
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			downlink_vif = &(vsrv->downlink_vif[i]);
			if (VRRP_LINK_SETTED == downlink_vif->set_flg)
			{
				sprintf(buf, "%s %s %s","sudo",addF ? "add_no_inf.sh" : "del_no_inf.sh" ,
							vsrv->vgateway_flag ? vgateway_vif->ifname : downlink_vif->ifname);
				system(buf);
				vrrp_syslog_event("notify to dhcp,%s ifname %s\n",
								addF ? "add" : "delete",
								vsrv->vgateway_flag ? vgateway_vif->ifname : downlink_vif->ifname);
			}
		}
	}
	else if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			uplink_vif = &(vsrv->uplink_vif[i]);
			if (VRRP_LINK_SETTED == uplink_vif->set_flg)
			{
				sprintf(buf, "%s %s %s","sudo",addF ? "add_no_inf.sh" : "del_no_inf.sh" ,
								vsrv->vgateway_flag ? vgateway_vif->ifname : uplink_vif->ifname);
				system(buf);
				vrrp_syslog_event("notify to dhcp,%s ifname %s\n",
								addF ? "add" : "delete",
								vsrv->vgateway_flag ? vgateway_vif->ifname: uplink_vif->ifname);
			}
		}
	}
	global_notify_count[global_notifying_flag] = 0;
	global_notifying_flag = VRRP_NOTIFY_NONE;
	return;
}

int had_notify_to_dhcp_failover
(
	vrrp_rt *vrrp,
	int state
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1;
	
	int op_ret = 0;
	int master_uplinkip = 0, master_downlinkip = 0, back_uplinkip = 0, back_downlinkip = 0;
	char *uplink_ifname = NULL, *downlink_ifname = NULL, *vgateway_ifname = NULL, *heartbeatlink_if = NULL;
	int virtual_uplink_ip = 0, virtual_downlink_ip = 0;
	int uplink_cnt = 0, downlink_cnt = 0,vgateway_cnt = 0;
	int i = 0;
	unsigned int failover_flag = 0;

	int ret_uplink = VRRP_RETURN_CODE_IF_NOT_EXIST, ret_downlink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int uplink_first_index = -1, downlink_first_index = -1;

	if (!vrrp) {
		vrrp_syslog_error("notify to dhcp null parameter error\n");
		return -1;
	} 

	/* bit[2] is dhcp */
	if (!(vrrp->notify_flg & VRRP_NOTIFY_BIT_DHCP)) {
		vrrp_syslog_dbg("notify flg %x, no notify to dhcp\n", vrrp->notify_flg);
		return -1;
	}

	failover_flag = vrrp->failover.localip & vrrp->failover.peerip;
	vrrp_syslog_dbg("notify to dhcp %s config dhcp failover\n", failover_flag ? "" : "not");

	/*trace log*/
	vrrp_state_trace(vrrp->profile,state, "NOTIFY DHCP", "start to notify");

	query = dbus_message_new_method_call(
						 DHCP_DBUS_BUSNAME,
						 DHCP_DBUS_OBJPATH,
						 DHCP_DBUS_INTERFACE,
						 DHCP_DBUS_SET_HA_STATE);
	dbus_error_init(&err);
	dbus_message_iter_init_append(query, &iter);

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &failover_flag);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(vrrp->profile));    /* should be hansi id, not vrip. 2013-06-28 ZD.*/ 
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &state);
	vrrp_syslog_event("notify to dhcp, hansi %d, vrid %d,state %d\n",vrrp->profile, vrrp->vrid, state);

	uplink_cnt = vrrp->uplink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &uplink_cnt);
	vrrp_syslog_event("uplink count %d\n",
						uplink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	/* get uplink first setted index */
	ret_uplink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_UPLINK, &uplink_first_index);
	/* get downlink first setted index */
	ret_downlink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_DOWNLINK, &downlink_first_index);

	/* uplink */
	if (VRRP_LINK_SETTED == vrrp->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->uplink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			uplink_ifname = NULL;
			virtual_uplink_ip = 0;
			master_uplinkip = 0;
			back_uplinkip = 0;
			
			uplink_ifname = vrrp->uplink_vif[i].ifname;
			virtual_uplink_ip = vrrp->uplink_vaddr[i].addr;
			
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_uplinkip = vrrp->uplink_vif[i].ipaddr;
					back_uplinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = vrrp->uplink_vif[i].ipaddr;
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
					back_uplinkip = 0;
					break;
				}
			}

			vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
								uplink_ifname,
								master_uplinkip,
								back_uplinkip,
								virtual_uplink_ip);
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container(&iter_array,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &master_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &back_uplinkip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_UINT32, &virtual_uplink_ip);
			dbus_message_iter_append_basic(&iter_struct,
						  DBUS_TYPE_STRING, &uplink_ifname);

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array);

	/* downlink */
	downlink_cnt = vrrp->downlink_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &downlink_cnt);
	vrrp_syslog_event("downlink count %d\n", downlink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);

	if (VRRP_LINK_SETTED == vrrp->downlink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->downlink_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			downlink_ifname = NULL;
			virtual_downlink_ip = 0;
			master_downlinkip = 0;
			back_downlinkip = 0;
		
			downlink_ifname = vrrp->downlink_vif[i].ifname;
			virtual_downlink_ip = vrrp->downlink_vaddr[i].addr;
			switch (state)
			{
				case VRRP_STATE_MAST:
				{
					master_downlinkip = vrrp->downlink_vif[i].ipaddr;
					back_downlinkip = 0;
					break;
				}
				case VRRP_STATE_BACK :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = vrrp->downlink_vif[i].ipaddr; 
					break;
				}
				case VRRP_STATE_DISABLE :
				{
					master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
					back_downlinkip = 0;
					break;
				}
			}
			vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
					downlink_ifname,
					master_downlinkip,
					back_downlinkip,
					virtual_downlink_ip);
			
			DBusMessageIter iter_struct1;
			dbus_message_iter_open_container(&iter_array1,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct1);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &master_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &back_downlinkip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &virtual_downlink_ip);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_STRING, &downlink_ifname);
			dbus_message_iter_close_container (&iter_array1, &iter_struct1);
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array1);
    	/* vgateway */
	vgateway_cnt = vrrp->vgateway_naddr;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &vgateway_cnt);
	vrrp_syslog_event("vgateway count %d\n", vgateway_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);

	if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT,vgateway_cnt > 0; i++)
		{
			if (VRRP_LINK_NO_SETTED == vrrp->vgateway_vif[i].set_flg)
			{
				continue;
			}
			
			/* init */
			vgateway_ifname = NULL;		
			vgateway_ifname = vrrp->vgateway_vif[i].ifname;
            
			vrrp_syslog_event("vgateway %d %s \n",
                    i+1,
					vgateway_ifname);
			
			DBusMessageIter iter_struct1;
			dbus_message_iter_open_container(&iter_array1,
											DBUS_TYPE_STRUCT,
											NULL,
											&iter_struct1);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_STRING, &vgateway_ifname);
			dbus_message_iter_append_basic(&iter_struct1,
						  DBUS_TYPE_UINT32, &(vrrp->vgateway_vaddr[i].addr));
			dbus_message_iter_close_container (&iter_array1, &iter_struct1);
            vgateway_cnt--;
		}
	}
	dbus_message_iter_close_container(&iter, &iter_array1);

	if(NULL != global_ht_ifname){
		heartbeatlink_if = global_ht_ifname;
	}
	else{
		if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
			{
				heartbeatlink_if = vrrp->uplink_vif[uplink_first_index].ifname;
			}
		}else {
			if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
			{
				heartbeatlink_if = vrrp->downlink_vif[downlink_first_index].ifname;
			}
		}
	}

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &heartbeatlink_if);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &global_ht_ip);
	if(!global_ht_opposite_ip && (~0UI != vrrp->failover.peerip)) {
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &vrrp->failover.peerip);
		vrrp_syslog_event("heartbeat %s ip %x, opposite ip %x\n", \
							heartbeatlink_if, global_ht_ip, vrrp->failover.peerip);
	}
	else {
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &global_ht_opposite_ip);
		vrrp_syslog_event("heartbeat %s ip %x, opposite ip %x\n",  \
							heartbeatlink_if, global_ht_ip, global_ht_opposite_ip);
	}

	reply = dbus_connection_send_with_reply_and_block (vrrp_notify_dbus_connection,query,60000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vrrp_syslog_error("had %d state %d notify dhcp failed to get reply!\n", vrrp->vrid, state);
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
				vrrp_syslog_event("Success to get return value %d from wid!\n",op_ret);
			} 
			else {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
					vrrp_syslog_error("%s raised: %s",err.name,err.message);
				}
			}
			dbus_message_unref(reply);
	}
	

	/*trace log*/
	vrrp_state_trace(vrrp->profile,state,"NOTIFY DHCP","notify over");

	return op_ret; 
}


void had_notify_to_trap(vrrp_rt* vsrv)
{
#if 0
	  DBusMessage *query = NULL;
	  DBusError err;
	  char sysmac[6] = {0};

	if(NULL != vsrv) { 
		query = dbus_message_new_signal(NPD_DBUS_OBJPATH,\
							"aw.trap",NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP);
        vrrp_syslog_dbg("NOTIFY TO TRAP called!\n");
		dbus_error_init(&err);

        had_get_sys_mac(sysmac);
		
		dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&vsrv->state,
							DBUS_TYPE_BYTE,&sysmac[0],
							DBUS_TYPE_BYTE,&sysmac[1],
							DBUS_TYPE_BYTE,&sysmac[2],
							DBUS_TYPE_BYTE,&sysmac[3],
							DBUS_TYPE_BYTE,&sysmac[4],
							DBUS_TYPE_BYTE,&sysmac[5],
							DBUS_TYPE_INVALID);

		dbus_connection_send (vrrp_cli_dbus_connection,query,NULL);
		dbus_message_unref(query);
	}
	  return;
#endif
#if 1
	if(VRRP_OFF == vsrv->vrrp_trap_sw){/*0--disable,1--enable*/
		vrrp_syslog_event("%s,%d,Trap switch is disable.!\n",__func__,__LINE__);
		return;
	}else{

	}
	DBusMessage* msg = NULL;
	static DBusConnection* conn = NULL;
	DBusError err;
	int ret = 0;
	unsigned int serial = 0;
	char sysmac[6] = {0};
	global_notifying_flag = VRRP_NOTIFY_TRAP;
	global_notify_count[global_notifying_flag] = 0;

	if (NULL != vsrv) {
		dbus_error_init(&err);

		if (NULL == conn) {
			/* connect to the DBUS system bus, and check for errors */
			conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
			if (dbus_error_is_set(&err)) {
				vrrp_syslog_error("notify trap get dbus connection Error (%s)\n", err.message);
				dbus_error_free(&err);			
				global_notify_count[global_notifying_flag] = 0;
				global_notifying_flag = VRRP_NOTIFY_NONE;
				return;
			}
			if (NULL == conn) {
				vrrp_syslog_error("notify trap get dbus Connection null\n");			
				global_notify_count[global_notifying_flag] = 0;
				global_notifying_flag = VRRP_NOTIFY_NONE;
				return;
			}
		}
		/* register our name on the bus, and check for errors */
		ret = dbus_bus_request_name(conn,
									"aw.vrrp.signal",
									DBUS_NAME_FLAG_REPLACE_EXISTING,
									&err);
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("notify trap dbus request Name Error (%s)\n", err.message);
			dbus_error_free(&err);			
			global_notify_count[global_notifying_flag] = 0;
			global_notifying_flag = VRRP_NOTIFY_NONE;
			return;
		}
		
		/* create a signal & check for errors */
		msg = dbus_message_new_signal(NPD_DBUS_OBJPATH,				/* object name of the signal */
									  "aw.trap", 					/* interface name of the signal */
									  NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_BY_VRRP); /* name of the signal */
		if (NULL == msg) {
			vrrp_syslog_error("notify trap dbus new Message Null\n");			
			global_notify_count[global_notifying_flag] = 0;
			global_notifying_flag = VRRP_NOTIFY_NONE;
			return;
		}

		had_get_sys_mac(sysmac);
		vrrp_syslog_dbg("trap message:state %d,mac %02x:%02x:%02x:%02x:%02x:%02x\n",
						vsrv->state, sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);

		/* append arguments onto signal */
		dbus_message_append_args(msg,
								DBUS_TYPE_UINT32,&vsrv->profile,
								DBUS_TYPE_UINT32,&vsrv->state,
								DBUS_TYPE_BYTE,&sysmac[0],
								DBUS_TYPE_BYTE,&sysmac[1],
								DBUS_TYPE_BYTE,&sysmac[2],
								DBUS_TYPE_BYTE,&sysmac[3],
								DBUS_TYPE_BYTE,&sysmac[4],
								DBUS_TYPE_BYTE,&sysmac[5],
								DBUS_TYPE_INVALID);
		
		/* send the message and flush the connection */
		if (!dbus_connection_send(conn, msg, &serial)) {
			vrrp_syslog_error("notify trap Signal send error, Out Of Memory!\n"); 			
			global_notify_count[global_notifying_flag] = 0;
			global_notifying_flag = VRRP_NOTIFY_NONE;
			dbus_message_unref(msg);
			return;
		}
		vrrp_syslog_dbg("Trap signal sent!\n");
		dbus_connection_flush(conn);
		vrrp_syslog_event("sent message[state %s mac %02x-%02x-%02x-%02x-%02x-%02x] to trap.\n", \
						VRRP_STATE_DESCRIPTION(vsrv->state), sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);
		
		/* free the message */
		dbus_message_unref(msg);
	}			
	global_notify_count[global_notifying_flag] = 0;
	global_notifying_flag = VRRP_NOTIFY_NONE;
	return;
#endif
}

void had_notify_to_snmp_mib(vrrp_rt* vsrv)
{
	DBusMessage* msg = NULL;
	static DBusConnection* conn = NULL;
	DBusError err;
	int ret = 0;
	unsigned int serial = 0;

	if (NULL != vsrv) {
		dbus_error_init(&err);

		if (NULL == conn) {
			/* connect to the DBUS system bus, and check for errors */
			conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
			if (dbus_error_is_set(&err)) {
				vrrp_syslog_error("notify mibget dbus connection Error (%s)\n", err.message);
				dbus_error_free(&err);
				return;
			}
			if (NULL == conn) {
				vrrp_syslog_error("notify mib get dbus Connection null\n");
				return;
			}
		}
		/* register our name on the bus, and check for errors */
		ret = dbus_bus_request_name(conn,
									"aw.vrrp.signal",
									DBUS_NAME_FLAG_REPLACE_EXISTING,
									&err);
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("notify mib dbus request Name Error (%s)\n", err.message);
			dbus_error_free(&err);
			return;
		}
		
		/* create a signal & check for errors */
		msg = dbus_message_new_signal(NPD_DBUS_OBJPATH,				/* object name of the signal */
									  "aw.snmp.mib", 					/* interface name of the signal */
									  NPD_DBUS_ROUTE_METHOD_NOTIFY_SNMP_MIB_BY_VRRP); /* name of the signal */
		if (NULL == msg) {
			vrrp_syslog_error("notify mib dbus new Message Null\n");
			return;
		}

		/* append arguments onto signal */
		dbus_message_append_args(msg,
								DBUS_TYPE_UINT32, &vsrv->state,
								DBUS_TYPE_UINT32, &vsrv->profile,
								DBUS_TYPE_UINT32, &vsrv->vrid,
								DBUS_TYPE_INVALID);
		
		/* send the message and flush the connection */
		if (!dbus_connection_send(conn, msg, &serial)) {
			vrrp_syslog_error("notify snmp mib Signal send error, Out Of Memory!\n"); 
			return;
		}
		vrrp_syslog_dbg("Snmp mib signal sent! \n");
		dbus_connection_flush(conn);		
		vrrp_syslog_event("sent message[state %s profile %d vrid %d] to snmp mib.\n", \
						VRRP_STATE_DESCRIPTION(vsrv->state), vsrv->profile, vsrv->vrid);
		/* free the message */
		dbus_message_unref(msg);
	}
	return;
}

static void had_state_goto_transfer
(
	vrrp_rt* vsrv
)
{
	if (!vsrv) {
		vrrp_syslog_error("goto transfer null parameter error\n");
		return;
	}
	
	vsrv->state = VRRP_STATE_TRANSFER;
	hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
    /*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"TRANSFER SET","set transfer state");
    return;
}

void had_state_goto_master
(
	vrrp_rt *vsrv,
	int opposite_state
)
{
	int	i = 0;

	/*vrrp_if	*vif = &vsrv->vif;*/
	vrrp_if *vgateway_vif = NULL;
	char vrrp_hwaddr[6] = {0};

	if (!vsrv) {
		vrrp_syslog_error("goto master null parameter error\n");
		return;
	}

	if (VRRP_STATE_MAST == vsrv->state)
	{/*already is master,do nothing*/
		return;
	}

	if (opposite_state != VRRP_STATE_MAST)
	{
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO MASTER","set virtual ip");				
		had_ipaddr_ops( vsrv, 1 );

		vrrp_syslog_dbg("vrrp %d start to goto master...\n",vsrv->vrid);
		
		if (!vsrv->no_vmac)
		{	/* use virtual mac */
			vrrp_hwaddr[0] = 0x00;
			vrrp_hwaddr[1] = 0x00;
			vrrp_hwaddr[2] = 0x5E;
			vrrp_hwaddr[3] = 0x00;
			vrrp_hwaddr[4] = 0x01;
			vrrp_hwaddr[5] = vsrv->vrid;

			/* update mac address of uplink interfaces,
			 * and add multiaddress to receive table of interface
			 */
			if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
			{
				/*trace log*/
				vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO MASTER","change uplink sysmac to virtual mac");
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
					{
						had_hwaddr_set(vsrv->uplink_vif[i].ifname, vrrp_hwaddr, sizeof(vrrp_hwaddr) );
						had_rcvhwaddr_op(vsrv->uplink_vif[i].ifname,
									 vsrv->uplink_vif[i].hwaddr, sizeof(vsrv->uplink_vif[i].hwaddr), 1);
					}
				}
			}

			/* update mac address of downlink interfaces,
			 * and add multiaddress to receive table of interface
			 */
			if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
			{
				/*trace log*/
				vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO MASTER","change downlink sysmac to virtual mac");
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
					{
						had_hwaddr_set(vsrv->downlink_vif[i].ifname, vrrp_hwaddr, sizeof(vrrp_hwaddr) );
						had_rcvhwaddr_op(vsrv->downlink_vif[i].ifname,
									vsrv->downlink_vif[i].hwaddr, sizeof(vsrv->downlink_vif[i].hwaddr),	1);
					}
				}
			}
			
			if(VRRP_LINK_NO_SETTED != vsrv->vgateway_flag){
				/*trace log*/
				vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO MASTER","change vgateway sysmac to virtual mac");			
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
					if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg) {
						vgateway_vif = &(vsrv->vgateway_vif[i]);
						if (vsrv->smart_vmac_enable ){
							had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 0);
							had_hwaddr_set(vgateway_vif->ifname, vrrp_hwaddr, sizeof(vrrp_hwaddr) );
							had_rcvhwaddr_op(vgateway_vif->ifname, vrrp_hwaddr, sizeof(vrrp_hwaddr), 1);
							vrrp_syslog_dbg("vrrp %d set smart_vmac to master\n",vsrv->vrid);
                                    		}
						else{
							if(strncmp(vgateway_vif->ifname,"ebr",3)&& \
										strncmp(vgateway_vif->ifname,"wlan",4)){
								had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 0);
								had_hwaddr_set(vgateway_vif->ifname, vrrp_hwaddr, sizeof(vrrp_hwaddr) );
								had_rcvhwaddr_op(vgateway_vif->ifname, vrrp_hwaddr, sizeof(vrrp_hwaddr), 1);
							}
							else{
								char buf[128] = {0};
								sprintf(buf, "%s %s %02x:%02x:%02x:%02x:%02x:%02x %d.%d.%d.%d", \
											"setebrmac.sh",vgateway_vif->ifname,vrrp_hwaddr[0],vrrp_hwaddr[1],vrrp_hwaddr[2],vrrp_hwaddr[3],\
											vrrp_hwaddr[4],vrrp_hwaddr[5],(vgateway_vif->ipaddr & 0xff000000)>>24,(vgateway_vif->ipaddr & 0xff0000)>>16,\
											(vgateway_vif->ipaddr & 0xff00)>>8,vgateway_vif->ipaddr & 0xff);
								system(buf);
								vrrp_syslog_event("call setebrmac.sh,ifname %s,mac %02x:%02x:%02x:%02x:%02x:%02x,ip %d.%d.%d.%d\n",\
											vgateway_vif->ifname,vrrp_hwaddr[0],vrrp_hwaddr[1],vrrp_hwaddr[2],vrrp_hwaddr[3],\
											vrrp_hwaddr[4],vrrp_hwaddr[5],(vgateway_vif->ipaddr & 0xff000000)>>24,(vgateway_vif->ipaddr & 0xff0000)>>16,\
											(vgateway_vif->ipaddr & 0xff00)>>8,vgateway_vif->ipaddr & 0xff);
							}
						}
					}
				}
			}		
		}		
	}
	
	if (opposite_state == VRRP_STATE_MAST)
	{
		/*set bath syn state*/
		vrrp_set_wid_bathsyn_start(vsrv->profile);
		vrrp_set_portal_bathsyn_start(vsrv->profile);
	}

	/*send msg to wtp*/
	#if 0
	if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
		(VRRP_LINK_SETTED == vsrv->downlink_flag)) {
		for (i = 0; i < vsrv->downlink_naddr; i++)
		{
			had_notify_to_wid(vsrv->vrid,VRRP_STATE_MAST,0,vsrv->downlink_vif[i].ipaddr,0,vsrv->downlink_vaddr[i].addr);
	        if (global_protal) {
			    had_notify_to_protal(vsrv->vrid,VRRP_STATE_MAST,0,vsrv->downlink_vif[i].ipaddr,0,vsrv->downlink_vaddr[i].addr);
	        }
		}
	}else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
			  (VRRP_LINK_NO_SETTED == vsrv->downlink_flag)) {
		for (i = 0; i < vsrv->uplink_naddr; i++)
		{
			had_notify_to_wid(vsrv->vrid, VRRP_STATE_MAST, vsrv->uplink_vif[i].ipaddr, 0, vsrv->uplink_vaddr[i].addr, 0);
	        if ( global_protal) {
			    had_notify_to_protal(vsrv->vrid, VRRP_STATE_MAST, vsrv->uplink_vif[i].ipaddr, 0, vsrv->uplink_vaddr[i].addr, 0);
	        }
		}
	}else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
			  (VRRP_LINK_SETTED == vsrv->downlink_flag)){
        had_notify_to_wid(vsrv->vrid,VRRP_STATE_MAST,vsrv->uplink_vif[i].ipaddr,vsrv->downlink_vif[i].ipaddr,vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);
		if(global_protal)
		   had_notify_to_protal(vsrv->vrid,VRRP_STATE_MAST,vsrv->uplink_vif[i].ipaddr,vsrv->downlink_vif[i].ipaddr,vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);
	}
	#else 
	if (opposite_state == VRRP_STATE_MAST) { 	
		had_notify_to_wid(vsrv, VRRP_STATE_BACK);
		if (global_protal) {
		   had_notify_to_protal(vsrv,VRRP_STATE_BACK);
		}
	#ifndef _VERSION_18SP7_	
		if(global_pppoe){
			had_notify_to_pppoe(vsrv,VRRP_STATE_BACK);
		}
	#endif	
had_notify_to_hmd(vsrv,VRRP_STATE_BACK);

		had_notify_to_dhcp_failover(vsrv,VRRP_STATE_BACK);
		vrrp_syslog_dbg("state goto transfer...\n");		
		had_state_goto_transfer(vsrv);
		return ;
	}else{
		had_notify_to_wid(vsrv, VRRP_STATE_MAST);
		if (global_protal) {
		   had_notify_to_protal(vsrv,VRRP_STATE_MAST);
		}
	#ifndef _VERSION_18SP7_		
		if (global_pppoe) {
		   had_notify_to_pppoe(vsrv,VRRP_STATE_MAST);
		}
	#endif		
	had_notify_to_hmd(vsrv,VRRP_STATE_MAST);
		had_notify_to_dhcp_failover(vsrv,VRRP_STATE_MAST);
	}
	#endif
	
	global_state_change_bit = 1;	
	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO MASTER","set advertisements");				
	vrrp_send_adv( vsrv, vsrv->priority );	
	vrrp_syslog_dbg("start to send gratuitous arp!\n");

	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO MASTER","set gratuitous arp"); 			
	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag){		
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) {
                had_send_uplink_gratuitous_arp( vsrv, vsrv->uplink_vif[i].ipaddr, i, 1 );
				had_send_uplink_gratuitous_arp( vsrv, vsrv->uplink_vaddr[i].addr, i, 1 );
			}
		}
	}
	if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag){
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) {
                had_send_downlink_gratuitous_arp( vsrv, vsrv->downlink_vif[i].ipaddr, i, 1 );
				had_send_downlink_gratuitous_arp( vsrv, vsrv->downlink_vaddr[i].addr, i, 1 );
			}
		}
	}	
	if(0 != vsrv->vgateway_flag){		
		/*hardware get*/
//		had_hwaddr_get(vsrv->vgateway_vif.ifname,vsrv->vgateway_vif.hwaddr,6);
//		had_send_vgateway_gratuitous_arp(vsrv,vsrv->vgateway_vif.ipaddr,1);

        for(i = 0; i < VRRP_LINK_MAX_CNT; i++)
        {
            if(VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg){
                had_send_vgateway_gratuitous_arp(vsrv,vsrv->vgateway_vif[i].ipaddr,i,1);
                had_send_vgateway_gratuitous_arp(vsrv,vsrv->vgateway_vaddr[i].addr,i,1);
            }
        }
	}
	had_vrrp_vif_rcv_pck_ON(vsrv);
	/* goto master state ,inc switch times */
    vsrv->backup_switch_times++;
	vsrv->state = VRRP_STATE_MAST;	
	hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
	/*trace log*/
	
	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag){
		VRRP_TIMER_SET( vsrv->uplink_adver_timer, vsrv->adver_int );
		master_ipaddr_uplink[vsrv->vrid] = 0;
	}
	if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag){
		VRRP_TIMER_SET( vsrv->downlink_adver_timer, vsrv->adver_int );
		master_ipaddr_downlink[vsrv->vrid] = 0;
	}
    if((NULL != global_ht_ifname)&&(0 != global_ht_ip)){
        global_ht_opposite_ip = 0;
	}	
	#if 0
	/*notify to dhcp*/
	had_notify_to_dhcp(vsrv,1);
	#endif
	/*send msg to trap*/
	had_notify_to_trap(vsrv);
	
	/*send msg to snmp mib */
	had_notify_to_snmp_mib(vsrv);
	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"MASTER LOOP","start master loop"); 			
	return;
}   

/****************************************************************
 NAME	: had_state_leave_master			00/02/07 00:15:26
 AIM	:
 REMARK	: called when the state is no more MASTER
****************************************************************/
void had_state_leave_master
(
	vrrp_rt *vsrv,
	int advF,
	struct iphdr* uplink_iph,
	struct iphdr* downlink_iph
)
{
	uint32_t addr[1024] = {0};
	int delay = 0;
	vrrp_if *uplink_vif = NULL;
	vrrp_if *downlink_vif = NULL;
	vrrp_if *vgateway_vif = NULL;
	vrrp_pkt *uplink_hd = NULL;
	vrrp_pkt *downlink_hd = NULL;
	int i = 0;

	if (!vsrv) {
		vrrp_syslog_error("leave master null parameter error\n");
		return;
	}
	
	if(uplink_iph){
	    uplink_hd = (vrrp_pkt *)((char *)uplink_iph + (uplink_iph->ihl<<2));		
	}
	if(downlink_iph){
	    downlink_hd = (vrrp_pkt *)((char *)downlink_iph + (downlink_iph->ihl<<2));
	}
	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"LEAVE MASTER","start to leave master");

	/* restore the original MAC addresses */
	#if 1
	vrrp_syslog_dbg("vrrp %d start to leave master!\n",vsrv->vrid);
	if (!vsrv->no_vmac)
	{
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"LEAVE MASTER","chang virtual mac to sysmac");
		
		if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
		{
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
				{
					had_hwaddr_set(vsrv->uplink_vif[i].ifname,
								vsrv->uplink_vif[i].hwaddr, sizeof(vsrv->uplink_vif[i].hwaddr) );
					had_rcvhwaddr_op(vsrv->uplink_vif[i].ifname,
								vsrv->uplink_vif[i].hwaddr, sizeof(vsrv->uplink_vif[i].hwaddr), 0);
				}
			}
		}
        if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
		{
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
				{
					had_hwaddr_set(vsrv->downlink_vif[i].ifname,
								vsrv->downlink_vif[i].hwaddr, sizeof(vsrv->downlink_vif[i].hwaddr) );
					had_rcvhwaddr_op(vsrv->downlink_vif[i].ifname,
								vsrv->downlink_vif[i].hwaddr, sizeof(vsrv->downlink_vif[i].hwaddr), 0);
				}
			}
		}
		if (VRRP_LINK_NO_SETTED != vsrv->vgateway_flag)
		{
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
				if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg) {
					vgateway_vif = &(vsrv->vgateway_vif[i]);
					
				if (vsrv->smart_vmac_enable ){
					had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 0);
					had_hwaddr_set(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr) );
					had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 1);
					vrrp_syslog_dbg("vrrp %d set smart_vmac to leaving master\n",vsrv->vrid);
					}
				else if(strncmp(vgateway_vif->ifname,"ebr",3)&&\
				strncmp(vgateway_vif->ifname,"wlan",4)){
				had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 0);
				had_hwaddr_set(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr) );
				had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 1);
			}
			else{
				char buf[128] = {0};
				sprintf(buf, "%s %s %02x:%02x:%02x:%02x:%02x:%02x %d.%d.%d.%d", \
					"setebrmac.sh",vgateway_vif->ifname,vgateway_vif->hwaddr[0],vgateway_vif->hwaddr[1],vgateway_vif->hwaddr[2],vgateway_vif->hwaddr[3],\
					vgateway_vif->hwaddr[4],vgateway_vif->hwaddr[5],(vgateway_vif->ipaddr & 0xff000000)>>24,(vgateway_vif->ipaddr & 0xff0000)>>16,\
					(vgateway_vif->ipaddr & 0xff00)>>8,vgateway_vif->ipaddr & 0xff);
				system(buf);
				vrrp_syslog_event("call setebrmac.sh,ifname %s,mac %02x:%02x:%02x:%02x:%02x:%02x,ip %d.%d.%d.%d\n",\
					vgateway_vif->ifname,vgateway_vif->hwaddr[0],vgateway_vif->hwaddr[1],vgateway_vif->hwaddr[2],vgateway_vif->hwaddr[3],\
					vgateway_vif->hwaddr[4],vgateway_vif->hwaddr[5],(vgateway_vif->ipaddr & 0xff000000)>>24,(vgateway_vif->ipaddr & 0xff0000)>>16,\
					(vgateway_vif->ipaddr & 0xff00)>>8,vgateway_vif->ipaddr & 0xff);
					}
				}
			}
		}
	}
	#endif
	/* remove the ip addresses */
	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"LEAVE MASTER","cancel virtual ip");
	
	had_ipaddr_ops( vsrv, 0 );

	/* if we stop vrrpd, warn the other routers to speed up the recovery */
	if( advF ){	
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"LEAVE MASTER","send stop advertisement packet");		
		vrrp_send_adv( vsrv, VRRP_PRIO_STOP );
		
	}
	
	/* send gratuitous ARP for all the non-vrrp ip addresses to update
	** the cache of remote hosts using these addresses */
	#if 1
	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"LEAVE MASTER","send interface real ip gratuitous arp");
	
	if( !vsrv->no_vmac ){
		int i = 0, naddr = 0;
		int j = 0;
		/*
		naddr = had_ipaddr_list( had_ifname_to_idx(vif->ifname), addr
				, sizeof(addr)/sizeof(addr[0]) );
		for( i = 0; i < naddr; i++ )
			had_send_gratuitous_arp( vsrv, addr[i], 0 );
		*/
		/*uplink */
		if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag){
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
				uplink_vif = NULL;
				if (VRRP_LINK_SETTED == vsrv->uplink_vif[j].set_flg) {
					memset(addr, 0, sizeof(addr));
					uplink_vif = &(vsrv->uplink_vif[j]);
					naddr = had_ipaddr_list( had_ifname_to_idx(uplink_vif->ifname), addr
							, sizeof(addr)/sizeof(addr[0]) );
					had_send_uplink_gratuitous_arp( vsrv, vsrv->uplink_vif[j].ipaddr, j, 0 );
					for( i = 0; i < naddr; i++ ) {
						had_send_uplink_gratuitous_arp( vsrv, addr[i], j, 0 );
					}
				}
			}

		}
		/*downlink */
		if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag){
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
				downlink_vif = NULL;
				if (VRRP_LINK_SETTED == vsrv->downlink_vif[j].set_flg) {
					memset(addr, 0, sizeof(addr));
					downlink_vif = &(vsrv->downlink_vif[j]);
					naddr = had_ipaddr_list( had_ifname_to_idx(downlink_vif->ifname), addr
							, sizeof(addr)/sizeof(addr[0]) );
                    
					had_send_downlink_gratuitous_arp( vsrv, vsrv->downlink_vif[j].ipaddr, j, 0 );
					for( i = 0; i < naddr; i++ ) {
						had_send_downlink_gratuitous_arp( vsrv, addr[i], j, 0 );
					}
				}
			}

		}	
		/*vgateway*/
		if ((VRRP_LINK_NO_SETTED != vsrv->vgateway_flag) &&
			(! vsrv->no_vmac ))
		{
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
			
				if (VRRP_LINK_NO_SETTED == vsrv->vgateway_vif[j].set_flg) {
					continue;
				}
				memset(addr,0,sizeof(addr));
				naddr = had_ipaddr_list( had_ifname_to_idx(vsrv->vgateway_vif[j].ifname), addr
						, sizeof(addr)/sizeof(addr[0]) );			
				/*hardware get*/
				/*had_hwaddr_get(vsrv->vgateway_vif[j].ifname,vsrv->vgateway_vif[j].hwaddr,6);*/
				for( i = 0; i < naddr; i++ ){
					had_send_vgateway_gratuitous_arp( vsrv, addr[i], j, 0 );
				}
			}
		}		
	}
	#endif
	if(!uplink_iph && !downlink_iph){
		if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
            master_ipaddr_uplink[vsrv->vrid] = 0;
		if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
		    master_ipaddr_downlink[vsrv->vrid] = 0;
		if((NULL != global_ht_ifname)&&(0 != global_ht_ip)){
            global_ht_opposite_ip = 0;
		}
	}
	
	/*leave vrrp*/
	if((uplink_iph && (VRRP_AUTH_FAIL == uplink_hd->auth_type)) || \
		(downlink_iph && (VRRP_AUTH_FAIL == downlink_hd->auth_type))){
        if(uplink_iph){
			#if 0
			if(NULL != global_ht_ifname && 0 != global_ht_ip){
			   if(global_ht_ip != uplink_hd->old_master_ip){
				   vrrp_syslog_dbg("receive packets old master ip %d,not equal to global ht ip %d,goto back!\n",uplink_hd->old_master_ip,global_ht_ip);
				   goto back;
			   }
			
			}
			else if(vsrv->uplink_vif.ipaddr != uplink_hd->old_master_ip){			
				vrrp_syslog_dbg("receive packets old master ip %d,not equal to uplink ip %d,goto back!\n",uplink_hd->old_master_ip,vsrv->uplink_vif.ipaddr);
				goto back;
			}
			#endif
            if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
				(vrrp_is_match_ipaddr(vsrv, VRRP_LINK_TYPE_UPLINK, uplink_hd->old_master_ip)))
			{
				vrrp_syslog_dbg("receive packets old master ip %d,not equal to uplink ip,goto back!\n",
								uplink_hd->old_master_ip);
				goto back;
			}			
		}
		if(downlink_iph){
			#if 0
			if(NULL != global_ht_ifname && 0 != global_ht_ip){
			   if(global_ht_ip != downlink_hd->old_master_ip){
				   vrrp_syslog_dbg("receive packets old master ip %d,not equal to global ht ip %d,goto back!\n",downlink_hd->old_master_ip,global_ht_ip);
				   goto back;
			   }
			
			}
			else if(vsrv->downlink_vif.ipaddr != downlink_hd->old_master_ip){			
				vrrp_syslog_dbg("receive packets old master ip %d,not equal to downlink ip %d,goto back!\n",downlink_hd->old_master_ip,vsrv->downlink_vif.ipaddr);
				goto back;
			}
			#endif
            if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
				(vrrp_is_match_ipaddr(vsrv, VRRP_LINK_TYPE_DOWNLINK, downlink_hd->old_master_ip)))
			{			
				vrrp_syslog_dbg("receive packets old master ip %d,not equal to downlink ip,goto back!\n",
								downlink_hd->old_master_ip);
				goto back;
			}			
		}

        vrrp_syslog_dbg("vrrp %d goto disable state!\n",vsrv->vrid);
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","receive packets is force-to-fail,goto disable");
   		vrrp_syslog_event("%s,%d,hansi[%d] receive packets is force-to-fail,goto disable.\n",__func__,__LINE__,vsrv->profile);
		/* from master to disable ,inc switch times */
		vsrv->backup_switch_times++;
		vsrv->state = VRRP_STATE_DISABLE; 
		hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"DISABLE SET","set disable state");	
		
		/*nofity to wtp*/
		if(uplink_iph){
			master_ipaddr_uplink[vsrv->vrid] = uplink_hd->uplink_ip;
			master_ipaddr_downlink[vsrv->vrid] = uplink_hd->downlink_ip;			
		}
		if(downlink_iph){
			master_ipaddr_uplink[vsrv->vrid] = downlink_hd->uplink_ip;
			master_ipaddr_downlink[vsrv->vrid] = downlink_hd->downlink_ip;
		}	
		if ((NULL != global_ht_ifname) &&
			(0 != global_ht_ip))
		{
			if(uplink_iph){
				global_ht_opposite_ip = uplink_hd->heartbeatlinkip;
			}
			if(downlink_iph){
				global_ht_opposite_ip = downlink_hd->heartbeatlinkip;
			}            
		}

		#if 0
		if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
			(VRRP_LINK_SETTED == vsrv->downlink_flag)) {
			int i = 0;
			for (i = 0; i < vsrv->downlink_naddr; i++)
			{
				had_notify_to_wid(vsrv->vrid,VRRP_STATE_DISABLE,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);				
				if (global_protal) {
					had_notify_to_protal(vsrv->vrid,VRRP_STATE_DISABLE,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);
				}
			}
		}
		else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
				 (VRRP_LINK_NO_SETTED == vsrv->downlink_flag)) {
			int i = 0;
			for (i = 0; i < vsrv->uplink_naddr; i++)
			{
				had_notify_to_wid(vsrv->vrid, VRRP_STATE_DISABLE,
									master_ipaddr_uplink[vsrv->vrid -1],
									0,
									vsrv->uplink_vaddr[i].addr,
									0);
				if (global_protal) {
					had_notify_to_protal(vsrv->vrid, VRRP_STATE_DISABLE,
											master_ipaddr_uplink[vsrv->vrid -1],
											0,
											vsrv->uplink_vaddr[i].addr,
											0);
				}
			}
		}
		else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
				 (VRRP_LINK_SETTED == vsrv->downlink_flag)) {
		    had_notify_to_wid(vsrv->vrid,VRRP_STATE_DISABLE,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);				
			if(global_protal)
		    	had_notify_to_protal(vsrv->vrid,VRRP_STATE_DISABLE,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);				
		}
		#else 
		had_notify_to_wid(vsrv, VRRP_STATE_DISABLE);
		if (global_protal) {
		   had_notify_to_protal(vsrv,VRRP_STATE_DISABLE);
		}
	#ifndef _VERSION_18SP7_
		if(global_pppoe) {
		   had_notify_to_pppoe(vsrv,VRRP_STATE_DISABLE);
		}
	#endif	
		/* book add, 2011-5-11 */
		had_notify_to_hmd(vsrv,VRRP_STATE_DISABLE);
		
		had_notify_to_dhcp_failover(vsrv,VRRP_STATE_DISABLE);
		#endif
	} else if (uplink_iph == NULL && downlink_iph == NULL) {

		vsrv->backup_switch_times++;
	    vsrv->state = VRRP_STATE_DISABLE;
		hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));	
		had_vrrp_vif_drop_pck(vsrv);
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"DISABLE SET","set disable state");
	
	    vrrp_syslog_dbg("vrrp %d goto disable state!\n",vsrv->vrid);	
		delay	= global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
		/*reset both uplink and downlink master down timer*/
		VRRP_TIMER_SET( vsrv->uplink_ms_down_timer, delay );			
		VRRP_TIMER_SET( vsrv->downlink_ms_down_timer, delay );
		VRRP_TIMER_CLR( vsrv->uplink_adver_timer );
		VRRP_TIMER_CLR( vsrv->downlink_adver_timer); 		

		/*nofity to wtp*/
		had_notify_to_wid(vsrv, VRRP_STATE_DISABLE);
		if (global_protal) {
		   had_notify_to_protal(vsrv,VRRP_STATE_DISABLE);
		}
	#ifndef _VERSION_18SP7_	
		if (global_pppoe) {
		   had_notify_to_pppoe(vsrv,VRRP_STATE_DISABLE);
		}
	#endif	
		had_notify_to_hmd(vsrv,VRRP_STATE_DISABLE);
		had_notify_to_dhcp_failover(vsrv,VRRP_STATE_DISABLE);
	}
	else{/*set to back state*/
back:
		/* from master to back ,inc switch times */
		vsrv->backup_switch_times++;
	    vsrv->state = VRRP_STATE_BACK;
		hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));	
		had_vrrp_vif_drop_pck(vsrv);
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"BACK SET","set back state");
	
	        vrrp_syslog_dbg("vrrp %d goto back state!\n",vsrv->vrid);	
		delay	= global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
		/*reset both uplink and downlink master down timer*/
		VRRP_TIMER_SET( vsrv->uplink_ms_down_timer, delay );			
		VRRP_TIMER_SET( vsrv->downlink_ms_down_timer, delay );
		VRRP_TIMER_CLR( vsrv->uplink_adver_timer );
		VRRP_TIMER_CLR( vsrv->downlink_adver_timer); 		

		/*nofity to wtp*/
		#if 0
		if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
			(VRRP_LINK_SETTED == vsrv->downlink_flag)) {
			int i = 0;
			for (i = 0; i < vsrv->downlink_naddr; i++)
			{
				had_notify_to_wid(vsrv->vrid,VRRP_STATE_BACK,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);		 
				if (global_protal) {
					had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);		 
				}
			}
		}
		else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
				 (VRRP_LINK_NO_SETTED == vsrv->downlink_flag)) {
			int i = 0;
			for (i = 0; i < vsrv->uplink_naddr; i++)
			{
				had_notify_to_wid(vsrv->vrid, VRRP_STATE_BACK,
									master_ipaddr_uplink[vsrv->vrid -1],
									0,
									vsrv->uplink_vaddr[i].addr,
									0);
				if (global_protal) {
					had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,
											master_ipaddr_uplink[vsrv->vrid -1],
											0,
											vsrv->uplink_vaddr[i].addr,
											0);
				}
			}
		}
		else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
				 (VRRP_LINK_SETTED == vsrv->downlink_flag)) {
		   had_notify_to_wid(vsrv->vrid,VRRP_STATE_BACK,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);		
		   if(global_protal)
		      had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);		
		}
		#else 
		had_notify_to_wid(vsrv, VRRP_STATE_BACK);
		if (global_protal) {
		   had_notify_to_protal(vsrv,VRRP_STATE_BACK);
		}
	#ifndef _VERSION_18SP7_	
		if (global_pppoe) {
		   had_notify_to_pppoe(vsrv,VRRP_STATE_BACK);
		}
	#endif	
		 had_notify_to_hmd(vsrv,VRRP_STATE_BACK);
		had_notify_to_dhcp_failover(vsrv,VRRP_STATE_BACK);
		#endif
	}
	#if 0
	/*notify dhcp*/
	had_notify_to_dhcp(vsrv,0);	
	#endif
	/*send msg to trap*/
	had_notify_to_trap(vsrv);	
	/*send msg to snmp mib */
	had_notify_to_snmp_mib(vsrv);
}

/**************************************************************
 * had_br_uplink_oldstate_set
 *         set the br uplink interfaces back to the state we got before shutdown them
 *  INPUT:
 *		vsrv  : vrrp_rt * -- set for this vrrp
 * OUTPUT:
 *		NONE
 * RETURN:
 *		VRRP_RETURN_CODE_SUCCESS              --            the only return value
 *
 **************************************************************/
unsigned int had_br_uplink_rcv_pck
(
	vrrp_rt* vsrv
)
{	
	int i = 0;
	int ret = 0;
	int count = 0;
	unsigned int rxState = TRUE;
	if(!vsrv){
		return VRRP_RETURN_CODE_ERR;
		}
	for(i=0;i<VRRP_LINK_MAX_CNT;i++)
		{
			if(!strncmp(vsrv->l2_uplink_vif[i].ifname, "ebr", strlen("ebr"))){
				if(vsrv->l2_uplink_vif[i].set_flg != VRRP_LINK_SETTED){
					continue;
					}
				while(count < 4)
					{	if(vsrv->l2_uplink_vif[i].l2uplinkinfo[count].setflag == VRRP_LINK_NO_SETTED)
						{	
							count ++;
							continue;
						}
						
						rxState = TRUE;
						ret = had_set_intf_state_byIoctl(vsrv->l2_uplink_vif[i].l2uplinkinfo[count].ifname, rxState);
						vrrp_syslog_event("set br uplink interface rcv packets %s, ret %#x\n", 
								vsrv->l2_uplink_vif[i].l2uplinkinfo[count].ifname, ret);					
						count ++;
					}
			}else{
					if(vsrv->l2_uplink_vif[i].set_flg != VRRP_LINK_NO_SETTED && strcmp(vsrv->l2_uplink_vif[i].ifname, "")){
						rxState = TRUE;
						ret = had_set_intf_state_byIoctl(vsrv->l2_uplink_vif[i].ifname, rxState);
						vrrp_syslog_event("set br uplink interface rcv packets %s, ret %#x\n", 
							vsrv->l2_uplink_vif[i].ifname, ret);
					}
			}	
		}
	had_br_uplink_info_clear(vsrv); /* leave this or delete it ? both ok */
	return VRRP_RETURN_CODE_SUCCESS;
}

/**************************************************************
 * had_shutdown_br_uplink
 *         shutdown all the br uplink interfaces of l2-uplink
 *  INPUT:
 *		vsrv  : vrrp_rt * -- shut br uplink interfaces for this vrrp
 * OUTPUT:
 *		NONE
 * RETURN:
 *		VRRP_RETURN_CODE_SUCCESS              --            the only return value
 *
 **************************************************************/
unsigned int had_br_uplink_drop_pck
(
	vrrp_rt* vsrv
)
{	
	int i = 0;
	int ret = 0;
	int count = 0;	
	unsigned int rxState = FALSE;
	
	if(!vsrv){
		return VRRP_RETURN_CODE_ERR;
		}
	had_br_uplink_info_clear(vsrv);
	had_br_uplink_info_get(vsrv);
	for(i = 0;i < VRRP_LINK_MAX_CNT;i++){	
		if(vsrv->l2_uplink_vif[i].set_flg != VRRP_LINK_SETTED){
			continue;
			}
		if(!strncmp(vsrv->l2_uplink_vif[i].ifname, "ebr", strlen("ebr"))){
			while(count < 4){
				if(vsrv->l2_uplink_vif[i].l2uplinkinfo[count].setflag != VRRP_LINK_SETTED)
					{
						count++;
						continue;
					}											
					rxState = FALSE;
					ret = had_set_intf_state_byIoctl(vsrv->l2_uplink_vif[i].l2uplinkinfo[count].ifname, rxState);
					vrrp_syslog_event("br uplink interface drop packets%s, ret %#x\n", 
						vsrv->l2_uplink_vif[i].l2uplinkinfo[count].ifname, ret);
					count ++;
			}
		}
		else{
			if(vsrv->l2_uplink_vif[i].set_flg != VRRP_LINK_NO_SETTED && strcmp(vsrv->l2_uplink_vif[i].ifname, "")){
					rxState = FALSE;
					ret = had_set_intf_state_byIoctl(vsrv->l2_uplink_vif[i].ifname, rxState);
					vrrp_syslog_event("br uplink interface drop packets%s, ret %#x\n", 
						vsrv->l2_uplink_vif[i].ifname, ret);
			}
		}
	}
	return VRRP_RETURN_CODE_SUCCESS;
}

unsigned int had_vrrp_vif_drop_pck
(
	vrrp_rt* vsrv
)
{	
	int i = 0;
	int ret = 0;
	
	if(!vsrv){
		return VRRP_RETURN_CODE_ERR;
		}
	vrrp_syslog_dbg("vrrp vif interfaces drop packets\n");
	if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag)&&(VRRP_ON == vsrv->uplink_back_down_flag))
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
			{
				ret = had_set_intf_state_byIoctl(vsrv->uplink_vif[i].ifname,FALSE);
				vrrp_syslog_dbg("uplink vif drop packets %s %s\n",vsrv->uplink_vif[i].ifname, ret ? "failed" : "ok");
			}
		}
	}
	if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag)&&(VRRP_ON == vsrv->downlink_back_down_flag))
		{
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
				{
					ret = had_set_intf_state_byIoctl(vsrv->downlink_vif[i].ifname,FALSE);
					vrrp_syslog_dbg("downlink vif drop packets %s %s\n",vsrv->downlink_vif[i].ifname, ret ? "failed" : "ok");
				}
			}
		}
	if ((VRRP_LINK_NO_SETTED != vsrv->vgateway_flag)&&(VRRP_ON == vsrv->vgateway_back_down_flag))
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg)
			{
				ret = had_set_intf_state_byIoctl(vsrv->vgateway_vif[i].ifname,FALSE);
				vrrp_syslog_dbg("vgateway vif drop packets %s %s\n",vsrv->vgateway_vif[i].ifname, ret ? "failed" : "ok");
			}
		}
	}
	if((VRRP_LINK_NO_SETTED != vsrv->l2_uplink_flag)&&(VRRP_ON == vsrv->l2_uplink_back_down_flag)){
		had_br_uplink_drop_pck(vsrv);/* shutdown l2-uplink sensitive interfaces */
	}
	
	return VRRP_RETURN_CODE_SUCCESS;
}
unsigned int had_vrrp_vif_rcv_pck_ON
(
	vrrp_rt* vsrv
)
{	
	int i = 0;
	int ret = 0;
	if(!vsrv){
		return VRRP_RETURN_CODE_ERR;
		}
	vrrp_syslog_dbg("set vrrp vif interfaces rcv packets\n");
	if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag)&&(VRRP_ON == vsrv->uplink_back_down_flag))
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
			{
				ret = had_set_intf_state_byIoctl(vsrv->uplink_vif[i].ifname,TRUE);
				vrrp_syslog_dbg("set uplink vrrp vif %s rx OK %s\n",vsrv->uplink_vif[i].ifname, ret ? "failed" : "ok");
			
			}
		}
	}
	if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag)&&(VRRP_ON == vsrv->downlink_back_down_flag))
		{
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
				{
					ret = had_set_intf_state_byIoctl(vsrv->downlink_vif[i].ifname,TRUE);
					vrrp_syslog_dbg("set downlink vrrp vif %s rx OK %s\n",vsrv->downlink_vif[i].ifname, ret ? "failed" : "ok");
				
				}
			}
		}
	if ((VRRP_LINK_NO_SETTED != vsrv->vgateway_flag)&&(VRRP_ON == vsrv->vgateway_back_down_flag))
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg)
			{
				ret = had_set_intf_state_byIoctl(vsrv->vgateway_vif[i].ifname,TRUE);
				vrrp_syslog_dbg("set vgateway vrrp vif %s rx OK %s\n",vsrv->vgateway_vif[i].ifname, ret ? "failed" : "ok");
			
			}
		}
	}
	if((VRRP_LINK_NO_SETTED != vsrv->l2_uplink_flag)&&(VRRP_ON == vsrv->l2_uplink_back_down_flag)){
		had_br_uplink_rcv_pck(vsrv);/* set l2-uplink interfaces to old state  */
	}
	
	return VRRP_RETURN_CODE_SUCCESS;
}

unsigned int had_vrrp_vif_rcv_pck_OFF
(
	vrrp_rt* vsrv
)
{	
	int i = 0;
	int ret = 0;
	if(!vsrv){
		return VRRP_RETURN_CODE_ERR;
		}
	vrrp_syslog_dbg("vrrp vif interfaces rcv packets\n");
	if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag)&&(VRRP_OFF == vsrv->uplink_back_down_flag))
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
			{
				ret = had_set_intf_state_byIoctl(vsrv->uplink_vif[i].ifname,TRUE);
				vrrp_syslog_dbg("uplink vrrp vif %s rx OK %s\n",vsrv->uplink_vif[i].ifname, ret ? "failed" : "ok");
			
			}
		}
	}
	if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag)&&(VRRP_OFF == vsrv->downlink_back_down_flag))
		{
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
				{
					ret = had_set_intf_state_byIoctl(vsrv->downlink_vif[i].ifname,TRUE);
					vrrp_syslog_dbg("downlink vrrp vif %s rx OK %s\n",vsrv->downlink_vif[i].ifname, ret ? "failed" : "ok");
				
				}
			}
		}
	if ((VRRP_LINK_NO_SETTED != vsrv->vgateway_flag)&&(VRRP_OFF == vsrv->vgateway_back_down_flag))
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg)
			{
				ret = had_set_intf_state_byIoctl(vsrv->vgateway_vif[i].ifname,TRUE);
				vrrp_syslog_dbg("vgateway vrrp vif %s rx OK %s\n",vsrv->vgateway_vif[i].ifname, ret ? "failed" : "ok");
			
			}
		}
	}
	if((VRRP_LINK_NO_SETTED != vsrv->l2_uplink_flag)&&(VRRP_OFF == vsrv->l2_uplink_back_down_flag)){
		had_br_uplink_rcv_pck(vsrv);/* set l2-uplink interfaces to old state  */
	}
	
	return VRRP_RETURN_CODE_SUCCESS;
}



/**************************************************************
 * had_br_uplink_info_get
 *         get the current br uplink interfaces , and add them to link had_br_uplink_info
 *  INPUT:
 *		vsrv  : vrrp_rt * -- get br uplink info of this vrrp
 * OUTPUT:
 *		NONE
 * RETURN:
 *		getCount   --  return the count we got
 *
 **************************************************************/

unsigned int had_br_uplink_info_get
(
	vrrp_rt* vsrv
)
{
#define HAD_BRIDGE_INTERFACE_PATH "/sys/class/net/%s/bridge/uplink_port"
	unsigned int i = 0;
	unsigned char uplink_file[128];
	unsigned char strbuf[256]; /* there are less than 4 uplink interfaces in one file */
	unsigned char * ifname = NULL;
	unsigned int count = 0;
	unsigned int getCount = 0;
	if(!vsrv){return VRRP_RETURN_CODE_ERR;}
	had_br_uplink_info_clear(vsrv);
	for(i = 0;i < VRRP_LINK_MAX_CNT;i++){	
		if(vsrv->l2_uplink_vif[i].set_flg != VRRP_LINK_SETTED){
			continue;
			}
		if(!strncmp(vsrv->l2_uplink_vif[i].ifname, "ebr", strlen("ebr"))){
			sprintf(uplink_file, HAD_BRIDGE_INTERFACE_PATH, vsrv->l2_uplink_vif[i].ifname);
			int fd = open(uplink_file, O_RDONLY);
			if(fd < 0){
				vrrp_syslog_error("open file %s failed when get uplink interfaces of %s\n", uplink_file, vsrv->l2_uplink_vif[i].ifname);
				return -1;
			}
			if(0 > read(fd, strbuf, 256)){
				vrrp_syslog_error("read failed when get uplink interfaces of %s,err: %s \n", vsrv->l2_uplink_vif[i].ifname,strerror(errno));
				close(fd);
				return -1;
			}
			vrrp_syslog_dbg("read string %s from file %s success\n",strbuf, uplink_file);
			close(fd);
			ifname = strtok(strbuf, " \r\n\t");
			while(count<4){
				if(ifname == NULL)
				{
					break;
				}
				strcpy(vsrv->l2_uplink_vif[i].l2uplinkinfo[count].ifname, ifname);
				vsrv->l2_uplink_vif[i].l2uplinkinfo[count].setflag= VRRP_LINK_SETTED;
				count ++ ;
				getCount ++;
				ifname = strtok(NULL, " \r\n\t");
			}
		}
	}
	vrrp_syslog_dbg("had_br_uplink_info_get %d nodes add to the table\n",getCount);
	return getCount;
}
/**************************************************************
 * had_br_uplink_info_clear
 *         clear the link had_br_uplink_infos
 *  INPUT:
 *		NONE
 * OUTPUT:
 *		NONE
 * RETURN:
 *		count   --  return the count we cleared
 *
 **************************************************************/

unsigned int had_br_uplink_info_clear
(
	vrrp_rt *vsrv
)
{

	if(!vsrv){return VRRP_RETURN_CODE_ERR;}

	int i = 0;
	for(i = 0;i < VRRP_LINK_MAX_CNT;i++)
		{
			if(vsrv->l2_uplink_vif[i].set_flg != VRRP_LINK_SETTED){
				continue;
				}
			memset(vsrv->l2_uplink_vif[i].l2uplinkinfo,0,sizeof(l2uplink_portinfo)*4);
		}
	return VRRP_RETURN_CODE_SUCCESS;;
}

/**************************************************************
 * had_l2_uplink_ifname_add
 *  INPUT:
 *		vsrv  :  struct vrrp_rt*  --  config for this vrrp structure
 *		ifname : string  -- the interface name we want to delete
 * OUTPUT:
 *		NONE
 * RETURN:
 *		VRRP_RETURN_CODE_ERR                       --  null pointer input
 *		VRRP_RETURN_CODE_IF_EXIST               --  the interface we want to add exists
 *		VRRP_RETURN_CODE_IF_UP_LIMIT       --  the interface added full
 *		VRRP_RETURN_CODE_SUCCESS		   --  success
 *
 **************************************************************/
unsigned int had_l2_uplink_ifname_add
(
	vrrp_rt *vsrv,
	unsigned char * ifname
)
{
	int i = 0;
	if(!vsrv){return VRRP_RETURN_CODE_ERR;}
	if(!ifname){return VRRP_RETURN_CODE_ERR;}
	for(i = 0; i < VRRP_LINK_MAX_CNT; i++){
		if(!strcmp(vsrv->l2_uplink_vif[i].ifname, ifname)){
			return VRRP_RETURN_CODE_IF_EXIST;
		}
	}
	for(i = 0; i < VRRP_LINK_MAX_CNT; i++){
		if(!strcmp(vsrv->l2_uplink_vif[i].ifname, "")){
			strcpy(vsrv->l2_uplink_vif[i].ifname, ifname);
			vsrv->l2_uplink_vif[i].set_flg = VRRP_LINK_SETTED;
			vsrv->l2_uplink_naddr ++;
			return VRRP_RETURN_CODE_SUCCESS;
		}
	}
	return VRRP_RETURN_CODE_IF_UP_LIMIT;
}
/**************************************************************
 * had_l2_uplink_ifname_delete
 *  INPUT:
 *		vsrv  :  struct vrrp_rt*  --  config for this vrrp structure
 *		ifname : string  -- the interface name we want to delete
 * OUTPUT:
 *		NONE
 * RETURN:
 *		VRRP_RETURN_CODE_ERR                       --  null pointer input
 *		VRRP_RETURN_CODE_IF_NOT_EXIST        --  the interface we want to delete not exists
 *		VRRP_RETURN_CODE_SUCCESS		   --  success
 *
 **************************************************************/
unsigned int had_l2_uplink_ifname_delete
(
	vrrp_rt *vsrv,
	unsigned char * ifname
)
{
	int i = 0;
	if(!vsrv){return VRRP_RETURN_CODE_ERR;}
	if(!ifname){return VRRP_RETURN_CODE_ERR;}
	for(i = 0; i < VRRP_LINK_MAX_CNT; i++){
		if(!strcmp(vsrv->l2_uplink_vif[i].ifname, ifname)){
			memset(vsrv->l2_uplink_vif[i].ifname, 0, MAX_IFNAME_LEN);
			vsrv->l2_uplink_vif[i].set_flg = VRRP_LINK_NO_SETTED;
			vsrv->l2_uplink_naddr --;
			if(!vsrv->l2_uplink_naddr){
				vsrv->l2_uplink_flag = VRRP_LINK_NO_SETTED;
			}
			return VRRP_RETURN_CODE_SUCCESS;
		}
	}
	return VRRP_RETURN_CODE_IF_NOT_EXIST;
}

uint32_t had_set_intf_state_byIoctl
(
	char *ifname,
	unsigned int rx_flag
)
{

#define IFF_RX_PREVENT 0x1
#define SIOCGIFUDFFLAGS	0x893d		/* get udf_flags			*/
#define SIOCSIFUDFFLAGS	0x893e		/* set udf_flags			*/

	struct ifreq ifr;
	int fd = -1;

	if (!ifname) {
		vrrp_syslog_error("get interface link status with null name!\n");
		return (1);
	}

	memset(&ifr, 0, sizeof(struct ifreq));
#ifndef VRRP_GLOBAL_SOCKET_FD
	fd	= socket(AF_INET, SOCK_DGRAM, 0);
#else 
	fd = vrrp_socket_inet_dgram_0_fd;
#endif
	if (fd < 0) {
		vrrp_syslog_error("get interface %s link status init socket error %s!\n",
							ifname, strerror(errno));
		return VRRP_RETURN_CODE_ERR;
	}

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	/* get before set*/
	if (ioctl(fd, SIOCGIFUDFFLAGS, (char *)&ifr) == 0) {
		vrrp_syslog_dbg("get interface %s ifr.ifr_flags:%x",ifname,ifr.ifr_flags);
	}else {
		vrrp_syslog_error("get interface %s flags ioctl failed %s!\n",
							ifname, strerror(errno));
#ifndef VRRP_GLOBAL_SOCKET_FD
		close(fd);
#endif
		return VRRP_RETURN_CODE_ERR;
	}
	
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if(FALSE != rx_flag){
		ifr.ifr_flags &= ~(IFF_RX_PREVENT);
	}
	else{		
		ifr.ifr_flags |= (IFF_RX_PREVENT);/* set rx prevent */
	}
	if (ioctl(fd, SIOCSIFUDFFLAGS, (char *)&ifr) == 0) {
		vrrp_syslog_dbg("set interface %s RX %s ifr.ifr_flags:%x",ifname, rx_flag ? "OK":"PREVENT", ifr.ifr_flags);
	}else {
		vrrp_syslog_error("set interface %s link status ioctl failed %s!\n",
							ifname, strerror(errno));
#ifndef VRRP_GLOBAL_SOCKET_FD
		close(fd);
#endif
		return VRRP_RETURN_CODE_ERR;
	}
	
#ifndef VRRP_GLOBAL_SOCKET_FD
	close(fd);
#endif

	return VRRP_RETURN_CODE_SUCCESS;
}


/****************************************************************
 NAME	: had_state_goto_disable			00/02/07 00:15:26
 AIM	:
 REMARK	: called when the state is no more MASTER
****************************************************************/
void had_state_goto_disable
(
	vrrp_rt *vsrv,
	int advF,
	struct iphdr* uplink_iph,
	struct iphdr* downlink_iph
)
{
	vrrp_if *vgateway_vif = NULL;
	vrrp_pkt *uplink_hd = NULL;
	vrrp_pkt *downlink_hd = NULL;
	int i = 0;
	int delay = 0;
	/* be allow uplink_iph & downlink_iph is NULL */
	if (!vsrv) {
		vrrp_syslog_error("state goto disable null parameter error\n");
		return;
	}
	
	if (VRRP_STATE_DISABLE == vsrv->state) {
		return;
	}

	if (uplink_iph) {
		uplink_hd = (vrrp_pkt *)((char *)uplink_iph + (uplink_iph->ihl<<2));		
	}
	if (downlink_iph) {
		downlink_hd = (vrrp_pkt *)((char *)downlink_iph + (downlink_iph->ihl<<2));
	}
	
	/* if we stop vrrpd, warn the other routers to speed up the recovery */
	if (advF) {	
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","send stop packet to opposite");
		vrrp_send_adv( vsrv, VRRP_PRIO_STOP );		
	}

	/* restore the original MAC addresses */
	vrrp_syslog_dbg("vrrp %d start to leave master to disable!\n",vsrv->vrid);
	if ((VRRP_STATE_MAST == vsrv->state) &&
		(!vsrv->no_vmac))
	{	/* uplink */
		if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
		{
			/*trace log*/
			vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","chang uplink virtual mac to sysmac");
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
				{
					had_hwaddr_set(vsrv->uplink_vif[i].ifname,
								vsrv->uplink_vif[i].hwaddr, sizeof(vsrv->uplink_vif[i].hwaddr) );
					had_rcvhwaddr_op(vsrv->uplink_vif[i].ifname,
								vsrv->uplink_vif[i].hwaddr, sizeof(vsrv->uplink_vif[i].hwaddr), 0);
				}
			}
		}
		
		/* downlink */
        if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
		{
			/*trace log*/
			vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","chang downlink virtual mac to sysmac");
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
				{
					had_hwaddr_set(vsrv->downlink_vif[i].ifname,
								vsrv->downlink_vif[i].hwaddr, sizeof(vsrv->downlink_vif[i].hwaddr) );
					had_rcvhwaddr_op(vsrv->downlink_vif[i].ifname,
								vsrv->downlink_vif[i].hwaddr, sizeof(vsrv->downlink_vif[i].hwaddr), 0);
				}
			}
		}

		/* vgateway */
        if (VRRP_LINK_NO_SETTED != vsrv->vgateway_flag)
		{
			/*trace log*/
			vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","chang vgateway virtual mac to sysmac");
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
				if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg) {
					vgateway_vif = &(vsrv->vgateway_vif[i]);
					if (vsrv->smart_vmac_enable ){
						had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 0);
						had_hwaddr_set(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr) );
						had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 1);
						vrrp_syslog_dbg("vrrp %d set smart_vmac to disable\n",vsrv->vrid);
						}
        			else if(strncmp(vgateway_vif->ifname,"ebr",3)&& \
					strncmp(vgateway_vif->ifname,"wlan",4)){
				had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 0);
				had_hwaddr_set(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr) );
				had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 1);
			}
			else{
				char buf[128] = {0};
				sprintf(buf, "%s %s %02x:%02x:%02x:%02x:%02x:%02x %d.%d.%d.%d", \
							"setebrmac.sh",
							vgateway_vif->ifname,
							vgateway_vif->hwaddr[0], vgateway_vif->hwaddr[1], vgateway_vif->hwaddr[2],
							vgateway_vif->hwaddr[3], vgateway_vif->hwaddr[4], vgateway_vif->hwaddr[5],
							(vgateway_vif->ipaddr & 0xff000000)>>24,
							(vgateway_vif->ipaddr & 0xff0000)>>16,
							(vgateway_vif->ipaddr & 0xff00)>>8,
							vgateway_vif->ipaddr & 0xff);
				system(buf);
				vrrp_syslog_event("call setebrmac.sh,ifname %s,mac %02x:%02x:%02x:%02x:%02x:%02x,ip %d.%d.%d.%d\n",\
								vgateway_vif->ifname,
								vgateway_vif->hwaddr[0], vgateway_vif->hwaddr[1], vgateway_vif->hwaddr[2],
								vgateway_vif->hwaddr[3], vgateway_vif->hwaddr[4], vgateway_vif->hwaddr[5],
								(vgateway_vif->ipaddr & 0xff000000)>>24,
								(vgateway_vif->ipaddr & 0xff0000)>>16,\
								(vgateway_vif->ipaddr & 0xff00)>>8,
								vgateway_vif->ipaddr & 0xff);
					}
        		}
			}
		}
	}

	if(VRRP_STATE_MAST == vsrv->state){
		/* remove the ip addresses */	
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","delete virtaul ip");
		had_ipaddr_ops( vsrv, 0 );
		/* from master to disable ,inc switch times */
	    vsrv->backup_switch_times++;
	}
	
	/* send gratuitous ARP for all the non-vrrp ip addresses to update
	** the cache of remote hosts using these addresses */
	#if 0
	if((VRRP_STATE_MAST == vsrv->state)&&(!vsrv->no_vmac)){
		int		i, naddr;
		/*
		naddr = had_ipaddr_list( had_ifname_to_idx(vif->ifname), addr
				, sizeof(addr)/sizeof(addr[0]) );
		for( i = 0; i < naddr; i++ )
			had_send_gratuitous_arp( vsrv, addr[i], 0 );
		*/
		/*uplink */
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","sending uplink gratuitous arp");
		
		if(0 != vsrv->uplink_flag){
			naddr = had_ipaddr_list( had_ifname_to_idx(uplink_vif->ifname), addr
					, sizeof(addr)/sizeof(addr[0]) );
			for( i = 0; i < naddr; i++ )
				had_send_uplink_gratuitous_arp( vsrv, addr[i], 0 );

		}
		/*downlink */
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","sending downlink gratuitous arp");
		
		if(0 != vsrv->downlink_flag){
			naddr = had_ipaddr_list( had_ifname_to_idx(downlink_vif->ifname), addr
					, sizeof(addr)/sizeof(addr[0]) );
			for( i = 0; i < naddr; i++ )
				had_send_downlink_gratuitous_arp( vsrv, addr[i], 0 );

		}		
		/*vgateway */
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO DISABLE","sending vgateway gratuitous arp");
		
		if((0 != vsrv->vgateway_flag) && (! vsrv->no_vmac )) {
			naddr = had_ipaddr_list( had_ifname_to_idx(vgateway_vif->ifname), addr
					, sizeof(addr)/sizeof(addr[0]) );
			/*hardware get*/
				had_hwaddr_get(vsrv->vgateway_vif.ifname,vsrv->vgateway_vif.hwaddr,6);
				for( i = 0; i < naddr; i++ )				
					had_send_vgateway_gratuitous_arp( vsrv, addr[i], 0 );

		}			
	}	
	#endif 
	vsrv->state = VRRP_STATE_DISABLE;
	hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
	/*trace log*/
	vrrp_state_trace(vsrv->profile, vsrv->state, "SET DISABLE","set disable state");
	
	if (!uplink_iph &&
		!downlink_iph)
	{
		if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag) {
            master_ipaddr_uplink[vsrv->vrid] = 0;
		}
		if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag) {
		    master_ipaddr_downlink[vsrv->vrid] = 0;
		}
		if ((NULL != global_ht_ifname) &&
			(0 != global_ht_ip))
		{
            global_ht_opposite_ip = 0;
		}
	}
	
	/*leave vrrp*/
    vrrp_syslog_dbg("vrrp %d goto disable state!\n",vsrv->vrid);
	/*nofity to wtp*/
	#if 0
	if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
		(VRRP_LINK_SETTED == vsrv->downlink_flag)) {
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"DISABLE START","notify wid disable state");
		int i = 0;
		for (i = 0; i < vsrv->downlink_naddr; i++)
		{
			had_notify_to_wid(vsrv->vrid,VRRP_STATE_DISABLE,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);		 
			if (global_protal) {
				had_notify_to_protal(vsrv->vrid,VRRP_STATE_DISABLE,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);
			}
		}
	}
	else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
			 (VRRP_LINK_NO_SETTED == vsrv->downlink_flag)) {
		/*trace log*/
		vrrp_state_trace(vsrv->profile, vsrv->state, "DISABLE START", "notify portal disable state");
		int i = 0;
		for (i = 0; i < vsrv->uplink_naddr; i++)
		{
			had_notify_to_wid(vsrv->vrid, VRRP_STATE_DISABLE,
								master_ipaddr_uplink[vsrv->vrid -1],
								0,
								vsrv->uplink_vaddr[i].addr,
								0);
			if (global_protal) {
				had_notify_to_protal(vsrv->vrid, VRRP_STATE_DISABLE,
										master_ipaddr_uplink[vsrv->vrid -1],
										0,
										vsrv->uplink_vaddr[i].addr,
										0);
			}
		}
	}
	else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
			 (VRRP_LINK_SETTED == vsrv->downlink_flag)) {
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"DISABLE START","notify portal disable state");
		
	   had_notify_to_wid(vsrv->vrid,VRRP_STATE_DISABLE,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);		
	   if(global_protal)
	      had_notify_to_protal(vsrv->vrid,VRRP_STATE_DISABLE,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);		
	}
	#else 
	had_notify_to_wid(vsrv, VRRP_STATE_DISABLE);
	if (global_protal) {
	   had_notify_to_protal(vsrv,VRRP_STATE_DISABLE);
	}
#ifndef _VERSION_18SP7_	
	if (global_pppoe) {
	   had_notify_to_pppoe(vsrv,VRRP_STATE_DISABLE);
	}
#endif	
	/* book add, 2011-5-11 */
	had_notify_to_hmd(vsrv,VRRP_STATE_DISABLE);
	
	had_notify_to_dhcp_failover(vsrv,VRRP_STATE_DISABLE);
	#endif

	#if 0
	/*notify dhcp*/
	had_notify_to_dhcp(vsrv,0);	
	#endif
	/*send msg to trap*/
	had_notify_to_trap(vsrv);	
	/*send msg to snmp mib */
	had_notify_to_snmp_mib(vsrv);/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"DISABLE LOOP","goto disable loop");

	/*MASTER will go to DISBALE reset mater uplink and downlink down timer*/
	delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv) +1001000;
	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
		VRRP_TIMER_SET( vsrv->uplink_ms_down_timer, delay );
	if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
		VRRP_TIMER_SET( vsrv->downlink_ms_down_timer, delay ); 
	vrrp_syslog_info("reset uplink/downlink master down timer!\n",vsrv->vrid);
	vrrp_syslog_info("master down timer(u-%d d-%d) \n",vsrv->uplink_ms_down_timer, vsrv->downlink_ms_down_timer);
	return;
}

int had_state_from_disable
(
	vrrp_rt* vsrv,
	vrrp_pkt* hd
)
{
	int priority = 0, i = 0;

	if (!vsrv || !hd) {
		vrrp_syslog_error("state from disable null parameter error\n");
		return 0;
	}
	
	#if 1
	/* add by jinpec, for */
	if ((VRRP_VGATEWAY_TF_FLG_ON == vsrv->vgateway_tf_flag) &&
			(0 != vsrv->vgateway_flag)) {
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++) {
			if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg) {
				if(INTERFACE_UP == had_ifname_to_status(vsrv, vsrv->vgateway_vif[i].ifname)) {
					break;
				}

			}
		}
		
		if(VRRP_LINK_MAX_CNT == i) {
			vrrp_syslog_error("all vgateway are linkdown or not setm but tf flag is on.\n");
			return 0;
		}
	}
	#endif
	
	priority = hd->priority;
	vrrp_syslog_dbg("vrrp %d priority %d, received packets priority %d, state %d\n",
					vsrv->vrid, vsrv->priority, priority, hd->state);

	/* peer state is LEARN,
	 * and peer priority smaller
	 */
	if ((hd->state == VRRP_STATE_LEARN) &&
		(priority < vsrv->priority))
	{
		/*directly goto master*/
		vrrp_syslog_dbg("goto master state from learn directly!\n");
		/*trace log*/
		vrrp_state_trace(vsrv->profile, vsrv->state,
							"GOTO MASTER","opposite is learn state and priority is smaller,goto master");		
		vrrp_syslog_info("state %s opposite state learn and low privilege(pri %d mine %d), goto master!\n", \
						VRRP_STATE_DESCRIPTION(vsrv->state), priority, vsrv->priority);
		had_state_goto_master(vsrv,VRRP_STATE_LEARN);
		return 0;
	}
	
	/*reset back_master_transfer_by_heartbeat link , for heartbeat link has been repared!*/
	if (vsrv->preempt &&
		(priority < vsrv->priority))
	{
		/*set bath syn state*/
		vrrp_set_wid_bathsyn_start(vsrv->profile);
		vrrp_set_portal_bathsyn_start(vsrv->profile);
	   
		vrrp_syslog_dbg("receive packets priority is smaller,notify wid and state goto transfer!\n");
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,
							"GOTO TRANSFER","preempt and opposite priority is smaller,goto transfer for bach syn");	   
		#if 0
	   if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
	   	   (VRRP_LINK_SETTED == vsrv->downlink_flag)) {
			int i = 0;
			for (i = 0; i < vsrv->downlink_naddr; i++)
			{
				had_notify_to_wid(vsrv->vrid,VRRP_STATE_BACK,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);	   
				if (global_protal) {
					had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);
				}
			}
	   }
	   else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
				(VRRP_LINK_NO_SETTED == vsrv->downlink_flag)) {
			int i = 0;
			for (i = 0; i < vsrv->uplink_naddr; i++)
			{
				had_notify_to_wid(vsrv->vrid, VRRP_STATE_BACK,
									master_ipaddr_uplink[vsrv->vrid -1],
									0,
									vsrv->uplink_vaddr[i].addr,
									0);
				if (global_protal) {
					had_notify_to_protal(vsrv->vrid, VRRP_STATE_BACK,
											master_ipaddr_uplink[vsrv->vrid -1],
											0,
											vsrv->uplink_vaddr[i].addr,
											0);
				}
			}
	   }
	   else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
				(VRRP_LINK_SETTED == vsrv->downlink_flag)) {
		 had_notify_to_wid(vsrv->vrid,VRRP_STATE_BACK,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);		  
		 if(global_protal)
			had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);	  
	   }
	   	#else 
		had_notify_to_wid(vsrv, VRRP_STATE_BACK);
		if (global_protal) {
		   had_notify_to_protal(vsrv, VRRP_STATE_BACK);
		}
	#ifndef _VERSION_18SP7_
		if (global_pppoe) {
		   had_notify_to_pppoe(vsrv, VRRP_STATE_BACK);
		}
	#endif	
		/* book add, 2011-5-11 */
    	had_notify_to_hmd(vsrv, VRRP_STATE_BACK);
    	
		had_notify_to_dhcp_failover(vsrv,VRRP_STATE_BACK);
		#endif
	   had_state_goto_transfer(vsrv);
	}
	else{
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO BACK","priority is smaller,goto back");

		had_state_back(vsrv,hd);
	}
	return 0;
}


int had_state_from_learn
(
   vrrp_rt* vsrv,
   vrrp_pkt* hd
)
{
	int priority = 0;

	if (!vsrv || !hd) {
		vrrp_syslog_error("state from learn null parameter error\n");
		return 1;
	}

	priority = hd->priority;
	vrrp_syslog_dbg("vrrp %d priority %d, received packets priority %d,state %d\n",
					vsrv->vrid, vsrv->priority, priority, hd->state);
	if ((priority < vsrv->priority) && \
		((VRRP_PRIO_OWNER == vsrv->priority) ||
		 (vsrv->preempt)))
	{
       /*if receive form learn state and opposite priority is smaller*/
	   if(VRRP_STATE_LEARN == hd->state){
	   	  /*trace log*/
          vrrp_state_trace(vsrv->profile,vsrv->state,"CONTINUE LEARN","received priority is smaller,continue");
	   	  return 0;
	   }
	   /*set bath syn state*/
	   vrrp_set_wid_bathsyn_start(vsrv->profile);
	   vrrp_set_portal_bathsyn_start(vsrv->profile);
	   
	   vrrp_syslog_dbg("receive packets priority is smaller,notify wid and state goto transfer!\n");
	   /*trace log*/
	   vrrp_state_trace(vsrv->profile,vsrv->state,"LEARN CHANG","received priority is better,goto transfer for bath syn");
		#if 0
	   if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
	   	   (VRRP_LINK_SETTED == vsrv->downlink_flag)) {
			int i = 0;
			for (i = 0; i < vsrv->downlink_naddr; i++)
			{
				had_notify_to_wid(vsrv->vrid,VRRP_STATE_BACK,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);	   
				if	(global_protal) {
					had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);
				}
			}
	   }
	   else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
				(VRRP_LINK_NO_SETTED == vsrv->downlink_flag)) {
			int i = 0;
			for (i = 0; i < vsrv->uplink_naddr; i++)
			{
				had_notify_to_wid(vsrv->vrid, VRRP_STATE_BACK,
									master_ipaddr_uplink[vsrv->vrid -1],
									0,
									vsrv->uplink_vaddr[i].addr,
									0);
				if (global_protal) {
					had_notify_to_protal(vsrv->vrid, VRRP_STATE_BACK,
								 	master_ipaddr_uplink[vsrv->vrid -1],
										0,
										vsrv->uplink_vaddr[i].addr,
										0);
				}
			}
	   }
	   else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
				(VRRP_LINK_SETTED == vsrv->downlink_flag)) {
		 had_notify_to_wid(vsrv->vrid,VRRP_STATE_BACK,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);		  
		 if(global_protal)
			had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr);	  
	   }
	   	#else 
		had_notify_to_wid(vsrv, VRRP_STATE_BACK);
		if (global_protal) {
		   had_notify_to_protal(vsrv, VRRP_STATE_BACK);
		}
	#ifndef _VERSION_18SP7_	
		if (global_pppoe) {
		   had_notify_to_pppoe(vsrv, VRRP_STATE_BACK);
		}
	#endif	
		/* book add, 2011-5-11 */
    	 had_notify_to_hmd(vsrv, VRRP_STATE_BACK);
    	
		had_notify_to_dhcp_failover(vsrv,VRRP_STATE_BACK);
		#endif
	   had_state_goto_transfer(vsrv);
	}
	else{
       had_state_back(vsrv,hd);
	}	
	return 0;
}

int had_state_back
(
   vrrp_rt* vsrv,
   vrrp_pkt* hd
)
{
	int delay = 0;
	int priority = 0;
	int i;

	if (!vsrv || !hd) {
		vrrp_syslog_error("state to back null parameter error\n");
		return 0;
	}
	
	priority = hd->priority;
	vrrp_syslog_dbg("vrrp %d priority %d, received packets priority %d,state %d\n",
					vsrv->vrid, vsrv->priority, priority, hd->state);
	#if 0
	if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
		(VRRP_LINK_SETTED == vsrv->downlink_flag))
	{
		int i = 0;
		for (i = 0; i < vsrv->downlink_naddr; i++)
		{
			had_notify_to_wid(vsrv->vrid,VRRP_STATE_BACK,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);		
			if (global_protal) {
				had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,0,master_ipaddr_downlink[vsrv->vrid -1],0,vsrv->downlink_vaddr[i].addr);
			}
		}
    }
	else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
			 (VRRP_LINK_NO_SETTED == vsrv->downlink_flag))
	{
		int i = 0;
		for (i = 0; i < vsrv->uplink_naddr; i++)
		{
			had_notify_to_wid(vsrv->vrid, VRRP_STATE_BACK,
								master_ipaddr_uplink[vsrv->vrid -1],
								0,
								vsrv->uplink_vaddr[i].addr,
								0);		 
			if (global_protal) {
				had_notify_to_protal(vsrv->vrid, VRRP_STATE_BACK,
										master_ipaddr_uplink[vsrv->vrid -1],
										0,
										vsrv->uplink_vaddr[i].addr,
										0); 	 
			}
		}
	}
    else if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
			 (VRRP_LINK_SETTED == vsrv->downlink_flag)) {
	  had_notify_to_wid(vsrv->vrid,VRRP_STATE_BACK,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr); 	   
	  if(global_protal)
	     had_notify_to_protal(vsrv->vrid,VRRP_STATE_BACK,master_ipaddr_uplink[vsrv->vrid -1],master_ipaddr_downlink[vsrv->vrid -1],vsrv->uplink_vaddr[0].addr,vsrv->downlink_vaddr[0].addr); 	   
    }	
	#else 
	had_notify_to_wid(vsrv, VRRP_STATE_BACK);
	if (global_protal) {
	   had_notify_to_protal(vsrv, VRRP_STATE_BACK);
	}
#ifndef _VERSION_18SP7_	
	if (global_pppoe) {
	   had_notify_to_pppoe(vsrv, VRRP_STATE_BACK);
	}
#endif	
	/* book add, 2011-5-11 */
	had_notify_to_hmd(vsrv, VRRP_STATE_BACK);
	
	had_notify_to_dhcp_failover(vsrv,VRRP_STATE_BACK);
	#endif

	delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
		VRRP_TIMER_SET( vsrv->uplink_ms_down_timer, delay );	
	if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
		VRRP_TIMER_SET( vsrv->downlink_ms_down_timer, delay );
	#if 0
	/*notify dhcp*/
	had_notify_to_dhcp(vsrv,0);	
	#endif

	vsrv->state = VRRP_STATE_BACK;	
	hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
	had_vrrp_vif_drop_pck(vsrv);
	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"BACK SET","set back stats");	
	/* notify trap */
	had_notify_to_trap(vsrv);
	if(time_synchronize_enable){
		/*querry packet for system time*/
		vsrv->que_f = 1;
		vrrp_send_adv(vsrv,vsrv->priority);	
		vsrv->que_f = 0;
	}
	if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag){
    	for (i = 0; i < VRRP_LINK_MAX_CNT; i++){
    		if( strlen(vsrv->uplink_vif[i].ifname) == 0 ){
				continue;
    		}
    		had_state_notifier_rtmd((vsrv->uplink_vif[i].ifname),VRRP_NOTIFY_RTMD_STATE_BACK);
			vrrp_syslog_info("uplink interface name : %s . vrrp state to BACK ,netlink notifier rtmd success!!\n",vsrv->uplink_vif[i].ifname);
    	}
	}
	if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag){
    	for (i = 0; i < VRRP_LINK_MAX_CNT; i++){
    		if( strlen(vsrv->downlink_vif[i].ifname) == 0 ){
    		    continue;
    		}
    		had_state_notifier_rtmd((vsrv->downlink_vif[i].ifname),VRRP_NOTIFY_RTMD_STATE_BACK);
			vrrp_syslog_info("down link interface name : %s . vrrp state to BACK ,netlink notifier rtmd success!!\n",vsrv->downlink_vif[i].ifname);
    	}
	}
	return 0;
}

static void had_get_sys_mac
(
   char* sysmac
)
{
   	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
    int op_ret = 0;
	unsigned int ifindex = 0;
	char mac[6] = {0};
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VRRP_OBJPATH,			\
								NPD_DBUS_VRRP_INTERFACE,					\
								NPD_DBUS_VRRP_METHOD_GET_SYSMAC);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ifindex,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (vrrp_notify_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vrrp_syslog_dbg("Failed to get system reply!\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		memcpy(sysmac,mac,sizeof(mac));
		return;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if (0 != op_ret){
	   dbus_message_unref(reply);
	   op_ret = VRRP_RETURN_CODE_ERR;
	}
	else {
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&mac[0]);
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&mac[1]);		
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&mac[2]);
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&mac[3]);
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&mac[4]);
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&mac[5]);
		memcpy(sysmac,mac,sizeof(mac));
		dbus_message_unref(reply);
	}  
	return;
}

static void had_change_mac
(
	vrrp_rt* vsrv
)
{
	int i = 0;
	vrrp_if *vgateway_vif = NULL;
	char mac[VRRP_MAC_ADDRESS_LEN] = {0};
	char vir_mac[VRRP_MAC_ADDRESS_LEN] = {0};

	if (!vsrv) {
		vrrp_syslog_error("change mac null parameter error\n");
		return;
	}
	vrrp_syslog_dbg("start to change virtual mac!\n");

	/* set virtual mac of vrrp instance */
	memcpy(vir_mac, vrrp_vmac, VRRP_MAC_ADDRESS_LEN);
	vir_mac[5] = vsrv->vrid;

	/* uplink */
	if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg)
			{
				memset(mac, 0, VRRP_MAC_ADDRESS_LEN);
				if (!vsrv->no_vmac)	{
					memcpy(mac, vir_mac, VRRP_MAC_ADDRESS_LEN);
				}else {
					memcpy(mac, vsrv->uplink_vif[i].hwaddr, VRRP_MAC_ADDRESS_LEN);
				}
				vrrp_syslog_dbg("uplink %s set to new mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
								vsrv->uplink_vif[i].ifname,
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

				had_rcvhwaddr_op(vsrv->uplink_vif[i].ifname,vsrv->uplink_vif[i].hwaddr,
							sizeof(vsrv->uplink_vif[i].hwaddr), 0);
				had_hwaddr_set(vsrv->uplink_vif[i].ifname, mac, sizeof(mac) );
				had_rcvhwaddr_op(vsrv->uplink_vif[i].ifname, mac, sizeof(mac), 1);
			}
		}
	}
	
	if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg)
			{
				memset(mac, 0, VRRP_MAC_ADDRESS_LEN);
				if (!vsrv->no_vmac)	{
					memcpy(mac, vir_mac, VRRP_MAC_ADDRESS_LEN);
				}else {
					memcpy(mac, vsrv->downlink_vif[i].hwaddr, VRRP_MAC_ADDRESS_LEN);
				}
				vrrp_syslog_dbg("downlink %s set to new mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
								vsrv->downlink_vif[i].ifname,
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

				had_rcvhwaddr_op(vsrv->downlink_vif[i].ifname, vsrv->downlink_vif[i].hwaddr,
							sizeof(vsrv->downlink_vif[i].hwaddr), 0);
				had_hwaddr_set(vsrv->downlink_vif[i].ifname, mac, sizeof(mac) );
				had_rcvhwaddr_op(vsrv->downlink_vif[i].ifname, mac, sizeof(mac), 1);
			}
		}
	}
	
	if (VRRP_LINK_NO_SETTED != vsrv->vgateway_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			vgateway_vif = &(vsrv->vgateway_vif[i]);
			if (VRRP_LINK_NO_SETTED == vgateway_vif->set_flg) {
				continue;
			}
		memset(mac, 0, VRRP_MAC_ADDRESS_LEN);
		if (!vsrv->no_vmac) {
			memcpy(mac, vir_mac, VRRP_MAC_ADDRESS_LEN);
		}else {
			memcpy(mac, vgateway_vif->hwaddr, VRRP_MAC_ADDRESS_LEN);
		}
		vrrp_syslog_dbg("vgateway %s set to new mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
						vgateway_vif->ifname,
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		if (vsrv->smart_vmac_enable ){
			had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 0);
			had_hwaddr_set(vgateway_vif->ifname, mac, sizeof(mac) );
			had_rcvhwaddr_op(vgateway_vif->ifname, mac, sizeof(mac), 1);
			vrrp_syslog_dbg("vrrp %d set smart_vmac to vmac change\n",vsrv->vrid);
			}
		else if (strncmp(vgateway_vif->ifname,"ebr",3)&&\
                            strncmp(vgateway_vif->ifname,"wlan",4)){	
            /* for non-ebr interface */
			had_rcvhwaddr_op(vgateway_vif->ifname, vgateway_vif->hwaddr, sizeof(vgateway_vif->hwaddr), 0);
			had_hwaddr_set(vgateway_vif->ifname, mac, sizeof(mac) );
			had_rcvhwaddr_op(vgateway_vif->ifname, mac, sizeof(mac), 1);
		}
		else
		{	/* for ebr interface */
			char buf[128] = {0};
			sprintf(buf, "%s %s %02x:%02x:%02x:%02x:%02x:%02x %d.%d.%d.%d",
						"setebrmac.sh",
						vgateway_vif->ifname,
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
						(vgateway_vif->ipaddr & 0xff000000) >> 24,
						(vgateway_vif->ipaddr & 0xff0000) >> 16,
						(vgateway_vif->ipaddr & 0xff00) >> 8,
						vgateway_vif->ipaddr & 0xff);
			system(buf);
			vrrp_syslog_event("call setebrmac.sh,ifname %s,mac %02x:%02x:%02x:%02x:%02x:%02x,ip %d.%d.%d.%d\n",\
								vgateway_vif->ifname,
								mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
								(vgateway_vif->ipaddr & 0xff000000)>>24,
								(vgateway_vif->ipaddr & 0xff0000)>>16,
								(vgateway_vif->ipaddr & 0xff00)>>8,
								vgateway_vif->ipaddr & 0xff);
		}
	}
	}

	/*send uplink arp*/
	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag){
		for( i = 0; i < VRRP_LINK_MAX_CNT; i++ ){
			if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) {
				had_send_uplink_gratuitous_arp( vsrv, vsrv->uplink_vif[i].ipaddr, i, 1 );
				had_send_uplink_gratuitous_arp( vsrv, vsrv->uplink_vaddr[i].addr, i, 1 );
			}
		}
		if(vsrv->state == VRRP_STATE_MAST){
			VRRP_TIMER_SET( vsrv->uplink_adver_timer, vsrv->adver_int );
		}
	}
	/*send downlink arp*/
	if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag){
		for( i = 0; i < VRRP_LINK_MAX_CNT; i++ ){
			if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) {
				had_send_downlink_gratuitous_arp( vsrv, vsrv->downlink_vif[i].ipaddr, i, 1 );
				had_send_downlink_gratuitous_arp( vsrv, vsrv->downlink_vaddr[i].addr, i, 1 );
			}
		}
		if(vsrv->state == VRRP_STATE_MAST){
			VRRP_TIMER_SET( vsrv->downlink_adver_timer, vsrv->adver_int );
		}
	}
	if(VRRP_LINK_NO_SETTED != vsrv->vgateway_flag){
		vrrp_syslog_dbg("send vgateway gratuitous arp!\n");
		for( i = 0; i < VRRP_LINK_MAX_CNT; i++ ){
			if (VRRP_LINK_SETTED == vsrv->vgateway_vif[i].set_flg) {
                had_send_vgateway_gratuitous_arp(vsrv,vsrv->vgateway_vif[i].ipaddr,i,1);
		        had_send_vgateway_gratuitous_arp(vsrv,vsrv->vgateway_vaddr[i].addr,i,1);
			}
		}
	}

	return;
}

/****************************************************************
 NAME	: had_state_init				00/02/07 00:15:26
 AIM	:
 REMARK	: rfc2338.6.4.1
****************************************************************/
static void had_state_init
(
vrrp_rt *vsrv
)
{
	int delay = 0;
	unsigned int uplink_state = INTERFACE_DOWN;
	unsigned int downlink_state = INTERFACE_DOWN;

	if (NULL == vsrv) {
		vrrp_syslog_error("state init paramter is null\n");
		return;
	}
	
	if ((VRRP_LINK_NO_SETTED == vsrv->uplink_flag) &&
		(VRRP_LINK_NO_SETTED == vsrv->downlink_flag))
	{
		vrrp_syslog_dbg("uplink and downlink not configured,next loop!\n");
		goto be_init;
	}
	if (VRRP_SERVICE_DISABLE == service_enable[vsrv->profile])
	{
		/*action trace*/
		vrrp_state_trace(vsrv->profile,VRRP_STATE_INIT,"INIT LOOP","service not enable");		 
		goto be_init;
	}
	if (VRRP_STATE_MAST == vsrv->wantstate)
	{
		had_state_goto_master(vsrv,VRRP_STATE_MAST);
		return;
	}

	if (((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
		 (INTERFACE_UP != (uplink_state = vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_UPLINK)))) ||
		((VRRP_LINK_NO_SETTED !=vsrv->downlink_flag) &&
		 (INTERFACE_UP != (downlink_state = vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_DOWNLINK)))) ||
		((NULL != global_ht_ifname) &&
		 (0 != global_ht_ip) &&
		 (!global_ht_state)))
	{
		vrrp_syslog_dbg("vrrp %d interfaces no ip or not up,next loop!\n",vsrv->vrid);
		/*action trace*/
		if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
			(INTERFACE_UP != uplink_state))
		{
			vrrp_state_trace(vsrv->profile,VRRP_STATE_INIT,"INIT LOOP","uplink interface down");
		}
		
		if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
			(INTERFACE_UP != downlink_state))
		{
			vrrp_state_trace(vsrv->profile,VRRP_STATE_INIT,"INIT LOOP","downlink interface down");
		}	
		if (!global_ht_state) {
			vrrp_state_trace(vsrv->profile,VRRP_STATE_INIT,"INIT LOOP","heartbeatlink interface down");
		}
		goto be_init;
	} 

#if 0
    if(((0 !=vsrv->uplink_flag)&&(!had_ifname_to_status(vsrv,vsrv->uplink_vif.ifname)))|| \
		  ((0 !=vsrv->downlink_flag)&&(!had_ifname_to_status(vsrv,vsrv->downlink_vif.ifname)))){
         vrrp_syslog_dbg("vrrp %d one interface no ip or not up,goto back!\n",vsrv->vrid);
		 goto be_back;
	}	
#endif
	vrrp_syslog_dbg("vrrp %d start init to learn!\n",vsrv->vrid);
	delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
		VRRP_TIMER_SET( vsrv->uplink_ms_down_timer, delay );
	if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
		VRRP_TIMER_SET( vsrv->downlink_ms_down_timer, delay );
	vsrv->state = VRRP_STATE_LEARN; 
	hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"LEARN SET","set learning state");	
	return;

be_init:	
	    vrrp_syslog_dbg("vrrp %d start init loop!\n",vsrv->vrid);
		vsrv->state = VRRP_STATE_INIT;	
		hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
		/*trace log*/
		vrrp_state_trace(vsrv->profile,vsrv->state,"INIT SET","set init state");	
		
		return;	
}

static void had_state_learn_loop
(
	vrrp_rt *vsrv
)
{
	unsigned char uplink_timeout = 0, downlink_timeout = 0;
	if (!vsrv) {
		vrrp_syslog_error("state learn loop paramter is null\n");
		return;
	}
	vrrp_syslog_dbg("state learning loop!\n");

	uplink_timeout = VRRP_TIMER_EXPIRED(vsrv->uplink_ms_down_timer);
	downlink_timeout =  VRRP_TIMER_EXPIRED(vsrv->downlink_ms_down_timer);
	if (((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) && uplink_timeout)|| \
		((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) && downlink_timeout)) {
		vrrp_syslog_dbg("time expire,state change from learn to master!\n");
		vrrp_state_trace(vsrv->profile,VRRP_STATE_LEARN,"LEARNING LOOP","learn expired,start goto master");
        /*goto master*/
		vrrp_syslog_info("state %s timer(u-%d d-%d) %s%sexpires, goto master!\n", \
						VRRP_STATE_DESCRIPTION(vsrv->state), vsrv->uplink_ms_down_timer, vsrv->downlink_ms_down_timer, \
						uplink_timeout ? "uplink ":"", downlink_timeout ? "downlink":"");
		had_state_goto_master( vsrv,VRRP_STATE_NONE);
	}
	else{
	 	/*LEARN NOTIFY*/
		vrrp_syslog_dbg("learn state,notify advertisement!\n");
		/*action trace*/
		vrrp_state_trace(vsrv->profile,VRRP_STATE_LEARN,"LEARNING LOOP","loop,send advertisement");		
		vrrp_send_adv(vsrv,vsrv->priority);
	}

	return;
}

static void had_state_transfer_loop
(
	vrrp_rt *vsrv
)
{
	if (!vsrv) {
		vrrp_syslog_error("state transfer loop paramter is null\n");
		return;
	}
	
	if((time_synchronize_enable)&&(!global_timer_reply)&&( global_timer_request < 3)){/*request for three time max*/
	     vsrv->que_f =1;
	     vrrp_send_adv(vsrv,vsrv->priority);
	     vsrv->que_f = 0;
	     global_timer_request++;
		 return;
	 }
	 vrrp_syslog_dbg("wid_transfer_state = %d global_protal = %d portal_transfer_state = %d\n"
	 	            ,(wid_transfer_state[vsrv->profile])
            	 	,global_protal
            	 	,(portal_transfer_state[vsrv->profile]));
	 if((wid_transfer_state[vsrv->profile])||(global_protal &&(portal_transfer_state[vsrv->profile]))){
         /*trace step*/
		 if((wid_transfer_state[vsrv->profile])&&(global_protal &&(portal_transfer_state[vsrv->profile]))){
	     	 vrrp_state_trace(vsrv->profile,VRRP_STATE_TRANSFER,"TRANSFER LOOP","wid & portal bach syning");		
		 }
         else if(wid_transfer_state[vsrv->profile]){
             vrrp_state_trace(vsrv->profile,VRRP_STATE_TRANSFER,"TRANSFER LOOP","wid bach syning");
		 }
		 else if(global_protal &&(portal_transfer_state[vsrv->profile])){
             vrrp_state_trace(vsrv->profile,VRRP_STATE_TRANSFER,"TRANSFER LOOP","portal bach syning");
		 }
		 /*log*/		 
		 vrrp_syslog_dbg("transfer loop...\n");
		 return;
	 }
	 /*reset timer request and reply*/
	 global_timer_request = 0;
	 global_timer_reply = 0;
	 vrrp_syslog_event("transfer over ,goto master!\n");
	 /*trace log*/
	 vrrp_state_trace(vsrv->profile,VRRP_STATE_TRANSFER,"TRANSFER LOOP","transfer over,start goto master");
     /*action*/	 
	 had_state_goto_master(vsrv,VRRP_STATE_TRANSFER);
	 return;
}

/*
 *******************************************************************************
 *had_set_link_auth_type()
 *
 *  DESCRIPTION:
 *		set link authentification type of the special link type(uplink or downlink).
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,				- vrrps
 *		unsigned int linkType,		- link type
 *		unsigned int authType		- authentification type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR		- failed
 *		VRRP_RETURN_CODE_OK			- success
 *
 *******************************************************************************
 */
unsigned int had_set_link_auth_type
(
	vrrp_rt *vsrv,
	unsigned int linkType,
	unsigned int authType
)
{
	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int i = 0;

	if (!vsrv) {
		vrrp_syslog_error("set link auth type null parameter error\n");
		return VRRP_RETURN_CODE_ERR;
	}

	switch (linkType)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) {
						vsrv->uplink_vif[i].auth_type = authType;
					}
				}
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) {
						vsrv->downlink_vif[i].auth_type = authType;
					}
				}
				break;
			}
		default :
			{
				vrrp_syslog_error("set link auth type not such link type %d\n", linkType);
				ret = VRRP_RETURN_CODE_ERR;
				break;
			}
	}

	vrrp_syslog_dbg("set %s auth type %s %s.\n",
					VRRP_LINK_TYPE_DESCANT(linkType),
					VRRP_AUTH_TYPE_DESCANT(authType),
					(VRRP_RETURN_CODE_OK == ret) ? "success" : "fail");
	
	return ret;
}

/*
 *******************************************************************************
 *had_link_auth_type_cmp()
 *
 *  DESCRIPTION:
 *		compare link authentification type of the special link type(uplink or downlink)
 *		with parameter authentification type.
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,				- vrrps
 *		unsigned int linkType,		- link type
 *		unsigned int authType		- authentification type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		0		- equal
 *		1		- not equal
 *
 *******************************************************************************
 */
unsigned int had_link_auth_type_cmp
(
	vrrp_rt *vsrv,
	unsigned int linkType,
	unsigned int authType
)
{
	unsigned int ret = 0;
	unsigned int i = 0;

	if (!vsrv) {
		vrrp_syslog_error("set link auth type null parameter error\n");
		return 1;
	}

	switch (linkType)
	{
		case VRRP_LINK_TYPE_UPLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if ((VRRP_LINK_SETTED == vsrv->uplink_vif[i].set_flg) &&
						(authType != vsrv->uplink_vif[i].auth_type))
					{
						ret = 1;
					}
				}
				break;
			}
		case VRRP_LINK_TYPE_DOWNLINK :
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if ((VRRP_LINK_SETTED == vsrv->downlink_vif[i].set_flg) &&
						(authType != vsrv->downlink_vif[i].auth_type))
					{
						ret = 1;
					}
				}
				break;
			}
		default :
			{
				vrrp_syslog_error("compare link auth type not such link type %d\n", linkType);
				ret = VRRP_RETURN_CODE_ERR;
				break;
			}
	}

	vrrp_syslog_dbg("compare %s auth type %s with %s.\n",
					VRRP_LINK_TYPE_DESCANT(linkType),
					(0 == ret) ? "equal" : "no equal",
					VRRP_AUTH_TYPE_DESCANT(authType));
	
	return ret;
}

/*
 * check if vif state machine retain or transits
 */
static int had_sm_check_vif_state_trans
(
	vrrp_rt* vsrv
)
{
	int transfer = 0;

	if (!vsrv) {
		vrrp_syslog_error("state machine check vif link down time parameter is null\n");
		return transfer;
	}

	/* transfer, 1: heartbeat 2:uplink 3:downlink */
	if ((NULL != global_ht_ifname) &&
		(0 != global_ht_ip) &&
		(!global_ht_state))
	{
        /*log trace*/
		vrrp_state_trace(vsrv->profile,VRRP_STATE_BACK,"BACK CHANGING","heartbeat link down,goto master");
		vrrp_syslog_info("state %s heartbeat %s ip %d.%d.%d.%d link down,goto master\n", \
						VRRP_STATE_DESCRIPTION(vsrv->state), global_ht_ifname, \
						(global_ht_ip>>24)&0xFF,(global_ht_ip>>16)&0xFF,(global_ht_ip>>8)&0xFF, global_ht_ip&0xFF);
		transfer = 1;
	}   
    else if (((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
			  VRRP_TIMER_EXPIRED(vsrv->uplink_ms_down_timer)) ||
			 ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
			  VRRP_TIMER_EXPIRED(vsrv->downlink_ms_down_timer)))
	{
        /*log trace*/
		if (VRRP_LINK_NO_SETTED != vsrv->uplink_flag &&
			VRRP_TIMER_EXPIRED(vsrv->uplink_ms_down_timer))
		{
			vrrp_state_trace(vsrv->profile,VRRP_STATE_BACK,
							"BACK CHANGING","uplink not receive packets in down timer,goto master");
		   	vrrp_syslog_info("state %s uplink not receive packets in master down timer(%d),goto master\n", \
							VRRP_STATE_DESCRIPTION(vsrv->state), vsrv->uplink_ms_down_timer);
			transfer = 2;
		}
		if (VRRP_LINK_NO_SETTED != vsrv->downlink_flag &&
			VRRP_TIMER_EXPIRED(vsrv->downlink_ms_down_timer))
		{
			vrrp_state_trace(vsrv->profile,VRRP_STATE_BACK,
							"BACK CHANGING","downlink not receive packets in down timer,goto master");
		   vrrp_syslog_info("state %s downlink not receive packets in master down timer(%d),goto master\n", \
		   					VRRP_STATE_DESCRIPTION(vsrv->state),vsrv->downlink_ms_down_timer);
			transfer = 3;
		}		
	}

	if (0 != transfer) {
		vrrp_syslog_event("%s, will goto master\n",
						(1 == transfer) ? "heartbeat link down" :
						((2 == transfer) ? "uplink not receive packets in down timer" :
						((3 == transfer) ? "downlink not receive packets in down timer" : "error!")));
	}
	return transfer;
}

static void had_state_back_loop
(
	vrrp_rt *vsrv
)
{
    int delay = 0;
#if 0
	static int stateFlg = 0;
#endif
	int transfer = 0;

	if (!vsrv) {
		vrrp_syslog_error("state backup loop parameter is null\n");
		return;
	}

	transfer = had_sm_check_vif_state_trans(vsrv);
	if(VRRP_SERVICE_DISABLE == service_enable[vsrv->profile]){
	   /*trace log*/
	   vrrp_state_trace(vsrv->profile,VRRP_STATE_BACK,"BACK LOOP","service not enabled,goto init");
	   vrrp_syslog_info("vrrp profile %d vrid %d service disabled, goto init!\n", vsrv->profile, vsrv->vrid);
       goto be_init;
	}

    if (((VRRP_LINK_NO_SETTED !=vsrv->uplink_flag) &&
		 (!vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_UPLINK))) ||
		((VRRP_LINK_NO_SETTED !=vsrv->downlink_flag) &&
		 (!vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_DOWNLINK))))
	{
           vrrp_syslog_dbg("vrrp %d uplinks or downlinks not up,next loop!\n", vsrv->vrid);
		   delay   = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
		   if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag){
			   VRRP_TIMER_SET( vsrv->uplink_ms_down_timer, delay );
			   VRRP_TIMER_CLR( vsrv->uplink_adver_timer );
		   }
		   if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag){
		   	   VRRP_TIMER_SET( vsrv->downlink_ms_down_timer, delay );		   
		   	   VRRP_TIMER_CLR( vsrv->downlink_adver_timer );
		   }
		   /*state trace*/
		   vrrp_state_trace(vsrv->profile,VRRP_STATE_BACK,"BACK LOOP","interface down,back loop");		   
		   return;
	}  

	/*only kill time,when get packet,the down timer will be reset,if in downtime it is not reset ,go to master*/
	if(transfer){
		 vrrp_syslog_dbg("vrrp %d goto master from back for ms down time expired!\n",vsrv->vrid);
		 /* send an advertisement */
		 if(!stateFlg){
             stateFlg = 1;
			 vrrp_syslog_dbg("first master down timer timeout or heartbeat link down, set stateflg 1\n");
			 if(VRRP_LINK_NO_SETTED !=  vsrv->uplink_flag){
			    VRRP_TIMER_SET(vsrv->uplink_ms_down_timer,vsrv->adver_int);
			 }
			 if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag){
				 VRRP_TIMER_SET(vsrv->downlink_ms_down_timer,vsrv->adver_int);
			 }
			 vrrp_syslog_dbg("set auth type to fail!\n");
			 if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag) {
				had_set_link_auth_type(vsrv, VRRP_LINK_TYPE_UPLINK, VRRP_AUTH_FAIL);
			 }
			 if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag) {
				had_set_link_auth_type(vsrv, VRRP_LINK_TYPE_DOWNLINK, VRRP_AUTH_FAIL);
			 }
			 /*log trace*/
			 vrrp_state_trace(vsrv->profile,VRRP_STATE_BACK,
			 				"BACK CHANG","send fail-to-receive packet to notify old master,then goto master");
			 
			 had_vrrp_vif_rcv_pck_ON(vsrv);
			 /*first notify to old master to force to fail!*/
			 vrrp_send_adv( vsrv, vsrv->priority );	
			 /*recover the type value*/
			 if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag) {
				had_set_link_auth_type(vsrv, VRRP_LINK_TYPE_UPLINK, VRRP_AUTH_PASS);;
			 }
			 if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag) {
				had_set_link_auth_type(vsrv, VRRP_LINK_TYPE_DOWNLINK, VRRP_AUTH_PASS);
			 }
			 return;
		 }         
		 had_state_goto_master( vsrv,VRRP_STATE_NONE);	
		 stateFlg = 0;
		 vrrp_syslog_dbg("recover auth tyep to pass!\n");
		 return;
	}
	/*back loop trace log*/	
	vrrp_state_trace(vsrv->profile,VRRP_STATE_BACK,"BACK LOOP","back loop");
	return;
be_init:	
    vrrp_syslog_dbg("vrrp %d start init loop!\n",vsrv->vrid);
	vsrv->state = VRRP_STATE_INIT;	
	hansi_write_hansi_state_to_file(vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));	
	/*trace log*/
	vrrp_state_trace(vsrv->profile,vsrv->state,"INIT SET","set init state");	
	return;
}

static void had_state_mast_loop
(
	vrrp_rt *vsrv
)
{
	unsigned int uplink_state = INTERFACE_DOWN;
	unsigned int downlink_state = INTERFACE_DOWN, vgateway_state = INTERFACE_DOWN;
	unsigned int l2_uplink_state = INTERFACE_DOWN;
	int delay = 0;
	if (!vsrv) {
		vrrp_syslog_error("state master loop paramter is null\n");
		return;
	}
	
	if (((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
		 (INTERFACE_UP != (uplink_state = vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_UPLINK)))) ||
		((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
		 (INTERFACE_UP != (downlink_state = vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_DOWNLINK)))) ||
		 ((VRRP_LINK_NO_SETTED != vsrv->l2_uplink_flag) &&
		 (INTERFACE_UP != (l2_uplink_state = vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_L2_UPLINK))))||
		((VRRP_VGATEWAY_TF_FLG_ON == vsrv->vgateway_tf_flag) &&
		 (VRRP_LINK_NO_SETTED != vsrv->vgateway_flag) &&
		 (INTERFACE_UP != (vgateway_state = vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_VGATEWAY)))))
	{
		vrrp_syslog_dbg("vrrp %d interface no ip or not up,goto disable!\n",vsrv->vrid);
		/*trace log*/
		if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
			(INTERFACE_UP != uplink_state))
		{
			vrrp_state_trace( vsrv->profile,VRRP_STATE_MAST,"MASTER CHANG","uplink down,goto disable");
				vrrp_syslog_info("vrrp %d state %s uplink down, goto disable!\n", \
								vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
		}
		if ((VRRP_LINK_NO_SETTED != vsrv->l2_uplink_flag) &&
			(INTERFACE_UP != l2_uplink_state))
		{
			vrrp_state_trace( vsrv->profile,VRRP_STATE_MAST,"MASTER CHANG","l2-uplink down,goto disable");
				vrrp_syslog_info("vrrp %d state %s l2-uplink down, goto disable!\n", \
								vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
		}
		if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
			(INTERFACE_UP != downlink_state))
		{
			vrrp_state_trace( vsrv->profile,VRRP_STATE_MAST,"MASTER CHANG","downlink down,goto disable");
			
				 vrrp_syslog_info("vrrp %d state %s downlink down, goto disable!\n", \
								 vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
		}
		#if 1
		/* add by jinpc@autelan.com,
		 * according to vgateway tf_flag and link state,
		 * set STATE MACHINE state.
		 */
		if ((VRRP_VGATEWAY_TF_FLG_ON == vsrv->vgateway_tf_flag) &&
			(VRRP_LINK_NO_SETTED != vsrv->vgateway_flag) &&
			(INTERFACE_UP != vgateway_state))
		{
			vrrp_state_trace(vsrv->profile, VRRP_STATE_MAST, "MASTER CHANG", "vgateway down, goto disable");
			
				 vrrp_syslog_info("vrrp %d state %s vgateway down, goto disable!\n", \
								 vsrv->profile, VRRP_STATE_DESCRIPTION(vsrv->state));
		}
		#endif
		had_state_goto_disable( vsrv,1,NULL,NULL);
		return;	
	}

	if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
		(VRRP_TIMER_EXPIRED(vsrv->uplink_adver_timer)) &&
		(INTERFACE_UP == uplink_state))
	{
        /*trace log*/
		vrrp_state_trace( vsrv->profile,VRRP_STATE_MAST,"MASTER SENDING","send uplink advertisement"); 			   		
		vrrp_send_uplink_adv( vsrv, vsrv->priority );
		VRRP_TIMER_SET(vsrv->uplink_adver_timer,vsrv->adver_int);
	}
	if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
		(VRRP_TIMER_EXPIRED(vsrv->downlink_adver_timer)) &&
		(INTERFACE_UP == downlink_state))
	{
		/*trace log*/
		vrrp_state_trace( vsrv->profile,VRRP_STATE_MAST,"MASTER SENDING","send downlink advertisement"); 			   		
		vrrp_send_downlink_adv(vsrv, vsrv->priority );
		VRRP_TIMER_SET(vsrv->downlink_adver_timer,vsrv->adver_int);
	}
	else{
		vrrp_state_trace(vsrv->profile,VRRP_STATE_MAST,"MASTER LOOP","master loop");
        return;
	}
	return;
}

static void had_state_disable_loop
(
	vrrp_rt* vsrv
)
{
	int delay = 0;   

	if (!vsrv) {
		vrrp_syslog_error("state disable loop paramter is null\n");
		return;
	}
   
	delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
	if ((!global_ht_state) ||
		(VRRP_LINK_NO_SETTED != vsrv->uplink_flag && !vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_UPLINK)) ||
		(VRRP_LINK_NO_SETTED != vsrv->l2_uplink_flag && !vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_L2_UPLINK)) ||
		(VRRP_LINK_NO_SETTED != vsrv->downlink_flag && !vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_DOWNLINK)) ||
		((VRRP_VGATEWAY_TF_FLG_ON == vsrv->vgateway_tf_flag) &&
		 (VRRP_LINK_NO_SETTED != vsrv->vgateway_flag) && (!vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_VGATEWAY))))
	{
		vrrp_state_trace(vsrv->profile,vsrv->state,"DISABLE LOOP","disable loop");
		if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag)
			VRRP_TIMER_SET( vsrv->uplink_ms_down_timer, delay );
		if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag)
			VRRP_TIMER_SET( vsrv->downlink_ms_down_timer, delay ); 
		return;
	}
	else if((VRRP_LINK_NO_SETTED != vsrv->uplink_flag && (VRRP_TIMER_EXPIRED(vsrv->uplink_ms_down_timer))) ||
			(VRRP_LINK_NO_SETTED != vsrv->downlink_flag && (VRRP_TIMER_EXPIRED(vsrv->downlink_ms_down_timer))))
	{
		vrrp_state_trace(vsrv->profile,vsrv->state,"GOTO MASTER","linkup,but not packet received,goto master");
		/*link up but not receive packets in ms down timer,goto master*/
	   vrrp_syslog_info("state %s master down timer(u-%d d-%d) expires(uplink or downlink), goto master!\n", \
					   VRRP_STATE_DESCRIPTION(vsrv->state), vsrv->uplink_ms_down_timer, vsrv->downlink_ms_down_timer);
		had_state_goto_master(vsrv,VRRP_STATE_NONE);
		return;
	}

	return;
}

void had_set_heartbeatlink_sock
(
  void
)
{
	int i = 1;
	vrrp_rt* vsrv = NULL;
	
	for (; i < MAX_HANSI_PROFILE; i++) {
       vsrv = had_check_if_exist(i);
	   if(NULL != vsrv){
	   	    if(0 != vsrv->sockfd){
               close(vsrv->sockfd);
			}
	        vsrv->sockfd = had_SocketInit2();		
			if(vsrv->sockfd <= 0)
			{
				vrrp_syslog_error("%s,%d,err vsrv->sockfd:%d.",__func__,__LINE__,vsrv->sockfd);
				return ;
			}
			struct sockaddr_ll sll;
			memset( &sll, 0, sizeof(sll) );
			sll.sll_family = AF_PACKET;
			sll.sll_ifindex = had_ifname_to_idx(global_ht_ifname);
			sll.sll_protocol = htons(ETH_P_IP);
			sll.sll_pkttype = PACKET_OTHERHOST;
			if( bind(vsrv->sockfd, (struct sockaddr *) &sll, sizeof(sll)) == -1 ) {
			   int err = errno;
			   vrrp_syslog_error("bind interface index %d to socket %d failed! %s\n",
			   					sll.sll_ifindex,vsrv->sockfd, strerror(err));
			   close(vsrv->sockfd);
			   return;
			}
			vrrp_syslog_dbg("vrrp %d bind heartbeat link sock fd %d \n",vsrv->vrid,vsrv->sockfd);	
			had_LIST_UPDATE(vsrv,vsrv->vrid);
	   }
	}
	return;
}

/****************************************************************
 NAME	: had_open_sock				00/02/07 12:40:00
 AIM	: open the socket and join the multicast group.
 REMARK	:
****************************************************************/
int had_open_sock
(
	vrrp_rt *vsrv
)
{
	char* uplink_ifname = NULL,*downlink_ifname = NULL;
	int i = 0;

	if (!vsrv) {
		vrrp_syslog_error("open socket faild, null parameter error\n");
		return -1;
	}

/*send sock*/
	sendsock = socket(PF_PACKET, SOCK_PACKET,0x300); /* 0x300 is magic */
// WORK:
	if( sendsock < 0 ){
		perror( "socket" );
		return -1;
	}

	/* for heartbeat */
	if(NULL != global_ht_ifname && 0 != global_ht_ip){
        vsrv->sockfd = had_SocketInit2();		
		if(vsrv->sockfd <= 0){
			vrrp_syslog_error("%s,%d,had_SocketInit2 error\n",__func__,__LINE__);
			return -1;
		}
		struct sockaddr_ll sll;
		memset( &sll, 0, sizeof(sll) );
		sll.sll_family = AF_PACKET;
		sll.sll_ifindex = had_ifname_to_idx(global_ht_ifname);
		sll.sll_protocol = htons(ETH_P_IP);
		sll.sll_pkttype = PACKET_OTHERHOST;
		if( bind(vsrv->sockfd, (struct sockaddr *) &sll, sizeof(sll)) == -1 ) {
		   int err = errno;
		   vrrp_syslog_error("bind interface index %d to socket %d failed! %s\n",
		   					sll.sll_ifindex,vsrv->sockfd, strerror(err));
		   close(vsrv->sockfd);
		   return -1;
		}
		vrrp_syslog_dbg("vrrp %d bind sock fd %d \n",vsrv->vrid,vsrv->sockfd);						
	}

	/* for uplink */
    if (VRRP_LINK_SETTED == vsrv->uplink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED != vsrv->uplink_vif[i].set_flg)
			{
				/*
				vrrp_syslog_dbg("uplink_vif[%d] not setted\n", i);
				*/
				continue;
			}
			uplink_ifname = NULL;
			uplink_ifname = vsrv->uplink_vif[i].ifname;
			vsrv->uplink_fd[i] = had_SocketInit2();
			if (vsrv->uplink_fd[i] < 0) {
			vrrp_syslog_error("initialize uplink socket error\n");
				return -1;
			} 
			else{/*bind to the interface*/
				struct sockaddr_ll sll;
				memset( &sll, 0, sizeof(sll) );
				sll.sll_family = AF_PACKET;
				sll.sll_ifindex = had_ifname_to_idx(uplink_ifname);
				sll.sll_protocol = htons(ETH_P_IP);
				sll.sll_pkttype = PACKET_OTHERHOST;
				if( bind(vsrv->uplink_fd[i], (struct sockaddr *) &sll, sizeof(sll)) == -1 ) {
				   int err = errno;
			   vrrp_syslog_error("bind uplink interface %s index %d to socket %d failed! %s\n",
				   				uplink_ifname, sll.sll_ifindex,vsrv->uplink_fd[i], strerror(err));
				   close(vsrv->uplink_fd[i]);
				   return -1;
				}
				vrrp_syslog_dbg("vrrp %d bind uplink fd %d \n",vsrv->vrid,vsrv->uplink_fd[i]);						
			}
		}
	}

	/* for downlink */
	if (VRRP_LINK_SETTED == vsrv->downlink_flag)
	{
		for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
		{
			if (VRRP_LINK_SETTED != vsrv->downlink_vif[i].set_flg)
			{
				vrrp_syslog_dbg("uplink_vif[%d] not setted\n", i);
				continue;
			}
			downlink_ifname = NULL;
	        downlink_ifname = vsrv->downlink_vif[i].ifname;
			vsrv->downlink_fd[i] = had_SocketInit2();
			if( vsrv->downlink_fd[i] < 0 ){
				int err = errno;
			vrrp_syslog_error("initialize downlink socket error\n");
				return -1;
			}
			
			else{/*bind to the interface*/
				struct sockaddr_ll sll;
				memset( &sll, 0, sizeof(sll) );
				sll.sll_family = AF_PACKET;
				sll.sll_ifindex = had_ifname_to_idx(downlink_ifname);
				sll.sll_protocol = htons(ETH_P_IP);
				sll.sll_pkttype = PACKET_OTHERHOST;
				if( bind(vsrv->downlink_fd[i], (struct sockaddr *) &sll, sizeof(sll)) == -1 ) {
				   int err = errno;
			   vrrp_syslog_error("bind downlink interface %s index %d to socket %d failed! %s\n",
				   				downlink_ifname, sll.sll_ifindex,vsrv->downlink_fd[i], strerror(err));
				   return -1;
				}
				vrrp_syslog_dbg("vrrp %d bind downlink fd %d \n",vsrv->vrid,vsrv->downlink_fd[i]);
			}
		}
	}	
	/*zhangcl modified for dead lock*/
	had_LIST_UPDATE(vsrv,vsrv->vrid);
	
	return 0;
}

int had_init_trace(int profile){
   struct state_trace* log = NULL;
   char *action = NULL,*step = NULL;
   log = (struct state_trace*)malloc(sizeof(struct state_trace));
   if(NULL == log){
   	  vrrp_syslog_error("profile %d initialize trace log failed:out of memory!\n",profile);
	  return VRRP_RETURN_CODE_MALLOC_FAILED;
   }
   memset(log,0,sizeof(struct state_trace));
   action = (char *)malloc(sizeof(char)*128);
   if(NULL == action){
   	 vrrp_syslog_error("profile %d initialize trace log action failed:out of memory!\n",profile);
     free(log);
	 return VRRP_RETURN_CODE_MALLOC_FAILED;
   }  
   memset(action,0,sizeof(char)*128);
   step = (char *)malloc(sizeof(char)*128);
   if(NULL == step){
   	 vrrp_syslog_error("profile %d initialize trace log step failed:out of memory!\n",profile);
     free(log);
	 free(action);
	 return VRRP_RETURN_CODE_MALLOC_FAILED;
   }  
   memset(step,0,sizeof(char)*128);
   TRACE_LOG[profile] = log;
   log->profile = profile;
   log->state = VRRP_STATE_INIT;
   log->action = action;
   log->step = step;
   return VRRP_RETURN_CODE_OK;
}

void hansi_init(void)
{
    int i = 0;
	unsigned char tmpMacStr[18] = {0};
	vrrp_syslog_dbg("start hansi init...\n");
	g_hansi = (hansi_s**)malloc(sizeof(void*) * MAX_HANSI_PROFILE);
	if(NULL == g_hansi){
       vrrp_syslog_error("alloc memory for hansi list failed!\n");\
	   return;
	}
	global_enable = 0;
	for(;i < MAX_HANSI_PROFILE;i++){
       g_hansi[i] = NULL;
	   service_enable[i] = VRRP_SERVICE_DISABLE;
	}
	sock_list = had_LIST_CREATE();
	/*trace link add*/
	TRACE_LOG = (struct state_trace**)malloc(sizeof(struct state_trace)*MAX_HANSI_PROFILE);
	if(NULL == TRACE_LOG){
       vrrp_syslog_error("alloc memory for trace log failed!\n");
	   return;
	}	
	memset(TRACE_LOG, 0, sizeof(struct state_trace)*MAX_HANSI_PROFILE);	
	if(!hansi_read_string_from_file(tmpMacStr, VRRP_DEVINFO_MAC_PATH, VRRP_DEVINFO_MAC_FILENAME, VRRP_DEVINFO_MAC_LEN)){
		hansi_mac_addr_parse(tmpMacStr, vrrp_global_mac);
		vrrp_syslog_info("basemac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",\
					vrrp_global_mac[0],vrrp_global_mac[1],vrrp_global_mac[2],\
					vrrp_global_mac[3],vrrp_global_mac[4],vrrp_global_mac[5]);
	}
}

int had_get_runing_hansi
(
   void
)
{
    int i = 0,count = 0;
	vrrp_rt* vrrp = NULL;
	for (i = 0; i < MAX_HANSI_PROFILE; i++) {
       vrrp = had_check_if_exist(i);
	   if(NULL != vrrp){
          count++;
	   }
	}
	return count;
}

int had_ipv6_start
(
   unsigned int profile,
   unsigned int priority,
   unsigned int link_local,
   char*  uplink_if,
   struct iaddr  *uplink_ip,
   unsigned int   uplink_mask,
   char*  downlink_if,
   struct iaddr *downlink_ip,
   unsigned int  downlink_mask
)

{
   int ret = 0;
//   unsigned int ipaddr = 0;
   vrrp_rt*	vrrp = NULL;
   hansi_s *hansiNode = had_get_profile_node(profile);;
   int vrid = 0;
   struct in6_addr real_ipv6;
   int realip_index = -1;
   	unsigned int	addr[1024] = {0};
	int naddr = 0;
	int i = 0;
	int ifinit=0;
	vrid = profile;
	if (VRRP_IS_BAD_VID(vrid))
	{
		vrrp_syslog_dbg("bad vrid %d, valid range is [1~255]!\n", vrid);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}

  if(NULL != uplink_if){
	vrrp_syslog_dbg(
		"***********had_ipv6_start: get arguments  uplink ifname = %s \
		uplink ipv6 addr = "NIP6QUAD_FMT" \
		prefix_length_up = %d \n",
		uplink_if,
		NIP6QUAD(uplink_ip->iabuf),
		uplink_mask
	);
  }
  if(NULL != downlink_if)
	vrrp_syslog_dbg(
		"***********had_ipv6_start: get arguments downlink ifname = %s \
		downlink ipv6 addr = "NIP6QUAD_FMT" \
        prefix_length_down = %d \n",
		downlink_if,
		NIP6QUAD(downlink_ip->iabuf),
		downlink_mask
	);
	/*check interface valid or error*/
	if(NULL != uplink_if){

		memset(&real_ipv6,0,sizeof(real_ipv6));
		ret = had_ifname_to_ipv6(uplink_if, &real_ipv6);// no ip
		if (VRRP_RETURN_CODE_OK != ret) {
			vrrp_syslog_error("ifname %s no interface found!\n",uplink_if);
			return VRRP_RETURN_CODE_BAD_PARAM;
		}
		naddr = had_ipv6addr_list( had_ifname_to_idx(uplink_if),addr,sizeof(addr)/sizeof(addr[0]));
		for( i = 0; i < naddr; i+=4 ){
			if(!memcmp(uplink_ip->iabuf, &(addr[i]),sizeof(struct in6_addr))){// real ip == vir ip
				return VRRP_RETURN_CODE_BAD_PARAM;
			}
		}
	}
#if 1
	if(NULL != downlink_if){
		//vrrp_syslog_info("***** %s,%d *****\n",__func__,__LINE__);
		memset(&real_ipv6,0,sizeof(real_ipv6));
		ret = had_ifname_to_ipv6(downlink_if, &real_ipv6);//no ip
		if (VRRP_RETURN_CODE_OK != ret) {
			vrrp_syslog_dbg("ifname %s no interface found!\n",downlink_if);
			return VRRP_RETURN_CODE_BAD_PARAM;
		}
		naddr = had_ipv6addr_list( had_ifname_to_idx(downlink_if),addr,sizeof(addr)/sizeof(addr[0]) );
		for( i = 0; i < naddr; i+=4 ){
			if(!memcmp(downlink_ip->iabuf, &(addr[i]),sizeof(struct in6_addr))){//real ip == vir ip
				return VRRP_RETURN_CODE_BAD_PARAM;
			}
		}
	}
#endif
	//vrrp_syslog_info("%s,%d\n",__func__,__LINE__);
	/*end*/
   if(NULL != hansiNode){
   	   vrrp_syslog_dbg("hansi instance %d exist,remove exist vrrp first!\n", profile);
	   /*check the vrrp instance if exist already(0 means exist),if not,malloc */
	   vrrp = hansiNode->vlist;
	   if(NULL != vrrp){
		 vrrp_syslog_dbg("first to delete old vrrp!\n");
		 if( NULL != uplink_if 
		 		&& strlen(vrrp->uplink_vif[0].ifname) != 0 
		 		&& memcmp(vrrp->uplink_vif[0].ifname,uplink_if,strlen(uplink_if))!=0){
		 		return VRRP_RETURN_CODE_BAD_PARAM;
		 }
		 
		 if( NULL != downlink_if 
		 		&& strlen(vrrp->downlink_vif[0].ifname) != 0 
		 		&& memcmp(vrrp->downlink_vif[0].ifname,downlink_if,strlen(downlink_if))!=0){
		 		return VRRP_RETURN_CODE_BAD_PARAM;
		 }
		 //ret = had_end(profile);	
	   }	   
   }

   if(NULL == vrrp ){

		vrrp_syslog_dbg("malloc new vrrp %d!\n",vrid);
		vrrp = ( vrrp_rt*)malloc(sizeof(vrrp_rt));
		if(NULL == vrrp){
			return VRRP_RETURN_CODE_MALLOC_FAILED;
		}
		memset(vrrp, 0, sizeof(vrrp_rt));
#if 0
		/*get sysmac as intf stored mac*/
		had_get_sys_mac(sysmac);
#endif

		had_init_virtual_srv(vrrp);
		ifinit = 1;
   	}
   /*get if name*/
   if(NULL != uplink_if){
   		if(strlen(vrrp->uplink_vif[0].ifname) == 0 ){
		   memset(vrrp->uplink_vif[0].ifname,0,MAX_IFNAME_LEN);
	       memcpy(vrrp->uplink_vif[0].ifname,uplink_if,strlen(uplink_if));	
		   
		   vrrp->uplink_vif[0].ifindex = had_ifname_to_idx(vrrp->uplink_vif[0].ifname);
		   
		   memcpy(vrrp->uplink_vif[0].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
		   
		   /*init aute_type*/
		   vrrp->uplink_vif[0].auth_type = VRRP_AUTH_PASS;
		   vrrp->uplink_flag = 1;
		   vrrp->uplink_vif[0].linkstate = vrrp_get_ifname_linkstate(vrrp->uplink_vif[0].ifname);
		   //vrrp->uplink_vif[0].set_flg = VRRP_LINK_SETTED;
   		}
	   /*get uplink if ip addr*/

       /*check if appoint real ip*/
	   if (VRRP_RETURN_CODE_IF_NOT_EXIST == 
			had_get_link_realip_index_by_ifname(hansiNode, uplink_if, VRRP_LINK_TYPE_UPLINK, &realip_index))
	   {
		  memset(&real_ipv6,0,sizeof(real_ipv6));
		  ret = had_ifname_to_ipv6(uplink_if, &real_ipv6);
		  if (VRRP_RETURN_CODE_OK != ret) {
			   vrrp_syslog_dbg("ifname %s no interface found!\n",uplink_if);
			   if(1 == ifinit)
				    free(vrrp);
			   return VRRP_RETURN_CODE_BAD_PARAM;
		  }
	      memcpy(&vrrp->uplink_vif[0].ipv6_addr, &real_ipv6,sizeof(real_ipv6));
	   }
	   else{
          memcpy(&vrrp->uplink_vif[0].ipv6_addr,
		  	&hansiNode->uplink_real_ip[realip_index].real_ipv6,
		  	sizeof(struct in6_addr));
	   }
	   vrrp_syslog_dbg("ipv6 addr = "NIP6QUAD_FMT" \n",NIP6QUAD(uplink_ip->iabuf)); //interface virtual link local address
	   
	   /*get uplink the hwaddr  */
	   /* replace with vrrp_global_mac
	   if( had_hwaddr_get(vrrp->uplink_vif[0].ifname,vrrp->uplink_vif[0].hwaddr, sizeof(vrrp->uplink_vif[0].hwaddr)) ){
			vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n",uplink_if );
			free(vrrp);
			return VRRP_RETURN_CODE_BAD_PARAM;
	   }
	   */
	   //memcpy(vrrp->uplink_vif[0].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
	   /*
	   memcpy(vrrp->uplink_vif.hwaddr,sysmac,6);
	   */
	   /*uplink vritual ip*/
	  /* ipaddr = inet_addr(uplink_ip);*/
	   //ipaddr = uplink_ip;
	   vrrp->uplink_ipv6_naddr = 0;
#if 1
	   /*store virtual link local address*/
	   if(link_local){
    	   had_cfg_add_uplink_ipv6addr(vrrp,uplink_ip,uplink_mask, 0,1);
    	   vrrp_syslog_dbg(
    			"********had_ipv6_start: store uplink virtual link local address "NIP6QUAD_FMT" \n",
    			NIP6QUAD(vrrp->uplink_local_ipv6_vaddr[0].sin6_addr.s6_addr)
    			);
       }
	   else if(uplink_ip && !ipv6_addr_eq_null(uplink_ip))  /*store virtual ipv6 address*/
	   {
	   	    if(ipv6_addr_eq_null(vrrp->uplink_local_ipv6_vaddr[0].sin6_addr.s6_addr)){
				vrrp_syslog_error("uplink %s : error, set link-local address firstly!\n",uplink_if);
				if(1 == ifinit)
				    free(vrrp);
				return VRRP_RETURN_CODE_VIP_FIRST_LINKLOCAL;
			}
    	   had_cfg_add_uplink_ipv6addr(vrrp,uplink_ip,uplink_mask, 0,0);
    	  vrrp_syslog_dbg(
    			"********had_ipv6_start: store uplink virtual ipv6 address "NIP6QUAD_FMT" \n",
    			NIP6QUAD(vrrp->uplink_ipv6_vaddr[1].sin6_addr.s6_addr)
    			);
       }
	   else 
            vrrp_syslog_error("Must first configure the virtual link local address\n");
	   	
#endif
   }
   else{
   	    ;
	   //vrrp->uplink_flag = 0;

   }
   if(NULL != downlink_if){
	   memset(vrrp->downlink_vif[0].ifname,0,MAX_IFNAME_LEN);
       memcpy(vrrp->downlink_vif[0].ifname,downlink_if,strlen(downlink_if));
	   /*downlink*/
	   vrrp_syslog_dbg("get downlink ifname %s info::\n",vrrp->downlink_vif[0].ifname);
       /*real ip if appointed*/
	   realip_index = -1;
	   if (VRRP_RETURN_CODE_IF_NOT_EXIST == 
			had_get_link_realip_index_by_ifname(hansiNode, downlink_if, VRRP_LINK_TYPE_DOWNLINK, &realip_index))
	   	{
		   memset(&real_ipv6,0,sizeof(real_ipv6));
		   ret = had_ifname_to_ipv6(downlink_if, &real_ipv6);
		   if (VRRP_RETURN_CODE_OK != ret) {
				vrrp_syslog_error("ifname %s no interface found!\n",downlink_if);
				if(1 == ifinit)
				    free(vrrp);
				return VRRP_RETURN_CODE_BAD_PARAM;
		   }
		   memcpy(&vrrp->downlink_vif[0].ipv6_addr,&real_ipv6,sizeof(real_ipv6));
	   }
	   else{
	   	   memcpy(&vrrp->downlink_vif[0].ipv6_addr,
		   	&hansiNode->downlink_real_ip[realip_index].real_ipv6,
		   	sizeof(struct in6_addr));    	    
	   }
	   // interface real link local address
       vrrp_syslog_info("if = %s  ipv6 addr = "NIP6QUAD_FMT" \n",downlink_if,NIP6QUAD(vrrp->downlink_vif[0].ipv6_addr));

	   vrrp->downlink_vif[0].ifindex = had_ifname_to_idx(vrrp->downlink_vif[0].ifname);

	   /* replace with vrrp_global_mac	  
	   if( had_hwaddr_get(vrrp->downlink_vif[0].ifname,vrrp->downlink_vif[0].hwaddr, sizeof(vrrp->downlink_vif[0].hwaddr)) ){
			vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n",downlink_if );
			free(vrrp);
			return VRRP_RETURN_CODE_BAD_PARAM;
	   }	    
	   */
	   memcpy(vrrp->downlink_vif[0].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
	   /*
	    memcpy(vrrp->downlink_vif.hwaddr,sysmac,6);
	    */
	   /*downlink vritual ip*/
	   /*ipaddr = inet_addr(downlink_ip);*/
	   //ipaddr = downlink_ip;
#if 1
	   /*store virtual link local address*/
	   if(link_local){
    	   had_cfg_add_downlink_ipv6addr(vrrp,downlink_ip,downlink_mask, 0, 1);
    	   vrrp_syslog_info(
    			"store uplink virtual link local address "NIP6QUAD_FMT" \n",
    			NIP6QUAD(downlink_ip->iabuf)
    			);
       }
	   else if(downlink_ip && !ipv6_addr_eq_null(downlink_ip))  /*store virtual ipv6 address*/
	   {
		   if(ipv6_addr_eq_null(vrrp->downlink_local_ipv6_vaddr[0].sin6_addr.s6_addr)){
				vrrp_syslog_error("downlink %s : error, set link-local address firstly!\n",downlink_if);
                if(1 == ifinit)
					free(vrrp);
			   return VRRP_RETURN_CODE_VIP_FIRST_LINKLOCAL;
	   	    }
    	   had_cfg_add_downlink_ipv6addr(vrrp,downlink_ip,downlink_mask, 0,0);
    	   vrrp_syslog_info(
    			"       store uplink virtual ipv6 address "NIP6QUAD_FMT" \n",
    			NIP6QUAD(downlink_ip->iabuf)
    			);
       }
	   else 
            vrrp_syslog_error("Must first configure the virtual link local address\n");
	   	
#endif

	   /*init aute_type*/
	   vrrp->downlink_vif[0].auth_type = VRRP_AUTH_PASS;
	   vrrp->downlink_flag = 1;
	   vrrp->downlink_vif[0].linkstate = vrrp_get_ifname_linkstate(vrrp->downlink_vif[0].ifname);
	   //vrrp->downlink_vif[0].set_flg = VRRP_LINK_SETTED;
   }
   else{
   	;
	  // vrrp->downlink_flag = 0;
   }
	if(ifinit == 1 ){
	   vrrp->vrid = vrid;
	   vrrp->profile = profile;
	   vrrp_syslog_dbg("init vrrp vrid to %d\n",vrrp->vrid);
	   vrrp->no_vmac = 1; 
	   /*set priority*/
	   if(0 == link_local){
	   	  vrrp->priority = priority;
	   }  
	   vrrp->vrrp_trap_sw = VRRP_ON;
	   /* check if the minimal configuration has been done */
	   if( had_chk_min_cfg(vrrp) ){
		  free(vrrp);
		  return VRRP_RETURN_CODE_BAD_PARAM;
	   }

#if 0
		/* shoud move to service enable */
	   if( had_open_sock(vrrp) ){/*set socket to add to multicast ip group*/
			free(vrrp);
			return VRRP_RETURN_CODE_ERR;
	   }
#endif
	   /* the init is completed */
	   vrrp->initF = 1;
	   vrrp->next = NULL;
	   

	   if(NULL == hansiNode){
	      ret = had_profile_create(profile,vrid);
		  hansiNode = had_get_profile_node(profile);
		  if(hansiNode == NULL){
		  	vrrp_syslog_error("profile is:%d!\n",profile);
		  	return VRRP_RETURN_CODE_BAD_PARAM;
		  }
	   }
	   /*insert into the global list*/
	   if(NULL == hansiNode->vlist){
		   hansiNode->vlist = vrrp;
		   hansiNode->vrid = vrid;
	   }
	   /*log trace */
	   had_init_trace(profile);
   }

   return  VRRP_RETURN_CODE_OK;
}

int had_start
(
   unsigned int profile,
   unsigned int priority,
   char*  uplink_if,
   unsigned long  uplink_ip,
   unsigned int   uplink_mask,
   char*  downlink_if,
   unsigned long downlink_ip,
   unsigned int  downlink_mask
)
{
   int ret = 0;
   unsigned int ipaddr = 0;
   vrrp_rt*	vrrp = NULL;
   hansi_s *hansiNode = had_get_profile_node(profile);;
   int vrid = 0;
   unsigned int real_ip = 0;
   int realip_index = -1;
   	unsigned int	addr[1024] = {0};
	int naddr = 0;
	int i = 0;
	vrid = profile;
	if (VRRP_IS_BAD_VID(vrid))
	{
		vrrp_syslog_dbg("bad vrid %d, valid range is [1~255]!\n", vrid);
		return VRRP_RETURN_CODE_BAD_PARAM;
	}
	/*check interface valid or error*/
	if(NULL != uplink_if){
		real_ip = 0;
		vrrp_syslog_info("%s,%d\n",__func__,__LINE__);
		ret = had_ifname_to_ip(uplink_if, &real_ip);// no ip
		if (VRRP_RETURN_CODE_OK != ret) {
			vrrp_syslog_error("ifname %s no interface found!\n",uplink_if);
			return VRRP_RETURN_CODE_BAD_PARAM;
		}
		naddr = had_ipaddr_list( had_ifname_to_idx(uplink_if),addr,sizeof(addr)/sizeof(addr[0]));
		for( i = 0; i < naddr; i++ ){
			if(uplink_ip == addr[i]){// real ip == vir ip
				return VRRP_RETURN_CODE_BAD_PARAM;
			}
		}
	}
	if(NULL != downlink_if){
		real_ip = 0;
		vrrp_syslog_info("%s,%d\n",__func__,__LINE__);
		ret = had_ifname_to_ip(downlink_if, &real_ip);//no ip
		if (VRRP_RETURN_CODE_OK != ret) {
			vrrp_syslog_dbg("ifname %s no interface found!\n",downlink_if);
			return VRRP_RETURN_CODE_BAD_PARAM;
		}
		naddr = had_ipaddr_list( had_ifname_to_idx(downlink_if),addr,sizeof(addr)/sizeof(addr[0]) );
		for( i = 0; i < naddr; i++ ){
			if(downlink_ip == addr[i]){//real ip == vir ip
				return VRRP_RETURN_CODE_BAD_PARAM;
			}
		}
	}
	/*end*/
   if(NULL != hansiNode){
   	   vrrp_syslog_dbg("hansi instance %d exist,remove exist vrrp first!\n", profile);
	   /*check the vrrp instance if exist already(0 means exist),if not,malloc */
	   vrrp = hansiNode->vlist;
	   if(NULL != vrrp){
		 vrrp_syslog_dbg("first to delete old vrrp!\n");
		 ret = had_end(profile);	
	   }	   
   }

   vrrp_syslog_dbg("malloc new vrrp %d!\n",vrid);
   vrrp = ( vrrp_rt*)malloc(sizeof(vrrp_rt));
   if(NULL == vrrp){
   	 return VRRP_RETURN_CODE_MALLOC_FAILED;
   }
   memset(vrrp, 0, sizeof(vrrp_rt));
   #if 0
   /*get sysmac as intf stored mac*/
   had_get_sys_mac(sysmac);
   #endif
   
   had_init_virtual_srv(vrrp);

   /*
    if(((NULL == uplink_if)||(NULL == downlink_if))||((NULL == uplink_ip)||(NULL == downlink_ip))){
        return VRRP_RETURN_CODE_BAD_PARAM;
    }
   */
   /*get if name*/
   if(NULL != uplink_if){
	   memset(vrrp->uplink_vif[0].ifname,0,MAX_IFNAME_LEN);
       memcpy(vrrp->uplink_vif[0].ifname,uplink_if,strlen(uplink_if));	
	   /*get uplink if ip addr*/
	   vrrp_syslog_dbg("get uplink ifname %s info::\n",vrrp->uplink_vif[0].ifname);
       /*check if appoint real ip*/
	   if (VRRP_RETURN_CODE_IF_NOT_EXIST == 
			had_get_link_realip_index_by_ifname(hansiNode, uplink_if, VRRP_LINK_TYPE_UPLINK, &realip_index))
	   {
		  real_ip = 0;
		  ret = had_ifname_to_ip(uplink_if, &real_ip);
		  if (VRRP_RETURN_CODE_OK != ret) {
			   vrrp_syslog_dbg("ifname %s no interface found!\n",uplink_if);
			   free(vrrp);
			   return VRRP_RETURN_CODE_BAD_PARAM;
		  }
	      vrrp->uplink_vif[0].ipaddr	= real_ip;
	   }
	   else{
          vrrp->uplink_vif[0].ipaddr	= hansiNode->uplink_real_ip[realip_index].real_ip;
	   }
  
	   vrrp->uplink_vif[0].ifindex = had_ifname_to_idx(vrrp->uplink_vif[0].ifname);
	   
	   vrrp_syslog_dbg("ip 0x%x\n",vrrp->uplink_vif[0].ipaddr);
	   /*get uplink the hwaddr  */
	   /* replace with vrrp_global_mac
	   if( had_hwaddr_get(vrrp->uplink_vif[0].ifname,vrrp->uplink_vif[0].hwaddr, sizeof(vrrp->uplink_vif[0].hwaddr)) ){
			vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n",uplink_if );
			free(vrrp);
			return VRRP_RETURN_CODE_BAD_PARAM;
	   }
	   */
	   memcpy(vrrp->uplink_vif[0].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
	   /*
	   memcpy(vrrp->uplink_vif.hwaddr,sysmac,6);
	   */
	   /*uplink vritual ip*/
	  /* ipaddr = inet_addr(uplink_ip);*/
	   ipaddr = uplink_ip;
	   vrrp->uplink_naddr = 0;
	   had_cfg_add_uplink_ipaddr(vrrp,ipaddr,uplink_mask, 0);
	   vrrp_syslog_dbg("store uplink virtual ip %02x\n",ntohl(ipaddr));
	   
	   /*init aute_type*/
	   vrrp->uplink_vif[0].auth_type = VRRP_AUTH_PASS;
	   vrrp->uplink_flag = 1;
	   vrrp->uplink_vif[0].linkstate = vrrp_get_ifname_linkstate(vrrp->uplink_vif[0].ifname);
	   vrrp->uplink_vif[0].set_flg = VRRP_LINK_SETTED;
   }
   else{
	   vrrp->uplink_flag = 0;

   }
   if(NULL != downlink_if){
	   memset(vrrp->downlink_vif[0].ifname,0,MAX_IFNAME_LEN);
       memcpy(vrrp->downlink_vif[0].ifname,downlink_if,strlen(downlink_if));
	   /*downlink*/
	   vrrp_syslog_dbg("get downlink ifname %s info::\n",vrrp->downlink_vif[0].ifname);
       /*real ip if appointed*/
	   realip_index = -1;
	   if (VRRP_RETURN_CODE_IF_NOT_EXIST == 
			had_get_link_realip_index_by_ifname(hansiNode, downlink_if, VRRP_LINK_TYPE_DOWNLINK, &realip_index))
	   	{
		   real_ip = 0;
		   ret = had_ifname_to_ip(downlink_if, &real_ip);
		   if (VRRP_RETURN_CODE_OK != ret) {
				vrrp_syslog_dbg("ifname %s no interface found!\n",downlink_if);
				free(vrrp);
				return VRRP_RETURN_CODE_BAD_PARAM;
		   }
		   vrrp->downlink_vif[0].ipaddr	 = real_ip;
	   }
	   else{
		   vrrp->downlink_vif[0].ipaddr = hansiNode->downlink_real_ip[realip_index].real_ip;
	   }

	   vrrp_syslog_dbg("interface %s ip addr %x\n",downlink_if,vrrp->downlink_vif[0].ipaddr);
	   vrrp->downlink_vif[0].ifindex = had_ifname_to_idx(vrrp->downlink_vif[0].ifname);
	   vrrp_syslog_dbg("ip 0x%x\n",vrrp->downlink_vif[0].ipaddr);
	   /* replace with vrrp_global_mac	  
	   if( had_hwaddr_get(vrrp->downlink_vif[0].ifname,vrrp->downlink_vif[0].hwaddr, sizeof(vrrp->downlink_vif[0].hwaddr)) ){
			vrrp_syslog_dbg("Can't read the hwaddr on interface %s!\n",downlink_if );
			free(vrrp);
			return VRRP_RETURN_CODE_BAD_PARAM;
	   }	    
	   */
	   memcpy(vrrp->downlink_vif[0].hwaddr, vrrp_global_mac, sizeof(vrrp_global_mac));
	   /*
	    memcpy(vrrp->downlink_vif.hwaddr,sysmac,6);
	    */
	   /*downlink vritual ip*/
	   /*ipaddr = inet_addr(downlink_ip);*/
	   ipaddr = downlink_ip;
	   had_cfg_add_downlink_ipaddr(vrrp,ipaddr,downlink_mask, 0);	 
	   vrrp_syslog_dbg("store downlink virtual ip %02x\n",ntohl(ipaddr));	 

	   /*init aute_type*/
	   vrrp->downlink_vif[0].auth_type = VRRP_AUTH_PASS;
	   vrrp->downlink_flag = 1;
	   vrrp->downlink_vif[0].linkstate = vrrp_get_ifname_linkstate(vrrp->downlink_vif[0].ifname);
	   vrrp->downlink_vif[0].set_flg = VRRP_LINK_SETTED;
   }
   else{
	   vrrp->downlink_flag = 0;
   }

   vrrp->vrid = vrid;
   vrrp->profile = profile;
   vrrp_syslog_dbg("init vrrp vrid to %d\n",vrrp->vrid);
   vrrp->no_vmac = 1; 
   /*set priority*/
   vrrp->priority = priority;  
   vrrp->vrrp_trap_sw = VRRP_ON;
   /* check if the minimal configuration has been done */
   if( had_chk_min_cfg(vrrp) ){
		free(vrrp);
	  return VRRP_RETURN_CODE_BAD_PARAM;
   }

#if 0
	/* shoud move to service enable */
   if( had_open_sock(vrrp) ){/*set socket to add to multicast ip group*/
		free(vrrp);
		return VRRP_RETURN_CODE_ERR;
   }
#endif
   /* the init is completed */
   vrrp->initF = 1;
   vrrp->next = NULL;

   if(NULL == hansiNode){
      ret = had_profile_create(profile,vrid);
	  hansiNode = had_get_profile_node(profile);
	  if(hansiNode == NULL){
	  	vrrp_syslog_error("profile is:%d!\n",profile);
	  	return VRRP_RETURN_CODE_BAD_PARAM;
	  }
   }
   /*insert into the global list*/
   if(NULL == hansiNode->vlist){
	   hansiNode->vlist = vrrp;
	   hansiNode->vrid = vrid;
   }
   /*log trace */
   had_init_trace(profile);
	return  VRRP_RETURN_CODE_OK;

}

int had_vip6_gateway
(
   unsigned int profile,
   char*  vgateway_if,
   struct iaddr *vgateway_ip ,
   unsigned int prefix_length,
   unsigned int link_local
   
)
{
   int  i =0,ret = 0,err = 0;
   vrrp_rt*	vrrp = NULL;
   hansi_s* hansi = NULL;
   struct in6_addr in;
   unsigned int addr[1024] = {0};
   int index = 0;
   //add check vip realip conflict
   int naddr = 0;
   struct in6_addr real_ipv6;

   hansi = had_get_profile_node(profile);
   if(NULL == hansi){
      return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   vrrp = hansi->vlist;
   if(NULL == vrrp){
	 ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;	
	 return ret;
   }
   /*check virtual ip conflict real ip*/
	if(NULL != vgateway_if){
		memset(&real_ipv6,0,sizeof(real_ipv6));
		ret = had_ifname_to_ipv6(vgateway_if, &real_ipv6);// no ip
		if (VRRP_RETURN_CODE_OK != ret) {
			vrrp_syslog_error("ifname %s no interface found!\n",vgateway_if);
			return VRRP_RETURN_CODE_BAD_PARAM;
		}
		naddr = had_ipv6addr_list( had_ifname_to_idx(vgateway_if),addr,sizeof(addr)/sizeof(addr[0]));
		for( i = 0; i < naddr; i+=4 ){
			if(!memcmp(vgateway_ip->iabuf, &(addr[i]),sizeof(struct in6_addr))){// real ip == vir ip
				return VRRP_RETURN_CODE_BAD_PARAM;
			}
		}
	}
	vrrp_syslog_dbg("%s: ip6 = "NIP6QUAD_FMT"/%d \n",__func__,NIP6QUAD(vgateway_ip->iabuf), prefix_length);			
	ret = had_add_vipv6( profile,vgateway_if,vgateway_ip,prefix_length,link_local,VRRP_LINK_TYPE_VGATEWAY);
	if(ret != VRRP_RETURN_CODE_OK){
		return ret;
	}
   /*check if ip already exist in the interface*/
	ret = had_ipv6addr_list( had_ifname_to_idx(vgateway_if),addr,sizeof(addr)/sizeof(addr[0]) );
	for( i = 0; i < ret; i+=4 ){
	   if(!memcmp(vgateway_ip->iabuf, &(addr[i]),sizeof(struct in6_addr))){
		  break;
	   	}
	} 
	
	if(i != ret){
       /*already exist before*/
	   if(VRRP_STATE_MAST == vrrp->state){
	   	   had_ndisc_send_vgateway_unsolicited_na(vrrp,&vrrp->vgateway_vif[index].ipv6_addr,index,1);
	   }
	}
	else if(VRRP_STATE_MAST == vrrp->state){
		if(0 != (err = had_ip6addr_op_withmask(vrrp->vgateway_vif[index].ifindex, \
								vrrp->vgateway_vif[index].ipv6_addr,prefix_length,1))){
           memcpy(&in.s6_addr,&vrrp->vgateway_vif[index].ipv6_addr,sizeof(struct in6_addr)); 		
		   vrrp_syslog_error("cant set the address "NIP6QUAD_FMT"%to vgateway %s\n", 
					   		in.s6_addr, vrrp->vgateway_vif[index].ifname);
		   return VRRP_RETURN_CODE_ERR;
	   }
	   /*send arp*/
	   had_ndisc_send_vgateway_unsolicited_na(vrrp,&vrrp->vgateway_vif[index].ipv6_addr,index,1);
	}

   return  VRRP_RETURN_CODE_OK;
}

int had_v_gateway
(
   unsigned int profile,
   char*  vgateway_if,
   unsigned long  vgateway_ip ,
   unsigned int mask
)
{
   int  i =0,ret = 0,err = 0;
   vrrp_rt*	vrrp = NULL;
   hansi_s* hansi = NULL;
   struct in_addr in;
   unsigned int addr[1024] = {0};
   int index = 0;
   //add check vip realip conflict
	int naddr = 0;
   	unsigned int real_ip =0;

   hansi = had_get_profile_node(profile);
   if(NULL == hansi){
      return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   vrrp = hansi->vlist;
   if(NULL == vrrp){
	 ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;	
	 return ret;
   }
   /*check virtual ip conflict real ip*/
	if(NULL != vgateway_if){
		real_ip = 0;
		vrrp_syslog_info("%s,%d\n",__func__,__LINE__);
		ret = had_ifname_to_ip(vgateway_if, &real_ip);// no ip
		if (VRRP_RETURN_CODE_OK != ret) {
			vrrp_syslog_error("ifname %s no interface found!\n",vgateway_if);
			return VRRP_RETURN_CODE_BAD_PARAM;
		}
		naddr = had_ipaddr_list( had_ifname_to_idx(vgateway_if),addr,sizeof(addr)/sizeof(addr[0]));
		for( i = 0; i < naddr; i++ ){
			if(vgateway_ip == addr[i]){// real ip == vir ip
				return VRRP_RETURN_CODE_BAD_PARAM;
			}
		}
	}
   #if 0
   /* delete by jinpc@autelan.com
	* for not in use
	*/
   if(0 == vrrp->vgateway_flag){
	   vrrp->vgateway_vif.ifname = (char*)malloc(sizeof(char)*MAX_IFNAME_LEN);
	   if(NULL == vrrp->vgateway_vif.ifname){
	       return VRRP_RETURN_CODE_BAD_PARAM;
	   }
   }
   #endif

   #if 0
   memset(vrrp->vgateway_vif[index].ifname,0,MAX_IFNAME_LEN);
   memcpy(vrrp->vgateway_vif[index].ifname,vgateway_if,strlen(vgateway_if));	
   /*get uplink if ip addr*/
   vrrp_syslog_dbg("get vgateway ifname %s info::\n",  \
   				vrrp->vgateway_vif[index].ifname ? vrrp->vgateway_vif[index].ifname :"nil");
   vrrp->vgateway_vif[index].ifindex = had_ifname_to_idx(vrrp->vgateway_vif[index].ifname);
   had_hwaddr_get(vgateway_if,ifmac,6);
   memcpy(vrrp->vgateway_vif[index].hwaddr,ifmac,6);
   vrrp->vgateway_vif[index].linkstate = vrrp_get_ifname_linkstate(vrrp->vgateway_vif[index].ifname);
   vrrp->vgateway_vif[index].mask = mask;/*set this value as mask*/
   vrrp->vgateway_flag = 1;
   vrrp->vgateway_vif[index].ipaddr = vgateway_ip;
   #endif
   	ret = had_add_vip(profile,vgateway_if, vgateway_ip, mask, VRRP_LINK_TYPE_VGATEWAY);
	if(ret != VRRP_RETURN_CODE_OK){
		return ret;
	}
   /*check if ip already exist in the interface*/
	ret = had_ipaddr_list( had_ifname_to_idx(vgateway_if),addr,sizeof(addr)/sizeof(addr[0]) );
	for( i = 0; i < ret; i++ ){
	   if(vgateway_ip == addr[i]){
		  break;
	   	}
	} 
	
	if(i != ret){
       /*already exist before*/
	   if(VRRP_STATE_MAST == vrrp->state){
		   had_send_vgateway_gratuitous_arp(vrrp,vrrp->vgateway_vif[index].ipaddr, index, 1);
	   }
	}
	else if(VRRP_STATE_MAST == vrrp->state){
		if(0 != (err = had_ipaddr_op_withmask(vrrp->vgateway_vif[index].ifindex, \
								vrrp->vgateway_vif[index].ipaddr,vrrp->vgateway_vif[index].mask,1))){
		   in.s_addr = htonl(vrrp->vgateway_vif[index].ipaddr);
		   vrrp_syslog_error("cant set the address %#x to vgateway %s\n", 
					   		in.s_addr, vrrp->vgateway_vif[index].ifname);
		   return VRRP_RETURN_CODE_ERR;
	   }
	   /*send arp*/
	   had_send_vgateway_gratuitous_arp(vrrp,vrrp->vgateway_vif[index].ipaddr, index,1);
	}

   return  VRRP_RETURN_CODE_OK;
}


int had_no_v_gateway
(
   unsigned int profile,
   char*  vgateway_if,
   unsigned long  vgateway_ip ,
   unsigned int mask
)
{
   int  i =0,ret = 0,err = 0;
   vrrp_rt*	vrrp = NULL;
   hansi_s* hansi = NULL;
   struct in_addr in;
   unsigned int addr[1024];
   int index = 0;
   hansi = had_get_profile_node(profile);
   if(NULL == hansi){
      return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   vrrp = hansi->vlist;
   if(NULL == vrrp){
	 ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;	
	 return ret;
   }
   if(0 == vrrp->vgateway_flag){
      return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   ret = had_ipaddr_list( had_ifname_to_idx(vgateway_if),addr,sizeof(addr)/sizeof(addr[0]) );
   for( i = 0; i < ret; i++ ){
	  if(vgateway_ip == addr[i]){
		break;
	  }
   } 
   if((vrrp->state == VRRP_STATE_MAST)&&(i == ret)){/*not found vgateway ip*/
      return VRRP_RETURN_CODE_IF_NOT_EXIST;
   }
   
   if(VRRP_STATE_MAST == vrrp->state){
		if(0 != (err = had_ipaddr_op_withmask(vrrp->vgateway_vif[index].ifindex, \
								vrrp->vgateway_vif[index].ipaddr,vrrp->vgateway_vif[index].mask,0))){
		   in.s_addr = htonl(vrrp->vgateway_vif[index].ipaddr);
		   vrrp_syslog_error("cant remove the address %s from vgateway %s\n",
		   				inet_ntoa(in), vrrp->vgateway_vif[index].ifname);
		   return VRRP_RETURN_CODE_ERR;
	   }
	}
   had_delete_vip(profile, vgateway_if, vgateway_ip, mask,VRRP_LINK_TYPE_VGATEWAY);
   #if 0
   vrrp->vgateway_flag = VRRP_LINK_NO_SETTED;
   memset(&(vrrp->vgateway_vif[index]), 0, sizeof(vrrp_if));
   #endif
   #if 0
	/* delete by jinpc@autelan.com
	 * for not in use
	 */
   /* free memory, add by jinpc@autelan.com 2009.11.17 */
   free(vrrp->vgateway_vif.ifname);
   vrrp->vgateway_vif.ifname = NULL;
   #endif
   vrrp->vgateway_tf_flag = VRRP_VGATEWAY_TF_FLG_OFF;

   return  VRRP_RETURN_CODE_OK;
}

int had_end
(
   unsigned int profile
)
{
    hansi_s* hansiNode = had_get_profile_node(profile);
	struct state_trace* log = NULL;
	
    if(hansiNode == NULL){
       return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

    vrrp_rt* vsrv = hansiNode->vlist;
	if(NULL == vsrv){
       return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	/*
	vrrp_rt* p = NULL;
	while((vsrv)&&(vrid != vsrv->vrid)){
		p = vsrv;
		vsrv = vsrv->next;
	}
	if(NULL == p)
		hansiNode->vlist = vsrv->next;
	else
		p = vsrv->next;
	*/
	/* remove the pid file */
	/* if the deamon is master, leave this state */
	had_vrrp_vif_rcv_pck_ON(vsrv);
	if( vsrv->state == VRRP_STATE_MAST ){
		vrrp_syslog_info("%s,%d,start to leave master!\n",__func__,__LINE__);
		had_state_leave_master( vsrv, 1,NULL,NULL );
	}
	if(VRRP_LINK_NO_SETTED != vsrv->uplink_flag){
		master_ipaddr_uplink[vsrv->vrid] = 0;
		memset(hansiNode->uplink_real_ip, 0, VRRP_LINK_MAX_CNT * sizeof(vrrp_if_real_ip));
	}
	if(VRRP_LINK_NO_SETTED != vsrv->downlink_flag){
		master_ipaddr_downlink[vsrv->vrid] = 0;
		memset(hansiNode->downlink_real_ip, 0, VRRP_LINK_MAX_CNT * sizeof(vrrp_if_real_ip));
	}
	if(VRRP_LINK_NO_SETTED != vsrv->vgateway_flag){
		memset(&(vsrv->vgateway_vif), 0, VRRP_LINK_MAX_CNT * sizeof(vrrp_if));
		#if 0
		free(vsrv->vgateway_vif.ifname);
		vsrv->vgateway_vif.ifname = NULL;
		#endif
	}
	/*free trace log*/
    if((NULL != TRACE_LOG)&&(NULL !=(log = TRACE_LOG[vsrv->profile]))){
        if(NULL != log->action){
		  free(log->action);
		  log->action = NULL;
        }
		if(log->step){
		  free(log->step);
		  log->step = NULL;
		}
		free(log);
		TRACE_LOG[vsrv->profile] = NULL;
	}
	had_LIST_DEL(vsrv);
    free(vsrv);
	hansiNode->vlist = NULL;
	return VRRP_RETURN_CODE_OK;
}

int had_priority_cfg
(
   int profile,
   int priority
)
{
   vrrp_rt* vrrp = NULL;
   hansi_s* hansiNode = had_get_profile_node(profile);
   if(NULL == hansiNode){     
	  return VRRP_RETURN_CODE_PROFILE_NOTEXIST;      
   }
   vrrp = hansiNode->vlist;
   if(NULL == vrrp){
      return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   vrrp->priority = priority;
   return VRRP_RETURN_CODE_OK;
}

int had_global_vmac_cfg
(
   int profile,
   int gvmac_enable 
)
{
   vrrp_rt* vrrp = NULL;
   hansi_s* hansiNode = had_get_profile_node(profile);
   if(NULL == hansiNode){
      return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   vrrp = hansiNode->vlist;
   if(NULL == vrrp){
     return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   else if(VRRP_SERVICE_ENABLE == service_enable[profile]){
	   return VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
   }
   else if (vrrp->no_vmac){
		return VRRP_RETURN_CODE_VMAC_NOT_PREPARE;
   }
       vrrp->smart_vmac_enable = gvmac_enable;
       return VRRP_RETURN_CODE_OK;
}

int had_preempt_cfg
(
   int profile,
   int preempt
)
{
   vrrp_rt* vrrp = NULL;
   hansi_s* hansiNode = had_get_profile_node(profile);
   if(NULL == hansiNode){
      return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   vrrp = hansiNode->vlist;
   if(NULL == vrrp){
     return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   else if(VRRP_SERVICE_ENABLE == service_enable[profile]){\
	   return VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
   }
   vrrp->preempt = preempt;
   return VRRP_RETURN_CODE_OK;
}

/*
 *******************************************************************************
 *had_set_notify_obj_on_off()
 *
 *  DESCRIPTION:
 *		set notify obj and on/off flg
 *
 *  INPUTS:
 * 		int profile,
 *		unsigned char notify_obj,
 *		unsigned char notify_flg
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
unsigned int had_set_notify_obj_on_off
(
	int profile,
	unsigned char notify_obj,
	unsigned char notify_flg	
)
{
	vrrp_rt* vrrp = NULL;
	hansi_s* hansiNode = NULL;

	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	vrrp = hansiNode->vlist;
	if (NULL == vrrp) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	if (VRRP_NOTIFY_OBJ_TPYE_WID == notify_obj) {
		/* bit[0]: wid */
		if (VRRP_NOTIFY_ON == notify_flg) {
			vrrp->notify_flg |= VRRP_NOTIFY_BIT_WID;
		}else if (VRRP_NOTIFY_OFF == notify_flg) {
			vrrp->notify_flg &= VRRP_NOTIFY_MASK_WID;
		}
	}else if (VRRP_NOTIFY_OBJ_TPYE_PORTAL == notify_obj) {
		/* bit[1]: portal */
		if (VRRP_NOTIFY_ON == notify_flg) {
			vrrp->notify_flg |= VRRP_NOTIFY_BIT_PORTAL;
		}else if (VRRP_NOTIFY_OFF == notify_flg) {
			vrrp->notify_flg &= VRRP_NOTIFY_MASK_PORTAL;
		}
	}else if(VRRP_NOTIFY_OBJ_TYPE_DHCP == notify_obj) {
		/* bit[2]: portal */
		if(VRRP_NOTIFY_ON == notify_flg) {
			vrrp->notify_flg |= VRRP_NOTIFY_BIT_DHCP;

			/*add by sunjc@autelan.com*/
			had_notify_to_dhcp_failover(vrrp, vrrp->state);
		}
		else if(VRRP_NOTIFY_OFF == notify_flg) {
			vrrp->notify_flg &= VRRP_NOTIFY_MASK_DHCP;
		}
	}else if (VRRP_NOTIFY_OBJ_TPYE_PORTAL == notify_obj) {
		/* bit[4]: pppoe */
		if (VRRP_NOTIFY_ON == notify_flg) {
			vrrp->notify_flg |= VRRP_NOTIFY_BIT_PPPOE;
		}else if (VRRP_NOTIFY_OFF == notify_flg) {
			vrrp->notify_flg &= VRRP_NOTIFY_MASK_PPPOE;
		}
	}

	return VRRP_RETURN_CODE_OK;
}

/*
 *******************************************************************************
 *had_set_vgateway_tf_flag_on_off()
 *
 *  DESCRIPTION:
 *		set vgateway transform flag on/off
 *
 *  INPUTS:
 * 		int profile,
 *		unsigned int vgateway_tf_flg
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_VGATEWAY_NO_SETTED		- vgataway not setted
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
unsigned int had_set_vgateway_tf_flag_on_off
(
	int profile,
	unsigned int vgateway_tf_flg	
)
{
	vrrp_rt* vrrp = NULL;
	hansi_s* hansiNode = NULL;

	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	vrrp = hansiNode->vlist;
	if (NULL == vrrp) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	/* check vgateway is setted. */
	if (0 == vrrp->vgateway_flag) {
		vrrp_syslog_error("set vgateway tf flag error, for interface not setted\n");
		return VRRP_RETURN_CODE_VGATEWAY_NO_SETTED;
	}
	vrrp->vgateway_tf_flag = vgateway_tf_flg;

	vrrp_syslog_dbg("set vgateway transform flag %s(%d).\n",
					(VRRP_VGATEWAY_TF_FLG_ON == vgateway_tf_flg) ? "ON" : "OFF",
					vgateway_tf_flg);
	
	return VRRP_RETURN_CODE_OK;
}
	
/*
 *******************************************************************************
 *had_set_back_down_flag_on_off()
 *
 *	DESCRIPTION:
 *		set vip back down flag on/off
 *
 *	INPUTS:
 *		int profile,
 *		unsigned int set_flg   --   set back down flag to this value (on/off)
 *		unsigned int link_type_flg  --  vip link type (uplink/downlink/vgateway/l2-uplink)
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_VIP_NOT_EXIST 	- the vip not setted
 *		VRRP_RETURN_CODE_BAD_PARAM  -       unknown link_type_flg 
 *		VRRP_RETURN_CODE_OK 					- set success
 *
 *******************************************************************************
 */
unsigned int had_set_back_down_flag_on_off
(
	int profile,
	unsigned int set_flg,
	unsigned int link_type_flg
)
{
	vrrp_rt* vrrp = NULL;
	hansi_s* hansiNode = NULL;

	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	vrrp = hansiNode->vlist;
	if (NULL == vrrp) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	switch(link_type_flg){
		case VRRP_LINK_TYPE_UPLINK:
			if(VRRP_LINK_NO_SETTED == vrrp->uplink_flag){
				vrrp_syslog_error("uplink not configured when set smart uplink flag\n");
				return VRRP_RETURN_CODE_VIP_NOT_EXIST;
			}
			vrrp->uplink_back_down_flag = set_flg;
			break;
		case VRRP_LINK_TYPE_DOWNLINK:
			if(VRRP_LINK_NO_SETTED == vrrp->downlink_flag){
				vrrp_syslog_error("downlink not configured when set smart downlink flag\n");
				return VRRP_RETURN_CODE_VIP_NOT_EXIST;
			}
			vrrp->downlink_back_down_flag = set_flg;
			break;
		case VRRP_LINK_TYPE_VGATEWAY:
			if(VRRP_LINK_NO_SETTED == vrrp->vgateway_flag){
				vrrp_syslog_error("vgateway not configured when set smart vgateway flag\n");
				return VRRP_RETURN_CODE_VIP_NOT_EXIST;
			}
			vrrp->vgateway_back_down_flag = set_flg;
			break;
		case VRRP_LINK_TYPE_L2_UPLINK:
			if(VRRP_LINK_NO_SETTED == vrrp->l2_uplink_flag){
				vrrp_syslog_error("l2-uplink not configured when set l2-uplink flag\n");
				return VRRP_RETURN_CODE_VIP_NOT_EXIST;
			}
			vrrp->l2_uplink_back_down_flag = set_flg;
			break;
		default:
			vrrp_syslog_error("unknown link type when set smart link flag\n");
			return VRRP_RETURN_CODE_BAD_PARAM;
			break;
	}	
	if(vrrp->state == VRRP_STATE_BACK){
		if (set_flg == VRRP_ON){
			had_vrrp_vif_drop_pck(vrrp);
			}
		if (set_flg == VRRP_OFF){
			had_vrrp_vif_rcv_pck_OFF(vrrp);
			}
	}
	vrrp_syslog_dbg("set back down flag to %d for type %d ok\n",set_flg, link_type_flg);
	
	return VRRP_RETURN_CODE_OK;
}
	
/*
 *******************************************************************************
 *had_set_vrrp_trap_on_off()
 *
 *	DESCRIPTION:
 *		set vrrp trap switch flag on/off
 *
 *	INPUTS:
 *		int profile,
 *		unsigned int set_flg   --   set vrrp trap switcw to this value (on/off)
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_VIP_NOT_EXIST 	- the vip not setted
 *		VRRP_RETURN_CODE_BAD_PARAM  -       unknown link_type_flg 
 *		VRRP_RETURN_CODE_OK 					- set success
 *
 *******************************************************************************
 */
unsigned int had_set_vrrp_trap_on_off
(
	int profile,
	unsigned int set_flg
)
{
	vrrp_rt* vrrp = NULL;
	hansi_s* hansiNode = NULL;

	hansiNode = had_get_profile_node(profile);
	if (NULL == hansiNode) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}

	vrrp = hansiNode->vlist;
	if (NULL == vrrp) {
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	vrrp->vrrp_trap_sw = set_flg;

	vrrp_syslog_dbg("set profile:%d trap sw %s ok\n",profile,(set_flg==VRRP_OFF)?"disable":"enable");
	
	return VRRP_RETURN_CODE_OK;
}

int had_advert_cfg
(
   int profile,
   int time
)
{
   vrrp_rt* vrrp = NULL;
   hansi_s* hansiNode = had_get_profile_node(profile);
   if(NULL == hansiNode){
     return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   vrrp = hansiNode->vlist;
   if(NULL == vrrp){
     return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   if(time < 1){
      return VRRP_RETURN_CODE_BAD_PARAM;
   }
   else{
      vrrp->adver_int = time*VRRP_TIMER_HZ;
   }
   return VRRP_RETURN_CODE_OK;
}

int had_mac_cfg
(
   int profile,
   int mac
)
{
   vrrp_rt* vrrp = NULL;
   hansi_s* hansiNode = had_get_profile_node(profile);
   if(NULL == hansiNode){
     return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   vrrp = hansiNode->vlist;
   if(NULL == vrrp){
     return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
   }
   else if(VRRP_SERVICE_ENABLE == service_enable[profile]){
     return VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
   }
   else{
      vrrp->no_vmac = mac;
	  if(VRRP_STATE_MAST == vrrp->state){
		  had_change_mac(vrrp);
	  }
   }  
   vrrp_syslog_dbg("config vrrp %d use virtual mac %s\n",vrrp->vrid,vrrp->no_vmac ? "NO" : "YES");
   return VRRP_RETURN_CODE_OK;
}


vrrp_rt* had_show_cfg
(
   int profile
)
{
   vrrp_rt* vrrp = NULL;
   hansi_s* hansiNode = had_get_profile_node(profile);
   if(NULL == hansiNode){
      return NULL;
   }
   vrrp = hansiNode->vlist;
   if(NULL == vrrp){
      return NULL;
   }
   return vrrp;
}

int had_get_min_advtime()
{
    int profile = 0,advtime = 0;
	hansi_s* hansiNode = NULL;
	vrrp_rt* vrrp = NULL;
	for(profile = 0;profile < MAX_HANSI_PROFILE;profile++){
       hansiNode = g_hansi[profile];
	   if((NULL == hansiNode)||(NULL == (vrrp = hansiNode->vlist))){
           continue;
	   }
	   else{
           if(0 == advtime)advtime = vrrp->adver_int;
		   else advtime = (advtime < vrrp->adver_int ? advtime : vrrp->adver_int);
	   }
	}
	vrrp_syslog_dbg("get min advtime %d\n",advtime);
	return advtime;
}

void had_sleep_time(void)
{
	struct timeval	timeout;
	uint32_t	time	= 1001000;

	/*
	int base = 0,delta = 0;
 
	if( VRRP_TIMER_IS_RUNNING( vsrv->adver_timer ) ){
		int32_t	delta = VRRP_TIMER_DELTA(vsrv->adver_timer);
		if( delta < 0 )	delta = 0;
		next = VRRP_MIN( next, delta );
	}else{	
		int32_t	delta = VRRP_TIMER_DELTA(vsrv->ms_down_timer);
		assert( VRRP_TIMER_IS_RUNNING( vsrv->ms_down_timer ) );
		if( delta < 0 )	delta = 0;
		next = VRRP_MIN( next, delta );
	}*/
	/*set sleep time manually*/
	/*
	base = had_get_runing_hansi();
	
	if(base){
       time = had_get_min_advtime();
	   time = time/base;
	}
	*/
	/* setup the select() */
	timeout.tv_sec	= time/ VRRP_TIMER_HZ;
	timeout.tv_usec = time % VRRP_TIMER_HZ;
	/*wait timeout*/
	select( 0, NULL, NULL, NULL, &timeout );
	return;

}

void had_sleep_seconds(int second)
{
	struct timeval	timeout;
	uint32_t	time	= second;
    vrrp_syslog_dbg("start to sleep %d seconds!\n",second);
	/* setup the select() */
	timeout.tv_sec	= time;
	timeout.tv_usec = 0;
	/*wait timeout*/
	select( 0, NULL, NULL, NULL, &timeout );
	return;

}

void had_update_uplink
(
   vrrp_rt* vsrv,
   char* uplink_buf,
   unsigned int uplink_len  
)
{
	struct iphdr *uplink_iph = NULL;
	vrrp_pkt *uplink_hd = NULL;

	unsigned int chang_Flag = 0;
	unsigned int delay = 0;

	if (!vsrv || ! uplink_buf) {
		vrrp_syslog_error("update uplink null parameter error\n");
		return;
	}

	uplink_iph = (struct iphdr *)uplink_buf;
	uplink_hd = (vrrp_pkt *)((char *)uplink_iph + (uplink_iph->ihl<<2));

    if(VRRP_STATE_LEARN == vsrv->state){				
		uplink_iph = (struct iphdr *)uplink_buf;
		vrrp_syslog_dbg("update vrrp %d back state according uplink!\n",vsrv->vrid);
		if (VRRP_LINK_NO_SETTED == vsrv->uplink_flag) {
			vrrp_syslog_dbg("uplink interface not setted, no update!\n");
			goto be_back;
		}
		
		if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
			(!vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_UPLINK)))
		{
			 vrrp_syslog_dbg("vrrp %d uplink interface no ip or not up,no update!\n",vsrv->vrid);
			 goto be_back;
		}	

		/*the master leave master,normal exchang*/
		if ( 0 == uplink_hd->priority) {
			vrrp_syslog_info("state %s uplink received packets priority is 0,goto master!\n", \
							VRRP_STATE_DESCRIPTION(vsrv->state));
            had_state_goto_master(vsrv,VRRP_STATE_BACK);
			return;
		} 
		else {
			master_ipaddr_uplink[vsrv->vrid] = uplink_iph->saddr;
			if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
				(0 == master_ipaddr_downlink[vsrv->vrid]))
			{
				vrrp_syslog_dbg("downlink not received message,wait!\n");
			}
			else{
				if ((NULL != global_ht_ifname) &&
					(0 != global_ht_ip))
				{
                   global_ht_opposite_ip = uplink_hd->heartbeatlinkip;
				}
				delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
				VRRP_TIMER_SET(vsrv->uplink_ms_down_timer, delay);
				VRRP_TIMER_SET(vsrv->downlink_ms_down_timer, delay);
				had_state_from_learn(vsrv,uplink_hd);
			}
		}
		return ;      
    }

   if(VRRP_STATE_TRANSFER == vsrv->state){
      if((time_synchronize_enable)&&(1 == uplink_hd->que_f)){
        settimeofday(&(uplink_hd->tv),NULL);
        global_timer_reply = 1;
      }
      return;
   }
   if(VRRP_STATE_BACK == vsrv->state){	
		/*check if querry back packet and set system time*/
		if((time_synchronize_enable)&&(1 == uplink_hd->que_f)){
		   VRRP_TIMER_CLR(vsrv->uplink_ms_down_timer);
		   VRRP_TIMER_CLR(vsrv->downlink_ms_down_timer);
           settimeofday(&(uplink_hd->tv),NULL);
	       delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
		   VRRP_TIMER_SET(vsrv->uplink_ms_down_timer, delay);
		   VRRP_TIMER_SET(vsrv->downlink_ms_down_timer, delay);		   
		   return;
		}		
		uplink_iph = (struct iphdr *)uplink_buf;
		vrrp_syslog_dbg("update vrrp %d back state according uplink!\n",vsrv->vrid);
		if (VRRP_LINK_NO_SETTED == vsrv->uplink_flag) {
			vrrp_syslog_dbg("uplink interface null,no update!\n");
			goto be_back;
		}
		
		if ((VRRP_LINK_SETTED == vsrv->uplink_flag) &&
			(!vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_UPLINK)))
		{
			 vrrp_syslog_dbg("vrrp %d interface no ip or not up,no update!\n",vsrv->vrid);
			 goto be_back;
		}	

		/*the master leave master,normal exchang*/
		if ( 0 == uplink_hd->priority) {
			vrrp_syslog_info("state %s uplink received packets priority is 0,goto master!\n",  \
							VRRP_STATE_DESCRIPTION(vsrv->state));
            had_state_goto_master(vsrv,VRRP_STATE_BACK);
			return;
		} 
		else if (!vsrv->preempt ||
				 (uplink_hd->priority > vsrv->priority) ||
				 ((uplink_hd->priority == vsrv->priority) &&
				  (ntohl(uplink_iph->saddr) > vrrp_get_link_max_ipaddr(vsrv, VRRP_LINK_TYPE_UPLINK))))
		{
			delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
			VRRP_TIMER_SET(vsrv->uplink_ms_down_timer, delay);
			vrrp_syslog_dbg("vrrp is non preempt or uplink received packet priority is bigger,reset the down timer to %u delta %d\n!",
							vsrv->uplink_ms_down_timer, delay);
		}
		else if ((vsrv->preempt &&
				  (uplink_hd->priority < vsrv->priority)) ||
				 ((uplink_hd->priority == vsrv->priority) &&
				  (ntohl(uplink_iph->saddr) < vrrp_get_link_max_ipaddr(vsrv, VRRP_LINK_TYPE_UPLINK))))
		{
            vrrp_syslog_info("state %s uplink rcv pkts with low privilege(pri %d mine %d, or ip %#x mine),goto master!\n", \
							VRRP_STATE_DESCRIPTION(vsrv->state), uplink_hd->priority, vsrv->priority, \
							ntohl(uplink_iph->saddr));
			had_state_goto_master(vsrv,uplink_hd->state);
			return;
		}

		if(master_ipaddr_uplink[vsrv->vrid] != uplink_iph->saddr){
            vrrp_syslog_dbg("master ip chang from %x to %x\n",
							master_ipaddr_uplink[vsrv->vrid], uplink_iph->saddr);
            master_ipaddr_uplink[vsrv->vrid] = uplink_iph->saddr;
            chang_Flag = 1;
		}
		if(chang_Flag){/*notify to wid the master ip changed*/
            ;
		}
		return ;      
    }
	else if(VRRP_STATE_MAST == vsrv->state){		
		uplink_iph = (struct iphdr *)uplink_buf;
		if((time_synchronize_enable)&&(1 == uplink_hd->que_f)){
            struct timeval tv;
			gettimeofday(&tv,NULL);
			vsrv->que_f = 1;
			vrrp_send_adv(vsrv,vsrv->priority);
			vsrv->que_f = 0;
			return;
		}
		if(VRRP_STATE_LEARN == uplink_hd->state){
			vrrp_syslog_dbg("update master,but opposite state is learn,drop!\n");
			return;
		}
		vrrp_syslog_dbg("update vrrp %d master state according uplink!\n",vsrv->vrid);
		if(uplink_hd->priority == 0){
			vrrp_syslog_dbg("uplink recieve packets priority zero(stop vrrp),send packets,priority %d!\n",vsrv->priority);		
			vrrp_send_uplink_adv( vsrv, vsrv->priority );		
			VRRP_TIMER_SET(vsrv->uplink_adver_timer,vsrv->adver_int);
		} 
		else if(VRRP_AUTH_FAIL == uplink_hd->auth_type){
			vrrp_syslog_info("%s,%d,according uplink message,auth failed,leave master!\n",__func__,__LINE__);
			had_state_leave_master( vsrv, 0,uplink_iph,NULL);			
		}
		else if ((uplink_hd->priority > vsrv->priority) ||
				 ((uplink_hd->priority == vsrv->priority) &&
				  (ntohl(uplink_iph->saddr) > vrrp_get_link_max_ipaddr(vsrv, VRRP_LINK_TYPE_UPLINK))))
		{
			vrrp_syslog_dbg("vrrp %d uplink received better priority msg,leave master!\n",vsrv->vrid);
			uplink_leave_master_timer[vsrv->vrid] = had_TIMER_CLK();			
			master_ipaddr_uplink[vsrv->vrid] = uplink_iph->saddr;			
			if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
				(0 == master_ipaddr_downlink[vsrv->vrid]))
			{
			   vrrp_syslog_dbg("master downlink ip is 0,wait!\n");
               return;
			}
			else{
				if ((VRRP_LINK_NO_SETTED != vsrv->downlink_flag) &&
					((uplink_leave_master_timer[vsrv->vrid] - downlink_leave_master_timer[vsrv->vrid]) > 2*vsrv->adver_int))
				{
                  vrrp_syslog_dbg("uplink wait too long time,return\n");
				  return;
			   }
			   else{
					if ((NULL != global_ht_ifname) &&
						(0 != global_ht_ip))
					{
	                   global_ht_opposite_ip = uplink_hd->heartbeatlinkip;
					}			   	
				   vrrp_syslog_dbg("leave master,master uplink ip %x,downlink ip %x!\n",
									master_ipaddr_uplink[vsrv->vrid], master_ipaddr_downlink[vsrv->vrid]);
				   vrrp_syslog_info("state %s uplink receive msg with privilege(prio %d mine %d, or ip %#x mine),leave master!\n", \
								   VRRP_STATE_DESCRIPTION(vsrv->state), uplink_hd->priority, vsrv->priority,ntohl(uplink_iph->saddr));
				   had_state_leave_master( vsrv, 0,uplink_iph,NULL);
				   uplink_leave_master_timer[vsrv->vrid] = 0;
				   downlink_leave_master_timer[vsrv->vrid] = 0;
			   }
			}
			return;
		}
	    return;     
	}
	be_back:
		if(VRRP_STATE_MAST == vsrv->state){ 		 
		   vrrp_syslog_info("%s,%d,uplink leave master.\n",__func__,__LINE__);
		   had_state_leave_master( vsrv, 0,NULL,NULL);
		}
		return;

}


void had_update_downlink
(
   vrrp_rt* vsrv,
   char* downlink_buf,
   unsigned int downlink_len  
)
{
	struct iphdr *downlink_iph = NULL;
	vrrp_pkt *downlink_hd = NULL;
	unsigned int *downlink_iparr = NULL;
	unsigned int chang_Flag = 0;
	unsigned int delay = 0;

	if (!vsrv || ! downlink_buf) {
		vrrp_syslog_error("update downlink null parameter error\n");
		return;
	}
	
	downlink_iph = (struct iphdr *)downlink_buf;
	downlink_hd = (vrrp_pkt *)((char *)downlink_iph + (downlink_iph->ihl<<2));

    if(VRRP_STATE_LEARN == vsrv->state){		
		vrrp_syslog_dbg("update vrrp %d back state according downlink packets!\n",vsrv->vrid);
		if (VRRP_LINK_NO_SETTED == vsrv->downlink_flag)
		{
			vrrp_syslog_dbg("downlink interface not setted, no update!\n");
			goto be_back;
		}
		
		if ((VRRP_LINK_SETTED == vsrv->downlink_flag) &&
			(!vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_DOWNLINK)))
		{
			 vrrp_syslog_dbg("vrrp %d downlink interface no ip or not up,no update!\n",vsrv->vrid);
			 goto be_back;
		}	

		if ( 0 == downlink_hd->priority) {
			vrrp_syslog_info("state %s downlink received packets priority is 0,go to master!\n", \
							VRRP_STATE_DESCRIPTION(vsrv->state));
			had_state_goto_master(vsrv,VRRP_STATE_BACK);
			return;
		} 
		else{
			master_ipaddr_downlink[vsrv->vrid] = downlink_iph->saddr;
			if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
				(0 == master_ipaddr_uplink[vsrv->vrid]))
			{
				vrrp_syslog_dbg("uplink not received message,wait!\n");
			}
			else{
				if ((NULL != global_ht_ifname) &&
					(0 != global_ht_ip))
				{
                   global_ht_opposite_ip = downlink_hd->heartbeatlinkip;
				}				
				delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);				
				VRRP_TIMER_SET(vsrv->uplink_ms_down_timer, delay);
				VRRP_TIMER_SET(vsrv->downlink_ms_down_timer, delay);
				had_state_from_learn(vsrv,downlink_hd);
			}
		}
		return ;      
    }
    if(VRRP_STATE_TRANSFER == vsrv->state){
       if((time_synchronize_enable)&&(1 == downlink_hd->que_f)){
          settimeofday(&(downlink_hd->tv),NULL);
	      global_timer_reply = 1;
       }
       return;
    }    
    if(VRRP_STATE_BACK == vsrv->state){	
		/*check if querry back packet and set system time*/
		if((time_synchronize_enable)&&(1 == downlink_hd->que_f)){
		   VRRP_TIMER_CLR(vsrv->uplink_ms_down_timer);
		   VRRP_TIMER_CLR(vsrv->downlink_ms_down_timer);
           settimeofday(&(downlink_hd->tv),NULL);
		   delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);				
		   VRRP_TIMER_SET(vsrv->uplink_ms_down_timer, delay);
		   VRRP_TIMER_SET(vsrv->downlink_ms_down_timer, delay);		   
		   return;
		}
		vrrp_syslog_dbg("update vrrp %d back state according downlink packets!\n",vsrv->vrid);
		if (VRRP_LINK_NO_SETTED == vsrv->downlink_flag)
		{
			vrrp_syslog_dbg("downlink interface not setted,no update!\n");
			goto be_back;
		}
		
		if ((VRRP_LINK_SETTED == vsrv->downlink_flag) &&
			(!vrrp_get_link_state(vsrv, VRRP_LINK_TYPE_DOWNLINK)))
		{
			 vrrp_syslog_dbg("vrrp %d downlink interface no ip or not up,no update!\n",vsrv->vrid);
			 goto be_back;
		}	

		if ( 0 == downlink_hd->priority) {
			vrrp_syslog_info("state %s downlink received packets priority is 0, goto master!\n", \
						VRRP_STATE_DESCRIPTION(vsrv->state));
			had_state_goto_master(vsrv,VRRP_STATE_BACK);
			return;
		} 
		else if (!vsrv->preempt ||
				 (downlink_hd->priority > vsrv->priority) ||
				 ((downlink_hd->priority == vsrv->priority) &&
				  (ntohl(downlink_iph->saddr) > vrrp_get_link_max_ipaddr(vsrv, VRRP_LINK_TYPE_DOWNLINK))))
		{
			delay = global_ms_down_packet_count*vsrv->adver_int + VRRP_TIMER_SKEW(vsrv);
			VRRP_TIMER_SET(vsrv->downlink_ms_down_timer, delay);
			vrrp_syslog_dbg("vrrp is non preempt or downlink received packet priority is bigger,reset the down timer to %u delta %d\n!",
							vsrv->downlink_ms_down_timer, delay);
		}
		else if ((vsrv->preempt &&
				  (downlink_hd->priority < vsrv->priority)) ||
				 ((downlink_hd->priority == vsrv->priority) &&
				  (ntohl(downlink_iph->saddr) < vrrp_get_link_max_ipaddr(vsrv, VRRP_LINK_TYPE_DOWNLINK))))
		{
            vrrp_syslog_info("state %s downlink rcv pkts lower privilege(pri %d mine %d,or ip %#x mine),goto master\n", \
							VRRP_STATE_DESCRIPTION(vsrv->state),downlink_hd->priority, vsrv->priority, \
							ntohl(downlink_iph->saddr));
			had_state_goto_master(vsrv,downlink_hd->state);
			return;
		}
		
		downlink_iparr	= (unsigned int *)((char *)downlink_hd+sizeof(*downlink_hd));
		if(master_ipaddr_downlink[vsrv->vrid] != downlink_iph->saddr){
            vrrp_syslog_dbg("master ip chang from %x to %x\n",master_ipaddr_downlink[vsrv->vrid], downlink_iparr[0]);
            master_ipaddr_downlink[vsrv->vrid] = downlink_iph->saddr;
            chang_Flag = 1;
		}
		if(chang_Flag){/*notify to wid the master ip changed*/
            ;
		}		
		return ;      
    }
	else if(VRRP_STATE_MAST == vsrv->state){
		vrrp_syslog_dbg("update vrrp %d master state!\n",vsrv->vrid);
	    vrrp_syslog_dbg("vrrp %d read packet downlink len %d \n",vsrv->vrid,downlink_len);
		if((time_synchronize_enable)&&(1 == downlink_hd->que_f)){
            struct timeval tv;
			gettimeofday(&tv,NULL);
			vsrv->que_f = 1;
			vrrp_send_adv(vsrv,vsrv->priority);
			vsrv->que_f = 0;
			return;
		}
		if(VRRP_STATE_LEARN == downlink_hd->state){
 			vrrp_syslog_dbg("update master,bug opposite state is learn,drop!\n");
			return;           
		}
		if(downlink_hd->priority == 0){
			vrrp_syslog_dbg("downlink recieve packets priority zero(stop vrrp),send packets,priority %d!\n",vsrv->priority);		
			vrrp_send_downlink_adv( vsrv, vsrv->priority );		
			VRRP_TIMER_SET(vsrv->uplink_adver_timer,vsrv->adver_int);
		}
		else if(VRRP_AUTH_FAIL == downlink_hd->auth_type){
			vrrp_syslog_info("%s,%d,according downlink message,auth failed,leave master!\n",__func__,__LINE__);
			had_state_leave_master( vsrv, 0,NULL,downlink_iph);
		}	
		else if ((downlink_hd->priority > vsrv->priority) ||
				 ((downlink_hd->priority == vsrv->priority ) &&
				  (ntohl(downlink_iph->saddr) > vrrp_get_link_max_ipaddr(vsrv, VRRP_LINK_TYPE_DOWNLINK))))
		{
			vrrp_syslog_dbg("vrrp %d downlink received better priority msg,just to if leave master!\n",vsrv->vrid);
			downlink_leave_master_timer[vsrv->vrid] = had_TIMER_CLK();			
			master_ipaddr_downlink[vsrv->vrid] = downlink_iph->saddr;			
			if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
				(0 == master_ipaddr_uplink[vsrv->vrid]))
			{
			   vrrp_syslog_dbg("uplink master ip is 0,wait\n");
               return;
			}
			else{
               if ((VRRP_LINK_NO_SETTED != vsrv->uplink_flag) &&
					((downlink_leave_master_timer[vsrv->vrid]-uplink_leave_master_timer[vsrv->vrid]) > 2*vsrv->adver_int))
			   {
                 vrrp_syslog_dbg("downlink wait too long time,return\n");
				  return;
			   }
			   else{
					if((NULL != global_ht_ifname)&&(0 != global_ht_ip)){
	                   global_ht_opposite_ip = downlink_hd->heartbeatlinkip;
					}				   	
				   vrrp_syslog_dbg("leave master,master uplink ip %x,downlink ip %x!\n",
				   					master_ipaddr_uplink[vsrv->vrid],master_ipaddr_downlink[vsrv->vrid]);
				   vrrp_syslog_info("state %s downlink rcv pkts with privilege(pri %d mine %d or ip %#x mine),leave master!\n", \
				   					VRRP_STATE_DESCRIPTION(vsrv->state), downlink_hd->priority, vsrv->priority, \
									ntohl(downlink_iph->saddr));
				   had_state_leave_master( vsrv, 0,NULL,downlink_iph);
				   uplink_leave_master_timer[vsrv->vrid] = 0;
				   downlink_leave_master_timer[vsrv->vrid] = 0;
			   }
			}
			return;
		} 	
	    return;     
	}
be_back:
	if(VRRP_STATE_MAST == vsrv->state){ 		 
		vrrp_syslog_info("%s,%d,downlink leave master.\n",__func__,__LINE__);
	   had_state_leave_master( vsrv,0,NULL,NULL);
	}
    return;
}

#if 0
/* delete by jinpc@autelan.com
 * for not in use
 */
vrrp_rt* vrrp_get_instance_by_ifname(char* ifname){
   int i = 0;
   vrrp_rt* vrrp = NULL;
   if(NULL == ifname){
       return NULL;
   }
	for(i = 1; i <= MAX_HANSI_PROFILE; i++) {
       vrrp = had_check_if_exist(i);
	   if(NULL != vrrp){
          if((vrrp->uplink_flag && (!strcmp(ifname,vrrp->uplink_vif.ifname))) ||
		  	 (vrrp->downlink_flag && (!strcmp(ifname,vrrp->downlink_vif.ifname)))){
              return vrrp;
		  }
	   }
	}  
	return NULL;
}
#endif

/**********************************************************************************
 * had_check_boot_status
 *	This method check up boot flag to verify that system is in boot stage or not.
 *
 *	INPUT:
 *		profile - hansi prifle id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1 - if check boot status files error
 *		0 - system is not in boot stage(in running stage)
 *		1 - system is in boot stage
 *		
 *	NOTE:
 *
 **********************************************************************************/
int had_check_boot_status
(
	int profile
)
{
	unsigned char buf[4] = {0};
	static int count[MAX_HANSI_PROFILE] = {0};
	int n = -1;

	if(profile >= MAX_HANSI_PROFILE) {
		vrrp_syslog_error("check boot stage with profile %d out of range error!\n", profile);
		return -2;
	}
	
	++count[profile];
	if(aw_state_fd < 0) {
		aw_state_fd = open(HAD_SYSTEM_STARTUP_STATE0_PATH,O_RDONLY);
		if(aw_state_fd < 0) {
			if(0 == (count[profile] % HAD_BOOT_CHECK_MSG_HZ)) {
		  		vrrp_syslog_error("inst%d open %d times system boot state file %s error\n", \
									profile, count[profile], HAD_SYSTEM_STARTUP_STATE0_PATH);
			}
		  	return -1;
		}
	}
	else {
		lseek(aw_state_fd,0,SEEK_SET);
		if((n = read(aw_state_fd,buf,1)) < 0){
			vrrp_syslog_error("inst%d read file %s error!\n", profile, HAD_SYSTEM_STARTUP_STATE0_PATH);
				return -1;	
		}
		if(HAD_SYSTEM_STARTUP_STATE0_FLAG == buf[0]) {
			vrrp_syslog_info("inst%d check %d times found leave boot stage!\n", profile, count[profile]);
			g_boot_check_flag = 0;
			close(aw_state_fd);
			aw_state_fd = -1;
			return 0;
		}
		else {
			vrrp_syslog_info("inst%d check %d times found in boot stage!\n", profile, count[profile]);
			return 1;
		}
	}

}

void had_state_thread_main(void){
  	int				i = 0, result = 0, count[MAX_HANSI_PROFILE] = {0};
    vrrp_rt*       vrrp = NULL;
	hansi_s*   hansi = NULL;
	
	vrrp_debug_tell_whoami("stateMachine", 0);
	while( 1 ){
		if(!global_enable){
                        had_sleep_time();
		        continue;
		}
		i = 0;
		for(;i<MAX_HANSI_PROFILE;i++){				
			if (
				#if 1
				/* for debug double master */
				(VRRP_SERVICE_ENABLE == service_enable[i]) &&
				#endif
				(NULL != g_hansi[i]) &&
				(NULL != (vrrp = g_hansi[i]->vlist))) {
				pthread_mutex_lock(&StateMutex); 
				hansi = g_hansi[i];
				if((NULL == hansi)||(vrrp->vrid != hansi->vrid)){						
					pthread_mutex_unlock(&StateMutex);	
					break;
				}
				/* check boot stage */
				if(g_boot_check_flag) {
					++count[i];
					result = had_check_boot_status(i);
					if(result < 0) { /* error condition */
						if(0 == (count[i] % HAD_BOOT_CHECK_MSG_HZ)) {
							vrrp_syslog_error("inst%d state thread check %d times in boot stage error %d\n", \
												i, count[i] ,result);
						}
						pthread_mutex_unlock(&StateMutex);
						had_sleep_time();
						continue;
					}
					else if(1 == result) { /* in boot stage */
						vrrp_syslog_info("inst%d state thread wait %d times in boot stage!\n", i, count[i]);
						pthread_mutex_unlock(&StateMutex);
						had_sleep_time();
						continue;
					}
				}
				
				vrrp_syslog_dbg("g_hansi %d vrrp %d loop,state %s\n",
								i,
								vrrp->vrid,
								(VRRP_STATE_INIT == vrrp->state) ? "INIT" :
								(VRRP_STATE_MAST == vrrp->state) ? "MAST" :
								(VRRP_STATE_BACK == vrrp->state) ? "BACK" :
								(VRRP_STATE_DISABLE == vrrp->state) ? "DISALBE" : " ");
	 			switch( vrrp->state){
					case VRRP_STATE_INIT:							
							had_state_init(vrrp);	
							break;
					case VRRP_STATE_LEARN:	
							had_state_learn_loop(vrrp);	
							break;
					case VRRP_STATE_BACK:	
							had_state_back_loop(vrrp);
							break;
					case VRRP_STATE_TRANSFER:	
						    had_state_transfer_loop(vrrp);
							break;		
					case VRRP_STATE_MAST:	
							had_state_mast_loop(vrrp);
						    break;
					case VRRP_STATE_DISABLE:
						    had_state_disable_loop(vrrp);
						    break;							
					default:                
							break;
				} 
				pthread_mutex_unlock(&StateMutex);	
				had_sleep_time();
			}
		}
	}
}

/*
 *******************************************************************************
 *had_wid_init()
 *
 *  DESCRIPTION:
 *		use vrrp instance no splice string
 *			global_wid_dbusname
 *			global_wid_bak_objpath
 *			global_wid_bak_interfce,
 *		for send dbus message to wid
 *
 *  INPUTS:
 * 	 	NULL
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *
 *******************************************************************************
 */
void had_wid_init
(
	void
)
{
int local = 0;//same as had, must be remote wid
#if 0
	sprintf(global_wid_dbusname,
			"%s%d",
			WID_DBUS_BUSNAME, global_current_instance_no);

	sprintf(global_wid_bak_objpath,
			"%s%d%s",
			WID_BAK_OBJPATH_PREFIX, global_current_instance_no, WID_BAK_OBJPATH_SUFFIX);
	sprintf(global_wid_bak_interfce,
			"%s%d%s",
			WID_BAK_INTERFACE_PREFIX, global_current_instance_no, WID_BAK_INTERFACE_SUFFIX);
#else
	sprintf(global_wid_dbusname,
			"%s%d_%d",
			WID_DBUS_BUSNAME,local, global_current_instance_no);

	sprintf(global_wid_bak_objpath,
			"%s%d_%d%s",
			WID_BAK_OBJPATH_PREFIX,local, global_current_instance_no, WID_BAK_OBJPATH_SUFFIX);
	sprintf(global_wid_bak_interfce,
			"%s%d_%d%s",
			WID_BAK_INTERFACE_PREFIX,local, global_current_instance_no, WID_BAK_INTERFACE_SUFFIX);
#endif
	vrrp_syslog_dbg("dbus name [%s]\n", global_wid_dbusname);
	vrrp_syslog_dbg("obj path  [%s]\n", global_wid_bak_objpath);
	vrrp_syslog_dbg("interface [%s]\n", global_wid_bak_interfce);
	return;
}

/**
 * check if eag(portal) process has started completely.
 * now we use 'ps -ef | grep eag' format to indicate 
 * the corresponding process is running or not
 */
int had_check_eag_started
(
	unsigned int profileId
)
{

	int ret = 0;
	int fd = -1;
	int iCnt = 0;	
	char commandBuf[VRRP_SYS_COMMAND_LEN] = {0};
	char readBuf[4] = {0};

	/* check if eag(portal) is running or not 
	 * with following shell command:
	 *  'ps -ef | grep \"eag\" | wc -l'
	 * if count result gt 1, running, else not running.
	 */
	sprintf(commandBuf, "sudo ps auxww | grep \"eag\" | grep -v grep |wc -l > /var/run/had%d.eag",
						profileId);
	ret = system(commandBuf);
	if (ret) {
		vrrp_syslog_error("Hansi instance %d check eag process failed!\n", profileId);
		return VRRP_INST_EAG_CHECK_FAILED;
	}

	/* get the process */
	memset(commandBuf, 0, VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "/var/run/had%d.eag", profileId);
	if ((fd = open(commandBuf, O_RDONLY))< 0) {
		vrrp_syslog_error("Hansi instance %d check eag threads count failed!\n", profileId);
		return VRRP_INST_EAG_CHECK_FAILED;
	}

	memset(readBuf, 0, 4);
	read(fd, readBuf, 4);
	iCnt = strtoul(readBuf, NULL, 10);
	vrrp_syslog_dbg("Hansi %d check eag process thread count %d\n",
					profileId, iCnt);

	if (iCnt > 0) {
		ret = VRRP_INST_EAG_CREATED;
	}
	else {
		ret = VRRP_INST_EAG_NO_CREATED;
	}

	/* release file resources */
	close(fd);

	memset(commandBuf, 0, VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "rm /var/run/had%d.eag", profileId);
	system(commandBuf);

	return ret;
}

/*
 *******************************************************************************
 *had_hmd_init()
 *
 *  DESCRIPTION:
 *		check eag(hmd) process state,
 *		if exist, set global_hmd 1, and notify eag(hmd) 
 *		when STATE MACHINE change state.
 *
 *  INPUTS:
 * 	 	NULL
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *
 *******************************************************************************
 */
void had_hmd_init
(
	void
)
{
	int ret = VRRP_INST_EAG_CHECK_FAILED;
	
	ret = had_check_eag_started(global_current_instance_no);
	if (VRRP_INST_EAG_CREATED == ret) {
		vrrp_syslog_dbg("eag service have enabled.\n");
		/* [1] set global portal flag */
		global_hmd = 1;
		/* [2] send message to eag, tell had instance no. */

	}else if (VRRP_INST_EAG_NO_CREATED == ret) {
		vrrp_syslog_dbg("eag service not enabled.\n");
	}else {
		vrrp_syslog_warning("check eag service failed.\n");
	}

	return;
}


/*
 *******************************************************************************
 *had_portal_init()
 *
 *  DESCRIPTION:
 *		check eag(portal) process state,
 *		if exist, set global_portal 1, and notify eag(portal) 
 *		when STATE MACHINE change state.
 *
 *  INPUTS:
 * 	 	NULL
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *
 *******************************************************************************
 */
void had_portal_init
(
	void
)
{
	int ret = VRRP_INST_EAG_CHECK_FAILED;
	
	ret = had_check_eag_started(global_current_instance_no);
	if (VRRP_INST_EAG_CREATED == ret) {
		vrrp_syslog_dbg("eag service have enabled.\n");
		/* [1] set global portal flag */
		global_protal = 1;
		/* [2] send message to eag, tell had instance no. */

	}else if (VRRP_INST_EAG_NO_CREATED == ret) {
		vrrp_syslog_dbg("eag service not enabled.\n");
	}else {
		vrrp_syslog_warning("check eag service failed.\n");
	}

	return;
}

/****************************************************************
 NAME	: main					00/02/06 08:48:02
 AIM	:
 REMARK	:
****************************************************************/
int main(int argc, char** argv)
{
    int ret = 0;

	#if 0
	unsigned int i = 0;
	unsigned int process_count = VRRP_CHILD_PROCESS_CNT;
	
	/* parent process allowed to fork, child process prohibit fork action. */
	unsigned int allow_fork_flag = VRRP_FORK_FLG_ALLOW;

	pid_t current = 0;
	#endif

  	my_pid = getpid();

	#if 1
	if (2 > argc || argc > 2) {
		vrrp_syslog_error("parameters number error\n");
		exit(1);
	}
	
	global_current_instance_no = strtoul((char *)argv[1], NULL, 0);
	if ((0 > global_current_instance_no) ||
		(VRRP_INSTANCE_CNT < global_current_instance_no)){
		vrrp_syslog_error("instance no [%d] illegal or out of valid range[0~%d]\n",
						global_current_instance_no, VRRP_INSTANCE_CNT);
		exit(1);
	}
	vrrp_syslog_dbg("start vrrpd pid %d\n", my_pid);

	#else
	/* parent process allowed to fork, child process prohibit fork action. */
	unsigned int allow_fork_flag = VRRP_FORK_FLG_ALLOW;
	pid_t pid = 0;

	if (VRRP_FORK_FLG_ALLOW == allow_fork_flag) {
		if ((pid = fork()) < 0) {
			vrrp_syslog_error("create child process faild.\n");
			exit(1);

		}else if (pid > 0) {
			/* parent process */
			vrrp_syslog_dbg("create child process success, parent process return.\n");
			exit(0);
		}else {
			/* child process */
			#if 0
			signal(SIGHUP, vrrp_sig_hup);	/* satablish signal handler */
			kill(getpid(), SIGTSTP);		/* stop ourself */
			#endif
			allow_fork_flag = VRRP_FORK_FLG_PROHIBIT;
			vrrp_syslog_dbg("start vrrpd pid %d instance no %d\n",
							getpid(), global_current_instance_no);
			
			vrrp_debug_tell_whoami("main", 0);
			ret = had_init();
		}
	}
	#endif

	#if 0
	/* init value of instace no,
	 * the value of parent process is
	 *		(process_count + 1)
	 */
	global_current_instance_no = 1;

	vrrp_syslog_dbg("start vrrpd pid %d\n", my_pid);


	for (i = 0; i < process_count; i++) {
		if (VRRP_FORK_FLG_ALLOW == allow_fork_flag) {
			current = fork();
			if (0 != current) {
				/* parent process */
				global_current_instance_no++;
				continue;
			}
			else {
				/* child process */
				allow_fork_flag = VRRP_FORK_FLG_PROHIBIT;
			}
		}
	}

	

#endif
	vrrp_syslog_dbg("start vrrpd pid %d instance no %d\n",
					getpid(), global_current_instance_no);

	vrrp_debug_tell_whoami("main", 0);
	
	ret = had_init();

	return 0;
}

int had_init
(
	void
)
{
    int ret = 0;
#ifdef VRRP_GLOBAL_SOCKET_FD
	vrrp_socket_inet_dgram_0_fd = socket(AF_INET, SOCK_DGRAM, 0);
	vrrp_syslog_event("vrrp create global socket(inet,dgram,0), fd %d\n",\
		vrrp_socket_inet_dgram_0_fd);
#endif

	had_splice_objpath_string(VRRP_DBUS_OBJPATH,
								global_current_instance_no,
								global_current_objpath);
	had_splice_objpath_string(VRRP_DBUS_BUSNAME,
								global_current_instance_no,
								global_cli_dbusname);	

	had_wid_init();

	ret = had_dbus_init2();
	if(ret) {
		vrrp_syslog_warning("init vrrp dbus for notify error!\n");
		return VRRP_RETURN_CODE_ERR;
	}
	
	ret = had_dbus_init();
	if(0 != ret) {
		vrrp_syslog_warning("init vrrp dbus for cli error!\n");
		return VRRP_RETURN_CODE_ERR;
	}
	
	hansi_init();
	ret = had_profile_create(global_current_instance_no,global_current_instance_no);	
	/*dbus thread */
	dbus_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dbus_thread_attr);
	ret = pthread_create(dbus_thread,&dbus_thread_attr,had_dbus_thread_main,NULL);
    if(0 != ret){
       vrrp_syslog_error("create dbus thread failed in had!\n");
	   return VRRP_RETURN_CODE_ERR;
	}
	/*dbus packet receive*/
	/*
	packet_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&packet_thread_attr);
	ret = pthread_create(packet_thread,&packet_thread_attr,vrrp_packet_thread_main,NULL);
    if(0 != ret){
	   pthread_join(*dbus_thread,NULL);
	   free(dbus_thread);
       vrrp_syslog_error("create packets thread failed in had!\n");
	   return VRRP_RETURN_CODE_ERR;
	}
	*/
	packet_thread2 = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&packet_thread_attr2);
	ret = pthread_create(packet_thread2,&packet_thread_attr2,(void*)had_packet_thread2_main,NULL);
    if(0 != ret){
	   pthread_join(*dbus_thread,NULL);
	   free(dbus_thread);
       vrrp_syslog_error("create packets thread failed in had!\n");
	   return VRRP_RETURN_CODE_ERR;
	}

	state_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&state_thread_attr);
	ret = pthread_create(state_thread,&state_thread_attr,(void*)had_state_thread_main,NULL);
    if(0 != ret){
	   pthread_join(*dbus_thread,NULL);
	   pthread_join(*packet_thread2,NULL);
	   free(dbus_thread);
	   free(packet_thread2);
       vrrp_syslog_error("create state thread failed in had!\n");
	   return VRRP_RETURN_CODE_ERR;
	}
	link_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&link_thread_attr);
	ret = pthread_create(link_thread,&link_thread_attr,(void*)had_link_thread_main,NULL);
    if(0 != ret){
	   pthread_join(*dbus_thread,NULL);
	   pthread_join(*packet_thread2,NULL);
	   free(dbus_thread);
	   free(packet_thread2);
       vrrp_syslog_error("create link change thread failed in had!\n");
	   return VRRP_RETURN_CODE_ERR;
	}
	arp_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&arp_thread_attr);
	ret = pthread_create(arp_thread,&arp_thread_attr,(void*)had_arp_thread_main,NULL);
    if(0 != ret){
	   pthread_join(*dbus_thread,NULL);
	   pthread_join(*packet_thread2,NULL);
	    pthread_join(*link_thread,NULL);
	   free(dbus_thread);
	   free(packet_thread2);
	   free(link_thread);
       vrrp_syslog_error("create arp thread failed in had!\n");
	   return VRRP_RETURN_CODE_ERR;
	}
#if 0
	timesyn_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&timesyn_thread_attr);
	pthread_create(timesyn_thread,&timesyn_thread_attr,(void*)had_timesyn_thread_main,NULL);
#else
	timer_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&timer_thread_attr);
	ret = pthread_create(timer_thread,&timer_thread_attr,(void*)vrrp_timer_thread_main,NULL);
#endif

    if(0 != ret){
	   pthread_join(*dbus_thread,NULL);
	   pthread_join(*packet_thread2,NULL);
	   pthread_join(*link_thread,NULL);
	   pthread_join(*arp_thread,NULL);
	   free(dbus_thread);
	   free(packet_thread2);
	   free(link_thread);
	   free(arp_thread);
       vrrp_syslog_error("create time syn thread failed in had!\n");
	   return VRRP_RETURN_CODE_ERR;
	}	
    vrrp_syslog_dbg("******create ND thread success in had!******\n");

	ndisc_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&ndisc_thread_attr);
	ret = pthread_create(ndisc_thread,&ndisc_thread_attr,(void*)had_ndisc_thread_main,NULL);
    if(0 != ret){
	   pthread_join(*dbus_thread,NULL);
	   pthread_join(*packet_thread2,NULL);
	   pthread_join(*link_thread,NULL);
	   pthread_join(*arp_thread,NULL);
	   pthread_join(*timer_thread,NULL);

	   free(dbus_thread);
	   free(packet_thread2);
	   free(link_thread);
	   free(arp_thread);
	   free(timer_thread);
       vrrp_syslog_error("create ndisc thread failed in had!\n");
	   return VRRP_RETURN_CODE_ERR;
	}
	
	had_portal_init();
	had_hmd_init();     //book add
	
	if(had_netlink_init() < 0)
    {
        vrrp_syslog_error("Fail had_netlink_init\n");
	} 
	
	pthread_join(*dbus_thread,NULL);
	pthread_join(*packet_thread2,NULL);
	pthread_join(*state_thread,NULL);
	pthread_join(*link_thread,NULL);
	pthread_join(*arp_thread,NULL);
#if 0
	pthread_join(*timesyn_thread, NULL);
#else
	pthread_join(*timer_thread,NULL);
    pthread_join(*ndisc_thread,NULL);

#endif
	
	return VRRP_RETURN_CODE_OK;
}

/*********************************************************
 * write heartbeat ifname to file /var/run/had/had[profile]/heartbeat
 * RETURN:  0  - success ,  1  - failed
 *********************************************************/
int hansi_write_heartbeat_ifname_to_file(unsigned int profile, int writeIn){
	char * filePath[128] = {0};
	sprintf(filePath, "%s%d", VRRP_IFNAME_FILES_PATH, profile);
	return hansi_write_string_to_file(writeIn ? global_ht_ifname : "", filePath, VRRP_HEARTBEAT_IFNAME_FILENAME);
}

/********************************************************
 * write hansi state to file /var/run/had/hand[profile]/state
 * RETURN:  0  - success ,  1  - failed
 ********************************************************/
int hansi_write_hansi_state_to_file(unsigned int profile, char *stateStr)
{
	char * filePath[128] = {0};
	sprintf(filePath, "%s%d", VRRP_IFNAME_FILES_PATH, profile);
	if(!stateStr){
		vrrp_syslog_error("write hansi %d state to file failed, state is null pointer\n", profile);
		return -1;
	}
	return hansi_write_string_to_file(stateStr, filePath, VRRP_HANSI_STATE_FILENAME);
}

/*****************
  write ifname to a file 
  if success return 0
  else return -1
  *****************/
int hansi_write_string_to_file(char * strbuf, char * path, char * filename)
{
	char cmd[64] = {0};
	char pathfile [128] = {0};
	int ret = 0;
	if((!strbuf)||(!path)||(!filename)){
		vrrp_syslog_error("%s is null pointer when write string %s to file %s%s\n", \
			(strbuf ? (path ? "filename" : filename ? "path" : "path and filename") : \
			(path ? "string and filename" : filename ? "string and path" : "string,path and filename")),\
			strbuf ? strbuf : "", path ? path : "", filename ? filename : "?");
		return -1;
	}
	if(strlen(path) + strlen("mkdir -p \n") > 64){
		vrrp_syslog_error("path %s is too long when write string %s to file %s%s\n",\
			path, strbuf, path, filename);
		return -1;
	}
	sprintf(cmd, "mkdir -p %s\n", path);
	ret = system(cmd);
	if(ret){
		vrrp_syslog_error("create directory failed when write string %s to file %s%s", \
			strbuf, path, filename);
		return -1;
	}
	if(strlen(path) + strlen(filename) + strlen("/") > 128){
		vrrp_syslog_error("filename %s is too long write string %s to file %s%s\n", \
			filename, strbuf, path, filename);
		return -1;
	}
	sprintf(pathfile, "%s/%s", path, filename);
	int fd = open(pathfile, O_CREAT|O_WRONLY|O_TRUNC);
	if(fd<0){
		vrrp_syslog_error("open file %s failed ,err: %s \n",pathfile,strerror(errno));
		return -1;
	}
	if(0 > write(fd, strbuf, strlen(strbuf)+1)){
		vrrp_syslog_error("write failed when write string %s to file %s ,err: %s \n",strbuf, path,strerror(errno));
		close(fd);
		return -1;
	}
	sprintf(cmd, "chmod 777 %s -R \n", path);
	system(cmd);
	vrrp_syslog_dbg("write string %s to file %s success\n",strbuf, pathfile);
	close(fd);
	return 0;	
}


/************************************************************
 * hansi_read_string_from_file
 *
 * DESCRIPTION:
 * 		read a string from a file
 * INPUT:
 *		path: string  -  file path , eg. "/var/run/had"  
 *		filename: string - file name , eg. "heartbeat"
 *		buflen: int  - the max len we want to read
 * OUTPUT:
 *		strbuf : string - the string we get
 * RETURN:
 *		0  -  success
 *		1  -  failed
 * NOTE:
 *		if the buflen input is large then file'len,we would read as file'len
 *
 **************************************************************/
int hansi_read_string_from_file(char * strbuf, char * path, char * filename, int buflen){
	char pathfile [128] = {0};
	int ret = 0;
	if((!strbuf)||(!path)||(!filename)){
		vrrp_syslog_error("%s is null pointer when read string %s from file %s%s\n", \
			(strbuf ? (path ? "filename" : filename ? "path" : "path and filename") : \
			(path ? "string and filename" : filename ? "string and path" : "string,path and filename")),\
			strbuf ? strbuf : "", path ? path : "", filename ? filename : "?");
		return -1;
	}	
	sprintf(pathfile, "%s/%s", path, filename);
	int fd = open(pathfile, O_RDONLY);
	if(fd < 0){
		vrrp_syslog_error("open filt %s failed when read string from file\n", pathfile);
		return -1;
	}
	if(0 > read(fd, strbuf, buflen)){
		vrrp_syslog_error("read failed when read string from file %s ,err: %s \n", path,strerror(errno));
		close(fd);
		return -1;
	}
	vrrp_syslog_dbg("read string %s from file %s success\n",strbuf, pathfile);
	close(fd);
	return 0;	
}
/************************************************************
 * hansi_mac_addr_parse
 *
 * DESCRIPTION:
 * 		read a string from a file
 * INPUT:
 *		macStr : string - the string of mac address format as  ff:ff:ff:ff:ff:ff
 *										or format as ffffffffffff
 * OUTPUT:
 *		macAddr : char[ETH_ALEN] - the mac address we get
 * RETURN:
 *		0  -  success
 *		1  -  failed
 * NOTE:
 *		NONE
 *
 **************************************************************/

int hansi_mac_addr_parse(unsigned char *macStr, unsigned char *macAddr){
	int i = 0;
	if(!macStr || !macAddr){
		vrrp_syslog_error("%s null pointer when mac addr parse\n",
			macStr ? "macAddr" : macAddr ? "macStr" : "macStr and macAddr");
		return -1;
	}
	if(strlen("ffffffffffff") <= strlen(macStr)){/*for format as ffffffffffff and ff:ff:ff:ff:ff:ff*/
		unsigned char cur = 0,value = 0;
		unsigned char * input = macStr;
		for(i = 0; i <6;i++) {			
			value = 0;
			cur = *(input++);
			if(cur == ':') {
				i--;
				continue;
			}
			if((cur >= '0') &&(cur <='9')) {
				value = cur - '0';
			}
			else if((cur >= 'A') &&(cur <='F')) {
				value = cur - 'A';
				value += 0xa;
			}
			else if((cur >= 'a') &&(cur <='f')) {
				value = cur - 'a';
				value += 0xa;
			}
			else{
				vrrp_syslog_error("inavaliable mac address string %s,char %c ascii %d\n", macStr,cur,cur);
				return -1;
			}
			macAddr[i] = value;
			value = 0;
			cur = *(input++);	
			if((cur >= '0') &&(cur <='9')) {
				value = cur - '0';
			}
			else if((cur >= 'A') &&(cur <='F')) {
				value = cur - 'A';
				value += 0xa;
			}
			else if((cur >= 'a') &&(cur <='f')) {
				value = cur - 'a';
				value += 0xa;
			}
			else{
				vrrp_syslog_error("inavaliable mac address string %s,char %c ascii %d\n", macStr,cur,cur);
				return -1;
			}
			macAddr[i] = (macAddr[i]<< 4)|value;
		}
	}
	else{
		vrrp_syslog_error("unrecognizable mac string format: %s\n", macStr);
		return -1;
	}
	return 0;
}
#if 0
int parse_int_ve(char* str, unsigned int* slotid, unsigned int *vlanid){
	char c;
	char *tmp = NULL;
	char *endptr = NULL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*slotid= strtoul(str,&endptr,10);
		
		if(endptr[0] == '.'){
			tmp = endptr+1;
			*vlanid= strtoul(tmp,&endptr,10);//regard the ve interface name as right
			return 1;
		}
		else if((endptr[0] == 'f')||(endptr[0] == 's'))
			return 2;
	}
	else
		return WID_UNKNOWN_ID;
}

int check_ve_interface(char *ifname, char *name){
	
	int sockfd;
	unsigned int slotid = 0;
	unsigned int vlanid = 0;
	struct ifreq	ifr;
	if (0 != strncmp(ifname, "ve", 2)){
		vrrp_syslog_dbg("It's not ve interface\n");
		sprintf(name,"%s",ifname);
		return 0;
	}
	else{
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name)); 		
		
		if(parse_int_ve(ifname+2,&slotid,&vlanid)== 1)
		{
			vrrp_syslog_dbg("slotid = %d\n",slotid);
			vrrp_syslog_dbg("vlanid = %d\n",vlanid);

			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				vrrp_syslog_error("SIOCGIFINDEX error\n");

				//convert to new ve name
				if(slotid < 10)
					sprintf(name,"ve0%df1.%d",slotid,vlanid);
				else if(slotid >= 10)
					sprintf(name,"ve%df1.%d",slotid,vlanid);
				vrrp_syslog_dbg("ve name is %s\n",name);

				memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
				strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name)); 		
				if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
					vrrp_syslog_error("SIOCGIFINDEX error\n");
					close(sockfd);
					return -1;	//the new ve interface doesn't exist
				}		
				close(sockfd);
				return 0;	//the new ve interface exists
			}
			else{
				sprintf(name,"%s",ifname);
				close(sockfd);
				return 0;//the old ve interface exists
			}
		}
		else if(parse_int_ve(ifname+2,&slotid,&vlanid)== 2)
		{
			vrrp_syslog_dbg("slotid = %d\n",slotid);
		
			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				vrrp_syslog_dbg("SIOCGIFINDEX error\n");
				close(sockfd);
				return -1;	//the new ve interface doesn't exist
			}		
			sprintf(name,"%s",ifname);
			close(sockfd);
			return 0;	//the new ve interface exists
		}
		else{
			vrrp_syslog_dbg("the ve name is wrong\n");
			close(sockfd);
			return -1;
		}
	}
		
}


#else
int parse_int_ve(char* str, unsigned int* slotid, unsigned int *vlanid, unsigned int *vlanid2,char *cid, unsigned int *port){ /*fengwenchao add "vlanid2" for axsszfi-1506*/
	char c;
	char *tmp = NULL;
	char *endptr = NULL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*slotid= strtoul(str,&endptr,10);
		
		if(endptr[0] == '.'){
			tmp = endptr+1;
			*vlanid= strtoul(tmp,&endptr,10);
			/*fengwenchao modify 20130325 for axsszfi-1506 begin*/
			if(endptr[0] == '.')
			{
				tmp = endptr+1;
				*vlanid2 = strtoul(tmp,&endptr,10);
			if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return ASD_UNKNOWN_ID;
			}
			else if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return ASD_UNKNOWN_ID;
			/*fengwenchao modify end*/
			return 1;
		}
		else if((endptr[0] == 'f')||(endptr[0] == 's')){
			*cid = endptr[0];
			tmp = endptr+1;
			*port = strtoul(tmp,&endptr,10);
			/*fengwenchao modify 20130325 for axsszfi-1506 begin*/
			if(endptr[0] == '.'){
				tmp = endptr+1;
				*vlanid= strtoul(tmp,&endptr,10);
				if(endptr[0] == '.')
				{
					tmp = endptr+1;
					*vlanid2 = strtoul(tmp,&endptr,10);
				if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return ASD_UNKNOWN_ID;
				}
				else if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return ASD_UNKNOWN_ID;
				return 2;
			}
			else if((endptr[0] == '\0')||(endptr[0] == '\n'))
				return 2;
			/*fengwenchao modify end*/
			return ASD_UNKNOWN_ID;
		}
	}
	
	return ASD_UNKNOWN_ID;
}

int check_ve_interface(char *ifname, char *name){
	
	int sockfd;
	unsigned int slotid = 0;
	unsigned int vlanid = 0;
	unsigned int vlanid2 = 0;//fengwenchao add 20130325 for axsszfi-1506 
	unsigned int port = 0;
	char cpu = 'f';
	char *cpu_id = &cpu; 
	struct ifreq	ifr;
	if (0 != strncmp(ifname, "ve", 2)){
		sprintf(name,"%s",ifname);
		vrrp_syslog_dbg("interface name is %s\n",name); 
		return 0;
	}
	else{
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if( sockfd<0 ){
			vrrp_syslog_error("create socket error\n");
			return -1;
		}
		strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name)); 		
		
		if(parse_int_ve(ifname+2,&slotid,&vlanid,&vlanid2,cpu_id,&port)== 1)//fengwenchao add "vlanid2" for axsszfi-1506 
		{
			vrrp_syslog_dbg("slotid = %d,vlanid = %d\n",slotid,vlanid); 
			vrrp_syslog_dbg("cpu_id = %c,port = %d\n",*cpu_id,port); 

			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				vrrp_syslog_dbg("SIOCGIFINDEX error\n"); 

				//convert to new ve name
				/*fengwenchao modify "vlanid2" for axsszfi-1506 begin*/
				if(slotid < 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve0%df1.%d",slotid,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve0%df1.%d.%d",slotid,vlanid,vlanid2);
				}
				else if(slotid >= 10)
				{	
					if(vlanid2  == 0)
						sprintf(name,"ve%df1.%d",slotid,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve%df1.%d.%d",slotid,vlanid,vlanid2);
				}
				/*fengwenchao modify end*/
				vrrp_syslog_dbg("ve name is %s\n",name); 

				memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
				strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name)); 		
				if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
					vrrp_syslog_dbg("SIOCGIFINDEX error\n"); 
					close(sockfd);
					return -1;	//the new ve interface doesn't exist
				}		
				close(sockfd);
				return 0;	//the new ve interface exists
			}
			else{
				sprintf(name,"%s",ifname);
				vrrp_syslog_dbg("old ve name is %s\n",name); 
				close(sockfd);
				return 0;//the old ve interface exists
			}
		}
		else if(parse_int_ve(ifname+2,&slotid,&vlanid,&vlanid2,cpu_id,&port)== 2)//fengwenchao add "vlanid2" for axsszfi-1506 
		{
			vrrp_syslog_dbg("slotid = %d,vlanid = %d\n",slotid,vlanid); 
			vrrp_syslog_dbg("cpu_id = %c,port = %d\n",*cpu_id,port); 

			if(vlanid == 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d",slotid,*cpu_id,port);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d",slotid,*cpu_id,port);
			}
			else if(vlanid > 0){
				/*fengwenchao modify "vlanid2" for axsszfi-1506 begin*/
				if(slotid < 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve0%d%c%d.%d",slotid,*cpu_id,port,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve0%d%c%d.%d.%d",slotid,*cpu_id,port,vlanid,vlanid2);
				}
				else if(slotid >= 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve%d%c%d.%d",slotid,*cpu_id,port,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve%d%c%d.%d.%d",slotid,*cpu_id,port,vlanid,vlanid2);
				}
				/*fengwenchao modify end*/
			}
			vrrp_syslog_dbg("ve name is %s\n",name); 
		
			memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
			strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));		
			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				vrrp_syslog_dbg("SIOCGIFINDEX error\n"); 
				close(sockfd);
				return -1;	//the new ve interface doesn't exist
			}		

			close(sockfd);
			return 0;	//the new ve interface exists
		}
		else{
			vrrp_syslog_dbg("the ve name is wrong\n"); 
			close(sockfd);
			return -1;
		}
	}
		
}
#endif
#ifdef __cplusplus
}
#endif
