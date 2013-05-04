#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <linux/netlink.h>
#include <linux/tipc.h>

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-pow.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"


#include "se_agent_log.h"
#include "se_agent_user.h"




extern CVMX_SHARED int se_socket;

extern CVMX_SHARED int nl_socket;

extern CVMX_SHARED user_item_t*  user_bucket_tbl ;
extern CVMX_SHARED uint32_t user_static_tbl_size ;
extern CVMX_SHARED uint32_t user_dynamic_tbl_size ;


int read_user_rule(user_info_t *user_info, user_item_t *user_rule)
{
    if((NULL == user_info) || (NULL == user_rule))
        return SE_AGENT_RETURN_FAIL;
        
    user_info->user_ip = user_rule->user_info.user_ip;
    user_info->user_state = user_rule->user_info.user_state;
    user_info->forward_up_bytes = user_rule->user_info.forward_up_bytes;
    user_info->forward_up_packet = user_rule->user_info.forward_up_packet;
    user_info->forward_down_bytes = user_rule->user_info.forward_down_bytes;
    user_info->forward_down_packet = user_rule->user_info.forward_down_packet;

    return SE_AGENT_RETURN_OK;
}


static inline uint32_t user_cache_bucket_lookup (uint32_t ip_address)
{
	user_item_t *bucket = NULL;
	uint64_t idx = 0;

#if defined(__KERNEL__) && defined(linux)
	set_c0_status(ST0_CU2);
#endif
	CVMX_MT_CRC_POLYNOMIAL(0x1edc6f41);
	CVMX_MT_CRC_IV(0);
	CVMX_MT_CRC_WORD(*(uint32_t*)(&ip_address));
	CVMX_MF_CRC_IV(idx);
	idx &= (user_static_tbl_size - 1);

	bucket = &user_bucket_tbl[idx];
	cvmx_scratch_write64(CVM_SCR_USER_CACHE_PTR, (uint64_t) (CAST64(bucket)));
	return (idx);
}


/**********************************************************************************
 *  se_agent_user_online
 *
 *	DESCRIPTION:
 * 		Set  fast-fwd  module  user on line
 *
 *	INPUT:
 *		buf			 ->       pointer of store data buffer
 *          client_addr    ->        client socket address
 *		len 			 ->       the length of client socket address
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail  
 **********************************************************************************/
int se_agent_user_online(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret = -1;
	uint32_t user_ip = 0;
	
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_user_online  param error\n");
		return SE_AGENT_RETURN_FAIL;
	}
	se_buf=(se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		se_agent_syslog_err("se_agent_user_online  fastfwd module not load\n");
		return SE_AGENT_RETURN_FAIL;
	}
	user_ip = se_buf->fccp_cmd.fccp_data.user_info.user_ip;
	
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_USER_ONLINE;
	se_buf->fccp_cmd.cmd_len=sizeof(control_cmd_t);
	se_buf->fccp_cmd.agent_pid=getpid();
	if(SE_AGENT_RETURN_FAIL == (send_fccp(&(se_buf->fccp_cmd),0, FCCP_SEND_GROUP)))
	{
		se_agent_syslog_err("se_agent_user_online: user %d.%d.%d.%d online fail\n",IP_FMT(user_ip));
		return SE_AGENT_RETURN_FAIL;
	}
	return SE_AGENT_RETURN_OK;
}

/**********************************************************************************
 *  se_agent_user_offline
 *
 *	DESCRIPTION:
 * 		Set  fast-fwd  module  user off line
 *
 *	INPUT:
 *		buf			 ->       pointer of store data buffer
 *          client_addr    ->        client socket address
 *		len 			 ->       the length of client socket address
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail  
 **********************************************************************************/

