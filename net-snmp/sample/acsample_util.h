#ifndef _ACSAMPLE_UTIL_H_
#define _ACSAMPLE_UTIL_H_

/*
********************************************************
*	return code declare
********************************************************
*/
#define EAG_RETURN_CODE_BASE				(0x000000)							/* return code base				*/
#define EAG_RETURN_CODE_OK				(EAG_RETURN_CODE_BASE + 0x0)		/* success						*/
#define EAG_RETURN_CODE_ERROR				(EAG_RETURN_CODE_BASE + 0x1)		/* error 							*/
#define EAG_RETURN_CODE_ALREADY_SET		(EAG_RETURN_CODE_BASE + 0x2)		/* already been seted				*/
#define EAG_RETURN_CODE_ENABLE_GBL		(EAG_RETURN_CODE_BASE + 0x3)		/* EAG enabled global				*/
#define EAG_RETURN_CODE_NOT_ENABLE_GBL	(EAG_RETURN_CODE_BASE + 0x4)		/* EAG not enabled global			*/
#define EAG_RETURN_CODE_OUT_RANGE		(EAG_RETURN_CODE_BASE + 0x5)		/* timer value out of range			*/
#define EAG_RETURN_CODE_SAME_VALUE		(EAG_RETURN_CODE_BASE + 0x6)		/* set same value to EAG timers		*/
#define EAG_RETURN_CODE_GET_DETECT_E		(EAG_RETURN_CODE_BASE + 0x7)		/* get EAG detect timers error		*/
#define EAG_RETURN_CODE_GET_REDETECT_E	(EAG_RETURN_CODE_BASE + 0x8)		/* get EAG re-detect timers error	*/
#define EAG_RETURN_CODE_VLAN_NOT_EXIST	(EAG_RETURN_CODE_BASE + 0x9)		/* L2 vlan not exixt				*/
#define EAG_RETURN_CODE_NOTENABLE_VLAN	(EAG_RETURN_CODE_BASE + 0xA)		/* L2 vlan not enable EAG			*/
#define EAG_RETURN_CODE_HASENABLE_VLAN	(EAG_RETURN_CODE_BASE + 0xB)		/* L2 vlan has enable EAG			*/
#define EAG_RETURN_CODE_NULL_PTR			(EAG_RETURN_CODE_BASE + 0xC)		/* parameter pointer is null			*/
#define EAG_RETURN_CODE_HASH_TABLE_FULL (EAG_RETURN_CODE_BASE + 0xD)			/* hash table has full				*/
#define EAG_RETURN_CODE_HASH_DUPLICATED (EAG_RETURN_CODE_BASE + 0xE)			/* hash table has duplicated		*/
#define EAG_RETURN_CODE_HASH_NORESOURCE (EAG_RETURN_CODE_BASE + 0xF)			/* hash item alloc memory null		*/
#define EAG_RETURN_CODE_HASH_NOTEXISTS	(EAG_RETURN_CODE_BASE + 0x10)		/* hash item not exist				*/
#define EAG_RETURN_CODE_ALLOC_MEM_NULL  (EAG_RETURN_CODE_BASE + 0x11)		/* alloc memory null				*/
#define EAG_RETURN_CODE_HASH_FOUND		(EAG_RETURN_CODE_BASE + 0x12)		/* found hash item				*/
#define EAG_RETURN_CODE_HASH_NOTFOUND	(EAG_RETURN_CODE_BASE + 0x13)		/* not found hash iteml				*/
#define EAG_RETURN_CODE_FILE_NOT_FOUND  (EAG_RETURN_CODE_BASE + 0x14)		/* file not found when open 		*/
#define EAG_RETURN_CODE_SOCKET_CREATE_FAIL	(EAG_RETURN_CODE_BASE + 0x15)	/* socket create failed 				*/	
#define EAG_RETURN_CODE_SOCKET_OP_FAIL	(EAG_RETURN_CODE_BASE + 0x16)		/* socket operation(setopt/bind) failed */	
#define EAG_RETURN_CODE_FILE_OP_FAIL		(EAG_RETURN_CODE_BASE + 0x17) 		/* file operation(read/write) failed 	*/
#define EAG_RETURN_CODE_OUT_OF_MEMORY	(EAG_RETURN_CODE_BASE + 0x18) 		/* memory space is not enough 		*/
#define EAG_RETURN_CODE_NOT_FOUND		(EAG_RETURN_CODE_BASE + 0x19)		/* not found 						*/
#define EAG_RETURN_CODE_NOT_TIME_YET		(EAG_RETURN_CODE_BASE + 0x1A)		/* not time yet 					*/
#define EAG_RETURN_CODE_TOO_MANY_REQ		(EAG_RETURN_CODE_BASE + 0x1B)		/* too many request				*/
#define EAG_RETURN_CODE_PARAM_ERROR		(EAG_RETURN_CODE_BASE + 0x1C)		/* param error					*/


