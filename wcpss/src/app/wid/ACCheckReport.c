#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include "CWAC.h"
#include "wcpss/waw.h"
#include "ACDbus.h"
#include "ACCheckReport.h"

#define RAND(a,b)	(rand()%((b)-(a))+(a))

int check_snr(void *checkvalue, void *prevalue, int interval)
{
	char *p_snr = (char *)checkvalue;
	if((gMAX_WEB_REPORT_SNR < *p_snr) )
	{
		return 1;		
	}
	else if((gMIN_WEB_REPORT_SNR > *p_snr) )
	{
		return 1;		
	}
	return 0;
}

int modify_snr(void *checkvalue, void *prevalue, int interval)
{
	char *p_snr = (char *)checkvalue;
	if((gMAX_WEB_REPORT_SNR < *p_snr) )
	{
		*p_snr = gMAX_WEB_REPORT_SNR;		
	}
	else if((gMIN_WEB_REPORT_SNR > *p_snr) )
	{
		*p_snr = gMIN_WEB_REPORT_SNR;		
	}
	return 0;
}

int check_snr_avr(void *checkvalue, void *prevalue, int interval)
{
	double *p_snr = (double *)checkvalue;
	if(((double)gMAX_WEB_REPORT_SNR < *p_snr) )
	{
		return 1;	
	}
	else if(((double)gMIN_WEB_REPORT_SNR > *p_snr) )
	{
		return 1;	
	}
	return 0;
}

int modify_snr_avr(void *checkvalue, void *prevalue, int interval)
{
	double *p_snr = (double *)checkvalue;
	if(((double)gMAX_WEB_REPORT_SNR < *p_snr) )
	{
		*p_snr = (double)gMAX_WEB_REPORT_SNR;		
	}
	else if(((double)gMIN_WEB_REPORT_SNR > *p_snr) )
	{
		*p_snr = (double)gMIN_WEB_REPORT_SNR;		
	}
	return 0;
}

int check_wireless_flow_pkt(void *checkvalue, void *prevalue, int interval)
{
	unsigned int *p_flow_pkt = (unsigned int *)checkvalue;
	unsigned int *p_pre_flow_pkt = (unsigned int *)prevalue;
	if((NULL == checkvalue) || (NULL == prevalue) || (0 == interval))
	{
		return 1;
	}
	if(0 > (*p_flow_pkt - *p_pre_flow_pkt))
	{
		return 1;		
	}
	else if((NET_STD_WIRELESS_MAX_PKT_PER_SEC * interval) < (*p_flow_pkt - *p_pre_flow_pkt))
	{
		return 1;		
	}
	return 0;
}

int modify_wireless_flow_pkt(void *checkvalue, void *prevalue, int interval)
{
	unsigned int *p_flow_pkt = (unsigned int *)checkvalue;
	unsigned int *p_pre_flow_pkt = (unsigned int *)prevalue;
	if((NULL == checkvalue) || (NULL == prevalue) || (0 == interval))
	{
		return 1;
	}
	if(0 > (*p_flow_pkt - *p_pre_flow_pkt))
	{
		*p_flow_pkt = *p_pre_flow_pkt;		
	}
	else if((NET_STD_WIRELESS_MAX_PKT_PER_SEC * interval) < (*p_flow_pkt - *p_pre_flow_pkt))
	{
		*p_flow_pkt = *p_pre_flow_pkt + RAND(50*(NET_STD_WIRELESS_MAX_PKT_PER_SEC * interval),100*(NET_STD_WIRELESS_MAX_PKT_PER_SEC * interval))/100;
		
	}
	return 0;
}

int check_wireless_flow_byte(void *checkvalue, void *prevalue, int interval)
{
	unsigned long long *p_flow_byte = (unsigned long long *)checkvalue;
	unsigned long long *p_pre_flow_byte = (unsigned long long *)prevalue;
	if((NULL == checkvalue) || (NULL == prevalue) || (0 == interval))
	{
		return 1;
	}
	if(0 > (*p_flow_byte - *p_pre_flow_byte))
	{
		return 1;		
	}
	else if((NET_STD_WIRELESS_MAX_OCT_PER_SEC * interval) < (*p_flow_byte - *p_pre_flow_byte))
	{
		return 1;		
	}
	return 0;
}

int modify_wireless_flow_byte(void *checkvalue, void *prevalue, int interval)
{
	unsigned long long *p_flow_byte = (unsigned long long *)checkvalue;
	unsigned long long *p_pre_flow_byte = (unsigned long long *)prevalue;
	if((NULL == checkvalue) || (NULL == prevalue) || (0 == interval))
	{
		return 1;
	}
	if(0 > (*p_flow_byte - *p_pre_flow_byte))
	{
		*p_flow_byte = *p_pre_flow_byte;		
	}
	else if((NET_STD_WIRELESS_MAX_OCT_PER_SEC * interval) < (*p_flow_byte - *p_pre_flow_byte))
	{
		*p_flow_byte = *p_pre_flow_byte + RAND(50*(NET_STD_WIRELESS_MAX_OCT_PER_SEC * interval),100*(NET_STD_WIRELESS_MAX_OCT_PER_SEC * interval))/100;
		
	}
	return 0;
}

int check_wire_flow_pkt(void *checkvalue, void *prevalue, int interval)
{
	unsigned int *p_flow_pkt = (unsigned int *)checkvalue;
	unsigned int *p_pre_flow_pkt = (unsigned int *)prevalue;
	if((NULL == checkvalue) || (NULL == prevalue) || (0 == interval))
	{
		return 1;
	}
	if(0 > (*p_flow_pkt - *p_pre_flow_pkt))
	{
		return 1;		
	}
	else if((NET_STD_WIRE_MAX_PKT_PER_SEC * interval) < (*p_flow_pkt - *p_pre_flow_pkt))
	{
		return 1;		
	}
	return 0;
}

int modify_wire_flow_pkt(void *checkvalue, void *prevalue, int interval)
{
	unsigned int *p_flow_pkt = (unsigned int *)checkvalue;
	unsigned int *p_pre_flow_pkt = (unsigned int *)prevalue;
	if((NULL == checkvalue) || (NULL == prevalue) || (0 == interval))
	{
		return 1;
	}
	if(0 > (*p_flow_pkt - *p_pre_flow_pkt))
	{
		*p_flow_pkt = *p_pre_flow_pkt;		
	}
	else if((NET_STD_WIRE_MAX_PKT_PER_SEC * interval) < (*p_flow_pkt - *p_pre_flow_pkt))
	{
		*p_flow_pkt = *p_pre_flow_pkt + rand()%(NET_STD_WIRE_MAX_PKT_PER_SEC * interval);
		
	}
	return 0;
}

