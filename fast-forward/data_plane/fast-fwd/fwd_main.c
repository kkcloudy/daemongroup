/***********************license start************************************
 * File name: fwd_main.c 
 * Auther     : lutao
 * 
 * Copyright (c) Autelan . All rights reserved.
 * 
 **********************license end**************************************/
#include <stdio.h>
#include <string.h>

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
#include "cvmx-tim.h"
#ifdef SDK_VERSION_2_2
#include "fastfwd-common-rnd.h"
#include "fastfwd-common-misc.h"
#include "fastfwd-common-defs.h"
#include "fastfwd-common-fpa.h"
#else
#include "cvm-common-rnd.h"
#include "cvm-common-misc.h"
#include "cvm-common-defs.h"
#include "cvm-common-fpa.h"
#endif
#include "autelan_product_info.h"
#include "shell.h"
#include "acl.h"
#include "fwd_main.h"
#include "cvm-ip-in.h"
#include "capwap.h"
#include "fccp.h"
#include "car.h"
#include "traffic_monitor.h"
#include "fwd_rpa.h"
#include "cvmcs-nic-api.h"
#include "cvm_ip_reass.h"
#include "equipment_test.h"
#include "cvm_ratelimit.h"

#include "fwd_debug.h"


//#define PASSTHOUGH_L2_PERFORMANCE_TEST
extern CVMX_SHARED product_info_t product_info;
extern CVMX_SHARED port_info_t pip_port_info[CVMX_PIP_NUM_INPUT_PORTS];
#ifdef OCTEON_DEBUG_LEVEL
extern CVMX_SHARED uint32_t debug_tag_val;    /* for test */
#endif

CVMX_SHARED cvmx_spinlock_t gl_wait_lock;
CVMX_SHARED int cvm_ip_icmp_enable = FUNC_DISABLE;
CVMX_SHARED int cvm_car_enable = FUNC_DISABLE;
CVMX_SHARED int cvm_ip_only_enable = FUNC_DISABLE; /*Only fot the evaluation test*/
CVMX_SHARED int cvm_passthough_enable = FUNC_DISABLE;   /* Only for the test */
CVMX_SHARED int cvm_pure_payload_acct = FUNC_DISABLE;
CVMX_SHARED uint16_t passthough_vlan_id = 0;


CVMX_SHARED int cvm_drop_dump_enable = FUNC_DISABLE;
/* If set qos_enable=1, then all the 802.11 frame will qos frame */
CVMX_SHARED int cvm_qos_enable = FUNC_DISABLE;
CVMX_SHARED int cvm_rate_limit_enabled = FUNC_DISABLE;

uint32_t set_tag_flag = 0;	/*Prevent deadlocks*/
CVMX_SHARED int32_t gbl_80211_id; /*Not sure, use share or not ?  lutao tag*/

CVMX_SHARED	int fpa_packet_pool_count = 0;
CVMX_SHARED	int fpa_wqe_pool_count = 0;
CVMX_SHARED	int fpa_output_pool_count = 0;
CVMX_SHARED	int fpa_timer_pool_count = 0;
CVMX_SHARED	int fpa_packet_log_pool_count = 0;


/*流保序功能开关*/
CVMX_SHARED	int flow_sequence_enable = FUNC_DISABLE;
CVMX_SHARED	int wait_linux_buffer_count = 100;

#define DEFAULT_DOWNLINK_MTU    1442    /* eth=>capwap */
CVMX_SHARED uint32_t downlink_mtu = DEFAULT_DOWNLINK_MTU;


CVMX_SHARED traffic_stats_t traffic_stats;
CVMX_SHARED int cvm_traffic_monitor_func = FUNC_DISABLE;
extern CVMX_SHARED uint8_t *base_mac_table ;
CVMX_SHARED fwd_virtual_port_t fwd_virtual_port;
CVMX_SHARED uint32_t fwd_equipment_test_enable = FUNC_DISABLE;
CVMX_SHARED int32_t (*fwd_equipment_test_fun)(cvmx_wqe_t*) = NULL; /*for equipment test*/

CVMX_SHARED uint64_t linux_sec = 0;
CVMX_SHARED uint64_t fwd_sec = 0;

void clear_fau_64();

/**
 * Debug routine to dump the packet structure to the console
 *
 * @param work   Work queue entry containing the packet to dump
 * @return
 */
 /*
int cvmx_dump_128_packet(cvmx_wqe_t *work)
{
	uint64_t        count;
	uint64_t        remaining_bytes;
	cvmx_buf_ptr_t  buffer_ptr;
	uint64_t        start_of_buffer;
	uint8_t *       data_address;
	uint8_t *       end_of_data;

	cvmx_dprintf("Packet Length:   %u\n", CVM_WQE_GET_LEN(work));
	cvmx_dprintf("    Input Port:  %u\n", CVM_WQE_GET_PORT(work));
	cvmx_dprintf("    QoS:         %u\n", CVM_WQE_GET_QOS(work));
	cvmx_dprintf("    Buffers:     %u\n", work->word2.s.bufs);

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
			if (work->word2.s.is_v6)
				buffer_ptr.s.addr += 2;
			else
				buffer_ptr.s.addr += 6;
		}
	}
	else
		buffer_ptr = work->packet_ptr;

	remaining_bytes = 128;

	while (remaining_bytes)
	{
		start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
		cvmx_dprintf("    Buffer Start:%llx\n", (unsigned long long)start_of_buffer);
		cvmx_dprintf("    Buffer I   : %u\n", buffer_ptr.s.i);
		cvmx_dprintf("    Buffer Back: %u\n", buffer_ptr.s.back);
		cvmx_dprintf("    Buffer Pool: %u\n", buffer_ptr.s.pool);
		cvmx_dprintf("    Buffer Data: %llx\n", (unsigned long long)buffer_ptr.s.addr);
		cvmx_dprintf("    Buffer Size: %u\n", buffer_ptr.s.size);

		cvmx_dprintf("\t\t");
		data_address = (uint8_t *)cvmx_phys_to_ptr(buffer_ptr.s.addr);
		end_of_data = data_address + buffer_ptr.s.size;
		count = 0;
		while (data_address < end_of_data)
		{
			if (remaining_bytes == 0)
				break;
			else
				remaining_bytes--;
			cvmx_dprintf("%02x", (unsigned int)*data_address);
			data_address++;
			if (remaining_bytes && (count == 7))
			{
				cvmx_dprintf("\n\t\t");
				count = 0;
			}
			else
				count++;
		}
		cvmx_dprintf("\n");

		if (remaining_bytes)
			buffer_ptr = *(cvmx_buf_ptr_t*)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
	}
	return 0;
}
*/

int32_t disable_fastfwd()
{
	int port_index;

	/* Change the group for only the port we're interested in */
	for (port_index=0; port_index < CVMX_PIP_NUM_INPUT_PORTS; port_index++)
	{
		if(pip_port_info[port_index].s.if_used  == PORT_USED)
		{
			cvmx_pip_port_tag_cfg_t tag_config;
			tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));

			tag_config.s.grp = product_info.to_linux_group;
			cvmx_write_csr(CVMX_PIP_PRT_TAGX(port_index), tag_config.u64);
		}
	}   

	return RETURN_OK;
}

int32_t enable_fastfwd()
{
	int port_index;

	/* Change the group for only the port we're interested in */
	for (port_index=0; port_index < CVMX_PIP_NUM_INPUT_PORTS; port_index++)
	{
		if(pip_port_info[port_index].s.if_used  == PORT_USED)
		{
			cvmx_pip_port_tag_cfg_t tag_config;
			tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));

			tag_config.s.grp = product_info.panel_interface_group;
			cvmx_write_csr(CVMX_PIP_PRT_TAGX(port_index), tag_config.u64);
		}
	}   

	return RETURN_OK;
}

int fastfwd_config_tag_type(control_cmd_t *fccp_cmd)
{
    int port_index;
    cvmx_pip_port_tag_cfg_t tag_config;
    cvmx_pow_tag_type_t  tmp_type;
    
    if(NULL == fccp_cmd)
        return RETURN_ERROR;

    tmp_type=fccp_cmd->fccp_data.tag_type;
    for(port_index=0;port_index<CVMX_PIP_NUM_INPUT_PORTS;port_index++)
    {
        if(pip_port_info[port_index].s.if_used  == PORT_USED )
        {
            tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));
    		tag_config.s.tcp6_tag_type = tmp_type; 
    		tag_config.s.tcp4_tag_type = tmp_type;
    		tag_config.s.ip6_tag_type = tmp_type;
    		tag_config.s.ip4_tag_type = tmp_type;
    		tag_config.s.non_tag_type = tmp_type;
    		cvmx_write_csr(CVMX_PIP_PRT_TAGX(port_index), tag_config.u64);
        }
        else   
            continue;
    }

    return RETURN_OK;
}

int fastfwd_get_tag_type(control_cmd_t *fccp_cmd)
{
    int port_index,flag = 0;

    if(NULL == fccp_cmd)
        return RETURN_ERROR;
    for(port_index=0;port_index<CVMX_PIP_NUM_INPUT_PORTS;port_index++)
    {
        if(pip_port_info[port_index].s.if_used  == PORT_USED)
        {
            flag = 1;
            break;
        }
        else   
            continue;
    }
    if(flag)
    {
        cvmx_pip_port_tag_cfg_t tag_config;
        tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));
    	fccp_cmd->fccp_data.tag_type = tag_config.s.ip4_tag_type;
    }else return RETURN_ERROR;
    
    return RETURN_OK;
}

void display_version_info(void)
{
	printf("\n");
	printf("Autelan fast forwarding code compiled on %s %s\n", __DATE__, __TIME__);
	printf("SDK Version: %s\n", cvmx_helper_get_version());
	printf("Common version      : %s\n", cvm_common_get_version());
	printf("Fastfwd mode: %s\n", (product_info.se_mode == SE_MODE_COEXIST) ? "COEXIST" : "STANDALONE");
	printf("\n");
}


/* Allocate the memory */
static int setup_free_mem(void)
{
	void *memory=NULL;
	void *base = NULL;

#if 1
	/* check if the sys_mem is returning Mbytes, not the bytes */
	cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();
	uint64_t sys_mem = sys_info_ptr->system_dram_size;

	if (sys_mem <= 16*1024)
	{
		/* convert Mbytes to bytes */
		sys_mem = sys_mem * 1024ull * 1024ull;
	}

	switch (sys_mem)
	{
		case 256 * 1024 * 1024:
			fpa_packet_pool_count = 8000; /* max input packets + # recv queues */
			fpa_wqe_pool_count = 8000 + 256;
			fpa_output_pool_count = 256; 
			fpa_timer_pool_count = 256;
			fpa_packet_log_pool_count = 256;
			break;

		case 512 * 1024 * 1024: 
			fpa_packet_pool_count = 20000; /* max input packets + # recv queues */
			fpa_wqe_pool_count = 20000 + 512;
			fpa_output_pool_count = 512;
			fpa_timer_pool_count = 512; 
			fpa_packet_log_pool_count = 512;
			break;

		case 1024ull * 1024ull * 1024ull:   
			fpa_packet_pool_count = 40000; /* max input packets + # recv queues */
			fpa_wqe_pool_count = 40000 + 1024;
			fpa_output_pool_count = 4096;
			fpa_timer_pool_count = 1024;  
			fpa_packet_log_pool_count = 1024;
			break;

		case 2048ull * 1024ull * 1024ull:       
			fpa_packet_pool_count = 40000; /* max input packets + # recv queues */
			fpa_wqe_pool_count = 40000 + 1024;
			fpa_output_pool_count = 4096;
			fpa_timer_pool_count = 1024;
			fpa_packet_log_pool_count = 1024;
			break;

		case 4096ull * 1024ull * 1024ull:       
			fpa_packet_pool_count = 40000; /* max input packets + # recv queues */
			fpa_wqe_pool_count = 40000 + 1024;
			fpa_output_pool_count = 4096;
			fpa_timer_pool_count = 1024;
			fpa_packet_log_pool_count = 1024;
			break;

		case 8192ull * 1024ull * 1024ull:       
			fpa_packet_pool_count = 40000; /* max input packets + # recv queues */
			fpa_wqe_pool_count = 40000 + 1024;
			fpa_output_pool_count = 4096;
			fpa_timer_pool_count = 1024;
			fpa_packet_log_pool_count = 1024;
			break;

		default:
			printf("setup_free_mem: System memory does not match (sys_mem=%lld)\n", CAST64(sys_mem));
			return (RETURN_ERROR);
	}
#endif

    if((product_info.se_mode == SE_MODE_COEXIST) && (product_info.board_type == AUTELAN_BOARD_AX71_CRSMU_MODULE_NUM))
    {
        fpa_packet_pool_count = 4000; /* max input packets + # recv queues */
        fpa_wqe_pool_count = 4000 + 256;
        fpa_output_pool_count = 1024;
        fpa_timer_pool_count = 1024;
		fpa_packet_log_pool_count = 256;
    }

	cvmx_fpa_enable();

	memory = cvmx_bootmem_alloc(CVMX_FPA_PACKET_POOL_SIZE * fpa_packet_pool_count, CVMX_FPA_PACKET_POOL_SIZE);
	if (memory == NULL)
	{
		printf("Out of memory initializing fpa pool CVMX_FPA_PACKET_POOL.\n");
		return (RETURN_ERROR);
	}
	base =CVM_COMMON_INIT_FPA_CHECKS(memory, fpa_packet_pool_count, CVMX_FPA_PACKET_POOL_SIZE);
	cvmx_fpa_setup_pool(CVMX_FPA_PACKET_POOL, "Packet Buffers", memory, CVMX_FPA_PACKET_POOL_SIZE, fpa_packet_pool_count);
	cvm_common_fpa_add_pool_info(CVMX_FPA_PACKET_POOL, memory, CVMX_FPA_PACKET_POOL_SIZE, fpa_packet_pool_count, (uint64_t)(CAST64(base)));

	memory = cvmx_bootmem_alloc(CVMX_FPA_WQE_POOL_SIZE * fpa_wqe_pool_count, CVMX_FPA_WQE_POOL_SIZE);
	if (memory == NULL)
	{
		printf("Out of memory initializing fpa pool CVMX_FPA_WQE_POOL.\n");
		return (RETURN_ERROR);
	}
	base =CVM_COMMON_INIT_FPA_CHECKS(memory, fpa_wqe_pool_count, CVMX_FPA_WQE_POOL_SIZE);
	cvmx_fpa_setup_pool(CVMX_FPA_WQE_POOL, "Work Queue Entries", memory, CVMX_FPA_WQE_POOL_SIZE, fpa_wqe_pool_count);
	cvm_common_fpa_add_pool_info(CVMX_FPA_WQE_POOL, memory, CVMX_FPA_WQE_POOL_SIZE, fpa_wqe_pool_count, (uint64_t)base);

	memory = cvmx_bootmem_alloc(CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE * fpa_output_pool_count, CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE);
	if (memory == NULL)
	{
		printf("Out of memory initializing fpa pool CVMX_FPA_OUTPUT_BUFFER_POOL.\n");
		return (RETURN_ERROR);
	}
	base =CVM_COMMON_INIT_FPA_CHECKS(memory, fpa_output_pool_count, CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE);
	cvmx_fpa_setup_pool(CVMX_FPA_OUTPUT_BUFFER_POOL, "PKO Command Buffers", memory, CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE, fpa_output_pool_count);
	cvm_common_fpa_add_pool_info(CVMX_FPA_OUTPUT_BUFFER_POOL, memory, CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE, fpa_output_pool_count, (uint64_t)(CAST64(base)));


#ifdef CVMX_ENABLE_TIMER_FUNCTIONS
	memory = cvmx_bootmem_alloc(CVMX_FPA_TIMER_POOL_SIZE * fpa_timer_pool_count, CVMX_FPA_TIMER_POOL_SIZE);
	if (memory == NULL)
	{
		printf("Out of memory initializing fpa pool CVMX_FPA_TIMER_POOL.\n");
		return (RETURN_ERROR);
	}
	base =CVM_COMMON_INIT_FPA_CHECKS(memory, fpa_timer_pool_count, CVMX_FPA_TIMER_POOL_SIZE);
	cvmx_fpa_setup_pool(CVMX_FPA_TIMER_POOL, "TIM Command Buffers", memory, CVMX_FPA_TIMER_POOL_SIZE, fpa_timer_pool_count);
	cvm_common_fpa_add_pool_info(CVMX_FPA_TIMER_POOL, memory, CVMX_FPA_TIMER_POOL_SIZE, fpa_timer_pool_count, (uint64_t)(CAST64(base)));
#endif

	memory = cvmx_bootmem_alloc(CVMX_FPA_PACKET_LOG_POOL_SIZE * fpa_packet_log_pool_count, CVMX_FPA_PACKET_LOG_POOL_SIZE);
	if (memory == NULL)
	{
		printf("Out of memory initializing fpa pool CVMX_FPA_PACKET_LOG_POOL.\n");
		return (RETURN_ERROR);
	}
	base =CVM_COMMON_INIT_FPA_CHECKS(memory, fpa_packet_log_pool_count, CVMX_FPA_PACKET_LOG_POOL_SIZE);
	cvmx_fpa_setup_pool(CVMX_FPA_PACKET_LOG_POOL, "Packet Log Buffers", memory, CVMX_FPA_PACKET_LOG_POOL_SIZE, fpa_packet_log_pool_count);
	cvm_common_fpa_add_pool_info(CVMX_FPA_PACKET_LOG_POOL, memory, CVMX_FPA_PACKET_LOG_POOL_SIZE, fpa_packet_log_pool_count, (uint64_t)(CAST64(base)));
	
	if(OCTEON_IS_MODEL(OCTEON_CN68XX) && (cvmx_helper_initialize_sso(fpa_wqe_pool_count)))
       return RETURN_ERROR;

	return (RETURN_OK);
}

