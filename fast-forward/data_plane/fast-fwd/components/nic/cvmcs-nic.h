/*
 *
 * OCTEON SDK
 *
 * Copyright (c) 2011 Cavium Networks. All rights reserved.
 *
 * This file, which is part of the OCTEON SDK which also includes the
 * OCTEON SDK Package from Cavium Networks, contains proprietary and
 * confidential information of Cavium Networks and in some cases its
 * suppliers. 
 *
 * Any licensed reproduction, distribution, modification, or other use of
 * this file or the confidential information or patented inventions
 * embodied in this file is subject to your license agreement with Cavium
 * Networks. Unless you and Cavium Networks have agreed otherwise in
 * writing, the applicable license terms "OCTEON SDK License Type 5" can be found 
 * under the directory: $OCTEON_ROOT/components/driver/licenses/
 *
 * All other use and disclosure is prohibited.
 *
 * Contact Cavium Networks at info@caviumnetworks.com for more information.
 *
 */


#ifndef __CVMCS_NIC_H__
#define __CVMCS_NIC_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-wqe.h"

#include "cvm-driver-defs.h"
#include "cvm-drv.h"
#include "cvm-drv-debug.h"

#include "octeon-nic-common.h"
#include "octeon-opcodes.h"





/* Enable this define to let the application print information at regular
   intervals */
#define CVMCS_DUTY_CYCLE


/* Interval in seconds after which the duty cycle prints information. */
#define DISPLAY_INTERVAL         30

#define LINK_CHECK_INTERVAL      1


/* Maximum number of cores the application supports. */
#define MAX_CORES                32

#define INIT_PACKET_COUNT        (1 * 1024)

/* Size of each FPA pool */
/* The packet and WQE pool is used by hardware and PCI core driver. */
#define FPA_PACKET_POOL_COUNT    (10 * INIT_PACKET_COUNT)
#define FPA_WQE_POOL_COUNT       (10 * INIT_PACKET_COUNT)

/* PKO queue command buffers are allocated by simple exec from this pool. */
#define FPA_OQ_POOL_COUNT        INIT_PACKET_COUNT


/** SSO Pool count - used in CN68xx */
#define FPA_SSO_POOL_COUNT       1024


/* If OCTEON_DEBUG_LEVEL is defined all 
   application debug messages are enabled. */
//#ifdef OCTEON_DEBUG_LEVEL
//#define DBG                      printf
//#else
#ifndef DBG
#define DBG(format, args...)     do{ }while(0)
#endif
//#endif

#define  MAX_OCTEON_ETH_PORTS    32

/* Enable this flag if you want the NIC app to send packets across multiple
   PCI output queues. This is required if you enable multi-core Rx processing
   or NAPI in the host driver. */
#define  USE_MULTIPLE_OQ





/** Stats for each NIC port in a single direction. */
struct nic_stats_t {

	int64_t        total_rcvd;

	int64_t        total_fwd;

	int64_t        err_pko;

	int64_t        err_link;

	int64_t        err_drop;
};




typedef struct {

	struct nic_stats_t  fromwire;
	struct nic_stats_t  fromhost;

} oct_link_stats_t;

#define OCT_LINK_STATS_SIZE   (sizeof(oct_link_stats_t))





typedef struct {

	uint64_t    present:1;
	uint64_t    active:1;
	uint64_t    rx_on:1;
	uint64_t    rsvd:45;
	uint64_t    ifflags:16;

} octnic_if_state_t;




typedef struct {

	octnic_if_state_t   state;

	oct_link_info_t     linfo;

	oct_link_stats_t    stats;

} octnic_port_info_t;




typedef struct {

	uint64_t            board_type:16;
	uint64_t            nports:16;
	uint64_t            numpciqs:8;
	uint64_t            rsvd:24;
	uint64_t            macaddrbase;
 
	octnic_port_info_t  port[MAX_OCTEON_ETH_PORTS];


}  octnic_dev_t;



extern CVMX_SHARED octnic_dev_t  *octnic;



#define   INTERFACE(port) (cvmx_helper_get_interface_num(port))
/* Ports 0-15 are interface 0, 16-31 are interface 1 */
#define   INDEX(port)     (cvmx_helper_get_interface_index_num(port))


static inline uint64_t
cvmcs_nic_mac_to_64bit(uint8_t  *mac)
{
	uint64_t  macaddr=0;
	int i;
	for(i = 0; i < 6; i++)
		macaddr = (macaddr << 8) | mac[i];
	return macaddr;
}


uint64_t
cvmcs_nic_get_port_default_speed(int port);



int
cvmcs_nic_find_idx(int port);



int
cvmcs_nic_send_link_info(cvmx_wqe_t  *wqe);

void
cvmcs_nic_check_link_status(void);

int
cvmcs_nic_change_multicast_list(int port, octnet_ifflags_t  flags);


int 
cvmcs_nic_change_mac_address(uint8_t *addr, int port);


int
cvmcs_nic_process_cmd(cvmx_wqe_t    *wqe);


int
cvmcs_nic_setup_interfaces(void);

void
cvmcs_nic_print_link_status(oct_link_status_t  *st, int ifidx, int port);


#endif


/* $Id: cvmcs-nic.h 62415 2011-07-15 23:17:33Z panicker $ */
