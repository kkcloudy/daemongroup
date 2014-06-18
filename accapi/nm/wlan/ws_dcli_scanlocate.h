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
* ws_dcli_scanlocate.h
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

#ifndef _WS_DCLI_SCANLOCATE_H
#define _WS_DCLI_SCANLOCATE_H

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "ws_dbus_list_interface.h"

extern void Free_show_wtp_wifi_locate_public_config_all_cmd(struct dcli_wifi_locate_public_config *public_config);
/*只要调用，就通过Free_show_wtp_wifi_locate_public_config_all_cmd()释放空间*/
/*frequency:0表示"2.4G"，1表示"5.8G"*/
/*extern int show_wtp_wifi_locate_public_config_all_cmd(int instance_id,unsigned char frequency,struct dcli_wifi_locate_public_config **public_config,unsigned int *wtp_num);*//*返回0表示失败，返回1表示成功*/


extern int show_wtp_wifi_locate_public_config_all_cmd(dbus_parameter parameter, DBusConnection *connection, struct dcli_wifi_locate_public_config **public_config, long frequency);/*返回0表示失败，返回1表示成功*/



//extern int set_wifi_locate_change_server_ip_cmd(int instance_id,unsigned char *wtpMac,unsigned int ip,unsigned char channel_flag);/*返回0表示失败，返回1表示成功*/
extern int set_wifi_locate_change_server_ip_cmd(unsigned int local_id, unsigned int instance_id,unsigned char *wtpMac,unsigned int ip,unsigned char channel_flag, void*connection);/*返回0表示失败，返回1表示成功*/

/*port的范围是1-65535*/
//extern int set_wifi_locate_change_server_port_cmd(int instance_id,unsigned char *wtpMac,unsigned short port,unsigned char channel_flag);
																					/*返回0表示失败，返回1表示成功*/
																					/*返回-1表示port  should be 1 to 65535*/
extern int set_wifi_locate_change_server_port_cmd(unsigned int local_id, int instance_id,unsigned char *wtpMac,unsigned short port,unsigned char channel_flag,void* connection);

																					

/*scan_interval的范围是0-2000*/
//extern int set_wifi_locate_change_channel_scan_interval_cmd(int instance_id,unsigned char *wtpMac,unsigned short scan_interval,unsigned char channel_flag);
																					/*返回0表示失败，返回1表示成功*/
																					/*返回-1表示scan_interval  should be 0 to 2000*/

extern int set_wifi_locate_change_channel_scan_interval_cmd(unsigned int local_id, int instance_id,unsigned char *wtpMac,unsigned short scan_interval,unsigned char channel_flag,void* connection);
																					

/*scan_time的范围是1-2000*/
//extern int set_wifi_locate_change_channel_scan_time_cmd(int instance_id,unsigned char *wtpMac,unsigned short scan_time,unsigned char channel_flag);
																					/*返回0表示失败，返回1表示成功*/
																					/*返回-1表示scan_time  should be 1 to 2000*/
extern int set_wifi_locate_change_channel_scan_time_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned short scan_time,unsigned char channel_flag,void* connection);

//extern int set_wifi_locate_change_channellist_cmd(int instance_id,unsigned char *wtpMac,unsigned long long channellist,unsigned char channel_flag);/*返回0表示失败，返回1表示成功*/

extern int set_wifi_locate_change_channellist_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned long long channellist,unsigned char channel_flag, void*connection);/*返回0表示失败，返回1表示成功*/

/*scan_type的范围是0-2*/
/*0:All; 1:Associate; 2:NonAssociated*/
//extern int set_wifi_locate_change_scan_type_cmd(int instance_id,unsigned char *wtpMac,unsigned char scan_type,unsigned char channel_flag);
																					/*返回0表示失败，返回1表示成功*/
																				    /*返回-1表示scan_type  should be 0 to 2*/
extern int set_wifi_locate_change_scan_type_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned char scan_type,unsigned char channel_flag,void*connection);

/*report_interval的范围是1000-20000*/
//extern int set_wifi_locate_change_report_interval_cmd(int instance_id,unsigned char *wtpMac,unsigned short report_interval,unsigned char channel_flag);
																					/*返回0表示失败，返回1表示成功*/
																				    /*返回-1表示report_interval  should be 1000 to 20000*/

extern int set_wifi_locate_change_report_interval_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned short report_interval,unsigned char channel_flag,void* connection);


/*report_interval的范围是0-95*/
//extern int set_wifi_locate_change_rssi_cmd(int instance_id,unsigned char *wtpMac,unsigned char rssi,unsigned char channel_flag);
																					/*返回0表示失败，返回1表示成功*/
																				    /*返回-1表示rssi	should be 0 to 95*/
extern int set_wifi_locate_change_rssi_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned char rssi,unsigned char channel_flag,void* connection);
/*isenable的范围是0-1*/
/*0:off; 1:on*/



//extern int set_wifi_locate_change_on_off_cmd(int instance_id,unsigned char *wtpMac,unsigned char isenable,unsigned char channel_flag);
																					/*返回0表示失败，返回1表示成功*/
																				    /*返回-1表示isenable  should be 0 to 1*/
extern int set_wifi_locate_change_on_off_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned char isenable,unsigned char channel_flag,void*connection);

/*port的范围是1-65535*/
/*scan_interval的范围是0-2000*/
/*scan_time的范围是1-2000*/
/*scan_type的范围是0-2*/
/*report_interval的范围是1000-20000*/
/*report_interval的范围是0-95*/
/*isenable的范围是0-1*/
extern int set_wifi_locate_public_para_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned int ip,
										unsigned short port,unsigned short scan_interval,
										unsigned short scan_time,unsigned long long channellist,
										unsigned char scan_type,unsigned short report_interval,
										unsigned char rssi,unsigned char isenable,
										unsigned char channel_flag,void* connection);/*返回0表示失败，返回1表示成功*/
															    /*返回-1表示port  should be 1 to 65535*/
															    /*返回-2表示scan_interval  should be 0 to 2000*/
															    /*返回-3表示scan_time  should be 1 to 2000*/
															    /*返回-4表示scan_type  should be 0 to 2*/
															    /*返回-5表示report_interval	should be 1000 to 20000*/
															    /*返回-6表示rssi  should be 0 to 95*/
															    /*返回-7表示isenable  should be 0 to 1*/

/*isenable的范围是0-1*/
/*0:off; 1:on*/
//extern int set_wifi_locate_change_on_off_all_wtp_cmd(int instance_id,unsigned char isenable,unsigned char channel_flag);
																				/*返回0表示失败，返回1表示成功*/
																			    /*返回-1表示isenable  should be 0 to 1*/
extern int set_wifi_locate_change_on_off_all_wtp_cmd(unsigned int local_id,int instance_id,unsigned char isenable,unsigned char channel_flag,void* connection);

#endif

