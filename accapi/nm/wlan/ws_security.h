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
* ws_security.h
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

#ifndef _WS_SECURITY_H
#define _WS_SECURITY_H

#include <unistd.h>
#include <syslog.h>
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"

#define SLOT_LLEGAL(slot_no)     ((slot_no)>0&&(slot_no)<=4)//xm 08/08/27
#define PORT_LLEGAL(port_no)     ((port_no)>0&&(port_no)<=24)//xm 08/08/27


#define SLOT_PORT_SPLIT_SLASH	'/'	//xm 08/08/27
#define SLOT_PORT_SPLIT_DASH 	'-'	//xm 08/08/27
#define SLOT_PORT_SPLIT_COMMA 	','	//xm 08/08/27

#define MAX_NUM_OF_VLANID 256		//xm 08/08/27

#define MAXINTERFACES 48			//ht 08.12.02
#define MAXNAMSIZ 16				//ht
#define MAX_VLAN_NUM 4094			//ht add ->sz


typedef struct  {
       unsigned char slot;
       unsigned char port;
}SLOT_PORT_S;					//xm 08/08/27

typedef enum {
		TEST_SLOT_STATE=1,
		TEST_SPLIT_STATE,
		TEST_PORT_STATE,
		TEST_COMMA_STATE,
		TEST_END_STATE,
		TEST_FAILURE_STATE,
		TEST_SUCESS_STATE
}PARSE_PORT_STATE;		//xm 08/08/27  

/////////////////////////////////////////////////
//sz20080825 

typedef struct  {
       unsigned int vlanid;
       unsigned int stat;
	   
}VLAN_ENABLE;

typedef enum{
	check_vlanid_state=0,
	check_comma_state,
	check_fail_state,
	check_end_state,
	check_success_state
}vlan_list_state;


struct asd_trap_state_info
{
	unsigned char type[8];
};

struct asd_global_variable_info
{
	int  asd_notice_sta_info_to_portal;
	int  asd_notice_sta_info_to_portal_timer;
	int  wtp_send_response_to_mobile;
	int  asd_dbus_count_switch;
	unsigned int  sta_static_fdb_able;
	unsigned int  asd_switch;
	unsigned char  asd_station_arp_listen;
	unsigned char  asd_station_static_arp;
	unsigned int  asd_sta_idle_time;
	unsigned char asd_sta_idle_time_switch;
	unsigned int asd_bak_sta_update_time;
	unsigned char  asd_ipset_switch;
	unsigned char asd_getip_from_dhcpsnp;
};


extern int parse_security_char_ID(char* str,unsigned char* ID);
extern int RemoveListRepId(int list[],int num);
extern int parse_vlan_list(char* ptr,int* count,int vlanId[]);
extern int parse_port(char* ptr,/*int* slot,int* port*/SLOT_PORT_VLAN_SECURITY* sp);

extern int _parse_port(char* ptr,/*int* slot,int* port*/SLOT_PORT_VLAN_ENABLE* sp);



extern int _parse_port_list(char* ptr,int* count,SLOT_PORT_S spL[]);  //xm 08/08/27


extern  void CheckSecurityType(char *type, unsigned int SecurityType);

extern 	void CheckEncryptionType(char *type, unsigned int EncryptionType);
extern  void CheckRekeyMethod(char *type, unsigned char SecurityType);

extern  void Free_security_head(struct dcli_security *sec);
/*返回1时，调用Free_security_head()释放空间*/
extern 	int show_security_list(dbus_parameter parameter, DBusConnection *connection,struct dcli_security **sec,int *security_num);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_security_one(struct dcli_security *sec);
/*返回1时，调用Free_security_one()释放空间*/
extern 	int show_security_one(dbus_parameter parameter, DBusConnection *connection,int id,struct dcli_security **sec);
																					 /*返回0表示失败，,返回1表示成功，返回-1表示security ID非法*/
																					 /*返回-2表示Security id is not exited，返回-3表示error*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					 
extern  int create_security(dbus_parameter parameter, DBusConnection *connection,char * id, char *sec_name);	/*返回0表示失败，返回1表示成功，返回-1表示security id非法，返回-2表示security ID existed，返回-3表示error*/
																					 							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					 
