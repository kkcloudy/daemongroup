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
* wsm_dbus.c
*
*
* DESCRIPTION:
*  WSM Module Dbus implement.
*
* DATE:
*  2009-10-13
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2009-10-13 <guoxb> Create file.
*  2009-11-04 <guoxb> Add wsm watchdog switch
*  2009-11-12 <guoxb> Add display wtp ipv6 address
*  2009-11-25 <guoxb> Add function dbus_msg_is_method_call 
*                                and get_vrid_path for Hansi.
*  2009-12-10 <guoxb> Add pmtu show running.
*  2009-12-28 <guoxb> Review source code.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*  2010-02-09 <guoxb> Change command about ipfwd because ipfwd module changed
*                               name to ip-fast-forwarding.ko.
*  2010-03-23 <guoxb> Add command set ipfrag_ignoredf disable in show running func.
*  2010-03-25 <guoxb> Modified command set ipfrag_ignoredf.
*
******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <dbus/wcpss/wsm_dbus_def.h>
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wsm_dbus.h"
#include <sys/syslog.h>

#include "wsm_wifi_bind.h"
#include "wsm_types.h"
#include "timerlib.h"


extern unsigned char tunnel_run;
extern unsigned char dbus_wd_enable;
extern unsigned int total_sta_cnt;
extern unsigned int total_wtp_cnt;
extern WTP_Element WTPtable[HASHTABLE_SIZE];
extern unsigned char gLog_switch[LOG_TYPE_MAX];
extern unsigned int vrrid;
extern void wsm_set_logswitch(unsigned char logtype, unsigned char new_state);
extern unsigned int g_wid_wsm_error_handle_state;


static DBusConnection * wsm_dbus_conn = NULL;


/**
* Discription:
*   Get STA count in a BSS
*
* Parameter:
*   sta: sta List pointer
*
* Return:
*  STA count in this sta List
*
*/
int get_sta_cnt_in_bss(STA_Element *sta)
{
	STA_Element *p = NULL;
	int i = 0;
	
	if (sta == NULL)
		return 0;
	p = sta;
	
	while (p)
	{
		i++;
		p = p->next;
	}

	return i;
}

/**
* Discription:
*  Find BSS_Info by BSSID
*
* Paramter:
*  mac: BSSID
*
* Return:
*  Successed : BSS_info
*  Failed : NULL       
*
*/
BSS_Element* find_bss(unsigned char *mac, unsigned int *WTPID)
{
	BSSID_BSSIndex_Element *pbss = NULL;
	unsigned int wtpid = 0;
	unsigned int bss_local_idx = 0;
	
	pbss = (BSSID_BSSIndex_Element *)wsm_tbl_search(BSSID_BSSIndex_TYPE, mac,wtpid);

	if (!pbss)
	{
		return NULL;
	}

	wtpid = pbss->BSSIndex / BSS_ARRAY_SIZE;
	bss_local_idx = pbss->BSSIndex % BSS_ARRAY_SIZE;

	if (WTPtable[wtpid].flag == 0 || WTPtable[wtpid].next == NULL)
	{
		return NULL;
	}

	if (WTPtable[wtpid].next[bss_local_idx].flag == 0)
	{
		return NULL;
	}

	if (WTPID)
	{
		*WTPID = wtpid;
	}
	
	return &WTPtable[wtpid].next[bss_local_idx];
}