int setup_input_ports()
{
    int delta = 1;
    int ipd_port = 0;
    int num_ports = 0;
    int interface = 0;
    int num_interfaces = 0;
    cvmx_helper_interface_mode_t mode;
    cvmx_pip_port_tag_cfg_t tag_config;
    cvmx_pip_port_cfg_t     port_config;
    cvmx_pip_gbl_ctl_t pip_gbl_ctl;
    cvmx_pip_stat_ctl_t pip_stat_ctl;

    if ( (cvmx_helper_initialize_packet_io_global()) == -1)
    {
        printf("setup_input_ports : Failed to initialize/setup input ports\n");
        return (RETURN_ERROR);
    }
	
    /* Change the input group for all ports before input is enabled */
    num_interfaces = cvmx_helper_get_number_of_interfaces();
    printf("total interface number = %d\n", num_interfaces);
    for (interface = 0; interface < num_interfaces; interface++) 
    {       
        ipd_port = cvmx_helper_get_ipd_port(interface, 0);
        num_ports = cvmx_helper_ports_on_interface(interface);
        mode = cvmx_helper_interface_get_mode(interface);

        printf("interface %d has %d port, mode = %d, base port is %d\n", interface, num_ports, mode, ipd_port);       
        if (octeon_has_feature(OCTEON_FEATURE_PKND))
        {
            if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII)
            delta = 16;
        }

        while(num_ports--)
        {
            port_config.u64 = 0;
            port_config.s.qos = DEFAULT_PACKET_SSO_QUEUE;       /* Have all packets goto POW queue 1, queue 0 is for the messages between the cores*/
            port_config.s.mode = CVMX_PIP_PORT_CFG_MODE_SKIPL2; /* Process the headers and place the IP header in the work queue */
#ifdef CVM_IP_DYNAMIC_SHORT_SUPPORT
            port_config.s.dyn_rs = 1;
#endif

            /* setup the ports again for NULL tag */
            tag_config.u64 = 0;
            tag_config.s.inc_prt_flag = 0;

            tag_config.s.tcp6_tag_type = CVMX_POW_TAG_TYPE_ATOMIC; /* Keep the order of each port */
            tag_config.s.tcp4_tag_type = CVMX_POW_TAG_TYPE_ATOMIC;
            tag_config.s.ip6_tag_type = CVMX_POW_TAG_TYPE_ATOMIC;
            tag_config.s.ip4_tag_type = CVMX_POW_TAG_TYPE_ATOMIC;
            tag_config.s.non_tag_type = CVMX_POW_TAG_TYPE_ATOMIC;

            tag_config.s.grp = DEFAULT_PACKET_GRP;           /* Put all packets in group 15. Other groups can be used by the app */
            /*tag_config.s.grptag = 1;*/      /*设置GRP值为根据TAG值产生*/
            tag_config.s.inc_prt_flag  = CVMX_HELPER_INPUT_TAG_INPUT_PORT;

            tag_config.s.ip4_pctl_flag = CVMX_HELPER_INPUT_TAG_IPV4_PROTOCOL;
            tag_config.s.ip6_nxth_flag = CVMX_HELPER_INPUT_TAG_IPV6_NEXT_HEADER;
            tag_config.s.ip6_dst_flag  = CVMX_HELPER_INPUT_TAG_IPV6_DST_IP;
            tag_config.s.ip4_dst_flag  = CVMX_HELPER_INPUT_TAG_IPV4_DST_IP;
            tag_config.s.ip6_src_flag  = CVMX_HELPER_INPUT_TAG_IPV6_SRC_IP;
            tag_config.s.ip4_src_flag  = CVMX_HELPER_INPUT_TAG_IPV4_SRC_IP;

            tag_config.s.ip6_sprt_flag = CVMX_HELPER_INPUT_TAG_IPV6_SRC_PORT;
            tag_config.s.ip6_dprt_flag = CVMX_HELPER_INPUT_TAG_IPV6_DST_PORT;
            tag_config.s.ip4_sprt_flag = CVMX_HELPER_INPUT_TAG_IPV4_SRC_PORT;
            tag_config.s.ip4_dprt_flag = CVMX_HELPER_INPUT_TAG_IPV4_DST_PORT;

            /* Finally do the actual setup */
			cvmx_pip_config_port(ipd_port, port_config, tag_config);
            ipd_port += delta;
        } 
    }
    
    cvmx_helper_setup_red(PACKET_DROP_PIP_HIGH, PACKET_DROP_PIP_LOW);

    /* enable IPD */
#ifdef CVM_IP_DYNAMIC_SHORT_SUPPORT
    cvmx_write_csr(CVMX_PIP_IP_OFFSET, 2);
#endif

    pip_gbl_ctl.u64 = cvmx_read_csr(CVMX_PIP_GBL_CTL);
#if IP6_EXT_HDR
    /* Enable IPv6 extension headers */
    pip_gbl_ctl.s.ip6_eext = 0;
#endif
    pip_gbl_ctl.s.ip4_opts = 0;
    cvmx_write_csr(CVMX_PIP_GBL_CTL, pip_gbl_ctl.u64);

    pip_stat_ctl.u64 = 0;   /*stat registers hold value when read*/
    cvmx_write_csr(CVMX_PIP_STAT_CTL, pip_stat_ctl.u64);

	return (RETURN_OK);
}

/*
 * Is capwap pkt with wifi qos?
 * Yes: 1.   NO: 0.
 *
 * Note: the work must be the valid capwap pkt.
 */
static inline int32_t capwap_with_wifi_qos(cvmx_wqe_t *work)
{
    cvm_common_ip_hdr_t *ip = NULL;
    struct ieee80211_frame *ieee80211_hdr = NULL;
    uint8_t *pkt_ptr = NULL;

    if(work == NULL)
        return RETURN_ERROR;
        
    pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
    ip = (cvm_common_ip_hdr_t *)(pkt_ptr + work->word2.s.ip_offset); 
    ieee80211_hdr = (struct ieee80211_frame*)((uint8_t*)ip + IP_H_LEN + UDP_H_LEN + CW_H_LEN);
    if (ieee80211_hdr->i_fc[0] & IEEE80211_FC0_QOS_MASK) {
        return 1;
    }

    return 0;
}


/*
 * free: enable or disable hardware free the packet memory 
 * port: PIP port 
 */
static inline int send_packet_pip_out(cvmx_wqe_t *work, rule_item_t *prule, uint8_t free, int lock_type)
{
	cvmx_pko_command_word0_t pko_command;
	cvmx_buf_ptr_t packet_ptr;
	int queue = 0;
	int ret = 0;
	int tag = 0;
	uint64_t port = 0;
	int ip_offset = 0;
	
	if((work == NULL) || (prule == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"send_packet_pip_out param is NULL!\n");
		return RETURN_ERROR;    		
	}		

	port = prule->rules.forward_port;
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
	    queue = cvmx_pko_get_base_queue(port);
	else
	    queue = cvmx_pko_get_base_queue(port) + 1;
	

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"send_packet_pip_out: Send the packet!Outport=%ld, queue=%d\n",port,queue);

	
	cvmx_pko_send_packet_prepare(port, queue, lock_type);
	


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
		//cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0);
		tag = 1;
	}
	pko_command.s.dontfree = free;

	if(cvmx_phys_to_ptr(packet_ptr.s.addr) == NULL)
	{
		printf("send_packet_pip_out: pkt ptr is NULL!\n");
	}
	if(packet_ptr.s.size == 0)
	{
		printf("send_packet_pip_out: packet_ptr.s.size is 0\n");
	}
	if(pko_command.s.total_bytes == 0)
	{
		printf("send_packet_pip_out: pko_command.s.total_bytes == 0\n");
	}

	//add by yin for fwd nat  need add prule 
	/*wangjian_nat*/
	
	if (prule) 
	{
	    if(prule->rules.nat_flag == 1)
	    {
	        ip_offset = 14;
	        if(0 != prule->rules.out_ether_type)
	            ip_offset += 4;
	        if(0 != prule->rules.in_ether_type)
	            ip_offset += 4;
	        if(0 != prule->rules.dsa_info)
	            ip_offset += 8;
			if(0 != prule->rules.pppoe_flag)   /*add by wangjian for support pppoe 2013-3-21*/
				ip_offset += 8;

            work->word2.s.ip_offset = ip_offset;
            
    		if (prule->rules.action_type == FLOW_ACTION_ETH_FORWARD) 
    		{
    			pko_command.s.ipoffp1 = ip_offset + 1;
    		}
    		else if (prule->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD)
    		{
    			int qos = capwap_with_wifi_qos(work);
                if(qos == 1){
    			    pko_command.s.ipoffp1 = IP_H_LEN + UDP_H_LEN + CW_H_LEN + 
    						IEEE80211_QOS_H_LEN + LLC_H_LEN + ip_offset + 1;
    			}
    			else{
    			    pko_command.s.ipoffp1 = IP_H_LEN + UDP_H_LEN + CW_H_LEN + 
    						IEEE80211_H_LEN + LLC_H_LEN + ip_offset + 1;
    			}
    		}
    		else if (prule->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD) 
    		{
    			pko_command.s.ipoffp1 = IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN + ip_offset + 1;
    		}
		}
	}

	
	ret = 	cvmx_pko_send_packet_finish(port, queue, pko_command, packet_ptr, lock_type);
	
	if (ret != CVMX_PKO_SUCCESS)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
				"send_packet_pip_out failed to send packet using cvmx_pko_send_packet_finish, ErrorCode=%d\n",ret);

		cvmx_helper_free_packet_data(work); /*free data*/
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		return RETURN_ERROR; 		
	}
	
	cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS, 1);
	cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_BYTES, CVM_WQE_GET_LEN(work));

	if(tag == 1)
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"send_packet_pip_out: send out from port %ld, success\r\n", port);
	return RETURN_OK; 	
}

/*
 * Setup the Cavium Simple Executive Libraries using defaults
 */
static int cvmx_init(void)
{

	if (setup_free_mem() != RETURN_OK)
	{
		printf("Error allocating FPA memory\n");
		return (RETURN_ERROR);
	}

	if (setup_input_ports() != RETURN_OK)
	{
		printf("Error setup OCTEON input ports\n");
		return RETURN_ERROR;
	}
	CVMX_SYNCWS;

	printf("\nSE OCTEON  hardware init OK\n");

	return RETURN_OK;
}
/**
 * Description:
 *  Encap pppoe Header
 *
 * Parameter:
 *  work: 
 *  rule: 
 *  pkt_ptr: Internal IP 
 *
 * Return:
 *  void
 *
 */
static inline void encap_pppoe(cvmx_wqe_t *work, rule_item_t *rule, uint8_t **pkt_ptr)
{
	/*add by wangjian for support pppoe 2013-3-20 */
	*pkt_ptr = (uint8_t *)*pkt_ptr - 2;
	*(uint16_t *)*pkt_ptr = PPPOE_IP_TYPE;
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 2);
	*pkt_ptr = (uint8_t *)*pkt_ptr - 2;
	*(uint16_t *)*pkt_ptr = CVM_WQE_GET_LEN(work);   		/* length ? */
	*pkt_ptr = (uint8_t *)*pkt_ptr - 2;
	*(uint16_t *)*pkt_ptr = rule->rules.pppoe_session_id;  	/* session_id */
	*pkt_ptr = (uint8_t *)*pkt_ptr - 2;
	*(uint16_t *)*pkt_ptr = 0x1100;               			/*  ver0x1 type0x1 code0x00 */
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 6);

	return;
}


/*
 * Encapsulate Eth type packet by ipfwd rule.
 * offset: the offset of the ip address from the begining of the packet
 */
static inline void encap_eth_packet(cvmx_wqe_t *work, rule_item_t *rule, uint32_t offset)
{
	uint8_t *pkt_ptr = NULL;
	cvm_common_ip_hdr_t *ip = NULL;
	uint8_t vlan_flag = 0;

	if((work == NULL) || (rule == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"encap_eth_packet: work or rule is Null!\r\n");
		return ;
	}

	pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
	ip = (cvm_common_ip_hdr_t *)(pkt_ptr + offset); 	

	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) - offset);


	/* add by yin for fwd nat *//*wangjian_nat*/
	if (rule->rules.nat_flag == 1) 
	{
		cvm_common_udp_hdr_t *uh = NULL;
		cvm_common_tcp_hdr_t *th = NULL;

		uh = ( cvm_common_udp_hdr_t*)((uint32_t *)ip + ip->ip_hl);
		th = ( cvm_common_tcp_hdr_t*)uh;

		ip->ip_src = rule->rules.nat_sip;
		ip->ip_dst = rule->rules.nat_dip;
		th->th_sport = rule->rules.nat_sport;
		th->th_dport = rule->rules.nat_dport;
	}

	ip->ip_ttl--;
	ip->ip_sum= 0;
	ip->ip_sum = cvm_ip_calculate_ip_header_checksum(ip);

	/*add by wangjian for support pppoe 2013-3-12 be more carefule pkt_ptr can use uint8_t **pkt_ptr */
	pkt_ptr = (uint8_t *)ip;
	if (rule->rules.pppoe_flag)
	{
		/* different process by flag */
		encap_pppoe(work, rule, &pkt_ptr);
		cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_ETH_PPPOE, 1);
	}
	/*add by wangjian for support pppoe 2013-3-12*/
	
	/* [DMAC-6][SMAC-6][DSA-8][TAG1-4][TAG4][TYPE-2]*/    /*倒序赋值*/
	/*add by wangjian for support pppoe 2013-3-12*/
	pkt_ptr = (uint8_t *)pkt_ptr - 2;
	/*add by wangjian for support pppoe 2013-3-14 type 0x8864? */
	*(uint16_t *)pkt_ptr = rule->rules.ether_type;
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 2);

	if(rule->rules.in_tag) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.in_tag;

		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.in_ether_type;

		vlan_flag = 1;
		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 4);		
	}

	if(rule->rules.out_tag) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.out_tag;

		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.out_ether_type;

		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 4);	
		if (vlan_flag)
		{
			cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_QINQ, 1);
		}
		else
		{
			cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_8021Q, 1);
		}
	}

	if(rule->rules.dsa_info) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - PACKET_DSA_HEADER_LEN;
		*(uint64_t *)pkt_ptr = rule->rules.dsa_info;
		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + PACKET_DSA_HEADER_LEN);	
	}

	pkt_ptr = pkt_ptr - 12;
	memcpy(pkt_ptr, rule->rules.ether_dhost, 12);
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 12);
	work->packet_ptr.s.addr = cvmx_ptr_to_phys(pkt_ptr);
	work->packet_ptr.s.back   = CVM_COMMON_CALC_BACK(work->packet_ptr);

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"encap_eth_packet: back is %d, addr at 0x%llx\r\n",work->packet_ptr.s.back,(unsigned long long)work->packet_ptr.s.addr);

#ifdef OCTEON_DEBUG_LEVEL
	if((fastfwd_common_debug_level == FASTFWD_COMMON_DBG_LVL_INFO) &&((1 << cvmx_get_core_num() ) & (core_mask) ))
	{
		//cvmx_dump_128_packet(work);
		fwd_debug_dump_packet(work);
	}
#endif

	return;
}

/**
 * Encapsulate 802.11 capwap type packet by ipfwd rule.
 * offset: the offset of the rule(internal) ip address from the begining of the packet
 */
