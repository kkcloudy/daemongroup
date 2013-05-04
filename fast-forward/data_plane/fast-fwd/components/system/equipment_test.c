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
*equipment_test.c
*
*CREATOR:
*   <zhaohan@autelan.com>
*
*DESCRIPTION:
*     equipment test for boards
*
*DATE:
*   08/20/2012	
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1 $
*
*******************************************************************************/
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
#include "autelan_product_info.h"
#include "equipment_test.h"

extern CVMX_SHARED product_info_t product_info;

static inline int send_pkt_out(cvmx_wqe_t *work, uint64_t port)
{
	cvmx_pko_command_word0_t pko_command;
	cvmx_buf_ptr_t packet_ptr;
	int queue = 0;
	int ret = 0;
	int tag = 0;
	
	if(work == NULL)
		return -1;    			
	
	queue = cvmx_pko_get_base_queue(port);	
	cvmx_pko_send_packet_prepare(port, queue, CVMX_PKO_LOCK_ATOMIC_TAG);
	/* Build a PKO pointer to this packet */
	if (work->word2.s.bufs == 0)
	{
		/* Packet is entirely in the WQE. Give the WQE to PKO and have it free it */
		pko_command.s.total_bytes = CVM_WQE_GET_LEN(work);
		pko_command.s.segs = 1;
		packet_ptr.u64 = 0;
		packet_ptr.s.pool = CVMX_FPA_WQE_POOL;
		packet_ptr.s.size = CVMX_FPA_WQE_POOL_SIZE;
		
		packet_ptr.s.addr = cvmx_ptr_to_phys(work->packet_data); 
		
		if (cvmx_likely(!work->word2.s.not_IP))
		{
			/* The beginning of the packet moves for IP packets */
			if (work->word2.s.is_v6)
				packet_ptr.s.addr += 2;
			else
				packet_ptr.s.addr += 6;
		}
	}
	else
	{
		pko_command.s.total_bytes = CVM_WQE_GET_LEN(work);
		pko_command.s.segs = work->word2.s.bufs;
		packet_ptr = work->packet_ptr;
		tag = 1;
	}
	pko_command.s.dontfree = 0; /*AUTO_FREE*/	
	ret = 	cvmx_pko_send_packet_finish(port, queue, pko_command, packet_ptr, CVMX_PKO_LOCK_ATOMIC_TAG);
	
	if (ret != CVMX_PKO_SUCCESS)
	{
		cvmx_helper_free_packet_data(work); /*free data*/
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		return -1; 		
	}
	
	cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS, 1);
	cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_BYTES, CVM_WQE_GET_LEN(work));

	if(tag == 1)
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		
	return 0; 	
}


static inline void change_vlan(cvmx_wqe_t* work, uint16_t vlan_id)
{
	struct vlan_ethhdr* eth_header = NULL;
	eth_header = (struct vlan_ethhdr*)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
	eth_header->out_tag.vlan_id = vlan_id;
	return;
}

int32_t fwd_equipment_test_AC4X(cvmx_wqe_t* work)
{
    if(product_info.se_mode == SE_MODE_COEXIST)
    {
        if(CVM_WQE_GET_PORT(work) == AC4X_PORT_VEF1)
        {
            if(work->word2.s.vlan_valid)
            {
                if(work->word2.s.vlan_id == 1000)
                {
                    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
                        "-------change vlan dump pkt 1000->2000. VEF1(16) to VEF1(16).\n");                
                    change_vlan(work, 2000);
                    send_pkt_out(work, AC4X_PORT_VEF1);
                    return RETURN_OK;
                }
                else if(work->word2.s.vlan_id == 2000)
                {
                    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
                        "-------change vlan dump pkt 2000->1000. VEF1(16) to VEF1(16).\n");
                    change_vlan(work, 1000);
                    send_pkt_out(work, AC4X_PORT_VEF1);
                    return RETURN_OK;
                }
            }
        }
    }
    else/*SE_MODE_STANDALONE*/
    {
        if(CVM_WQE_GET_PORT(work) == AC4X_PORT_VES1)
        {
            if(work->word2.s.vlan_valid)
            {
                if(work->word2.s.vlan_id == 2000)
                {
                    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
                        "-------change vlan dump pkt 2000->3000. VES1(2112) to VES1(2112).\n");                
                    change_vlan(work, 3000);
                    send_pkt_out(work, AC4X_PORT_VES1);
                    return RETURN_OK;
                }
                else if(work->word2.s.vlan_id == 3000)
                {
                    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
                        "-------change vlan dump pkt 3000->2000. VES1(2112) to VES1(2112).\n");                
                    change_vlan(work, 2000);
                    send_pkt_out(work, AC4X_PORT_VES1);
                    return RETURN_OK;
                }
            }
        }
        else if(CVM_WQE_GET_PORT(work) == AC4X_PORT_VES2)
        {
            if(work->word2.s.vlan_valid)
            {
                if(work->word2.s.vlan_id == 3000)
                {
                    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
                        "-------change vlan dump pkt 3000->4000. VES2(2624) to VES3(3136).\n");                
                    change_vlan(work, 4000);
                    send_pkt_out(work, AC4X_PORT_VES3);
                    return RETURN_OK;
                }
            }
        }
        else if(CVM_WQE_GET_PORT(work) == AC4X_PORT_VES3)
        {
            if(work->word2.s.vlan_valid)
            {
                if(work->word2.s.vlan_id == 4000)
                {
                    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"-------change vlan dump pkt 4000->3000. VES3(3136) to VES2(2624).\n");                
                    change_vlan(work, 3000);
                    send_pkt_out(work, AC4X_PORT_VES2);
                    return RETURN_OK;
                }
            }
        }
    }

    /* don't free packet. just return */
    return RETURN_ERROR;
}