/**
* Descripton:
*  Enable or Disable ipfwd module.
*
*/
DBusMessage *wsm_dbus_set_ipfwd_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	unsigned int able;
	int ret = 0; 

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&able,
								DBUS_TYPE_INVALID)))
	{
		WSMLog(L_ERR, "%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			WSMLog(L_ERR, "%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	if (able) 
	{
		if (getuid()) 
			system("sudo insmod /lib/modules/2.6.16.26-Cavium-Octeon/misc/ip-fast-forwarding.ko");
		else
			system("insmod /lib/modules/2.6.16.26-Cavium-Octeon/misc/ip-fast-forwarding.ko");
	} 
	else
	{
		if (getuid())
			system("sudo rmmod ip_fast_forwarding");
		else
			system("rmmod ip_fast_forwarding");
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}

/**
* Descripton:
*  Enable or Disable wsm receive or transmit function.
*
*/
DBusMessage *wsm_dbus_set_wsm_receive_transmit_func(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	unsigned int type;
	unsigned int able;
	int ret = 0; 

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_UINT32,&able,
								DBUS_TYPE_INVALID)))
	{
		WSMLog(L_ERR, "%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			WSMLog(L_ERR, "%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	if (type) 
	{
		if(able){
			if (getuid()) 
				system("sudo echo 1 > /sys/module/wifi_ethernet/parameters/wifi_rx_switch");
			else
				system("echo 1 > /sys/module/wifi_ethernet/parameters/wifi_rx_switch");
		}else{
			if (getuid()) 
				system("sudo echo 0 > /sys/module/wifi_ethernet/parameters/wifi_rx_switch");
			else
				system("echo 0 > /sys/module/wifi_ethernet/parameters/wifi_rx_switch");
		}
	} 
	else
	{
		if(able){
			if (getuid()) 
				system("sudo echo 1 > /sys/module/wifi_ethernet/parameters/wifi_tx_switch");
			else
				system("echo 1 > /sys/module/wifi_ethernet/parameters/wifi_tx_switch");
		}else{
			if (getuid()) 
				system("sudo echo 0 > /sys/module/wifi_ethernet/parameters/wifi_tx_switch");
			else
				system("echo 0 > /sys/module/wifi_ethernet/parameters/wifi_tx_switch");
		}
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}

/**
* Description:
*  Display wsm syslog level state
*
*/
DBusMessage *wsm_dbus_show_log_wsm_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusError err;
	int ret = 0;
	int i = 0;

	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &gLog_switch[L_EMERG]);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &gLog_switch[L_ALERT]);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &gLog_switch[L_CRIT]);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &gLog_switch[L_ERR]);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &gLog_switch[L_WARNING]);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &gLog_switch[L_NOTICE]);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &gLog_switch[L_INFO]);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &gLog_switch[L_DEBUG]);

	return reply;
}

/**
* Description:
*  Set wsm syslog level
*
*/
DBusMessage *wsm_dbus_set_log_wsm_level(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	int level = 0;
	int ret = 0; 
	int i = 0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
								DBUS_TYPE_UINT32,&level,
								DBUS_TYPE_INVALID)))
	{
		WSMLog(L_WARNING, "%s: dbus_message_get args failed\n", __func__);			
		if (dbus_error_is_set(&err))
		{
			WSMLog(L_WARNING, "%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if (level >= 0)
	{
		
		for (i = 0; i < LOG_TYPE_MAX; i++)
		{
			if (i <= level)
				gLog_switch[i] = LOG_ON;
			else
				gLog_switch[i] = LOG_OFF;
		}
	}
	else
	{
		ret = -1;
		WSMLog(L_WARNING, "%s: level = %d err\n", __func__, level);
	}
		
	
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
}

/**
* Description:
*  Display all of WTPs
*
*/
DBusMessage *wsm_dbus_show_wtp_list(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage *reply;
	DBusMessageIter iter, iter_array, iter_struct;
	DBusError err;
	int ret = 0, i = 0, j = 0, n = 0, loop = 0;
	unsigned int wtp_cnt = 0, sta_cnt = 0;
	unsigned char ip_buf[8] = {0};
	
	dbus_error_init(&err);

	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
	wtp_cnt = total_wtp_cnt;
	sta_cnt = total_sta_cnt;
	
	if (!total_wtp_cnt)
		ret = -1;
	
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &wtp_cnt);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &sta_cnt);
	if (ret)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		WSMLog(L_DEBUG, "%s: Assoc WTP Number is : 0\n", __func__);
		return reply;
	}
	
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING /* WTPID */
										DBUS_TYPE_UINT16_AS_STRING /* Addr Family */
										DBUS_TYPE_BYTE_AS_STRING /* WTPIP */
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING /* sta_cnt */
										DBUS_TYPE_BYTE_AS_STRING /* bss_cnt */
										DBUS_TYPE_BYTE_AS_STRING /* WTPMAC */
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	for (i = 0; i < HASHTABLE_SIZE; i++)
	{
		if (WTPtable[i].flag == 0)
			continue;
			
		dbus_message_iter_open_container (&iter_array, DBUS_TYPE_STRUCT, NULL, &iter_struct);
		dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &i);
		dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT16, &WTPtable[i].WTP.WTPIP.addr_family);
		bzero(ip_buf, 8);
		if (WTPtable[i].WTP.WTPIP.addr_family == AF_INET)
		{
			*((unsigned int*)ip_buf) = WTPtable[i].WTP.WTPIP.m_v4addr;
		}
		else /* Just display ipv6 the first 4 bytes and the last 4 bytes */
		{
			memcpy(ip_buf, WTPtable[i].WTP.WTPIP.m_v6addr, 4);
			memcpy(&ip_buf[4], &WTPtable[i].WTP.WTPIP.m_v6addr[12], 4);
		}
		for (loop = 0; loop < 8; loop++)
		{
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &ip_buf[loop]);
		}
		
		dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &WTPtable[i].sta_cnt);
		dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &WTPtable[i].count_BSS);
		
		for (n = 0; n < MAC_LEN; n++)
		{
			dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_BYTE, &WTPtable[i].WTP.WTPMAC[n]);
		}
		
		dbus_message_iter_close_container (&iter_array, &iter_struct);
		j++;
	}
	
	WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
	
	dbus_message_iter_close_container (&iter, &iter_array);
	
	if (j != wtp_cnt)
	{
		WSMLog(L_ERR, "%s: WTP_CNT error. wtp_cnt = %u, j = %d\n", __func__, wtp_cnt, j);
	}
	
	return reply;
}