static inline void encap_802_11_cw_packet(cvmx_wqe_t *work, rule_item_t *rule, uint32_t offset)
{
	uint8_t *pkt_ptr = NULL;
	uint8_t *pkt_ptr_tmp = NULL;
	struct ieee80211_llc *llc_hdr = NULL;
	struct ieee80211_frame *ieee80211_hdr = NULL;
	union capwap_hd *cw_hdr = NULL;
	cvm_common_udp_hdr_t *ext_uh = NULL;
	cvm_common_ip_hdr_t *ext_ip = NULL;
	cvm_common_ip_hdr_t *ip = NULL;
	uint32_t in_ip_totlen = 0, ieee80211_len = 0;
	uint8_t vlan_flag = 0;
	uint8_t is_qos = 0;

	if((work == NULL) || (rule == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"encap_cw_packet: work or rule is Null!\r\n");
		return ;
	}

	if ( cvm_qos_enable || (rule->rules.acl_tunnel_wifi_header_fc[0] & IEEE80211_FC0_QOS_MASK))
		is_qos = 1;

	pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);	
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) - offset);
	ip = (cvm_common_ip_hdr_t *)(pkt_ptr + offset); 	

	/* add by yin for fwd nat *//*wangjian_nat*/
	if (rule->rules.nat_flag == 1) 
	{
		cvm_common_udp_hdr_t *uh = NULL;
		uh = ( cvm_common_udp_hdr_t*)((uint32_t *)ip + ip->ip_hl);
		ip->ip_src = rule->rules.nat_sip;
		ip->ip_dst = rule->rules.nat_dip;
		uh->uh_sport = rule->rules.nat_sport;
		uh->uh_dport = rule->rules.nat_dport;
	}

	
	ip->ip_ttl--;
	in_ip_totlen = ip->ip_len;	
	ip->ip_sum= 0;
	ip->ip_sum = cvm_ip_calculate_ip_header_checksum(ip);


	/*
	   CAPWAP frame format:

	   Ethernet II header
	   IP header (External IP header)
	   UDP header (External UDP header)
	   CAPWAP header
	   IEEE802.11 header
	   LLC header
	   IP header (Internal IP header)
	   TCP/UDP header (Internal TCP/UDP header)
	   payload
	 */

	/*add by wangjian for support pppoe 2013-3-12*/
	pkt_ptr_tmp = (uint8_t *)ip;
	if (rule->rules.pppoe_flag)
	{
		encap_pppoe(work, rule, &pkt_ptr_tmp);
		cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_CAPWAP_PPPOE, 1);
	}
	/*add by wangjian for support pppoe 2013-3-12*/

	/* Encap LLC */	

	/*add by wangjian for support pppoe 2013-3-14*/
	if (rule->rules.pppoe_flag)
	{	
		llc_hdr = (struct ieee80211_llc *)(pkt_ptr + offset - LLC_H_LEN - PPPOE_H_LEN); 	
		llc_hdr->llc_ether_type[0] = 0x88;
		llc_hdr->llc_ether_type[1] = 0x64;
	}
	else
	{
		llc_hdr = (struct ieee80211_llc *)(pkt_ptr + offset - LLC_H_LEN); 	
		llc_hdr->llc_ether_type[0] = 0x08;
		llc_hdr->llc_ether_type[1] = 0x0;
	}
	/*add by wangjian for support pppoe 2013-3-14*/
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + LLC_H_LEN);
	llc_hdr->llc_dsap = 0xaa;
	llc_hdr->llc_ssap = 0xaa;
	llc_hdr->llc_cmd = 0x03;
	llc_hdr->llc_org_code[0] = 0;
	llc_hdr->llc_org_code[1] = 0;
	llc_hdr->llc_org_code[2] = 0;
	
	
	
	/* Encap IEEE802.11 */
	if (is_qos == 0) 
	{
		ieee80211_len = IEEE80211_H_LEN;
		ieee80211_hdr = (struct ieee80211_frame*)((uint8_t*)llc_hdr - IEEE80211_H_LEN);	
	}	
	else 
	{
		ieee80211_len = IEEE80211_QOS_H_LEN;
		ieee80211_hdr = (struct ieee80211_frame*)((uint8_t*)llc_hdr - IEEE80211_QOS_H_LEN);
	}

	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + ieee80211_len);
	ieee80211_hdr->i_fc[0] = rule->rules.acl_tunnel_wifi_header_fc[0];
	ieee80211_hdr->i_fc[1] = rule->rules.acl_tunnel_wifi_header_fc[1];
	if (cvmx_unlikely(is_qos)) 
	{
		ieee80211_hdr->i_fc[0] |= IEEE80211_FC0_QOS_MASK;
		((struct ieee80211_qosframe*)ieee80211_hdr)->i_qos[0] = rule->rules.acl_tunnel_wifi_header_qos[0];
		((struct ieee80211_qosframe*)ieee80211_hdr)->i_qos[1] = rule->rules.acl_tunnel_wifi_header_qos[1];
		unsigned char *pchNull = NULL;
		pchNull = ((struct ieee80211_qosframe*)ieee80211_hdr)->i_qos + 2;

		*pchNull = 0x00;
		pchNull += 1;
		*pchNull = 0x00;
	}	
	ieee80211_hdr->i_dur[0] = 0;
	ieee80211_hdr->i_dur[1] = 0;

	memcpy(ieee80211_hdr->i_addr1, &rule->rules.acl_tunnel_wifi_header_addr[0], MAC_LEN * 3);

	ieee80211_hdr->i_seq[0] = (((uint16_t)gbl_80211_id) << 4) & 0xf0;
	ieee80211_hdr->i_seq[1] = (((uint16_t)gbl_80211_id) >> 4) & 0xff;
	cvmx_atomic_add32_nosync(&gbl_80211_id,1);
	if(gbl_80211_id >= 4096)
	{
		gbl_80211_id = 0;
	}

	/* Encap CAPWAP */
	cw_hdr = (union capwap_hd*)((uint8_t*)ieee80211_hdr - CW_H_LEN);
	memcpy(cw_hdr, capwap_cache_bl[rule->rules.tunnel_index].cw_hd, CW_H_LEN);
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + CW_H_LEN);

	/* Encap externel UDP */
	ext_uh =  (cvm_common_udp_hdr_t*)((uint8_t*)cw_hdr - UDP_H_LEN);
	ext_uh->uh_sport = capwap_cache_bl[rule->rules.tunnel_index].sport;
	ext_uh->uh_dport = capwap_cache_bl[rule->rules.tunnel_index].dport;
	ext_uh->uh_ulen = in_ip_totlen +  LLC_H_LEN + ieee80211_len + CW_H_LEN + UDP_H_LEN;
	ext_uh->uh_sum= 0;
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + UDP_H_LEN);

	/* Encap external IP */
	ext_ip = (cvm_common_ip_hdr_t*)((uint8_t*)ext_uh - IP_H_LEN);
	ext_ip->ip_v = 4;
	ext_ip->ip_hl = 5;
	ext_ip->ip_tos = capwap_cache_bl[rule->rules.tunnel_index].tos;
	ext_ip->ip_len = ext_uh->uh_ulen + IP_H_LEN;
	ext_ip->ip_id = 0;
	ext_ip->ip_off = 0x4000;
	ext_ip->ip_ttl = DEFAULT_TTL;
	ext_ip->ip_p = CVM_COMMON_IPPROTO_UDP;
	ext_ip->ip_src = capwap_cache_bl[rule->rules.tunnel_index].sip;
	ext_ip->ip_dst = capwap_cache_bl[rule->rules.tunnel_index].dip;
	ext_ip->ip_sum = 0;
	ext_ip->ip_sum = cvm_ip_calculate_ip_header_checksum(ext_ip);
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + IP_H_LEN);

	/* [DMAC-6][SMAC-6][DSA-8][TAG1-4][TAG4][TYPE-2]*/    /*倒序赋值*/	
	pkt_ptr = (uint8_t *)ext_ip - 2;
	*(uint16_t *)pkt_ptr = rule->rules.ether_type;
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 2);

	if(rule->rules.in_tag) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.in_tag;

		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.in_ether_type;

		vlan_flag = 1;
		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 4);
	}

	if(rule->rules.out_tag) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.out_tag;

		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.out_ether_type;

		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 4);
		if (vlan_flag)
		{
			cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_QINQ, 1);
		}
		else
		{
			cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_8021Q, 1);
		}
	}

	if(rule->rules.dsa_info) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - PACKET_DSA_HEADER_LEN;
		*(uint64_t *)pkt_ptr = rule->rules.dsa_info;
		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + PACKET_DSA_HEADER_LEN);
	}

	pkt_ptr = pkt_ptr - 12;
	memcpy(pkt_ptr, rule->rules.ether_dhost, 12);
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 12);
	work->packet_ptr.s.addr = cvmx_ptr_to_phys(pkt_ptr);
	work->packet_ptr.s.back   = CVM_COMMON_CALC_BACK(work->packet_ptr);

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"encap_cw_packet: back is %d, addr at 0x%llx\r\n",work->packet_ptr.s.back,(unsigned long long)work->packet_ptr.s.addr);

#ifdef OCTEON_DEBUG_LEVEL
	if((fastfwd_common_debug_level == FASTFWD_COMMON_DBG_LVL_INFO) &&((1 << cvmx_get_core_num() ) & (core_mask) ))
	{
		//cvmx_dump_128_packet(work);
		fwd_debug_dump_packet(work);
	}
#endif

	return;
}

/**
 * Encapsulate 802.3 capwap type packet by ipfwd rule.
 * offset: the offset of the rule(internal) ip address from the begining of the packet
 */
static inline void encap_802_3_cw_packet(cvmx_wqe_t *work, rule_item_t *rule, uint32_t offset)
{
	uint8_t *pkt_ptr = NULL;
	union capwap_hd *cw_hdr = NULL;
	cvm_common_udp_hdr_t *ext_uh = NULL;
	cvm_common_ip_hdr_t *ext_ip = NULL;
	cvm_common_ip_hdr_t *ip = NULL;
	uint32_t in_ip_totlen = 0;
	uint8_t vlan_flag = 0;

	if((work == NULL) || (rule == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"encap_cw_packet: work or rule is Null!\r\n");
		return ;
	}

	pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);			
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) - offset);
	ip = (cvm_common_ip_hdr_t *)(pkt_ptr + offset); 	

	/* add by yin for fwd nat *//*wangjian_nat*/
	if (rule->rules.nat_flag == 1) 
	{
		cvm_common_udp_hdr_t *uh = NULL;
		uh = ( cvm_common_udp_hdr_t*)((uint32_t *)ip + ip->ip_hl);
		ip->ip_src = rule->rules.nat_sip;
		ip->ip_dst = rule->rules.nat_dip;
		uh->uh_sport = rule->rules.nat_sport;
		uh->uh_dport = rule->rules.nat_dport;
	}

	
	ip->ip_ttl--;
	in_ip_totlen = ip->ip_len;	
	ip->ip_sum= 0;
	ip->ip_sum = cvm_ip_calculate_ip_header_checksum(ip);

	/*
	   CAPWAP 802.3 frame format:

	   Ethernet II header
	   IP header (External IP header)
	   UDP header (External UDP header)
	   CAPWAP header
	   Ethernet II header
	   IP header (Internal IP header)
	   TCP/UDP header (Internal TCP/UDP header)
	   payload
	 */

	/*add by wangjian for support pppoe 2013-3-12*/
	pkt_ptr = (uint8_t *)ip;
	if (rule->rules.pppoe_flag)
	{
		encap_pppoe(work, rule, &pkt_ptr);
		cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_CAPWAP_PPPOE, 1);
	}
	/*add by wangjian for support pppoe 2013-3-12*/

	/* Encap internal MAC head */	
	/*add by wangjian for support pppoe 2013-3-12*/
	pkt_ptr = (uint8_t *)pkt_ptr - 2;
	*(uint16_t *)pkt_ptr = rule->rules.acl_tunnel_eth_header_ether;   /* add by wangjian for support pppoe 2013-3-14 0x8864? */

	pkt_ptr = pkt_ptr - 12;
	memcpy(pkt_ptr, rule->rules.acl_tunnel_eth_header_dmac, 12);
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + ETH_H_LEN);

	/* Encap CAPWAP */
	cw_hdr = (union capwap_hd*)(pkt_ptr - CW_H_LEN);
	memcpy(cw_hdr, capwap_cache_bl[rule->rules.tunnel_index].cw_hd, CW_H_LEN);	
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + CW_H_LEN);

	/* Encap externel UDP */
	ext_uh = (cvm_common_udp_hdr_t*)((uint8_t*)cw_hdr - UDP_H_LEN);
	ext_uh->uh_sport = capwap_cache_bl[rule->rules.tunnel_index].sport;
	ext_uh->uh_dport = capwap_cache_bl[rule->rules.tunnel_index].dport;
	ext_uh->uh_ulen= in_ip_totlen +  ETH_H_LEN  + CW_H_LEN + UDP_H_LEN;
	ext_uh->uh_sum= 0;
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + UDP_H_LEN);

	/* Encap external IP */
	ext_ip = (cvm_common_ip_hdr_t*)((uint8_t*)ext_uh - IP_H_LEN);
	ext_ip->ip_v = 4;
	ext_ip->ip_hl = 5;
	ext_ip->ip_tos = capwap_cache_bl[rule->rules.tunnel_index].tos;
	ext_ip->ip_len = ext_uh->uh_ulen + IP_H_LEN;
	ext_ip->ip_id = 0;
	ext_ip->ip_off = 0x4000;
	ext_ip->ip_ttl = DEFAULT_TTL;
	ext_ip->ip_p= CVM_COMMON_IPPROTO_UDP;
	ext_ip->ip_src= capwap_cache_bl[rule->rules.tunnel_index].sip;
	ext_ip->ip_dst = capwap_cache_bl[rule->rules.tunnel_index].dip;
	ext_ip->ip_sum = 0;
	ext_ip->ip_sum = cvm_ip_calculate_ip_header_checksum(ext_ip);
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + IP_H_LEN);

	/* [DMAC-6][SMAC-6][DSA-8][TAG1-4][TAG4][TYPE-2]*/    /*倒序赋值*/	
	pkt_ptr = (uint8_t *)ext_ip - 2;
	*(uint16_t *)pkt_ptr = rule->rules.ether_type;
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 2);

	if(rule->rules.in_tag) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.in_tag;

		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.in_ether_type;

		vlan_flag = 1;
		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 4);
	}

	if(rule->rules.out_tag) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.out_tag;

		pkt_ptr = (uint8_t *)pkt_ptr - 2;
		*(uint16_t *)pkt_ptr = rule->rules.out_ether_type;

		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 4);
		if (vlan_flag)
		{
			cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_QINQ, 1);
		}
		else
		{
			cvmx_fau_atomic_add64(CVM_FAU_ENET_OUTPUT_PACKETS_8021Q, 1);
		}
	}

	if(rule->rules.dsa_info) 
	{
		pkt_ptr = (uint8_t *)pkt_ptr - PACKET_DSA_HEADER_LEN;
		*(uint64_t *)pkt_ptr = rule->rules.dsa_info;
		CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + PACKET_DSA_HEADER_LEN);
	}

	pkt_ptr = pkt_ptr - 12;
	memcpy(pkt_ptr, rule->rules.ether_dhost, 12);
	CVM_WQE_SET_LEN(work, CVM_WQE_GET_LEN(work) + 12);
	work->packet_ptr.s.addr = cvmx_ptr_to_phys(pkt_ptr);
	work->packet_ptr.s.back   = CVM_COMMON_CALC_BACK(work->packet_ptr);

	FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
			"encap_cw_packet: back is %d, addr at 0x%llx\r\n",work->packet_ptr.s.back,(unsigned long long)work->packet_ptr.s.addr);

#ifdef OCTEON_DEBUG_LEVEL
	if((fastfwd_common_debug_level == FASTFWD_COMMON_DBG_LVL_INFO) &&((1 << cvmx_get_core_num() ) & (core_mask) ))
	{
		//cvmx_dump_128_packet(work);
		fwd_debug_dump_packet(work);
	}
#endif

	return;
}

/**
 * Description:
 * Decap received frame Lay2 header and get IP header offset.
 * Not finish lutao tag
 * Return:
 *  0: Successfully
 * -1: Failed
 *
 *
static inline int8_t rx_l2hdr_decap(cvmx_wqe_t *work, cvm_common_ip_hdr_t **ip)
{
	return RETURN_OK;
}*/

/*
Parsing the L2 header
@eth_head  the pointer of L2 header
@in_head    the pointer of address of ip header ,return ip header address
*/
inline int8_t rx_l2hdr_decap( uint8_t* eth_head,cvm_common_ip_hdr_t **ip_head)
{	
	uint16_t  protocol = 0;
	uint8_t *tmp_eth_head = NULL;
	
	if((NULL == eth_head) || (NULL == ip_head))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,\
								"rx_l2hdr_decap param error\n");
		return RETURN_ERROR;
	}

	tmp_eth_head = (uint8_t*)eth_head;
	protocol =*(uint16_t*)(tmp_eth_head + MAC_LEN*2);

	if(CVM_ETH_P_IP == protocol)
	{
		*ip_head = (cvm_common_ip_hdr_t*)(tmp_eth_head + ETH_H_LEN);
		return RETURN_OK;
	}
	else if(CVM_ETH_P_8021Q == protocol)
	{
		protocol = *(uint16_t*)(tmp_eth_head + ETH_H_LEN + VLAN_PROTO_LEN);
		if(protocol == CVM_ETH_P_8021Q)
		{
			protocol = *(uint16_t*)(tmp_eth_head + ETH_H_LEN + VLAN_TAG_LEN + VLAN_PROTO_LEN);
			if(CVM_ETH_P_IP == protocol)
			{
				*ip_head = (cvm_common_ip_hdr_t*)(tmp_eth_head + ETH_H_LEN + VLAN_TAG_LEN*2);
				return RETURN_OK;
			}
			/*add by wangjian for support pppoe 2013-3-14*/
			else if (PPPOE_TYPE == protocol)
			{
				if (PPPOE_IP_TYPE == *(uint16_t*)(tmp_eth_head + ETH_H_LEN + VLAN_TAG_LEN*2 + 6))
				{
					*ip_head = (cvm_common_ip_hdr_t*)(tmp_eth_head + ETH_H_LEN + VLAN_TAG_LEN*2 + PPPOE_H_LEN);
					return RETURN_OK;
				}
				else
				{
					cvmx_fau_atomic_add64(CVM_FAU_ENET_ETH_PPPOE_NONIP_PACKETS, 1);
					return RETURN_ERROR;
				}
			}
			/*add by wangjian for support pppoe 2013-3-14*/
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"rx_l2hdr_decap Not normal QinQ packets,eth type = %04x \r\n",protocol);
				return RETURN_ERROR;
			}
		}
		else if (CVM_ETH_P_IP == protocol)
		{
			*ip_head = (cvm_common_ip_hdr_t*)(tmp_eth_head + ETH_H_LEN + VLAN_TAG_LEN);
			return RETURN_OK;
		}
		/*add by wangjian for support pppoe 2013-3-14*/
		else if (PPPOE_TYPE == protocol)
		{
			if (PPPOE_IP_TYPE == *(uint16_t*)(tmp_eth_head + ETH_H_LEN + VLAN_TAG_LEN + 6))
			{
				*ip_head = (cvm_common_ip_hdr_t*)(tmp_eth_head + ETH_H_LEN + VLAN_TAG_LEN + PPPOE_H_LEN);
				return RETURN_OK;
			}
			else
			{
				cvmx_fau_atomic_add64(CVM_FAU_ENET_ETH_PPPOE_NONIP_PACKETS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"rx_l2hdr_parse NonIP packets,eth type = %04x\r\n",protocol);
				return RETURN_ERROR;
			}
		}
		/*add by wangjian for support pppoe 2013-3-14*/
		else
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"rx_l2hdr_parse Not normal 802.1 packets,eth type = %04x\r\n",protocol);
			return RETURN_ERROR;
		}
	}
	/*add by wangjian for support pppoe 2013-3-14*/
	else if (PPPOE_TYPE == protocol)
	{
		if (PPPOE_IP_TYPE == *(uint16_t*)(tmp_eth_head + ETH_H_LEN + 6))
		{
			*ip_head = (cvm_common_ip_hdr_t*)(tmp_eth_head + ETH_H_LEN + PPPOE_H_LEN);
			return RETURN_OK;
		}
		else
		{
			cvmx_fau_atomic_add64(CVM_FAU_ENET_ETH_PPPOE_NONIP_PACKETS, 1);
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"rx_l2hdr_parse NonIP packets,eth type = %04x\r\n",protocol);
			return RETURN_ERROR;
		}
	}
	/*add by wangjian for support pppoe 2013-3-14*/
	else
	{ 
		/* no  ip */
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"rx_l2hdr_parse NonIP packets,eth type = %04x\r\n",protocol);
		return RETURN_ERROR;
	}
}


/**
 * Description:
 *  Decap CAPWAP packet, parse 802.11 frame and get internal IP and TCP Pointer.
 *
 * Parameter:
 *  ex_uh: External UDP header
 *  in_ip: Internal IP header
 *  in_th: Internal IP header
 *  is_qos: 802.11 qos flag
 *
 * Return:
 *  0: Successfully
 *  -1: Failed
 *
 */
