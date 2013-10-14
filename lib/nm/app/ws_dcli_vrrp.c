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
* ws_dcli_vrrp.c
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
//////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>
////////////////////////////

#include <ctype.h>
/*#include "ws_ec.h"*/
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include "dbus/hmd/HmdDbusDef.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include <util/npd_list.h>
#include <unistd.h>
#include "npd/nbm/npd_bmapi.h"
#include <netdb.h>  
#include "ws_returncode.h"
#include "wcpss/wid/WID.h"
#include <fcntl.h> 
#include <signal.h>
#include "ws_dcli_vrrp.h"

/**************

$version dcli_vrrp.c 1.353
         dcli_vrrp.h v1.9
         
$date  2009-12-11

***************/

inline void reset_sigmaskz() {
sigset_t psmask;
memset(&psmask,0,sizeof(psmask));
sigprocmask(SIG_SETMASK,&psmask,NULL);
}
//add new
int ccgi_vrrp_check_service_started
(
	unsigned char *service_name,
	unsigned int profileId
)
{
	int ret = 0;
	int fd = -1;
	int iCnt = 0;	
	char commandBuf[DCLI_VRRP_SYS_COMMAND_LEN] = {0};
	char readBuf[4] = {0};

	if (!service_name) {
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	/* check if hansi is running or not 
	 * with following shell command:
	 *	'ps -ef | grep \"%s %d$\" | wc -l'
	 *	if count result gt 1, running, else not running.
	 */
	sprintf(commandBuf, "sudo ps auxww | grep \"%s %d$\" | wc -l > /var/run/%s%d.boot",
						service_name, profileId, service_name, profileId);
	ret = system(commandBuf);
	if (ret) {
		//vty_out(vty, "%% Check %s instance %d failed!\n", service_name, profileId);
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	/* get the process # */
	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "/var/run/%s%d.boot", service_name, profileId);
	if ((fd = open(commandBuf, O_RDONLY))< 0) {
		//vty_out(vty, "%% Check %s instance %d count failed!\n", service_name, profileId);
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	memset(readBuf, 0, 4);
	read(fd, readBuf, 4);
	iCnt = strtoul(readBuf, NULL, 10);
	//vty_out(vty, "%s %d thread count %d\n", service_name, profileId, iCnt);

	if (!strncmp(service_name, "had", 3)) {
		if (DCLI_VRRP_THREADS_CNT == iCnt) {
			ret = DCLI_VRRP_INSTANCE_CREATED;
		}
		else {
			ret = DCLI_VRRP_INSTANCE_NO_CREATED;
		}
	}else {
		/* for wcpss, include wid/asd/wsm process */
		if (3 <= iCnt) {
			ret = DCLI_VRRP_INSTANCE_CREATED;
		}
		else {
			ret = DCLI_VRRP_INSTANCE_NO_CREATED;
		}
	}	
	
	/* release file resources */
	close(fd);

	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "sudo rm /var/run/%s%d.boot", service_name, profileId);
	system(commandBuf);

	return ret;
}
void ccgi_vrrp_splice_objpath_string
(
	char *common_objpath,
	unsigned int profile,
	char *splice_objpath
)
{
	sprintf(splice_objpath,
			"%s%d",
			common_objpath, profile);
	return;
}
void ccgi_vrrp_splice_dbusname
(
	char *common_dbusname,
	unsigned int profile,
	char *splice_dbusname
)
{
	sprintf(splice_dbusname,
			"%s%d",
			common_dbusname, profile);
	return;
}

//config hansi-profile
int ccgi_config_hansi_profile(char *pro_num)/*返回0表示成功，返回-1表示error，返回-2表示create hansi faild，返回-3表示create  instrance  timeout*/
                                            /*返回-4表示create wcpss  faild*/
{
    reset_sigmaskz();
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	int instRun = 0;
	int retu = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	char cmd[DCLI_VRRP_DBUSNAME_LEN] = {0};
	int cr_timeout = 0;
	int check_result = DCLI_VRRP_INSTANCE_CREATED;
	unsigned char service_name[4][4] = {"had","wid", "asd", "wsm"};
	unsigned int service_index = 0;

	profile = strtoul((char *)pro_num,NULL,0);
	if((profile < 1)||profile > 16)
	{
        //vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return -3;
	}
	instRun = ccgi_vrrp_hansi_is_running(profile);
	
	if(DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		sprintf(cmd, "sudo /etc/init.d/had start %d &", profile);
		if (system(cmd)) {
			//vty_out(vty, "create hansi %d faild.\n", profile);
			//return -2;
			return -2;
		}

		/* check wheather had instance started completely. */
		while (1) {
			cr_timeout++;
			check_result = ccgi_vrrp_check_service_started(service_name[0], profile);
			if (DCLI_VRRP_INSTANCE_NO_CREATED == check_result) {
				//vty_out(vty, "create %s instrance %d time %d s.\n", service_name[0], profile, cr_timeout);
				/* 3524 1s */
				if (4 == cr_timeout) {
					//vty_out(vty, "create %s instrance %d timeout.\n", service_name[0], profile);
					//return -2;
					return -3;
				}
				sleep(1);
				continue;
			}else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == check_result) {
				//return -2;
				return -1;
			}else if (DCLI_VRRP_INSTANCE_CREATED == check_result) {
				//vty_out(vty, "create %s instrance %d used %d s.\n", service_name[0], profile, cr_timeout);
				cr_timeout = 0;
				break;
			}
		}		

		/* add for create wcpss process */
		memset(cmd,0,DCLI_VRRP_DBUSNAME_LEN);
		sprintf(cmd, "sudo /etc/init.d/wcpss start %d &", profile);
		//printf("wcpss %s\n",cmd);
		if (system(cmd)) {
			//vty_out(vty, "create wcpss %d faild.\n", profile);
			//return -2;
			return -4;
		}

		/* check wheather wcpss instance started completely.
		 * wsm not support multi-process.
		 */
		for (service_index = 1; service_index < 3; service_index++) {
			cr_timeout = 0;
			while (1) {
				cr_timeout++;
				check_result = ccgi_vrrp_check_service_started(service_name[service_index], profile);
				if (DCLI_VRRP_INSTANCE_NO_CREATED == check_result) {
					//vty_out(vty, "create %s instrance %d time %d s.\n", service_name[service_index], profile, cr_timeout);
					if (100 == cr_timeout) {
						//vty_out(vty, "create %s instrance %d timeout.\n", service_name[service_index], profile);
						//return -2;
						return -3;
					}
					sleep(1);
					continue;
				}else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == check_result) {
					//return -2;
					return -1;
				}else if (DCLI_VRRP_INSTANCE_CREATED == check_result) {
					//vty_out(vty, "create %s instrance %d used %d s.\n", service_name[service_index], profile, cr_timeout);
					cr_timeout = 0;
					break;
				}
			}
		}
		sleep(3);	/* for wait asd dbus thread create ok */
	}
	else if(DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		//return -2;
		return -1;
	}

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	//vty_out(vty, "path[%s] name[%s]\n", vrrp_obj_path, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_CONFIG_HANSI_PROFILE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu  = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&profile,
		DBUS_TYPE_INVALID)){	

		if(DCLI_VRRP_RETURN_CODE_OK ==op_ret){

			/*
			if(CONFIG_NODE==(vty->node)){
					vty->node= HANSI_NODE;	
					vty->index = (void *)profile;
			}
			*/
			retu = 0;
		}
		else{
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            
            //错误类型
			retu = op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	return retu;
}
int ccgi_config_hansi_profile_web(char *pro_num,int slotid,DBusConnection *connection)
/*返回0表示成功，返回-1表示error，返回-2表示The Slot is not active master board, permission denial，返回-3表示The Slot is not EXIST*/
                                            /*返回-4表示remote hansi not exist*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int hmd_ret = 0;
	unsigned int profile = 0;
	 int slot_id = slotid;
	DBusMessage *query2 = NULL, *reply2 = NULL;
	DBusError err2 = {0};
	unsigned int op_ret = 0;
	int instRun = 0;
	char cmd[DCLI_VRRP_DBUSNAME_LEN] = {0}; /*wcl add*/
	unsigned char insID = 0;
	int flag = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	
	fprintf(stderr,"pro_num=%s\n",pro_num);
	fprintf(stderr,"slotid=%d\n",slotid);
	fprintf(stderr,"connection=%p\n",connection);
	profile = strtoul((char *)pro_num,NULL,0);
	if((profile < 1)||profile > 16)
	{
		return -1;
	}

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_REMOTE_HANSI);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,150000, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}

	if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&hmd_ret,
				DBUS_TYPE_INVALID))
	{	
		if(hmd_ret == 0){
			return 0;
		}
		else if(hmd_ret == HMD_DBUS_PERMISSION_DENIAL){
			return -2;
		}else if(hmd_ret == HMD_DBUS_SLOT_ID_NOT_EXIST){
			return -3;
		}else if (hmd_ret == HMD_DBUS_ID_NO_EXIST){
			return -4;
		}else{
			return -1;
		}

	} 
	else 
	{	
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
		return -1;
	}
	
	dbus_message_unref(reply);
	reply = NULL;
	
	return 0;
}

int delete_hansi_profile(char *profileid)/*返回0表示成功，返回-1表示profileid超出范围，返回-2表示config hansi  service disable faild*/
                                         /*返回-3表示delete hansi  faild，返回-4表示create hansi  faild.返回-5表示error，返回-6表示hansi no exist*/
{
	unsigned int ret = 0;
	unsigned int profile = 0;
	int instRun = 0;
	char cmd[DCLI_VRRP_DBUSNAME_LEN * 2] = {0};

	//if (argc == 1) {
		profile =(unsigned int) strtoul((char *)profileid, NULL, 0);
		if ((profile < 1) ||
			(profile > 16)) {
			//vty_out(vty, "%% Bad parameter : %s !", argv[0]);
			//return -2;
			return -1;
		}
	//}

	/* system to fork had process */
	instRun = ccgi_vrrp_hansi_is_running( profile);
	if (DCLI_VRRP_INSTANCE_CREATED == instRun) {
		/* [1] disable the had instance service,
		 *		for it will tell other service(eag/wcpss) its state,
		 *		 when STATE MACHINE transfer to other state.
		 */
		//vty_out(vty, "%% Disable hansi %d service ...\n", profile);
		ret = ccgi_vrrp_config_service_disable( profile);
		if (0 != ret) {
			//vty_out(vty, "config hansi %d service disable faild.\n", profile);
			//return -2;
			return -2;
		}

		/* [2] kill the special had instance process which instance no is profile. */
		//vty_out(vty, "%% Delete hansi instance %d ...\n", profile);
		#if 0
		sprintf(cmd,
				"PIDS=`ps -ef | grep \"had %d$\" | grep -v grep | awk '{print $2}'`; father=`echo $PIDS | awk '{print $1}'`; sudo kill $father",
				profile);
		#else
		sprintf(cmd, "/etc/init.d/had stop %d &", profile);
		#endif
		if (system(cmd)) {
			//vty_out(vty, "delete hansi %d faild.\n", profile);
			//return -2;
			return -3;
		}

		/* [3] clear tmp file of had instance */
		memset(cmd, 0, DCLI_VRRP_DBUSNAME_LEN * 2);
		sprintf(cmd,
				"sudo rm /var/run/had%d.pidmap",
				profile);
		if (system(cmd)) {
			//vty_out(vty, "delete hansi %d faild.\n", profile);
			//return -2;
			return -3;
		}
		usleep(1000000);

		/* [4] kill wid(...) process */
		//vty_out(vty, "%% Delete wcpss instance %d ...\n", profile);
		memset(cmd, 0, DCLI_VRRP_DBUSNAME_LEN * 2);
		sprintf(cmd, "sudo /etc/init.d/wcpss stop %d &", profile);
		if (system(cmd)) {
			//vty_out(vty, "create hansi %d faild.\n", profile);
			//return -2;
			return -4;
		}
	}
	else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
			//return -2;
			return -5;
	}else if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		//vty_out(vty, "hansi %d no exist.\n", profile);
		//return 0;
		return -6;
	}

	//return 0;
	return 0;
}
int delete_hansi_profile_web(char *profileid,DBusConnection *connection)/*返回0表示成功，返回-1表示profileid超出范围，返回-2表示config hansi  service disable faild*/
                                         /*返回-3表示delete hansi  faild，返回-4表示create hansi  faild.返回-5表示error，返回-6表示hansi no exist*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;

	profile = atoi(profileid);
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_END_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -5;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            return -5;
		}
	}
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return 0;
}
int config_vrrp_heartbeat_cmd_func(char * profid,char *ifnamez,char *ipz)/*返回0表示成功，返回-1表示失败，返回-2表示心跳线ip为空，返回-3表示心跳线接口名为空*/
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//unsigned int vrid = 0,priority = 0;
	//int add = 1;
	char* heartbeat_ifname = NULL;
    char* heartbeat_ip = NULL;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
	
	heartbeat_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == heartbeat_ifname){
       return -3;
	}	
	memset(heartbeat_ifname,0,MAX_IFNAME_LEN);
	memcpy(heartbeat_ifname,ifnamez,strlen(ifnamez));

	
	heartbeat_ip = (char *)malloc(MAX_IPADDR_LEN);
    if(NULL == heartbeat_ip){
       //return -2;
       free(heartbeat_ifname);
       return -2;
	}	
	memset(heartbeat_ip,0,MAX_IPADDR_LEN);
	memcpy(heartbeat_ip,ipz,strlen(ipz));

    //current node 
	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile=(unsigned int)strtoul(profid,0,10);

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_HEARTBEAT_LINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
		                     DBUS_TYPE_STRING,&heartbeat_ifname,
							 DBUS_TYPE_STRING,&heartbeat_ip,						 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            //return error;
			return op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		/*dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	free(heartbeat_ip);
	free(heartbeat_ifname);
	dbus_message_unref(reply);
	return 0;
}

int config_vrrp_heartbeat_cmd_func_web(char * profid,char *ifnamez,char *ipz,DBusConnection *connection)/*返回0表示成功，返回-1表示失败，返回-2表示心跳线ip为空，返回-3表示心跳线接口名为空*/
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	char* heartbeat_ifname = NULL;
    char* heartbeat_ip = NULL;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
	
	heartbeat_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == heartbeat_ifname){
       return -3;
	}	
	memset(heartbeat_ifname,0,MAX_IFNAME_LEN);
	memcpy(heartbeat_ifname,ifnamez,strlen(ifnamez));

	
	heartbeat_ip = (char *)malloc(MAX_IPADDR_LEN);
    if(NULL == heartbeat_ip){
		if(heartbeat_ifname){
			free(heartbeat_ifname);
			heartbeat_ifname=NULL;
		}
       return -2;
	}	
	memset(heartbeat_ip,0,MAX_IPADDR_LEN);
	memcpy(heartbeat_ip,ipz,strlen(ipz));

	profile=(unsigned int)strtoul(profid,0,10);

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_HEARTBEAT_LINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
		                     DBUS_TYPE_STRING,&heartbeat_ifname,
							 DBUS_TYPE_STRING,&heartbeat_ip,						 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			return op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	free(heartbeat_ip);
	free(heartbeat_ifname);
	dbus_message_unref(reply);
	return 0;
}


