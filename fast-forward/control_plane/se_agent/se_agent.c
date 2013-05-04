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
 * se_agent.c
 *
 *
 * CREATOR:
 *		sunjc@autelan.com
 *
 * DESCRIPTION:
 *		
 *
 * DATE:
 *		08/12/2011
 *
 *  FILE REVISION NUMBER:
 *  		$Revision:  $	
 *******************************************************************************/
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
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/time.h>

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
#include "se_agent_octnic.h"


#include "se_agent.h"

#include <time.h>
enum
{
	SYSTEM_INITIAL,
	SYSTEM_DSICOVER,
	SYSTEM_READY,
	SYSTEM_IDLE,
	SYSTEM_RUNNING
};


/*buf store date which receive from dcli */
#define BUF_LEN 	                 1024


#define ETH_H_LEN               14
#define FCCP_PACKET_LEN         ETH_H_LEN + sizeof(control_cmd_t)
#define MAC_LEN                 6

/*fast-fwd module aging time*/
CVMX_SHARED  unsigned int aging_time = DEFAULT_AGENT_TIME;
CVMX_SHARED cvmx_pow_tag_type_t tag_type = -1;

/*invalid socket fd*/
#define INVALID_FD   (-1)
/*se_agent socket */
CVMX_SHARED int se_socket = INVALID_FD;

#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)

CVMX_SHARED rule_item_t  *acl_bucket_tbl = NULL;
CVMX_SHARED uint32_t acl_static_tbl_size = 0;

CVMX_SHARED capwap_cache_t *capwap_cache_tbl = NULL;
CVMX_SHARED uint32_t capwap_cache_tbl_size = 0;

CVMX_SHARED rule_item_t  *acl_dynamic_tbl = NULL;
CVMX_SHARED uint32_t acl_dynamic_tbl_size = 0;

CVMX_SHARED user_item_t*  user_bucket_tbl = NULL;
CVMX_SHARED uint32_t user_static_tbl_size = 0;

CVMX_SHARED user_item_t*  user_dynamic_tbl = NULL;
CVMX_SHARED uint32_t user_dynamic_tbl_size = 0;
CVMX_SHARED func_item_t *se_agent_cmd_func_tbl = NULL;
#define SLOTID_PATH         "/dbm/local_board/slot_id"

#define MAX_SLOTID_LEN 	10
CVMX_SHARED uint32_t taffic_monitor_func = FUNC_DISABLE;
CVMX_SHARED uint64_t  traffic_minitor_old_time  = 0;
CVMX_SHARED traffic_stats_t old_traffic_stats;
CVMX_SHARED char* ipfwd_learn_name = NULL;
CVMX_SHARED int32_t agent_pcie_channel_exist = AGENT_FALSE;
CVMX_SHARED pthread_t ntid; /* receive fccp from  grp14 thread */

#define MAX_HEARTBEAT_FAIL_TIMES   3
CVMX_SHARED uint32_t heartbeat_fail_counter = 0;
CVMX_SHARED pthread_t heartbeat_tid; /* heartbeat thread */

//clear aged rules in adjustable time,default is 600s.If 0,stop this function
CVMX_SHARED uint32_t clr_aging_interval = 600;

#define AVAILABLE_PORT  0
#define INVALID_PORT    -1
int check_pip_offset(unsigned long offset)
{
	if (!(
        (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 3) || ((offset >= 16) && (offset <= 19)) || ((offset >= 32) && (offset <= 35)) || ((offset >= 36) && (offset <= 39)))) ||
        (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 2) || ((offset >= 32) && (offset <= 33)))) ||
        (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 2) || ((offset >= 32) && (offset <= 33)))) ||
        (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 35))) ||
        (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 2) || ((offset >= 32) && (offset <= 33)))) ||
        (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 35))) ||
        (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3) || ((offset >= 32) && (offset <= 35)) || ((offset >= 36) && (offset <= 39))))))
        return INVALID_PORT;
	else
		return AVAILABLE_PORT;
}

#define AGENT_MSG_MAX_LEN 512
struct msg_buf
{ 
    int mtype; 
    char data[AGENT_MSG_MAX_LEN];
};

CVMX_SHARED int se_agent_msgid=-1;

int se_agent_init_msg()
{
    key_t key; 

    int ret; 
    struct msg_buf msgbuf;
    key = ftok("temp/1",'a'); 

    se_agent_msgid = msgget(key,IPC_CREAT|IPC_EXCL|0666); 
    if(se_agent_msgid ==-1) 
    {  
        se_agent_msgid = msgget(key,IPC_PRIVATE); 
    } 
    if(se_agent_msgid ==-1) 
    {  
        se_agent_syslog_err("create msg\n");  
        return SE_AGENT_RETURN_FAIL; 
    } 

    return SE_AGENT_RETURN_OK;
}

int se_agent_send_msg(char* data, int len)
{
    int ret;
    struct msg_buf msgbuf;
    msgbuf.mtype = getpid(); 
    memcpy(msgbuf.data, data, len);

    ret = msgsnd(se_agent_msgid,&msgbuf,sizeof(msgbuf.data),IPC_NOWAIT); 
    if( ret==-1 ) 
    {  
        se_agent_syslog_err("send message error\n");  
        return SE_AGENT_RETURN_FAIL; 
    } 
    return SE_AGENT_RETURN_OK;
}

int se_agent_recv_msg(char *buf,unsigned int len,struct timeval *overtime)
{
    int32_t ret = 0;
    struct msg_buf msgbuf;
    uint32_t i = 0;
    uint32_t wait_time = 0;

    if((NULL == buf) || (NULL == overtime))
    {
        return SE_AGENT_RETURN_FAIL;
    }

    wait_time = 0xffff * overtime->tv_sec;
    memset(&msgbuf,0,sizeof(msgbuf)); 

    for(i = 0; i < wait_time; i++)
    {
        ret = msgrcv(se_agent_msgid,&msgbuf,sizeof(msgbuf.data),0,IPC_NOWAIT); 
        if(ret != -1) 
        {    
            break;
        }

        usleep(1);
    }

    if(ret == -1)
    {
        return SE_AGENT_RETURN_FAIL;
    }
    memcpy(buf, msgbuf.data, len);

    return SE_AGENT_RETURN_OK;
}

void se_agent_clear_msg()
{
    int ret;
    struct msg_buf msgbuf;
    int i = 0;
    
    memset(&msgbuf,0,sizeof(msgbuf)); 

    while(1)
    {
        ret = msgrcv(se_agent_msgid,&msgbuf,sizeof(msgbuf.data),0,IPC_NOWAIT); 
        if(ret == -1) 
        {    
            break;
        }

        usleep(0);
    }

    return;
}

/**********************************************************************************
 *  se_agent_create_socket
 *
 *	DESCRIPTION:
 * 		se_agent module create tipc server socket
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		other                                  -> socket fd
 *		SE_AGENT_RETURN_FAIL      -> fail  
 *		
 **********************************************************************************/
