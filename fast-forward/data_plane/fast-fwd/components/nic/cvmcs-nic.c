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




#include <stdio.h>
#include <string.h>

#include "cvmcs-nic.h"
#include "cvmx-helper.h"
#include "cvm-pci-loadstore.h"





extern     uint32_t    core_id;

CVMX_SHARED uint32_t            cvm_first_buf_size_after_skip;
CVMX_SHARED uint32_t            cvm_subs_buf_size_after_skip;


void
cvmcs_nic_print_link_status(oct_link_status_t  *st, int ifidx, int port)
{
	printf("ifidx %d Port %d: %d Mbps %s duplex %s\n", ifidx, port, st->s.speed,
	        (st->s.duplex)?"Full":"Half", (st->s.status)?"UP":"DOWN");
}

int
cvmcs_nic_find_idx(int port)
{
	int i;

	for(i = 0; i < octnic->nports; i++) {
		if(octnic->port[i].linfo.gmxport == port)
			return i;
	}

	return -1;
}


uint64_t
cvmcs_nic_get_port_default_speed(int port)
{
    uint64_t speed = 0;
    int interface = cvmx_helper_get_interface_num(port);
    int index = cvmx_helper_get_interface_index_num(port);

   if (index >= cvmx_helper_ports_on_interface(interface))
       return speed;

   switch (cvmx_helper_interface_get_mode(interface))
   {
       case CVMX_HELPER_INTERFACE_MODE_DISABLED:
       case CVMX_HELPER_INTERFACE_MODE_PCIE:
           /* Network links are not supported */
           break;
       case CVMX_HELPER_INTERFACE_MODE_XAUI:
       case CVMX_HELPER_INTERFACE_MODE_RXAUI:
       case CVMX_HELPER_INTERFACE_MODE_SPI:
           speed = 10000;
           break;
       case CVMX_HELPER_INTERFACE_MODE_RGMII:    
       case CVMX_HELPER_INTERFACE_MODE_GMII:
       case CVMX_HELPER_INTERFACE_MODE_SGMII:
       case CVMX_HELPER_INTERFACE_MODE_PICMG:
       case CVMX_HELPER_INTERFACE_MODE_SRIO:
       case CVMX_HELPER_INTERFACE_MODE_ILK:
           speed = 1000;
           break;
       case CVMX_HELPER_INTERFACE_MODE_NPI:
       case CVMX_HELPER_INTERFACE_MODE_LOOP:
           /* Network links are not supported */
           break;
       default:
            break;
   }    

   return speed;
}




uint64_t
cvmcs_nic_update_link_status(int port)
{
    oct_link_status_t         st;

    if(cvmx_sysinfo_get()->board_type == 17)   // 12C board
    {
        //int interface = cvmx_helper_get_interface_num(port);
        //int index = cvmx_helper_get_interface_index_num(port);

        st.u64 = 0;
    	st.s.duplex = 1;
    	st.s.status = 1;
        st.s.speed = cvmcs_nic_get_port_default_speed(port);        
    }
    else
    {
        cvmx_helper_link_info_t   link = cvmx_helper_link_autoconf(port);
        st.s.duplex = link.s.full_duplex;
        st.s.status = link.s.link_up;
        st.s.speed  = link.s.speed;
    }

	return st.u64;
}







int
cvmcs_nic_change_multicast_list(int port, octnet_ifflags_t  flags)
{
	//cvmx_gmxx_prtx_cfg_t gmx_cfg;
	//int                  interface = INTERFACE(port);
	//int                  index = INDEX(port);
	cvmx_gmxx_rxx_adr_ctl_t control;

	control.u64    = 0;
	control.s.bcst = 1;     /* Allow broadcast MAC addresses */

	/* Force accept multicast packets. This is required to support IPv6. */
	control.s.mcst = 2;

	if (flags & OCTNET_IFFLAG_PROMISC)
		control.s.cam_mode = 0; /* Reject matches if promisc. Since CAM is shut off, should accept everything */
	else
		control.s.cam_mode = 1; /* Filter packets based on the CAM */

    /*delete by zhaohan*/
#if 0
	gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(index, interface));
	cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64 & ~1ull);

	cvmx_write_csr(CVMX_GMXX_RXX_ADR_CTL(index, interface), control.u64);

	if (flags & OCTNET_IFFLAG_PROMISC)
		cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM_EN(index, interface), 0);
	else
		cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM_EN(index, interface), 1);

	cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64);
#endif
	return 0;
}





