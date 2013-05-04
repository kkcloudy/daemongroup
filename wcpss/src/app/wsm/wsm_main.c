/******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
*******************************************************************************

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
*******************************************************************************
* wsm_main.c
*
*
* DESCRIPTION:
*  wsm module main routine. wsm module tramsmits CAPWAP datagram between WTPs 
*  and wifi.ko. WSM received CW(CAPWAP) datagram from WTPs and changed it to 
*  Ethernet II type frame, send to wifi.ko last. Oppositely, WSM received 
*  Ethernet II datagram from wifi.ko and changed it to CW datagram and send to
*  WTPs last. The WTP-BSS-STA relation tables that received from ASD and WID,
*  WSM assembled frame depend on it.
*
* DATE:
*  2008-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* VERSION:
*  V2.0: Depend on V1.0 by luoxun <2008-04-29>
*
* CHANGE LOG:
*  2009-10-13  <guoxb> Support DBus
*  2009-11-04  <guoxb> Add watchdog dbus switch.
*  2009-11-06  <guoxb> Add CAPWAP IPv6 data channel
*  2009-11-13  <guoxb> Rewrite function WSM_recv_DataMsg()
*  2009-11-21  <guoxb> Add N+1 hansi backup.
*  2009-11-26  <guoxb> Adjust log print level.
*  2009-12-11  <guoxb> Add Inter-AC roamging.
*  2009-12-21  <guoxb> Add WTP access AC via NAT.
*  2009-12-23  <guoxb> 1.Inter-AC roaming function only exist in instance0
*                                2. Modified sched_setaffinity in wsm_recv_wtpmsg
*                                3. Modified wsm log initialize function.
*  2009-12-28  <guoxb> Review source code.
*  2010-02-09  <guoxb> Modified files name, copyright etc.
*  2010-03-22  <guoxb> Add mutex for dynamic interface reg/unreg
*
******************************************************************************/

/* System header files */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sched.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <netdb.h>

/* CAPWAP public library header files */
#include "CWCommon.h"
#include "CWProtocol.h"
#include "CWNetwork.h"
#include "CWLog.h"
#include "CWErrorHandling.h"

/* wcpss header files */
#include "wcpss/waw.h"

/* WSM private header files */
#include "wsm_types.h"
#include "wsm_main_tbl.h"
#include "wsm_frame_transform.h"
#include "wsm_wifi_bind.h"
#include "wsm_shared_mem.h"
#include "wsm_dbus.h"


/* CAPWAP data tunnel port number */
#define CW_STD_DATA_PORT			5247
#define CW_TMP_DATA_PORT			8888
#define STD_DATA_PORT_STR			"5247"
#define TMP_DATA_PORT_STR			"8888"

/* Inter-AC roaming port and shared memory offset */
#define CW_RAW_AC_DATA_PORT		8889
#define CW_ROAM_AC_DATA_PORT		8890
#define RAW_AC_OFFSET				54
#define ROAM_AC_OFFSET				(54 - sizeof(roam_head_t))
#define ROAM_AC_DATA_OFFSET		54
#define THIS_IS_ROAM_AC			0	
#define THIS_IS_RAW_AC				1

/* CAPWAP header length */
#define CAPWAP_HEAD_LEN			12
#define CAPWAP_HEAD_ASSIGN		8
#define IEEE802_11_HEAD_LEN		30

/* Message type that received from WTP */
#define WSM_DATA_ERR				0x0
#define WSM_DATA_TYPE				0x1
#define WSM_MGMT_TYPE				0x2
#define WSM_EAP_TYPE				0x3

/* Shared memory flag and offset */
#define WSM_FREE_BUFF				0
#define WSM_SEND_BUFF				1
#define WSM_RECVED_BUFF			2
#define WSM_BUFF_OFFSET			10

/* Send to wifi L3IF interface type */
#define BSS_AS_L3IF_UNI				6
#define BSS_AS_L3IF_MULTI			2
#define WLAN_AS_L3IF_UNI			5
#define WLAN_AS_L3IF_MULTI			1

/* Get mapped ipv4 addr from ipv6 addr */
#define GET_MAPPED_V4ADDR(ip,addr)	((ip)=(*((unsigned int*)&((addr)[12]))))

/* rwlock and mutex switch macro */
#ifdef __RWLOCK_TEST
	pthread_rwlock_t wsm_tables_rwlock;
#else
	pthread_mutex_t wsm_tables_rwlock;
#endif


/* Define socket file descriptors */
int iWTPsocket;
/* For supporting wsm data multi-port by Guo Xuebin, 2009-06-02 */
int iWTPNewSocket;
int iDATAsocket;
int iTABLEsocket;
int iASDsocket;
int iSTAPktsocket;
int iBSSPktsocket;
int iSTARoamingsocket;
int iWSMSplitMAC;
/* The local sockaddr length used to send packet to ASD */
int glen;
int gSockaddr_len;
/* Inter-AC roaming socket fd */
int roam_ac_fd;
int raw_ac_fd;
int rrm_rid = 0;
/* Inter-AC roaming, Roam-AC addr */
struct sockaddr_in roam_ac_addr;
/* Inter-AC roaming, Raw-AC addr */
struct sockaddr_in raw_ac_addr;
/* used for receiving DataMsg from ASD */
struct sockaddr_un WSM_Data_local_sockaddr;
/* used for receiving TableMsg from ASD or WID */
struct sockaddr_un WSM_Table_local_sockaddr;
/* used for sending packet to ASD */
struct sockaddr_un WSM_ASD_local_sockaddr;
/* used for sta pkt statistics */
struct sockaddr_un WSM_ASD_PKT_sockaddr;
/* used for bss pkt statistics */
struct sockaddr_un WSM_BSS_PKT_sockaddr;
/* used for bss pkt statistics */
struct sockaddr_un WID_sockaddr;
/* used for sta pkt statistics */
struct sockaddr_un ASD_Table_sockaddr;
/* used for sta roaming, ASD path */
struct sockaddr_un ASD_Roaming_sockaddr;
struct sockaddr_un STA_local_Roaming_sockaddr;

/* Used by Mainly thread for recv/send packet */
char DATAbuf[dot11_Max_Len];
char TABLEbuf[dot11_Max_Len];
CWProtocolMessage WTPmsg;
CWProtocolMessage DATAmsg;
CWProtocolMessage TABLEmsg;

/* For Split MAC Queue, when the first queue is full, then fill in the next 
    queue. 
*/
CWProtocolMessage toWifiBuff[WSM_QUEUE_NUM];
CWProtocolMessage fromWifiBuff[WSM_QUEUE_NUM];

/* WSM Mutexs */
pthread_mutex_t split_recv_mutex;
pthread_mutex_t split_send_mutex;
pthread_mutex_t sta_statistics_mutex;
pthread_mutex_t sta_statistics_update_mutex;
pthread_mutex_t if_reg_mutex;

/* Only Instance 0 exist Inter-AC roaming function */
unsigned char is_instance0;
/* VRID for N+1 backup */
unsigned int vrrid;

unsigned int slotid = 0;
unsigned int local = 1;

/* Registered interface count */
int g_sock_cnt;
/* For supporting wsm data multi-port by Guo Xuebin, 2009-06-02 */
fd_set gfds;
int maxfd;
/* WSM syslog switch */
unsigned char gLog_switch[LOG_TYPE_MAX];

/* WSM split mac data structure */
struct wsm_index_t gToWifiIndex[WSM_QUEUE_NUM] = {0};
struct wsm_data_t gToWifiData[WSM_QUEUE_NUM][WSM_QUEUE_LEN] = {0};

/* Total assoc STA count */
unsigned int total_sta_cnt = 0;
unsigned int total_wtp_cnt = 0;

/* Watch dog packet count */
unsigned long g_pkt_cnt = 0;
unsigned char tunnel_run = 0;
unsigned char dbus_wd_enable = 0;

/* WSM Tables */
WTP_Element WTPtable[HASHTABLE_SIZE];	
BSSID_BSSIndex_Element* BSSID_BSSIndex_table[HASHTABLE_SIZE/4];
WTPIP_WTPID_Element* WTPIP_WTPID_table[HASHTABLE_SIZE];

/* Inter-AC roaming table */
roam_tbl_t *roam_tbl[HASHTABLE_SIZE / 4];

/* Registered interface list header */
if_list_t *g_fd_list = NULL;


/* Shared Memory header pointer */
extern struct WSM_MMAP_HEAD *MM_Head;
extern struct mmap_head *toWifi_Head;
extern struct mmap_head *fromWifi_Head;
/* AC CoreMask */
extern unsigned short gCoreMask;
/* Wlan-BSS table */
extern struct WSM_WLANID_BSSID_TABLE WLANID_BSSID_Table[WIFI_BIND_MAX_NUM];

unsigned int g_wid_wsm_error_handle_state = WID_WSM_ERROR_HANDLE_STATE_DEFAULT;//wid wsm error handle state(0--disable;1--enable)

/* Invoked by CAPWAP public library,  it's no use but if you delete it, there 
    will be a compile error. 
*/
#define LOG_FILE_NAME  "wsm.log.txt"
char gLogFileName[]=LOG_FILE_NAME;
CWThreadMutex gFileMutex;
CWThreadMutex gCreateIDMutex;
int gMaxLogFileSize = 1000;
int gEnabledLog = 1;

 __inline__ int CWGetFragmentID()
 {
 	static int fragID = 0;
	int r;

	if(!CWThreadMutexLock(&gCreateIDMutex))
	{
		CWDebugLog("Error Locking a mutex");
	}
	r = fragID;
	if (fragID==CW_MAX_FRAGMENT_ID) 
		fragID=0;
	else 
		fragID++;

	CWThreadMutexUnlock(&gCreateIDMutex);

	return r;
}


/* Intrinsic functions declare */
void wsm_log_init(void);
int wsm_init(void);
int sock_init(const char *host, const char *serv, socklen_t *plen);
__inline__ void WSMLog(unsigned char type, char *format, ...);
void wsm_set_logswitch(unsigned char logtype, unsigned char new_state);
int parse_fromWTP_msg(int index, DataMsgHead *pdata, struct sockaddr_storage client_sock, int fd);
int sta_statistics_to_asd(int cnt, unsigned char* buff);
int WTPmsg_to_ASD(int index, int offset, DataMsgHead datamsghead);
int WTPmsg_to_Wifi(int index, unsigned char *buff);
CWBool ReceiveMessage(CWProtocolMessage *msgPtr, 
	CWProtocolTransportHeaderValues *pCAPWAPheadvalue, int sock, struct sockaddr *addrPtr);
void WSM_recv_WTP(void *p);
void WSM_recv_DataMsg(void);
void WSM_recv_TableMsg(void);
void WSM_recv_Wifi(void *p);
void sig_print(int sig);
void wsm_mutex_init(void);
void wsm_recv_wtpmsg(void);
int sta_collision_detection(TableMsg *p);
__inline__ void sta_roam_check(unsigned char* STAMAC, BSSID_BSSIndex_Element* bssid_bssindex_element);
void watch_dog(void);
int watch_dog_alert(int dump_cnt, int wsm_cnt);
void watch_dog_say_should_die(int dump_cnt, int wsm_cnt);
int vrrp_if_reg(vrrp_interface *p);
int write_pid_file(void);
__inline__ int ip_cmp(struct mixwtpip *p1, struct mixwtpip *p2);


/**
*
* WSM module main function, the function of WSM module is:
*   @1: Maintain some tables about WTPs BSSs and STAs for CW 
*          data channel.
*   @2: Maintain a CAPWAP Data channel between AC and WTPs.
*
* In this module, there mainly has 4 threads.
*
*   @Thread1: [WSM_recv_WTP] thread array. In this thread array, there
*                   has 16 threads, and every thread bind to the specific core.
*                   The function of there threads is receive packet from WTP.
*                   There has two types packets, one is the packets that should
*                   send to ASD, and other is Data Channel packet which should
*                   send to wifi.ko.
*
*   @Thread2: [WSM_recv_Wifi] thread array. In this thread array, they receive
*                   packets from wifi.ko by shared memory, after assemble the
*                   packets from 802.3 to 802.11, these packets would be send to
*                   WTPs.
*
*   @Thread3: [WSM_recv_TableMsg]. This thread mainly update Tables about
*                    WTPs, BSSs, STAs, Wlans.These tables would be used in assemble
*                    802.3 frame to 802.11 frame and authenticate the Stations are 
*                    authorization or not.
*                                                      
*   @Thread4: [WSM_recv_DataMsg]. This thread received messages from ASD,the
*                   data in this channel mainly was authentication data for STA association
*                   interact in prophase. 
*   @Thread5: [watch_dog]. This thread watch capwap datagram. If there has an error,
*                   restart wsm module.
*
*/
int main(int argc, char **argv)
{
	int thread_param[WSM_QUEUE_NUM];
	int tmp = 0;
	unsigned short core_flag = 1;
	CWThread thread_recv_WTP[WSM_QUEUE_NUM];
	CWThread thread_recv_Wifi[WSM_QUEUE_NUM];
	CWThread thread_recv_TableMsg;
	CWThread thread_recv_DataMsg;
	CWThread thread_recv_WTPMsg;
	CWThread thread_watch_dog;
	CWThread thread_dbus;

	/* Initialize wsm log */
	wsm_log_init();
#ifdef _DISTRIBUTION_	
	if(argc > 2){
		local =  atoi(argv[1]);
		//slotid =  atoi(argv[2]);
		vrrid =  atoi(argv[2]);
	}else if(argc != 1){
		printf("argc %d, something wrong\n",argc);
		//return 0;
	}
#else
	if (argc < 2)
	{
		vrrid = 0;
	}
	else
	{
		vrrid = atoi(argv[argc - 1]);
	}
#endif
	if (vrrid >= MAX_VRID)
	{
		WSMLog(L_EMERG, "Parameter VRID[%d] out of range.\n", vrrid);
		exit(-1);
	}
	
	for (tmp = 0; tmp < WSM_QUEUE_NUM; tmp++)
		thread_param[tmp] = tmp;


	/* WSM Initialize */
	if(wsm_init() < 0)
	{
		WSMLog(L_EMERG, "wsm_init failed.\n");
		exit(-1);
	}
	WSMLog(L_INFO, "Global variables initialize Successed.\n");
	
	/* WSM Tables Initialize */
	wsm_tbl_init();
	WSMLog(L_INFO, "Tables initialize Successed.\n");
	
	/* Start threads (recv/send) */
	if(!CWErr(CWCreateThread(&thread_recv_DataMsg, WSM_recv_DataMsg, NULL, 0))) 
	{
		WSMLog(L_EMERG, "Thread[WSM_recv_DataMsg] initialize failed.\n");
		exit(-1);
	}
	
	if(!CWErr(CWCreateThread(&thread_recv_TableMsg, WSM_recv_TableMsg, NULL, 0))) 
	{
		WSMLog(L_EMERG, "Thread[WSM_recv_TableMsg] initialize failed.\n");
		exit(-1);
	}
#ifdef __wsm__

	if(!CWErr(CWCreateThread(&thread_recv_WTPMsg, wsm_recv_wtpmsg, NULL, 0))) 
	{
		WSMLog(L_EMERG, "Thread[WSM_recv_TableMsg] initialize failed.\n");
		exit(-1);
	}
#endif	
	if(!CWErr(CWCreateThread(&thread_watch_dog, watch_dog, NULL, 0))) 
	{
		WSMLog(L_EMERG, "Thread[thread_watch_dog] initialize failed.\n");
		exit(-1);
	}


	if (!CWErr(CWCreateThread(&thread_dbus, wsm_dbus_main, NULL, 0)))
	{
		WSMLog(L_EMERG, "Thread[thread_dbus] initialize failed.\n");
		exit(-1);
	}
	
#ifdef __wsm__
	for (tmp = 0; tmp < WSM_QUEUE_NUM; tmp++)
	{
		/*start up threads base on Coremask*/
		if ((gCoreMask | (core_flag << tmp)) != gCoreMask)
			continue;
			
		if(!CWErr(CWCreateThread(&(thread_recv_WTP[tmp]), WSM_recv_WTP, (void*)&thread_param[tmp], 0))) 
		{
			WSMLog(L_EMERG, "Thread:WSM_recv_WTP[%d] initialize failed.\n", tmp);
			exit(-1);
		}
		if(!CWErr(CWCreateThread(&(thread_recv_Wifi[tmp]), WSM_recv_Wifi, (void*)&thread_param[tmp], 0))) 
		{
			WSMLog(L_EMERG, "Thread:WSM_recv_Wifi[%d] initialize failed.\n", tmp);
			exit(-1);
		}
	}
#endif
	WSMLog(L_INFO, "WSM has started.\n");

	while(1)
	{
		sleep(10000);
	}
	
	return 0;
}


