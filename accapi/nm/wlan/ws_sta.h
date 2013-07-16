/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


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
********************************************************************************
* ws_sta.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#ifndef _WS_STA_H
#define _WS_STA_H

#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"
#include "ws_dcli_acl.h"




#define MAC2STRZ(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

#define OKB 1024
#define OMB (1024*1024)
#define OGB (1024*1024*1024)



struct station_profile
{
  unsigned char mac[6];
  unsigned char in_addr[16];
  unsigned char ieee80211_state[20];
  unsigned char PAE[20];
  unsigned char BACKEND[20];    
  char StaTime[30];
  unsigned long long rxPackets;
  unsigned long long txPackets;
  double  flux;
  time_t online_time;
  int hour;
  int min;
  int sec;
  unsigned int sta_traffic_limit;
  unsigned int sta_send_traffic_limit;
  unsigned int vlan_id;
  struct station_profile *next;
};


struct extend_sta_info
{
	unsigned long long rx_bytes;
	unsigned long long tx_bytes;
	unsigned char mac[6];
	unsigned char mode;
	unsigned char channel;
	unsigned char rssi;
	unsigned short nRate; 
	unsigned char isPowerSave;
	unsigned char isQos;
	unsigned char in_addr[20];
	unsigned int snr;
	unsigned long long rr;
	unsigned long long tr;
	unsigned long long tp;
	double flux_rr;
	double flux_tr;
	double flux_tp;
	unsigned long long rx_pkts;
	unsigned long long tx_pkts;
	double rtx;
	time_t online_t;
	char  StaccTime[60];
	unsigned long long rtx_pkts;
	unsigned long long err_pkts;
	struct extend_sta_info *next;
};

struct extend_sta_profile
{
	unsigned int deny_num;
	unsigned int bss_num;
	unsigned int sta_num;
	unsigned int acc_tms;
	unsigned int auth_tms;
	unsigned int repauth_tms;
	unsigned int local_roam_count;
	unsigned int total_unconnect_count;
	struct extend_sta_info head;
};

struct wapi_protocol_info_profile
{
	unsigned int ConfigVersion;
	unsigned char WapiSupported;
	unsigned char WapiPreAuth;
	unsigned char WapiPreauthEnabled;
	unsigned char UnicastCipherEnabled;
	unsigned char AuthenticationSuiteEnabled;
	unsigned char MulticastRekeyStrict;
	unsigned char UnicastKeysSupported;
	unsigned int BKLifetime;
	unsigned int BKReauthThreshold;
	unsigned int SATimeout;
	unsigned char UnicastCipherSelected[4];
	unsigned char MulticastCipherSelected[4]; 
	unsigned char UnicastCipherRequested[4];
	unsigned char MulticastCipherRequested[4];
	unsigned int MulticastCipherSize;
	unsigned char MulticastCipher[4];
	unsigned int UnicastCipherSize;
	unsigned char UnicastCipher[4];
};

struct mac_list_profile
{
	unsigned char mac[6];
	struct mac_list_profile *next;
};
struct ac_mac_list_profile
{
	unsigned int maclist_acl;
	unsigned int black_mac_num;
	unsigned int white_mac_num;
	struct mac_list_profile *black_mac_list;
	struct mac_list_profile *white_mac_list;
};

extern 	void Free_bss_bymac(struct dcli_sta_info *sta);
/*只要调用函数，就调用Free_bss_bymac()释放空间*/
extern  int show_sta_bymac(dbus_parameter parameter, DBusConnection *connection,char *arg_mac,struct dcli_sta_info **sta); /*返回0表示失败，返回1表示成功，返回-1表示station does not exist,返回-2表示error*/
																																/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern  int kick_sta_MAC(dbus_parameter parameter, DBusConnection *connection,char * mac_addr);
																	/*返回0表示失败，返回1表示成功，返回-1表示station does not exist*/
																	/*返回-2表示Unknown mac addr format*/
																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_sta_summary(struct dcli_ac_info *ac);
/*返回1时，调用Free_sta_summary()释放空间*/
/*sort_type为可选参数，参数范围是"ascend"或"descend"，默认为"descend"*/
extern int show_sta_summary(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac,char *sort_type);
																												 /*返回0表示失败，返回1表示成功，返回-1表示error*/
																												 /*返回-2表示input patameter should only be 'ascend' or 'descend'*/
																												 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1时，调用Free_sta_summary()释放空间*/
extern  int show_station_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1时，调用Free_sta_summary()释放空间*/
extern 	int show_station_list_by_group(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac);/*失败返回0，成功返回1，返回-1表示error*/
																											   			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_sta_bywlanid(struct dcli_wlan_info *wlan);