/*sec_type的范围是"open","shared","802.1x","WPA_P","WPA2_P","WPA_E","WPA2_E","MD5","WAPI_PSK"或"WAPI_AUTH"*/
extern	int security_type(dbus_parameter parameter, DBusConnection *connection,int id,char *sec_type);  
																	 /*返回0表示失败，返回1表示成功，返回-1表示unknown security type，返回-2表示security id not exist*/
				                                                     /*返回-3表示This Security Profile be used by some Wlans,please disable these Wlans first，返回-4表示error，返回-5表示Security ID非法*/
																	 /*返回-6表示The radius heart test is on,turn it off first!*/
																	 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																	 
extern	int encryption_type(dbus_parameter parameter, DBusConnection *connection,int id,char *enc_type);  
																	 	/*返回0表示失败，返回1表示成功，返回-1表示unknown encryption type*/
				                                                        /*返回-2表示encryption type dosen't match with security type，返回-3表示security id not exist*/
				                                                        /*返回-4表示This Security Profile be used by some Wlans,please disable these Wlans first*/
				                                                        /*返回-5表示error，返回-6表示Security ID非法，返回-7表示The radius heart test is on,turn it off first!*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*service_type==1表示enable，service_type==0表示disable*/																		
extern  int extensible_authentication(dbus_parameter parameter, DBusConnection *connection,int id,int service_type);	 
																					 /*返回0表示失败，返回1表示成功*/
																					 /*返回-1表示encryption type dosen't match with security type，返回-2表示security id not exist*/
																					 /*返回-3表示This Security Profile be used by some Wlans,please disable these Wlans first*/
																					 /*返回-4表示error，返回-5表示Security ID非法*/
																					 /*返回-6表示extensible auth is supported open or shared*/
																					 /*返回-7表示The radius heart test is on,turn it off first!*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*service_type==1表示wired，service_type==0表示wireless*/																					 
extern  int radius_server(dbus_parameter parameter, DBusConnection *connection,int id,int service_type); 
																	  /*返回0表示失败，返回1表示成功*/
																 	  /*返回-1表示encryption type dosen't match with security type，返回-2表示security id not exist*/
																	  /*返回-3表示This Security Profile be used by some Wlans,please disable these Wlans first*/
																	  /*返回-4表示error，返回-5表示Security ID非法*/
																	  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*inputype的范围是"ascii"或"hex"*/																		
extern 	int security_key(dbus_parameter parameter, DBusConnection *connection,int id,char *SecurityKey,char*inputype); 	
																	  				/*返回0表示失败，返回1表示成功，返回-1表示security not exist*/
							                                                        /*返回-2表示security key not permit set，返回-3表示key length error，返回-4表示Key has been set up*/
							                                                        /*返回-5表示This Security Profile be used by some Wlans,please disable these Wlans first，返回-6表示error*/
																					/*返回-7表示hex key length error，返回-8表示key format is incorrect(key should be '0' to '9' or 'a' to 'f')*/
																				    /*返回-9表示Security ID非法*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*aflag==0表示"acct"，aflag==1表示"auth"*/																				
extern	int security_auth_acct(dbus_parameter parameter, DBusConnection *connection,int aflag,int id,char *secu_ip,int secu_port,char *secret);
																											 /*返回0表示失败，返回1表示成功，返回-1表示unknown port*/
				                                                                                             /*返回-2表示security type which you choose not supported 802.1X，返回-3表示Change radius info not permited*/
				                                                                                             /*返回-4表示This Security Profile be used by some Wlans,please disable these Wlans first，返回-5表示error*/
				                                                                                             /*返回-6表示Security ID非法，返回-7表示The radius heart test is on,turn it off first!*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											 
extern  int delete_security(dbus_parameter parameter, DBusConnection *connection,int id);  /*返回0表示 删除失败，返回1表示删除成功*/
																							/*返回-1表示security ID非法，返回-2表示security ID not existed*/
																							/*返回-3表示This Security Profile be used by some Wlans,please disable these Wlans first*/
																							/*返回-4表示出错，返回-5表示The radius heart test is on,turn it off first!*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																											 
extern  int config_port_cmd_func(char *post_list,char *secu_id);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																	/*返回-2表示security id should be 1 to WLAN_NUM-1，返回-3表示unknown port format*/
																	/*返回-4表示illegal input:Input exceeds the maximum value of the parameter type*/
																	
