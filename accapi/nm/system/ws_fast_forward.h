/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


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
********************************************************************************
* ws_fast_forward.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/
#ifndef _WS_FAST_FORWARD_H
#define _WS_FAST_FORWARD_H

#include <stdint.h>
#include "se_agent/se_agent_def.h"


#define DCLI_IP_STRING_MAXLEN	(strlen("255.255.255.255"))
#define DCLI_IP_STRING_MINLEN   (strlen("0.0.0.0"))

#define CMD_FAILURE -1
#define CMD_SUCCESS 0
#define READ_ERROR   (-1)
#define WRITE_ERROR  (-1)
#define MIN_PORT   1
#define MAX_PORT   6


#define CVMX_FPA_NUM_POOLS      (8)
#define DCLI_IPPORT_STRING_MAXLEN	(strlen("255.255.255.255:65535"))
#define DCLI_IPPORT_STRING_MINLEN   (strlen("0.0.0.0:0"))

#define IP_FMT(m)	\
				((uint8_t*)&(m))[0], \
				((uint8_t*)&(m))[1], \
				((uint8_t*)&(m))[2], \
				((uint8_t*)&(m))[3]
#define MAC_FMT(m)  \
					((uint8_t*)(m))[0], \
					((uint8_t*)(m))[1], \
					((uint8_t*)(m))[2], \
					((uint8_t*)(m))[3], \
					((uint8_t*)(m))[4], \
					((uint8_t*)(m))[5]


				
#define PROTO_STR(t)  ((t) == 0x6 ? "TCP" : ((t) == 0x11 ? "UDP" : "Unknown"))


typedef enum
{
    CVMX_POW_TAG_TYPE_ORDERED   = 0L,   /**< Tag ordering is maintained */
    CVMX_POW_TAG_TYPE_ATOMIC    = 1L,   /**< Tag ordering is maintained, and at most one PP has the tag */
    CVMX_POW_TAG_TYPE_NULL      = 2L,   /**< The work queue entry from the order
                                            - NEVER tag switch from NULL to NULL */
    CVMX_POW_TAG_TYPE_NULL_NULL = 3L    /**< A tag switch to NULL, and there is no space reserved in POW
                                            - NEVER tag switch to NULL_NULL
                                            - NEVER tag switch from NULL_NULL
                                            - NULL_NULL is entered at the beginning of time and on a deschedule.
                                            - NULL_NULL can be exited by a new work request. A NULL_SWITCH load can also switch the state to NULL */
} cvmx_pow_tag_type_t;

struct to_linux_flow_r
{
	uint64_t to_linux_bps;
	uint64_t to_linux_pps;
};
struct fast_fwd_info{
	uint32_t fast_fwd_enable;	/*fast forward enable wj*/
	uint32_t fast_fwd_coremask; /*fast forward occupy core */
};


extern int ccgi_config_fast_forward_enable_cmd(char *state,int slot_id);
														//-1:Failed:communication failure with agent module;-2:Failed:agent is not responding;-3:cmd_data.cmd_result!=AGENT_RETURN_OK
extern int ccgi_show_fast_forward_info_cmd(int slot_id,struct fast_fwd_info *info);
														//0:success;	-1:socket failed;-2:Failed:communication failure with agent module;
														//-3:Failed:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;-5:Failed:command failed
extern int ccgi_set_fastfwd_bucket_entry_cmd(char *value,int slot_id);
														//0:success;	-1:Failed:agent is not responding;-2:cmd_data.cmd_result!=AGENT_RETURN_OK;
														//-3:cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK;-4:ccgi_se_agent_init failed;
extern int ccgi_show_fastfwd_bucket_entry_cmd(int slot_id, unsigned int *max_entry);
														//-1:Failed:communication failure with agent module;-2:Failed:agent is not responding;
														//-3:cmd_data.cmd_result!=AGENT_RETURN_OK;-4:cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK;
														//-5:ccgi_se_agent_init failed;0:success
extern int ccgi_config_fast_forward_tag_type_cmd(char *state,int slot_id,int flag);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
														//-3:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;

extern int ccgi_show_fast_forward_tag_type_cmd(int slot_id,int flag,char *info);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
														//-3:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;-5:read tag type error
extern int ccgi_clear_aging_rule_cmd(int slot_id);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
extern int ccgi_clear_rule_all_cmd(int slot_id,int flag);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;-3:agent is not responding;
														//-4:cmd_data.cmd_result!=AGENT_RETURN_OK;-5:cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_Ok
extern int ccgi_clear_rule_ip_cmd(char *ip,int slot_id);
														//-1:ip address format error;-2:ccgi_se_agent_init failed;-3:communication failure with agent module
extern int ccgi_fastfwd_learned_icmp_enable_cmd(char *state,int slot_id);
														//0:success;-1:ccgi_se_agent_init failed;-2:command parameter format error
														//-3:communication failure with agent module;-4:agent is not responding;
														//-5:cmd_data.cmd_result!=AGENT_RETURN_OK;
extern int ccgi_fastfwd_pure_ip_enable_cmd(int slot_id,char *state);
														//0:success;-1:ccgi_se_agent_init failed;-2:command parameter format error
														//-3:communication failure with agent module;-4:agent is not responding;
														//-5:cmd_data.cmd_result!=AGENT_RETURN_OK;
extern int ccgi_show_fwd_pure_ip_enable_cmd(int slot_id, int *state);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;-3:agent is not responding;
														//--4:cmd_data.cmd_result!=AGENT_RETURN_OK;-5:cmd_data.fccp_cmd.ret_val != FCCP_RETURN_OK
extern int ccgi_show_aging_rule_cnt_cmd(char *value,int slot_id,unsigned long long *dynamic_a,unsigned long long *static_ag);
														//0:success;-1:ccgi_se_agent_init failed;-2:command parameter format error;-3:communication failure with agent module;
														//-4:agent is not responding;-5:cmd_data.cmd_result!=AGENT_RETURN_OK;
extern int ccgi_show_rule_five_tuple_cmd(char *pro,char *sport,char *dport,int slot_id,se_interative_t  *info);
														//0:success;-1:ccgi_se_agent_init failed;-2:protocol is not tcp or udp;-3:source ip address and port format error;
														//-4:destination ip address and port format error;-5:communication failure with agent module;
														//-6:agent is not responding;-7:cmd_data.cmd_result!=AGENT_RETURN_OK;
extern int ccgi_show_rule_stats_cmd(int slot_id,int flag,rule_stats_t *statistics);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;-3:agent is not responding;
														//-4:cmd_data.cmd_result!=AGENT_RETURN_OK;
extern int ccgi_show_tolinux_flow_cmd(int slot_id, struct to_linux_flow_r *data);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;-3:agent is not responding;
														//-4:cmd_data.cmd_result!=AGENT_RETURN_OK;
extern int ccgi_show_user_acl_stats_cmd(char *ip,int slot_id,rule_stats_t *statistics);
														//0:success;-1:ccgi_se_agent_init failed;-2:command parameter format error;-3:communication failure with agent module;
														//-4:agent is not responding;-5:cmd_data.cmd_result!=AGENT_RETURN_OK;
extern int ccgi_show_fpa_buff_counter_cmd(int slot_id, int flag,unsigned int *p);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
														//-3:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;
extern int ccgi_show_fast_forward_running_config_cmd(int slot_id,int *state);
														//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
														//-3:agent is not responding;





#endif


