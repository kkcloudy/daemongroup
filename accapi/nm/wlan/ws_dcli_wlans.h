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
* ws_dcli_wlans.h
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


#ifndef _WS_DCLI_WLANS_H
#define _WS_DCLI_WLANS_H

#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "dbus/wcpss/dcli_wid_radio.h"
#include "ws_dbus_list_interface.h"

#define VLANID_RANGE_MAX			4094
#define DCLIAC_BUF_LEN	16
#define CMD_FAILURE -1
#define WTP_WTP_IP_LEN 21
#define STRING_LEN 256


#define PARSE_RADIO_IFNAME_SUB '-'
#define PARSE_RADIO_IFNAME_POINT '.'


typedef enum{
	e_check_wtpid=0,
	e_check_sub,
	e_check_radioid,
	e_check_wlanid,
	e_check_point,
	e_check_fail,
	e_check_end,
	e_check_success
}radio_ifname_state;



#define RATE_SPLIT_COMMA 	','
#define RADIO_IF_NAME_LEN 20
#define RADIO_RATE_LIST_LEN 20
#define WLAN_IF_NAME_LEN 20
#define WTP_ARRAY_NAME_LEN 20
#define WTP_COMMAND_LEN 256
#define WTP_IP_BUFF_LEN 16
#define WTP_WLAN_VLAN_LEN 4
#define WTP_LIST_SPLIT_COMMA 	','

#define WTP_LIST_SPLIT_COMMA 	','	
#define WTP_LIST_SPLIT_BAR 	'-'	



typedef enum{
	check_rate=0,
	check_comma,
	check_fail,
	check_end,
	check_success
}rate_list_state;

#if 0
struct extern_info{
	int wtpid;
  float cpu;
  unsigned int tx_mgmt;
  unsigned int rx_mgmt;
  unsigned int totle_mgmt;
  unsigned int tx_packets;
  unsigned int tx_errors;
  unsigned int tx_retry;
  unsigned char eth_count;
  unsigned char eth_updown_time[AP_ETH_IF_NUM];
  unsigned char ath_count;
  unsigned char ath_updown_time[AP_ATH_IF_NUM];
} ;
typedef struct extern_info wid_info;
#endif

struct wtp_eth_inf_infor_profile
{
	char *name;
	unsigned char state;
	struct wtp_eth_inf_infor_profile *next;
};

struct ap_reportinterval_profile
{
	unsigned int moment_report_value;
	unsigned int routine_report_value;
	unsigned char moment_report_switch;
	int collect_time;
	unsigned int sample_time;
};

typedef enum{
	dcli_wtp_check_wtpid=0,
	dcli_wtp_check_comma,
	dcli_wtp_check_fail,
	dcli_wtp_check_end,
	dcli_wtp_check_success
}wtp_list_state;

struct SSIDStatsInfo_sub_wlan_v2{
	unsigned char wlanCurrID;
	/*should input update wtp bss pakets infomation*/
	/*------------------------------------------*/
	unsigned long long SSIDChStatsDwlinkTotRetryPkts;	//SSID下行重传包数		//wtp f
	unsigned long long SSIDChStatsUplinkUniFrameCnt;			//SSID上行单播帧数//wtp f
	unsigned long long SSIDChStatsDwlinkUniFrameCnt;			//SSID下行单播帧数//wtp f
	unsigned long long SSIDUpChStatsFrameNonUniFrameCnt;		//SSID上行非单播帧数//wtp f
	unsigned long long SSIDDownChStatsFrameNonUniFrameCnt;		//SSID下行非单播帧数//wtp f
	
	unsigned long long SSIDDwlinkTotFrameCnt;		//SSID下行的总帧数//wtp f
	unsigned long long SSIDUplinkTotFrameCnt;		//SSID上行的总帧数//wtp f
	/*------------------------------------------*/

	unsigned int SSIDRxCtrlFrameCnt;			//SSID接收的控制帧数
	unsigned int SSIDRxDataFrameCnt;			//SSID接收的数据帧数
	unsigned int SSIDRxAuthenFrameCnt;			//SSID接收的认证帧数
	unsigned int SSIDRxAssociateFrameCnt;		//SSID接收的关联帧数
	unsigned int SSIDTxCtrlFrameCnt;			//SSID发送的控制帧数
	unsigned int SSIDTxDataFrameCnt;			//SSID发送的数据帧数
	unsigned int SSIDTxAuthenFrameCnt;			//SSID发送的认证帧数
	unsigned int SSIDTxAssociateFrameCnt;		//SSID发送的关联帧数

	unsigned int SSIDDwErrPkts;                 //SSID下行的错误包数
	unsigned int SSIDDwDropPkts;				//SSID下行的总丢帧数
	unsigned int SSIDDwTotErrFrames;			//SSID下行的总错帧数
	unsigned int SSIDUpErrPkts;					//SSID上行的错误包数
	unsigned int SSIDUpDropPkts;				//SSID上行的总丢帧数
	unsigned int SSIDUpTotErrFrames;			//SSID上行的总错帧数
	
	unsigned int SSIDApChStatsNumStations;			//使用该信道的终端数
	unsigned int SSIDAccessTimes;				//access times
	/*table 34*/
	/*--------------------------------------*/
	/*在wtp 结点下输入update wtp bss pakets infomation*/
	unsigned char *wtpSSIDName;
	unsigned int wtpSSIDMaxLoginUsr;
	unsigned char wtpSSIDState;
	unsigned char wtpSSIDSecurityPolicyID;
	unsigned char wtpSSIDLoadBalance;
	char *wtpSSIDESSID;
	unsigned int vlanid;	//xiaodawei add vlanid 20101028

	/*--------------------------------------*/

	struct SSIDStatsInfo_sub_wlan_v2 *next;
};

struct SSIDStatsInfo_v2 {
    unsigned char *wtpMacAddr;
    int wtpCurrID;

    unsigned char wtpBwlanRadioNum;//wtp 下可用的radio数
    unsigned int wtpSupportRadioId[L_RADIO_NUM];
    
 	unsigned char wtpSupportSSID[WLAN_NUM] ; 		//should make ]
 	unsigned char wtpBssNum;
	
    unsigned int SSIDTxSignalPkts;			//SSID发送的信令包数	//wtp f
    unsigned int SSIDRxSignalPkts;			//SSID接收的信令包数	//wtp f
    
    unsigned int SSIDTxDataPkts;			//SSID发送的数据包数//wtp f
    unsigned int SSIDRxDataPkts;			//SSID接收的数据包数//wtp f
    unsigned long long SSIDUplinkDataOctets;			//SSID接收的字节数//wtp f
    unsigned long long SSIDDwlinkDataOctets;			//SSID发送的字节数//wtp f
    unsigned int SSIDApChStatsFrameFragRate;			//信道上帧的分段速率//wtp f
	
	struct SSIDStatsInfo_sub_wlan_v2 *SSIDStatsInfo_sub_wlan_head;
    struct SSIDStatsInfo_v2 *next;
};

#if 0
/*dt 2013.11.6*/
struct WlanDataPktsInfo_v2{
		
	long  wlanCurrID;			//Wlan当前ID
	unsigned long long wtpSsidSendTermAllByte;		//指定SSID AP发送到终端的字节数
	unsigned long wtpSsidRecvTermAllPack;		//指定SSID AP从终端接收的包数
	unsigned long long wtpSsidRecvTermAllByte;		//指定SSID AP从终端接收的字节数	
	unsigned long long wtpSsidWirelessMacRecvDataRightByte;	/*xiaodawei modify,20101116, 指定SSID MAC层接收的正确的数据字节数*/
	unsigned long long wtpSsidWirelessMacSendDataRightByte;	/*xiaodawei modify,20101116, 指定SSID MAC层发送的正确的数据字节数*/
	unsigned long wtpSsidWiredMacRecvDataWrongPack;	//指定SSID MAC层接收的错误的数据包数
	unsigned long wtpNetWiredRecvPack;		//指定SSID 网络测（有线侧）接收的包数
	unsigned long wtpUsrWirelessMacRecvDataPack;/*xiaodawei modify,20101116, 无线侧MAC层收到的数据包数*/
	unsigned long wtpUsrWirelessMacSendDataPack;/*xiaodawei modify,20101116, 无线侧MAC层发送的数据包数*/
	unsigned long wtpNetWiredSendPack;		//指定SSID 网络侧（有线侧）发送的包数
	unsigned long WtpWirelessSendFailPkts;		//指定SSID 无线侧发送失败的包数
	unsigned long wtpWirelessResendPkts; 	//指定SSID 无线侧总的重传包数
	char *wtpWirelessWrongPktsRate; 	//指定SSID 无线侧错包率
	unsigned long wtpWirelessSendBroadcastMsgNum;	//指定SSID 无线侧发送的广播包数
	unsigned long wtpStaUplinkMaxRate;		//指定SSID station上行最大速率
	unsigned long wtpStaDwlinkMaxRate;		//指定SSID station下行最大速率
	unsigned long wtpNetWiredRecvErrPack;		//指定SSID 网络侧（有线侧）接收的错包数
	unsigned long wtpNetWiredRecvRightPack;		//指定SSID 网络侧（有线侧）接收的正确包数
	unsigned long long wtpNetWiredRecvByte;		//指定SSID 网络侧（有线侧）接收的字节数
	unsigned long long wtpNetWiredSendByte;		//指定SSID 网络侧（有线侧）发送的字节数
	unsigned long wtpNetWiredSendErrPack;		//指定SSID 网络侧（有线侧）发送的错包数
	unsigned long wtpNetWiredSendRightPack;		//指定SSID 网络侧（有线侧）发送的正确包数
	unsigned long wtpSsidSendDataAllPack;		//制定SSID 网络侧（有线侧）发送的所有包数
	char *wtpNetWiredRxWrongPktsRate;	//指定SSID 网络侧（有线侧）接收的错误包百分比
	char *wtpNetWiredTxWrongPktsRate;	//指定SSID 网络侧（有线侧）发送的错误包百分比
	unsigned int wtpSsidTxDataDropPkts;
	unsigned int wtpSsidRxDataDropPkts;
	
	struct WlanDataPktsInfo_v2 *next;
};
struct WtpWlanDataPktsInfo_v2 {
	long  wtpCurrID;			//AP当前ID
	char *wtpMacAddr;           //AP的MAC地址
	unsigned int wlan_num;    //numbers of entries in wlan_list
	struct WlanDataPktsInfo_v2* wlan_list;
	
	struct WtpWlanDataPktsInfo_v2* next;
};

#endif




struct ap_num_by_interface
{
	char if_name[255];
	int ap_num;
	struct ap_num_by_interface *next;
};
typedef struct ap_num_by_interface AP_NUM_INF;

typedef enum{
	dcli_mcs_check_mcsid=0,
	dcli_mcs_check_comma=1,
	dcli_mcs_check_fail=2,
	dcli_mcs_check_end=3,
	dcli_mcs_check_success=4,
	dcli_mcs_check_bar=5
}mcs_list_state;
#define MCS_LIST_SPLIT_COMMA  ','	
#define MCS_LIST_SPLIT_BAR 	  '-'

struct model_tar_file {
	char *apmodel;
	char *filename;
    struct model_tar_file *next;
};


extern int parse_short_ID(char* str,unsigned short* ID);
extern int parse_char_ID(char* str,unsigned char* ID);
extern int parse_int_ID(char* str,unsigned int* ID);
extern void str2lower(char **str);



extern int create_wlan(dbus_parameter parameter, DBusConnection *connection,char * id, char *wlan_name, char *wlan_essid);/*返回0表示创建失败，返回1表示创建成功，返回-1表示WLAN ID非法，返回-2表示wlan已存在，返回-3表示出错*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int create_wlan_cmd_cn(dbus_parameter parameter, DBusConnection *connection,char *id, unsigned char *wlan_name, unsigned char *wlan_essid);
																					  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																					  /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示wlan name is too long,out of the limit of 15*/
																					  /*返回-4表示wlan name is illegal，返回-5表示essid is too long,out of the limit of 32*/
																					  /*返回-6表示wlan id exist，返回-7表示illegal input，返回-8表示error*/
																					  /*返回-9表示illegal input:Input exceeds the maximum value of the parameter type*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int delete_wlan(dbus_parameter parameter, DBusConnection *connection,int id);    /*返回0表示 删除失败，返回1表示删除成功*/
																						/*返回-1表示WLAN ID非法，返回-2表示WLAN ID NOT existed*/
																						/*返回-3表示please disable WLAN first，返回-4表示出错*/
																						/*返回-5表示some radios interface in ebr,please delete it from ebr first*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int config_wlan_service(dbus_parameter parameter, DBusConnection *connection,int id,char *state);	 
																		 /*返回0表示失败，返回1表示成功，返回-1表示no security profile binded，返回-2表示wtp interface policy conflict*/
						                                                 /*返回-3表示you map layer3 interace error，返回-4表示you should bind interface first，返回-5表示error，返回-6示WLAN ID非法*/
																		 /*返回-7表示wlan bingding securithindex same*/
																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																		 
extern int wlan_apply_interface(dbus_parameter parameter, DBusConnection *connection,int id,char *inter_name);   
																		 	   /*返回0表示失败，返回1表示成功，返回-1表示the length of interface name excel 16*/
						                                                       /*返回-2表示wlan id does not exist，返回-3表示interface dose not exist*/
						                                                       /*返回-4表示wlan is enable,please disable it first，返回-5表示error，返回-6示WLAN ID非法*/
																			   /*返回-7表示is no local interface, permission denial，返回-8表示interface has be binded in other hansi*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int wlan_apply_ipv6interface(dbus_parameter parameter, DBusConnection *connection,int id,char *inter_name);
																			    /*返回0表示失败，返回1表示成功*/
																				/*返回-1表示input parameter size is excel the limit of 16*/
																				/*返回-2表示wlan id does not exist，返回-3表示interface dose not exist*/
																				/*返回-4表示wlan is enable,please disable it first*/
																				/*返回-5表示wlan bingding ipv6 addr error make sure interface have ipv6 address*/
																				/*返回-6表示error，返回-7示WLAN ID非法*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																				
extern int  set_interface_nasid_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *inter_name,char *nasid); 
																							  /*返回0表示失败，返回1表示成功*/
		 																					  /*返回-1表示the length of input parameter is excel the limit of 16*/
		 																					  /*返回-2表示the length of input parameter is excel the limit of 128*/
		 																					  /*返回-3表示nas identifier include unknow character*/
		 																					  /*返回-4表示interface does not exist*/
		 																					  /*返回-5表示interface unused*/
		 																					  /*返回-6表示you should apply security first*/
		 																					  /*返回-7表示no nas_id needed,please apply interface without nas_identifier*/
		 																					  /*返回-8表示wlan be enable,please service disable first*/
		 																					  /*返回-9表示error，返回-10示WLAN ID非法*/
																							  /*返回-11表示interface has be binded in other hansi*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int remove_interface_nasid_cmd_func(dbus_parameter parameter, DBusConnection *connection,int WlanID,char *if_name);
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示the length of if_name is excel the limit of 16*/
																							/*返回-2表示wlan id is not exist*/
																							/*返回-3表示wlan WlanID is not binding interface if_name*/
																							/*返回-4表示interface error*/
																							/*返回-5表示no nas_id needed,please use <wlan apply interface IFNAME>,without nas_identifier*/
																							/*返回-6表示wlan be enable,please service disable first*/
																							/*返回-7表示error，返回-8示WLAN ID非法*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																							
extern int wlan_delete_interface(dbus_parameter parameter, DBusConnection *connection,int id,char *inter_name);  
																			   /*返回0表示失败，返回1表示成功，返回-1表示wlan doesn't binding this interface，返回-2表示Wlan ID Not existed*/
																			   /*返回-3表示Interface not existed，返回-4表示Wlan is enable binding error! please disable first，返回-5表示error，返回-6示WLAN ID非法*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			   
extern void CheckWIDIfPolicy(char *whichinterface, unsigned char wlan_if_policy);
														   
extern void Free_wlan_head(DCLI_WLAN_API_GROUP *WLANINFO);
/*返回1时，调用Free_wlan_head()释放空间*/														   
extern int show_wlan_list(dbus_parameter parameter, DBusConnection *connection,DCLI_WLAN_API_GROUP **WLANINFO);/*返回0表示失败，返回1表示成功，返回-1表示wlan not exsit*/
																												  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_one_wlan_head(DCLI_WLAN_API_GROUP *WLANINFO);
/*返回1时，调用Free_one_wlan_head()释放空间*/
extern int show_wlan_one(dbus_parameter parameter, DBusConnection *connection,int id,DCLI_WLAN_API_GROUP **WLANINFO); 
																					 /*返回0表示失败，返回1表示成功*/
						                                                             /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
						                                                             /*返回-2表示wlan id is not exited，返回-3表示error*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*num的范围是1-65536*/																					 
extern int config_wlan_max_sta_num(dbus_parameter parameter, DBusConnection *connection,int id,char * num);  
																				/*返回0表示失败，返回1表示成功*/
																				/*返回-1表示unknown NUM format，返回-2表示wlan id not exist.*/
																				/*返回-3表示more sta(s) has accessed before you set max sta num*/
																				/*返回-4表示operation fail，返回-5表示error，返回-6示WLAN ID非法*/
																				/*返回-7表示input max sta num should be 1-65536*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*sec_ID表示security id;w_id表示wlan id*/																
extern int apply_wlanID(dbus_parameter parameter, DBusConnection *connection,int sec_id,int w_id);
																/*返回0表示失败，返回1表示成功*/
			                                                    /*返回-1表示wlan id not exist，返回-2表示asd security profile not exist*/
			                                                    /*返回-3表示asd security profile not integrity，返回-4表示encryption type dosen't match with security type*/
			                                                    /*返回-5表示should be disable wlan first，返回-6表示security ID非法，返回-7示WLAN ID非法*/
																/*返回-8表示security rdc has not config!*/
																/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*Hessid==1表示yes，Hessid==0表示no*/																
extern int set_hideessid(dbus_parameter parameter, DBusConnection *connection,int id,char *Hessid);
																/*返回0表示失败，返回1表示成功，返回-1表示wlan id not exist*/
												  				/*返回-2表示WLAN is enable,please disable it first，返回-3表示error，返回-4示WLAN ID非法*/
																/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																
extern int wlan_map_interface(dbus_parameter parameter, DBusConnection *connection,int id);	
															/*返回0表示失败，返回1表示成功，返回-1表示Wlan ID Not existed*/
														    /*返回-2表示WLAN is enable, please disable it first*/
				                                            /*返回-3表示WLAN have already binding vlan,please undo wlan-vlan binding first*/
				                                            /*返回-4表示error，返回-5示WLAN ID非法*/
															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
															
extern int wlan_unmap_interface(int id);	/*返回0表示失败，返回1表示成功，返回-1表示Wlan ID Not existed*/
												/*返回-2表示WLAN is enable, please disable it first，返回-3表示error，返回-4示WLAN ID非法*/
extern int wlan_bss_map_interface(int id);	  /*返回0表示失败，返回1表示成功，返回-1表示Wlan ID Not existed*/
											      /*返回-2表示WLAN is enable, please disable it first，返回-3表示error，返回-4示WLAN ID非法*/
extern int wlan_bss_unmap_interface(int id);		/*返回0表示失败，返回1表示成功，返回-1表示Wlan ID Not existed*/
     													/*返回-2表示WLAN is enable, please disable it first，返回-3表示error，返回-4示WLAN ID非法*/		  


extern int no_interface_ifname(char *ifname);//取消三层接口映射

/*type为"number","flow"或"disable"*/
extern int config_wlan_load_balance(dbus_parameter parameter, DBusConnection *connection,int wid,char *type);/*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist，返回-2表示operation fail，返回-3表示error，返回-4表示WLAN ID非法*/
																												  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						
extern int config_wlan_number_balance_parameter(dbus_parameter parameter, DBusConnection *connection,int id,char* param);
																							 /*返回0表示失败，返回1表示成功，返回-1表示balance parameter should be 1 to 10*/
					                                                                         /*返回-2表示wlan id does not exist，返回-3表示operation fail ，返回-4表示error，返回-5示WLAN ID非法*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																							 
extern int config_wlan_flow_balance_parameter(dbus_parameter parameter, DBusConnection *connection,int wid,char * para);/*返回0表示失败，返回1表示成功，返回-1表示balance parameter should be 1 to 30，返回-2表示wlan id does not exist， 返回-3表示operation fail，返回-4表示error，返回-5示WLAN ID非法*/
																							 								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*type为"l3"*/
extern int wlan_roam_policy(dbus_parameter parameter, DBusConnection *connection,int id,char *type,char*stat);	
																				/*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist*/
																				/*返回-2表示operation fail，返回-3表示wlan should be disable first*/
																				/*返回-4表示roaming should be disable first，返回-5表示error，返回-6示WLAN ID非法*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																				
extern int set_wlan_vlan_id(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *vlan_id);  
																			/*返回0表示失败，返回1表示成功，返回-1表示unknown input*/
																			/*返回-2表示input parameter should be 1 to 4094，返回-3表示wlan id does not exist*/
																			/*返回-4表示wlan is in other L3 interface，返回-5表示wlan should be disable first*/
																			/*返回-6表示error，返回-7示WLAN ID非法*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			
extern int set_wlan_vlan_priority(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *value);  
																				/*返回0表示失败，返回1表示成功，返回-1表示unknown input*/
																				/*返回-2表示input parameter should be 0 to 7，返回-3表示wlan id does not exist*/
																				/*返回-4表示wlan is in other L3 interface，返回-5表示wlan has not binding vlan*/
																				/*返回-6表示wlan should be disable first，返回-7表示wlan is under tunnel wlan-vlan policy*/
																				/*返回-8表示error，返回-9示WLAN ID非法*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																
extern int undo_wlan_vlan_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id);   
																	/*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist*/
																	/*返回-2表示wlan is in other L3 interface，返回-3表示wlan should be disable first*/
																	/*返回-4表示error，返回-5示WLAN ID非法*/
																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_wlan_vlan_info(DCLI_WLAN_API_GROUP *WLANINFO);
/*返回1时，调用Free_wlan_vlan_info()释放空间*/													
extern int show_wlan_vlan_info(dbus_parameter parameter, DBusConnection *connection,int wlan_id,DCLI_WLAN_API_GROUP **WLANINFO); /*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist，返回-2表示error，返回-3示WLAN ID非法*/
																																	  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int wlan_set_bridge_isolation_func(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *STATE);
																						/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																						/*返回-2表示wlan id does not exist，返回-3表示wlan is not wlan if policy，返回-4表示wlan should be disable first*/
																						/*返回-5表示wlan bridge error，返回-6表示system cmd process error*/
																						/*返回-7表示sameportswitch and isolation are conflict,disable sameportswitch first，返回-8表示error，返回-9示WLAN ID非法*/
																						/*返回-10表示apply security in this wlan first*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						
extern int wlan_set_bridge_multicast_isolation_func(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *STATE); 
																									/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																									/*返回-2表示wlan id does not exist，返回-3表示wlan is not wlan if policy，返回-4表示wlan should be disable first*/
																									/*返回-5表示wlan bridge error，返回-6表示system cmd process error*/
																									/*返回-7表示sameportswitch and isolation are conflict,disable sameportswitch first，返回-8表示error，返回-9示WLAN ID非法*/
																									/*返回-10表示apply security in this wlan first*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_bridge_isolation(DCLI_WLAN_API_GROUP *WLANINFO);
/*返回1时，调用Free_bridge_isolation()释放空间*/
extern int wlan_show_bridge_isolation_func(dbus_parameter parameter, DBusConnection *connection,int wlan_id,DCLI_WLAN_API_GROUP **WLANINFO);/*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist，返回-2表示error，返回-3示WLAN ID非法*/
																																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_tunnel_wlan_vlan_cmd_func(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *STATE,char *IFNAME);  
																										/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'add' or 'delete'*/
																										/*返回-2表示input interface name should only start with 'radio',other interface you should use ebr configuration*/
																										/*返回-3表示if name too long，返回-4表示malloc error，返回-5表示wlan id does not exist*/
																										/*返回-6表示wlan is in local wlan-vlan interface，返回-7表示wlan should be disable first，返回-8表示input ifname is wrong*/
																										/*返回-9表示wlan is not in tunnel mode，返回-10表示if is already STATE,or system cmd error，返回-11表示error，返回-12示WLAN ID非法*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_tunnel_wlan_vlan_head(DCLI_WLAN_API_GROUP *WLANINFO);
/*返回1时，调用Free_tunnel_wlan_vlan()释放空间*/
extern int show_tunnel_wlan_vlan_cmd_func(dbus_parameter parameter, DBusConnection *connection,int wlan_id,DCLI_WLAN_API_GROUP **WLANINFO);
																			/*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist*/
																			/*返回-2表示wlan is in local wlan-vlan interface，返回-3表示wlan should be disable first*/
																			/*返回-4表示input ifname is wrong，返回-5表示wlan is not in tunnel mode*/
																			/*返回-6表示add if to br fail，返回-7表示remove if from br fail，返回-8表示error，返回-9示WLAN ID非法*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			
extern int wlan_set_sameportswitch_func(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *spswitch_state);
																								/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																								/*返回-2表示wlan id does not exist，返回-3表示wlan is not wlan if policy，返回-4表示wlan should be disable first*/
																								/*返回-5表示wlan bridge error，返回-6表示system cmd process error*/
																								/*返回-7表示sameportswitch and isolation are conflict,disable isolation first，返回-8表示error，返回-9示WLAN ID非法*/
																								/*返回-10表示apply security in this wlan first*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*type为"wds"或"mesh"*/
