#ifndef __SE_AGENT_USER_H__
#define __SE_AGENT_USER_H__

#include "cvmx-spinlock.h"
#include "se_agent.h"
#include <linux/tipc.h>
#include "cvmx-rwlock.h"



/********************************************************
 *	user tables
 *********************************************************/
typedef struct  user_item_s{
	struct user_item_s *next;
	struct user_info_s user_info;
	//cvmx_spinlock_t      lock; /*only the first bucket lock is used*/
	cvmx_rwlock_wp_lock_t lock;
	uint16_t valid_entries;  /*only the first bucket valid_entries is used for the number of rule entry*/
	uint8_t reserved[2];	
}user_item_t;


#define USER_TBL_RULE_NAME           "user_tbl"
#define USER_DYNAMIC_TBL_RULE_NAME   "user_dynamic_tbl"

extern int se_agent_user_offline(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern int se_agent_user_online(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern int se_agent_get_user_flow_statistics(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern int se_agent_config_pure_payload_acct(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_fwd_user_stats(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern int se_agent_show_user_rule_by_ip(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);
extern void se_agent_show_fwd_user_rule_all(char *buf,struct sockaddr_tipc *client_addr,unsigned int len);

#endif
