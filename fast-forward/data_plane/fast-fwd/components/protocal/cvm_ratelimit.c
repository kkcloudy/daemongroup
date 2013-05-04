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
*cvm_ratelimit.c
*
*CREATOR:
*   <zhaohan@autelan.com>
*
*DESCRIPTION:
*     for cavium rate limit by protocol priority
*
*DATE:
*   08/15/2012	
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
#include "cvm_ratelimit.h"
#include "fastfwd-common-defs.h"

extern CVMX_SHARED int cvm_rate_limit_enabled;

CVMX_SHARED uint64_t cvm_rate_limit_drop_counter = 0;
CVMX_SHARED uint64_t cvm_rate_limit_pass_counter = 0;
CVMX_SHARED int32_t cvm_rate_limit_debug = 0;
CVMX_SHARED uint64_t car_rate_scaled = 0;
CVMX_SHARED uint64_t car_rate_pps_scaled = 0;
CVMX_SHARED int ratelimit_flag = RATELIMIT_PPS;
CVMX_SHARED void* protocol_rate_limiter_mem_base = NULL;
CVMX_SHARED global_various_t * global_various_ptr = NULL;
CVMX_SHARED protocol_match_item_t* protocol_rate_limiter = NULL;
CVMX_SHARED int32_t count_in_one_brust = 0;
CVMX_SHARED uint64_t prev_drop_cycles = 0;

#define CVM_RATE_NO_LIMIT			5000001
#define TP_DFLT_PPS					50000
#define TP_ARP_PPS 					4000
#define TP_VRRP_PPS					800
#define TP_ICMP_PPS					1000
#define TP_CAPWAPC_PPS				50000 /* user control packet */
#define TP_CAPWAPD_D_EAP_PPS		30000 /* user control packet, data frame with eap*/
#define TP_CAPWAPD_D_ARP_PPS		3000 /* user control packet, data frame arp */
#define TP_CAPWAPD_D_ICMP_PPS		1000 /* user control packet, icmp packet */
#define TP_CAPWAPD_D_DHCPS_PPS		500 /* user control packet, data frame dhcp request ,as dhcp server */
#define TP_CAPWAPD_D_TCP_PPS		30000 /* user control packet, tcp */
#define TP_CAPWAPD_D_UDP_PPS		30000 /* user control packet, udp */
#define TP_CAPWAPD_D_PPS			50000 /* user control packet, other packet */
#define TP_CAPWAPD_M_PPS			50000 /* user control packet, manage frame */
#define TP_PORTAL_PPS				25000
#define TP_TELNETS_PPS				15000
#define TP_DHCPS_PPS				500
#define TP_RADIUS_AU_C_PPS			21000 /* RADIUS auth client and acct client */
#define TP_RADIUS_AU_C2_PPS			21000 /* RADIUS auth client 2 */
#define TP_RADIUS_AC_C2_PPS			21000 /* RADIUS acct client 2 */
#define TP_SNMPC_PPS				18000 /* SNMP client */
#define TP_HTTPSS_PPS				30000 /* HTTPS server */
#define TP_TCP_PPS					30000
#define TP_UDP_PPS					30000
#define TP_TIPC_PPS					CVM_RATE_NO_LIMIT