/**
* Description:
*   Display WTP by WTPID
*
*/
DBusMessage *wsm_dbus_show_wtp_by_wtpid(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter, iter_bss_array, iter_bss_struct;
	DBusError err;
	unsigned int wtpid = 0, loop = 0;
	int ret = 0, i = 0, j = 0;
	unsigned int bss_sta_cnt = 0;
	unsigned char wtpip[16] = {0};
	unsigned short addr_family = 0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
								DBUS_TYPE_UINT32, &wtpid,
								DBUS_TYPE_INVALID)))
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}

	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
	if (!WTPtable[wtpid].flag)
	{
		ret = -1;
	}
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);

	if (ret)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return reply;
	}

	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &WTPtable[wtpid].count_BSS);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &WTPtable[wtpid].sta_cnt);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT16, &WTPtable[wtpid].addr_family);
	
	if (WTPtable[wtpid].addr_family == AF_INET)
		*((unsigned int*)wtpip) = WTPtable[wtpid].WTP.WTPIP.m_v4addr;
	else
		memcpy(wtpip, WTPtable[wtpid].WTP.WTPIP.m_v6addr, 16);
	for (loop = 0; loop < 16; loop++)
	{
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &wtpip[loop]);
	}
	
	for (i = 0; i < MAC_LEN; i++)
	{
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &WTPtable[wtpid].WTP.WTPMAC[i]);
	}

	if (WTPtable[wtpid].count_BSS == 0)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return reply;
	}
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING /* sta cnt */
										DBUS_TYPE_BYTE_AS_STRING /* BSSID */
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_bss_array);

	for (i = 0; i < BSS_ARRAY_SIZE; i++)
	{
		if (WTPtable[wtpid].next[i].flag == 0)
			continue;
		
		dbus_message_iter_open_container (&iter_bss_array, DBUS_TYPE_STRUCT, NULL, &iter_bss_struct);
		bss_sta_cnt = get_sta_cnt_in_bss(WTPtable[wtpid].next[i].next);
		dbus_message_iter_append_basic(&iter_bss_struct, DBUS_TYPE_UINT32, &bss_sta_cnt);
		for (j = 0; j < MAC_LEN; j++)
		{
			dbus_message_iter_append_basic(&iter_bss_struct, DBUS_TYPE_BYTE, 
					&WTPtable[wtpid].next[i].BSS.BSSID[j]);
		}
		dbus_message_iter_close_container (&iter_bss_array, &iter_bss_struct);
	}
	
	dbus_message_iter_close_container (&iter, &iter_bss_array);
	WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
	
	return reply;					
}