/**
*  Description:
*   Initialize socket for supporting ipv4 and ipv6 mixed protocol stack.
*
*  Parameter:
*   host: hostname, is host = NULL, then listen 0.0.0.0 interface
*   serv: service name or port number string.
*   plen: socket addr len
*
*  Return:
*    >0   sockfd
*    <0  failed
*
*/
int sock_init(const char *host, const char *serv, socklen_t *plen)
{
	#define SOL_UDP 	17
	#define UDP_ENCAP	100
	#define UDP_ENCAP_CAPWAP 4
	int sockfd = 0;
	int option = UDP_ENCAP_CAPWAP;
	int yes = 1;
	struct addrinfo hints, *res = NULL, *ressave = NULL;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC; /* Support IPv4 and IPv6 */
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(host, serv, &hints, &res) != 0)
	{
		WSMLog(L_ERR, "%s: getaddrinfo failed.\n", __func__);
		return -1;
	}
	ressave = res;

	do 
	{
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		WSMLog(L_INFO, "%s: line=%d,sockfd=%d.\n", __func__,__LINE__,sockfd);	
		if (sockfd < 0)
			continue; /* error -- try next one */
		if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0){
			if (setsockopt (sockfd, SOL_UDP,UDP_ENCAP, &option, sizeof (option)) < 0){
				WSMLog(L_ERR, "%s: setsockopt failed.\n", __func__);				
			}
			WSMLog(L_ERR, "%s: setsockopt successfull.\n", __func__);	
				break; /* success */
		}
		close(sockfd);
	}
	while ((res = res->ai_next) != NULL);

	if (res == NULL)
	{
		WSMLog(L_ERR, "%s: Create sockfd failed.\n", __func__);
		return -1;
	}
	if (plen)
		*plen = res->ai_addrlen;

	freeaddrinfo(ressave);

	return sockfd;
}

/**
* Description:
*   Compare IP address
* Parameter:
*   p1,p2: IP addr pointer
*
* Return:
*   0:  equal
*   -1: not equal
*
*/
__inline__ int ip_cmp(struct mixwtpip *p1, struct mixwtpip *p2)
{
	if (p1 == NULL || p2 == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter p1 or p2 == NULL.\n", __func__);
		return -1;
	}

	if (p1->addr_family != p2->addr_family)
	{
		return -1;
	}

	if (p1->addr_family == AF_INET)
	{
		if (p1->m_v4addr == p2->m_v4addr)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	
	if (p2->addr_family == AF_INET6)
	{
		if (memcmp(p1->m_v6addr, p2->m_v6addr, MIXIPLEN) == 0)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}

	return -1;
}

/**
*  Discription:
*   Register VRRP interface.
* 
* Parameter:
*   Register interface info that received from control module(asd or wid, I don't know)
*
* Return:
*   0  : OK
*   -1:  Failed
*
*/
int vrrp_if_reg(vrrp_interface *p)
{
	if_list_t *if_info_8888 = NULL, *if_info_5247 = NULL;
	if_list_t *if_pre = NULL, *if_next = NULL;
	if_list_t *if_tmp = NULL;
	unsigned char ip_str[64] = {0};
	unsigned char *addr = NULL;
	int fd = 0;
	struct mixwtpip ip;
	
	if (!p)
	{
		WSMLog(L_ERR, "%s: Parameter p == NULL.\n", __func__);
		return -1;
	}

	bzero(&ip, sizeof(struct mixwtpip));
	memcpy(&ip, &p->ip, sizeof(struct mixwtpip));
	
	if (p->op == VRRP_REG_IF)
	{
		/* Get IP addr string */
		if (p->ip.addr_family == AF_INET)
		{
			addr = (unsigned char*)(&p->ip.m_v4addr);
			sprintf (ip_str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
		}
		else if (p->ip.addr_family == AF_INET6)
		{
			addr = p->ip.m_v6addr;
			sprintf(ip_str, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
						"%02x%02x:%02x%02x:%02x%02x:%02x%02x",
					addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], 
					addr[7], addr[8], addr[9], addr[10], addr[11], addr[12], addr[13], 
					addr[14], addr[15]);
		}
		else
		{
			WSMLog(L_ERR, "%s: unknow protocol family[%d]\n", __func__, p->ip.addr_family);
			return -1;
		}
		
		/* Initialize port 8888 socket */
		if ((if_info_8888 = (if_list_t*)malloc(sizeof(if_list_t))) == NULL)
		{
			WSMLog(L_CRIT, "%s: malloc if_list_t failed in Port 8888 case.\n", __func__);
			return -1;
		}
		bzero(if_info_8888, sizeof(if_list_t));
		
		fd = sock_init(ip_str, TMP_DATA_PORT_STR, NULL);
		if (fd < 0)
		{
			WSMLog(L_ERR, "%s: Initialize %s:8888 socket failed\n", __func__, ip_str);
			free(if_info_8888);
			if_info_8888 = NULL;
		}
		else
		{
			WSMLog(L_DEBUG, "%s: Initialize %s:8888 successed.\n", __func__, ip_str);
			if_info_8888->fd = fd;
			memcpy(&if_info_8888->ip, &ip, sizeof(struct mixwtpip));
		}

		/* Initialize port 5247 socket */
		if ((if_info_5247 = (if_list_t*)malloc(sizeof(if_list_t))) == NULL)
		{
			WSMLog(L_CRIT, "%s: malloc if_list_t failed in Port 5247 case.\n", __func__);
			return -1;
		}
		bzero(if_info_5247, sizeof(if_list_t));
		
		fd = sock_init(ip_str, STD_DATA_PORT_STR, NULL);
		if (fd < 0)
		{
			WSMLog(L_ERR, "%s: Initialize %s:5247 socket failed\n", __func__, ip_str);
			free(if_info_5247);
			if_info_5247 = NULL;
		}
		else
		{
			WSMLog(L_DEBUG, "%s: Initialize %s:5247 successed.\n", __func__, ip_str);
			if_info_5247->fd = fd;
			memcpy(&if_info_5247->ip, &ip, sizeof(struct mixwtpip));
		}

		/* Add to interface list and update g_sock_cnt */
		if (if_info_8888 == NULL && if_info_5247 == NULL)
		{
			return -1;
		}

		if (if_info_8888 != NULL && if_info_5247 != NULL)
		{
			pthread_mutex_lock(&if_reg_mutex);
			if_info_8888->next = if_info_5247;
			if_info_5247->next = g_fd_list;
			g_fd_list = if_info_8888;
			g_sock_cnt += 2;
			pthread_mutex_unlock(&if_reg_mutex);
			
			return 0;
		}
		else if (if_info_8888 != NULL)
		{
			pthread_mutex_lock(&if_reg_mutex);
			if_info_8888->next = g_fd_list;
			g_fd_list = if_info_8888;
			g_sock_cnt += 1;
			pthread_mutex_unlock(&if_reg_mutex);
			
			return 0;
		}
		else if (if_info_5247 != NULL)
		{
			pthread_mutex_lock(&if_reg_mutex);
			if_info_5247->next = g_fd_list;
			g_fd_list = if_info_5247;
			g_sock_cnt += 1;
			pthread_mutex_unlock(&if_reg_mutex);
			
			return 0;
		}
		
		return -1;
	}
	
	else if (p->op == VRRP_UNREG_IF)
	{
		pthread_mutex_lock(&if_reg_mutex);
		if_pre = if_next = g_fd_list;
		
		while (if_next != NULL)
		{
			if (ip_cmp(&if_next->ip, &p->ip) == 0)
			{
				if (if_next == g_fd_list)
				{
					if_tmp = if_next;
					g_fd_list = g_fd_list->next;
					close(if_tmp->fd);
					free(if_tmp);
					if_tmp = NULL;
					if_pre = if_next = g_fd_list;
					--g_sock_cnt;
				}
				else
				{
					if_tmp = if_next;
					if_pre->next = if_next->next;
					close(if_tmp->fd);
					free(if_tmp);
					if_tmp = NULL;
					if_next = if_pre->next;
					--g_sock_cnt;
				}
				continue;
			}

			if_pre = if_next;
			if_next = if_next->next;
		}
		pthread_mutex_unlock(&if_reg_mutex);
		
		return 0;	
	}
	else 
	{
		WSMLog(L_ERR, "%s: Parameter p->op[%d] unknown type.\n", __func__, p->op);
		return -1;
	}

}

/**
*  Description:
*   Write WSM Instance pid to $pidfile
*
*  Paramter:
*    Void, relation to vrrid
*
*  Return:
*    0: OK
*    -1: Failed
*
*/
int write_pid_file(void)
{
	unsigned char cmd[64] = {0};
	int rval = 0, stat = 0;

	/* User is root */
	if (getuid() == 0){
#ifndef _DISTRIBUTION_
		sprintf (cmd, "echo %d > /var/run/wcpss/wsm%d.pid", getpid(), vrrid);
#else
		sprintf (cmd, "echo %d > /var/run/wcpss/wsm%d_%d.pid",getpid(),local,vrrid);
#endif
	}
	else{
#ifndef _DISTRIBUTION_
		sprintf (cmd, "sudo echo %d > /var/run/wcpss/wsm%d.pid", getpid(), vrrid);
#else
		sprintf (cmd, "sudo echo %d > /var/run/wcpss/wsm%d_%d.pid",getpid(),local,vrrid);
#endif
	}

	rval = system(cmd);
	stat = WEXITSTATUS(rval);

	if (stat != 0)
	{
		WSMLog(L_ERR, "%s: Invoke shell to wirte pid_file failed.\n", __func__);
		return -1;
	}

	return 0;	
}

/**
* Description:
*  Initialize WSM log system. Default setting just close debug level.
*
* Parameter:
*  void
* Return:
*  void
*
*/
void wsm_log_init(void)
{
	openlog("wsm", LOG_NDELAY, LOG_DAEMON);
	wsm_set_logswitch(L_EMERG, LOG_ON);
	wsm_set_logswitch(L_ALERT, LOG_ON);
	wsm_set_logswitch(L_CRIT, LOG_ON);
	wsm_set_logswitch(L_ERR, LOG_ON);
	wsm_set_logswitch(L_WARNING, LOG_ON);
	wsm_set_logswitch(L_NOTICE, LOG_ON);
	wsm_set_logswitch(L_INFO, LOG_ON);
	wsm_set_logswitch(L_DEBUG, LOG_OFF);
}

/**
* Description:
*  WSM module mainly initialize function.
*
* Parameter:
*  void
*
* Return:
*  0 : Successed
*  Other: Failed.
*
*/
int wsm_init(void)
{
	int len = 0;
	int tmp;
	int sndbuf = SOCK_BUFSIZE;
	char WSM_DATA_PATH[WSM_PATH_MAX] = {0};
	char WSM_TABLE_PATH[WSM_PATH_MAX] = {0};
	char WSM_ASD_PATH[WSM_PATH_MAX] = {0};
	char WSM_ASD_PKT_PATH[WSM_PATH_MAX]  = {0};
	char WSM_BSS_PKT_PATH[WSM_PATH_MAX] = {0};
	#ifdef ASD_MULTI_THREAD_MODE
	char ASD_TABLE_PATH[WSM_PATH_MAX] = {0};
	char ASD_PATH[WSM_PATH_MAX] = {0};
	#else
	char ASD_TABLE_PATH[WSM_PATH_MAX] = {0};
	char ASD_PATH[WSM_PATH_MAX] = {0};
	#endif
	char WID_PATH[WSM_PATH_MAX] = {0};
	char WSM_STA_ROAM_PATH[WSM_PATH_MAX] = {0};
	/* zhanglei add for socket rcv */
	/* int rcvbuf = SOCK_BUFSIZE; */ 

	/* Inter-AC roaming enable flag */
	if (vrrid == 0)
		is_instance0 = 1;
	else
		is_instance0 = 0;
		
	g_sock_cnt = 0;
	g_fd_list = NULL;
	
	DATAmsg.msg = DATAbuf;
	DATAmsg.offset = 0;
	DATAmsg.msgLen = 0;
	TABLEmsg.msg = TABLEbuf;
	TABLEmsg.offset = 0;
	TABLEmsg.msgLen = 0;
	WTPmsg.msg = NULL;
	WTPmsg.offset = 0;
	WTPmsg.msgLen = 0;

	/* Initialize Inter-AC roaming socket fd, this function just exist in 
	    instance 0 
	*/
	if (is_instance0)
	{
		if ((raw_ac_fd = socket(PF_INET, SOCK_DGRAM, 0))< 0)
		{
			WSMLog(L_CRIT, "%s: raw_ac_fd create failed.\n", __func__);
			return -1;
		}
		bzero(&raw_ac_addr, sizeof(struct sockaddr_in));
		raw_ac_addr.sin_family = AF_INET;
		raw_ac_addr.sin_port = htons(CW_RAW_AC_DATA_PORT);
		raw_ac_addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(raw_ac_fd, (struct sockaddr*)&raw_ac_addr, sizeof(struct sockaddr_in)) < 0)
		{
			WSMLog(L_CRIT, "%s: raw_ac_fd bind failed\n", __func__);
			return -1;
		}

		if ((roam_ac_fd = socket(PF_INET, SOCK_DGRAM, 0))< 0)
		{
			WSMLog(L_CRIT, "%s: roam_ac_fd create failed.\n", __func__);
			return -1;
		}
		bzero(&roam_ac_addr, sizeof(struct sockaddr_in));
		roam_ac_addr.sin_family = AF_INET;
		roam_ac_addr.sin_port = htons(CW_ROAM_AC_DATA_PORT);
		roam_ac_addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(roam_ac_fd, (struct sockaddr*)&roam_ac_addr, sizeof(struct sockaddr_in)) < 0)
		{
			WSMLog(L_CRIT, "%s: roam_ac_fd bind failed\n", __func__);
			return -1;
		}
	}
	
	/* Dynamic create unix socket path by vrid */
#ifndef _DISTRIBUTION_
	sprintf(WSM_DATA_PATH, "/var/run/wcpss/wsm_data_socket%d", vrrid);
	sprintf(WSM_TABLE_PATH, "/var/run/wcpss/wsm_table_socket%d", vrrid);
	sprintf(WSM_ASD_PATH, "/var/run/wcpss/asd%d", vrrid);
	sprintf(WSM_ASD_PKT_PATH, "/var/run/wcpss/wsm_sta_pkt_socket%d", vrrid);
	sprintf(WSM_BSS_PKT_PATH, "/var/run/wcpss/wsm_bss_pkt_socket%d", vrrid);
	#ifdef ASD_MULTI_THREAD_MODE
	sprintf(ASD_TABLE_PATH, "/var/run/wcpss/get_wsm_info%d", vrrid);
	sprintf(ASD_PATH, "/var/run/wcpss/get_wsm_info%d", vrrid);
	#else
	sprintf(ASD_TABLE_PATH, "/var/run/wcpss/asd_table%d", vrrid);
	sprintf(ASD_PATH, "/var/run/wcpss/asd_table%d", vrrid);
	#endif
	sprintf(WID_PATH, "/var/run/wcpss/wid%d", vrrid);
	sprintf(WSM_STA_ROAM_PATH, "/var/run/wcpss/wsm_sta_roaming%d", vrrid);
#else
	sprintf(WSM_DATA_PATH, "/var/run/wcpss/wsm_data_socket%d_%d",local, vrrid);
	sprintf(WSM_TABLE_PATH, "/var/run/wcpss/wsm_table_socket%d_%d",local, vrrid);
	sprintf(WSM_ASD_PATH, "/var/run/wcpss/asd%d_%d",local, vrrid);
	sprintf(WSM_ASD_PKT_PATH, "/var/run/wcpss/wsm_sta_pkt_socket%d_%d",local, vrrid);
	sprintf(WSM_BSS_PKT_PATH, "/var/run/wcpss/wsm_bss_pkt_socket%d_%d",local, vrrid);
#ifdef ASD_MULTI_THREAD_MODE
	sprintf(ASD_TABLE_PATH, "/var/run/wcpss/get_wsm_info%d_%d",local, vrrid);
	sprintf(ASD_PATH, "/var/run/wcpss/get_wsm_info%d_%d",local, vrrid);
#else
	sprintf(ASD_TABLE_PATH, "/var/run/wcpss/asd_table%d_%d",local, vrrid);
	sprintf(ASD_PATH, "/var/run/wcpss/asd_table%d_%d",local,vrrid);
#endif
	sprintf(WID_PATH, "/var/run/wcpss/wid%d_%d",local,vrrid);
	sprintf(WSM_STA_ROAM_PATH, "/var/run/wcpss/wsm_sta_roaming%d_%d",local, vrrid);
