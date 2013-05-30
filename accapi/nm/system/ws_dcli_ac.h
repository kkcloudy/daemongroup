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
* ws_dcli_ac.h
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


#ifndef _DCLI_AC_H
#define _DCLI_AC_H

#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_returncode.h"
#include "ws_public.h"

void dcli_ac_init(void);
#define DCLIAC_MAC_LEN	6
#define DCLIAC_RADIO_NUM_LEN	4
#define DCLIAC_OUI_LEN	3
#define WIDS_TYPE_LEN 16
#define WTP_WTP_IP_LEN 21
#define DCLIAC_BUF_LEN	16



struct mac_profile
{
    unsigned char       macaddr[DCLIAC_MAC_LEN];
	struct mac_profile *next;
};

typedef struct mac_profile WIDMACADDR;


typedef struct
{
    unsigned char       oui[DCLIAC_OUI_LEN];
}OUI_S;


struct acAccessWtpCount { 
    unsigned int license_type;
    unsigned int license_count[5];

    struct acAccessWtpCount *next;
};
struct acAccessBindLicCount { 
	int cur_count;
	int max_count;
	int bind_flag;

    struct acAccessBindLicCount *next;
};



extern int parse_int_ID(char* str,unsigned int* ID);
extern void CheckWIDSType(char *pattacktype, char* pframetype, unsigned char attacktype,unsigned char frametype);

extern void Free_wc_config(DCLI_AC_API_GROUP_FIVE *wirelessconfig);
/*返回1时，调用Free_wc_config()释放空间*/
extern int show_wc_config(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **wirelessconfig); /*返回0表示失败，返回1表示成功，返回-1表示error*/
																															 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wid_hw_version_func(dbus_parameter parameter, DBusConnection *connection,char *para);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wid_sw_version_func(dbus_parameter parameter, DBusConnection *connection,char *para);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wid_lev3_protocol_func(dbus_parameter parameter, DBusConnection *connection,char *protocol_type);
																				/*返回0表示失败，返回1表示成功，返回-1表示This version only surport IPv4,we will update later*/
																				/*返回-2表示input patameter should only be 'IPv4' or 'IPv6'，返回-3表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wid_auth_security_func(dbus_parameter parameter, DBusConnection *connection,char *auth_secu_type);/*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'X509_CERTIFCATE' or 'PRESHARED'，返回-2表示error*/
																													  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wid_name_func(dbus_parameter parameter, DBusConnection *connection,char *para);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int config_wireless_max_mtu(dbus_parameter parameter, DBusConnection *connection,char*mtu);
																  /*返回0表示失败，返回1表示成功*/
																  /*返回-1表示input parameter should be 500 to 1500*/
																  /*返回-2表示error*/
																  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int	set_wid_log_switch_cmd(dbus_parameter parameter, DBusConnection *connection,char*stat);
																    /*返回0表示失败，返回1表示成功*/
																	/*返回-1表示error*/
																	/*返回-2表示input patameter should only be 'ON' or 'OFF'*/
																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wid_log_level_func(dbus_parameter parameter, DBusConnection *connection,char *log_lever_type);/*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'info' 'debug' or 'all'，返回-2表示error*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为open或者close*/
/*type为"default"/"dbus"/"wtp"/"mb"/"all"*/
extern int set_wid_daemonlog_debug_open_func(dbus_parameter parameter, DBusConnection *connection,char *type,char *state);
																							   /*返回0表示失败，返回1表示成功*/
																							   /*返回-1表示input patameter should only be default|dbus|wtp|mb|all*/
																							   /*返回-2表示input patameter should only be 'open' or 'close'，返回-3表示error*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int  set_wid_log_size_cmd(dbus_parameter parameter, DBusConnection *connection,char *size);
																		  /*返回0表示失败，返回1表示成功*/	 
																		  /*返回-1表示input parameter should be 1000000 to 500000000*/
																		  /*返回-2表示error*/
																		  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_ap_scanning_report_interval_cmd(dbus_parameter parameter, DBusConnection *connection,char*time); 
																		  		 /*返回0表示失败，返回1表示成功*/
																				 /*返回-1表示input patameter should be 30 to 32767*/
																				 /*返回-2表示error*/
																				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int update_ap_scanning_info_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																				 					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state==0表示"disable"，state==1表示"enable"*/