static inline int8_t cw_802_11_decap(cvm_common_udp_hdr_t *ex_uh, 
		cvm_common_ip_hdr_t **in_ip, 
		cvm_common_tcp_hdr_t **in_th, 
		uint8_t *is_qos,
		uint8_t *is_pppoe)
{
	struct ieee80211_frame *ieee80211_hdr = NULL;
	struct ieee80211_llc *llc_hdr = NULL;
	uint16_t len = 0;

	if((ex_uh == NULL) || (in_ip == NULL) || (in_th == NULL) || (is_qos == NULL) || (is_pppoe == NULL))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"cw_skb_decap: input pointer NULL\n");
		return RETURN_ERROR;
	}

	ieee80211_hdr = (struct ieee80211_frame*)((uint8_t*)ex_uh + UDP_H_LEN + CW_H_LEN);

	if ((ieee80211_hdr->i_fc[0] & IEEE80211_FC0_VERSION_MASK) != IEEE80211_FC0_VERSION_0)
	{
		cvmx_fau_atomic_add64(CVM_FAU_CW_80211_ERR, 1);
		return RETURN_ERROR;
	}
	if ((ieee80211_hdr->i_fc[0] & IEEE80211_FC0_TYPE_MASK) != IEEE80211_FC0_TYPE_DATA)
	{
		cvmx_fau_atomic_add64(CVM_FAU_CW_80211_ERR, 1);
		return RETURN_ERROR;
	}

    /* get qos flag by 802.11header FC bit */
	if (ieee80211_hdr->i_fc[0] & IEEE80211_FC0_QOS_MASK) 
	{ 
		len = IEEE80211_QOS_H_LEN;
		*is_qos = 1;
	} 
	else
	{ 
		len = IEEE80211_H_LEN;
		*is_qos = 0;
	}

	llc_hdr = (struct ieee80211_llc*)((uint8_t*)ieee80211_hdr + len);

	/* IP */
	if ((llc_hdr->llc_ether_type[1] == 0x0) && (llc_hdr->llc_ether_type[0] == 0x08))
	{
		*in_ip = (cvm_common_ip_hdr_t*)((uint8_t*)llc_hdr + LLC_H_LEN);
	}
	/* PPPOE */
	else if ((llc_hdr->llc_ether_type[1] == 0x64) && (llc_hdr->llc_ether_type[0] == 0x88))
	{
		/* PPPOE_IP */
		if (PPPOE_IP_TYPE == *(uint16_t *)((uint8_t *)llc_hdr + LLC_H_LEN + 6))
		{
			*in_ip = (cvm_common_ip_hdr_t*)((uint8_t*)llc_hdr + LLC_H_LEN + PPPOE_H_LEN);
			*is_pppoe = 1;
		}
		else
		{
			cvmx_fau_atomic_add64(CVM_FAU_CW_NOIP_PACKETS,1);
			return RETURN_ERROR;
		}
	}
	else
	{
		cvmx_fau_atomic_add64(CVM_FAU_CW_NOIP_PACKETS,1);
		return RETURN_ERROR;
	}
	
	/*add by wangjian for support pppoe 2013-3-14*/
	
	if (SPE_IP_ADDR((*in_ip)->ip_src, (*in_ip)->ip_dst) || SPE_IP_HDR(*in_ip))
	{
		cvmx_fau_atomic_add64(CVM_FAU_CW_SPE_PACKETS,1);
		return RETURN_ERROR;
	}
    /* filter frag */
	if(FRAG_IP_PKT(*in_ip))
	{
		cvmx_fau_atomic_add64(CVM_FAU_CW_FRAG_PACKETS,1);
		return RETURN_ERROR;
	}


	if (1 == *is_pppoe)
	{
		*in_th = (cvm_common_tcp_hdr_t*)((uint8_t*)llc_hdr + LLC_H_LEN + IP_H_LEN + PPPOE_H_LEN);
	}
	else 
	{
		*in_th = (cvm_common_tcp_hdr_t*)((uint8_t*)llc_hdr + LLC_H_LEN + IP_H_LEN);
	}


	/* decap tcp */
	if ((*in_ip)->ip_p == CVM_COMMON_IPPROTO_TCP)
	{
	    /* modify for NAT requirement. zhaohan 2012-12-14 */		
        if (SPE_TCP_HDR(*in_th)) 
        {
            cvmx_fau_atomic_add64(CVM_FAU_CW_SPE_TCP_HDR, 1);
            return RETURN_ERROR;
        }  
	}

    /* decap udp */
    if((*in_ip)->ip_p == CVM_COMMON_IPPROTO_UDP)
    {
        /* udp port num = 0 */
        if((*in_th)->th_dport == 0)
        {
            cvmx_fau_atomic_add64(CVM_FAU_CW_SPE_TCP_HDR, 1);
            return RETURN_ERROR;
        }
    }

    /* decap icmp */
    if((*in_ip)->ip_p == CVM_COMMON_IPPROTO_ICMP)
    {
        cvmx_fau_atomic_add64(CVM_FAU_CAPWAP_ICMP, 1);
    }

	return RETURN_OK;
}


/**
 * Description:
 *  Decap CAPWAP packet, parse 802.3 frame and get internal IP and TCP Pointer.
 *
 * Parameter:
 *  ex_uh: External UDP header
 *  in_ip: Internal IP header
 *  in_th: Internal th header
 *
 * Return:
 *  0: Successfully
 *  -1: Failed
 *
 */
static inline int8_t cw_802_3_decap(cvm_common_udp_hdr_t *ex_uh, 
		cvm_common_ip_hdr_t **in_ip, 
		cvm_common_tcp_hdr_t **in_th,
		uint8_t *is_pppoe)
{
    eth_hdr* eth_header = NULL;

	if((NULL == ex_uh) || (NULL == in_ip) || (NULL == in_th) || (NULL == is_pppoe))
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"Warning:cw_802_3_decap input pointer NULL\n");

		return RETURN_ERROR;
	}

    eth_header = (eth_hdr*)((uint8_t*)ex_uh + UDP_H_LEN + CW_H_LEN);


	
	if (ETH_P_IP == eth_header->h_vlan_proto)
	{
		*in_ip = (cvm_common_ip_hdr_t*)((uint8_t*)ex_uh + UDP_H_LEN + CW_H_LEN + ETH_H_LEN);
	} 
	else if (PPPOE_TYPE == eth_header->h_vlan_proto)
	{
		if (PPPOE_IP_TYPE == *(uint16_t *)((uint8_t *)ex_uh + UDP_H_LEN + CW_H_LEN + ETH_H_LEN + 6))	
		{
			*in_ip = (cvm_common_ip_hdr_t*)((uint8_t*)ex_uh + UDP_H_LEN + CW_H_LEN + ETH_H_LEN + PPPOE_H_LEN);
			*is_pppoe = 1;
		}
		else 
		{
			cvmx_fau_atomic_add64(CVM_FAU_CW_NOIP_PACKETS,1);
			return RETURN_ERROR;
		}
	}
	else
	{
		cvmx_fau_atomic_add64(CVM_FAU_CW_NOIP_PACKETS,1);
		return RETURN_ERROR;
	}

    /* filter special ip header/address */
	if (SPE_IP_ADDR((*in_ip)->ip_src, (*in_ip)->ip_dst) || SPE_IP_HDR(*in_ip))
	{
	    cvmx_fau_atomic_add64(CVM_FAU_CW_SPE_PACKETS,1);
		return RETURN_ERROR;
	}
    /* filter frag */
    if(FRAG_IP_PKT(*in_ip))
    {
        cvmx_fau_atomic_add64(CVM_FAU_CW_FRAG_PACKETS,1);
        return RETURN_ERROR;
    }


	if (1 == *is_pppoe)
	{
		*in_th = (cvm_common_tcp_hdr_t*)((uint8_t*)*in_ip + IP_H_LEN + PPPOE_H_LEN);
	}
	else
	{
		*in_th = (cvm_common_tcp_hdr_t*)((uint8_t*)*in_ip + IP_H_LEN);
	}
	
    /* decap tcp */
    if ((*in_ip)->ip_p == CVM_COMMON_IPPROTO_TCP)
    {
        /* modify for NAT requirement. zhaohan 2012-12-14 */        
        if (SPE_TCP_HDR(*in_th)) 
        {
            cvmx_fau_atomic_add64(CVM_FAU_CW_SPE_TCP_HDR, 1);
            return RETURN_ERROR;
        }  
    }

    /* decap udp */
    if((*in_ip)->ip_p == CVM_COMMON_IPPROTO_UDP)
    {
        /* udp port num = 0 */
        if((*in_th)->th_dport == 0)
        {
            cvmx_fau_atomic_add64(CVM_FAU_CW_SPE_TCP_HDR, 1);
            return RETURN_ERROR;
        }
    }

    /* decap icmp */
    if((*in_ip)->ip_p == CVM_COMMON_IPPROTO_ICMP)
    {
        cvmx_fau_atomic_add64(CVM_FAU_CAPWAP_ICMP, 1);
    }

    return RETURN_OK;
}


int32_t flow_icmp_fast_path(rule_item_t *prule, cvmx_wqe_t *work, uint8_t is_qos, uint8_t is_pppoe)
{
	if((prule == NULL) || (work == NULL))
	{
		return RETURN_ERROR;
	}

	if(prule->rules.action_type == FLOW_ACTION_ICMP || \
	   prule->rules.action_type == FLOW_ACTION_RPA_ICMP)
	{
		if(CVM_WQE_GET_UNUSED(work) ==  PACKET_TYPE_ICMP)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: ETH ICMP ==> ETH ICMP.\n");
			encap_eth_packet(work, prule,work->word2.s.ip_offset);
			if(prule->rules.action_type == FLOW_ACTION_RPA_ICMP)
			{
				add_rpa_head(work, prule);
			}
			
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"flow_icmp_fast_path : Sending the icmp packet ERROR!\r\n");                    
			}
			return RETURN_OK;
		}
		else if(CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_11)
		{
			uint32_t offset = 0;
			offset =  IP_H_LEN + UDP_H_LEN + CW_H_LEN + 
				IEEE80211_H_LEN + LLC_H_LEN + work->word2.s.ip_offset;
			if (is_qos)
				offset += IEEE80211_QOS_DIFF_LEN;

			/*add by wangjian for support pppoe 2013-4-9 be careful with offset*/
			if (is_pppoe)
			{
				offset += PPPOE_H_LEN;
			}
			/*add by wangjian for support pppoe 2013-4-9 be careful with offset*/
			
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: CAPWAP 802.11 ICMP ==> ETH ICMP.\n");
			encap_eth_packet(work, prule, offset);
			if(prule->rules.action_type == FLOW_ACTION_RPA_ICMP)
			{
				add_rpa_head(work, prule);
			}
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"flow_icmp_fast_path : Sending the icmp packet ERROR!\r\n");                    
			}
			return RETURN_OK;
		}
		else if(CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_3)
		{
			uint32_t offset = 0;
			offset =  IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN - work->word2.s.ip_offset;

			/*add by wangjian for support pppoe 2013-4-9 be careful with offset*/
			if (is_pppoe)
			{
				offset += PPPOE_H_LEN;
			}
			/*add by wangjian for support pppoe 2013-4-9 be careful with offset*/

			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process:CAPWAP 802.3 ICMP ==> ETH ICMP.\n");
			encap_eth_packet(work, prule,offset);
			if(prule->rules.action_type == FLOW_ACTION_RPA_ICMP)
			{
				add_rpa_head(work, prule);
			}
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"flow_icmp_fast_path : Sending the icmp packet ERROR!\r\n");                    
			}
			return RETURN_OK;
		}
	}
	if(prule->rules.action_type == FLOW_ACTION_CAPWAP_802_11_ICMP)
	{
		if(CVM_WQE_GET_UNUSED(work) ==  PACKET_TYPE_ICMP)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: ETH ICMP ==> CAPWAP 802.11 ICMP .\n");
			encap_802_11_cw_packet(work, prule,work->word2.s.ip_offset);
			if(prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_11_ICMP)
			{
				add_rpa_head(work, prule);
			}
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"flow_icmp_fast_path : Sending the icmp packet ERROR!\r\n");                    
			}
			return RETURN_OK;
		}
		else if(CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_11)
		{
			uint32_t offset = 0;
			offset =  IP_H_LEN + UDP_H_LEN + CW_H_LEN + 
				IEEE80211_H_LEN + LLC_H_LEN + work->word2.s.ip_offset;
			if (is_qos)
				offset += IEEE80211_QOS_DIFF_LEN;

			/*add by wangjian for support pppoe 2013-4-9 be careful with offset*/
			if (is_pppoe)
			{
				offset += PPPOE_H_LEN;
			}
			/*add by wangjian for support pppoe 2013-4-9 be careful with offset*/

			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: CAPWAP 802.11 ICMP ==> CAPWAP 802.11 ICMP.\n");
			encap_802_11_cw_packet(work, prule, offset);
			if(prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_11_ICMP)
			{
				add_rpa_head(work, prule);
			}
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"flow_icmp_fast_path : Sending the icmp packet ERROR!\r\n");                    
			}
			return RETURN_OK;
		}
	}
	if(prule->rules.action_type == FLOW_ACTION_CAPWAP_802_3_ICMP)
	{
		if(CVM_WQE_GET_UNUSED(work) ==  PACKET_TYPE_ICMP)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: ETH ICMP ==> CAPWAP 802.3 ICMP .\n");
			encap_802_3_cw_packet(work, prule,work->word2.s.ip_offset);
			if(prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_3_ICMP)
			{
				add_rpa_head(work, prule);
			}
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"flow_icmp_fast_path : Sending the icmp packet ERROR!\r\n");                    
			}
			return RETURN_OK;
		}
		else if(CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_3)
		{
			uint32_t offset = 0;
			offset =  IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN - work->word2.s.ip_offset;

			/*add by wangjian for support pppoe 2013-4-9 be careful with offset*/
			if (is_pppoe)
			{
				offset += PPPOE_H_LEN;
			}
			/*add by wangjian for support pppoe 2013-4-9 be careful with offset*/

			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: CAPWAP 802.3 ICMP ==> CAPWAP 802.3 ICMP.\n");
			encap_802_3_cw_packet(work, prule,offset);
			if(prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_3_ICMP)
			{
				add_rpa_head(work, prule);
			}
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"flow_icmp_fast_path : Sending the icmp packet ERROR!\r\n");                    
			}
			return RETURN_OK;
		}
	}

	return RETURN_OK;
}


/*
 * send packet to linux from pow interface
 */
inline void pow_pkt_to_control(cvmx_wqe_t *work)
{
	if(cvm_traffic_monitor_func == FUNC_ENABLE)
	{
		traffic_statistics(work);
	}

	// move this to cvm_rate_limit function
    //if(cvm_rate_limit_enabled == FUNC_ENABLE)
    {
        if(cvm_rate_limit(work))
        {
            cvmx_helper_free_packet_data(work); /*free data*/                
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
			return;
        }
    }
	
	/* cvmx_pow_tag_sw_desched(work->tag, CVMX_POW_TAG_TYPE_ATOMIC, grp, 0); */
    cvmx_pow_work_submit(work, 
                            CVM_WQE_GET_TAG(work), 
                            CVM_WQE_GET_TAG_TYPE(work), 
                            CVM_WQE_GET_QOS(work), 
                            product_info.to_linux_group
                           );
}

/*
 * send fccp cmd to linux from pow interface
 */
inline void pow_fccp_to_control(cvmx_wqe_t *work)
{
    cvmx_pow_work_submit(work, 
                            CVM_WQE_GET_TAG(work), 
                            CVM_WQE_GET_TAG_TYPE(work), 
                            0, 
                            product_info.to_linux_fccp_group
                           );
}

/*
 * send packet to linux from pcie interface
 */
inline void nic_pkt_to_control(cvmx_wqe_t *work)
{
	if(cvm_traffic_monitor_func == FUNC_ENABLE)
	{
		traffic_statistics(work);
	}
	
	/* standalone mode, send packet to control plane through pcie interface */
	cvmcs_nic_forward_packet_to_host(work);
}

/*
 * send fccp to linux from pcie interface
 */
inline void nic_fccp_to_control(cvmx_wqe_t *work)
{
    cvmcs_process_instruction(work);
}


/* regist virual port */
/* vp_type: support virt */
int32_t fwd_register_virtual_port(uint32_t vp_type)
{   
    int32_t ret = RETURN_ERROR;
    
    switch(vp_type)
    {
        case VIRTUAL_PORT_POW:
            fwd_virtual_port.port_type = vp_type;
            fwd_virtual_port.ops.vp_pkt_to_control = pow_pkt_to_control;
            fwd_virtual_port.ops.vp_fccp_to_control = pow_fccp_to_control;
            ret = RETURN_OK;
            break;
        case VIRTUAL_PORT_PCIE:
            fwd_virtual_port.port_type = vp_type;
            fwd_virtual_port.ops.vp_pkt_to_control = nic_pkt_to_control;
            fwd_virtual_port.ops.vp_fccp_to_control = nic_fccp_to_control;
            ret = RETURN_OK;
            break;
        case VIRTUAL_PORT_XAUI:
            /* not support yet */
            break;
        default:
            break;
    }

    return ret;
}


/* add by sunjianchao */ 
static inline void send_wait_packet_link(extend_link_t *wait_link, rule_item_t *prule, int cnt_type)
{	
	work_node_t *cur = NULL, *per = NULL;

	int  lock_type;


	if(prule == NULL)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"send_wait_packet_link param is NULL!\n");
		return;    		
	}	

	for(cur = wait_link->head_link; cur; per = cur, cur = cur->next)
	{
		if(cur == wait_link->head_link)
		{
			//lock_type = CVMX_PKO_LOCK_ATOMIC_TAG;
			lock_type = CVMX_PKO_LOCK_NONE;
		}
		else
		{
			lock_type = CVMX_PKO_LOCK_NONE;
		}

		if(send_packet_pip_out(cur->work, prule,MEM_AUTO_FREE, lock_type) == RETURN_ERROR)
		{		

			cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
					"send_wait_packet_link: Sending the packet ERROR!\r\n");					
		}

		if(FLOW_ACTION_ETH_FORWARD == cnt_type)
		{
			atomic_add64_nosync(&(rule_cnt_info.fwd_eth_cnt));  
		}
		else if(FLOW_ACTION_CAPWAP_FORWARD == cnt_type)
		{
			atomic_add64_nosync(&(rule_cnt_info.fwd_cw_cnt));  
		}
		else if(FLOW_ACTION_CAP802_3_FORWARD == cnt_type)
		{
			atomic_add64_nosync(&(rule_cnt_info.fwd_cw_802_3_cnt));  
		}		 							 

		if(per)
		{
			cvmx_free(per);
			per = NULL;
		}
	}

	return ;

}

/*
   发送报文，包括处理缓存链表里的延迟发送的报文
 */