/*返回1时，调用Free_sta_bywlanid()释放空间*/
extern	int show_sta_bywlanid(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_wlan_info **wlan);
																						/*返回0表示失败，返回1表示成功*/
																						/*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																						/*返回-2表示wlan does not exist，返回-3表示error*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						
extern void Free_sta_bywtpid(struct dcli_wtp_info *wtp);
/*返回1时，调用Free_sta_bywtpid()释放空间*/
extern	int show_sta_bywtpid(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_wtp_info **wtp);
																					 /*返回0表示失败，返回1表示成功*/
																					 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																					 /*返回-2表示wtp does not provide service or it maybe does not exist*/
																					 /*返回-3表示error*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					 
/*stat为"black"或"white"*/
extern int wlan_add_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat,char *arg_mac);
																					  /*返回0表示失败，返回1表示成功*/
																					  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																					  /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																					  /*返回-3表示Unknown mac addr format*/
																					  /*返回-4表示wlan isn't existed，返回-5表示mac add already*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*stat为"black"或"white"*/																					 
extern int wlan_delete_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat,char *arg_mac);
																					  	/*返回0表示失败，返回1表示成功*/
																						/*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																						/*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																						/*返回-3表示Unknown mac addr format*/
																						/*返回-4表示wlan isn't existed，返回-5表示mac is not in the list*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*stat为"none"/"black"/"white"*/
extern int wlan_use_none_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat);
																			  /*返回0表示失败，返回1表示成功*/
																			  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																			  /*返回-2表示input patameter should only be 'black/white/none' or 'b/w/n'*/
																			  /*返回-3表示wlan isn't existed，返回-4表示wids has open,can't be set other except blank list*/
																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*stat为"black"或"white"*/																	  
extern int wtp_add_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat,char *arg_mac);
																			  		 /*返回0表示失败，返回1表示成功*/
																					 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																					 /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																					 /*返回-3表示Unknown mac addr format*/
																					 /*返回-4表示wtp is not existed，返回-5表示mac add already*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*stat为"black"或"white"*/																			 
extern int wtp_delete_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat,char *arg_mac);
																					   /*返回0表示失败，返回1表示成功*/
																					   /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																					   /*返回-2返回input patameter should only be 'black/white' or 'b/w'*/
																					   /*返回-3表示Unknown mac addr format，返回-4表示wtp is not existed*/
																					   /*返回-5表示mac is not in the list*/
																					   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*stat为"none"/"black"/"white"*/																				 
extern int wtp_use_none_black_white(dbus_parameter parameter, DBusConnection *connection,int id,char *stat); 
																			  /*返回0表示失败，返回1表示成功*/
																			  /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																			  /*返回-2表示input patameter should only be 'black/white' or 'b/w'*/
																			  /*返回-3表示wtp isn't existed*/
																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*stat为"black"或"white"*/																	  
extern int radio_bss_add_black_white(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlanID,char *stat,char *arg_mac); 
																			  							 /*返回0表示失败，返回1表示成功*/
																										 /*返回-1表示radio id should be 1 to G_RADIO_NUM-1*/
																										 /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																										 /*返回-3表示Unknown mac addr format*/
																										 /*返回-4表示bss is not exist，返回-5表示mac add already*/
																										 /*返回-6表示unknown id format，返回-7表示wlan is not exist*/
																										 /*返回-8表示radio has not apply wlan，返回-9表示radio id is not exist*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*stat为"black"或"white"*/																							 
extern int radio_bss_delete_black_white(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlanID,char *stat,char *arg_mac); 
																										    /*返回0表示失败，返回1表示成功*/
																											/*返回-1表示radio id should be 1 to G_RADIO_NUM-1*/
																											/*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																											/*返回-3表示Unknown mac addr format*/
																											/*返回-4表示bss is not exist，返回-5表示mac is not in the list*/
																											/*返回-6表示unknown id format，返回-7表示wlan is not exist*/
																											/*返回-8表示radio has not apply wlan，返回-9表示radio id is not exist*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*stat为"none"/"black"/"white"*/																								
extern int radio_bss_use_none_black_white(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlanID,char *stat); 
																							  /*返回0表示失败，返回1表示成功*/
																							  /*返回-1表示radio id should be 1 to G_RADIO_NUM-1*/
																							  /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																							  /*返回-3表示bss is not exist*/
																							  /*返回-4表示unknown id format，返回-5表示wlan is not exist*/
																							  /*返回-6表示radio has not apply wlan，返回-7表示mac add already*/
																						 	  /*返回-8表示radio id is not exist*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					  

/*返回1时，调用Free_sta_bywlanid()释放函数*/
extern int show_wlan_mac_list(dbus_parameter parameter, DBusConnection *connection,char* id,struct dcli_wlan_info **wlan);
																						  /*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示unknown id format*/
																						  /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																						  /*返回-3表示wlan isn't existed，返回-4表示error*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1时，调用Free_sta_bywtpid()释放空间*/