int ccgi_config_realip_downlink(char *profid,char *downifname,char *downip)/*返回0表示成功，返回-1表示error,返回-2表示接口名为空*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//unsigned int vrid = 0,priority = 0;
	//int add = 1;
	char* downlink_ifname = NULL;
    char* downlink_ip = NULL;


	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       //return -2;
       return -2;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,downifname,strlen(downifname));
	
	downlink_ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == downlink_ip){
       //return -2;
       return -2;
	}
	memset(downlink_ip,0,MAX_IPADDR_LEN);
	memcpy(downlink_ip,downip,strlen(downip));


	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
    profile = (unsigned int)strtoul(profid,0,10);
    
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_REAL_IP_DOWNLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_STRING,&downlink_ip,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            return op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		/*dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	free(downlink_ip);
	dbus_message_unref(reply);
	//return 0;
	free(downlink_ifname);
	return 0;
}

int  set_hansi_vrid(char *profid,char *vidz)/*返回0表示成功，返回-1表示error，返回-2表示vidz超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0,vrid = 0;
	vrid = strtoul((char *)vidz,NULL,10);
	if((vrid < 1)||vrid > 255){
       // vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		//return -2;
	   return -2;
	}

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
    profile = (unsigned int )strtoul(profid,0,10);
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SET_VRRPID);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&vrid,						 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            return op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		/*dcli_vrrp_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	//return 0;
	return 0;
}


 
//"uplink IFNAME IP downlink IFNAME IP priority <1-255>"

int ccgi_downanduplink_ifname(char *provrrp,char *upifname,char *upip,char *downifname,char *downip,char *prio)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0,priority = 0;
	//int add = 1;
	char* uplink_ifname = NULL,*downlink_ifname = NULL;
    //char* uplink_ip = NULL,*downlink_ip = NULL;
    unsigned long uplink_ip = 0,downlink_ip = 0;
	unsigned int  uplink_mask = 32,downlink_mask = 32;
	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == uplink_ifname){
       //return -2;
       return -2;
	}	
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,upifname,strlen(upifname));

	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       //return -2;
       free(uplink_ifname);
       return -2;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,downifname,strlen(downifname));

	/*
	uplink_ip = (char *)malloc(MAX_IPADDR_LEN);
    if(NULL == uplink_ip){
       return -2;
	}	
	memset(uplink_ip,0,MAX_IPADDR_LEN);
	memcpy(uplink_ip,upip,strlen(upip));

	downlink_ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == downlink_ip){
       return -2;
	}
	memset(downlink_ip,0,MAX_IPADDR_LEN);
	memcpy(downlink_ip,downip,strlen(downip));
    */

	uplink_ip = inet_addr((unsigned char*)upip);
    downlink_ip = inet_addr((unsigned char*)downip);


	/*(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(provrrp,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&uplink_ifname,
							 DBUS_TYPE_UINT32,&uplink_ip,
							 DBUS_TYPE_UINT32,&uplink_mask,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,	
							 DBUS_TYPE_UINT32,&downlink_mask,	
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            return op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	free(uplink_ifname);
	free(downlink_ifname);
	dbus_message_unref(reply);
	//return 0;
	return 0;
}

int ccgi_downanduplink_ifname_mask(char *provrrp,char *upifname,char *upip,char *downifname,char *downip,char *prio,int upmask,int downmask)
                                                  /*返回0表示成功，返回-1表示error，返回-2表示ifname为空,返回-3表示ip为空,返回-4表示优先值超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	int retu=0;
	unsigned int profile = 0,priority = 0;
	//unsigned int vrid = 0;
	int split = 0;
	char* uplink_ifname = NULL,*downlink_ifname = NULL;
    /*char* uplink_ip = NULL,*downlink_ip = NULL;*/
	unsigned long uplink_ip = 0,downlink_ip = 0;
	unsigned int  uplink_mask = 0,downlink_mask = 0;
	
	uplink_mask=(unsigned int)upmask;
	downlink_mask=(unsigned int)downmask;

	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == uplink_ifname){
       //return -2;
	   return -2;
	}	
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,upifname,strlen(upifname));

	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       //return -2;
       return -2;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,downifname,strlen(downifname));
    op_ret = ccgi_vrrp_check_ip_format((char*)upip,&split);
    if(0 == op_ret){
	   if(0 == split){
		   uplink_ip = (unsigned long)inet_addr((char*)upip);
		   uplink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&upip,&uplink_ip,&uplink_mask); 
		   if(op_ret==-1){
			   free(uplink_ifname);
			   free(downlink_ifname);
			   //return -1;
			   return -3;
		   }
	   }
	}
	else{
	   free(uplink_ifname);
	   free(downlink_ifname);
	   //return -1;	
	   return -1;
	}
    op_ret = ccgi_vrrp_check_ip_format((char*)downip,&split);
    if(0 == op_ret){
	   if(0 == split){
		   downlink_ip = (unsigned long)inet_addr((char*)downip);
		   downlink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&downip,&downlink_ip,&downlink_mask); 
		   if(op_ret==-1){
			   free(uplink_ifname);
			   free(downlink_ifname);
			   //return -1;
			   return -1;
		   }
	   }
	}
	else{
	   free(uplink_ifname);
	   free(downlink_ifname);
	   return -1;		
	}
	priority = strtoul((char *)prio,NULL,10);
	if((priority < 1)||priority > 255){
        //vty_out(vty,"%% Bad parameter : %s !",argv[4]);
		//return -2;
		free(uplink_ifname);
		free(downlink_ifname);
		return -4;
	}

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/

	profile=(unsigned int)strtoul(provrrp,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&uplink_ifname,
							 DBUS_TYPE_UINT32,&uplink_ip,
							 DBUS_TYPE_UINT32,&uplink_mask,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,
							 DBUS_TYPE_UINT32,&downlink_mask,						 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	free(uplink_ifname);
	free(downlink_ifname);
	dbus_message_unref(reply);
	return retu;
}
int ccgi_downanduplink_ifname_mask_web(char *provrrp,char *upifname,char *upip,char *downifname,char *downip,char *prio,int upmask,int downmask,DBusConnection *connection)
                                                  /*返回0表示成功，返回-1表示error，返回-2表示ifname为空,返回-3表示ip为空,返回-4表示优先值超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	int retu=0;
	unsigned int profile = 0,priority = 0;
	int split = 0;
	char* uplink_ifname = NULL,*downlink_ifname = NULL;
	unsigned long uplink_ip = 0,downlink_ip = 0;
	unsigned int  uplink_mask = 0,downlink_mask = 0;
	
	uplink_mask=(unsigned int)upmask;
	downlink_mask=(unsigned int)downmask;

	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == uplink_ifname){
	   return -2;
	}	
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,upifname,strlen(upifname));

	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       return -2;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,downifname,strlen(downifname));
    op_ret = ccgi_vrrp_check_ip_format((char*)upip,&split);
    if(0 == op_ret){
	   if(0 == split){
		   uplink_ip = (unsigned long)inet_addr((char*)upip);
		   uplink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&upip,&uplink_ip,&uplink_mask); 
		   if(op_ret==-1){
			   free(uplink_ifname);
			   free(downlink_ifname);
			   return -3;
		   }
	   }
	}
	else{
	   free(uplink_ifname);
	   free(downlink_ifname);
	   return -1;
	}
    op_ret = ccgi_vrrp_check_ip_format((char*)downip,&split);
    if(0 == op_ret){
	   if(0 == split){
		   downlink_ip = (unsigned long)inet_addr((char*)downip);
		   downlink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&downip,&downlink_ip,&downlink_mask); 
		   if(op_ret==-1){
			   free(uplink_ifname);
			   free(downlink_ifname);
			   //return -1;
			   return -1;
		   }
	   }
	}
	else{
	   free(uplink_ifname);
	   free(downlink_ifname);
	   return -1;		
	}
	priority = strtoul((char *)prio,NULL,10);
	if((priority < 1)||priority > 255){
		free(uplink_ifname);
	    free(downlink_ifname);
		return -4;
	}
	profile=(unsigned int)strtoul(provrrp,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&uplink_ifname,
							 DBUS_TYPE_UINT32,&uplink_ip,
							 DBUS_TYPE_UINT32,&uplink_mask,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,
							 DBUS_TYPE_UINT32,&downlink_mask,						 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	free(uplink_ifname);
	free(downlink_ifname);
	dbus_message_unref(reply);
	return retu;
}
int config_vrrp_uplink(char *provrrp,char *upifname,char *upip,char *prio,int upmask)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//unsigned int vrid = 0;
	unsigned int priority = 0;
	int retu=0;
	//int add = 1;
	int split = 0;
	char *uplink_ifname = NULL;
	unsigned long uplink_ip = 0;
	
	unsigned int uplink_mask = 0;	
	uplink_mask = (unsigned int)upmask;
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == uplink_ifname) {
		//return -2;
		return -2;
	}
	memset(uplink_ifname, 0, MAX_IFNAME_LEN);
	memcpy(uplink_ifname, upifname, strlen(upifname));
	
	op_ret = ccgi_vrrp_check_ip_format((char*)upip, &split);
	if (0 == op_ret) {
		if (0 == split) {
			/* mask default is 32 */
			uplink_ip = (unsigned long)inet_addr((char*)upip);
			uplink_mask = 32;
		}
		else if (1 == split) {
			op_ret = ip_address_format2ulong((char**)&upip, &uplink_ip, &uplink_mask); 
			if (0 != op_ret) {
				free(uplink_ifname);
				//vty_out(vty, "%% Bad parameter: %s !", argv[1]);
				//return -2;
				return -3;
			}
		}
	}
	else {
		free(uplink_ifname);
		//vty_out(vty, "%%Illegal IP address %s!", argv[1]);
		//return -2;	
		return -4;
	}

	priority = strtoul((char *)prio,NULL,10);
	if (priority < 1 ||
		priority > 255) {
		//vty_out(vty, "%%error priority %s, valid range [1-255]!", argv[2]);
		//return -2;
		free(uplink_ifname);
		return -5;
	}

	/*if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(provrrp,0,10);

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP_UPLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &priority,
							 DBUS_TYPE_STRING, &uplink_ifname,
							 DBUS_TYPE_UINT32, &uplink_ip,
							 DBUS_TYPE_UINT32, &uplink_mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{	
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            //vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu=op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	/*release malloc mem*/
	free(uplink_ifname);
	dbus_message_unref(reply);
	return retu;
}
int config_vrrp_uplink_web(char *provrrp,char *upifname,char *upip,char *prio,int upmask,DBusConnection *connection)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	unsigned int profile = 0;
	unsigned int priority = 0;
	int retu=0;
	int split = 0;
	char *uplink_ifname = NULL;
	unsigned long uplink_ip = 0;
	
	unsigned int uplink_mask = 0;	
	uplink_mask = (unsigned int)upmask;
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == uplink_ifname) {
		return -2;
	}
	memset(uplink_ifname, 0, MAX_IFNAME_LEN);
	memcpy(uplink_ifname, upifname, strlen(upifname));
	
	op_ret = ccgi_vrrp_check_ip_format((char*)upip, &split);
	if (0 == op_ret) {
		if (0 == split) {
			/* mask default is 32 */
			uplink_ip = (unsigned long)inet_addr((char*)upip);
			uplink_mask = 32;
		}
		else if (1 == split) {
			op_ret = ip_address_format2ulong((char**)&upip, &uplink_ip, &uplink_mask); 
			if (0 != op_ret) {
				free(uplink_ifname);
				return -3;
			}
		}
	}
	else {
		free(uplink_ifname);
		return -4;
	}

	priority = strtoul((char *)prio,NULL,10);
	if (priority < 1 ||
		priority > 255) {
		if(uplink_ifname){
			free(uplink_ifname);
			uplink_ifname=NULL;
		}
		return -5;
	}

	profile = (unsigned int)strtoul(provrrp,0,10);

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP_UPLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &priority,
							 DBUS_TYPE_STRING, &uplink_ifname,
							 DBUS_TYPE_UINT32, &uplink_ip,
							 DBUS_TYPE_UINT32, &uplink_mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{	
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            retu=op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	/*release malloc mem*/
	free(uplink_ifname);
	dbus_message_unref(reply);
	return retu;
}

/*"config downlink IFNAME IP priority <1-255>"*/
int ccgi_config_downlink(char *profid,char * dlinkname,char *dlinkip,char *dlinkprio)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int vrid = 0,priority = 0;
	unsigned int profile = 0;
	//int add = 1;
	int retu = 0;
	char* downlink_ifname = NULL;
    char*downlink_ip = NULL;

	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       //return -2;
       return -1;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,dlinkname,strlen(dlinkname));
	
	downlink_ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == downlink_ip){
       //return -2;
       return -1;
	}
	memset(downlink_ip,0,MAX_IPADDR_LEN);
	memcpy(downlink_ip,dlinkip,strlen(dlinkip));

	priority = strtoul((char *)dlinkprio,NULL,10);
	if((priority < 1)||priority > 255){
        //vty_out(vty,"%% Bad parameter : %s !",dlinkprio);
		//return -2;
		free(downlink_ifname);
		free(downlink_ip);
		return -1;
	}

    /*
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		vrid = profile;
	}
	*/
	profile = (unsigned int)strtoul(profid,0,10);
	query = dbus_message_new_method_call(VRRP_DBUS_BUSNAME,
										 VRRP_DBUS_OBJPATH,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP_DOWNLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&vrid,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_STRING,&downlink_ip,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu  = op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		
		//dcli_vrrp_notify_to_npd(vty,NULL,downlink_ifname,add);
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu  =-1;
	}
	/*release malloc mem*/
	free(downlink_ifname);
	free(downlink_ip);
	dbus_message_unref(reply);
	//return 0;
	return retu;
}