/**
* Description:
*  Display BSS by BSSID
*
*/
DBusMessage *wsm_dbus_show_bss_bssid(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter, iter_sta_array, iter_sta_struct;
	DBusError err;
	BSS_Element *pbss = NULL;
	STA_Element *psta = NULL;
	unsigned char bssid[MAC_LEN] = {0};
	int ret = 0, i = 0;
	unsigned int sta_cnt = 0;
	unsigned int wtpid = 0;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
								DBUS_TYPE_BYTE, &bssid[0],
								DBUS_TYPE_BYTE, &bssid[1],
								DBUS_TYPE_BYTE, &bssid[2],
								DBUS_TYPE_BYTE, &bssid[3],
								DBUS_TYPE_BYTE, &bssid[4],
								DBUS_TYPE_BYTE, &bssid[5],
								DBUS_TYPE_INVALID)))
	{		
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}

	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
	
	pbss = find_bss(bssid, &wtpid);
	if (!pbss)
	{
		ret = -1;
	}
	
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);

	if (ret)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return reply;
	}
	
	sta_cnt = get_sta_cnt_in_bss(pbss->next);
	psta = pbss->next;
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &pbss->BSS.BSSIndex);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &pbss->BSS.Radio_G_ID);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &wtpid);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &pbss->BSS.WlanID);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &sta_cnt);
	
	if (!sta_cnt)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return reply;					
	}

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING /* STAMAC */
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
										DBUS_TYPE_BYTE_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_sta_array);

	while (psta)
	{
		dbus_message_iter_open_container (&iter_sta_array, DBUS_TYPE_STRUCT, NULL, &iter_sta_struct);
		for (i = 0; i < MAC_LEN; i++)
		{	
			dbus_message_iter_append_basic(&iter_sta_struct, DBUS_TYPE_BYTE, &psta->STA.STAMAC[i]);
		}
		dbus_message_iter_close_container (&iter_sta_array, &iter_sta_struct);
		psta = psta->next;
	}

	dbus_message_iter_close_container (&iter, &iter_sta_array);
	
	WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
	
	return reply;
}

/**
* Description:
*   Display STA by STAMAC
*
*/
DBusMessage *wsm_dbus_show_sta_stamac(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter, iter_sta_array, iter_sta_struct;
	DBusError err;
	struct wsm_stamac_bssid_wtpip *psta = NULL;
	BSS_Element *pbss = NULL;
	unsigned char mac[MAC_LEN] = {0};
	unsigned char bssid[MAC_LEN] = {0};
	unsigned char wlanid = 0;
	unsigned int wtpid = 0;
	int ret = 0, i = 0;
	unsigned int bss_local_idx = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args (msg, &err,
							DBUS_TYPE_BYTE, &mac[0],
							DBUS_TYPE_BYTE, &mac[1],
							DBUS_TYPE_BYTE, &mac[2],
							DBUS_TYPE_BYTE, &mac[3],
							DBUS_TYPE_BYTE, &mac[4],
							DBUS_TYPE_BYTE, &mac[5],
							DBUS_TYPE_INVALID)))
	{
		WSMLog(L_WARNING, "%s: dbus_message_get args failed\n", __func__);			
		if (dbus_error_is_set(&err))
		{
			WSMLog(L_WARNING, "%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	WSM_RDLOCK_LOCK(&wsm_tables_rwlock);
	psta = wsm_wifi_table_search(mac, WSM_STAMAC_BSSID_WTPIP_TYPE);
	if (psta)
	{
		pbss = find_bss(psta->BSSID, &wtpid);
	}

	if (!psta || !pbss)
	{
		ret = -1;
	}
	
	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);

	if (ret)
	{
		WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
		return reply;
	}

	for (i = 0; i < MAC_LEN; i++)
	{	
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &psta->BSSID[i]);	
	}
	
	wlanid = pbss->BSS.WlanID;
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &wlanid);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &wtpid);

	WSM_RWLOCK_UNLOCK(&wsm_tables_rwlock);
	
	return reply;
	
}