extern int show_wtp_mac_list(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_wtp_info **wtp);
																					 /*返回0表示失败，返回1表示成功*/
																					 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																					 /*返回-2表示wtp isn't existed，返回-3表示error*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_mac_head(struct dcli_bss_info *bss);
/*返回1时，调用Free_mac_head() 释放空间*/																			 
extern int show_radio_bss_mac_list(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlanID,struct dcli_bss_info **bss); 
																										  /*返回0表示失败，返回1表示成功*/
																										  /*返回-1表示radio id should be 1 to G_RADIO_NUM-1*/
																										  /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																										  /*返回-3表示bss isn't existed，返回-4表示error*/
																										  /*返回-5表示unknown id format*/
																										  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1时，调用Free_sta_summary()释放空间*/																							 
extern int show_all_wlan_mac_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac);/*返回0表示失败，返回1表示成功，返回-1表示wlan isn't existed，返回-2表示error*/
																										  			 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1时，调用Free_sta_summary()释放空间*/																									 
extern int show_all_wtp_mac_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac);
																				/*返回0表示失败，返回1表示成功，返回-1表示wtp isn't existed*/
																				/*返回-2表示input wtp id should be 1 to WTP_NUM-1，返回-3表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1时，调用Free_sta_summary()释放空间*/																		
extern int show_all_bss_mac_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_ac_info **ac);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																				
extern int extend_show_sta_mac(dbus_parameter parameter, DBusConnection *connection,char *MAC,struct extend_sta_info *sta_info);  /*返回0表示失败，返回1表示成功，返回-1表示station does not exist*/
																																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*只要调用函数，就调用Free_sta_bywtpid()释放空间*/
extern int extend_show_sta_bywtpid(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_wtp_info **wtp);
																							 /*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																							 /*返回-2表示WTP does not provide service*/
																							 /*返回-3表示wtp does not provide service or it maybe does not exist*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern struct extend_sta_info *get_extend_sta_by_mac( struct extend_sta_profile *psta_profile, char mac[6]  );

/*只要调用函数，就调用Free_sta_bywtpid(struct dcli_wtp_info *wtp)释放空间*/
extern int show_bss_info_cmd(dbus_parameter parameter, DBusConnection *connection,char *WTPID,struct dcli_wtp_info **wtp);
																						  /*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																						  /*返回-2表示wtp does not provide service or it maybe does not exist*/
																						  /*返回-3表示error，返回-4表示unknown id format*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*只要调用函数，就调用Free_sta_bywtpid(struct dcli_wtp_info *wtp)释放空间*/																				 
extern int show_radio_info_cmd(dbus_parameter parameter, DBusConnection *connection,int WTPID,struct dcli_wtp_info **wtp);
																						  /*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																						  /*返回-2表示wtp does not provide service or it maybe does not exist*/
																						  /*返回-3表示error*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																				  
extern void Free_channel_access_time_head( struct dcli_channel_info *channel);
/*只要调用函数，就调用Free_channel_access_time_head()释放空间*/
extern int show_channel_access_time_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_channel_info **channel);
																				/*返回0表示失败，返回1表示成功，返回-1表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_mib_info_head( struct dcli_radio_info *radio);
/*只要调用函数，就调用Free_mib_info_head()释放空间*/
extern int show_mib_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,int Rid,struct dcli_radio_info **radio);
																								 /*返回0表示失败，返回1表示成功，返回-1表示radio id invalid*/
																								 /*返回-2表示radio does not provide service or it maybe does not exist，返回-3表示error*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1时，调用Free_sta_bywlanid()释放空间*/																						 
extern int show_wlan_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,int WLANID,struct dcli_wlan_info **wlan);
																								   /*返回0表示失败，返回1表示成功，返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																								   /*返回-2表示wlan does not provide service or it maybe does not exist，返回-3表示error*/
																								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						   
