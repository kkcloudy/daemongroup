#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <syslog.h>
#include "hmd.h"
#include "HmdDbus.h"
#include "HmdThread.h"
#include "HmdLog.h"
#include "hmd/hmdpub.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "wcpss/waw.h"
#include "HmdDbusHandler.h"

/*fengwenchao add 20120704*/
int parse_int_ve(char* str, unsigned int* slotid, unsigned int *vlanid, unsigned int *vlanid2,char *cid, unsigned int *port){ /*fengwenchao add "vlanid2" for axsszfi-1506 */
	char c;
	char *tmp = NULL;
	char *endptr = NULL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*slotid= strtoul(str,&endptr,10);
		
		if(endptr[0] == '.'){
			tmp = endptr+1;
			*vlanid= strtoul(tmp,&endptr,10);
			/*fengwenchao modify 20130325 for axsszfi-1506 begin*/
			if(endptr[0] == '.')
			{
				tmp = endptr+1;
				*vlanid2 = strtoul(tmp,&endptr,10);
			if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return -1;
			}
			else if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return -1;
			/*fengwenchao modify end*/
			return 1;
		}
		else if((endptr[0] == 'f')||(endptr[0] == 's')){
			*cid = endptr[0];
			tmp = endptr+1;
			*port = strtoul(tmp,&endptr,10);
			/*fengwenchao modify 20130325 for axsszfi-1506 begin*/
			if(endptr[0] == '.'){
				tmp = endptr+1;
				*vlanid= strtoul(tmp,&endptr,10);
				if(endptr[0] == '.')
				{
					tmp = endptr+1;
					*vlanid2 = strtoul(tmp,&endptr,10);
				if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return -1;
				}
				else if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return -1;
				return 2;
			}
			else if((endptr[0] == '\0')||(endptr[0] == '\n'))
				return 2;
			/*fengwenchao modify end*/			
			return -1;
		}
	}
	
	return -1;
}

int check_ve_interface(char *ifname, char *name){
	
	int sockfd = -1;
	unsigned int slotid = 0;
	unsigned int vlanid = 0;
	unsigned int vlanid2 = 0;//fengwenchao add 20130325 for axsszfi-1506 
	unsigned int port = 0;
	char cpu = 'f';
	char *cpu_id = &cpu; 
	struct ifreq	ifr;
	if (0 != strncmp(ifname, "ve", 2)){
		sprintf(name,"%s",ifname);
		hmd_syslog_info("interface name is %s\n",name); 
		return 0;
	}
	else{
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd <= 0){
			hmd_syslog_err("%s,%d,sockfd=%d.\n",__func__,__LINE__,sockfd); 
			return -1;
		}
		strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name)); 		
		
		if(parse_int_ve(ifname+2,&slotid,&vlanid,&vlanid2,cpu_id,&port)== 1)//fengwenchao add "vlanid2" for axsszfi-1506 
		{
			hmd_syslog_info("slotid = %d,vlanid = %d\n",slotid,vlanid); 
			hmd_syslog_info("cpu_id = %c,port = %d\n",*cpu_id,port); 

			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				hmd_syslog_err("SIOCGIFINDEX error\n"); 

				//convert to new ve name
				/*fengwenchao modify "vlanid2" for axsszfi-1506 begin*/
				if(slotid < 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve0%df1.%d",slotid,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve0%df1.%d.%d",slotid,vlanid,vlanid2);
				}
				else if(slotid >= 10)
				{	
					if(vlanid2  == 0)
						sprintf(name,"ve%df1.%d",slotid,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve%df1.%d.%d",slotid,vlanid,vlanid2);
				}
				/*fengwenchao modify end*/
				hmd_syslog_info("ve name is %s\n",name); 

				memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
				strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name)); 		
				if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
					hmd_syslog_err("SIOCGIFINDEX error\n"); 
					close(sockfd);
					return -1;	//the new ve interface doesn't exist
				}		
				close(sockfd);
				return 0;	//the new ve interface exists
			}
			else{
				sprintf(name,"%s",ifname);
				hmd_syslog_info("old ve name is %s\n",name); 
				close(sockfd);
				return 0;//the old ve interface exists
			}
		}
		else if(parse_int_ve(ifname+2,&slotid,&vlanid,&vlanid2,cpu_id,&port)== 2)//fengwenchao add "vlanid2" for axsszfi-1506 
		{
			hmd_syslog_info("slotid = %d,vlanid = %d\n",slotid,vlanid); 
			hmd_syslog_info("cpu_id = %c,port = %d\n",*cpu_id,port); 

			if(vlanid == 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d",slotid,*cpu_id,port);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d",slotid,*cpu_id,port);
			}
			else if(vlanid > 0){
				/*fengwenchao modify "vlanid2" for axsszfi-1506 begin*/
				if(slotid < 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve0%d%c%d.%d",slotid,*cpu_id,port,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve0%d%c%d.%d.%d",slotid,*cpu_id,port,vlanid,vlanid2);
				}
				else if(slotid >= 10)
				{
					if(vlanid2  == 0)
						sprintf(name,"ve%d%c%d.%d",slotid,*cpu_id,port,vlanid);
					else if(vlanid2  > 0)
						sprintf(name,"ve%d%c%d.%d.%d",slotid,*cpu_id,port,vlanid,vlanid2);
				}
				/*fengwenchao modify end*/
			}
			hmd_syslog_info("ve name is %s\n",name); 
		
			memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
			strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));		
			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				hmd_syslog_err("SIOCGIFINDEX error\n"); 
				close(sockfd);
				return -1;	//the new ve interface doesn't exist
			}		

			close(sockfd);
			return 0;	//the new ve interface exists
		}
		else{
			hmd_syslog_err("the ve name is wrong\n"); 
			close(sockfd);
			return -1;
		}
	}
		
}