/********************************************************************************
* radius.h
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/

#ifndef _RADIUS_H
#define _RADIUS_H

#define RADIUS_NONETWORK   0x01
#define RADIUS_NOBROADCAST 0x02

#define RADIUS_AUTHPORT 1812
#define RADIUS_ACCTPORT 1813

/* Radius packet types */
#define RADIUS_CODE_ACCESS_REQUEST            1
#define RADIUS_CODE_ACCESS_ACCEPT             2
#define RADIUS_CODE_ACCESS_REJECT             3
#define RADIUS_CODE_ACCOUNTING_REQUEST        4
#define RADIUS_CODE_ACCOUNTING_RESPONSE       5
#define RADIUS_CODE_ACCESS_CHALLENGE         11
#define RADIUS_CODE_STATUS_SERVER            12
#define RADIUS_CODE_STATUS_CLIENT            13
#define RADIUS_CODE_DISCONNECT_REQUEST       40
#define RADIUS_CODE_DISCONNECT_ACK           41
#define RADIUS_CODE_DISCONNECT_NAK           42
#define RADIUS_CODE_COA_REQUEST              43
#define RADIUS_CODE_COA_ACK                  44
#define RADIUS_CODE_COA_NAK                  45
#define RADIUS_CODE_STATUS_REQUEST           46
#define RADIUS_CODE_STATUS_ACCEPT            47
#define RADIUS_CODE_STATUS_REJECT            48