int config_vrrp_downlink_mask(char *proid,char *downifname,char *downip,char *prio,int downmask)/*返回0表示成功，返回-1表示error，返回-2表示接口名为空，返回-3表示优先值超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	int retu=0;
	unsigned int profile = 0,priority = 0;
	//unsigned int vrid = 0;
	//int add = 1;
	int split = 0;
	char* downlink_ifname = NULL;
    unsigned  long downlink_ip = 0;
	
	unsigned int downlink_mask = 0;	
    downlink_mask = (unsigned int)downmask;
	
	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       //return -2;
       return -2;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,downifname,strlen(downifname));
	
    op_ret = ccgi_vrrp_check_ip_format((char*)downip,&split);
    if(0 == op_ret){
	   if(0 == split){
		   downlink_ip = (unsigned long)inet_addr((char*)downip);
		   downlink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&downip,&downlink_ip,&downlink_mask); 
		   if(op_ret==-1){
			   free(downlink_ifname);
			   //return -1;
			   return -1;
		   }
	   }
	}
	else{
	   free(downlink_ifname);
	   //return -1;	
	   return -1;
	}


	priority = strtoul((char *)prio,NULL,10);
	if((priority < 1)||priority > 255){
        //vty_out(vty,"%% Bad parameter : %s !",argv[2]);
		//return -2;
		free(downlink_ifname);
		return -3;
	}

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/

	profile = (unsigned int)strtoul(proid,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP_DOWNLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,
							 DBUS_TYPE_UINT32,&downlink_mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
           // vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
          retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		
		/*dcli_vrrp_notify_to_npd(vty,NULL,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	/*release malloc mem*/
	free(downlink_ifname);
	dbus_message_unref(reply);
	//return 0;
	return retu;
}
int config_vrrp_downlink_mask_web(char *proid,char *downifname,char *downip,char *prio,int downmask,DBusConnection *connection)/*返回0表示成功，返回-1表示error，返回-2表示接口名为空，返回-3表示优先值超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	int retu=0;
	unsigned int profile = 0,priority = 0;
	int split = 0;
	char* downlink_ifname = NULL;
    unsigned  long downlink_ip = 0;
	
	unsigned int downlink_mask = 0;	
    downlink_mask = (unsigned int)downmask;
	
	downlink_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == downlink_ifname){
       return -2;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,downifname,strlen(downifname));
	
    op_ret = ccgi_vrrp_check_ip_format((char*)downip,&split);
    if(0 == op_ret){
	   if(0 == split){
		   downlink_ip = (unsigned long)inet_addr((char*)downip);
		   downlink_mask = 32;
	   }
	   else if(1 == split){
		   op_ret = ip_address_format2ulong((char**)&downip,&downlink_ip,&downlink_mask); 
		   if(op_ret==-1){
			   free(downlink_ifname);
			   return -1;
		   }
	   }
	}
	else{
	   free(downlink_ifname);
	   return -1;
	}


	priority = strtoul((char *)prio,NULL,10);
	if((priority < 1)||priority > 255){
		if(downlink_ifname){
		free(downlink_ifname);
		downlink_ifname=NULL;
		}
		return -3;
	}


	profile = (unsigned int)strtoul(proid,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_VRRP_DOWNLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&priority,
							 DBUS_TYPE_STRING,&downlink_ifname,
							 DBUS_TYPE_UINT32,&downlink_ip,
							 DBUS_TYPE_UINT32,&downlink_mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
          retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	/*release malloc mem*/
	free(downlink_ifname);
	dbus_message_unref(reply);
	return retu;
}
int config_vrrp_link_add_vip(char * input_type, char *profid,char *linktype,char *ifnamez,char *ipz)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

    int retu=0;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//int add = 1;
	int split = 0;
	unsigned opt_type = DCLI_VRRP_VIP_OPT_TYPE_INVALID;
	unsigned link_type = DCLI_VRRP_LINK_TYPE_INVALID;
	char *ifname = NULL;
	unsigned long virtual_ip = 0;
	unsigned int mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(input_type, "add", strlen(input_type))) {
		opt_type = DCLI_VRRP_VIP_OPT_TYPE_ADD;
	}else if (!strncmp(input_type, "delete", strlen(input_type))) {
		opt_type = DCLI_VRRP_VIP_OPT_TYPE_DEL;
	}else {
		return -1;
	}

	if (!strncmp(linktype, "uplink", strlen(linktype))) {
		link_type = DCLI_VRRP_LINK_TYPE_UPLINK;
	}else if (!strncmp(linktype, "downlink", strlen(linktype))) {
		link_type = DCLI_VRRP_LINK_TYPE_DOWNLINK;
	}
	else if(!strncmp(linktype, "vgateway", strlen(linktype))) {
		link_type = DCLI_VRRP_LINK_TYPE_VGATEWAY;
	}else {
		//vty_out(vty, "%% Unknown link type %s!\n", argv[0]);
		//return -2;
		return -3;
	}

	ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if (NULL == ifname) {
		//return -2;
		return -2;
	}
	memset(ifname, 0, MAX_IFNAME_LEN);
	memcpy(ifname, ifnamez, strlen(ifnamez));
	
	op_ret = ccgi_vrrp_check_ip_format((char*)ipz, &split);
	if (0 == op_ret)
	{
		if (0 == split)
		{
			/* mask default is 32 */
			virtual_ip = (unsigned long)inet_addr((char*)ipz);
			mask = 32;
		}
		else if (1 == split)
		{
			op_ret = ip_address_format2ulong((char**)&ipz, &virtual_ip, &mask); 
			if (0 != op_ret) {
				free(ifname);
				//vty_out(vty, "%% Bad parameter: %s !", argv[2]);
				//return -2;
				return -4;
			}
		}
	}
	else {
		free(ifname);
		//vty_out(vty, "%%Illegal IP address %s!", argv[2]);
		//return -2;		
		return -5;
	}


	/*if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}*/

	profile = (unsigned int)strtoul(profid,0,10);

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_VIP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &opt_type,		                     
							 DBUS_TYPE_UINT32, &link_type,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &virtual_ip,
							 DBUS_TYPE_UINT32, &mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
           // vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
           retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			//printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	
	/*release malloc mem*/
	free(ifname);
	dbus_message_unref(reply);
	return retu;
}
int config_vrrp_link_add_vip_web(char * input_type, char *profid,char *linktype,char *ifname,char *ipz,DBusConnection *connection)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};

    int retu=0;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//int add = 1;
	int split = 0;
	unsigned opt_type = DCLI_VRRP_VIP_OPT_TYPE_INVALID;
	unsigned link_type = DCLI_VRRP_LINK_TYPE_INVALID;
	unsigned long virtual_ip = 0;
	unsigned int mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(input_type, "add", strlen(input_type))) {
		opt_type = DCLI_VRRP_VIP_OPT_TYPE_ADD;
	}else if (!strncmp(input_type, "delete", strlen(input_type))) {
		opt_type = DCLI_VRRP_VIP_OPT_TYPE_DEL;
	}else {
		return -1;
	}

	if (!strncmp(linktype, "uplink", strlen(linktype))) {
		link_type = DCLI_VRRP_LINK_TYPE_UPLINK;
	}else if (!strncmp(linktype, "downlink", strlen(linktype))) {
		link_type = DCLI_VRRP_LINK_TYPE_DOWNLINK;
	}
	else if(!strncmp(linktype, "vgateway", strlen(linktype))) {
		link_type = DCLI_VRRP_LINK_TYPE_VGATEWAY;
	}else {
		return -3;
	}

	op_ret = ccgi_vrrp_check_ip_format((char*)ipz, &split);
	if (0 == op_ret)
	{
		if (0 == split)
		{
			/* mask default is 32 */
			virtual_ip = (unsigned long)inet_addr((char*)ipz);
			mask = 32;
		}
		else if (1 == split)
		{
			op_ret = ip_address_format2ulong((char**)&ipz, &virtual_ip, &mask); 
			if (0 != op_ret) {
				return -4;
			}
		}
	}
	else {
		return -5;
	}

	profile = (unsigned int)strtoul(profid,0,10);

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_VIP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_UINT32, &opt_type,		                     
							 DBUS_TYPE_UINT32, &link_type,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_UINT32, &virtual_ip,
							 DBUS_TYPE_UINT32, &mask,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
		   retu =  (op_ret - DCLI_VRRP_RETURN_CODE_OK);
		   return retu;
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
	}
	
	/*release malloc mem*/
	dbus_message_unref(reply);
	return 33;
}


int config_vrrp_start_state(char *profid,char *statez)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0,enable = 0;
	unsigned int profile = 0;

	int retu=0;

	if(!strcmp("yes",(char *)statez)){
        enable = 1;
	}
	else if(!strcmp("no",(char *)statez)){
        enable = 0;
	}

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_WANT_STATE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,	
		                     DBUS_TYPE_UINT32,&enable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

int config_vrrp_service(char *profid,char *ablez)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	int retu = 0;
	unsigned int profile = 0;
	//unsigned int priority = 0,vrid = 0;
	int enable = -1;

	if(!strcmp("enable",(char *)ablez)){
        enable = 1;
	}
	else if(!strcmp("disable",(char *)ablez)){
        enable = 0;
	}

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&enable,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu = op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	return retu;
}
int config_vrrp_service_web(char *profid,char *ablez,DBusConnection *connection)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	int retu = 0;
	unsigned int profile = 0;
	//unsigned int priority = 0,vrid = 0;
	int enable = -1;

	if(!strcmp("enable",(char *)ablez)){
        enable = 1;
	}
	else if(!strcmp("disable",(char *)ablez)){
        enable = 0;
	}

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&enable,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu = op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	return retu;
}

int  config_vrrp_gateway(char *profid,char *gwifname,char *gwip)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	int retu = 0;
	unsigned int profile = 0;
	//unsigned int vrid = 0,priority = 0;
	//int add = 1;
	char* vgateway_ifname = NULL;
   // char* vgateway_ip = NULL;
    unsigned long dipno = 0;
	unsigned int dipmaskLen = 0;
	vgateway_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == vgateway_ifname){
       return -2;
	}
	memset(vgateway_ifname,0,MAX_IFNAME_LEN);
	memcpy(vgateway_ifname,gwifname,strlen(gwifname));
	
	op_ret = ip_address_format2ulong((char**)&gwip,&dipno,&dipmaskLen);	
	if(op_ret==-1)  return -1;

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_V_GATEWAY);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_STRING,&vgateway_ifname,
							 DBUS_TYPE_UINT32,&dipno,	
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			  // vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			  retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	/*release malloc mem*/
	free(vgateway_ifname);
	dbus_message_unref(reply);
	//return 0;
	return retu;
}

int  cancel_vrrp_gateway(char *profid,char *gwifname,char *gwip)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//unsigned int vrid = 0,priority = 0;
	//int add = 1;
	int retu = 0;
	char* vgateway_ifname = NULL;
    //char* vgateway_ip = NULL;
    unsigned long dipno = 0;
	unsigned int dipmaskLen = 0;
	vgateway_ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == vgateway_ifname){
       return -2;
	}
	memset(vgateway_ifname,0,MAX_IFNAME_LEN);
	memcpy(vgateway_ifname,gwifname,strlen(gwifname));

	op_ret = ip_address_format2ulong((char**)&gwip,&dipno,&dipmaskLen);	
	if(op_ret==-1)  
		//return -1;
		return -1;

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/

	profile = (unsigned int)strtoul(profid,0,10);

	/*vty_out(vty,"vgateway ifname %s,ip %d.%d.%d.%d,masklen %d\n",vgateway_ifname,\
		(dipno & 0xff000000)>>24,(dipno & 0xff0000)>>16,(dipno & 0xff00)>>8,dipno & 0xff,dipmaskLen);*/

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_NO_V_GATEWAY);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_STRING,&vgateway_ifname,
							 DBUS_TYPE_UINT32,&dipno,	
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		
		/*dcli_vrrp_notify_to_npd(vty,NULL,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	/*release malloc mem*/
	free(vgateway_ifname);
	dbus_message_unref(reply);
	return retu;
}

