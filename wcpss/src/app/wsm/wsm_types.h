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
* wsm_types.h
*
*
* DESCRIPTION:
*  WSM module public macro or structure define.
*
* DATE:
*  2009-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2008-07-11 <guoxb> Create file.
*  2009-12-14 <guoxb> Add Inter-AC roaming table structure.
*  2009-12-21 <guoxb> Add TBL_MASK for WTP access AC via NAT Server.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*
******************************************************************************/

#ifndef _WSM_TYPES_H
#define _WSM_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include<netinet/in.h> 

#include "wcpss/waw.h"


#define __packed __attribute__((packed))
#define WSM_MEM_MAX_OFFSET			58
/* Wifi bind Max Number */
#define WIFI_BIND_MAX_NUM				256
#define WSM_STA_FDB_MAX_NUM			65536

/* LLC Data */
#define WSM_LLC_DSAP 					0xAA
#define WSM_LLC_SSAP 					0xAA
#define WSM_LLC_CMD  					0x03
#define WSM_LLC_ORG_CODE_8021H 		0xF8
#define WSM_LLC_ORG_CODE_RFC1042		0x0
#define WSM_LLC_ETHER_IP 				0x0800
#define WSM_LLC_ETHER_ARP 				0x0806

/* size of 802.11 address */
#define	IEEE80211_ADDR_LEN			6
#define	IEEE80211_IS_MULTICAST(_a)		(*(_a) & 0x01)

/* 802.3 Mac Header Parse */
#define IEEE8023_DEST_MAC_START		0
#define IEEE8023_SRC_MAC_START			6
#define IEEE8023_MAC_LEN				6
#define IEEE8023_TYPE_START				12
#define IEEE8023_TYPE_LEN				2

/* 802.11 Mac Header Parse */
#define	IEEE80211_FC0_VERSION_MASK		0x03
#define	IEEE80211_FC0_VERSION_SHIFT		0
#define	IEEE80211_FC0_VERSION_0			0x00
#define	IEEE80211_FC0_TYPE_MASK			0x0c
#define	IEEE80211_FC0_TYPE_SHIFT			2
#define	IEEE80211_FC0_TYPE_MGT			0x00
#define	IEEE80211_FC0_TYPE_CTL			0x04
#define	IEEE80211_FC0_TYPE_DATA			0x08
#define	IEEE80211_FC0_SUBTYPE_MASK		0xf0
#define	IEEE80211_FC0_SUBTYPE_SHIFT		4

/* for TYPE_DATA (bit combination) */
#define	IEEE80211_FC0_SUBTYPE_DATA				0x00
#define	IEEE80211_FC0_SUBTYPE_CF_ACK				0x10
#define	IEEE80211_FC0_SUBTYPE_CF_POLL			0x20
#define	IEEE80211_FC0_SUBTYPE_CF_ACPL			0x30
#define	IEEE80211_FC0_SUBTYPE_NODATA			0x40
#define	IEEE80211_FC0_SUBTYPE_CFACK				0x50
#define	IEEE80211_FC0_SUBTYPE_CFPOLL				0x60
#define	IEEE80211_FC0_SUBTYPE_CF_ACK_CF_ACK	0x70
#define	IEEE80211_FC0_SUBTYPE_QOS				0x80
#define	IEEE80211_FC0_SUBTYPE_QOS_NULL			0xc0

#define	IEEE80211_FC1_DIR_MASK		0x03
#define	IEEE80211_FC1_DIR_NODS		0x00  /* STA->STA */
#define	IEEE80211_FC1_DIR_TODS		0x01  /* STA->AP  */
#define	IEEE80211_FC1_DIR_FROMDS		0x02  /* AP ->STA */
#define	IEEE80211_FC1_DIR_DSTODS		0x03  /* AP ->AP  */