int se_agent_create_socket(void)
{
    struct sockaddr_tipc server_addr;
    int slot_id = 0;
    char buf[MAX_SLOTID_LEN] = {0};
    FILE *fp = NULL;

    fp = fopen(SLOTID_PATH, "r");
    if(NULL == fp)
    {
        se_agent_syslog_err("/dbm/local_board/slot_id open failed!\n");
        return SE_AGENT_RETURN_FAIL;
    }
    if(NULL == fgets(buf, MAX_SLOTID_LEN, fp))
    {
        se_agent_syslog_err("read slot_id failed!\n");
		fclose(fp);
        return SE_AGENT_RETURN_FAIL;
    }
	fclose(fp);
    slot_id = atoi(buf);
    if(slot_id > MAX_SLOT_NUM)
    {
        se_agent_syslog_err("slot_id is larger than the max number of slot\n");
        return SE_AGENT_RETURN_FAIL;
    }
    se_agent_syslog_dbg("slotid is %d\n", slot_id);

	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAME;
	server_addr.addr.name.name.type = SE_AGENT_SERVER_TYPE;
	server_addr.addr.name.name.instance = SE_AGENT_SERVER_LOWER_INST+slot_id;
	server_addr.addr.name.domain = 0;
	server_addr.scope = TIPC_CLUSTER_SCOPE;

	se_socket = socket(AF_TIPC, SOCK_DGRAM, 0);
	
	if (bind(se_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) 
	{
		se_agent_syslog_err("bind socket failed\n");
		close(se_socket);
		return SE_AGENT_RETURN_FAIL;
	}

	return se_socket;
}





/**********************************************************************************
 *  send_fccp
 *
 *	DESCRIPTION:
 * 		send fccp command to fast-fwd module
 *
 *	INPUT:
 *		cmd                ->     pointer to fccp control packet data 
 *		qos_data         ->     packet work pos
 *		
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *          SE_AGENT_RETURN_FAIL	   -> failed
 *          SE_AGENT_RETURN_OK        ->success
 **********************************************************************************/
int send_fccp(control_cmd_t  *fccp_cmd,unsigned char qos_data, unsigned char grp)
{
	if(NULL==fccp_cmd)
	{
		se_agent_syslog_err("send_fccp param error\n");
		return SE_AGENT_RETURN_FAIL;
	}
	void *pkt_buffer = NULL;
	void *pkt_ptr = NULL;
	void *tmp_ptr = NULL;
	
	cvmx_wqe_t *work = cvmx_fpa_alloc(CVMX_FPA_WQE_POOL);
	if (NULL == work)    
	{		
		se_agent_syslog_err("send fccp fpa alloc work fail\n");
		return SE_AGENT_RETURN_FAIL;    
	}

	pkt_buffer = cvmx_fpa_alloc(CVMX_FPA_PACKET_POOL);
	if (NULL == pkt_buffer)    
	{		
		se_agent_syslog_err( "send fccp fpa alloc packet buffer fail\n");
		cvmx_fpa_free(work,CVMX_FPA_WQE_POOL,0);
		return SE_AGENT_RETURN_FAIL;    
	}
	memset(pkt_buffer, 0, CVMX_FPA_PACKET_POOL_SIZE); 	
	pkt_ptr = pkt_buffer + sizeof(uint64_t);
	pkt_ptr += ((CVMX_HELPER_FIRST_MBUFF_SKIP+7)&0xfff8) + 6;

	tmp_ptr = pkt_ptr;
	tmp_ptr += MAC_LEN*2 ;
	*((uint16_t *)tmp_ptr) = FCCP_L2_ETH_TYPE;
	tmp_ptr += sizeof(uint16_t);
	memcpy(tmp_ptr, fccp_cmd, sizeof(control_cmd_t)); 

	CVM_WQE_SET_LEN(work,FCCP_PACKET_LEN);
	CVM_WQE_SET_PORT(work,0);
	if(qos_data > 7)
	{
        CVM_WQE_SET_QOS(work,0);
	}
	else
	{
	    CVM_WQE_SET_QOS(work,(qos_data));
	}
	if(grp > 15)
	{
	    CVM_WQE_SET_GRP(work, FCCP_SEND_GROUP);
	}
	else
	{
        CVM_WQE_SET_GRP(work, grp);
	}
	CVM_WQE_SET_TAG_TYPE(work, CVMX_POW_TAG_TYPE_NULL);
	CVM_WQE_SET_TAG(work, FCCP_DEFAULT_TAG);
	
	work->word2.u64     = 0;    /* Default to zero. Sets of zero later are commented out */
	work->word2.s.bufs  = 1;
	work->packet_ptr.u64 = 0;
	work->packet_ptr.s.addr = cvmx_ptr_to_phys(pkt_ptr);

	work->packet_ptr.s.pool = CVMX_FPA_PACKET_POOL;
	work->packet_ptr.s.size = CVMX_FPA_PACKET_POOL_SIZE;
	work->packet_ptr.s.back = (pkt_ptr - pkt_buffer)>>7; 

	work->word2.snoip.not_IP        = 1; /* IP was done up above */
	CVM_WQE_SET_UNUSED(work, FCCP_WQE_TYPE);
	//work->unused = FCCP_WQE_TYPE;
	
	/* Submit the packet to the POW */
	cvmx_pow_work_submit(work, CVM_WQE_GET_TAG(work),\
	                        CVM_WQE_GET_TAG_TYPE(work), \
	                        CVM_WQE_GET_QOS(work), \
	                        CVM_WQE_GET_GRP(work));
                      
	return SE_AGENT_RETURN_OK;
}


/**********************************************************************************
 *  fast_forward_module_load_check
 *
 *	DESCRIPTION:
 * 		check if fastfwd module exist
 *
 *	INPUT:
 *		
 *	OUTPUT:
 *
 * 	RETURN:
 *          FASTFWD_LOADED
 *          FASTFWD_NOT_LOADED
 **********************************************************************************/
int fast_forward_module_load_check()
{
	uint64_t fastforward_load;
	fastforward_load = cvmx_fau_fetch_and_add64(CVM_FAU_SE_COEXIST_FLAG,0);
	if(SE_MAGIC_NUM == fastforward_load)
		return FASTFWD_LOADED;
	else
		return FASTFWD_NOT_LOADED;
}

/**********************************************************************************
 *  se_agent_fccp_process
 *
 *	DESCRIPTION:
 * 		send fccp to fastfwd, and recv response if need_response is required
 *
 *	INPUT:
 *		buf         ->     fccp buf
 *		len         ->     fccp len
 *		need_response -> 1: need response   0: no response
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *          SE_AGENT_RETURN_FAIL	   -> failed
 *          SE_AGENT_RETURN_OK        ->success
 **********************************************************************************/
int se_agent_fccp_process(char *buf, unsigned int len, char need_response)
{
    se_interative_t *se_buf = NULL;
	struct timeval  overtime;
	
	if((NULL == buf) || (0 == len))
		return SE_AGENT_RETURN_FAIL;
		
	se_buf=(se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		return SE_AGENT_RETURN_FAIL;
	}

	se_buf->fccp_cmd.cmd_len = sizeof(control_cmd_t);
	se_buf->fccp_cmd.agent_pid = getpid();
	se_agent_clear_msg();
	if(SE_AGENT_RETURN_FAIL == (send_fccp(&(se_buf->fccp_cmd),0, FCCP_SEND_GROUP)))
	{
		strncpy((se_buf->err_info),SEND_FCCP_FAIL_STR,strlen(SEND_FCCP_FAIL_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		return SE_AGENT_RETURN_FAIL;
	}

	if(need_response)
	{
        overtime.tv_sec=AGENT_WAIT_TIME;
        if(se_agent_recv_msg((char*)(&(se_buf->fccp_cmd)),sizeof(control_cmd_t),&overtime) < 0)
        {
        	strncpy((se_buf->err_info),FASTFWD_NO_RESPOND_STR,strlen(FASTFWD_NO_RESPOND_STR));
        	se_buf->cmd_result = AGENT_RETURN_FAIL;
        	return SE_AGENT_RETURN_FAIL;
        }
        se_buf->cmd_result = AGENT_RETURN_OK;
	}
	
	return SE_AGENT_RETURN_OK;    
}


/**********************************************************************************
 *  se_agent_fccp_process_pcie
 *
 *	DESCRIPTION:
 * 		send fccp to slave cpu fastfwd from pcie channel, and recv response
 *
 *	INPUT:
 *		buf         ->     fccp buf
 *		len         ->     fccp len
 *		need_response -> 1: need response   0: no response
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *          SE_AGENT_RETURN_FAIL	   -> failed
 *          SE_AGENT_RETURN_OK        ->success
 **********************************************************************************/
int se_agent_fccp_process_pcie(char *buf, unsigned int len, char need_response)
{
    se_interative_t *se_buf = NULL;
    uint8_t* pkt_buf = NULL;
    uint8_t* tmp_ptr = NULL;
	int ret = 0;
	
	if((NULL == buf) || (0 == len))
		return SE_AGENT_RETURN_FAIL;

	se_buf=(se_interative_t *)buf;
	se_buf->fccp_cmd.cmd_len = sizeof(control_cmd_t);
	se_buf->fccp_cmd.agent_pid = getpid();
	
    if(agent_pcie_channel_exist != AGENT_TRUE)
    {
        se_agent_syslog_err("se_agent_fccp_process_pcie pcie channel none exist\n");
        strncpy((se_buf->err_info),"pcie channel none exist",strlen("pcie channel none exist"));
        return SE_AGENT_RETURN_FAIL;
    }

    pkt_buf = (uint8_t*)malloc(FCCP_PACKET_LEN);
    if(pkt_buf == NULL)
    {
        se_agent_syslog_err("se_agent_fccp_process_pcie malloc fail.no memory\n");
        strncpy((se_buf->err_info),"pcie malloc fail.no memory",strlen("pcie malloc fail.no memory"));
        se_buf->cmd_result = AGENT_RETURN_FAIL;
        return SE_AGENT_RETURN_FAIL;         
    }
    
    memset(pkt_buf, 0, FCCP_PACKET_LEN);
    tmp_ptr = pkt_buf + MAC_LEN*2;
    *((uint16_t *)tmp_ptr) = FCCP_L2_ETH_TYPE;
    tmp_ptr += sizeof(uint16_t);
    memcpy(tmp_ptr, &se_buf->fccp_cmd, sizeof(control_cmd_t));    
    ret = se_agent_send_fccp_from_pci(pkt_buf, FCCP_PACKET_LEN, pkt_buf, FCCP_PACKET_LEN);    
    if(ret != SE_AGENT_RETURN_OK)
    {
        se_agent_syslog_err("se_agent_send_fccp_from_pci return FAIL\n");
        strncpy((se_buf->err_info),"send_fccp_from_pci return FAIL",strlen("send_fccp_from_pci return FAIL"));
        se_buf->cmd_result = AGENT_RETURN_FAIL;
        free(pkt_buf);
        return SE_AGENT_RETURN_FAIL; 
    }
    
    memcpy(&se_buf->fccp_cmd, tmp_ptr, sizeof(control_cmd_t)); 
    se_buf->cmd_result = AGENT_RETURN_OK;
    free(pkt_buf);
	return SE_AGENT_RETURN_OK;    
}


/**********************************************************************************
 *  se_agent_uart_switch
 *
 *	DESCRIPTION:
 * 		switch uart to se module
 *
 *	INPUT:
 *		uart_index -> uart index
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		NULL
 **********************************************************************************/
void se_agent_uart_switch(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	int i = 0;
	se_interative_t *se_buf=NULL;
	int ret = -1;
	int uart_index = 0;
	cvmx_ciu_intx0_t irq_control;
	cvmx_ciu_fuse_t fuse;
	uint32_t cpu = 0;
	unsigned int fuse_tmp = 0;
	uint32_t se_first_core = 0;
	uint32_t se_mask = 0;
	fuse.u64 = cvmx_read_csr(CVMX_CIU_FUSE);
	fuse_tmp = fuse.s.fuse;
	se_buf=(se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;
	}
	
	se_mask = (uint32_t)cvmx_fau_fetch_and_add64(CVM_FAU_SE_CORE_MASK, 0);
	se_agent_syslog_dbg("se mask : 0x%x\n", se_mask);

	if(se_mask == 0)
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;
	}

	while(se_mask)
	{
		if((se_mask & 0x1) != 0)
		{
			break;
		}
		se_first_core++;
		se_mask = se_mask >> 1;
	}
	se_agent_syslog_dbg("se first core : %d\n", se_first_core);

	while(fuse_tmp)
	{
		if((fuse_tmp & 0x1) != 0)
		{
			irq_control.u64 = cvmx_read_csr(CVMX_CIU_INTX_EN0(cpu*2));
			irq_control.s.uart=0;
			if (cpu == se_first_core)
			{
				irq_control.s.uart=1<<uart_index;
			}
			cvmx_write_csr(CVMX_CIU_INTX_EN0(cpu*2), irq_control.u64);
		}
		cpu++;
		fuse_tmp = fuse_tmp>>1;
	}
	se_buf->cmd_result = AGENT_RETURN_OK;
SENDTO_DCLI:
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_config_aging_time send to dcli failed\n");
		return ;
	}
	return ;

}

/**********************************************************************************
 *  se_agent_config_aging_time
 *
 *	DESCRIPTION:
 * 		Set  fast-fwd  module  aging time
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
int se_agent_config_aging_time(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	unsigned int tmp_time;
    int ret = 0;
    
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_config_aging_time param error\n");
		return SE_AGENT_RETURN_FAIL;
	}    
	
	se_buf=(se_interative_t *)buf;
	tmp_time=se_buf->fccp_cmd.fccp_data.aging_timer;
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_SET_AGEING_TIMER;
	
    switch(se_buf->cpu_tag)
    {
        case 0:
            ret = se_agent_fccp_process(buf, len, 1);
            break;
        case 1:
            ret = se_agent_fccp_process_pcie(buf, len, 1);
            break;
        default:
            se_agent_syslog_err("se_agent_config_aging_time cpu_tag invalid\n");
		    return SE_AGENT_RETURN_FAIL;
    }
    
	if(ret == SE_AGENT_RETURN_OK)
	{
		aging_time=tmp_time;
	}

	if(sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len) < 0)
	{
		se_agent_syslog_err("se_agent_show_aging_time send to dcli failed\n");
		return SE_AGENT_RETURN_FAIL;
	}

    return SE_AGENT_RETURN_OK;
}

/**********************************************************************************
 *  se_agent_show_aging_time
 *
 *	DESCRIPTION:
 * 		Show   fast-fwd  module  aging time
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
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail
 *		  
 **********************************************************************************/
int se_agent_show_aging_time(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;

	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_aging_time param error\n");
		return SE_AGENT_RETURN_FAIL;
	}  

	se_buf=(se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_GET_AGEING_TIMER;

    switch(se_buf->cpu_tag)
    {
        case 0:
            se_agent_fccp_process(buf, len, 1);
            break;
        case 1:
            se_agent_fccp_process_pcie(buf, len, 1);
            break;
        default:
            se_agent_syslog_err("se_agent_show_aging_time cpu_tag invalid\n");
		    return SE_AGENT_RETURN_FAIL;
    }

    if(sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len) < 0)
	{
		se_agent_syslog_err("se_agent_show_aging_time send to dcli failed\n");
		return SE_AGENT_RETURN_FAIL;
	}

	return SE_AGENT_RETURN_OK;
}


/**********************************************************************************
 *  se_agent_show_aging_time_sc
 *
 *	DESCRIPTION:
 * 		Show   fast-fwd  module  aging time
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
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail
 *		  
 **********************************************************************************/
int se_agent_show_aging_time_sc(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
    int ret = 0;
    
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_config_aging_time param error\n");
		return SE_AGENT_RETURN_FAIL;
	}    
    
	se_buf=(se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_GET_AGEING_TIMER;
	se_agent_fccp_process_pcie(buf, len, 1);
 
    if(sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len) < 0)
	{
		se_agent_syslog_err("se_agent_show_aging_time send to dcli failed\n");
		return SE_AGENT_RETURN_FAIL;
	}

    return SE_AGENT_RETURN_OK;
}



/**********************************************************************************
 *  se_agent_config_tag_type
 *
 *	DESCRIPTION:
 * 		Set  fast-fwd  module  tag type
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
 *		SE_AGENT_RETURN_OK          ->     success
 *		SE_AGENT_RETURN_FAIL        ->     fail  
 **********************************************************************************/
void se_agent_config_tag_type(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)                         
{
	int port_index = 0;
	int ret = -1;
	se_interative_t *se_buf=NULL;
	cvmx_pip_port_tag_cfg_t tag_config;
	cvmx_pow_tag_type_t  tmp_type;
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_config_tag_type  param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;

	if(se_buf->cpu_tag == 1)
    {
        se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	    se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	    se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_CONFIG_TAG_TYPE;

        se_agent_fccp_process_pcie(buf, len, 1);
        if(se_buf->fccp_cmd.ret_val == FCCP_RETURN_ERROR)
    	{
    	    strncpy(se_buf->err_info,READ_RULE_ERROR,sizeof(READ_RULE_ERROR));
		    se_buf->cmd_result = AGENT_RETURN_FAIL;
    	}
        goto SENDTO_DCLI;
    }
    
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;
	}
	tmp_type=se_buf->fccp_cmd.fccp_data.tag_type;	
	for (port_index = 0; port_index < CVMX_PIP_NUM_INPUT_PORTS; port_index++)
	{
		if(AVAILABLE_PORT != check_pip_offset(port_index))
		{
			continue;
		}
		tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));
		tag_config.s.tcp6_tag_type = tmp_type; 
		tag_config.s.tcp4_tag_type = tmp_type;
		tag_config.s.ip6_tag_type = tmp_type;
		tag_config.s.ip4_tag_type = tmp_type;
		tag_config.s.non_tag_type = tmp_type;
		cvmx_write_csr(CVMX_PIP_PRT_TAGX(port_index), tag_config.u64);
	}
	tag_type = tmp_type;
	se_buf->cmd_result=AGENT_RETURN_OK;
	
SENDTO_DCLI:
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("send to dcli failed\n");
		return ;
	}
}



/**********************************************************************************
 *  se_agent_show_tag_type
 *
 *	DESCRIPTION:
 * 		Show  fast-fwd  module  tag type
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
 *		NULL
 **********************************************************************************/