/**
* Description:
*  Enable or Disable wsm watch dog
*
*/
DBusMessage *wsm_dbus_set_wsm_watchdog_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter iter;
	DBusError err;
	unsigned char able;
	int ret = 0; 

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_BYTE, &able,
								DBUS_TYPE_INVALID)))
	{
		WSMLog(L_ERR, "%s: dbus_message_get args failed\n", __func__);		
		if (dbus_error_is_set(&err))
		{
			WSMLog(L_ERR, "%s: %s raised %s\n", __func__, err.name, err.message);
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	
	dbus_wd_enable = able;
	WSMLog(L_DEBUG, "%s: Dbus set dbus_wd_enable = %d\n", __func__, dbus_wd_enable);
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
	
}


/**
* Description:
*  Display wsm watchdog state
*
*/
DBusMessage *wsm_dbus_show_wsm_watchdog_state(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusError err;
	int ret = 0;
	int i = 0;

	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &tunnel_run);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_BYTE, &dbus_wd_enable);

	return reply;
}


/**
* Description:
*  CAPWAP tunnel dbus node, show-running function.
*
*/
DBusMessage *wsm_dbus_capwap_tunnel_show_running(DBusConnection *conn, 
			DBusMessage *msg,
			void *user_data)
{
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusError err;
	unsigned char *cmd = NULL;
	int rval = 0, stat = 0;
	FILE *stream1 = NULL, *stream2 = NULL, *stream3 = NULL,*stream4 = NULL,*stream5 = NULL,*stream6=NULL,*stream7=NULL;
	int rstat1 = 0, rstat2 = 0, rstat3 = 0, rstat4 = 0, rstat5 = 0, rstat6 = 0, rstat7 = 0;
	int len = 0, total_len = 0;
	unsigned char retbuf1[64] = {0};
	unsigned char retbuf2[64] = {0};
	unsigned char retbuf3[64] = {0};
	unsigned char retbuf4[64] = {0};
	unsigned char retbuf5[64] = {0};
	unsigned char retbuf6[64] = {0};
	unsigned char retbuf7[64] = {0};
	dbus_error_init(&err);
	cmd = (unsigned char*)malloc(1024);
	if (!cmd)
	{
		WSMLog(L_CRIT, "%s: malloc failed.\n", __func__);
		goto out;	
	}
	bzero(cmd, 1024);
	
	rval = system("test -d /sys/module/ip_fast_forwarding");
	stat = WEXITSTATUS(rval);
	/* ipfwd has been loaded */
	if (!stat)
	{
		len = sprintf(cmd, "config capwap-tunnel\nset flow-based-forwarding enable\nexit\n");
		total_len = len;
	}

	/* Set pmtu enable or disable */
	stream1 = popen("cat /proc/sys/net/ipv4/ipfrag_ingress_pmtu", "r");
	stream2 = popen("cat /proc/sys/net/ipv4/ipfrag_inform_nhmtu_instead", "r");
	stream3 = popen("cat /proc/sys/net/ipv4/ipfrag_ignoredf", "r");
	stream4 = popen("cat /sys/module/ip_fast_forwarding/parameters/qos_enable", "r");
	stream5 = popen("cat /sys/module/wifi_ethernet/parameters/wifi_QoS_open", "r");
	stream6 = popen("cat /sys/module/wifi_ethernet/parameters/wifi_rx_switch", "r");
	stream7 = popen("cat /sys/module/wifi_ethernet/parameters/wifi_tx_switch", "r");
	if (stream1 != NULL)
	{
		fread(retbuf1, sizeof(char), sizeof(retbuf1), stream1);
		pclose(stream1);
		rstat1 = atoi(retbuf1);
	}
	if (stream2 != NULL)
	{
		fread(retbuf2, sizeof(char), sizeof(retbuf2), stream2);
		pclose(stream2);
		rstat2 = atoi(retbuf2);
	}
	if (stream3 != NULL)
	{
		fread(retbuf3, sizeof(char), sizeof(retbuf3), stream3);
		pclose(stream3);
		rstat3 = atoi(retbuf3);
	}
	if (stream4 != NULL)
	{
		fread(retbuf4, sizeof(char), sizeof(retbuf4), stream4);
		pclose(stream4);
		rstat4 = atoi(retbuf4);
	}
	if (stream5 != NULL)
	{
		fread(retbuf5, sizeof(char), sizeof(retbuf5), stream5);
		pclose(stream5);
		rstat5= atoi(retbuf5);
	}
	if (stream6 != NULL)
	{
		fread(retbuf6, sizeof(char), sizeof(retbuf6), stream6);
		pclose(stream6);
		rstat6= atoi(retbuf6);
	}
	if (stream7 != NULL)
	{
		fread(retbuf7, sizeof(char), sizeof(retbuf7), stream7);
		pclose(stream7);
		rstat7= atoi(retbuf7);
	}
	if (rstat1 == 1)
	{
		len = sprintf(cmd + total_len, "config capwap-tunnel\nset ipfrag_ingress_pmtu enable\nexit\n");
		total_len += len;
	}
	if (rstat2 == 1)
	{
		len = sprintf(cmd + total_len, "config capwap-tunnel\nset ipfrag_inform_nhmtu_instead enable\nexit\n");
		total_len += len;
	}
	if (rstat3 == 0)
	{
		len = sprintf(cmd + total_len, "config capwap-tunnel\nset ipfrag_ignoredf enable\nexit\n");
		total_len += len;
	}
	else
	{
		len = sprintf(cmd + total_len, "config capwap-tunnel\nset ipfrag_ignoredf disable\nexit\n");
		total_len += len;
	}
	if (rstat5== 1)
	{
		len = sprintf(cmd + total_len, "config capwap-tunnel\nset wifi_qos enable\nexit\n");
		total_len += len;
	}
	if (rstat4 == 1)
	{
		len = sprintf(cmd + total_len, "config capwap-tunnel\nset fast_forwarding_qos enable\nexit\n");
		total_len += len;
	}
	if (rstat6 == 1)
	{
		len = sprintf(cmd + total_len, "config capwap-tunnel\nset wsm receive enable\nexit\n");
		total_len += len;
	}
	if (rstat7 == 1)
	{
		len = sprintf(cmd + total_len, "config capwap-tunnel\nset wsm transmit enable\nexit\n");
		total_len += len;
	}
	if(g_wid_wsm_error_handle_state != WID_WSM_ERROR_HANDLE_STATE_DEFAULT){
			  len = sprintf(cmd + total_len,"set wsm error handle %s\n", g_wid_wsm_error_handle_state ? "enable":"disable");
			  total_len += len;

	   }	

	
out:	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &cmd);
	
	free(cmd);
	return reply;
}

