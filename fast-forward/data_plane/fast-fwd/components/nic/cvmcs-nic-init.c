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



#include "cvmcs-common.h"
#include "cvmcs-nic.h"
#include "cvmx-helper.h"
#include "cvm-pci-loadstore.h"



extern CVMX_SHARED uint32_t            cvm_first_buf_size_after_skip;
extern CVMX_SHARED uint32_t            cvm_subs_buf_size_after_skip;

extern CVMX_SHARED cvmx_sysinfo_t     *appinfo;
extern CVMX_SHARED octnic_dev_t       *octnic;






static void
cvmcs_nic_init_pkt_skip_sizes(void)
{
	cvm_first_buf_size_after_skip = (((cvmx_read_csr(CVMX_IPD_PACKET_MBUFF_SIZE) & 0xfff) - ((cvmx_read_csr(CVMX_IPD_1ST_MBUFF_SKIP) & 0x3f) + 1) ) * 8);
	cvm_subs_buf_size_after_skip = ((cvmx_read_csr(CVMX_IPD_PACKET_MBUFF_SIZE) & 0xfff) - ((cvmx_read_csr(CVMX_IPD_NOT_1ST_MBUFF_SKIP) & 0x3f) + 1)) * 8;

	printf("First MBUF size: %u\n Subsequent MBUF size: %u\n", cvm_first_buf_size_after_skip, cvm_subs_buf_size_after_skip);
}





/* PCIe Port #1 support added for 68XX NIC4E_68 NIC card */
static void
cn68xx_nic_pci_load_store()
{
        int                            i, pcie_port = 1;
        cvmx_sli_mem_access_subidx_t   mem_access;
 
        mem_access.u64    = 0;
        mem_access.s.port = pcie_port; /* Port the request is sent to. */
        mem_access.s.esr  = 1;     /* Endian-swap for Reads. */
        mem_access.s.esw  = 1;     /* Endian-swap for Writes. */
 
 
        for (i=12; i<16 ; i++)
        {
                cvmx_write_csr(CVMX_PEXP_SLI_MEM_ACCESS_SUBIDX(i), mem_access.u64);
                mem_access.cn68xx.ba += 1; /* Set each SUBID to extend the addressable range */
        }
}









/* NIC includes all ports from firstport to lastport (inclusive) */
static void
__add_port_to_nic(octnic_dev_t  *nic, int firstport, int lastport)
{
	int i, port = firstport;

	while(port <= lastport) {
		i = nic->nports;
		nic->port[i].state.present = 1;
		nic->port[i].state.active  = 1;
		nic->port[i].state.rx_on   = 0;
		nic->port[i].linfo.ifidx   = i;
		nic->port[i].linfo.gmxport = port;
		nic->port[i].linfo.txpciq  = (nic->numpciqs > 1)?(i % nic->numpciqs):0;
		nic->port[i].linfo.rxpciq  = (nic->numpciqs > 1)?(i % nic->numpciqs):0;
		printf("Packets from port %d will be sent to PCI on txq: %d rxq: %d\n",
			       nic->port[i].linfo.gmxport,
			       nic->port[i].linfo.txpciq,
			       nic->port[i].linfo.rxpciq);
		nic->nports++;
		port++;
	}
}











static void
__auto_find_ports(octnic_dev_t  *nic)
{
	int i, j;

	printf("\n Initiating auto search of network interfaces\n");

	for(i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		cvmx_helper_interface_mode_t  mode;
		mode = cvmx_helper_interface_get_mode(i);
		if(!((mode == CVMX_HELPER_INTERFACE_MODE_SGMII) ||
		    (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI) ||
		    (mode == CVMX_HELPER_INTERFACE_MODE_XAUI) )) {
			printf("\n Interface %d: interface type 0x%x not supported\n", i,mode);
			continue;
		}
		
		for (j = 0; j < cvmx_helper_ports_on_interface(i); j++) {
			int port;
			port = cvmx_helper_get_ipd_port(i,j);
			printf("Adding Interface %d index %d (mode 0x%x) port: %d\n",
				i, j, mode, port);
			__add_port_to_nic(nic, port, port);
		}
	}
}