extern int set_radio_resource_management(dbus_parameter parameter, DBusConnection *connection,int state);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_mac_whitelist(dbus_parameter parameter, DBusConnection *connection,char *mac);
														  /*返回0表示失败，返回1 表示成功*/
														  /*返回-1表示Unknown mac addr format*/
														  /*返回-2表示error*/
														  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int delete_mac_whitelist(dbus_parameter parameter, DBusConnection *connection,char *mac);
														  	  /*返回0表示失败，返回1 表示成功*/
														 	  /*返回-1表示Unknown mac addr format*/
															  /*返回-2表示error*/
															  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_wireless_control_whitelist(DCLI_AC_API_GROUP_ONE *LIST);
/*返回1时，调用Free_wireless_control_whitelist()释放空间*/
extern int show_wireless_control_whitelist(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE  **LIST);
																								/*返回0表示失败，返回1表示成功，返回2表示there is no white list，返回-1表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_mac_blacklist(dbus_parameter parameter, DBusConnection *connection,char *mac);
														  /*返回0表示失败，返回1 表示成功*/
												  		  /*返回-1表示Unknown mac addr format*/
														  /*返回-2表示error*/
														  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int delete_mac_blacklist(dbus_parameter parameter, DBusConnection *connection,char *mac);
														  	  /*返回0表示失败，返回1 表示成功*/
															  /*返回-1表示Unknown mac addr format*/
															  /*返回-2表示error*/
															  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_wireless_control_blacklist(DCLI_AC_API_GROUP_ONE *LIST);
/*返回1时，调用Free_wireless_control_blacklist()释放空间*/
extern int show_wireless_control_blacklist(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE  **LIST);
																							/*返回0表示失败，返回1表示成功，返回2表示there is no black list，返回-1表示error*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_rogue_ap_head(DCLI_AC_API_GROUP_TWO *LIST);
/*返回1时，调用Free_ap_head()释放空间*/
extern int show_rogue_ap_list(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_TWO **LIST);  
																				  /*返回0表示失败，返回1表示成功，返回2表示there is no rouge ap*/
                                                                                  /*返回-1表示radio resource managment is disable please enable first*/ 
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_rogue_aplist_bywtpid(DCLI_AC_API_GROUP_TWO *LIST);
/*返回1时，调用Free_rogue_aplist_bywtpid()释放空间*/
extern int show_rogue_aplist_bywtpid(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_AC_API_GROUP_TWO **LIST);  
																							  /*返回0表示失败，返回1表示成功，返回2表示there is no rouge ap*/
																							  /*返回-1表示input wtp id should be 1 to WTP_NUM-1，返回-2表示wtp does not exist*/
																							  /*返回-3表示radio resource managment is disable please enable first*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_neighbor_aplist_bywtpid(DCLI_AC_API_GROUP_TWO *LIST);
/*返回1时，调用Free_neighbor_aplist_bywtpid()释放空间*/
extern int show_neighbor_aplist_bywtpid(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_AC_API_GROUP_TWO **LIST);
																									   /*返回0表示失败，返回1表示成功，返回2表示there is no neighbor ap*/
																								       /*返回-1表示input wtp id should be 1 to WTP_NUM-1，返回-2表示wtp does not exist*/
																									   /*返回-3表示radio resource managment is disable please enable first*/	
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_neighbor_ap_list_bywtpid2_cmd(DCLI_AC_API_GROUP_TWO *dcli_list);				
/*返回1时，调用Free_show_neighbor_ap_list_bywtpid2_cmd()释放空间*/
extern int show_neighbor_ap_list_bywtpid2_cmd(dbus_parameter parameter, DBusConnection *connection,char *wtp_id,DCLI_AC_API_GROUP_TWO **dcli_list);
																											   /*返回0表示失败，返回1表示成功*/
				                                                                                               /*返回-1表示input wtp id should be 1 to WTP_NUM-1*/
				                                                                                               /*返回-2表示wtp does not existt*/
				                                                                                               /*返回-3表示radio resource managment is disable please enable first*/
																											   /*返回-4表示there is no neighbor ap*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int dynamic_channel_selection_cmd(dbus_parameter parameter, DBusConnection *connection,char*state);
																			/*返回0表示失败，返回1表示成功*/
		                                                                    /*返回-1表示input patameter only with 'enable' or 'disable'*/
		                                                                    /*返回-2表示you should enable radio resource management first*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*wtp_id和radio_G_id只需填写一个，分别表示在wtp节点和radio节点下配置*/