int check_wire_flow_byte(void *checkvalue, void *prevalue, int interval)
{
	unsigned long long *p_flow_byte = (unsigned long long *)checkvalue;
	unsigned long long *p_pre_flow_byte = (unsigned long long *)prevalue;
	if((NULL == checkvalue) || (NULL == prevalue) || (0 == interval))
	{
		return 1;
	}
	if(0 > (*p_flow_byte - *p_pre_flow_byte))
	{
		return 1;		
	}
	else if((NET_STD_WIRE_MAX_OCT_PER_SEC * interval) < (*p_flow_byte - *p_pre_flow_byte))
	{
		return 1;		
	}
	return 0;
}

int modify_wire_flow_byte(void *checkvalue, void *prevalue, int interval)
{
	unsigned long long *p_flow_byte = (unsigned long long *)checkvalue;
	unsigned long long *p_pre_flow_byte = (unsigned long long *)prevalue;
	if((NULL == checkvalue) || (NULL == prevalue) || (0 == interval))
	{
		return 1;
	}
	if(0 > (*p_flow_byte - *p_pre_flow_byte))
	{
		*p_flow_byte = *p_pre_flow_byte;		
	}
	else if((NET_STD_WIRE_MAX_OCT_PER_SEC * interval) < (*p_flow_byte - *p_pre_flow_byte))
	{
		*p_flow_byte = *p_pre_flow_byte + rand()%(NET_STD_WIRE_MAX_OCT_PER_SEC * interval);
		
	}
	return 0;
}

int check_pkt_size()
{
	return 0;
}

/*
static web_report_table_t web_rpt_standard_table[] = {
	{_WEB_RPT_SNR_NEW, 				check_snr, 				modify_snr, 					NULL},
	{_WEB_RPT_SNR_MIN, 				check_snr, 				modify_snr, 					NULL},
	{_WEB_RPT_SNR_MAX, 				check_snr, 				modify_snr, 					NULL},
	{_WEB_RPT_SNR_AVR, 				check_snr_avr, 			modify_snr_avr, 				NULL},
	{_WEB_RPT_WIRELESS_IF_RX_PKT, 	check_wireless_flow_pkt, 	modify_wireless_flow_pkt, 		NULL},
	{_WEB_RPT_WIRELESS_IF_TX_PKT, 	check_wireless_flow_pkt, 	modify_wireless_flow_pkt, 		NULL},
	{_WEB_RPT_WIRELESS_IF_RX_BYTE, 	check_wireless_flow_byte, modify_wireless_flow_byte,	NULL},
	{_WEB_RPT_WIRELESS_IF_TX_BYTE, 	check_wireless_flow_byte, modify_wireless_flow_byte, 	NULL},
	{_WEB_RPT_WIRE_IF_RX_PKT, 		check_snr, 				modify_snr, 					NULL},
	{_WEB_RPT_WIRE_IF_TX_PKT, 		check_snr, 				modify_snr, 					NULL},
	{_WEB_RPT_WIRE_IF_RX_BYTE, 		check_snr, 				modify_snr, 					NULL},
	{_WEB_RPT_WIRE_IF_TX_BYTE, 		check_snr, 				modify_snr, 					NULL},

};
*/