/*state为"enable"或"disable"*/																								
extern int config_wds_service_cmd_func(dbus_parameter parameter, DBusConnection *connection,int wlanID,char *type,char *state);  
																						/*返回0表示失败，返回1表示成功，返回-1表示you should bind interface first*/
																						/*返回-2表示you map layer3 interace error，返回-3表示you must first service enable wlan*/
																						/*返回-4表示error，返回-5示WLAN ID非法*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						
extern int set_wlan_essid_func(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *ESSID);
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示essid is too long,out of the limit of 32*/
																			/*返回-2表示wlan id does not exist*/
																			/*返回-3表示wlan is enable,please disable it first*/
																			/*返回-4表示error，返回-5示WLAN ID非法*/
																			/*返回-6表示UNKNOWN COMMAND*/
																			/*返回-7表示illegal essid name!! ` \ \" & * ( ) not supported!*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_wlan_sta_ip_mac_binding_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *state);
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示input patameter only with 'enable' or 'disable'*/
																							/*返回-2表示error，返回-3示WLAN ID非法*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*type为(starttimer|stoptimer)，state为(enable|disable)*/
extern int set_wlan_timer_able_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *time_type,char *state);
																								 /*返回0表示失败，返回1表示成功*/
																								 /*返回-1表示first input patameter only with 'starttimer' or 'stoptimer'*/
																								 /*返回-2表示second input patameter only with 'enable' or 'disable'*/
																								 /*返回-3表示error，返回-4示WLAN ID非法*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*action为(start|stop)，Time's format should be like 12:32:56，Time为(once|cycle)*/
/*argv_num表示输入的weekdays的个数*/
/*char *weekdays[]存放输入的星期*/
extern int set_wlan_servive_timer_func_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *action,char *Time,char *type,int argv_num,char *weekdays[]);
																						 /*返回0表示失败，返回1表示成功*/
																					     /*返回-1表示input patameter only with 'start' or 'stop'*/
																					     /*返回-2表示input patameter format should be 12:32:56*/
																					     /*返回-3表示input patameter only with 'once' or 'cycle'*/
																					     /*返回-4表示weekdays you want (like Sun Mon Tue Wed Thu Fri Sat or hebdomad)*/																																	  
																					     /*返回-5表示error，返回-6示WLAN ID非法*/	
																						 /*返回-7表示the starttimer or stoptimer should be disabled*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*value的范围是0-300000*/
extern int set_whole_wlan_traffic_limit_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *value); 
																						   /*返回0表示失败，返回1表示成功*/
																						   /*返回-1表示input parameter error*/
																						   /*返回-2表示wlan id does not exist*/
																						   /*返回-3表示error，返回-4示WLAN ID非法*/
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						   /*返回-5表示input parameter should be 0~300000*/

/*value的范围是0-300000*/
extern int set_whole_wlan_send_traffic_limit_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *value);
																						   	 	/*返回0表示失败，返回1表示成功*/
																							    /*返回-1表示input parameter error*/
																							    /*返回-2表示wlan id does not exist*/
																							    /*返回-3表示error，返回-4示WLAN ID非法*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								/*返回-5表示input parameter should be 0~300000*/

/*value的范围是0-300000*/																						   
extern int set_whole_wlan_station_average_traffic_limit_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *value);
																											/*返回0表示失败，返回1表示成功*/
																											/*返回-1表示input parameter error*/
																											/*返回-2表示wlan id does not exist*/
																											/*返回-3表示error，返回-4示WLAN ID非法*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											/*返回-5表示input parameter should be 0~300000*/

/*value的范围是0-300000*/
extern int set_whole_wlan_station_average_send_traffic_limit_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *value);
																												   /*返回0表示失败，返回1表示成功*/
																												   /*返回-1表示input parameter error*/
																												   /*返回-2表示wlan id does not exist*/
																												   /*返回-3表示error，返回-4示WLAN ID非法*/
																												   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																												   /*返回-5表示input parameter should be 0~300000*/

extern void Free_show_all_wlan_wapi_basic_information_cmd(struct ConfigWapiInfo *WlanHead);		
/*只要调用，就通过Free_show_all_wlan_wapi_basic_information_cmd()释放空间*/
extern int show_all_wlan_wapi_basic_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct ConfigWapiInfo **WlanHead);
																											   /*返回0表示失败，返回1表示成功*/
																											   /*返回-1表示There is no Wlan now*/
																											   /*返回-2表示There is no Wapi Wlan now*/
																											   /*返回-3表示error*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wlan_ssid_config_information_cmd(struct SSIDConfigInfo *WlanHead);																											   
/*当*WlanHead不为空时，调用Free_show_all_wlan_ssid_config_information_cmd()释放空间*/
extern int show_all_wlan_ssid_config_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct SSIDConfigInfo **WlanHead);
																												/*返回0表示失败，返回1表示成功*/
																											    /*返回-1表示there is no wlan*/
																											    /*返回-2表示error*/
																												/*返回-3表示security profile does not exist*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wlan_unicast_information_cmd(struct UnicastInfo *WlanHead);
/*只要调用，就通过Free_show_all_wlan_unicast_information_cmd()释放空间*/
extern int show_all_wlan_unicast_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct UnicastInfo **WlanHead);
																										/*返回0表示失败，返回1表示成功*/
																									    /*返回-1表示There is no Wlan now*/
																									    /*返回-2表示There is no Wapi Wlan now*/
																									    /*返回-3表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																												
extern void Free_show_all_wlan_wapi_performance_stats_information_cmd(struct WtpWAPIPerformanceStatsInfo *WlanHead);
/*只要调用，就通过Free_show_all_wlan_wapi_performance_stats_information_cmd()释放空间*/
extern int show_all_wlan_wapi_performance_stats_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpWAPIPerformanceStatsInfo **WlanHead);
																					   /*返回0表示失败，返回1表示成功*/
																					   /*返回-1表示There is no Wlan now*/
																					   /*返回-2表示There is no Wapi Wlan now*/
																					   /*返回-3表示error*/
																					   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																																		   
extern void Free_show_all_wlan_wapi_extend_config_information_cmd(struct WtpWAPIExtendConfigInfo *WlanHead);
/*只要调用，就通过Free_show_all_wlan_wapi_extend_config_information_cmd()释放空间*/
extern int show_all_wlan_wapi_extend_config_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpWAPIExtendConfigInfo **WlanHead);
																								  /*返回0表示失败，返回1表示成功*/
																								  /*返回-1表示There is no Wlan now*/
																								  /*返回-2表示There is no Wapi Wlan now*/
																								  /*返回-3表示error*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wlan_bss_wapi_performance_stats_information_cmd(struct BssWAPIPerformanceStatsInfo *WlanHead);
/*只要调用，就通过Free_show_all_wlan_bss_wapi_performance_stats_information_cmd()释放空间*/
extern int show_all_wlan_bss_wapi_performance_stats_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct BssWAPIPerformanceStatsInfo **WlanHead);
																									/*返回0表示失败，返回1表示成功*/
																									/*返回-1表示There is no Wlan now*/
																									/*返回-2表示There is no Wapi Wlan now*/
																									/*返回-3表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_wlan_ascii_essid_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,unsigned char *new_essid); 
																								/*返回0表示失败，返回1表示成功*/
																								/*返回-1表示essid is too long,out of the limit of 64*/
																								/*返回-2表示wlan id does not exist*/
																								/*返回-3表示wlan is enable,please disable it first*/
																								/*返回-4表示error，返回-5示WLAN ID非法*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_wlan_of_all_cmd(struct WLAN_INFO *wlan_info);
/*只要调用，就通过Free_show_wlan_of_all_cmd()释放空间*/
extern int show_wlan_of_all_cmd(dbus_parameter parameter, DBusConnection *connection,struct WLAN_INFO **wlan_info);
																				   /*返回0表示失败，返回1表示成功*/
																				   /*返回-1表示wlan id does not exist*/
																				   /*返回-2表示error*/

/*Switch的取值是"enable"或"disable"*/
extern int set_wlan_bss_multi_user_optimize_cmd(dbus_parameter parameter, DBusConnection *connection,int wlan_id,char *Switch);
																			   /*返回0表示失败，返回1表示成功*/
																			   /*返回-1表示WLAN ID非法，返回-2表示bss not exist*/
																			   /*返回-3表示operation fail，返回-4表示wlan is not binded radio*/
																			   /*返回-5表示error*/

/*wlan_id为0表示全局配置*/
/*state的取值是"enable"或"disable"*/
extern int set_wlan_not_response_sta_probe_request_cmd(dbus_parameter parameter, DBusConnection *connection,int wlanID,char *state);
																			   /*返回0表示失败，返回1表示成功*/
																			   /*返回-1表示input patameter only with 'enable' or 'disable'*/
																			   /*返回-2表示WLAN ID非法，返回-3表示wlan does not exist*/
																			   /*返回-4表示wlan is enable, please disable it first*/
																			   /*返回-5表示you want to some wlan, and the operation of the wlan was not successful*/
																			   /*返回-6表示error*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*Type的取值是"tunnel"或"local"*/
extern int set_wlan_tunnel_mode_enable_cmd(dbus_parameter parameter, DBusConnection *connection,int wlanID,char *Type);
																			   /*返回0表示失败，返回1表示成功*/
																			   /*返回-1表示WLAN ID非法*/
																			   /*返回-2表示wlan is enable, please disable it first*/
																			   /*返回-3表示wlanid is not exist*/
																			   /*返回-4表示you want to delete wlan, please do not operate like this*/
																			   /*返回-5表示some radio interface in ebr*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/





/*WTP*/												  
extern void CheckWTPState(char *state, unsigned char WTPstate);
extern void CheckWTPQuitReason(char *quitreason,unsigned char quitstate);
//modify by qiaojie
#if 0
extern void DcliWReInit();   
#endif
extern void DcliWInit();
extern int wtp_check_wtp_ip_addr(char *ipaddr, char *WTPIP);
extern int create_wtp(dbus_parameter parameter, DBusConnection *connection,char * id, char *wtp_name, char* wtp_model, char *wtp_sn);  
																									/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																									/*返回-2表示wtp id should be 1 to WTP_NUM-1，返回-3表示wtp name is too long,should be 1 to DEFAULT_LEN-1*/
																									/*返回-4表示the model is wrong，返回-5表示wtp id exist*/
																									/*返回-6表示create wtp count reach to max wtp count，返回-7表示wtp sn exist*/
																									/*返回-8表示error，返回-9表示wtp sn is too long,should be 1 to 127*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																									
extern int create_wtp_bymac_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *id, char *wtp_name, char* wtp_model, char *wtp_mac); 
																									/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																									/*返回-2表示wtp id should be 1 to WTP_NUM-1，返回-3表示wtp name is too long,should be 1 to DEFAULT_LEN-1*/
																									/*返回-4表示Unknow mac addr format，返回-5表示input should not be broadcast or multicast mac*/
																									/*返回-6表示the model is wrong，返回-7表示wtp id exist*/
																									/*返回-8表示create wtp count reach to max wtp count，返回-9表示wtp mac exist，返回-10表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																														
extern int delete_wtp(dbus_parameter parameter, DBusConnection *connection,int id); /*返回0表示删除失败，返回1表示删除成功*/
																					 /*返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																					 /*返回-2表示wtp id not exist，返回-3表示please unused first*/
																					 /*返回-4表示error，返回-5表示input wtp has some radios interface in ebr,please delete it first*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_one_wtp_head(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用Free_one_wtp_head()释放空间*/
extern int show_wtp_one(dbus_parameter parameter, DBusConnection *connection,int id,DCLI_WTP_API_GROUP_ONE **WTPINFO);
																					  /*返回0表示失败，返回1表示成功*/
		                                                                              /*返回-1表示wtp id should be 1 to WTP_NUM-1*/
																					  /*返回-2表示wtp不存在，返回-3表示error*/		
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					  
extern void Free_wtp_list_new_head(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用Free_wtp_list_new_head()释放空间*/
extern int show_wtp_list_new_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示no wtp exist*/
																																	 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					  
extern void Free_wtp_list_by_sn_head(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用Free_wtp_list_by_sn_head()释放空间*/
extern int show_wtp_list_by_sn_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);  /*返回0表示失败，返回1表示成功*/
																																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_wtp_list_by_mac_head(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用Free_wtp_list_by_mac_head()释放空间*/
extern int show_wtp_list_by_mac_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);/*返回0表示失败，返回1表示成功*/
																																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_wtp_apply_interface_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_wtp_apply_interface_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int wtp_apply_interface_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *inter_name,struct WtpList **WtpList_Head);
						/*返回0表示失败，返回1表示成功，返回-1表示the length of interface name excel 16*/
						/*返回-2表示interface does not exist，返回-3表示if you want to change binding interface, please delete binding wlan id first*/
						/*返回-4表示error，返回-5示WTP ID非法，返回-6表示Group ID非法，返回-7表示partial failure*/
						/*返回-8表示is no local interface, permission denial*/
#endif

extern int wtp_apply_interface(dbus_parameter parameter, DBusConnection *connection,int id,char *inter_name);  
																			  /*返回0表示失败，返回1表示成功，返回-1表示the length of interface name excel 16*/
																			  /*返回-2表示interface does not exist，返回-3表示if you want to change binding interface, please delete binding wlan id first*/
																			  /*返回-4表示error，返回-5示WTP ID非法*/
																			  /*返回-8表示is no local interface, permission denial*/
																			  /*返回-9表示if you want to change binding interface, please unused wtp  first*/
																			  /*返回-10表示interface has be binded in other hansi*/
																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			  
extern int wtp_apply_ipv6interface(dbus_parameter parameter, DBusConnection *connection,int id,char *inter_name);
																			    /*返回0表示失败，返回1表示成功*/
																				/*返回-1表示the length of input parameter is excel the limit of 16*/
																				/*返回-2表示wtp id does not exist，返回-3表示wtp is using*/
																				/*返回-4表示interface does not exist，返回-5表示apply interface failed*/
																				/*返回-6表示if you want to change binding interface, please delete binding wlan id first*/
																				/*返回-7表示wtp bingding ipv6 addr error make sure interface have ipv6 address*/
																				/*返回-8表示error，返回-9示WTP ID非法*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_wtp_apply_wlan_group(struct WtpList *WtpList_Head);

/*返回-15时，调用Free_wtp_apply_wlan_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断wlan ID的合法性1--15*/
extern int wtp_apply_wlan_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int wlanID,struct WtpList **WtpList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed*/
										/*返回-2表示WTP be using, you can't binding wlan ID，返回-3表示 binding wlan is not exist*/
										/*返回-4表示Wlan does not binding interface，返回-5表示 Wtp does not binding interface*/
										/*返回-6表示wlan and wtp binding interface don't match，返回-7表示 Clear wtp binding wlan list successfully*/
										/*返回-8表示wlan being enable，返回-9表示wtp over max bss count*/
										/*返回-10表示wtp over max wep wlan count，返回-11表示 error，返回-12示WTP ID非法*/
										/*返回-13表示wep conflict, wtp binding wlan securityindex is same with others，返回-14表示Group ID非法*/
										/*返回-15表示partial failure*/
#endif

extern int wtp_apply_wlan(dbus_parameter parameter, DBusConnection *connection,int wtpID,int wlanID);   
																	   /*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed*/
				                                                       /*返回-2表示WTP be using, you can't binding wlan ID，返回-3表示 binding wlan is not exist*/
				                                                       /*返回-4表示Wlan does not binding interface，返回-5表示 Wtp does not binding interface*/
				                                                       /*返回-6表示wlan and wtp binding interface don't match，返回-7表示 Clear wtp binding wlan list successfully*/
																	   /*返回-8表示wlan being enable，返回-9表示wtp over max bss count*/
																	   /*返回-10表示wtp over max wep wlan count，返回-11表示 error，返回-12示WTP ID非法*/
																	   /*返回-13表示wep conflict, wtp binding wlan securityindex is same with others*/
																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*wlanID==0，表示all*/																	   
extern int wtp_delete_wlan(dbus_parameter parameter, DBusConnection *connection,int wtpID,int wlanID);    
																	    /*返回0表示失败，返回1表示成功*/
				                                                        /*返回-1表示 wtp id not exist，返回-2表示WTP be using, you can't binding wlan ID*/
				                                                        /*返回-3表示wlan is not exist，返回-4表示 Wlan does not binding interface*/
				                                                        /*返回-5表示Wtp does not binding interface，返回-6表示 wlan and wtp binding interface don't match*/
				                                                        /*返回-7表示Clear wtp binding wlan list successfully，返回-8表示 wlan being enable*/
				                                                        /*返回-9表示BSS being enable，返回-10表示wtp doesn't binding this wlan id，返回-11表示 error*/
																		/*返回-12示WTP ID非法，返回-13表示wtp interface is in ebr,please delete it from ebr first*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_wtp_disable_wlan_group(struct WtpList *WtpList_Head);

/*返回-8时，调用Free_wtp_disable_wlan_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断wlan ID的合法性1--15*/
extern int wtp_disable_wlan_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int wlanID,struct WtpList **WtpList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed*/
										/*返回-2表示binding wlan is not exist，返回-3表示 Wtp does not binding this wlan*/
										/*返回-4表示wtp binding wlan id not match，返回-5表示 error，返回-6示WTP ID非法*/
										/*返回-7表示Group ID非法，返回-8表示partial failure，返回-9表示group id does not exist*/
#endif

extern int wtp_disable_wlan(dbus_parameter parameter, DBusConnection *connection,int wtpID,int wlanID);	
																		/*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed*/
																	 	/*返回-2表示binding wlan is not exist，返回-3表示 Wtp does not binding this wlan*/
																		/*返回-4表示wtp binding wlan id not match，返回-5表示 error，返回-6示WTP ID非法*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_wtp_enable_wlan_group(struct WtpList *WtpList_Head);

/*返回-13时，调用Free_wtp_enable_wlan_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断wlan ID的合法性1--15*/
extern int wtp_enable_wlan_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int wlanID,struct WtpList **WtpList_Head);
														/*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed*/
														/*返回-2表示binding wlan is not exist，返回-3表示 Wtp does not binding this wlan*/
														/*返回-4表示wtp not in run state，返回-5表示 wtp binding wlan id not match*/
														/*返回-6表示Wlan is not enable，返回-7表示map L3 interace no ip error*/
														/*返回-8表示BSS interface policy conflict，返回-9代表wtp over max wep wlan count*/
														/*返回-10表示 error，返回-11示WTP ID非法，返回-12表示Group ID非法*/
														/*返回-13表示partial failure，返回-14表示group id does not exist*/
#endif

extern int wtp_enable_wlan(dbus_parameter parameter, DBusConnection *connection,int wtpID,int wlanID);	
																		/*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed*/
																   	    /*返回-2表示binding wlan is not exist，返回-3表示 Wtp does not binding this wlan*/
																		/*返回-4表示wtp not in run state，返回-5表示 wtp binding wlan id not match*/
																		/*返回-6表示Wlan is not enable，返回-7表示map L3 interace no ip error*/
																		/*返回-8表示BSS interface policy conflict，返回-9代表wtp over max wep wlan count*/
																		/*返回-10表示 error，返回-11示WTP ID非法*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_wtp_used_group(struct WtpList *WtpList_Head);

/*返回-9时，调用Free_wtp_used_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*use_sta=0表示"used"，use_sta=1表示"unused"*/
extern int wtp_used_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int use_sta,struct WtpList **WtpList_Head);
				/*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed，返回-2表示You should be apply interface first*/
				/*返回-3表示You should be apply wlan id first，返回-4表示map L3 interace error，返回-5表示BSS interface policy conflict*/
				/*返回-6表示error，返回-7示WTP ID非法，返回-8表示Group ID非法，返回-9表示partial failure*/
#endif

extern int wtp_used(dbus_parameter parameter, DBusConnection *connection,int id,int use_sta);
														   /*返回0表示失败，返回1表示成功，返回-1表示WTP ID Not existed，返回-2表示You should be apply interface first*/
				                                           /*返回-3表示You should be apply wlan id first，返回-4表示map L3 interace error，返回-5表示BSS interface policy conflict*/
				                                           /*返回-6表示error，返回-7示WTP ID非法*/
														   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_wtp_model(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用Free_wtp_model()释放空间*/
extern  int show_version(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);/*返回1表示成功，返回0表示失败*/
																												  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern  int delete_model(dbus_parameter parameter, DBusConnection *connection,char*model);/*返回0表示失败，返回1表示成功*/
											 											   /*返回-1表示model does not exist or the model is system default model*/
											 											   /*返回-2表示error*/            
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern  int set_ap_model(dbus_parameter parameter, DBusConnection *connection,char*model,char*ver,char*path,char* radio,char *bss);  /*返回0表示失败，返回1表示成功，返回-1表示wireless-control does not surport this model，返回-2表示error*/
													   																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*echotime的范围是3-30*/
/*wtp_id为0时，表示全局配置*/
extern  int set_ap_echotimer(dbus_parameter parameter, DBusConnection *connection,int wtp_id,int echotime); /*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示WTP ID非法，返回-3表示input echotimer should be 3~30*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_ap_echotimer(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*retu =1 的时候调用释放函数free_show_ap_echotimer(DCLI_WTP_API_GROUP_THREE *WTPINFO)*/
extern 	int show_ap_echotimer(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_THREE **WTPINFO); /*返回0表示失败，返回1表示成功*/
																															 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int download_ap_version(char *url,char *user,char *passwd);//下载ap的最新版本

#if _GROUP_POLICY
extern void Free_config_wtp_max_sta_num_group(struct WtpList *WtpList_Head);

/*返回-8时，调用Free_config_wtp_max_sta_num_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int config_wtp_max_sta_num_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char*max_num,struct WtpList **WtpList_Head,int *fail_num);  
															/*返回0表示失败，返回1表示成功，返回-1表示wtp not exist*/
															/*返回-2表示more sta(s) has accessed before you set max sta num*/
															/*返回-3表示operation fail，返回-4表示error，返回-5示WTP ID非法*/
															/*返回-6表示unknown NUM format，返回-7表示Group ID非法*/
															/*返回-8表示partial failure，返回-9表示group id does not exist*/
															/*返回-10表示input num should be 0-64*/
#endif
/*max_num的范围是0-32767*/
extern int config_wtp_max_sta_num(dbus_parameter parameter, DBusConnection *connection,int id,char*max_num);  
																				/*返回0表示失败，返回1表示成功，返回-1表示wtp not exist*/
																				/*返回-2表示more sta(s) has accessed before you set max sta num*/
																				/*返回-3表示operation fail，返回-4表示error，返回-5示WTP ID非法*/
																				/*返回-10表示input num should be 0-32767*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_wtp_triger_num_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_config_wtp_triger_num_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*triger_num的范围是1-64*/
extern int config_wtp_triger_num_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char* triger_num,struct WtpList **WtpList_Head);
												/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist*/
												/*返回-2表示operation fail，返回-3表示triger number must be little than max sta num*/
												/*返回-4表示error，返回-5示WTP ID非法，返回-6表示Group ID非法*/
												/*返回-7表示partial failure，返回-8表示group id does not exist*/
												/*返回-9表示unknown id format，返回-10表示input triger num should be 1~64*/
#endif

/*triger_num的范围是1-64*/
extern int config_wtp_triger_num(dbus_parameter parameter, DBusConnection *connection,int id,char* triger_num);	/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示operation fail，返回-3表示triger number must be little than max sta num，返回-4表示error，返回-5示WTP ID非法*/
																													/*返回-9表示unknown id format，返回-10表示input triger num should be 1~64*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																				
#if _GROUP_POLICY
extern void Free_set_wtp_flow_trige_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_wtp_flow_trige_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*triger_num的范围是0-1024*/
extern int set_wtp_flow_trige_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char* triger_num,struct WtpList **WtpList_Head);
											/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist*/
											/*返回-2表示operation fail，返回-3表示flow triger must be <0-1024>*/
											/*返回-4表示error，返回-5示WTP ID非法，返回-6表示Group ID非法*/
											/*返回-7表示partial failure，返回-8表示group id does not exist*/
											/*返回-9表示unknown id format，返回-10表示input flow triger should be 0~1024*/
#endif