/*fengwenchao add end*/
int CheckAndCreateHansi(struct HmdMsgQ HMsgq){
	unsigned int profile = HMsgq.mqinfo.InstID;
	unsigned int local = HMsgq.mqinfo.local;
	int instRun = 0;
	char cmd[SYS_COMMAND_LEN] = {0};
	int cr_timeout = 0;
	int check_result = INSTANCE_CREATED;
	char service_name[4][4] = {"had","wid", "asd", "wsm"};
	unsigned int service_index = 0;

	/* system to fork had process */
	instRun = vrrp_hansi_is_running(profile);
	hmd_syslog_info("%s,instRun=%d,profile=%d\n",__func__,instRun,profile);
	if(INSTANCE_NO_CREATED == instRun) {
		
		sprintf(cmd, "sudo /etc/init.d/had start %d &", profile);
		if (system(cmd)) {
			hmd_syslog_err("create hansi %d faild.\n", profile);
			return HMD_DBUS_WARNNING;
		}

		/* check wheather had instance started completely. */
		while (1) {
			cr_timeout++;
			check_result = check_hansi_service_started(service_name[0], profile);
			hmd_syslog_err("%s,1,check_result=%d.\n",__func__,check_result);
			if (INSTANCE_NO_CREATED == check_result) {
				//vty_out(vty, "create %s instrance %d time %d s.\n", service_name[0], profile, cr_timeout);
				/* 3524 1s */
				if (4 == cr_timeout) {
					hmd_syslog_err("create %s instrance %d timeout.\n", service_name[0], profile);
					return HMD_DBUS_WARNNING;
				}
				sleep(1);
				continue;
			}else if (INSTANCE_CHECK_FAILED == check_result) {
				hmd_syslog_err("%s,2,check_result=%d.\n",__func__,check_result);
				return HMD_DBUS_WARNNING;
			}else if (INSTANCE_CREATED == check_result) {
				hmd_syslog_err("%s,3,check_result=%d.\n",__func__,check_result);
				//vty_out(vty, "create %s instrance %d used %d s.\n", service_name[0], profile, cr_timeout);
				cr_timeout = 0;
				break;
			}
		}		
		hmd_syslog_err("%s,check_result=%d,profile=%d.\n",__func__,check_result,profile);
		/* profile = 0, it's not necessary create wcpss process */
		if (0 != profile) {
			hmd_syslog_err("%s,check_result=%d.\n",__func__,check_result);
			/* add for create wcpss process */
			memset(cmd,0,SYS_COMMAND_LEN);
			sprintf(cmd, "sudo /etc/init.d/wcpss start %d %d &",local, profile);
			//printf("wcpss %s\n",cmd);
			if (system(cmd)) {
				hmd_syslog_err("create wcpss %d faild.\n", profile);
				return HMD_DBUS_WARNNING;
			}
			/*add for eag  shaojunwu 20110620*/
			memset(cmd, 0, SYS_COMMAND_LEN);
			sprintf(cmd,"sudo /etc/init.d/eag_modules start %d %d &",local, profile);
			if (system(cmd)) {
				hmd_syslog_err("create eag %d faild.\n", profile);
			}			
			/*end for eag*/

			/* add for pppoe lixiang 20120817 */
		#ifndef _VERSION_18SP7_ 	
			memset(cmd, 0, SYS_COMMAND_LEN);
			sprintf(cmd,"sudo /etc/init.d/pppoe start %d %d &",local, profile);
			if (system(cmd)) {
				hmd_syslog_err("create pppoe %d faild.\n", profile);
			}
		#endif	
			/* end for pppoe */
			
			/* check wheather wcpss instance started completely.
			 * wsm not support multi-process.
			 */
			for (service_index = 1; service_index < 3; service_index++) {
				cr_timeout = 0;
				while (1) {
					cr_timeout++;
					check_result = check_hansi_service_started(service_name[service_index], profile);
					if (INSTANCE_NO_CREATED == check_result) {
						//vty_out(vty, "create %s instrance %d time %d s.\n", service_name[service_index], profile, cr_timeout);
						if (100 == cr_timeout) {
							hmd_syslog_err("create %s instrance %d timeout.\n", service_name[service_index], profile);
							return HMD_DBUS_WARNNING;
						}
						sleep(1);
						continue;
					}else if (INSTANCE_CHECK_FAILED == check_result) {
						return HMD_DBUS_WARNNING;
					}else if (INSTANCE_CREATED == check_result) {
						//vty_out(vty, "create %s instrance %d used %d s.\n", service_name[service_index], profile, cr_timeout);
						cr_timeout = 0;
						break;
					}
				}
			}
			sleep(3);	/* for wait asd dbus thread create ok */
		}
	}
	return HMD_DBUS_SUCCESS;
}

