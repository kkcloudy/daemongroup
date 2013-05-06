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
* ws_dcli_wqos.h
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



#ifndef _WS_DCLI_WQOS_H
#define _WS_DCLI_WQOS_H
#include <syslog.h>
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_dcli_wlans.h"

typedef enum{
	qos_check_dot1p_state=0,
	qos_check_comma_state,
	qos_check_fail_state,
	qos_check_end_state,
	qos_check_success_state
}dot1p_list_state;


#define QOS_CWMIN_NUM	16
#define QOS_CWMAX_NUM	16
#define QOS_AIFS_NUM	16
#define QOS_TXOPLIMIT_NUM	8192
#define QOS_DOT1P_COMMA 	','	
#define DCLIWQOS_QOS_FLOW_NUM	4
#define DCLIWQOS_DOT1P_LIST_NUM	16



extern int create_qos(dbus_parameter parameter, DBusConnection *connection,char *id, char *qos_name);
																  /*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示qos id should be 1 to QOS_NUM-1*/
												    			  /*返回-3表示qos name is too long,it should be 1 to 15，返回-4表示qos id exist，返回-5表示error*/
																  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int delete_qos(dbus_parameter parameter, DBusConnection *connection,char *id);
												  /*返回0表示 删除失败，返回1表示删除成功，返回-1表示unknown id format，返回-2表示qos id should be 1 to QOS_NUM-1*/
												  /*返回-3表示qos id does not exist，返回-4表示this qos profile be used by some radios,please disable them first，返回-5表示error*/
												  /*返回-6表示this qos now be used by some radios,please delete them*/
												  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_qos_one(DCLI_WQOS *WQOS);
/*返回1时，调用Free_qos_one()释放空间*/									
extern int show_qos_one(dbus_parameter parameter, DBusConnection *connection,char *id,DCLI_WQOS **WQOS);
																		/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示qos id should be 1 to QOS_NUM-1*/
				                                                        /*返回-3表示qos id does not exist，返回-4表示error*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_qos_head(DCLI_WQOS *WQOS);
/*返回1时，调用Free_qos_head()释放空间*/																		
extern int show_wireless_qos_profile_list(dbus_parameter parameter, DBusConnection *connection,DCLI_WQOS **WQOS);/*返回0表示失败，返回1表示成功，返回-1表示qos not exsit*/
																													 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*qos_stream的范围是"voice","video","besteffort"或"background"*/
/*ACK的范围是"ack"或"noack"*/
extern int config_radio_qos_service(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *qos_stream,char *CWMIN,char *CWMAX,char *AIFS,char *TXOPLIMIT,char *ACK);  
																									/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
																						 			/*返回-3表示qos cwmin should be 0 to QOS_CWMIN_NUM-1，返回-4表示qos cwmax should be 0 to QOS_CWMAX_NUM-1*/
																						 			/*返回-5表示qos aifs should be 0 to QOS_AIFS_NUM-1，返回-6表示qos txoplimit should be 0 to QOS_TXOPLIMIT_NUM*/
																						 			/*返回-7表示qos profile does not exist，返回-8表示this qos profile is used by some radios,please disable them first*/
																						 			/*返回-9表示error，返回-10表示WQOS ID非法，返回-11表示cwmin is not allow larger than cwmax*/
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*qos_stream的范围是"voice","video","besteffort"或"background"*/
extern int config_client_qos_service(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *qos_stream,char *CWMIN,char *CWMAX,char *AIFS,char *TXOPLIMIT);  
																										  /*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
																										  /*返回-3表示qos cwmin should be 0 to QOS_CWMIN_NUM-1，返回-4表示qos cwmax should be 0 to QOS_CWMAX_NUM-1*/
																										  /*返回-5表示qos aifs should be 0 to QOS_AIFS_NUM-1，返回-6表示qos txoplimit should be 0 to QOS_TXOPLIMIT_NUM*/
																										  /*返回-7表示qos profile does not exist，返回-8表示this qos profile is used by some radios,please disable them first*/
																										  /*返回-9表示error，返回-10表示WQOS ID非法，返回-11表示cwmin is not allow larger than cwmax*/
																										  /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*state的范围是"enable"或"disable"*/
extern int config_wmm_service(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *state);  
																			 /*返回0表示失败，返回1表示成功，返回-1表示input parameter should be only 'enable' or 'disable'*/
																		     /*返回-2表示qos id does not exist，返回-3表示this qos profile be used by some radios,please disable them first*/
																		     /*返回-4表示error，返回-5表示WQOS ID非法*/
																			 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*WMM_ORDER的范围是"voice","video","besteffort"或"background"*/
extern int config_wmm_map_dotlp(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *WMM_ORDER,char *DOTLP);  
																			 					 /*返回0表示失败，返回1表示成功，返回-1表示input parameter should be only 'voice' 'video' 'besteffort' or 'background'*/
																								 /*返回-2表示unknown id format，返回-3表示qos dot1p should be 0 to 7，返回-4表示qos id does not exist*/
																								 /*返回-5表示this qos profile be used by some radios,please disable them first，返回-6表示this qos map is disable,please enable it first*/
																								 /*返回-7表示error，返回-8表示WQOS ID非法*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*WMM_ORDER的范围是"voice","video","besteffort"或"background"*/
