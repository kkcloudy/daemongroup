#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <linux/netlink.h>

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-fpa.h"
#include "cvmx-pko.h"
#include "cvmx-pow.h"
#include "cvmx-pip.h"
#include "cvmx-ipd.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-helper.h"
#include "cvmx-bootmem.h"

#include "se_agent.h"


uint64_t get_sec()
{
	uint64_t sec;
	sec= (uint64_t ) (cvmx_clock_get_count(CVMX_CLOCK_SCLK) / cvmx_sysinfo_get()->cpu_clock_hz);
	return sec  ;
}
 
int acl_aging_check(rule_param_t *check_rule)
{
	uint64_t sec = get_sec();
	if(check_rule->time_stamp > sec) /*·´×ª*/
	{
		return 0;			
	}
	else
	{
		if(sec -check_rule->time_stamp > aging_time)
			return 1;
	}
	return 0;
}

int acl_agingstamp_cmp(rule_param_t *check_rule,uint64_t timestamp)
{
	uint64_t sec = get_sec();
	if ((sec - check_rule->time_stamp) > (timestamp*60))
	{
		return 1;
	}
	
	return 0;
}

int  read_capwap(se_interative_t *se_buf,capwap_cache_t *capwap)
{
	if(NULL == se_buf || NULL == capwap)
		return SE_AGENT_RETURN_FAIL;
	se_buf->fccp_cmd.fccp_data.rule_info.cw_cache.use_num = capwap->use_num;
	se_buf->fccp_cmd.fccp_data.rule_info.cw_cache.dip = capwap->dip;
	se_buf->fccp_cmd.fccp_data.rule_info.cw_cache.sip = capwap->sip;
	se_buf->fccp_cmd.fccp_data.rule_info.cw_cache.dport = capwap->dport;
	se_buf->fccp_cmd.fccp_data.rule_info.cw_cache.sport = capwap->sport;
	se_buf->fccp_cmd.fccp_data.rule_info.cw_cache.tos = capwap->tos;
	memcpy(se_buf->fccp_cmd.fccp_data.rule_info.cw_cache.cw_hd,capwap->cw_hd,CW_H_LEN);
	return SE_AGENT_RETURN_OK;
}
int read_specified_rule(rule_param_t *dst_rule,rule_item_t *src_rule)
{
	if(NULL == dst_rule || NULL == src_rule)
		return SE_AGENT_RETURN_FAIL;
	dst_rule->dsa_info = src_rule->rules.dsa_info;
	memcpy(dst_rule->ether_dhost,src_rule->rules.ether_dhost,6);
	memcpy(dst_rule->ether_shost,src_rule->rules.ether_shost,6);
	dst_rule->out_ether_type = src_rule->rules.out_ether_type;
	dst_rule->out_tag = src_rule->rules.out_tag;
	dst_rule->in_ether_type = src_rule->rules.in_ether_type;
	dst_rule->in_tag = src_rule->rules.in_tag;
	dst_rule->ether_type = src_rule->rules.ether_type;
	
	dst_rule->sip = src_rule->rules.sip;
	dst_rule->dip = src_rule->rules.dip;
	dst_rule->sport = src_rule->rules.sport;
	dst_rule->dport = src_rule->rules.dport;
	dst_rule->protocol = src_rule->rules.protocol;
	
	dst_rule->forward_port = src_rule->rules.forward_port;
	dst_rule->action_type = src_rule->rules.action_type;
	dst_rule->rule_state = src_rule->rules.rule_state;
	if(acl_aging_check((&src_rule->rules)) >0 )
		dst_rule->time_stamp=RULE_IS_AGE;
	else 
		dst_rule->time_stamp=RULE_IS_NEW;
	switch (src_rule->rules.action_type)
	{
		case FLOW_ACTION_RPA_ETH_FORWARD :
		case FLOW_ACTION_RPA_CAPWAP_FORWARD :
		case FLOW_ACTION_RPA_CAP802_3_FORWARD :
		case FLOW_ACTION_RPA_CAPWAP_802_3_ICMP :
		case FLOW_ACTION_RPA_CAPWAP_802_11_ICMP :
		case FLOW_ACTION_RPA_ICMP :
			dst_rule->rpa_header.rpa_type = src_rule->rules.rpa_header.rpa_type;
			dst_rule->rpa_header.rpa_dnetdevNum = src_rule->rules.rpa_header.rpa_dnetdevNum;
			dst_rule->rpa_header.rpa_snetdevNum = src_rule->rules.rpa_header.rpa_snetdevNum;
			dst_rule->rpa_header.rpa_d_s_slotNum = src_rule->rules.rpa_header.rpa_d_s_slotNum;
			break;
		default:
			break;

	}

	/*wangjian add for slave cpu ip is 0 20121214*/
	if (src_rule->rules.nat_flag == 1)
	{
		dst_rule->nat_flag = src_rule->rules.nat_flag;
		dst_rule->nat_sip = src_rule->rules.nat_sip;
		dst_rule->nat_dip = src_rule->rules.nat_dip;
		dst_rule->nat_sport = src_rule->rules.nat_sport;
		dst_rule->nat_dport = src_rule->rules.nat_dport;
	}

	/*add by wangjian for support pppoe 2013-3-15*/
	if (src_rule->rules.pppoe_flag == 1)
	{
		dst_rule->pppoe_flag = src_rule->rules.pppoe_flag;
		dst_rule->pppoe_session_id = src_rule->rules.pppoe_session_id;
	}
	
	return SE_AGENT_RETURN_OK;
}