/*wtp_id为0时，表示全局配置*/
extern int set_system_country_code_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,int radio_G_id,char *Country_code);
																					/*返回0表示失败，返回1表示成功，返回-1表示input country code should be capital letters*/
																					/*返回-2表示input country code error，返回-3表示system country code is already Country_code, no need to change*/
																					/*返回-4表示system country code error，返回-5表示WTP ID非法，返回-6表示Radio ID非法*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					
extern int undo_system_country_code_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功，返回-1表示system country code is default, no need to change，返回-2表示system country code error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					
/*state为open或close*/
/*type为"default"/"dbus"/"80211"/"1x"/"wpa"/"wapi"/"leave"/"all"*/
extern int set_asd_daemonlog_debug_open_func(dbus_parameter parameter, DBusConnection *connection,char *type,char *state);
																							   /*返回0表示失败，返回1表示成功*/
																					 		   /*返回-1表示input patameter should only be default|dbus|80211|1x|wpa|wapi|leave|all*/
																							   /*返回-2表示input patameter should only be 'open' or 'close'，返回-3表示error*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_hostapd_logger_printflag_open_func(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'open' or 'close'，返回-2表示error*/
																							   							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																							   
extern int dynamic_power_selection_cmd(dbus_parameter parameter, DBusConnection *connection,char*state);/*返回0表示失败，返回1表示成功，返回-1表示you should enable radio resource management first*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_transmit_power_control_scope(dbus_parameter parameter, DBusConnection *connection,char *state); /*返回0表示失败，返回1表示成功，返回-1表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wirelesscontrol_auto_ap_switch(dbus_parameter parameter, DBusConnection *connection,char *stat);	
																				/*返回0表示失败，返回1表示成功*/
																		        /*返回-1表示input patameter should only be 'enable' or 'disable'*/
																				/*返回-2表示auto_ap_login interface has not set yet*/
																				/*返回-3表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wirelesscontrol_auto_ap_binding_l3_interface(dbus_parameter parameter, DBusConnection *connection,char *state,char* inter); 
																											 /*返回0表示失败，返回1表示成功，返回-1表示interface name is too long,should be no more than 15*/
																											 /*返回-2表示input patameter only with 'uplink'or 'downlink'，返回-3表示auto ap login switch is enable,you should disable it first*/
																											 /*返回-4表示interface argv[1] error, no index or interface down，返回-5表示interface argv[1] is down，返回-6表示interface argv[1] is no flags*/
																											 /*返回-7表示interface argv[1] is no index，返回-8表示interface argv[1] error，返回-9表示interface has be binded in other hansi*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											 
extern int set_wirelesscontrol_auto_ap_binding_wlan(dbus_parameter parameter, DBusConnection *connection,char*wlan_id,char *ifname);	
																										/*返回0表示失败，返回1表示成功，返回-1表示interface name is too long,should be no more than 15*/
																										/*返回-2表示wlanid should be 1 to WLAN_NUM-1，返回-3表示wlan not exist，返回-4表示wlan has not bind interface*/
																										/*返回-5表示 interface not in the auto ap login interface，返回-6表示auto_ap_login interface has not set yet*/
																										/*返回-7表示auto_ap_login interface argv[1] wlan num is already L_BSS_NUM*/
																										/*返回-8表示auto ap login switch is enable,you should disable it first*/
																										/*返回-9表示is no local interface, permission denial，返回-10表示interface error, no index or interface down*/
																										/*返回-11表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										
extern int del_wirelesscontrol_auto_ap_binding_wlan_func(dbus_parameter parameter, DBusConnection *connection,char *wlan_id,char *ifname); 
																												/*返回0表示失败，返回1表示成功，返回-1表示interface name is too long,should be no more than 15*/
																												/*返回-2表示wlanid should be 1 to WLAN_NUM-1，返回-3表示wlan not exist，返回-4表示wlan has not bind interface*/
																												/*返回-5表示interface argv[1] not in the auto ap login interface，返回-6表示auto_ap_login interface has not set yet*/
																												/*返回-7表示auto_ap_login interface argv[1] wlan num is 0，返回-8表示auto ap login switch is enable,you should disable it first*/
																												/*返回-9表示input interface dosen't exist!，返回-10表示error*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																												
extern int set_wirelesscontrol_auto_ap_save_config_switch(dbus_parameter parameter, DBusConnection *connection,char *stat);
																							  /*返回0表示失败，返回1表示成功*/
																							  /*返回-1表示input patameter should only be 'enable' or 'disable'*/
																							  /*返回-2表示auto ap login switch is enable,you should disable it first*/
																							  /*返回-3表示error*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_auto_ap_config(DCLI_AC_API_GROUP_FIVE *auto_ap_login);
/*返回1时，调用Free_auto_ap_config()释放空间*/
extern int show_auto_ap_config(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **auto_ap_login);/*返回0表示失败，返回1表示成功*/
																																/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_ap_timestamp(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_monitor_time(dbus_parameter parameter, DBusConnection *connection,char * mon_time);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示error*/
																					  				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_sample_time(dbus_parameter parameter, DBusConnection *connection,char * sample_time);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示error*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_sample_info(DCLI_AC_API_GROUP_FIVE *sample_info);
/*返回1时，调用Free_qos_head()释放空间*/
extern int show_sample_info(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **sample_info);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_monitor_enable(dbus_parameter parameter, DBusConnection *connection,char * able);
															  /*返回0表示失败，返回1表示成功*/
															  /*返回-1表示input patameter only with 'enable' or 'disable'*/
															  /*返回-2表示error*/
															  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_sample_enable(dbus_parameter parameter, DBusConnection *connection,char * able);/*返回0表示失败，返回1表示成功*/
																								  /*返回-1表示input patameter only with 'enable' or 'disable'*/
																								  /*返回-2表示error*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_txpower_control(DCLI_AC_API_GROUP_FIVE *tx_control);
/*返回1时，调用Free_ap_txpower_control()释放空间*/
extern int show_ap_txpower_control(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **tx_control);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_receiver_signal_level(dbus_parameter parameter, DBusConnection *connection,int level_num);/*返回0表示失败，返回1表示成功*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_receiver_signal_level(DCLI_AC_API_GROUP_FIVE *receiver_sig_lev);
/*返回1时，调用Free_receiver_signal_level()释放空间*/
extern int show_receiver_signal_level(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **receiver_sig_lev);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																		  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int clear_auto_ap_config_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*able==1表示"enable",able == 0表示"disable"*/
extern int set_ap_statistics(dbus_parameter parameter, DBusConnection *connection,int able);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_attack_mac_list_head(DCLI_AC_API_GROUP_ONE  *LIST);
/*返回1时，调用Free_attack_mac_list_head()释放空间*/
extern int show_attack_mac_list(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE  **LIST);/*返回0表示失败，返回1表示成功，返回-1表示there is no attack mac list，返回-2表示error*/
																														/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_legal_essid_list_head(DCLI_AC_API_GROUP_ONE *LIST);
/*返回1时，调用Free_legal_essid_list_head()释放空间*/
extern int show_legal_essid_list_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE **LIST);/*返回0表示失败，返回1表示成功，返回-1表示there is no legal essid list，返回-2表示error*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_manufacturer_oui_list_head(DCLI_AC_API_GROUP_ONE *LIST);
/*返回1时，调用Free_manufacturer_oui_list_head()释放空间*/
extern int show_manufacturer_oui_list(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_ONE **LIST);/*返回0表示失败，返回1表示成功，返回-1表示there is no legal manufacturer list，返回-2表示error*/
																															  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int add_legal_manufacturer_func(dbus_parameter parameter, DBusConnection *connection,char *OUI);/*返回0表示失败，返回1表示成功，返回-1表示Unknown OUI format，返回-2表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int add_legal_essid_func(dbus_parameter parameter, DBusConnection *connection,char *ESSID);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																									  /*返回-2表示essid is too long,out of the limit of 32*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int add_attack_ap_mac_func(dbus_parameter parameter, DBusConnection *connection,char *MAC);/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format，返回-2表示error*/
																									  	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int del_legal_manufacturer_func(dbus_parameter parameter, DBusConnection *connection,char *OUI);/*返回0表示失败，返回1表示成功，返回-1表示Unknown OUI format，返回-2表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int del_legal_essid_func(dbus_parameter parameter, DBusConnection *connection,char *ESSID);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																									/*返回-2表示essid is too long,out of the limit of 32*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int del_attack_ap_mac_func(dbus_parameter parameter, DBusConnection *connection,char *MAC);/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format，返回-2表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_ipfwd_func(char *state);/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/

extern void Free_ApMode_head(DCLI_AC_API_GROUP_FOUR *conf_info);
/*返回1时，调用Free_ApMode_head()释放空间*/
extern int show_model_list_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FOUR **conf_info);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_model_cmd(DCLI_AC_API_GROUP_FOUR *modelinfo);
/*返回1时，调用Free_model_cmd()释放空间*/
extern int show_model_cmd(dbus_parameter parameter, DBusConnection *connection,char *mode,DCLI_AC_API_GROUP_FOUR **modelinfo);/*返回0表示失败，返回1表示成功，返回-1表示this model doesn't exist*/
																																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_model_cmd(dbus_parameter parameter, DBusConnection *connection,char *mode,char *new_mode);/*返回0表示失败，返回1表示成功，返回-1表示new model is not configuration please change other name，返回-2表示this model doesn't exist*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_model_code(DCLI_AC_API_GROUP_FOUR *codeinfo);
/*返回1时，调用Free_ap_model_code()释放空间*/
extern int show_ap_model_code_func(dbus_parameter parameter, DBusConnection *connection, char *mode,DCLI_AC_API_GROUP_FOUR **codeinfo);/*返回0表示失败，返回1表示成功，返回-1表示this model doesn't exist*/
																																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_rogue_ap_list_v1_head(DCLI_AC_API_GROUP_TWO *LIST);
