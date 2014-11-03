
#ifndef _EAG_IPSET6_H
#define _EAG_IPSET6_H

#include <sys/types.h>
#include <netdb.h>
#include "session.h"

#define PRIVATE_MSG_BUFLEN      256
#define IPSET6_MAXNAMELEN       32

enum {
	IPSET6_NFPROTO_IPV4   =  2,
	IPSET6_NFPROTO_IPV6   =  10,
};

enum ipset6_cmd {
	IPSET6_CMD_PROTOCOL = 1,   /* 1: Return protocol version */
	IPSET6_CMD_ADD      = 9,   /* 9: Add an element to a set */
	IPSET6_CMD_DEL      = 10,  /* 10: Delete an element from a set */
	IPSET6_CMD_HEADER   = 12,  /* 12: Get set header data only */
};

int
set_user_in_ipset6( const int user_id, 
					const int hansitype,
					user_addr_t *user_addr,  
					enum ipset6_cmd cmd );

int
set_preauth_user_in_ipset6( const int user_id, 
							const int hansitype,
							user_addr_t *user_addr, 
							enum ipset6_cmd cmd );

int
eag_ipset6_init();

int
eag_ipset6_exit();

#endif