/*zhaoruijia,20101103,for wid wsm error handle control,start*/
DBusMessage * wsm_dbus_set_wsm_error_handle_state(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int wid_wsm_error_handle_state = 0;

	dbus_error_init(&err);
	int ret = 0;

	if (!(dbus_message_get_args ( msg, &err,
								DBUS_TYPE_UINT32,&wid_wsm_error_handle_state,
								DBUS_TYPE_INVALID))){

		printf("Unable to get input args\n");
				
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
   }

  
   
	if(wid_wsm_error_handle_state == 1){
           g_wid_wsm_error_handle_state  = 1;
		}
	else if(wid_wsm_error_handle_state == 0){
           g_wid_wsm_error_handle_state  = 0;
		}
	else{
          g_wid_wsm_error_handle_state  = 0;
		  ret = 1;
	}

   WSMLog(L_DEBUG,"Wsm Error handle state %d\n",g_wid_wsm_error_handle_state);
  
    reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;	

}


DBusMessage * wsm_dbus_checking_wsm(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;

	dbus_error_init(&err);
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    WSMLog(L_DEBUG,"%s,%d.\n",__func__,__LINE__);
	return reply;	

}

/* book add for wsm deamon quit, 2011-5-23 */
DBusMessage * wsm_dbus_method_quit(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
    
	dbus_error_init(&err);
    reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    WSMLog(L_INFO,"ret = %d    %s,%d.\n",ret, __func__,__LINE__);
    
    exit(2);
    
	return reply;	

}