/*返回1时，调用Free_rogue_ap_list_v1_head()释放空间*/
extern int show_rogue_ap_list_v1_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_TWO **LIST);/*返回0表示失败，返回1表示成功，返回-1表示radio resource managment is disable please enable first，返回-2表示good luck there is no rouge ap*/
																															   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_txpower_threshold_cmd(dbus_parameter parameter, DBusConnection *connection,char *value);/*返回0表示失败，返回1表示成功，返回-1表示input patameter should be 20 to 35，返回-2表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_coverage_threshold_cmd(dbus_parameter parameter, DBusConnection *connection,char *value);/*返回0表示失败，返回1表示成功，返回-1表示input patameter should be 5 to 15，返回-2表示error*/
																											  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_neighbor_rssi_info_bywtpid(DCLI_AC_API_GROUP_THREE *RSSI);
/*返回1时，调用Free_neighbor_rssi_info_bywtpid()释放空间*/
extern int show_neighbor_rssi_info_bywtpid_cmd(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_AC_API_GROUP_THREE **RSSI); 
																				/*返回0表示失败，返回1表示成功，返回-1表示wtp id should be 1 to WTP_NUM-1*/
																				/*返回-2表示wtp does not exist，返回-3表示radio resource managment is disable please enable first*/
																				/*返回-4表示transmit power control is disable please enable first，返回-5表示there is no neighbor ap*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																									
extern int set_ap_cm_threshold_func(dbus_parameter parameter, DBusConnection *connection,char *thr_type,char *thr_value); 
																							/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'cpu','temperature'or 'memoryuse'*/
																							/*返回-2表示unknown id format，返回-3表示ap cpu threshold parameters error，返回-4表示ap memory use threshold parameters error*/
																							/*返回-5表示wtp id does not run，返回-6表示error*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_threshold(DCLI_AC_API_GROUP_FIVE *ap_threshold);
/*返回1时，调用Free_ap_threshold()释放空间*/																			
extern int show_ap_threshold_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **ap_threshold);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_rrm_config(DCLI_AC_API_GROUP_FIVE *resource_mg);
/*返回1时，调用Free_ap_rrm_config()释放空间*/
extern int show_ap_rrm_config_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **resource_mg);/*返回0表示失败，返回1表示成功 ，返回-1表示error*/
																																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int show_ipfwd_func(int *ipfwd_state);	/*ipfwd_state==0表示disable，ipfwd_state==1表示enable*/
													/*返回0表示失败，返回1表示成功，返回-1表示unexpected flow-based-forwarding state*/

