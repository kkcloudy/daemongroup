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
*fwd_debug.c
*
*CREATOR:
*   <zhaohan@autelan.com>
*
*DESCRIPTION:
*     dump fast-fwd log to memory for each core
*
*DATE:
*   09/20/2012	
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-fpa.h"
#include "cvmx-pip.h"
#include "cvmx-ipd.h"
#include "cvmx-pko.h"
#include "cvmx-dfa.h"
#include "cvmx-pow.h"
#include "cvmx-gmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"
#include "cvmx-malloc.h"
#include "cvmx-atomic.h"

#include "fastfwd-common-defs.h"

#include "fwd_debug.h"

#include "acl.h"
#include "fccp.h"


#define FASTFWD_LOG_MEM_NAME    "fastfwd_log_core%d"

CVMX_SHARED char *log_mem_base[32];
CVMX_SHARED uint64_t log_mem_size = 1024*1024; /* 2M to 1M to fit 256m memory on AX71_CRSMU, code modified by caojia */
uint64_t log_mem_offset = 0; /* local, do not use CVMX_SHARED */
CVMX_SHARED int32_t fwd_debug_log_enable = FUNC_ENABLE;
CVMX_SHARED int32_t fwd_debug_logtime_enable = FUNC_DISABLE;


extern CVMX_SHARED uint64_t linux_sec;
extern CVMX_SHARED uint64_t fwd_sec;


#define KES_MEM_BLOCK_SIZE (1 * 1024 * 1024)
#define HALF_KES_MEM_BLOCK_SIZE (512 * 1024)
#define QUARTER_KES_MEM_BLOCK_SIZE (256 * 1024)

#define KES_MEM_BLOCK_NAME "__kes_mem"
CVMX_SHARED char *kes_mem_addr_fastfwd = NULL;
CVMX_SHARED int32_t kes_mem_offset_fastfwd = (HALF_KES_MEM_BLOCK_SIZE + QUARTER_KES_MEM_BLOCK_SIZE);  


extern CVMX_SHARED product_info_t product_info;
extern CVMX_SHARED int cvm_ip_only_enable;


#define FCCP_DEFAULT_TAG        456