int config_vrrp_preempt(char *profid,char *statez)/*返回0表示成功，返回-1表示失败，*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0,preempt = 0;
	//unsigned int vrid = 0,state = 0;
	int retu=0;

	if(!strcmp("yes",(char *)statez)){
        preempt = 1;
	}
	else if(!strcmp("no",(char *)statez)){
        preempt = 0;
	}

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_PREEMPT_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,	
							 DBUS_TYPE_UINT32,&preempt,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}
int config_vrrp_preempt_web(char *statez,int profid,DBusConnection *connection)/*返回0表示成功，返回-1表示失败，*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0,preempt = 0;
	//unsigned int vrid = 0,state = 0;
	int retu=0;

	if(!strcmp("yes",(char *)statez)){
        preempt = 1;
	}
	else if(!strcmp("no",(char *)statez)){
        preempt = 0;
	}

	profile = (unsigned int)strtoul(profid,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_PREEMPT_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,	
							 DBUS_TYPE_UINT32,&preempt,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}
int cancel_vrrp_transfer(char *profid)/*返回0表示成功，返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//unsigned int vrid = 0,priority = 0;
	int retu = 0;

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_NO_TRANSFER);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
           ;		
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	//return 0;
	return retu;
}

int config_vrrp_max_down_count(char *maxcount)/*返回0是成功，返回-1是失败，返回-2是输入参数超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	int retu= 0 ;
	unsigned int profile = 0;
	//unsigned int vrid = 0,mac = 0;
	int count = 0;
	count = strtoul((char *)maxcount,NULL,10);
	if((count < 1)||count > 255){
        //vty_out(vty,"%% Bad parameter : %s !",argv[0]);
	//	return -2;
		return -2;
	}

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_MS_DOWN_PACKT_COUNT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&count,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return 0;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu =-1;
	}
	dbus_message_unref(reply);
	//return 0;
	return retu;
}

int config_vrrp_multi_link_detect(char *profid,char *statez)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	int detect = 0;
	int retu =0;
	if(!strcmp("on",(char *)statez)){
        detect = 1;
	}
	else if(!strcmp("off",(char *)statez)){
        detect = 0;
	}
	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_MULTI_LINK_DETECT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,	
							 DBUS_TYPE_UINT32,&detect,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return 0;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	//return 0;
	return retu;
}
int config_vrrp_multi_link_detect_web(char *statez,char *profid,DBusConnection *connection)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	int detect = 0;
	int retu =0;
	if(!strcmp("on",(char *)statez)){
        detect = 1;
	}
	else if(!strcmp("off",(char *)statez)){
        detect = 0;
	}
	profile = (unsigned int)strtoul(profid,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_MULTI_LINK_DETECT);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,	
							 DBUS_TYPE_UINT32,&detect,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		else 
			retu=0;
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	return retu;
}

int config_vrrp_set_vgateway_transform(char *profid,char *statez)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

    int retu=0;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	unsigned int vgateway_tf_flg = DCLI_VRRP_VGATEWAY_TF_FLG_OFF;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(statez, "on", strlen(statez))) {
		vgateway_tf_flg = DCLI_VRRP_VGATEWAY_TF_FLG_ON;
	}else if (!strncmp(statez, "off", strlen(statez))) {
		vgateway_tf_flg = DCLI_VRRP_VGATEWAY_TF_FLG_OFF;
	}else {
		//vty_out(vty, "%% Unknown command format!\n");
		//return -2;
		return -2;
	}

	/*if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SET_VGATEWAY_TRANSFORM_FLG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_UINT32, &vgateway_tf_flg,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		//return 0;
		retu = -1;
	}
	
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            //vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
           retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	}
	else {
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	//return 0;
	return retu;
}

int config_vrrp_notify(char *profid,char *typez,char *statez)/*返回0表示成功，返回-1表示失败，返回-2Unknown command format*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int op_ret = 0;
	int retu=0;
	unsigned int profile = 0;
	unsigned int notify_obj = DCLI_VRRP_NOTIFY_OBJ_TYPE_INVALID;
	unsigned char notify_flg = DCLI_VRRP_NOTIFY_OFF;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	if (!strncmp(typez, "wireless-control", strlen(typez))) {
		notify_obj = DCLI_VRRP_NOTIFY_OBJ_TYPE_WID;
	}else if (!strncmp(typez, "easy-access-gateway", strlen(typez))) {
		notify_obj = DCLI_VRRP_NOTIFY_OBJ_TYPE_PORTAL;
	}else {
		//vty_out(vty, "%% Unknown notification object!\n");
		return -2;
	}

	if (!strncmp(statez, "on", strlen(statez))) {
		notify_flg = DCLI_VRRP_NOTIFY_ON;
	}else if (!strncmp(statez, "off", strlen(statez))) {
		notify_flg = DCLI_VRRP_NOTIFY_OFF;
	}else {
		//vty_out(vty, "%% Unknown command format!\n");
		//return -2;
		return -2;
	}

	/*if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SET_NOTIFY_OBJ_AND_FLG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_BYTE, &notify_obj,
							 DBUS_TYPE_BYTE, &notify_flg,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		//return 0;
		retu = -1;
	}
	
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
           // vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
           retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	}
	else {
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}

	dbus_message_unref(reply);
	//return 0;
	return retu;
}




//"no hansi"
int ccgi_nohansi(char *provrrp)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//char *uplink_ifname = NULL,*downlink_ifname = NULL;
	//int add = 0;
	int retu =0;
	int tempvrrp;

    /*
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		vrid = profile;
	}
	*/
	tempvrrp=strtoul(provrrp,0,10);
	profile = (unsigned int)tempvrrp;
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_END_VRRP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return 0;
		return -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	}
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

//要处理的函数
int ccgi_vrrp_get_if_by_profile
(
    int profile,
    char* ifname1,
    char* ifname2
)
{
    DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	char* uplink_ifname = NULL;
	char* downlink_ifname = NULL;
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										vrrp_obj_path,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_GET_IFNAME);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&profile,
									DBUS_TYPE_INVALID);
 	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		//vty_out(vty,"failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return 0;
		return -1;
	}
	if (dbus_message_get_args ( reply, &err,
		                            DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_STRING,&uplink_ifname,
									DBUS_TYPE_STRING,&downlink_ifname,
									DBUS_TYPE_INVALID)) {
         //vty_out(vty,"get hansi %d uplinkifname %s,downlinkifname %s\n",profile,uplink_ifname,downlink_ifname);
		 memcpy(ifname1,uplink_ifname,strlen(uplink_ifname));
		 memcpy(ifname2,downlink_ifname,strlen(downlink_ifname));
	}
	else {		
		//vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
	}
		
	dbus_message_unref(reply);
	return ret;  
}

//函数做处理
void ccgi_vrrp_notify_to_npd
(
    char* ifname1,
    char* ifname2,
    int   add
)
{
	DBusMessage *query, *reply;
	DBusError err;
    unsigned int op_ret;
	if(NULL == ifname1)
	{
       ifname1 = ifname2;
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_OBJPATH,			\
								NPD_DBUS_INTERFACE,					\
								NPD_DBUS_FDB_METHOD_CREATE_VRRP_BY_IFNAME);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&ifname1,
							 DBUS_TYPE_STRING,&ifname2,
							 DBUS_TYPE_UINT32,&add,
							 DBUS_TYPE_INVALID);
    
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed to get reply!\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
			//vty_out(vty,"Failed to get reply!\n");
		}
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
                //vty_out(vty,"get return value %d\n",op_ret);
			} 
			else {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
					//vty_out(vty,"failed to get return value!\n");
				}
			}
	}
	dbus_message_unref(reply);
	return ;

}


//"config hansi priority <1-255>"
int ccgi_config_hansi_priority(char *provrrp,char * prio_num)/*返回0表示成功，返回-1表示失败,返回-3表示优先值超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int vrid = 0,priority = 0;
	//unsigned int preempt = 0;
	int retu = 0;
	unsigned int profile = 0;

	priority = strtoul((char *)prio_num,NULL,10);
	if((priority < 1)||priority > 255){
        //vty_out(vty,"%% Bad parameter : %s !",argv[0]);        
		return -3;
	}

	//hansi_node 
	/*
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		vrid = profile;
	}
	*/
	
	int tempvrrp;
	tempvrrp=strtoul(provrrp,0,10);
	vrid = (unsigned int)tempvrrp;
	profile = vrid;
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_PROFILE_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&vrid,
							 DBUS_TYPE_UINT32,&priority,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
           // vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
           //return (op_ret - DCLI_VRRP_RETURN_CODE_OK);
           retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	return retu;
}
int ccgi_config_hansi_priority_web(char *provrrp,char * prio_num,DBusConnection *connection)/*返回0表示成功，返回-1表示失败,返回-3表示优先值超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int vrid = 0,priority = 0;
	//unsigned int preempt = 0;
	int retu = 0;
	unsigned int profile = 0;

	priority = strtoul((char *)prio_num,NULL,10);
	if((priority < 1)||priority > 255){
        //vty_out(vty,"%% Bad parameter : %s !",argv[0]);        
		return -3;
	}

	//hansi_node 
	/*
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		vrid = profile;
	}
	*/
	
	int tempvrrp;
	tempvrrp=strtoul(provrrp,0,10);
	vrid = (unsigned int)tempvrrp;
	profile = vrid;
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_PROFILE_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&vrid,
							 DBUS_TYPE_UINT32,&priority,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
           // vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
           //return (op_ret - DCLI_VRRP_RETURN_CODE_OK);
           retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	return retu;
}

//"config hansi advertime TIME"
int ccgi_config_hansi_advertime(char *provrrp,char *adtime)/*返回0表示成功，返回-1表示失败，返回-2表示参数超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int  vrid = 0,advert = 0;
	int retu = 0;
	unsigned int profile=0;
	
	advert = strtoul((char *)adtime,NULL,10);
	if((advert < 1)||advert > 255){
        //vty_out(vty,"%% Bad parameter : %s !",argv[0]);     
		return -2;
	}

    //hansi_node 
    /*
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		vrid = profile;
	}
	*/
	int tempvrrp;
	tempvrrp=strtoul(provrrp,0,10);
	vrid = (unsigned int)tempvrrp;
	profile = vrid;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_ADVERT_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&vrid,
							 DBUS_TYPE_UINT32,&advert,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return 0;
		return -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            //return (op_ret - DCLI_VRRP_RETURN_CODE_OK);
			retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}
int ccgi_config_hansi_advertime_web(char *provrrp,char *adtime,DBusConnection *connection)/*返回0表示成功，返回-1表示失败，返回-2表示参数超出范围*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int  vrid = 0,advert = 0;
	int retu = 0;
	unsigned int profile=0;
	
	advert = strtoul((char *)adtime,NULL,10);
	if((advert < 1)||advert > ADVERTIME){
		return -2;
	}

	int tempvrrp;
	tempvrrp=strtoul(provrrp,0,10);
	vrid = (unsigned int)tempvrrp;
	profile = vrid;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_ADVERT_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&vrid,
							 DBUS_TYPE_UINT32,&advert,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
		else
		{
			retu = 0;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

//"config hansi virtual mac (yes|no)"
int ccgi_config_hansi_virtualmac(char *provrrp,char *macstates)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int vrid = 0,mac = 0;
	unsigned int profile = 0;
	int retu = 0;

	if(!strcmp("yes",(char *)macstates)){
       mac = 0;
	}
	else if(!strcmp("no",(char *)macstates)){
        mac = 1;
	}

	//hansi node
	/*
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
		vrid = profile;
	}
	*/
	int tempvrrp;
	tempvrrp=strtoul(provrrp,0,10);
	vrid = (unsigned int)tempvrrp;	
	profile = vrid;
	
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VIRTUAL_MAC_VALUE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&vrid,
							 DBUS_TYPE_UINT32,&mac,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return 0;
		return -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
        if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
            //vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            //return (op_ret - DCLI_VRRP_RETURN_CODE_OK);
            retu =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}


//"show hansi [<1-16>]" 单个不循环用结构体
int ccgi_show_hansi(char *hnum,Z_VRRP *zvrrp)
{

    ccgi_dbus_init();	
	unsigned int op_ret = 0;
	int profile = 0;

	//hansi node
	/*
	if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}
	*/
	//if(1 == argc){
		profile = strtoul((char *)hnum,NULL,10);
		if((profile < 1)||profile > 16){
			//vty_out(vty,"%% Bad parameter : %s !",argv[0]);
			return -3;
		}
	//}
	/*参数个数一定为1，不出现0的情况
	//hansi node
	if((0 == argc)&&(HANSI_NODE != vty->node)){
        for(profile = 1;profile < 17;profile++){
		   //另外有函数
           op_ret += ccgi_show_hansi_profile(&zvrrp,profile);
		}
		if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST*16 ==op_ret ){
			//vty_out(vty,"hansi profile not exist!\n");
			return -3;
		}
		return 0;
	}
	*/
	
	op_ret = ccgi_show_hansi_profile(zvrrp,profile);
	if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
	   //vty_out(vty,"hansi profile %d not exist!\n",profile);
	   return -3;
	}
	return 0;
}