/*只要调用函数，就调用Free_sta_bywtpid()释放空间*/
extern int show_wapi_mib_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *wtp_id,struct dcli_wtp_info **wtp);
																								   		/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示wtpid id invalid*/
																										/*返回-3表示wtp does not provide service or it maybe does not exist，返回-4表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								
extern int show_wapi_protocol_info_cmd_func(struct wapi_protocol_info_profile *wapi_protocol_info);
																										
/*只要调用函数，就调用Free_sta_bywtpid()释放空间*/
extern int show_wapi_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID,struct dcli_wtp_info **wtp); 
																								   /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																								   /*返回-2表示wtp id should be 1 to WTP_NUM-1，返回-3表示WTP does not provide service*/
																								   /*返回-4表示wtp does not provide service or it maybe does not exist，返回-5表示error*/
																								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state的范围是"enable"或"disable"*/																						   
extern int set_ac_flow_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WlanID,char *state);
																			   /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																			   /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																			   /*返回-3表示input patameter only with 'enable' or 'disable'，返回-4表示wlan isn't existed*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			   
/*返回1时，调用Free_sta_summary(释放空间*/
extern int show_traffic_limit_info_rd_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *RID,struct dcli_ac_info **ac);
																			   							/*返回0表示失败，返回1表示成功*/
																										/*返回-1表示unknown id format，返回-2表示radio id invalid*/
																										/*返回-3表示radio does not provide service or it maybe does not exist*/
																										/*返回-4表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								
/*当返回1时，调用Free_traffic_limit_info()释放空间*/
extern int show_traffic_limit_info_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *BssIndex,struct dcli_bss_info **bss);
																											/*返回0表示失败，返回1表示成功，返回-1表示unknown bssindex format*/
																											/*返回-2表示bssindex invalid，返回-3表示bssindex BssIndex does not provide service or it maybe does not exist*/
																										    /*返回-4表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1时，调用Free_sta_bywlanid()释放空间*/
extern int show_wlan_wids_MAC_list_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WLAN_ID,struct dcli_wlan_info  **wlan);
																												  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																												  /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示wlan isn't existed*/
																												  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_wlan_extern_balance_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WlanID,char *state);
																						      /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																							  /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																							  /*返回-3表示input patameter should only be 'enable' or 'disable'*/
																							  /*返回-4表示wlan isn't existed*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																							  
extern void asd_state_check(unsigned char *ieee80211_state, unsigned int sta_flags, unsigned char *PAE, unsigned int pae_state, unsigned char *BACKEND, unsigned int backend_state);

extern void Free_sta_static_head(struct sta_static_info *tab);
/*返回1时，调用Free_sta_static_head( )释放空间*/
extern int show_all_sta_static_info_cmd(dbus_parameter parameter, DBusConnection *connection,struct sta_static_info **tab);/*返回0表示失败，返回1表示成功，返回-1表示there is no static sta，返回-2表示station does not exist，返回-3表示error*/
																																/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*Type为(vlanid | limit | send_limit)*/
/*当Type为vlanid时，Value的范围是0-4095*/
/*wlanID为可选参数，不配置wlanID时，请输入NULL*/
extern int set_sta_static_info_traffic_limit_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC,char *Type,char *Value,char *wlanID);
																									 /*返回0表示失败，返回1表示成功*/
																								     /*返回-1表示input parameter should only be 'vlanid' or 'limit' or 'send_limit'*/
																								     /*返回-2表示vlan id should be 0 to 4095，返回-3表示wtp isn't existed*/
																								     /*返回-4表示radio isn't existed*/
																									 /*返回-5表示station traffic limit send value is more than bss traffic limit send value*/
																									 /*返回-6表示radio doesn't bing wlan，返回-7表示wlan isn't existed*/
																									 /*返回-8表示check sta set invalid value，返回-9表示error*/
																									 /*返回-10表示wlanid is not legal，返回-11表示wlanid should be 1 to WLAN_NUM-1*/
																									 /*返回-12表示wtp doesn't bing wlan，返回-13表示input wlanid is not sta accessed wlan*/
																									 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*wlanID为可选参数，不配置wlanID时，请输入NULL*/
extern int del_sta_static_info_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC,char *wlanID);
																 /*返回0表示失败，返回1表示成功*/
																 /*返回-1表示station does not exist，返回-2表示error*/
																 /*返回-3表示wlanid is not legal，返回-4表示wlanid should be 0 to WLAN_NUM-1*/
																 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/* 返回1时，调用Free_sta_static_head( )释放空间*/
/*wlanID为可选参数，不配置wlanID时，请输入NULL*/
extern int show_sta_mac_static_info_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC,struct sta_static_info **sta,char *wlanID);
																 									   /*返回0表示失败，返回1表示成功*/
																									   /*返回-1表示station does not exist，返回-2表示error*/
																									   /*返回-3表示wlanid is not legal，返回-4表示wlanid should be 1 to WLAN_NUM-1*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*type为(add|del)*/