int se_agent_user_offline(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	int ret = -1;
	uint32_t user_ip = 0;
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_user_online  param error\n");
		return SE_AGENT_RETURN_FAIL;
	}
	se_buf=(se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		se_agent_syslog_err("se_agent_user_offline  fastfwd module not load\n");
		return SE_AGENT_RETURN_FAIL;
	}
	user_ip = se_buf->fccp_cmd.fccp_data.user_info.user_ip;
	
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_USER_OFFLINE;
	se_buf->fccp_cmd.cmd_len=sizeof(control_cmd_t);
	se_buf->fccp_cmd.agent_pid=getpid();
	if(SE_AGENT_RETURN_FAIL == (send_fccp(&(se_buf->fccp_cmd),0, FCCP_SEND_GROUP)))
	{
		se_agent_syslog_err("se_agent_user_offline: user %d.%d.%d.%d offline fail\n",IP_FMT(user_ip));
		return SE_AGENT_RETURN_FAIL;
	}
	return SE_AGENT_RETURN_OK;
}
/**********************************************************************************
 *  se_agent_show_user_flow_statistics
 *
 *	DESCRIPTION:
 * 		Set  fast-fwd  module  user off line
 *
 *	INPUT:
 *		buf			 ->       pointer of store data buffer
 *          client_addr    ->        client socket address
 *		len 			 ->       the length of client socket address
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail  
 **********************************************************************************/
int se_agent_get_user_flow_statistics(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	user_item_t * user_tbl = NULL;
	int ret = 0;
	uint32_t user_ip = 0;
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_user_flow_statistics  param error\n");
		return SE_AGENT_RETURN_FAIL;
	}
	se_buf=(se_interative_t *)buf;
	user_ip = se_buf->fccp_cmd.fccp_data.user_info.user_ip;
	
	if(NULL == user_bucket_tbl)
	{
		strncpy((se_buf->err_info),FIND_USER_TLB_ERROR_STR,strlen(FIND_USER_TLB_ERROR_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;
	}
	
	user_cache_bucket_lookup(user_ip);
	user_tbl = CASTPTR(user_item_t, cvmx_scratch_read64(CVM_SCR_USER_CACHE_PTR));
	if(0 == user_tbl->valid_entries )
	{
		strncpy((se_buf->err_info),USER_TBL_NO_USER_STR,strlen(USER_TBL_NO_USER_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;			
	}
	while(1)
	{
		if(user_tbl->user_info.user_state == USER_IS_EMPTY || user_tbl->user_info.user_ip != user_ip)
		{
			if(NULL == user_tbl->next)
			{
				strncpy((se_buf->err_info),USER_TBL_NO_USER_STR,strlen(USER_TBL_NO_USER_STR));
				se_buf->cmd_result = AGENT_RETURN_FAIL;
				goto SENDTO_DCLI;	
			}
			user_tbl = (user_item_t *)cvmx_phys_to_ptr((uint64_t)user_tbl->next);
		}
		else
		{
			se_buf->fccp_cmd.fccp_data.user_info.forward_up_bytes = user_tbl->user_info.forward_up_bytes;
			se_buf->fccp_cmd.fccp_data.user_info.forward_up_packet = user_tbl->user_info.forward_up_packet;
			se_buf->fccp_cmd.fccp_data.user_info.forward_down_bytes = user_tbl->user_info.forward_down_bytes;	
			se_buf->fccp_cmd.fccp_data.user_info.forward_down_packet = user_tbl->user_info.forward_down_packet;
			se_buf->cmd_result = AGENT_RETURN_OK;
			goto SENDTO_DCLI;	
		}
			
	}
SENDTO_DCLI:
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_get_user_flow_statistics send to dcli failed\n");
		return SE_AGENT_RETURN_FAIL;
	}
	return SE_AGENT_RETURN_OK;

}

/**********************************************************************************
 *  se_agent_config_user_flow_stats_mode
 *
 *	DESCRIPTION:
 * 		Configuration fast forward module whether contains the header on the user flow statistics
 *
 *	INPUT:
 *		buf			 ->       pointer of store data buffer
 *          client_addr    ->        client socket address
 *		len 			 ->       the length of client socket address
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail  
 **********************************************************************************/

int se_agent_config_pure_payload_acct(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret = -1;
	
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_config_pure_payload_acct param error\n");
		return SE_AGENT_RETURN_FAIL;
	}
	se_buf = (se_interative_t *)buf;
	
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_ENABLE_PURE_PAYLOAD_ACCT;

    se_agent_fccp_process(buf, len, 1);

	ret = sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_config_pure_payload_acct send to dcli failed\n");
		return SE_AGENT_RETURN_FAIL;
	}
	return SE_AGENT_RETURN_OK;
}