#endif

	/* Save instance pid in pidfile */
	if (write_pid_file() != 0)
	{
		WSMLog(L_ERR, "%s: Wirte pidfile for wcpss failed\n", __func__);
	}
	
	/* WSM Split MAC queue buffer initialize */
	for (tmp = 0; tmp < WSM_QUEUE_NUM; tmp++)
	{
		memset(toWifiBuff + tmp, 0, sizeof(CWProtocolMessage));
		memset(fromWifiBuff + tmp, 0, sizeof(CWProtocolMessage));
	}
	#ifdef __wsm__
	/* wsm shared memory initialize */
	if (wsm_mmap_init() != 0)
	{
		WSMLog(L_CRIT, "%s: Shared memory initialize failed.\n", __func__);
		return -1;
	}
	#endif
	/* ASDSocket for sendto asd */
	if((iASDsocket = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		WSMLog(L_ERR, "%s: iASDsocket initialize failed.\n", __func__);
		return -1;
	}
	
	if ((setsockopt(iASDsocket,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0)
	{	
		WSMLog(L_ERR, "%s: Set iASDsocket option failed\n", __func__);
		return -1;
	}
	
	fcntl(iASDsocket, F_SETFL, O_NONBLOCK);

	/* DATA Socket */
	if( (iDATAsocket = socket(PF_UNIX, SOCK_DGRAM, 0))<0 )
	{
		WSMLog(L_ERR, "%s: iDATAsocket initialize failed.\n", __func__);
		return -1;
	}
	
	WSM_Data_local_sockaddr.sun_family = AF_UNIX;
 	strcpy(WSM_Data_local_sockaddr.sun_path, WSM_DATA_PATH);
 	unlink(WSM_Data_local_sockaddr.sun_path);
 	len = strlen(WSM_Data_local_sockaddr.sun_path) + sizeof(WSM_Data_local_sockaddr.sun_family);

	if( bind(iDATAsocket, (struct sockaddr *)&WSM_Data_local_sockaddr, len)<0 )
	{
		WSMLog(L_ERR, "%s: iDATAsocket bind failed.\n", __func__);
		return -1;
	}
	
	if( (iTABLEsocket = socket(PF_UNIX, SOCK_DGRAM, 0))<0 )
	{
		WSMLog(L_EMERG, "%s: iTABLEsocket initialize failed.\n", __func__);
		return -1;
	}
	
	/*
	if ((setsockopt(iTABLEsocket,SOL_SOCKET,SO_RCVBUF,&rcvbuf,sizeof(rcvbuf))) < 0) 
	{
		WSMLog(LOG_NOR, LOG_CRIT, "Could not set sock RCVBUF\n");
		close(iTABLEsocket);
		return -1;
	}
	*/
	
	WSM_Table_local_sockaddr.sun_family = AF_UNIX;
 	strcpy(WSM_Table_local_sockaddr.sun_path, WSM_TABLE_PATH);
 	unlink(WSM_Table_local_sockaddr.sun_path);	
 	len = strlen(WSM_Table_local_sockaddr.sun_path) + sizeof(WSM_Table_local_sockaddr.sun_family);

	if( bind(iTABLEsocket, (struct sockaddr *)&WSM_Table_local_sockaddr, len)<0 )
	{
		WSMLog(L_ERR, "%s: iTABLEsocket bind failed.\n", __func__);
		return -1;
	}

	WSM_ASD_local_sockaddr.sun_family = AF_UNIX;
	strcpy(WSM_ASD_local_sockaddr.sun_path, WSM_ASD_PATH);
	glen = strlen(WSM_ASD_local_sockaddr.sun_path) + sizeof(WSM_ASD_local_sockaddr.sun_family);

	ASD_Table_sockaddr.sun_family = AF_UNIX;
	strcpy(ASD_Table_sockaddr.sun_path, ASD_TABLE_PATH);
	gSockaddr_len = strlen(ASD_Table_sockaddr.sun_path) + sizeof(ASD_Table_sockaddr.sun_family);

	WID_sockaddr.sun_family = AF_UNIX;
	strcpy(WID_sockaddr.sun_path, WID_PATH);
	
	if( (iSTAPktsocket = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		WSMLog(L_ERR, "%s: iSTAPktsocket initialize failed.\n", __func__);
		return -1;
	}
	
	if ((setsockopt(iSTAPktsocket,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) 
	{	
		WSMLog(L_ERR, "%s: Set iSTAPktsocket option failed\n", __func__);
		return -1;
	}
	fcntl(iSTAPktsocket, F_SETFL, O_NONBLOCK);

	WSM_ASD_PKT_sockaddr.sun_family = AF_UNIX;
	strcpy(WSM_ASD_PKT_sockaddr.sun_path, WSM_ASD_PKT_PATH);
	unlink(WSM_ASD_PKT_sockaddr.sun_path);
	len = strlen(WSM_ASD_PKT_sockaddr.sun_path) + sizeof(WSM_ASD_PKT_sockaddr.sun_family);

	if( bind(iSTAPktsocket, (struct sockaddr *)&WSM_ASD_PKT_sockaddr, len)<0 )
	{
		WSMLog(L_ERR, "%s: iSTAPktsocket bind failed.\n", __func__);
		return -1;
	}
	
	if( (iBSSPktsocket = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		WSMLog(L_ERR, "%s: iBSSPktsocket initialize failed.\n", __func__);
		return -1;
	}
	
	if ((setsockopt(iBSSPktsocket,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0) 
	{	
		WSMLog(L_ERR, "%s: Set iBSSPktsocket option failed\n", __func__);
		return -1;
	}
	fcntl(iBSSPktsocket, F_SETFL, O_NONBLOCK);

	WSM_BSS_PKT_sockaddr.sun_family = AF_UNIX;
	strcpy(WSM_BSS_PKT_sockaddr.sun_path, WSM_BSS_PKT_PATH);
	unlink(WSM_BSS_PKT_sockaddr.sun_path);
	len = strlen(WSM_BSS_PKT_sockaddr.sun_path) + sizeof(WSM_BSS_PKT_sockaddr.sun_family);

	if( bind(iBSSPktsocket, (struct sockaddr *)&WSM_BSS_PKT_sockaddr, len)<0 )
	{
		WSMLog(L_ERR, "%s: iBSSPktsocket bind failed.\n", __func__);
		return -1;
	}
	
	ASD_Roaming_sockaddr.sun_family = AF_UNIX;	
	strcpy(ASD_Roaming_sockaddr.sun_path, ASD_PATH);

	if( (iSTARoamingsocket = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		WSMLog(L_ERR, "%s: iSTARoamingsocket initialize failed.\n", __func__);
		return -1;
	}
	
	if ((setsockopt(iSTARoamingsocket,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf))) < 0)
	{	
		WSMLog(L_ERR, "%s: Set iSTARoamingsocket option failed\n", __func__);
		return -1;
	}
	/* zhanglei add for socket NONBLOCK */
	fcntl(iSTARoamingsocket, F_SETFL, O_NONBLOCK);

	STA_local_Roaming_sockaddr.sun_family = AF_UNIX;
	strcpy(STA_local_Roaming_sockaddr.sun_path, WSM_STA_ROAM_PATH);
	unlink(STA_local_Roaming_sockaddr.sun_path);
	len = strlen(STA_local_Roaming_sockaddr.sun_path) + sizeof(STA_local_Roaming_sockaddr.sun_family);

	if(bind(iSTARoamingsocket, (struct sockaddr *)&STA_local_Roaming_sockaddr, len) < 0)
	{
		WSMLog(L_ERR, "%s: iSTARoamingsocket bind failed.\n", __func__);
		return -1;
	}
	
	/* For WSM tables print,register signal handle function */
	if (signal(SIGUSR1, sig_print) == SIG_ERR) 
		WSMLog(L_WARNING, "Signal: SIGUSR1 bind failed.\n");
	
	if (signal(SIGUSR2, sig_print) == SIG_ERR) 
		WSMLog(L_WARNING, "Signal: SIGUSR2 bind failed.\n");

	/* Initialize WSM Mutexs */
	wsm_mutex_init();

	WSMLog(L_INFO, "%s: Initialize Successed.\n", __func__);
	
	return 0;
}


/**
* Description:
*  WSM Syslog print Function
*
* Parameter:
*  type: Log level
*  format: string format
*
* Return:
*  void
*
*/
__inline__ void WSMLog(unsigned char type, char *format, ...)
{
	va_list ptr;
	unsigned char buff[2048] = {0};
	unsigned int offset = 0;

	sprintf(buff, "%s[%d] ", "Instance", vrrid);
	offset = strlen(buff);

	if (gLog_switch[type])
	{
		switch(type)
		{
			case L_EMERG: 	
				va_start(ptr, format);
				vsprintf(buff + offset, format, ptr);
				va_end(ptr);
				syslog(LOG_EMERG, buff);
				return;
				
			case L_ALERT: 	
				va_start(ptr, format);
				vsprintf(buff + offset, format, ptr);
				va_end(ptr);
				syslog(LOG_ALERT, buff);
				return;
				
			case L_CRIT: 
				va_start(ptr, format);
				vsprintf(buff + offset, format, ptr);
				va_end(ptr);
				syslog(LOG_CRIT, buff);
				return;
				
			case L_ERR: 
				va_start(ptr, format);
				vsprintf(buff + offset, format, ptr);
				va_end(ptr);
				syslog(LOG_ERR, buff);
				return;
				
			case L_WARNING:
				va_start(ptr, format);
				vsprintf(buff + offset, format, ptr);
				va_end(ptr);
				syslog(LOG_WARNING, buff);
				return;
				
			case L_NOTICE:
				va_start(ptr, format);
				vsprintf(buff + offset, format, ptr);
				va_end(ptr);
				syslog(LOG_NOTICE, buff);
				return;
				
			case L_INFO:
				va_start(ptr, format);
				vsprintf(buff + offset, format, ptr);
				va_end(ptr);
				syslog(LOG_INFO, buff);
				return;	
				
			case L_DEBUG:
				va_start(ptr, format);
				vsprintf(buff + offset, format, ptr);
				va_end(ptr);
				syslog(LOG_DEBUG, buff);
				return;
				
			default:
				return;
		}
	}
	
	return;
}


/**
* Description:
*  WSM Log Switch, open or close the print level.
*
* Parameter:
*  logtype: log level
*  new_state: new state..
*
* Return:
*  void
*
*/
void wsm_set_logswitch(unsigned char logtype, unsigned char new_state)
{
	if (logtype >= LOG_TYPE_MAX || new_state >= LOG_STATE_MAX)
		return;
	
	gLog_switch[logtype] = new_state;
	
	return;
}

/**
* Description:
*  Check whether this packet is a roaming packet or not, and update tables for 
*  roaming. 
*
* Parameter:
*  STAMAC: Find BSSID by STAMAC
*  bssid_bssindex_element: New BSSID
*
* Return:
*  void.
*
*/
__inline__ void sta_roam_check(unsigned char* STAMAC, 
			BSSID_BSSIndex_Element* bssid_bssindex_element)
{
	struct wsm_stamac_bssid_wtpip *stamac_bssid_data = NULL;
	BSSID_BSSIndex_Element* old_bssindex;
	struct wsm_bssid_wlanid* old_bss = NULL;
	struct wsm_bssid_wlanid* new_bss = NULL;
	aWSM_STA old_sta, new_sta;
	TableMsg data;
	int len;
	
	if ((STAMAC == NULL) || (bssid_bssindex_element == NULL))
	{
		return;
	}
	new_bss = (struct wsm_bssid_wlanid *)wsm_wifi_table_search(
		bssid_bssindex_element->BSSID, WSM_BSSID_WLANID_TYPE);
	if((WLANID_BSSID_Table[new_bss->WlanID - 1].Roaming_policy == ROAMING_DISABLE))
		return;
	/* Get old sta table */
	if ((stamac_bssid_data = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
							STAMAC, WSM_STAMAC_BSSID_WTPIP_TYPE)) == NULL)
	{
		return;
	}

	/* Is it a roaming packet? */
	if (!memcmp(bssid_bssindex_element->BSSID, stamac_bssid_data->BSSID, MAC_LEN))
	{
		return;
	}

	/* Roaming packet */
	old_bss = (struct wsm_bssid_wlanid *)wsm_wifi_table_search(
		stamac_bssid_data->BSSID, WSM_BSSID_WLANID_TYPE);

	if ((new_bss == NULL) || (old_bss == NULL))
	{
		return;
	}

	/* Not belong to the same wlan or roaming policy is disable, cannot roam */
	if ((new_bss->WlanID != old_bss->WlanID) || 
		(WLANID_BSSID_Table[new_bss->WlanID - 1].Roaming_policy == ROAMING_DISABLE))
	{
		return;
	}
	if ((old_bssindex = (BSSID_BSSIndex_Element *)wsm_tbl_search(
		BSSID_BSSIndex_TYPE, stamac_bssid_data->BSSID,0)) == NULL)
	{
		return;
	}

	/* Fill old STA structure */
	old_sta.BSSIndex = old_bssindex->BSSIndex;
	old_sta.StaState = stamac_bssid_data->STAState;
	old_sta.WTPID = old_bssindex->BSSIndex / BSS_ARRAY_SIZE;
	WSM_COPY_MAC(old_sta.STAMAC, stamac_bssid_data->STAMAC);
	/* Fill new STA structure */
	new_sta.BSSIndex = bssid_bssindex_element->BSSIndex;
	new_sta.StaState = stamac_bssid_data->STAState;
	new_sta.WTPID = new_sta.BSSIndex / BSS_ARRAY_SIZE;
	WSM_COPY_MAC(new_sta.STAMAC, stamac_bssid_data->STAMAC);

	/* Notify ASD that STA has changed BSS */
	data.Type = STA_TYPE;
	data.Op = WID_MODIFY;
	data.u.STA.StaState = new_sta.StaState;
	data.u.STA.WTPID = new_sta.WTPID;
	data.u.STA.preBSSIndex = old_sta.BSSIndex;
	data.u.STA.BSSIndex = new_sta.BSSIndex;
	WSM_COPY_MAC(data.u.STA.STAMAC, new_sta.STAMAC);
			
	/* Change read lock to write lock */
	WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
	WSM_WRLOCK_LOCK(&wsm_tables_rwlock);
	wsm_tbl_del(STA_TYPE,  (void *)&old_sta);
	/* avoid  ADD STA again */
	if ((stamac_bssid_data = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
				STAMAC, WSM_STAMAC_BSSID_WTPIP_TYPE)) != NULL)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
		WSMLog(L_NOTICE, "%s: Found duplication sta, add STA cancel.\n", __func__);
		return;
	}
	wsm_tbl_add(STA_TYPE, (void*)&new_sta);
	WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);

	/* Notify ASD */
	len = strlen(ASD_Roaming_sockaddr.sun_path) + sizeof(ASD_Roaming_sockaddr.sun_family);
	if (sendto(iSTARoamingsocket, (unsigned char*)&data, sizeof(data), 0, 
			(struct sockaddr *)&ASD_Roaming_sockaddr, len) < 0)
	{
		WSMLog(L_INFO, "%s: Notify ASD, sendto failed.\n", __func__);
		return;
	}
			
	return;
}

/**
* Description:
*   Receive packet between roam_ac and raw_ac.
*
* Parameter:
*   msg: received buff len and offset
*   buff: shared memory pointer
*   addr: client addr
*   type: roam ac or raw ac
*
* Return:
*   0 : Successed.
*  -1: Failed
*
*/
int roam_pkt_recv(CWProtocolMessage *msg, unsigned char *buff, struct sockaddr_storage *addr, int type)
{
	int max_len = 1800;
	socklen_t len = sizeof(struct sockaddr_storage);
	
	switch(type)
	{
		case THIS_IS_RAW_AC:
			msg->msg = buff + RAW_AC_OFFSET;
			msg->msgLen = 0;
			msg->offset = RAW_AC_OFFSET;

			msg->msgLen = recvfrom(raw_ac_fd, msg->msg, max_len, 0, (struct sockaddr*)addr, &len);
			if (msg->msgLen <= 0)
			{
				WSMLog(L_ERR, "%s: raw_ac_fd recv failed[%d]\n", __func__, msg->msgLen);
				return -1;
			}
			return 0;
			
		case THIS_IS_ROAM_AC:
			msg->msg = buff + ROAM_AC_OFFSET;
			msg->msgLen = 0;
			msg->offset = ROAM_AC_OFFSET;

			msg->msgLen = recvfrom(roam_ac_fd, msg->msg, max_len, 0, (struct sockaddr*)addr, &len);
			if (msg->msgLen <= 0)
			{
				WSMLog(L_ERR, "%s: roam_ac_fd recv failed[%d].\n", __func__, msg->msgLen);
				return -1;
			}
			return 0;
			
		default:
			WSMLog(L_ERR, "%s: unknown type[%d].\n", __func__, type);
			return -1;
	}
}

/**
*  Description:
*   Raw AC received packet from Roam-AC, and send it to wifi.
*
*  Parameter:
*   buff: shared memory page head pointer.
*   msglen: packet length.
*
*  Return:
*   void
*
*/
void raw_ac_routine(unsigned char *buff, int msglen)
{
	unsigned char *smac = NULL;
	struct wsm_stamac_bssid_wtpip *sta_info = NULL;
	BSSID_BSSIndex_Element *bss_info = NULL;
	struct page_head *page = NULL;
	unsigned int bss_idx = 0;
	
	WSMLog(L_DEBUG, "%s: Raw AC trasmit packet to Wifi Begin.\n", __func__);

	if (msglen <= 0)
	{
		WSMLog(L_ERR, "%s: packet length : %d error.\n", __func__, msglen);
		return;
	}

	smac = buff + RAW_AC_OFFSET + 6;
	page = (struct page_head*)buff;

	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
	sta_info = wsm_wifi_table_search(smac, WSM_STAMAC_BSSID_WTPIP_TYPE);
	if (!sta_info)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_WARNING, "%s: Cannot find Roaming STA[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, smac[0], smac[1], smac[2], smac[3], smac[4], smac[5]);
		return;
	}
	if (sta_info->ac_roam_tag != RAW_AC)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_WARNING, "%s: STA[%02x:%02x:%02x:%02x:%02x:%02x] RoamingTag != RAW_AC.\n",
				__func__, smac[0], smac[1], smac[2], smac[3], smac[4], smac[5]);
		return;
	}
	
	bss_info =  (BSSID_BSSIndex_Element *)wsm_tbl_search(BSSID_BSSIndex_TYPE, sta_info->BSSID,0);
	if (!bss_info)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_WARNING, "%s: Cannot find BSSIndex by BSSID[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, sta_info->BSSID[0], sta_info->BSSID[1], sta_info->BSSID[2], sta_info->BSSID[3], 
				sta_info->BSSID[4], sta_info->BSSID[5]);
		return;
	}

	bss_idx = bss_info->BSSIndex;
	WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);

	if (msglen >= 64)
		page->len = msglen;
	else
		page->len = 64;
		
	page->BSSIndex = bss_idx;
	page->offset = RAW_AC_OFFSET;
	page->Wlanid = 0;

	WSMLog(L_DEBUG, "%s: Raw AC trasmit packet to Wifi End.\n", __func__);
	
	return;
}