int fwd_debug_packet_to_log(cvmx_wqe_t *work)
{
    uint64_t        count;
    uint64_t        remaining_bytes;
    cvmx_buf_ptr_t  buffer_ptr;
    uint64_t        start_of_buffer;
    uint8_t *       data_address;
    uint8_t *       end_of_data;

    fwd_debug_printf("Packet Length:   %u\n", cvmx_wqe_get_len(work));
    fwd_debug_printf("    Input Port:  %u\n", cvmx_wqe_get_port(work));
    fwd_debug_printf("    QoS:         %u\n", cvmx_wqe_get_qos(work));
    fwd_debug_printf("    Buffers:     %u\n", work->word2.s.bufs);

    if (work->word2.s.bufs == 0)
    {
        cvmx_ipd_wqe_fpa_queue_t wqe_pool;
        wqe_pool.u64 = cvmx_read_csr(CVMX_IPD_WQE_FPA_QUEUE);
        buffer_ptr.u64 = 0;
        buffer_ptr.s.pool = wqe_pool.s.wqe_pool;
        buffer_ptr.s.size = 128;
        buffer_ptr.s.addr = cvmx_ptr_to_phys(work->packet_data);
        if (cvmx_likely(!work->word2.s.not_IP))
        {
            cvmx_pip_ip_offset_t pip_ip_offset;
            pip_ip_offset.u64 = cvmx_read_csr(CVMX_PIP_IP_OFFSET);
            buffer_ptr.s.addr += (pip_ip_offset.s.offset<<3) - work->word2.s.ip_offset;
            buffer_ptr.s.addr += (work->word2.s.is_v6^1)<<2;
        }
        else
        {
            /* WARNING: This code assume that the packet is not RAW. If it was,
                we would use PIP_GBL_CFG[RAW_SHF] instead of
                PIP_GBL_CFG[NIP_SHF] */
            cvmx_pip_gbl_cfg_t pip_gbl_cfg;
            pip_gbl_cfg.u64 = cvmx_read_csr(CVMX_PIP_GBL_CFG);
            buffer_ptr.s.addr += pip_gbl_cfg.s.nip_shf;
        }
    }
    else
        buffer_ptr = work->packet_ptr;
    remaining_bytes = cvmx_wqe_get_len(work);

    while (remaining_bytes)
    {
        start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
        fwd_debug_printf("    Buffer Start:%llx\n", (unsigned long long)start_of_buffer);
        fwd_debug_printf("    Buffer I   : %u\n", buffer_ptr.s.i);
        fwd_debug_printf("    Buffer Back: %u\n", buffer_ptr.s.back);
        fwd_debug_printf("    Buffer Pool: %u\n", buffer_ptr.s.pool);
        fwd_debug_printf("    Buffer Data: %llx\n", (unsigned long long)buffer_ptr.s.addr);
        fwd_debug_printf("    Buffer Size: %u\n", buffer_ptr.s.size);

        fwd_debug_printf("\t\t");
        data_address = (uint8_t *)cvmx_phys_to_ptr(buffer_ptr.s.addr);
        end_of_data = data_address + buffer_ptr.s.size;
        count = 0;
        while (data_address < end_of_data)
        {
            if (remaining_bytes == 0)
                break;
            else
                remaining_bytes--;
            fwd_debug_printf("%02x", (unsigned int)*data_address);
            data_address++;
            if (remaining_bytes && (count == 7))
            {
                fwd_debug_printf("\n\t\t");
                count = 0;
            }
            else
                count++;
        }
        fwd_debug_printf("\n");

        if (remaining_bytes)
            buffer_ptr = *(cvmx_buf_ptr_t*)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
    }
    return 0;
}


int fwd_debug_dump_packet(cvmx_wqe_t *work)
{
    if(FUNC_ENABLE == fwd_debug_log_enable)
        return fwd_debug_packet_to_log(work);
    else
        return cvmx_helper_dump_packet(work);
}

/*********************print the rule to /proc/fastfwd_log_proc start**************************
  ***********************refer to the shell_common.c print_rule function*********************/
static void print_rpa_head(rule_item_t* rule)
{
	fwd_debug_printf("rap header :\n");
	fwd_debug_printf("rpa_type = %u,rpa_d_s_slotNum = %u\n",rule->rules.rpa_header.rpa_type,\
		rule->rules.rpa_header.rpa_d_s_slotNum);
	fwd_debug_printf("rpa_dnetdevNum = %u,rpa_snetdevNum = %u\n",rule->rules.rpa_header.rpa_dnetdevNum,\
		rule->rules.rpa_header.rpa_snetdevNum);
}
static void print_icmp_rule(rule_item_t* rule)
{
	if(rule == NULL)
		return;

	if(rule->rules.protocol != 1)
	{
		fwd_debug_printf("print_icmp_rule error: not icmp rule!\n");
		return;
	}
	fwd_debug_printf("  five tuple: %d.%d.%d.%d => %d.%d.%d.%d    icmp\n",
			IP_FMT(rule->rules.sip), IP_FMT(rule->rules.dip));

	switch(rule->rules.action_type)
	{
		case FLOW_ACTION_ICMP:
			fwd_debug_printf("  action_type = FLOW_ACTION_ICMP\n");
			break;
		case FLOW_ACTION_CAPWAP_802_11_ICMP:
			fwd_debug_printf("  action_type = FLOW_ACTION_CAPWAP_802_11_ICMP\n");
			break;
		case FLOW_ACTION_CAPWAP_802_3_ICMP:
			fwd_debug_printf("  action_type = FLOW_ACTION_CAPWAP_802_3_ICMP\n");
			break;
		case FLOW_ACTION_RPA_ICMP:
			fwd_debug_printf("  action_type = FLOW_ACTION_RPA_ICMP\n");
			print_rpa_head(rule);
			break;
		case FLOW_ACTION_RPA_CAPWAP_802_11_ICMP:
			fwd_debug_printf("  action_type = FLOW_ACTION_RPA_CAPWAP_802_11_ICMP\n");
			print_rpa_head(rule);
			break;
		case FLOW_ACTION_RPA_CAPWAP_802_3_ICMP:
			fwd_debug_printf("  action_type = FLOW_ACTION_RPA_CAPWAP_802_3_ICMP\n");
			print_rpa_head(rule);
			break;
		default:
			fwd_debug_printf("  action_type = unknow\n");
			break;
	}
	fwd_debug_printf("  forward port = %d\n", rule->rules.forward_port);
}