void se_agent_show_tag_type(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)                         
{
	int port_index = 0;
	int ret = -1;
	se_interative_t *se_buf=NULL;
	cvmx_pip_port_tag_cfg_t tag_config;
	cvmx_pow_tag_type_t tag_type;
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_show_tag_type  param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;

	if(se_buf->cpu_tag == 1)
    {
        se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	    se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	    se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_GET_TAG_TYPE;

        se_agent_fccp_process_pcie(buf, len, 1);
        if(se_buf->fccp_cmd.ret_val == FCCP_RETURN_ERROR)
    	{
    	    strncpy(se_buf->err_info,READ_RULE_ERROR,sizeof(READ_RULE_ERROR));
		    se_buf->cmd_result = AGENT_RETURN_FAIL;
    	}
        goto SENDTO_DCLI;
    }
    
	while(AVAILABLE_PORT != check_pip_offset(port_index))
	{
		port_index++;
		if(port_index > CVMX_PIP_NUM_INPUT_PORTS)
		{
			se_buf->cmd_result = AGENT_RETURN_FAIL;
			strcpy(se_buf->err_info,"read tag type failed\n");
			goto SENDTO_DCLI;
		}	
	}
	tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));
	tag_type = tag_config.s.ip4_tag_type;
	se_buf->fccp_cmd.fccp_data.tag_type=tag_type;
	se_buf->cmd_result = AGENT_RETURN_OK;
	
SENDTO_DCLI:
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
			se_agent_syslog_err("se_agent_show_tag_type send to dcli failed\n");
			return ;
	}
}



/**********************************************************************************
 *  se_agent_save_cfg
 *
 *	DESCRIPTION:
 * 		Save the configuration of the se_agent module
 *
 *	INPUT:
 *		buf			 ->       pointer of store data buffer
 *		len 			 ->       the size of buffer
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		NULL		
 **********************************************************************************/
void se_agent_save_cfg(unsigned char *buf,unsigned int bufLen)
{
	unsigned int ret = -1 ;
	unsigned int length = 0;
	unsigned char status ;
	char *showStr = NULL;
	char *current = NULL;
	FILE *file_icmp = NULL;
	FILE *file_fastfwd = NULL;
	char file_path[100] = {0};
	unsigned char stricmp[64] = {0};
	unsigned char strfastfwd[64] = {0};
	int icmp_enable = FUNC_DISABLE;
	int fastfwd_enable = FUNC_ENABLE;

	if (NULL == buf) 
	{
		se_agent_syslog_err("se_agent_save_cfg parameter is null error\n");
		return ;
	}
	if(NULL == ipfwd_learn_name)
	{
		se_agent_syslog_err("se_agent_save_cfg not find ipfwd_learn module\n");
		return ;
	}
	
	showStr = (char *)buf;
	current = showStr;
	if ((length + 30) < bufLen) 
	{
		length += sprintf(current, "config fast-forward\n");
		current = showStr + length;
	}
	
	sprintf(file_path,"/sys/module/%s/parameters/icmp_enable",ipfwd_learn_name);
	file_icmp = fopen(file_path,"r");
	if(file_icmp !=NULL)
	{
		ret=fread(stricmp,sizeof(int),sizeof(stricmp),file_icmp);
		fclose(file_icmp);
		icmp_enable=atoi((char*)stricmp);
	}
	if((length +30)<bufLen)
	{
		if(icmp_enable==1)
		{
			length +=sprintf(current," config fast-icmp enable\n");
			current =showStr + length;
		}
	}

	memset(file_path, 0, sizeof(file_path));
	sprintf(file_path,"/sys/module/%s/parameters/learn_enable",ipfwd_learn_name);
	file_fastfwd=fopen(file_path,"r");
	if(file_fastfwd !=NULL)
	{
		ret=fread(strfastfwd,sizeof(int),sizeof(strfastfwd),file_fastfwd);
		fclose(file_fastfwd);
		fastfwd_enable=atoi((char*)strfastfwd);
	}
	#if 0
	if((length +30)<bufLen)
	{
		if(fastfwd_enable==0)
		{
			length +=sprintf(current,"config fastfwd disable\n");
			current =showStr + length;
		}
	}
	#endif
	if((aging_time > 0) && (aging_time != DEFAULT_AGENT_TIME))
	{
		if ((length + 30) < bufLen) 
		{
			length += sprintf(current, " config aging-time %d\n",aging_time);
			current = showStr + length;
		}
	}
	if((length +30 ) <bufLen)
	{
		switch (tag_type)
		{
			case CVMX_POW_TAG_TYPE_ORDERED :
				length += sprintf(current, " config tag type %s\n","ordered");
				current = showStr + length;
				break;
			case CVMX_POW_TAG_TYPE_ATOMIC :
				length += sprintf(current, " config tag type %s\n","atomic");
				current = showStr + length;
				break;
			case CVMX_POW_TAG_TYPE_NULL :
				length += sprintf(current, " config tag type %s\n","null");
				current = showStr + length;
				break;
			default:
				break;
		}
	}
	if ((length + 30) < bufLen) 
	{
		length += sprintf(current, " exit\n");
		current = showStr + length;
	}
} 


/**********************************************************************************
 *  se_agent_show_running_config
 *
 *	DESCRIPTION:
 * 		se_agent_show_running_config
 *
 *	INPUT:
 *          client_addr     ->        client socket address
 *		len 			  ->       the length of client socket address
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *				
 **********************************************************************************/
void se_agent_show_running_config(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	unsigned char *showStr = NULL;
	unsigned char en_dis = 0;
	int ret=-1;

	showStr = (unsigned char*)malloc(SE_AGENT_RUNNING_CFG_MEM);
	if (NULL == showStr) 
	{
		se_agent_syslog_err("se_agent_show_running_config memory malloc error\n");
		return ;
	}
	memset(showStr, 0, SE_AGENT_RUNNING_CFG_MEM);

	se_agent_save_cfg(showStr, SE_AGENT_RUNNING_CFG_MEM);

	ret=sendto(se_socket,showStr, SE_AGENT_RUNNING_CFG_MEM,0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("show_running_config send to dcli failed:%s\n",strerror(errno));
		return ;
	}	
	free(showStr);
	showStr = NULL;
}



/**********************************************************************************
 *  se_agent_show_fpa_buff_count
 *
 *	DESCRIPTION:
 * 		se_agent_show_fpa_buff_count
 *
 *	INPUT:
 *		client_addr   ->dcli client address
 *		len ->client address length
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *				
 **********************************************************************************/
void se_agent_show_fpa_buff_count(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	int ret = -1,i = 0;
    se_interative_t *se_buf = NULL;

	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_aging_time param error\n");
		return ;
	}  

	se_buf=(se_interative_t *)buf;
	if(se_buf->cpu_tag == 1)
	{
    	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
    	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
    	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_SHOW_FPA_BUFF;
    	se_agent_fccp_process_pcie(buf, len, 1);
    	if(se_buf->fccp_cmd.ret_val == FCCP_RETURN_ERROR)
    	{
    	    strncpy(se_buf->err_info,READ_RULE_ERROR,sizeof(READ_RULE_ERROR));
		    se_buf->cmd_result = AGENT_RETURN_FAIL;
    	}
    	goto SEND_TODCLI;
    }
	
	for(i=0;i<8;i++)
	{
		se_buf->fccp_cmd.fccp_data.pool_buff_count[i] = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(i));
	}

    se_buf->cmd_result = AGENT_RETURN_OK;
SEND_TODCLI:
	ret = sendto(se_socket,buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("show_fpa_buff_count send to dcli failed:%s\n",strerror(errno));
		return ;
	}	
}
/**********************************************************************************
 *  se_agent_read_reg
 *
 *	DESCRIPTION:
 * 		Read the processor's registers
 *
 *	INPUT:
 *		buf			  ->       pointer of store data buffer
 *          client_addr     ->        client socket address
 *		len 			  ->       the length of client socket address
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *				
 **********************************************************************************/
void se_agent_read_reg(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	uint64_t addr;
	se_interative_t *se_buf=NULL;
	int ret;
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_read_reg  parameter error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;
	addr=se_buf->fccp_cmd.fccp_data.reg_param.address;
	se_buf->fccp_cmd.fccp_data.reg_param.reg_data=cvmx_read_csr(CVMX_ADD_IO_SEG(addr));
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_read_reg send to dcli failed:%s\n",strerror(errno));
		return ;
	}
}

/**********************************************************************************
 *  se_agent_write_reg
 *
 *	DESCRIPTION:
 * 		Write the processor's register
 *
 *	INPUT:
 *		buf			  ->       pointer of store data buffer
 *          client_addr     ->        client socket address
 *		len 			  ->       the length of client socket address
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *				
 **********************************************************************************/
void se_agent_write_reg(char *buf,struct sockaddr_tipc*client_addr,unsigned int len)
{
	uint64_t addr;
	uint64_t reg_val;
	se_interative_t *se_buf=NULL;
	int ret;
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_write_reg  param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;
	addr=se_buf->fccp_cmd.fccp_data.reg_param.address;
	reg_val=se_buf->fccp_cmd.fccp_data.reg_param.reg_data;
	cvmx_write_csr(CVMX_ADD_IO_SEG(addr), reg_val);
    se_agent_syslog_dbg("\n\tValue 0x%llX written to register = 0x%llX\n\n", CAST64(reg_val), CAST64(addr));
	se_buf->cmd_result=AGENT_RETURN_OK;
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_write_reg send to dcli failed:%s\n",strerror(errno));
		return ;
	}
}
/**********************************************************************************
 *  se_agent_show_fau_dump_64
 *
 *	DESCRIPTION:
 * 		se_agent_show_fau_dump_64
 *
 *	INPUT:
 *		client_addr   ->dcli client address
 *		len ->client address length
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *				
 **********************************************************************************/