extern void Free_wids_device_head(DCLI_AC_API_GROUP_TWO *LIST);
/*返回1时，调用Free_wids_device_head()释放空间*/
extern int show_wids_device_list_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_TWO **LIST);/*返回0表示失败，返回1表示成功，返回-1表示there is no wids device，返回-2表示error*/
																																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_wids_device_list_bywtpid_head(DCLI_AC_API_GROUP_TWO *LIST);
/*返回1时，调用Free_wids_device_list_bywtpid_head()释放空间*/
extern int show_wids_device_list_bywtpid_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID,DCLI_AC_API_GROUP_TWO **LIST);
																							 /*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																					 		 /*返回-2表示wtp does not exist，返回-3表示there is no wids device，返回-4表示error*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int clear_wids_device_list_cmd_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																													 
extern int clear_wids_device_list_bywtpid_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID);
																						/*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																						/*返回-2表示wtp does not exist，返回-3表示error*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_wids_statistics_list_bywtpid_head(DCLI_AC_API_GROUP_THREE *widsstatis);
/*返回1时，调用Free_wids_statistics_list_bywtpid_head()释放空间*/																		
extern int show_wids_statistics_list_bywtpid_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID,DCLI_AC_API_GROUP_THREE **widsstatis);
																										/*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																									    /*返回-2表示wtp does not exist，返回-3表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_wids_statistics_list_head(DCLI_AC_API_GROUP_THREE *widsstatis);
/*返回1时，调用Free_wids_statistics_list_head()释放空间*/																																
extern int show_wids_statistics_list_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_THREE **widsstatis);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int clear_wids_statistics_list_bywtpid_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *WtpID);
																						  /*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																						  /*返回-2表示wtp does not exist，返回-3表示error*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						  
extern int clear_wids_statistics_list_cmd_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																						  				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						  
extern int set_neighbordead_interval_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *value);
																				  /*返回0表示失败，返回1表示成功*/
																				  /*返回-1表示input patameter should be 20 to 2000，返回-2表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_neighbordead_interval(DCLI_AC_API_GROUP_FIVE *interval);
/*返回1时，调用Free_neighbordead_interval()释放空间*/
extern int show_neighbordead_interval_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **interval);/*返回0表示失败，返回1表示成功*/
																																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int update_bak_ac_config_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int synchronize_wsm_table_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int notice_vrrp_state_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功*/
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*para的范围是1-5，单位是second*/
extern int set_wtp_wids_interval_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *para);
																			/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																			/*返回-2表示wtp wids interval error,should be 1 to 5 second*/
																			/*返回-3表示wids switch is enable，返回-4表示error*/
																			/*返回-5表示illegal input:Input exceeds the maximum value of the parameter type*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*policy_type为"probe"或"other"*/
