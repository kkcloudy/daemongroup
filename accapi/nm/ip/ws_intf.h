#ifndef _WS_INTF_H_
#define _WS_INTF_H_
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>

/* lixiang modify at 2012-02-15 
 * change ifname size from 10 to 32
 */
#define IFNAME_SIZE 32

#define IS_RADIO 2
#define GD_OK 3

#define NLMSG_NOOP		0x1	/* Nothing.		*/
#define NLMSG_ERROR		0x2	/* Error		*/
#define NLMSG_DONE		0x3	/* End of a dump	*/
#define NLMSG_OVERRUN		0x4	/* Data lost		*/

#define NLMSG_MIN_TYPE		0x10	/* < 0x10: reserved control messages */
#define NETLINK_ROUTE		0	/* Routing/device hook				*/

#define RTM_NEWLINK	16
#define RTM_DELLINK	17
#define RTM_GETLINK	18


/* Flags values */
#define NLM_F_REQUEST		1	/* It is request message. 	*/
#define NLM_F_MULTI		2	/* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK		4	/* Reply with ack, with zero or error code */
#define NLM_F_ECHO		8	/* Echo this request 		*/

/* Modifiers to GET request */
#define NLM_F_ROOT	0x100	/* specify tree	root	*/
#define NLM_F_MATCH	0x200	/* return all matching	*/
#define NLM_F_ATOMIC	0x400	/* atomic GET		*/
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)

/* struct ifinfomsg
 * passes link level specific information, not dependent
 * on network protocol.
 */
struct ifinfomsg
{
	unsigned char	ifi_family;
	unsigned char	__ifi_pad;
	unsigned short	ifi_type;		/* ARPHRD_* */
	int		ifi_index;		/* Link index	*/
	unsigned	ifi_flags;		/* IFF_* flags	*/
	unsigned	ifi_change;		/* IFF_* change mask */
};

struct rtgenmsg
{
	unsigned char		rtgen_family;
};

struct intflib_handle
{
	int			fd;
	struct sockaddr_nl	local;
	struct sockaddr_nl	peer;
	unsigned int	seq;
	unsigned int	dump;
};

struct rtattr
{
	unsigned short	rta_len;
	unsigned short	rta_type;
};


#if 0
struct sockaddr_nl
{
	unsigned short	nl_family;	/* AF_NETLINK	*/
	unsigned short	nl_pad;		/* zero		*/
	unsigned int	nl_pid;		/* process pid	*/
    unsigned int	nl_groups;	/* multicast groups mask */
};




struct nlmsghdr
{
	unsigned int		nlmsg_len;	/* Length of message including header */
	unsigned short		nlmsg_type;	/* Message content */
	unsigned short		nlmsg_flags;	/* Additional flags */
	unsigned int		nlmsg_seq;	/* Sequence number */
	unsigned int		nlmsg_pid;	/* Sending process PID */
};


struct nlmsgerr
{
	int		error;
	struct nlmsghdr msg;
};


struct nlmsg_list
{
	struct nlmsg_list *next;
	struct nlmsghdr	  h;
};


/* 
   Generic structure for encapsulation of optional route information.
   It is reminiscent of sockaddr, but with sa_family replaced
   with attribute type.
 */
#endif


/*netlink normal Macros*/
#define NLMSG_ALIGNTO	4
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_HDRLEN	 ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_LENGTH(len) ((len)+NLMSG_ALIGN(NLMSG_HDRLEN))
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
#define NLMSG_NEXT(nlh,len)	 ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
				  (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))


/* Macros to handle rtattributes */
#define RTA_ALIGNTO	4
#define RTA_ALIGN(len) ( ((len)+RTA_ALIGNTO-1) & ~(RTA_ALIGNTO-1) )
#define RTA_OK(rta,len) ((len) >= (int)sizeof(struct rtattr) && \
			 (rta)->rta_len >= sizeof(struct rtattr) && \
			 (rta)->rta_len <= (len))
#define RTA_NEXT(rta,attrlen)	((attrlen) -= RTA_ALIGN((rta)->rta_len), \
				 (struct rtattr*)(((char*)(rta)) + RTA_ALIGN((rta)->rta_len)))
#define RTA_LENGTH(len)	(RTA_ALIGN(sizeof(struct rtattr)) + (len))
#define RTA_SPACE(len)	RTA_ALIGN(RTA_LENGTH(len))
#define RTA_DATA(rta)   ((void*)(((char*)(rta)) + RTA_LENGTH(0)))
#define RTA_PAYLOAD(rta) ((int)((rta)->rta_len) - RTA_LENGTH(0))


enum
{
	IFLA_UNSPEC,
	IFLA_ADDRESS,
	IFLA_BROADCAST,
	IFLA_IFNAME,
	IFLA_MTU,
	IFLA_LINK,
	IFLA_QDISC,
	IFLA_STATS,
	IFLA_COST,
#define IFLA_COST IFLA_COST
	IFLA_PRIORITY,
#define IFLA_PRIORITY IFLA_PRIORITY
	IFLA_MASTER,
#define IFLA_MASTER IFLA_MASTER
	IFLA_WIRELESS,		/* Wireless Extension event - see wireless.h */
#define IFLA_WIRELESS IFLA_WIRELESS
	IFLA_PROTINFO,		/* Protocol specific information for a link */
#define IFLA_PROTINFO IFLA_PROTINFO
	IFLA_TXQLEN,
#define IFLA_TXQLEN IFLA_TXQLEN
	IFLA_MAP,
#define IFLA_MAP IFLA_MAP
	IFLA_WEIGHT,
#define IFLA_WEIGHT IFLA_WEIGHT
	IFLA_OPERSTATE,
	IFLA_LINKMODE,
	__IFLA_MAX
};