CVMX_SHARED protocol_match_item_t protocol_rate_limiter_template[MAX_MATCH_RULES_NUM] = 
{	/*MATCH_TYPE : unsigned long long */
	{	/*DEFAULT work when no item match */ 
		"DEFAULT",
		{0x0}/*rule*/,
		{0x0}/*mask*/,40960/*rateLimit (kbps)*/,TP_DFLT_PPS/*rateLimit (pps)*/
	},
	{	/*arp request/reply*/
		"ARP",
		{0x0,0x08060000}/*rule*/,
		{0x0,0xffff0000}/*mask*/,10240/*rateLimit (kbps)*/,TP_ARP_PPS/*rateLimit (pps)*/
	},
	{	/*VRRP*/
		"VRRP",
		{0x01005e0000000000,0x08004000,0x00000070}/*rule*/,
		{0xffffff0000000000,0xfffff000,0x000000ff}/*mask*/,10240/*rateLimit (kbps)*/,TP_VRRP_PPS/*rateLimit (pps)*/
	},
	{	/*ICMP*/
		"ICMP",
		{0x0,0x08004000,0x00000001}/*rule*/,
		{0x0,0xfffff000,0x000000ff}/*mask*/,20480/*rateLimit (kbps)*/,TP_ICMP_PPS/*rateLimit (pps)*/
	},
	{	/*CAPWAP control packet (UDP: sport 32768, dport 5246)*/
		"CAPWAP_C",
		{0x0,0x08004500,0x00000011,0x0,0x00008000147e0000}/*rule*/,
		{0x0,0xffffff00,0x000000ff,0x0,0x0000ffffffff0000}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPC_PPS/*rateLimit (pps)*/
		
	},
	{	/*CAPWAP data packet (UDP: sport 32769, dport 5247)*/
		"CAPWAP_D_D_EAP",
		{0x0,0x08004500,0x11,0x0,0x8001147f0000,0x0000002000000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0x888e000000000000/*LLC bit 48-63*/}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffffffff0000,0x0000ffff00000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0xffff000000000000/*LLC bit 48-63*/}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPD_D_EAP_PPS/*rateLimit (pps)*/
	},
	{	/*CAPWAP data packet (UDP: sport 32769, dport 5247)*/
		"CAPWAP_D_D_ARP",
		{0x0,0x08004500,0x11,0x0,0x8001147f0000,0x0000002000000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0x0806000000000000/*LLC bit 48-63*/}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffffffff0000,0x0000ffff00000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0xffff000000000000/*LLC bit 48-63*/}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPD_D_ARP_PPS /*rateLimit (pps)*/
	},
	{	/*CAPWAP data packet (UDP: sport 32769, dport 5247)*/
		"CAPWAP_D_D_ICMP",
		{0x0,0x08004500,0x11,0x0,0x8001147f0000,0x0000002000000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0x0800450000000000/*LLC bit 48-63*/, 0x0100000000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffffffff0000,0x0000ffff00000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0xffffff0000000000/*LLC bit 48-63*/, 0xff00000000}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPD_D_ICMP_PPS /*rateLimit (pps)*/
	},
	{	/*CAPWAP data packet (UDP: sport 32769, dport 5247)*/
		"CAPWAP_D_D_DHCP_request",
		{0x0,0x08004500,0x11,0x0,0x8001147f0000,0x0000002000000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0x0800450000000000/*LLC bit 48-63*/, 0x1100000000, 0x0044, 0x0043000000000000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffffffff0000,0x0000ffff00000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0xffffff0000000000/*LLC bit 48-63*/, 0xff00000000, 0xffff, 0xffff000000000000}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPD_D_DHCPS_PPS /*rateLimit (pps)*/
	},
	{	/*CAPWAP data packet (UDP: sport 32769, dport 5247)*/
		"CAPWAP_D_D_TCP",
		{0x0,0x08004500,0x11,0x0,0x8001147f0000,0x0000002000000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0x0800450000000000/*LLC bit 48-63*/, 0x0600000000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffffffff0000,0x0000ffff00000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0xffffff0000000000/*LLC bit 48-63*/, 0xff00000000}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPD_D_TCP_PPS /*rateLimit (pps)*/
	},
	{	/*CAPWAP data packet (UDP: sport 32769, dport 5247)*/
		"CAPWAP_D_D_UDP",
		{0x0,0x08004500,0x11,0x0,0x8001147f0000,0x0000002000000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0x0800450000000000/*LLC bit 48-63*/, 0x1100000000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffffffff0000,0x0000ffff00000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/,0x0,0x0,0x0,0xffffff0000000000/*LLC bit 48-63*/, 0xff00000000}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPD_D_UDP_PPS /*rateLimit (pps)*/
	},
	{	/*CAPWAP data packet (UDP: sport 32769, dport 5247)*/
		"CAPWAP_D_D",
		{0x0,0x08004500,0x11,0x0,0x8001147f0000,0x0000002000000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffffffff0000,0x0000ffff00000000/*capwap bit18-22*/,0x0,0x0/*802.11 bit2-4*/}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPD_D_PPS /*rateLimit (pps)*/
	},
	{	/*CAPWAP data packet (UDP: sport 32769, dport 5247)*/
		"CAPWAP_D_M",
		{0x0,0x08004500,0x11,0x0,0x8001147f0000,0x02000000/*capwap bit18-22*/,0x0,0x000000000000/*802.11 bit2-3*/}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffffffff0000,0x3e000000/*capwap bit18-22*/,0x0,0x300000000000/*802.11 bit2-3*/}/*mask*/,10240/*rateLimit (kbps)*/,TP_CAPWAPD_M_PPS/*rateLimit (pps)*/
	},
	{	/*PORTAL (UDP: dport 2000) */
		"PORTAL",
		{0x0,0x08004500,0x00000011,0x0,0x0000000007d00000}/*rule*/,
		{0x0,0xffffff00,0x000000ff,0x0,0x00000000ffff0000}/*mask*/,10240/*rateLimit (kbps)*/,TP_PORTAL_PPS/*rateLimit (pps)*/
	},
	{	/*TELNET server and SSH server*/
		"TELNET_SSH_Request",
		{0x0,0x08004500,0x06,0x0,0x00160000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xfffe0000}/*mask*/,10240/*rateLimit (kbps)*/,TP_TELNETS_PPS/*rateLimit (pps)*/
	},
	{	/*DHCP server*/
		"DHCP_Request",
		{0x0,0x08004500,0x00000011,0x0,0x0000004400430000}/*rule*/,
		{0x0,0xffffff00,0x000000ff,0x0,0x0000ffffffff0000}/*mask*/,10240/*rateLimit (kbps)*/,TP_DHCPS_PPS/*rateLimit (pps)*/
	},
	{	/*RADIUS auth client and RADIUS acct client*/
		"RADIUS_auth_acct_client",
		{0x0,0x08004500,0x11,0x0,0x071400000000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xfffe00000000}/*mask*/,10240/*rateLimit (kbps)*/,TP_RADIUS_AU_C_PPS/*rateLimit(pps)*/
	},
	{	/*RADIUS auth client and RADIUS acct client*/
		"RADIUS_auth_client2",
		{0x0,0x08004500,0x11,0x0,0x066d00000000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffff00000000}/*mask*/,10240/*rateLimit (kbps)*/,TP_RADIUS_AU_C2_PPS/*rateLimit(pps)*/
	},
	{	/*RADIUS auth client and RADIUS acct client*/
		"RADIUS_acct_client2",
		{0x0,0x08004500,0x11,0x0,0x066e00000000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0xffff00000000}/*mask*/,10240/*rateLimit (kbps)*/,TP_RADIUS_AC_C2_PPS/*rateLimit(pps)*/
	},
	{	/*RADIUS auth client and RADIUS acct client*/
		"SNMP_client",
		{0x0,0x08004500,0x11,0x0,0x000000a10000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0x0000ffff0000}/*mask*/,10240/*rateLimit (kbps)*/,TP_SNMPC_PPS/*rateLimit(pps)*/
	},
	{	/*RADIUS auth client and RADIUS acct client*/
		"HTTPS_server",
		{0x0,0x08004500,0x06,0x0,0x000001bb0000}/*rule*/,
		{0x0,0xffffff00,0xff,0x0,0x0000ffff0000}/*mask*/,10240/*rateLimit (kbps)*/,TP_HTTPSS_PPS/*rateLimit(pps)*/
	},
	{	/*TCP*/
		"TCP",
		{0x0,0x08004500,0x06}/*rule*/,
		{0x0,0xffffff00,0xff}/*mask*/,10240/*rateLimit (kbps)*/,TP_TCP_PPS/*rateLimit(pps)*/
	},
	{	/*UDP*/
		"UDP",
		{0x0,0x08004500,0x11}/*rule*/,
		{0x0,0xffffff00,0xff}/*mask*/,10240/*rateLimit (kbps)*/,TP_UDP_PPS/*rateLimit(pps)*/
	},
	{	/*TIPC */
		"TIPC",
		{0x0,0x88ca0000,0x00000000,0x0,0x0000000000000000}/*rule*/,
		{0x0,0xffff0000,0x00000000,0x0,0x0000000000000000}/*mask*/,10240/*rateLimit (kbps)*/,TP_TIPC_PPS/*rateLimit (pps)*/
	},
};