extern int config_dotlp_map_wmm(dbus_parameter parameter, DBusConnection *connection,int qos_id,char *DOTLP,char *WMM_ORDER); 
																								 /*返回0表示失败，返回1表示成功，返回-1表示input parameter is illegal*/
																						 	     /*返回-2表示input parameter should be only 'voice' 'video' 'besteffort' or 'background'，返回-3表示qos id does not exist*/
																								 /*返回-4表示this qos profile be used by some radios,please disable them first，返回-5表示this qos map is disable,please enable it first*/
																								 /*返回-6表示error，返回-7表示WQOS ID非法*/
																								 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern void Free_qos_extension_info(DCLI_WQOS *WQOS);																								 
/*返回1时，调用Free_qos_extension_info()释放空间*/																								 
extern int show_qos_extension_info(dbus_parameter parameter, DBusConnection *connection,char *wqos_id,DCLI_WQOS **WQOS);
																						 /*返回0表示失败，返回1表示成功*/
																						 /*返回-1表示unknown id format*/
																						 /*返回-2表示qos id should be 1 to QOS_NUM-1*/
																						 /*返回-3表示qos id does not exist，返回-4表示error*/
																						 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_qos_total_bandwidth(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *value);
																				 /*返回0表示失败，返回1表示成功*/
																				 /*返回-1表示unknown id format，返回-2表示qos dot1p should be 1 to 25*/
																				 /*返回-3表示qos id does not exist，返回-4表示error，返回-5表示WQOS ID非法*/
																				 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*flow_type的范围是"besteffort","background","video"或"voice"*/
/*par_type的范围是"averagerate","maxburstiness","managepriority","shovepriority","grabpriority","maxparallel","bandwidth"或"bandwidthpercentage"*/
extern int wid_config_set_qos_flow_parameter(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *flow_type,char *par_type,char *para_value);
																				 						 /*返回0表示失败，返回1表示成功，返回-1表示unknown qos flow type*/
																										 /*返回-2表示unknown qos parameter type，返回-3表示unknown value format*/
																										 /*返回-4表示qos id does not exist，返回-5表示error，返回-6表示WQOS ID非法*/
																										 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*par_type的范围是"totalbandwidth","resourcescale","sharebandwidth"或"resourcesharescale"*/
extern int wid_config_set_qos_parameter(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *par_type,char *par_value);   
																										 	 /*返回0表示失败，返回1表示成功*/
																											 /*返回-1表示unknown qos parameter type，返回-2表示unknown value format*/
																											 /*返回-3表示qos id does not exist，返回-4表示error，返回-5表示WQOS ID非法*/
																											 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*policy_type的范围是"grab"或"shove"*/
extern int wid_config_set_qos_policy_used(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *policy_type,char *policy_state);  
																											 		 /*返回0表示失败，返回1表示成功*/
																													 /*返回-1表示unknown qos policy type*/
																													 /*返回-2表示qos id does not exist，返回-3表示error*/
																													 /*返回-4表示WQOS ID非法*/
																													 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/*policy_type的范围是"grab"或"shove"*/
extern int wid_config_set_qos_policy_name(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *policy_type,char *policy_name);   
																													 /*返回0表示失败，返回1表示成功*/
																													 /*返回-1表示policy name too long，返回-2表示unknown qos policy type*/
																													 /*返回-3表示qos id does not exist，返回-4表示error，返回-5表示WQOS ID非法*/
																													 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int wid_config_set_qos_manage_arithmetic_name(dbus_parameter parameter, DBusConnection *connection,int wqos_id,char *arithmetic_name);  
																													 /*返回0表示失败，返回1表示成功*/
																													 /*返回-1表示arithmetic name too long*/
																													 /*返回-2表示qos id does not exist，返回-3表示error*/
																													 /*返回-4表示WQOS ID非法*/
																													 /*返回SNMPD_CONNECTION_ERROR表示connection error*/

/* type为"besteffort"|"background"|"video"|"voice" */
/*R_L_ID的范围是0-3*/
/*返回1时，调用Free_qos_one()释放空间*/
extern int wid_show_qos_radio_cmd(dbus_parameter parameter, DBusConnection *connection,char *wtp_id,char *R_L_ID,char *type,DCLI_WQOS **WQOS);
																												/*返回0表示失败，返回1表示成功，返回-1表示unknown wtpid format*/
																											    /*返回-2表示unknown local radio id format，返回-3表示wtp id should be 1 to WTP_NUM-1*/
																											    /*返回-4表示local radio id should be 0 to 3，返回-5表示unknown qos flow type*/
																											    /*返回-6表示wtp id not exist，返回-7表示radio id not exist，返回-8表示qos id not exist*/
																											    /*返回-9表示wtp WTPID radio radio_l_id didn't bind qos，返回-10表示qos id should be 1 to QOS_NUM-1*/
																											    /*返回-11表示error*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
#endif	