/**
 * check if had process has started before or not
 * now we use 'ps -ef | grep had #' format to indicate 
 * the corresponding process is running or not
 */
int vrrp_hansi_is_running
(
	unsigned int profileId
)
{
	char commandBuf[SYS_COMMAND_LEN] = {0};
	FILE *pidmap_file = NULL, *hadpid_file = NULL, *pathpid_file = NULL;
	char msg[SYS_COMMAND_LEN] = {0}, str_pid[SYS_COMMAND_LEN] = {0}, pathpid[SYS_COMMAND_LEN] = {0};
	char *sep=" ", *token = NULL;
	int hadpid = 0;
	hmd_syslog_info("%s,profileId=%d.\n",__func__,profileId);
	sprintf(commandBuf, "/var/run/had%d.pid", profileId);
	hadpid_file = fopen(commandBuf, "r");
	if (!hadpid_file) {
		return INSTANCE_NO_CREATED;
	}
	fclose(hadpid_file);
	
	memset(commandBuf, 0, SYS_COMMAND_LEN);
	sprintf(commandBuf, "/var/run/had%d.pidmap", profileId);
	pidmap_file = fopen(commandBuf, "r");
	if (!pidmap_file) {
		return INSTANCE_NO_CREATED;
	}
	while (fgets(msg, SYS_COMMAND_LEN, pidmap_file)) {
		token = strtok(msg, sep);
		while (token) {
			strcpy(str_pid, token);
			token = strtok(NULL, sep);
		}
		hadpid = strtoul(str_pid, NULL, 10);
		sprintf(pathpid, "/proc/%d", hadpid);
		pathpid_file = fopen(pathpid, "r");
		if (!pathpid_file) {
			fclose(pidmap_file);
			return INSTANCE_NO_CREATED;
		}
		fclose(pathpid_file);
		memset(msg, 0, SYS_COMMAND_LEN);
		memset(pathpid, 0, SYS_COMMAND_LEN);
	}
	
	fclose(pidmap_file);
	
	if (!hadpid) {
		return INSTANCE_NO_CREATED;
	}
	return INSTANCE_CREATED;
}