static int cvm_car_init(void)
 {
     cvmx_sysinfo_t    *sys_info_ptr  = cvmx_sysinfo_get();
     uint64_t           cpu_clock_hz  = sys_info_ptr->cpu_clock_hz;
     car_rate_scaled = ((cpu_clock_hz/1024) * (SCALE_FACTOR * 8));
//     car_rate_pps_scaled = (cpu_clock_hz * SCALE_FACTOR);
     car_rate_pps_scaled = (cpu_clock_hz);
     return 0;
 }

static inline int cvm_car_bps_set(ratelimit_t* ratelimit_bps, int rate_bps)
{
	if((rate_bps <0) || (ratelimit_bps == NULL))
		return -1;

	ratelimit_bps->rate = rate_bps;
	ratelimit_bps->depth = 10000;
    ratelimit_bps->cycles_prev = 0;

	if (ratelimit_bps->rate == 0)
	{
		ratelimit_bps->rate_in_cycles_per_byte = 0x0FFFFFFFFFFFFFFFUL;
		ratelimit_bps->depth_in_cycles         = 0;
	}
	else
	{
		ratelimit_bps->rate_in_cycles_per_byte = (car_rate_scaled/ratelimit_bps->rate);
		ratelimit_bps->depth_in_cycles         = (ratelimit_bps->depth * ratelimit_bps->rate_in_cycles_per_byte);
	}

	return 0;
}