/* Radius attributes */
#define RADIUS_ATTR_USER_NAME                 1     /* string */
#define RADIUS_ATTR_USER_PASSWORD             2     /* string (encrypt) */
#define RADIUS_ATTR_CHAP_PASSWORD             3     /* octets */
#define RADIUS_ATTR_NAS_IP_ADDRESS            4     /* ipaddr */
#define RADIUS_ATTR_NAS_PORT                  5     /* integer */
#define RADIUS_ATTR_SERVICE_TYPE              6     /* integer */
#define RADIUS_ATTR_FRAMED_PROTOCOL           7     /* integer */
#define RADIUS_ATTR_FRAMED_IP_ADDRESS         8     /* ipaddr */
#define RADIUS_ATTR_FRAMED_IP_NETMASK         9     /* ipaddr */
#define RADIUS_ATTR_FRAMED_ROUTING           10     /* integer */
#define RADIUS_ATTR_FILTER_ID                11     /* string */
#define RADIUS_ATTR_FRAMED_MTU               12     /* integer */
#define RADIUS_ATTR_FRAMED_COMPRESSION       13     /* integer */
#define RADIUS_ATTR_LOGIN_IP_HOST            14     /* ipaddr */
#define RADIUS_ATTR_LOGIN_SERVICE            15     /* integer */
#define RADIUS_ATTR_LOGIN_TCP_PORT           16     /* integer */
#define RADIUS_ATTR_REPLY_MESSAGE            18     /* string */
#define RADIUS_ATTR_CALLBACK_NUMBER          19     /* string */
#define RADIUS_ATTR_CALLBACK_ID              20     /* string */
#define RADIUS_ATTR_FRAMED_ROUTE             22     /* string */
#define RADIUS_ATTR_FRAMED_IPX_NETWORK       23     /* ipaddr */
#define RADIUS_ATTR_STATE                    24     /* octets */
#define RADIUS_ATTR_CLASS                    25     /* octets */
#define RADIUS_ATTR_VENDOR_SPECIFIC          26     /* octets */
#define RADIUS_ATTR_SESSION_TIMEOUT          27     /* integer */
#define RADIUS_ATTR_IDLE_TIMEOUT             28     /* integer */
#define RADIUS_ATTR_TERMINATION_ACTION       29     /* integer */
#define RADIUS_ATTR_CALLED_STATION_ID        30     /* string */
#define RADIUS_ATTR_CALLING_STATION_ID       31     /* string */
#define RADIUS_ATTR_NAS_IDENTIFIER           32     /* string */
#define RADIUS_ATTR_PROXY_STATE              33     /* octets */
#define RADIUS_ATTR_LOGIN_LAT_SERVICE        34     /* string */
#define RADIUS_ATTR_LOGIN_LAT_NODE           35     /* string */
#define RADIUS_ATTR_LOGIN_LAT_GROUP          36     /* octets */
#define RADIUS_ATTR_FRAMED_APPLETALK_LINK    37     /* integer */
#define RADIUS_ATTR_FRAMED_APPLETALK_NETWORK 38     /* integer */
#define RADIUS_ATTR_FRAMED_APPLETALK_ZONE    39     /* string */
#define RADIUS_ATTR_ACCT_STATUS_TYPE         40     /* integer */
#define RADIUS_ATTR_ACCT_DELAY_TIME          41     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_OCTETS        42     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_OCTETS       43     /* integer */
#define RADIUS_ATTR_ACCT_SESSION_ID          44     /* string */
#define RADIUS_ATTR_ACCT_AUTHENTIC           45     /* integer */
#define RADIUS_ATTR_ACCT_SESSION_TIME        46     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_PACKETS       47     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_PACKETS      48     /* integer */
#define RADIUS_ATTR_ACCT_TERMINATE_CAUSE     49     /* integer */
#define RADIUS_ATTR_ACCT_MULTI_SESSION_ID    50     /* string */
#define RADIUS_ATTR_ACCT_LINK_COUNT          51     /* integer */
#define RADIUS_ATTR_ACCT_INPUT_GIGAWORDS     52     /* integer */
#define RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS    53     /* integer */
#define RADIUS_ATTR_EVENT_TIMESTAMP          55     /* date */
#define RADIUS_ATTR_CHAP_CHALLENGE           60     /* string */
#define RADIUS_ATTR_NAS_PORT_TYPE            61     /* integer */
#define RADIUS_ATTR_PORT_LIMIT               62     /* integer */
#define RADIUS_ATTR_LOGIN_LAT_PORT           63     /* integer */
#define RADIUS_ATTR_ACCT_TUNNEL_CONNECTION   68     /* string */
#define RADIUS_ATTR_ARAP_PASSWORD            70     /* string */
#define RADIUS_ATTR_ARAP_FEATURES            71     /* string */
#define RADIUS_ATTR_ARAP_ZONE_ACCESS         72     /* integer */
#define RADIUS_ATTR_ARAP_SECURITY            73     /* integer */
#define RADIUS_ATTR_ARAP_SECURITY_DATA       74     /* string */
#define RADIUS_ATTR_PASSWORD_RETRY           75     /* integer */
#define RADIUS_ATTR_PROMPT                   76     /* integer */
#define RADIUS_ATTR_CONNECT_INFO             77     /* string */
#define RADIUS_ATTR_CONFIGURATION_TOKEN      78     /* string */
#define RADIUS_ATTR_EAP_MESSAGE              79     /* string */
#define RADIUS_ATTR_MESSAGE_AUTHENTICATOR    80     /* octets */
#define RADIUS_ATTR_ARAP_CHALLENGE_RESPONSE  84     /* string # 10 octets */
#define RADIUS_ATTR_ACCT_INTERIM_INTERVAL    85     /* integer */
#define RADIUS_ATTR_NAS_PORT_ID              87     /* string */
#define RADIUS_ATTR_FRAMED_POOL              88     /* string */
#define RADIUS_ATTR_NAS_IPV6_ADDRESS         95     /* octets (IPv6) */
#define RADIUS_ATTR_FRAMED_INTERFACE_ID      96     /* octets # 8 octets */
#define RADIUS_ATTR_FRAMED_IPV6_PREFIX       97     /* octets ??? */
#define RADIUS_ATTR_LOGIN_IPV6_HOST          98     /* octets (IPv6) */
#define RADIUS_ATTR_FRAMED_IPV6_ROUTE        99     /* string */
#define RADIUS_ATTR_FRAMED_IPV6_POOL        100     /* string */
#define RADIUS_ATTR_DIGEST_RESPONSE         206     /* string */
#define RADIUS_ATTR_DIGEST_ATTRIBUTES       207     /* octets  ??? */