/**
*  Description:
*   Roam AC received packet from Raw-AC, and send it to WTP.
*
*  Parameter:
*   buff: shared memory page head pointer.
*   msglen: packet length.
*
*  Return:
*   void.
*
*/

void roam_ac_routine(unsigned char *buff, int msglen)
{
	unsigned char *stamac = NULL;
	struct wsm_stamac_bssid_wtpip *sta_info = NULL;
	BSSID_BSSIndex_Element *bssidx_info = NULL;
	struct wsm_bssid_wlanid *bss_info = NULL;
	struct page_head *page = NULL;
	WSMData_T sta_data;
	unsigned int bss_idx = 0;
	roam_head_t *roam_head;
	unsigned int wireless_cnt = 0;
	unsigned int protect_type = 0;
	unsigned int cw_offset = 20;
	unsigned int wtpid = 0;
	
	page = (struct page_head*)buff;
	roam_head = (roam_head_t *)(buff + ROAM_AC_OFFSET);
	bzero(&sta_data, sizeof(WSMData_T));

	if (roam_head->type == ROAM_UNICAST)
	{
		stamac = roam_head->stamac;
		WSMLog(L_DEBUG, "%s: Unicast begin. mac: %02x:%02x:%02x:%02x:%02x:%02x\n", 
			__func__, stamac[0], stamac[1], stamac[2], stamac[3], stamac[4], stamac[5]);
		wireless_cnt = getSeqNum();
		
		WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
		sta_info = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
					stamac, WSM_STAMAC_BSSID_WTPIP_TYPE);
		if (sta_info == NULL)
		{
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			WSMLog(L_ERR, "%s: Unicast, cannot find STA[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, stamac[0], stamac[1], stamac[2], stamac[3], stamac[4], stamac[5]);
			return;
		}
		
		sta_data.BSSID = sta_info->BSSID;
		bss_info = (struct wsm_bssid_wlanid *)wsm_wifi_table_search(sta_data.BSSID, WSM_BSSID_WLANID_TYPE);
		if (bss_info == NULL)
		{
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			WSMLog(L_ERR, "%s: Unicast, cannot find BSS[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, sta_data.BSSID[0], sta_data.BSSID[1], sta_data.BSSID[2], 
				sta_data.BSSID[3], sta_data.BSSID[4], sta_data.BSSID[5]);
			return;
		}
		
		sta_data.Radio_L_ID = bss_info->Radio_L_ID;
		sta_data.WlanID = bss_info->WlanID;
		sta_data.WTPID = sta_info->WTPID;
		protect_type = bss_info->protect_type;
		
		if (ieee8023_to_ieee80211(buff, msglen - sizeof(roam_head_t), 16, wireless_cnt, sta_data.BSSID, protect_type))
		{
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			WSMLog(L_WARNING, "%s: Transfer 802.3 frame to 802.11 frame failed.\n", __func__);
			return;
		}
		
		/* 802.11==>CAPWAP */
		assemble_capwap(buff, cw_offset, msglen - sizeof(roam_head_t), &sta_data);
		/* For statistics STA tx/rx packets information */
		wsm_sta_statistics_update(sta_info->STAMAC, page->len, WSM_STA_RX_UNI_TYPE);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_DEBUG, "%s: Unicast end.\n", __func__);
		return;
	}
	else if (roam_head->type == ROAM_MULTICAST)
	{
		stamac = roam_head->stamac;
		wireless_cnt = getSeqNum();
		WSMLog(L_DEBUG, "%s:Multicast begin.\n", __func__);
		WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
		sta_info = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
					stamac, WSM_STAMAC_BSSID_WTPIP_TYPE);
		if (sta_info == NULL)
		{
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			WSMLog(L_ERR, "%s: Multicast, cannot find STA[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, stamac[0], stamac[1], stamac[2], stamac[3], stamac[4], stamac[5]);
			return;
		}
		
		sta_data.BSSID = sta_info->BSSID;
		bss_info = (struct wsm_bssid_wlanid *)wsm_wifi_table_search(sta_data.BSSID, WSM_BSSID_WLANID_TYPE);
		if (bss_info == NULL)
		{
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			WSMLog(L_ERR, "%s: Multicast, cannot find BSS[%02x:%02x:%02x:%02x:%02x:%02x].\n",
				__func__, sta_data.BSSID[0], sta_data.BSSID[1], sta_data.BSSID[2], 
				sta_data.BSSID[3], sta_data.BSSID[4], sta_data.BSSID[5]);
			return;
		}
		sta_data.Radio_L_ID = bss_info->Radio_L_ID;
		sta_data.WlanID = bss_info->WlanID;
		sta_data.WTPID = sta_info->WTPID;
		protect_type = bss_info->protect_type;
		wireless_cnt = getSeqNum();
		
		if (ieee8023_to_ieee80211(buff, msglen - sizeof(roam_head_t), 16, 
			wireless_cnt, sta_data.BSSID, protect_type))
		{
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			return;
		}
				
		assemble_capwap(buff, cw_offset, msglen - sizeof(roam_head_t), &sta_data);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_DEBUG, "%s:Multicast end.\n", __func__);
		return;
	}
	else
	{
		WSMLog(L_ERR, "%s: unknown type[%d].\n", __func__, roam_head->type);
		return;
	}	
}


/**
* Description:
*   Print tables when debug. 
*   For example:
*     kill -USR1 wsm_pid, kill -USR2 wsm_pid
* 
* Parameter:
*  sig: signal(SIGUSR1, SIGUSR2)
*
* Return:
*  void
*
*/
void sig_print(int sig)
{
	int tmp;
	
	if (sig == SIGUSR1)
	{
		wsm_tbl_print(WTPIP_WTPID_TYPE);
		wsm_tbl_print(BSSID_BSSIndex_TYPE);
		wsm_tbl_print(WTP_BSS_STA_TYPE);
	}
	if (sig == SIGUSR2)
	{
		wsm_wifi_table_print(WSM_BSSID_WLANID_TYPE);
		wsm_wifi_table_print(WSM_STAMAC_BSSID_WTPIP_TYPE);
		wsm_wifi_table_print(WSM_WLANID_BSSID_TYPE);
		wsm_wifi_table_print(WSM_STA_Statistics_TYPE);
		printf ("\nShared Memory:\n");

		printf ("-------------------------------------------------------\n");
		for (tmp = 0; tmp < WSM_QUEUE_NUM; tmp++)
		{
			printf ("[To_Wifi]  CoreID = %d  wsm_index = %d wifi_index = %d\n", 
				tmp + 1, toWifi_Head[tmp].wsm_index, toWifi_Head[tmp].wifi_index);
		}
		printf ("-------------------------------------------------------\n");
		for (tmp = 0; tmp < WSM_QUEUE_NUM; tmp++)
		{
			printf ("[from_Wifi] CoreID = %d wsm_index = %d wifi_index = %d\n",
				tmp + 1, fromWifi_Head[tmp].wsm_index, fromWifi_Head[tmp].wifi_index);
		}		
	}
}

/**
* Description:
*  Watch dog watch the CAPWAP datagram, if there occur error, restart wsm module.
*
* Parameter:
*  void.
*
* Return:
*  void
*
*/
void watch_dog(void)
{
	FILE *fp = NULL;
	unsigned char filename[] = "/opt/bin/wsm_watch_dog";
	unsigned int cnt = 0;
	unsigned int sleep_time = 60;
	unsigned char buff[128] = {0};
	unsigned int bad_cnt = 0;
	unsigned int loop = 0;
	unsigned long pre_cnt = 0;
	
	fp = fopen(filename, "w");
	if (!fp)
	{
		WSMLog(L_ERR, "%s: fopen failed.\n", __func__);
		return;
	}

	/* Build shell script */
	fputs("#!/bin/bash\n\n", fp);
	fputs("FILE=/tmp/.wsm_dump\n", fp);
	fputs("TCPDUMP=/usr/sbin/tcpdump\n", fp);
	fputs("GAP_TIME=5\n\n", fp);
	fputs("$TCPDUMP -n udp src port 32769 and dst port 8888 or 5247 1>$FILE 2>/dev/null &\n", fp);
	fputs("PID=$!\n", fp);
	fputs("sleep $GAP_TIME\n", fp);
	fputs("kill $PID\n", fp);
	fputs("COUNT=$(cat $FILE | grep -v ^$ | wc -l | awk '{print $1}')\n", fp);
	fputs("exit $COUNT\n", fp);
	fclose(fp);

	/* Set permission */
	sprintf(buff, "chmod a+x %s", filename);
	system(buff);
	
	while(1)
	{
		if (!(tunnel_run && dbus_wd_enable))
		{
			WSMLog(L_DEBUG, "%s: tunnel_run = %d dbus_wd_enable = %d\n", __func__, tunnel_run, 
					dbus_wd_enable);
			sleep(sleep_time);
			continue;
		}
		
		pre_cnt = g_pkt_cnt;
		sleep(sleep_time);
		
		if (g_pkt_cnt != pre_cnt)
		{
			continue;
		}

		/* Start capture capwap packets from system network devices */
		pre_cnt = g_pkt_cnt;
		cnt = system(filename);
		
		if (watch_dog_alert(cnt, (g_pkt_cnt - pre_cnt)))
		{
			if (loop > 4)
			{
				watch_dog_say_should_die(cnt, g_pkt_cnt);
				exit(-1);
			}
			else
			{
				loop++;				
				WSMLog(L_ERR, "%s: loop = %d dump cnt = %d, wsm cnt = %d\n", __func__, loop, cnt, g_pkt_cnt);
			}
		}
		else
		{
			loop = 0;
		}	
	}	
	
}

/**
* Discription:
*    Watch dog checks packet statistics
* 
* Paramter:
*    dump_cnt: tcpdump packet statistics
*    wsm_cnt: capwap data channel packet statistics
*
* Return:
*    0 : Normal condition
*    1 : should die
*
*/
int watch_dog_alert(int dump_cnt, int wsm_cnt)
{
	if (wsm_cnt == 0)
	{
		if (dump_cnt == 0)
			return 0;
		else
			return 1;
	}
	else
		return 0;
}

/**
* Description:
*  Watch dog print when error occur.
*
* Parameter:
*  dump_cnt: tcpdump captured packets count.
*  wsm_cnt: wsm received packets count.
*
* Return:
*  void
*
*/
void watch_dog_say_should_die(int dump_cnt, int wsm_cnt)
{
	WSMLog(L_EMERG, "%s: tcpdump packet statistics: %d\n", __func__, dump_cnt);
	WSMLog(L_EMERG, "%s: wsm data tunnel statistics: %d\n", __func__, wsm_cnt);

	return;
}

/**
* Description:
*   Initialize Mutexs for WSM Module.
*   In fact, the lock mechanism is wsm module's bottle-neck. Every bucket has a lock
*   is better than a big lock that lock the whole table. But now, it's not very imporant
*   because ipfwd is the mainly data switching module. If you have time, it's better to
*   change it.
*
* Parameter:
*   Void.
*
* Return:
*   Void.
*
*/
void wsm_mutex_init(void)
{
	/* Initialize mutexs */
	pthread_mutex_init(&split_recv_mutex, NULL);
	pthread_mutex_init(&split_send_mutex, NULL);
	pthread_rwlock_init(&wsm_tables_rwlock, NULL);
	pthread_mutex_init(&sta_statistics_mutex, NULL);
	pthread_mutex_init(&sta_statistics_update_mutex, NULL);
	pthread_mutex_init(&if_reg_mutex, NULL);
}

/**
* Description:
*   Receive message from WTP by CAPWAP Data channel
*
* Parameter:
*   msgPtr: to get the buffer pointer which has hold the data from WTP
*               These functions can be used by WSM, ASD, WID to maintain 
*               hashtable, such as WTPtable, BSStable, STAtable
*   pCAPWAPheadvalue: CAPWAP header information.
*   sock: Received socket fd.
*   addrptr: Received socket's addr.
*
* Return:
*  CW_FALSE, CW_TRUE.
*
*/
CWBool ReceiveMessage(CWProtocolMessage *msgPtr, 
	CWProtocolTransportHeaderValues *pCAPWAPheadvalue,
	int sock, 
	struct sockaddr *addrPtr) 
{
	CWList fragments = NULL;
	int readBytes;
	/* For wsm shared memory */
	int wsm_mem_offset = 1800;
	char *buf = msgPtr->msg;
	
	CW_REPEAT_FOREVER 
	{
		CW_ZERO_MEMORY(buf, wsm_mem_offset);	

		if((CWNetworkReceiveUnsafe(sock, buf, wsm_mem_offset, 0, 
			(CWNetworkLev4Address *)addrPtr, &readBytes))<0) 
		{
			WSMLog(L_CRIT, "%s: CWNetworkReceiveUnsafe failed.\n", __func__);
			return CW_FALSE;
		}

		CWBool dataFlag = CW_FALSE;
		if(!CWProtocolParseFragment_GetCAPWAPheadInfo(buf, 
			readBytes, &fragments, msgPtr, &dataFlag, pCAPWAPheadvalue)) 
		{
			WSMLog(L_ERR, "%s: Fragment data, error.\n", __func__);
			/* Discard fragment capwap packet by guoxb, 2009-10-17 */
			return CW_FALSE;
				
			if(CWErrorGetLastErrorCode() == CW_ERROR_NEED_RESOURCE) 
			{ 
				/* we need at least one more fragment */
				continue;
			}
			else
			{
				/* error */
				CWErrorCode error;
				error = CWErrorGetLastErrorCode();
							
				switch(error)
				{
					case CW_ERROR_SUCCESS: 
						break;
					case CW_ERROR_OUT_OF_MEMORY:  
						break;
					case CW_ERROR_WRONG_ARG: 
						break;
					case CW_ERROR_INTERRUPTED:
						break;
					case CW_ERROR_NEED_RESOURCE:
						break;
					case CW_ERROR_COMUNICATING: 
						break;
					case CW_ERROR_CREATING:
						break;
					case CW_ERROR_GENERAL:
						continue;
					case CW_ERROR_OPERATION_ABORTED:
						break;
					case CW_ERROR_SENDING: 
						break;
					case CW_ERROR_RECEIVING:
						break;
					case CW_ERROR_INVALID_FORMAT:
						break;
					case CW_ERROR_TIME_EXPIRED:
						break;
					case CW_ERROR_NONE:
						break;
				}
				return CW_FALSE;
			}
		} 
		else 
		{
			/* the message is fully reassembled */
			break; 
		}
	}
		
	return CW_TRUE;
}

