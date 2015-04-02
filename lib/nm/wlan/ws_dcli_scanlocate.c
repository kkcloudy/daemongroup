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
* ws_dcli_scanlocate.c
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


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h> 
#include <dbus/dbus.h>
#include <syslog.h>
#include <sys/wait.h>
#include <dirent.h>
#include <dlfcn.h>
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ws_dcli_scanlocate.h"

void Free_show_wtp_wifi_locate_public_config_all_cmd(struct dcli_wifi_locate_public_config *public_config)
{	
	if(public_config!= NULL)
	{
		free(public_config);
		public_config = NULL;
	}	
}



/*只要调用，就通过Free_show_wtp_wifi_locate_public_config_all_cmd()释放空间*/
/*frequency:0表示"2.4G"，1表示"5.8G"*/

int show_wtp_wifi_locate_public_config_all_cmd(dbus_parameter parameter, DBusConnection *connection, struct dcli_wifi_locate_public_config **public_config, long frequency)/*返回0表示失败，返回1表示成功*/
{
	unsigned int ret = 0;
	unsigned int wtp_num;
	//unsigned int index = 0;
	unsigned char channel_flag = 0;
	int retu = 0;
	
	if((frequency < 0) || (frequency > 1))
	{
		return 0;
	}
	
	channel_flag = frequency;
	
	//index = instance_id;

	unsigned int(*dcli_init_func)(
							 int ,
							unsigned int, 
							struct dcli_wifi_locate_public_config** , 
							unsigned int *,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_show_public_config_all_wtp");
		if(NULL != dcli_init_func)
		{
			ret = (*dcli_init_func)
				  (
				  	 (int)parameter.local_id,
					 parameter.instance_id,
					 public_config, 
					 &wtp_num, 
					 channel_flag,
					 connection
				  );
			
	
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(0 == ret)
	{
		retu = 1;	
	}
	else
	{
		retu = 0;
	}

	return retu; 
}

int set_wifi_locate_change_server_ip_cmd(unsigned int local_id, unsigned int instance_id,unsigned char *wtpMac,unsigned int ip,unsigned char channel_flag, void*connection)/*返回0表示失败，返回1表示成功*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;
	
	unsigned int(*dcli_init_func)(
		                             int, 
							unsigned int , 
							unsigned char* ,
							unsigned int ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_server_ip");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				     (int)local_id,
					 index,
					 wtpMac,
					 ip,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*port的范围是1-65535*/
int set_wifi_locate_change_server_port_cmd(unsigned int local_id, int instance_id,unsigned char *wtpMac,unsigned short port,unsigned char channel_flag,void* connection)
																/*返回0表示失败，返回1表示成功*/
																/*返回-1表示port  should be 1 to 65535*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((port < 1) || (port > 65535))
		return -1;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int , 
							unsigned char* ,
							unsigned short ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_server_port");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				     (int)local_id,
					 index,
					 wtpMac,
					 port,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*scan_interval的范围是0-2000*/
int set_wifi_locate_change_channel_scan_interval_cmd(unsigned int local_id, int instance_id,unsigned char *wtpMac,unsigned short scan_interval,unsigned char channel_flag,void* connection)
																				/*返回0表示失败，返回1表示成功*/
																				/*返回-1表示scan_interval  should be 0 to 2000*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((scan_interval < 0) || (scan_interval > 2000))
		return -1;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int , 
							unsigned char* ,
							unsigned short ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_channel_scan_interval");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				     (int)local_id,
					 index,
					 wtpMac,
					 scan_interval,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*scan_time的范围是1-2000*/
int set_wifi_locate_change_channel_scan_time_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned short scan_time,unsigned char channel_flag,void* connection)
																		/*返回0表示失败，返回1表示成功*/
																		/*返回-1表示scan_time  should be 1 to 2000*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((scan_time < 1) || (scan_time > 2000))
		return -1;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int , 
							unsigned char* ,
							unsigned short ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_channel_scan_time");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				    (int)local_id,
					 index,
					 wtpMac,
					 scan_time,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

int set_wifi_locate_change_channellist_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned long long channellist,unsigned char channel_flag, void*connection)/*返回0表示失败，返回1表示成功*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int , 
							unsigned char* ,
							unsigned long long ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_channellist");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				    (int)local_id,
					 index,
					 wtpMac,
					 channellist,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*scan_type的范围是0-2*/
/*0:All; 1:Associate; 2:NonAssociated*/
int set_wifi_locate_change_scan_type_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned char scan_type,unsigned char channel_flag,void*connection)
																	/*返回0表示失败，返回1表示成功*/
																    /*返回-1表示scan_type  should be 0 to 2*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((scan_type < 0) || (scan_type > 2))
		return -1;
	
	unsigned int(*dcli_init_func)(
		                             int, 
							unsigned int , 
							unsigned char* ,
							unsigned char ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_scan_type");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				    (int)local_id,
					 index,
					 wtpMac,
					 scan_type,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*report_interval的范围是1000-20000*/
int set_wifi_locate_change_report_interval_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned short report_interval,unsigned char channel_flag,void* connection)
																	/*返回0表示失败，返回1表示成功*/
																    /*返回-1表示report_interval  should be 1000 to 20000*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((report_interval < 1000) || (report_interval > 20000))
		return -1;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int , 
							unsigned char* ,
							unsigned short ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_report_interval");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  ( 
				    (int)local_id,
					 index,
					 wtpMac,
					 report_interval,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*report_interval的范围是0-95*/