#define RADIUS_VENDOR_MS                    311
#define RADIUS_ATTR_MS_CHAP_RESPONSE          1
#define RADIUS_ATTR_MS_MPPE_ENCRYPTION_POLICY 7
#define RADIUS_ATTR_MS_MPPE_ENCRYPTION_TYPES  8
#define RADIUS_ATTR_MS_CHAP_CHALLENGE        11
#define RADIUS_ATTR_MS_CHAP_MPPE_KEYS        12
#define RADIUS_ATTR_MS_MPPE_SEND_KEY         16
#define RADIUS_ATTR_MS_MPPE_RECV_KEY         17
#define RADIUS_ATTR_MS_CHAP2_RESPONSE        25
#define RADIUS_ATTR_MS_CHAP2_SUCCESS         26

#define RADIUS_VENDOR_SJW		31656
#define RADIUS_ATTR_SJW_ENTERPRISE	1/*string*/


#define RADIUS_SERVICE_TYPE_LOGIN             1
/*add for test*/
#define RADIUS_SERVICE_TYPE_Framed_User_LOGIN	2
/*end*/
#define RADIUS_SERVICE_TYPE_ADMIN_USER        6

#define RADIUS_STATUS_TYPE_START              1
#define RADIUS_STATUS_TYPE_STOP               2
#define RADIUS_STATUS_TYPE_INTERIM_UPDATE     3
#define RADIUS_STATUS_TYPE_ACCOUNTING_ON      7
#define RADIUS_STATUS_TYPE_ACCOUNTING_OFF     8

#define RADIUS_NAS_PORT_TYPE_VIRTUAL          5
#define RADIUS_NAS_PORT_TYPE_WIRELESS_802_11 19
#define RADIUS_NAS_PORT_TYPE_WIRELESS_UMTS   23

