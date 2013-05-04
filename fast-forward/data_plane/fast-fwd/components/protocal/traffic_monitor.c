
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
#include "fastfwd-common-misc.h"
#include "fastfwd-common-defs.h"
#include "fastfwd-common-rnd.h"
#include "fastfwd-common-misc.h"
#include "fastfwd-common-fpa.h"
#else
#include "cvm-common-misc.h"
#include "cvm-common-defs.h"
#include "cvm-common-rnd.h"
#include "cvm-common-misc.h"
#include "cvm-common-fpa.h"
#endif

#include "autelan_product_info.h"
#include "shell.h"
#include "acl.h"




#include "fwd_main.h"
#include "cvm-ip-in.h"
#include "capwap.h"
#include "fccp.h"
#include "traffic_monitor.h"




extern CVMX_SHARED traffic_stats_t traffic_stats;

void clear_traffic_stats()
{
	memset((char*)&traffic_stats,0,sizeof(traffic_stats));
	return;
}
int get_traffic_stats(fccp_data_t* fccp_data)
{
	if(NULL == fccp_data )
	{
		return RETURN_ERROR; 
	}
	memset(fccp_data, 0, sizeof(fccp_data_t));
	memcpy((char*)(&(fccp_data->traffic_stats)),(char*)(&traffic_stats),sizeof(traffic_stats)); 
	return RETURN_OK;
}


static inline void capwap_stats(cvm_common_udp_hdr_t *ex_uh)
{
	if(NULL == ex_uh )
	{
		return;
	}
	struct ieee80211_frame *ieee80211_hdr = NULL;
	struct ieee80211_llc *llc_hdr = NULL;
	cvm_common_ip_hdr_t *in_ip = NULL;
	uint16_t len = 0;

	ieee80211_hdr = (struct ieee80211_frame*)((uint8_t*)ex_uh + UDP_H_LEN + CW_H_LEN);

	if ((ieee80211_hdr->i_fc[0] & IEEE80211_FC0_VERSION_MASK) != IEEE80211_FC0_VERSION_0)
	{
		return;
	}
	if ((ieee80211_hdr->i_fc[0] & IEEE80211_FC0_TYPE_MASK) != IEEE80211_FC0_TYPE_DATA)
	{
		return;
	}

	if (ieee80211_hdr->i_fc[0] & IEEE80211_FC0_QOS_MASK) 
	{ 
		len = IEEE80211_QOS_H_LEN;
	} 
	else
	{ 
		len = IEEE80211_H_LEN;
	}

	llc_hdr = (struct ieee80211_llc*)((uint8_t*)ieee80211_hdr + len);
	if ( llc_hdr->llc_ether_type[1] == 0x06 && llc_hdr->llc_ether_type[0] == 0x08)
	{
		cvmx_atomic_add32(&(traffic_stats.cw_arp_pkt), 1);
		return;
	}
	if ( 0x0 ==llc_hdr->llc_ether_type[1]  && 0x08 == llc_hdr->llc_ether_type[0])
	{
	
		in_ip = (cvm_common_ip_hdr_t*)((uint8_t*)llc_hdr + LLC_H_LEN);
		if((4 != in_ip->ip_v ) || (5 != in_ip->ip_hl ))
		{
		    cvmx_atomic_add32(&(traffic_stats.cw_ip_exc_pkt), 1);
			return;
		}
		if(0 != (in_ip->ip_off & 0x3fff) )
		{
	        cvmx_atomic_add32(&(traffic_stats.cw_ip_frag_pkt), 1);
	        return;
		}
		if(CVM_COMMON_IPPROTO_TCP == in_ip->ip_p)
		{
			cvmx_atomic_add32(&(traffic_stats.cw_tcp_pkt), 1);
			return;
		}
		if(CVM_COMMON_IPPROTO_UDP == in_ip->ip_p)
		{
			cvmx_atomic_add32(&(traffic_stats.cw_udp_pkt), 1);
			return;
		}
		if(CVM_COMMON_IPPROTO_ICMP == in_ip->ip_p)
		{
			cvmx_atomic_add32(&(traffic_stats.cw_icmp_pkt), 1);
			return;
		}
	}
	return;
}