static int
cvmcs_nic_init_board_info()
{
	octnic = cvmx_bootmem_alloc(sizeof(octnic_dev_t), CVMX_CACHE_LINE_SIZE);
	if(octnic == NULL) {
		printf("%s Allocation failed for octnic\n", __FUNCTION__);
		return 1;
	}

	memset(octnic, 0, sizeof(octnic_dev_t));

#ifdef USE_MULTIPLE_OQ
	octnic->numpciqs = 4; // Multiple Queues are required for NAPI support in host.
#else
	octnic->numpciqs = 1;
#endif

    appinfo = cvmx_sysinfo_get();
	if(appinfo) {
		char   boardstring[32];
		switch(appinfo->board_type) {

			case CVMX_BOARD_TYPE_EBT3000:
				strcpy(boardstring, "EBT3000");
				__add_port_to_nic(octnic, 16, 19);
				break;
			case CVMX_BOARD_TYPE_EBT5800:
				strcpy(boardstring, "EBT5800");
				__add_port_to_nic(octnic, 16, 19);
				break;
			case CVMX_BOARD_TYPE_THUNDER:
				strcpy(boardstring, "THUNDER");
				__add_port_to_nic(octnic, 16, 19);
				break;
			case CVMX_BOARD_TYPE_NICPRO2:
				strcpy(boardstring, "NICPRO2");
				__add_port_to_nic(octnic, 16, 19);
				break;
			case CVMX_BOARD_TYPE_NIC_XLE_4G:
				strcpy(boardstring, "XLE_4G");
				__add_port_to_nic(octnic, 16, 19);
				break;
			case CVMX_BOARD_TYPE_NIC_XLE_10G:
				strcpy(boardstring, "XLE_10G");
				__add_port_to_nic(octnic, 0, 0);
				__add_port_to_nic(octnic, 16, 16);
 				break;
			case CVMX_BOARD_TYPE_NIC2E:
				if(OCTEON_IS_MODEL(OCTEON_CN63XX)) {
					strcpy(boardstring, "CN63XX-NIC2E");
					__add_port_to_nic(octnic, 0, 1);
				}

				if(OCTEON_IS_MODEL(OCTEON_CN66XX)) {
					strcpy(boardstring, "CN66XX-NIC2E");
					__add_port_to_nic(octnic, 0, 1);
				}	
				break;
			case CVMX_BOARD_TYPE_NIC4E:
				if(OCTEON_IS_MODEL(OCTEON_CN63XX)) {
					strcpy(boardstring, "CN63XX-NIC4E");
					__add_port_to_nic(octnic, 0, 3);
				}
				
				if(OCTEON_IS_MODEL(OCTEON_CN66XX)) {
					strcpy(boardstring, "CN66XX-NIC4E");
					__add_port_to_nic(octnic, 0, 3);
				}
				break;
			/*case CVMX_BOARD_TYPE_NIC10E_66:
				strcpy(boardstring, "NIC10E_66");
				__add_port_to_nic(octnic, 0, 0);
				__add_port_to_nic(octnic, 16, 16);
 				break;
			*/
			case CVMX_BOARD_TYPE_NIC68_4:
				strcpy(boardstring, "NIC68_4");
				__add_port_to_nic(octnic, 2112, 2112);
				__add_port_to_nic(octnic, 2368, 2368);
				__add_port_to_nic(octnic, 2880, 2880);
				__add_port_to_nic(octnic, 3136, 3136);

				cn68xx_nic_pci_load_store();
 				break; 
 			/* 12C board. add by zhaohan */
 #if 0			
 			case CVMX_BOARD_TYPE_EBH5600:
 			    strcpy(boardstring, "12C-EBH5600");
 			    __add_port_to_nic(octnic, 0, 3);
				__add_port_to_nic(octnic, 16, 16);
				break;
#endif				
			default:
				__auto_find_ports(octnic);
				sprintf(boardstring, "Unknown (board_type: %d)", appinfo->board_type);
				break;
		}
		printf("\n boardtype: %s \n", boardstring);
	}



	return 0;
}