#define RADIUS_TERMINATE_CAUSE_USER_REQUEST          1
#define RADIUS_TERMINATE_CAUSE_LOST_CARRIER          2
#define RADIUS_TERMINATE_CAUSE_LOST_SERVICE          3
#define RADIUS_TERMINATE_CAUSE_IDLE_TIMEOUT          4
#define RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT       5
#define RADIUS_TERMINATE_CAUSE_ADMIN_RESET           6
#define RADIUS_TERMINATE_CAUSE_ADMIN_REBOOT          7
#define RADIUS_TERMINATE_CAUSE_PORT_ERROR            8
#define RADIUS_TERMINATE_CAUSE_NAS_ERROR             9
#define RADIUS_TERMINATE_CAUSE_NAS_REQUEST          10
#define RADIUS_TERMINATE_CAUSE_NAS_REBOOT           11
#define RADIUS_TERMINATE_CAUSE_PORT_UNNEEDED        12
#define RADIUS_TERMINATE_CAUSE_PORT_PREEMPTED       13
#define RADIUS_TERMINATE_CAUSE_PORT_SUSPEND         14
#define RADIUS_TERMINATE_CAUSE_SERVICE_UNAVAILABLE  15
#define RADIUS_TERMINATE_CAUSE_CALLBACK             16
#define RADIUS_TERMINATE_CAUSE_USER_ERROR           17
#define RADIUS_TERMINATE_CAUSE_HOST_REQUEST         18
#define RADIUS_TERMINATE_CAUSE_ADMIN_NTFLOGOUT	    19/*add for china mobile*/

//#include "limits2.h"
/********************************************************************************
* limits2.h
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/
#ifndef _LIMITS_H
#define _LIMITS_H

/*
 * extracted from various .h files, needs some cleanup.
 */

#define LEAKY_BUCKET 0
/* If the constants below are defined packets which have been dropped
   by the traffic shaper will be counted towards accounting and
   volume limitation */
/* #define COUNT_DOWNLINK_DROP 1 */
/* #define COUNT_UPLINK_DROP 1 */

/*#define BUCKET_SIZE                   300000 -* Size of leaky bucket (~200 packets) */
/* Time length of leaky bucket in milliseconds */
/* Bucket size = BUCKET_TIME * Bandwidth-Max radius attribute */
/* Not used if BUCKET_SIZE is defined */
#define BUCKET_TIME                     5000  /* 5 seconds */
#define BUCKET_SIZE_MIN                15000 /* Minimum size of leaky bucket (~10 packets) */
#define CHECK_INTERVAL                     1 /* Time between checking connections */

/* options */
#define OPT_IPADDRLEN                    256
#define OPT_IDLETIME                      10 /* Options idletime between each select */
#define MAX_PASS_THROUGHS                128 /* Max number of allowed UAM pass-throughs */
#define UAMSERVER_MAX                      8
#define MACOK_MAX                         16

/* redir */
#define REDIR_MAXLISTEN                   32
#define REDIR_MAXTIME                    100 /* Seconds */
#define REDIR_HTTP_MAX_TIME               3/*10*/ /* Seconds *//*modify to 3 second 20100524 . old is 10*/
#define REDIR_HTTP_SELECT_TIME        500000 /*  microseconds = 0.5 seconds   change to 2.5s by shaojunwu 20090904*/
#define REDIR_RADIUS_MAX_TIME             3 /* Seconds */
#define REDIR_RADIUS_SELECT_TIME      500000 /* microseconds = 0.5 seconds */
#define REDIR_CHALLEN                     16
#define REDIR_MD5LEN                      16
#define REDIR_MACSTRLEN                   17
#define REDIR_MAXCHAR                     64 /* 1024 */
#define REDIR_MAXBUFFER                 5125

#define REDIR_USERNAMESIZE               256 /* Max length of username */
#define REDIR_MAXQUERYSTRING            2048
#define REDIR_USERURLSIZE               2048 /* Max length of URL requested by user */
#define REDIR_USERAGENTSIZE              256
#define REDIR_ADVERTISING_URL_SIZE		256	/*advertisingURL size*/
#define REDIR_LANGSIZE                    16
#define REDIR_IDENTSIZE                   16

#define REDIR_MAXCONN                     16

#define REDIR_CHALLENGETIMEOUT1          300 /* Seconds */
#define REDIR_CHALLENGETIMEOUT2          600 /* Seconds */

#define REDIR_URL_LEN                   2048
#define REDIR_SESSIONID_LEN               17

