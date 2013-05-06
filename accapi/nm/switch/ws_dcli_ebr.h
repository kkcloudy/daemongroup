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
* ws_dcli_ebr.h
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



#ifndef _WS_DCLI_EBR_H
#define _WS_DCLI_EBR_H

#include <syslog.h>
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"

extern int create_ethereal_bridge_cmd(dbus_parameter parameter, DBusConnection *connection,char *id,char *brname);  
																					/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																					/*返回-2表示ebr id should be 1 to EBR_NUM-1，返回-3表示ebr name is too long,it should be 1 to 15*/
																					/*返回-4表示ebr id exist，返回-5表示ebr  is already exist，返回-6表示system cmd error，返回-7表示error*/
																					/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																					
extern int delete_ethereal_bridge_cmd(dbus_parameter parameter, DBusConnection *connection,char *id);  
																		/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示ebr id should be 1 to EBR_NUM-1*/
																		/*返回-3表示ebr id does not exist，返回-4表示system cmd error，返回-5表示ebr is enable,please disable it first，返回-6表示error*/		
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																		
extern void Free_ethereal_bridge_one_head(DCLI_EBR_API_GROUP *EBRINFO);
/*返回1时，调用Free_ethereal_bridge_one_head()释放空间*/
extern int show_ethereal_bridge_one(dbus_parameter parameter, DBusConnection *connection,char *id,DCLI_EBR_API_GROUP **EBRINFO );  
																									/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																									/*返回-2表示ebr id should be 1 to EBR_NUM-1，返回-3表示ebr id does not exist*/
																									/*返回-4表示error*/ 			
																									/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																									
extern void Free_ethereal_bridge_head(DCLI_EBR_API_GROUP *EBRINFO);
/*返回1时，调用Free_ethereal_bridge_head()释放空间*/
extern int show_ethereal_bridge_list(dbus_parameter parameter, DBusConnection *connection,DCLI_EBR_API_GROUP **EBRINFO);/*返回0表示失败，返回1表示成功，返回-1表示no ebr exist*/
																															/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int config_ethereal_bridge_enable_cmd(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *ebr_state);
																								/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																								/*返回-2表示ebr id does not exist，返回-3表示ebr if error，返回-4表示system cmd process error*/
																								/*返回-5表示error，返回-6示EBR ID非法*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								
extern int ebr_set_bridge_isolation_func(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *isolate_state);	
																								/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																								/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																								/*返回-5表示system cmd process error，返回-6表示sameportswitch and isolation are conflict,disable sameportswitch first*/
																								/*返回-7表示error，返回-8示EBR ID非法，返回-9表示apply security in this wlan first*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								
extern int ebr_set_bridge_multicast_isolation_func(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *mult_isolate_state);   
																												/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																												/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																												/*返回-5表示system cmd process error，返回-6表示sameportswitch and isolation are conflict,disable sameportswitch first*/
																												/*返回-7表示error，返回-8示EBR ID非法，返回-9表示apply security in this wlan first*/
																												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																												
extern int set_ebr_add_del_if_cmd(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *if_state,char *if_name);
																								/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'add' or 'delete'*/
																								/*返回-2表示if name too long，返回-3表示ebr id does not exist，返回-4表示ebr should be disable first*/
																								/*返回-5表示if_name already exist/remove some br,or system cmd process error，返回-6表示input ifname error*/
																								/*返回-7表示ebr if error，返回-8表示error，返回-9示EBR ID非法*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
																								
extern int ebr_set_bridge_sameportswitch_func(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *spswitch_state);  
																										/*返回0表示失败，返回1表示成功，返回-1表示input parameter should only be 'enable' or 'disable'*/
																										/*返回-2表示ebr id does not exist，返回-3表示ebr should be disable first，返回-4表示ebr if error*/
																										/*返回-5表示system cmd process error，返回-6表示isolation or multicast are enable,disable isolation and multicast first*/
																										/*返回-7表示error，返回-8示EBR ID非法*/
																										/*返回SNMPD_CONNECTION_ERROR表示connection error*/

extern int set_ebr_add_del_uplink_cmd(dbus_parameter parameter, DBusConnection *connection,int ebr_id,char *addordel,char *ifnamez); 
																									  /*返回0表示失败，返回1表示成功，返回-1表示error*/
																									  /*返回-2表示input parameter should only be 'add' or 'delete'，返回-3表示if name too long*/
																									  /*返回-4表示malloc error，返回-5表示ebr should be disable first*/
																									  /*返回-6表示already exist/remove some br,or system cmd process error，返回-7表示input ifname error*/
																									  /*返回-8表示ebr if error，返回-9表示interface does not add to br or br uplink，返回-10表示ebr id does not exist*/
																									  /*返回-11示EBR ID非法*/
																									  /*返回SNMPD_CONNECTION_ERROR表示connection error*/
#endif