static void
cvmcs_nic_init_mtu(void)
{
	int   i, port;

	if(octnic->nports == 0) {
		printf("%s: No active ports found\n", __FUNCTION__);
		return;
	}

	port = octnic->port[0].linfo.gmxport;


	if (!OCTEON_IS_MODEL(OCTEON_CN3XXX) && !OCTEON_IS_MODEL(OCTEON_CN58XX)) {
		cvmx_pip_frm_len_chkx_t     frm_len_chk;
		cvmx_pip_prt_cfgx_t         pip_prt;
		int                         lastport, pknd;


		frm_len_chk.u64     = 0;
		frm_len_chk.s.minlen = 64;
		frm_len_chk.s.maxlen = OCTNET_MAX_FRM_SIZE;


		if (octeon_has_feature(OCTEON_FEATURE_PKND))  {
			pknd = cvmx_helper_get_pknd(INTERFACE(port), INDEX(port));
			pip_prt.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(pknd));
			cvmx_write_csr(CVMX_PIP_FRM_LEN_CHKX(pip_prt.cn68xx.len_chk_sel), frm_len_chk.u64);
		} else {
			cvmx_write_csr(CVMX_PIP_FRM_LEN_CHKX(INTERFACE(port)), frm_len_chk.u64);
		}


		lastport = port;
		i = 1; 
		while (octnic->port[i].state.present) {
			port = octnic->port[i].linfo.gmxport;

			if (octeon_has_feature(OCTEON_FEATURE_PKND))  {
				pknd = cvmx_helper_get_pknd(INTERFACE(port), INDEX(port));
				pip_prt.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(pknd));
				cvmx_write_csr(CVMX_PIP_FRM_LEN_CHKX(pip_prt.cn68xx.len_chk_sel),
				               frm_len_chk.u64);
			} else {
				if(INTERFACE(port) != INTERFACE(lastport)) {
					cvmx_write_csr(CVMX_PIP_FRM_LEN_CHKX(INTERFACE(port)),
 			               frm_len_chk.u64);
				}
			}
			lastport = port;
			i++;
		}
	}

	i = 0;
	while(octnic->port[i].state.present) {
		port = octnic->port[i].linfo.gmxport;
		cvmx_write_csr(CVMX_GMXX_RXX_JABBER(INDEX(port), INTERFACE(port)),
		               ((OCTNET_DEFAULT_FRM_SIZE + 7) & ~7));
		i++;
	}

}







/*
static void
cvmcs_nic_init_macaddr(void)
{
	int        i, port;
	uint8_t   *oct_mac_addr_base;

	oct_mac_addr_base = cvmcs_app_get_macaddr_base();

	octnic->macaddrbase = 0;
	for(i = 0; i < 6; i++, octnic->macaddrbase <<= 8)
		octnic->macaddrbase |= oct_mac_addr_base[i];

	octnic->macaddrbase >>=8;

	i = 0;
	while(octnic->port[i].state.present) {
		uint64_t  hw_addr;

		port = octnic->port[i].linfo.gmxport;
		hw_addr = ((octnic->macaddrbase & 0x0000ffffffffffffULL) + i);
		octnic->port[i].linfo.hw_addr = hw_addr;
		cvmcs_nic_change_mac_address((uint8_t *)&hw_addr, port);
		octnic->port[i].state.ifflags = OCTNET_IFFLAG_MULTICAST;
		cvmcs_nic_change_multicast_list(port, octnic->port[i].state.ifflags);
		i++;
	}

}

*/






int 
cvmcs_nic_setup_interfaces(void)
{
	/* Get the board type and determine the number of ports, the first usable
	   port etc. */
	if(cvmcs_nic_init_board_info()) {
		printf("%s Board Init failed\n", __FUNCTION__);
		return 1;
	}

	/* Initialize the MTU value for all ports to their defaults. */
	cvmcs_nic_init_mtu();

	/* Setup the MAC address for each port. */
	//cvmcs_nic_init_macaddr();

	/* Get the skip sizes for first and subsequent wqe pkt buffers. */
	cvmcs_nic_init_pkt_skip_sizes();

	CVMX_SYNCW;

	return 0;
}