/**********************************************************************************
 *  se_agent_delete_specified_rule
 *
 *	DESCRIPTION:
 * 		Delete   fast-fwd  module  rule
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
void se_agent_delete_specified_rule(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret = -1;
	
	if(NULL == buf || NULL == client_addr ||0 == len)
	{
		se_agent_syslog_err("se_agent_delete_specified_rule  param error\n");
		return ;
	}
	
	se_buf=(se_interative_t *)buf;

	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_DELETE;

    se_agent_fccp_process(buf, len, 1);    
    
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_delete_specified_rule send to dcli failed\n");
		return ;
	}
	return ;
}

void se_agent_show_specified_rule(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	rule_item_t     *rule   = NULL;
	capwap_cache_t  *capwap = NULL;
	int ret = -1;
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_specified_rule  param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;
	}
	
	cvm_five_tuple_hash(se_buf->fccp_cmd.fccp_data.rule_tuple.ip_dst, se_buf->fccp_cmd.fccp_data.rule_tuple.ip_src, 
			se_buf->fccp_cmd.fccp_data.rule_tuple.ip_p, se_buf->fccp_cmd.fccp_data.rule_tuple.th_dport, 
			se_buf->fccp_cmd.fccp_data.rule_tuple.th_sport);
	rule = CASTPTR(rule_item_t, cvmx_scratch_read64(CVM_SCR_ACL_CACHE_PTR));
	if(rule->valid_entries== 0)
		goto NO_RULE_ERR;
	while(1)
	{ 		 
		if((rule->rules.dip == se_buf->fccp_cmd.fccp_data.rule_tuple.ip_dst) && (rule->rules.sip == se_buf->fccp_cmd.fccp_data.rule_tuple.ip_src) &&
				(rule->rules.dport == se_buf->fccp_cmd.fccp_data.rule_tuple.th_dport) &&(rule->rules.sport == se_buf->fccp_cmd.fccp_data.rule_tuple.th_sport) &&
				(rule->rules.protocol == se_buf->fccp_cmd.fccp_data.rule_tuple.ip_p)) 
		{
			if(SE_AGENT_RETURN_OK != read_specified_rule(&(se_buf->fccp_cmd.fccp_data.rule_info.rule_param),rule))
				goto READ_RULE_ERR;
					
			if((rule->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD) ||
					(rule->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD))
			{
				capwap=&(capwap_cache_tbl[rule->rules.tunnel_index]);
				se_agent_syslog_dbg("se_agent_show_specified_rule: find the capwap channel,capwap=0x%p\n",capwap);
				if(SE_AGENT_RETURN_OK != read_capwap(se_buf,capwap))
					goto READ_RULE_ERR;
			}
			se_buf->cmd_result=AGENT_RETURN_OK;
			goto SENDTO_DCLI;
		} 		   
		if(rule->next != NULL)
		{
			rule = (rule_item_t *)((uint64_t)(rule->next)|0x8000000000000000);	
		}
		else
			goto NO_RULE_ERR;
	}
NO_RULE_ERR:
	strncpy(se_buf->err_info,NO_THIS_RULE_STR,strlen(NO_THIS_RULE_STR));
	se_buf->cmd_result=AGENT_RETURN_FAIL;
	goto SENDTO_DCLI;
READ_RULE_ERR:
	strncpy(se_buf->err_info,READ_RULE_ERROR,strlen(READ_RULE_ERROR));
	se_buf->cmd_result=AGENT_RETURN_FAIL;
	goto SENDTO_DCLI;
SENDTO_DCLI:
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_specified_rule send to dcli failed\n");
		return ;
	}
	return;
}

void se_agent_show_rule_sum(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	rule_item_t     *rule   = NULL;
	capwap_cache_t  *capwap = NULL;
	int ret;
	uint32_t s_tbl_used_rule = 0;
	uint32_t s_tbl_aged_rule = 0;
	uint32_t s_tbl_learned_rule = 0;
	uint32_t s_tbl_learning_rule = 0;
	uint32_t s_tbl_static_rule = 0;
	uint32_t d_tbl_used_rule = 0;
	uint32_t d_tbl_aged_rule = 0;
	uint32_t d_tbl_learned_rule = 0;
	uint32_t d_tbl_learning_rule = 0;
	uint32_t d_tbl_static_rule = 0;
	uint32_t i = 0;
	rule_item_t * s_tbl_rule = NULL;
	rule_item_t * d_tbl_rule = NULL;
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_rule_sum  param error\n");
			return ;
	}
	se_buf=(se_interative_t *)buf;
	
	if(se_buf->cpu_tag == 1)
    {
        se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	    se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	    se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_GET_RULE_STATS;

        se_agent_fccp_process_pcie(buf, len, 1);
        if(se_buf->fccp_cmd.ret_val == FCCP_RETURN_ERROR)
    	{
    	    strncpy(se_buf->err_info,READ_RULE_ERROR,sizeof(READ_RULE_ERROR));
		    se_buf->cmd_result = AGENT_RETURN_FAIL;
    	}
        goto func_end;
    }
	
	if((FASTFWD_NOT_LOADED == fast_forward_module_load_check()) || (NULL == capwap_cache_tbl) || (NULL == acl_bucket_tbl))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto func_end;
	}
	for(i = 0; i < acl_static_tbl_size; i++)
	{
		s_tbl_rule = (rule_item_t *)acl_bucket_tbl+i;
		if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(NULL == s_tbl_rule->next)
				continue;             
		}
		else
		{
			switch(s_tbl_rule->rules.rule_state)
			{
				case RULE_IS_LEARNED:
					s_tbl_learned_rule++;
					break;
				case RULE_IS_LEARNING:
					s_tbl_learning_rule++;
					break;
				case RULE_IS_STATIC:
					s_tbl_static_rule++;
					break;    
				default:
					se_agent_syslog_dbg("error rule state %d!\n", s_tbl_rule->rules.rule_state);
					goto find_err;
			}
			s_tbl_used_rule++;
			if(acl_aging_check(&(s_tbl_rule->rules)) > 0)
			{
				s_tbl_aged_rule++;
			}
		}
		
		/* dynamic info */
		d_tbl_rule = s_tbl_rule->next;
		while(d_tbl_rule != NULL)
		{
			d_tbl_rule=cvmx_phys_to_ptr((uint64_t)d_tbl_rule);/*in SE physical address is equal to the virtual address*/
			switch(d_tbl_rule->rules.rule_state)
			{
				case RULE_IS_LEARNED:
					d_tbl_learned_rule++;
					break;
				case RULE_IS_LEARNING:
					d_tbl_learning_rule++;
					break;
				case RULE_IS_STATIC:
					d_tbl_static_rule++;
					break;    
				default:
					se_agent_syslog_err("error rule state %d!\n", d_tbl_rule->rules.rule_state);
					goto find_err;
			}
			d_tbl_used_rule++;
			if(acl_aging_check(&(d_tbl_rule->rules)) > 0)
			{
				d_tbl_aged_rule++;
			}
			d_tbl_rule =d_tbl_rule->next;
		}
	}

	/* capwap table info */
	uint32_t cw_tbl_used = 0;
	uint32_t cw_tbl_802_3_num = 0;
	uint32_t cw_tbl_802_11_num = 0;
	union capwap_hd* cw_hdr;
	for(i = 0; i < MAX_CAPWAP_CACHE_NUM; i++)
	{ 
		if(capwap_cache_tbl[i].use_num != 0)
		{
			cw_tbl_used++;
			cw_hdr = (union capwap_hd*)(capwap_cache_tbl[i].cw_hd);
			if(cw_hdr->m_t == 0)
			{
				cw_tbl_802_3_num++;
			}            
			else if(cw_hdr->m_wbid == 1)
			{
				cw_tbl_802_11_num++;
			}
		}
	}
	se_buf->fccp_cmd.fccp_data.rule_sum.acl_dynamic_tbl_size=acl_dynamic_tbl_size;
	se_buf->fccp_cmd.fccp_data.rule_sum.acl_static_tbl_size=acl_static_tbl_size;
	se_buf->fccp_cmd.fccp_data.rule_sum.capwap_cache_tbl_size=capwap_cache_tbl_size;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule=s_tbl_used_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_static_rule=s_tbl_static_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_learning_rule=s_tbl_learning_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_learned_rule=s_tbl_learned_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule=s_tbl_aged_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule=d_tbl_used_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_static_rule=d_tbl_static_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_learning_rule=d_tbl_learning_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_learned_rule=d_tbl_learned_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule = d_tbl_aged_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.cw_tbl_802_11_num = cw_tbl_802_11_num;
	se_buf->fccp_cmd.fccp_data.rule_sum.cw_tbl_802_3_num=cw_tbl_802_3_num;
	se_buf->fccp_cmd.fccp_data.rule_sum.cw_tbl_used=cw_tbl_used;
	se_buf->cmd_result=AGENT_RETURN_OK;
	goto func_end;