static inline void foward_action_process(cvmx_wqe_t *work, uint32_t action_type,
		cvm_common_ip_hdr_t *ip, cvm_common_tcp_hdr_t *th,rule_item_t *prule)
{
	extend_link_t *wait_link = NULL;
	cvmx_spinlock_t   *wait_lock = NULL;
	int lock_type = CVMX_PKO_LOCK_ATOMIC_TAG;

	if(flow_sequence_enable == FUNC_DISABLE)
	{
		/*发送当前work 的报文*/
		if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
		{
			cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
					"foward_action_process - FLOW_ACTION_ETH_FORWARD: Sending the packet ERROR!\r\n");                    
		}
		if(FLOW_ACTION_ETH_FORWARD == action_type)
		{
			atomic_add64_nosync(&(rule_cnt_info.fwd_eth_cnt));  
		}
		else if(FLOW_ACTION_CAPWAP_FORWARD == action_type)
		{
			atomic_add64_nosync(&(rule_cnt_info.fwd_cw_cnt));  
		}
		else if(FLOW_ACTION_CAP802_3_FORWARD == action_type)
		{
			atomic_add64_nosync(&(rule_cnt_info.fwd_cw_802_3_cnt));  
		}
		return;
	}


	wait_lock = &gl_wait_lock;
	cvmx_spinlock_lock(wait_lock);
	/*发往linux  处理的报文没有完全返回*/
	if(prule->rules.packet_wait > 0)
	{                   

		/*缓存的报文链表为空，创建链表*/
		if(NULL == prule->rules.extend_index)
		{
			prule->rules.extend_index = (extend_link_t*) cvmx_malloc(cvm_common_arenas, sizeof(extend_link_t));                      
			if(NULL == prule->rules.extend_index)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, 
						FASTFWD_COMMON_DBG_LVL_ERROR,
						"foward_action_process: can not get memory for packet buffer,line=%d\n",__LINE__);
				cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);			
				goto failed_handle_packet ;
			}

			memset(prule->rules.extend_index, 0, sizeof(extend_link_t));

			wait_link = prule->rules.extend_index;

			wait_link->head_link = (work_node_t *)cvmx_malloc(cvm_common_arenas, sizeof(work_node_t));
			if(NULL == wait_link->head_link)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, 
						FASTFWD_COMMON_DBG_LVL_ERROR,
						"foward_action_process: can not get memory for packet buffer,line=%d\n",__LINE__);
				cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);			
				goto failed_handle_packet ;
			}

			memset(wait_link->head_link, 0, sizeof(work_node_t));

			wait_link->tail_link = wait_link->head_link;
			wait_link->tail_link->work = work;
			wait_link->tail_link->next = NULL;
			wait_link->link_node_num++;
			CVMX_SYNC;                      

		}
		/*继续插入等待链表尾*/
		else
		{   

			wait_link = prule->rules.extend_index;
			wait_link->tail_link->next =(work_node_t *)cvmx_malloc(cvm_common_arenas, sizeof(work_node_t));
			if(NULL == prule->rules.extend_index)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, 
						FASTFWD_COMMON_DBG_LVL_ERROR,
						"foward_action_process: can not get memory for packet buffer,line=%d\n",__LINE__);
				cvmx_fau_atomic_add64(CVM_FAU_ALLOC_RULE_FAIL, 1);			
				goto failed_handle_packet ;
			}

			memset(wait_link->tail_link->next, 0, sizeof(work_node_t));
			wait_link->tail_link = wait_link->tail_link->next;
			wait_link->tail_link->work = work;
			wait_link->tail_link->next = NULL;
			wait_link->link_node_num++;

			CVMX_SYNC;                      

			/*当等待报文到达一定值，把链表报文发送*/
			if(wait_linux_buffer_count == wait_link->link_node_num)
			{                               
				send_wait_packet_link(wait_link, prule, action_type);

				if(prule->rules.extend_index)
				{
					cvmx_free(prule->rules.extend_index);
					prule->rules.extend_index = NULL;
				}

				prule->rules.packet_wait = 0;                       
			}

		}                   
		cvmx_spinlock_unlock(wait_lock);                    
		return;
	}
	else
		/*
		   该流转发到linux的报文数量是0，不需要因为保序继续缓存报文，
		   直接进行发送处理
		 */	
	{

		/*有缓存过的报文，先发送等待链表中的报文*/
		if(prule->rules.extend_index)
		{                   
			wait_link = prule->rules.extend_index;                   
			send_wait_packet_link(wait_link, prule, action_type);

			if(prule->rules.extend_index)
			{
				cvmx_free(prule->rules.extend_index);
				prule->rules.extend_index = NULL;
			}                       

			cvmx_spinlock_unlock(wait_lock);

			/*发送当前work 的报文*/
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_NONE) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"foward_action_process : Sending the packet ERROR!\r\n");
			}
			if(FLOW_ACTION_ETH_FORWARD == action_type)
			{
				atomic_add64_nosync(&(rule_cnt_info.fwd_eth_cnt));  
			}
			else if(FLOW_ACTION_CAPWAP_FORWARD == action_type)
			{
				atomic_add64_nosync(&(rule_cnt_info.fwd_cw_cnt));  
			}
			else if(FLOW_ACTION_CAP802_3_FORWARD == action_type)
			{
				atomic_add64_nosync(&(rule_cnt_info.fwd_cw_802_3_cnt));  
			}
		}           
		else /*无缓存的报文直接发送*/
		{
			cvmx_spinlock_unlock(wait_lock);
			/*发送当前work 报文*/
			if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, CVMX_PKO_LOCK_ATOMIC_TAG) == RETURN_ERROR)
			{
				cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"foward_action_process : Sending the packet ERROR!\r\n");   
			}
			if(FLOW_ACTION_ETH_FORWARD == action_type)
			{
				atomic_add64_nosync(&(rule_cnt_info.fwd_eth_cnt));  
			}
			else if(FLOW_ACTION_CAPWAP_FORWARD == action_type)
			{
				atomic_add64_nosync(&(rule_cnt_info.fwd_cw_cnt));  
			}
			else if(FLOW_ACTION_CAP802_3_FORWARD == action_type)
			{
				atomic_add64_nosync(&(rule_cnt_info.fwd_cw_802_3_cnt));  
			}
		}

		return;

	}            

	/*链表出错，则清空当前链表，并把当前work的报文一起发出去*/
failed_handle_packet:

	if(prule->rules.extend_index)
	{
		send_wait_packet_link(wait_link, prule, action_type);
		lock_type = CVMX_PKO_LOCK_NONE;
	}

	if(prule->rules.extend_index)
	{
		cvmx_free(prule->rules.extend_index);
		prule->rules.extend_index = NULL;
	}                       

	cvmx_spinlock_unlock(wait_lock);

	/*发送当前work 的报文*/
	if(send_packet_pip_out(work,prule,MEM_AUTO_FREE, lock_type) == RETURN_ERROR)
	{
		cvmx_fau_atomic_add64(CVM_FAU_PKO_ERRORS, 1);
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
				"foward_action_process - FLOW_ACTION_ETH_FORWARD: Sending the packet ERROR!\r\n");                    
	}

	return;

}


/*
 * execute the action
 *  lutao add
 */
void flow_action_process(cvmx_wqe_t *work, uint32_t action_type,
            cvm_common_ip_hdr_t *ip, cvm_common_tcp_hdr_t *th,rule_item_t *prule,uint8_t is_qos, cvmx_spinlock_t *first_lock, uint8_t is_pppoe)
{
	cvm_common_udp_hdr_t *uh = NULL;
	if(work == NULL)
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_WARNING,
				"flow_action_process: work has Null pointer!\r\n");
		return ;
	}

	if(prule == NULL) /*Get action info from action_type*/
	{
		/*Drop the packets*/
		if(action_type == FLOW_ACTION_DROP)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"Action: dropping the packet.....\r\n");

			cvmx_fau_atomic_add64(CVM_FAU_ENET_DROP_PACKETS, 1);

#ifdef OCTEON_DEBUG_LEVEL

			if(cvm_drop_dump_enable == FUNC_ENABLE)
			{
				printf("Drop the packet: \r\n");
				//cvmx_helper_dump_packet(work);
				fwd_debug_dump_packet(work);
			}
#endif
			cvmx_helper_free_packet_data(work); /*free data*/
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
			return;
		}
		else if(action_type == FLOW_ACTION_TOLINUX)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: Sending the packet to Control.....\r\n");
			cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 1);    
			cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_BYTES, CVM_WQE_GET_LEN(work)); 			
			fwd_virtual_port.ops.vp_pkt_to_control(work);
			return ;
		}
#if 0		
		else if(action_type == FLOW_ACTION_HEADER_FRAGMENTS_STORE) {		
			//printf("flow_action_process: Saving fragment packet .....\r\n");
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: Saving fragment packet .....\r\n"); 
			cvm_ip_reass_hash_additem(work, action_type, ip, th, prule, is_qos, is_pppoe);
			return ;
		} // add by yin	
#endif		
		else
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: action_type ERROR action_type=%d!\r\n",action_type);
			/*forward to the Linux*/
			fwd_virtual_port.ops.vp_pkt_to_control(work);
			cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 1);    
			cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_BYTES, CVM_WQE_GET_LEN(work)); 

			return ;
		}
	}
	else
	{
		/*********************************************************************/
#ifdef USER_TABLE_FUNCTION		
#ifdef CVM_CAR       
        /*对于上行报文，使用的是没有封装前的用户报文长度
           对于下行报文，使用的是没有去掉隧道的报文长度*/
        if(prule->rules.action_mask & FLOW_ACTION_METER )
        {
            cvmx_spinlock_lock(first_lock);
            if(!cvm_car_result(CVM_WQE_GET_LEN(work), prule->rules.user_index, prule->rules.user_link_index))
            {
                //cvmx_fau_atomic_add64(CVM_FAU_ENET_CAR_DROP_PACKETS, 1);
                cvmx_helper_free_packet_data(work); /*free data*/
                cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
                cvmx_spinlock_unlock(first_lock);       
                return;
            }
            cvmx_spinlock_unlock(first_lock);
        }
#endif
#endif
		
		if(prule->rules.action_type== FLOW_ACTION_DROP)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"Action: dropping the packet.....\r\n");

			cvmx_fau_atomic_add64(CVM_FAU_ENET_DROP_PACKETS, 1);

#ifdef OCTEON_DEBUG_LEVEL
			if(cvm_drop_dump_enable == FUNC_ENABLE)
			{
				printf("Drop the packet: \r\n");
				//cvmx_helper_dump_packet(work);
				fwd_debug_dump_packet(work);
			}
#endif
			cvmx_helper_free_packet_data(work); /*free data*/
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
			return ;
		}
		else if(prule->rules.action_type == FLOW_ACTION_TOLINUX)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: Sending the packet to Control.....\r\n"); 

			/*add by sunjc@autelan.com*/
			prule->rules.packet_wait++;
			CVMX_SYNC;

			if(CVM_COMMON_IPPROTO_UDP == ip->ip_p )
			{
				uh = (cvm_common_udp_hdr_t *)th;
				if((uh->uh_dport == PORTAL_PORT) || (uh->uh_sport == PORTAL_PORT) ||\
				(uh->uh_dport == ACCESS_RADUIS_PORT) || (uh->uh_sport == ACCESS_RADUIS_PORT)||\
				(uh->uh_dport == ACCOUNT_RADUIS_PORT) || (uh->uh_sport == ACCOUNT_RADUIS_PORT))
				{
					CVM_WQE_SET_QOS(work,0);
				}
			}
			cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 1);    
			cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_BYTES, CVM_WQE_GET_LEN(work)); 
			fwd_virtual_port.ops.vp_pkt_to_control(work);
			return ;
		}
		else if(prule->rules.action_type == FLOW_ACTION_ETH_FORWARD || \
			    prule->rules.action_type == FLOW_ACTION_RPA_ETH_FORWARD)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: eth forward the packet with the type = %d\r\n",CVM_WQE_GET_UNUSED(work));   

			if (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_ETH_IP) /* Eth ==> Eth */
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"flow_action_process: Eth ==> Eth.\n");

				encap_eth_packet(work, prule,work->word2.s.ip_offset);
				if(prule->rules.action_type == FLOW_ACTION_RPA_ETH_FORWARD)
				{
					add_rpa_head(work,prule);
				}

				foward_action_process(work, prule->rules.action_type,ip, th, prule);
				return;
			}
			else if (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_11) /* CAPWAP 802.11 ==> ETH */
			{           
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"flow_action_process: now in CAPWAP 802.11 ==> ETH.\n");

				uint32_t offset = 0;

				offset =  IP_H_LEN + UDP_H_LEN + CW_H_LEN + 
					IEEE80211_H_LEN + LLC_H_LEN + work->word2.s.ip_offset;
				if (is_qos)
					offset += IEEE80211_QOS_DIFF_LEN;

				/*add by wangjian for support pppoe 2013-3-14 be careful with offset*/
				if (is_pppoe)
				{
					offset += PPPOE_H_LEN;
				}
				/*add by wangjian for support pppoe 2013-3-14 be careful with offset*/

				encap_eth_packet(work, prule, offset);
				if(prule->rules.action_type == FLOW_ACTION_RPA_ETH_FORWARD)
				{
					add_rpa_head(work,prule);
				}

				foward_action_process(work, prule->rules.action_type,ip, th, prule);
				return;          

			}
			else if (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_3)  /* CAPWAP 802.3 ==> ETH */
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"flow_action_process: now in CAPWAP 802.3 ==> ETH.\n");
				uint32_t offset = 0;
				offset =  IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN + work->word2.s.ip_offset;

				/*add by wangjian for support pppoe 2013-3-14 be careful with offset*/
				if (is_pppoe)
				{
					offset += PPPOE_H_LEN;
				}
				/*add by wangjian for support pppoe 2013-3-14 be careful with offset*/

				encap_eth_packet(work, prule,offset);
				if(prule->rules.action_type == FLOW_ACTION_RPA_ETH_FORWARD)
				{
					add_rpa_head(work,prule);
				}

				foward_action_process(work, prule->rules.action_type,ip, th, prule);
				return;     
			}       
			else
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"flow_action_process: eth forward error!\r\n");
				/*forward to the Linux*/
				fwd_virtual_port.ops.vp_pkt_to_control(work);
				cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 1);	
				cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_BYTES, CVM_WQE_GET_LEN(work)); 

				return ;
			}
		}   
		else if(prule->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD ||\
			    prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_FORWARD)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: capwap forward the packet with the type = %d\r\n",CVM_WQE_GET_UNUSED(work));    

			if (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_ETH_IP) /* Eth ==> CAPWAP 802.11 */
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"flow_action_process: Eth ==> CAPWAP 802.11.\n");

				encap_802_11_cw_packet(work, prule,work->word2.s.ip_offset);               
				if(prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_FORWARD)
				{	
					add_rpa_head(work,prule);
				}

				foward_action_process(work, prule->rules.action_type,ip, th, prule);
				return;  
			}
			else if (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_11) /* CAPWAP 802.11 ==> CAPWAP 802.11*/
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"flow_action_process: CAPWAP 802.11 ==> CAPWAP 802.11.\n");

				uint32_t offset = 0;

				offset =  IP_H_LEN + UDP_H_LEN + CW_H_LEN + 
					IEEE80211_H_LEN + LLC_H_LEN + work->word2.s.ip_offset;

				if (is_qos)
					offset += IEEE80211_QOS_DIFF_LEN;

				/*add by wangjian for support pppoe 2013-3-14 be careful with offset*/
				if (is_pppoe)
				{
					offset += PPPOE_H_LEN;
				}
				/*add by wangjian for support pppoe 2013-3-14 be careful with offset*/
				
				encap_802_11_cw_packet(work, prule, offset);
				if(prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_FORWARD)
				{	
					add_rpa_head(work,prule);
				}

				foward_action_process(work, prule->rules.action_type,ip, th, prule);
				return;  
			}
		}
		else if(prule->rules.action_type == FLOW_ACTION_CAP802_3_FORWARD ||\
			    prule->rules.action_type == FLOW_ACTION_RPA_CAP802_3_FORWARD)
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: 802.3 capwap forward the packet with the type = %d\r\n",CVM_WQE_GET_UNUSED(work));  

			if(CVM_WQE_GET_UNUSED(work) ==  PACKET_TYPE_ETH_IP)  /* Eth ==> CAPWAP 802.3 */
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"flow_action_process: eth 802.3 ==> CAPWAP 802.3.\n");

				encap_802_3_cw_packet(work, prule,work->word2.s.ip_offset);

				if(prule->rules.action_type == FLOW_ACTION_RPA_CAP802_3_FORWARD)
				{
					add_rpa_head(work,prule);
				}
				foward_action_process(work, prule->rules.action_type,ip, th, prule);
				return;  
			}
			else if(CVM_WQE_GET_UNUSED(work) ==  PACKET_TYPE_CAPWAP_802_3) /* CAPWAP 802.3 ==> CAPWAP 802.3*/
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"flow_action_process: capwap 802.3 ==> CAPWAP 802.3.\n");

				uint32_t offset = 0;

				offset =  work->word2.s.ip_offset + IP_H_LEN + UDP_H_LEN + CW_H_LEN + ETH_H_LEN;

				/*add by wangjian for support pppoe 2013-3-14 be careful with offset*/
				if (is_pppoe)
				{
					offset += PPPOE_H_LEN;
				}
				/*add by wangjian for support pppoe 2013-3-14 be careful with offset*/

				encap_802_3_cw_packet(work, prule, offset);
				if(prule->rules.action_type == FLOW_ACTION_RPA_CAP802_3_FORWARD)
				{
					add_rpa_head(work,prule);
				}

				foward_action_process(work, prule->rules.action_type,ip, th, prule);
				return;  
			}
		}       
		else if((prule->rules.action_type == FLOW_ACTION_ICMP) ||
				(prule->rules.action_type == FLOW_ACTION_CAPWAP_802_11_ICMP) ||
				(prule->rules.action_type == FLOW_ACTION_CAPWAP_802_3_ICMP)||
				(prule->rules.action_type == FLOW_ACTION_RPA_ICMP) ||
				(prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_11_ICMP) ||
				(prule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_802_3_ICMP))
		{
			flow_icmp_fast_path(prule, work, is_qos, is_pppoe);
			return;
		}
		else
		{
			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"flow_action_process: action_type ERROR action_type=%d!\r\n",action_type);
			/*forward to the Linux*/
			fwd_virtual_port.ops.vp_pkt_to_control(work);
			cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_PACKETS, 1);    
			cvmx_fau_atomic_add64(CVM_FAU_ENET_TO_LINUX_BYTES, CVM_WQE_GET_LEN(work)); 

			return ;
		}
	}

}



/**
 * Description:
 *  return fccp packet to linux
 *
 *  Parameter:
 * 
 * Return:
 *  0: Successfully
 *  -1: Failed
 */
int32_t return_fccp(cvmx_wqe_t* work, uint32_t ret_val, control_cmd_t* fccp_cmd, uint8_t grp)
{
	eth_hdr* eth = NULL;                          /* eth header */
	uint8_t tmp_mac[MAC_LEN] = {0};       /* swap mac addr */
	uint16_t tmp_module = 0;        /* swap src_module and dst_module */

	if((work == NULL) || (fccp_cmd == NULL) || (grp > 15))
	{
		printf("send_debug_fccp: invalid args\n");
		return RETURN_ERROR;
	}
	
	/* swap mac addr */  
	eth = (eth_hdr*)((uint8_t*)fccp_cmd - ETH_H_LEN);
	memcpy(tmp_mac, eth->h_source, MAC_LEN);
	memcpy(eth->h_source, eth->h_dest, MAC_LEN);
	memcpy(eth->h_dest, tmp_mac, MAC_LEN);

	/* swap module */
	tmp_module = fccp_cmd->src_module;
	fccp_cmd->src_module = fccp_cmd->dest_module;
	fccp_cmd->dest_module = tmp_module;
	fccp_cmd->ret_val = ret_val;

	/* make wqe */
	CVM_WQE_SET_UNUSED(work, FCCP_WQE_TYPE);

	/* Submit the packet to the POW */
	fwd_virtual_port.ops.vp_fccp_to_control(work);

	return RETURN_OK;
}