extern int set_wtp_flow_trige(dbus_parameter parameter, DBusConnection *connection,int id,char* triger_num);  /*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示operation fail，返回-3表示flow triger must be <0-1024>，返回-4表示error，返回-5示WTP ID非法*/
																												/*返回-9表示unknown id format，返回-10表示input flow triger should be 0~1024*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_max_throughout_group(struct WtpList *WtpList_Head);

/*返回-8时，调用Free_set_ap_max_throughout_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_max_throughout_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *para,struct WtpList **WtpList_Head);
													/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
													/*返回-2表示ap max throughout should be 1 to 108，返回-3表示wtp id does not exist*/
													/*返回-4表示error，返回-5示WTP ID非法*/
													/*返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
													/*返回-7表示Group ID非法，返回-8表示partial failure*/
													/*返回-9表示group id does not exist*/
#endif

extern int set_ap_max_throughout(dbus_parameter parameter, DBusConnection *connection,int id,char *para); /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																											   /*返回-2表示ap max throughout should be 1 to 108，返回-3表示wtp id does not exist*/
																											   /*返回-4表示error，返回-5示WTP ID非法*/
																											   /*返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_extension_command_func_group(struct WtpList *WtpList_Head);

/*返回-6时，调用Free_set_ap_extension_command_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*该命令是在AC上直接操作AP的一个隐藏命令，不开放给用户，WEB上不用实现。*/
extern int set_ap_extension_command_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,const char *command,struct WtpList **WtpList_Head);
												   /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
												   /*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
												   /*返回-5表示Group ID非法，返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

extern int set_ap_extension_command_func(dbus_parameter parameter, DBusConnection *connection,int WtpId,const char *command);
																										/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																										/*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										
extern int set_ap_ip_address(dbus_parameter parameter, DBusConnection *connection,int id,char *IP,char *MASK,char *GATEWAY);  
																							/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format，返回-2表示unknown id format*/
																							/*返回-3表示unknown mask parameters，返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
																							/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										
extern void free_show_wtp_runtime(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*retu==1调用释放函数free_show_wtp_runtime(DCLI_WTP_API_GROUP_THREE *WTPINFO)*/
extern int show_wtp_runtime(dbus_parameter parameter, DBusConnection *connection,int id,DCLI_WTP_API_GROUP_THREE **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示WTP ID非法*/
																																/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_wtp_location_group(struct WtpList *WtpList_Head);

/*返回-5时，调用Free_set_wtp_location_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_wtp_location_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Location,struct WtpList **WtpList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示wtp location is too long,should be 1 to 256*/
										/*返回-2表示wtp id does not exist，返回-3示WTP ID非法，返回-4表示Group ID非法*/
										/*返回-5表示partial failure，返回-6表示group id does not exist*/
#endif

extern int set_wtp_location(dbus_parameter parameter, DBusConnection *connection,int id,char *Location); /*返回0表示失败，返回1表示成功，返回-1表示wtp location is too long,should be 1 to 256，返回-2表示wtp id does not exist，返回-3示WTP ID非法*/
																										   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																		
extern void free_show_wtp_location(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*返回1时，调用free_show_wtp_location()释放空间*/
extern int show_wtp_location(dbus_parameter parameter, DBusConnection *connection,int id,DCLI_WTP_API_GROUP_THREE **WTPINFO);   /*返回0表示失败，返回1表示成功，返回-1表示wtp does not set location，返回-2表示wtp id does not exist，返回-3示WTP ID非法*/
																																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_ip_bywtpid(DCLI_AC_API_GROUP_THREE *network);
/*返回1时，调用Free_ap_ip_bywtpid()释放空间*/
extern int show_ap_ip_bywtpid_cmd(dbus_parameter parameter, DBusConnection *connection,int id,DCLI_AC_API_GROUP_THREE **network);
																							   /*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1*/
																							   /*返回-2表示ap have not ip information，返回-3表示wtp id no exist*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_ap_model_infomation(DCLI_AC_API_GROUP_FOUR *modelinfo);
/*返回1时，调用Free_ap_model_infomation()释放空间*/																								   
extern int show_ap_model_infomation(dbus_parameter parameter, DBusConnection *connection,char *modetype,DCLI_AC_API_GROUP_FOUR **modelinfo);
																								/*返回0表示失败，返回1表示成功，返回-1表示input model is wrong*/
																							    /*返回-2表示ac support model does not set，返回-3表示this model does not supportted, set it first*/
																							    /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_wtp_netid_group(struct WtpList *WtpList_Head);

/*返回-5时，调用Free_set_wtp_netid_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_wtp_netid_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char*content,struct WtpList **WtpList_Head);
													  /*返回0表示失败，返回1表示成功*/
													  /*返回-1表示wtp location is too long,should be 1 to 32*/
													  /*返回-2表示wtp id does not exist，返回-3示WTP ID非法*/
													  /*返回-4表示Group ID非法，返回-5表示partial failure*/
													  /*返回-6表示group id does not exist*/
#endif

extern int set_wtp_netid(dbus_parameter parameter, DBusConnection *connection,int id,char*content);
																/*返回0表示失败，返回1表示成功*/
																/*返回-1表示wtp location is too long,should be 1 to 32*/
																/*返回-2表示wtp id does not exist，返回-3示WTP ID非法*/
																/*返回SNMPD_CONNECTION_ERROR表示connection error*/
														
extern void free_show_wtp_netid(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*retu ==1 调用释放函数free_show_wtp_netid(DCLI_WTP_API_GROUP_THREE *WTPINFO)*/																
extern int show_wtp_netid(dbus_parameter parameter, DBusConnection *connection,int id,DCLI_WTP_API_GROUP_THREE **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp does not set netid，返回-2表示 wtp id does not exist，返回-3示WTP ID非法*/
																															  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																
extern int show_wtp_extension_information(int id,wid_wifi_info *head);

extern void free_show_wtp_sample_throughput_information(DCLI_WTP_API_GROUP_TWO *INFO);
/*retu==1 调用释放函数free_show_wtp_sample_throughput_information(DCLI_WTP_API_GROUP_TWO *INFO)*/
extern int show_wtp_sample_throughput_information(dbus_parameter parameter, DBusConnection *connection,int id,DCLI_WTP_API_GROUP_TWO **INFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp is not in run state，返回-2表示wtp id does not exist，返回-3表示sample switch disable,enable it first，返回-4示WTP ID非法*/
																																					   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_wtp_wtpname_group(struct WtpList *WtpList_Head);

/*返回-5时，调用Free_set_wtp_wtpname_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_wtp_wtpname_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char * wtpname,struct WtpList **WtpList_Head);
																/*返回0表示失败，返回1表示成功*/
																/*返回-1表示wtp name is too long,should be 1 to DEFAULT_LEN-1*/
																/*返回-2表示wtp id does not exist，返回-3示WTP ID非法*/
																/*返回-4表示Group ID非法，返回-5表示partial failure*/
																/*返回-6表示group id does not exist*/
#endif

extern int set_wtp_wtpname(dbus_parameter parameter, DBusConnection *connection,int id,char * wtpname);
																	   /*返回0表示失败，返回1表示成功*/
																	   /*返回-1表示wtp name is too long,should be 1 to DEFAULT_LEN-1*/
																	   /*返回-2表示wtp id does not exist，返回-3示WTP ID非法*/
																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_extension_infomation_enable_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_ap_extension_infomation_enable_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_extension_infomation_enable_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct WtpList **WtpList_Head);
						   /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
						   /*返回-2表示wtp id does not exist，返回-3表示wtp id does not run，返回-4表示error*/
						   /*返回-5示WTP ID非法，返回-6表示Group ID非法，返回-7表示partial failure*/
						   /*返回-8表示group id does not exist*/
#endif

extern int set_ap_extension_infomation_enable(dbus_parameter parameter, DBusConnection *connection,int wtp_id,char *state);
																					/*返回0表示失败，返回1表示成功*/
																					/*返回-1表示input patameter only with 'enable' or 'disable'*/
																					/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run*/
																					/*返回-4表示error，返回-5示WTP ID非法*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_extension_infomation_reportinterval_group(struct WtpList *WtpList_Head);

/*返回-9时，调用Free_set_ap_extension_infomation_reportinterval_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_extension_infomation_reportinterval_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *para,struct WtpList **WtpList_Head);
									/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
									/*返回-2表示ap extension infomation reportinterval error，返回-3表示wtp is not in run state*/
									/*返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
									/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
									/*返回-8表示Group ID非法，返回-9表示partial failure，返回-10表示group id does not exist*/
#endif

extern int set_ap_extension_infomation_reportinterval(dbus_parameter parameter, DBusConnection *connection,int WTPID,char *para);  
																									/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																									/*返回-2表示ap extension infomation reportinterval error，返回-3表示wtp is not in run state*/
																									/*返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
																									/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_wtp_wlan_vlan_information(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*retu==1,调用free_show_wtp_wlan_vlan_information(DCLI_WTP_API_GROUP_THREE *WTPINFO)*/																									
extern int show_wtp_wlan_vlan_information(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_THREE **WTPINFO);  /*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示WTP ID非法*/
																																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int show_wtp_extension_information_v2(int wtp_id,wid_wifi_info *exten_info);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示wtp extension info report switch disable，返回-3示WTP ID非法*/

extern void free_how_wtp_extension_information_v3(DCLI_WTP_API_GROUP_TWO *WTPINFO);
/*retu==1 调用释放函数free_how_wtp_extension_information_v3(DCLI_WTP_API_GROUP_TWO *WTPINFO)*/ 																									
extern int show_wtp_extension_information_v3(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_TWO **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示wtp extension info report switch disable，返回-3示WTP ID非法*/
																																					   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int show_wtp_ethernet_interface_information(dbus_parameter parameter, DBusConnection *connection,int wtp_id,struct wtp_eth_inf_infor_profile *eth_inf_info_head,int *NUM,int *temperature);  /*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示WTP ID非法*/
																																					   													   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_l2_isolation_func_group(struct WtpList *WtpList_Head);

/*返回-10时，调用Free_set_ap_l2_isolation_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*wlan_id的范围是1-15*/
/*state为"enable"或"disable"*/
extern int set_ap_l2_isolation_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int wlan_id,int state,struct WtpList **WtpList_Head);
																	/*返回0表示失败，返回1表示成功*/
																	/*返回-1表示input patameter only with 'enable' or 'disable'*/
																	/*返回-2表示wlan not exist，返回-3表示wtp not binding wlan*/
																	/*返回-4表示wtp id does not run，返回-5表示binding wlan error*/
																	/*返回-6表示error，返回-7示WTP ID非法*/
																	/*返回-8表示wlan l2 isolation state already，返回-9表示Group ID非法*/
																	/*返回-10表示partial failure，返回-11表示group id does not exist*/
																	/*返回-12表示input wlanid should be 1~15*/
#endif

/*wlan_id的范围是1-15*/
extern int set_ap_l2_isolation_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,int wlan_id,int state);	
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示input patameter only with 'enable' or 'disable'*/
																							/*返回-2表示wlan not exist，返回-3表示wtp not binding wlan*/
																							/*返回-4表示wtp id does not run，返回-5表示binding wlan error*/
																							/*返回-6表示error，返回-7示WTP ID非法*/
																							/*返回-8表示wlan l2 isolation state already*/
																							/*返回-12表示input wlanid should be 1~15*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_dos_def_func_group(struct WtpList *WtpList_Head);

/*返回-6时，调用Free_set_ap_dos_def_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为1表示state，state为0表示disable*/
extern int set_ap_dos_def_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int state,struct WtpList **WtpList_Head);
						/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
						/*返回-2表示wtp id does not run，返回-3表示error，返回-4示WTP ID非法*/
						/*返回-5表示Group ID非法，返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

/*state为1表示enable，state为0表示disable*/																							
extern int set_ap_dos_def_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,int state); /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示wtp id does not run，返回-3表示error，返回-4示WTP ID非法*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_igmp_snoop_func_group(struct WtpList *WtpList_Head);

/*返回-6时，调用Free_set_ap_igmp_snoop_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为1表示enable，state为0表示disable*/
extern int set_ap_igmp_snoop_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int state,struct WtpList **WtpList_Head);
					/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
					/*返回-2表示wtp id does not run，返回-3表示error，返回-4示WTP ID非法*/
					/*返回-5表示Group ID非法，返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

/*state为1表示enable，state为0表示disable*/																							
extern int set_ap_igmp_snoop_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,int state); /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'，返回-2表示wtp id does not run，返回-3表示error，返回-4示WTP ID非法*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_ap_mib_information(DCLI_WTP_API_GROUP_TWO *WTPINFO);
/*retu==1,调用释放函数free_show_ap_mib_information(DCLI_WTP_API_GROUP_TWO *WTPINFO)*/
extern int show_ap_mib_information_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_TWO **WTPINFO); /*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示WTP ID非法*/
																																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_ap_cm_statistics(DCLI_WTP_API_GROUP_TWO *WTPINFO);
/*返回1时，调用free_show_ap_cm_statistics()释放空间*/
extern int show_ap_cm_statistics_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_TWO **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示WTP ID非法*/
																																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_sta_infomation_report_enable_func_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_ap_sta_infomation_report_enable_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_sta_infomation_report_enable_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct WtpList **WtpList_Head);
									/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
									/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run，返回-4表示error*/
									/*返回-5示WTP ID非法，返回-6表示Group ID非法，返回-7表示partial failure*/
									/*返回-8表示group id does not exist*/
#endif

/*wtp_id为0表示全局配置*/
extern int set_ap_sta_infomation_report_enable_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,char *state); 
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示input patameter only with 'enable' or 'disable'*/
																							/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run*/
																							/*返回-4表示error，返回-5示WTP ID非法*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_sta_infomation_reportinterval_cmd_func_group(struct WtpList *WtpList_Head);

/*返回-9时，调用Free_set_ap_sta_infomation_reportinterval_cmd_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_sta_infomation_reportinterval_cmd_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *para,struct WtpList **WtpList_Head);
												/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
												/*返回-2表示ap sta infomation reportinterval error，返回-3表示wtp is not in run state*/
												/*返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
												/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
												/*返回-8表示Group ID非法，返回-9表示partial failure*/
												/*返回-10表示group id does not exist*/
#endif

/*para的范围是5-32767*/
extern int set_ap_sta_infomation_reportinterval_cmd_func(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *para); 
																								/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																								/*返回-2表示ap sta infomation reportinterval error,should be 5-32767，返回-3表示wtp is not in run state*/
																								/*返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
																								/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_if_info_report_enable_func_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_ap_if_info_report_enable_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_ap_if_info_report_enable_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct WtpList **WtpList_Head);
									/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
									/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run，返回-4表示error*/
									/*返回-5示WTP ID非法，返回-6表示Group ID非法，返回-7表示partial failure*/
									/*返回-8表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_ap_if_info_report_enable_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,char *state); 
																							/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
																							/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run，返回-4表示error，返回-5示WTP ID非法*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_if_info_reportinterval_cmd_func_group(struct WtpList *WtpList_Head);

/*返回-9时，调用Free_set_ap_if_info_reportinterval_cmd_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_if_info_reportinterval_cmd_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *para,struct WtpList **WtpList_Head);
													/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
													/*返回-2表示ap sta infomation reportinterval error，返回-3表示wtp is not in run state*/
													/*返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
													/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
													/*返回-8表示Group ID非法，返回-9表示partial failure，返回-10表示group id does not exist*/
#endif

extern int set_ap_if_info_reportinterval_cmd_func(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *para);   
																						/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																						/*返回-2表示ap sta infomation reportinterval error，返回-3表示wtp is not in run state*/
																						/*返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
																						/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_if_updown_func_group(struct WtpList *WtpList_Head);

/*返回-9时，调用Free_set_ap_if_updown_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*inter_type为"eth"或"wifi"*/
/*state为"uplink"或"downlink"*/
extern int set_ap_if_updown_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *inter_type,char *If_index,char *state,struct WtpList **WtpList_Head);
								/*返回0表示失败，返回1表示成功，返回-1表示input interface only with 'ath' 'eth' or 'wifi'*/
								/*返回-2表示unknown id format，返回-3表示input interface only with 'uplink' or 'downlink'*/
								/*返回-4表示wtp is not in run state，返回-5表示wtp id does not exist，返回-6表示error*/
								/*返回-7示WTP ID非法，返回-8表示Group ID非法，返回-9表示partial failure*/
								/*返回-10表示group id does not exist*/
#endif

extern int set_ap_if_updown_func(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *inter_type,char *id,char *state); 
																						/*返回0表示失败，返回1表示成功，返回-1表示input interface only with 'ath' 'eth' or 'wifi'*/
																						/*返回-2表示unknown id format，返回-3表示input interface only with 'uplink' or 'downlink'*/
																						/*返回-4表示wtp is not in run state，返回-5表示wtp id does not exist，返回-6表示error，返回-7示WTP ID非法*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_if_rate_cmd_group(struct WtpList *WtpList_Head);

/*返回-9时，调用Free_set_ap_if_rate_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*rate的范围是"10","100"或"1000"*/
extern int set_ap_if_rate_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *if_index,char *if_rate,struct WtpList **WtpList_Head);
											/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
											/*返回-2表示input interface only with '10'  or '100' or '1000'，返回-3表示wtp is not in run state*/
											/*返回-4表示wtp id does not exist，返回-5表示eth if_index does not exist*/
											/*返回-6表示error，返回-7示WTP ID非法，返回-8表示Group ID非法*/
											/*返回-9表示partial failure，返回-10表示group id does not exist*/
#endif

/*WtpID为0表示全局配置*/
/*rate的范围是"10","100"或"1000"*/
extern int set_ap_if_rate_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *if_index,char *if_rate);
																						  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																						  /*返回-2表示input interface only with '10' or '100' or '1000'，返回-3表示wtp is not in run state*/
																						  /*返回-4表示wtp id does not exist，返回-5表示eth if_index does not exist*/
																						  /*返回-6表示error，返回-7示WTP ID非法*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_ap_if_info(DCLI_WTP_API_GROUP_TWO *WTPINFO);
/*retu=1,释放函数free_show_ap_if_info(DCLI_WTP_API_GROUP_TWO *WTPINFO)*/																						  
extern int show_ap_if_info_func(dbus_parameter parameter, DBusConnection *connection, int wtp_id,DCLI_WTP_API_GROUP_TWO **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示wtp interface infomation report switch is disable，返回-3示WTP ID非法*/
																																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_get_wtp_bss_pkt_info_func_group(struct WtpList *WtpList_Head);

/*返回-6时，调用Free_get_wtp_bss_pkt_info_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int get_wtp_bss_pkt_info_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,struct WtpList **WtpList_Head);
												/*返回0表示失败，返回1表示成功，返回-1表示not expect error*/
												/*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
												/*返回-5表示Group ID非法，返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

extern int get_wtp_bss_pkt_info_func(dbus_parameter parameter, DBusConnection *connection,int WtpID);   
																		/*返回0表示失败，返回1表示成功，返回-1表示not expect error*/
																		/*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_wtp_bss_pkt_info(DCLI_WTP_API_GROUP_TWO *INFO);
/*当retu==1调用释放函数free_show_wtp_bss_pkt_info(DCLI_WTP_API_GROUP_TWO *INFO)*/																		
extern int show_wtp_bss_pkt_info_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_TWO **INFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示WTP ID非法*/
																																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_wtp_eth_pkt_info_info(DCLI_WTP_API_GROUP_TWO *WTPINFO);
/*当retu==1调用释放函数free_dcli_wtp_free_fun_two(DCLI_WTP_API_GROUP_TWO *WTPINFO)*/
extern int show_wtp_eth_pkt_info_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_TWO **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示WTP ID非法*/
																																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_wtp_radio_pkt_info(DCLI_WTP_API_GROUP_TWO *WTPINFO);
/*当retu==1时，调用free_show_wtp_radio_pkt_info(DCLI_WTP_API_GROUP_TWO *WTPINFO)释放空间*/
extern int show_wtp_radio_pkt_info_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_TWO **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示WTP ID非法*/
																																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_wtp_wifi_snr(DCLI_WTP_API_GROUP_TWO *WTPINFO);
/*返回1时，调用free_show_wtp_wifi_snr()释放空间*/
extern int show_wtp_wifi_snr_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_TWO **WTPINFO);   
																										/*返回0表示失败，返回1表示成功*/
																										/*返回-1表示wtp id does not exist*/
																										/*返回-2表示wtp wifi snr report switch is，返回-3示WTP ID非法*/ 	
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ac_ap_ntp_func_group(struct WtpList *WtpList_Head);

/*返回-8时，调用Free_set_ac_ap_ntp_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*type为"start"或"stop"*/
/*value的范围是60-65535*/
extern int set_ac_ap_ntp_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Type,char *value,struct WtpList **WtpList_Head);
												/*返回0表示失败，返回1表示成功*/
												/*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
												/*返回-5表示input interface only with 'start' or 'stop'，返回-6表示interval should be 60-65535*/
												/*返回-7表示Group ID非法，返回-8表示partial failure，返回-9表示group id does not exist*/
#endif

/*type为"start"或"stop"*/
/*value的范围是60-65535s，默认3600s*/
extern int set_ac_ap_ntp_func(dbus_parameter parameter, DBusConnection *connection,int WtpId,char *type,char *value);
																					/*返回0表示失败，返回1表示成功*/
																					/*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
																					/*返回-5表示input interface only with 'start' or 'stop'，返回-6表示interval should be 60-65535*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_config_update_func_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_ap_config_update_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_config_update_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *IP,struct WtpList **WtpList_Head);
											/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
											/*返回-2表示wtp is not in run state，返回-3表示wtp id does not exist*/
											/*返回-4表示error，返回-5示WTP ID非法，返回-6表示Group ID非法*/
											/*返回-7表示partial failure，返回-8表示group id does not exist*/
#endif

extern int set_ap_config_update_func(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *IP); 
																				/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
																				/*返回-2表示wtp is not in run state，返回-3表示wtp id does not exist*/
																				/*返回-4表示error，返回-5示WTP ID非法*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																				
extern void free_show_wtp_extension_information_v4(DCLI_WTP_API_GROUP_TWO *WTPINFO);
/*reut = 1 调用释放函数free_show_wtp_extension_information_v4(DCLI_WTP_API_GROUP_TWO *WTPINFO)*/
extern int show_wtp_extension_information_v4_func(dbus_parameter parameter, DBusConnection *connection,int wtp_id,DCLI_WTP_API_GROUP_TWO **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示wtp id does not exist，返回-2表示wtp extension info report switch disable，返回-3示WTP ID非法*/
																																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_ap_wids_set_cmd(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*retu==1调用释放函数 free_show_ap_wids_set_cmd(DCLI_WTP_API_GROUP_THREE *WTPINFO)*/																				
extern int show_ap_wids_set_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_THREE **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																	  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
/*20130617 start*/																																	  
extern int show_dhcp_flooding_status_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_THREE **WTPINFO);
extern int show_sfd_status_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_THREE **WTPINFO);																																	  

/*20130617 end*/

extern int set_ap_wids_set_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *type1,char *type2,char *type3,char *state); /*返回0表示失败，返回1表示成功，返回-1表示input parameter error，返回-2表示error*/
																																	  			 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_wtp_max_power_base_model(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*retu = 1 时，调用free_show_wtp_max_power_base_model()释放空间*/
extern int show_wtp_max_power_base_model_func(dbus_parameter parameter, DBusConnection *connection,int WtpID,DCLI_WTP_API_GROUP_THREE **WTPINFO);
																										/*返回0表示失败，返回1表示成功*/
																									    /*返回-1表示wtp id does not exist*/
																									    /*返回-2表示wtp model is wrong，返回-3示WTP ID非法*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_reboot_func_group(struct WtpList *WtpList_Head);

/*返回-6时，调用Free_set_ap_reboot_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_reboot_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,struct WtpList **WtpList_Head);
												/*返回0表示失败，返回1表示成功，返回-1表示wtp is not in run state*/
												/*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
												/*返回-5表示Group ID非法，返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

extern int set_ap_reboot_func(dbus_parameter parameter, DBusConnection *connection,int WtpID);
															 /*返回0表示失败，返回1表示成功，返回-1表示wtp is not in run state*/
															 /*返回-2表示wtp id does not exist，返回-3表示error，返回-4示WTP ID非法*/
															 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
															 
extern int set_ap_reboot_all_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功，返回-1表示error*/
															 								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
															 
extern int set_ap_reboot_by_wlanid_func(dbus_parameter parameter, DBusConnection *connection,char *WLAN_ID);
																			 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																			 /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																			 /*返回-3表示wlan id does not exist，返回-4表示error*/
																			 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			 
extern int set_ap_reboot_by_interface_func(dbus_parameter parameter, DBusConnection *connection,char *IF_Name);
																			   /*返回0表示失败，返回1表示成功*/
																			   /*返回-1表示the length of input parameter is excel the limit of 16*/
																			   /*返回-2表示interface error，返回-3表示error*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			   
extern int set_ap_reboot_by_model_func(dbus_parameter parameter, DBusConnection *connection,char *AP_Model);
																			  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																		      /*返回-2表示model is not exist，返回-3表示error*/
																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			  
extern int set_ap_reboot_by_list_func(dbus_parameter parameter, DBusConnection *connection,char *ap_list);
																		 /*返回0表示失败，返回1表示成功*/
																	     /*返回-1表示input wtp id is too long,you should input less than 80 letters*/
																	     /*返回-2表示parse wtp list failed，返回-3表示error*/
																		 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为(join|configure|datacheck|run|quit|imagedata|bak_run)*/
/*返回1时，调用Free_wtp_list_new_head()释放空间*/
extern int show_wtp_list_by_state_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);
																		 	  /*返回0表示失败，返回1表示成功，返回2表示no wtp exist*/
																			  /*返回-1表示input patameter should only be join|configure|datacheck|run|quit|imagedata|bak_run*/
																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										  
