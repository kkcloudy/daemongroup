/***********************************************************************

  OCTEON TOOLKITS                                                         
  Copyright (c) 2007 Cavium Networks. All rights reserved.

  This file, which is part of the OCTEON TOOLKIT from Cavium Networks,
  contains proprietary and confidential information of Cavium Networks
  and in some cases its suppliers.

  Any licensed reproduction, distribution, modification, or other use of
  this file or confidential information embodied in this file is subject
  to your license agreement with Cavium Networks. The applicable license
  terms can be found by contacting Cavium Networks or the appropriate
  representative within your company.

  All other use and disclosure is prohibited.

  Contact Cavium Networks at info@caviumnetworks.com for more information.

 ************************************************************************/ 
 
#ifndef __COMMON_CONFIG_H__
#define __COMMON_CONFIG_H__



/* Content below this point is only used by the cvmx-config tool, and is
 * not used by any C files as CAVIUM_COMPONENT_REQUIREMENT is never defined.
 */

#ifdef CAVIUM_COMPONENT_REQUIREMENT
 
        /* global resource requirement */
 
        cvmxconfig
        {
                fau CVM_FAU_REG_FPA_OOB_COUNT
                            size = 4
                            count = 8
    		        description = "FPA out of buffer counters";

                fau CVM_FAU_REG_POOL_0_USE_COUNT
                        size = 4
                        description = "pool 0 use count";

                fau CVM_FAU_REG_POOL_1_USE_COUNT
                        size = 4
                        description = "pool 1 use count";

                fau CVM_FAU_REG_POOL_2_USE_COUNT
                        size = 4
                        description = "pool 2 use count";

                fau CVM_FAU_REG_POOL_3_USE_COUNT
                        size = 4
                        description = "pool 3 use count";

                fau CVM_FAU_REG_POOL_4_USE_COUNT
                        size = 4
                        description = "pool 4 use count";

                fau CVM_FAU_REG_POOL_5_USE_COUNT
                        size = 4
                        description = "pool 5 use count";

                fau CVM_FAU_REG_POOL_6_USE_COUNT
                        size = 4
                        description = "pool 6 use count";

                fau CVM_FAU_REG_POOL_7_USE_COUNT
                        size = 4
                        description = "pool 7 use count";
                        
                fau CVMX_FAU_REG_OQ_ADDR_INDEX
                                   size        = 8
                                   count       = 128
                                   description = "FAU registers for the position in PKO command buffers";

	fau CVM_FAU_SE_COEXIST_FLAG
		size = 8
		description = "se coexist flag";	
                fau CVM_FAU_INIT_STATE_FLAG
                        size = 8
                        description = "init state flag";	
                        
	fau CVM_FAU_SE_CORE_MASK
		size = 8
		description = "se core mask";
                fau CVM_FAU_PKO_ERRORS
                        size = 8
                        description = "64-bit counter used for total ethernet output error packets";
				
                fau CVM_FAU_ENET_INPUT_BYTES
                        size = 8
                        description = "64-bit counter used for total ethernet input bytes";

                fau CVM_FAU_ENET_INPUT_PACKETS
                        size = 8
                        description = "64-bit counter used for total ethernet input packets";

                fau CVM_FAU_ENET_OUTPUT_BYTES
                        size = 8
                        description = "64-bit counter used for total ethernet output bytes";

                fau CVM_FAU_ENET_OUTPUT_PACKETS
                        size = 8
                        description = "64-bit counter used for total ethernet output packets";
				
                fau CVM_FAU_ENET_DROP_PACKETS
                        size = 8
                        description = "64-bit counter used for total ethernet drop packets";

                fau CVM_FAU_ENET_TO_LINUX_PACKETS
                        size = 8
                        description = "64-bit counter used for total ethernet to linux packets";

                fau CVM_FAU_ENET_NONIP_PACKETS
                        size = 8
                        description = "64-bit counter used for total ethernet input noneip packets";

                fau CVM_FAU_ENET_ERROR_PACKETS
                        size = 8
                        description = "64-bit counter used for total ethernet input noneip packets";

                fau CVM_FAU_ENET_FRAGIP_PACKETS
                        size = 8
                        description = "64-bit counter used for total ethernet input noneip packets";

                fau CVM_FAU_IP_SHORT_PACKETS
                        size = 8
                        description = "64-bit counter used for short IP packets rcvd";
                
                fau CVM_FAU_IP_BAD_HDR_LEN
                        size = 8
                        description = "64-bit counter used for IP packets with bad hdr len";
                
                fau CVM_FAU_IP_BAD_LEN
                        size = 8
                        description = "64-bit counter used for IP packets with bad len";

                fau CVM_FAU_IP_BAD_VERSION
                        size = 8
                        description = "64-bit counter used for IP packets with bad version";

                fau CVM_FAU_IP_SKIP_ADDR
                        size = 8
                        description = "64-bit counter used for IP packets with SKIP addr";

                fau CVM_FAU_IP_ICMP
                        size = 8
                        description = "64-bit counter used for ICMP packets";

                fau CVM_FAU_CAPWAP_ICMP
                        size = 8
                        description = "64-bit counter used for Capwap ICMP packets";
				
                fau CVM_FAU_IP_PROTO_ERROR
                        size = 8
                        description = "64-bit counter used for ip packets with proto error";

                fau CVM_FAU_UDP_BAD_DPORT
                        size = 8
                        description = "64-bit counter used for udp dport=0 packets";

                fau CVM_FAU_UDP_BAD_LEN
                        size = 8
                        description = "64-bit counter used for udp packets with len error";

                fau CVM_FAU_UDP_TO_LINUX
                        size = 8
                        description = "64-bit counter used for udp packets that trap to Linux";

                fau CVM_FAU_FLOWTABLE_HIT_PACKETS
                        size = 8
                        description = "64-bit counter used for ACL HIT packets number";

                fau CVM_FAU_ENET_OUTPUT_PACKETS_8021Q
                        size = 8
                        description = "64-bit counter used for total ethernet output 802.1qpackets";

                fau CVM_FAU_ENET_OUTPUT_PACKETS_QINQ
                        size = 8
                        description = "64-bit counter used for total ethernet output qinq packets";

                fau CVM_FAU_FLOW_LOOKUP_ERROR
                        size = 8
                        description = "64-bit counter used for total flow lookup failed counter";	

	fau CVM_FAU_RECV_FCCP_PACKETS
		size = 8
		description = "64-bit counter used for total received fccp packets counter";	

	fau CVM_FAU_RECV_TOTAL_WORKS
		size = 8
		description = "64-bit counter used for total received works counter, include input packets and fccp packets";   

	fau CVM_FAU_TOTAL_ACL_LOOKUP
		size = 8
		description = "64-bit counter used for total acl lookup counter";   

	fau CVM_FAU_ACL_REG
		size = 8
		description = "64-bit counter used for total acl setup and regist packets counter";       

	fau CVM_FAU_CW802_11_DECAP_ERROR
		size = 8
		description = "64-bit counter used for total capwap 802.11 decap error";   

	fau CVM_FAU_CW802_3_DECAP_ERROR
		size = 8
		description = "64-bit counter used for total capwap 802.3 decap error";   

    fau CVM_FAU_ENET_TO_LINUX_BYTES
            size = 8
            description = "64-bit counter used for total ethernet to linux bytes";

    fau CVM_FAU_ALLOC_RULE_FAIL
            size = 8
            description = "64-bit counter used for total alloc rule fail counter";

    fau CVM_FAU_MAX_RULE_ENTRIES
            size = 8
            description = "64-bit counter used for reach max rule entries";
	fau CVM_FAU_CW_80211_ERR
            size = 8
            description = "64-bit counter used for 802.11 error packet";
	fau CVM_FAU_CW_NOIP_PACKETS
            size = 8
            description = "64-bit counter used for capwap no ip packet";
	fau CVM_FAU_CW_SPE_PACKETS
            size = 8
            description = "64-bit counter used for capwap special packet";
	fau CVM_FAU_CW_FRAG_PACKETS
            size = 8
            description = "64-bit counter used for capwap frag packet";
	fau CVM_FAU_MCAST_PACKETS
            size = 8
            description = "64-bit counter used for mcast packet";
	fau CVM_FAU_RPA_PACKETS
            size = 8
            description = "64-bit counter used for rpa packet";
	fau CVM_FAU_RPA_TOLINUX_PACKETS
            size = 8
            description = "64-bit counter used for rpa to linux packet";
	fau CVM_FAU_TIPC_PACKETS
            size = 8
            description = "64-bit counter used for tipc packet";
	fau CVM_FAU_LARGE_ETH2CW_PACKET
            size = 8
            description = "64-bit counter used for large eth->capwap packet";
	fau CVM_FAU_LARGE_CW_RPA_FWD_PACKET
            size = 8
            description = "64-bit counter used for large xxx->capwap_rpa packet";
	fau CVM_FAU_LARGE_CW8023_RPA_FWD_PACKET
            size = 8
            description = "64-bit counter used for large xxx->capwap8023_rpa packet";
	fau CVM_FAU_SPE_TCP_HDR
            size = 8
            description = "64-bit counter used for special tcp header packet(syn/fin/reset)";
	fau CVM_FAU_CW_SPE_TCP_HDR
            size = 8
            description = "64-bit counter used for special tcp header packet over capwap(syn/fin/reset)";

	fau CVM_FAU_ENET_ETH_PPPOE_NONIP_PACKETS
    		size = 8
    		description = "64-bit counter used for total ethernet input eth pppoe noneip packets";
	fau CVM_FAU_ENET_CAPWAP_PPPOE_NONIP_PACKETS
    		size = 8
    		description = "64-bit counter used for total ethernet input capwap pppoe noneip packets";
	fau CVM_FAU_ENET_OUTPUT_PACKETS_ETH_PPPOE
    		size = 8
    		description = "64-bit counter used for total ethernet output eth pppoe packets";
	fau CVM_FAU_ENET_OUTPUT_PACKETS_CAPWAP_PPPOE
    		size = 8
    		description = "64-bit counter used for total ethernet output capwap pppoe packets";
  


	scratch CVMX_SCR_OQ_BUF_PRE_ALLOC
		size        = 8
		iobdma      = true
		permanent   = true
		description = "Pre allocation for PKO queue command buffers";

	scratch CVMX_SCR_SCRATCH
		size        = 8
		iobdma      = true
		permanent   = true
		description = "Generic scratch iobdma area";

	scratch CVMX_SCR_SCRATCH_RESERVED0
		size        = 8
		iobdma      = true
		permanent   = true
		description = "Generic scratch iobdma area";

	scratch CVMX_SCR_SCRATCH_RESERVED1
		size        = 8
		iobdma      = true
		permanent   = true
		description = "Generic scratch iobdma area";                   


	scratch CVM_SCR_ACL_CACHE_PTR
		size = 8
		iobdma = true
		permanent = true
		description = "Pointer to acl entry";

	scratch CVM_SCR_USER_CACHE_PTR
		size = 8
		iobdma = true
		permanent = true
		description = "Pointer to user_table entry";

	scratch CVM_SCR_MBUFF_INFO_PTR
		size = 8
		iobdma = true
		permanent = true
		description = "MBuff information";

	scratch CVM_DRV_SCR_PACKET_BUF_PTR
		size = 8
		iobdma = true
		permanent = true
		description = "shit!";

	scratch CVM_DRV_SCR_WQE_BUF_PTR
		size = 8
		iobdma = true
		permanent = true
		description = "shit!";
}
#endif
#endif  /* __COMMON_CONFIG_H__ */