/**
 * Description:
 *  acl handle function. include self-learning rule, static insert rule, static del rule...
 *
 *  Parameter:
 *  fccp_cmd: fccp command word
 * 
 * Return:
 *  0: Successfully
 *  -1: Failed
 */
static inline int acl_cache_flow(cvmx_wqe_t* work, control_cmd_t * fccp_cmd)
{
	rule_param_t *rule_para = NULL;
	capwap_cache_t *cw_cache = NULL;
	five_tuple_t* five_tuple = NULL;
	//fccp_data_t fccp_data;

	if((work == NULL) || (fccp_cmd == NULL) || (fccp_cmd->cmd_len < sizeof(control_cmd_t)))
	{
		printf("acl_cache_flow: invalid args\n");
		return RETURN_ERROR;
	}

	/* self-learning or static-insert */
	if(fccp_cmd->cmd_opcode == FCCP_CMD_INSERT)
	{
		rule_para = (rule_param_t*)(&(fccp_cmd->fccp_data.rule_info.rule_param));
		cw_cache = (capwap_cache_t*)(&(fccp_cmd->fccp_data.rule_info.cw_cache));

		/* self learning */
		if(rule_para->rule_state == RULE_IS_LEARNED)
		{
			if(acl_self_learn_rule(rule_para, cw_cache) != RETURN_OK)
			{
				/* self-learn not send fccp back */
				cvmx_helper_free_packet_data(work); /*free data*/                
				cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
				return RETURN_ERROR;
			}
			cvmx_helper_free_packet_data(work); /*free data*/                
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		}
		/* static insert */
		else if(rule_para->rule_state == RULE_IS_STATIC)
		{
			if(acl_insert_static_rule(rule_para, cw_cache) != RETURN_OK)
			{
				printf("acl_cache_flow : acl_insert_static_rule return error\n");
				return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
				return RETURN_ERROR;
			}
			return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
		}
		else
		{
			printf("rule_state 0x%x is unkown!\n", rule_para->rule_state);
			cvmx_helper_free_packet_data(work); /*free data*/                
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
			return RETURN_ERROR;
		}
	}
	/* static-delete */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_DELETE)
	{        
		five_tuple = (five_tuple_t*)(&(fccp_cmd->fccp_data.rule_tuple));
		if(acl_delete_rule(five_tuple) != RETURN_OK)  
		{
			printf("acl_cache_flow : acl_delete_rule return error\n");
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}

	/* icmp learn. add by zhaohan */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_INSERT_ICMP)
	{
		rule_para = (rule_param_t*)(&(fccp_cmd->fccp_data.rule_info.rule_param));
		cw_cache = (capwap_cache_t*)(&(fccp_cmd->fccp_data.rule_info.cw_cache));

		/* self learning */
		if(rule_para->rule_state == RULE_IS_LEARNED)
		{
			if(acl_self_learn_icmp_rule(rule_para, cw_cache) != RETURN_OK)
			{
				/* self-learn not send fccp back */
				cvmx_helper_free_packet_data(work); /*free data*/                
				cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
				return RETURN_ERROR;
			}
			cvmx_helper_free_packet_data(work); /*free data*/                
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		}
		/* static insert */
		else if(rule_para->rule_state == RULE_IS_STATIC)
		{
			if(acl_insert_static_icmp_rule(rule_para, cw_cache) != RETURN_OK)
			{
				printf("acl_cache_flow : acl_insert_static_rule return error\n");
				return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
				return RETURN_ERROR;
			}
			return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
		}
		else
		{
			printf("rule_state 0x%x is unkown!\n", rule_para->rule_state);
			cvmx_helper_free_packet_data(work); /*free data*/                
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
			return RETURN_ERROR;
		}
	}

	/* set aging timer */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_SET_AGEING_TIMER)
	{
		if(acl_set_aging_timer(fccp_cmd->fccp_data.aging_timer) != RETURN_OK)
		{
			printf("acl_cache_flow : acl_set_aging_timer return error\n");
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	/* get aging timer */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_GET_AGEING_TIMER)
	{
		//memset(&fccp_data, 0, sizeof(fccp_data_t));
		fccp_cmd->fccp_data.aging_timer = acl_get_aging_timer();
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	/* icmp enable/disable */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_ENABLE_ICMP)
	{
		if((fccp_cmd->fccp_data.module_enable != FUNC_ENABLE) && 
				(fccp_cmd->fccp_data.module_enable != FUNC_DISABLE))
		{
			printf("acl_cache_flow : set icmp enable/disable error, module = %d\n", fccp_cmd->fccp_data.module_enable);
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		cvm_ip_icmp_enable = fccp_cmd->fccp_data.module_enable;
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	/* show icmp state */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_GET_ICMP_STATE)
	{
		//memset(&fccp_data, 0, sizeof(fccp_data_t));
		fccp_cmd->fccp_data.module_enable = cvm_ip_icmp_enable;
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	/* show rule statistic */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_GET_RULE_STATS)
	{
		//memset(&fccp_data, 0, sizeof(fccp_data_t));
		if(RETURN_ERROR == acl_get_rule_stats(&(fccp_cmd->fccp_data.rule_stats)))
		{
			printf("acl_cache_flow : show rule statistic error\n");
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	/* clear rule all */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_CLEAR_RULE)
	{   
	    disable_fastfwd();
		acl_clear_rule();
		enable_fastfwd();
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	/* clear aging rule all */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_CLEAR_AGING_RULE)
	{	
	    acl_clear_aging_rule();
		//return_fccp(work, FCCP_RETURN_OK, &fccp_data, product_info.to_linux_fccp_group);
		cvmx_helper_free_packet_data(work); /*free data*/                
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
	}
	/* set bucket max entry */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_SET_BUCKET_MAX_ENTRY)
	{
		if(acl_set_bucket_max_entries(fccp_cmd->fccp_data.bucket_max_entry) != RETURN_OK)
		{
			printf("acl_cache_flow : acl_set_bucket_max_entries return error\n");
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	/* get bucket max entry */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_GET_BUCKET_MAX_ENTRY)
	{
		//memset(&fccp_data, 0, sizeof(fccp_data_t));
		fccp_cmd->fccp_data.bucket_max_entry = acl_get_bucket_max_entries();
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_USER_ONLINE)
	{
		if(user_action_online(fccp_cmd->fccp_data.user_info.user_ip) != RETURN_OK)
		{
			//return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			cvmx_helper_free_packet_data(work); /*free data*/                
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
			return RETURN_ERROR;
		}
		//return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
		cvmx_helper_free_packet_data(work); /*free data*/                
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		return RETURN_OK;
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_USER_OFFLINE)
	{
		if(user_action_offline(fccp_cmd->fccp_data.user_info.user_ip) != RETURN_OK)
		{
			//return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			cvmx_helper_free_packet_data(work); /*free data*/                
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
			return RETURN_ERROR;
		}
		cvmx_helper_free_packet_data(work); /*free data*/                
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		//return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
		return RETURN_OK;
	}	
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_USER_STATS_CLEAR)
	{
            if(user_stats_clear(fccp_cmd->fccp_data.user_info.user_ip) != RETURN_OK)
            {
                return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
                return RETURN_ERROR;
            }
            return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_ENABLE_PURE_PAYLOAD_ACCT)
	{
    	if((fccp_cmd->fccp_data.module_enable != FUNC_ENABLE) && 
				(fccp_cmd->fccp_data.module_enable != FUNC_DISABLE))
		{
			printf("acl_cache_flow : set pure payload account enable/disable error, module = %d\n", \
				       fccp_cmd->fccp_data.module_enable);
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		cvm_pure_payload_acct = fccp_cmd->fccp_data.module_enable;
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_ENABLE_TRAFFIC_MONITOR)
	{
		
		if((fccp_cmd->fccp_data.module_enable != FUNC_ENABLE) && 
				(fccp_cmd->fccp_data.module_enable != FUNC_DISABLE))
		{
			printf("acl_cache_flow : set traffic monitor enable/disable error, module = %d\n", fccp_cmd->fccp_data.module_enable);
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		cvm_traffic_monitor_func = fccp_cmd->fccp_data.module_enable;
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_CLEAR_TRAFFIC_MONITOR)
	{
		clear_traffic_stats();
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_GET_TRAFFIC_MONITOR)
	{
		if(RETURN_OK != get_traffic_stats(&(fccp_cmd->fccp_data)))
		{
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_DEL_RULE_BY_IP)
    {/*
        printf("fast_del_rule_by_ip: user ip: %d.%d.%d.%d\n", 
			IP_FMT(fccp_cmd->fccp_data.user_info.user_ip));*/
			//wangjian
        fast_del_rule_by_ip(fccp_cmd->fccp_data.user_info.user_ip);
        cvmx_helper_free_packet_data(work); /*free data*/                
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
    }
    else if(fccp_cmd->cmd_opcode == FCCP_CMD_TEST)  //wangjian
	{
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
    else if(fccp_cmd->cmd_opcode == FCCP_CMD_EQUIPMENT_TEST)
	{
	    if(fccp_cmd->fccp_data.equipment_test.enable == FUNC_ENABLE)
	    {
	        switch(fccp_cmd->fccp_data.equipment_test.board_type)
	        {
	            case AUTELAN_BOARD_AX81_AC_4X:
	                fwd_equipment_test_fun = fwd_equipment_test_AC4X;
	                fwd_equipment_test_enable = FUNC_ENABLE;
                    printf("fastfwd go into AC4X board equipment test mode.\n");
	                break;
	            default:
	                break;
	        }
	    }
	    else if(fccp_cmd->fccp_data.equipment_test.enable == FUNC_DISABLE)
	    {
            fwd_equipment_test_enable = FUNC_DISABLE;
          	fwd_equipment_test_fun = NULL;  
          	printf("fastfwd go into normal mode.\n");
	    }
	    return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
		//cvmx_helper_free_packet_data(work); /*free data*/                
		//cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
	}	
	/* log to memory  enable/disable */
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_ENABLE_LOG)
	{
		if((fccp_cmd->fccp_data.module_enable != FUNC_ENABLE) && 
				(fccp_cmd->fccp_data.module_enable != FUNC_DISABLE))
		{
			printf("acl_cache_flow : set fwdlog enable/disable error, module = %d\n", fccp_cmd->fccp_data.module_enable);
			return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
			return RETURN_ERROR;
		}
		fwd_debug_log_enable = fccp_cmd->fccp_data.module_enable;
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_TIMESYNC)
	{
		if (fccp_cmd->fccp_data.time_sec)
		{
			printf("acl_cache_flow : set TIMESYNC enable \n");	
			linux_sec = fccp_cmd->fccp_data.time_sec;          
			fwd_sec = get_sec();                       /*when get linux_sec fwd_sec value*/
			fwd_debug_logtime_enable = FUNC_ENABLE;
		} 
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	} 
	else if (fccp_cmd->cmd_opcode == FCCP_CMD_ENABLE_LOG_SHOW)
	{
		fccp_cmd->fccp_data.module_enable = fwd_debug_log_enable;
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if (fccp_cmd->cmd_opcode == FCCP_CMD_SET_LOG_LEVEL)
	{
		fastfwd_common_debug_level = fccp_cmd->fccp_data.share;
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if (fccp_cmd->cmd_opcode == FCCP_CMD_SHOW_LOG_LEVEL)
	{
		fccp_cmd->fccp_data.share = fastfwd_common_debug_level;
		return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_CONFIG_TAG_TYPE)
	{
	    if(RETURN_ERROR == fastfwd_config_tag_type(fccp_cmd))
	        return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
        else
            return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_GET_TAG_TYPE)
	{
	    if(RETURN_ERROR == fastfwd_get_tag_type(fccp_cmd))
	        return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
        else
            return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_SHOW_FAU64)
	{
	    if(RETURN_ERROR == fastfwd_show_fau64(&fccp_cmd->fccp_data.fau64_info))
	        return_fccp(work, FCCP_RETURN_ERROR, fccp_cmd, product_info.to_linux_fccp_group);
        else
            return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_CLEAR_FAU64)
	{
	    uint64_t fau_addr = 0;
    	int32_t i = (CVM_FAU_PKO_ERRORS - CVMX_FAU_REG_64_START) >> 3;

    	for(;fau_addr < CVMX_FAU_REG_64_END; i++)
    	{
    		fau_addr = CVMX_FAU_REG_64_ADDR(i);
    		cvmx_fau_atomic_write64(fau_addr, 0);
    	}
	    return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else if(fccp_cmd->cmd_opcode == FCCP_CMD_SHOW_FPA_BUFF)
	{   
	    int i = 0;
	    for(i=0;i<8;i++)
    	{
    		fccp_cmd->fccp_data.pool_buff_count[i] = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(i));
    	}
    	return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);
	}
	else
	{
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"acl_cache_flow : fccp cmd opcode unknown!, opcode = %d\n", fccp_cmd->cmd_opcode);
		cvmx_helper_free_packet_data(work); /*free data*/                
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		return RETURN_ERROR;
	}

	return RETURN_OK;
}


//#ifdef PASSTHOUGH_L2_PERFORMANCE_TEST
static inline void swap_mac_addr(uint64_t pkt_ptr)
{
	uint16_t s;
	uint32_t w;

	/* assuming an IP/IPV6 pkt i.e. L2 header is 2 byte aligned, 4 byte non-aligned */
	s = *(uint16_t*)pkt_ptr;
	w = *(uint32_t*)(pkt_ptr+2);
	*(uint16_t*)pkt_ptr = *(uint16_t*)(pkt_ptr+6);
	*(uint32_t*)(pkt_ptr+2) = *(uint32_t*)(pkt_ptr+8);
	*(uint16_t*)(pkt_ptr+6) = s;
	*(uint32_t*)(pkt_ptr+8) = w;
}

CVMX_SHARED short passthough_port = -1;
static inline int send_packet_out(cvmx_wqe_t *work)
{
	uint64_t        port;
	cvmx_pko_command_word0_t pko_command;
	cvmx_buf_ptr_t  packet_ptr;
	int is_ip = !work->word2.s.not_IP; /*swap the MAC*/

	/* Build a PKO pointer to this packet */
	pko_command.u64 = 0;

	/*
	 * Begin packet output by requesting a tag switch to atomic.
	 * Writing to a packet output queue must be synchronized across cores.
	 *
	 */
	if((passthough_port >= 0) && (passthough_port < 32))
	{
		port = passthough_port;
	}
	else
	{
		port = CVM_WQE_GET_PORT(work);
	}
	int queue = cvmx_pko_get_base_queue(port) +1;
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
	}
	pko_command.s.dontfree = MEM_AUTO_FREE;


	if (is_ip)  /*swap the MAC*/
		swap_mac_addr((uint64_t)cvmx_phys_to_ptr((uint64_t)packet_ptr.s.addr));

	/*
	 * Send the packet and wait for the tag switch to complete before
	 * accessing the output queue. This ensures the locking required
	 * for the queue.
	 *
	 */
	if (cvmx_pko_send_packet_finish(port, queue, pko_command, packet_ptr, CVMX_PKO_LOCK_ATOMIC_TAG) != CVMX_PKO_SUCCESS)
	{
		cvmx_helper_free_packet_data(work); /*free data*/
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
		return RETURN_ERROR;
	}

	cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0);
	return RETURN_OK;	
}



/* send debug information */
int32_t send_debug_info(cvmx_wqe_t* work, control_cmd_t* fccp_cmd)
{
	if((work == NULL) || (fccp_cmd == NULL))
		return RETURN_ERROR;

	switch(fccp_cmd->cmd_opcode)
	{
		case FCCP_CMD_RULE_CNT:
			memcpy(&(fccp_cmd->fccp_data.rule_cnt_info), &rule_cnt_info, sizeof(rule_cnt_info_t));
			return return_fccp(work, FCCP_RETURN_OK, fccp_cmd, product_info.to_linux_fccp_group);

		default:
			printf("cmd opcode error\n");
			cvmx_helper_free_packet_data(work); /*free data*/                
			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
			return RETURN_ERROR;
	}

	return RETURN_OK;
}

static inline void fwd_process_fccp_pkt(cvmx_wqe_t *work, control_cmd_t *fccp_cmd)
{
    uint16_t proto;
    eth_hdr* eth_header = NULL;
    eth_header = (eth_hdr *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
    proto = eth_header->h_vlan_proto;

    cvmx_fau_atomic_add64(CVM_FAU_RECV_FCCP_PACKETS, 1);
    
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
            "FCCP dest module=%d\r\n",fccp_cmd->dest_module);

    switch(fccp_cmd->dest_module)
    {
        case FCCP_MODULE_ACL:
            acl_cache_flow(work, fccp_cmd);
            break;
        case FCCP_MODULE_DBG:
            send_debug_info(work, fccp_cmd);
            break;
        case FCCP_MODULE_NAT:
        case FCCP_MODULE_CAR:
        default:
            FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
                    "Receive fccp command with not support dest module=%d\r\n",fccp_cmd->dest_module);                  
            cvmx_helper_free_packet_data(work); /*free data*/                
            cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0); /*free wqe*/
            break;

    }

}



/**
 * Description:
 *  filter large pkts.(maybe over mtu) 
 *  eth=>capwap, eth will add about 76 bytes, maybe over 1500 bytes.
 *  so we filter it.
 *  so does rpa.
 *
 * Parameter:
 *  rule:
 *  work:
 *
 */
void fwd_filter_large_pkts(rule_item_t  *rule, cvmx_wqe_t* work)
{
    if((rule == NULL) || (work == NULL))
        return;

    /* if packet len > downlink mtu, send to linux */
    if(rule->rules.action_type == FLOW_ACTION_CAPWAP_FORWARD)
    {
        if (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_ETH_IP) /* Eth ==> CAPWAP 802.11 */
        { 
            FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
                    "flow_action_process: Eth ==> CAPWAP 802.11.\n");

            if(downlink_mtu) 
            {
                uint32_t mtu = downlink_mtu;
                uint32_t cmp_len = CVM_WQE_GET_LEN(work) - work->word2.s.ip_offset + ETH_H_LEN;
                
                if(rule->rules.out_tag)
                    mtu -= 4;
                if(rule->rules.in_tag)
                    mtu -= 4;

                if(cmp_len > mtu) 
                {
                    cvmx_fau_atomic_add64(CVM_FAU_LARGE_ETH2CW_PACKET, 1);
                    rule->rules.action_type = FLOW_ACTION_TOLINUX;
                }
            }
        }
    }
    if(rule->rules.action_type == FLOW_ACTION_RPA_ETH_FORWARD)
    {
        if(CVM_WQE_GET_LEN(work) > (int)(downlink_mtu - RPA_HEAD_LEN))
        {
            cvmx_fau_atomic_add64(CVM_FAU_LARGE_CW_RPA_FWD_PACKET, 1);
            rule->rules.action_type = FLOW_ACTION_TOLINUX;
        }
    }
    if(rule->rules.action_type == FLOW_ACTION_RPA_CAPWAP_FORWARD)
    {
        if(CVM_WQE_GET_LEN(work) > (int)(downlink_mtu - RPA_HEAD_LEN))
        {
            cvmx_fau_atomic_add64(CVM_FAU_LARGE_CW8023_RPA_FWD_PACKET, 1);
            rule->rules.action_type = FLOW_ACTION_TOLINUX;
        }
    }
}