static inline int cvm_car_bps_calculate(int bytes, ratelimit_t* ratelimit_bps)
{
	uint64_t cycles_curr    = 0;
	uint64_t cycles_elapsed = 0;
	uint64_t cycles_reqd    = 0;
	int      retval         = DROP_PKT;

	cycles_curr    = cvmx_get_cycle();
	cycles_reqd    = (ratelimit_bps->rate_in_cycles_per_byte * bytes);
	cycles_elapsed = ((cycles_curr - ratelimit_bps->cycles_prev) << SCALE_FACTOR_BIT_SHIFT);

	if (cvmx_unlikely(cycles_elapsed > ratelimit_bps->depth_in_cycles))
	{
		cycles_elapsed  = ratelimit_bps->depth_in_cycles;
		ratelimit_bps->cycles_prev = cycles_curr - (ratelimit_bps->depth_in_cycles >> SCALE_FACTOR_BIT_SHIFT);
	}

	if (cycles_elapsed < cycles_reqd)
	{
		retval = DROP_PKT;
	}
	else
	{
		ratelimit_bps->cycles_prev = ratelimit_bps->cycles_prev + (cycles_reqd >> SCALE_FACTOR_BIT_SHIFT);
		retval = PASS_PKT;
	}

	return retval;
}

static inline int cvm_car_pps_set(ratelimit_pps_t* ratelimit_pps, int rate_pps)
{
	if((rate_pps <0) || (ratelimit_pps == NULL))
		return -1;

	ratelimit_pps->rate_pps = rate_pps;
    ratelimit_pps->cycles_prev = 0;
	if (ratelimit_pps->rate_pps == 0)
	{
		ratelimit_pps->rate_in_cycles_per_pkt = 0x0FFFFFFFFFFFFFFFUL;
	}
	else if(CVM_RATE_NO_LIMIT == ratelimit_pps->rate_pps)
	{
		/*for nolimit */
		ratelimit_pps->rate_in_cycles_per_pkt = 0;
	}
	else
	{
		ratelimit_pps->rate_in_cycles_per_pkt = (car_rate_pps_scaled/ratelimit_pps->rate_pps);
	}

	return 0;
}