#if  0
int ccgi_show_hansi_profile_detail(Z_VRRP *zvrrp,int profile)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	unsigned int op_ret = 0;
	unsigned int advert = 0,preempt = 0,priority = 0,state = 0,vir_mac = 0;
	char *uplink_ifname = NULL,*downlink_ifname =NULL,*heartbeatlink_ifname = NULL,*vgateway_ifname = NULL;
    char *action = NULL,*step = NULL;
	int uplink_ip = 0,downlink_ip = 0,heartbeatlink_ip = 0,ip = 0;
	int wid_transfer = 0,portal_enable = 0,portal_transfer = 0;
    int if_vgateway = 0,vgateway_ip = 0,vgateway_mask = 0;
    unsigned char link = 0;
	unsigned int  heartbeat_link = 0,vgateway_link = 0;
	unsigned int  log = 0;
	unsigned int uplink_flag = 0;
	unsigned int downlink_flag = 0;
	unsigned int heartbeat_flag = 0;
	int vgateway_naddr =0;
	int i = 0;
	int uplink_naddr = 0;
	int downlink_naddr = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	int instRun = 0;

   	char link_ip[64];
	memset(link_ip,0,64);

	instRun = ccgi_vrrp_hansi_is_running(profile);
	if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		return DCLI_VRRP_RETURN_CODE_ERR;
	}

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW_DETAIL);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,				 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);		
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
	   return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	//vty_out(vty,"HANSI %d detail info:\n",profile);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&log);
	//vty_out(vty,"STATE: %s\n",(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
    strcpy(zvrrp->state,(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
	if(log){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&action);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&step);
        //vty_out(vty,"	action:	%s\n",action);
		//vty_out(vty,"	detal : %s\n",step);
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority);
	//vty_out(vty,"PRIORITY:%d\n",priority);
	zvrrp->priority = priority;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&advert);
	//vty_out(vty,"ADVERTISEMENT TIMER:%d(s)\n",advert);
	zvrrp->advert = advert;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&preempt);
	//vty_out(vty,"PREEMPT:%s\n",preempt ? "Yes" : "No");
	strcpy(zvrrp->preempt,preempt ? "Yes" : "No");
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vir_mac);
	//vty_out(vty,"VIRTUAL MAC:%s\n",vir_mac ? "No" : "Yes");	
    strcpy(zvrrp->macstate,vir_mac ? "No" : "Yes");	
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&uplink_flag);
	if(1 == uplink_flag){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &uplink_naddr);
		for (i = 0; i < uplink_naddr; i++)
		{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&uplink_ifname);	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&link);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&ip);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&uplink_ip);		
		//vty_out(vty,"UPLINK INFO :\n");
		//vty_out(vty,"		UPLINK IFNAM: %s, %s\n",uplink_ifname,link ? "UP" : "DOWN");	
		/*vty_out(vty,"		REAL IP:%d.%d.%d.%d \n",((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
					((ip& 0xff00) >> 8),(ip & 0xff));*/
		/*vty_out(vty,"		VIRTUAL IP:%d.%d.%d.%d \n",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16),	\
					((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));*/

        strcpy(zvrrp->uplink_ifname,uplink_ifname);
		strcpy(zvrrp->upstate,link ? "UP" : "DOWN");	
		//real ip 
		memset(link_ip,0,64);		
		sprintf(link_ip,"%d.%d.%d.%d ",((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
					((ip& 0xff00) >> 8),(ip & 0xff));
	    strcpy(zvrrp->realipup,link_ip);
		zvrrp->urip1=((ip & 0xff000000) >> 24);
		zvrrp->urip2=((ip & 0xff0000) >> 16);
		zvrrp->urip3=((ip& 0xff00) >> 8);
		zvrrp->urip4=(ip & 0xff);
        //virtual ip
        memset(link_ip,0,64);	
        sprintf(link_ip,"%d.%d.%d.%d ",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16),	\
					((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));
	    strcpy(zvrrp->uplink_ip,link_ip);
		zvrrp->ulip1=((uplink_ip & 0xff000000) >> 24);
		zvrrp->ulip2=((uplink_ip & 0xff0000) >> 16);
		zvrrp->ulip3=((uplink_ip& 0xff00) >> 8);
		zvrrp->ulip4=(uplink_ip & 0xff);
		}
		
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&downlink_flag);
	if (1 == downlink_flag) {
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &downlink_naddr);

		for (i = 0; i < downlink_naddr; i++)
		{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&downlink_ifname);	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&link);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&ip);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&downlink_ip);		
		/*vty_out(vty,"DOWNLINK INFO :\n");
		vty_out(vty,"		DOWNLINK IFNAM: %s, %s\n",downlink_ifname,link ? "UP" : "DOWN");	
		vty_out(vty,"		REAL IP:%d.%d.%d.%d \n",((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
					((ip& 0xff00) >> 8),(ip & 0xff));	  
		vty_out(vty,"		VIRTUAL IP:%d.%d.%d.%d\n ",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
					((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff));	  */

        strcpy(zvrrp->downlink_ifname,downlink_ifname);
		strcpy(zvrrp->downstate,link ? "UP" : "DOWN");	
		//real ip
			memset(link_ip,0,64);	
			sprintf(link_ip,"%d.%d.%d.%d ",((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
							((ip& 0xff00) >> 8),(ip & 0xff));
			strcpy(zvrrp->realipdown,link_ip);
			zvrrp->drip1=((ip & 0xff000000) >> 24);
			zvrrp->drip2=((ip & 0xff0000) >> 16);
			zvrrp->drip3=((ip& 0xff00) >> 8);
			zvrrp->drip4=(ip & 0xff);

		//virtual ip
			memset(link_ip,0,64);	
			sprintf(link_ip,"%d.%d.%d.%d ",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
						((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff));
			strcpy(zvrrp->downlink_ip,link_ip);
			zvrrp->dlip1=((downlink_ip & 0xff000000) >> 24);
			zvrrp->dlip2=((downlink_ip & 0xff0000) >> 16);
			zvrrp->dlip3=((downlink_ip& 0xff00) >> 8);
			zvrrp->dlip4=(downlink_ip & 0xff);
		}

	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wid_transfer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_enable);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_transfer);
	dbus_message_iter_next(&iter);
    if(state == 6){
	   //vty_out(vty,"BATHSYN INFO:\n");
       //vty_out(vty,"		WID STATE: %s\n",wid_transfer ? "TRANSFERING" : "RUN");
       strcpy(zvrrp->widstate,wid_transfer ? "TRANSFERING" : "RUN");
	   if(portal_enable){
         // vty_out(vty,"		PORTAL STATE: %s\n",portal_transfer ? "TRANSFERING" : "RUN");
        strcpy(zvrrp->portalstate,portal_transfer ? "TRANSFERING" : "RUN");
	   }
	   else{
         // vty_out(vty,"		PORTAL STATE: NOT ELECTION \n");
         strcpy(zvrrp->portalstate, "NONE" );
	   }
	}		
	dbus_message_iter_get_basic(&iter,&heartbeatlink_ip);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeat_link);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&heartbeat_flag);
	if (1 == heartbeat_flag)
	{
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&heartbeatlink_ifname);
	//vty_out(vty,"HEART-BEAT-LINK INFO:\n");	
	if(0 != heartbeatlink_ip){
		/*vty_out(vty," 	IFNAME: %s(%s) IP:%d.%d.%d.%d\n",heartbeatlink_ifname, heartbeat_link ? "UP":"DOWN",((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
					((heartbeatlink_ip& 0xff00) >> 8),heartbeatlink_ip & 0xff);*/
       strcpy(zvrrp->hbinf,heartbeatlink_ifname);
	   strcpy(zvrrp->hbstate,heartbeat_link ? "UP":"DOWN");
	   //heartbeatlink ip
   		memset(link_ip,0,64);	
		sprintf(link_ip,"%d.%d.%d.%d ",((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
					((heartbeatlink_ip& 0xff00) >> 8),(heartbeatlink_ip & 0xff));
		strcpy(zvrrp->hbip,link_ip);
		zvrrp->hbip1=((heartbeatlink_ip & 0xff000000) >> 24);
		zvrrp->hbip2=((heartbeatlink_ip & 0xff0000) >> 16);
		zvrrp->hbip3=((heartbeatlink_ip& 0xff00) >> 8);
		zvrrp->hbip4=(heartbeatlink_ip & 0xff);

	}
	else{
        //vty_out(vty," 	NOT ELECTION\n");
	}
	}else{
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&if_vgateway);
	//vty_out(vty,"VGATEWAY INFO:\n");
	
	if(0 != if_vgateway){
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &vgateway_naddr);
		zvrrp->gw_number=vgateway_naddr;
		for(i = 0; i < vgateway_naddr; i++) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_ifname);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&ip);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_ip);			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&vgateway_link);			

			strcpy(zvrrp->gw[i].gwname,vgateway_ifname);
			strcpy(zvrrp->gw[i].gwstate,vgateway_link ? "UP":"DOWN");
			//gateway ip
	   		memset(link_ip,0,64);	
			sprintf(link_ip,"%d.%d.%d.%d ",((vgateway_ip & 0xff000000) >> 24),((vgateway_ip & 0xff0000) >> 16),	\
						((vgateway_ip& 0xff00) >> 8),(vgateway_ip & 0xff));
			strcpy(zvrrp->gw[i].gwip,link_ip);
			zvrrp->gw[i].gwip1=((vgateway_ip & 0xff000000) >> 24);
			zvrrp->gw[i].gwip2=((vgateway_ip & 0xff0000) >> 16);
			zvrrp->gw[i].gwip3=((vgateway_ip& 0xff00) >> 8);
			zvrrp->gw[i].gwip4=(vgateway_ip & 0xff);
		}
	}
	else{
	}

	dbus_message_unref(reply);
	return DCLI_VRRP_RETURN_CODE_OK;
}
#endif 


#if 1
int ccgi_show_hansi_profile_detail(Z_VRRP *zvrrp,int profile,int *upnum,int *downnum,int* vgatenum,char *upstate[],char *downstate[],char *uplink[],char *downlink[],char *vgate[],char* vuplinkip[],char *vdownlinkip[],char* ruplinkip[],char* rdownlinkip[],char *vgateip[])
{
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err = {0};
    DBusMessageIter  iter;
    unsigned int op_ret = 0;
    unsigned int advert = 0,preempt = 0,priority = 0,state = 0,vir_mac = 0;
    char *uplink_ifname = NULL,*downlink_ifname =NULL,*heartbeatlink_ifname = NULL,*vgateway_ifname = NULL;
    char *action = NULL,*step = NULL;
    int uplink_ip = 0,downlink_ip = 0,heartbeatlink_ip = 0,ip = 0;
    int wid_transfer = 0,portal_enable = 0,portal_transfer = 0;
    int if_vgateway = 0,vgateway_ip = 0,vgateway_mask = 0;
    unsigned char link = 0;
    unsigned int  heartbeat_link = 0,vgateway_link = 0;
    unsigned int  log = 0;
    unsigned int uplink_flag = 0;
    unsigned int downlink_flag = 0;
    unsigned int heartbeat_flag = 0;
    int vgateway_naddr =0;
    int i = 0;
    int uplink_naddr = 0;
    int downlink_naddr = 0;
    char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
    char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
    int instRun = 0;

    char link_ip[64];
    memset(link_ip,0,64);

    instRun = ccgi_vrrp_hansi_is_running(profile);
    if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
        return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
    }
    else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
        return DCLI_VRRP_RETURN_CODE_ERR;
    }

    ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
    ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
    
    query = dbus_message_new_method_call(vrrp_dbus_name,
                            			 vrrp_obj_path,
                                		 VRRP_DBUS_INTERFACE,
                                         VRRP_DBUS_METHOD_SHOW_DETAIL);
    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&profile,				 
                             DBUS_TYPE_INVALID);
    
    reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    dbus_message_unref(query);
    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
    	}
        dbus_message_unref(reply);		
        return DCLI_VRRP_RETURN_CODE_ERR;
    }
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&op_ret);
    if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
       return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
    }
    //vty_out(vty,"HANSI %d detail info:\n",profile);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&state);

    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&log);
    //vty_out(vty,"STATE: %s\n",(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
    strcpy(zvrrp->state,(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
    if(log){
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&action);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&step);
        //vty_out(vty," action: %s\n",action);
        //vty_out(vty," detal : %s\n",step);
    }
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&priority);
    //vty_out(vty,"PRIORITY:%d\n",priority);
    zvrrp->priority = priority;
    
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&advert);
    //vty_out(vty,"ADVERTISEMENT TIMER:%d(s)\n",advert);
    zvrrp->advert = advert;
    
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&preempt);
    //vty_out(vty,"PREEMPT:%s\n",preempt ? "Yes" : "No");
    strcpy(zvrrp->preempt,preempt ? "Yes" : "No");
    
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&vir_mac);
    //vty_out(vty,"VIRTUAL MAC:%s\n",vir_mac ? "No" : "Yes");	
    strcpy(zvrrp->macstate,vir_mac ? "No" : "Yes"); 
    
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&uplink_flag);
    if(1 == uplink_flag){
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &uplink_naddr);
		//kehao add 
        *upnum = uplink_naddr;
		////////////////////////////////////////////
        for (i = 0; i < uplink_naddr; i++)
    	{
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&uplink_ifname);	

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&link);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&ip);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&uplink_ip);		
        //vty_out(vty,"UPLINK INFO :\n");
        //vty_out(vty,"     UPLINK IFNAM: %s, %s\n",uplink_ifname,link ? "UP" : "DOWN");	
        /*vty_out(vty,"     REAL IP:%d.%d.%d.%d \n",((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
                    ((ip& 0xff00) >> 8),(ip & 0xff));*/
        /*vty_out(vty,"     VIRTUAL IP:%d.%d.%d.%d \n",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16),	\
                    ((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));*/

        //kehao modify 20110519
        //strcpy(zvrrp->uplink_ifname,uplink_ifname);
        strcpy(uplink[i],uplink_ifname);
		/////////////////////////////////////////////
		//kehao modify 20110520
        //strcpy(zvrrp->upstate,link ? "UP" : "DOWN");	
        strcpy(upstate[i],link ? "UP" : "DOWN"); //上行接口状态
        //real ip 
        memset(link_ip,0,64);	
        sprintf(link_ip,"%d.%d.%d.%d ",((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
                    ((ip& 0xff00) >> 8),(ip & 0xff));
		//kehao modify 20110520
        //strcpy(zvrrp->realipup,link_ip);//这是上行接口的实ip
        strcpy(ruplinkip[i],link_ip);
        /////////////////////////////////////////////////////
        zvrrp->urip1=((ip & 0xff000000) >> 24);
        zvrrp->urip2=((ip & 0xff0000) >> 16);
        zvrrp->urip3=((ip& 0xff00) >> 8);
        zvrrp->urip4=(ip & 0xff);
        //virtual ip
        memset(link_ip,0,64);   
        sprintf(link_ip,"%d.%d.%d.%d ",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16), \
                    ((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));
		//kehao modify 20110519
        //strcpy(zvrrp->uplink_ip,link_ip);
        strcpy(vuplinkip[i],link_ip);  //这是上行接口的虚ip
		////////////////////////////////
        zvrrp->ulip1=((uplink_ip & 0xff000000) >> 24);
        zvrrp->ulip2=((uplink_ip & 0xff0000) >> 16);
        zvrrp->ulip3=((uplink_ip& 0xff00) >> 8);
        zvrrp->ulip4=(uplink_ip & 0xff);
    	}
    	
    }
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&downlink_flag);
    if (1 == downlink_flag) {
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &downlink_naddr);
		//kehao add 20110519
        *downnum = downlink_naddr;
        ////////////////////////////
        for (i = 0; i < downlink_naddr; i++)
    	{
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&downlink_ifname);	

        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&link);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&ip);
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&downlink_ip);		
        /*vty_out(vty,"DOWNLINK INFO :\n");
        vty_out(vty,"       DOWNLINK IFNAM: %s, %s\n",downlink_ifname,link ? "UP" : "DOWN");	
        vty_out(vty,"       REAL IP:%d.%d.%d.%d \n",((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
                    ((ip& 0xff00) >> 8),(ip & 0xff));	  
        vty_out(vty,"       VIRTUAL IP:%d.%d.%d.%d\n ",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
                    ((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff));	  */

		//kehao modify 20110519
        //strcpy(zvrrp->downlink_ifname,downlink_ifname);
        strcpy(downlink[i],downlink_ifname);
        /////////////////////////////////////////////////////////
        //kehao modify  20110520
        //strcpy(zvrrp->downstate,link ? "UP" : "DOWN");	
        strcpy(downstate[i],link ? "UP" : "DOWN"); //下行接口状态
        //real ip
            memset(link_ip,0,64);	
            sprintf(link_ip,"%d.%d.%d.%d ",((ip & 0xff000000) >> 24),((ip & 0xff0000) >> 16),	\
                            ((ip& 0xff00) >> 8),(ip & 0xff));
            //kehao modify  20110520
            //strcpy(zvrrp->realipdown,link_ip);
            strcpy(rdownlinkip[i],link_ip);
            /////////////////////////////////////////////////////////////
            zvrrp->drip1=((ip & 0xff000000) >> 24);
            zvrrp->drip2=((ip & 0xff0000) >> 16);
            zvrrp->drip3=((ip& 0xff00) >> 8);
            zvrrp->drip4=(ip & 0xff);

        //virtual ip
            memset(link_ip,0,64);	
            sprintf(link_ip,"%d.%d.%d.%d ",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
                        ((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff));
			//kehao modify 20110520
            //strcpy(zvrrp->downlink_ip,link_ip);
            strcpy(vdownlinkip[i],link_ip);
			///////////////////////////////////////////////
            zvrrp->dlip1=((downlink_ip & 0xff000000) >> 24);
            zvrrp->dlip2=((downlink_ip & 0xff0000) >> 16);
            zvrrp->dlip3=((downlink_ip& 0xff00) >> 8);
            zvrrp->dlip4=(downlink_ip & 0xff);
    	}

    }
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&wid_transfer);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&portal_enable);	
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&portal_transfer);
    dbus_message_iter_next(&iter);
    if(state == 6){
       //vty_out(vty,"BATHSYN INFO:\n");
       //vty_out(vty,"      WID STATE: %s\n",wid_transfer ? "TRANSFERING" : "RUN");
       strcpy(zvrrp->widstate,wid_transfer ? "TRANSFERING" : "RUN");
       if(portal_enable){
         // vty_out(vty,"       PORTAL STATE: %s\n",portal_transfer ? "TRANSFERING" : "RUN");
        strcpy(zvrrp->portalstate,portal_transfer ? "TRANSFERING" : "RUN");
       }
       else{
         // vty_out(vty,"       PORTAL STATE: NOT ELECTION \n");
         strcpy(zvrrp->portalstate, "NONE" );
       }
    }		
    dbus_message_iter_get_basic(&iter,&heartbeatlink_ip);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&heartbeat_link);
    dbus_message_iter_next(&iter);	
    dbus_message_iter_get_basic(&iter,&heartbeat_flag);
    if (1 == heartbeat_flag)
    {
    dbus_message_iter_next(&iter);	
    dbus_message_iter_get_basic(&iter,&heartbeatlink_ifname);
    //vty_out(vty,"HEART-BEAT-LINK INFO:\n");	
    if(0 != heartbeatlink_ip){
        /*vty_out(vty,"     IFNAME: %s(%s) IP:%d.%d.%d.%d\n",heartbeatlink_ifname, heartbeat_link ? "UP":"DOWN",((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
                    ((heartbeatlink_ip& 0xff00) >> 8),heartbeatlink_ip & 0xff);*/
       strcpy(zvrrp->hbinf,heartbeatlink_ifname);
       strcpy(zvrrp->hbstate,heartbeat_link ? "UP":"DOWN");
       //heartbeatlink ip
        memset(link_ip,0,64);	
        sprintf(link_ip,"%d.%d.%d.%d ",((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
                    ((heartbeatlink_ip& 0xff00) >> 8),(heartbeatlink_ip & 0xff));
        strcpy(zvrrp->hbip,link_ip);
        zvrrp->hbip1=((heartbeatlink_ip & 0xff000000) >> 24);
        zvrrp->hbip2=((heartbeatlink_ip & 0xff0000) >> 16);
        zvrrp->hbip3=((heartbeatlink_ip& 0xff00) >> 8);
        zvrrp->hbip4=(heartbeatlink_ip & 0xff);

    }
    else{
        //vty_out(vty,"     NOT ELECTION\n");
    }
    }else{
    }
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&if_vgateway);
    //vty_out(vty,"VGATEWAY INFO:\n");
    
    if(0 != if_vgateway){
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter, &vgateway_naddr);
		//kehao add 20110519
        *vgatenum = vgateway_naddr;
		////////////////////////////
        zvrrp->gw_number=vgateway_naddr;
        for(i = 0; i < vgateway_naddr; i++) {
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter,&vgateway_ifname);
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter,&ip);
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter,&vgateway_ip);			
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter,&vgateway_link);			

            strcpy(zvrrp->gw[i].gwname,vgateway_ifname);
            strcpy(zvrrp->gw[i].gwstate,vgateway_link ? "UP":"DOWN");
            //gateway ip
            memset(link_ip,0,64);	
            sprintf(link_ip,"%d.%d.%d.%d ",((vgateway_ip & 0xff000000) >> 24),((vgateway_ip & 0xff0000) >> 16),	\
                        ((vgateway_ip& 0xff00) >> 8),(vgateway_ip & 0xff));
			//kehao modify  20110520
            //strcpy(zvrrp->gw[i].gwip,link_ip);
            strcpy(vgateip[i],link_ip);
			///////////////////////////////
            zvrrp->gw[i].gwip1=((vgateway_ip & 0xff000000) >> 24);
            zvrrp->gw[i].gwip2=((vgateway_ip & 0xff0000) >> 16);
            zvrrp->gw[i].gwip3=((vgateway_ip& 0xff00) >> 8);
            zvrrp->gw[i].gwip4=(vgateway_ip & 0xff);
    	}
    }
    else{
    }

    dbus_message_unref(reply);
    return DCLI_VRRP_RETURN_CODE_OK;
}
#endif 