/*zhaoruijia,20100916,add ap auto update service tftp,end*/


/*
* Description:
*  Re-format path string based on vrid.
*
* Parameter:
*  path: new string buffer
*  base: old string pointer
*
* Return:
*  NULL: Failed.
*  path: Successed.  
*
*/
unsigned char *get_vrid_path(unsigned char *path, unsigned char *base)
{
	if (path == NULL || base == NULL)
	{
		WSMLog(L_ERR, "%s: Parameter Dbus path == NULL.\n", __func__);
		return NULL;
	}
#ifndef _DISTRIBUTION_	
	sprintf (path, "%s%d", base, vrrid);
#else
	sprintf (path, "%s%d_%d", base,local,vrrid);
#endif
	return path;
}



/*
* Description:
*  Re-format Path string for Hansi N + 1
*
* Parameter:
*  same to function dbus_message_is_method_call
*
* Return:
*  0: Failed.
*  nonzero: Successed, see return value of dbus_message_is_method_call for detail.
*
*/
unsigned int dbus_msg_is_method_call(DBusMessage *msg, unsigned char *dbus_if, unsigned char *path)
{
	unsigned char if_path[128] = {0};
	unsigned char cmd_path[128] = {0};

	if (get_vrid_path(if_path, dbus_if) == NULL || get_vrid_path(cmd_path, path) == NULL)
	{
		WSMLog(L_ERR, "%s: get_vrid_path failed\n", __func__);
		return 0;
	}

	return dbus_message_is_method_call(msg, if_path, cmd_path);
}

/**
* Description:
*  WSM module Dbus handler 
* 
*/
static DBusHandlerResult wsm_dbus_msg_handler (
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data)
{
	DBusMessage *reply = NULL;
	unsigned char wsm_dbus_vrid_path[WSM_PATH_MAX] = {0};
#ifndef _DISTRIBUTION_	
	sprintf(wsm_dbus_vrid_path, "%s%d", WSM_DBUS_OBJPATH, vrrid);
#else
	sprintf(wsm_dbus_vrid_path, "%s%d_%d", WSM_DBUS_OBJPATH,local, vrrid);
#endif
	if (strcmp(dbus_message_get_path(msg), wsm_dbus_vrid_path) != 0)
	{
		WSMLog(L_CRIT, "%s: Dbus Path match error.\n", __func__);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	WSMLog(L_INFO,"message path %s\n",dbus_message_get_path(msg));
	WSMLog(L_INFO,"message interface %s\n",dbus_message_get_interface(msg));
	WSMLog(L_INFO,"message member %s\n",dbus_message_get_member(msg));
	WSMLog(L_INFO,"message destination %s\n",dbus_message_get_destination(msg));	
	WSMLog(L_INFO,"message type %d\n",dbus_message_get_type(msg));
	if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SET_TUNNEL))
		reply = wsm_dbus_set_ipfwd_state(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_WTP_LIST))
		reply = wsm_dbus_show_wtp_list(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_LOG_WSM_STATE))
		reply = wsm_dbus_show_log_wsm_state(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SET_LOG_WSM_LEVEL))
		reply = wsm_dbus_set_log_wsm_level(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_WTP_WTPID))
		reply = wsm_dbus_show_wtp_by_wtpid(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_BSS_BSSID))
		reply = wsm_dbus_show_bss_bssid(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_STA_STAMAC))
		reply = wsm_dbus_show_sta_stamac(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SET_WSM_WATCHDOG_STATE))
		reply = wsm_dbus_set_wsm_watchdog_state(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SHOW_WSM_WATCH_DOG_STATE))
		reply = wsm_dbus_show_wsm_watchdog_state(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CAPWAP_TUNNEL_SHOW_RUNNING))
		reply = wsm_dbus_capwap_tunnel_show_running(conn, msg, user_data);
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_METHOD_SET_WSM_ERROR_HANDLE_STATE)){
        reply = wsm_dbus_set_wsm_error_handle_state(conn,msg,user_data);
	}else if(dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_METHOD_CHECKING)){
		reply = wsm_dbus_checking_wsm(conn,msg,user_data);
	}
	else if(dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_METHOD_QUIT)){
		reply = wsm_dbus_method_quit(conn,msg,user_data);
	}
	else if (dbus_msg_is_method_call(msg, WSM_DBUS_INTERFACE, WSM_DBUS_CONF_SET_TUNNEL_WSM_RX_TX))
		reply = wsm_dbus_set_wsm_receive_transmit_func(conn, msg, user_data);

	if (reply)
	{
		dbus_connection_send (conn, reply, NULL);
		dbus_connection_flush(conn);
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED ;
}