/*返回1时，调用Free_wtp_list_new_head()释放空间*/
extern int show_wtp_list_by_ip_func(dbus_parameter parameter, DBusConnection *connection,char *IP,char *MASK,DCLI_WTP_API_GROUP_ONE **WTPINFO);
																			  			   /*返回0表示失败，返回1表示成功，返回2表示no wtp exist*/
																						   /*返回-1表示unknown ip format*/
																						   /*返回-2表示unknown mask format*/
																						   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
/*返回1时，调用Free_wtp_list_new_head()释放空间*/
extern int show_wtp_list_by_macex_func(dbus_parameter parameter, DBusConnection *connection,char *MAC,char *MASK,DCLI_WTP_API_GROUP_ONE **WTPINFO);
																						   			 /*返回0表示失败，返回1表示成功，返回2表示no wtp exist*/
																								     /*返回-1表示Unknown mac addr format*/
																								     /*返回-2表示Unknown macmask addr format*/
																									 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																													 
extern void Free_wtp_model_list_head(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用Free_wtp_model_list_head()释放空间*/
extern int show_wtp_model_list_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示no wtp exist*/
																																 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int wtp_set_ap_update_config_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *path,char *count,char *model,char *version);
																											   /*返回0表示失败，返回1表示成功*/
																											   /*返回-1表示set update failed due to system cann't find file*/
																											   /*返回-2表示set update failed due to file version error*/
																											   /*返回-3表示wireless-control does not surport model MODEL*/
																											   /*返回-4表示update is process,please wait several minutes*/
																											   /*返回-5表示error*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_wtp_show_ap_update_config(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*返回1时，调用free_wtp_show_ap_update_config()释放空间*/
extern int wtp_show_ap_update_config_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_THREE **WTPINFO);
																									  /*返回0表示失败，返回1表示成功*/
																									  /*返回-1表示no update config information*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					  
extern int wtp_clear_ap_update_config_func(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功，返回-1表示no update config information*/
																									   /*返回-2表示upgrade is in process,changement of configuration is not allowed now*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_update_wtp_list(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用Free_show_update_wtp_list()释放空间*/
extern int show_update_wtp_list_func(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);/*返回0表示失败，返回1表示成功，返回-1表示update wtp does not exist*/
																																  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int uplink_detect_cmd_func(dbus_parameter parameter, DBusConnection *connection,char *ip,char *WlanID,char *State);
																						  /*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示unknown ip format*/
																						  /*返回-2表示unknown wlanid format*/
																						  /*返回-3表示wlan id should be 1 to WLAN_NUM-1*/
						                                                                  /*返回-4表示wlan no exist*/
																						  /*返回-5表示illegal input:Input exceeds the maximum value of the parameter type*/
																						  /*返回-6表示unknown format,just be enable or disable*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_wtp_model_code_version(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用free_show_wtp_model_code_version()释放空间*/																						  
extern int show_wtp_model_code_version_cmd_func(dbus_parameter parameter, DBusConnection *connection,int WtpID,DCLI_WTP_API_GROUP_ONE **WTPINFO);
																														/*返回0表示失败，返回1表示成功*/
																											  			/*返回-1表示wtp id does not exist*/
																											  			/*返回-2表示error，返回-3示WTP ID非法*/
																														/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_update_wtp_img_cmd_func_group(struct WtpList *WtpList_Head);

/*返回-6时，调用Free_update_wtp_img_cmd_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*time为"now"或"later"*/
extern int update_wtp_img_cmd_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *filename,char *version,char *time,struct WtpList **WtpList_Head);
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示set update failed due to system cann't find file*/
																			/*返回-2表示set update failed due to file version error*/
																			/*返回-3表示error，返回-4示WTP ID非法*/
																			/*返回-5表示Group ID非法，返回-6表示partial failure*/
																			/*返回-7表示group id does not exist*/
#endif

/*time为"now"或"later"*/
extern int update_wtp_img_cmd_func(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *filename,char *version,char *time);
																											 /*返回0表示失败，返回1表示成功*/
																											 /*返回-1表示set update failed due to system cann't find file*/
																											 /*返回-2表示set update failed due to file version error*/
																											 /*返回-3表示error，返回-4示WTP ID非法*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_clear_wtp_img_cmd_func_group(struct WtpList *WtpList_Head);

/*返回-4时，调用Free_clear_wtp_img_cmd_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int clear_wtp_img_cmd_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,struct WtpList **WtpList_Head);
							 /*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示WTP ID非法*/
							 /*返回-3表示Group ID非法，返回-4表示partial failure，返回-5表示group id does not exist*/
#endif

extern int clear_wtp_img_cmd_func(dbus_parameter parameter, DBusConnection *connection,int WtpID);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示WTP ID非法*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_networkaddr_command_cmd_group(struct WtpList *WtpList_Head);

/*返回-9时，调用Free_set_ap_networkaddr_command_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_networkaddr_command_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *ap_ip,char *ap_mask,char *ap_gateway,char *ap_dns1,char *ap_dns2,struct WtpList **WtpList_Head);
														/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
														/*返回-2表示unknown mask format，返回-3表示unknown gateway format*/
														/*返回-4表示unknown dns format，返回-5表示wtp id does not exist*/
														/*返回-6表示error，返回-7示WTP ID非法，返回-8表示Group ID非法*/
														/*返回-9表示partial failure，返回-10表示group id does not exist*/
#endif

extern int set_ap_networkaddr_command_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *ap_ip,char *ap_mask,char *ap_gateway,char *ap_dns1,char *ap_dns2);
														/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
														/*返回-2表示unknown mask format，返回-3表示unknown gateway format*/
														/*返回-4表示unknown dns format，返回-5表示wtp id does not exist*/
														/*返回-6表示error，返回-7示WTP ID非法*/
														/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_old_ap_img_data(DCLI_WTP_API_GROUP_THREE *WTPINFO);
/*retu = 1 调用释放函数free_show_old_ap_img_data(DCLI_WTP_API_GROUP_THREE *WTPINFO)*/														
extern int show_old_ap_img_data_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_THREE **WTPINFO );/*返回0表示失败，返回1表示成功*/
																																	  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
														
extern int old_ap_img_data_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功*/														
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_update_img_file_name(dbus_parameter parameter, DBusConnection *connection,char *fpath,char *fversion,char *ftime,char *plist);
																							/*返回0表示失败，返回1表示成功，返回-1表示set update failed due to system cann't find file*/
																							/*返回-2表示set update failed due to file version error，返回-3表示set wtp list error，返回-4表示error*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											
extern int clear_ap_img_info(dbus_parameter parameter, DBusConnection *connection,char *iflist);/*返回0表示失败，返回1表示成功，返回-1表示set wtp list error，返回-2表示error*/
																								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											
extern void free_show_ac_access_wtp_info(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用free_show_ac_access_wtp_info()释放空间*/
extern int show_ac_access_wtp_info_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);/*返回0表示失败，返回1表示成功*/
																																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void free_show_update_fail_wtp_list(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用free_show_update_fail_wtp_list()释放空间*/
extern int show_update_fail_wtp_list(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);/*返回0表示失败，返回1表示成功*/
																																/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int clear_update_fail_wtp(dbus_parameter parameter, DBusConnection *connection,char *iflist);/*返回0表示失败，返回1表示成功，返回-1表示set wtp list error，返回-2表示error*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_wtp_sn_group(struct WtpList *WtpList_Head);

/*返回-6时，调用Free_set_wtp_sn_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_wtp_sn_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wtpsn,struct WtpList **WtpList_Head);
								/*返回0表示失败，返回1表示成功，返回-1表示wtp sn is too long,should be 1 to 127*/
								/*返回-2表示wtp id does not exist，返回-3表示wtp be running，返回-4示WTP ID非法*/
								/*返回-5表示Group ID非法，返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

extern int set_wtp_sn(dbus_parameter parameter, DBusConnection *connection,int id,char *wtpsn);
 															/*返回0表示失败，返回1表示成功，返回-1表示wtp sn is too long,should be 1 to 127*/
															/*返回-2表示wtp id does not exist，返回-3表示wtp be running，返回-4示WTP ID非法*/
															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
															
extern int clean_acaccess_wtpinfo_list(dbus_parameter parameter, DBusConnection *connection);/*返回0表示失败，返回1表示成功*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
/*wtp_list为"all"或like 1,8,9-20,33*/
extern int set_wtp_list_dhcp_snooping_enable_cmd(dbus_parameter parameter, DBusConnection *connection,char *state,char *wtp_list);
																								 	  /*返回0表示失败，返回1表示成功*/
																									  /*返回-1表示input patameter only with 'enable' or 'disable'*/
																									  /*返回-2表示set wtp list error,like 1,8,9-20,33*/
																									  /*返回-3表示error*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_wtp_dhcp_snooping_enable_cmd_group(struct WtpList *WtpList_Head);

/*返回-5时，调用Free_set_wtp_dhcp_snooping_enable_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_wtp_dhcp_snooping_enable_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct WtpList **WtpList_Head);
																  /*返回0表示失败，返回1表示成功*/
																  /*返回-1表示input patameter only with 'enable' or 'disable'*/
																  /*返回-2表示error，返回-3示WTP ID非法*/
																  /*返回-4表示Group ID非法，返回-5表示partial failure*/
																  /*返回-6表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_wtp_dhcp_snooping_enable_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																						 /*返回0表示失败，返回1表示成功*/
																						 /*返回-1表示input patameter only with 'enable' or 'disable'*/
																						 /*返回-2表示error，返回-3示WTP ID非法*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
/*wtp_list为"all"或like 1,8,9-20,33*/
extern int set_wtp_list_sta_info_report_enable_cmd(dbus_parameter parameter, DBusConnection *connection,char *state,char *wtp_list);
																						 			 /*返回0表示失败，返回1表示成功*/
																								     /*返回-1表示input patameter only with 'enable' or 'disable'*/
																								     /*返回-2表示set wtp list error,like 1,8,9-20,33*/
																								     /*返回-3表示error*/
																									 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_wtp_sta_info_report_enable_cmd_group(struct WtpList *WtpList_Head);

/*返回-5时，调用Free_set_wtp_sta_info_report_enable_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_wtp_sta_info_report_enable_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct WtpList **WtpList_Head);
																 /*返回0表示失败，返回1表示成功*/
																 /*返回-1表示input patameter only with 'enable' or 'disable'*/
																 /*返回-2表示error，返回-3示WTP ID非法*/
																 /*返回-4表示Group ID非法，返回-5表示partial failure*/
																 /*返回-6表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_wtp_sta_info_report_enable_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																						 /*返回0表示失败，返回1表示成功*/
																					     /*返回-1表示input patameter only with 'enable' or 'disable'*/
																					     /*返回-2表示error，返回-3示WTP ID非法*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*WtpID为0表示全局配置*/
/*trap_type为"rogueap","rogueterminal","cpu"或"memory"*/
/*cpu use threshold范围是0-100*/
/*ap memory use threshold范围是0-100*/
/*rogueap and rogueterminal 范围是0-50000*/																						 
extern int set_wtp_trap_threshold_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *trap_type,char *VALUE);
																						 		    /*返回0表示失败，返回1表示成功*/
																									/*返回-1表示input patameter only with 'rogueap','rogueterminal','cpu' or 'memory'*/
																									/*返回-2表示unknown id format，返回-3表示ap cpu use threshold parameters error,should be 0-100*/
																									/*返回-4表示ap memory use threshold parameters error,should be 0-100*/
																									/*返回-5表示wtp id does not run，返回-6表示error，返回-7示WTP ID非法*/
																						   			/*返回-8表示parameters error, threshold should be less than 50000*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_ap_trap_threshold(WID_TRAP_THRESHOLD *INFIO);
/*WtpID为0表示全局配置*/																									
/*只要调用函数，就调用Free_show_ap_trap_threshold()释放空间*/
extern int show_ap_trap_rogue_ap_ter_cpu_mem_threshold_cmd(dbus_parameter parameter, DBusConnection *connection, int WtpID,WID_TRAP_THRESHOLD **INFIO);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示WTP ID非法*/
																																									 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_wtp_trap_switch_cmd_group(struct WtpList *WtpList_Head);

/*返回-5时，调用Free_set_wtp_trap_switch_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_wtp_trap_switch_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct WtpList **WtpList_Head);
																		/*返回0表示失败，返回1表示成功*/
																	    /*返回-1表示input patameter only with 'enable'or'disable'*/
																		/*返回-2表示error，返回-3示WTP ID非法*/
																		/*返回-4表示Group ID非法，返回-5表示partial failure*/
																		/*返回-6表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_wtp_trap_switch_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *state);
																				/*返回0表示失败，返回1表示成功*/
																			    /*返回-1表示input patameter only with 'enable'or'disable'*/
																				/*返回-2表示error，返回-3示WTP ID非法*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_sta_wapi_info_report_enable_cmd_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_ap_sta_wapi_info_report_enable_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_ap_sta_wapi_info_report_enable_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct WtpList **WtpList_Head);
						/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable'or'disable'*/
						/*返回-2表示wtp id does not exist，返回-3表示wtp id does not run，返回-4表示error，返回-5示WTP ID非法*/
						/*返回-6表示Group ID非法，返回-7表示partial failure，返回-8表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_ap_sta_wapi_info_report_enable_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *state);
																								 /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable'or'disable'*/
																								 /*返回-2表示wtp id does not exist，返回-3表示wtp id does not run，返回-4表示error，返回-5示WTP ID非法*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_sta_wapi_info_reportinterval_cmd_group(struct WtpList *WtpList_Head);

/*返回-9时，调用Free_set_ap_sta_wapi_info_reportinterval_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*para的范围是1-120*/
extern int set_ap_sta_wapi_info_reportinterval_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *para,struct WtpList **WtpList_Head);
										 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
										 /*返回-2表示ap sta infomation reportinterval error，返回-3表示wtp is not in run state*/
										 /*返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
										 /*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
										 /*返回-8表示Group ID非法，返回-9表示partial failure，返回-10表示group id does not exist*/
#endif

/*para的范围是1-32767*/
extern int set_ap_sta_wapi_info_reportinterval_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *para);
																								/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																								/*返回-2表示ap sta infomation reportinterval error,should be 1-32767，返回-3表示wtp is not in run state*/
																								/*返回-4表示wtp id does not exist，返回-5表示error，返回-6示WTP ID非法*/
																								/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_the_radio_para_information_cmd(struct WtpRadioParaInfo *WlanHead);
/*只要调用，就通过Free_show_all_wtp_the_radio_para_information_cmd()释放空间*/
extern int show_all_wtp_the_radio_para_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpRadioParaInfo **WtpHead);
																												   /*返回0表示失败，返回1表示成功*/
																												   /*返回-1表示There is no WTP now*/
																												   /*返回-2表示error*/
																												   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_radio_stats_information_cmd(struct RadioStatsInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_radio_stats_information_cmd()释放空间*/
extern int show_all_wtp_radio_stats_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct RadioStatsInfo **WtpHead);
																											 /*返回0表示失败，返回1表示成功*/
																											 /*返回-1表示There is no WTP now*/
																											 /*返回-2表示error*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																												   
extern void Free_show_all_wtp_rogue_ap_info_information_cmd(struct RogueAPInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_rogue_ap_info_information_cmd()释放空间*/
extern int show_all_wtp_rogue_ap_info_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct RogueAPInfo **WtpHead);
																											  /*返回0表示失败，返回1表示成功*/
																											  /*返回-1表示There is no WTP now*/
																											  /*返回-2表示ap scanning disable ,please enable it*/
																											  /*返回-3表示no ap has neighbor ap*/
																											  /*返回-4表示error*/
																											  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											 
extern void Free_show_all_wtp_user_link_information_cmd(struct UsrLinkInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_user_link_information_cmd()释放空间*/
extern int show_all_wtp_user_link_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct UsrLinkInfo **WtpHead);
																										/*返回0表示失败，返回1表示成功*/
																									    /*返回-1表示There is no WTP now*/
																									    /*返回-2表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											  
extern void Free_show_all_wtp_data_pkts_information_cmd(struct WtpDataPktsInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_data_pkts_information_cmd()释放空间*/
extern int show_all_wtp_data_pkts_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpDataPktsInfo **WtpHead);
																											/*返回0表示失败，返回1表示成功*/
																											/*返回-1表示There is no WTP now*/
																											/*返回-2表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										
extern void Free_show_all_wtp_device_information_cmd(struct WtpDeviceInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_device_information_cmd()释放空间*/
extern int show_all_wtp_device_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpDeviceInfo **WtpHead);
																									   /*返回0表示失败，返回1表示成功*/
																									   /*返回-1表示There is no WTP now*/
																									   /*返回-2表示error*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_terminal_information_cmd(struct WtpTerminalInfo *WtpHead);
/*返回1时，调用Free_show_all_wtp_terminal_information_cmd()释放空间*/
extern int show_all_wtp_terminal_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpTerminalInfo **WtpHead);
																										   /*返回0表示失败，返回1表示成功*/
																										   /*返回-1表示There is no WTP now*/
																										   /*返回-2表示error*/
																										   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																									   
extern void Free_show_all_wtp_eth_port_information_cmd(struct WtpEthPortInfo *WtpHead);																										   
/*只要调用，就通过Free_show_all_wtp_eth_port_information_cmd()释放空间*/
extern int show_all_wtp_eth_port_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpEthPortInfo **WtpHead);
																										  /*返回0表示失败，返回1表示成功*/
																										  /*返回-1表示There is no WTP now*/
																										  /*返回-2表示error*/
																										  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										   
extern void Free_show_all_wtp_ifname_information_cmd(struct WtpIfnameInfo *WtpHead);																										  
/*只要调用，就通过Free_show_all_wtp_ifname_information_cmd()释放空间*/
extern int show_all_wtp_ifname_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpIfnameInfo **WtpHead);
																									   /*返回0表示失败，返回1表示成功*/
																									   /*返回-1表示There is no WTP now*/
																									   /*返回-2表示error*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_information_cmd(struct WtpInfor *WtpHead);
/*只要调用，就通过Free_show_all_wtp_information_cmd()释放空间*/
extern int show_all_wtp_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpInfor **WtpHead);
																						 /*返回0表示失败，返回1表示成功*/
																						 /*返回-1表示There is no WTP now*/
																						 /*返回-2表示error*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																									   
extern void Free_show_all_wtp_para_information_cmd(struct WtpParaInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_para_information_cmd()释放空间*/
extern int show_all_wtp_para_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpParaInfo **WtpHead); 
																									/*返回0表示失败，返回1表示成功*/
																									/*返回-1表示There is no WTP now*/
																									/*返回-2表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_station_information_cmd(struct WtpStaInfo *StaHead);
/*返回1时，调用Free_show_all_wtp_station_information_cmd()释放空间*/
extern int show_all_wtp_station_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpStaInfo **StaHead);
																									/*返回0表示失败，返回1表示成功*/
																								    /*返回-1表示There is no sta now*/
																								    /*返回-2表示There is no WTP now，返回-3表示error*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_stats_information_cmd(struct WtpStatsInfo *WtpHead);																									
/*只要调用，就通过Free_show_all_wtp_stats_information_cmd()释放空间*/
extern int show_all_wtp_stats_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpStatsInfo **WtpHead);
																									/*返回0表示失败，返回1表示成功*/
																									/*返回-1表示There is no WTP now*/
																									/*返回-2表示error，返回-3表示WTPID is larger than MAX*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_wired_ifstats_information_cmd(struct WtpWiredIfStatsInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_wired_ifstats_information_cmd()释放空间*/
extern int show_all_wtp_wired_ifstats_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpWiredIfStatsInfo **WtpHead);
																													/*返回0表示失败，返回1表示成功*/
																													/*返回-1表示There is no WTP now*/
																													/*返回-2表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																									
extern void Free_show_all_wtp_wireless_if_information_cmd(struct WtpWirelessIfInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_wireless_if_information_cmd()释放空间*/
extern int show_all_wtp_wireless_if_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpWirelessIfInfo **WtpHead);
																												/*返回0表示失败，返回1表示成功*/
																												/*返回-1表示There is no WTP now*/
																												/*返回-2表示error*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																													
extern void Free_show_all_wtp_wirelessifstats_information_cmd(struct WtpWirelessIfstatsInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_wirelessifstats_information_cmd()释放空间*/
extern int show_all_wtp_wirelessifstats_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpWirelessIfstatsInfo **WtpHead);
																														/*返回0表示失败，返回1表示成功*/
																														/*返回-1表示There is no WTP now*/
																														/*返回-2表示error*/
																														/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_wlan_data_pkts_information_cmd(struct WtpWlanDataPktsInfo *WtpHead);
/*返回1时，调用Free_show_all_wtp_wlan_data_pkts_information_cmd()释放空间*/
extern int show_all_wtp_wlan_data_pkts_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpWlanDataPktsInfo **WtpHead);
																										/*返回0表示失败，返回1表示成功*/
																										/*返回-1表示There is no WTP now*/
																										/*返回-2表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																														
extern void Free_show_all_wlan_stats_information_cmd(struct WtpWlanStatsInfo *WtpHead);
/*只要调用，就通过Free_show_all_wlan_stats_information_cmd()释放空间*/
extern int show_all_wlan_stats_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpWlanStatsInfo **WtpHead);
																										 /*返回0表示失败，返回1表示成功*/
																										 /*返回-1表示There is no WTP now*/
																										 /*返回-2表示error*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_radio_config_information_cmd(struct WtpConfigRadioInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_radio_config_information_cmd()释放空间*/
extern int show_all_wtp_radio_config_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpConfigRadioInfo **WtpHead);
																												  /*返回0表示失败，返回1表示成功*/
																												  /*返回-1表示There is no WTP now*/
																												  /*返回-2表示error*/
																												  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_basic_information_cmd(struct WtpBasicInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_basic_information_cmd()释放空间*/
extern int show_all_wtp_basic_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpBasicInfo **WtpHead);
																									 /*返回0表示dbus error，返回1表示成功*/
																									 /*返回-1表示There is no WTP now*/
																									 /*返回-2表示error*/
																									 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int ccgi_set_ap_longitude_latitude_cmd(dbus_parameter parameter, DBusConnection *connection, unsigned int id, char *lon, char *lat);

extern void Free_show_all_wtp_new_wtp_wireless_ifinfo_information_cmd(struct NewWtpWirelessIfInfo *RadioHead);
/*只要调用，就通过Free_show_all_wtp_new_wtp_wireless_ifinfo_information_cmd()释放空间*/
extern int show_all_wtp_new_wtp_wireless_ifinfo_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct NewWtpWirelessIfInfo **RadioHead);
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示There is no WTP now*/
																							/*返回-2表示error*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wlan_ssid_stats_information_cmd(struct SSIDStatsInfo *WtpHead);
/*只要调用，就通过Free_show_all_wlan_ssid_stats_information_cmd()释放空间*/
extern int show_all_wlan_ssid_stats_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct SSIDStatsInfo **WtpHead);
																											/*返回0表示失败，返回1表示成功*/
																											/*返回-1表示There is no WTP now*/
																											/*返回-2表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wlan_ssid_stats_information_cmd_v2(struct SSIDStatsInfo_v2 *WtpHead_v2);
/*返回1时，调用Free_show_all_wlan_ssid_stats_information_cmd_v2()释放空间*/
extern int show_all_wlan_ssid_stats_information_cmd_v2(dbus_parameter parameter, DBusConnection *connection,struct SSIDStatsInfo_v2 **WtpHead_v2);


/*dt*/
extern void Free_show_all_wtp_wlan_data_pkts_information_cmd_v2(struct WtpWlanDataPktsInfo *WtpHead_v2);/*返回0表示失败，返回1表示成功*/
extern int show_all_wtp_wlan_data_pkts_information_cmd_v2(dbus_parameter parameter, DBusConnection *connection,struct WtpWlanDataPktsInfo **WtpHead_v2);																													  /*返回-1表示There is no WTP now*/
																												



