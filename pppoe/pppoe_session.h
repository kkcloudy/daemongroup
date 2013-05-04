#ifndef _PPPOE_SESSION_H
#define _PPPOE_SESSION_H

enum {
	SESSION_NONE,
	SESSION_INIT,
	SESSION_LCPPREOPEN,
	SESSION_LCPPOSTOPEN,
	SESSION_LCPOPEN,
	SESSION_AUTHPRE,
	SESSION_AUTH,
	SESSION_AUTHPOST,
	SESSION_AUTHSUCCESS,
	SESSION_IPCPPREOPEN,
	SESSION_IPCPPOSTOPEN,
	SESSION_IPCPOPEN,
	SESSION_ONLINEPRE,
	SESSION_ONLINEPOST,
	SESSION_ONLINE,
	SESSION_OFFLINEPRE,
	SESSION_OFFLINEPOST,
	SESSION_OFFLINE,
	SESSION_IPCPPRECLOSE,
	SESSION_IPCPPOSTCLOSE,
	SESSION_IPCPCLOSE,
	SESSION_LCPPRECLOSE,
	SESSION_LCPPOSTCLOSE,
	SESSION_LCPCLOSE,
	SESSION_DISTERM,
	SESSION_DEAD,
};

enum {
	SESSEXIT_OK,
	SESSEXIT_FATAL_ERROR,
	SESSEXIT_OPTION_ERROR,
	SESSEXIT_USER_REQUEST,
	SESSEXIT_PEER_DEAD,
	SESSEXIT_CONNECT_FAILED,
	SESSEXIT_TRAFFIC_LIMIT,
	SESSEXIT_AUTH_FAILED,
	SESSEXIT_AUTH_TOPEER_FAILED,
	SESSEXIT_IDLE_TIMEOUT,
};

typedef enum {
	SESSION_TIMEOUT_TIMER,	/*this timer can not over 10 second */
	SESSION_ECHO_TIMER,
	SESSION_RETRANS_TIMER,	/*this timer can not over 10 second */
	SESSION_UPDATE_TIMER,
} sessionTimerType;

struct session_options {
	uint8 passive;				/* Don't die if we don't get a response */
	uint8 silent;				/* Wait for the other end to start first */
	uint8 restart;				/* Restart vs. exit after close */
	uint8 neg_mru;				/* Negotiate the MRU? */
	uint8 neg_asyncmap;			/* Negotiate the async map? */
	uint8 neg_upap;				/* Ask for UPAP authentication? */
	uint8 neg_chap;				/* Ask for CHAP authentication? */
	uint8 neg_eap;				/* Ask for EAP authentication? */
	uint8 neg_magicnumber;		/* Ask for magic number? */
	uint8 neg_pcompression;		/* HDLC Protocol Field Compression? */
	uint8 neg_accompression;	/* HDLC Address/Control Field Compression? */
	uint8 neg_lqr;				/* Negotiate use of Link Quality Reports */
	uint8 neg_cbcp;				/* Negotiate use of CBCP */
	uint8 neg_mrru;				/* negotiate multilink MRRU */
	uint8 neg_ssnhf;			/* negotiate short sequence numbers */
	uint8 neg_endpoint;			/* negotiate endpoint discriminator */

	uint8 neg_ipcompression;	/* negotiate IPCP compression */
	uint8 neg_ipaddr;			/* negotiate IPCP addr */
	uint8 neg_ipmsdns;			/* negotiate IPCP DNS */
	uint8 neg_ipmswins;			/* negotiate IPCP WINS */
	
	uint32 mru;					/* Value of MRU */
	uint32 mrru;				/* Value of MRRU, and multilink enable */
	uint32 chap_mdtype;			/* which MD types (hashing algorithm) */
	uint32 asyncmap;			/* Value of async map */
	uint32 numloops;			/* Number of loops during magic number neg. */
	uint32 lqr_period;			/* Reporting period for LQR 1/100ths second */
};

struct session_stat {
	unsigned long long	rx_packets;		/* total packets received	*/
	unsigned long long	tx_packets;		/* total packets transmitted	*/

	unsigned long long	rx_bytes;		/* total bytes received 		*/
	unsigned long long	tx_bytes;		/* total bytes transmitted	*/
};