/*para的范围是1-100*/
extern int set_wtp_wids_threshold_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *policy_type,char *para);
																								 /*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'probe' or 'other'*/
																								 /*返回-2表示unknown id format，返回-3表示wtp wids threshold error,should be 1 to 100*/
																								 /*返回-4表示wids switch is enable，返回-5表示error，返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*para的范围是1-36000*/
extern int set_wtp_wids_lasttime_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *para);
																			 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																			 /*返回-2表示wtp wids lasttime in black error,should be 1 to 36000*/
																			 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*level的范围是0-25*/
extern int set_wid_trap_open_func(dbus_parameter parameter, DBusConnection *connection,char *level);/*返回1表示成功，返回0表示失败，返回-1表示error*/
																			 							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_rogue_ap_trap_threshold(DCLI_AC_API_GROUP_FIVE *rogue_trap);
/*返回1时，调用Free_rogue_ap_trap_threshold()释放空间*/															 
extern int show_rogue_ap_trap_threshold_func(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **rogue_trap);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*value的范围是1-200*/
extern int set_rogue_ap_trap_threshold_func(dbus_parameter parameter, DBusConnection *connection,char *value);
																			   /*返回0表示失败，返回1表示成功，返回-1表示input patameter should be 1 to 200*/
																			   /*返回-2表示radio resource managment is disable please enable first，返回-3表示error*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			   
/*type为"add"或"del"*/
extern int set_wirelesscontrol_auto_ap_binding_l3_interface_new_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *ifname);
																			   										   /*返回0表示失败，返回1表示成功*/
																													   /*返回-1表示interface name is too long,should be no more than 15*/
																													   /*返回-2表示input patameter only with 'add'or 'del'*/
																													   /*返回-3表示auto ap login switch is enable,you should disable it first*/
																													   /*返回-4表示interface argv[1] error, no index or interface down*/
																													   /*返回-5表示interface argv[1] is down，返回-6表示interface argv[1] is no flags*/
																													   /*返回-7表示interface argv[1] is no index*/
																													   /*返回-8表示is no local interface, permission denial*/
																													   /*返回-9表示interface argv[1] error*/
																													   /*返回-10表示interface has not been added or has already been deleted*/
																													   /*返回-11表示interface has be binded in other hansi*/
																													   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*update_time的范围是5-3600*/																													   
extern int set_ap_update_img_timer_cmd(dbus_parameter parameter, DBusConnection *connection,char *update_time);
																				 /*返回0表示失败，返回1表示成功*/
																				 /*返回-1表示unknown id format，返回-2表示error*/
																				 /*返回-3表示input time should be 5-3600*/
																				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_update_img_timer(DCLI_AC_API_GROUP_FIVE *up_timer);
