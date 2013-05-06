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
* ws_dcli_bss.c
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

#include "ws_dcli_bss.h"

void Free_ap_statistics_head(DCLI_AC_API_GROUP_THREE *statics)
{
	void (*dcli_init_free_func)(char *,DCLI_AC_API_GROUP_THREE *);
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_free_func = dlsym(ccgi_dl_handle,"dcli_ac_free_fun_three");
		if(NULL != dcli_init_free_func)
		{
			dcli_init_free_func(WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST,statics);
			statics = NULL;
		}
	}
}

/*返回1时，调用Free_ap_statistics_head()释放空间*/
int show_ap_statistics_list_bywtp(dbus_parameter parameter ,DBusConnection *connection,int id,DCLI_AC_API_GROUP_THREE **statics)/*返回0表示失败，返回1表示成功，返回-1表示input wtp id should be 1 to WTP_NUM-1，返回-2表示ap have not statistics information，返回-3表示wtp id no exist*/
																																	/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
        
	int ret = -1;
	int retu = 0;
	int wtpid = 0;

	wtpid = id;
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		return -1;
		}

	void*(*dcli_init_func)(
					int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int ,
					unsigned int* ,
					unsigned int* ,
					unsigned int* ,
					DBusConnection *,
					char *
					);

    *statics = NULL;
	if(NULL != ccgi_dl_handle)
	{
		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_ac_show_api_group_three");
		if(NULL != dcli_init_func)
		{
			*statics =(*dcli_init_func)
				  (
					parameter.instance_id,
					SIXTH,/*"show ap statistics list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&(parameter.local_id),
					connection,
					WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST
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
	
	if(ret == -1)
	{
		retu = SNMPD_CONNECTION_ERROR;
	}
	else if((ret == 0)&&(*statics))
	{
		retu = 1;
	}
	else if(ret == WID_AP_NO_STATICS)
	{
		retu = -2;
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{
		retu = -3;
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		retu = -1;
	}
		
	return retu;
}



#ifdef __cplusplus
}
#endif

   