/**
 * check if had/wid/asd/wsm process has started completely.
 * now we use 'ps -ef | grep \"%s %d$\"' format to indicate 
 * the corresponding process is running or not
 */
int check_hansi_service_started
(
	char *service_name,
	unsigned int profileId
)
{
	int ret = 0;
	int fd = -1;
	int iCnt = 0;	
	char commandBuf[SYS_COMMAND_LEN] = {0};
	char readBuf[4] = {0};

	if (!service_name) {
		return INSTANCE_CHECK_FAILED;
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
		hmd_syslog_err("%% Check %s instance %d failed!\n", service_name, profileId);
		return INSTANCE_CHECK_FAILED;
	}

	/* get the process # */
	memset(commandBuf, 0, SYS_COMMAND_LEN);
	sprintf(commandBuf, "/var/run/%s%d.boot", service_name, profileId);
	if ((fd = open(commandBuf, O_RDONLY))< 0) {
		hmd_syslog_err("%% Check %s instance %d count failed!\n", service_name, profileId);
		return INSTANCE_CHECK_FAILED;
	}

	memset(readBuf, 0, 4);
	read(fd, readBuf, 4);
	iCnt = strtoul(readBuf, NULL, 10);
	//vty_out(vty, "%s %d thread count %d\n", service_name, profileId, iCnt);

	if (!strncmp(service_name, "had", 3)) {
		if (VRRP_THREADS_CNT == iCnt) {
			ret = INSTANCE_CREATED;
		}
		else {
			ret = INSTANCE_NO_CREATED;
		}
	}else {
		/* for wcpss, include wid/asd/wsm process */
		if (3 <= iCnt) {
			ret = INSTANCE_CREATED;
		}
		else {
			ret = INSTANCE_NO_CREATED;
		}
	}	
	
	/* release file resources */
	close(fd);

	memset(commandBuf, 0, SYS_COMMAND_LEN);
	sprintf(commandBuf, "sudo rm /var/run/%s%d.boot", service_name, profileId);
	system(commandBuf);

	return ret;
}