int 
cvmcs_nic_change_mac_address(uint8_t *addr, int port)
{
	uint8_t             *ptr = (uint8_t *)addr + 2;
	uint64_t             mac = 0;
	cvmx_gmxx_prtx_cfg_t gmx_cfg;
	int                  i;
	int                  interface = INTERFACE(port);
	int                  index = INDEX(port);


	for (i=0; i < 6; i++) {
		mac = (mac<<8) | (uint64_t)(ptr[i]);
		printf(" %02x ", ptr[i]);
	}
	printf("\n");

	gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(index, interface));

	cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64 & ~1ull);
	cvmx_write_csr(CVMX_GMXX_SMACX(index, interface), mac);
	cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM0(index, interface), ptr[0]);
	cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM1(index, interface), ptr[1]);
	cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM2(index, interface), ptr[2]);
	cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM3(index, interface), ptr[3]);
	cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM4(index, interface), ptr[4]);
	cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM5(index, interface), ptr[5]);
	cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64);

	return 0;
}








int
cvmcs_nic_change_mtu(int port, int new_mtu)
{
	int max_frm_size = new_mtu + 18;
	int interface = INTERFACE(port);
	int index = INDEX(port);

	/* Limit the MTU to make sure the ethernet packets are between 64 bytes
	   and 65535 bytes */
	if ( (max_frm_size < OCTNET_MIN_FRM_SIZE)
		 || (max_frm_size > OCTNET_MAX_FRM_SIZE))  {
		printf("MTU must be between %d and %d.\n", OCTNET_MIN_FRM_SIZE - 18,
		       OCTNET_MAX_FRM_SIZE - 18);
		return 1;
	}


	if (OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN58XX)) {
		/* Signal errors on packets larger than the MTU */
		cvmx_write_csr(CVMX_GMXX_RXX_FRM_MAX(index, interface), max_frm_size);
	}

	/* Set the hardware to truncate packets larger than the MTU. The
	    jabber register must be set to a multiple of 8 bytes, so round up */
	cvmx_write_csr(CVMX_GMXX_RXX_JABBER(index, interface), (max_frm_size + 7) & ~7u);
	printf("MTU for port %d changed to %d\n\n", port, new_mtu);

	return 0;
}












static int
cvmcs_nic_prepare_link_info_pkt(uint64_t  *buf)
{
	int                i, size = 0;
	oct_link_info_t   *linfo;


	if(octnic->nports == 0)
		return 0;

	/* 64-bit link info field tells host driver the number of links
	   that the core has information for. */
	buf[0] = octnic->nports;

	linfo = (oct_link_info_t *)&buf[1];

	i = 0;
	while(octnic->port[i].state.present) {
		int port = octnic->port[i].linfo.gmxport;

		linfo->link.u64 = cvmcs_nic_update_link_status(port);
		linfo->ifidx    = octnic->port[i].linfo.ifidx;
		linfo->gmxport  = octnic->port[i].linfo.gmxport;
		linfo->hw_addr  = octnic->port[i].linfo.hw_addr;
		linfo->txpciq   = octnic->port[i].linfo.txpciq;
		linfo->rxpciq   = octnic->port[i].linfo.rxpciq;

		DBG("linfo @ %p ifidx: %d gmxport: %d rxq: %d txq: %d hw_addr: %lx\n",
			linfo, linfo->ifidx, linfo->gmxport, linfo->rxpciq, linfo->txpciq, linfo->hw_addr); 
			
        DBG("speed %d Mbps %s duplex %s\n", linfo->link.s.speed,
                    (linfo->link.s.duplex)?"Full":"Half", (linfo->link.s.status)?"UP":"DOWN");

		linfo++;
		i++;
	}

	size = (OCT_LINK_INFO_SIZE * octnic->nports) + 8;

	CVMX_SYNCWS;
	return size;
}








int
cvmcs_nic_send_link_info(cvmx_wqe_t  *wqe)
{
	cvm_pci_dma_cmd_t      cmd;
	cvmx_buf_ptr_t         lptr;
	cvm_dma_remote_ptr_t   rptr;
	cvmx_raw_inst_front_t *f;
	uint64_t              *buf;

	cmd.u64  = 0;
	lptr.u64 = 0;


	f = (cvmx_raw_inst_front_t *)wqe->packet_data;

	cmd.s.pcielport = f->irh.s.pcie_port;
	rptr.s.addr = f->rptr;
	rptr.s.size = f->irh.s.rlenssz;


	if(cvmx_unlikely(rptr.s.size > CVMX_FPA_PACKET_POOL_SIZE)) {
		printf("[ DRV ] Cannot use packet pool buf for sending link info\n");
		return 1;
	}

	lptr.s.size = rptr.s.size;

	/* Re-use the packet pool buffer to send the link info to host. */
	buf = (uint64_t *)cvmx_phys_to_ptr(wqe->packet_ptr.s.addr);

	/* Reset all bytes so that unused fields don't have any value. */
	memset(buf, 0, rptr.s.size);
	
	lptr.s.addr = CVM_DRV_GET_PHYS(buf);

	/* First 8 bytes is response header. No information in it. 
	   The link data starts from byte offset 8. */
	if(cvmcs_nic_prepare_link_info_pkt(&buf[1]) == 0) {
		printf("[ DRV ] prepare link info pkt failed\n");
		return 1;
	}

	lptr.s.i    = 1;
	lptr.s.pool = CVMX_FPA_PACKET_POOL;
    lptr.s.back = CVM_COMMON_CALC_BACK_POWER_2(wqe->packet_ptr);

	cmd.s.nl = cmd.s.nr = 1;

	cvmx_fpa_free(wqe, CVMX_FPA_WQE_POOL, 0);

	return cvm_pci_dma_send_data(&cmd, &lptr, &rptr);
}