/*返回1时，调用Free_ap_update_img_timer()释放空间*/
extern int show_ap_update_img_timer_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **up_timer);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int update_wtpcompatible_cmd(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_ap_update_fail_count_cmd(dbus_parameter parameter, DBusConnection *connection,char *update_fail_count);/*返回0表示失败，返回1表示成功*/
																															/*返回-1表示unknown id format*/
																															/*返回-2表示error*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_update_fail_count(DCLI_AC_API_GROUP_FIVE *update_fail);
/*返回1时，调用Free_ap_update_fail_count()释放空间*/
extern int show_ap_update_fail_count_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **update_fail);/*返回0表示失败，返回1表示成功*/
																																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wid_watch_dog_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																										  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_network_bywtpid(DCLI_AC_API_GROUP_THREE *network);
/*返回1时，调用Free_ap_network_bywtpid()释放空间*/
extern int show_ap_network_bywtpid_cmd(dbus_parameter parameter, DBusConnection *connection,char *wtp_id,DCLI_AC_API_GROUP_THREE **network);
																												/*返回0表示失败，返回1表示成功*/
																											    /*返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																											    /*返回-2表示ap have not ip information*/
																											    /*返回-3表示wtp id no exist*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																												
/*method_type为"number","flow"或"disable"*/
extern int ac_load_balance_cmd(dbus_parameter parameter, DBusConnection *connection,char *method_type);/*返回0表示失败，返回1表示成功，返回-1表示operation fail，返回-2表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ac_balance(DCLI_AC_API_GROUP_FIVE *balance);
/*返回1时，调用Free_ac_balance()释放空间*/
extern int show_ac_balance_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **balance);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																   
/*state为"enable"或"disable"*/
extern int set_ap_hotreboot_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_ap_access_through_nat_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
																												   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*policy_type为"no"或"forbid"*/
extern int set_wtp_wids_policy_cmd(dbus_parameter parameter, DBusConnection *connection,char *policy_type);/*返回0表示失败，返回1表示成功，返回-1表示wids switch is enable，返回-2表示error*/
																												 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int add_wids_mac_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC);/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format，返回-2表示error*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int del_wids_mac_cmd(dbus_parameter parameter, DBusConnection *connection,char *MAC);/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format，返回-2表示error*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*返回1且mac_num>0时，调用Free_maclist_head()释放空间*/
extern int show_wids_mac_list_cmd(dbus_parameter parameter, DBusConnection *connection,WIDMACADDR *mac_head,int *mac_num);/*返回0表示失败，返回1表示成功，返回-1表示there is no wids ignore mac list，返回-2表示error*/
																								 								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_ap_countermeasures_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*mode为"ap"或"adhoc"或"all"*/
extern int set_ap_countermeasures_mode_cmd(dbus_parameter parameter, DBusConnection *connection,char *mode);/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示error*/
																													  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int dynamic_channel_selection_range_cmd(dbus_parameter parameter, DBusConnection *connection,int argc,char **argv);
																							  /*返回0表示失败，返回1表示成功*/
																							  /*返回-1表示range of the channel num is 2-4.，返回-2表示patameter format error.*/
																							  /*返回-3表示you should enable radio resource management first*/
																							  /*返回-4表示illegal input:Input exceeds the maximum value of the parameter type*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_vrrp_state(DCLI_AC_API_GROUP_FIVE *vrrp_state);
/*返回1时，调用Free_vrrp_state()释放空间*/
extern int show_vrrp_state_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FIVE **vrrp_state);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																															  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_vrrp_sock(DCLI_AC_API_GROUP_FOUR *baksock);
/*返回1时，调用Free_vrrp_sock()释放空间*/
extern int show_vrrp_sock_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_FOUR **baksock);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																														   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*type为"number"或"flow"*/
/*number参数的范围是1-10*/
/*flow参数的范围是1-30*/
extern int ac_balance_parameter_cmd(dbus_parameter parameter, DBusConnection *connection,char *type,char *para);
																				  /*返回0表示失败，返回1表示成功，返回-1表示unknown input*/
																				  /*返回-2表示balance parameter should be 1 to 10，返回-3表示balance parameter should be 1 to 30*/
																				  /*返回-4表示operation fail，返回-5表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*trap_type为"ap_run_quit","ap_cpu_threshold","ap_mem_threshold","ap_update_fail","rrm_change","rogue_ap_threshold",
				  "rogue_terminal_threshold","rogue_ap","rogue_device","wireless_interface_down","channel_count_minor"或"channel_change"*/