int get_ap_stats(unsigned int wtpid)
{
	int i = 0, j = 0;
	unsigned char ethid = 0;
	unsigned char radioid = 0;

	//int subtraction = 0;

	/* get ap stats and save into web_manager_stats */
	for(j = 0; j < TOTAL_AP_IF_NUM; j++)
	{
		/* ath stats */
		if((AC_WTP[wtpid]->apstatsinfo[j].radioId < TOTAL_AP_IF_NUM+1)
			&& (AC_WTP[wtpid]->apstatsinfo[j].type == 0))		
		{
			radioid = AC_WTP[wtpid]->apstatsinfo[j].radioId;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].rx_pkt_unicast += AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_unicast;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].tx_pkt_unicast += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_unicast;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].rx_pkt_multicast += AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_multicast + AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_broadcast;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].tx_pkt_multicast += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_multicast + AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_broadcast;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].rx_data_pkts +=  AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data;  //fengwenchao add 20110617
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].tx_data_pkts +=  AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data;  //fengwenchao add 20110617
			
			if(wirelessdata_switch)
			{
				AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_packets += (AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data > AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_multicast)?
									(AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data - AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_multicast):
									(AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data);
									
				AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_packets += (AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data > AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_multicast)?
									(AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data - AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_multicast):
									(AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data);
									
				AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_bytes += (AC_WTP[wtpid]->apstatsinfo[j].rx_bytes > AC_WTP[wtpid]->apstatsinfo[j].rx_multicast)?
									(AC_WTP[wtpid]->apstatsinfo[j].rx_bytes - AC_WTP[wtpid]->apstatsinfo[j].rx_multicast):
									(AC_WTP[wtpid]->apstatsinfo[j].rx_multicast - AC_WTP[wtpid]->apstatsinfo[j].rx_bytes);
									
				AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_bytes += (AC_WTP[wtpid]->apstatsinfo[j].tx_bytes > AC_WTP[wtpid]->apstatsinfo[j].tx_multicast)?
									(AC_WTP[wtpid]->apstatsinfo[j].tx_bytes - AC_WTP[wtpid]->apstatsinfo[j].tx_multicast):
									(AC_WTP[wtpid]->apstatsinfo[j].tx_multicast - AC_WTP[wtpid]->apstatsinfo[j].tx_bytes);
			}
			else
			{
				AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_packets += AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data;
				AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_packets += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data;
				AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_bytes += AC_WTP[wtpid]->apstatsinfo[j].rx_bytes;
				AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_bytes += AC_WTP[wtpid]->apstatsinfo[j].tx_bytes;
				
			}

			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_errors += AC_WTP[wtpid]->apstatsinfo[j].tx_errors;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_errors += AC_WTP[wtpid]->apstatsinfo[j].rx_errors; //xiaodawei add, 20110406
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_drops += AC_WTP[wtpid]->apstatsinfo[j].tx_drop;	//xiaodawei add, 20110406
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_drops += AC_WTP[wtpid]->apstatsinfo[j].rx_drop;	//xiaodawei add, 20110406

			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_ast_rx_crcerr += AC_WTP[wtpid]->apstatsinfo[j].ast_rx_crcerr; 				//c6
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_ast_rx_badcrypt += AC_WTP[wtpid]->apstatsinfo[j].ast_rx_badcrypt;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_ast_rx_badmic += AC_WTP[wtpid]->apstatsinfo[j].ast_rx_badmic;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_ast_rx_phyerr += AC_WTP[wtpid]->apstatsinfo[j].ast_rx_phyerr; 				//c9

			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_pkt_mgmt += AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_mgmt;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_pkt_mgmt += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_mgmt;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_mgmt += AC_WTP[wtpid]->apstatsinfo[j].rx_mgmt;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_mgmt += AC_WTP[wtpid]->apstatsinfo[j].tx_mgmt;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_total_rx_bytes += AC_WTP[wtpid]->apstatsinfo[j].rx_sum_bytes;//book modify,2011-1-20
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_total_tx_bytes += AC_WTP[wtpid]->apstatsinfo[j].tx_sum_bytes;//
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_total_rx_pkt += AC_WTP[wtpid]->apstatsinfo[j].rx_packets;//
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_total_tx_pkt += AC_WTP[wtpid]->apstatsinfo[j].tx_packets;//book modify,2011-1-20

			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_pkt_control += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_control;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_pkt_control += AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_control;

			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].tx_pkt_signal += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_mgmt;		
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].rx_pkt_signal += AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_mgmt;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].dwlink_retry_pkts += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_retry;//ap reports total retry pkts, used for data pkts
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].stats_retry_frames += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_retry*103/100; 		//retry pkts * 103%
		}

		/* eth stats */
		if((AC_WTP[wtpid]->apstatsinfo[j].radioId < TOTAL_AP_IF_NUM+1)
			&&(AC_WTP[wtpid]->apstatsinfo[j].type == 1))
		{
			ethid = AC_WTP[wtpid]->apstatsinfo[j].wlanId;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_packets = AC_WTP[wtpid]->apstatsinfo[j].rx_packets;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_packets = AC_WTP[wtpid]->apstatsinfo[j].tx_packets;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_errors  = AC_WTP[wtpid]->apstatsinfo[j].rx_errors;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_errors  = AC_WTP[wtpid]->apstatsinfo[j].tx_errors;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_bytes   = AC_WTP[wtpid]->apstatsinfo[j].rx_bytes;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_bytes   = AC_WTP[wtpid]->apstatsinfo[j].tx_bytes;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_drop    = AC_WTP[wtpid]->apstatsinfo[j].rx_drop;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_drop    = AC_WTP[wtpid]->apstatsinfo[j].tx_drop;
		
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_pkt_broadcast = AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_broadcast;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_pkt_unicast = AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_unicast;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_pkt_broadcast = AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_broadcast;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_pkt_unicast = AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_unicast;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_pkt_multicast = AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_multicast;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_pkt_multicast = AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_multicast;
			
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_sum_bytes = AC_WTP[wtpid]->apstatsinfo[j].rx_sum_bytes;
			AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_sum_bytes = AC_WTP[wtpid]->apstatsinfo[j].tx_sum_bytes;
		}

		/* wifi stats */
		if((AC_WTP[wtpid]->apstatsinfo[j].radioId < TOTAL_AP_IF_NUM + 1)
			&&(AC_WTP[wtpid]->apstatsinfo[j].type == 2))
		{			
			if(wirelessdata_switch)
			{
				AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_rx_packets += (AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data > AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_multicast)?
									(AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data - AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_multicast):
									(AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data);
									
				AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_packets += (AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data > AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_multicast)?
									(AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data - AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_multicast):
									(AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data);
									
				AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_rx_bytes += (AC_WTP[wtpid]->apstatsinfo[j].rx_bytes > AC_WTP[wtpid]->apstatsinfo[j].rx_multicast)?
									(AC_WTP[wtpid]->apstatsinfo[j].rx_bytes - AC_WTP[wtpid]->apstatsinfo[j].rx_multicast):
									(AC_WTP[wtpid]->apstatsinfo[j].rx_multicast - AC_WTP[wtpid]->apstatsinfo[j].rx_bytes);
									
				AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_bytes += (AC_WTP[wtpid]->apstatsinfo[j].tx_bytes > AC_WTP[wtpid]->apstatsinfo[j].tx_multicast)?
									(AC_WTP[wtpid]->apstatsinfo[j].tx_bytes - AC_WTP[wtpid]->apstatsinfo[j].tx_multicast):
									(AC_WTP[wtpid]->apstatsinfo[j].tx_multicast - AC_WTP[wtpid]->apstatsinfo[j].tx_bytes);
			}
			else
			{
				AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_rx_packets += AC_WTP[wtpid]->apstatsinfo[j].rx_pkt_data;
				AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_packets += AC_WTP[wtpid]->apstatsinfo[j].tx_pkt_data;
				AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_rx_bytes += AC_WTP[wtpid]->apstatsinfo[j].rx_bytes;
				AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_bytes += AC_WTP[wtpid]->apstatsinfo[j].tx_bytes;
			}
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_errors += AC_WTP[wtpid]->apstatsinfo[j].tx_errors;
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_ast_rx_crcerr += AC_WTP[wtpid]->apstatsinfo[j].ast_rx_crcerr;
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_ast_rx_badcrypt += AC_WTP[wtpid]->apstatsinfo[j].ast_rx_badcrypt;
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_ast_rx_badmic += AC_WTP[wtpid]->apstatsinfo[j].ast_rx_badmic;
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_ast_rx_phyerr += AC_WTP[wtpid]->apstatsinfo[j].ast_rx_phyerr;
		}
	}
	/* check ap stats of web_manager_stats */
	/* check ath stats */
	for(i = 0; i < AC_WTP[wtpid]->RadioCount; i++)
	{
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].rx_pkt_unicast),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].rx_pkt_unicast),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].rx_pkt_unicast),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].rx_pkt_unicast),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].tx_pkt_unicast),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].tx_pkt_unicast),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].tx_pkt_unicast),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].tx_pkt_unicast),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].rx_pkt_multicast),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].rx_pkt_multicast),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].rx_pkt_multicast),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].rx_pkt_multicast),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].tx_pkt_multicast),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].tx_pkt_multicast),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].tx_pkt_multicast),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].tx_pkt_multicast),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].rx_data_pkts),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].rx_data_pkts),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].rx_data_pkts),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].rx_data_pkts),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].tx_data_pkts),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].tx_data_pkts),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].tx_data_pkts),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].tx_data_pkts),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_packets),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_packets),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_packets),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_packets),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		/* sub_wirelessIfTxDataPkts  ONLINEBUG-361 */
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_packets),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_packets),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_packets),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_packets),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		/* sub_wirelessIfRxDataPkts  ONLINEBUG-361 */
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_bytes),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_bytes),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_bytes),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_bytes),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_bytes),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_bytes),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_bytes),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_bytes),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		/* --- 10 values just for count --- */
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_errors),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_errors),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_errors),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_errors),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_errors),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_errors),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_errors),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_errors),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_drops),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_drops),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_drops),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_drops),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_drops),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_drops),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_drops),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_drops),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_ast_rx_crcerr),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_ast_rx_crcerr),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_ast_rx_crcerr),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_ast_rx_crcerr),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_ast_rx_badcrypt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_ast_rx_badcrypt),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_ast_rx_badcrypt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_ast_rx_badcrypt),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_ast_rx_badmic),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_ast_rx_badmic),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_ast_rx_badmic),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_ast_rx_badmic),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_ast_rx_phyerr),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_ast_rx_phyerr),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_ast_rx_phyerr),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_ast_rx_phyerr),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_pkt_mgmt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_pkt_mgmt),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_pkt_mgmt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_pkt_mgmt),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_pkt_mgmt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_pkt_mgmt),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_pkt_mgmt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_pkt_mgmt),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		/* --- 20 values just for count --- */
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_mgmt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_mgmt),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_mgmt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_mgmt),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_mgmt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_mgmt),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_mgmt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_mgmt),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_rx_bytes),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_rx_bytes),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_rx_bytes),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_rx_bytes),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_tx_bytes),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_tx_bytes),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_tx_bytes),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_tx_bytes),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_rx_pkt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_rx_pkt),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_rx_pkt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_rx_pkt),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_tx_pkt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_tx_pkt),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_tx_pkt),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_tx_pkt),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_pkt_control),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_pkt_control),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_pkt_control),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_pkt_control),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_pkt_control),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_pkt_control),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_pkt_control),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_pkt_control),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].tx_pkt_signal),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].tx_pkt_signal),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].tx_pkt_signal),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].tx_pkt_signal),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].rx_pkt_signal),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].rx_pkt_signal),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].rx_pkt_signal),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].rx_pkt_signal),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		/* --- 30 values just for count --- */
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].dwlink_retry_pkts),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].dwlink_retry_pkts),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].dwlink_retry_pkts),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].dwlink_retry_pkts),
									AC_WTP[wtpid]->apstatisticsinterval);
		}		
		if(check_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].stats_retry_frames),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].stats_retry_frames),
									AC_WTP[wtpid]->apstatisticsinterval))
		{
			modify_wireless_flow_pkt(&(AC_WTP[wtpid]->web_manager_stats.ath_stats[i].stats_retry_frames),
									&(AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].stats_retry_frames),
									AC_WTP[wtpid]->apstatisticsinterval);
		}
		/* check multi value logic relation */
		

	}
	/* check eth stats */

	/* check wifi stats */
	return 0;
}