extern int sta_arp_set_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *IP,char *MAC,char *if_name);
																						   /*返回0表示失败，返回1表示成功*/
																						   /*返回-1表示Unkown Command*/
																						   /*返回-2表示unknown ip format*/
																						   /*返回-3表示Unknow mac addr format*/
																						   /*返回-4表示set sta arp failed*/
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*type为"black"或"white"*/
extern int ac_add_MAC_list_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *MAC);
																		  /*返回0表示失败，返回1表示成功*/
																		  /*返回-1表示input patameter should only be 'black' or 'white'*/
																		  /*返回-2表示Unknown mac addr format，返回-3表示error*/
																		  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*type为"black"或"white"*/
extern int ac_del_MAC_list_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *MAC);
																		 /*返回0表示失败，返回1表示成功*/
																		 /*返回-1表示input patameter should only be 'black' or 'white'*/
																		 /*返回-2表示Unknown mac addr format，返回-3表示error*/
																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																		 /*返回-4表示mac is not in the list*/

/*type为"none"、"black"或"white"*/
extern int ac_use_MAC_list_cmd(dbus_parameter parameter, DBusConnection *connection,char *type);
																 /*返回0表示失败，返回1表示成功*/
																 /*返回-1表示input patameter should only be 'none','black' or 'white'*/
																 /*返回-2表示Wids is enable,ac can only use black list!，返回-3表示error*/
																 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ac_mac_head(struct ac_mac_list_profile *info);
/*返回1时，调用Free_ac_mac_head()释放空间*/
extern int show_ac_MAC_list_cmd(dbus_parameter parameter, DBusConnection *connection,struct ac_mac_list_profile *info);/*返回0表示失败，返回1表示成功*/
																															  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_wlan_sta_of_all_cmd(struct dcli_wlan_info *wlan);
/*返回1时，调用Free_show_wlan_sta_of_all_cmd()释放空间*/
extern int show_wlan_sta_of_all_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_wlan_info **wlan);
																						/*返回0表示失败，返回1表示成功*/
																						/*返回-1表示wlan does not exist，返回-2表示error*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
extern void Free_show_distinguish_information_of_all_wtp_cmd(struct dcli_wtp_info *wtp);
/*只要调用函数，就调用Free_show_distinguish_information_of_all_wtp_cmd()释放空间*/
extern int show_distinguish_information_of_all_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_wtp_info **wtp);
																											/*返回0表示失败，返回1表示成功*/
																											/*返回-1表示wtp does not provide service or it maybe does not exist*/
																											/*返回-2表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
extern void Free_show_wapi_mib_info_of_all_wtp_cmd(struct dcli_wtp_info *wtp);
/*只要调用函数，就调用Free_show_wapi_mib_info_of_all_wtp_cmd()释放空间*/
extern int show_wapi_mib_info_of_all_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_wtp_info **wtp);
																								  /*返回0表示失败，返回1表示成功*/
																								  /*返回-1表示wtp does not provide service or it maybe does not exist*/
																								  /*返回-2表示error*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
extern void Free_show_sta_wapi_mib_info_of_all_wtp_cmd(struct wapi_mib_wtp_info *wtp);
/*只要调用函数，就调用Free_show_sta_wapi_mib_info_of_all_wtp_cmd()释放空间*/
extern int show_sta_wapi_mib_info_of_all_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,struct wapi_mib_wtp_info **wtp);
																										   /*返回0表示失败，返回1表示成功*/
																										   /*返回-1表示wtp does not provide service or it maybe does not exist*/
																										   /*返回-2表示error*/
																										   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
extern void Free_show_wlan_info_allwlan_cmd(struct dcli_wlan_info *wlan);

/*只要调用函数，就调用Free_show_wlan_info_allwlan_cmd()释放空间*/
extern int show_wlan_info_allwlan_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_wlan_info **wlan);
																						  /*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示wlan does not provide service or it maybe does not exist*/
																						  /*返回-2表示error*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_sta_base_info_cmd(struct dcli_base_bss_info *bsshead);
/*返回1时，调用Free_show_all_sta_base_info_cmd()释放空间*/
extern int show_all_sta_base_info_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_base_bss_info **bsshead);
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示there is no station，返回-2表示error*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*Type为"listen"或"listen_and_set"*/
/*state为"enable"或"disable"*/
extern int set_asd_sta_arp_listen_cmd(dbus_parameter parameter, DBusConnection *connection,char *Type,char *state);
														 /*返回0表示失败，返回1表示成功*/
														 /*返回-1表示input para error*/
														 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#endif