extern void Free_show_all_wtp_collect_information_cmd(struct WtpCollectInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_collect_information_cmd()释放空间*/
extern int show_all_wtp_collect_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpCollectInfo **WtpHead);
																										/*返回0表示失败，返回1表示成功*/
																										/*返回-1表示There is no WTP now*/
																										/*返回-2表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_ath_statistics_info_of_all_wtp_cmd(struct WtpAthStatisticInfo *WtpHead);
/*返回1时，调用Free_show_ath_statistics_info_of_all_wtp_cmd()释放空间*/
extern int show_ath_statistics_info_of_all_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpAthStatisticInfo **WtpHead);
																											   /*返回0表示失败，返回1表示成功*/
																											   /*返回-1表示there is no ath interface*/
																											   /*返回-2表示WID can't find wtp*/
																											   /*返回-3表示ASD can't find bss*/
																											   /*返回-4表示error*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_ap_moment_infomation_enable_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *state);
																							   /*返回0表示失败，返回1表示成功*/
																							   /*返回-1表示input patameter only with 'enable' or 'disable'*/
																							   /*返回-2表示WTP ID非法，返回-3表示wtp id does not exist*/
																							   /*返回-4表示wtp id does not run，返回-5表示The switch is already state*/
																							   /*返回-6表示error*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*value的范围是2-32767*/
extern int set_ap_moment_infomation_reportinterval_cmd(dbus_parameter parameter, DBusConnection *connection,char *value);
																							 /*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示ap moment infomation reportinterval error*/
																							 /*返回-2表示The value is no need to change，返回-3表示error*/
																							 /*返回-4表示unknown id format*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*value的范围是2-32767*/
extern int set_ap_sample_infomation_reportinterval_cmd(dbus_parameter parameter, DBusConnection *connection,char *value); 
																							 /*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示ap sample infomation reportinterval error*/
																							 /*返回-2表示The value is no need to change，返回-3表示error*/
																							 /*返回-4表示unknown id format*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*value的范围是10-32767*/
extern int set_ap_routine_infomation_reportinterval_cmd(dbus_parameter parameter, DBusConnection *connection,char *value); 
																							 /*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示ap routine infomation reportinterval error*/
																							 /*返回-2表示The value is no need to change，返回-3表示error*/
																							 /*返回-4表示unknown id format*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int show_ap_moment_information_reportinterval_cmd(dbus_parameter parameter, DBusConnection *connection,struct ap_reportinterval_profile *reportinterval_info);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																							 																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_trap_ignore_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *percent);
																				 /*返回0表示失败，返回1表示成功*/
																				 /*返回-1表示WTP ID非法，返回-2表示input patameter error*/
																				 /*返回-3表示input patameter must be number*/
																				 /*返回-4表示input patameter must between 0 and 100，返回-5表示error*/
																				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int show_hide_quit_wtp_cmd(dbus_parameter parameter, DBusConnection *connection,int *value);
																	 /*返回0表示失败，返回1表示成功*/
																	 /*返回-1表示the value of hide_quit_wtp_in is invalid*/
																	 /*返回-2表示error*/
																	 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*type为"neighborchannelrssi"或"samechannelrssi"*/
/*Value的范围是-120到0*/
extern int set_wtp_trap_neighbor_same_channelrssithreshold_cmd(dbus_parameter parameter, DBusConnection *connection, int WtpID,char *type,char *Value); 
																	 										  /*返回0表示失败，返回1表示成功*/
																											  /*返回-1表示WTP ID非法*/
																											  /*返回-2表示input patameter only with 'neighborchannelrssi' or 'samechannelrssi'*/
																											  /*返回-3表示unknown id format，返回-4表示input patameter should be -120 to 0*/
																											  /*返回-5表示wtp id does not run，返回-6表示error*/
																											  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*config节点下WtpID为0*/
/*value的范围是5-900*/
extern int set_ap_cpu_collect_time_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *Value);
																					/*返回0表示失败，返回1表示成功*/
																				    /*返回-1表示WTP ID非法，返回-2表示error*/
																					/*返回-3表示unknown id format*/
																				    /*返回-4表示input collect time should be 5~900*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_terminal_distrub_infomation_switch_cmd_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_ap_terminal_distrub_infomation_switch_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_ap_terminal_distrub_infomation_switch_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct WtpList **WtpList_Head);
							/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
							/*返回-2表示WTP ID非法，返回-3表示wtp id does not exist，返回-4表示wtp id does not run*/
							/*返回-5表示error，返回-6表示Group ID非法，返回-7表示partial failure*/
							/*返回-8表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_ap_terminal_distrub_infomation_switch_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *state);
																										/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
																										/*返回-2表示WTP ID非法，返回-3表示wtp id does not exist，返回-4表示wtp id does not run*/
																										/*返回-5表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_terminal_distrub_infomation_pkt_cmd_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_ap_terminal_distrub_infomation_pkt_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*value的范围是0-50000*/
extern int set_ap_terminal_distrub_infomation_pkt_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *value,struct WtpList **WtpList_Head);
							/*返回0表示失败，返回1表示成功，返回-1表示ap terminal distrub infomation reportpkt error*/
							/*返回-2表示WTP ID非法，返回-3表示wtp id does not exist，返回-4表示wtp id does not run*/
							/*返回-5表示error，返回-6表示Group ID非法，返回-7表示partial failure*/
							/*返回-8表示group id does not exist，返回-9表示unknown id format*/
#endif

/*value的范围是0-50000*/
extern int set_ap_terminal_distrub_infomation_pkt_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *value);
																									/*返回0表示失败，返回1表示成功，返回-1表示ap terminal distrub infomation reportpkt error*/
																									/*返回-2表示WTP ID非法，返回-3表示wtp id does not exist，返回-4表示wtp id does not run*/
																									/*返回-5表示error，返回-9表示unknown id format*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_terminal_distrub_infomation_sta_num_cmd_group(struct WtpList *WtpList_Head);

/*返回-7时，调用Free_set_ap_terminal_distrub_infomation_sta_num_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*value的范围是1-50000*/
extern int set_ap_terminal_distrub_infomation_sta_num_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *value,struct WtpList **WtpList_Head);
							/*返回0表示失败，返回1表示成功，返回-1表示ap terminal distrub infomation report sta_num error*/
							/*返回-2表示WTP ID非法，返回-3表示wtp id does not exist，返回-4表示wtp id does not run*/
							/*返回-5表示error，返回-6表示Group ID非法，返回-7表示partial failure*/
							/*返回-8表示group id does not exist，返回-9表示unknown id format*/
#endif

/*value的范围是1-50000*/
extern int set_ap_terminal_distrub_infomation_sta_num_cmd(dbus_parameter parameter, DBusConnection *connection,int WtpID,char *value); 
																											/*返回0表示失败，返回1表示成功，返回-1表示ap terminal distrub infomation report sta_num error*/
																											/*返回-2表示WTP ID非法，返回-3表示wtp id does not exist，返回-4表示wtp id does not run*/
																											/*返回-5表示error*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_wlan_radio_information_cmd(struct WtpWlanRadioInfo *WtpHead);
/*返回1时，调用Free_show_wlan_radio_information_cmd()释放空间*/
extern int show_wlan_radio_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpWlanRadioInfo **WtpHead);
																									  /*返回0表示失败，返回1表示成功*/
																									  /*返回-1表示there is no wlan，返回-2表示Malloc Error*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_network_info_cmd(struct WtpNetworkInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_network_info_cmd()释放空间*/
extern int show_all_wtp_network_info_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpNetworkInfo **WtpHead);
																								 /*返回0表示失败，返回1表示成功*/
																							     /*返回-1表示error*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_security_mech_information_cmd(struct SecurityMechInfo *WtpHead);
/*只要调用，就通过Free_show_all_wtp_security_mech_information_cmd()释放空间*/
extern int show_all_wtp_security_mech_information_cmd(dbus_parameter parameter, DBusConnection *connection,struct SecurityMechInfo **WtpHead); 
																													/*返回0表示失败，返回1表示成功*/
																													/*返回-1表示There is no WTP now，返回-2表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_wtp_config_of_all_cmd(struct WTP_CONFIG_INFORMATION *WTPconfig);
/*只要调用，就通过Free_show_all_wtp_security_mech_information_cmd()释放空间*/
extern int show_wtp_config_of_all_cmd(dbus_parameter parameter, DBusConnection *connection,struct WTP_CONFIG_INFORMATION **WTPconfig);
																										/*返回0表示失败，返回1表示成功*/
																										/*返回-1表示error*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_wtp_list_byinterface_cmd(DCLI_WTP_API_GROUP_ONE *WTPINFO);
/*返回1时，调用Free_show_all_wtp_security_mech_information_cmd()释放空间*/
extern int show_wtp_list_byinterface_cmd(dbus_parameter parameter, DBusConnection *connection,DCLI_WTP_API_GROUP_ONE **WTPINFO);/*返回0表示失败，返回1表示成功*/
																																	  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_count_ap_num_by_interface_head(AP_NUM_INF *head);
/*返回1时，调用Free_count_ap_num_by_interface_head()释放空间*/
extern int count_ap_num_by_interface(dbus_parameter parameter, DBusConnection *connection,AP_NUM_INF *ANIF_head);/*返回0表示失败，返回1表示成功*/

extern void Free_show_statistcs_information_of_all_wtp_whole_cmd(struct WtpAthStatisticInfo *WtpHead);

/*返回1时，调用Free_show_statistcs_information_of_all_wtp_whole_cmd()释放空间*/
extern int show_statistcs_information_of_all_wtp_whole_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpAthStatisticInfo **WtpHead);
																	/*返回0表示失败，返回1表示成功*/
																	/*返回-1表示there is no ath interface*/
																	/*返回-2表示WID can't find wtp*/
																	/*返回-3表示error*/
																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wtp_station_statistic_information_cmd(struct WtpStationinfo *StaHead);
/*返回1时，调用Free_show_statistcs_information_of_all_wtp_whole_cmd()释放空间*/
extern int show_statistcs_information_of_all_wtp_whole_cmd(dbus_parameter parameter, DBusConnection *connection,struct WtpAthStatisticInfo **WtpHead);
																	/*返回0表示失败，返回1表示成功*/
																	/*返回-1表示there is no ath interface*/
																	/*返回-2表示WID can't find wtp*/
																	/*返回-3表示error*/
																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_ac_sta_information_cmd(struct WtpStationinfo *StaNode);
/*返回1时，调用Free_show_all_wtp_station_statistic_information_cmd()释放空间*/
extern int show_ac_sta_information_cmd(dbus_parameter parameter, DBusConnection *connection, struct WtpStationinfo **StaNode);
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示There is no sta now*/
																			/*返回-2表示There is no BSS now，返回-3表示error*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*tar_file表示版本压缩文件名称，压缩格式是.tar.bz2*/
extern int bind_ap_model_with_file_config(dbus_parameter parameter, DBusConnection *connection,char *model,char *tar_file);
																								/*返回0表示失败，返回1表示成功*/
																								/*返回-1表示buf malloc failed*/
																								/*返回-2表示bind failed due to system cann't find file*/
																								/*返回-3表示wireless-control does not surport model*/
																								/*返回-4表示update is process,please wait several minutes*/
																								/*返回-5表示free memory is not enough*/
																								/*返回-6表示this model has been bound ever,please delete the bind relationship first*/
																								/*返回-7表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*Count_onetime表示同时升级的AP个数，范围是1-50*/
extern int wtp_set_ap_update_base_model_config(dbus_parameter parameter, DBusConnection *connection,char *model);
																								/*返回0表示失败，返回1表示成功*/
																								/*返回-1表示malloc failed*/
																								/*返回-2表示wireless-control does not surport model*/
																								/*返回-3表示update is process,please wait several minutes*/
																								/*返回-4表示model has been set ever, there's no need to set again*/
																								/*返回-5表示error*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_model_tar_file_bind_info(struct model_tar_file *head);
/*返回1且model_tar_file_num>0时，调用Free_show_model_tar_file_bind_info()释放空间*/
extern int show_model_tar_file_bind_info(dbus_parameter parameter, DBusConnection *connection,struct model_tar_file *head,int *model_tar_file_num);
																/*返回0表示失败，返回1表示成功，返回-1表示error*/
																/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int delete_model_bind_info_config(dbus_parameter parameter, DBusConnection *connection,char *model);
																	/*返回0表示失败，返回1表示成功*/
																	/*返回-1表示buf malloc failed*/
																	/*返回-2表示wireless-control does not surport model*/
																	/*返回-3表示update is process,please wait several minutes*/
																	/*返回-4表示this model hasn't been bound ever,please make sure that first*/
																	/*返回-5表示error*/
																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int wtp_clear_ap_one_model_update_config(dbus_parameter parameter, DBusConnection *connection,char *model);
																					  /*返回0表示失败，返回1表示成功*/
																					  /*返回-1表示buf malloc failed*/
																					  /*返回-2表示upgrade is in process,changement of configuration is not allowed now*/
																					  /*返回-3表示no update config information of model*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*Count_onetime表示同时升级的AP个数，范围是1-50*/
extern int wtp_set_ap_update_count_config(dbus_parameter parameter, DBusConnection *connection,char *Count_onetime);
																					  /*返回0表示失败，返回1表示成功*/
 																					  /*返回-1表示unknown id format*/
 																					  /*返回-2表示upgrade has already started,it's not allowed to set the value unless upgrade is stopped*/
 																					  /*返回-3表示error*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state的范围是"start"或"stop"*/
extern int wtp_set_ap_update_control_config(dbus_parameter parameter, DBusConnection *connection,char *state);
																			   /*返回0表示失败，返回1表示成功*/
																			   /*返回-1表示input parameter can only be 'start' or 'stop'*/
																			   /*返回-2表示there's no upgrade configuration,it should be configured first*/
																			   /*返回-3表示error*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/



/*Radio*/										  
extern void Radio_Type(int RType,char *r);

extern void  Free_radio_head(DCLI_RADIO_API_GROUP_ONE *RADIO);
/*返回1时，调用Free_radio_head()释放空间*/
extern int show_radio_list(dbus_parameter parameter, DBusConnection *connection,DCLI_RADIO_API_GROUP_ONE **RADIO);/*返回0表示失败，返回1表示成功*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_radio_one_head(DCLI_RADIO_API_GROUP_ONE *RADIO);
/*返回1时，调用Free_radio_one_head()释放空间*/
extern int show_radio_one(dbus_parameter parameter, DBusConnection *connection,int radio_id,DCLI_RADIO_API_GROUP_ONE **RADIO);
																							 /*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示radio id does not exist*/
																							 /*返回-2表示radio id should be 1 to G_RADIO_NUM*/ 
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY		
extern void Free_config_radio_channel_group(struct RadioList *RadioList_Head);
/*返回-17时，调用Free_config_radio_channel_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int config_radio_channel_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char* channel_value,struct RadioList **RadioList_Head);
											 /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
											 /*返回-2表示Radio is disable,	please enable it first，返回-3表示error*/
											 /*返回-4表示channel  is invalid in CHINA，返回-5表示channel  is invalid in EUROPE*/
											 /*返回-6表示channel  is invalid in USA，返回-7表示channel	is invalid in JAPAN*/
											 /*返回-8表示channel  is invalid in FRANCE，返回-9表示channel  is invalid in SPAIN*/
											 /*返回-10表示input parameter error，返回-11表示11a receive channel list is:  36 ..;149 153 157 161*/
											 /*返回-12表示radio type doesn,t support this channel，返回-13表示error，返回-14表示Radio ID非法*/
											 /*返回-15表示illegal input:Input exceeds the maximum value of the parameter type*/
											 /*返回-16表示Group ID非法，返回-17表示partial failure，返回-18表示group id does not exist*/
#endif
																							 
extern int config_radio_channel(dbus_parameter parameter, DBusConnection *connection,int id,char* channel_value);
																				/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
		 																	    /*返回-2表示Radio is disable,  please enable it first，返回-3表示error*/
		 																	    /*返回-4表示channel  is invalid in CHINA，返回-5表示channel  is invalid in EUROPE*/
																				/*返回-6表示channel  is invalid in USA，返回-7表示channel  is invalid in JAPAN*/
																			 	/*返回-8表示channel  is invalid in FRANCE，返回-9表示channel  is invalid in SPAIN*/
		 																	    /*返回-10表示input parameter error，返回-11表示11a receive channel list is:  36 ..;149 153 157 161*/
		 																	    /*返回-12表示radio type doesn,t support this channel，返回-13表示error，返回-14表示Radio ID非法*/
																				/*返回-15表示illegal input:Input exceeds the maximum value of the parameter type*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_radio_txpower_group(struct RadioList *RadioList_Head);

/*返回-10时，调用Free_config_radio_txpower_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*txpower为100表示"auto"*/
extern int config_radio_txpower_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int txpower,struct RadioList **RadioList_Head);
																				/*返回0表示失败，返回1表示成功，返回-1表示txpower conflict with country-code，返回-2表示radio id does not exist*/
																				/*返回-3表示radio is disable, please enable it first，返回-4表示radio mode is 11n,not allow to set txpower，返回-5表示this radio max txpower is 20*/
																				/*返回-6表示this radio max txpower is 27，返回-7表示出错，返回-8表示Radio ID非法，返回-9表示Group ID非法*/
																				/*返回-10表示partial failure，返回-11表示group id does not exist*/
#endif

/*txpower为100表示"auto"*/																							 
extern int config_radio_txpower(dbus_parameter parameter, DBusConnection *connection,int id,int txpower);	
																		/*返回0表示失败，返回1表示成功，返回-1表示txpower conflict with country-code，返回-2表示radio id does not exist*/
																		/*返回-3表示radio is disable, please enable it first，返回-4表示radio mode is 11n,not allow to set txpower，返回-5表示this radio max txpower is 20*/
																		/*返回-6表示this radio max txpower is 27，返回-7表示出错，返回-8表示Radio ID非法*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
#if _GROUP_POLICY
extern void Free_config_radio_rate_group(struct RadioList *RadioList_Head);

/*返回-15时，调用Free_config_radio_rate_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*rate list:10,20,55,60,90,110,120,180,240,360,480,540*/
extern int config_radio_rate_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char* radioRate,struct RadioList **RadioList_Head);
																/*返回0表示失败，返回1表示成功，返回-1表示radio id does not exist，返回-2表示mode 11b support rate list:10 20 55 110*/
																/*返回-3表示mode 11a support rate list:60 90 120 180 240 360 480 540，返回-4表示mode 11g support rate list:60 90 120 180 240 360 480 540*/
																/*返回-5表示mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540，返回-6表示wtp radio does not support this rate,please check first*/ 
																/*返回-7表示radio is disable, please enable it first，返回-8表示radio list is empty，返回-9表示radio mode is 11n,not allow to set rate*/
																/*返回-10表示radio support rate does not exist，返回-11表示radio type is conflict, please check it first，返回-12表示出错，返回-13表示Radio ID非法*/
																/*返回-14表示Group ID非法，返回-15表示partial failure，返回-16表示group id does not exist*/
																/*返回-17表示mode 11an support rate list:60 90 120 180 240 360 480 540，返回-18表示mode 11gn support rate list:60 90 120 180 240 360 480 540*/
																/*返回-19表示mode 11a/an support rate list:60 90 120 180 240 360 480 540，返回-20表示mode 11g/gn support rate list:60 90 120 180 240 360 480 540*/
																/*返回-21表示mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540*/
#endif

/*rate list:10,20,55,60,90,110,120,180,240,360,480,540*/
extern int config_radio_rate(dbus_parameter parameter, DBusConnection *connection,int id,char* radioRate); 
																		/*返回0表示失败，返回1表示成功，返回-1表示radio id does not exist，返回-2表示mode 11b support rate list:10 20 55 110*/
							                                            /*返回-3表示mode 11a support rate list:60 90 120 180 240 360 480 540，返回-4表示mode 11g support rate list:60 90 120 180 240 360 480 540*/
																		/*返回-5表示mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540，返回-6表示wtp radio does not support this rate,please check first*/	
																		/*返回-7表示radio is disable, please enable it first，返回-8表示radio list is empty，返回-9表示radio mode is 11n,not allow to set rate*/
																		/*返回-10表示radio support rate does not exist，返回-11表示radio type is conflict, please check it first，返回-12表示出错，返回-13表示Radio ID非法*/
																		/*返回-14表示mode 11an support rate list:60 90 120 180 240 360 480 540，返回-15表示mode 11gn support rate list:60 90 120 180 240 360 480 540*/
																		/*返回-16表示mode 11a/an support rate list:60 90 120 180 240 360 480 540，返回-17表示mode 11g/gn support rate list:60 90 120 180 240 360 480 540*/
																		/*返回-18表示mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
#if _GROUP_POLICY
extern void Free_config_radio_txpower_offset_group(struct RadioList *RadioList_Head);

/*返回-13时，调用Free_config_radio_txpower_offset_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int config_radio_txpower_offset_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int txpower_off,struct RadioList **RadioList_Head);
																	/*返回0表示失败，返回1表示成功，返回-1表示input parameter txpower error，返回-2表示txpower conflict with country-code*/
																	/*返回-3表示radio id does not exist，返回-4表示radio is disable, please enable it first，返回-5表示radio mode is 11n,not allow to set txpower*/
																	/*返回-6表示this radio max txpower is 20，返回-7表示this radio max txpower is 27，返回-8表示this wtp is not in run state*/
																	/*返回-9表示this radio is not binding wlan,binding wlan first.，返回-10表示error，返回-11表示Radio ID非法，返回-12表示Group ID非法*/
																	/*返回-13表示partial failure，返回-14表示group id does not exist，返回-15表示txpoweroffset is larger than max txpower!Please checkout txpowerstep!*/
#endif

extern int config_radio_txpower_offset(dbus_parameter parameter, DBusConnection *connection,int id,int txpower_off);
																				   /*返回0表示失败，返回1表示成功，返回-1表示input parameter txpower error，返回-2表示txpower conflict with country-code*/
																				   /*返回-3表示radio id does not exist，返回-4表示radio is disable, please enable it first，返回-5表示radio mode is 11n,not allow to set txpower*/
																				   /*返回-6表示this radio max txpower is 20，返回-7表示this radio max txpower is 27，返回-8表示this wtp is not in run state*/
																				   /*返回-9表示this radio is not binding wlan,binding wlan first.，返回-10表示error，返回-11表示Radio ID非法*/
																				   /*返回SNMPD_CONNECTION_ERROR表示connection error，返回-15表示txpoweroffset is larger than max txpower!Please checkout txpowerstep!*/

#if _GROUP_POLICY
extern void Free_config_radio_mode_group(struct RadioList *RadioList_Head);

/*返回-8时，调用Free_config_radio_mode_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*mode list:11a,11b,11g,11n,11b/g,11b/g/n,11a/n*/
extern int config_radio_mode_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *radioMode,struct RadioList **RadioList_Head);
										   /*返回0表示失败，返回1表示成功，返回-1表示mode非法*/
										   /*返回-2表示radio id 不存在，返回-3表示Radio is disable,  please enable it first*/
										   /*返回-4表示radio mode not allow to set with 11n，返回-5表示出错*/
										   /*返回-6表示Radio ID非法，返回-7表示Group ID非法*/
										   /*返回-8表示partial failure，返回-9表示group id does not exist*/
#endif

/*mode list:11a,11b,11g,11gn,11g/gn,11b/g,11b/g/n,11a/an,11an*/
extern int config_radio_mode(dbus_parameter parameter, DBusConnection *connection,int id,char *radioMode);
																			/*返回0表示失败，返回1表示成功，返回-1表示mode非法*/
																			/*返回-2表示radio id 不存在，返回-3表示Radio is disable,  please enable it first*/
																			/*返回-4表示radio mode not allow to set with 11n，返回-5表示出错*/
																			/*返回-6表示Radio ID非法*/
																		    /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_max_rate_group(struct RadioList *RadioList_Head);

/*返回-15时，调用Free_config_max_rate_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int config_max_rate_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char*rad_rate,struct RadioList **RadioList_Head);
															/*返回0表示失败，返回1返回成功，返回-1表示radio id does not exist，返回-2表示mode 11b support rate list:10 20 55 110*/
															/*返回-3表示mode 11a support rate list:60 90 120 180 240 360 480 540，返回-4表示mode 11g support rate list:60 90 120 180 240 360 480 540*/	 
															/*返回-5表示mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540，返回-6表示wtp radio does not support this rate,please check first*/
															/*返回-7表示radio is disable, please enable it first，返回-8表示radio mode is 11n,not allow to set rate，返回-9表示radio list is empty*/
															/*返回-10表示radio support rate does not exist，返回-11表示radio type is conflict, please check it first，返回-12表示error，返回-13表示Radio ID非法*/
															/*返回-14表示Group ID非法	，返回-15表示partial failure，返回-16表示group id does not exist*/
															/*返回-17表示mode 11an support rate list:60 90 120 180 240 360 480 540，返回-18表示mode 11gn support rate list:60 90 120 180 240 360 480 540*/
															/*返回-19表示mode 11a/an support rate list:60 90 120 180 240 360 480 540，返回-20表示mode 11g/gn support rate list:60 90 120 180 240 360 480 540*/
															/*返回-21表示mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540*/
#endif