void free_ccgi_show_hansi_profile(Z_VRRP *zvrrp)
{
    vrrp_link_ip *f1 = NULL;

	while(zvrrp->uplink_list)
	{
		f1 =zvrrp->uplink_list->next;
		free(zvrrp->uplink_list);
		zvrrp->uplink_list = f1;
	}
	while(zvrrp->downlink_list)
	{
		f1 =zvrrp->downlink_list->next;
		free(zvrrp->downlink_list);
		zvrrp->downlink_list = f1;
	}
	while(zvrrp->vgatewaylink_list)
	{
		f1 =zvrrp->vgatewaylink_list->next;
		free(zvrrp->vgatewaylink_list);
		zvrrp->vgatewaylink_list = f1;
	}
	zvrrp->uplink_list = NULL;
	zvrrp->downlink_list = NULL;
	zvrrp->vgatewaylink_list = NULL;
	return;
}

/*  只要调用  ,  就用  free_ccgi_show_hansi_profile  释放空间      */
int ccgi_show_hansi_profile(Z_VRRP *zvrrp,int profile)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;

	char link_ip[64];
	memset(link_ip,0,64);

    ///////////////////////////
	unsigned int op_ret = 0;
	unsigned int advert = 0,preempt = 0,priority = 0,state = 0;
	char *uplink_ifname = NULL,*downlink_ifname =NULL,*heartbeatlink_ifname = NULL,*vgateway_ifname = NULL;
	int uplink_ip = 0,downlink_ip = 0,heartbeatlink_ip = 0;
	int wid_transfer = 0,portal_enable = 0,portal_transfer = 0;
    int if_vgateway = 0,vgateway_ip = 0,vgateway_mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
	int instRun = 0;
	int i=0;
	unsigned int failover_peer = 0, failover_local = 0;
	/* 0: not setted, 1: setted */
	unsigned int uplink_set_flg = 0;
	unsigned int downlink_set_flg = 0;
	unsigned int heartbeat_set_flg = 0;

    #if 1
	instRun = ccgi_vrrp_hansi_is_running(profile);
	if (DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		//vty_out(vty, "had instance %d not created!\n", profile);
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		//vty_out(vty, "check had instance %d whether created was failed!\n",
		//			profile);
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
    #endif
	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	///////////////////////////
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,				 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);		
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if((DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret)||(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret)){
		dbus_message_unref(reply);	
	   return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	//vty_out(vty,"-----------------------------HANSI %d--------------------------------\n",profile);
	//vty_out(vty,"STATE--PRI--ADVERT--PREEMPT--UPLINK------IP-----DOWNLINK-----IP------\n");
	
	//êy?Y′?μ??á11ì?è￥,?ù′?μY
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);

	//vty_out(vty,"STATE: %s\n",(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");

	strcpy(zvrrp->state,(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority);
	
	//vty_out(vty,"%-7d",priority);
	zvrrp->priority=priority;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&advert);
	
	//vty_out(vty,"%-8d",advert);
	zvrrp->advert=advert;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&preempt);
	
	//vty_out(vty,"%-5s",preempt ? "Y" : "N");	
	strcpy(zvrrp->preempt,(preempt ? "Y" : "N"));

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&uplink_set_flg);
	if (uplink_set_flg) {	
		for(i = 0; i < uplink_set_flg; i++)
		{
		
			vrrp_link_ip *new_uplink = (vrrp_link_ip *)malloc(sizeof(vrrp_link_ip));
			if(NULL == new_uplink) {
				return DCLI_VRRP_RETURN_CODE_MALLOC_FAILED;
			}
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&uplink_ifname);
			
			//vty_out(vty,"%-10s",uplink_ifname);	
			if(NULL == uplink_ifname)
			{
			  //vty_out(vty,"--""%-8s");
			  strcpy(zvrrp->uplink_ifname,"");
			  strcpy(new_uplink->ifname,"");
			}
			else
			{
			  //vty_out(vty,"%-10s",uplink_ifname); 
			  strcpy(zvrrp->uplink_ifname,uplink_ifname);
			  strcpy(new_uplink->ifname,uplink_ifname);
			}	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&uplink_ip);
			
			/*vty_out(vty,"%d.%d.%d.%d ",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16),	\
							((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff)); */
			
			memset(link_ip,0,64);
			if(0 == uplink_ip)
			{
				//vty_out(vty,"--");
				strcpy(zvrrp->uplink_ip,"--");
				zvrrp->ulip1=0;
				zvrrp->ulip2=0;
				zvrrp->ulip3=0;
				zvrrp->ulip4=0;
				
				strcpy(new_uplink->link_ip,"--");
				new_uplink->ip1=0;
				new_uplink->ip2=0;
				new_uplink->ip3=0;
				new_uplink->ip4=0;
			}
			else 
			{
			
				sprintf(link_ip,"%d.%d.%d.%d ",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16), \
							((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));
				strcpy(zvrrp->uplink_ip,link_ip);
				zvrrp->ulip1=((uplink_ip & 0xff000000) >> 24);
				zvrrp->ulip2=((uplink_ip & 0xff0000) >> 16);
				zvrrp->ulip3=((uplink_ip& 0xff00) >> 8);
				zvrrp->ulip4=(uplink_ip & 0xff);
				strncat(zvrrp->uplink_ip_list,zvrrp->uplink_ip,sizeof(zvrrp->uplink_ip_list)-strlen(zvrrp->uplink_ip_list)-1);
				strncat(zvrrp->uplink_ip_list,";",sizeof(zvrrp->uplink_ip_list)-strlen(zvrrp->uplink_ip_list)-1);
				
				strcpy(new_uplink->link_ip,link_ip);
				new_uplink->ip1=((uplink_ip & 0xff000000) >> 24);
				new_uplink->ip2=((uplink_ip & 0xff0000) >> 16);
				new_uplink->ip3=((uplink_ip& 0xff00) >> 8);
				new_uplink->ip4=(uplink_ip & 0xff);
			}
			new_uplink->next = zvrrp->uplink_list;
			zvrrp->uplink_list = new_uplink;
		}

	}else{
		//vty_out(vty,"%-8s", "-");
		//vty_out(vty,"%-15s", "-");
		}
	
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&downlink_set_flg);
	if (downlink_set_flg) {
		for(i = 0; i < downlink_set_flg; i++)
		{   
			vrrp_link_ip *new_downlink = (vrrp_link_ip *)malloc(sizeof(vrrp_link_ip));
			if(NULL == new_downlink) {
				return DCLI_VRRP_RETURN_CODE_MALLOC_FAILED;
			}
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&downlink_ifname);

			//vty_out(vty,"%-10s",downlink_ifname);
			strcpy(zvrrp->downlink_ifname,downlink_ifname);
			strcpy(new_downlink->ifname,downlink_ifname);

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&downlink_ip);		
			/*vty_out(vty,"%d.%d.%d.%d ",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
							((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff)); */

			memset(link_ip,0,64);	
			sprintf(link_ip,"%d.%d.%d.%d ",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
							((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff));
			strcpy(zvrrp->downlink_ip,link_ip);
			zvrrp->dlip1=((downlink_ip & 0xff000000) >> 24);
			zvrrp->dlip2=((downlink_ip & 0xff0000) >> 16);
			zvrrp->dlip3=((downlink_ip& 0xff00) >> 8);
			zvrrp->dlip4=(downlink_ip & 0xff);
			
			strncat(zvrrp->downlink_ip_list,zvrrp->downlink_ip,sizeof(zvrrp->downlink_ip_list)-strlen(zvrrp->downlink_ip_list)-1);
			strncat(zvrrp->downlink_ip_list,";",sizeof(zvrrp->downlink_ip_list)-strlen(zvrrp->downlink_ip_list)-1);
			strcpy(new_downlink->link_ip,link_ip);
			new_downlink->ip1=((downlink_ip & 0xff000000) >> 24);
			new_downlink->ip2=((downlink_ip & 0xff0000) >> 16);
			new_downlink->ip3=((downlink_ip& 0xff00) >> 8);
			new_downlink->ip4=(downlink_ip & 0xff);
			
			new_downlink->next = zvrrp->downlink_list;
			zvrrp->downlink_list = new_downlink;

		}
	}
	else {
			//vty_out(vty,"%-8s", "-");
			//vty_out(vty,"%-15s", "-");
	}

  	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wid_transfer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_enable);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_transfer);
	dbus_message_iter_next(&iter);

    if(state == 6){
	  // vty_out(vty,"BATHSYN INFO:\n");
       //vty_out(vty,"		WID STATE: %s\n",wid_transfer ? "TRANSFERING" : "RUN");
	   if(portal_enable){
          //vty_out(vty,"		PORTAL STATE: %s\n",portal_transfer ? "TRANSFERING" : "RUN");
	   }
	   else{
          //vty_out(vty,"		PORTAL STATE: NOT ELECTION \n");
	   }
	}		
	dbus_message_iter_get_basic(&iter,&heartbeat_set_flg);
	if (1 == heartbeat_set_flg) {
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeatlink_ip);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeatlink_ifname);
	
	if(0 != heartbeatlink_ip){
		/*vty_out(vty,"HEART-BEAT-LINK %s %d.%d.%d.%d\n",heartbeatlink_ifname,((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
					((heartbeatlink_ip& 0xff00) >> 8),heartbeatlink_ip & 0xff);*/
       strcpy(zvrrp->hbinf,heartbeatlink_ifname);
	   //heartbeatlink ip
   		memset(link_ip,0,64);	
		sprintf(link_ip,"%d.%d.%d.%d ",((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
					((heartbeatlink_ip& 0xff00) >> 8),(heartbeatlink_ip & 0xff));
		strcpy(zvrrp->hbip,link_ip);
		zvrrp->hbip1=((heartbeatlink_ip & 0xff000000) >> 24);
		zvrrp->hbip2=((heartbeatlink_ip & 0xff0000) >> 16);
		zvrrp->hbip3=((heartbeatlink_ip& 0xff00) >> 8);
		zvrrp->hbip4=(heartbeatlink_ip & 0xff);
	}
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&if_vgateway);
	zvrrp->gw_number=if_vgateway;
	if(0 != if_vgateway){
		
		for(i = 0; i < if_vgateway; i++) {
		
		vrrp_link_ip *new_vgatelink = (vrrp_link_ip *)malloc(sizeof(vrrp_link_ip));
		if(NULL == new_vgatelink) {
			return DCLI_VRRP_RETURN_CODE_MALLOC_FAILED;
		}
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_ifname);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_ip);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_mask);
		fprintf(stderr,"zvrrp.gw[gw_i].gw_mask2=%d\n",vgateway_mask);
		strcpy(zvrrp->gw[i].gwname,vgateway_ifname);
		strncpy(new_vgatelink->ifname,vgateway_ifname,sizeof(new_vgatelink->ifname)-strlen(new_vgatelink->ifname)-1);
		memset(link_ip,0,64);	
		sprintf(link_ip,"%d.%d.%d.%d ",((vgateway_ip & 0xff000000) >> 24),((vgateway_ip & 0xff0000) >> 16),	\
					((vgateway_ip& 0xff00) >> 8),(vgateway_ip & 0xff));
		strcpy(zvrrp->gw[i].gwip,link_ip);
		zvrrp->gw[i].gwip1=((vgateway_ip & 0xff000000) >> 24);
		zvrrp->gw[i].gwip2=((vgateway_ip & 0xff0000) >> 16);
		zvrrp->gw[i].gwip3=((vgateway_ip& 0xff00) >> 8);
		zvrrp->gw[i].gwip4=(vgateway_ip & 0xff);
		zvrrp->gw[i].gw_mask=vgateway_mask;
		
		strncpy(new_vgatelink->link_ip,link_ip,sizeof(new_vgatelink->link_ip)-strlen(new_vgatelink->link_ip)-1);
		new_vgatelink->ip1=((vgateway_ip & 0xff000000) >> 24);
		new_vgatelink->ip2=((vgateway_ip & 0xff0000) >> 16);
		new_vgatelink->ip3=((vgateway_ip& 0xff00) >> 8);
		new_vgatelink->ip4=(vgateway_ip & 0xff);

		new_vgatelink->next = zvrrp->vgatewaylink_list;
		zvrrp->vgatewaylink_list = new_vgatelink;
		}
	}
	else {
		
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_peer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_local);
	//vty_out(vty, "-DHCP FAILOVER-\n");
	if(~0UI != failover_peer) {
		//vty_out(vty, "%-7speer %d.%d.%d.%d", "", (failover_peer >> 24) & 0xFF, \
			//(failover_peer >> 16) & 0xFF, (failover_peer >> 8) & 0xFF, failover_peer & 0xFF);
		if(~0UI != failover_local) {
			//vty_out(vty, " local %d.%d.%d.%d", (failover_local >> 24) & 0xFF, \
				//(failover_local >> 16) & 0xFF, (failover_local >> 8) & 0xFF, failover_local & 0xFF);
		}
		//vty_out(vty, "\n");
	}
	else {
		//vty_out(vty, "%-7snot configured\n", "");
	}

	dbus_message_unref(reply);
	return DCLI_VRRP_RETURN_CODE_OK;
}