/**
* Description:
*   WSM dbus filter function.
*
*
*/
DBusHandlerResult wsm_dbus_filter_function(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	if (dbus_message_is_signal(msg, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		strcmp(dbus_message_get_path(msg), DBUS_PATH_LOCAL) == 0 )
	{
		WSMLog(L_CRIT, "%s: Got disconnected from the system message bus.\n",
				__func__);
		dbus_connection_close (wsm_dbus_conn);
		wsm_dbus_conn = NULL;
	}
	else if (dbus_message_is_signal(msg, DBUS_INTERFACE_DBUS, "NameOwnerChanged"))
	{
	}
	else
	{
		return 1;
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}


/**
* Description:
*   WSM Debus initialize function
*
* Parameter:
*   NULL
*
* Return:
*   0: Successed; -1: failed
*
*/
int wsm_dbus_init(void)
{
	DBusError err;
	DBusObjectPathVTable wsm_vtbl = {NULL, &wsm_dbus_msg_handler, NULL, NULL, NULL, NULL};
	unsigned char dbus_name[WSM_PATH_MAX] = {0};
	unsigned char obj_path[WSM_PATH_MAX] = {0};

	sprintf(dbus_name, "%s%d_%d", WSM_DBUS_BUSNAME,local, vrrid);
	sprintf(obj_path, "%s%d_%d", WSM_DBUS_OBJPATH,local, vrrid);

	WSMLog(L_INFO, "%s: dbus name : %s\n", __func__, dbus_name);
	WSMLog(L_INFO, "%s: obj name : %s\n", __func__, obj_path);
	
	dbus_threads_init_default();
	dbus_connection_set_change_sigpipe (1);
	dbus_error_init (&err);
	wsm_dbus_conn = dbus_bus_get_private (DBUS_BUS_SYSTEM, &err);

	if (!wsm_dbus_conn)
	{
		WSMLog(L_CRIT, "%s: wsm_dbus_conn = NULL.\n", __func__);
		return -1;
	}

	if (!dbus_connection_register_fallback (wsm_dbus_conn, obj_path, &wsm_vtbl, NULL)) 
	{
		WSMLog(L_CRIT, "%s: register fallback failed.\n", __func__);
		return -1;
	}
	
	dbus_bus_request_name (wsm_dbus_conn, dbus_name, 0, &err);

	if (dbus_error_is_set (&err))
	{
		WSMLog(L_CRIT, "%s: dbus_bus_request_name() ERR:%s\n", __func__,
				err.message);
		return -1;
	}
	
	dbus_connection_add_filter (wsm_dbus_conn, wsm_dbus_filter_function, NULL, NULL);
	dbus_bus_add_match (wsm_dbus_conn,
				"type='signal'"
				",interface='"DBUS_INTERFACE_DBUS"'"
				",sender='"DBUS_SERVICE_DBUS"'"
				",member='NameOwnerChanged'",
				NULL);

	return 0;
}

/**
* Description:
*   WSM DBus thread function.
*
* Parameter:
*   NULL
*
* Return:
*   Void
*
*/
void wsm_dbus_main(void)
{
	if (wsm_dbus_init())
	{
		WSMLog(L_CRIT, "%s: Initialize failed.\n", __func__);
		exit(-1);
	}
	
	while (1)
	{
		if (!dbus_connection_read_write_dispatch(wsm_dbus_conn, DBUS_TIMEOUT))
		{
			break;
		}
	}
	
	WSMLog(L_CRIT, "%s: Out of main loop, Critical ERROR.\n", __func__);
}