/**
* Description:
*  Send STA rx/tx packets statistics to ASD.
*
* Parameter:
*   cnt: Number of STAs' statistics
*  buff: send buffer
*
* Return:
*  -1: error; 0: ok
*
*/
int sta_statistics_to_asd(int cnt, unsigned char* buff)
{
	STAStatisticsMsg *pdata = (STAStatisticsMsg *)buff;
	int len = sizeof(STAStatisticsMsg) + sizeof(STAStatistics) * cnt;
	
	pdata->type = STA_PKT_TYPE;
	pdata->cnt = cnt;

	if (sendto(iSTAPktsocket, buff, len, 0, 
		(struct sockaddr *)&ASD_Table_sockaddr, gSockaddr_len) <= 0)
	{
		WSMLog(L_INFO, "%s: sendto failed.\n", __func__);
		return -1;
	}
	
	return 0;
}


/*
* Description:
*  STA hash collision detection.
*
* Parameter:
*   p: Buffer pointer which received from ASD about table element.
*
* Return:
*  0: STA Hash collision.
*  -1: Not has hash collision or not STA_ADD operation or other.
*
*/
int sta_collision_detection(TableMsg *p)
{
	struct wsm_stamac_bssid_wtpip *sta_info = NULL;
	BSSID_BSSIndex_Element *bss_info = NULL;
	aWSM_STA old_sta;
	TableMsg to_asd;
	int len = 0;

	if (p == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter p == NULL.\n", __func__);
		return -1;
	}
	
	if (p->Type != STA_TYPE)
	{
		return -1;
	}

	sta_info = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
				p->u.STA.STAMAC, 
				WSM_STAMAC_BSSID_WTPIP_TYPE);
	if (!sta_info)
	{
		return -1;
	}
	
	bzero(&old_sta, sizeof(old_sta));
	bzero(&to_asd, sizeof(to_asd));

	memcpy(old_sta.STAMAC, sta_info->STAMAC, MAC_LEN);
	bss_info = (BSSID_BSSIndex_Element *)wsm_tbl_search(
				BSSID_BSSIndex_TYPE, 
				sta_info->BSSID,0);
	/* STAMAC is the same, but BSSID is different, ignore the STA_ADD operation */
	if (!bss_info)
	{
		WSMLog(L_ERR, "%s: STA[%02x:%02x:%02x:%02x:%02x:%02x] collision,"
				" but cannot find the BSS.\n", __func__, sta_info->STAMAC[0], 
				sta_info->STAMAC[1], sta_info->STAMAC[2], sta_info->STAMAC[3],
				sta_info->STAMAC[4],sta_info->STAMAC[5]);
		return 0; 
	}

	old_sta.BSSIndex = bss_info->BSSIndex;
	old_sta.WTPID = bss_info->BSSIndex / BSS_ARRAY_SIZE;
	/* BSSIndex is the same, ingore this */
	if (old_sta.BSSIndex == p->u.STA.BSSIndex)
	{
		/* Inter-AC roaming check */
		if (sta_info->ac_roam_tag != p->u.STA.ac_roam_tag)
		{
			WSMLog(L_DEBUG, "%s: Inter-AC roaming STA change state.\n", __func__);
			wsm_tbl_del(STA_TYPE, &old_sta);
			wsm_tbl_add(p->Type,	(void *)&(p->u));
			return 0;
		}
		else
			WSMLog(L_DEBUG, "%s: Inter-AC roaming STA not change state.\n", __func__);
		
		WSMLog(L_NOTICE, "%s: STA[%02x:%02x:%02x:%02x:%02x:%02x] collision,"
				" but BSSIndex[%u] is same.\n", __func__, sta_info->STAMAC[0], 
				sta_info->STAMAC[1], sta_info->STAMAC[2], sta_info->STAMAC[3],
				sta_info->STAMAC[4],sta_info->STAMAC[5], old_sta.BSSIndex);
		return 0;
	}

	WSMLog(L_DEBUG, "%s: STA[%02x:%02x:%02x:%02x:%02x:%02x] is collision."
			" Old BSSIndex[%u] New BSSIndex[%u]\n", __func__, sta_info->STAMAC[0], 
			sta_info->STAMAC[1], sta_info->STAMAC[2], sta_info->STAMAC[3],
			sta_info->STAMAC[4],sta_info->STAMAC[5], old_sta.BSSIndex, p->u.STA.BSSIndex);
			
	/* Hash collision and BSSIndexs are different, update table and notify asd */
	to_asd.Op = WID_CONFLICT;
	to_asd.Type = STA_TYPE;
	memcpy(to_asd.u.STA.STAMAC, old_sta.STAMAC, MAC_LEN);
	to_asd.u.STA.WTPID = old_sta.WTPID;
	to_asd.u.STA.preBSSIndex = old_sta.BSSIndex;
	to_asd.u.STA.BSSIndex = p->u.STA.BSSIndex;
	/* Delete old sta, add new sta */
	wsm_tbl_del(STA_TYPE, &old_sta);
	wsm_tbl_add(p->Type,	(void *)&(p->u));

	/* Notify ASD */
	len = strlen(ASD_Roaming_sockaddr.sun_path) + 
		sizeof(ASD_Roaming_sockaddr.sun_family);
	if (sendto(iSTARoamingsocket, (unsigned char*)&to_asd, sizeof(to_asd), 0, 
			(struct sockaddr *)&ASD_Roaming_sockaddr, len) < 0)
	{
		WSMLog(L_INFO, "%s: sendto asd failed.\n", __func__);
		return 0;
	}

	return 0;
}

#ifdef __wsm__

/**
* Description:
*   Recv capwap datagram from WTPs by CAPWAP data tunnel. It just fill in the
*   first queue until the first queue is full, then it fill in the next queue. This func
*   just receive packet not parse it.
*
* Parameter:
*  Void.
*
* Return:
*  Void
*
*/

void wsm_recv_wtpmsg(void)
{
	int index = 0, queue_num = 0, loop_cnt = 0;
	int loop_max = 64, core_flag = 1;
	CWProtocolTransportHeaderValues CAPWAPheadvalue;
	CWProtocolMessage *recv_buff;
	struct sockaddr_storage *recv_sock = NULL;
	unsigned char *buff = NULL;
	struct page_head *tmp_page = NULL;
	unsigned long core_mask = gCoreMask & ~1u;
	struct timespec tv;
	unsigned int *ac_fd = NULL;
	if_list_t *p_list = NULL;
	int rval = 0;
	struct timeval tmval;
	
	tv.tv_sec = 0;
	tv.tv_nsec = 1;
	
	/* If AC power on with multi-cpu, set the affinity to other core not core 0 */
	if (gCoreMask > 1)
	{
		if (sched_setaffinity(0, sizeof(core_mask), &core_mask) < 0)
		{
			WSMLog(L_WARNING, "%s: sched_setaffinity failed. \n", __func__);
		}
	}
	
	while (1)
	{
		for (index = 0; index < WSM_QUEUE_NUM; index++)
		{
			if ((gCoreMask | (core_flag << index)) != gCoreMask)
				continue;
					
			while (1)
			{
				while (g_sock_cnt < 1)
				{
					nanosleep(&tv, NULL);
				}
				
				buff = (unsigned char*)(((unsigned char*)MM_Head + toWifi_Head[index].offset) + 
					(gToWifiIndex[index].toWifiIndex)* BUFF_SIZE);
				tmp_page = (struct page_head*)buff;
				 
				if (tmp_page->finish != WSM_FREE_BUFF)
				{
					WSMLog(L_DEBUG, "%s: Shared mem flag is not free.\n", __func__);
					break;		
				}
				WSMLog(L_DEBUG, "%s: Get shared memory buff ok.\n", __func__);
re_recv:								
				recv_buff = &(gToWifiData[index][gToWifiIndex[index].toWifiIndex].cw_data);
				recv_sock = &(gToWifiData[index][gToWifiIndex[index].toWifiIndex].sock_data);	
				ac_fd = &(gToWifiData[index][gToWifiIndex[index].toWifiIndex].ac_fd);
				
				recv_buff->msg = buff + PAGE_HEAD_LEN + WSM_BUFF_OFFSET;
				recv_buff->msgLen = recv_buff->offset = 0;

				/* For supporting wsm data tunnel multi-port by Guo Xuebin, 2009-06-02 */
				tmval.tv_sec = 2;
				tmval.tv_usec = 0;

				pthread_mutex_lock(&if_reg_mutex);
				
				FD_ZERO(&gfds);
				p_list = g_fd_list;
				maxfd = 0;
				
				while (p_list != NULL)
				{
					FD_SET(p_list->fd, &gfds);
					if (p_list->fd > maxfd)
					{
						maxfd = p_list->fd;
					}
					p_list = p_list->next;
				}
				
				/* Add Inter-AC socket fd, if the AC don't have other fd expect inter-ac fd, 
				    it will not work 
				  */
				if (is_instance0)
				{
					FD_SET(raw_ac_fd, &gfds);
					FD_SET(roam_ac_fd, &gfds);
					if (raw_ac_fd > maxfd)
						maxfd = raw_ac_fd;
					if (roam_ac_fd > maxfd)
						maxfd = roam_ac_fd;
				}
				
				maxfd += 1;
				pthread_mutex_unlock(&if_reg_mutex);
				
				WSMLog(L_DEBUG, "%s: Before select.\n", __func__);

				rval = select(maxfd, &gfds, NULL, NULL, &tmval);
				if (rval == 0)
				{
					goto re_recv;
				}
				else if (rval < 0)
				{
					WSMLog(L_EMERG, "%s: function select error.\n", __func__);
					exit(-1);
				}

				WSMLog(L_DEBUG, "%s: select received packet.\n", __func__);

				pthread_mutex_lock(&if_reg_mutex);
				p_list = g_fd_list;
				
				while (p_list != NULL)
				{	
					if (FD_ISSET(p_list->fd, &gfds))
					{
						if (!ReceiveMessage(recv_buff, &CAPWAPheadvalue, p_list->fd, (struct sockaddr *)recv_sock))
						{
							pthread_mutex_unlock(&if_reg_mutex);
							WSMLog(L_DEBUG, "%s: Receive CW data port goto re_recv.\n", __func__);
							goto re_recv;
						}
						*ac_fd = p_list->fd;
						pthread_mutex_unlock(&if_reg_mutex);
						
						tmp_page->finish = WSM_RECVED_BUFF;
						gToWifiIndex[index].toWifiIndex = (gToWifiIndex[index].toWifiIndex + 1) % WSM_QUEUE_LEN;
						WSMLog(L_DEBUG, "%s: Receive packet from CAPWAP data port.\n", __func__);
						
						break;
					}
					p_list = p_list->next;
				}
				pthread_mutex_unlock(&if_reg_mutex);
				
				if (is_instance0)
				{
					if (FD_ISSET(raw_ac_fd, &gfds))
					{
						WSMLog(L_DEBUG, "%s: raw ac received packet from roaming ac.\n", __func__);
						if (roam_pkt_recv(recv_buff, buff, recv_sock, THIS_IS_RAW_AC) != 0)
						{
							WSMLog(L_DEBUG, "%s: Receive packet from raw_ac goto re_recv.\n", __func__);
							goto re_recv;
						}
						*ac_fd = raw_ac_fd;	
						tmp_page->finish = WSM_RECVED_BUFF;
						gToWifiIndex[index].toWifiIndex = (gToWifiIndex[index].toWifiIndex + 1) % WSM_QUEUE_LEN;
					}
				
					if (FD_ISSET(roam_ac_fd, &gfds))
					{
						WSMLog(L_DEBUG, "%s: roaming ac received packet from raw ac.\n", __func__);
						if (roam_pkt_recv(recv_buff, buff, recv_sock, THIS_IS_ROAM_AC) != 0)
						{
							WSMLog(L_DEBUG, "%s: Receive packet from roam_ac goto re_recv.\n", __func__);
							goto re_recv;
						}
						
						*ac_fd = roam_ac_fd;	
						tmp_page->finish = WSM_RECVED_BUFF;
						gToWifiIndex[index].toWifiIndex = (gToWifiIndex[index].toWifiIndex + 1) % WSM_QUEUE_LEN;
					
					}
				}		
				g_pkt_cnt++;						
			}
					
		}
	}
}
#endif

/**
* Description:
*   Parse message which received from WTP, maybe MGMT or CW data channel data, 
*   if it's a MGMT or EAP etc which should send to asd, I should fill data for "pdata", 
*
* Parameter:
*  index: Number of Queue,in the wsm there has 16 Queues, and every 
*              queue has self-governed thread to deal with its. the index is
*              queue number, but also is thread index.
*
*  pdata: If it's a packet which should send to asd, the pdata would contain the buffer head
*              of send to asd.
*  client_sock: in this func, the ip and port_num will be used in there.
*
* Return:
*  Type of data, EAP or MGMT or data packet or error.
*
*/
int parse_fromWTP_msg(int index, DataMsgHead *pdata, struct sockaddr_storage client_sock, int fd)
{
	unsigned int wtpid, bssindex, bss_local_index;
	unsigned int radio_g_id, data_type;
	unsigned short isEAP;
	WTPIP_WTPID_Element *wtpip_wtpid_element = NULL;
	BSSID_BSSIndex_Element *bssid_bssindex_element = NULL;
	BSS_Element *pbss = NULL;
	STA_Element *p_sta = NULL;
	unsigned char proto_ver, type, toDS_fromDS, qos_flag;
	CWProtocolMessage *tmpMsg; 
	unsigned char BSSID[MAC_LEN];
	unsigned char STAMAC[MAC_LEN];
	unsigned char broadcast_addr[MAC_LEN];
	unsigned char proto_type = 0;
	struct mixwtpip ip;
	struct sockaddr_in6 *ipv6_sock = NULL;
	struct sockaddr *p_sock = NULL;
	unsigned short int port_no = 0;
	wtpid = 0;
	ipv6_sock = (struct sockaddr_in6*)&client_sock;
	bzero(&ip, sizeof(ip));
	memset(broadcast_addr, 0xff, MAC_LEN);
	p_sock = (struct sockaddr*)&client_sock;

	if (p_sock->sa_family == AF_INET)
	{
		ip.addr_family = AF_INET;
		ip.m_v4addr = ((struct sockaddr_in*)p_sock)->sin_addr.s_addr;
		ip.port = ((struct sockaddr_in*)p_sock)->sin_port;
		port_no = ((struct sockaddr_in*)p_sock)->sin_port;
		
	}
	else
	{
		/* Normal IPv6 addr or IPv4 mapped IPv6 addr */
		proto_type = IN6_IS_ADDR_V4MAPPED(ipv6_sock->sin6_addr.s6_addr);
		/* client is IPv4 socket */
		if (proto_type)
		{
			ip.addr_family = AF_INET;
			GET_MAPPED_V4ADDR(ip.m_v4addr, ipv6_sock->sin6_addr.s6_addr);
		}
		else /* client is IPv6 socket */
		{
			ip.addr_family = AF_INET6;
			memcpy(ip.m_v6addr, ipv6_sock->sin6_addr.s6_addr, MIXIPLEN);
		}
		ip.port = ipv6_sock->sin6_port;
		port_no = ipv6_sock->sin6_port;
	}
	
	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
	/*wuwl move wtpip_wtpid_element after  bssid_bssindex_element in order to calc wtpid from bssindex, for nat support*/
	#if 0
	/* Get WTPID by WTPIP */
	wtpip_wtpid_element = (WTPIP_WTPID_Element *)wsm_tbl_search(WTPIP_WTPID_TYPE, 
				(unsigned char*)&ip);

	if (wtpip_wtpid_element == NULL)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_WARNING, "%s: Search WTPID_WTPIP table by WTPIP failed.\n", __func__);
		return WSM_DATA_ERR;
	}

	wtpid = wtpip_wtpid_element->WTPID;
	/* Initialize WTP sockaddr */
	if (WTPtable[wtpid].WTPportNum == 0)
	{
		/* If mov this off this case, then the wsm support dynamic WTP IP address */
		WTPtable[wtpid].WTPportNum = port_no;
		memcpy(&WTPtable[wtpid].WTPSock, &client_sock, sizeof(struct sockaddr_storage));
		WTPtable[wtpid].ac_fd = fd;
	}
	