find_err:
	strncpy(se_buf->err_info,READ_RULE_ERROR,strlen(READ_RULE_ERROR));
	se_buf->cmd_result=AGENT_RETURN_FAIL;
func_end:	
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("send to dcli failed\n");
		return ;
	}
	return;
	
	
}

void se_agent_clear_rule_all(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	int ret;
	
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_clear_rule_all  param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;

	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_CLEAR_RULE;

    switch(se_buf->cpu_tag)
    {
        case 0:
            se_agent_fccp_process(buf, len, 1);
            break;
        case 1:
            se_agent_fccp_process_pcie(buf, len, 1);
            break;
        default:
            se_agent_syslog_err("se_agent_clear_rule_all cpu_tag invalid\n");
		    return;
    }

	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_clear_rule_all send to dcli failed\n");
		return ;
	}
	return ;
}
void se_agent_show_capwap_rule(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	struct timeval  overtime;
	union capwap_hd* cw_hdr=NULL;
	uint32_t i=0, j=0;
	uint32_t total_count=0;
	int ret;
	uint32_t cw_cache_index=0;
	uint32_t loop=0,index=0;
	uint32_t first_flag = 1;
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_capwap_rule param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;
	if((NULL == capwap_cache_tbl)||(FASTFWD_NOT_LOADED == fast_forward_module_load_check()))
	{
		strncpy(se_buf->err_info,FASTFWD_NO_RESPOND_STR,sizeof(FASTFWD_NO_RESPOND_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SEND_DCLI;
	}
	index=se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_index;
	if(index == 0)
	{
		for(loop = 0; loop < capwap_cache_tbl_size; loop++)
		{ 
			if(capwap_cache_tbl[loop].use_num != 0)
			{
				if(first_flag)
				{
					first_flag=0;
					cw_cache_index=loop;
				}
				total_count++;
			}
		}
		se_buf->cmd_result=AGENT_RETURN_OK;
		se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_index=cw_cache_index+1;
		se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_use_cnt=total_count;
		goto SEND_DCLI;
	}
	else
	{
		for(loop=index; loop < capwap_cache_tbl_size; loop++)
		{ 
			if(capwap_cache_tbl[loop].use_num != 0)
			{
				se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.use_num = capwap_cache_tbl[loop].use_num;
				se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.dip = capwap_cache_tbl[loop].dip;
				se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.sip = capwap_cache_tbl[loop].sip;
				se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.dport = capwap_cache_tbl[loop].dport;
				se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.sport = capwap_cache_tbl[loop].sport;
				se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.tos = capwap_cache_tbl[loop].tos;
				memcpy(se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.cw_hd,capwap_cache_tbl[loop].cw_hd,CW_H_LEN);
				se_buf->fccp_cmd.fccp_data.cw_cache_info.cw_cache_index=loop+1;
				se_buf->cmd_result=AGENT_RETURN_OK;
				goto SEND_DCLI;
			}
		}
	}
SEND_DCLI:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_capwap_rule send to dcli failed:%s\n",strerror(errno));
		return ;
	}
	return;
}
void se_agent_show_learned_acl(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	struct timeval  overtime;
	rule_item_t 	*rule	= NULL;
	int ret=0;
	uint32_t i = 0;
	rule_item_t * s_tbl_rule = NULL;
	rule_item_t * d_tbl_rule = NULL;
	int32_t total_count = 0;
	int32_t static_count=0;
	//uint32_t acl_static_index = 0xFFFFFFFF;
	uint32_t first_static = 1;
	uint32_t static_loop = 0;
	uint32_t static_index = 0;
	uint32_t dynamic_index = 0;
	
	capwap_cache_t * capwap=NULL;
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_learned_acl param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;

	if((acl_bucket_tbl == NULL) || (FASTFWD_NOT_LOADED == fast_forward_module_load_check()))
	{
		strncpy(se_buf->err_info,FASTFWD_NO_RESPOND_STR,sizeof(FASTFWD_NO_RESPOND_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SEND_DCLI;
	}
	static_index=se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index;

	if(0xFFFFFFFF == static_index)
	{
		for(static_loop = 0; static_loop < acl_static_tbl_size; static_loop++)
		{ 
			s_tbl_rule = (rule_item_t *)acl_bucket_tbl+static_loop;
			if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
			{
				if(NULL == s_tbl_rule->next)
					continue;             
			}
			else
			{
				switch(s_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNED:
						if(first_static==1)
						{
							first_static=0;
							static_index=static_loop;
						}
						total_count++;
						static_count++;
						break;
					default:
						break;
				}
			}
			/* dynamic info */
			d_tbl_rule = s_tbl_rule->next;
			while(d_tbl_rule != NULL)
			{
				
				d_tbl_rule = cvmx_phys_to_ptr((uint64_t)d_tbl_rule);
				switch(d_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNED:
						total_count++;
						break;
					default:
						break;
				}

				d_tbl_rule = d_tbl_rule->next;
			}		
		}
		se_buf->cmd_result=AGENT_RETURN_OK;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index = static_index;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_static_cnt=static_count;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_cnt=total_count;
		goto SEND_DCLI;
	}
	else
	{
		for(static_loop=static_index; static_loop < acl_static_tbl_size; static_loop++)
		{ 
			s_tbl_rule = (rule_item_t *)acl_bucket_tbl+static_loop;
			if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
			{
				continue;             
			}
			else
			{
				switch(s_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNED:
						read_specified_rule(&(se_buf->fccp_cmd.fccp_data.rule_info.rule_param),s_tbl_rule);
						se_buf->cmd_result=AGENT_RETURN_OK;
						se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index=static_loop+1;
						goto SEND_DCLI;
						break;
					default:
						break;
				}
			}
		}
				
	}
	se_buf->cmd_result=AGENT_RETURN_FAIL;
	strncpy(se_buf->err_info,NO_THIS_RULE_STR,strlen(NO_THIS_RULE_STR));
	goto SEND_DCLI;
SEND_DCLI:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_learned_acl send to dcli failed:%s\n",strerror(errno));
		return ;
	}
	return;
}


void se_agent_show_learning_acl(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	struct timeval  overtime;
	rule_item_t 	*rule	= NULL;
	int ret=0;
	uint32_t i = 0;
	rule_item_t * s_tbl_rule = NULL;
	rule_item_t * d_tbl_rule = NULL;
	int32_t total_count = 0;
	int32_t static_count = 0;
	//uint32_t acl_static_index= -1;
	uint32_t first_static = 1;
	uint32_t static_loop = 0;
	uint32_t static_index = 0;
	
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_learning_acl param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;
	if(acl_bucket_tbl == NULL || (FASTFWD_NOT_LOADED == fast_forward_module_load_check()))
	{
		strncpy(se_buf->err_info,FASTFWD_NO_RESPOND_STR,sizeof(FASTFWD_NO_RESPOND_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SEND_DCLI;
	}
	static_index=se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index;
	if(0xFFFFFFFF == static_index)
	{
		for(static_loop = 0; static_loop < acl_static_tbl_size; static_loop++)
		{ 
			s_tbl_rule = (rule_item_t *)acl_bucket_tbl+static_loop;
			if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
			{
				if(NULL == s_tbl_rule->next)
					continue;             
			}
			else
			{
				switch(s_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNING:
						if(first_static==1)
						{
							first_static=0;
							static_index=static_loop;
						}
						total_count++;
						static_count++;
						break;
					default:
						break;
				}
			}
			/* dynamic info */
			d_tbl_rule = s_tbl_rule->next;
			while(d_tbl_rule != NULL)
			{
				
				d_tbl_rule = cvmx_phys_to_ptr((uint64_t)d_tbl_rule);
				switch(d_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNING:
						total_count++;
						break;
					default:
						break;
				}

				d_tbl_rule = d_tbl_rule->next;
			}		
		}
		se_buf->cmd_result=AGENT_RETURN_OK;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index = static_index;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_static_cnt=static_count;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_cnt=total_count;
		goto SEND_DCLI;
	}
	else
	{
		for(static_loop=static_index; static_loop < acl_static_tbl_size; static_loop++)
		{ 
			s_tbl_rule = (rule_item_t *)acl_bucket_tbl+static_loop;
			
			
			if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
			{
				continue;             
			}
			else
			{
				switch(s_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNING:
						read_specified_rule(&(se_buf->fccp_cmd.fccp_data.rule_info.rule_param),s_tbl_rule);
						se_buf->cmd_result=AGENT_RETURN_OK;
						se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index=static_loop+1;
						goto SEND_DCLI;
						break;
					default:
						break;
				}
			}
		}
				
	}
	se_buf->cmd_result=AGENT_RETURN_FAIL;
	strncpy(se_buf->err_info,NO_THIS_RULE_STR,strlen(NO_THIS_RULE_STR));
	goto SEND_DCLI;
SEND_DCLI:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_learning_acl send to dcli failed:%s\n",strerror(errno));
		return ;
	}
	return;
}

void se_agent_clear_aging_rule(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	int ret = -1;
	
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_clear_aging_rule  param error\n");
		return ;
	}
	
	se_buf=(se_interative_t *)buf;

	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_CLEAR_AGING_RULE;

    se_agent_fccp_process(buf, len, 0);

	return ;
}

void se_agent_show_aging_rule_cnt(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	rule_item_t     *rule   = NULL;
	int ret=0;
	uint32_t time_stamp=0;
	uint32_t i = 0;
	uint64_t  static_aging_cnt=0;
	uint64_t  dynamic_aging_cnt=0;
	rule_item_t * s_tbl_rule = NULL;
	rule_item_t * d_tbl_rule = NULL;
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_aging_rule_cnt  param error\n");
			return ;
	}
	se_buf=(se_interative_t *)buf;
	if((FASTFWD_NOT_LOADED == fast_forward_module_load_check())|| (NULL == capwap_cache_tbl) || (NULL == acl_bucket_tbl))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SEND_DCLI;
	}
	time_stamp=se_buf->fccp_cmd.fccp_data.aging_timer;
	for(i = 0; i < acl_static_tbl_size; i++)
	{
		s_tbl_rule = (rule_item_t *)acl_bucket_tbl+i;
		if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(NULL == s_tbl_rule->next)
				continue;             
		}
		else
		{
			if(1 == acl_agingstamp_cmp(&(s_tbl_rule->rules),time_stamp))
				static_aging_cnt++;
		}
		
		/* dynamic info */
		d_tbl_rule = s_tbl_rule->next;
		
		while(d_tbl_rule != NULL)
		{
			d_tbl_rule=(rule_item_t *)cvmx_phys_to_ptr((uint64_t)d_tbl_rule);/*in SE physical address is equal to the virtual address*/
			if(1 == acl_agingstamp_cmp(&(d_tbl_rule->rules),time_stamp))
				dynamic_aging_cnt++;
			d_tbl_rule =d_tbl_rule->next;
		}
	}

	
	se_buf->fccp_cmd.fccp_data.rule_aging_cnt.static_aging_cnt=static_aging_cnt;
	se_buf->fccp_cmd.fccp_data.rule_aging_cnt.dynamic_aging_cnt=dynamic_aging_cnt;

	se_buf->cmd_result=AGENT_RETURN_OK;