extern 	int config_port_enable_cmd_func(char *post_list,char *state);/*返回0表示失败，返回1表示成功，返回-1表示unknow port format，返回-2表示error*/
extern  int set_acct_interim_interval(dbus_parameter parameter, DBusConnection *connection,int sid,int time);  
																			/*返回0表示 失败，返回1表示成功，返回-1表示input time value should be 0 to 32767*/
																	        /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																	        /*返回-4表示Can't set acct interim interval under current security type，返回-5表示Security ID非法*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			
extern int	secondary_radius_acct(dbus_parameter parameter, DBusConnection *connection,int id,char *secuip,int secu_port,char *secret);  
																										 /*返回0表示失败，返回1表示成功，返回-1表示security type which you choose not supported 802.1X*/
																										 /*返回-2表示Change radius info not permited，返回-3表示This Security Profile be used by some Wlans,please disable these Wlans first*/
																										 /*返回-4表示Please use radius acct ip port shared_secret first,command failed，返回-5表示error，返回-6表示Security ID非法*/
																										 /*返回-7表示The radius heart test is on,turn it off first!，返回-8表示unknown port id*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																										 
extern int secondary_radius_auth(dbus_parameter parameter ,DBusConnection *connection,int id,char * secuip,int secport,char *secr);
																								   /*返回0表示失败，返回1表示成功，返回-1表示security type which you choose not supported 802.1X*/
																								   /*返回-2表示Change radius info not permited，返回-3表示This Security Profile be used by some Wlans,please disable these Wlans first*/
																								   /*返回-4表示Please use radius auth ip port shared_secret first,command failed，返回-5表示error，返回-6表示Security ID非法*/
																								   /*返回-7表示The radius heart test is on,turn it off first!，返回-8表示unknown port id*/
																								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								   
extern int config_vlan_list_enable_cmd_func(char *vlanlist,char *state);/*返回0表示失败，返回1表示成功，返回-1表示input parameter is illegal，返回-2表示error*/
																								   
extern int config_vlan_list_security_cmd_func(char *vlanlist,char *secu_id);   /*返回0表示失败，返回1表示成功，返回-1表示input parameter illegal，返回-2表示unknown id format*/
																					/*返回-3表示security id should be 1 to WLAN_NUM-1，返回-4表示error*/
																					/*返回-5表示illegal input:Input exceeds the maximum value of the parameter type*/
																					
extern int config_port_vlan_security_cmd_func(char *port_id,char *vlan_id,char *secu_id);/*返回0表示失败，返回1表示成功，返回-1表示input parameter illegal*/
																								/*返回-2表示unknown id format，返回-3表示vlan id should be 1 to 4094*/
																								/*返回-4表示security id should be 1 to WLAN_NUM-1，返回-5表示error*/
																								/*返回-6表示illegal input:Input exceeds the maximum value of the parameter type*/
																								
extern int config_port_vlan_enable_cmd_func(char *port_id,char *vlan_id,char *state);/*返回0表示失败，返回1表示成功，返回-1表示input parameter illegal*/
																							/*返回-2表示unknown id format，返回-3表示vlan id should be 1 to 4094*/

/*time的范围是0-32767*/
extern int set_eap_reauth_period_cmd(dbus_parameter parameter, DBusConnection *connection,int id,int time);
																			/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示input period value should be 0 to 32767*/
																			/*返回-2表示security profile does not exist*/
																			/*返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																			/*返回-4表示Can't set eap reauth period under current security type*/
																			/*返回-5表示error，返回-6表示Security ID非法*/
																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int security_host_ip(dbus_parameter parameter, DBusConnection *connection,int id,char * ip_addr);   
																		/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
					                                                    /*返回-2表示check_local_ip error，返回-3表示not local ip，返回-4表示security profile does not exist*/
				                                                        /*返回-5表示this security profile is used by some wlans,please disable them first，返回-6表示error，返回-7表示Security ID非法*/
																		/*返回-8表示The radius heart test is on,turn it off first!*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*cer_type的范围是"X.509"或"GBW"*/																		