#endif
	/* Parse 802.11 */
	tmpMsg = toWifiBuff + index;
	proto_ver = (tmpMsg->msg)[tmpMsg->offset] & IEEE80211_FC0_VERSION_MASK;
	type = (tmpMsg->msg)[tmpMsg->offset] & IEEE80211_FC0_TYPE_MASK;
	qos_flag = (tmpMsg->msg)[tmpMsg->offset] & IEEE80211_FC0_SUBTYPE_QOS;
	toDS_fromDS = (tmpMsg->msg)[tmpMsg->offset + 1] & IEEE80211_FC1_DIR_MASK;

	if (proto_ver != IEEE80211_FC0_VERSION_0)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_DEBUG, "%s: 802.11 frame version error.\n", __func__);
		return WSM_DATA_ERR;
	}
	
	/* Get BSSID and STAMAC */
	switch(toDS_fromDS)
	{ 
		case IEEE80211_FC1_DIR_TODS: /* To DS */
			memcpy(BSSID, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR1_START), MAC_LEN); 
			memcpy(STAMAC, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR2_START), MAC_LEN);
			break;
 
		case IEEE80211_FC1_DIR_NODS: /* Ad hoc */
			memcpy(BSSID, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR3_START), MAC_LEN);
			memcpy(STAMAC, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR2_START), MAC_LEN); 
			break;
 
		case IEEE80211_FC1_DIR_FROMDS: /* From DS */
			memcpy(BSSID, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR2_START), MAC_LEN); 
			memcpy(STAMAC, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR3_START), MAC_LEN); 
			break;

		case IEEE80211_FC1_DIR_DSTODS: /* WDS */
		default:
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			WSMLog(L_WARNING, "%s: toDS_fromDS error. value = %d\n", __func__, toDS_fromDS);
			return WSM_DATA_ERR;
		}
	/* Check out the validity of BSS */
	bssid_bssindex_element = (BSSID_BSSIndex_Element *)wsm_tbl_search(BSSID_BSSIndex_TYPE, BSSID,wtpid);
	

	/* For STA roaming */
	sta_roam_check(STAMAC, bssid_bssindex_element);
	if(bssid_bssindex_element)
		bssindex = bssid_bssindex_element->BSSIndex;
	/*wuwl modify wtpid from bssindex for nat support*/
	wtpid = bssindex / BSS_ARRAY_SIZE;
	WSMLog(L_DEBUG, "%s:bssindex=%d, wtpid=%d.\n", __func__,bssindex, wtpid);
	/* Get WTPID by WTPIP */
	wtpip_wtpid_element = (WTPIP_WTPID_Element *)wsm_tbl_search(WTPIP_WTPID_TYPE, 
				(unsigned char*)&ip,wtpid);

	if (wtpip_wtpid_element == NULL)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_WARNING, "%s: Search WTPID_WTPIP table by WTPIP failed.\n", __func__);
		return WSM_DATA_ERR;
	}

	//wtpid = wtpip_wtpid_element->WTPID;//wuwl move for nat support,now calc wtpid frome bssindex
	/* Initialize WTP sockaddr */
	if (((wtpid > 0)&&wtpid < HASHTABLE_SIZE)&&(WTPtable[wtpid].WTPportNum == 0))
	{
		/* If mov this off this case, then the wsm support dynamic WTP IP address */
		WTPtable[wtpid].WTPportNum = port_no;
		memcpy(&WTPtable[wtpid].WTPSock, &client_sock, sizeof(struct sockaddr_storage));
		WTPtable[wtpid].ac_fd = fd;
	}
#if 0
	/* Parse 802.11 */
	tmpMsg = toWifiBuff + index;
	proto_ver = (tmpMsg->msg)[tmpMsg->offset] & IEEE80211_FC0_VERSION_MASK;
	type = (tmpMsg->msg)[tmpMsg->offset] & IEEE80211_FC0_TYPE_MASK;
	qos_flag = (tmpMsg->msg)[tmpMsg->offset] & IEEE80211_FC0_SUBTYPE_QOS;
	toDS_fromDS = (tmpMsg->msg)[tmpMsg->offset + 1] & IEEE80211_FC1_DIR_MASK;

	if (proto_ver != IEEE80211_FC0_VERSION_0)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_DEBUG, "%s: 802.11 frame version error.\n", __func__);
		return WSM_DATA_ERR;
	}

	/* Get BSSID and STAMAC */
	switch(toDS_fromDS)
	{ 
		case IEEE80211_FC1_DIR_TODS: /* To DS */
			memcpy(BSSID, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR1_START), MAC_LEN); 
			memcpy(STAMAC, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR2_START), MAC_LEN);
			break;

		case IEEE80211_FC1_DIR_NODS: /* Ad hoc */
			memcpy(BSSID, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR3_START), MAC_LEN);
			memcpy(STAMAC, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR2_START), MAC_LEN); 
			break;

		case IEEE80211_FC1_DIR_FROMDS: /* From DS */
			memcpy(BSSID, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR2_START), MAC_LEN); 
			memcpy(STAMAC, (tmpMsg->msg + tmpMsg->offset + IEEE80211_ADDR3_START), MAC_LEN); 
			break;

		case IEEE80211_FC1_DIR_DSTODS: /* WDS */
		default:
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			WSMLog(L_WARNING, "%s: toDS_fromDS error. value = %d\n", __func__, toDS_fromDS);
			return WSM_DATA_ERR;
		}
#endif	

	/*wtpid modify end*/
	if (wtpid != bssindex / BSS_ARRAY_SIZE)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_ERR, "%s: WTPID or BSSIndex error WTPID*32!=BSSIndex.\n", __func__);
		return WSM_DATA_ERR;
	}
	bss_local_index = bssindex % BSS_ARRAY_SIZE;
	if (WTPtable[wtpid].next == NULL)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_ERR, "%s: Found empty WTP[%u].\n", __func__, wtpid);
		return WSM_DATA_ERR;
	}
	pbss = WTPtable[wtpid].next;
	pbss += bss_local_index;

	if ((pbss->flag  == 0) || (bssindex != pbss->BSS.BSSIndex))
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_WARNING, "%s: BSS nonexistent or BssIndex not same.\n", __func__);
		return WSM_DATA_ERR;
	}
	radio_g_id = bssindex / BSS_PER_RADIO;

	/* Fill in DataMsgHead */
	pdata->BSSIndex = bssindex;
	pdata->DataLen = tmpMsg->msgLen;
	pdata->Radio_G_ID = radio_g_id;
	pdata->WTPID = wtpid;

	/* Judge Message type */
	switch(type)
	{
		case IEEE80211_FC0_TYPE_DATA:
			p_sta = pbss->next;
			/* Check out the validity of STA */ 
			while((p_sta != NULL)&&(memcmp(p_sta->STA.STAMAC, STAMAC, MAC_LEN) != 0))
			{
				p_sta = p_sta->next;
			}
			/* can not find the STA */
			if(p_sta == NULL)
			{
				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
				WSMLog(L_WARNING, "%s: Cannot find STA[%02x:%02x:%02x:%02x:%02x:%02x] "
						"in BSS[%02x:%02x:%02x:%02x:%02x:%02x].\n", __func__, STAMAC[0], 
						STAMAC[1], STAMAC[2], STAMAC[3], STAMAC[4], STAMAC[5], 
						pbss->BSS.BSSID[0], pbss->BSS.BSSID[1], pbss->BSS.BSSID[2], 
						pbss->BSS.BSSID[3], pbss->BSS.BSSID[4], pbss->BSS.BSSID[5]);
				return WSM_DATA_ERR;
			}
			/* Only EAP data can be send to ASD, and other should be send to Wifi.ko */
			if (qos_flag)
				memcpy(&isEAP, (tmpMsg->msg + tmpMsg->offset + IEEE802_11_HEAD_LEN + 4), 2);
			else
				memcpy(&isEAP, (tmpMsg->msg + tmpMsg->offset + IEEE802_11_HEAD_LEN), 2);
				
			/* CAPWAP Data tunnel Data */
			if ((isEAP != 0x888E) && (isEAP != 0x88C7) && (isEAP != 0x88b4))
			{	
				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
				return WSM_DATA_TYPE;
			}
				
			/* EAP Data that should send to ASD */
			data_type = IEEE802_11_EAP;
			pdata->Type = data_type;
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			return WSM_EAP_TYPE;
				
		case IEEE80211_FC0_TYPE_MGT:
			data_type = IEEE802_11_MGMT;
			pdata->Type = data_type;
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			return WSM_MGMT_TYPE;
				
		case IEEE80211_FC0_TYPE_CTL:
		default:
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			return WSM_DATA_ERR;
	}

}


/**
* Description:
*   Send the message which recv from WTP to ASD
*
* Parameter:
*  index: Number of Queue,in the wsm there has 16 Queues, and every 
*              queue has self-governed thread to deal with its. the index is
*              queue number, but also is thread index.
*  offset: buffer offset
*  datamsghead: private header used between wsm and asd.
* 
* Return:
*   0: successed; -1: error
*
*/
int WTPmsg_to_ASD(int index, int offset, DataMsgHead datamsghead)
{
	CWProtocolMessage *tmpMsg;

	tmpMsg = toWifiBuff + index;
	tmpMsg->msg = tmpMsg->msg + offset;
	tmpMsg->msgLen = tmpMsg->msgLen + 20;
	memcpy(tmpMsg->msg, (unsigned char*)&datamsghead, sizeof(datamsghead));

	pthread_mutex_lock(&sta_statistics_mutex);
	if(sendto(iASDsocket, tmpMsg->msg, tmpMsg->msgLen, 0, 
			(struct sockaddr *)&WSM_ASD_local_sockaddr, glen) <= 0)
	{
		pthread_mutex_unlock(&sta_statistics_mutex);
		WSMLog(L_INFO, "%s: sendto failed.\n", __func__);
		return -1;
	}
	pthread_mutex_unlock(&sta_statistics_mutex);
	
	return 0;
}


/**
* Description:
*  Assemble 802.11 frame to 802.3 frame and send it to Wifi.ko
*
* Parameter:
*  index: Number of Queue,in the wsm there has 16 Queues, and every 
*              queue has self-governed thread to deal with its. the index is
*              queue number, but also is thread index.
*
*  buff: packet buffer(shared memory)
*
* Return:
*  0: Successed; -1: error
*
*/
int WTPmsg_to_Wifi(int index, unsigned char *buff)
{
	CWProtocolMessage *tmpMsg = toWifiBuff + index;
	struct wsm_stamac_bssid_wtpip *bssid_data = NULL;
	struct wsm_bssid_wlanid *wlanid_data = NULL;
	BSSID_BSSIndex_Element *bssindex_data = NULL;
	struct sockaddr_in roam_addr;
	unsigned int bufflen;
	unsigned char retry_flag = 0;
	unsigned char stamac[MAC_LEN] = {0};
	unsigned char isBroadcast = 0;
	unsigned char broadcast[MAC_LEN] = {0};
	unsigned char *pmac = NULL;
	unsigned char str_ip[64] = {0};
	unsigned char *p_ip = NULL;
	struct page_head *page = NULL;
	
	memset(broadcast, 0xff, MAC_LEN);
	bzero(&roam_addr, sizeof(struct sockaddr_in));
	page = (struct page_head*)buff;
	
	/* 802.11 frame ==> 802.3 frame */
	if (ieee80211_to_ieee8023(index, &retry_flag, stamac))
	{
		WSMLog(L_DEBUG, "%s: Transfer 802.11 frame to Ethernet frame failed.\n", __func__);
		return -1;
	}
	
	((struct page_head*)buff)->offset = tmpMsg->offset + PAGE_HEAD_LEN;
	
	/* Ethernet frame is larger than 64 bytes */
	if (tmpMsg->msgLen < 64)
		bufflen = ((struct page_head*)buff)->len = 64;									
	else
		bufflen = ((struct page_head*)buff)->len = tmpMsg->msgLen;

	/*  Broadcast or unicast */
	if (memcmp((buff + ((struct page_head*)buff)->offset), broadcast, MAC_LEN))
		isBroadcast = 0;
	else
		isBroadcast = 1;
	
	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
	
	/* Get BSSID_DATA by STAMAC */
	bssid_data = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
				tmpMsg->msg + tmpMsg->offset + MAC_LEN, 
				WSM_STAMAC_BSSID_WTPIP_TYPE);
	if (bssid_data == NULL)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		pmac = tmpMsg->msg + tmpMsg->offset + MAC_LEN;
		WSMLog(L_WARNING, "%s: Cannot find BSSID data by STA[%02x:%02x:%02x:"
				"%02x:%02x:%02x].\n", __func__, pmac[0], pmac[1], pmac[2], pmac[3],
				pmac[4], pmac[5]);
		/* Err packet */
		wsm_sta_statistics_update(stamac, 0, WSM_STA_ERR_TYPE);
		return -1;
	}


	/* Transmit the packet to Raw AC for Inter-AC roaming by Guo Xuebin, 2009-12-17 */
	if (is_instance0 && bssid_data->ac_roam_tag == ROAMING_AC)
	{
		WSMLog(L_DEBUG, "%s: Roaming AC recv pkt from CW data port, trasmit"
				" the pkt to raw AC.\n", __func__);
		if (bssid_data->ac_ip.addr_family != AF_INET)
		{
			WSMLog(L_ERR, "%s: ac_ip addr_family = %d, unsupport.\n", 
					__func__, bssid_data->ac_ip.addr_family);
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			return -1;
		}
		
		p_ip = (char *)(&bssid_data->ac_ip.m_v4addr);
		sprintf(str_ip, "%d.%d.%d.%d", p_ip[0], p_ip[1], p_ip[2], p_ip[3]);
		roam_addr.sin_family = AF_INET;
		roam_addr.sin_port = htons(CW_RAW_AC_DATA_PORT);
		roam_addr.sin_addr.s_addr = inet_addr(str_ip);
		
		if (sendto(raw_ac_fd, buff + page->offset, page->len, 0, 
			(struct sockaddr *)&roam_addr, sizeof(struct sockaddr_in)) < 0)
		{
			WSMLog(L_INFO, "%s: send to raw-ac failed.\n", __func__);
		}
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return -1;
	}
	
	
	/* Get Wlan Data by BSSID. Add roaming flag for roaming by GuoXuebin 2009-05-06*/
	if (bssid_data->roaming_flag)
	{
		wlanid_data = (struct wsm_bssid_wlanid *)wsm_wifi_table_search(
				bssid_data->BSSID_before, WSM_BSSID_WLANID_TYPE);
	}
	else
		wlanid_data = (struct wsm_bssid_wlanid *)wsm_wifi_table_search(
				bssid_data->BSSID,WSM_BSSID_WLANID_TYPE);
	
	if (wlanid_data == NULL)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		if (bssid_data->roaming_flag)
			WSMLog(L_WARNING, "%s: Cannot find WlanID data by BSSID[%02x:"
					"%02x:%02x:%02x:%02x:%02x].\n", __func__,
					bssid_data->BSSID_before[0], bssid_data->BSSID_before[1], 
					bssid_data->BSSID_before[2], bssid_data->BSSID_before[3], 
					bssid_data->BSSID_before[4], bssid_data->BSSID_before[5]);
		else
			WSMLog(L_WARNING, "%s: Cannot find WlanID data by BSSID[%02x:"
					"%02x:%02x:%02x:%02x:%02x].\n", __func__, 
					bssid_data->BSSID[0], bssid_data->BSSID[1], bssid_data->BSSID[2], 
					bssid_data->BSSID[3], bssid_data->BSSID[4], bssid_data->BSSID[5]);
		/* Err packet */
		wsm_sta_statistics_update(stamac, 0, WSM_STA_ERR_TYPE);
		return -1;
	}
	/* Get BSSIndex */
	/* Add roaming flag by GuoXuebin 2009-05-06 */
	if (bssid_data->roaming_flag)
	{
		bssindex_data = (BSSID_BSSIndex_Element *)wsm_tbl_search(
			BSSID_BSSIndex_TYPE, bssid_data->BSSID_before,0);
	}
	else
		bssindex_data = (BSSID_BSSIndex_Element *)wsm_tbl_search(
			BSSID_BSSIndex_TYPE, bssid_data->BSSID,0);

	if (bssindex_data == NULL)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_WARNING, "%s: Cannot find BSSIndex data by BSSID[%02x:"
				"%02x:%02x:%02x:%02x:%02x].\n", __func__, 
				bssid_data->BSSID[0], bssid_data->BSSID[1], bssid_data->BSSID[2], 
				bssid_data->BSSID[3], bssid_data->BSSID[4], bssid_data->BSSID[5]);
		/* Err packet */
		wsm_sta_statistics_update(stamac, 0, WSM_STA_ERR_TYPE);
		return -1;
	}

	/* Judge L3IF type */
	if ((wlanid_data->wlan_if_type == WLAN_IF && wlanid_data->bss_if_type == NO_IF) ||
	    (wlanid_data->wlan_if_type == NO_IF && wlanid_data->bss_if_type != BSS_IF))
	{
		WSMLog(L_WARNING, "%s: Interface type error.\n", __func__);
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		/* Err packet */
		wsm_sta_statistics_update(stamac, 0, WSM_STA_ERR_TYPE);
		return -1;
	}
	
	if (wlanid_data->bss_if_type == BSS_IF)
	{
		((struct page_head*)buff)->Wlanid = 0;
		((struct page_head*)buff)->BSSIndex = bssindex_data->BSSIndex;
	}
	else if ((wlanid_data->wlan_if_type == WLAN_IF && wlanid_data->bss_if_type == WLAN_IF))
	{
		((struct page_head*)buff)->Wlanid = wlanid_data->WlanID;
		((struct page_head*)buff)->BSSIndex = 0;
	}
	else
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return -1;
	}
		
	WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
	set_mem_index(WSM_TOWIFI_TYPE, index, buff);
	
	/* STA packets statistics */ 
	if (retry_flag)
	{
		wsm_sta_statistics_update(bssid_data->STAMAC, bufflen, 
				WSM_STA_RETRY_TYPE);
	}
	else
	{
		if (isBroadcast)
			wsm_sta_statistics_update(bssid_data->STAMAC, bufflen, 
					WSM_STA_TX_BRD_TYPE);
		else
			wsm_sta_statistics_update(bssid_data->STAMAC, bufflen, 
					WSM_STA_TX_UNI_TYPE);
	}
	
	return 0;	
}