static inline int cvm_car_pps_calculate(ratelimit_pps_t* ratelimit_pps)
{
	uint64_t cycles_curr    = 0;
	uint64_t cycles_elapsed = 0;
	uint64_t cycles_reqd    = 0;
	int      retval         = DROP_PKT;

	cycles_curr    = cvmx_get_cycle();
	cycles_reqd    = ratelimit_pps->rate_in_cycles_per_pkt;
//	cycles_elapsed = ((cycles_curr - ratelimit_pps->cycles_prev) << SCALE_FACTOR_BIT_SHIFT);
	cycles_elapsed = (cycles_curr - ratelimit_pps->cycles_prev);

    if(cvm_rate_limit_debug)
    {
        printf("cycles_curr = %lu, cycle_prev = %lu, cycles_reqd = %lu, cycles_elapsed = %lu count_in_one_brust = %d prev_drop_cycles %lu\n", 
                cycles_curr, ratelimit_pps->cycles_prev, cycles_reqd, cycles_elapsed, count_in_one_brust, prev_drop_cycles);
    }

	if (cycles_elapsed < cycles_reqd)
	{
		if(count_in_one_brust >= BURST_PASS_COUNT)
		{
			prev_drop_cycles = cycles_curr;
			retval = DROP_PKT;
		}
		else
		{/*avoid collision cause drop */
			//count_in_one_brust ++;
			cvmx_atomic_add32(&count_in_one_brust,1);
			ratelimit_pps->cycles_prev = cycles_curr;
			retval = PASS_PKT;
		}
	}
	else
	{
//	    ratelimit_pps->cycles_prev = ratelimit_pps->cycles_prev + (cycles_reqd >> SCALE_FACTOR_BIT_SHIFT);
		if(count_in_one_brust && (prev_drop_cycles < ratelimit_pps->cycles_prev))
		{
			//count_in_one_brust = 0;
			cvmx_atomic_set32(&count_in_one_brust,0);
		}
		ratelimit_pps->cycles_prev = cycles_curr;
		retval = PASS_PKT;
	}

	return retval;
}


 /*********************************************************
 * DESCRIPTION:
 *  get the match rule or default rule
 * INPUT:
 *  uint32 * - skbdata -- the pointer of skb->data
 *  uint32   - len  -- skb->len: the length of skb->data
 * OUTPUT:
 *  NONE
 * RETURN:
 *  1~(MAX_MATCH_RULES_NUM-1)  -- the matched rule index 
 *  0   --  the  default rule index (no rule matched )
 * NOTE:
 *  NONE
 *
 *********************************************************/
static inline int32_t get_protocol_match_rule(cvmx_wqe_t *work)
{
	int i = 0;
	int j = 0;
	uint8_t* pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);   
	int data_len = CVM_WQE_GET_LEN(work);
	int match = MATCH_SUCCESS;
	int match_len = 0;
	MATCH_TYPE * rules_ptr = NULL;
	MATCH_TYPE * masks_ptr = NULL;
	uint32_t * tmp_pkt_ptr = (uint32_t*)pkt_ptr;
	MATCH_TYPE * new_pkt_ptr = (MATCH_TYPE *)pkt_ptr;
	uint64_t * long_pkt_ptr = NULL;
	unsigned int tag1 = 0;
	unsigned int tag2 = 0;

    if(NULL == work)
        return MATCH_FAILED;
    
	if(ETH_TYPE_8021Q == ((tmp_pkt_ptr[3] & 0xffff0000) >> 16))
	{/*tag process*/
		if(ETH_TYPE_8021Q == ((tmp_pkt_ptr[4] & 0xffff0000) >> 16))
		{/*qinq process*/
			new_pkt_ptr = (uint64_t*)(tmp_pkt_ptr + 2);
			data_len -= ETH_VLAN_LEN*2;
			tag2 = tmp_pkt_ptr[4] & 0xfff;
		}
		else
		{/*sigal tag process*/
			new_pkt_ptr = (uint64_t*)(tmp_pkt_ptr + 1);
			data_len -= ETH_VLAN_LEN;			
			tag1 = tmp_pkt_ptr[3] & 0xfff;
		}
	}
	long_pkt_ptr = new_pkt_ptr;
	
	for(i = 1;i < MAX_MATCH_RULES_NUM;i++)
	{/*start to match rules from 1, because 0 is reserve for default*/
		if(i > (int)global_various_ptr->last_valid_index)
		{
			break;
		}
		
		match_len = protocol_rate_limiter[i].rules_length;
		if(data_len < (match_len<<3))
		{
			continue;
		}

		
		if(match_len)
		{
			match = MATCH_SUCCESS;/*set match*/
			rules_ptr = protocol_rate_limiter[i].protocol_match_rule;
			masks_ptr = protocol_rate_limiter[i].protocol_match_mask;
			uint64_t * long_mask_ptr = masks_ptr;
			uint64_t * long_rule_ptr = rules_ptr;
			uint64_t * long_skb_ptr  = (MATCH_TYPE *)pkt_ptr;
			if(long_mask_ptr[0])
			{
				if((long_skb_ptr[0] & long_mask_ptr[0])\
					!= (long_rule_ptr[0] & long_mask_ptr[0]))
				{
					match = MATCH_FAILED;/*not match*/
					continue;
				}
			}

			if(long_mask_ptr[1] & 0xffffffff00000000)
			{
				if(((long_skb_ptr[1] & long_mask_ptr[1]) & 0xffffffff00000000)\
					!= ((long_rule_ptr[1] & long_mask_ptr[1]) & 0xffffffff00000000))
				{
					match = MATCH_FAILED;/*not match*/
					continue;
				}
			}			
			
			if(long_mask_ptr[1] & 0x00000000ffffffff)
			{
				if(((long_pkt_ptr[1] & long_mask_ptr[1]) & 0x00000000ffffffff)\
					!= ((long_rule_ptr[1] & long_mask_ptr[1]) & 0x00000000ffffffff))
				{
					match = MATCH_FAILED;/*not match*/
					continue;
				}
			}
			
			for(j = 2;j < match_len;j++)
			{/*skb has 16 bytes reserved , so even the last bytes not enough 8 bytes we can use it as 8 bytes*/
				if(masks_ptr[j])
				{
					if((new_pkt_ptr[j] & masks_ptr[j])\
						!= (rules_ptr[j] & masks_ptr[j]))
					{
						match = MATCH_FAILED;/*not match*/
						break;
					}
				}
			}
						
			if(MATCH_SUCCESS == match)
			{
			
				if(cvm_rate_limit_debug)
				{
					printf("rate limit: match rules[%d] %s tag1 %d tag2 %d ethertype %04x data len %d rete_bps %d rete_pps %d\n", \
						i, protocol_rate_limiter[i].name, tag1, tag2, \
						tag1 ? (tag2 ? (tmp_pkt_ptr[5] & 0xffff0000) : (tmp_pkt_ptr[4] & 0xffff0000)) : (tmp_pkt_ptr[3] & 0xffff0000), data_len, \
						protocol_rate_limiter[i].rate_bps, protocol_rate_limiter[i].ratelimit_pps.rate_pps);
				}				
				return i;
			}
		}
	}
	
	if(cvm_rate_limit_debug)
	{
		printf("rate limit: match rules[%d] %s rate_bps %d rate_pps %d\n", \
			CVM_RCV_PROTOCOL_UNKNOWN, protocol_rate_limiter[CVM_RCV_PROTOCOL_UNKNOWN].name, \
			protocol_rate_limiter[CVM_RCV_PROTOCOL_UNKNOWN].ratelimit_pps.rate_pps, \
			protocol_rate_limiter[CVM_RCV_PROTOCOL_UNKNOWN].ratelimit_pps.rate_pps);
	}

	return CVM_RCV_PROTOCOL_UNKNOWN;
}