void se_agent_show_fau_dump_64(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	int ret = -1,i = 0;
	fau64_info_t  fau64_info;
	se_interative_t *se_buf = NULL;
	memset(&fau64_info,0,sizeof(fau64_info));

	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_show_fau_dump_64 param error\n");
		return ;
	}  

	se_buf=(se_interative_t *)buf;
	
    if(se_buf->cpu_tag == 1)
    {
        se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
    	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
    	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_SHOW_FAU64;
        se_agent_fccp_process_pcie(buf, len, 1);
        goto SEND_DCLI;
    }
   
	//fau64_info.fau_reg_oq_addr_index= cvmx_fau_fetch_and_add64(CVMX_FAU_REG_OQ_ADDR_INDEX, 0);
	fau64_info.fau_pko_errors=cvmx_fau_fetch_and_add64(CVM_FAU_PKO_ERRORS, 0);
	fau64_info.fau_enet_input_bytes=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_INPUT_BYTES, 0);
	fau64_info.fau_enet_input_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_INPUT_PACKETS, 0);
	fau64_info.fau_enet_output_bytes=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_BYTES, 0);
	fau64_info.fau_enet_output_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS, 0);
	fau64_info.fau_enet_drop_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_DROP_PACKETS, 0);
	fau64_info.fau_enet_to_linux_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 0);
	fau64_info.fau_enet_nonip_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_NONIP_PACKETS, 0);
	fau64_info.fau_enet_eth_pppoe_nonip_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_ETH_PPPOE_NONIP_PACKETS, 0);   			/* add by wangjian 2013-3-18*/
	fau64_info.fau_enet_capwap_pppoe_nonip_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_CAPWAP_PPPOE_NONIP_PACKETS, 0); 	/* add by wangjian 2013-3-18*/
	fau64_info.fau_enet_error_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_ERROR_PACKETS, 0);
	fau64_info.fau_enet_fragip_packets=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_FRAGIP_PACKETS, 0);
	fau64_info.fau_ip_short_packets=cvmx_fau_fetch_and_add64(CVM_FAU_IP_SHORT_PACKETS, 0);
	fau64_info.fau_ip_bad_hdr_len=cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_HDR_LEN, 0);
	fau64_info.fau_ip_bad_len=cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_LEN, 0);
	fau64_info.fau_ip_bad_version=cvmx_fau_fetch_and_add64(CVM_FAU_IP_BAD_VERSION, 0);
	fau64_info.fau_ip_skip_addr=cvmx_fau_fetch_and_add64(CVM_FAU_IP_SKIP_ADDR, 0);
	fau64_info.fau_ip_icmp=cvmx_fau_fetch_and_add64(CVM_FAU_IP_ICMP, 0);
	fau64_info.fau_capwap_icmp=cvmx_fau_fetch_and_add64(CVM_FAU_CAPWAP_ICMP, 0);
	fau64_info.fau_ip_proto_error=cvmx_fau_fetch_and_add64(CVM_FAU_IP_PROTO_ERROR, 0);
	fau64_info.fau_udp_bad_dropt=cvmx_fau_fetch_and_add64(CVM_FAU_UDP_BAD_DPORT, 0);
	fau64_info.fau_udp_bad_len=cvmx_fau_fetch_and_add64(CVM_FAU_UDP_BAD_LEN, 0);
	fau64_info.fau_udp_to_linux=cvmx_fau_fetch_and_add64(CVM_FAU_UDP_TO_LINUX, 0);
	fau64_info.fau_flowtable_hit_packets=cvmx_fau_fetch_and_add64(CVM_FAU_FLOWTABLE_HIT_PACKETS, 0);
	fau64_info.fau_enet_output_packets_8021q=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_8021Q, 0);
	fau64_info.fau_enet_output_packets_qinq=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_QINQ, 0);
	fau64_info.fau_enet_output_packets_eth_pppoe=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_ETH_PPPOE, 0); 			/* add by wangjian 2013-3-18*/
	fau64_info.fau_enet_output_packets_capwap_pppoe=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_OUTPUT_PACKETS_CAPWAP_PPPOE, 0); 	/* add by wangjian 2013-3-18*/
	fau64_info.fau_flow_lookup_error=cvmx_fau_fetch_and_add64(CVM_FAU_FLOW_LOOKUP_ERROR, 0);
	fau64_info.fau_recv_fccp=cvmx_fau_fetch_and_add64(CVM_FAU_RECV_FCCP_PACKETS,0);
	fau64_info.fau_recv_works=cvmx_fau_fetch_and_add64(CVM_FAU_RECV_TOTAL_WORKS,0);
	fau64_info.fau_acl_lookup=cvmx_fau_fetch_and_add64(CVM_FAU_TOTAL_ACL_LOOKUP,0);
	fau64_info.fau_acl_reg=cvmx_fau_fetch_and_add64(CVM_FAU_ACL_REG,0);
	fau64_info.fau_cw802_11_decap_err=cvmx_fau_fetch_and_add64(CVM_FAU_CW802_11_DECAP_ERROR,0);
	fau64_info.fau_cw802_3_decap_err=cvmx_fau_fetch_and_add64(CVM_FAU_CW802_3_DECAP_ERROR,0);
	fau64_info.fau_enet_to_linux_bytes=cvmx_fau_fetch_and_add64(CVM_FAU_ENET_TO_LINUX_BYTES,0);
	fau64_info.fau_alloc_rule_fail=cvmx_fau_fetch_and_add64(CVM_FAU_ALLOC_RULE_FAIL,0);
	fau64_info.fau_max_rule_entries=cvmx_fau_fetch_and_add64(CVM_FAU_MAX_RULE_ENTRIES,0);
	fau64_info.fau_cw_80211_err = cvmx_fau_fetch_and_add64(CVM_FAU_CW_80211_ERR,0);
	fau64_info.fau_cw_noip_packets = cvmx_fau_fetch_and_add64(CVM_FAU_CW_NOIP_PACKETS,0);
	fau64_info.fau_cw_spe_packets = cvmx_fau_fetch_and_add64(CVM_FAU_CW_SPE_PACKETS,0);
	fau64_info.fau_cw_frag_packets = cvmx_fau_fetch_and_add64(CVM_FAU_CW_FRAG_PACKETS,0);
    fau64_info.fau_mcast_packets = cvmx_fau_fetch_and_add64(CVM_FAU_MCAST_PACKETS,0);
    fau64_info.fau_rpa_packets = cvmx_fau_fetch_and_add64(CVM_FAU_RPA_PACKETS,0);
    fau64_info.fau_rpa_tolinux_packets = cvmx_fau_fetch_and_add64(CVM_FAU_RPA_TOLINUX_PACKETS,0);
    fau64_info.fau_tipc_packets = cvmx_fau_fetch_and_add64(CVM_FAU_TIPC_PACKETS,0);
    fau64_info.fau_large_eth2cw_packets = cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_ETH2CW_PACKET,0);
    fau64_info.fau_large_cw_rpa_fwd_packets = cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_CW_RPA_FWD_PACKET,0);
    fau64_info.fau_large_cw8023_rpa_fwd_packets = cvmx_fau_fetch_and_add64(CVM_FAU_LARGE_CW8023_RPA_FWD_PACKET,0);
    fau64_info.fau_spe_tcp_hdr = cvmx_fau_fetch_and_add64(CVM_FAU_SPE_TCP_HDR,0);
    fau64_info.fau_cw_spe_tcp_hdr = cvmx_fau_fetch_and_add64(CVM_FAU_CW_SPE_TCP_HDR,0);

	memcpy(&se_buf->fccp_cmd.fccp_data.fau64_info, &fau64_info, sizeof(fau64_info_t));
	se_buf->cmd_result = AGENT_RETURN_OK;
SEND_DCLI:
	ret=sendto(se_socket,buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
			se_agent_syslog_err("show_fau_dump_64 send to dcli failed:%s\n",strerror(errno));
			return ;
	}	
}
void se_agent_clear_fau64(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
		se_interative_t *se_buf=NULL;
		int ret;
		uint64_t fau_addr = 0;
		int32_t i;
		if(NULL==buf || NULL==client_addr ||0==len)
		{
				se_agent_syslog_err("se_agent_clear_fau64  param error\n");
				return ;
		}
		se_buf=(se_interative_t *)buf;

		if(se_buf->cpu_tag == 1)
        {
            se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
        	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
        	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_CLEAR_FAU64;
            se_agent_fccp_process_pcie(buf, len, 1);
            goto SEND_DCLI;
        }
    
		i = (CVM_FAU_PKO_ERRORS - CVMX_FAU_REG_64_START) >> 3;
		for(;fau_addr < CVMX_FAU_REG_64_END; i++)
		{
				fau_addr = CVMX_FAU_REG_64_ADDR(i);
				cvmx_fau_atomic_write64(fau_addr, 0);
		}
		se_buf->cmd_result=AGENT_RETURN_OK;
SEND_DCLI:
		ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
		if(ret<0)
		{
				se_agent_syslog_err("se_agent_clear_fau64 send to dcli failed:%s\n",strerror(errno));
				return ;
		}
}


/* fast-fwd group add for  logsave */
#define MAX_CORE_NUM 32
CVMX_SHARED uint32_t log_mem_size = 1024*1024;
#define FASTFWD_LOG_MEM_NAME    "fastfwd_log_core%d"  
void se_agent_config_fwdlog_enable(char *buf, struct sockaddr_tipc *client_addr, unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret;
	
	if(NULL == buf || NULL == client_addr || 0 ==len)
	{
		se_agent_syslog_err("se_agent_config_fwdlog_enable param error\n");
		return; 
	}
	
	if(NULL == ipfwd_learn_name)
	{
		se_agent_syslog_err("se_agent_config_fwdlog_enable not find ipfwd_learn module\n");
		return; 
	}
	
	
	se_buf = (se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info), FASTFWD_NOT_LOADED_STR, strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto func_end;
	}
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_ENABLE_LOG;

    ret = se_agent_fccp_process(buf, len, 1);
	if(ret == SE_AGENT_RETURN_OK)
	{
		se_buf->cmd_result = AGENT_RETURN_OK;
		goto func_end;
	}
	else
	{
		goto se_log_err;
	}

se_log_err:
	se_buf->cmd_result = AGENT_RETURN_FAIL;
	strncpy(se_buf->err_info, "se_agent_config_fwdlog_enable set fastfwd log failed\n", ERR_INFO_SIZE);
	goto func_end;
func_end:
	ret = sendto(se_socket, (char*)buf,sizeof(se_interative_t), 0, (struct sockaddr*)client_addr, len);
	if(ret < 0)
	{
		se_agent_syslog_err("se_agent_config_fwdlog_enable send to dcli failed:%s\n", strerror(errno));
		return; 
	}
	
	return; 
}

void se_agent_show_fwdlog_enable(char *buf, struct sockaddr_tipc *client_addr, unsigned int len)
{
	se_interative_t *se_buf = NULL;

	if(NULL == buf || NULL == client_addr || 0 ==len)
	{
		se_agent_syslog_err("se_agent_show_fwdlog_enable param error\n");
		return; 
	}  

	se_buf = (se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_ENABLE_LOG_SHOW ;

    se_agent_fccp_process(buf, len, 1);

    if(sendto(se_socket, buf, sizeof(se_interative_t), 0, (struct sockaddr*)client_addr,len) < 0)
	{
		se_agent_syslog_err("se_agent_show_fwdlog_enable send to dcli failed\n");
		return; 
	}

	return; 

}

void se_agent_config_fwdlog_level(char *buf, struct sockaddr_tipc *client_addr, unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret;
	
	if(NULL == buf || NULL == client_addr ||0 == len)
	{
		se_agent_syslog_err("se_agent_config_fwdlog_level param error\n");
		return; 
	}
	
	if(NULL == ipfwd_learn_name)
	{
		se_agent_syslog_err("se_agent_config_fwdlog_level not find ipfwd_learn module\n");
		return; 
	}
	
	se_buf = (se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info), FASTFWD_NOT_LOADED_STR, strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto func_end;
	}
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_SET_LOG_LEVEL;

    ret = se_agent_fccp_process(buf, len, 1);
	if(ret == SE_AGENT_RETURN_OK)
	{
		se_buf->cmd_result=AGENT_RETURN_OK;
		goto func_end;
	}
	else
	{
		goto se_log_level_err;
	}

se_log_level_err:
	se_buf->cmd_result = AGENT_RETURN_FAIL;
	strncpy(se_buf->err_info, "set fastfwd log level failed\n", ERR_INFO_SIZE);
	goto func_end;
func_end:
	ret = sendto(se_socket, (char*)buf, sizeof(se_interative_t), 0, (struct sockaddr*)client_addr, len);
	if(ret < 0)
	{
		se_agent_syslog_err("se_agent_config_fwdlog_level send to dcli failed:%s\n",strerror(errno));
		return; 
	}
	
	return; 
}

void se_agent_show_fwdlog_level(char *buf, struct sockaddr_tipc *client_addr, unsigned int len)
{
	se_interative_t *se_buf = NULL;

	if(NULL == buf || NULL == client_addr || 0 == len)
	{
		se_agent_syslog_err("se_agent_show_fwdlog_enable param error\n");
		return; 
	}  

	se_buf = (se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_SHOW_LOG_LEVEL;

    se_agent_fccp_process(buf, len, 1);

    if(sendto(se_socket,buf, sizeof(se_interative_t), 0, (struct sockaddr*)client_addr,len) < 0)
	{
		se_agent_syslog_err("se_agent_show_fwdlog_level send to dcli failed\n");
		return; 
	}

	return; 

}

void se_agent_clear_fwdlog(char *buf, struct sockaddr_tipc *client_addr, unsigned int len)
{
	se_interative_t *se_buf=NULL;
	struct nlmsghdr *nl_buf=NULL;
	struct timeval	overtime;
	unsigned int tmp_time;
	int ret,nl_len;
	char* fwd_log_memory = NULL;
	char bootmem_name[64];
	int i;
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_clear_fwdlog param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto SENDTO_DCLI;
	}
	
	const cvmx_bootmem_named_block_desc_t *fwd_log_mem_desc = NULL;
        
    for(i = 0; i < MAX_CORE_NUM; i++)
    {
        memset(bootmem_name, 0, sizeof(bootmem_name));
        sprintf(bootmem_name, FASTFWD_LOG_MEM_NAME, i);
   
    	fwd_log_mem_desc = cvmx_bootmem_find_named_block(bootmem_name);
    	if(NULL != fwd_log_mem_desc)
    	{
    		//fwd_log_mem_addr[i] = octeon_phys_to_ptr(fwd_log_mem_desc->base_addr);
			//memset(octeon_phys_to_ptr(fwd_log_mem_desc->base_addr), 0, log_mem_size);
			memset(cvmx_phys_to_ptr(fwd_log_mem_desc->base_addr), 0, log_mem_size);
		}    	
	}

	se_buf->cmd_result = AGENT_RETURN_OK;

SENDTO_DCLI:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("send to dcli failed\n");
		return;
	}
	return;

}



/* fast-fwd group add for  logsave */

void se_agent_icmp_enable(char *buf, struct sockaddr_tipc *client_addr, unsigned int len)
{
	uint32_t enable;
	se_interative_t *se_buf=NULL;
	int ret,rval,status;
	char str[100]={0};
	
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_icmp_enable  param error\n");
		return ;
	}
	if(NULL == ipfwd_learn_name)
	{
		se_agent_syslog_err("se_agent_icmp_enable not find ipfwd_learn module\n");
		return ;
	}
	
	se_buf=(se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto func_end;
	}
	enable=se_buf->fccp_cmd.fccp_data.module_enable;
	sprintf(str,"echo %d > /sys/module/%s/parameters/icmp_enable",enable, ipfwd_learn_name);
	rval=system(str);
	status=WEXITSTATUS(rval);
	if(status)
	{
		se_agent_syslog_err("set fast_forward icmp learned error\n");
		goto learned_icmp_err;
	}
	
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_ENABLE_ICMP;

    ret = se_agent_fccp_process(buf, len, 1);
	if(ret ==SE_AGENT_RETURN_OK)
	{
		se_buf->cmd_result=AGENT_RETURN_OK;
		goto func_end;
	}
	else
	{
		goto se_icmp_err;
	}
	