int set_wifi_locate_change_rssi_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned char rssi,unsigned char channel_flag,void* connection)
														/*返回0表示失败，返回1表示成功*/
													    /*返回-1表示rssi  should be 0 to 95*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((rssi < 0) || (rssi > 95))
		return -1;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int , 
							unsigned char* ,
							unsigned char ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_rssi");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				     (int)local_id,
					 index,
					 wtpMac,
					 rssi,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*isenable的范围是0-1*/
/*0:off; 1:on*/
int set_wifi_locate_change_on_off_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned char isenable,unsigned char channel_flag,void*connection)
															/*返回0表示失败，返回1表示成功*/
														    /*返回-1表示isenable  should be 0 to 1*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((isenable < 0) || (isenable > 1))
		return -1;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int , 
							unsigned char* ,
							unsigned char ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_on_off");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				     (int)local_id,
					 index,
					 wtpMac,
					 isenable,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*port的范围是1-65535*/
/*scan_interval的范围是0-2000*/
/*scan_time的范围是1-2000*/
/*scan_type的范围是0-2*/
/*report_interval的范围是1000-20000*/
/*report_interval的范围是0-95*/
/*isenable的范围是0-1*/
int set_wifi_locate_public_para_cmd(unsigned int local_id,int instance_id,unsigned char *wtpMac,unsigned int ip,
										unsigned short port,unsigned short scan_interval,
										unsigned short scan_time,unsigned long long channellist,
										unsigned char scan_type,unsigned short report_interval,
										unsigned char rssi,unsigned char version_num, 
										unsigned char result_filter,unsigned char isenable,
										
										unsigned char channel_flag,void* connection)/*返回0表示失败，返回1表示成功*/
																  /*返回-1表示port  should be 1 to 65535*/
																  /*返回-2表示scan_interval  should be 0 to 2000*/
																  /*返回-3表示scan_time  should be 1 to 2000*/
																  /*返回-4表示scan_type  should be 0 to 2*/
																  /*返回-5表示report_interval  should be 1000 to 20000*/
																  /*返回-6表示rssi	should be 0 to 95*/
																  /*返回-7表示isenable  should be 0 to 1*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}
	
	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((port < 1) || (port > 65535))
		return -1;

	if((scan_interval < 0) || (scan_interval > 2000))
		return -2;

	if((scan_time < 1) || (scan_time > 2000))
		return -3;

	if((scan_type < 0) || (scan_type > 2))
		return -4;

	if((report_interval < 1000) || (report_interval > 20000))
		return -5;

	if((rssi < 0) || (rssi > 95))
		return -6;
	
	if((isenable < 0) || (isenable > 1))
		return -7;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int ,
							unsigned char *,	
							unsigned int ,
							unsigned short ,
							unsigned short ,
							unsigned short ,
							unsigned long long ,
							unsigned char ,
							unsigned short ,
							unsigned char ,
							unsigned char ,
							unsigned char ,
							unsigned char ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_set_public_config");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				     local_id,
					 index,
					 isenable,
					 wtpMac,	
					 report_interval,
					 scan_type,
					 channellist,
					 scan_interval,
					 scan_time,
					 rssi,
					 version_num,
	                 result_filter,
					 ip,
					 port,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}

/*isenable的范围是0-1*/
/*0:off; 1:on*/
int set_wifi_locate_change_on_off_all_wtp_cmd(unsigned int local_id,int instance_id,unsigned char isenable,unsigned char channel_flag,void* connection)
																	/*返回0表示失败，返回1表示成功*/
																    /*返回-1表示isenable  should be 0 to 1*/
{
	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}
	
	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;

	if((isenable < 0) || (isenable > 1))
		return -1;
	
	unsigned int(*dcli_init_func)(
		                             int,
							unsigned int , 
							unsigned char ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_on_off_all_wtp");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				     (int)local_id,
					 index,
					 isenable,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}


int set_wifi_locate_change_version_cmd(unsigned int local_id,unsigned int instance_id,
									   unsigned char *wtpMac,
									   unsigned char version_num,
									   unsigned char channel_flag,
									   void *connection)/*返回0表示失败，返回1表示成功*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;
	
	unsigned int(*dcli_init_func)(
							unsigned int , 
							unsigned int , 
							unsigned char* ,
							unsigned char ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_version_num");
		if(NULL != dcli_init_func)
		{
			
			ret =(*dcli_init_func)
				  (
				  	 (int)local_id,
					 index,
					 wtpMac,
					 version_num,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}


int set_wifi_locate_change_filter_cmd( unsigned int local_id,unsigned int instance_id,
									   unsigned char *wtpMac,
									   unsigned char filter,
									   unsigned char channel_flag,
									   void *connection)/*返回0表示失败，返回1表示成功*/
{
	if(NULL == wtpMac)
		return 0;

	if((channel_flag < 0) || (channel_flag > 1))
	{
		return 0;
	}

	unsigned int ret = 0;
	unsigned int index = 0;
	index = instance_id;
	
	unsigned int(*dcli_init_func)(			
							unsigned int ,
							unsigned int , 
							unsigned char* ,
							unsigned char ,
							unsigned char ,
							DBusConnection *
						);

	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"snmp_wifi_locate_change_result_filter");
		if(NULL != dcli_init_func)
		{
			ret =(*dcli_init_func)
				  (
				     (int)local_id,
					 index,
					 wtpMac,
					 filter,
					 channel_flag,
					 connection
				  );
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	if(ret == 0)
		return 1;
	else
		return 0;
}





#ifdef __cplusplus
}
#endif