extern int config_wapi_auth(dbus_parameter parameter, DBusConnection *connection,int sid,char *ip_addr,char *cer_type);	
																						/*返回0表示失败，返回1表示成功，返回-1表示unknown certification type*/
																						/*返回-2表示unknown ip format*/
																						/*返回-3表示security type which you chose does not support wapi authentication*/
																						/*返回-4表示this security profile be used by some wlans,please disable them first*/
																						/*返回-5表示error，返回-6表示Security ID非法*/
																						/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*cer_type的范围是"as","ae"或"ca"*/																						
extern int config_wapi_auth_path(dbus_parameter parameter, DBusConnection *connection,int sid,char *cer_type,char *path);  
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示certification isn't exit or can't be read*/
																							/*返回-2表示security type which you chose does not support wapi authentication*/
																							/*返回-3表示this security profile be used by some wlans,please disable them first*/
																							/*返回-4表示this security profile isn't integrity，返回-5表示error，返回-6表示Security ID非法*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state的范围是"enable"或"disable"*/																							
extern int config_pre_auth_cmd_func(dbus_parameter parameter, DBusConnection *connection,int sec_id,char *state); 
																					/*返回0表示失败，返回1表示 成功，返回-1表示unknown encryption type*/
																					/*返回-2表示encryption type does not match security type，返回-3表示security profile does not exist*/
																					/*返回-4表示this security profile is used by some wlans,please disable them first，返回-5表示error*/
																					/*返回-6表示Security ID非法*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					
/*Uorm为"unicast"或"multicast"，Torp为"time"或"packet"，para为<0-400000000>*/																										
extern int set_wapi_rekey_para_cmd_func(dbus_parameter parameter, DBusConnection *connection,int id,char *Uorm,char *Torp,char *para);/*返回0表示失败，返回1表示成功，返回-1表示unknown command format*/
																										/*返回-2表示input value should be 0 to 400000000，返回-3表示security profile does not exist*/
																										/*返回-4表示This Security Profile be used by some Wlans,please disable them first*/
																										/*返回-5表示Can't set wapi rekey parameter under current config，返回-6表示error，返回-7表示Security ID非法*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*Uorm为"unicast"或"multicast"，Method为"disable"、"time_based"、"packet_based"或"both_based"*/
extern int set_wapi_ucast_rekey_method_cmd_func(dbus_parameter parameter, DBusConnection *connection,int id,char *Uorm,char *Method);
																										  /*返回0表示失败，返回1表示成功，返回-1表示unknown command format*/
																										  /*返回-2表示security profile does not exist*/
																										  /*返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																										  /*返回-4表示Can't set wapi unicast rekey method under current security type*/
																										  /*返回-5表示error，返回-6表示Security ID非法*/
																										  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*period为<0-65535>*/
extern int set_security_quiet_period_cmd_func(dbus_parameter parameter, DBusConnection *connection,int sec_id,char *period);
																							 /*返回0表示失败，返回1表示成功，返回-1表示input time value should be 0 to 65535*/
																						     /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																						     /*返回-4表示Can't set 1x quiet period under current security type，返回-5表示error，返回-6表示Security ID非法*/
																							 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																							 
/*state为"enable"或"disable"*/
extern int config_accounting_on_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																			   /*返回0表示失败，返回1表示成功，返回-1表示unknown encryption type*/
																			   /*返回-2表示security type is needn't 802.1X，返回-3表示security profile does not exist*/
																			   /*返回-4表示this security profile is used by some wlans,please disable them first*/
																			   /*返回-5表示errorthis security profile is used by some wlans,please disable them first*/
																			   /*返回-6表示Security ID非法*/
																			   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			  
/*state为"enable"或"disable"*/
extern int set_mobile_open_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示input should be enable or disable，返回-2表示error*/
																			   						  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																			   