#define APP_NUM_CONN                    1024
#define EAP_LEN                         256 /* TODO: Rather large */
#define MESSAGE_LEN						256
#define MACSTRLEN                         17
#define MS2SUCCSIZE                       40 /* MS-CHAPv2 authenticator response as ASCII */
#define DATA_LEN                        1500 /* Max we allow */
#define USERNAMESIZE                     256 /* Max length of username */
#define CHALLENGESIZE                     24 /* From chap.h MAX_CHALLENGE_LENGTH */
#define USERURLSIZE                      256 /* Max length of URL requested by user */

/* dhcp */
#define DHCP_DEBUG                         0 /* Print debug information */
#define DHCP_MTU                        1492 /* Maximum MTU size */

/* radius */
#define RADIUS_SECRETSIZE                128 /* No secrets that long */
#define RADIUS_MD5LEN                     16 /* Length of MD5 hash */
#define RADIUS_AUTHLEN                    16 /* RFC 2865: Length of authenticator */
#define RADIUS_PWSIZE                    128 /* RFC 2865: Max 128 octets in password */
#define RADIUS_QUEUESIZE                 256 /* Same size as id address space */
#define RADIUS_ATTR_VLEN                 253
#define RADIUS_PACKSIZE                 4096
#define RADIUS_HDRSIZE                    20
#define RADIUS_PASSWORD_LEN               16
#define RADIUS_MPPEKEYSSIZE               32 /* Length of MS_CHAP_MPPE_KEYS attribute */ 
#define RADIUS_DEFINTERVAL_TIME			300/*default radius  account update interval time interval*/

#endif


struct radius_packet_t {
  uint8_t code;
  uint8_t id;
  uint16_t length;
  uint8_t authenticator[RADIUS_AUTHLEN];
  uint8_t payload[RADIUS_PACKSIZE-RADIUS_HDRSIZE];
} __attribute__((packed));


struct radius_queue_t {      /* Holder for queued packets */
  int state;                 /* 0=empty, 1=full */
  void *cbp;                 /* Pointer used for callbacks */
  struct timeval timeout;    /* When do we retransmit this packet? */
  int retrans;               /* How many times did we retransmit this? */
  int lastsent;              /* 0 or 1 indicates last server used */
  struct sockaddr_in peer;   /* Address packet was sent to / received from */
  struct radius_packet_t p;  /* The packet stored */
  uint16_t seq;              /* The sequence number */
  uint8_t type;              /* The type of packet */
  size_t l;                  /* Length of the packet */
  struct qmsg_t *seqnext;    /* Pointer to next in sequence hash list */
  int next;                  /* Pointer to the next in queue. -1: Last */
  int prev;                  /* Pointer to the previous in queue. -1: First */
  int this;                  /* Pointer to myself */

/*add for domain radius  shaojunwu 20090731*/
  struct radius_conf_t *radius_conf;

  struct in_addr auth_addr;       /* Authenticate  server address */
  struct in_addr bk_auth_addr;   	/* Backup authenticate server address */
  uint16_t authport;             /* His port for authentication */
  uint16_t bk_authport;             /* His port for authentication */
  struct in_addr acct_addr;		  /* Account server address */
  struct in_addr bk_acct_addr;	  /* Backup account server address */
  uint16_t acctport;			   /* His port for accounting */
  uint16_t bk_acctport;			  /* His port for accounting */

};


struct radius_t {
  int fd;                        /* Socket file descriptor */
  FILE *urandom_fp;              /* /dev/urandom FILE pointer */
  struct in_addr ouraddr;        /* Address to listen to */
  uint16_t ourport;              /* Port to listen to */
  int coanocheck;                /* Accept coa from all IP addresses */
  int lastreply;                 /* 0 or 1 indicates last server reply */
  uint16_t authport;             /* His port for authentication */
  uint16_t acctport;             /* His port for accounting */
  uint16_t bk_authport;             /* His port for authentication */
  uint16_t bk_acctport;             /* His port for accounting */
  //struct in_addr hisaddr0;       /* Server address */
  //struct in_addr hisaddr1;       /* Server address */
  