/*****************************************************
 * DESCRIPTION:
 *      the main function for cavium-ethernet driver hook and call
 * INPUT:
 *  uint32 * - skbdata -- the pointer of skb->data
 *  uint32   - len  -- skb->len: the length of skb->data
 * OUTPUT:
 *  NONE
 * RETURN:
 *  -1 -- means receive counter is more than limiter,
 *          the package thould be drop
 *  0  -- means the package should pass
 *****************************************************/
int32_t cvm_rate_limit(cvmx_wqe_t *work)
{
	protocol_match_item_t * protocol_match_rule_ptr = NULL;
	int i = 0;
	if(!(global_various_ptr->cvm_rate_limit_enabled))
	{
		return 0;
	}
	i = get_protocol_match_rule(work);
	protocol_match_rule_ptr = &protocol_rate_limiter[i];
	if(protocol_match_rule_ptr)
	{
	    if(ratelimit_flag == RATELIMIT_KBPS)   
	    {
            if(cvm_car_bps_calculate(CVM_WQE_GET_LEN(work), &protocol_rate_limiter[i].ratelimit_bps) == DROP_PKT)
            {
                //global_various_ptr->cvm_rate_limit_drop_counter ++;
    			//protocol_match_rule_ptr->drop_counter++;
    			cvmx_atomic_add64((int64_t*)(&global_various_ptr->cvm_rate_limit_drop_counter),1);
	            cvmx_atomic_add64((int64_t*)(&protocol_match_rule_ptr->drop_counter),1);  
    			return -1;
            }
        }
        else
        {
            if(cvm_car_pps_calculate(&protocol_rate_limiter[i].ratelimit_pps) == DROP_PKT)
            {
                //global_various_ptr->cvm_rate_limit_drop_counter ++;
    			//protocol_match_rule_ptr->drop_counter++;
	            cvmx_atomic_add64((int64_t*)(&global_various_ptr->cvm_rate_limit_drop_counter),1);
	            cvmx_atomic_add64((int64_t*)(&protocol_match_rule_ptr->drop_counter),1);    			
    			return -1;
            }
        }
	}
	//global_various_ptr->cvm_rate_limit_pass_counter ++;
	//protocol_match_rule_ptr->pass_counter++;
	cvmx_atomic_add64((int64_t*)(&global_various_ptr->cvm_rate_limit_pass_counter),1);
	cvmx_atomic_add64((int64_t*)(&protocol_match_rule_ptr->pass_counter),1);
	return 0;
}