/*state为"enable"或"disable"*/
extern int config_radius_extend_attr_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																				  /*返回0表示失败，返回1表示成功，返回-1表示unknown encryption type*/
																				  /*返回-2表示security type is needn't 802.1X，返回-3表示security profile does not exist*/
																				  /*返回-4表示this security profile is used by some wlans,please disable them first*/
																				  /*返回-5表示error，返回-6表示Security ID非法*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_wapi_sub_attr_wapipreauth_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																				  		 /*返回0表示失败，返回1表示成功，返回-1表示WapiPreauth should be enable or disable*/
																					     /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																					     /*返回-4表示Can't set WapiPreauth under current security type，返回-5表示error，返回-6表示Security ID非法*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																						
/*state为"enable"或"disable"*/
extern int set_wapi_sub_attr_multicaserekeystrict_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																						 		 /*返回0表示失败，返回1表示成功，返回-1表示MulticaseRekeyStrict should be enable or disable*/
																								 /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																								 /*返回-4表示Can't set MulticaseRekeyStrict under current security type，返回-5表示error，返回-6表示Security ID非法*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								 
/*state为"enable"或"disable"*/
extern int set_wapi_sub_attr_unicastcipherenabled_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																								   /*返回0表示失败，返回1表示成功，返回-1表示UnicastCipherEnabled should be enable or disable*/
																								   /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																								   /*返回-4表示Can't set UnicastCipherEnabled under current security type，返回-5表示error，返回-6表示Security ID非法*/
																								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								   
/*state为"enable"或"disable"*/
extern int set_wapi_sub_attr_authenticationsuiteenable_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																								   	   /*返回0表示失败，返回1表示成功，返回-1表示AuthenticationSuiteEnable should be enable or disable*/
																									   /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																									   /*返回-4表示Can't set AuthenticationSuiteEnable under current security type，返回-5表示error，返回-6表示Security ID非法*/
																									   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																									  
/*value的范围是0-64*/
extern int set_wapi_sub_attr_certificateupdatecount_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *value);
																								   /*返回0表示失败，返回1表示成功，返回-1表示input retry value should be 0 to 64*/
																								   /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																								   /*返回-4表示Can't set CertificateUpdateCount under current security type，返回-5表示error，返回-6表示Security ID非法*/
																								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								   
/*value的范围是0-64*/
extern int set_wapi_sub_attr_multicastupdatecount_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *value);
																								   /*返回0表示失败，返回1表示成功，返回-1表示input retry value should be 0 to 64*/
																								   /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																								   /*返回-4表示Can't set MulticastUpdateCount under current security type，返回-5表示error，返回-6表示Security ID非法*/
																								   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								   
/*value的范围是0-64*/
extern int set_wapi_sub_attr_unicastupdatecount_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *value);
																								 /*返回0表示失败，返回1表示成功，返回-1表示input	value should be 0 to 64*/
																								 /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																								 /*返回-4表示Can't set UnicastUpdateCount under current security type，返回-5表示error，返回-6表示Security ID非法*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								 
/*value的范围是0-86400*/
extern int set_wapi_sub_attr_bklifetime_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *value);
																					  /*返回0表示失败，返回1表示成功，返回-1表示input value should be 0 to 86400*/
																					  /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																					  /*返回-4表示Can't set BKLifetime under current security type，返回-5表示error，返回-6表示Security ID非法*/
																					  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					  
/*value的范围是0-99*/
extern int set_wapi_sub_attr_bkreauththreshold_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *value);
																					  		   /*返回0表示失败，返回1表示成功，返回-1表示input  value should be 0 to 99*/
																							   /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																							   /*返回-4表示Can't set BKReauthThreshold under current security type，返回-5表示error，返回-6表示Security ID非法*/
																							   /*返回SNMPD_CONNECTION_ERROR表示connection error*/
																							   