void free_ccgi_show_hansi_profile_web(Z_VRRP_web *zvrrp)
{
    vrrp_link_ip_web *f1 = NULL;

	while(zvrrp->uplink_list)
	{
		f1 =zvrrp->uplink_list->next;
		free(zvrrp->uplink_list);
		zvrrp->uplink_list = f1;
	}
	while(zvrrp->downlink_list)
	{
		f1 =zvrrp->downlink_list->next;
		free(zvrrp->downlink_list);
		zvrrp->downlink_list = f1;
	}
	while(zvrrp->vgatewaylink_list)
	{
		f1 =zvrrp->vgatewaylink_list->next;
		free(zvrrp->vgatewaylink_list);
		zvrrp->vgatewaylink_list = f1;
	}
	zvrrp->uplink_list = NULL;
	zvrrp->downlink_list = NULL;
	zvrrp->vgatewaylink_list = NULL;
	return;
}


/*  只要调用  ,  就用  free_ccgi_show_hansi_profile  释放空间      */
int ccgi_show_hansi_profile_web(Z_VRRP_web *zvrrp,int profile,int slotid,DBusConnection *connection)
{
   	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;

	char link_ip[32] = {0};

    ///////////////////////////
	unsigned int op_ret = 0;
	unsigned int advert = 0,preempt = 0,priority = 0,state = 0;
	char *uplink_ifname = NULL,*downlink_ifname =NULL,*heartbeatlink_ifname = NULL,*vgateway_ifname = NULL;
	int uplink_ip = 0,downlink_ip = 0,heartbeatlink_ip = 0;
	int wid_transfer = 0,portal_enable = 0,portal_transfer = 0;
    int if_vgateway = 0,vgateway_ip = 0,vgateway_mask = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	
	int instRun = 0;
	int i=0;
	unsigned int failover_peer = 0, failover_local = 0;
	/* 0: not setted, 1: setted */
	unsigned int uplink_set_flg = 0;
	unsigned int downlink_set_flg = 0;
	unsigned int heartbeat_set_flg = 0;
	unsigned int vgateway_set_flg = 0;
	unsigned int l2_uplink_set_flg = 0;
	char * l2_uplink_ifname = NULL;
	vrrp_link_ip_web *uplink_tail,*uq=NULL;
	vrrp_link_ip_web *downlink_tail,*dq=NULL;
	vrrp_link_ip_web *vgwlink_tail,*vq=NULL;
	
	instRun = ccgi_vrrp_hansi_is_running_dis(slotid,0,profile);
	if (INSTANCE_NO_CREATED == instRun) {
		return -2;
	}	
	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	///////////////////////////
	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,				 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);		
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if((DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret)||(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret)){
		dbus_message_unref(reply);	
	   return -3;
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);

	strcpy(zvrrp->state,(state==3)?"MASTER":(state == 2)?"BACK" : (state == 99)?"DISABLE" : (state == 4)?"LEARNING" :(state == 6)? "TRANSFER" :"INIT");
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&priority);
	
	zvrrp->priority=priority;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&advert);
	
	zvrrp->advert=advert;
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&preempt);
	
	strcpy(zvrrp->preempt,(preempt ? "Y" : "N"));

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&uplink_set_flg);
	if (uplink_set_flg) {	
		for(i = 0; i < uplink_set_flg; i++)
		{
		
			uq = (vrrp_link_ip_web *)malloc(sizeof(vrrp_link_ip_web)+1);
			if(NULL== uq)
			{
				return -4;
			}
			memset(uq,0,sizeof(vrrp_link_ip_web)+1);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&uplink_ifname);
			
			if(NULL == uplink_ifname)
			{
			  strcpy(uq->ifname,"");
			}
			else
			{
			  strcpy(uq->ifname,uplink_ifname);
			}	
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&uplink_ip);
			
			
			memset(link_ip,0,32);
			if(0 == uplink_ip)
			{
				strcpy(uq->link_ip,"--");
			}
			else 
			{
			
				sprintf(link_ip,"%d.%d.%d.%d ",((uplink_ip & 0xff000000) >> 24),((uplink_ip & 0xff0000) >> 16), \
							((uplink_ip& 0xff00) >> 8),(uplink_ip & 0xff));
				strcpy(uq->link_ip,link_ip);
			}
			
			uq->next = zvrrp->uplink_list;
			zvrrp->uplink_list = uq;
		}
	}
	else{
		}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&l2_uplink_set_flg);
	if (l2_uplink_set_flg) {	
		for(i = 0; i < l2_uplink_set_flg; i++) {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&l2_uplink_ifname);
		}
	} 
	else {
	}
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&downlink_set_flg);
	if (downlink_set_flg) {
		for(i = 0; i < downlink_set_flg; i++)
		{   
			dq = (vrrp_link_ip_web *)malloc(sizeof(vrrp_link_ip_web)+1);
			if(NULL== dq)
			{
				return -5;
			}
			memset(dq,0,sizeof(vrrp_link_ip_web)+1);
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&downlink_ifname);

			strcpy(dq->ifname,downlink_ifname);

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&downlink_ip);		

			memset(link_ip,0,32);	
			sprintf(link_ip,"%d.%d.%d.%d ",((downlink_ip & 0xff000000) >> 24),((downlink_ip & 0xff0000) >> 16),	\
							((downlink_ip& 0xff00) >> 8),(downlink_ip & 0xff));
			strcpy(dq->link_ip,link_ip);
			
			dq->next = zvrrp->downlink_list;
			zvrrp->downlink_list= dq;
		}
	}
	else {
	}

  	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wid_transfer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_enable);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&portal_transfer);
	dbus_message_iter_next(&iter);

    if(state == 6){
	   if(portal_enable){
	   }
	   else{
	   }
	}		
	dbus_message_iter_get_basic(&iter,&heartbeat_set_flg);
	if (1 == heartbeat_set_flg) {
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeatlink_ip);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&heartbeatlink_ifname);
	
	if(0 != heartbeatlink_ip){
       strcpy(zvrrp->hbinf,heartbeatlink_ifname);
	   //heartbeatlink ip
   		memset(link_ip,0,64);	
		sprintf(link_ip,"%d.%d.%d.%d ",((heartbeatlink_ip & 0xff000000) >> 24),((heartbeatlink_ip & 0xff0000) >> 16),	\
					((heartbeatlink_ip& 0xff00) >> 8),(heartbeatlink_ip & 0xff));
		strcpy(zvrrp->hbip,link_ip);
		}
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&if_vgateway);
	
	if(0 != if_vgateway){
		
		for(i = 0; i < if_vgateway; i++) {
		
		vq = (vrrp_link_ip_web *)malloc(sizeof(vrrp_link_ip_web)+1);
		if(NULL== vq)
		{
			return -6;
		}
		memset(vq,0,sizeof(vrrp_link_ip_web)+1);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_ifname);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_ip);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vgateway_mask);

		strcpy(vq->ifname,vgateway_ifname);
		vq->maskint = vgateway_mask;

		memset(link_ip,0,32);	
		sprintf(link_ip,"%d.%d.%d.%d ",((vgateway_ip & 0xff000000) >> 24),((vgateway_ip & 0xff0000) >> 16),	\
					((vgateway_ip& 0xff00) >> 8),(vgateway_ip & 0xff));
		strcpy(vq->link_ip,link_ip);
		vq->next = zvrrp->vgatewaylink_list;
		zvrrp->vgatewaylink_list= vq;
		}
	}
	else {
		
	}

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_peer);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&failover_local);

	dbus_message_unref(reply);
	return 0;
}

unsigned long ccgi_ip2ulong_vrrp(char *str)
{
	char *sep=".";
	char *token;
	unsigned long ip_long[4]; 
    unsigned long ip;
	int i = 1;
	
	token=strtok(str,sep);
	if(NULL != token){
	    ip_long[0] = strtoul(token,NULL,10);
	}
	while((token!=NULL)&&(i<4))
	{
		token=strtok(NULL,sep);
		if(NULL != token){
		    ip_long[i] = strtoul(token,NULL,10);
		}
		i++;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];

	return ip;
}

int ccgi_str2ulong_vrrp(char *str,unsigned int *Value)
{
	char *endptr = NULL;
	char c;
	int ret = 0;
	if (NULL == str) return NPD_FAIL;

	ret = ccgi_checkPoint_vrrp(str);
	if(ret == 1){
		return NPD_FAIL;
	}

	c = str[0];
	if((strlen(str) > 1)&&('0' == c)){
		/* string(not single "0") should not start with '0'*/
		return NPD_FAIL;
	}		
	*Value= strtoul(str,&endptr,10);
	if('\0' != endptr[0]){
		return NPD_FAIL;
	}
	//return NPD_SUCCESS;	
	return 0;
}

int ccgi_checkPoint_vrrp(char *ptr)
{
	int ret = 0;
	while(*ptr != '\0') {
		if(((*ptr) < '0')||((*ptr) > '9')){
			ret = 1;
	 		break;
		}
		ptr++;
	}
	return ret;
}
int ccgi_vrrp_check_ip_format
(
   char* buf,
   int* split_count
)
{
    //char *split = "/";
	char *str = NULL;
	int length = 0,i = 0,splitCount = 0;
	if(NULL == buf){
       // return -2;
       return -1;
	}	
	str = buf;
	length = strlen(str);
	if( length > DCLI_IPMASK_STRING_MAXLEN ||  \
		length < DCLI_IP_STRING_MINLEN ){
		//return -2;
		return -1;
	}
	if((str[0] > '9')||(str[0] < '1')){
		//return -2;
		return -1;
	}
	for(i = 0; i < length; i++){
		if('/' == str[i]){
            splitCount++;
			if((i == length - 1)||('0' > str[i+1])||(str[i+1] > '9')){
                //return -2;
                return -1;
			}
		}
		if((str[i] > '9'||str[i]<'0') &&  \
			str[i] != '.' &&  \
			str[i] != '/' &&  \
			str[i] != '\0'
		){
                //return -2;
                return -1;
		}
	}  
    *split_count = splitCount;
	return 0;
}