/**
* Description:
*  Mainly thread of wsm module. Receive message from ASD and send it to WTP. 
*
* Parameter:
*  Void
*
* Return:
*  Void.
*
*/
void WSM_recv_DataMsg(void)
{
	int ERROR = -1;
	struct sockaddr_un localaddr;
	struct sockaddr_storage wtp_addr;
	unsigned int val = 0;
	int sendsize = 0;
	int readBytes = 0;
	DataMsgHead *p = NULL;
	unsigned char	WlanID;
	unsigned int Radio_L_ID = 0;
	unsigned int BSS_Local_Index = 0;
	BSS_Element *pBSS = NULL;
	unsigned char *buff = NULL;
	unsigned int WTPID = 0;

	while(1)
	{		
		if(recvfrom(iDATAsocket, DATAmsg.msg, dot11_Max_Len-1, 0, &localaddr,
				&readBytes) <= 0)
		{
			WSMLog(L_CRIT, "%s: recvfrom failed.\n", __func__);
			continue;
		}
				
		p = (DataMsgHead *)DATAmsg.msg;
		WTPID = p->WTPID;
		BSS_Local_Index = (p->BSSIndex) % BSS_ARRAY_SIZE;

		if (WTPID >= HASHTABLE_SIZE || WTPID == 0)
		{
			WSMLog(L_ERR, "%s: WTPID[%d] out of range, send to wtp msg failed\n",
					__func__, WTPID);
			continue;
		}
		if (WTPtable[WTPID].flag == 0 || WTPtable[WTPID].WTPportNum == 0)
		{
			WSMLog(L_ERR, "%s: WTPID[%d] is not active or not has sockadrr data.\n",
					__func__, WTPID);
			continue;
		}
		
		WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
		pBSS = WTPtable[WTPID].next;
		if (pBSS == NULL)
		{
			WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
			WSMLog(L_ERR, "%s: Cannot find BSS in WTP table.\n", __func__);
			continue;
		}
		pBSS += BSS_Local_Index;
		WlanID = pBSS->BSS.WlanID;
		memcpy(&wtp_addr, &(WTPtable[WTPID].WTPSock), sizeof(struct sockaddr_storage));
		Radio_L_ID = pBSS->BSS.Radio_L_ID;
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		
		DATAmsg.msgLen = p->DataLen + CAPWAP_HEAD_LEN + 4;
		buff = DATAmsg.msg + 4;

		CWSetField32(val, 
		     CW_TRANSPORT_HEADER_VERSION_START,
		     CW_TRANSPORT_HEADER_VERSION_LEN,
		     CW_PROTOCOL_VERSION); /* CAPWAP VERSION */

		CWSetField32(val,
		     CW_TRANSPORT_HEADER_TYPE_START,
		     CW_TRANSPORT_HEADER_TYPE_LEN,
		     0);
	
		CWSetField32(val,
		     CW_TRANSPORT_HEADER_HLEN_START,
		     CW_TRANSPORT_HEADER_HLEN_LEN,
		     4/*16*/);

		CWSetField32(val,
		     CW_TRANSPORT_HEADER_RID_START,
		     CW_TRANSPORT_HEADER_RID_LEN,
		     Radio_L_ID); /* Radio local ID */
	
		CWSetField32(val,
		     CW_TRANSPORT_HEADER_WBID_START,
		     CW_TRANSPORT_HEADER_WBID_LEN,
		     1); /* Wireless Binding ID */
	
		CWSetField32(val,
		     CW_TRANSPORT_HEADER_T_START,
		     CW_TRANSPORT_HEADER_T_LEN,
		     1);

		CWSetField32(val,
		     CW_TRANSPORT_HEADER_F_START,
		     CW_TRANSPORT_HEADER_F_LEN,
		     0); /* is fragment */

		CWSetField32(val,
		     CW_TRANSPORT_HEADER_L_START,
		     CW_TRANSPORT_HEADER_L_LEN,
		     0); /* last fragment */
	
		CWSetField32(val,
		     CW_TRANSPORT_HEADER_W_START,
		     CW_TRANSPORT_HEADER_W_LEN,
		     1); /* have wireless option header */
	

		CWSetField32(val,
		     CW_TRANSPORT_HEADER_M_START,
		     CW_TRANSPORT_HEADER_M_LEN,
		     0); /* no radio MAC address */

		CWSetField32(val,
		     CW_TRANSPORT_HEADER_K_START,
		     CW_TRANSPORT_HEADER_K_LEN,
		     0); /* Keep alive flag */

		CWSetField32(val,
		     CW_TRANSPORT_HEADER_FLAGS_START,
		     CW_TRANSPORT_HEADER_FLAGS_LEN,
		     0); /* required */
		     
		val = htonl(val);
		WSMProtocolStore32(buff, val);
		buff += 4;
		val = 0;
		
		CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN,
		     0); /* fragment ID */
	
		CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN,
		     0); /* fragment offset */

		CWSetField32(val,
		     CW_TRANSPORT_HEADER_RESERVED_START,
		     CW_TRANSPORT_HEADER_RESERVED_LEN,
		     0); /* required */
		     
		val = htonl(val);
		WSMProtocolStore32(buff, val);
		buff += 4;
		val = 0;
		CWSetField32(val, 0, 8, WlanID);
		val = htonl(val);
		WSMProtocolStore32(buff, val);
		buff += 4;
		val = 0;
		val = htonl(val);
		WSMProtocolStore32(buff, val);
		pthread_mutex_lock(&split_send_mutex);
		if (WTPtable[WTPID].ac_fd <= 0)
		{
			WSMLog(L_WARNING, "%s: ac file descriptor is %d, error.\n", 
					__func__, WTPtable[WTPID].ac_fd);
			pthread_mutex_unlock(&split_send_mutex);
			continue;
		}
		
		if((sendsize = sendto(WTPtable[WTPID].ac_fd, DATAmsg.msg + 4, DATAmsg.msgLen,
			0, (struct sockaddr *)&wtp_addr, sizeof(struct sockaddr_in6))) < 0)
		{		
			WSMLog(L_INFO, "%s: sendto failed, return val:%d.\n", __func__, sendsize);
			pthread_mutex_unlock(&split_send_mutex);
			continue;
		}
		pthread_mutex_unlock(&split_send_mutex);		
	}
	
}


/**
* Description:
*  Updata WTP-BSS-STA relation tables by the message which received from ASD.
*
* Parameter:
*  void.
*
* Return:
*  void
*
*/

void WSM_recv_TableMsg(void)
{
	int ERROR = 0;
	struct sockaddr_un localaddr;
	int readBytes = 0;
	TableMsg *p = NULL;
	/*
	int flags = fcntl(iTABLEsocket, F_GETFL);
	if (flags < 0) return -1;
	*/
	
	while(1)
	{
		/* fcntl(iTABLEsocket, F_SETFL, flags & (~O_NONBLOCK)); */	
		if(recvfrom(iTABLEsocket, TABLEmsg.msg, dot11_Max_Len-1, 0, &localaddr,
				&readBytes) <= 0)
		{
			WSMLog(L_CRIT, "%s: recvfrom failed.\n", __func__);
			continue;
		}
			
		/* fcntl(iTABLEsocket, F_SETFL, O_NONBLOCK); */
		p = (TableMsg *)TABLEmsg.msg;
		ERROR = 0;

		switch(p->Op)
		{
			case WID_ADD:
				ERROR = 1;
				WSMLog(L_DEBUG, "%s: WID_ADD before lock.\n", __func__);
				
				WSM_WRLOCK_LOCK(&wsm_tables_rwlock);
				/* Not has STA hash collision */
				if (sta_collision_detection(p))
				{
					ERROR = wsm_tbl_add(p->Type,  (void *)&(p->u));
					if (!tunnel_run)
					{
						if (ERROR)
						{
							WSMLog(L_DEBUG, "%s: Set watch dog tunnel_run = 1.\n",
									__func__);
							tunnel_run = 1;
						}
						else
						{
							WSMLog(L_DEBUG, "%s: Set watchdog ERROR != 1.\n", 
									__func__);
						}
					}
			
					WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
					WSMLog(L_DEBUG, "%s: WID_ADD end lock.\n", __func__);
					break;
				}
				
				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
				WSMLog(L_DEBUG, "%s: WID_ADD end lock.\n", __func__);
				
				break;
							
			case WID_DEL:
				WSMLog(L_DEBUG, "%s: WID_DEL before lock.\n", __func__);
				WSM_WRLOCK_LOCK(&wsm_tables_rwlock);
				ERROR = wsm_tbl_del(p->Type,  (void *)&(p->u));
				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
				WSMLog(L_DEBUG, "%s: WID_DEL end lock.\n", __func__);
					
				#ifdef WSM_TABLE_PRINT
				wsm_wifi_table_print(WSM_BSSID_WLANID_TYPE);
				wsm_wifi_table_print(WSM_STAMAC_BSSID_WTPIP_TYPE);
				#endif
						
				break;
						
			case  WID_MODIFY:
				WSMLog(L_DEBUG, "%s: WID_MODIFY before lock.\n", __func__);
				WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
				ERROR = wsm_tbl_modify(p->Type, (void*)&(p->u));
				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
				WSMLog(L_DEBUG, "%s: WID_MODIFY end lock.\n", __func__);
					
				break;
			case VRRP_IF:
				WSMLog(L_DEBUG, "%s: before vrrp_if type\n", __func__);
				if (vrrp_if_reg(&p->u.vrrp_if) == 0)
					ERROR = 1;
				else
					ERROR = 0;
				WSMLog(L_DEBUG, "%s: end vrrp_if type.\n", __func__);
				
				break;
/*			case STA_INFO:
				wsm_get_STAStatistics();
				ERROR = 1;		
				break;
						
			case BSS_INFO:
				ERROR = wsm_get_BSSStatistics(p->u.bss_header.WTPID);		
				break;
*/						
			default:
				break;
		}
					
		if (ERROR != 1)
		{
			WSMLog(L_ERR, "%s: Op[%d], Type[%d] Update table failed.\n", 
					__func__, p->Op, p->Type);
		}
	}
}


/**
* Description:
*  Receive message from WTPs, maybe CAPWAP data channel data or MGMT,EAP to ASD.
*
* Parameter:
*  p: Number of Queue,in the wsm there has 16 Queues, and every 
*        queue has self-governed thread to deal with its. the index is
*        queue number, but also thread index.
*
* Return:
*  void.
*
*/
void WSM_recv_WTP(void *p)
{
	CWProtocolTransportHeaderValues CAPWAPheadvalue;
	DataMsgHead datamsghead;
	unsigned char *buff;
	int index = *((int*)p);
	unsigned int flag = 1;
	unsigned long core_mask = flag << index;
	int buff_offset = 0;
	int data_type = 0;
	int tmp_index = 0;
	
	bzero(&CAPWAPheadvalue, sizeof(CAPWAPheadvalue));
	/* Set CPU affinity */
	if (sched_setaffinity(0, sizeof(core_mask), &core_mask) < 0)
	{
		WSMLog(L_WARNING, "%s: sched_setaffinity failed. CoreID = %d\n",
				__func__, index);
	}

	while(1)
	{
		buff = wsm_get_mem_node(WSM_TOWIFI_TYPE, index, &tmp_index);
		memcpy(&toWifiBuff[index], &(gToWifiData[index][tmp_index].cw_data), 
				sizeof(CWProtocolMessage));

		/* RAW_AC and ROAM_AC type judged by offset length */
		if (is_instance0)
		{
			/* RAW AC received buffer */
			if (toWifiBuff[index].offset == RAW_AC_OFFSET)
			{
				WSMLog(L_DEBUG, "%s: raw_ac receive packet from roam_ac.\n", __func__);
				raw_ac_routine(buff, toWifiBuff[index].msgLen);
				set_mem_index(WSM_TOWIFI_TYPE, index, buff);
				continue;
			}
			/* Roam AC received buffer */
			if (toWifiBuff[index].offset == ROAM_AC_OFFSET)
			{
				WSMLog(L_DEBUG, "%s: roam_ac receive packet from raw ac.\n", __func__);
				roam_ac_routine(buff, toWifiBuff[index].msgLen);
				set_mem_index(WSM_TOWIFI_TYPE, index, buff);
				continue;
			}
		}
		
		/* Reset msg and offset */
		buff_offset = toWifiBuff[index].offset - WSM_BUFF_OFFSET;
		
		toWifiBuff[index].offset = CAPWAP_HEAD_LEN + CAPWAP_HEAD_ASSIGN + buff_offset;
		toWifiBuff[index].msg = buff + PAGE_HEAD_LEN;
		WSMLog(L_DEBUG, "%s: Get shared memory buffer ok.\n", __func__);
		
		switch(parse_fromWTP_msg(index, &datamsghead, gToWifiData[index][tmp_index].sock_data,
				gToWifiData[index][tmp_index].ac_fd))
		{
			/* Send to Wifi */
			case WSM_DATA_TYPE:
				/* 802.11 ==> 802.3: Successed */
				if (!WTPmsg_to_Wifi(index, buff))
				{
					buff = NULL;
					continue;
				}
				else
				{	
					set_mem_index(WSM_TOWIFI_TYPE, index, buff);
					continue;
				}
			/* Send to ASD */
			case WSM_MGMT_TYPE:
			case WSM_EAP_TYPE:
				WTPmsg_to_ASD(index, buff_offset, datamsghead);
			/* Discard */	
			case WSM_DATA_ERR:
				set_mem_index(WSM_TOWIFI_TYPE, index, buff);		
		}
	}
}


/**
* Description:
*  Main thread: Receive message from wifi.ko and send the packet to WTP
*
* Parameter:
*  p: Number of Queue,in the wsm there has 16 Queues, and every 
*        queue has self-governed thread to deal with its. the index is
*        queue number, but also is thread index.
*
* Return:
*  Void.
*
*/