learned_icmp_err:
	se_buf->cmd_result=AGENT_RETURN_FAIL;
	strncpy((char*)(se_buf->err_info),"set ipfwd_learned icmp failed\n",ERR_INFO_SIZE);
	goto func_end;
se_icmp_err:
	se_buf->cmd_result=AGENT_RETURN_FAIL;
	strncpy(se_buf->err_info,"set fastfwd icmp failed\n",ERR_INFO_SIZE);
	goto func_end;
func_end:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_icmp_enable send to dcli failed:%s\n",strerror(errno));
		return ;
	}
	return ;
}

int32_t get_fastfwd_stats()
{
	int32_t port_index = 0;
    int32_t enable = FUNC_DISABLE;
    
    for (port_index=0; port_index < CVMX_PIP_NUM_INPUT_PORTS; port_index++)
    {
        if(AVAILABLE_PORT != check_pip_offset(port_index))
        {
            continue;
        }
        cvmx_pip_port_tag_cfg_t tag_config;
        tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));
        if (tag_config.s.grp != TO_LINUX_GROUP) 
        {
            enable = FUNC_ENABLE;
            break;
        } 
        else 
        {
            enable = FUNC_DISABLE;
            break;
        }
    }

    return enable;
}

int32_t set_fastfwd_enable(uint32_t enable)
{
	int32_t port_index = 0;
	int32_t rval = 0;;
	int32_t status = 0;
    uint8_t  group;
	char str[100]={0};

    /* enable or disable fastfwd */
	if(FUNC_ENABLE == enable)
		group=PANEL_PORT_GROUP;
	else
		group = TO_LINUX_GROUP;
	for (port_index=0; port_index < CVMX_PIP_NUM_INPUT_PORTS; port_index++)
	{
		if(AVAILABLE_PORT != check_pip_offset(port_index))
		{
			continue;
		}
		cvmx_pip_port_tag_cfg_t tag_config;
		tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));
		tag_config.s.grp = group;
		cvmx_write_csr(CVMX_PIP_PRT_TAGX(port_index), tag_config.u64);
	}   

    /* enable or disable ipfwd_learn */
    if(NULL != ipfwd_learn_name)
    {
    	sprintf(str,"echo %d > /sys/module/%s/parameters/learn_enable",enable, ipfwd_learn_name);
    	rval=system(str);
    	status=WEXITSTATUS(rval);
	}

    return status;
}

void se_agent_fastfwd_enable(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	uint32_t enable;
	se_interative_t *se_buf=NULL;
	int ret,status;
	
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_fastfwd_enable param error\n");
		return ;
	}
	if(NULL == ipfwd_learn_name)
	{
		se_agent_syslog_err("se_agent_fastfwd_enable not find ipfwd_learn module\n");
		return ;
	}
	
	se_buf=(se_interative_t *)buf;
	if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
	{
		strncpy((se_buf->err_info),FASTFWD_NOT_LOADED_STR,strlen(FASTFWD_NOT_LOADED_STR));
		se_buf->cmd_result = AGENT_RETURN_FAIL;
		goto func_end;
	}
	enable=se_buf->fccp_cmd.fccp_data.module_enable;
	if(!(FUNC_ENABLE == enable ||FUNC_DISABLE == enable))
	{
		strncpy((char*)(se_buf->err_info),"fastfwd enable param error\0",ERR_INFO_SIZE);
		se_buf->cmd_result=AGENT_RETURN_FAIL;
		goto func_end;
	}	

    status = set_fastfwd_enable(enable);
	if(status)
	{
		se_agent_syslog_err("config ipfwd_learn failed\n");
		strncpy((char*)(se_buf->err_info),"config ipfwd_learn failed\0",ERR_INFO_SIZE);
		se_buf->cmd_result=AGENT_RETURN_FAIL;
		goto func_end;
	}
	
	se_buf->cmd_result=AGENT_RETURN_OK;
func_end:
	ret=sendto(se_socket,(char*)buf,sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_icmp_enable send to dcli failed:%s\n",strerror(errno));
		return ;
	}
	return ;
}

void se_agent_set_bucket_entry(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
    int ret = -1;
    
	se_buf=(se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_SET_BUCKET_MAX_ENTRY;

    se_agent_fccp_process(buf, len, 1);

	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("send to dcli failed\n");
		return ;
	}
	return ;
}

void se_agent_get_bucket_entry(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	int ret = -1;
	
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_get_bucket_entry  param error\n");
		return ;
	}
	se_buf=(se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_GET_BUCKET_MAX_ENTRY;

    se_agent_fccp_process(buf, len, 1);

	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("send to dcli failed\n");
		return ;
	}
	return ;
}

void se_agent_config_traffic_monitor(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret = -1;

	if(NULL == buf || NULL == client_addr ||0 == len)
	{
		se_agent_syslog_err("se_agent_config_traffic_monitor param error\n");
		return ;
	}
	
	se_buf = (se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_ENABLE_TRAFFIC_MONITOR;

    se_agent_fccp_process(buf, len, 1);

	ret = sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_config_traffic_monitor send to dcli failed\n");
		return ;
	}
	return ;
}

void se_agent_clear_traffic_monitor(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret = 0;

	if(NULL == buf || NULL == client_addr ||0 == len)
	{
		se_agent_syslog_err("se_agent_clear_traffic_monitor param error\n");
		return ;
	}
	se_buf = (se_interative_t *)buf;
	
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_CLEAR_TRAFFIC_MONITOR;

	ret = se_agent_fccp_process(buf, len, 1);
	if(ret == SE_AGENT_RETURN_OK)
	{
    	memset((char*)&old_traffic_stats,0,sizeof(old_traffic_stats));
    	traffic_minitor_old_time = 0;
	}

	ret = sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_clear_traffic_monitor send to dcli failed\n");
		return ;
	}
	return ;
}

void calculate_traffic_rate(traffic_stats_t *traffic_stats)
{
	uint64_t interval_time = 0;
	interval_time = get_sec()- traffic_minitor_old_time;
	if(0 == interval_time)
	{
		memcpy((char*)&old_traffic_stats,(char*)traffic_stats,sizeof(old_traffic_stats));
		traffic_minitor_old_time = get_sec();
		return;
	}
	/*total packet*/
	traffic_stats->total_pps = ((traffic_stats->total_pkt - old_traffic_stats.total_pkt)/interval_time);
	/*error packet*/
	traffic_stats->err_pps = ((traffic_stats->err_pkt - old_traffic_stats.err_pkt)/interval_time);
	/*bcast packet*/
	traffic_stats->bcast_pps = ((traffic_stats->bcast_pkt - old_traffic_stats.bcast_pkt)/interval_time);
	/*mcast packet*/
	traffic_stats->mcast_pps = ((traffic_stats->mcast_pkt - old_traffic_stats.mcast_pkt)/interval_time);
	/*total_noip  packet*/
	traffic_stats->total_noip_pps = ((traffic_stats->total_noip_pkt - old_traffic_stats.total_noip_pkt)\
	                                  /interval_time);
	/*arp packet*/
	traffic_stats->arp_pps = ((traffic_stats->arp_pkt - old_traffic_stats.arp_pkt)/interval_time);
	/*rarp packet*/
	traffic_stats->rarp_pps = ((traffic_stats->rarp_pkt - old_traffic_stats.rarp_pkt)/interval_time);
	/*vrrp packet*/
	traffic_stats->vrrp_pps = ((traffic_stats->vrrp_pkt - old_traffic_stats.vrrp_pkt)/interval_time);
	/*_802_1x packet*/
	traffic_stats->_802_1x_pps = ((traffic_stats->_802_1x_pkt - old_traffic_stats._802_1x_pkt)/interval_time);
	/* total ip  packet*/
	traffic_stats->total_ip_pps = ((traffic_stats->total_ip_pkt - old_traffic_stats.total_ip_pkt)/interval_time);
	/*ip_exception  packet*/
	traffic_stats->ip_exception_pps = ((traffic_stats->ip_exception_pkt - old_traffic_stats.ip_exception_pkt)\
	                                   /interval_time);
	/*frag  packet*/
	traffic_stats->frag_pps = ((traffic_stats->frag_pkt - old_traffic_stats.frag_pkt)/interval_time);
	/*ipv6  packet*/
	traffic_stats->ipv6_pps = ((traffic_stats->ipv6_pkt - old_traffic_stats.ipv6_pkt)/interval_time);
	/*l4_err  packet*/
	traffic_stats->l4_err_pps = ((traffic_stats->l4_err_pkt - old_traffic_stats.l4_err_pkt)/interval_time);
	/*total tcp  packet*/
	traffic_stats->total_tcp_pps = ((traffic_stats->total_tcp_pkt - old_traffic_stats.total_tcp_pkt)\
	                                  /interval_time);
	/*telnet  packet*/
	traffic_stats->telnet_pps = ((traffic_stats->telnet_pkt - old_traffic_stats.telnet_pkt)\
	                                 /interval_time);
	/*ssh  packet*/
	traffic_stats->ssh_pps = ((traffic_stats->ssh_pkt - old_traffic_stats.ssh_pkt)\
	                                 /interval_time);
	/*total_udp packet*/
	traffic_stats->total_udp_pps = ((traffic_stats->total_udp_pkt - old_traffic_stats.total_udp_pkt)\
	                                 /interval_time);
	/*dhcp  packet*/
	traffic_stats->dhcp_pps = ((traffic_stats->dhcp_pkt - old_traffic_stats.dhcp_pkt)/interval_time);
	/*rip  packet*/
	traffic_stats->rip_pps = ((traffic_stats->rip_pkt - old_traffic_stats.rip_pkt)/interval_time);
	/*cw_ctl  packet*/
	traffic_stats->cw_ctl_pps = ((traffic_stats->cw_ctl_pkt - old_traffic_stats.cw_ctl_pkt)/interval_time);
	/*cw_dat  packet*/
	traffic_stats->cw_dat_pps = ((traffic_stats->cw_dat_pkt - old_traffic_stats.cw_dat_pkt)/interval_time);
	/*cw_arp packet*/
	traffic_stats->cw_arp_pps = ((traffic_stats->cw_arp_pkt - old_traffic_stats.cw_arp_pkt)/interval_time);
	/*cw_ip_exc  packet*/
	traffic_stats->cw_ip_exc_pps = ((traffic_stats->cw_ip_exc_pkt - old_traffic_stats.cw_ip_exc_pkt)\
	                                  /interval_time);
	/*cw_ip_frag  packet*/
	traffic_stats->cw_ip_frag_pps = ((traffic_stats->cw_ip_frag_pkt - old_traffic_stats.cw_ip_frag_pkt)\
	                                  /interval_time);
	/*cw_8023_dat  packet*/
	traffic_stats->cw_8023_dat_pps = ((traffic_stats->cw_8023_dat_pkt - old_traffic_stats.cw_8023_dat_pkt)\
	                                  /interval_time);
	/*access_radius packet*/
	traffic_stats->access_radius_pps = ((traffic_stats->access_radius_pkt - old_traffic_stats.access_radius_pkt)\
	                                    /interval_time);
	/*account_radius packet*/
	traffic_stats->account_radius_pps = ((traffic_stats->account_radius_pkt - old_traffic_stats.account_radius_pkt)\
	                                    /interval_time);
	/*portal packet*/
	traffic_stats->portal_pps = ((traffic_stats->portal_pkt - old_traffic_stats.portal_pkt)/interval_time);
	/*inter_ac_roaming packet*/
	traffic_stats->inter_ac_roaming_pps = ((traffic_stats->inter_ac_roaming_pkt - old_traffic_stats.inter_ac_roaming_pkt)\
	                                    /interval_time);
	/*cw_tcp  packet*/
	traffic_stats->cw_tcp_pps = ((traffic_stats->cw_tcp_pkt - old_traffic_stats.cw_tcp_pkt)/interval_time);
	/*cw_udp  packet*/
	traffic_stats->cw_udp_pps = ((traffic_stats->cw_udp_pkt - old_traffic_stats.cw_udp_pkt)/interval_time);
	/*cw_icmp  packet*/
	traffic_stats->cw_icmp_pps = ((traffic_stats->cw_icmp_pkt - old_traffic_stats.cw_icmp_pkt)/interval_time);	
	/* icmp  packet*/
	traffic_stats->icmp_pps = ((traffic_stats->icmp_pkt - old_traffic_stats.icmp_pkt)/interval_time);
	
	memcpy((char*)&old_traffic_stats,(char*)traffic_stats,sizeof(old_traffic_stats));
	
	traffic_minitor_old_time = get_sec();
	
	return;
	
}
void se_agent_show_traffic_monitor(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret = 0;
	uint32_t nl_len = 0;
	struct nlmsghdr *nl_buf=NULL;
	struct timeval  overtime;
	
	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_show_traffic_monitor param error\n");
		return ;
	}

	if(0 == traffic_minitor_old_time)
	{
		traffic_minitor_old_time = get_sec();
	}

	se_buf = (se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module = FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module = FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode = FCCP_CMD_GET_TRAFFIC_MONITOR;

    ret = se_agent_fccp_process(buf, len, 1);
	if(ret == SE_AGENT_RETURN_OK)
	{
	    calculate_traffic_rate((traffic_stats_t*)(&(se_buf->fccp_cmd.fccp_data)));
	}
	
	ret = sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_traffic_monitor send to dcli failed\n");
		return ;
	}
	return ;
}