/*value的范围是0-120*/
extern int set_wapi_sub_attr_satimeout_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *value);
																					 /*返回0表示失败，返回1表示成功，返回-1表示input retry value should be 0 to 120*/
																					 /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																					 /*返回-4表示Can't set SATimeout under current security type，返回-5表示error，返回-6表示Security ID非法*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_security_wapi_info(struct dcli_security *sec);
/*只要调用，就通过Free_security_wapi_info()释放空间*/
extern int  show_security_wapi_info_cmd(dbus_parameter parameter, DBusConnection *connection,char *id,struct dcli_security **sec);
											 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
											 /*返回-2表示security id should be 1 to WLAN_NUM-1，返回-3表示security id does not exist*/
											 /*返回-4表示security id should be wapi_psk or wapi_auth*/
											 /*返回-5表示security's wapi config  is not intergrity，返回-6表示error*/
											 /*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/
											 
/*state为"enable"或"disable"*/
extern int  config_asd_get_sta_info_able_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示parameter illegal，返回-2表示error*/
											 																		  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
											 
/*value的范围是5-3600*/
extern int config_asd_get_sta_info_time_cmd(dbus_parameter parameter, DBusConnection *connection,char *value);/*返回0表示失败，返回1表示成功，返回-1表示input time value should be 5 to 3600，返回-2表示error*/
																													/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int set_notice_sta_info_to_portal_open_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);/*返回0表示失败，返回1表示成功，返回-1表示input should be enable or disable，返回-2表示error*/
																														   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int config_wapi_multi_cert_cmd(dbus_parameter parameter, DBusConnection *connection,int SID,char *state);
																				/*返回0表示失败，返回1表示成功，返回-1表示parameter illegal*/
																				/*返回-2表示security type which you chose does not support wapi authentication*/
																				/*返回-3表示this security profile be used by some wlans,please disable them first*/
																				/*返回-4表示error，返回-5表示Security ID非法*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																				
extern void Free_wlanid_security_wapi_info(struct dcli_security *sec);
/*只要调用，就通过Free_wlanid_security_wapi_info()释放空间*/
extern int show_wlanid_security_wapi_config_cmd(dbus_parameter parameter, DBusConnection *connection,char *wlanID,struct dcli_security	**sec);
																												   /*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																												   /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示wlan id does not exist*/
																												   /*返回-4表示security id should be wapi_psk or wapi_auth*/
																												   /*返回-5表示security's wapi config  is not intergrity，返回-6 表示wlan has not apply any security*/
																											       /*返回-7表示error，返回-8表示illegal input:Input exceeds the maximum value of the parameter type*/
																												   /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_radius_cmd(struct dcli_security *sec);
/*返回1时，调用Free_radius_cmd()释放空间*/																																 
extern int show_radius_cmd(dbus_parameter parameter, DBusConnection *connection,char *Radius_ID,struct dcli_security **sec);
																							/*返回0表示失败，返回1表示成功*/
																							/*返回-1表示unknown id format*/
																							/*返回-2表示radius id should be 1 to WLAN_NUM-1*/
																							/*返回-3表示radius does not exist，返回-4表示error*/
																							/*返回-5表示illegal input:Input exceeds the maximum value of the parameter type*/
																							/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*value的范围是1-255*/
extern int set_wapi_sub_attr_multicast_cipher_cmd(dbus_parameter parameter, DBusConnection *connection,int sid,char *value);
																							  /*返回0表示失败，返回1表示成功，返回-1表示input retry value should be 1 to 255*/
																							  /*返回-2表示security profile does not exist，返回-3表示This Security Profile be used by some Wlans,please disable them first*/
																							  /*返回-4表示Can't set multicast sipher under current security type，返回-5表示error，返回-6表示Security ID非法*/
																							  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int config_asd_ieee_80211n_able_cmd(dbus_parameter parameter, DBusConnection *connection,char *state);
																				/*返回0表示失败，返回1表示成功*/
																			    /*返回-1表示parameter illegal，返回-2表示error*/
																				/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*sec_index的范围是1-4*/
extern int set_security_wep_index_cmd(dbus_parameter parameter, DBusConnection *connection,int sid,char *sec_index);
																					 /*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示input security index should be 1 to 4*/
																					 /*返回-3表示security profile does not exist，返回-4表示This Security Profile be used by some Wlans,please disable them first*/
																					 /*返回-5表示the encryption type of the security should be wep，返回-6表示error，返回-7表示Security ID非法*/
																					 /*返回-8表示illegal input:Input exceeds the maximum value of the parameter type*/
																					 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*trap_type为"wtp_deny_sta","sta_verify_failed","sta_assoc_failed","wapi_invalid_cert","wapi_challenge_replay","wapi_mic_juggle","wapi_low_safe","wapi_addr_redirection"*/