static inline void udp_stats(cvm_common_udp_hdr_t* uh)
{
	if(NULL == uh )
	{
		return;
	}
	cvmx_atomic_add32(&(traffic_stats.total_udp_pkt), 1);
    if((DHCP_CLIENT_PORT == uh->uh_sport ) || (DHCP_CLIENT_PORT == uh->uh_dport))
    {
        cvmx_atomic_add32(&(traffic_stats.dhcp_pkt), 1);
    }
    else if((DHCP_SERVER_PORT == uh->uh_sport ) || (DHCP_SERVER_PORT == uh->uh_dport))
    {
        cvmx_atomic_add32(&(traffic_stats.dhcp_pkt), 1);
    }
    else if((RIP_PORT == uh->uh_sport) || (RIP_PORT == uh->uh_dport))
    {
        cvmx_atomic_add32(&(traffic_stats.rip_pkt), 1);
    }
    /* CAPWAP data packet */
	else if ((CW_DAT_AP_PORT == uh->uh_sport) && 
			((CW_DAT_STD_PORT == uh->uh_dport) || 
			 (CW_DAT_TMP_PORT == uh->uh_dport )))
	{
	    if(get_capwap_type((union capwap_hd *)((char *)uh + UDP_H_LEN)) == PACKET_TYPE_CAPWAP_802_11)
		{    
		    cvmx_atomic_add32(&(traffic_stats.cw_dat_pkt), 1);
		    capwap_stats(uh);
        }
		else
		{
		    cvmx_atomic_add32(&(traffic_stats.cw_8023_dat_pkt), 1);
		}    
	}
	else if ((CW_CTL_AP_PORT == uh->uh_sport ) && 
			((CW_CTL_STD_PORT == uh->uh_dport ) ||
			 (CW_CTL_TMP_PORT == uh->uh_dport )))
	{
		cvmx_atomic_add32(&(traffic_stats.cw_ctl_pkt), 1);
	}
	/* Inter-AC roaming packet */
	else if ((RAW_AC_PORT == uh->uh_sport  && RAW_AC_PORT == uh->uh_dport) ||
			(ROAM_AC_PORT == uh->uh_sport && ROAM_AC_PORT == uh->uh_dport))
	{
		cvmx_atomic_add32(&(traffic_stats.inter_ac_roaming_pkt), 1);
	}
	/*radius  */
	else if (PORTAL_PORT == uh->uh_sport || PORTAL_PORT == uh->uh_dport)
	{
		cvmx_atomic_add32(&(traffic_stats.portal_pkt), 1);
	}
	else if (ACCESS_RADUIS_PORT == uh->uh_sport || ACCESS_RADUIS_PORT == uh->uh_dport)
	{
		cvmx_atomic_add32(&(traffic_stats.access_radius_pkt), 1);
	}
	else if (ACCOUNT_RADUIS_PORT == uh->uh_sport || ACCOUNT_RADUIS_PORT == uh->uh_dport )
	{
		cvmx_atomic_add32(&(traffic_stats.account_radius_pkt), 1);
	}
	/* Common UDP packet */
	else
	{
		return;
	}
	return;
}