SEND_DCLI:
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("send to dcli failed\n");
		return ;
	}
	return;
}



void se_agent_show_user_rule_stats(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	rule_item_t 	*rule	= NULL;
	int ret=0;
	uint32_t user_ip=0;
	uint32_t i = 0;
	rule_item_t * s_tbl_rule = NULL;
	rule_item_t * d_tbl_rule = NULL;

	uint64_t s_tbl_used_rule = 0;
	uint64_t s_tbl_aged_rule = 0;
	uint64_t s_tbl_learned_rule = 0;
	uint64_t s_tbl_learning_rule = 0;
	uint64_t s_tbl_static_rule = 0;
	uint64_t s_tbl_uplink_rule = 0;
	uint64_t s_tbl_downlink_rule = 0;

	uint64_t d_tbl_used_rule = 0;
	uint64_t d_tbl_aged_rule = 0;
	uint64_t d_tbl_learned_rule = 0;
	uint64_t d_tbl_learning_rule = 0;
	uint64_t d_tbl_static_rule = 0;
	uint64_t d_tbl_uplink_rule = 0;
	uint64_t d_tbl_downlink_rule = 0;
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_aging_rule_cnt  param error\n");
			return ;
	}
	se_buf=(se_interative_t *)buf;
	if((FASTFWD_NOT_LOADED == fast_forward_module_load_check())|| (NULL == capwap_cache_tbl) || (NULL == acl_bucket_tbl))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SEND_DCLI;
	}
	user_ip=se_buf->fccp_cmd.fccp_data.rule_tuple.ip_src;
	for(i = 0; i < acl_static_tbl_size; i++)
	{
		s_tbl_rule = (rule_item_t *)acl_bucket_tbl+i;

		if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
		{
			if(NULL == s_tbl_rule->next)
				continue;             
		}
		else
		{
			if((s_tbl_rule->rules.sip == user_ip) || (s_tbl_rule->rules.dip == user_ip))
			{
				if(s_tbl_rule->rules.sip == user_ip)
					s_tbl_uplink_rule++;
				else
					s_tbl_downlink_rule++;

				switch(s_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNED:
						s_tbl_learned_rule++;
						break;
					case RULE_IS_LEARNING:
						s_tbl_learning_rule++;
						break;
					case RULE_IS_STATIC:
						s_tbl_static_rule++;
						break;    
					default:
						break;
				}
				s_tbl_used_rule++;
				if(acl_aging_check(&(s_tbl_rule->rules)) > 0)
				{
					s_tbl_aged_rule++;
				}
			}
		}
		/* dynamic info */
		d_tbl_rule = s_tbl_rule->next;
		while(d_tbl_rule != NULL)
		{
			d_tbl_rule = cvmx_phys_to_ptr((uint64_t)d_tbl_rule);

			if((d_tbl_rule->rules.sip == user_ip) || (d_tbl_rule->rules.dip == user_ip))
			{
				if(d_tbl_rule->rules.sip == user_ip)
					d_tbl_uplink_rule++;
				else
					d_tbl_downlink_rule++;

				switch(d_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNED:
						d_tbl_learned_rule++;
						break;
					case RULE_IS_LEARNING:
						d_tbl_learning_rule++;
						break;
					case RULE_IS_STATIC:
						d_tbl_static_rule++;
						break;    
					default:
						break;
				}
				d_tbl_used_rule++;
				if(acl_aging_check(&(d_tbl_rule->rules)) > 0)
				{
					d_tbl_aged_rule++;
				}
			}

			d_tbl_rule = d_tbl_rule->next;
		}		
	}
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule =s_tbl_used_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule =s_tbl_aged_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_learned_rule =s_tbl_learned_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_learning_rule =s_tbl_learning_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_static_rule =s_tbl_static_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule =d_tbl_used_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule =d_tbl_aged_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_learned_rule =d_tbl_learned_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_learning_rule =d_tbl_learning_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_static_rule =d_tbl_static_rule;
	
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_uplink_rule =s_tbl_uplink_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_uplink_rule =d_tbl_uplink_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.s_tbl_downlink_rule =s_tbl_downlink_rule;
	se_buf->fccp_cmd.fccp_data.rule_sum.d_tbl_downlink_rule =d_tbl_downlink_rule; 
	se_buf->cmd_result=AGENT_RETURN_OK;