/**********************************************************************************
 *  se_agent_show_fwd_user_stats
 *
 *	DESCRIPTION:
 * 		Show info of user table,including total num,used num,and info of all users
 *
 *	INPUT:
 *		buf                 ->       pointer of store data buffer
 *      client_addr         ->       client socket address
 *		len                 ->       the length of client socket address
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail  
 **********************************************************************************/

void se_agent_show_fwd_user_stats(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
    se_interative_t *se_buf=NULL;
	user_item_t     *s_tbl_rule = NULL;
	user_item_t     *d_tbl_rule = NULL;
	int         ret=0;
    int32_t     s_tbl_used_rule = 0;
	int32_t     d_tbl_used_rule=0;
	uint32_t    static_loop = 0;

	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_fwd_user_stats param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;

	if((user_bucket_tbl == NULL) || (FASTFWD_NOT_LOADED == fast_forward_module_load_check()))
	{
		strncpy(se_buf->err_info,FASTFWD_NO_RESPOND_STR,sizeof(FASTFWD_NO_RESPOND_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SEND_DCLI;
	}
	
	for(static_loop = 0; static_loop < user_static_tbl_size; static_loop++)
	{ 
		s_tbl_rule = (user_item_t *)user_bucket_tbl+static_loop;
		if(s_tbl_rule->user_info.user_state == USER_IS_EMPTY)
		{
			if(NULL == s_tbl_rule->next)
				continue;             
		}
		else
		{
            s_tbl_used_rule++;
		}
		/* dynamic info */
		d_tbl_rule = s_tbl_rule->next;
		while(d_tbl_rule != NULL)
		{
			d_tbl_rule = cvmx_phys_to_ptr((uint64_t)d_tbl_rule);
            d_tbl_used_rule++;
			d_tbl_rule = d_tbl_rule->next;
		}		
	}
	
	se_buf->fccp_cmd.fccp_data.user_stats.user_static_tbl_size = user_static_tbl_size;
	se_buf->fccp_cmd.fccp_data.user_stats.user_dynamic_tbl_size = user_dynamic_tbl_size;
	se_buf->fccp_cmd.fccp_data.user_stats.s_tbl_used_rule = s_tbl_used_rule;
	se_buf->fccp_cmd.fccp_data.user_stats.d_tbl_used_rule = d_tbl_used_rule;
    se_buf->cmd_result=AGENT_RETURN_OK;

SEND_DCLI:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_fwd_user_stats send to dcli failed:%s\n",strerror(errno));
		return ;
	}
	return;
}

/**********************************************************************************
 *  se_agent_show_fwd_user_rule_all
 *
 *	DESCRIPTION:
 * 		Show info of all users
 *
 *	INPUT:
 *		buf                 ->       pointer of store data buffer
 *      client_addr         ->       client socket address
 *		len                 ->       the length of client socket address
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail  
 **********************************************************************************/

void se_agent_show_fwd_user_rule_all(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
    se_interative_t *se_buf=NULL;
	user_item_t 	*rule	= NULL;
	int         ret=0;
	uint32_t    i = 0;
	uint32_t     static_loop = 0;
	int32_t     static_index = 0;
	uint32_t    dynamic_index = 0;

	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_fwd_user_rule_all param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;

	if((user_bucket_tbl == NULL) || (FASTFWD_NOT_LOADED == fast_forward_module_load_check()))
	{
		strncpy(se_buf->err_info,FASTFWD_NO_RESPOND_STR,sizeof(FASTFWD_NO_RESPOND_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SEND_DCLI;
	}

    static_index = se_buf->fccp_cmd.fccp_data.user_rule.static_index;
    dynamic_index = se_buf->fccp_cmd.fccp_data.user_rule.dynamic_index;
    
	for(static_loop=static_index; static_loop < user_static_tbl_size; static_loop++)
	{ 
		rule = (user_item_t *)user_bucket_tbl+static_loop;

        for(i=0; i<dynamic_index; i++)
        {
            rule = cvmx_phys_to_ptr((uint64_t)rule);
            rule = rule->next;
        }

		while(NULL != rule)
		{
		    rule = cvmx_phys_to_ptr((uint64_t)rule);
		    if(USER_IS_EMPTY != rule->user_info.user_state)
		    {
		        ret = read_user_rule(&(se_buf->fccp_cmd.fccp_data.user_rule.user_info), rule);
		        if(SE_AGENT_RETURN_OK != ret)
		        {
		            se_buf->cmd_result=AGENT_RETURN_FAIL;
                    strncpy(se_buf->err_info,READ_RULE_ERROR,strlen(READ_RULE_ERROR));
                    goto SEND_DCLI;
		        }
		        se_buf->fccp_cmd.fccp_data.user_rule.user_index = static_loop;
		        se_buf->fccp_cmd.fccp_data.user_rule.user_link_index = dynamic_index;
		        se_buf->fccp_cmd.fccp_data.user_rule.static_index = static_loop;
		        se_buf->fccp_cmd.fccp_data.user_rule.dynamic_index = dynamic_index + 1;
		        se_buf->cmd_result=AGENT_RETURN_OK;
				goto SEND_DCLI;
		    }
		    rule = rule->next;
		    dynamic_index++;
		}
        dynamic_index = 0;
	}

    se_buf->cmd_result=AGENT_RETURN_OK;
    se_buf->fccp_cmd.fccp_data.user_rule.static_index = -1;
SEND_DCLI:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_fwd_user_rule_all send to dcli failed:%s\n",strerror(errno));
		return ;
	}
	return;
}

/**********************************************************************************
 *  se_agent_show_fwd_user_rule_by_ip
 *
 *	DESCRIPTION:
 * 		Show info of specific user
 *
 *	INPUT:
 *		buf                 ->       pointer of store data buffer
 *      client_addr         ->       client socket address
 *		len                 ->       the length of client socket address
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail  
 **********************************************************************************/

int se_agent_show_user_rule_by_ip(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	user_item_t * user_tbl = NULL;
	int ret = 0;
	uint32_t user_ip = 0;
	uint32_t user_index = 0xffffffff;
    uint32_t user_link_index = 0;
    
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_user_rule_by_ip  param error\n");
		return SE_AGENT_RETURN_FAIL;
	}
	se_buf=(se_interative_t *)buf;
	user_ip = se_buf->fccp_cmd.fccp_data.user_rule.user_info.user_ip;
	
	if(NULL == user_bucket_tbl)
	{
		strncpy((se_buf->err_info),FIND_USER_TLB_ERROR_STR,strlen(FIND_USER_TLB_ERROR_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;
	}
	
	user_index = user_cache_bucket_lookup(user_ip);
	user_tbl = CASTPTR(user_item_t, cvmx_scratch_read64(CVM_SCR_USER_CACHE_PTR));
	if(0 == user_tbl->valid_entries )
	{
		strncpy((se_buf->err_info),USER_TBL_NO_USER_STR,strlen(USER_TBL_NO_USER_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;			
	}
	while(1)
	{
		if(user_tbl->user_info.user_state == USER_IS_EMPTY || user_tbl->user_info.user_ip != user_ip)
		{
			if(NULL == user_tbl->next)
			{
				strncpy((se_buf->err_info),USER_TBL_NO_USER_STR,strlen(USER_TBL_NO_USER_STR));
				se_buf->cmd_result = AGENT_RETURN_FAIL;
				goto SENDTO_DCLI;	
			}
			user_tbl = (user_item_t *)cvmx_phys_to_ptr((uint64_t)user_tbl->next);
			user_link_index++;
		}
		else
		{
			se_buf->fccp_cmd.fccp_data.user_rule.user_index = user_index;
    		se_buf->fccp_cmd.fccp_data.user_rule.user_link_index = user_link_index;
    		ret = read_user_rule(&(se_buf->fccp_cmd.fccp_data.user_rule.user_info), user_tbl);
	        if(SE_AGENT_RETURN_OK != ret)
	        {
	            se_buf->cmd_result=AGENT_RETURN_FAIL;
                strncpy(se_buf->err_info,READ_RULE_ERROR,strlen(READ_RULE_ERROR));
                goto SENDTO_DCLI;
	        }
			se_buf->cmd_result = AGENT_RETURN_OK;
			goto SENDTO_DCLI;	
		}
			
	}
SENDTO_DCLI:
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_user_rule_by_ip send to dcli failed\n");
		return SE_AGENT_RETURN_FAIL;
	}
	return SE_AGENT_RETURN_OK;

}

