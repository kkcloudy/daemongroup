#ifndef ACCHECK_REPORT_H
#define ACCHECK_REPORT_H

/* mobile std 162Mbit/s */
#define NET_STD_WIRELESS_MAX_FLOWBIT (162)	
#define NET_STD_WIRELESS_MAX_PKT_PER_SEC	(NET_STD_WIRELESS_MAX_FLOWBIT/8*1024*1024/1500)
#define NET_STD_WIRELESS_MAX_OCT_PER_SEC	(NET_STD_WIRELESS_MAX_FLOWBIT/8*1024)

/* mobile std 300Mbit/s */
#define NET_STD_WIRE_MAX_FLOWBIT (300)	
#define NET_STD_WIRE_MAX_PKT_PER_SEC	(NET_STD_WIRE_MAX_FLOWBIT/8*1024*1024/1500)
#define NET_STD_WIRE_MAX_OCT_PER_SEC	(NET_STD_WIRE_MAX_FLOWBIT/8*1024)

#define MAX_PACKET_SIZE (1500)
#define MIN_PACKET_SIZE (64)
#define FLOW_2M_BIT (2*1024*1024/8)
typedef enum web_report_type_e{
	/* EXTENTION SNR */
	_WEB_RPT_SNR_NEW = 0,
	_WEB_RPT_SNR_MIN,
	_WEB_RPT_SNR_MAX,
	_WEB_RPT_SNR_AVR,
	/* WIRELESS STATICS */
	_WEB_RPT_WIRELESS_IF_RX_PKT,
	_WEB_RPT_WIRELESS_IF_TX_PKT,
	_WEB_RPT_WIRELESS_IF_RX_BYTE,
	_WEB_RPT_WIRELESS_IF_TX_BYTE,
	/* WIRE STATICS */
	_WEB_RPT_WIRE_IF_RX_PKT,
	_WEB_RPT_WIRE_IF_TX_PKT,
	_WEB_RPT_WIRE_IF_RX_BYTE,
	_WEB_RPT_WIRE_IF_TX_BYTE,
	_WEB_RPT_UNKNOWN
}web_report_type_t;

typedef int (*web_rpt_check_t)(void *checkvalue, void *prevalue, int interval);
typedef int (*web_rpt_modify_t)(void *checkvalue, void *prevalue, int interval);


typedef struct web_report_table_s {
    web_report_type_t	type;
    web_rpt_check_t		check;
    web_rpt_modify_t    modify;
    void                *cookie;
} web_report_table_t;

void get_ap_stats_v2(unsigned int wtpid);
void check_snr_v2(void *checkvalue, void *prevalue, int flag);
#endif