SEND_DCLI:
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("send to dcli failed\n");
		return ;
	}
	return;
}

void se_agent_show_tolinux_flow(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	rule_item_t 	*rule	= NULL;
	int ret=0;
	uint64_t wait_time=5;
	uint64_t count_start_time=0;
	uint64_t tolinux_bype_start=0;
	uint64_t tolinux_bype_end=0;
	uint64_t tolinux_packet_start=0;
	uint64_t tolinux_packet_end=0;
	uint64_t tolinux_bps=0;
	uint64_t tolinux_pps=0;
	
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_tolinux_flow  param error\n");
			return ;
	}
	se_buf=(se_interative_t *)buf;
	count_start_time=get_sec();
	tolinux_bype_start=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_BYTES,0);
	tolinux_packet_start=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 0);
	while((get_sec() - count_start_time)< wait_time);
	tolinux_bype_end=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_BYTES,0);
	tolinux_packet_end=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 0);
	if((tolinux_bype_end - tolinux_bype_start)>0)
		tolinux_bps=(tolinux_bype_end - tolinux_bype_start)/wait_time;
	else
		tolinux_bps=0;
	if((tolinux_packet_end - tolinux_packet_start)>0)
		tolinux_pps=(tolinux_packet_end - tolinux_packet_start)/wait_time;
	else 
		tolinux_pps=0;
	se_buf->fccp_cmd.fccp_data.to_linux_flow.to_linux_bps=tolinux_bps;
	se_buf->fccp_cmd.fccp_data.to_linux_flow.to_linux_pps=tolinux_pps;
	
	se_buf->cmd_result=AGENT_RETURN_OK;
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("send to dcli failed\n");
		return ;
	}
	return;
}