/*wangjian 2012.07.09 add fwd info */
void se_agent_show_fast_fwd_info(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
	int ret = -1;
	int port_index = 0;

	if(NULL==buf || NULL==client_addr ||0==len)
	{
		se_agent_syslog_err("se_agent_show_fast_fwd_info  param error\n");
		return; 
	}
	
	se_buf=(se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_TEST;
    ret = se_agent_fccp_process(buf, len, 1);
    
	if(ret == SE_AGENT_RETURN_OK)
	{
    	/*fast forword enable ?*/
    	for (port_index=0; port_index < CVMX_PIP_NUM_INPUT_PORTS; port_index++)
    	{
    			if(AVAILABLE_PORT != check_pip_offset(port_index))
    			{
    				continue;
    			}
    			cvmx_pip_port_tag_cfg_t tag_config;
    			tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));
    			if (tag_config.s.grp != TO_LINUX_GROUP) 
    			{
                    se_buf->fccp_cmd.fccp_data.fast_fwd_info.fast_fwd_enable= 1;
                    break;
    			} else {
    			    se_buf->fccp_cmd.fccp_data.fast_fwd_info.fast_fwd_enable= 0;
                    break;
    			}
    	} 

    	/*core mask*/
    	se_buf->fccp_cmd.fccp_data.fast_fwd_info.fast_fwd_coremask = (uint32_t)cvmx_fau_fetch_and_add64(CVM_FAU_SE_CORE_MASK,0);	    
	} 

	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_show_fast_fwd_info send to dcli failed\n");
		return;
	}
	return;
}

int se_agent_equipment_test_enable(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
    int ret = 0;
    
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_config_aging_time param error\n");
		return SE_AGENT_RETURN_FAIL;
	}    
	
	se_buf=(se_interative_t *)buf;
	se_buf->fccp_cmd.dest_module=FCCP_MODULE_ACL;
	se_buf->fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	se_buf->fccp_cmd.cmd_opcode=FCCP_CMD_EQUIPMENT_TEST;
    ret = se_agent_fccp_process(buf, len, 0);

    if(agent_pcie_channel_exist == AGENT_TRUE)
    {
        uint8_t* pkt_buf = (uint8_t*)malloc(FCCP_PACKET_LEN);
        uint8_t* tmp_ptr = pkt_buf;
        memset(pkt_buf, 0, FCCP_PACKET_LEN);
	    tmp_ptr += MAC_LEN*2 ;
	    *((uint16_t *)tmp_ptr) = FCCP_L2_ETH_TYPE;
	    tmp_ptr += sizeof(uint16_t);
	    memcpy(tmp_ptr, &se_buf->fccp_cmd, sizeof(control_cmd_t));    
        se_agent_syslog_err("se_agent se_agent_send_fccp_from_pci\n");
        ret = se_agent_send_fccp_from_pci(pkt_buf, FCCP_PACKET_LEN, pkt_buf, FCCP_PACKET_LEN);    
        if(ret == SE_AGENT_RETURN_OK)
        {
            se_agent_syslog_err("se_agent se_agent_send_fccp_from_pci return OK!\n");
        }
        else
        {
            se_agent_syslog_err("se_agent se_agent_send_fccp_from_pci return FAIL~~~\n");
        }
        free(pkt_buf);
    }
    else
    {
        se_agent_syslog_err("se_agent agent_pcie_channel_exist false~~~\n");
    }
    return SE_AGENT_RETURN_OK;
}

/*2013.2.28   Command to set interval for clearing aged rules */
int se_agent_config_clear_aged_rule_time(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
    unsigned int tmp_time;
    
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_config_clear_aged_rule_time param error\n");
		return SE_AGENT_RETURN_FAIL;
	}    
	
	se_buf=(se_interative_t *)buf;
	tmp_time=se_buf->fccp_cmd.fccp_data.aging_timer;

	if(tmp_time > 4294967294)
	{
	    se_agent_syslog_err("se_agent_config_clear_aged_rule_time time_range [0,4294967294]\n");
    }else
    {
        clr_aging_interval = tmp_time;
    }
    
    se_buf->cmd_result = AGENT_RETURN_OK;
    
	if(sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len) < 0)
	{
		se_agent_syslog_err("se_agent_config_clear_aged_rule_time send to dcli failed\n");
		return SE_AGENT_RETURN_FAIL;
	}

    return SE_AGENT_RETURN_OK;
}

/*2013.2.28  Command to get interval for clearing aged rule*/
int se_agent_get_clear_aged_rule_time(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf=NULL;
    
	if(NULL==buf || NULL==client_addr || 0==len)
	{
		se_agent_syslog_err("se_agent_get_clear_aged_rule_time param error\n");
		return SE_AGENT_RETURN_FAIL;
	}    
	
	se_buf=(se_interative_t *)buf;
	se_buf->fccp_cmd.fccp_data.aging_timer = clr_aging_interval;
	se_buf->cmd_result = AGENT_RETURN_OK;
	
	if(sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len) < 0)
	{
		se_agent_syslog_err("se_agent_get_clear_aged_rule_time send to dcli failed\n");
		return SE_AGENT_RETURN_FAIL;
	}

    return SE_AGENT_RETURN_OK;
}

unsigned int  se_agent_string_hash(const void *name,  unsigned size)
{
	register unsigned int accum = 0;
	register const unsigned char *s = (const unsigned char *)name;
	int i = strlen(name);
	while (i--) 
	{
		accum = (accum << 1) + *s++;
		while (accum > 65535) 
		{
			accum = (accum & 65535) + (accum >> 16);
		}
	}
	return accum % size;
}
int se_agent_cmd_func_register(char *name,cmd_handle_func func)
{
	unsigned int index = 0;
	func_item_t *func_item_head = NULL;
	func_item_t *func_item_tmp = NULL;
	func_item_t *func_item_tail = NULL;
	if(NULL == name || NULL == func)
	{
		se_agent_syslog_err("se_agent_cmd_func_register param error\n");
		return SE_AGENT_RETURN_FAIL;
	}
	if(NULL == se_agent_cmd_func_tbl)
	{
		se_agent_syslog_err("se_agent_cmd_func_register se_agent_cmd_func_tbl NULL\n");
		return SE_AGENT_RETURN_FAIL;
	}
	index = se_agent_string_hash(name,CMD_HANDLE_FUNC_NUM);
	func_item_head = se_agent_cmd_func_tbl + index;
	func_item_tail = func_item_head;
	if(0 != func_item_tail->valid_entries)
	{
		while(func_item_tail->next != NULL)
		{
			func_item_tail = func_item_tail->next;
		}
		
		func_item_tmp = malloc(sizeof(func_item_t));
		if(NULL == func_item_tmp)
		{
			se_agent_syslog_err("se_agent_cmd_func_register function %s malloc error\n",name);
			return SE_AGENT_RETURN_FAIL;
		}
		strncpy(func_item_tmp->cmd_name,name,strlen(name));
		func_item_tmp->func = func;
		func_item_tmp->next = NULL;
		func_item_tail->next = func_item_tmp;
		func_item_head->valid_entries++;
	}
	else
	{
		strncpy(func_item_tail->cmd_name,name,strlen(name));
		func_item_tail->func = func;
		func_item_tail->next = NULL;
		func_item_head->valid_entries++;
	}
	return SE_AGENT_RETURN_OK;
}


int se_agent_cmd_func_table_init()
{
	se_agent_cmd_func_tbl = malloc(CMD_HANDLE_FUNC_NUM * sizeof(func_item_t));
	if(NULL == se_agent_cmd_func_tbl)
	{
		se_agent_syslog_err("se_agent_cmd_func_table_init malloc command function table error\n");
		return SE_AGENT_RETURN_FAIL;
	}
	memset(se_agent_cmd_func_tbl,0,(CMD_HANDLE_FUNC_NUM * sizeof(func_item_t)));
	se_agent_cmd_func_register(SE_AGENT_SET_AGING_TIME,(cmd_handle_func)se_agent_config_aging_time);
	se_agent_cmd_func_register(SE_AGENT_DELETE_RULE,(cmd_handle_func)se_agent_delete_specified_rule);
	se_agent_cmd_func_register(SE_AGENT_SET_FWD_TAG_TYPE,(cmd_handle_func)se_agent_config_tag_type);
	se_agent_cmd_func_register(SE_AGENT_SHOW_FWD_TAG_TYPE,(cmd_handle_func)se_agent_show_tag_type);
	se_agent_cmd_func_register(SE_AGENT_SHOW_AGING_TIME,(cmd_handle_func)se_agent_show_aging_time);
	se_agent_cmd_func_register(SE_AGENT_SET_BUCKET_ENTRY,(cmd_handle_func)se_agent_set_bucket_entry);
	se_agent_cmd_func_register(SE_AGENT_SHOW_BUCKET_ENTRY,(cmd_handle_func)se_agent_get_bucket_entry);
	se_agent_cmd_func_register(SE_AGENT_CLEAR_FAU64,(cmd_handle_func)se_agent_clear_fau64);
	se_agent_cmd_func_register(SE_AGENT_ICMP_ENABLE,(cmd_handle_func)se_agent_icmp_enable);
	se_agent_cmd_func_register(SE_AGENT_FASTFWD_ENABLE,(cmd_handle_func)se_agent_fastfwd_enable);
	se_agent_cmd_func_register(SE_AGENT_SHOW_ACL_STATS,(cmd_handle_func)se_agent_show_rule_sum);
	se_agent_cmd_func_register(SE_AGENT_SHOW_CAPWAP,(cmd_handle_func)se_agent_show_capwap_rule);
	se_agent_cmd_func_register(SE_AGENT_SHOW_FIVE_TUPLE_ACL,(cmd_handle_func)se_agent_show_specified_rule);
	se_agent_cmd_func_register(SE_AGENT_SHOW_AGING_RULE_CNT,(cmd_handle_func)se_agent_show_aging_rule_cnt);
	se_agent_cmd_func_register(SE_AGENT_SHOW_USER_ACL,(cmd_handle_func)se_agent_show_user_rule_stats);
	se_agent_cmd_func_register(SE_AGENT_SHOW_ACL_LEARNED,(cmd_handle_func)se_agent_show_learned_acl);
	se_agent_cmd_func_register(SE_AGENT_SHOW_ACL_LEARNING,(cmd_handle_func)se_agent_show_learning_acl);
	se_agent_cmd_func_register(SE_AGENT_SHOW_TOLINUX_FLOW,(cmd_handle_func)se_agent_show_tolinux_flow);
	se_agent_cmd_func_register(SE_AGENT_CLEAR_AGING_RULE,(cmd_handle_func)se_agent_clear_aging_rule);
	se_agent_cmd_func_register(SE_AGENT_CLEAR_RULE_ALL,(cmd_handle_func)se_agent_clear_rule_all);
	
	se_agent_cmd_func_register(SE_AGENT_UART_SWITCH,(cmd_handle_func)se_agent_uart_switch);
	se_agent_cmd_func_register(SE_AGENT_SHOW_RUNNING_CFG,(cmd_handle_func)se_agent_show_running_config);
	se_agent_cmd_func_register(SE_AGENT_SHOW_AVAILIABLE_BUFF_COUNT,(cmd_handle_func)se_agent_show_fpa_buff_count);
	se_agent_cmd_func_register(SE_AGENT_SHOW_FAU64,(cmd_handle_func)se_agent_show_fau_dump_64);
	se_agent_cmd_func_register(SE_AGENT_CONFIG_AGENT_DEBUG_LEVEL,(cmd_handle_func)se_agent_log_config_debug_level);
	se_agent_cmd_func_register(SE_AGENT_USER_ONLINE,(cmd_handle_func)se_agent_user_online);
	se_agent_cmd_func_register(SE_AGENT_USER_OFFLINE,(cmd_handle_func)se_agent_user_offline);
	se_agent_cmd_func_register(SE_AGENT_GET_USER_FLOWS,(cmd_handle_func)se_agent_get_user_flow_statistics);
	se_agent_cmd_func_register(SE_AGENT_CONFIG_PURE_PAYLOAD_ACCT,(cmd_handle_func)se_agent_config_pure_payload_acct);
	se_agent_cmd_func_register(SE_AGENT_CONFIG_TRAFFIC_MONITOR,(cmd_handle_func)se_agent_config_traffic_monitor);
	se_agent_cmd_func_register(SE_AGENT_CLEAR_TRAFFIC_MONITOR,(cmd_handle_func)se_agent_clear_traffic_monitor);
	se_agent_cmd_func_register(SE_AGENT_SHOW_TRAFFIC_MONITOR,(cmd_handle_func)se_agent_show_traffic_monitor);
	se_agent_cmd_func_register(SE_AGENT_READ_REG,(cmd_handle_func)se_agent_read_reg);
	se_agent_cmd_func_register(SE_AGENT_WRITE_REG,(cmd_handle_func)se_agent_write_reg);
	se_agent_cmd_func_register(SE_AGENT_CLEAR_RULE_IP,(cmd_handle_func)se_agent_clear_rule_ip); /* wangjian clear */
	se_agent_cmd_func_register(SE_AGENT_SHOW_FAST_FWD_INFO,(cmd_handle_func)se_agent_show_fast_fwd_info);   /*wangjian 2012.07.09 add fwd info */
	se_agent_cmd_func_register(SE_AGENT_SHOW_RULE_IP,(cmd_handle_func)se_agent_show_rule_ip);               /*wangjian 2012.07.09 add ip */
	se_agent_cmd_func_register(SE_AGENT_CONFIG_FWDLOG_ENABLE,(cmd_handle_func)se_agent_config_fwdlog_enable);
	se_agent_cmd_func_register(SE_AGENT_SHOW_FWDLOG_ENABLE,(cmd_handle_func)se_agent_show_fwdlog_enable);
	se_agent_cmd_func_register(SE_AGENT_CONFIG_FWDLOG_LEVEL,(cmd_handle_func)se_agent_config_fwdlog_level);
	se_agent_cmd_func_register(SE_AGENT_SHOW_FWDLOG_LEVEL,(cmd_handle_func)se_agent_show_fwdlog_level);
	se_agent_cmd_func_register(SE_AGENT_CLEAR_FWDLOG,(cmd_handle_func)se_agent_clear_fwdlog);
	se_agent_cmd_func_register(SE_AGENT_EQUIPMENT_TEST_ENABLE,(cmd_handle_func)se_agent_equipment_test_enable);
	se_agent_cmd_func_register(SE_AGENT_SHOW_FWD_USER_STATS,(cmd_handle_func)se_agent_show_fwd_user_stats);
    se_agent_cmd_func_register(SE_AGENT_SHOW_FWD_USER_RULE_ALL,(cmd_handle_func)se_agent_show_fwd_user_rule_all);
    se_agent_cmd_func_register(SE_AGENT_SET_CLEAR_AGED_RULE_TIME,(cmd_handle_func)se_agent_config_clear_aged_rule_time);
    se_agent_cmd_func_register(SE_AGENT_GET_CLEAR_AGED_RULE_TIME,(cmd_handle_func)se_agent_get_clear_aged_rule_time);
    se_agent_cmd_func_register(SE_AGENT_SHOW_USER_RULE_BY_IP,(cmd_handle_func)se_agent_show_user_rule_by_ip);
	return SE_AGENT_RETURN_OK;
}