int fwd_debug_rule_to_log(rule_item_t  *rule)
{
	if(rule == NULL)
	{
		return 0;
	}
	
	if((rule->rules.action_type == FLOW_ACTION_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_CAPWAP_802_11_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_CAPWAP_802_3_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_RPA_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_11_ICMP) ||
			(rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_3_ICMP))
	{
		print_icmp_rule(rule);
		return 0;
	}

	if(cvm_ip_only_enable == FUNC_ENABLE)
	{
		fwd_debug_printf("  five tuple: any-ip:any-port => %d.%d.%d.%d:any-port   any(tcp|udp)\n",
				IP_FMT(rule->rules.dip));
	}
	else
	{
		fwd_debug_printf("  five tuple: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d    %s\n",
				IP_FMT(rule->rules.sip), rule->rules.sport,
				IP_FMT(rule->rules.dip), rule->rules.dport, PROTO_STR(rule->rules.protocol));	
	}

	if(rule->rules.rule_state == RULE_IS_LEARNING)
	{
		fwd_debug_printf("  rule_state = LEARNING\t");
		if(acl_aging_check(&(rule->rules)) > 0)
			fwd_debug_printf("age\n");
		else
			fwd_debug_printf("new\n");
		return 0;
	}

	if(rule->rules.action_type == FLOW_ACTION_DROP)
	{
		fwd_debug_printf("  action_type = FLOW_ACTION_DROP\n");
		return 0;
	}
	if(rule->rules.action_type == FLOW_ACTION_TOLINUX)
	{
		fwd_debug_printf("  action_type = FLOW_ACTION_TOLINUX\n");
		return 0;
	}

	fwd_debug_printf("  smac: %02x-%02x-%02x-%02x-%02x-%02x", MAC_FMT(rule->rules.ether_shost));
	fwd_debug_printf("  dmac: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_FMT(rule->rules.ether_dhost));
	fwd_debug_printf("  eth protocol: %04x\n", rule->rules.ether_type);

	switch(rule->rules.action_type)
	{
		case FLOW_ACTION_ETH_FORWARD:
			fwd_debug_printf("  action_type = FLOW_ACTION_ETH_FORWARD\n");
			break;
		case FLOW_ACTION_RPA_ETH_FORWARD:
			fwd_debug_printf("  action_type = FLOW_ACTION_RPA_ETH_FORWARD\n");
			print_rpa_head(rule);
			break;
		case FLOW_ACTION_CAP802_3_FORWARD:
		case FLOW_ACTION_RPA_CAP802_3_FORWARD:
			fwd_debug_printf("    tunnel_index: %d\n", rule->rules.tunnel_index);
			fwd_debug_printf("    capwap use_num = %d\n", capwap_cache_bl[rule->rules.tunnel_index].use_num);
			fwd_debug_printf("    capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x\n",
					IP_FMT(capwap_cache_bl[rule->rules.tunnel_index].sip), capwap_cache_bl[rule->rules.tunnel_index].sport,
					IP_FMT(capwap_cache_bl[rule->rules.tunnel_index].dip), capwap_cache_bl[rule->rules.tunnel_index].dport,  
					capwap_cache_bl[rule->rules.tunnel_index].tos);
			if(rule->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD)
			{
				fwd_debug_printf("  action_type = FLOW_ACTION_CAP802_3_FORWARD\n");
			}
			else if(rule->rules.action_type == FLOW_ACTION_RPA_CAP802_3_FORWARD)
			{
				fwd_debug_printf("  action_type = FLOW_ACTION_RPA_CAP802_3_FORWARD\n");
				print_rpa_head(rule);
			}
			break;
		case FLOW_ACTION_CAPWAP_FORWARD:
		case FLOW_ACTION_RPA_CAPWAP_FORWARD:
			
			fwd_debug_printf("    tunnel_index: %d\n", rule->rules.tunnel_index);
			fwd_debug_printf("    capwap use_num = %d\n", capwap_cache_bl[rule->rules.tunnel_index].use_num);
			fwd_debug_printf("    capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x\n",
					IP_FMT(capwap_cache_bl[rule->rules.tunnel_index].sip), capwap_cache_bl[rule->rules.tunnel_index].sport,
					IP_FMT(capwap_cache_bl[rule->rules.tunnel_index].dip), capwap_cache_bl[rule->rules.tunnel_index].dport,  
					capwap_cache_bl[rule->rules.tunnel_index].tos);
			if(rule->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD)
			{
				fwd_debug_printf("  action_type = FLOW_ACTION_CAPWAP_FORWARD\n");
			}
			else if(rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_FORWARD)
			{
				fwd_debug_printf("  action_type = FLOW_ACTION_RPA_CAPWAP_FORWARD\n");
				print_rpa_head(rule);
			}
			break;
		default:
			fwd_debug_printf("  action_type = UNKNOWN\n");
			break;
	}
	fwd_debug_printf("  forward port = %d\n", rule->rules.forward_port);
	fwd_debug_printf("  time stamp: %lu\n", rule->rules.time_stamp);
	if(rule->rules.rule_state == RULE_IS_STATIC)
	{
		fwd_debug_printf("  rule_state = STATIC\n");
	}
	else
	{
		if(rule->rules.rule_state == RULE_IS_LEARNED)
		{
			fwd_debug_printf("  rule_state = LEARNED\t");
		}
		if(acl_aging_check(&(rule->rules)) > 0)
			fwd_debug_printf("age\n");
		else
			fwd_debug_printf("new\n");
	}
	fwd_debug_printf("dsa_info: 0x%08lx\n", rule->rules.dsa_info);
	fwd_debug_printf("out_type:0x%02x   out_tag:0x%02x   in_type:0x%02x   in_tag:0x%02x\n", rule->rules.out_ether_type, rule->rules.out_tag, rule->rules.in_ether_type, rule->rules.in_tag);
	fwd_debug_printf("packet_wait = %d\n", rule->rules.packet_wait);
	fwd_debug_printf("extend_index = 0x%lx\n", (int64_t)(rule->rules.extend_index));
	fwd_debug_printf("action mask = 0x%x\n", rule->rules.action_mask);
	switch (rule->rules.direct_flag)
	{
		case DIRECTION_UP:
			fwd_debug_printf("direct :up\n");
			break;
		case DIRECTION_DOWN:
			fwd_debug_printf("direct :down\n");
			break;
		case DIRECTION_UP_DOWN:
			fwd_debug_printf("direct :up-down\n");
			break;
		default:
			fwd_debug_printf("direct :undefine\n");
			break;
	}
	

	// TODO: print user info
	return 0;
}

int fwd_debug_dump_rule(rule_item_t  *rule)
{
    if(FUNC_ENABLE == fwd_debug_log_enable)
    {
        return fwd_debug_rule_to_log(rule);
    }
    else
    {
        return 0;//cvmx_helper_dump_rule(rule);   dont support print to serial 
    }
}

/*****************print the rule to /proc/fastfwd_log_proc end**************************************/

int fwd_debug_printf(const char * fmt,...)
{
    char* fwd_log_memory = NULL;
	int msg_size = 0;
	uint64_t input_len = 0;
	va_list args;
	
    fwd_log_memory = log_mem_base[cvmx_get_core_num()];  

    if(fwd_log_memory == NULL)
        return RETURN_ERROR;

	input_len = strlen(fmt);
	if(input_len > (log_mem_size - log_mem_offset))
	{
		log_mem_offset = 0;
	}

	va_start(args, fmt);
	msg_size = vsnprintf((char *)(fwd_log_memory + log_mem_offset), 
		log_mem_size - log_mem_offset, fmt, args);
	va_end(args);
	
	log_mem_offset += msg_size;

	return msg_size;
}


extern int32_t return_fccp(cvmx_wqe_t* work, uint32_t ret_val, fccp_data_t* fccp_data, uint8_t grp);


int fwd_debug_agent_printf(const char * fmt,...)
{
	int msg_size = 0;
	uint64_t input_len = 0;
	va_list args;
	void *pkt_buffer = NULL;
	void *pkt_ptr = NULL;
	void *tmp_ptr = NULL;
	control_cmd_t  *fccp_cmd = NULL;
	
	input_len = strlen(fmt);

	if(input_len > sizeof(fccp_data_t)) 
	{
		return RETURN_ERROR;
	}

	cvmx_wqe_t *work = cvmx_fpa_alloc(CVMX_FPA_WQE_POOL);
	if (NULL == work)    
	{		
		return RETURN_ERROR;
	}
	memset(work, 0, CVMX_FPA_WQE_POOL_SIZE); 

	pkt_buffer = cvmx_fpa_alloc(CVMX_FPA_PACKET_LOG_POOL);
	if (NULL == pkt_buffer)    
	{		
		cvmx_fpa_free(work,CVMX_FPA_WQE_POOL,0);
		return RETURN_ERROR;
	}
	memset(pkt_buffer, 0, CVMX_FPA_PACKET_LOG_POOL_SIZE); 	
	pkt_ptr = pkt_buffer + sizeof(uint64_t);
	pkt_ptr += ((CVMX_HELPER_FIRST_MBUFF_SKIP+7)&0xfff8) + 6;
	tmp_ptr = pkt_ptr;
	tmp_ptr += MAC_LEN*2 ;
	*((uint16_t *)tmp_ptr) = FCCP_L2_ETH_TYPE;
	tmp_ptr += sizeof(uint16_t);
	fccp_cmd = tmp_ptr;
	fccp_cmd->cmd_len = sizeof(control_cmd_t);
	//fccp_cmd.agent_pid = getpid();     /*2.0 not use, 1.3 maybe need pid & mac */

	/* return_fccp will swap module */
	fccp_cmd->dest_module=FCCP_MODULE_ACL;
	fccp_cmd->src_module=FCCP_MODULE_AGENT_DBG;   /*only for debug print*/

	va_start(args, fmt);
		msg_size = vsnprintf((char *)(&(fccp_cmd->fccp_data)), sizeof(fccp_data_t), fmt, args);
	va_end(args);

	CVM_WQE_SET_LEN(work,ETH_H_LEN + sizeof(control_cmd_t));
	CVM_WQE_SET_PORT(work,0);
	CVM_WQE_SET_TAG_TYPE(work, CVMX_POW_TAG_TYPE_NULL);
	CVM_WQE_SET_TAG(work, FCCP_DEFAULT_TAG);
	/*QOS GRP dont need return_fccp have*/
	work->word2.u64     = 0;    	/* Default to zero. Sets of zero later are commented out */
	work->word2.s.bufs  = 1;
	work->packet_ptr.u64 = 0;
	work->packet_ptr.s.addr = cvmx_ptr_to_phys(pkt_ptr);

	work->packet_ptr.s.pool = CVMX_FPA_PACKET_LOG_POOL;
	work->packet_ptr.s.size = CVMX_FPA_PACKET_LOG_POOL_SIZE;
	work->packet_ptr.s.back = (pkt_ptr - pkt_buffer)>>7; 
	work->word2.snoip.not_IP = 1; 	/* IP was done up above */
	
	return_fccp(work, FCCP_RETURN_OK, &(fccp_cmd->fccp_data), product_info.to_linux_fccp_group);

	return msg_size;
}



/* fast-fwd group add for exception_msg logsave */

/********************************************************************** 
Function : 
	kes_mem_addr_get
Description : 
	get kes mem the uboot alloced
Input : 
	void
Output : 
	kes_mem_addr
Return :
	kes_mem pointer
Others : 
	none
**********************************************************************/
static void *kes_mem_addr_get_fastfwd(void)
{
	
	kes_mem_offset_fastfwd = HALF_KES_MEM_BLOCK_SIZE + QUARTER_KES_MEM_BLOCK_SIZE;
	const cvmx_bootmem_named_block_desc_t *kes_mem_desc = NULL;

	kes_mem_desc = cvmx_bootmem_find_named_block(KES_MEM_BLOCK_NAME);
	if(NULL == kes_mem_desc)
	{
		printf("fwd kes_mem get addr error.\n");
		return NULL;
	}

	kes_mem_addr_fastfwd = (char *)cvmx_phys_to_ptr(kes_mem_desc->base_addr);

	/*get the offset after lasttime*/
	while (0 != *(kes_mem_addr_fastfwd + kes_mem_offset_fastfwd))
	{
		kes_mem_offset_fastfwd++;
	}

	if ((kes_mem_offset_fastfwd >= KES_MEM_BLOCK_SIZE) || (kes_mem_offset_fastfwd == (HALF_KES_MEM_BLOCK_SIZE + QUARTER_KES_MEM_BLOCK_SIZE)))
	{
		kes_mem_offset_fastfwd = (HALF_KES_MEM_BLOCK_SIZE + QUARTER_KES_MEM_BLOCK_SIZE);
		memset(kes_mem_addr_fastfwd + kes_mem_offset_fastfwd,0,QUARTER_KES_MEM_BLOCK_SIZE);
	}
	
	return kes_mem_addr_fastfwd;
	
}

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

struct sec_time {
	int32_t tm_sec;
	int32_t tm_min;
	int32_t tm_hour;
	int32_t tm_mday;
	int32_t tm_mon;
	int32_t tm_year;
	int32_t tm_wday;
	int32_t tm_yday;
	int32_t tm_isdst;
};

static uint32_t is_leap_year(int32_t year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

static const unsigned char rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};


static int rtc_month_days(int32_t month, int32_t year)
{
	if (is_leap_year(year) && (month == 1))
	{
		return rtc_days_in_month[month] + 1;
	}
	else 
	{
		return rtc_days_in_month[month];
	}
}


/*
 * Convert seconds since 01-01-1970 00:00:00 to Gregorian date.
 */
void sec_to_tm(uint64_t time, struct sec_time *tm)
{
	int32_t month, year;
	int32_t days;

	days = time / 86400;
	//time -= (unsigned int) days * 86400;
	time -= days * 86400;

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ LEAPS_THRU_END_OF(year - 1)
		- LEAPS_THRU_END_OF(1970 - 1);
	if (days < 0) {
		year -= 1;
		days += 365 + is_leap_year(year);
	}
	tm->tm_year = year - 1900;
	tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int32_t newdays;

		newdays = days - rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	//tm->tm_mon = month;
	tm->tm_mon = month + 1; /*month less one */
	tm->tm_mday = days + 1;

	tm->tm_hour = time / 3600;
	time -= tm->tm_hour * 3600;
	tm->tm_min = time / 60;
	tm->tm_sec = time - tm->tm_min * 60;
}

/*only for print time before log print */
static int32_t print_time(const char * fmt,...)
{
	int32_t msg_size = 0;
	int32_t input_len = 0;
	va_list args;
	input_len = strlen(fmt);

	if (NULL == kes_mem_addr_fastfwd)
	{
		if (NULL == kes_mem_addr_get_fastfwd())
		{
			return 0; /*msg_size 0*/
		}
	}
	if(input_len > (KES_MEM_BLOCK_SIZE  - kes_mem_offset_fastfwd))
	{
		kes_mem_offset_fastfwd = HALF_KES_MEM_BLOCK_SIZE + QUARTER_KES_MEM_BLOCK_SIZE;
		memset(kes_mem_addr_fastfwd + kes_mem_offset_fastfwd,0,QUARTER_KES_MEM_BLOCK_SIZE);
	}

	va_start(args, fmt);
	msg_size = vsnprintf((char *)(kes_mem_addr_fastfwd + kes_mem_offset_fastfwd), KES_MEM_BLOCK_SIZE - kes_mem_offset_fastfwd, fmt, args);
	va_end(args);
	
	kes_mem_offset_fastfwd += msg_size;

	return msg_size;
}

int kes_fastfwd_print(const char * fmt,...)
{
	int32_t msg_size = 0;
	int32_t input_len = 0;
	va_list args;
	input_len = strlen(fmt);

	if (NULL == kes_mem_addr_fastfwd)
	{
		if (NULL == kes_mem_addr_get_fastfwd())
		{
			return 0;  /*msg_size 0*/
		}	
	}
	
	if(input_len > (KES_MEM_BLOCK_SIZE  - kes_mem_offset_fastfwd))
	{
		kes_mem_offset_fastfwd = HALF_KES_MEM_BLOCK_SIZE + QUARTER_KES_MEM_BLOCK_SIZE;
		memset(kes_mem_addr_fastfwd + kes_mem_offset_fastfwd,0,QUARTER_KES_MEM_BLOCK_SIZE);
	}

	/*before print log add print time*/
	struct sec_time tm;
	uint64_t sec = 0;
	if (fwd_debug_logtime_enable)
	{
		sec = linux_sec + (get_sec() - fwd_sec);
		sec_to_tm(sec, &tm);
		if (0 == print_time("[T%d_%d_%d_%d_%d]", tm.tm_mon, tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec))
		{
			return 0;
		}
	}
	va_start(args, fmt);
	msg_size = vsnprintf((char *)(kes_mem_addr_fastfwd + kes_mem_offset_fastfwd), KES_MEM_BLOCK_SIZE - kes_mem_offset_fastfwd, fmt, args);
	va_end(args);
	
	kes_mem_offset_fastfwd += msg_size;

	return msg_size;
}


void fwd_debug_init_global(void)
{
    memset(log_mem_base, 0 , sizeof(log_mem_base));
	kes_mem_addr_get_fastfwd();
}

/* called by each core */
int fwd_debug_init_local(void)
{
    char* fwd_log_memory = NULL;
    char bootmem_name[64];

    memset(bootmem_name, 0, sizeof(bootmem_name));
    sprintf(bootmem_name, FASTFWD_LOG_MEM_NAME, cvmx_get_core_num());

    fwd_log_memory =  (char *) cvmx_bootmem_alloc_named(log_mem_size, 128, bootmem_name);
    if(fwd_log_memory == NULL)
    {
        printf("mem: %s alloc failed!\n", bootmem_name);
        return RETURN_ERROR;
    }
    memset(fwd_log_memory, 0, log_mem_size);
    log_mem_base[cvmx_get_core_num()] = fwd_log_memory;    
    log_mem_offset = 0;
    printf("mem: %s alloc success!\n", bootmem_name);

    return RETURN_OK;
}


void fwd_log_dump(int32_t core_id)
{
    char* log_base = NULL;
    uint64_t i = 0;
    
    if(core_id >= 32)
    {
        printf("fwd_log_dump: core_id error!\n");
        return;
    }    
    log_base = log_mem_base[core_id];
    if(log_base == NULL)
    {
        printf("core%d: log_base is NULL\n", core_id);
        return;
    }

    for(i = 0; i < log_mem_size; i++)
    {
        if(log_base[i] == '\0')
            break;
        printf("%c", log_base[i]);    
    }
}