/**
 * Process incoming packets.
 *
 */
static void application_main_loop(unsigned int coremask_data)
{
	cvmx_wqe_t *    work;
	cvmx_pko_command_word0_t pko_command;
	cvm_common_ip_hdr_t *ip = NULL;
	cvm_common_tcp_hdr_t *th = NULL;
	cvm_common_udp_hdr_t *uh = NULL;
	cvm_common_ip_hdr_t *in_ip = NULL;
	cvm_common_tcp_hdr_t *in_th = NULL;  
	cvm_common_ip_hdr_t *true_ip = NULL; /* capwap pkt: true_ip = in_ip, eth pkt: true_ip = ip */
	cvm_common_tcp_hdr_t *true_th = NULL;
	uint8_t *pkt_ptr = NULL;	
	uint32_t action_type =0;
	rule_item_t  *rule = NULL;
	uint16_t hlen;
	cvmx_spinlock_t   *first_lock =NULL;
	uint32_t loop_tag = 1;
	/* Build a PKO pointer to this packet */
	pko_command.u64 = 0;

	if(cvmx_coremask_first_core(coremask_data)) 
	{
		cvmx_spinlock_init(&gl_wait_lock);
		print_prompt();
	}

	while (1)
	{
		uint8_t is_qos = 0;
		uint8_t is_pppoe = 0;

		if(cvmx_coremask_first_core(coremask_data)) 
		{
			if((loop_tag != 1) && (set_tag_flag != 1))
			{
				//cvmx_pow_tag_sw_null();
			}
			shell_run();
		}

		/* get the next packet/work to process from the POW unit. */
		work = cvmx_pow_work_request_sync(CVMX_POW_WAIT);
		
		if (work == NULL) {
			loop_tag = 1;
			continue;
		}
		loop_tag = 0;
		

		/* Errata PKI-100 fix. We need to fix chain pointers on segmneted
		   packets. Although the size is also wrong on a single buffer packet,
		   PKO doesn't care so we ignore it */
		if (cvmx_unlikely(work->word2.s.bufs > 1))
			cvmx_helper_fix_ipd_packet_chain(work);

		cvmx_fau_atomic_add64(CVM_FAU_RECV_TOTAL_WORKS, 1);

#ifdef OCTEON_DEBUG_LEVEL
		debug_tag_val = CVM_WQE_GET_TAG(work);    /* for test */

		if((fastfwd_common_debug_level == FASTFWD_COMMON_DBG_LVL_INFO) &&((1 << cvmx_get_core_num() ) & (core_mask) ))
		{
			//cvmx_helper_dump_packet(work);
			fwd_debug_dump_packet(work);
		}
#endif

		/* Check for errored packets, and drop.  If sender does not respond
		 ** to backpressure or backpressure is not sent, packets may be truncated if
		 ** the GMX fifo overflows. We ignore the CVMX_PIP_OVER_ERR error so we
		 ** can support jumbo frames */
		if (cvmx_unlikely(work->word2.snoip.rcv_error) && (work->word2.snoip.err_code != CVMX_PIP_OVER_ERR))
		{

			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
					"Action: dropping the packet with NULL point error=%d, code=0x%x\r\n",work->word2.snoip.rcv_error,work->word2.snoip.err_code);

			cvmx_fau_atomic_add64(CVM_FAU_ENET_ERROR_PACKETS, 1);
			action_type = FLOW_ACTION_DROP; /* Work has error, so drop */
			goto scheme_execute;
		}

		/*Passthough test*/
		//#ifdef PASSTHOUGH_L2_PERFORMANCE_TEST
		if(cvm_passthough_enable == FUNC_ENABLE)
		{
			uint16_t vlan_tag;
			eth_hdr* eth_header = NULL;
			eth_header = (eth_hdr *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
			vlan_tag = eth_header->h_vlan_proto;
			if(vlan_tag == 0x8100)
			{
				*((uint16_t*)((uint8_t*)eth_header + ETH_H_LEN)) = passthough_vlan_id;
			}
			if(send_packet_out(work) != RETURN_OK)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"Passthough test: Failed to send the packets\r\n");
			}
			continue;
		}
		//#endif

		/* wqe software bit is set indicate that recv raw packet from pcie port. */
        /* define in cvm_drv_init(). */
		if(IS_PCI_PKT(work))
		{   
			int32_t ret = 0;
			ret = cvmcs_nic_process_wqe(work);
			if(ret == NIC_FCCP_PKT)
			{
			    control_cmd_t *fccp_cmd = (control_cmd_t *)((uint8_t*)cvmx_phys_to_ptr(work->packet_ptr.s.addr) + 24 + FCCP_ETH_HEADER_LENGTH);
			    fwd_process_fccp_pkt(work, fccp_cmd); 
			}
			continue;
		}

		/* fccp command handle first. */
		if(CVM_WQE_GET_UNUSED(work) == FCCP_WQE_TYPE)
		{
		    control_cmd_t *fccp_cmd = (control_cmd_t *)((uint8_t*)cvmx_phys_to_ptr(work->packet_ptr.s.addr) + FCCP_ETH_HEADER_LENGTH);
            fwd_process_fccp_pkt(work, fccp_cmd);
            continue;
		}

		if(fwd_equipment_test_enable == FUNC_ENABLE)
		{
            if(fwd_equipment_test_fun)
            {
                if(fwd_equipment_test_fun(work) == RETURN_OK)
                    continue;
            }    
		}

		cvmx_fau_atomic_add64(CVM_FAU_ENET_INPUT_PACKETS, 1);
		cvmx_fau_atomic_add64(CVM_FAU_ENET_INPUT_BYTES, CVM_WQE_GET_LEN(work));

        /* filter bcast & mcast */
        if((work->word2.s.is_bcast) || (work->word2.s.is_mcast))
        {
            /* need add counter */
            cvmx_fau_atomic_add64(CVM_FAU_MCAST_PACKETS, 1);
            action_type = FLOW_ACTION_TOLINUX;          
            goto scheme_execute; 
        }

		CVM_WQE_SET_UNUSED(work, PACKET_TYPE_UNKNOW);
		uint16_t tmp_proto;
		eth_hdr* tmp_eth_header = NULL;

		tmp_eth_header = (eth_hdr *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
		tmp_proto = tmp_eth_header->h_vlan_proto;

		CVM_WQE_SET_UNUSED(work,PACKET_TYPE_UNKNOW);

		/*rpa packet handle*/
		/*if unknown eth type ,software parse,else hardware parse*/
		if((tmp_proto == RPA_COOKIE) || 
		    (work->word2.s.vlan_valid && (((vlan_eth_hdr_t*)tmp_eth_header)->h_eth_type == RPA_COOKIE)))
		{
		    cvmx_fau_atomic_add64(CVM_FAU_RPA_PACKETS, 1);
			if(RETURN_ERROR == rpa_packet_handle(work,&ip,&action_type))
			{
			    cvmx_fau_atomic_add64(CVM_FAU_RPA_TOLINUX_PACKETS, 1);
				action_type = FLOW_ACTION_TOLINUX;			
				goto scheme_execute; 
			}
		}
		else if(tmp_proto == TIPC_COOKIE)
		{
		    CVM_WQE_SET_QOS(work, 0);
		    cvmx_fau_atomic_add64(CVM_FAU_TIPC_PACKETS, 1);
            action_type = FLOW_ACTION_TOLINUX;			
			goto scheme_execute; 
		}
		/* add by wangjian for support pppoe 2013-3-14 */
		else if (PPPOE_TYPE == tmp_proto)
		{
			if (PPPOE_IP_TYPE == *(uint16_t *)((uint8_t *)tmp_eth_header + ETH_H_LEN + 6))
			{
				ip = (cvm_common_ip_hdr_t *)((uint8_t *)tmp_eth_header + ETH_H_LEN + PPPOE_H_LEN);
				pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
				work->word2.s.ip_offset = ((uint8_t*)ip - pkt_ptr);
				/*be careful work->word2.s.ip_offset can change?*/
			}
			else 
			{
				cvmx_fau_atomic_add64(CVM_FAU_ENET_ETH_PPPOE_NONIP_PACKETS, 1);
				action_type = FLOW_ACTION_TOLINUX;			
				goto scheme_execute; 
			}
		}
		/* add by wangjian for support pppoe 2013-3-14 */
		else
		{
		
			/******************************************************************
			 * hardware parse result check. 
			 ******************************************************************/
			if (work->word2.s.not_IP)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"Forward the NonIP packets to Linux\r\n");

				cvmx_fau_atomic_add64(CVM_FAU_ENET_NONIP_PACKETS, 1);
				action_type = FLOW_ACTION_TOLINUX;			
				goto scheme_execute; 
			}

    		if(work->word2.s.is_frag)
    		{
    			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
    					"Receive the frag IP packets\r\n");

    			cvmx_fau_atomic_add64(CVM_FAU_ENET_FRAGIP_PACKETS, 1);
    			action_type = FLOW_ACTION_TOLINUX;			
    			goto scheme_execute;
    		}

    		/**************************************************
        		 * Parse the packets, get L2,L3, information, check error too. 
        		 * OCTEON PIP hardware support QINQ parse, but the type field is 0x8100
        		 * So if the type is not 0x8100, be careful, maybe we need to call the function 
        		 * "rx_l2hdr_decap" to do the job in the future. lutao tag
        		 ***************************************************/
    		pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
    		ip = (cvm_common_ip_hdr_t *)(pkt_ptr + work->word2.s.ip_offset); 

			FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
					"Get the packet IP header, start =0x%p, ip =0x%p, offset =%d\r\n",pkt_ptr,ip,work->word2.s.ip_offset);
		}
		
		if(cvmx_unlikely(CVM_WQE_GET_LEN(work) < (int)(sizeof( cvm_common_ip_hdr_t)))) {
			cvmx_fau_atomic_add64(CVM_FAU_IP_SHORT_PACKETS, 1);
			action_type = FLOW_ACTION_DROP;			
			goto scheme_execute; 
		}

		if(cvmx_unlikely(ip->ip_v != CVM_IP_IPVERSION)) {
			cvmx_fau_atomic_add64(CVM_FAU_IP_BAD_VERSION, 1);
			action_type = FLOW_ACTION_TOLINUX;			
			goto scheme_execute; 
		}
		
		if(cvmx_unlikely(ip->ip_hl != 5)) {     /* ip has option */
		    cvmx_fau_atomic_add64(CVM_FAU_IP_BAD_HDR_LEN, 1);
			action_type = FLOW_ACTION_TOLINUX;			
			goto scheme_execute; 
		}

		hlen = ip->ip_hl << 2;
		if (cvmx_unlikely(hlen < sizeof(cvm_common_ip_hdr_t))) { /* minimum header length */
			cvmx_fau_atomic_add64(CVM_FAU_IP_BAD_HDR_LEN, 1);
			action_type = FLOW_ACTION_DROP;			
			goto scheme_execute; 
		}

		if(cvmx_unlikely(ip->ip_len < hlen)) {
			cvmx_fau_atomic_add64(CVM_FAU_IP_BAD_LEN, 1);
			action_type = FLOW_ACTION_DROP;			
			goto scheme_execute; 
		}

		if (SPE_IP_ADDR(ip->ip_src, ip->ip_dst)) /* skip specail IP address */
		{
			cvmx_fau_atomic_add64(CVM_FAU_IP_SKIP_ADDR, 1);
			action_type = FLOW_ACTION_TOLINUX;			
			goto scheme_execute; 
		}

		/* 169.254.0.0 to linux with high qos */
		if(IP_IN_MANAGE(ip->ip_src) || IP_IN_MANAGE(ip->ip_dst))
		{
			cvmx_fau_atomic_add64(CVM_FAU_IP_SKIP_ADDR, 1);
			CVM_WQE_SET_QOS(work, 0);
			action_type = FLOW_ACTION_TOLINUX;			
			goto scheme_execute; 
		}

        /* set true_ip = ip first. when capwap pkt,change it */
        true_ip = ip;

		/**************************************************
		 * Parse the icmp packets. zhaohan add
		 ***************************************************/  
		if(ip->ip_p == CVM_COMMON_IPPROTO_ICMP)
		{
			CVM_WQE_SET_UNUSED(work, PACKET_TYPE_ICMP);
			cvmx_fau_atomic_add64(CVM_FAU_IP_ICMP, 1);
			if(cvm_ip_icmp_enable == FUNC_DISABLE)
			{
				action_type = FLOW_ACTION_TOLINUX;  
				goto scheme_execute; 
			}
		}

		/**************************************************
		 * Parse the packets l4 information and check too. lutao add
		 ***************************************************/        
		else if (ip->ip_p == CVM_COMMON_IPPROTO_TCP) 
		{
			th = (cvm_common_tcp_hdr_t *)((uint32_t *)ip + ip->ip_hl);
			true_th = th;
#if 1	/* modify for NAT requirement. zhaohan 2012-12-14 */		
			/* Defense syn flood */
            if (SPE_TCP_HDR(th)) 
            {
                FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
                "Forward the TCP syn,rst,fin flag packet to Linux\r\n");
                cvmx_fau_atomic_add64(CVM_FAU_SPE_TCP_HDR, 1);
                action_type = FLOW_ACTION_TOLINUX;			
                goto scheme_execute; 
            }
#endif			 
			CVM_WQE_SET_UNUSED(work, PACKET_TYPE_ETH_IP);
		}
		else if (ip->ip_p == CVM_COMMON_IPPROTO_UDP) 
		{
			uint32_t len;
			int32_t tmp;
			uh = ( cvm_common_udp_hdr_t*)((uint32_t *)ip + ip->ip_hl);
			th = ( cvm_common_tcp_hdr_t*)uh;
            true_th = th;
            
			/* destination port of 0 is illegal, based on RFC768. */
			if (uh->uh_dport == 0)
			{
				action_type = FLOW_ACTION_DROP;
				cvmx_fau_atomic_add64(CVM_FAU_UDP_BAD_DPORT, 1);
				goto scheme_execute; 
			}

			/*
			 * Make data length reflect UDP length.
			 * If not enough data to reflect UDP length, drop.
			 */
			len = cvm_common_ntohs((uint16_t)uh->uh_ulen);

			if (ip->ip_len != len) 
			{
				if (len > ip->ip_len || len < sizeof(cvm_common_udp_hdr_t)) 
				{
					FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
							"The UDP length is ERROR: udp len=%d, ip->ip_len=%d\r\n",len,ip->ip_len);		  

					cvmx_fau_atomic_add64(CVM_FAU_UDP_BAD_LEN, 1);
					action_type = FLOW_ACTION_DROP;			
					goto scheme_execute; 
				}
			}

			if ((tmp = rx_udp_decap(uh)) == PACKET_TYPE_UNKNOW)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
						"UDP is special CAPWAP frame, trap to linux.\n");
				cvmx_fau_atomic_add64(CVM_FAU_UDP_TO_LINUX, 1);
				action_type = FLOW_ACTION_TOLINUX;			
				goto scheme_execute; 
			}

			CVM_WQE_SET_UNUSED(work, tmp);		
		}
		else /*not support other proto type yet*/
		{
			cvmx_fau_atomic_add64(CVM_FAU_IP_PROTO_ERROR, 1);
			action_type = FLOW_ACTION_TOLINUX;			
			goto scheme_execute; 	
		}

		/***********************************************************************
		 * capwap pkts decap.
		 ***********************************************************************/
		if(CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_11)
		{
			if (cw_802_11_decap(uh, &in_ip, &in_th, &is_qos, &is_pppoe) != 0) 
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"Warning]: recv capwap packet, decap capwap failed\n");
				action_type = FLOW_ACTION_TOLINUX;			
				goto scheme_execute; 
			}
			true_ip = in_ip;
			true_th = in_th;		
		}
		else if(CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_3)
		{
			if (cw_802_3_decap(uh, &in_ip, &in_th,&is_pppoe) != 0) 
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"Warning]: recv capwap 802.3 packet, decap capwap failed\n");
				action_type = FLOW_ACTION_TOLINUX;			
				goto scheme_execute; 	
			}
			true_ip = in_ip;	
			true_th = in_th;
		}

		/***********************************************************************
		 * packet parse end, based on the result, handle the flow lookup, etc. lutao add
		 ***********************************************************************/
		if(true_ip->ip_p == CVM_COMMON_IPPROTO_ICMP)
		{
			rule = acl_table_icmp_lookup(true_ip);
			if(rule == NULL)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"Flow table lookup failed.\r\n");
				cvmx_fau_atomic_add64(CVM_FAU_FLOW_LOOKUP_ERROR, 1);
				action_type = FLOW_ACTION_TOLINUX;			
				goto scheme_execute; 					
			}
		}
		else
		{
			rule = acl_table_lookup(true_ip,true_th,&first_lock);
			if(rule == NULL)
			{
				FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_ERROR,
						"Flow table lookup failed.\r\n");
				cvmx_fau_atomic_add64(CVM_FAU_FLOW_LOOKUP_ERROR, 1);
				action_type = FLOW_ACTION_TOLINUX;			
				goto scheme_execute; 					
			}

		}

		/***********************************************************************
		 * scheme_execute begin. lutao add
		 ************************************************************************/
scheme_execute:
		FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN, FASTFWD_COMMON_DBG_LVL_DEBUG,
				"Action: work=0x%p,ip=0x%p,th=0x%p,rule=0x%p\r\n",work,ip,th,rule); 
	

		/* user packets statistics */
        if((CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_ETH_IP) ||
            (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_11) ||
            (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_CAPWAP_802_3) ||
            (CVM_WQE_GET_UNUSED(work) == PACKET_TYPE_ICMP))
        {
            user_flow_statistics_process(work,rule,true_ip);
        }

        /* filter large pkts.(maybe over mtu) */
		fwd_filter_large_pkts(rule, work);

		flow_action_process(work,action_type,ip,th,rule,is_qos,first_lock,is_pppoe);
		action_type = 0;
		ip = NULL;
		th = NULL;
		uh = NULL;
		rule = NULL;
		pkt_ptr = NULL; 
		first_lock = NULL;	
		true_ip = NULL;
		true_th = NULL;
		in_ip = NULL;
		in_th = NULL;
	}
}