/*wangjian clear*/
void se_agent_clear_rule_ip(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
    se_interative_t *se_buf = NULL;
    se_buf=(se_interative_t *)buf;
    se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
    se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
    se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_DEL_RULE_BY_IP;
    se_agent_fccp_process(buf, len, 0);

    return ;
}





/*wangjian 2012.07.09 add ip */
void se_agent_show_rule_ip(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	struct timeval  overtime;
	rule_item_t 	*rule	= NULL;
	int ret=0;
	uint32_t i = 0;
	rule_item_t * s_tbl_rule = NULL;
	rule_item_t * d_tbl_rule = NULL;
	int32_t static_count=0;
	int32_t total_count=0;
	//uint32_t acl_static_index = -1;
	uint32_t first_static = 1;
	uint32_t static_loop=0;
	uint32_t static_index=0;
	
	uint32_t rule_ip=0;
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_rule_ip param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;

	if((acl_bucket_tbl == NULL) || (FASTFWD_NOT_LOADED == fast_forward_module_load_check()))
	{
		strncpy(se_buf->err_info,FASTFWD_NO_RESPOND_STR,sizeof(FASTFWD_NO_RESPOND_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SEND_DCLI;
	}
	rule_ip = se_buf->fccp_cmd.fccp_data.acl_info.acl_param.sip;
	static_index=se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index;
	if(0xFFFFFFFF == static_index)
	{
		for(static_loop = 0; static_loop < acl_static_tbl_size; static_loop++)
		{ 
			s_tbl_rule = (rule_item_t *)acl_bucket_tbl+static_loop;
			if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
			{
				if(NULL == s_tbl_rule->next)
					continue;             
			}
			else
			{   
			    if ((s_tbl_rule->rules.dip != rule_ip) && (s_tbl_rule->rules.sip != rule_ip))
			    {
			        continue;
			    }
				switch(s_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNED:
					case RULE_IS_LEARNING:
						if(first_static==1)
						{
							first_static=0;
							static_index=static_loop;
						}
						total_count++;
						static_count++;
						break;
					default:
						break;
				}
			}
			/* dynamic info */
			d_tbl_rule = s_tbl_rule->next;
			while(d_tbl_rule != NULL)
			{
				
				d_tbl_rule = cvmx_phys_to_ptr((uint64_t)d_tbl_rule);
			   	if ((d_tbl_rule->rules.dip != rule_ip) && (d_tbl_rule->rules.sip != rule_ip))
			    {
			    	d_tbl_rule = d_tbl_rule->next; 
			        continue;
			    }
			    

				switch(d_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNED:
						total_count++;
						break;
					case RULE_IS_LEARNING:
						total_count++;
						break;
					default:
						break;
				}

				d_tbl_rule = d_tbl_rule->next;
			}		
			
		}
		se_buf->cmd_result=AGENT_RETURN_OK;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index = static_index;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_static_cnt=static_count;
		se_buf->fccp_cmd.fccp_data.acl_info.acl_cnt=total_count;
		goto SEND_DCLI;
	}
	else
	{
		for(static_loop=static_index; static_loop < acl_static_tbl_size; static_loop++)
		{ 
			s_tbl_rule = (rule_item_t *)acl_bucket_tbl+static_loop;
			if(s_tbl_rule->rules.rule_state == RULE_IS_EMPTY)
			{
				continue;             
			}
			else
			{
			    if ((s_tbl_rule->rules.dip != rule_ip) && (s_tbl_rule->rules.sip != rule_ip))
			    {
			        continue;
			    }
				switch(s_tbl_rule->rules.rule_state)
				{
					case RULE_IS_LEARNED:
					case RULE_IS_LEARNING:
						read_specified_rule(&(se_buf->fccp_cmd.fccp_data.acl_info.acl_param),s_tbl_rule);
						se_buf->cmd_result=AGENT_RETURN_OK;
						se_buf->fccp_cmd.fccp_data.acl_info.acl_static_index=static_loop+1;
						goto SEND_DCLI;
						break;
					default:
						break;
				}
			}
		}
				
	}
	se_buf->cmd_result=AGENT_RETURN_FAIL;
	strncpy(se_buf->err_info,NO_THIS_RULE_STR,strlen(NO_THIS_RULE_STR));
	goto SEND_DCLI;
SEND_DCLI:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_rule_ip send to dcli failed:%s\n",strerror(errno));
		return ;
	}
	return;
}

