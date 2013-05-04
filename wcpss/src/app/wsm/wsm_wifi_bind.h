/******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
*******************************************************************************

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
*******************************************************************************
* wsm_wifi_bind.h
*
*
* DESCRIPTION:
*  In this file, there has some functions and structures about wifi bind L3IF.
*
* DATE:
*  2009-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2008-07-11 <guoxb> Create file.
*  2009-12-14 <guoxb> Add Inter-AC roaming table operation functions.
*  2009-12-30 <guoxb> Review source code.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*
******************************************************************************/

#ifndef _WSM_WIFI_BIND_H
#define _WSM_WIFI_BIND_H

#include <stdio.h>
#include <string.h>

#include "wcpss/waw.h"

#include "wsm_types.h"
#include "wsm_main_tbl.h"


#define WSM_BSSID_WLANID_TYPE				0x01
#define WSM_STAMAC_BSSID_WTPIP_TYPE		0x02
#define WSM_WLANID_BSSID_TYPE				0x04
#define WSM_STA_Statistics_TYPE				0x08

/* For STA Statistics */
#define WSM_STA_TX_UNI_TYPE				0x1 /* TX Unicast type */
#define WSM_STA_TX_BRD_TYPE				0x2 /* TX Broadcast type */
#define WSM_STA_RX_UNI_TYPE				0x3 /* RX Unicast type */
#define WSM_STA_RX_BRD_TYPE				0x4 /* RX Broadcast type */
#define WSM_STA_RETRY_TYPE				0x5
#define WSM_STA_ERR_TYPE					0x6 

#define WSM_COPY_MAC(dmac, smac)			memcpy(dmac, smac, MAC_LEN)


/* BSSID <-------> WlanID Table */
struct wsm_bssid_wlanid {
	unsigned char BSSID[MAC_LEN];
	unsigned char Radio_L_ID;
	unsigned int WlanID;
	unsigned int protect_type;
	wAW_IF_Type wlan_if_type;
	wAW_IF_Type bss_if_type;
};

struct WSM_BSSID_WLANID_TABLE
{
	struct wsm_bssid_wlanid element;
	struct WSM_BSSID_WLANID_TABLE *next;
};

/* STAMAC <-----> BSSID WTPIP Table */
struct wsm_stamac_bssid_wtpip {
	unsigned char STAMAC[MAC_LEN];
	unsigned char BSSID[MAC_LEN];
	/* For STA roaming 2009-05-06 GuoXuebin */
	unsigned char BSSID_before[MAC_LEN]; 
	unsigned int STAState;
	unsigned int WTPID;
	/* For STA roaming 2009-05-06 GuoXuebin */
	unsigned char roaming_flag;
	/* For Inter-AC roaming 2009-12-11 Guo Xuebin */
	roam_type ac_roam_tag; 
	struct mixwtpip ac_ip;
};

struct WSM_STAMAC_BSSID_WTPIP_TABLE {
	struct wsm_stamac_bssid_wtpip element;
	struct WSM_STAMAC_BSSID_WTPIP_TABLE *next;
};

struct wsm_wlanid_bssid {
	unsigned char BSSID[MAC_LEN];
	unsigned int BSSIndex;
	unsigned int protect_type;
	wAW_IF_Type wlan_if_type;
	wAW_IF_Type bss_if_type;
	struct wsm_wlanid_bssid *next;
};

/* Wlan Bind L3IF WlanID */
struct WSM_WLANID_BSSID_TABLE {
	unsigned int bsscount;
	unsigned char Roaming_policy;
	struct wsm_wlanid_bssid*next;
};

struct wsm_wlanid_bssid_element {
	unsigned int WlanID;
	unsigned int BSSIndex;
	unsigned char BSSID[MAC_LEN];
	unsigned int protect_type;
	wAW_IF_Type wlan_if_type;
	wAW_IF_Type bss_if_type;
};

/* STA rx/tx packets statistics */
struct WSM_STA_Statistics {
	STAStatistics element;
	struct WSM_STA_Statistics *next;
};

extern struct WSM_BSSID_WLANID_TABLE *BSSID_WLANID_Table[WIFI_BIND_MAX_NUM];
extern struct WSM_STAMAC_BSSID_WTPIP_TABLE *STAMAC_BSSID_WTPIP_Table[WSM_STA_FDB_MAX_NUM];
extern struct WSM_WLANID_BSSID_TABLE WLAN_BSSID_Table[WIFI_BIND_MAX_NUM]; 
extern struct WSM_STA_Statistics *WSM_STA_Statistics_Table[WSM_STA_FDB_MAX_NUM];


void wifi_bind_init(void);
unsigned short wsm_mac_table_hash(unsigned char *mac);
unsigned char wsm_mac_table_hash2(unsigned char *mac);
int wsm_wifi_table_add(unsigned char *data, int type);
void *wsm_wifi_table_search(unsigned char *data, int type);
int wsm_wifi_table_del(unsigned char *data, int type);
int wsm_wifi_table_print(int type);
void wsm_sta_statistics_update(unsigned char *mac, int size, int type);
__inline__ void wsm_bss_brd_statistics_update(STA_Element *pdata, unsigned int len);
int wsm_get_BSSStatistics(unsigned int WTPID);
__inline__ int wsm_get_bss_per_wtp(unsigned char *pdata, BSS_Element* BSS);
__inline__ int wsm_get_bss_statistics(BSS_Element *BSS, BSSStatistics *data);
__inline__ STAStatistics *wsm_get_sta_statistics(unsigned char *mac);
__inline__ int wsm_bss_info_sendto_wid(unsigned char *buff, unsigned int WTPID, unsigned int cnt);
void wsm_fill_in_pkt_statistics(BSSStatistics *dest, STAStatistics *src);
void wsm_get_STAStatistics(void);
__inline__ void WSM_PRINT_MAC(unsigned char *mac);
__inline__ void WSM_PRINT_IP(unsigned int ip);
int roam_tbl_add(roam_sta_ele_t *sta_info);
int roam_tbl_del(roam_sta_ele_t *sta_info);
roam_tbl_t *roam_tbl_search(unsigned char *bssid);
void roam_tbl_del_bss(unsigned char *bssid);

#endif