/*state为"enable"或"disable"*/
extern int config_set_asd_trap_able_cmd(dbus_parameter parameter, DBusConnection *connection,char *trap_type,char *state);/*返回0表示失败，返回1表示成功，返回-1表示parameter illegal，返回-2表示error*/
																					 											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					 
extern int show_asd_trap_state_cmd(dbus_parameter parameter, DBusConnection *connection,struct asd_trap_state_info *info);/*返回0表示失败，返回1表示成功，返回-1表示error*/
																																/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																				
/*cer_type为"as","ae"或"ca"*/
extern int config_wapi_p12_cert_auth_path_cmd(dbus_parameter parameter, DBusConnection *connection,int sid,char *cer_type,char *path,char *passwd);
																													  /*返回0表示失败，返回1表示成功*/
																													  /*返回-1表示certification isn't exit or can't be read*/
																													  /*返回-2表示security type which you chose does not support wapi authentication*/
																													  /*返回-3表示this security profile be used by some wlans,please disable them first*/
																													  /*返回-4表示this security profile isn't integrity*/
																													  /*返回-5表示p12 cert password error，返回-6表示error*/
																													  /*返回-7表示Security ID非法*/
extern void Free_show_radius_all_cmd(struct dcli_security *sec);
/*返回1时，调用Free_show_radius_all_cmd()释放空间*/
extern int show_radius_all_cmd(dbus_parameter parameter, DBusConnection *connection,struct dcli_security **sec); 
																				  /*返回0表示失败，返回1表示成功*/
																				  /*返回-1表示no security support radius*/
																				  /*返回-2表示error*/
																				  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state为"enable"或"disable"*/
extern int config_hybrid_auth_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *state);
																			   /*返回0表示失败，返回1表示成功，返回-1表示unknown security type，返回-2表示Security ID非法*/
																			   /*返回-3表示security profile does not exist，返回-4表示this security profile is used by some wlans,please disable them first*/
																			   /*返回-5表示error，返回SNMPD_CONNECTION_ERROR表示connection error*/

/*value must multiple of 15,range is 30-86400*/
extern int set_ap_max_detect_interval_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *value);
																				/*返回0表示失败，返回1表示成功，返回-1表示unknown format,please input number*/
																				/*返回-2表示the number is not be multiple of 15，返回-3表示Security ID非法*/
																				/*返回-4表示security profile does not exist，返回-5表示this security profile is used by some wlans,please disable them first*/
																				/*返回-6表示error*/

extern int show_asd_global_variable_cmd(dbus_parameter parameter, DBusConnection *connection, struct asd_global_variable_info *info);
																		/*返回0表示失败，返回1表示成功*/
																		/*返回-1表示error*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*value的范围是0-86400*/
extern int set_wpa_group_rekey_period_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *value);
																				 /*返回0表示失败，返回1表示成功*/
																				 /*返回-1表示input period value should be 0 to 86400*/
																				 /*返回-2表示Security ID非法*/
																				 /*返回-3表示security profile does not exist*/
																				 /*返回-4表示This Security Profile be used by some Wlans,please disable them first*/
																				 /*返回-5表示Can't set wpa group rekey period under current security type*/
																				 /*返回-6表示error*/
																				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_asd_rdc_para_cmd(dbus_parameter parameter, DBusConnection *connection,int id,char *slotID,char *insID);
																			  /*返回0表示失败，返回1表示成功*/
																			  /*返回-1表示slotid should be 0 to 16*/
																			  /*返回-2表示Security ID非法*/
																			  /*返回-3表示security type should be 802.1X, wpa_e or wpa2_e*/
																			  /*返回-4表示security profile does not exist*/
																			  /*返回-5表示this security profile is used by some wlans,please disable them first*/
																			  /*返回-6表示The radius heart test is on,turn it off first*/
																			  /*返回-7表示error*/
																			  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

#endif