/*Radio rate value: 11b/g: (10,20,55,60,90,110,120,180,240,360,480,540) */
/*11n: (65,130,135,150,195,260,270,300,390,405,450,520,540,585,600,650,780,810,900,1040,1080,1170,1200,1215,1300,1350,1500,1620,1800,2160,2400,2430,2700,3000)*/
extern int config_max_rate(dbus_parameter parameter, DBusConnection *connection,int id,char*rad_rate,struct RadioList **RadioList_Head);
																	/*返回0表示失败，返回1返回成功，返回-1表示Radio rate illegal*/
																	/*返回-2表示radio id does not exist，返回-3表示mode 11b support rate list:10 20 55 110*/
																	/*返回-4表示mode 11a support rate list:60 90 120 180 240 360 480 540*/
																	/*返回-5表示mode 11g support rate list:60 90 120 180 240 360 480 540*/	 
																	/*返回-6表示mode 11b/g support rate list:10 20 55 60 90 110 120 180 240 360 480 540*/
																	/*返回-7表示mode 11b/g/n support rate list:10 20 60 90 110 120 180 240 360 480 540*/
																	/*返回-8表示wtp radio does not support this rate,please check first*/
																	/*返回-9表示radio is disable, please enable it first*/
																	/*返回-10表示wtp is not binding wlan，返回-11表示radio list is empty*/
																	/*返回-12表示radio support rate does not exist，返回-13表示radio type is conflict, please check it first*/
																	/*返回-14表示error，返回-15表示Radio ID非法*/
																	/*返回-16表示mode 11an support rate list:60 90 120 180 240 360 480 540*/
																	/*返回-17表示mode 11gn support rate list:60 90 120 180 240 360 480 540*/
																	/*返回-18表示mode 11a/an support rate list:60 90 120 180 240 360 480 540*/
																	/*返回-19表示mode 11g/gn support rate list:60 90 120 180 240 360 480 540*/
#if _GROUP_POLICY
extern void Free_config_radio_beaconinterval_group(struct RadioList *RadioList_Head);

/*返回-6时，调用Free_config_radio_beaconinterval_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断BeaconInterval的范围，默认是100，设定范围是25-1000，单位ms*/
extern int config_radio_beaconinterval_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int BeaconInterval,struct RadioList **RadioList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
										/*返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
										/*返回-4表示Radio ID非法，返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

extern int config_radio_beaconinterval(dbus_parameter parameter, DBusConnection *connection,int id,int BeaconInterval);	  /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错，返回-4表示Radio ID非法*/
																															  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_radio_fragmentation_group(struct RadioList *RadioList_Head);

/*返回-6时，调用Free_config_radio_fragmentation_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断Fragmente的范围，默认是2346，设定范围是256-2346，单位byte*/
extern int config_radio_fragmentation_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int Fragmente,struct RadioList **RadioList_Head);
											/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
											/*返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
											/*返回-4表示Radio ID非法，返回-5表示Group ID非法*/
											/*返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

extern int config_radio_fragmentation(dbus_parameter parameter, DBusConnection *connection,int id,int Fragmente);    /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错，返回-4表示Radio ID非法*/
																														/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_radio_dtim_group(struct RadioList *RadioList_Head);

/*返回-6时，调用Free_config_radio_dtim_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断Dtim的范围，默认是1 ，取值范围是1-15*/
extern int config_radio_dtim_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int Dtim,struct RadioList **RadioList_Head);
													/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
													/*返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
													/*返回-4表示Radio ID非法，返回-6表示partial failure*/
													/*返回-7表示group id does not exist*/
#endif

extern int config_radio_dtim(dbus_parameter parameter, DBusConnection *connection,int id,int Dtim);    /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错，返回-4表示Radio ID非法*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_radio_rtsthreshold_group(struct RadioList *RadioList_Head);

/*返回-6时，调用Free_config_radio_rtsthreshold_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断TRS的范围，默认是2346 ，取值范围是256 - 2347*/
extern int config_radio_rtsthreshold_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int RTS,struct RadioList **RadioList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
										/*返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
										/*返回-4表示Radio ID非法，返回-5表示Group ID非法*/
										/*返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

extern int config_radio_rtsthreshold(dbus_parameter parameter, DBusConnection *connection,int id,int RTS);    /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错，返回-4表示Radio ID非法*/
																												 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_radio_service_group(struct RadioList *RadioList_Head);

/*返回-7时，调用Free_config_radio_service_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int config_radio_service_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct RadioList **RadioList_Head);
													/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
													/*返回-2表示wtp not in run state，返回-3表示出错，返回-4表示Radio ID非法*/
													/*返回-5表示input patameter should only be 'enable' or 'disable'，返回-6表示Group ID非法*/
													/*返回-7表示partial failure，返回-8表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/  
extern int config_radio_service(dbus_parameter parameter, DBusConnection *connection,int id,char *state); /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示wtp not in run state，返回-3表示出错，返回-4表示Radio ID非法*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_radio_preamble_group(struct RadioList *RadioList_Head);

/*返回-7时，调用Free_config_radio_preamble_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*Type为"short"或"long"*/
extern int config_radio_preamble_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Type,struct RadioList **RadioList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
										/*返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
										/*返回-4表示Radio ID非法，返回-5表示input parameter should only be 'long' or 'short'*/
										/*返回-6表示Group ID非法	，返回-7表示partial failure，返回-8表示group id does not exist*/
#endif

/*Type为"short"或"long"*/	
extern int config_radio_preamble(dbus_parameter parameter, DBusConnection *connection,int id,char *Type);/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错，返回-4表示Radio ID非法*/
																										 	 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_radio_longretry_group(struct RadioList *RadioList_Head);

/*返回-6时，调用Free_config_radio_longretry_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断Lretry的范围，默认值是4， 取值范围是 1 -15*/
extern int config_radio_longretry_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int Lretry,struct RadioList **RadioList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
										/*返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
										/*返回-4表示Radio ID非法，返回-5表示Group ID非法*/
									 	/*返回-6表示partial failure，返回-7表示group id does not exist*/
#endif

extern int config_radio_longretry(dbus_parameter parameter, DBusConnection *connection,int id,int Lretry);  /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,	please enable it first，返回-3表示出错，返回-4表示Radio ID非法*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_config_radio_shortretry_group(struct RadioList *RadioList_Head);

/*返回-6时，调用Free_config_radio_shortretry_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*注意判断Sretry的范围，默认值是7， 取值范围是 1 -15*/
extern int config_radio_shortretry_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,int Sretry,struct RadioList **RadioList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在*/
										/*返回-2表示Radio is disable,  please enable it first，返回-3表示出错*/
										/*返回-4表示Radio ID非法，返回-6表示partial failure*/
										/*返回-7表示group id does not exist*/

#endif

extern int config_radio_shortretry(dbus_parameter parameter, DBusConnection *connection,int id,int Sretry); /*返回0表示失败，返回1表示成功，返回-1表示radio id 不存在，返回-2表示Radio is disable,  please enable it first，返回-3表示出错，返回-4表示Radio ID非法*/
																											   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_bss_max_sta_num_group(struct RadioList *RadioList_Head);

/*返回-9时，调用Free_set_bss_max_sta_num_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_bss_max_sta_num_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Wlan_id,char*bss_num,struct RadioList **RadioList_Head);
									/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
                                    /*返回-2表示wlanid should be 1 to WLAN_NUM，返回-3表示max station number should be greater than 0,and not cross 32767*/
                                    /*返回-4表示bss not exist，返回-5表示more sta(s) has accessed before you set max sta num */
                                    /*返回-6表示operation fail!，返回-7表示Radio ID非法，返回-8表示Group ID非法*/
									/*返回-9表示partial failure，返回-10表示group id does not exist，返回-11表示wlan is not binded radio*/
									/*返回-12表示bss is not exist*/
#endif

/*bss_num的范围是0-32767*/
extern int set_bss_max_sta_num(dbus_parameter parameter, DBusConnection *connection,int id,char *Wlan_id,char*bss_num);
																						/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
				                                                                        /*返回-2表示wlanid should be 1 to WLAN_NUM，返回-3表示max station number should be greater than 0,and not cross 32767*/
				                                                                        /*返回-4表示bss not exist，返回-5表示more sta(s) has accessed before you set max sta num */
				                                                                        /*返回-6表示operation fail!，返回-7表示Radio ID非法，返回-11表示wlan is not binded radio*/	
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_bss_l3_policy_group(struct RadioList *RadioList_Head);

/*返回-17时，调用Free_set_radio_bss_l3_policy_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_bss_l3_policy_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,char *bss_policy,struct RadioList **RadioList_Head);
													/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
													/*返回-2表示BSS is enable, if you want to operate this, please disable it first*/
													/*返回-3表示BSS l3 interface is exist,you should delete this interface first*/
													/*返回-4表示BSS create l3 interface fail，返回-5表示BSS delete l3 interface fail*/
													/*返回-6表示WLAN policy is NO_INTERFACE,can not use this command*/
													/*返回-7表示 BSS is not exist，返回-8表示can not use this command*/
													/*返回-9表示RADIO is not exist，返回-10表示WTP is not exist*/
													/*返回-11表示WTP is not binding wlan，返回-12表示WLAN br is not exist*/
													/*返回-13表示add bss interface to wlan br fail，返回-14表示remove bss interface from wlan br fail*/
													/*返回-15表示Radio ID非法，返回-16表示Group ID非法*/
													/*返回-17表示partial failure，返回-18表示group id does not exist*/
													/*返回-19表示input parameter should be 1 to WLAN_NUM，返回-20表示wlan id is not exist*/
													/*返回-21表示this wlan id is not binding radio*/
#endif	

extern int set_radio_bss_l3_policy(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlan_id,char *bss_policy); 
																							   /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																							   /*返回-2表示BSS is enable, if you want to operate this, please disable it first*/
														  									   /*返回-3表示BSS l3 interface is exist,you should delete this interface first*/
																							   /*返回-4表示BSS create l3 interface fail，返回-5表示BSS delete l3 interface fail*/
																							   /*返回-6表示WLAN policy is NO_INTERFACE,can not use this command*/
																							   /*返回-7表示 BSS is not exist，返回-8表示can not use this command*/
																							   /*返回-9表示RADIO is not exist，返回-10表示WTP is not exist*/
																							   /*返回-11表示WTP is not binding wlan，返回-12表示WLAN br is not exist*/
																							   /*返回-13表示add bss interface to wlan br fail，返回-14表示remove bss interface from wlan br fail*/
																							   /*返回-15表示Radio ID非法，返回-19表示input parameter should be 1 to WLAN_NUM*/
																							   /*返回-20表示wlan id is not exist，返回-21表示this wlan id is not binding radio*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_apply_wlan_group(struct RadioList *RadioList_Head);

/*返回-16时，调用Free_radio_apply_wlan_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_apply_wlan_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,struct RadioList **RadioList_Head);
										   /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
										   /*返回-2表示WLAN ID非法，返回-3表示radio is not exist，返回-4表示WLAN is not exist*/
										   /*返回-5表示bss num is already L_BSS_NUM，返回-6表示wtp wlan binding interface not match*/
										   /*返回-7表示wtp not bind interface，返回-8表示wlan not bind interface*/
										   /*返回-9表示wlan create wlan bridge fail，返回-10表示add bss if to wlan bridge fail*/
										   /*返回-11表示wtp over max wep wlan count 4，返回-12表示Radio ID非法*/
										   /*返回-13表示illegal input:Input exceeds the maximum value of the parameter type*/
										   /*返回-14表示radio apply bingding securityindex is same with other*/
										   /*返回-15表示Group ID非法，返回-16表示partial failure*/
										   /*返回-17表示group id does not exist*/
										   /*返回-18表示radio has been binded this wlan already ,if you want use other ESSID,please unbind it first!*/
#endif

extern int radio_apply_wlan(dbus_parameter parameter, DBusConnection *connection,int rid,char *wlan_id);	
																		/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
				 														/*返回-2表示WLAN ID非法，返回-3表示radio is not exist，返回-4表示WLAN is not exist*/
																		/*返回-5表示bss num is already L_BSS_NUM，返回-6表示wtp wlan binding interface not match*/
																		/*返回-7表示wtp not bind interface，返回-8表示wlan not bind interface*/
																		/*返回-9表示wlan create wlan bridge fail，返回-10表示add bss if to wlan bridge fail*/
																		/*返回-11表示wtp over max wep wlan count 4，返回-12表示Radio ID非法*/
																		/*返回-13表示illegal input:Input exceeds the maximum value of the parameter type*/
																		/*返回-14表示radio apply bingding securityindex is same with other*/
																		/*返回-18表示radio has been binded this wlan already ,if you want use other ESSID,please unbind it first!*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_apply_qos_group(struct RadioList *RadioList_Head);

/*返回-7时，调用Free_radio_apply_qos_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*qos_id范围1-15*/
extern int radio_apply_qos_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *qos_id,struct RadioList **RadioList_Head);
												/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
												/*返回-2表示QOS ID非法，返回-3表示radio is not exist*/
												/*返回-4表示QOS is not exist，返回-5表示Radio ID非法*/
												/*返回-6表示Group ID非法，返回-7表示partial failure*/
												/*返回-8表示group id does not exist*/
#endif

/*qos_id范围1-15*/
extern int radio_apply_qos(dbus_parameter parameter, DBusConnection *connection,int rid,char *qos_id);   
																	   /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																	   /*返回-2表示QOS ID非法，返回-3表示radio is not exist*/
																	   /*返回-4表示QOS is not exist，返回-5表示Radio ID非法*/
																	   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_delete_qos_group(struct RadioList *RadioList_Head);

/*返回-8时，调用Free_radio_delete_qos_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*qos_id范围1-15*/
extern int radio_delete_qos_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *qos_id,struct RadioList **RadioList_Head);
												   /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
												   /*返回-2表示QOS ID非法，返回-3表示radio is not exist，返回-4表示QOS is not exist*/
												   /*返回-5表示radio is enable,please disable it first，返回-6表示Radio ID非法*/
												   /*返回-7表示Group ID非法，返回-8表示partial failure，返回-9表示group id does not exist*/
#endif

/*qos_id范围1-15*/
extern int radio_delete_qos(dbus_parameter parameter, DBusConnection *connection,int rid,char *qos_id);	
																		/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																	    /*返回-2表示QOS ID非法，返回-3表示radio is not exist，返回-4表示QOS is not exist*/
																	    /*返回-5表示radio is enable,please disable it first，返回-6表示Radio ID非法*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_bss_max_throughput_group(struct RadioList *RadioList_Head);

/*返回-7时，调用Free_set_bss_max_throughput_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_bss_max_throughput_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WLAN_ID,char *value,struct RadioList **RadioList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示BSS is not exist*/
										/*返回-2表示WTP is not exist，返回-3表示RADIO is not exist，返回-4表示error*/
										/*返回-5表示Radio ID非法，返回-6表示Group ID非法*/
										/*返回-7表示partial failure，返回-8表示group id does not exist*/
										/*返回-9表示input parameter error，返回-10表示input parameter should be 1 to WLAN_NUM*/
										/*返回-11表示input parameter should be 1-30，返回-12表示wlan id is not exist*/
#endif

extern int set_bss_max_throughput(dbus_parameter parameter, DBusConnection *connection,int RadioID,char *WLAN_ID,char *value);   
												/*返回0表示失败，返回1表示成功，返回-1表示BSS is not exist*/
												/*返回-2表示WTP is not exist，返回-3表示RADIO is not exist*/
												/*返回-4表示error，返回-5表示Radio ID非法*/
												/*返回-9表示input parameter error，返回-10表示input parameter should be 1 to WLAN_NUM*/
												/*返回-11表示input parameter should be 1-30，返回-12表示wlan id is not exist*/
												/*返回SNMPD_CONNECTION_ERROR表示connection error*/

																		
extern void Free_radio_bss_max_throughput_head(DCLI_RADIO_API_GROUP_ONE *RADIO);
/*返回1时，调用Free_radio_bss_max_throughput_head()释放空间*/
extern int show_radio_bss_max_throughput(dbus_parameter parameter, DBusConnection *connection,int radio_id ,DCLI_RADIO_API_GROUP_ONE **RADIO);
																												 /*返回0表示失败，返回1表示成功*/
																												 /*返回-1表示RADIO is not exist*/
																												 /*返回-2表示WTP is not exist*/
																												 /*返回-3表示error，返回-4表示Radio ID非法*/
																												 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_delete_wlan_cmd_group(struct RadioList *RadioList_Head);

/*返回-9时，调用Free_set_radio_delete_wlan_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_delete_wlan_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,struct RadioList **RadioList_Head);
											 /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
											 /*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
											 /*返回-4表示wlan not exist，返回-5表示radio delete wlan fail，返回-6表示Radio ID非法*/
											 /*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
											 /*返回-8表示Group ID非法，返回-9表示partial failure，返回-10表示group id does not exist*/
											 /*返回-12表示please delete radio interface from ebr first*/
											 /*返回-14表示radio interface is binded to this wlan used other ESSID，返回-15表示please disable wlan service first*/
#endif

extern int set_radio_delete_wlan_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *wlan_id);  
																					/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																					/*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
																					/*返回-4表示wlan not exist，返回-5表示radio delete wlan fail，返回-6表示Radio ID非法*/
																					/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type，返回-11表示bss is enable*/
																					/*返回-12表示radio interface is in ebr,please delete it from ebr first*/
																					/*返回-13表示you want to delete wlan, please do not operate like this*/
																					/*返回-14表示radio interface is binded to this wlan used other ESSID*/
																					/*返回-15表示please disable wlan service first*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_enable_wlan_cmd_group(struct RadioList *RadioList_Head);

/*返回-14时，调用Free_set_radio_enable_wlan_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_enable_wlan_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,struct RadioList **RadioList_Head);
												/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
												/*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
												/*返回-4表示wlan not exist， 返回-5表示wtp over max wep wlan count 4*/
												/*返回-6表示radio is already enable this wlan，返回-7表示wtp binding interface not match wlan binding interface*/
												/*返回-8表示radio is not binding this wlan，返回-9表示wlan is disable ,you should enable it first*/
												/*返回-10表示radio enable wlan fail，返回-11表示Radio ID非法*/
												/*返回-12表示illegal input:Input exceeds the maximum value of the parameter type*/
												/*返回-13表示Group ID非法，返回-14表示partial failure，返回-15表示group id does not exist*/
#endif

extern int set_radio_enable_wlan_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *wlan_id);  
																					/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																					/*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
																					/*返回-4表示wlan not exist， 返回-5表示wtp over max wep wlan count 4*/
																					/*返回-6表示radio is already enable this wlan，返回-7表示wtp binding interface not match wlan binding interface*/
																					/*返回-8表示radio is not binding this wlan，返回-9表示wlan is disable ,you should enable it first*/
																					/*返回-10表示radio enable wlan fail，返回-11表示Radio ID非法*/
																					/*返回-12表示illegal input:Input exceeds the maximum value of the parameter type*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_disable_wlan_cmd_group(struct RadioList *RadioList_Head);

/*返回-9时，调用Free_set_radio_disable_wlan_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_disable_wlan_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,struct RadioList **RadioList_Head);
													/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
													/*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
													/*返回-4表示wlan not exist，返回-5表示radio disable wlan fail，返回-6表示Radio ID非法*/
													/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
													/*返回-8表示Group ID非法，返回-9表示partial failure，返回-10表示group id does not exist*/
#endif

extern int set_radio_disable_wlan_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *wlan_id); 
																					/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																					/*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
																					/*返回-4表示wlan not exist，返回-5表示radio disable wlan fail，返回-6表示Radio ID非法*/
																					/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					
extern void Free_radio_channel_change(DCLI_RADIO_API_GROUP_ONE *RADIO);
/*返回1时，调用Free_radio_channel_change()释放空间*/
extern int show_radio_channel_change_func(dbus_parameter parameter, DBusConnection *connection,int rad_id,DCLI_RADIO_API_GROUP_ONE **RADIO);/*返回0表示失败，返回1表示成功，返回-1表示RADIO is not exist，返回-2表示WTP is not exist，返回-3表示error，返回-4表示Radio ID非法*/
																																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_apply_wlan_base_vlan_cmd_group(struct RadioList *RadioList_Head);

/*返回-20时，调用Free_radio_apply_wlan_base_vlan_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_apply_wlan_base_vlan_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *VlanID,struct RadioList **RadioList_Head);
													/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
													/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示vlan id should be 1 to VLANID_RANGE_MAX*/
													/*返回-4表示radio id does not exist，返回-5表示wtp is in use, you should unused it first*/
													/*返回-6表示binding wlan does not exist，返回-7表示wlan does not bind interface*/
													/*返回-8表示wtp does not bind interface，返回-9表示wlan and wtp bind interface did not match*/
													/*返回-10表示clear wtp binding wlan list successfully，返回-11表示wlan is enable,you should disable it first*/
													/*返回-12表示wtp over max bss count，返回-13表示bss is enable, you should disable it first*/
													/*返回-14表示wtp over max wep wlan count 4，返回-15表示error，返回-16表示Radio ID非法*/
													/*返回-17表示illegal input:Input exceeds the maximum value of the parameter type*/
													/*返回-18表示radio apply bingding securityindex is same with other*/
													/*返回-19表示Group ID非法，返回-20表示partial failure，返回-21表示group id does not exist*/
#endif

extern int radio_apply_wlan_base_vlan_cmd(dbus_parameter parameter, DBusConnection *connection,int RadioID,char *WlanID,char *VlanID);
																										/*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																										/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示vlan id should be 1 to VLANID_RANGE_MAX*/
																										/*返回-4表示radio id does not exist，返回-5表示wtp is in use, you should unused it first*/
																										/*返回-6表示binding wlan does not exist，返回-7表示wlan does not bind interface*/
																										/*返回-8表示wtp does not bind interface，返回-9表示wlan and wtp bind interface did not match*/
																										/*返回-10表示clear wtp binding wlan list successfully，返回-11表示wlan is enable,you should disable it first*/
																										/*返回-12表示wtp over max bss count，返回-13表示bss is enable, you should disable it first*/
																										/*返回-14表示wtp over max wep wlan count 4，返回-15表示error，返回-16表示Radio ID非法*/
																										/*返回-17表示illegal input:Input exceeds the maximum value of the parameter type*/
																										/*返回-18表示radio apply bingding securityindex is same with other*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_default_config_cmd_func_group(struct RadioList *RadioList_Head);

/*返回-4时，调用Free_set_radio_default_config_cmd_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_default_config_cmd_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,struct RadioList **RadioList_Head);
									/*返回0表示失败，返回1表示成功，返回-1表示recover default config fail*/
									/*返回-2表示Radio ID非法，返回-3表示Group ID非法*/
									/*返回-4表示partial failure，返回-5表示group id does not exist*/
#endif

extern int set_radio_default_config_cmd_func(dbus_parameter parameter, DBusConnection *connection,int rad_id);/*返回0表示失败，返回1表示成功，返回-1表示recover default config fail，返回-2表示Radio ID非法*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_max_throughout_func_group(struct RadioList *RadioList_Head);

/*返回-8时，调用Free_set_radio_max_throughout_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_max_throughout_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *value,struct RadioList **RadioList_Head);
												/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
												/*返回-2表示max throughout should be 1 to 108，返回-3表示WTP id does not exist*/
												/*返回-4表示radio id does not exist，返回-5表示error，返回-6表示Radio ID非法*/
												/*返回-7表示Group ID非法，返回-8表示partial failure，返回-9表示group id does not exist*/
#endif

extern int set_radio_max_throughout_func(dbus_parameter parameter, DBusConnection *connection,int rad_id,char *value);
																						/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																						/*返回-2表示max throughout should be 1 to 108，返回-3表示WTP id does not exist*/
																						/*返回-4表示radio id does not exist，返回-5表示error，返回-6表表示Radio ID非法*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						
extern void Free_show_radio_qos(DCLI_RADIO_API_GROUP_ONE *RADIO);
/*返回1时，调用Free_show_radio_qos()释放空间*/
extern int show_radio_qos_cmd_func(dbus_parameter parameter, DBusConnection *connection,DCLI_RADIO_API_GROUP_ONE **RADIO);/*返回0表示失败，返回1表示成功，返回-1表示there is no radio exist*/
																																 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_radio_bss_head(DCLI_RADIO_API_GROUP_ONE *RADIO);
/*返回1时，调用Free_radio_bss_head()释放空间*/
extern int show_radio_bss_cmd(dbus_parameter parameter, DBusConnection *connection,int radio_id,DCLI_RADIO_API_GROUP_ONE **RADIO);/*返回0表示失败，返回1表示成功，返回-1表示Radio ID非法*/
																																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_l2_isolation_func_group(struct RadioList *RadioList_Head);

/*返回-12时，调用Free_set_radio_l2_isolation_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_l2_isolation_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,char *state,struct RadioList **RadioList_Head);
												/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
												/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
												/*返回-4表示wlan not exist，返回-5表示wtp not binding wlan，返回-6表示wtp id does not run*/
												/*返回-7表示binding wlan error，返回-8表示error，返回-9表示Radio ID非法*/
												/*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
												/*返回-11表示Group ID非法，返回-12表示partial failure，返回-13表示group id does not exist*/
#endif