int32_t get_dsa_port()
{
	switch(product_info.product_type)
	{
		case AX_5612:
			pip_port_info[1].s.port_type = PORT_TYPE_BACK;
			break;
		case AX_5612I:
		case AX_7605:
			pip_port_info[0].s.port_type = PORT_TYPE_BACK;
			break;
		case AU_4624:
		case AU_4524:
		case AU_4626_P:
		case AU_4524_P:
		case AU_3524:
		case AU_3524_P:
		case AU_3052:
		case AU_3052E:
		case AU_3052_P:
		case AU_3028:
		case AU_3028E:
		case AU_3028_P:
			pip_port_info[2].s.port_type = PORT_TYPE_BACK;
			break;
		default:
			printf("product_info.id = %d, unknow product type\n", product_info.product_type);
			return RETURN_ERROR;
	}

	return RETURN_OK;
}

/**
 * Init the board information
 *
 * @param void
 * lutao tag 需要后续根据单板CPLD信息修改
 * @return Zero on success, negative on failure
 */
int board_info_init(int se_mode)
{
	int pip_port;

	switch(cvmx_sysinfo_get()->board_type) 
	{		            
		default:
			for(pip_port = 0; pip_port < CVMX_PIP_NUM_INPUT_PORTS; pip_port++) 
			{
				pip_port_info[pip_port].u64 = 0;
				pip_port_info[pip_port].s.if_used	= PORT_USED;			
				pip_port_info[pip_port].s.port_type = PORT_TYPE_PANEL;		
			}
			get_dsa_port();
			product_info.se_mode = se_mode;
			product_info.board_type = 0;				
			break;
	}


	if(product_info.se_mode == SE_MODE_STANDALONE)
	{
		product_info.panel_interface_group = PANEL_PORT_GROUP;	
		product_info.back_interface_group = BACK_PORT_GROUP;		
		product_info.from_linux_group = FROM_LINUX_GROUP;		
		product_info.to_linux_group = TO_LINUX_GROUP; /*useless*/
		product_info.to_linux_fccp_group = TO_LINUX_FCCP_GROUP; /*useless*/
	}
	else if(product_info.se_mode == SE_MODE_COEXIST)
	{
		product_info.panel_interface_group = PANEL_PORT_GROUP;	
		product_info.back_interface_group = BACK_PORT_GROUP;		
		product_info.from_linux_group = FROM_LINUX_GROUP;
		product_info.to_linux_group = TO_LINUX_GROUP;
		product_info.to_linux_fccp_group = TO_LINUX_FCCP_GROUP;
		product_info.rule_mem_size = RULE_MEM_256M;     /* add by zhaohan */
	}
	else
		printf("Warning: SE run mode ERROR! \r\n");

	return RETURN_OK;
}

/**
 * soft memory initt
 *
 * @return Zero on success, negative on failure
 */
int soft_mem_init(void)
{
	/* init common here */
	if(acl_table_init() == RETURN_ERROR)
	{
		printf("acl_table_init init failed\n");
		return RETURN_ERROR;
	}
	CVMX_SYNCWS;

	if(cvm_common_global_init() != RETURN_OK)
	{
		printf("Common global init failed\n");
		return RETURN_ERROR;
	}	
	CVMX_SYNCWS;

	// add by yin
	//cvm_ip_reass_init();
	printf("cvm_ip_reass_init init end\n");
	CVMX_SYNCWS;

	if(car_init() == RETURN_ERROR)
	{
		printf("car_init init failed\n");
		return RETURN_ERROR;
	}

	return RETURN_OK;	
}

#define SE_BLOCK_NAME   "__se_mem"
int32_t detect_mem()
{
	int status = 0;   
	const cvmx_bootmem_named_block_desc_t *block_desc = cvmx_bootmem_find_named_block(SE_BLOCK_NAME);

	if (block_desc)    
	{    
		printf("find __se_mem: address = 0x%lx,  size = 0x%lx\n", block_desc->base_addr, block_desc->size);
    		if(block_desc->size >= 0x70000000)  // 1.75G		
    		{			
			product_info.rule_mem_size = RULE_MEM_1792M;		
    		}		
    		else if(block_desc->size >= 0x10000000)  // 256M	
    		{			
			product_info.rule_mem_size = RULE_MEM_256M;		
    		}

		status = cvmx_bootmem_free_named(SE_BLOCK_NAME);        
		if (status == 0)        
		{            
			printf("FAIL: cvmx_bootmem_free_named didn't free block %s\n", SE_BLOCK_NAME);            
			return RETURN_ERROR;        
		} 
		printf("__se_mem has been success freed!\n");
		return RETURN_OK;
	}
	else
	{
		printf("error: not find __se_mem!\n");
		return RETURN_ERROR;
	}
}

/**
* get se mode from args.
* e.g. ' bootoct 0x100000 coremask=0xf mode=standalone '
*
*/
int32_t fwd_get_se_mode(int argc, char *argv[])
{
    int32_t i;
    int32_t se_mode = SE_MODE_COEXIST;

    for(i = 2; i < argc; i++)
    {
        if((strlen(argv[i]) > 5) && (0 == strncmp(argv[i], "mode=", 5)))
        {
            if(*(argv[i] + 5) == 's')
            {
                se_mode = SE_MODE_STANDALONE;
            }
        }
    }

    return se_mode;
}

/**
 * Main entry point
 *
 * @return exit code
 */
int main(int argc, char *argv[])
{
	cvmx_sysinfo_t *sysinfo;
	unsigned int coremask_data;
	int result = 0;
	int port_index = 0; /*PIP port index*/
    int se_mode = SE_MODE_COEXIST;
    
	cvmx_user_app_init();
    
	/* compute coremask_passthrough on all cores for the first barrier sync below */
	sysinfo = cvmx_sysinfo_get();
	coremask_data = sysinfo->core_mask;

	if (cvmx_coremask_first_core(coremask_data)) 
	{
		/* write fau se coexist flag, notice linux */  
		/* get se mode: coexist or standalone */
        se_mode = fwd_get_se_mode(argc, argv);
		board_info_init(se_mode);
		clear_fau_64();
		cvmx_fau_atomic_write64(CVM_FAU_SE_COEXIST_FLAG, SE_MAGIC_NUM);
		cvmx_fau_atomic_write64(CVM_FAU_SE_CORE_MASK, coremask_data);

		shell_init(0);
		shell_common_register();
		display_banner();
		display_version_info();

#ifdef OCTEON_DEBUG_LEVEL
		fastfwd_common_set_debug_level(FASTFWD_COMMON_DBG_LVL_WARNING);
		module_print[FASTFWD_COMMON_MOUDLE_MAIN] = 1;
		module_print[FASTFWD_COMMON_MOUDLE_SHELL] = 1;
		module_print[FASTFWD_COMMON_MOUDLE_FLOWTABLE] = 1;
		core_mask = 0xffffffff;
#endif		
	}

	if (product_info.se_mode == SE_MODE_STANDALONE)
	{
		/*
		 * elect a core to perform hardware initializations, as only one core needs to
		 * perform this function.
		 */
		if (cvmx_coremask_first_core(coremask_data)) {
			if ((result = cvmx_init()) != RETURN_OK) {
				printf("Simple Executive initialization failed.\n");
				return result;
			}
		}
	}
	else
	{
		/* Have one core do the hardware initialization */
		if (cvmx_coremask_first_core(coremask_data))
		{
		    cvmx_oct_set_product_id(&product_info);
		    printf("product ID = %d\n", product_info.product_type);
			printf("Waiting for ethernet module to complete initialization...\n\n\n");
#if 0             
			cvmx_ipd_ctl_status_t ipd_reg;           
			do
			{
				ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);                 
			} while (ipd_reg.u64 == 0);              
#endif
#if 1
			while(cvmx_fau_fetch_and_add64(CVM_FAU_INIT_STATE_FLAG, 0) != FPA_INIT_WAIT);
			if(detect_mem() == RETURN_ERROR) /*if not find a huge block memory, rule only alloc 256M mem, add by zhaohan */
			{
				product_info.rule_mem_size = RULE_MEM_256M;
			}
			if(product_info.board_type == AUTELAN_BOARD_AX71_CRSMU_MODULE_NUM ) /*7605 only have 2G memory, fastfwd only hold max 256M (sushaohua). add by zhaohan*/
            {
        		product_info.rule_mem_size = RULE_MEM_128M;      
            }
			setup_free_mem();
			cvmx_fau_atomic_write64(CVM_FAU_INIT_STATE_FLAG, FPA_INIT_OK);
			CVMX_SYNCWS;
			printf("Fast-fwd SE setup fpa ok!\n");
			while(cvmx_fau_fetch_and_add64(CVM_FAU_INIT_STATE_FLAG, 0) != IPD_EN_OK);
			cvmx_helper_setup_red(PACKET_DROP_PIP_HIGH, PACKET_DROP_PIP_LOW);
			printf("Linux init ipd ok!\n");            
#endif
			/* We need to call cvmx_cmd_queue_initialize() to get the pointer to
			   the named block. The queues are already setup by the ethernet
			   driver, so we don't actually need to setup a queue. Pass some
			   invalid parameters to cause the queue setup to fail */
			cvmx_cmd_queue_initialize(0, 0, -1, 0);

            /* add for TIPC qos watch. TIPC pkt goto grp15,qos0. add by zhaohan */
            {
                cvmx_pip_qos_watchx_t qos_watch;
                qos_watch.u64 = 0;
                qos_watch.cn56xx.match_type = 4;  /* match Ether type */
                qos_watch.cn56xx.match_value = 0x88ca; /* TIPC mark */
                qos_watch.cn56xx.qos = 0; /* high priority */
                qos_watch.cn56xx.grp = product_info.to_linux_group; /* to linux directly */
                qos_watch.cn56xx.mask = 0x0000;
                cvmx_write_csr(CVMX_PIP_QOS_WATCHX(0), qos_watch.u64);
            }

            /* config qos vlan */
            {
                int i = 0;
                cvmx_pip_qos_vlanx_t qos_vlan;
                qos_vlan.u64 = 0;
                qos_vlan.s.qos = 2;
                
                for(i = 0; i < 8; i++)
                {
                    cvmx_write_csr(CVMX_PIP_QOS_VLANX(i), qos_vlan.u64);
                }
            }

            /* setup RED by qos */
            {
                cvmx_helper_setup_red_queue(0, PACKET_DROP_PIP_QOS0_HIGH, PACKET_DROP_PIP_QOS0_LOW);
                cvmx_helper_setup_red_queue(1, PACKET_DROP_PIP_QOS1_HIGH, PACKET_DROP_PIP_QOS1_LOW);
                cvmx_helper_setup_red_queue(2, PACKET_DROP_PIP_QOS2_HIGH, PACKET_DROP_PIP_QOS2_LOW);
            }
		}
	}

	/* Wait for global hardware init to complete */
	cvmx_coremask_barrier_sync(coremask_data);

	if(cvmx_coremask_first_core(sysinfo->core_mask)) 
	{
#ifdef CVMX_ENABLE_TIMER_FUNCTIONS 
		cvmx_tim_setup(CVM_COMMON_TICK_LEN_US, CVM_COMMON_WHEEL_LEN_TICKS);
		CVMX_SYNCWS;

		/* set get work timeout to 1024-2048 cycles */
		cvmx_write_csr(CVMX_POW_NW_TIM, 0x0);    

		cvmx_tim_start();	
#endif

		/* initialize hardware random unit */
		cvm_common_hw_rand_init();

		/* Change the group for only the port we're interested in */
		for (port_index=0; port_index < CVMX_PIP_NUM_INPUT_PORTS; port_index++)
		{
			if(pip_port_info[port_index].s.if_used	== PORT_USED)
			{
				cvmx_pip_port_tag_cfg_t tag_config;
				tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(port_index));

				if(pip_port_info[port_index].s.port_type	== PORT_TYPE_PANEL)
				{
					tag_config.s.grp = product_info.panel_interface_group;
				}
				else if(pip_port_info[port_index].s.port_type	== PORT_TYPE_BACK)
				{
					tag_config.s.grp = product_info.back_interface_group;
				}
				else 
				{
					tag_config.s.grp = product_info.to_linux_group;
				}
				
				tag_config.s.tcp6_tag_type = CVMX_POW_TAG_TYPE_ATOMIC; 
				tag_config.s.tcp4_tag_type = CVMX_POW_TAG_TYPE_ATOMIC;
				tag_config.s.ip6_tag_type = CVMX_POW_TAG_TYPE_ATOMIC;
				tag_config.s.ip4_tag_type = CVMX_POW_TAG_TYPE_ATOMIC;
				tag_config.s.non_tag_type = CVMX_POW_TAG_TYPE_ATOMIC;

				cvmx_write_csr(CVMX_PIP_PRT_TAGX(port_index), tag_config.u64);

				/* set packet qos = 1, add by zhoahan */
				cvmx_pip_port_cfg_t     port_cfg;
				port_cfg.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(port_index));
				port_cfg.s.qos = DEFAULT_PACKET_SSO_QUEUE;       /* Have all packets goto POW queue 1, queue 0 is for the messages between the cores*/
                port_cfg.s.grp_wat = 1; /* enable qos_watcher0 grp */
                port_cfg.s.qos_wat = 1; /* enable qos_watcher0 qos */
                port_cfg.s.qos_vlan =1;/* enable qos_vlan bit. qos set by vlan_qos */
				cvmx_write_csr(CVMX_PIP_PRT_CFGX(port_index), port_cfg.u64);

			}
		}	
	}

	cvmx_coremask_barrier_sync(coremask_data);


	cvm_common_set_cycle(0);    
	/* take out the POW from null null state */
	cvmx_pow_work_request_null_rd();	
	cvmx_coremask_barrier_sync(sysinfo->core_mask);

	/* Setup scratch registers used to prefetch output queue buffers for packet output */
	cvmx_pko_initialize_local();



	/*lutao tag*/
	/* Accept any packet except for the ones destined to the Linux group */
	if(cvmx_coremask_first_core(sysinfo->core_mask)) 
	{
	    /* first core only get fccp pkt! */
	     cvmx_pow_set_group_mask(cvmx_get_core_num(), (1<<product_info.from_linux_group));
    }
    else
    {
        cvmx_pow_set_group_mask(cvmx_get_core_num(),
                    (1<<product_info.panel_interface_group)|(1<<product_info.from_linux_group)|(1<<product_info.back_interface_group));
    }

	/* set qos level to all cores and pow union */
	if(cvmx_coremask_first_core(sysinfo->core_mask)) 
	{
		cvmx_pow_pp_grp_mskx_t grp_msk;            
		int cpu = 0;
		cvmx_ciu_fuse_t fuse;
		unsigned int fuse_tmp;
		fuse.u64 = cvmx_read_csr(CVMX_CIU_FUSE);
		fuse_tmp = fuse.s.fuse;

		/* set qos level to all cores */
		do{
			/*优先级0最高，queue 1是报文对应的QOS队列，优先级低于控制报文的QOS队列*/
			grp_msk.u64 = cvmx_read_csr(CVMX_POW_PP_GRP_MSKX(cpu));
			grp_msk.s.qos1_pri = 1;
			grp_msk.s.qos2_pri = 2; /* SE => linux, send debug info, ues qos 2,  add by zhaohan*/
			cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(cpu), grp_msk.u64);
			cpu = cpu+1;
			fuse_tmp = fuse_tmp >> 1;
		}while(fuse_tmp&0x1);

		/* set pow qos rnd */
		{
			/*配置SSO的QOS调度优先级，保证核间通信的控制消息优先调度
			  每一个rnd表示一次循环是否遍历该QOS队列的WORK，这里有八个位每个位表示一个QOS队列。
			  每个寄存器对应四个RND队列，一共有八个寄存器，所以对应了32个RND循环*/		
			int i;
			cvmx_pow_qos_rndx_t qosmask;
			qosmask.s.rnd = 0x1;
			qosmask.s.rnd_p1 = 0x3;
			qosmask.s.rnd_p2 = 0x3;
			qosmask.s.rnd_p3 = 0x7;

			for (i = 0; i < 8; i++) 
			{
				cvmx_write_csr(CVMX_POW_QOS_RNDX(i),qosmask.u64);
			}
		}  
	}

	cvmx_coremask_barrier_sync(coremask_data);

	/* setup pcie core driver, only by standalone cpu */
	if(product_info.se_mode == SE_MODE_STANDALONE)
	{
		if(cvmx_coremask_first_core(sysinfo->core_mask)) 
		{
			if(cvmcs_nic_global_init() != RETURN_OK)
			{
				printf("cvmcs_nic_global_init failed\n");
				return RETURN_ERROR;
			}
		}
		cvmx_coremask_barrier_sync(coremask_data);

		/* Initialization local to each core */
		cvmcs_nic_local_init();
		cvmx_pow_work_request_null_rd();
	}

    /* init log system logsave*/
	if(cvmx_coremask_first_core(sysinfo->core_mask)) 
	{
	    fwd_debug_init_global();
	}
    cvmx_coremask_barrier_sync(coremask_data);
    if(fwd_debug_init_local() != RETURN_OK)
    {
        printf("core:%d  fast-fwd log system init failed!\n", cvmx_get_core_num());
    }


	if(cvmx_coremask_first_core(sysinfo->core_mask)) 
	{
        cvm_rate_limit_init();
		/* init common here */		
		if((result = soft_mem_init())  != RETURN_OK)
		{
			printf("soft_mem_init failed\n");
			return RETURN_ERROR;
		}
		CVMX_SYNCWS;

		/* start nic core driver */
		if (product_info.se_mode == SE_MODE_STANDALONE)
		{
		    if(cvmx_sysinfo_get()->board_type == 17)   // 12C board
		    {
				cvmx_helper_ipd_and_packet_input_enable();
				cvmx_ipd_disable();
                cvmcs_nic_set_link_status();
		    }
			cvmcs_nic_start(); 
			fwd_register_virtual_port(VIRTUAL_PORT_PCIE);
		}
		else
		{
		    cvmx_helper_ipd_and_packet_input_enable();
            fwd_register_virtual_port(VIRTUAL_PORT_POW);
		}
		cvmx_ipd_enable();
		printf("Fast-fwd SE enable ipd!\n\n");

	}

	cvmx_coremask_barrier_sync(coremask_data);

	printf("Forward Core %d is ready now!\r\n",cvmx_get_core_num());
	application_main_loop(coremask_data);

	/* We may never reach here, except on the application core */
	if(cvmx_coremask_is_member(coremask_data))
	{
		printf("Warning: Data path core exited\n");
		return(-1);
	}

	/*ipfwd_shutdown();*/
	CVMX_BREAK;

	return (result);

}