/**********************************************************************************
 *  se_agent_cmd_handle
 *
 *	DESCRIPTION:
 * 		handle dcli command funcation
 *
 *	INPUT:
 *		buf -> dcli data buff
 *		client_addr   ->dcli client address
 *		len ->client address length
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *				
 **********************************************************************************/
void se_agent_cmd_handle(char *buf,struct sockaddr_tipc *client_addr,unsigned int len)
{
	se_interative_t *se_buf = NULL;
	int ret = -1;
	func_item_t  * cmd_func = NULL;
	unsigned int index = 0;
	if(NULL == buf || NULL == client_addr )
	{
		se_agent_syslog_err("se_cmd_handle param is error\n");
		return ;
	}
	se_buf = (se_interative_t *)buf;
	index = se_agent_string_hash(se_buf->hand_cmd, CMD_HANDLE_FUNC_NUM);
	cmd_func = se_agent_cmd_func_tbl + index;
	if(0 == cmd_func ->valid_entries)
	{
		se_agent_syslog_err("not find handle function,vail_entries = 0\n");
		goto ERR_HANDLE;
	}
	else if(1 == cmd_func->valid_entries)
	{
		if(!strcmp(se_buf->hand_cmd , cmd_func ->cmd_name))
		{
			cmd_func->func(buf,client_addr,len);
		}
		else
		{
			se_agent_syslog_err("not find command %s handle function\n",cmd_func->cmd_name);
			goto ERR_HANDLE;
		}
	}
	else
	{
		while(strcmp(se_buf->hand_cmd , cmd_func ->cmd_name) && cmd_func->next !=NULL)
		{
			cmd_func = cmd_func->next;
		}
		if(cmd_func == NULL)
		{
			se_agent_syslog_err("not find command handle function\n");
			goto ERR_HANDLE;
		}
		else
		{
			cmd_func->func(buf,client_addr,len);
		}
	}
	return;
ERR_HANDLE:
	se_buf=(se_interative_t *)buf;
	strncpy(se_buf->err_info,NOT_HANDLE_FUNC_STR,sizeof(NOT_HANDLE_FUNC_STR));
	se_buf->cmd_result = AGENT_RETURN_FAIL;
	ret=sendto(se_socket,buf, sizeof(se_interative_t),0,(struct sockaddr*)client_addr,len);
	if(ret<0)
	{
		se_agent_syslog_err("se_agent_cmd_handle send to dcli failed\n");
		return ;
	}
	return ;
}
void find_fastfwd_table()
{
	const cvmx_bootmem_named_block_desc_t  *acl_bucket_tbl_block = NULL;
	const cvmx_bootmem_named_block_desc_t  *capwap_cache_block = NULL;
	const cvmx_bootmem_named_block_desc_t  *acl_dynamic_tbl_block = NULL;
	const cvmx_bootmem_named_block_desc_t  *user_bucket_tbl_block = NULL;
	const cvmx_bootmem_named_block_desc_t  *user_dynamic_tbl_block = NULL;
	acl_bucket_tbl_block=cvmx_bootmem_find_named_block(ACL_TBL_RULE_NAME);
	if(!acl_bucket_tbl_block)
	{
		se_agent_syslog_err("find acl_bucket_table block error\n");
	}
	else
	{
		acl_bucket_tbl=cvmx_phys_to_ptr(acl_bucket_tbl_block->base_addr);
		acl_static_tbl_size=(acl_bucket_tbl_block->size)/(sizeof(rule_item_t));
	}

	acl_dynamic_tbl_block=cvmx_bootmem_find_named_block(ACL_DYNAMIC_TBL_RULE_NAME);
	if(!acl_dynamic_tbl_block)
	{
		se_agent_syslog_err("find acl dynamic table block error\n");
	}
	else
	{
		acl_dynamic_tbl=cvmx_phys_to_ptr(acl_dynamic_tbl_block->base_addr);
		acl_dynamic_tbl_size=(acl_dynamic_tbl_block->size)/(sizeof(rule_item_t));
	}
	
	capwap_cache_block=cvmx_bootmem_find_named_block(CAPWAP_CACHE_TBL_NAME);
	if(!capwap_cache_block)
	{
		se_agent_syslog_err("find capwap_cache block error\n");
	}
	else
	{
		capwap_cache_tbl=cvmx_phys_to_ptr(capwap_cache_block->base_addr);
		capwap_cache_tbl_size=(capwap_cache_block->size)/(sizeof(capwap_cache_t));
	}
	
	user_bucket_tbl_block = cvmx_bootmem_find_named_block(USER_TBL_RULE_NAME);
	if(!user_bucket_tbl_block)
	{
		se_agent_syslog_err("find user table block error\n");
	}
	else
	{
		user_bucket_tbl = cvmx_phys_to_ptr(user_bucket_tbl_block->base_addr);
		user_static_tbl_size = (user_bucket_tbl_block->size)/sizeof(user_item_t);
	}

	user_dynamic_tbl_block = cvmx_bootmem_find_named_block(USER_DYNAMIC_TBL_RULE_NAME);
	if(!user_dynamic_tbl_block)
	{
		se_agent_syslog_err("find user dynamic table block error\n");
	}
	else
	{
		user_dynamic_tbl = cvmx_phys_to_ptr(user_dynamic_tbl_block->base_addr);
		user_dynamic_tbl_size = (user_dynamic_tbl_block->size)/sizeof(user_item_t);
	}
	return ;
}


int get_base_mac_table(uint8_t **base_mac_tbl)
{
	const cvmx_bootmem_named_block_desc_t *block_desc = NULL;
   
	block_desc = cvmx_bootmem_find_named_block(BASE_MAC_TABLE_NAME);
	if (block_desc)    
	{    
		*base_mac_tbl = (uint8_t*)cvmx_phys_to_ptr(block_desc->base_addr);
		return SE_AGENT_RETURN_OK;
	}
	else
	{
		se_agent_syslog_err("not find base mac table\n");
		return SE_AGENT_RETURN_FAIL;
	}
}

int se_agent_read_from_file(char *filename, char *buff, int len)
{
	FILE * fp = NULL;
	if((filename == NULL) || (buff == NULL))
	{
		return SE_AGENT_RETURN_FAIL;
	}

	fp = fopen(filename, "r");
    if(NULL == fp)
    {
        se_agent_syslog_err("%s open failed!\n",filename);
        return SE_AGENT_RETURN_FAIL;
    }
    if(NULL == fgets(buff, len, fp))
    {
        se_agent_syslog_err("read %s failed!\n",filename);
		fclose(fp);
        return SE_AGENT_RETURN_FAIL;
    }
	fclose(fp);
	return SE_AGENT_RETURN_OK; 
}

CVMX_SHARED cvmx_spinlock_t base_mac_lock; 

int se_agent_get_mac_addr(void)
{
    char statebuf[4] = {0};
	char slot_count_buf[8] = {0};
	char basemac_buf[16] = {0};
	unsigned char tmp_buf[3] = {0};
	unsigned int product_state = 0;
	int wait_time = 0;
	uint64_t base_mac = 0;	
	volatile uint64_t tmp_base_mac = 0;
	int i = 0;
	int j = 0;
	uint8_t *base_mac_tbl = NULL;
	volatile uint8_t *tmp_base_mac_tbl = NULL;
	int slot_count = 0;
	if(SE_AGENT_RETURN_FAIL == get_base_mac_table(&base_mac_tbl))
	{
		se_agent_syslog_err("get base mac table failed\n");
		return SE_AGENT_RETURN_FAIL;
	}

	while(product_state < SYSTEM_READY)
	{
	    se_agent_read_from_file(PRODUCT_STATE_PATH,statebuf,3);
	    product_state = atoi(statebuf); 
		if(product_state < SYSTEM_READY)
	    {
	    	se_agent_syslog_err("product not ready. product_state=%d\n", product_state);
	    	sleep(5);
			continue;
	    }
			
	}

	if(SE_AGENT_RETURN_FAIL == se_agent_read_from_file(SLOT_COUNT_PATH,slot_count_buf,7))
	{
		se_agent_syslog_err( "read slot count failed\n");
		return SE_AGENT_RETURN_FAIL;
	}
	slot_count = atoi(slot_count_buf);
	
	if(SE_AGENT_RETURN_FAIL == se_agent_read_from_file(BASE_MAC_PATH,basemac_buf,13))
	{
		se_agent_syslog_err( "read base mac failed\n");
		return SE_AGENT_RETURN_FAIL;
	}
	base_mac = strtoull(basemac_buf,NULL,16);
	cvmx_spinlock_lock(&base_mac_lock);
	for(i = 0;i < slot_count ;i++)
	{
		tmp_base_mac = (base_mac + i);
		tmp_base_mac_tbl = (base_mac_tbl + 6*i);
		for(j=0;j<3;j++)
		{
			*((uint16_t*)tmp_base_mac_tbl+j) = *((uint16_t*)&tmp_base_mac + j + 1);
		}
		CVMX_SYNCWS;
	}
	cvmx_spinlock_unlock(&base_mac_lock);
	return SE_AGENT_RETURN_OK;
}


char* detect_ipfwd_learn_module_name(void)
{
    FILE *file = NULL;
    file = fopen("/sys/module/ipfwd_learn/parameters/learn_enable","r");
    if(file !=NULL)
    {
        fclose(file);
        return "ipfwd_learn";
    }

    file = fopen("/sys/module/ipfwd_learn_coexist/parameters/learn_enable","r");
    if(file !=NULL)
    {
        fclose(file);
        return "ipfwd_learn_coexist";
    }

    file = fopen("/sys/module/ipfwd_learn_standalone/parameters/learn_enable","r");
    if(file !=NULL)
    {
        fclose(file);
        return "ipfwd_learn_standalone";
    }

    return NULL;
}



