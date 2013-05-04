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


#ifndef   __CVMCS_NIC_API_H__
#define   __CVMCS_NIC_API_H__


/* nic driver extern */
#define NIC_PROCESS_PKT_OK      0
#define NIC_PROCESS_PKT_ERROR   1
#define NIC_FCCP_PKT            2

extern int cvmcs_nic_global_init();
extern inline void cvmcs_nic_local_init();
extern inline int cvmcs_nic_start();
extern inline int cvmcs_nic_process_wqe(cvmx_wqe_t  *wqe);
extern inline void cvmcs_nic_forward_packet_to_host(cvmx_wqe_t  *work);
extern int cvmcs_process_instruction(cvmx_wqe_t   *wqe);
extern void cvmcs_nic_set_link_status(void);

#endif



/* $Id: cvmcs-nic-defs.h 67243 2011-11-19 19:25:57Z mchalla $ */