void dump_ratelimit_table(void)
{
    int i = 0;
    protocol_match_item_t* rl = NULL;

    printf("-------------------- dump_ratelimit_table -----------------\n");
    printf("cr_last_valid_rule_index = %d\n", global_various_ptr->last_valid_index);
    for(i = 0; i < CVM_RATE_LIMIT_DEFAULT_RULE_NUM; i++)
    {
        rl = &protocol_rate_limiter[i];
        printf("\tentry %d\n", i);
        printf("\tname: %s\n", rl->name);
        printf("\trateLimit: %u\n", rl->rate_bps);
        printf("\trateLimit_pps: %u\n", rl->ratelimit_pps.rate_pps);
        printf("\trules_length: %u\n", rl->rules_length);
        printf("\tdrop_counter: %lu\n", rl->drop_counter);
        printf("\n");
    }
    printf("-----------------------------------------------------------\n");
}



int32_t cvm_rate_limit_init(void)
{
    int i = 0;
    int j = 0;
	int32_t cr_last_valid_rule_index = 0;
    

    protocol_rate_limiter_mem_base =  (void *) cvmx_bootmem_alloc_named(MAX_MATCH_RULES_NUM*sizeof(protocol_match_item_t)+128, 128, RATELIMIT_TBL_NAME);
    if(protocol_rate_limiter_mem_base == NULL)
    {
        printf("Warning: No memory available for ratelimit table!\n");
        return RETURN_ERROR;
    }
	protocol_rate_limiter = (protocol_match_item_t *)((void *)protocol_rate_limiter_mem_base + 128);	
    memset((void *)protocol_rate_limiter, 0, MAX_MATCH_RULES_NUM*sizeof(protocol_match_item_t));
    memcpy((void *)protocol_rate_limiter, (void*)(void *)protocol_rate_limiter_template, MAX_MATCH_RULES_NUM*sizeof(protocol_match_item_t));

    for(i = 0; i < CVM_RATE_LIMIT_DEFAULT_RULE_NUM; i++)
    {
        for(j = PACKET_MATCH_BYTE_NUM; j > 0; j--)
    	{
			if(protocol_rate_limiter[i].protocol_match_mask[j-1])
			{
				protocol_rate_limiter[i].rules_length = j;
				break;
			}
		}
	}
	
    /*protocol_rate_limiter[0].next = 1;*/
    for(i = 1; i < MAX_MATCH_RULES_NUM; i++)
    {
        if(protocol_rate_limiter[i].rules_length)
    	{
            cr_last_valid_rule_index = i;
    	}
    }

    /* init car */
    cvm_car_init();
	global_various_ptr = (global_various_t *)protocol_rate_limiter_mem_base;
	memset(global_various_ptr, 0, sizeof(global_various_t));
	global_various_ptr->car_rate_scaled = car_rate_scaled;	
	global_various_ptr->car_rate_pps_scaled = car_rate_pps_scaled;
	global_various_ptr->last_valid_index = cr_last_valid_rule_index;
	// *((uint64_t *)protocol_rate_limiter_mem_base) = car_rate_scaled;
	// *(((uint64_t *)protocol_rate_limiter_mem_base) + 1) = car_rate_pps_scaled;
    for(i = 0; i < MAX_MATCH_RULES_NUM; i++)
    {
         cvm_car_bps_set(&protocol_rate_limiter[i].ratelimit_bps, protocol_rate_limiter[i].rate_bps);
         cvm_car_pps_set(&protocol_rate_limiter[i].ratelimit_pps, protocol_rate_limiter[i].rate_pps);
    }    
    
    return 0;
}