#define IFLA_MAX (__IFLA_MAX - 1)

#define IFLA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#define IFLA_PAYLOAD(n) NLMSG_PAYLOAD(n,sizeof(struct ifinfomsg))


/* The struct should be in sync with struct net_device_stats */
struct if_stats
{
  unsigned long long rx_packets;   /* total packets received       */
  unsigned long long tx_packets;   /* total packets transmitted    */
  unsigned long long rx_bytes;     /* total bytes received         */
  unsigned long long tx_bytes;     /* total bytes transmitted      */
  unsigned long long rx_errors;    /* bad packets received         */
  unsigned long long tx_errors;    /* packet transmit problems     */
  unsigned long long rx_dropped;   /* no space in linux buffers    */
  unsigned long long tx_dropped;   /* no space available in linux  */
  unsigned long long rx_multicast; /* multicast packets received   */
  unsigned long long tx_multicast; /* multicast packets transmitted   */
  unsigned long long rx_compressed;
  unsigned long long tx_compressed;
  unsigned long long collisions;

  /* detailed rx_errors: */
  unsigned long long rx_length_errors;
  unsigned long long rx_over_errors;       /* receiver ring buff overflow  */
  unsigned long long rx_crc_errors;        /* recved pkt with crc error    */
  unsigned long long rx_frame_errors;      /* recv'd frame alignment error */
  unsigned long long rx_fifo_errors;       /* recv'r fifo overrun          */
  unsigned long long rx_missed_errors;     /* receiver missed packet     */
  /* detailed tx_errors */
  unsigned long long tx_aborted_errors;
  unsigned long long tx_carrier_errors;
  unsigned long long tx_fifo_errors;
  unsigned long long tx_heartbeat_errors;
  unsigned long long tx_window_errors;

  /*lilong add 2015.2.8 */
  unsigned long long  Ip6InReceives;                           
  unsigned long long  Ip6InHdrErrors;                          
  unsigned long long  Ip6InTooBigErrors;                       
  unsigned long long  Ip6InNoRoutes;                           
  unsigned long long  Ip6InAddrErrors;                         
  unsigned long long  Ip6InUnknownProtos;                      
  unsigned long long  Ip6InTruncatedPkts;                      
  unsigned long long  Ip6InDiscards;                     
  unsigned long long  Ip6InDelivers;                          
  unsigned long long  Ip6OutForwDatagrams;                     
  unsigned long long  Ip6OutRequests;                          
  unsigned long long  Ip6OutDiscards;                          
  unsigned long long  Ip6OutNoRoutes;                          
  unsigned long long  Ip6ReasmTimeout;                         
  unsigned long long  Ip6ReasmReqds;                           
  unsigned long long  Ip6ReasmOKs;                             
  unsigned long long  Ip6ReasmFails;                           
  unsigned long long  Ip6FragOKs;                              
  unsigned long long  Ip6FragFails;                            
  unsigned long long  Ip6FragCreates;                          
  unsigned long long  Ip6InMcastPkts;                          
  unsigned long long  Ip6OutMcastPkts;                         
  unsigned long long  Ip6InOctets;                             
  unsigned long long  Ip6OutOctets;                            
  unsigned long long  Ip6InMcastOctets;                        
  unsigned long long  Ip6OutMcastOctets;                       
  unsigned long long  Ip6InBcastOctets;                        
  unsigned long long  Ip6OutBcastOctets;     
};



struct intflib_link_stats
{
	unsigned int	rx_packets;		/* total packets received	*/
	unsigned int	tx_packets;		/* total packets transmitted	*/
	unsigned int	rx_bytes;		/* total bytes received 	*/
	unsigned int	tx_bytes;		/* total bytes transmitted	*/
	unsigned int	rx_errors;		/* bad packets received		*/
	unsigned int	tx_errors;		/* packet transmit problems	*/
	unsigned int	rx_dropped;		/* no space in linux buffers	*/
	unsigned int	tx_dropped;		/* no space available in linux	*/
	unsigned int	multicast;		/* multicast packets received	*/
	unsigned int	collisions;

	/* detailed rx_errors: */
	unsigned int	rx_length_errors;
	unsigned int	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned int	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned int	rx_frame_errors;	/* recv'd frame alignment error */
	unsigned int	rx_fifo_errors;		/* recv'r fifo overrun		*/
	unsigned int	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	unsigned int	tx_aborted_errors;
	unsigned int	tx_carrier_errors;
	unsigned int	tx_fifo_errors;
	unsigned int	tx_heartbeat_errors;
	unsigned int	tx_window_errors;
	
	/* for cslip etc */
	unsigned int	rx_compressed;
	unsigned int	tx_compressed;
};

struct packetnum
{
	unsigned long	packets;		/* unicast packets received	*/
	unsigned long	bytes;		/* total bytes received 	*/
	unsigned long	errors;		/* bad packets received		*/
	unsigned long	dropped;		/* no space in linux buffers	*/
	unsigned long	multicast;		/* multicast(anti-unicast) packets received	*/
};

struct rxtx
{
	unsigned int index;
	struct packetnum rx;
	struct packetnum tx;
};

struct flow_data
{
    
	char ifname[IFNAME_SIZE];
	struct rxtx rtdata; 
	unsigned long long	rxsample;
	unsigned long long	txsample;
};

struct flow_data_list
{
	struct flow_data_list *next;
	struct flow_data ltdata;
};

struct if_stats_list {
	char ifname[IFNAME_SIZE];
    struct if_stats stats;
    struct if_stats_list *next;
};


typedef int (*rtnl_filter_t)(const struct sockaddr_nl *, 
			     struct nlmsghdr *n, void *);



#endif