extern int set_radio_l2_isolation_func(dbus_parameter parameter, DBusConnection *connection,int radio_id,char *wlan_id,char *state);  
																									/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																									/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
																									/*返回-4表示wlan not exist，返回-5表示wtp not binding wlan，返回-6表示wtp id does not run*/
																									/*返回-7表示binding wlan error，返回-8表示error，返回-9表示Radio ID非法*/
																									/*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_11n_cwmmode_func_group(struct RadioList *RadioList_Head);

/*返回-12时，调用Free_set_radio_11n_cwmmode_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_11n_cwmmode_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,char *cwmmode,struct RadioList **RadioList_Head);
													/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
													/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
													/*返回-4表示wlan not exist，返回-5表示wtp not binding wlan，返回-6表示wtp id does not run*/
													/*返回-7表示binding wlan error，返回-8表示error，返回-9表示Radio ID非法*/
													/*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
													/*返回-11表示Group ID非法，返回-12表示partial failure，返回-13表示group id does not exist*/
#endif

extern int set_radio_11n_cwmmode_func(dbus_parameter parameter, DBusConnection *connection,int radio_id,char *wlan_id,char *cwmmode);
																										/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																										/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
																										/*返回-4表示wlan not exist，返回-5表示wtp not binding wlan，返回-6表示wtp id does not run*/
																										/*返回-7表示binding wlan error，返回-8表示error，返回-9表示Radio ID非法*/
																										/*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
#if _GROUP_POLICY
extern void Free_set_wds_service_cmd_group(struct RadioList *RadioList_Head);

/*返回-12时，调用Free_set_wds_service_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_wds_service_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanId,char *state,struct RadioList **RadioList_Head);
									/*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'enable' or 'disable'*/
									/*返回-2表示wlan isn't existed，返回-3表示radio doesn't bind wlan argv[0]，返回-4表示radio id does not exist*/
									/*返回-5表示wtp is not in run state，返回-6表示another wds mode be used，返回-7表示error*/
									/*返回-8表示Radio ID非法，返回-9表示illegal input:Input exceeds the maximum value of the parameter type*/
									/*返回-10表示unknown id format，返回-11表示Group ID非法，返回-12表示partial failure*/
									/*返回-13表示group id does not exist*/
#endif
/*type为"wds"或"mesh"*/
/*state为"enable"或"disable"*/
extern int set_wds_service_cmd(dbus_parameter parameter, DBusConnection *connection,int RadID,char *WlanId,char *type,char *state);
																						/*返回0表示失败，返回1表示成功，返回-1表示input patameter should only be 'enable' or 'disable'*/
																						/*返回-2表示wlan isn't existed，返回-3表示radio doesn't bind wlan argv[0]，返回-4表示radio id does not exist*/
																						/*返回-5表示wtp is not in run state，返回-6表示another wds mode be used，返回-7表示error*/
																						/*返回-8表示Radio ID非法，返回-9表示illegal input:Input exceeds the maximum value of the parameter type*/
																						/*返回-10表示unknown id format，返回-11表示input patameter should only be 'wds' or 'mesh'*/
																						/*返回-12表示another mesh mode be used*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_wlan_wds_bssid_cmd_group(struct RadioList *RadioList_Head);

/*返回-10时，调用Free_radio_wlan_wds_bssid_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_wlan_wds_bssid_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanId,char *oper_type,char *MAC,struct RadioList **RadioList_Head);
										/*返回0表示失败，返回1表示成功，返回-1表示wlan id should be 1 to WLAN_NUM-1*/
										/*返回-2表示input patameter should only be 'add' or 'delelte'，返回-3表示Unknown mac addr format*/
										/*返回-4表示wlan isn't existed，返回-5表示radio doesn't bind wlan argv[0]*/
										/*返回-6表示another WDS mode be used, disable first，返回-7表示WDS op failed，返回-8表示Radio ID非法*/
										/*返回-9表示Group ID非法，返回-10表示partial failure，返回-11表示group id does not exist*/

#endif
/*type为"add"或"delete"*/
/*Ltype为"wds_bssid"或"mesh_bssid"*/
extern int radio_wlan_wds_bssid_cmd(dbus_parameter parameter, DBusConnection *connection,int RadID,char *WlanId,char *type,char *Ltype,char *MAC);
																										/*返回0表示失败，返回1表示成功，返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																										/*返回-2表示input patameter should only be 'add' or 'delelte'，返回-3表示Unknown mac addr format*/
																										/*返回-4表示wlan isn't existed，返回-5表示radio doesn't bind wlan argv[0]*/
																										/*返回-6表示another WDS mode be used, disable first，返回-7表示WDS op failed*/
																										/*返回-8表示Radio ID非法，返回-9表示input patameter should only be 'wds_bssid' or 'mesh_bssid'*/
																										/*返回-10表示another Mesh mode be used, disable first，返回-11表示Mesh op faild*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										
extern void Free_wlan_wds_bssid_list_head(DCLI_RADIO_API_GROUP_ONE *RADIOINFO);
/*type为"wds_bssid_list"或"mesh_bssid_list"*/
/*返回1时，调用Free_wlan_wds_bssid_list_head()释放空间*/																						
extern int show_wlan_wds_bssid_list_cmd(dbus_parameter parameter, DBusConnection *connection,int RadID,char *WlanId,char *type,DCLI_RADIO_API_GROUP_ONE **RADIOINFO);
																											/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																											/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示wlan id does not exist*/
																											/*返回-4表示error，返回-5表示Radio ID非法*/
																											/*返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
																											/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_radio_auto_channel_func_group(struct RadioList *RadioList_Head);

/*返回-7时，调用Free_set_ap_radio_auto_channel_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_radio_auto_channel_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct RadioList **RadioList_Head);
								/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
								/*返回-2表示wtp id does not exist，返回-3表示radio id does not exist，返回-4表示error*/
								/*返回-5表示Radio ID非法，返回-6表示Group ID非法，返回-7表示partial failure*/
								/*返回-8表示group id does not exist*/
#endif

extern int set_ap_radio_auto_channel_func(dbus_parameter parameter, DBusConnection *connection,int RID,char *state);
																					 /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
																					 /*返回-2表示wtp id does not exist，返回-3表示radio id does not exist，返回-4表示error，返回-5表示Radio ID非法*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_radio_auto_channel_cont_cmd_group(struct RadioList *RadioList_Head);

/*返回-7时，调用Free_set_ap_radio_auto_channel_cont_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_radio_auto_channel_cont_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct RadioList **RadioList_Head);
																 /*返回0表示失败，返回1表示成功*/
																 /*返回-1表示input patameter only with 'enable' or 'disable'*/
																 /*返回-2表示wtp id does not exist*/
																 /*返回-3表示radio id does not exist，返回-4表示error*/
																 /*返回-5表示Radio ID非法，返回-6表示Group ID非法*/
																 /*返回-7表示partial failure，返回-8表示group id does not exist*/
#endif

/*state为enable或disable*/
extern int set_ap_radio_auto_channel_cont_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *state); 
																							 /*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示input patameter only with 'enable' or 'disable'*/
																							 /*返回-2表示wtp id does not exist*/
																							 /*返回-3表示radio id does not exist，返回-4表示error*/
																							 /*返回-5表示Radio ID非法*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_radio_diversity_func_group(struct RadioList *RadioList_Head);

/*返回-9时，调用Free_set_ap_radio_diversity_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_radio_diversity_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct RadioList **RadioList_Head);
						 /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
						 /*返回-2表示wtp id does not exist，返回-3表示radio id does not exist*/
						 /*返回-4表示radio model not petmit to set diversity，返回-5表示error，返回-6表示Radio ID非法*/
						 /*返回-7表示Group ID非法，返回-8表示to enable this function, you should restart wtp*/
						 /*返回-9表示partial failure，返回-10表示group id does not exist*/
#endif

extern int set_ap_radio_diversity_func(dbus_parameter parameter, DBusConnection *connection,int RID,char *state); 
																				 /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'disable'*/
																				 /*返回-2表示wtp id does not exist，返回-3表示radio id does not exist*/
																				 /*返回-4表示radio model not petmit to set diversity，返回-5表示error，返回-6表示Radio ID非法*/
																				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_ap_radio_txantenna_func_group(struct RadioList *RadioList_Head);

/*返回-7时，调用Free_set_ap_radio_txantenna_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_ap_radio_txantenna_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *txan_type,struct RadioList **RadioList_Head);
					 /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'auto' 'main' or 'vice'*/
					 /*返回-2表示wtp id does not exist，返回-3表示radio id does not exist，返回-4表示error，返回-5表示Radio ID非法*/
					 /*返回-6表示Group ID非法，返回-7表示partial failure，返回-8表示group id does not exist*/
#endif

extern int set_ap_radio_txantenna_func(dbus_parameter parameter, DBusConnection *connection,int RID,char *txan_type);
																					 /*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'auto' 'main' or 'vice'*/
																					 /*返回-2表示wtp id does not exist，返回-3表示radio id does not exist，返回-4表示error，返回-5表示Radio ID非法*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_bss_traffic_limit_cmd_group(struct RadioList *RadioList_Head);

/*返回-10时，调用Free_radio_bss_traffic_limit_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int radio_bss_traffic_limit_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *state,struct RadioList **RadioList_Head);
														 /*返回0表示失败，返回1表示成功*/
														 /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
														 /*返回-2表示input patameter only with 'enable' or 'disable'*/
														 /*返回-3表示wlan isn't existed，返回-4表示wtp isn't existed*/
														 /*返回-5表示radio isn't existed*/
														 /*返回-6表示radio doesn't bind wlan argv[0]，返回-7表示error*/
														 /*返回-8表示Radio ID非法，返回-9表示Group ID非法*/
														 /*返回-10表示partial failure，返回-11表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int radio_bss_traffic_limit_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *state);
																							/*返回0表示失败，返回1表示成功*/
																						    /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																						    /*返回-2表示input patameter only with 'enable' or 'disable'*/
																						    /*返回-3表示wlan isn't existed，返回-4表示wtp isn't existed*/
																						    /*返回-5表示radio isn't existed*/
																						    /*返回-6表示radio doesn't bind wlan argv[0]，返回-7表示error*/
																							/*返回-8表示Radio ID非法*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_bss_traffic_limit_value_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_radio_bss_traffic_limit_value_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*VALUE的范围是1-884736*/
extern int radio_bss_traffic_limit_value_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *VALUE,struct RadioList **RadioList_Head);
													/*返回0表示失败，返回1表示成功*/
													/*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
													/*返回-2表示input parameter argv[1] error*/
													/*返回-3表示wlan isn't existed，返回-4表示wtp isn't existed*/
													/*返回-5表示radio isn't existed*/
													/*返回-6表示radio doesn't bind wlan argv[0]，返回-7表示error*/
													/*返回-8表示Radio ID非法，返回-9表示input value should be 1 to 884736*/
													/*返回-10表示Group ID非法，返回-11表示partial failure*/
													/*返回-12表示group id does not exist*/
#endif

/*VALUE的范围是1-884736*/
extern int radio_bss_traffic_limit_value_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *VALUE);
																								   /*返回0表示失败，返回1表示成功*/
																								   /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																								   /*返回-2表示input parameter argv[1] error*/
																								   /*返回-3表示wlan isn't existed，返回-4表示wtp isn't existed*/
																								   /*返回-5表示radio isn't existed*/
																								   /*返回-6表示radio doesn't bind wlan argv[0]，返回-7表示error*/
																								   /*返回-8表示Radio ID非法，返回-9表示input value should be 1 to 884736*/
																								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_bss_traffic_limit_average_value_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_radio_bss_traffic_limit_average_value_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_bss_traffic_limit_average_value_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *VALUE,struct RadioList **RadioList_Head);
												   /*返回0表示失败，返回1表示成功*/
												   /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
												   /*返回-2表示input parameter argv[1] error*/
												   /*返回-3表示wlan isn't existed，返回-4表示wtp isn't existed*/
												   /*返回-5表示radio isn't existed，返回-6表示radio doesn't bind wlan argv[0]*/
												   /*返回-7表示station traffic limit value is more than bss traffic limit value*/
												   /*返回-8表示error，返回-9表示Radio ID非法*/
												   /*返回-10表示Group ID非法，返回-11表示partial failure*/
												   /*返回-12表示group id does not exist，返回-13表示input value should be 1 to 884736*/
#endif

extern int radio_bss_traffic_limit_average_value_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *VALUE);
																											 /*返回0表示失败，返回1表示成功*/
																										     /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																										     /*返回-2表示input parameter argv[1] error*/
																										     /*返回-3表示wlan isn't existed，返回-4表示wtp isn't existed*/
																										     /*返回-5表示radio isn't existed，返回-6表示radio doesn't bind wlan argv[0]*/
																										     /*返回-7表示station traffic limit value is more than bss traffic limit value*/
																										     /*返回-8表示error，返回-9表示Radio ID非法*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											 /*返回-13表示input value should be 1 to 884736*/
																											 
extern int radio_bss_traffic_limit_sta_value_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *MAC,char *VALUE);
																											 	  /*返回0表示失败，返回1表示成功*/
																												  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																												  /*返回-2表示input parameter argv[2] error*/
																												  /*返回-3表示wlan doesn't work*/
																												  /*返回-4表示can't find sta under wlan WlanID*/
																												  /*返回-5表示station traffic limit value is more than bss traffic limit value*/
																												  /*返回-6表示error，返回-7表示Radio ID非法*/
																												  /*返回-8表示input value should be 1 to 884736*/

#if _GROUP_POLICY
extern void Free_radio_bss_traffic_limit_cancel_sta_value_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_radio_bss_traffic_limit_cancel_sta_value_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_bss_traffic_limit_cancel_sta_value_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *MAC,struct RadioList **RadioList_Head);
															  /*返回0表示失败，返回1表示成功*/
															  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
															  /*返回-2表示wlan doesn't work*/
															  /*返回-3表示can't find sta under wlan WlanID*/
															  /*返回-4表示wlan isn't existed，返回-5表示wtp isn't existed*/
															  /*返回-6表示radio isn't existed*/
															  /*返回-7表示radio doesn't bind wlan argv[0]，返回-8表示error*/
															  /*返回-9表示Radio ID非法，返回-10表示Group ID非法*/
															  /*返回-11表示partial failure，返回-12表示group id does not exist*/
#endif

extern int radio_bss_traffic_limit_cancel_sta_value_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *MAC);
																											  /*返回0表示失败，返回1表示成功*/
																										      /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																										      /*返回-2表示wlan doesn't work*/
																										      /*返回-3表示can't find sta under wlan WlanID*/
																										      /*返回-4表示wlan isn't existed，返回-5表示wtp isn't existed*/
																										      /*返回-6表示radio isn't existed*/
																										      /*返回-7表示radio doesn't bind wlan argv[0]，返回-8表示error*/
																											  /*返回-9表示Radio ID非法*/
																											  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_apply_wlan_clean_vlan_cmd_func_group(struct RadioList *RadioList_Head);

/*返回-12时，调用Free_radio_apply_wlan_clean_vlan_cmd_func_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_apply_wlan_clean_vlan_cmd_func_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,struct RadioList **RadioList_Head);
									/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
									/*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示wtp id does not exist*/
									/*返回-4表示radio id does not exist，返回-5表示binding wlan does not exist*/
									/*返回-6表示radio is not binding this wlan，返回-7表示bss is enable, you should disable it first*/
									/*返回-8表示error，返回-9表示Radio ID非法*/
									/*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
									/*返回-11表示Group ID非法，返回-12表示partial failure*/
									/*返回-13表示group id does not exist*/
#endif

extern int radio_apply_wlan_clean_vlan_cmd_func(dbus_parameter parameter, DBusConnection *connection,int Rid,char *WlanID);
																							  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																							  /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示wtp id does not exist*/
																							  /*返回-4表示radio id does not exist，返回-5表示binding wlan does not exist*/
																							  /*返回-6表示radio is not binding this wlan，返回-7表示bss is enable, you should disable it first*/
																							  /*返回-8表示error，返回-9表示Radio ID非法*/
																							  /*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_radio_bss_traffic_limit_send_value_cmd_group(struct RadioList *RadioList_Head);

/*返回-10时，调用Free_radio_bss_traffic_limit_send_value_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_bss_traffic_limit_send_value_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *VALUE,struct RadioList **RadioList_Head);
														  /*返回0表示失败，返回1表示成功*/
														  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
														  /*返回-2表示input parameter argv[1] error*/
														  /*返回-3表示wtp isn't existed，返回-4表示radio isn't existed*/
														  /*返回-5表示radio doesn't bind wlan argv[0]*/
														  /*返回-6表示wlan isn't existed，返回-7表示error*/
														  /*返回-8表示Radio ID非法，返回-9表示Group ID非法*/
														  /*返回-10表示partial failure，返回-11表示group id does not exist*/
														  /*返回-12表示input value should be 1 to 884736*/
#endif				

extern int radio_bss_traffic_limit_send_value_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *VALUE);
																										  /*返回0表示失败，返回1表示成功*/
																										  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																										  /*返回-2表示input parameter argv[1] error*/
																										  /*返回-3表示wtp isn't existed，返回-4表示radio isn't existed*/
																										  /*返回-5表示radio doesn't bind wlan argv[0]*/
																										  /*返回-6表示wlan isn't existed，返回-7表示error*/
																										  /*返回-8表示Radio ID非法*/
																										  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										  /*返回-12表示input value should be 1 to 884736*/

#if _GROUP_POLICY
extern void Free_radio_bss_traffic_limit_average_send_value_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_radio_bss_traffic_limit_average_send_value_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_bss_traffic_limit_average_send_value_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *VALUE,struct RadioList **RadioList_Head);
												  /*返回0表示失败，返回1表示成功*/
												  /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
												  /*返回-2表示input parameter argv[1] error*/
												  /*返回-3表示station traffic limit value is more than bss traffic limit value*/
												  /*返回-4表示wtp isn't existed，返回-5表示radio isn't existed*/
												  /*返回-6表示radio doesn't bind wlan argv[0]*/
												  /*返回-7表示wlan isn't existed，返回-8表示error*/
												  /*返回-9表示Radio ID非法，返回-10表示Group ID非法*/
												  /*返回-11表示partial failure，返回-12表示group id does not exist*/
												  /*返回-13表示input value should be 1 to 884736*/
#endif

extern int radio_bss_traffic_limit_average_send_value_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *VALUE);
																													/*返回0表示失败，返回1表示成功*/
																												    /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																												    /*返回-2表示input parameter argv[1] error*/
																												    /*返回-3表示station traffic limit value is more than bss traffic limit value*/
																												    /*返回-4表示wtp isn't existed，返回-5表示radio isn't existed*/
																												    /*返回-6表示radio doesn't bind wlan argv[0]*/
																												    /*返回-7表示wlan isn't existed，返回-8表示error*/
																													/*返回-9表示Radio ID非法*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																													/*返回-13表示input value should be 1 to 884736*/

#if _GROUP_POLICY
extern void Free_radio_bss_traffic_limit_sta_send_value_cmd_group(struct RadioList *RadioList_Head);

/*返回-9时，调用Free_radio_bss_traffic_limit_sta_send_value_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_bss_traffic_limit_sta_send_value_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *MAC,char *VALUE,struct RadioList **RadioList_Head);
																	/*返回0表示失败，返回1表示成功*/
																	/*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																	/*返回-2表示input parameter argv[2] error*/
																	/*返回-3表示wlan doesn't work*/
																	/*返回-4表示can't find sta under wlan WlanID*/
																	/*返回-5表示station traffic limit value is more than bss traffic limit value*/
																	/*返回-6表示error，返回-7表示Radio ID非法*/
																	/*返回-8表示Group ID非法，返回-9表示partial failure*/
																	/*返回-10表示group id does not exist*/
																	/*返回-11表示input value should be 1 to 884736*/
#endif

extern int radio_bss_traffic_limit_sta_send_value_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *MAC,char *VALUE);
																														/*返回0表示失败，返回1表示成功*/
																													    /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																													    /*返回-2表示input parameter argv[2] error*/
																													    /*返回-3表示wlan doesn't work*/
																													    /*返回-4表示can't find sta under wlan WlanID*/
																													    /*返回-5表示station traffic limit value is more than bss traffic limit value*/
																												        /*返回-6表示error，返回-7表示Radio ID非法*/
																														/*返回-11表示input value should be 1 to 884736*/

#if _GROUP_POLICY
extern void Free_radio_bss_traffic_limit_cancel_sta_send_value_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_radio_bss_traffic_limit_cancel_sta_send_value_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int radio_bss_traffic_limit_cancel_sta_send_value_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *WlanID,char *MAC,struct RadioList **RadioList_Head);
																	 /*返回0表示失败，返回1表示成功*/
																	 /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																	 /*返回-2表示wlan doesn't work*/
																	 /*返回-3表示can't find sta under wlan WlanID*/
																	 /*返回-4表示wtp isn't existed，返回-5表示radio isn't existed*/
																	 /*返回-6表示radio doesn't bind wlan argv[0]*/
																	 /*返回-7表示wlan isn't existed，返回-8表示error*/
																	 /*返回-9表示Radio ID非法，返回-10表示Group ID非法*/
																	 /*返回-11表示partial failure，返回-12表示group id does not exist*/
#endif

extern int radio_bss_traffic_limit_cancel_sta_send_value_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *WlanID,char *MAC);
																													 /*返回0表示失败，返回1表示成功*/
																												     /*返回-1表示wlan id should be 1 to WLAN_NUM-1*/
																												     /*返回-2表示wlan doesn't work*/
																												     /*返回-3表示can't find sta under wlan WlanID*/
																												     /*返回-4表示wtp isn't existed，返回-5表示radio isn't existed*/
																												     /*返回-6表示radio doesn't bind wlan argv[0]*/
																												     /*返回-7表示wlan isn't existed，返回-8表示error*/
																													 /*返回-9表示Radio ID非法*/
																													 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_set_sta_mac_vlanid_cmd(struct dcli_sta_info *sta);
/*当flag为1时，调用Free_set_sta_mac_vlanid_cmd()释放空间*/																													 
extern int set_sta_mac_vlanid_cmd(dbus_parameter parameter, DBusConnection *connection,unsigned int *flag,char *sta_mac,char *vlanID);
																									  /*返回0表示失败，返回1表示，返回 -1表示vlan id should be 0 to 4095*/
																								      /*返回-2表示input parameter argv[2] error，返回-3表示set info error*/
																									  /*返回-4表示wid set error，返回-5表示can't find sta*/
																									  /*返回-6表示check sta set invalid value，返回-7表示check sta error*/

#if _GROUP_POLICY
extern void Free_set_sta_dhcp_before_authorized_cmd_group(struct RadioList *RadioList_Head);

/*返回-12时，调用Free_set_sta_dhcp_before_authorized_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_sta_dhcp_before_authorized_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,char *state,struct RadioList **RadioList_Head);
								   /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
								   /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
								   /*返回-4表示wlan not exist，返回-5表示wtp not binding wlan argv[0]*/
								   /*返回-6表示wtp id does not run，返回-7表示binding wlan error，返回-8表示error，返回-9表示Radio ID非法*/
								   /*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
								   /*返回-11表示Group ID非法，返回-12表示partial failure，返回-13表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_sta_dhcp_before_authorized_cmd(dbus_parameter parameter, DBusConnection *connection,int Rid,char *wlan_id,char *state);
																										 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																										 /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
																										 /*返回-4表示wlan not exist，返回-5表示wtp not binding wlan argv[0]*/
																										 /*返回-6表示wtp id does not run，返回-7表示binding wlan error，返回-8表示error，返回-9表示Radio ID非法*/
																										 /*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_sta_ip_mac_binding_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_set_sta_ip_mac_binding_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_sta_ip_mac_binding_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,char *state,struct RadioList **RadioList_Head);
							 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
							 /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
							 /*返回-4表示wlan not exist，返回-5表示wtp not binding wlan argv[0]*/
							 /*返回-6表示binding wlan error，返回-7表示error，返回-8表示Radio ID非法*/
							 /*返回-9表示illegal input:Input exceeds the maximum value of the parameter type*/
							 /*返回-10表示Group ID非法，返回-11表示partial failure，返回-12表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_sta_ip_mac_binding_cmd(dbus_parameter parameter, DBusConnection *connection,int Rid,char *wlan_id,char *state);
																								/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																							    /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示input patameter only with 'enable' or 'disable'*/
																							    /*返回-4表示wlan not exist，返回-5表示wtp not binding wlan argv[0]*/
																							    /*返回-6表示binding wlan error，返回-7表示error，返回-8表示Radio ID非法*/
																								/*返回-9表示illegal input:Input exceeds the maximum value of the parameter type*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_guard_interval_cmd_group(struct RadioList *RadioList_Head);

/*返回-8时，调用Free_set_radio_guard_interval_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*interval为"800"或"400"*/
extern int set_radio_guard_interval_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Interval,struct RadioList **RadioList_Head);
										 /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
										 /*返回-2表示radio id does not exist，返回-3表示radio is disable, please enable it first*/
										 /*返回-4表示radio is not binging wlan，返回-5表示error，返回-6表示Radio ID非法*/
										 /*返回-7表示Group ID非法，返回-8表示partial failure，返回-9表示group id does not exist*/