unsigned int ccgi_vrrp_config_service_disable
(
	unsigned int profile
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = DCLI_VRRP_RETURN_CODE_OK;
	unsigned int enable = 0;	/* 0: disable */
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	/*if (!vty) {
		return 1;
	}*/

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										vrrp_obj_path,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_UINT32, &enable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&op_ret,
									DBUS_TYPE_INVALID))
	{
		if( DCLI_VRRP_RETURN_CODE_OK != op_ret){
			//vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			/* return 0 for delete hansi instance when service not enable. */
			return 0;
		}
	}else {		
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
	}
	
	dbus_message_unref(reply);
	
	return 0;
}
int  send_arp(char *profid,char *ifnamez,char *ipz,char *macz)/*返回0表示成功，返回-1表示error，返回-2表示错误mac形式*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	//unsigned int vrid = 0,priority = 0;
	//int add = 1;
	int retu = 0;
	char* ifname = NULL;
    char* ip = NULL;

	ifname = (char *)malloc(MAX_IFNAME_LEN);	
	if(NULL == ifname){
       return -2;
	}
	memset(ifname,0,MAX_IFNAME_LEN);
	memcpy(ifname,ifnamez,strlen(ifnamez));
	
	ip = (char *)malloc(MAX_IPADDR_LEN);	
	if(NULL == ip){
       return -2;
	}
	memset(ip,0,MAX_IPADDR_LEN);
	memcpy(ip,ipz,strlen(ipz));

	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));	
	op_ret = parse_mac_addr((char *)macz,&macAddr);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		//return 0;
		return -2;
	}

	/*if(HANSI_NODE==vty->node){
		profile = (unsigned int)(vty->index);
	}*/
	profile = (unsigned int)strtoul(profid,0,10);

	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_START_SEND_ARP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&ifname,
							 DBUS_TYPE_STRING,&ip,
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[0],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[1],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[2],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[3],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[4],
						 	 DBUS_TYPE_BYTE,&macAddr.arEther[5],							 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
           ;	
		/*dcli_vrrp_notify_to_npd(vty,NULL,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		retu = -1;
	}

	dbus_message_unref(reply);
	//return 0;
	free(ifname);
	free(ip);
	return retu;
}
#if 0
int ccgi_vrrp_hansi_is_running
(
	unsigned int profileId
)
{
	int  ret = 0, fd = -1, iCnt = 0;	
	char commandBuf[DCLI_VRRP_SYS_COMMAND_LEN] = {0};
	//int isRunning = 0;
	//char readBuf[4] = {0};
	sprintf(commandBuf, "sudo ps auxww | grep \"had %d$\" | wc -l > /var/run/had%d.num", profileId, profileId);
	ret = system(commandBuf);
	if(ret) {
		//vty_out(vty, "%% Check hansi instance %d failed!\n", profileId);
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	/* get the process # */
	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf,"/var/run/had%d.num", profileId);
	if((fd = open(commandBuf, O_RDONLY))< 0) {
		//vty_out(vty, "%% Check hansi instance %d count failed!\n", profileId);
		return DCLI_VRRP_INSTANCE_CHECK_FAILED;
	}

	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	read(fd, commandBuf, 4);
	iCnt = strtoul(commandBuf, NULL, 10);
	if(iCnt > 2) {
		ret = DCLI_VRRP_INSTANCE_CREATED;
	}
	else {
		ret = DCLI_VRRP_INSTANCE_NO_CREATED;
	}

	/* release file resources */
	close(fd);

	memset(commandBuf, 0, DCLI_VRRP_SYS_COMMAND_LEN);
	sprintf(commandBuf, "sudo rm /var/run/had%d.num", profileId);
	system(commandBuf);

	return ret;
}
#endif 
int ccgi_vrrp_hansi_is_running
(
	unsigned int profileId
)
{
	char filename[256] = {0};
	char line[256] = {0};
	FILE *fp = NULL;
	char *p = NULL;

	snprintf(filename, sizeof(filename), "/var/run/had%d.pid", profileId);
	if (access(filename, F_OK) != 0)
	{
		return DCLI_VRRP_INSTANCE_NO_CREATED;
	}

	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "/var/run/had%d.pidmap", profileId);
	if ( (fp = fopen(filename, "r")) == NULL)
	{
		return DCLI_VRRP_INSTANCE_NO_CREATED;
	}
	while (fgets(line, sizeof(line)-1, fp) != NULL)
	{
		for (p = line; *p; p++)
		{
			if ('\r' == *p || '\n' == *p)
			{
				*p = '\0';
				break;
			}
		}

		if ( (p = strstr(line, "has pid")) != NULL)
		{
			p += (strlen("has pid") +1);
			snprintf(filename, sizeof(filename), "/proc/%ld", strtoul(p, NULL, 10));
			if (access(filename, F_OK) != 0)
			{
				fclose(fp);
				return DCLI_VRRP_INSTANCE_NO_CREATED;
			}
		}
	}
	fclose(fp);
	
	return DCLI_VRRP_INSTANCE_CREATED;
}
int ccgi_vrrp_hansi_is_running_dis
(
	unsigned int slot_id,
	int islocal,
	int instID

)
{
	char commandBuf[DCLI_VRRP_SYS_COMMAND_LEN] = {0};
	sprintf(commandBuf, "/var/run/hmd/hmd%d-%d-%d.pid", islocal,slot_id,instID);
	if(access(commandBuf,0)!=0)
	{
		return INSTANCE_NO_CREATED;
	}
	return INSTANCE_CREATED;
}
int config_vrrp_real_ip_uplink(char *upif,char *upip,char *profid)/*返回0表示成功，返回-1表示失败，返回-2表示error*/
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
    int retu = 0;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	char *uplink_ifname = NULL;
	char *uplink_ip = NULL;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	uplink_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if (NULL == uplink_ifname) {
		return -2;
	}
	memset(uplink_ifname, 0, MAX_IFNAME_LEN);
	memcpy(uplink_ifname, upif, strlen(upif));

	uplink_ip = (char *)malloc(MAX_IPADDR_LEN);
	if (NULL == uplink_ip) {
		free(uplink_ifname);
		return -2;
	}
	memset(uplink_ip, 0, MAX_IPADDR_LEN);
	memcpy(uplink_ip, upip, strlen(upip));

	/*if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}*/
    profile=(unsigned int)strtoul(profid,0,10);
	
	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_REAL_IP_UPLINK);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_STRING, &uplink_ifname,
							DBUS_TYPE_STRING, &uplink_ip,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu  =-1;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
            //vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
            retu = op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);

	free(uplink_ifname);
	free(uplink_ip);
	return retu;
}


int config_vrrp_no_real_ip_downlink(char *downif,char *downip,char *profid)/*返回0表示成功，返回-1表示失败，返回-2表示error*/
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
    int retu = 0;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	char downlink_ifname[MAX_IFNAME_LEN] = {0};
	char downlink_ip[MAX_IPADDR_LEN] = {0};
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	memset(downlink_ifname, 0, MAX_IFNAME_LEN);
	memcpy(downlink_ifname, downif, strlen(downif));
	
	memset(downlink_ip, 0, MAX_IPADDR_LEN);
	memcpy(downlink_ip, downip, strlen(downip));

	/*if (HANSI_NODE==vty->node) {
		profile = (unsigned int)(vty->index);
	}*/
    profile =(unsigned int)strtoul(profid,0,10);
	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_NO_REAL_IP_DOWNLINK);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32, &profile,
							 DBUS_TYPE_STRING, &downlink_ifname,
							 DBUS_TYPE_STRING, &downlink_ip,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu  =-1;
	}
	else if (dbus_message_get_args(reply, &err,
										DBUS_TYPE_UINT32, &op_ret,
										DBUS_TYPE_INVALID))
	{
		if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
			//vty_out(vty,dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
			retu  =op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return retu ;
}

int config_vrrp_no_real_ip_uplink(char *upif,char *upip,char *profid)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
    int retu = 0;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	char uplink_ifname[MAX_IFNAME_LEN] = {0};
	char uplink_ip[MAX_IPADDR_LEN] = {0};
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};

	memset(uplink_ifname, 0, MAX_IFNAME_LEN);
	memcpy(uplink_ifname, upif, strlen(upif));

	memset(uplink_ip, 0, MAX_IPADDR_LEN);
	memcpy(uplink_ip, upip, strlen(upip));

	/*if (HANSI_NODE == vty->node) {
		profile = (unsigned int)(vty->index);
	}*/
    profile = (unsigned int)strtoul(profid,0,10);
	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_VRRP_NO_REAL_IP_UPLINK);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_STRING, &uplink_ifname,
							DBUS_TYPE_STRING, &uplink_ip,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	else if (dbus_message_get_args(reply, &err,
									DBUS_TYPE_UINT32, &op_ret,
									DBUS_TYPE_INVALID))
	{
        if (DCLI_VRRP_RETURN_CODE_OK != op_ret) {
           // vty_out(vty, dcli_vrrp_err_msg[op_ret - DCLI_VRRP_RETURN_CODE_OK]);
           retu = op_ret - DCLI_VRRP_RETURN_CODE_OK;
		}
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return retu;
}


int snmp_get_vrrp_state(int profile,int *hansi_state)/*返回0表示失败，返回1表示成功，返回-1表示实例不存在*/
{
    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter  iter;
    DBusMessageIter  iter_array;
    unsigned int ret;
    unsigned int flag = 0;
	unsigned int state = 0;
	unsigned int pid = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};
	int retu;

    //log_info( "HA_TEST:notify_had_backup_finished1111" );
	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SNMP_GET_VRRP_STATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &profile,
							DBUS_TYPE_INVALID);

    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    dbus_message_unref(query);
    if (NULL == reply) 
	{
        if (dbus_error_is_set(&err)) 
		{
            dbus_error_free(&err);
        }
        return 0;
    }

    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&pid);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&state);

	if(pid == 0)
	{
		retu = -1;
	}
	else
	{
		*hansi_state = state;
		retu = 1;
	}

    dbus_message_unref(reply);
    return retu;
}

int mask_bit(char *mask)
{
	int k=0,i;
	int flag = -1;
    char *mask_val[32] = {
             "128.0.0.0",
             "192.0.0.0",
             "224.0.0.0",
             "240.0.0.0",
             "248.0.0.0",
             "252.0.0.0",
             "254.0.0.0",
             "255.0.0.0",
             "255.128.0.0",
             "255.192.0.0",
             "255.224.0.0",
             "255.240.0.0",
             "255.248.0.0",
             "255.252.0.0",
             "255.254.0.0",
             "255.255.0.0",
             "255.255.128.0",
             "255.255.192.0",
             "255.255.224.0",
             "255.255.240.0",
             "255.255.248.0",
             "255.255.252.0",
             "255.255.254.0",
             "255.255.255.0",
             "255.255.255.128",
             "255.255.255.192",
             "255.255.255.224",
             "255.255.255.240",
             "255.255.255.248",
             "255.255.255.252",
             "255.255.255.254",
             "255.255.255.255"
    	};

	for(i=0;i<32;i++)
	{
	    k ++;
	    if(strcmp(mask,mask_val[i])==0)
		{
		    flag = 0;
			break;
		}		
	}
	if(flag == 0)
	    return k;
	else
		return flag;
}
int
show_vrrp_switch_times(unsigned int vrrp_id, unsigned long *switch_times) {
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;
	DBusMessageIter	 iter;

	if(!vrrp_id || vrrp_id > 16) {
		return -1;
	}

	if(NULL == switch_times) {
		return -1;
	}
	
	int instRun =0;
	unsigned int op_ret = 0;
	char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
	char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

	*switch_times = 0;
   
	instRun = ccgi_vrrp_hansi_is_running(vrrp_id);
	if(DCLI_VRRP_INSTANCE_NO_CREATED == instRun) {
		return DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	} else if(DCLI_VRRP_INSTANCE_CHECK_FAILED == instRun) {
		return DCLI_VRRP_RETURN_CODE_ERR;
	}
	
	ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, vrrp_id, vrrp_obj_path);
	ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, vrrp_id, vrrp_dbus_name);

	query = dbus_message_new_method_call(vrrp_dbus_name,
										 vrrp_obj_path,
										 VRRP_DBUS_INTERFACE,
										 VRRP_DBUS_METHOD_SHOW_SWITCH_TIMES);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &vrrp_id,				 
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}
	
	/* handle with abnormal return value */
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	
	if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret
		|| DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret){
			if(DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST == op_ret){
				 return op_ret;
			} else if(DCLI_VRRP_RETURN_CODE_NO_CONFIG == op_ret){
				 return op_ret;
			}	 
		dbus_message_unref(reply);	
		return op_ret;
	}
	
	/* get  backup_switch_times */
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, switch_times);
	
	dbus_message_unref(reply);
	
	return 0;
}

int show_vrrp_runconfig_by_hansi_web(int slotid,char *insid, DBusConnection *connection,char **info)
//-1:instance_id<0或instance_id>16;-2:Hansi instance  not created;-3:NULL == reply;-4:dbus_message_get_args error
{
	unsigned int op_ret = 0;
	int profile = 0;
	unsigned int slot_id = slotid;
	profile = (unsigned int)strtoul(insid,0,10);
    
		
	if((profile < 0)||(profile > 16))
	{
		return -1;
	}
	/* in config node or hansi node, show special instance */	
		DBusMessage *query = NULL;
		DBusMessage *reply = NULL;
		DBusError err;
	
		char *showStr = NULL;
		char tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	
		int instRun = 0;
		char vrrp_obj_path[DCLI_VRRP_OBJPATH_LEN] = {0};
		char vrrp_dbus_name[DCLI_VRRP_DBUSNAME_LEN] = {0};	

		/* [1] check if had process has started before or not */
		instRun = ccgi_vrrp_hansi_is_running_dis(slot_id,0,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			return -2;
		}

		memset(vrrp_obj_path, 0, DCLI_VRRP_OBJPATH_LEN);
		memset(vrrp_dbus_name, 0, DCLI_VRRP_DBUSNAME_LEN);
		memset(tmpBuf, 0, SHOWRUN_PERLINE_SIZE);
	
		ccgi_vrrp_splice_objpath_string(VRRP_DBUS_OBJPATH, profile, vrrp_obj_path);
		ccgi_vrrp_splice_dbusname(VRRP_DBUS_BUSNAME, profile, vrrp_dbus_name);
	
		query = dbus_message_new_method_call(vrrp_dbus_name,
											 vrrp_obj_path,
											 VRRP_DBUS_INTERFACE,
											 VRRP_DBUS_METHOD_SHOW_RUNNING);
		dbus_error_init(&err);
	
		reply = dbus_connection_send_with_reply_and_block (connection, query, -1, &err);
	
		dbus_message_unref(query);
		if (NULL == reply) {
			if(dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return -3;
		}
	
		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_STRING, &showStr,
						DBUS_TYPE_INVALID)) 
		{
			fprintf(stderr,"showStr=%s\n",showStr);
			*info=showStr;
		}
		else {
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			return -4;
		}
	
		dbus_message_unref(reply);
		return 0;	
}

int config_delete_hansi_cmd_web(int slot,char *ins,DBusConnection *connection)
//0:success;	-1:NULL == reply; -2:hansi not exist;-3:failed;-4:dbus_message_get_args error
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int hmd_ret = 0;
	unsigned int profile =0;
	unsigned int slot_id = slot;
	int retu=0;
	profile=(unsigned int)strtoul(ins,0,10);

        query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_REMOTE_HANSI);

    	dbus_error_init(&err);
    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&profile,
    							 DBUS_TYPE_UINT32,&slot_id,
    							 DBUS_TYPE_INVALID);
    	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return -1;
    	}
    	if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&hmd_ret,
    				DBUS_TYPE_INVALID))
    	{	
            if(hmd_ret == 0)
				retu=0;
			else if(hmd_ret == 2)
            	retu=-2;
            else
                retu=-3;
    	} 
    	else 
    	{	
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
                retu=-4;
    		}
    	}
    	dbus_message_unref(reply);	
	return retu;
}	

int show_vrrp_runconfig_cmd_by_ins(int slot,int ins)
	//-1:error; 0:INSTANCE_NO_CREATED; 1:INSTANCE_CREATED
{
	unsigned int op_ret = 0;
	int profile = ins;
	unsigned int slot_id = slot;
			
	if((profile < 1)||(profile > 16))
	{
		return -1;
	}
	
	op_ret = ccgi_vrrp_hansi_is_running_dis(slot,0,ins);

	return op_ret;
}

#ifdef __cplusplus
}
#endif