#define	IEEE80211_FC1_MORE_FRAG		0x04
#define	IEEE80211_FC1_RETRY			0x08
#define	IEEE80211_FC1_PWR_MGT		0x10
#define	IEEE80211_FC1_MORE_DATA		0x20
#define	IEEE80211_FC1_WEP				0x40
#define	IEEE80211_FC1_ORDER			0x80


/* Length and offset of Fields of 802.11 Mac Header */
#define IEEE80211_FC_START			0x0
#define IEEE80211_FC_LEN			0x02
#define IEEE80211_DUR_START 		0x02
#define IEEE80211_DUR_LEN 			0x02
#define IEEE80211_ADDR1_START 		0x04
#define IEEE80211_ADDR2_START		0x0a
#define IEEE80211_ADDR3_START   	0x10
#define IEEE80211_SEQ_START		0x16
#define IEEE80211_SEQ_LEN			0x02
#define IEEE80211_ADDR4_START		0x18
#define IEEE80211_HAS_ADDR4_QOS 	0x1e
#define IEEE80211_NO_ADDR4_QOS  	0x18
#define IEEE80211_QOS_LEN			0x02
#define IEEE80211_LLC_LEN			0x08

/* Sequence Number field */
/* 0xab 0xcd Frag = 0xb Seq = 0xcda*/
#define IEEE80211_SEQ_MASK				0x0f
#define	IEEE80211_NWID_LEN			32
#define	IEEE80211_QOS_TXOP			0x00ff

/* bit 8 is reserved */
#define	IEEE80211_QOS_ACKPOLICY		0x60
#define	IEEE80211_QOS_ACKPOLICY_S	5
#define	IEEE80211_QOS_EOSP			0x10
#define	IEEE80211_QOS_EOSP_S			4
#define	IEEE80211_QOS_TID				0x0f

/* WSM: rwlock/mutex macro */
#define __RWLOCK_TEST

#ifdef __RWLOCK_TEST
	//Read/Write Lock
	extern pthread_rwlock_t wsm_tables_rwlock;
	//Read/Write Lock
	#define WSM_RDLOCK_LOCK(a)		pthread_rwlock_rdlock((a))	
	#define WSM_WRLOCK_LOCK(a)		pthread_rwlock_wrlock((a))
	#define WSM_RWLOCK_UNLOCK(a)		pthread_rwlock_unlock((a))
#else
	extern pthread_mutex_t wsm_tables_rwlock;
	#define WSM_RDLOCK_LOCK(a)		pthread_mutex_lock((a))	
	#define WSM_WRLOCK_LOCK(a)		pthread_mutex_lock((a))
	#define WSM_RWLOCK_UNLOCK(a)		pthread_mutex_unlock((a))
#endif


/* Moved from wsm_main_tbl.h */
#define HASHTABLE_SIZE			4096/*2048 *//*wuwl change it from 1024 to 2048,2011-08-29*//*wuwl change it from 2048 to 4096,2012-02-06*/
#define WTPIP_WTPID_TYPE		7
#define BSSID_BSSIndex_TYPE		8
#define WTP_BSS_STA_TYPE		9

#define BSS_ARRAY_SIZE			128
#define BSS_PER_RADIO			32

#define WTP_BSS_PRINT_TYPE		10
#define WTP_STA_PRINT_TYPE		11

#define ROAMING_ENABLE			1
#define ROAMING_DISABLE		0

/* MAX VRID Number */
#define MAX_VRID				17
#define WSM_PATH_MAX			64

/* WTPIP_WTPID table mask */
#define TBL_MASK	(HASHTABLE_SIZE - 1)

typedef enum __ETHER_TYPE {
	ETHER_IP = 0,   		/* 0x0800 */
	ETHER_ARP,			/* 0x0806 */
	ETHER_IPV6,			/* 0x86dd */
	ETHER_PPP,			/* 0x880b */
	ETHER_MPLS_U,		/* 0x8847 */
	ETHER_MPLS_M,		/* 0x8848 */
	ETHER_PPPOE_DIS,	/* 0x8863 */
	ETHER_PPPOE_SESS,	/* 0x8864 */
	ETHER_EAPOL,		/* 0x888e */
	ETHER_LWAPP,		/* 0x88bb */
	ETHER_MAX 			/* 0xffff */
} ETHER_TYPE_T;