void check_snr_v2(void *checkvalue, void *prevalue, int flag)
{
	char *p_snr = (char *)checkvalue;
	double *p_snr_avr = (double *)checkvalue;
	if(NULL == checkvalue)
	{
		return;
	}
	
	switch(flag)
	{
		case 0:
			if((gMAX_WEB_REPORT_SNR < *p_snr) )
			{
				wid_syslog_info("%s %d: ERR new_snr %d\n",__func__,__LINE__,*p_snr);	
				*p_snr = gMAX_WEB_REPORT_SNR;		
			}
			else if((gMIN_WEB_REPORT_SNR > *p_snr) )
			{
				wid_syslog_info("%s %d: ERR new_snr %d\n",__func__,__LINE__,*p_snr);
				*p_snr = gMIN_WEB_REPORT_SNR;		
			}
			break;
		case 1:
			if((gMAX_WEB_REPORT_SNR < *p_snr)
				|| (gMIN_WEB_REPORT_SNR > *p_snr))
			{
				wid_syslog_info("%s %d: ERR max_snr %d\n",__func__,__LINE__,*p_snr);
				*p_snr = gMAX_WEB_REPORT_SNR;		
			}
			break;			
		case 2:
			if((gMAX_WEB_REPORT_SNR < *p_snr)
				|| (gMIN_WEB_REPORT_SNR > *p_snr))
			{
				wid_syslog_info("%s %d: ERR min_snr %d\n",__func__,__LINE__,*p_snr);
				*p_snr = gMIN_WEB_REPORT_SNR;		
			}
			break;			
		case 3:
			if(NULL == prevalue)
			{
				return;
			}
			if(((double)gMAX_WEB_REPORT_SNR < *p_snr_avr)
				|| ((double)gMIN_WEB_REPORT_SNR > *p_snr_avr))
			{
				wid_syslog_info("%s %d: ERR avr_snr %f\n",__func__,__LINE__,*p_snr_avr);
				*p_snr_avr = *(double *)prevalue;		
			}
			break;
		default:
			break;
	}
	
	return;		
}

void calc_ap_stats_ath(unsigned int wtpid, unsigned int apstatsid)
{
	unsigned char radioid = 0;
	/* ath stats */
	if(WTP_NUM <= wtpid)
	{
		return;
	}
	else if(NULL == AC_WTP[wtpid])
	{
		return;
	}
	
	if((TOTAL_AP_IF_NUM > apstatsid) 
		&& (TOTAL_AP_IF_NUM+1 > AC_WTP[wtpid]->apstatsinfo[apstatsid].radioId)
		&& (AC_WTP[wtpid]->apstatsinfo[apstatsid].type == 0))		
	{
		radioid = AC_WTP[wtpid]->apstatsinfo[apstatsid].radioId;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].rx_pkt_unicast += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_unicast;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].tx_pkt_unicast += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_unicast;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].rx_pkt_multicast += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast + AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_broadcast;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].tx_pkt_multicast += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast + AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_broadcast;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].rx_data_pkts +=  AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data;  //fengwenchao add 20110617
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].tx_data_pkts +=  AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data;  //fengwenchao add 20110617
		
		if(wirelessdata_switch)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_packets += (AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data > AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data);
								
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_packets += (AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data > AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data);
								
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_bytes += (AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes > AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes);
								
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_bytes += (AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes > AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes);
			/* calc the total wireless rx tx data */
			AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath += (AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data > AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data);
								
			AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath += (AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data > AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data);
								
			AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath += (AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes > AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes);
								
			AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath += (AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes > AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes);

		}
		else
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_packets += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_packets += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_bytes += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes;
			AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_bytes += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes;
			/* calc the total wireless rx tx data */
			AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data;
			AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data;
			AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes;
			AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes;
		}
		
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_errors += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_errors;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_errors += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_errors; //xiaodawei add, 20110406
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_drops += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_drop;	//xiaodawei add, 20110406
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_drops += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_drop;	//xiaodawei add, 20110406

		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_ast_rx_crcerr += AC_WTP[wtpid]->apstatsinfo[apstatsid].ast_rx_crcerr;				//c6
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_ast_rx_badcrypt += AC_WTP[wtpid]->apstatsinfo[apstatsid].ast_rx_badcrypt;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_ast_rx_badmic += AC_WTP[wtpid]->apstatsinfo[apstatsid].ast_rx_badmic;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_ast_rx_phyerr += AC_WTP[wtpid]->apstatsinfo[apstatsid].ast_rx_phyerr;				//c9

		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_pkt_mgmt += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_mgmt;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_pkt_mgmt += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_mgmt;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_mgmt += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_mgmt;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_mgmt += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_mgmt;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_total_rx_bytes += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_sum_bytes;//book modify,2011-1-20
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_total_tx_bytes += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_sum_bytes;//
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_total_rx_pkt += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_packets;//
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_total_tx_pkt += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_packets;//book modify,2011-1-20
		/* calc the total wireless rx tx data */
		AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_sum_bytes;//book modify,2011-1-20
		AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_sum_bytes;//
		AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_packets;//
		AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_packets;//book modify,2011-1-20
		
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_tx_pkt_control += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_control;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].sub_rx_pkt_control += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_control;

		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].tx_pkt_signal += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_mgmt; 	
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].rx_pkt_signal += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_mgmt;
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].dwlink_retry_pkts += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_retry;//ap reports total retry pkts, used for data pkts
		AC_WTP[wtpid]->web_manager_stats.ath_stats[radioid].stats_retry_frames += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_retry*103/100;		//retry pkts * 103%

	}
	return;
}