void
cvmcs_nic_check_link_status(void)
{
	int                 i;

	i = 0;
	while(octnic->port[i].state.present) {
		oct_link_status_t   link;
		int                 port = octnic->port[i].linfo.gmxport;

		link.u64 = cvmcs_nic_update_link_status(port);

		if(link.u64 != octnic->port[i].linfo.link.u64) {
			/* If link status has changed, update status info first. */
			octnic->port[i].linfo.link.u64 = link.u64;
			CVMX_SYNCW;
			/* Print the updated status. */
			cvmcs_nic_print_link_status(&link, i, port);
		}
		i++;
	}
}



int cvmcs_nic_process_cmd(cvmx_wqe_t    *wqe)
{
	cvmx_raw_inst_front_t  *f = (cvmx_raw_inst_front_t *)wqe->packet_data;
	octnet_cmd_t           *ncmd;
	int                     port, ifidx;
	uint64_t                retaddr = 0, ret = -1ULL;

	ncmd = (octnet_cmd_t  *) ((uint8_t *)f + CVM_RAW_FRONT_SIZE);

	DBG("NW Command packet received (0x%016lx)\n", ncmd->u64);

	ifidx = ncmd->s.param1;

	if(ifidx >= MAX_OCTEON_ETH_PORTS) {
		printf("%s: Invalid ifidx (%d) received in command\n", __FUNCTION__,
		       ifidx);
		return 1;
	}


	if( (octnic->port[ifidx].state.present == 0) 
	     ||  (octnic->port[ifidx].state.active == 0) ) {
		printf("%s: GMX Port at ifidx (%d) appears inactive\n", __FUNCTION__,
		       ifidx);
		return 1;
	}

	port = octnic->port[ncmd->s.param1].linfo.gmxport;

	switch(ncmd->s.cmd) {

		case OCTNET_CMD_RX_CTL:
			printf("Command for RX Control: (ifidx %d Command: %s)\n",
			        ifidx, (ncmd->s.param2?"Start":"Stop"));
			octnic->port[ifidx].state.rx_on = ncmd->s.param2;
			CVMX_SYNCWS;
			break;

		case OCTNET_CMD_CHANGE_MTU:
			printf("Command to change MTU (ifidx %d port: %d new_mtu %d)\n",
			        ifidx, port, ncmd->s.param2);
			ret = cvmcs_nic_change_mtu(port, ncmd->s.param2);
			if(f->rptr)
				retaddr = f->rptr + 8;
			break;

		case OCTNET_CMD_CHANGE_MACADDR:
			{
				uint64_t   *macaddr;

				macaddr = (uint64_t *)((uint8_t *)ncmd + sizeof(octnet_cmd_t));
				printf("Command to change MAC Addr (ifidx %d port %d MAC 0x%lx)\n",
				        ifidx, port, *macaddr);
				ret = cvmcs_nic_change_mac_address((uint8_t *)macaddr, port);
				if(f->rptr)
					retaddr = f->rptr + 8;
				break;
			}

		case OCTNET_CMD_CHANGE_DEVFLAGS:
			{
				octnet_ifflags_t    flags;

				flags = (octnet_ifflags_t)
				        *((uint64_t *)((uint8_t *)ncmd + sizeof(octnet_cmd_t)));
				printf("Command to change Flags (ifidx %d port %d Flags 0x%lx)\n",
				        ifidx, port, (unsigned long)flags);
				ret = 0;
				if(octnic->port[ifidx].state.ifflags != flags) {
					octnic->port[ifidx].state.ifflags = flags;
					ret = cvmcs_nic_change_multicast_list(port, flags);
				}
				if(f->rptr)
					retaddr = f->rptr + 8;
				break;
			}

		default:
			printf("Unknown NIC Command 0x%08x\n", ncmd->s.cmd);
			retaddr = 0;
			break;

	}

	if(retaddr)
		cvm_pci_mem_writell(retaddr, ret);

	cvm_free_host_instr(wqe);

	return 0;
}




/* $Id: cvmcs-nic.c 67685 2011-12-03 04:55:29Z panicker $ */
