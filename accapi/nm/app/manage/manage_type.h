#ifndef _MANAGE_TYPE_H
#define _MANAGE_TYPE_H

#define 	MANAGEERR_SUCCESS		(0)
#define	MANAGEERR_OPEN_FAIL		(-1)
#define	MANAGEERR_BAD_ADDRESS	(-2)
#define 	MANAGEERR_ADDR_MATCH	(-3)
#define 	MANAGEERR_BAD_PARSE		(-4)
#define 	MANAGEERR_MALLOC_FAIL	(-5)
#define 	MANAGEERR_BAD_PRE		(-6)
#define	MANAGEERR_BAD_INPUT		(-7)
#define	MANAGEERR_PACKET_PROXY	(-8)
#define	MANAGEERR_BAD_TYPE		(-9)


#define 	MANAGE_FLAGS_STREAM_SOCKET   			(0x1)
#define 	MANAGE_FLAGS_TIPC_SOCKET   				(0x1 << 1)
#define	MANAGE_FLAGS_RELAY_SOCKET				(0x1 << 2)
#define MANAGE_FLAGS_NETLINK_SEM_SOCKET   			(0x1 << 3)


#define	MANAGE_TIPC_TYPE	(0x7000)

enum {
	MANAGE_MESSAGE_SIGNAL,
	MANAGE_MESSAGE_METHOD_CALL,
	MANAGE_MESSAGE_METHOD_CALL_REPLY,
};


#ifndef MANAGE_FREE
#define MANAGE_FREE(obj_name)		{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#endif

typedef struct manage_tipc_addr_s			manage_tipc_addr;
typedef struct manage_netlink_addr_s			manage_netlink_addr;
typedef struct manage_tipc_addr_group_s	manage_tipc_addr_group;
typedef struct manage_netlink_addr_group_s	manage_netlink_addr_group;
typedef struct manage_session_s			manage_session;
typedef struct manage_message_s			manage_message;
typedef struct manage_method_s			manage_method;


typedef int (*manage_callback) (int, manage_session *, 
									int, void *, void *);

typedef int (*task_method) (void *, size_t, 
								void **, size_t *);

struct manage_tipc_addr_s {
	u_int type;
	u_int instance;
};
struct manage_netlink_addr_s {
	
	unsigned int protocol;//  
	unsigned int 	nl_pid;		/* port ID	*/
    unsigned int	nl_groups;	/* multicast groups mask */
};

struct manage_tipc_addr_group_s {

	/** dest tipc addr **/
	manage_tipc_addr dest;

	/** sour tipc addr **/
	manage_tipc_addr sour;
};

struct manage_netlink_addr_group_s {

	/** dest sem netlink addr **/
	manage_netlink_addr dest;

	/** src sem netlink addr **/
	manage_netlink_addr src;
};


struct manage_session_s {
	u_long	sessid; 
	
	u_long	flags;

	u_char 	*local;
	size_t	  local_len;

	u_char 	*remote;
	size_t	  remote_len;

	manage_callback callback;
	task_method		Method_s;

	/** copy of system errno */
	int             s_errno;
	/** copy of library errno */
	int             s_manage_errno;   

};

/* manage message 12 byte*/
struct manage_message_s {
	u_char	m_type;
	
	u_char 	m_errno;
	
	u_short	method_id;

	size_t 	data_length;

	u_int	m_check;
};

struct manage_method_s {
	u_short		method_id;
	task_method	method;
};

extern manage_tipc_addr tipc_relayaddr;


#endif