#endif

/*interval为"800"或"400"*/
extern int set_radio_guard_interval_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *Interval);
																					 /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																					 /*返回-2表示radio id does not exist，返回-3表示radio is disable, please enable it first*/
																					 /*返回-4表示radio is not binging wlan，返回-5表示error，返回-6表示Radio ID非法*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_mcs_cmd_group(struct RadioList *RadioList_Head);

/*返回-9时，调用Free_set_radio_mcs_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*mcs_value是一个以","分隔的数字集合*/
extern int set_radio_mcs_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *mcs_value,struct RadioList **RadioList_Head);
											/*返回0表示失败，返回1表示成功，返回-1表示parse mcs list failed*/
											/*返回-2表示radio id does not exist，返回-3表示radio is disable, please enable it first*/
											/*返回-4表示error，返回-5表示Radio ID非法，返回-6表示Group ID非法*/
										    /*返回-7表示partial failure，返回-8表示group id does not exist*/
										    /*返回-9表示mcs cross the border, if your stream is one,mcs should be 0~7,if your stream is two,mcs should be 8~15,and if your stream is three,mcs should be 16~23*/
#endif

/*mcs_value是一个以","分隔的数字集合*/
extern int set_radio_mcs_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *mcs_value);
																		   /*返回0表示失败，返回1表示成功，返回-1表示input mcs should be 0-31,format should be 1-31 or 1,2,31*/
																		   /*返回-2表示radio id does not exist，返回-3表示radio is disable, please enable it first*/
																		   /*返回-4表示error，返回-5表示Radio ID非法*/
																		   /*返回-9表示mcs cross the border, if your stream is one,mcs should be 0~7,if your stream is two,mcs should be 8~15,and if your stream is three,mcs should be 16~23*/
																		   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_cmmode_cmd_group(struct RadioList *RadioList_Head);

/*返回-10时，调用Free_set_radio_cmmode_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*cwmode_value为"ht20"或"ht20/40"或"ht40"*/
extern int set_radio_cmmode_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *cwmode_value,struct RadioList **RadioList_Head);
										  /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
										  /*返回-2表示radio id does not exist，返回-3表示radio is disable, please enable it first*/
										  /*返回-4表示radio is not binging wlan，返回-5表示error，返回-6表示Radio ID非法*/
										  /*返回-7表示radio mode is not 11N ,don't support this command，返回-8表示Group ID非法*/
										  /*返回-9表示channel offset should be set none，返回-10表示partial failure，返回-11表示group id does not exist*/
										  /*返回-12表示the current radio channel is larger than the max channel,you are not allowed to set channel offset up*/
#endif

/*cwmode_value为"ht20"或"ht20/40"或"ht40"*/
extern int set_radio_cmmode_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *cwmode_value);
																				  /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																				  /*返回-2表示radio id does not exist，返回-3表示radio is disable, please enable it first*/
																				  /*返回-4表示radio is not binging wlan，返回-5表示error，返回-6表示Radio ID非法*/
																				  /*返回-7表示radio mode is not 11N ,don't support this command*/
																				  /*返回-12表示the current radio channel is larger than the max channel,you are not allowed to set channel offset up!Please turn down channel*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_inter_vap_forwarding_cmd_group(struct RadioList *RadioList_Head);

/*返回-9时，调用Free_set_radio_inter_vap_forwarding_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_radio_inter_vap_forwarding_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct RadioList **RadioList_Head);
															/*返回0表示失败，返回1表示成功*/
															/*返回-1表示input patameter only with 'enable' or 'disable'*/
															/*返回-2表示wtp isn't existed，返回-3表示radio isn't existed*/
															/*返回-4表示radio doesn't bind wlan*/
															/*返回-5表示radio not support this command，返回-6表示error*/
															/*返回-7表示Radio ID非法，返回-8表示Group ID非法*/
															/*返回-9表示partial failure，返回-10表示group id does not exist*/
															/*返回SNMPD_CONNECTION_ERROR表示connection error*/
#endif

/*state为"enable"或"disable"*/
extern int set_radio_inter_vap_forwarding_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *state);
																						  /*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示input patameter only with 'enable' or 'disable'*/
																						  /*返回-2表示wtp isn't existed，返回-3表示radio isn't existed*/
																						  /*返回-4表示radio doesn't bind wlan*/
																						  /*返回-5表示radio not support this command，返回-6表示error*/
																						  /*返回-7表示Radio ID非法*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_keep_alive_period_cmd_group(struct RadioList *RadioList_Head);

/*返回-8时，调用Free_set_radio_keep_alive_period_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*value的范围是1-3600*/
extern int set_radio_keep_alive_period_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *value,struct RadioList **RadioList_Head);
																  /*返回0表示失败，返回1表示成功*/
																  /*返回-1表示input parameter should be 1-3600*/
																  /*返回-2表示wtp isn't existed*/
																  /*返回-3表示radio isn't existed*/
																  /*返回-4表示radio doesn't bind wlan*/
																  /*返回-5表示error，返回-6表示Radio ID非法*/
																  /*返回-7表示Group ID非法，返回-8表示partial failure*/
																  /*返回-9表示group id does not exist*/
																  /*返回-10表示this radio not supports those commands*/
#endif

/*value的范围是1-3600*/
extern int set_radio_keep_alive_period_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *value);  
																						/*返回0表示失败，返回1表示成功*/
																						/*返回-1表示input parameter should be 1-3600*/
																						/*返回-2表示wtp isn't existed*/
																						/*返回-3表示radio isn't existed*/
																						/*返回-4表示radio doesn't bind wlan*/
																						/*返回-5表示error，返回-6表示Radio ID非法*/
																						/*返回-10表示this radio not supports those commands*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_keep_alive_idle_time_cmd_group(struct RadioList *RadioList_Head);

/*返回-8时，调用Free_set_radio_keep_alive_idle_time_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*value的范围是1-3600*/
extern int set_radio_keep_alive_idle_time_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *value,struct RadioList **RadioList_Head);
														/*返回0表示失败，返回1表示成功*/
														/*返回-1表示input parameter should be 1-3600*/
														/*返回-2表示wtp isn't existed*/
														/*返回-3表示radio isn't existed*/
														/*返回-4表示radio doesn't bind wlan*/
														/*返回-5表示error，返回-6表示Radio ID非法*/
														/*返回-7表示Group ID非法*/
														/*返回-8表示partial failure，返回-9表示group id does not exist*/
														/*返回-10表示this radio not supports those commands*/
#endif

/*value的范围是1-3600*/
extern int set_radio_keep_alive_idle_time_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *value); 
																						  /*返回0表示失败，返回1表示成功*/
																						  /*返回-1表示input parameter should be 1-3600*/
																						  /*返回-2表示wtp isn't existed*/
																						  /*返回-3表示radio isn't existed*/
																						  /*返回-4表示radio doesn't bing wlan*/
																						  /*返回-5表示error，返回-6表示Radio ID非法*/
																						  /*返回-10表示this radio not supports those commands*/
																						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_congestion_avoidance_cmd_group(struct RadioList *RadioList_Head);

/*返回-8时，调用Free_set_radio_congestion_avoidance_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state的范围是(disable|tail-drop|red|fwred|)*/
extern int set_radio_congestion_avoidance_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct RadioList **RadioList_Head);
																  /*返回0表示失败，返回1表示成功*/
																  /*返回-1表示unknow command*/
																  /*返回-2表示wtp isn't existed*/
																  /*返回-3表示radio isn't existed*/
																  /*返回-4表示radio doesn't bind wlan*/
																  /*返回-5表示error，返回-6表示Radio ID非法*/
																  /*返回-7表示Group ID非法	  ，返回-8表示partial failure*/
																  /*返回-9表示group id does not exist*/
																  /*返回-10表示this radio not supports those commands*/
#endif

/*state的范围是(disable|tail-drop|red|fwred|)*/
extern int set_radio_congestion_avoidance_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *state); 
																							 /*返回0表示失败，返回1表示成功*/
																							 /*返回-1表示unknow command*/
																							 /*返回-2表示wtp isn't existed*/
																							 /*返回-3表示radio isn't existed*/
																							 /*返回-4表示radio doesn't bind wlan*/
																							 /*返回-5表示error，返回-6表示Radio ID非法*/
																							 /*返回-10表示this radio not supports those commands*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_wtp_list_sta_static_arp_enable_cmd(dbus_parameter parameter, DBusConnection *connection,char *wlan_id,char *wtp_list,char *state,char *if_name); 
																							 				 /*返回0表示失败，返回1表示成功*/
																											 /*返回-1表示unknown id format*/
																											 /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																											 /*返回-3表示set wtp list error,like 1,8,9-20,33*/
																											 /*返回-4表示input patameter only with 'enable' or 'disable'*/
																											 /*返回-5表示interface no exist，返回-6表示error*/
																											 /*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_wtp_sta_static_arp_enable_cmd_group(struct RadioList *RadioList_Head);

/*返回-8时，调用Free_set_wtp_sta_static_arp_enable_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"enable"或"disable"*/
extern int set_wtp_sta_static_arp_enable_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,char *state,char *if_name,struct RadioList **RadioList_Head);
														 /*返回0表示失败，返回1表示成功*/
														 /*返回-1表示unknown id format*/
														 /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
														 /*返回-3表示input patameter only with 'enable' or 'disable'*/
														 /*返回-4表示error，返回-5表示Radio ID非法*/
														 /*返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
														 /*返回-7表示Group ID非法，返回-8表示partial failure*/
														 /*返回-9表示group id does not exist*/
#endif

/*state为"enable"或"disable"*/
extern int set_wtp_sta_static_arp_enable_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *wlan_id,char *state,char *if_name);
																													 /*返回0表示失败，返回1表示成功*/
																													 /*返回-1表示unknown id format*/
																													 /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																													 /*返回-3表示input patameter only with 'enable' or 'disable'*/
																													 /*返回-4表示interface no exist*/
																												     /*返回-5表示error，返回-6表示Radio ID非法*/
																													 /*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
																													 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_11n_ampdu_able_cmd_group(struct RadioList *RadioList_Head);

/*返回-12时，调用Free_set_radio_11n_ampdu_able_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*Type为"ampdu"或"amsdu"*/
/*state为"enable"或"disable"*/
extern int set_radio_11n_ampdu_able_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Type,char *state,struct RadioList **RadioList_Head);
																 /*返回0表示失败，返回1表示成功*/
																 /*返回-1表示input patameter only with 'enable' or 'disable'*/
																 /*返回-2表示radio id does not exist*/
																 /*返回-3表示radio is not binding wlan, please bind it first*/
																 /*返回-4表示radio is disable, please enable it first*/
																 /*返回-5表示radio mode is not 11n,don't support this op*/
																 /*返回-6表示error，返回-7表示Radio ID非法*/
																 /*返回-8表示input patameter only with 'ampdu' or 'amsdu'*/
																 /*返回-9表示amsdu switch is enable, please disable it first*/
																 /*返回-10表示ampdu switch is enable, please disable it first*/
																 /*返回-11表示Group ID非法，返回-12表示partial failure*/
																 /*返回-13表示group id does not exist*/
#endif

/*Type为"ampdu"或"amsdu"*/
/*state为"enable"或"disable"*/
extern int set_radio_11n_ampdu_able_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *Type,char *state); 
																								 /*返回0表示失败，返回1表示成功*/
																							     /*返回-1表示input patameter only with 'enable' or 'disable'*/
																							     /*返回-2表示radio id does not exist*/
																							     /*返回-3表示radio is not binding wlan, please bind it first*/
																							     /*返回-4表示radio is disable, please enable it first*/
																								 /*返回-5表示radio mode is not 11n,don't support this op*/
																								 /*返回-6表示error，返回-7表示Radio ID非法*/
																								 /*返回-8表示input patameter only with 'ampdu' or 'amsdu'*/
																								 /*返回-9表示amsdu switch is enable, please disable it first*/
																								 /*返回-10表示ampdu switch is enable, please disable it first*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_11n_ampdu_limit_cmd_group(struct RadioList *RadioList_Head);

/*返回-12时，调用Free_set_radio_11n_ampdu_limit_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*Type为"ampdu"或"amsdu"*/
/*value的范围是ampdu:1024-65535 amsdu:2290-4096*/
extern int set_radio_11n_ampdu_limit_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Type,char *value,struct RadioList **RadioList_Head);
															 /*返回0表示失败，返回1表示成功*/
															 /*返回-1表示input patameter error*/
															 /*返回-2表示input patameter error,ampdu limit should be 1024-65535*/
															 /*返回-3表示radio id does not exist*/
															 /*返回-4表示radio is not binding wlan, please bind it first*/
															 /*返回-5表示radio is disable, please enable it first*/
															 /*返回-6表示radio mode is not 11n,don't support this op*/
															 /*返回-7表示error，返回-8表示Radio ID非法*/
															 /*返回-9表示input patameter only with 'ampdu' or 'amsdu'*/
															 /*返回-10表示input patameter error,amsdu limit should be 2290-4096*/
															 /*返回-11表示Group ID非法，返回-12表示partial failure*/
															 /*返回-13表示group id does not exist*/
#endif

/*Type为"ampdu"或"amsdu"*/
/*value的范围是ampdu:1024-65535 amsdu:2290-4096*/
extern int set_radio_11n_ampdu_limit_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *Type,char *value);  
																								  /*返回0表示失败，返回1表示成功*/
																								  /*返回-1表示input patameter error*/
																								  /*返回-2表示input patameter error,limit should be 1024-65535*/
																								  /*返回-3表示radio id does not exist*/
																								  /*返回-4表示radio is not binding wlan, please bind it first*/
																								  /*返回-5表示radio is disable, please enable it first*/
																								  /*返回-6表示radio mode is not 11n,don't support this op*/
																								  /*返回-7表示error，返回-8表示Radio ID非法*/
																								  /*返回-9表示input patameter only with 'ampdu' or 'amsdu'*/
																								  /*返回-10表示input patameter error,amsdu limit should be 2290-4096*/
																								  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_11n_ampdu_subframe_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_set_radio_11n_ampdu_subframe_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*Type为"ampdu"或"amsdu"*/
/*value的范围是2-64*/
extern int set_radio_11n_ampdu_subframe_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Type,char *value,struct RadioList **RadioList_Head);
														  /*返回0表示失败，返回1表示成功*/
														  /*返回-1表示input patameter only with 'ampdu' or 'amsdu'*/
														  /*返回-2表示input patameter error*/
														  /*返回-3表示input patameter error,limit should be 2-64*/
														  /*返回-4表示Radio ID非法，返回-5表示radio id does not exist*/
														  /*返回-6表示radio is not binding wlan, please bind it first*/
														  /*返回-7表示radio is disable, please enable it first*/
														  /*返回-8表示radio mode is not 11n,don't support this op，返回-9表示error*/
														  /*返回-10表示Group ID非法，返回-11表示partial failure*/
														  /*返回-12表示group id does not exist*/
#endif

/*Type为"ampdu"或"amsdu"*/
/*value的范围是2-64*/
extern int set_radio_11n_ampdu_subframe_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *Type,char *value);
																									   /*返回0表示失败，返回1表示成功*/
																									   /*返回-1表示input patameter only with 'ampdu' or 'amsdu'*/
																									   /*返回-2表示input patameter error*/
																									   /*返回-3表示input patameter error,limit should be 2-64*/
																									   /*返回-4表示Radio ID非法，返回-5表示radio id does not exist*/
																									   /*返回-6表示radio is not binding wlan, please bind it first*/
																									   /*返回-7表示radio is disable, please enable it first*/
																									   /*返回-8表示radio mode is not 11n,don't support this op，返回-9表示error*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_11n_puren_mixed_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_set_radio_11n_puren_mixed_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*workmode为"puren"或"mixed"*/
extern int set_radio_11n_puren_mixed_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *wlan_id,char *workmode,struct RadioList **RadioList_Head);
														   /*返回0表示失败，返回1表示成功*/
														   /*返回-1表示input patameter error*/
														   /*返回-2表示input patameter only with 'puren' or 'mixed'*/
														   /*返回-3表示radio id does not exist*/
														   /*返回-4表示radio is not binding wlan, please bind it first*/
														   /*返回-5表示radio is disable, please enable it first*/
														   /*返回-6表示radio mode is not 11n,don't support this op*/
														   /*返回-7表示error，返回-8表示Radio ID非法*/
														   /*返回-9表示illegal input:Input exceeds the maximum value of the parameter type*/
														   /*返回-10表示Group ID非法，返回-11表示partial failure*/
														   /*返回-12表示group id does not exist*/
#endif

/*workmode为"puren"或"mixed"*/
extern int set_radio_11n_puren_mixed_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *wlan_id,char *workmode);
																									  /*返回0表示失败，返回1表示成功*/
																									  /*返回-1表示input patameter error*/
																									  /*返回-2表示input patameter only with 'puren' or 'mixed'*/
																									  /*返回-3表示radio id does not exist*/
																									  /*返回-4表示radio is not binding wlan, please bind it first*/
																									  /*返回-5表示radio is disable, please enable it first*/
																									  /*返回-6表示radio mode is not 11n,don't support this op*/
																									  /*返回-7表示error，返回-8表示Radio ID非法*/
																									  /*返回-9表示illegal input:Input exceeds the maximum value of the parameter type*/
																									  /*返回-10表示now radio mode is an or gn, belong to puren,you can set it to mixed*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
#if 0
/*state为"enable"或"disable"*/
extern int set_tx_chainmask_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *list,char *state);/*返回0表示失败，返回1表示成功*/
																					 /*返回-1表示input patameter only with 'enable' or 'disable'*/
																					 /*返回-2表示input parameter only should be 0,1,2, one or more of them,such as 1,2 or 0,2*/
																					 /*返回-3表示radio id does not exist，返回-4表示radio not support this command*/
																					 /*返回-5表示error，返回-6表示Radio ID非法*/
#endif

#if _GROUP_POLICY
extern void Free_set_radio_11n_channel_offset_cmd_group(struct RadioList *RadioList_Head);

/*返回-12时，调用Free_set_radio_11n_channel_offset_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*state为"up"或"down"*/
extern int set_radio_11n_channel_offset_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *state,struct RadioList **RadioList_Head);
				/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'down'*/
				/*返回-2表示radio id does not exist，返回-3表示radio is not binding wlan, please bind it first*/			
				/*返回-4表示radio is disable, please enable it first，返回-5表示radio mode is not 11n,don't support this op*/
				/*返回-6表示error，返回-7表示Radio ID非法，返回-8表示radio channel bandwidth is not 40,don't support this op*/
			    /*返回-9表示the current radio channel is larger than the max channel,you are not allowed to set channel offset up*/
				/*返回-10表示the current radio channel is less than the min channel ,you are not allowed to set channel offset down*/
				/*返回-11表示Group ID非法，返回-12表示partial failure，返回-13表示group id does not exist*/
				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
#endif

/*state为"up"或"down"*/
extern int set_radio_11n_channel_offset_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *state);
																						/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with 'enable' or 'down'*/
																						/*返回-2表示radio id does not exist，返回-3表示radio is not binding wlan, please bind it first*/			
																						/*返回-4表示radio is disable, please enable it first，返回-5表示radio mode is not 11n,don't support this op*/
																						/*返回-6表示error，返回-7表示Radio ID非法，返回-8表示radio channel bandwidth is not 40,don't support this op*/																						
																						/*返回-9表示the current radio channel is larger than the max channel,you are not allowed to set channel offset up!Please turn down channel*/
																						/*返回-10表示the current radio channel is less than the min channel ,you are not allowed to set channel offset down!Please turn up channel*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_tx_chainmask_v2_cmd_group(struct RadioList *RadioList_Head);

/*返回-11时，调用Free_set_tx_chainmask_v2_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
/*Type为"tx_chainmask"或"rx_chainmask"*/
/*value为"1.0.0","0.1.0","1.1.0","0.0.1","1.0.1","0.1.1"或"1.1.1"*/
extern int set_tx_chainmask_v2_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *Type,char *value,struct RadioList **RadioList_Head);
		/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with '0.0.1','0.1.0','0.1.1','1.0.0','1.0.1','1.1.0' or '1.1.1'*/
		/*返回-2表示radio id does not exist，返回-3表示radio not support this command*/ 		
		/*返回-4表示radio mode is not 11N ,don't support this command，返回-5表示error，返回-6表示Radio ID非法*/
		/*返回-7表示input patameter only with 'tx_chainmask' or 'rx_chainmask'*/
		/*返回-8表示radio chainmask number is 1, don't support this value*/
		/*返回-9表示radio chainmask number is 2, don't support this value*/
		/*返回-10表示Group ID非法，返回-11表示partial failure，返回-12表示group id does not exist*/
		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
#endif

/*Type为"tx_chainmask"或"rx_chainmask"*/
/*value为"1.0.0","0.1.0","1.1.0","0.0.1","1.0.1","0.1.1"或"1.1.1"*/
extern int set_tx_chainmask_v2_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *Type,char *value); 
																						/*返回0表示失败，返回1表示成功，返回-1表示input patameter only with '0.0.1','0.1.0','0.1.1','1.0.0','1.0.1','1.1.0' or '1.1.1'*/
																						/*返回-2表示radio id does not exist，返回-3表示radio not support this command*/ 		  
																						/*返回-4表示radio mode is not 11N ,don't support this command，返回-5表示error，返回-6表示Radio ID非法*/
																						/*返回-7表示input patameter only with 'tx_chainmask' or 'rx_chainmask'*/
																						/*返回-8表示radio chainmask number is 1, don't support this value*/
																					    /*返回-9表示radio chainmask number is 2, don't support this value*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

#if _GROUP_POLICY
extern void Free_set_radio_txpowerstep_cmd_group(struct RadioList *RadioList_Head);

/*返回-7时，调用Free_set_radio_txpowerstep_cmd_group()释放空间*/
/*group_type为1，表示组配置*/
/*group_type为0，表示单独配置*/
extern int set_radio_txpowerstep_cmd_group(dbus_parameter parameter, DBusConnection *connection,int group_type,int group_id,char *txp_step,struct RadioList **RadioList_Head);
										/*返回0表示失败，返回1表示成功*/
										/*返回-1表示Input exceeds the maximum value of the parameter type*/
										/*返回-2表示unknown id format，返回-3表示RADIO ID非法*/ 		
										/*返回-4表示radio id does not exist，返回-5表示this radio is not binding wlan,binding wlan first*/
										/*返回-6表示Group ID非法，返回-7表示partial failure，返回-8表示group id does not exist*/
										/*返回-9表示txpowerstep should > 0*/
										/*返回SNMPD_CONNECTION_ERROR表示connection error*/
#endif

extern int set_radio_txpowerstep_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *txp_step); 
																					 /*返回0表示失败，返回1表示成功*/
																					 /*返回-1表示Input exceeds the maximum value of the parameter type*/
																					 /*返回-2表示unknown id format，返回-3表示RADIO ID非法*/		  
																					 /*返回-4表示radio id does not exist，返回-5表示this radio is not binding wlan,binding wlan first*/
																					 /*返回-9表示txpowerstep should > 0*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_show_all_wlan_ssid_stats_information_of_all_radio_cmd(struct SSIDStatsInfo *WtpHead);
/*只要调用，就通过Free_show_all_wlan_ssid_stats_information_of_all_radio_cmd()释放空间*/
extern int show_all_wlan_ssid_stats_information_of_all_radio_cmd(dbus_parameter parameter, DBusConnection *connection,struct SSIDStatsInfo **WtpHead); 
																									   /*返回0表示失败，返回1表示成功*/
																									   /*返回-1表示There is no WTP now*/
																									   /*返回-2表示error*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*Rssi的范围是0-95*/
extern int set_radio_wlan_limit_rssi_access_sta_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *wlan_id,char *Rssi);
																		   /*返回0表示失败，返回1表示成功*/
																		   /*返回-1表示unknown id format*/
																		   /*返回-2表示wlan id should be 1 to WLAN_NUM-1*/
																		   /*返回-3表示RSSI should be 0 to 95*/
																		   /*返回-4表示RADIO ID非法*/		   
																		   /*返回-5表示radio id does not exist*/
																		   /*返回-6表示wtp id does not exist*/
																		   /*返回-7表示wlan id does not exist*/
																		   /*返回-8表示bss id does not exist*/
																		   /*返回-9表示wlan is not bind by this radio*/

/*Switch的取值是"enable"或"disable"*/
extern int set_bss_multi_user_optimize_cmd(dbus_parameter parameter, DBusConnection *connection,int RID,char *wlanID,char *Switch);
																				   /*返回0表示失败，返回1表示成功*/
																				   /*返回-1表示input parameter error*/
																				   /*返回-2表示wlanid should be 1 to WLAN_NUM-1*/
																				   /*返回-3表示RADIO ID非法，返回-4表示bss not exist*/		   
																				   /*返回-5表示operation fail，返回-6表示wlan is not binded radio*/
																				   /*返回-7表示error*/

#endif