/*
* Move from wsm_main.c Used by sending message to ASD.
*
*/
typedef struct
{
	DataType Type;
	unsigned int WTPID;
	unsigned int Radio_G_ID;
	unsigned int BSSIndex;
	unsigned int DataLen;
} DataMsgHead;


enum {
	L_EMERG,		/* system is unusable */
	L_ALERT,		/* action must be taken immediately */
	L_CRIT,			/* critical conditions */
	L_ERR,			/* error conditions */
	L_WARNING,		/* warning conditions */
	L_NOTICE,		/* normal but significant condition */
	L_INFO,			/* informational */
	L_DEBUG,		/* debug-level messages */
	LOG_TYPE_MAX
};

/* wsm syslog function state */
enum {
	LOG_OFF,
	LOG_ON,
	LOG_STATE_MAX
};

typedef enum {
	ROAM_UNICAST = 0,
	ROAM_MULTICAST = 1
} roam_pkt_t;


/* STA HashTable Element */
typedef struct STA_element {
	aWSM_STA STA;
	struct STA_element *next;
} STA_Element;

typedef struct wAW_BSS_R{	
	unsigned int	BSSIndex;
	unsigned char	Radio_L_ID;
	unsigned int	Radio_G_ID;
	unsigned char	WlanID;
	unsigned char	BSSID[MAC_LEN];
	unsigned int    bss_max_sta_num;
	unsigned int	nas_id_len;
	unsigned int	protect_type;
	wAW_IF_Type	bss_ifaces_type;
	wAW_IF_Type	wlan_ifaces_type;
}wAW_BSS_r;	/* WID update BSS information to ASD and WSM, revised in WSM */

/* BSS HashTable Element */
typedef struct BSS_element {
	char flag;
	wAW_BSS_r BSS;
	STA_Element *next;
} BSS_Element;

/* WTP HashTable Element */
typedef struct WTP_element {
	char flag;   /* To mask if this space has been used in array. */
	unsigned char count_BSS;   /* To record how many valid BSS in this WTP. */
	unsigned short WTPportNum;
	unsigned int sta_cnt;
	unsigned short addr_family;
	int ac_fd; 
	struct sockaddr_storage WTPSock;
	wWSM_DataChannel WTP;
	BSS_Element *next;
} WTP_Element;


/* WTPIP_WTPID Table Element */
typedef struct WTPip_WTPid_element {
	unsigned int WTPID;
	struct mixwtpip wtp_ip;

	struct WTPip_WTPid_element *next;
} WTPIP_WTPID_Element;

/* BSSIndex_BSSID_table Element */
typedef struct BSSID_BSSIndex_element {
	unsigned int BSSIndex;
	unsigned char BSSID[MAC_LEN];
	struct BSSID_BSSIndex_element *next;
} BSSID_BSSIndex_Element;

typedef struct if_list {
	int fd;
	struct mixwtpip ip;
	struct if_list *next;
} if_list_t;

/* Inter-AC Roaming table, this table only exist in Raw-AC */
typedef struct roam_sta_info {
	unsigned char stamac[MAC_LEN];
	struct mixwtpip ac_ip;
	struct roam_sta_info *next;
} roam_sta_t;

typedef struct roam_tbl_info {
	unsigned char bssid[MAC_LEN];
	roam_sta_t *sta;
	struct roam_tbl_info *next;
} roam_tbl_t;

/* For table add function */
typedef struct roam_sta_element_t {
	unsigned char bssid[MAC_LEN];
	unsigned char stamac[MAC_LEN];
	struct mixwtpip ac_ip;
} roam_sta_ele_t;

typedef struct {
	roam_pkt_t type;
	unsigned char stamac[MAC_LEN];
} __attribute__((aligned(16))) roam_head_t;

#endif