  struct in_addr auth_addr;       /* Authenticate  server address */
  struct in_addr bk_auth_addr;   	/* Backup authenticate server address */
  struct in_addr acct_addr;       /* Account server address */
  struct in_addr bk_acct_addr;   	/* Backup account server address */
  
  char secret[RADIUS_SECRETSIZE];/* Shared secret */
 
  int authtype;
  int swap_octets;
  /*add for bas conf*/
  int radiustimeout;
  int radiusretry;
  int radiusretrysec;
  /*add for  different radius server process*/
  int  vendor_id;

  size_t secretlen;                 /* Length of sharet secret */
  int proxyfd;                   /* Proxy socket file descriptor */
  struct in_addr proxylisten;    /* Proxy address to listen to */
  uint16_t proxyport;            /* Proxy port to listen to */
  struct in_addr proxyaddr;      /* Proxy client address */
  struct in_addr proxymask;      /* Proxy client mask */
  char proxysecret[RADIUS_SECRETSIZE]; /* Proxy secret */
  size_t proxysecretlen;            /* Length of sharet secret */
  unsigned char nas_hwaddr[6];   /* Hardware address of NAS */

  int debug;                     /* Print debug messages */

  struct radius_queue_t queue[RADIUS_QUEUESIZE]; /* Outstanding replies */
  uint8_t next;                  /* Next location in queue to use */
  int first;                     /* First packet in queue (oldest timeout) */
  int last;                      /* Last packet in queue (youngest timeout) */

  int listsize;                  /* Total number of addresses */
  int hashsize;                  /* Size of hash table */
  int hashlog;                   /* Log2 size of hash table */
  int hashmask;                  /* Bitmask for calculating hash */
  int (*cb_ind)  (struct radius_t *radius, struct radius_packet_t *pack,
		  struct sockaddr_in *peer);
  int (*cb_auth_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		       struct radius_packet_t *pack_req, void *cbp);
  int (*cb_acct_conf) (struct radius_t *radius, struct radius_packet_t *pack,
		       struct radius_packet_t *pack_req, void *cbp);
  int (*cb_coa_ind)   (struct radius_t *radius, struct radius_packet_t *pack,
		       struct sockaddr_in *peer,void * eag_ins);
};

struct radiusm_t {
  struct in_addr addr;           /* IP address of this member */
  int inuse;                     /* 0=available; 1= inuse */
  struct RADIUSm_t *nexthash;    /* Linked list part of hash table */
  struct RADIUSm_t *prev, *next; /* Double linked list of available members */
  struct RADIUS_t *parent;       /* Pointer to parent */
  void *peer;                    /* Pointer to peer protocol handler */
};


struct radius_attr_t {
  uint8_t t;
  uint8_t l;
  union {
    uint32_t i;
    uint8_t  t[RADIUS_ATTR_VLEN];
    struct {
      uint32_t i;
      uint8_t t;
      uint8_t l;
      union {
	uint32_t i;
	uint8_t  t[RADIUS_ATTR_VLEN-4];
      } v;
    } vv;
  } v; 
} __attribute__((packed));

#endif

/*************************************
定义基本的数据类型
************************************/
#define INT8	char
#define UINT8	unsigned char
#define INT16	short
#define UINT16	unsigned short
#define INT32	int
#define UINT32	unsigned int
#define INT64	long long
#define UINT64	unsigned long long


typedef struct MD5Context {
  UINT32 buf[4];
  UINT32 bits[2];
  unsigned char in[64];
}MD5_CTX;



int 
radius_default_pack(struct radius_t *this,
		struct radius_packet_t *pack, 
		int code);

int 
radius_addattr(struct radius_t *this, struct radius_packet_t *pack, 
		uint8_t type, uint32_t vendor_id, uint8_t vendor_type,
		uint32_t value, uint8_t *data, uint16_t dlen);


#endif