void notice_hmd_server_hansi_info_update(int InstID,struct Hmd_Inst_Mgmt * Inst){
	struct HmdMsg tmsg;
	memset(&tmsg,0,sizeof(struct HmdMsg));
	int state = 0;
	int fd = -1;
	tmsg.D_SlotID = MASTER_SLOT_NO;
	tmsg.S_SlotID = HOST_SLOT_NO;
	tmsg.InstID = InstID;
	if(HOST_BOARD->Hmd_Inst[InstID]){
		state = HOST_BOARD->Hmd_Inst[InstID]->InstState;
		fd = HOST_BOARD->Hmd_Inst[InstID]->tipcfd;
	}else{
		hmd_syslog_err("%s %d,HOST_BOARD->Hmd_Inst[%d] is NULL!\n",__func__,__LINE__,InstID);
		return ;
	}
	tmsg.op = HMD_HANSI_UPDATE;
	tmsg.type = HMD_HANSI;
	tmsg.u.updateinfo.state = Inst->InstState;
	tmsg.u.updateinfo.isActive = Inst->isActive;
	tmsg.u.updateinfo.Inst_UNum = Inst->Inst_UNum;
	memcpy(&(tmsg.u.updateinfo.Inst_Hb), &(Inst->Inst_Hb),sizeof(struct Inst_Interface));
	memcpy(tmsg.u.updateinfo.Inst_Uplink, Inst->Inst_Uplink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
	tmsg.u.updateinfo.Inst_DNum = Inst->Inst_DNum;
	memcpy(tmsg.u.updateinfo.Inst_Downlink, Inst->Inst_Downlink,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
	tmsg.u.updateinfo.Inst_GNum = Inst->Inst_GNum;
	memcpy(tmsg.u.updateinfo.Inst_Gateway, Inst->Inst_Gateway,MAX_IFNAME_NUM*sizeof(struct Inst_Interface));
	
	if (0 > sendto(fd, &tmsg, sizeof(tmsg), 0,(struct sockaddr*)&(HOST_BOARD->tipcaddr), sizeof(HOST_BOARD->tipcaddr))) 
	{		
		hmd_syslog_err("%s %d,sendo to faile,tipc.addr.instace:%d!\n",__func__,__LINE__,fd,HOST_BOARD->tipcaddr.addr.name.name.instance);
	}
	return ;
}
int femto_service_switch_check(unsigned int type, unsigned int slotid, unsigned insid, unsigned int islocal, unsigned int femto_switch)
{
	int ret = HMD_DBUS_SUCCESS;
	if(islocal)	//local hansi
	{
		if(HMD_BOARD[slotid]->Hmd_Local_Inst[insid] != NULL)
		{
			if(type == IU_TYPE)	//iu
			{
				if(femto_switch == HMD_BOARD[slotid]->Hmd_Local_Inst[insid]->iu_service_switch)
					ret = HMD_DBUS_FEMTO_SERVICE_CONFLICT;
			}
			else					//iuh
			{
				if(femto_switch == HMD_BOARD[slotid]->Hmd_Local_Inst[insid]->iuh_service_switch)
					ret = HMD_DBUS_FEMTO_SERVICE_CONFLICT;
			}
		}
		else
			ret = HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST;
	}
	else			//remote hansi
	{
		if(HMD_BOARD[slotid]->Hmd_Inst[insid] != NULL)
		{
			if(type == IU_TYPE)	//iu
			{
				if(femto_switch == HMD_BOARD[slotid]->Hmd_Inst[insid]->iu_service_switch)
					ret = HMD_DBUS_FEMTO_SERVICE_CONFLICT;
			}
			else					//iuh
			{
				if(femto_switch == HMD_BOARD[slotid]->Hmd_Inst[insid]->iuh_service_switch)
					ret = HMD_DBUS_FEMTO_SERVICE_CONFLICT;
			}
		}
		else
			ret = HMD_DBUS_HANSI_ID_NOT_EXIST;
	}
	return ret;
}
int femto_service_state_check(unsigned int type, unsigned int slotid, unsigned insid, unsigned int islocal)
{
	int ret = HMD_DBUS_SUCCESS;
	ret = femto_service_switch_check(type, slotid, insid, islocal, 0);
	if(ret == HMD_DBUS_FEMTO_SERVICE_CONFLICT)
	{
		if(type == IU_TYPE)
			ret = HMD_DBUS_IU_NOT_IN_SERVICE;
		else
			ret = HMD_DBUS_IUH_NOT_IN_SERVICE;
	}
	return ret;
}

int femto_service_switch(unsigned int type, unsigned int slotid, unsigned int insid, int islocal, unsigned int service_switch)
{
	int ret = HMD_DBUS_SUCCESS;
	char cmd[SYS_COMMAND_LEN] = {0};
	//sudo /etc/init.d/iuh start 0 3 &
	sprintf(cmd, "sudo /etc/init.d/%s %s %d %d &", \
	type?"ranapproxy":"iuh", service_switch?"start":"stop", islocal, insid);
	if(system(cmd))
	{
		hmd_syslog_err("%s %s %d %d service faild.\n", \
			type?"ranapproxy":"iuh", service_switch?"start":"stop", islocal, insid);
		ret = HMD_DBUS_FEMTO_SERVICE_ERROR;
	}
	else
	{
		if(islocal)	//local hansi
		{
			if(HMD_BOARD[slotid]->Hmd_Local_Inst[insid] != NULL)
			{
				if(type == IU_TYPE)	//iu
				{
					HMD_BOARD[slotid]->Hmd_Local_Inst[insid]->iu_service_switch = service_switch;
				}
				else					//iuh
				{
					HMD_BOARD[slotid]->Hmd_Local_Inst[insid]->iuh_service_switch = service_switch;
				}
			}
		}
		else			//remote hansi
		{
			if(HMD_BOARD[slotid]->Hmd_Inst[insid] != NULL)
			{
				if(type == IU_TYPE)	//iu
				{
					HMD_BOARD[slotid]->Hmd_Inst[insid]->iu_service_switch = service_switch;
				}
				else					//iuh
				{
					HMD_BOARD[slotid]->Hmd_Inst[insid]->iuh_service_switch = service_switch;
				}
			}
		}
	}
	return ret;
}