void* se_agent_recv_fccp_thr(void* arg)
{
    eth_hd_r* eth_header = NULL;
    uint16_t proto = 0;
    control_cmd_t* fccp_cmd = NULL;
    
    while(1)
    {
        cvmx_wqe_t* work = cvmx_pow_work_request_sync(CVMX_POW_NO_WAIT);        
        if (work == NULL)        
        {            
            /* Yield to other processes since we don't have anything to do */            
            usleep(0);
            continue;        
        }

        if(14 == cvmx_wqe_get_grp(work))
		{	
			eth_header = (eth_hd_r *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
			proto = eth_header->h_vlan_proto;
			if(proto == FCCP_L2_ETH_TYPE)
			{
				fccp_cmd = (control_cmd_t *)((uint8_t*)eth_header + 14);
				switch(fccp_cmd->dest_module)
				{
					case FCCP_MODULE_AGENT_ACL:
						se_agent_send_msg((char*)fccp_cmd,sizeof(control_cmd_t));
						break;
					/* se fccp se_agent debug info */
					case FCCP_MODULE_AGENT_DBG:
					    se_agent_syslog_err("fwd_debug_se_agent:%s",&(fccp_cmd->fccp_data));
						break;
					case FCCP_MODULE_AGENT_NAT:
					case FCCP_MODULE_AGENT_CAR:
					default:
						se_agent_syslog_err("se_agent_recv_fccp_thr:  module unknown!\n");
						break;
				}				
			}
		}  
#if 0		
		else
		{
            se_agent_syslog_err("se_agent thread recv packet from grp %d\n", cvmx_wqe_get_grp(work));
		}
#endif
        cvmx_helper_free_packet_data(work); /*free data*/                
        cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
    }

    return NULL;
}


int32_t se_agent_send_heartbeat_msg()
{
	se_interative_t *se_buf=NULL;
	int ret = -1;
	int port_index = 0;
    control_cmd_t  fccp_cmd;
    struct timeval overtime;

    if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
    {
        return SE_AGENT_RETURN_FAIL;
    }

    memset(&fccp_cmd, 0, sizeof(control_cmd_t));
	fccp_cmd.dest_module=FCCP_MODULE_ACL;
	fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	fccp_cmd.cmd_opcode=FCCP_CMD_TEST; 
    fccp_cmd.cmd_len = sizeof(control_cmd_t);
    fccp_cmd.agent_pid = getpid();
    se_agent_clear_msg();

    /* notice: use grp 0 to send, not grp 2 ! */
    if(SE_AGENT_RETURN_FAIL == (send_fccp(&fccp_cmd,0, 0)))
    {
        return SE_AGENT_RETURN_FAIL;
    }

    memset(&fccp_cmd, 0, sizeof(control_cmd_t));
    overtime.tv_sec=1;
    if(se_agent_recv_msg((char*)(&fccp_cmd),sizeof(control_cmd_t),&overtime) < 0)
    {
        return SE_AGENT_RETURN_FAIL;
    }

    if((fccp_cmd.cmd_opcode == FCCP_CMD_TEST) && (fccp_cmd.ret_val == FCCP_RETURN_OK))
    {
        return SE_AGENT_RETURN_OK;
    }
    else
    {
        return SE_AGENT_RETURN_FAIL;
    }
}


/*2013.2.28   Tell fastfwd  to clear the aged rules*/
int32_t se_agent_send_clear_aged_msg()
{
	se_interative_t *se_buf=NULL;
	int ret = -1;
	int port_index = 0;
    control_cmd_t  fccp_cmd;

    if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
    {
        return SE_AGENT_RETURN_FAIL;
    }

    memset(&fccp_cmd, 0, sizeof(control_cmd_t));
	fccp_cmd.dest_module=FCCP_MODULE_ACL;
	fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	fccp_cmd.cmd_opcode=FCCP_CMD_CLEAR_AGING_RULE; 
    fccp_cmd.cmd_len = sizeof(control_cmd_t);

    /* notice: use grp 0 to send, not grp 2 ! */
    if(SE_AGENT_RETURN_FAIL == (send_fccp(&fccp_cmd, 0, 0)))
    {
        return SE_AGENT_RETURN_FAIL;
    }

    return SE_AGENT_RETURN_OK;
}
/* heartbeat thread. every 5 second send an heartbeat fccp. 3times fail, disable fastfwd */
void* se_agent_heartbeat_thr(void* arg)
{
    uint32_t count = 0;
    
    while(1)
    {
        sleep(5);
        count++;
        
        if((0 != clr_aging_interval) && (count >= clr_aging_interval/5))
        {
            if(SE_AGENT_RETURN_FAIL == se_agent_send_clear_aged_msg())
                se_agent_syslog_err("se_agent_clear_aged_rule_thr fail\n"); 
            count = 0;
        }
        
        if(SE_AGENT_RETURN_FAIL == se_agent_send_heartbeat_msg())
        {
            se_agent_syslog_err("fastfwd heartbeat fail!\n");
            heartbeat_fail_counter++;
            if(heartbeat_fail_counter > MAX_HEARTBEAT_FAIL_TIMES)
            {
                heartbeat_fail_counter = MAX_HEARTBEAT_FAIL_TIMES;
            }
        }
        else
        {
            if(heartbeat_fail_counter >= MAX_HEARTBEAT_FAIL_TIMES)
            {
                se_agent_syslog_err("fastfwd heartbeat recover, enable fastfwd!\n");
                set_fastfwd_enable(FUNC_ENABLE);
            }
            heartbeat_fail_counter = 0;
        }

        if(heartbeat_fail_counter == MAX_HEARTBEAT_FAIL_TIMES)
        {
            if(get_fastfwd_stats() == FUNC_ENABLE)
            {
                se_agent_syslog_err("fastfwd heartbeat fail for 3 times, disable fastfwd!\n");
                set_fastfwd_enable(FUNC_DISABLE);
                
                /* release all wqes block in sso grp */
                cvmx_pow_set_group_mask(cvmx_get_core_num(),0xfff);
                sleep(2);
                cvmx_pow_set_group_mask(cvmx_get_core_num(),(1<<14));
            }
        }
    }
    
    return NULL;
}


/**********************************************************************************
 *  se_agent_send_timesync_msg
 *
 *	DESCRIPTION:
 * 		se_agent_send_timesync_msg
 *		send linux sec (1970-now) to fastfwd for fastfwd use 
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		SE_AGENT_RETURN_OK    :time sync sucess
 *		SE_AGENT_RETURN_FAIL  :time sync fail
 *				
 **********************************************************************************/
int32_t se_agent_send_timesync_msg()
{
	se_interative_t *se_buf=NULL;
    control_cmd_t  fccp_cmd;
    struct timeval overtime;
	struct timeval tv;

    if(FASTFWD_NOT_LOADED == (fast_forward_module_load_check()))
    {
        return SE_AGENT_RETURN_FAIL;
    }

    memset(&fccp_cmd, 0, sizeof(control_cmd_t));
	gettimeofday(&tv, 0);
	//do_gettimeofday(&tv);  ///opt/lib/libdcli.so.0: undefined symbol: do_gettimeofdayRun without /opt/lib/libdcli.so.0
	fccp_cmd.fccp_data.time_sec = tv.tv_sec;//dont consider less hw time error + 51 * 60 + 20;	 /*gettimeofday less 51min around*/
	
	fccp_cmd.dest_module=FCCP_MODULE_ACL;
	fccp_cmd.src_module=FCCP_MODULE_AGENT_ACL;
	fccp_cmd.cmd_opcode=FCCP_CMD_TIMESYNC; 
    fccp_cmd.cmd_len = sizeof(control_cmd_t);
    fccp_cmd.agent_pid = getpid();
    se_agent_clear_msg();

    /* notice: use grp 0 to send, not grp 2 ! */
    if(SE_AGENT_RETURN_FAIL == (send_fccp(&fccp_cmd,0, 0)))
    {
        return SE_AGENT_RETURN_FAIL;
    }

    memset(&fccp_cmd, 0, sizeof(control_cmd_t));
    overtime.tv_sec=1;
    if(se_agent_recv_msg((char*)(&fccp_cmd),sizeof(control_cmd_t),&overtime) < 0)
    {
        return SE_AGENT_RETURN_FAIL;
    }

    if((fccp_cmd.cmd_opcode == FCCP_CMD_TIMESYNC) && (fccp_cmd.ret_val == FCCP_RETURN_OK))
    {
        return SE_AGENT_RETURN_OK;
    }
    else
    {
        return SE_AGENT_RETURN_FAIL;
    }
}


/**********************************************************************
 * Main entry point
 *
 * @return exit code
 **********************************************************************/
int main()
{
	cvmx_sysinfo_t *sysinfo = NULL;
	struct sockaddr_tipc client_addr;
	socklen_t cliaddr_len = 0;
	int sockfd = -1 ;
	int nready = -1;
	fd_set rset;	
	char buf[BUF_LEN] = {0};	
	int  maxfd = 0;	
	int receive_len = -1;
	int wait_time = 0;
	int err = -1;
	
	cvmx_user_app_init();
	sysinfo = cvmx_sysinfo_get();

	if(FASTFWD_LOADED == (fast_forward_module_load_check()))
	{
    /* Have one core do the hardware initialization */
	    if (cvmx_coremask_first_core(sysinfo->core_mask))
	    {        	
	        se_agent_syslog_dbg("Waiting for ethernet module to complete initialization...\n\n\n");
	        cvmx_ipd_ctl_status_t ipd_reg;
	        do
	        {        	
	            ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
				wait_time++;
				usleep(1000);
			} while ((!ipd_reg.s.ipd_en) && (wait_time<100));
			find_fastfwd_table();
			cvmx_spinlock_init(&base_mac_lock);
			if(SE_AGENT_RETURN_FAIL == se_agent_get_mac_addr())
			{
				se_agent_get_mac_addr();
			}

			/* se_agent in core 0. set se_agent can receive fccp pkt from grp 14 directly */
            cvmx_pow_set_group_mask(cvmx_get_core_num(),(1<<14));
            err = pthread_create(&ntid, NULL, se_agent_recv_fccp_thr, NULL);
            
			/* setup heartbeat thread. */
			/* if heartbeat fail for 3 times. disable fastfwd */
			err = pthread_create(&heartbeat_tid, NULL, se_agent_heartbeat_thr, NULL);
	 	}		
	}
	CVMX_SYNCWS;
	if (cvmx_coremask_first_core(sysinfo->core_mask))
	{      
		if(SE_AGENT_RETURN_FAIL == se_agent_cmd_func_table_init())
		{
			se_agent_syslog_err("se_agent init command function table error\n");
			return SE_AGENT_RETURN_FAIL;
		}
				
		sockfd = se_agent_create_socket();
		if(sockfd < 0)
		{
			se_agent_syslog_err("server socket create failed\n");
			return SE_AGENT_RETURN_FAIL;
		}

        /* add for muti-core */
		ipfwd_learn_name = detect_ipfwd_learn_module_name(); 

		se_agent_init_msg();

		if(se_agent_pci_channel_init() == SE_AGENT_RETURN_OK)
		{
            agent_pcie_channel_exist = AGENT_TRUE;
            se_agent_syslog_err("se_agent agent_pcie_channel_exist true!\n");
		}
		else
		{   
		    agent_pcie_channel_exist = AGENT_FALSE;
            se_agent_syslog_err("se_agent agent_pcie_channel_exist false!\n");
		}
	}

    cvmx_coremask_barrier_sync(sysinfo->core_mask);
		
	/* fast-fwd group add for  logsave */
	if (SE_AGENT_RETURN_FAIL == se_agent_send_timesync_msg())
	{
		se_agent_syslog_err("se_agent to fastfwd time sync fail!\n");
	}
	
	maxfd = sockfd;
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	while(1)
	{
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if(nready < 0)
		{
			se_agent_syslog_err("main select error\n");
			close(sockfd);
			return SE_AGENT_RETURN_FAIL;
		}		  

		if(FD_ISSET(sockfd, &rset))
		{
			memset(&client_addr,0,sizeof(client_addr));
			cliaddr_len=sizeof(client_addr);
			receive_len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &cliaddr_len);
			if(receive_len < 0)
			{
				se_agent_syslog_err("receive from dcli error\n");
				continue;
			}
			se_agent_cmd_handle(buf,&client_addr,cliaddr_len);	
		}
	}
	se_agent_syslog_err("se_agent close sockfd\n");
	close(sockfd);
	return 0;
}

