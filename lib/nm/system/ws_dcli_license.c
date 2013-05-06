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
* ws_dcli_license.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/wait.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_init_dbus.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_license.h"


/*dcli_license.c V1.2*/
/*author qiaojie*/
/*update time 09-6-23*/

int license_request_cmd_func()
{
	unsigned char cmd[DEFAULT_LEN];
	sprintf(cmd,"sudo /etc/lic/licreq_gen.sh > /var/run/license.tmp");
	system(cmd);
	return 0;
}

/*
	author zhangwl;
	update time 11-9-5;
	dcli_license.c V1.3
*/
int license_text_cmd_func()
{	
		
	unsigned char cmd[DEFAULT_LEN];
	sprintf(cmd,"/etc/lic/lic_dump.sh 0 > /var/run/maxwtpcount.lic ");
	system(cmd);
	return 0;

}

int license_install_cmd_func(char *SN,int *err_code)/*返回0表示失败，返回1表示成功，返回-1表示License fail,please contact vendor*/
{
	DBusMessage *query, *reply;	
	DBusMessage *query2, *reply2;
	DBusError err;
	DBusError err2;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter2;
	int ret;
	int ret2;
	int stat;
	int reason;
	unsigned char *cmd;	
	cmd = (unsigned char*)malloc(strlen(SN)+64);
	memset(cmd,0,strlen(SN)+64);
	sprintf(cmd,"/etc/lic/lic_inst.sh %s",SN);
	stat = system(cmd);
	reason = WEXITSTATUS(stat);

	
	if(reason != 0)
	{

		*err_code=reason;		
		
		if(cmd)
		{
			free(cmd);
			cmd = NULL;
		}
		return -1;
	}
	if(cmd)
	{

		free(cmd);
		cmd = NULL;
	}

	int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;
	ccgi_ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_WTP_COUNT);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	if(ret == WID_DBUS_SUCCESS){
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);
		ccgi_ReInitDbusPath(index,ASD_DBUS_BUSNAME,BUSNAME);
		ccgi_ReInitDbusPath(index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ccgi_ReInitDbusPath(index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,ASD_DBUS_SECURITY_METHOD_UPDATE_WTP_COUNT);
		
		dbus_error_init(&err2);
		reply2 = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query2,-1, &err2);	
		dbus_message_unref(query2);
		if (NULL == reply2) {
			if (dbus_error_is_set(&err2)) {
				dbus_error_free(&err2);
			}
			return 0;
		}
		
		dbus_message_iter_init(reply2,&iter2);
		dbus_message_iter_get_basic(&iter2,&ret2);
		dbus_message_unref(reply2);
		
		/*delete by qiaojie*/
		/*由于在web页面中，install license之后，不需要通过DcliWReInit函数重新计算WTP_NUM等全局变量*/
		/*如果需要在snmp中支持install license功能，就必须调用DcliWReInit函数*/
		
		/*if(ret2 == ASD_DBUS_SUCCESS){
			DcliWReInit();	
		}*/
	}
	return 1;
}



#ifdef __cplusplus
}
#endif