struct session_struct {
	uint32 state;
	uint32 refcnt;
	uint32 configState;
	uint32 terminate_cause;

	uint32 sid;
	uint8 mac[ETH_ALEN];
	uint8 serverMac[ETH_ALEN];
	uint32 serverIP;
	struct ipaddr_struct ipaddr;
	struct hlist_node 	hash_mac;
	struct list_head	next;

	/* wireless info */
	uint32 wtp_id;
	uint32 radio_g_id;
	uint32 bssindex;
	uint32 vlan_id;
	uint8 wtpmac[ETH_ALEN];
	uint8 wlan_id;
	uint8 security_id;
	uint8 radio_l_id;	

	long startTime;
	long onlineTime;
	long offlineTime;	

	uint32 ident;
	uint32 echoLoseTimes;

	uint32 termTimes;
	uint32 termMaxTimes;

	uint32 retryTimes;
	uint32 retryMaxTimes;
	
	uint32 auth_type;
	uint32 chap_mdtype;

	void *priv_data;

	uint32 seed;

	struct pppoe_buf *pbuf;
	struct session_options *options;
	
	session_opt_t 	*opt;
	
	thread_struct_t	*echo;
	thread_struct_t	*timeout;
	thread_struct_t	*retrans;
	thread_struct_t	*update;
	thread_master_t	*master;

	char sname[PPPOE_NAMELEN];	
	uint8 magic[MAGIC_LEN];
	uint8 peermagic[MAGIC_LEN];
	
	uint8 challenge[RADIUS_CHAP_CHAL_LEN];
	uint8 passwd[RADIUS_PASSWORD_LEN];
	char username[USERNAMESIZE];
	char acctSessID[ACCT_SESSIONIDSIZE];

	struct session_stat stat;
	struct session_stat bk_stat;
};

static inline void
session_hold(session_struct_t *sess) {
	sess->refcnt++;
}

static inline void
session_put(session_struct_t *sess) {
	sess->refcnt--;
}

/* this func is not thread safe, only use by device thread */
session_struct_t *__session_get_by_mac(pppoe_sessions_t *sessions, unsigned char *mac);
session_struct_t *__session_get_by_sid(pppoe_sessions_t *sessions, unsigned int sid);
struct list_head *__session_get_list(pppoe_sessions_t *sessions);

/* this func is thread safe */
session_struct_t *session_get_by_mac(pppoe_sessions_t *sessions, unsigned char *mac);
session_struct_t *session_get_by_sid(pppoe_sessions_t *sessions, unsigned int sid);

void session_termcause_setup(session_struct_t *sess, uint32 termcause);

int session_timer_init(session_struct_t *sess, sessionTimerType type,
						ThreadRunFunc func, long timer);
int session_timer_update(session_struct_t *sess, sessionTimerType type, long timer);
int session_timer_pause(session_struct_t *sess, sessionTimerType type);
void session_timer_destroy(session_struct_t *sess, sessionTimerType type);
void session_timer_clear(session_struct_t *sess);

int session_opt_perform(session_struct_t *sess, sessionOptType optType);
int session_opt_register(session_struct_t *sess, sessionOptType optType, 
							void *arg, sessOptFunc func);
int session_opt_unregister(session_struct_t *sess, sessionOptType optType);

void session_register(pppoe_sessions_t *sessions, session_struct_t *sess);
void session_unregister(pppoe_sessions_t *sessions, session_struct_t *sess);
session_struct_t *session_alloc(pppoe_sessions_t *sessions, uint32 sid, unsigned char *mac);
void session_free(pppoe_sessions_t *sessions, session_struct_t *sess);

int pppoe_sessions_opt_register(pppoe_sessions_t *sessions, sessionOptType optType, 
										void *arg, sessOptFunc func);
int pppoe_sessions_opt_unregister(pppoe_sessions_t *sessions, sessionOptType optType);

int pppoe_sessions_show_online(pppoe_sessions_t *sessions, 
								struct pppoeUserInfo **userarray, uint32 *userNum);

pppoe_sessions_t *pppoe_sessions_init(uint32 max_sid);
void pppoe_sessions_destroy(pppoe_sessions_t **sessions);

#endif 