inline void traffic_statistics(cvmx_wqe_t* work)
{
    uint16_t eth_protocol = 0;
    uint8_t* pkt_ptr = (uint8_t *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);         

    if(cvmx_unlikely(work == NULL))
    {
        return;
    }
    /* total packet */
    cvmx_atomic_add32(&(traffic_stats.total_pkt), 1);

    /* error packet */
    if(cvmx_unlikely(work->word2.s.rcv_error))
    {
        cvmx_atomic_add32(&(traffic_stats.err_pkt), 1);
        return;
    }
    
    /* bcast & mcast */
    if(work->word2.s.is_bcast)
    {
        cvmx_atomic_add32(&(traffic_stats.bcast_pkt), 1);
    }
    else if(work->word2.s.is_mcast)
    {
        cvmx_atomic_add32(&(traffic_stats.mcast_pkt), 1);
    }
    
    /* not ip packet */
    if(cvmx_unlikely(work->word2.s.not_IP))
    {
    	cvmx_atomic_add32(&(traffic_stats.total_noip_pkt), 1); 
        if(work->word2.snoip.is_arp)
        {
            cvmx_atomic_add32(&(traffic_stats.arp_pkt), 1);
			return;
        }
        else if(work->word2.snoip.is_rarp)
        {
            cvmx_atomic_add32(&(traffic_stats.rarp_pkt), 1);
			return;
        }
        else
        {
            if(cvmx_unlikely(work->word2.snoip.vlan_valid))
            {
                eth_protocol = *((uint16_t *)(pkt_ptr + 16));
            }
            else
            {
                eth_protocol = *((uint16_t *)(pkt_ptr + 12));
            }

            switch(eth_protocol)
            {
                case ETH_T_VRRP:
                   cvmx_atomic_add32(&(traffic_stats.vrrp_pkt), 1); 
                   return;
                case ETH_T_8021X:  
                   cvmx_atomic_add32(&(traffic_stats._802_1x_pkt), 1); 
                   return;
				case FCCP_L2_ETH_TYPE:
					cvmx_atomic_add32(&(traffic_stats.total_noip_pkt), -1);
					return;
                default:  
                   return;                   
            }
        }
    }
    /* ip packet */
    else
    {
        cvm_common_ip_hdr_t* ip = NULL;
        cvm_common_tcp_hdr_t* th = NULL;
        cvm_common_udp_hdr_t* uh = NULL;
		cvmx_atomic_add32(&(traffic_stats.total_ip_pkt), 1);
		
        if(cvmx_unlikely(work->word2.s.IP_exc))
        {
            cvmx_atomic_add32(&(traffic_stats.ip_exception_pkt), 1);
            return;
        }
        if(cvmx_unlikely(work->word2.s.is_frag))
        {
            cvmx_atomic_add32(&(traffic_stats.frag_pkt), 1);
            return;
        }
        
        if(cvmx_unlikely(work->word2.s.is_v6))
        {
            cvmx_atomic_add32(&(traffic_stats.ipv6_pkt), 1);
            return;
        }
        
        if(cvmx_unlikely(work->word2.s.L4_error))
        {
            cvmx_atomic_add32(&(traffic_stats.l4_err_pkt), 1);
            return;
        }

        ip = (cvm_common_ip_hdr_t*)(pkt_ptr + work->word2.s.ip_offset);

        switch(ip->ip_p)
        {
            case CVM_COMMON_IPPROTO_TCP:
				cvmx_atomic_add32(&(traffic_stats.total_tcp_pkt), 1);
                th = (cvm_common_tcp_hdr_t *)((uint32_t *)ip + ip->ip_hl);
                if((TELNET_PORT == th->th_sport ) || (TELNET_PORT == th->th_dport ))
                {
                    cvmx_atomic_add32(&(traffic_stats.telnet_pkt), 1);
					return;
                }
                else if((SSH_PORT == th->th_sport ) || (SSH_PORT == th->th_dport))
                {
                    cvmx_atomic_add32(&(traffic_stats.ssh_pkt), 1);
					return;
                }
                else
                {
					return;
                }    
                break;
            case CVM_COMMON_IPPROTO_UDP:
                uh = (cvm_common_udp_hdr_t *)((uint32_t *)ip + ip->ip_hl);
                udp_stats(uh);
                return;
            case CVM_COMMON_IPPROTO_ICMP:
                cvmx_atomic_add32(&(traffic_stats.icmp_pkt), 1);
                return;
            default:
                return;
        }
    }
}