void WSM_recv_Wifi(void *p)
{
	unsigned char *buff = NULL;
	unsigned int index = *((int*)p);
	unsigned int flag = 1;
	unsigned long core_mask = flag << index;
	unsigned int wireless_cnt = 0;
	struct wsm_stamac_bssid_wtpip *stamac_bssid_data;
	struct wsm_bssid_wlanid *bssid_wlanid_data;
	unsigned char broadcastMac[MAC_LEN];
	unsigned char multaddr[MAC_LEN] = {0};
	struct wsm_wlanid_bssid *pbssid;
	struct sockaddr_in roam_addr;
	WSMData_T wsmdata;
	WSMData_T *pdata = &wsmdata;
	unsigned int len = 0;
	unsigned int *wlanid = NULL;
	unsigned int *bssindex = NULL;
	unsigned int fixed_offset = 54;
	unsigned int head_len = 16;
	unsigned int wlanid_offset = 12;
	unsigned int bssindex_offset = 16;
	unsigned int len_offset = 4;
	unsigned int cw_offset = 20;
	unsigned char head_flag = 0;
	unsigned int tmp_wtpid;
	BSS_Element *tmp_BSS;
	unsigned int tmp_wlanid;
	unsigned int protect_type = 0;
	unsigned char isBroadcast = 0;
	STA_Element *sta_data = NULL;
	unsigned char *pmac = NULL;
	roam_tbl_t *roam_bss = NULL;
	roam_sta_t *roam_sta = NULL;
	unsigned char *p_ip = NULL;
	unsigned char str_ip[64] = {0};
	roam_head_t *roam_head = NULL;
	struct page_head *page = NULL;	

	/* Set CPU affinity */
	if (sched_setaffinity(0, sizeof(core_mask), &core_mask) < 0)
	{
		WSMLog(L_WARNING, "%s: sched_setaffinity failed. CoreID = %d\n", 
				__func__, index);
	}
	
	/* set broadcastMac to roadcast mac address */
	memset(broadcastMac, 0xff, MAC_LEN);
	multaddr[0] = 1;
	bzero(pdata, sizeof(wsmdata));
	bzero(&roam_addr, sizeof(struct sockaddr_in));
	
	while(1)
	{
		buff = wsm_get_mem_node(WSM_FROMWIFI_TYPE, index, NULL);
		page = (struct page_head*)buff;
		/* Get WlanID or BSSIndex, and make head_flag */
		wlanid = (unsigned int*)(buff + wlanid_offset);
		bssindex = (unsigned int*)(buff + bssindex_offset);
			
		/*
		* head_flag: 
		*    @Bit0: WlanID flag(1)            
		*    @Bit1: BSSIndex flag(1)
		*    @Bit2: 1: Unicast; 0: Broadcast
		*
		*  Example:
		*     if Wlan as L3IF and a unicast packet, so 
		*   head_flag = 0000 0101
		*/
		head_flag = 0;
		if (*wlanid > 0) 
			head_flag |= 0x1;
		if (*bssindex > 0) 
			head_flag |= 0x2;
		#if 0
		if (memcmp(broadcastMac, buff + fixed_offset, MAC_LEN)  != 0)
		{	
			head_flag |= 0x4; //Unicast
			isBroadcast = 0;
		}
		else
		{
			isBroadcast = 1;
		}
		#else
		if ((*(buff + fixed_offset)) & multaddr[0])
		{
			isBroadcast = 1;
		}
		else
		{
			head_flag |= 0x4;
			isBroadcast = 0;
		}
		#endif

		/*
		* head_flag:
		*  Bit:                       2                     1                0
		*          Unicast(1)/Broadcast(0)  BSSIndex(1)  WlanID(1)
		*
		*/
		switch(head_flag)
		{
			/*********************************************************/
			/* Wlan as L3IF and a unicast packet, 101 */
			/*********************************************************/
			case WLAN_AS_L3IF_UNI:
				#if 0 /* Now there hasn't wlan as lay3 interface */
				//Get wireless frame sequence number
				wireless_cnt = getSeqNum();	
				//Lock the read lock of tables
				WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
						
				//Get BSSID by STAMAC
				stamac_bssid_data = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
					buff+fixed_offset, WSM_STAMAC_BSSID_WTPIP_TYPE);
				if (stamac_bssid_data == NULL)
				{
					WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
					set_mem_index(WSM_FROMWIFI_TYPE, index, buff);
					buff = NULL;
					WSMLog(L_WARNING, "%s: Cannot find STAMAC data.\n", __func__);
					break;
				}
					
				//Get WlanID data by BSSID
				pdata->BSSID = stamac_bssid_data->BSSID;	
				bssid_wlanid_data = (struct wsm_bssid_wlanid *)wsm_wifi_table_search(pdata->BSSID, 
					WSM_BSSID_WLANID_TYPE);
				if (bssid_wlanid_data == NULL)
				{
					WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
					set_mem_index(WSM_FROMWIFI_TYPE, index, buff);
					buff = NULL;
					bzero(pdata, sizeof(wsmdata));
					WSMLog(L_WARNING, "%s: Cannot find BSSID data.\n", __func__);
					break;
				}

				//check type of interface and drop validity packet
				if ((bssid_wlanid_data->wlan_if_type != WLAN_IF) ||
						(bssid_wlanid_data->bss_if_type != WLAN_IF))
				{
					WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
					set_mem_index(WSM_FROMWIFI_TYPE, index, buff);
					buff = NULL;
					bzero(pdata, sizeof(wsmdata));
					WSMLog(L_WARNING, "%s: Cannot find BSSID data.\n", __func__);
					break;
				}
						
				pdata->Radio_L_ID = bssid_wlanid_data->Radio_L_ID;
				pdata->WlanID = bssid_wlanid_data->WlanID;
				pdata->WTPIP = stamac_bssid_data->wtpip;
				protect_type = bssid_wlanid_data->protect_type;
						
				//802.3==>802.11
				if (ieee8023_to_ieee80211(buff, ((struct page_head*)buff)->len, head_len, 
					wireless_cnt, pdata->BSSID, protect_type))
				{
					set_mem_index(WSM_FROMWIFI_TYPE, index, buff);
					bzero(pdata, sizeof(wsmdata));
					buff = NULL;
					//Free the read lock of tables
					WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
					WSMLog(L_WARNING, "%s: 802.3 frame to 802.11 frame failed.\n", __func__);
					break;
				}

				//Message Length
				len = *((int*)(buff + len_offset));
	
				//802.11==>CAPWAP
				assemble_capwap(buff, cw_offset, len, pdata);
						
				//Free the read lock of tables
				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);

				set_mem_index(WSM_FROMWIFI_TYPE, index, buff);
				bzero(pdata, sizeof(wsmdata));
				buff = NULL;
				//For statistics STA tx/rx packets information
				wsm_sta_statistics_update(stamac_bssid_data->STAMAC, len, WSM_STA_RX_UNI_TYPE);
				#endif
				break;

			/*********************************************************/
			/* Wlan as L3IF and a broadcast packet 001 */
			/*********************************************************/
			case WLAN_AS_L3IF_MULTI:
				#if 0 /* Now there hasn't wlan as layer 3 interface */
				//Lock the read lock of tables
				WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
						
				//Define Wlan as L3IF, find bssid by wlanid
				if ((pbssid = wsm_get_bss_of_wlan(*wlanid)) == NULL)
				{
					WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
					set_mem_index(WSM_FROMWIFI_TYPE, index, buff); 
					buff = NULL; 
					WSMLog(L_WARNING, "%s: Cannot find BSSID by WlanID.\n", __func__);
					break;
				}

				if (pbssid->wlan_if_type != WLAN_IF)
				{
					WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
					set_mem_index(WSM_FROMWIFI_TYPE, index, buff); 
					bzero(pdata, sizeof(wsmdata));
					buff = NULL;
					break;
				}
						
				while(pbssid != NULL)
				{
					if ((wsm_find_bssid(pbssid->BSSIndex, pdata)) == NULL)
					{
						pbssid = pbssid->next;
						bzero(pdata, sizeof(wsmdata));
						continue;
					}

					//Just broadcast to the BSS that WLAN as L3IF
					if (pbssid->bss_if_type != WLAN_IF)
					{
						pbssid = pbssid->next;
						bzero(pdata, sizeof(wsmdata));
						continue;
					}
					wireless_cnt = getSeqNum();
					protect_type = pbssid->protect_type;
					if (ieee8023_to_ieee80211(buff, ((struct page_head*)buff)->len, head_len, 
						wireless_cnt, pdata->BSSID, protect_type))
					{
						pbssid = pbssid->next;
						bzero(pdata, sizeof(wsmdata));
						continue;
					}
					
					len = *((int*)(buff + len_offset));
					assemble_capwap(buff, cw_offset, len, pdata);
					/* Updata STA broadcast Statistics for per STA in a BSS In the function 
					*  wsm_find_bssid there has checked validity, so I don't check here.
					*
					**/
					sta_data = (WTPtable[pbssid->BSSIndex / BSS_ARRAY_SIZE].next +
						(pbssid->BSSIndex % BSS_ARRAY_SIZE))->next;
								
					wsm_bss_brd_statistics_update(sta_data, len);
								
					bzero(pdata, sizeof(wsmdata));
					pbssid = pbssid->next;
				}
						
				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
				set_mem_index(WSM_FROMWIFI_TYPE, index, buff); 
				bzero(pdata, sizeof(wsmdata));
				buff = NULL;
				#endif
				break;
						
			/*********************************************************/
			/* BSS as L3IF and a broadcast packet  010 */
			/*********************************************************/
			case BSS_AS_L3IF_MULTI:
				WSM_RDLOCK_LOCK(&wsm_tables_rwlock);

				do 
				{
					/* Get BSSID data by BSSIndex from WTPtable */
					tmp_wtpid = (*bssindex) / BSS_ARRAY_SIZE;
					if (WTPtable[tmp_wtpid].flag == 0 || WTPtable[tmp_wtpid].next == NULL)
					{			
						break;
					}
								
					tmp_BSS = WTPtable[tmp_wtpid].next + ((*bssindex) % BSS_ARRAY_SIZE);
					if (tmp_BSS == NULL) 
					{
						WSMLog(L_DEBUG, "%s: WTP[%u], BSS_L_Index[%u] is nonexistent.\n", 
								__func__, tmp_wtpid, (*bssindex) % BSS_ARRAY_SIZE);
						break;
					}
					if (tmp_BSS->flag == 0)
					{
						WSMLog(L_DEBUG, "%s: Local_BSS's flag = 0.\n", __func__);
						break;
					}
						
					/* check out validity of BSS_IF type */
					if (tmp_BSS->BSS.bss_ifaces_type != BSS_IF)
					{
						break;
					}

					
					/* Check whether the BSS has Inter-AC roaming STA or not */
					if (is_instance0)
					{
						roam_bss = roam_tbl_search(tmp_BSS->BSS.BSSID);
						roam_head = (roam_head_t*)(buff + page->offset - sizeof(roam_head_t));
						bzero(str_ip, 64);
					}
					
					if (is_instance0 && roam_bss != NULL)
					{
						roam_sta = roam_bss->sta;
						WSMLog(L_DEBUG, "%s: Raw AC recv packet from wifi, multicast, to roam ac.\n", 
								__func__);
						while (roam_sta != NULL)
						{
							if (roam_sta->ac_ip.addr_family == AF_INET)
							{
								memcpy(roam_head->stamac, roam_sta->stamac, MAC_LEN);
								roam_head->type = ROAM_MULTICAST;
								p_ip = (unsigned char*)(&roam_sta->ac_ip.m_v4addr);
								sprintf(str_ip, "%d.%d.%d.%d", p_ip[0], p_ip[1], p_ip[2], p_ip[3]);
								WSMLog(L_DEBUG, "%s: Mul RawAC ==> RoamingAC, ac_ip = %s\n",
										__func__, str_ip);
								roam_addr.sin_family = AF_INET;
								roam_addr.sin_port = htons(CW_ROAM_AC_DATA_PORT);
								roam_addr.sin_addr.s_addr = inet_addr(str_ip);

								if (sendto(roam_ac_fd, (char*)roam_head, page->len + sizeof(roam_head_t), 0,
									(struct sockaddr*)&roam_addr, sizeof(struct sockaddr_in)) < 0)
								{
									WSMLog(L_INFO, "%s: sendto roaming-ac failed.\n", __func__);
								}
								else
								{
									WSMLog(L_DEBUG, "%s: Raw AC recv Multicast "
											"packet from wifi, to roam ac OK.\n", __func__);
								}
							}
							roam_sta = roam_sta->next;
						}
					}
				
					pdata->BSSID = tmp_BSS->BSS.BSSID;
					pdata->Radio_L_ID = tmp_BSS->BSS.Radio_L_ID;
					pdata->WlanID = tmp_BSS->BSS.WlanID;
					pdata->WTPID = tmp_wtpid;
					protect_type = tmp_BSS->BSS.protect_type;
					wireless_cnt = getSeqNum();

					if (ieee8023_to_ieee80211(buff, ((struct page_head*)buff)->len, 
						head_len, wireless_cnt, pdata->BSSID, protect_type))
						break;
					
					len = *((int*)(buff + len_offset));
					assemble_capwap(buff, cw_offset, len, pdata);
					/* Updata STA broadcast Statistics for per STA in a BSS */
					wsm_bss_brd_statistics_update(tmp_BSS->next, len);
					
					bzero(pdata, sizeof(wsmdata));
				} 
				while (0);

				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
				set_mem_index(WSM_FROMWIFI_TYPE, index, buff); 
				bzero(pdata, sizeof(wsmdata));
				buff = NULL;
				break;
						
			/*-----------------------------------------------------------------------------------*/
			/* BSS as L3IF and a unicast packet 110 */
			/*-----------------------------------------------------------------------------------*/
			case BSS_AS_L3IF_UNI:
			
				WSM_RDLOCK_LOCK(&wsm_tables_rwlock);

				do
				{
					/* Get wireless frame sequence number */
					wireless_cnt = getSeqNum();
					stamac_bssid_data = (struct wsm_stamac_bssid_wtpip *)wsm_wifi_table_search(
						buff+fixed_offset, WSM_STAMAC_BSSID_WTPIP_TYPE);
					if (stamac_bssid_data == NULL)
					{
						pmac = buff+fixed_offset;
						WSMLog(L_DEBUG, "%s: Cannot find STA[%02x:%02x:%02x:%02x:%02x:%02x] data.\n",
								__func__, pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
						break;
					}

					/* Inter-Ac roaming packet */
					if (is_instance0 && stamac_bssid_data->ac_roam_tag == RAW_AC)
					{
						WSMLog(L_DEBUG, "%s: Raw AC recv packet from wifi, Unicast, to roam ac.\n", __func__);
						roam_head = (roam_head_t*)(buff + page->offset - sizeof(roam_head_t));
						
						if (stamac_bssid_data->ac_ip.addr_family != AF_INET)
						{
							WSMLog(L_ERR, "%s: unicast, addr family = %d, unsupport.\n", 
								__func__, stamac_bssid_data->ac_ip.addr_family);
							break;
						}
						memcpy(roam_head->stamac, stamac_bssid_data->STAMAC, MAC_LEN);
						roam_head->type = ROAM_UNICAST;
						p_ip = (unsigned char *)(&stamac_bssid_data->ac_ip.m_v4addr);
						sprintf(str_ip, "%d.%d.%d.%d", p_ip[0], p_ip[1], p_ip[2], p_ip[3]);
						WSMLog(L_DEBUG, "%s: Uni RawAC == > Roaming AC, ac_ip = %s.\n", __func__, str_ip);
						roam_addr.sin_family = AF_INET;
						roam_addr.sin_port = htons(CW_ROAM_AC_DATA_PORT);
						roam_addr.sin_addr.s_addr = inet_addr(str_ip);
						if (sendto(roam_ac_fd, (char*)roam_head, page->len + sizeof(roam_head_t), 0,
									(struct sockaddr*)&roam_addr, sizeof(struct sockaddr_in)) < 0)
						{
							WSMLog(L_INFO, "%s: sendto unicast roaming-ac failed.\n", __func__);
						}
						else
						{
							WSMLog(L_DEBUG, "%s: Raw AC recv packet from wifi, Unicast, "
									"to roam ac OK.\n", __func__);
						}
						break;						
					}
					
					/* Get WlanID data by BSSID */
					pdata->BSSID = stamac_bssid_data->BSSID;
					bssid_wlanid_data = (struct wsm_bssid_wlanid *)wsm_wifi_table_search(pdata->BSSID, 
						WSM_BSSID_WLANID_TYPE);
					if (bssid_wlanid_data == NULL)
					{
						WSMLog(L_WARNING, "%s: Cannot find Wlan data by BSSID"
								"[%02x:%02x:%02x:%02x:%02x:%02x].\n", __func__, 
								pdata->BSSID[0], pdata->BSSID[1], pdata->BSSID[2],
								pdata->BSSID[3], pdata->BSSID[4], pdata->BSSID[5]);
						break;
					}
								
					if (bssid_wlanid_data->bss_if_type != BSS_IF)
					{
						WSMLog(L_WARNING, "%s: bss_if_type error.\n", __func__);
						break;
					}
						
					pdata->Radio_L_ID = bssid_wlanid_data->Radio_L_ID;
					pdata->WlanID = bssid_wlanid_data->WlanID;
					pdata->WTPID = stamac_bssid_data->WTPID;
					protect_type = bssid_wlanid_data->protect_type;
					/* 802.3==>802.11 */
					if (ieee8023_to_ieee80211(buff, ((struct page_head*)buff)->len, head_len, 
							wireless_cnt, pdata->BSSID, protect_type))
					{
						WSMLog(L_WARNING, "%s: Transfer 802.3 frame to 802.11 frame failed.\n", __func__);
						break;
					}

					/* Message Length */
					len = *((int*)(buff + len_offset));
					/* 802.11==>CAPWAP */
					assemble_capwap(buff, cw_offset, len, pdata);
					/* For statistics STA tx/rx packets information */
					wsm_sta_statistics_update(stamac_bssid_data->STAMAC, len, WSM_STA_RX_UNI_TYPE);
				}
				while (0);
						
				WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
				set_mem_index(WSM_FROMWIFI_TYPE, index, buff);
				bzero(pdata, sizeof(wsmdata));
				buff = NULL;
						
				break;
				
			/*-------------------------------------------------------------------*/
			/* Default value */
			/*-------------------------------------------------------------------*/
			default:
				set_mem_index(WSM_FROMWIFI_TYPE, index, buff);
				buff = NULL;
				bzero(pdata, sizeof(wsmdata));
				WSMLog(L_ERR, "%s: Unsupport Type, head_flag = %d", __func__, head_flag);
				break;
		}		
	}
}