void calc_ap_stats_eth(unsigned int wtpid, unsigned int apstatsid)
{
	unsigned char ethid = 0;

	if(WTP_NUM <= wtpid)
	{
		return;
	}
	else if(NULL == AC_WTP[wtpid])
	{
		return;
	}
	/* eth stats */
	if((TOTAL_AP_IF_NUM > apstatsid) 
		&&(AC_WTP[wtpid]->apstatsinfo[apstatsid].radioId < TOTAL_AP_IF_NUM+1)
		&&(AC_WTP[wtpid]->apstatsinfo[apstatsid].type == 1))
	{
		ethid = AC_WTP[wtpid]->apstatsinfo[apstatsid].wlanId;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_packets = AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_packets;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_packets = AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_packets;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_errors  = AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_errors;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_errors  = AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_errors;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_bytes	 = AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_bytes	 = AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_drop	 = AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_drop;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_drop	 = AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_drop;
	
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_pkt_broadcast = AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_broadcast;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_pkt_unicast = AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_unicast;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_pkt_broadcast = AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_broadcast;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_pkt_unicast = AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_unicast;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_pkt_multicast = AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_pkt_multicast = AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast;
		
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].rx_sum_bytes = AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_sum_bytes;
		AC_WTP[wtpid]->web_manager_stats.eth_stats[ethid].tx_sum_bytes = AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_sum_bytes;
		/* calc the total wire rx tx data */
		AC_WTP[wtpid]->web_manager_stats.rx_packets_eth += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_packets;
		AC_WTP[wtpid]->web_manager_stats.tx_packets_eth += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_packets;
		AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth	+= AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes;
		AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth	+= AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes;
		AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_sum_bytes;
		AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_sum_bytes;
	
	}
	return;
}
void calc_ap_stats_wifi(unsigned int wtpid, unsigned int apstatsid)
{
	if(WTP_NUM <= wtpid)
	{
		return;
	}
	else if(NULL == AC_WTP[wtpid])
	{
		return;
	}
	/* wifi stats */
	if((TOTAL_AP_IF_NUM > apstatsid) 
		&&(AC_WTP[wtpid]->apstatsinfo[apstatsid].radioId < TOTAL_AP_IF_NUM + 1)
		&&(AC_WTP[wtpid]->apstatsinfo[apstatsid].type == 2))
	{			
		if(wirelessdata_switch)
		{
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_rx_packets += (AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data > AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data);
								
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_packets += (AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data > AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data);
								
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_rx_bytes += (AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes > AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes);
								
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_bytes += (AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes > AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast)?
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast):
								(AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_multicast - AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes);
		}
		else
		{
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_rx_packets += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_pkt_data;
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_packets += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_pkt_data;
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_rx_bytes += AC_WTP[wtpid]->apstatsinfo[apstatsid].rx_bytes;
			AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_bytes += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_bytes;
		}
		AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_tx_errors += AC_WTP[wtpid]->apstatsinfo[apstatsid].tx_errors;
		AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_ast_rx_crcerr += AC_WTP[wtpid]->apstatsinfo[apstatsid].ast_rx_crcerr;
		AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_ast_rx_badcrypt += AC_WTP[wtpid]->apstatsinfo[apstatsid].ast_rx_badcrypt;
		AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_ast_rx_badmic += AC_WTP[wtpid]->apstatsinfo[apstatsid].ast_rx_badmic;
		AC_WTP[wtpid]->web_manager_stats.wifi_stats.wtp_ast_rx_phyerr += AC_WTP[wtpid]->apstatsinfo[apstatsid].ast_rx_phyerr;
	}

	return;
}