/*state为"enable"或"disable"*/
extern int set_wid_trap_switch_able_cmd(dbus_parameter parameter, DBusConnection *connection,char *trap_type,char *state);
																				  		   /*返回0表示失败，返回1表示成功，返回-1表示the first input patameter error*/
																						   /*返回-2表示input patameter should only be 'enable' or 'disable'，返回-3表示error*/
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int show_wid_trap_switch_info_cmd(dbus_parameter parameter, DBusConnection *connection,WID_TRAP_SWITCH **INFO);/*返回0表示失败，返回1表示成功*/

extern int modify_legal_essid_cmd(dbus_parameter parameter, DBusConnection *connection,char *old_essid,char *new_essid);
																		/*返回0表示失败，返回1表示成功*/
																		/*返回-1表示The essid list is null,there is no essid*/
																		/*返回-2表示The essid input is not exit*/
																		/*返回-3表示error*/
																		/*返回-4表示first essid is too long,out of the limit of 32*/
																		/*返回-5表示second essid is too long,out of the limit of 32*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_ac_all_ap_extension_information_enable_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);
																								/*返回0表示失败，返回1表示成功*/
																								/*返回-1表示input patameter only with 'enable' or 'disable'*/
																								/*返回-2表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_wid_mac_whitelist_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_wid_essid_whitelist_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																											  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int change_wirelesscontrol_whitelist_cmd(dbus_parameter parameter, DBusConnection *connection,char *old_mac,char *new_mac);
																								    /*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format*/
																									/*返回-2表示src mac isn't exist，返回-3表示dst mac already in white list，返回-4表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int change_wirelesscontrol_blacklist_cmd(dbus_parameter parameter, DBusConnection *connection,char *old_mac,char *new_mac);
																									/*返回0表示失败，返回1表示成功，返回-1表示Unknown mac addr format*/
																									/*返回-2表示src mac isn't exist，返回-3表示dst mac already in white list，返回-4表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

	

/*config节点下wtp_id为0*/
/*value的范围是1-32767*/
extern int set_ap_statistics_inter_cmd(dbus_parameter parameter, DBusConnection *connection,int wtp_id,char *value); 
																				   /*返回0表示失败，返回1表示成功*/
																				   /*返回-1表示input interval should be 1 to 32767*/
																				   /*返回-2表示invalid wtpid，返回-3表示error*/
																				   /*返回-4表示illegal input:Input exceeds the maximum value of the parameter type*/
																				   /*返回-5表示unknown id format*/
																				   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_neighbor_ap_list_cmd(struct allwtp_neighborap *neighborap);
/*只要调用函数，就调用Free_show_neighbor_ap_list_cmd()释放空间*/
extern int show_neighbor_ap_list_cmd(dbus_parameter parameter, DBusConnection *connection,struct allwtp_neighborap **neighborap);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_wids_device_of_all_cmd(DCLI_AC_API_GROUP_TWO *dcli_list);
/*返回1时，调用Free_show_wids_device_of_all_cmd()释放空间*/
extern int show_wids_device_of_all_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_AC_API_GROUP_TWO **dcli_list,unsigned int *last_time);
																														 /*返回0表示失败，返回1表示成功*/
																														 /*返回-1表示good luck there is no wids device*/
																														 /*返回-2表示error*/
																														 /*返回SNMPD_CONNECTION_ERROR表示connection error*/


extern void Free_show_ac_access_wtp_vendor_count(struct acAccessWtpCount *count_head, struct acAccessBindLicCount *head);
/*返回1时，调用Free_show_ac_access_wtp_vendor_count()释放空间*/
extern int show_ac_access_wtp_vendor_count(dbus_parameter parameter, DBusConnection *connection, struct acAccessWtpCount **count_head, struct acAccessBindLicCount **bind_lic_head);





/*config节点下wtp_id为0*/
/*scan_mode为"2--monitor"、"3--halfmonitor"或"1--disable"*/
extern int set_wids_monitor_mode_cmd(dbus_parameter parameter, DBusConnection *connection, unsigned int wtp_id, unsigned int scan_mode);
																																/*返回0表示失败，返回1表示成功，返回-1表示invalid input*/
																			                                                    /*返回-2表示invalid wtp id，返回-3表示error*/


#endif