void get_ap_stats_v2(unsigned int wtpid)
{
	int i = 0, j = 0;
	unsigned int revise = 0;
	unsigned long long revise_byte = 0;

	unsigned int report_interval = 0;
	unsigned char radiocnt = 0;	
	unsigned int wireless_pkts_max = 0;
	int wireless_pkts_interval = 0;
	long long wireless_bytes_interval = 0;
	
	unsigned char ethcnt = 0;
	unsigned int wired_pkts_max = 0;
	int wired_pkts_interval = 0;
	long long wired_bytes_interval = 0;
	long long wired_sum_bytes_interval = 0;

	web_manager_stats_t web_report_tmp;
	
	if(WTP_NUM <= wtpid)
	{
		return;
	}
	else if(NULL == AC_WTP[wtpid])
	{
		return;
	}
	pthread_mutex_lock(&(AC_WTP[wtpid]->mutex_web_report));
	memset(&AC_WTP[wtpid]->web_manager_stats, 0, sizeof(web_manager_stats_t));
	/* get ap stats and save into web_manager_stats */
	for(j = 0; j < TOTAL_AP_IF_NUM; j++)
	{		
		calc_ap_stats_ath(wtpid, j);
		calc_ap_stats_eth(wtpid, j);		
		calc_ap_stats_wifi(wtpid, j);
	}

	memset(&web_report_tmp, 0, sizeof(web_manager_stats_t));
	if(0 == memcmp(&AC_WTP[wtpid]->pre_web_manager_stats, &web_report_tmp, sizeof(web_manager_stats_t)))
	{
		/* ap first report, no check */
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		return;

	}
	/* check ap stats of web_manager_stats */
	radiocnt = AC_WTP[wtpid]->RadioCount;
	ethcnt = AC_WTP[wtpid]->apifinfo.eth_num;	
	wireless_pkts_max = NET_STD_WIRELESS_MAX_PKT_PER_SEC * AC_WTP[wtpid]->apstatisticsinterval;
	wired_pkts_max = NET_STD_WIRE_MAX_PKT_PER_SEC * AC_WTP[wtpid]->apstatisticsinterval;
	report_interval = AC_WTP[wtpid]->apstatisticsinterval;
	/* ath === sub_rx_packets & sub_rx_bytes === */
	wireless_pkts_interval = AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath;	
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath;

	if(0 > wireless_bytes_interval && 0 > wireless_pkts_interval)
	{
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_sub_rx_packets %lu,sub_rx_packets %lu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath);	
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));		
		return;
	}
	if(wireless_pkts_max < wireless_pkts_interval)
	{	
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_rx_packets %lu,sub_rx_packets %lu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath);	
		revise = RAND(40,80)*wireless_pkts_max/100;
		AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath + revise;
		for(i = 0; i < radiocnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_packets = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_packets + revise/radiocnt;
		}
	}
	/* 
	else if(0 > wireless_pkts_interval)
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_rx_packets %lu,sub_rx_packets %lu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath);
		AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath;
		for(i = 0; i < AC_WTP[wtpid]->RadioCount; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_packets = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_packets;
		}		
	}
	*/
	wireless_pkts_interval = AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath;
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath;
	if(0 > wireless_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_sub_rx_bytes %llu,sub_rx_bytes %llu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath);
		return;		
	}
	if(0 == wireless_pkts_interval)
	{
		if(0 != wireless_bytes_interval)
		{
			wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_rx_packets %lu,sub_rx_packets %lu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath);
			wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_rx_bytes %llu,sub_rx_bytes %llu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath);
			AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath;
			for(i = 0; i < radiocnt; i++)
			{
				AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_bytes;
			}
		}
		
	}
	else if((wireless_pkts_interval != 0) && (MAX_PACKET_SIZE < wireless_bytes_interval/wireless_pkts_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_rx_packets %lu,sub_rx_packets %lu D\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_rx_bytes %llu,sub_rx_bytes %llu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath);
		revise_byte = RAND(500,1000)*wireless_pkts_interval;	/* packet size 500~1000 */
		AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath + revise_byte;
		for(i = 0; i < radiocnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_rx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_rx_bytes + revise_byte/radiocnt;
		}		
	}
	
	/* ath === sub_tx_packets & sub_tx_bytes === */
	wireless_pkts_interval = AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_packets_ath;		
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath;

	if( 0 > wireless_pkts_interval && 0 > wireless_bytes_interval )
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_sub_tx_packets %lu,sub_tx_packets %lu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath);
		return;		
	}
	if(wireless_pkts_max < wireless_pkts_interval)
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_tx_packets %lu,sub_tx_packets %lu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath);
		revise = RAND(80,90)*wireless_pkts_max/100;
		AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_packets_ath + revise;
		for(i = 0; i < radiocnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_packets = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_packets + revise/radiocnt;
		}
	}
	/*
	else if(0 > (AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_packets_ath))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_tx_packets %lu,sub_tx_packets %lu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath);
		AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_packets_ath;
		for(i = 0; i < AC_WTP[wtpid]->RadioCount; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_packets = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_packets;
		}		
	}
	*/
	wireless_pkts_interval = AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_packets_ath;
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath;
	if(0 > wireless_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		
		
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_sub_tx_bytes %llu,sub_tx_bytes %llu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath);
		return;		
	}

	if(0 == wireless_pkts_interval)
	{
		if(0 != wireless_bytes_interval)
		{
			wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_tx_packets %lu,sub_tx_packets %lu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath);
			wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_tx_bytes %llu,sub_tx_bytes %llu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath);
			AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath;
			for(i = 0; i < radiocnt; i++)
			{
				AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_bytes;
			}
		}
		
	}
	else if((wireless_pkts_interval != 0) && (MAX_PACKET_SIZE < wireless_bytes_interval/wireless_pkts_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_tx_packets %lu,sub_tx_packets %lu D\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_packets_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_tx_bytes %llu,sub_tx_bytes %llu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath);
		revise_byte = RAND(500,1000)*wireless_pkts_interval;	/* packet size 500~1000 */
		AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath + revise_byte;
		for(i = 0; i < radiocnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_tx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_tx_bytes + revise_byte/radiocnt;
		}		
	}	
	
	/* ath === sub_total_rx_pkt & sub_total_rx_bytes === */
	wireless_pkts_interval = AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath;	
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_bytes_ath;
	if(0 > wireless_pkts_interval && 0 > wireless_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_sub_total_rx_pkt %lu,sub_total_rx_pkt %lu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath);
		return;		
	}

	if(wireless_pkts_max < wireless_pkts_interval)
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_rx_pkt %lu,sub_total_rx_pkt %lu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath);
		revise = RAND(90,100)*wireless_pkts_max/100;
		AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath + revise;
		for(i = 0; i < radiocnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_rx_pkt = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_rx_pkt + revise/radiocnt;
		}
	}
	/*
	else if(0 > (AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_rx_pkt %lu,sub_total_rx_pkt %lu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath);
		AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath;
		for(i = 0; i < AC_WTP[wtpid]->RadioCount; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_rx_pkt = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_rx_pkt;
		}		
	}
	*/
	wireless_pkts_interval = AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath;
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_bytes_ath;
	if(0 > wireless_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_sub_total_rx_bytes %llu,sub_total_rx_bytes %llu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath);
		return;		
	}

	if(0 == wireless_pkts_interval)
	{
		if(0 != wireless_bytes_interval)
		{
			wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_rx_pkt %lu,sub_total_rx_pkt %lu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath);
			wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_rx_bytes %llu,sub_total_rx_bytes %llu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath);
			AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_bytes_ath;
			for(i = 0; i < radiocnt; i++)
			{
				AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_rx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_rx_bytes;
			}
		}		
	}
	else if((wireless_pkts_interval != 0) && (MAX_PACKET_SIZE < wireless_bytes_interval/wireless_pkts_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_rx_pkt %lu,sub_total_rx_pkt %lu D\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_rx_bytes %llu,sub_total_rx_bytes %llu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath);
		revise_byte = RAND(1000,1500)*wireless_pkts_interval;
		AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_total_rx_bytes_ath + revise_byte;
		for(i = 0; i < radiocnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_rx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_rx_bytes + revise_byte/radiocnt;
		}		
	}	
	
	/* ath === sub_total_tx_pkt & sub_total_tx_bytes === */
	wireless_pkts_interval = AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath;	
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_bytes_ath;

	if(0 > wireless_pkts_interval && 0 > wireless_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_sub_total_tx_pkt %lu,sub_total_tx_pkt %lu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath);
		return;		
	}

	if(wireless_pkts_max < wireless_pkts_interval)
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_tx_pkt %lu,sub_total_tx_pkt %lu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath);

		revise = RAND(90,100)*wireless_pkts_max/100;
		AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath + revise;
		for(i = 0; i < radiocnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_tx_pkt = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_tx_pkt + revise/radiocnt;
		}
	}
	/*
	else if(0 > (AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_tx_pkt %lu,sub_total_tx_pkt %lu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath);
		AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath;
		for(i = 0; i < AC_WTP[wtpid]->RadioCount; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_tx_pkt = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_tx_pkt;
		}		
	}
	*/
	wireless_pkts_interval = AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath;
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_bytes_ath;
	if(0 > wireless_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_sub_total_tx_bytes %llu,sub_total_tx_bytes %llu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath);
		return;		
	}

	if(0 == wireless_pkts_interval)
	{
		if(0 != wireless_bytes_interval)
		{
			wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_tx_pkt %lu,sub_total_tx_pkt %lu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath);
			wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_tx_bytes %llu,sub_total_tx_bytes %llu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath);
			AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_bytes_ath;
			for(i = 0; i < radiocnt; i++)
			{
				AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_tx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_tx_bytes;
			}
		}
		
	}
	else if((wireless_pkts_interval != 0) && (MAX_PACKET_SIZE < wireless_bytes_interval/wireless_pkts_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_tx_pkt %lu,sub_total_tx_pkt %lu D\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_pkt_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_total_tx_bytes %llu,sub_total_tx_bytes %llu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath);
		revise_byte = RAND(1000,1500)*wireless_pkts_interval;
		AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath = AC_WTP[wtpid]->pre_web_manager_stats.sub_total_tx_bytes_ath + revise_byte;
		for(i = 0; i < radiocnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.ath_stats[i].sub_total_tx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.ath_stats[i].sub_total_tx_bytes + revise_byte/radiocnt;
		}		
	}

	/* eth === rx_packets & rx_bytes === */
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath;
	wired_bytes_interval = AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth - AC_WTP[wtpid]->pre_web_manager_stats.rx_bytes_eth;
	if(0 > wired_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_rx_bytes %llu,rx_bytes %llu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_bytes_eth,AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth);
		return;		
	}

	if((wired_bytes_interval >= wireless_bytes_interval)
		&& ((report_interval != 0) && (FLOW_2M_BIT < 600 * wired_bytes_interval/report_interval))
		&& ((wired_bytes_interval - wireless_bytes_interval)*4 > wired_bytes_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_tx_bytes %llu,sub_tx_bytes %llu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_rx_bytes %llu,rx_bytes %llu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_bytes_eth,AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth);
		revise_byte = 120 * wireless_bytes_interval/100;
		AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth = AC_WTP[wtpid]->pre_web_manager_stats.rx_bytes_eth + revise_byte;
		for(i = 0; i < ethcnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.eth_stats[i].rx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].rx_bytes + revise_byte/ethcnt;
		}
	}
	else if((wired_bytes_interval < wireless_bytes_interval)
			&& ((report_interval != 0) && (FLOW_2M_BIT < 600 * wireless_bytes_interval/report_interval))
			&& ((wireless_bytes_interval - wired_bytes_interval)*4 > wireless_bytes_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_tx_bytes %llu,sub_tx_bytes %llu D\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_tx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_rx_bytes %llu,rx_bytes %llu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_bytes_eth,AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth);
		revise_byte = 120 * wireless_bytes_interval/100;
		AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth = AC_WTP[wtpid]->pre_web_manager_stats.rx_bytes_eth + revise_byte;
		for(i = 0; i < ethcnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.eth_stats[i].rx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].rx_bytes + revise_byte/ethcnt;
		}	
	}
	/* eth === rx_sum_bytes === */
	wired_bytes_interval = AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth - AC_WTP[wtpid]->pre_web_manager_stats.rx_bytes_eth;
	wired_sum_bytes_interval = AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth - AC_WTP[wtpid]->pre_web_manager_stats.rx_sum_bytes_eth;
	if(0 > wired_sum_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_rx_sum_bytes %llu,rx_sum_bytes %llu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_sum_bytes_eth,AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth);
		return;		
	}

	if(NET_STD_WIRE_MAX_OCT_PER_SEC * report_interval < wired_sum_bytes_interval)
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_rx_bytes %llu,rx_bytes %llu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_bytes_eth,AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_rx_sum_bytes %llu,rx_sum_bytes %llu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_sum_bytes_eth,AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth);
		revise_byte = RAND(100,120)*wired_bytes_interval/100;
		AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth = AC_WTP[wtpid]->pre_web_manager_stats.rx_sum_bytes_eth + revise_byte;
		for(i = 0; i < radiocnt; i++)
		{
			if (ethcnt != 0)
				AC_WTP[wtpid]->web_manager_stats.eth_stats[i].rx_sum_bytes = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].rx_sum_bytes + revise_byte/ethcnt;
		}		
	}	
	
	wired_pkts_interval = AC_WTP[wtpid]->web_manager_stats.rx_packets_eth - AC_WTP[wtpid]->pre_web_manager_stats.rx_packets_eth;
	wired_sum_bytes_interval = AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth - AC_WTP[wtpid]->pre_web_manager_stats.rx_sum_bytes_eth;
	if(0 > wired_pkts_interval && 0 > wired_sum_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && &(AC_WTP[wtpid]->web_manager_stats)){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_rx_packets %lu,rx_packets %lu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_packets_eth,AC_WTP[wtpid]->web_manager_stats.rx_packets_eth);
		return;		
	}	
	if(0 == wired_pkts_interval)
	{
		wid_syslog_info("__ %s %d __", __func__, __LINE__);
		if(0 != wired_sum_bytes_interval)
		{
			wid_syslog_info("%s %d:WTP[%d] ERR pre_rx_sum_bytes %llu,rx_sum_bytes %llu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_sum_bytes_eth,AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth);
			wid_syslog_info("%s %d:WTP[%d] ERR pre_rx_packets %lu,rx_packets %lu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_packets_eth,AC_WTP[wtpid]->web_manager_stats.rx_packets_eth);
			revise = wired_sum_bytes_interval/RAND(1200,MAX_PACKET_SIZE);
			AC_WTP[wtpid]->web_manager_stats.rx_packets_eth = AC_WTP[wtpid]->pre_web_manager_stats.rx_packets_eth + revise;
			for(i = 0; i < radiocnt; i++)
			{
				if (ethcnt != 0)
					AC_WTP[wtpid]->web_manager_stats.eth_stats[i].rx_packets = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].rx_packets + revise/ethcnt;
			}
		}
	}
	else if((wired_pkts_interval != 0 ) && (MAX_PACKET_SIZE < wired_sum_bytes_interval/wired_pkts_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_rx_sum_bytes %llu,rx_sum_bytes %llu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_sum_bytes_eth,AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_rx_packets %lu,rx_packets %lu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.rx_packets_eth,AC_WTP[wtpid]->web_manager_stats.rx_packets_eth);
		revise = wired_sum_bytes_interval/RAND(1200,MAX_PACKET_SIZE);
		AC_WTP[wtpid]->web_manager_stats.rx_packets_eth = AC_WTP[wtpid]->pre_web_manager_stats.rx_packets_eth + revise;
		for(i = 0; i < radiocnt; i++)
		{
			if (ethcnt != 0)
				AC_WTP[wtpid]->web_manager_stats.eth_stats[i].rx_packets = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].rx_packets + revise/ethcnt;
		}
	}		
	
	/* eth === tx_packets & tx_bytes === */
	wireless_bytes_interval = AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath - AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath;
	wired_bytes_interval = AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth - AC_WTP[wtpid]->pre_web_manager_stats.tx_bytes_eth;
	if(0 > wired_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_tx_bytes %llu,tx_bytes %llu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_bytes_eth,AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth);
		return;		
	}	

	if((wired_bytes_interval >= wireless_bytes_interval)
		&& ((report_interval != 0) && (FLOW_2M_BIT < 600 * wired_bytes_interval/report_interval))
		&& ((wired_bytes_interval - wireless_bytes_interval)*4 > wired_bytes_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_rx_bytes %llu,sub_rx_bytes %llu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_tx_bytes %llu,tx_bytes %llu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_bytes_eth,AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth);
		revise_byte = 120 * wireless_bytes_interval/100;
		AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth = AC_WTP[wtpid]->pre_web_manager_stats.tx_bytes_eth + revise_byte;
		for(i = 0; i < ethcnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.eth_stats[i].tx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].tx_bytes + revise_byte/ethcnt;
		}
	}
	else if((wired_bytes_interval < wireless_bytes_interval)
			&& ((report_interval != 0) && (FLOW_2M_BIT < 600 * wireless_bytes_interval/report_interval))
			&& ((wireless_bytes_interval - wired_bytes_interval)*4 > wireless_bytes_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_sub_rx_bytes %llu,sub_rx_bytes %llu D\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.sub_rx_bytes_ath,AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_tx_bytes %llu,tx_bytes %llu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_bytes_eth,AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth);
		revise_byte = 120 * wireless_bytes_interval/100;
		AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth = AC_WTP[wtpid]->pre_web_manager_stats.tx_bytes_eth + revise_byte;
		for(i = 0; i < ethcnt; i++)
		{
			AC_WTP[wtpid]->web_manager_stats.eth_stats[i].tx_bytes = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].tx_bytes + revise_byte/ethcnt;
		}	
	}
	/* eth === tx_sum_bytes === */	
	wired_bytes_interval = AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth - AC_WTP[wtpid]->pre_web_manager_stats.tx_bytes_eth;
	wired_sum_bytes_interval = AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth - AC_WTP[wtpid]->pre_web_manager_stats.tx_sum_bytes_eth;
	if(0 > wired_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_tx_sum_bytes %llu,tx_sum_bytes %llu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_sum_bytes_eth,AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth);
		return;		
	}	

	if(NET_STD_WIRE_MAX_OCT_PER_SEC * report_interval < wired_sum_bytes_interval)
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_tx_bytes %llu,tx_bytes %llu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_bytes_eth,AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_tx_sum_bytes %llu,tx_sum_bytes %llu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_sum_bytes_eth,AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth);
		revise_byte = RAND(100,120)*wired_bytes_interval/100;
		AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth = AC_WTP[wtpid]->pre_web_manager_stats.tx_sum_bytes_eth + revise_byte;
		for(i = 0; i < radiocnt; i++)
		{
			if (ethcnt != 0)
				AC_WTP[wtpid]->web_manager_stats.eth_stats[i].tx_sum_bytes = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].tx_sum_bytes + revise_byte/ethcnt;
		}		
	}	
	
	wired_pkts_interval = AC_WTP[wtpid]->web_manager_stats.tx_packets_eth - AC_WTP[wtpid]->pre_web_manager_stats.tx_packets_eth;
	wired_sum_bytes_interval = AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth - AC_WTP[wtpid]->pre_web_manager_stats.tx_sum_bytes_eth;
	if(0 > wired_pkts_interval && 0 > wired_sum_bytes_interval)
	{
		if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
			memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
			}
		else
			{
			wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);
			}
		pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
		wid_syslog_info("%s %d:WTP[%d] ERR RETURN pre_tx_packets %lu,tx_packets %lu\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_packets_eth,AC_WTP[wtpid]->web_manager_stats.tx_packets_eth);
		return;		
	}	
	if(0 == wired_pkts_interval)
	{
		if(0 != wired_sum_bytes_interval)
		{
			wid_syslog_info("%s %d:WTP[%d] ERR pre_tx_sum_bytes %llu,tx_sum_bytes %llu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_sum_bytes_eth,AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth);
			wid_syslog_info("%s %d:WTP[%d] ERR pre_tx_packets %lu,tx_packets %lu A\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_packets_eth,AC_WTP[wtpid]->web_manager_stats.tx_packets_eth);
			revise = wired_sum_bytes_interval/RAND(1200,MAX_PACKET_SIZE);
			AC_WTP[wtpid]->web_manager_stats.tx_packets_eth = AC_WTP[wtpid]->pre_web_manager_stats.tx_packets_eth + revise;
			for(i = 0; i < radiocnt; i++)
			{
				if (ethcnt != 0)
					AC_WTP[wtpid]->web_manager_stats.eth_stats[i].tx_packets = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].tx_packets + revise/ethcnt;
			}
		}
	}
	else if((wired_pkts_interval != 0) && (MAX_PACKET_SIZE < wired_sum_bytes_interval/wired_pkts_interval))
	{
		wid_syslog_info("%s %d:WTP[%d] ERR pre_tx_sum_bytes %llu,tx_sum_bytes %llu C\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_sum_bytes_eth,AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth);
		wid_syslog_info("%s %d:WTP[%d] ERR pre_tx_packets %lu,tx_packets %lu B\n",__func__,__LINE__,wtpid,AC_WTP[wtpid]->pre_web_manager_stats.tx_packets_eth,AC_WTP[wtpid]->web_manager_stats.tx_packets_eth);
		revise = wired_sum_bytes_interval/RAND(1200,MAX_PACKET_SIZE);
		AC_WTP[wtpid]->web_manager_stats.tx_packets_eth = AC_WTP[wtpid]->pre_web_manager_stats.tx_packets_eth + revise;
		for(i = 0; i < radiocnt; i++)
		{
			if (ethcnt != 0)
				AC_WTP[wtpid]->web_manager_stats.eth_stats[i].tx_packets = AC_WTP[wtpid]->pre_web_manager_stats.eth_stats[i].tx_packets + revise/ethcnt;
		}
	}

	/* === end === */
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.sub_rx_packets_ath %lu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.sub_rx_packets_ath);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.sub_tx_packets_ath %lu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.sub_tx_packets_ath);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.sub_rx_bytes_ath %llu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.sub_rx_bytes_ath);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.sub_tx_bytes_ath %llu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.sub_tx_bytes_ath);

	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.sub_total_rx_pkt_ath %lu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.sub_total_rx_pkt_ath);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.sub_total_tx_pkt_ath %lu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.sub_total_tx_pkt_ath);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.sub_total_rx_bytes_ath %llu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.sub_total_rx_bytes_ath);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.sub_total_tx_bytes_ath %llu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.sub_total_tx_bytes_ath);

	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.rx_bytes_eth %llu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.rx_bytes_eth);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.tx_bytes_eth %llu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.tx_bytes_eth);	

	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.rx_packets_eth %lu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.rx_packets_eth);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.tx_packets_eth %lu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.tx_packets_eth);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.rx_sum_bytes_eth %llu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.rx_sum_bytes_eth);
	wid_syslog_debug_debug(WID_DEFAULT, "%s:WTP[%d]web_manager_stats.tx_sum_bytes_eth %llu\n", __func__, wtpid, AC_WTP[wtpid]->web_manager_stats.tx_sum_bytes_eth);
	

	if(&(AC_WTP[wtpid]->pre_web_manager_stats) && (&(AC_WTP[wtpid]->web_manager_stats))){
		memcpy(&(AC_WTP[wtpid]->pre_web_manager_stats), &(AC_WTP[wtpid]->web_manager_stats), sizeof(web_manager_stats_t));
		}
	else
		{
		wid_syslog_err("%s %d pointer is NULL\n",__FUNCTION__,__LINE__);}
	pthread_mutex_unlock(&(AC_WTP[wtpid]->mutex_web_report));
	return;
}


