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
* hmd.c
*
*
* CREATOR:
* zhanglei@autelan.com
*
* DESCRIPTION:
* Hansi Management module
*
*
*******************************************************************************/


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include "hmd.h"
#include "board/board_define.h"
#include "hmd/hmdpub.h"
#include "HmdThread.h"
#include "dbus/hmd/HmdDbusPath.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "dbus/wcpss/ACDBusPath.h"
#include "dbus/asd/ASDDbusPath.h"
#include "HmdManagement.h"
#include "HmdStateListen.h"
#include "HmdDbus.h"
#include "HmdTimeLib.h"
#include "HmdMonitor.h"
#include "HmdLog.h"
int HOST_SLOT_NO = 0;//server==0;client==1
int HOST_BOARD_TYPE = 0;//server==1;client==0
int isMaster = 0;
int	isDistributed = 0;
int isActive = 0;
int MASTER_SLOT_NO = 0;
int MASTER_BACKUP_SLOT_NO = 0;
int LicenseCount = 9;
int HANSI_CHECK_OP = 1;
int HANSI_TIMER_CONFIG_SAVE = 0; //fengwenchao add 20130412 for hmd timer config save
int HANSI_TIMER = 1800; //fengwenchao add 20130412 for hmd timer config save
int WTP_NUM = 4096+1;	//fengwenchao add 20130412 for hmd timer config save
int DHCP_RESTART_FLAG = 0; //DHCP AUTO 	restart flag  supf add 20130729
int DHCP_RESTART_OPEN = 0; //DHCP AUTO 	restart OPEN  supf add 20130729
struct Hmd_Board_Info *HOST_BOARD;
struct Hmd_For_Dhcp_restart *DHCP_MONITOR;
struct Hmd_Board_Info *HMD_BOARD[MAX_SLOT_NUM];
struct Hmd_L_Inst_Mgmt_Summary *HMD_L_HANSI[MAX_INSTANCE];
struct LicenseMgmt	*LICENSE_MGMT;
char MSGQ_PATH[PATH_LEN] = "/var/run/hmd/hmsgq";
int HMDMsgqID = 0;
unsigned int service_tftp_state = 0;//service tftp (0--disable;1--enable)
unsigned int service_ftp_state = 0;//service ftp  (0--disable;1--enable)


int parse_int_ID(char* str, int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if((c=='0')&&(str[1]!='\0')){
			 return -1;
		}
		else if((endptr[0] == '\0')||(endptr[0] == '\n')){
			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
}

/*fengwenchao copy from 1318 for AXSSZFI-839*/
int Set_Interface_binding_Info(char * ifname,char flag){/*add flag--1,clear flag--0*/
	//hmd_syslog_info(" accessinto %s \n",__func__);
	int sockfd=-1;
	struct ifreq	ifr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd <= 0){
		hmd_syslog_err("%s,%d,invalid fd:%d.\n",__func__,__LINE__,sockfd);
		return HMD_DBUS_ERROR;
	}else{
	}
	memset(&ifr,0,sizeof(struct ifreq));
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));			

	if(ioctl(sockfd, SIOCGIFUDFFLAGS, &ifr) == -1){
		hmd_syslog_err("get interface %s flags ioctl failed %s!\n",ifname);
	}else{
		hmd_syslog_info("get interface %s ifr.ifr_flags:%x",ifname,ifr.ifr_flags);
	}
	if(flag == 0){
		ifr.ifr_flags &= ~(IFF_BINDING_FLAG);
	}else{
		ifr.ifr_flags |= IFF_BINDING_FLAG;
	}
	hmd_syslog_info("%s,%d,set interface %s ifr.ifr_flags:%x,flag=%d.",__func__,__LINE__,ifname,ifr.ifr_flags,flag);
	if(ioctl(sockfd, SIOCSIFUDFFLAGS, (char *)&ifr) == -1){
		hmd_syslog_err("%s,%d,set interface %s flags ioctl failed!\n",__func__,__LINE__,ifname);
	}else{
		hmd_syslog_info("%s,%d,set interface %s ifr.ifr_flags:%x",__func__,__LINE__,ifname,ifr.ifr_flags);
	}
	close(sockfd);
	return HMD_DBUS_SUCCESS;
}

int Set_hmd_bakup_foreve_config(struct HmdMsg *hmdmsg){
	hmd_syslog_info(" accessinto %s \n",__func__);
	int ret = 0;
	int instid = 0,h_type = 0,sw_state = 0;
	instid = hmdmsg->InstID;
	h_type = hmdmsg->local;
	sw_state = hmdmsg->u.HANSI.hmdforevesw;
	hmd_syslog_info("%s,%d,instid=%d,h_type=%d,set remote hansi HmdBakForever sw %s. \n",__func__,__LINE__,instid,h_type,(sw_state == 1)?"enable":"disable");
	if((instid > 0)&&(instid < MAX_INSTANCE)){
		if(0 == h_type){
			if(HOST_BOARD->Hmd_Inst[instid])
			{		
				HOST_BOARD->Hmd_Inst[instid]->HmdBakForever = sw_state;
				hmd_syslog_info("%s,%d,set remote hansi HmdBakForever sw %s. \n",__func__,__LINE__,(sw_state == 1)?"enable":"disable");
			}
			else
			{
				ret = HMD_DBUS_ID_NO_EXIST; 	
			}
		}else{
			if(HOST_BOARD->Hmd_Local_Inst[instid])
			{		
				HOST_BOARD->Hmd_Local_Inst[instid]->HmdBakForever = sw_state;
				hmd_syslog_info("%s,%d,set localhansi HmdBakForever sw %s. \n",__func__,__LINE__,(sw_state == 1)?"enable":"disable");
			}
			else
			{
				ret = HMD_DBUS_ID_NO_EXIST; 	
			}
		}
	}else{
		ret = HMD_DBUS_NUM_OVER_MAX; 	
	}
	return HMD_DBUS_SUCCESS;
}

/*fengwenchao copy end*/
int get_dir_wild_file_count(char *dir, char *wildfile)
{
	DIR *dp = NULL;
	struct dirent *dirp;
	int wildfilecount = 0;
	dp = opendir(dir);
	if(dp == NULL)
	{
		return wildfilecount;
	}
	while((dirp = readdir(dp)) != NULL)
	{
		if((memcmp(dirp->d_name,wildfile,strlen(wildfile))) ==  0)
		{
			wildfilecount++;
		}
	}
	printf("last count = %d\n",wildfilecount);
	closedir(dp);
	return wildfilecount;
}

int read_ac_info(char *FILENAME,char *buff)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,DEFAULT_LEN);	
	
	if (len < 0) {
		close(fd);
		return 1;
	}
	if(len != 0)
	{
		if(buff[len-1] == '\n')
		{
			buff[len-1] = '\0';
		}
	}
	close(fd);
	return 0;
}

int read_ac_file(char *FILENAME,char *buff,int blen)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,blen);	
	
	if (len < 0) {
		close(fd);
		return 1;
	}
	if(len != 0)
	{
		if(buff[len-1] == '\n')
		{
			buff[len-1] = '\0';
		}
	}
	close(fd);
	return 0;
}
/*fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter*/
void read_master_ap_max_counter(int slot_id,int* ap_num)
{
	int fd,len =0;
	char master_ap_max_counter_path[] = "/dbm/product/slot/slot";
	char buff[DEFAULT_LEN] ={0};
	char buff_paths[DEFAULT_LEN] ={0};
	char counter_share[]="/board_ap_counter";
	sprintf(buff_paths,"%s%d%s",master_ap_max_counter_path,slot_id,counter_share);

	fd = open(buff_paths,O_RDONLY);
	if(fd <0)
	{
		hmd_syslog_err("%s fd <0\n",__func__);
		*ap_num= 1024;
		return;
	}
	
	len = read(fd,buff,DEFAULT_LEN);
	if (len < 0) {
		close(fd);
		hmd_syslog_err("%s len <0\n",__func__);
		*ap_num = 1024;
		return;
	}
	close(fd);
	parse_int_ID(buff,ap_num);
	if(*ap_num >= WTP_NUM)
	{
		hmd_syslog_err("%s we find *ap_num %d >= WTP_NUM\n",__func__,*ap_num);
		*ap_num = WTP_NUM-1;	
	}
	return;
}
/*fengwenchao add end*/

void hmd_license_init(){
	int licensecount = 0;
	int i = 0;	
	char WTP_COUNT_PATH_BASE[] = "/devinfo/maxwtpcount";
	char LIC_REQ_PATH_BASE[] = "/devinfo/licreq";
	char buf_base[DEFAULT_LEN];
	char strdir[DEFAULT_LEN];
		
	licensecount = get_dir_wild_file_count("/devinfo","maxwtpcount");
	if(licensecount < 9){
		LicenseCount = 9;
	}else{
		LicenseCount = licensecount;
	}

	LICENSE_MGMT = malloc((LicenseCount+1)*sizeof(struct LicenseMgmt));
	memset(LICENSE_MGMT, 0, (LicenseCount+1)*sizeof(struct LicenseMgmt));
	
	for(i=0; i<licensecount; i++)
	{
		
		memset(strdir,0,DEFAULT_LEN);
		memset(buf_base,0,DEFAULT_LEN);	

		if(i == 0)
		{
			if(read_ac_info(WTP_COUNT_PATH_BASE,buf_base) == 0)
			{
				if(parse_int_ID(buf_base, &LICENSE_MGMT[i].total_num)==-1)
					LICENSE_MGMT[i].total_num = DEFAULT_LICENSE_NUM;
			}
			else
			{

				LICENSE_MGMT[i].total_num = DEFAULT_LICENSE_NUM;
			}
			
			LICENSE_MGMT[i].free_num = LICENSE_MGMT[i].total_num;
			
			memset(buf_base,0,DEFAULT_LEN); 
			if(read_ac_info(LIC_REQ_PATH_BASE,buf_base) == 0)
			{
				memcpy(LICENSE_MGMT[i].licreq, buf_base, LICREQ_LEN);
			}
		}
		else
		{
			sprintf(strdir,"/devinfo/maxwtpcount%d",i+1);
			if(read_ac_info(strdir,buf_base) == 0)
			{
				if(parse_int_ID(buf_base, &LICENSE_MGMT[i].total_num)==-1)
				LICENSE_MGMT[i].total_num = DEFAULT_LICENSE_NUM;
			}
			else
			{
				LICENSE_MGMT[i].total_num = DEFAULT_LICENSE_NUM;
			}
			
			LICENSE_MGMT[i].free_num = LICENSE_MGMT[i].total_num;
			memset(strdir,0,DEFAULT_LEN);			
			sprintf(strdir,"/devinfo/licreq%d",i+1);
			memset(buf_base,0,DEFAULT_LEN); 
			if(read_ac_info(strdir,buf_base) == 0)
			{
				memcpy(LICENSE_MGMT[i].licreq, buf_base, LICREQ_LEN);
			}
		}

	}
	
	return;
}

void hmd_pid_write(int slot_num)
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	pid_t myPid = 0;
	int fd;
	sprintf(pidPath,"%s%d%s", "/var/run/hmd/hmd", \
				slot_num, ".pid");
		
	fd = open(pidPath, O_RDWR|O_CREAT);
	if(fd <= 0){
		hmd_syslog_err("%s,%d,invalid fd:%d.\n",__func__,__LINE__,fd);
	}else{
	myPid = getpid();	
	sprintf(pidBuf,"%d\n", myPid);
	write(fd, pidBuf, strlen(pidBuf));
	close(fd);
	}
	return;
}

void hmd_pid_write_v2(char *name,int slot_num)
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	char idbuf[128] = {0};
	pid_t myPid = 0;
	static int fd;
	static unsigned char opened = 0;
	sprintf(pidPath,"%s%d%s", "/var/run/hmd/hmd_thread", \
				slot_num, ".pid");
	if(!opened){
		
		fd = open(pidPath, O_RDWR|O_CREAT);
		hmd_syslog_info("fd=%d,%s,path:%s\n",fd,__func__,pidPath);
		if(fd <= 0){
			hmd_syslog_err("<err>,%s\n",__func__);
			return;
		}
		opened = 1;
	}
		
	sprintf(idbuf,"HMD[%d]%s",slot_num,name);
	myPid = getpid();
	sprintf(pidBuf,"%s-%d\n",idbuf,myPid);
	write(fd, pidBuf, strlen(pidBuf));
	return;
}

void hmd_pid_write_v3(int slot_num,int instID,int islocal)
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	pid_t myPid = 0;
	int fd = -1;
	sprintf(pidPath,"%s%d-%d-%d%s", "/var/run/hmd/hmd", \
				islocal,slot_num,instID, ".pid");
		
	fd = open(pidPath, O_RDWR|O_CREAT);
	if(fd > 0){
	myPid = getpid();	
	sprintf(pidBuf,"%d\n", myPid);
	write(fd, pidBuf, strlen(pidBuf));
	close(fd);
	}else{
		hmd_syslog_err("%s,%d,invalid fd:%d.\n",__func__,__LINE__,fd);
	}
	return;
}


int read_file_info(char *FILENAME,char *buff)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,DEFAULT_LEN);	
	
	if (len < 0) {
		close(fd);
		return 1;
	}
	if(len != 0)
	{
		if(buff[len-1] == '\n')
		{
			buff[len-1] = '\0';
		}
	}
	close(fd);
	return 0;
}

int HmdNoticeToClient(int slotid,int InstID,int localid,int op){
	struct sockaddr_tipc * addr = &(HMD_BOARD[slotid]->tipcaddr);
	int fd = HMD_BOARD[slotid]->tipcfd;
	if(fd <= 0){
		hmd_syslog_err("%s,%d,invaid fd:%d.\n",__func__,__LINE__,fd);
		return 0;
	}
	/* Send connectionless "hello" message: */
	
	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));
	msg.type=	(localid == 1) ? (HMD_LOCAL_HANSI) : (HMD_HANSI);
	msg.op	=	op;
	msg.D_SlotID = slotid;
	msg.S_SlotID = HOST_SLOT_NO;
	msg.InstID = InstID;
	msg.u.HANSI.slotid = slotid;
	msg.u.HANSI.vrrid = InstID;
	msg.u.HANSI.local = localid;

	
	char buf[4096];
	memset(buf,0,4096);
	int len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);
	
	if (0 > sendto(fd,buf,sizeof(msg)+1,0,
				   (struct sockaddr*)addr,
				   sizeof(struct sockaddr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
			//exit(1);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	return 0;
}

/*fengwenchao copy from 1318 for AXSSZFI-839*/
int HmdNoticeToClient_ForClearIfname(int slotid,char *ifname,int op)
{
	//hmd_syslog_info(" accessinto %s \n",__func__);
	if(ifname == NULL)
	{
		hmd_syslog_err("%s ,ifname is NULL \n",__func__);
		return 0;
	}
	struct sockaddr_tipc * addr = &(HMD_BOARD[slotid]->tipcaddr);
	int fd = HMD_BOARD[slotid]->tipcfd;
	if(fd <= 0){
		hmd_syslog_err("%s,%d,invaid fd:%d.\n",__func__,__LINE__,fd);
		return 0;
	}
	/* Send connectionless "hello" message: */
	
	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));

	msg.op	=	op;
	msg.D_SlotID = slotid;
	msg.S_SlotID = HOST_SLOT_NO;
	
	memcpy(msg.clear_ifname,ifname,strlen(ifname));
		
	msg.u.HANSI.slotid = slotid;
		
	char buf[4096];
	memset(buf,0,4096);
	int len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);
	
	if (0 > sendto(fd,buf,sizeof(msg)+1,0,
				   (struct sockaddr*)addr,
				   sizeof(struct sockaddr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
			//exit(1);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	return 0;
}
/*fengwenchao copy end*/

int HmdNoticeToClient_ForBakForeverConfig(struct HmdMsg *hmdmsg,int op)
{
	hmd_syslog_info(" accessinto %s \n",__func__);
	struct sockaddr_tipc * addr = &(HMD_BOARD[hmdmsg->D_SlotID]->tipcaddr);
	int fd = HMD_BOARD[hmdmsg->D_SlotID]->tipcfd;
	if(fd <= 0){
		hmd_syslog_err("%s,%d,invaid fd:%d.\n",__func__,__LINE__,fd);
		return 0;
	}
	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));

	msg.op	=	op;
	msg.D_SlotID = hmdmsg->D_SlotID;
	msg.S_SlotID = HOST_SLOT_NO;
	msg.InstID = hmdmsg->InstID;
	msg.local = hmdmsg->local;
		
	msg.u.HANSI.slotid = hmdmsg->u.HANSI.slotid;
	msg.u.HANSI.local = hmdmsg->u.HANSI.local;
	msg.u.HANSI.hmdforevesw = hmdmsg->u.HANSI.hmdforevesw;
	
	char buf[4096];
	memset(buf,0,4096);
	int len = sizeof(msg);
	memcpy(buf,(char*)&msg,len);
	
	if (0 > sendto(fd,buf,sizeof(msg)+1,0,
				   (struct sockaddr*)addr,
				   sizeof(struct sockaddr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	return 0;
}

int HmdSetAssambleHansiMsg(int slotid,int InstID,int localid,int op){
    struct HmdMsgQ msgq;
    msgq.mqinfo.InstID = InstID;
    msgq.mqinfo.local = localid;
    msgq.mqinfo.type = (localid == 1) ? (HMD_LOCAL_HANSI) : (HMD_HANSI);;
    msgq.mqinfo.op = op;
    msgq.mqinfo.D_SlotID = slotid;
	msgq.mqinfo.S_SlotID = HOST_SLOT_NO;
    msgq.mqid = localid*MAX_INSTANCE + InstID;//
    hmd_syslog_info("HMDMsgqID = %d\n",HMDMsgqID);
	if (msgsnd(HMDMsgqID, (struct HmdMsgQ *)&msgq, sizeof(msgq.mqinfo), 0) == -1)
		perror("msgsnd");
    return 0;
}

int configuration_server_to_client(int slotid,int profile, int neighbor_slotid){
	struct sockaddr_tipc * addr = &(HMD_BOARD[slotid]->tipcaddr);
	int fd = HMD_BOARD[slotid]->tipcfd;
	if(fd <= 0){
		hmd_syslog_err("%s,%d,invaid fd:%d.\n",__func__,__LINE__,fd);
		return 0;
	}
	/* Send connectionless "hello" message: */

	struct HmdMsg msg;
	memset(&msg,0,sizeof(struct HmdMsg));
	msg.type=	HMD_LOCAL_HANSI;
	msg.op	=	HMD_HANSI_ENABLE;
	msg.D_SlotID = slotid;
	msg.S_SlotID = HOST_SLOT_NO;
	msg.InstID = profile;
	if((profile < MAX_INSTANCE)&&(HMD_BOARD[slotid]->Hmd_Local_Inst[profile])){
		msg.u.updateinfo.NeighborSlotID = neighbor_slotid;
		msg.u.updateinfo.NeighborActive = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive1;
		msg.u.updateinfo.NeighborState =HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->InstState1;
		msg.u.updateinfo.slot_num = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->slot_num;
		msg.u.updateinfo.state = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->InstState;
		msg.u.updateinfo.isActive = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->isActive;
		msg.u.updateinfo.Inst_DNum = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_DNum;
		msg.u.updateinfo.priority = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->priority; //
		int dlen = sizeof(msg.u.updateinfo.Inst_Downlink);
		memcpy(msg.u.updateinfo.Inst_Downlink,HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_Downlink,dlen);

		msg.u.updateinfo.Inst_UNum = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_UNum;
		int ulen = sizeof(msg.u.updateinfo.Inst_Uplink);
		memcpy(msg.u.updateinfo.Inst_Uplink,HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_Uplink,ulen);

		msg.u.updateinfo.Inst_GNum = HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_GNum;
		int glen = sizeof(msg.u.updateinfo.Inst_Gateway);
		memcpy(msg.u.updateinfo.Inst_Gateway,HMD_BOARD[slotid]->Hmd_Local_Inst[profile]->Inst_Gateway,glen);

	}else{
		return HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST;
	}
	
	char buf2[4096];
	memset(buf2,0,4096);
	int len = sizeof(msg);
	hmd_syslog_info("%s,len=%d.\n",__func__,len);
	memcpy(buf2,(char*)&msg,len);
	
	if (0 > sendto(fd,buf2,sizeof(msg)+1,0,
				   (struct sockaddr*)addr,
				   sizeof(struct sockaddr))){
			perror("Server: Failed to send");
			hmd_syslog_info("Server: %s,Failed to send.\n",__func__);
			//exit(1);
	}else{
		hmd_syslog_info("Server: %s,success to send.\n",__func__);
	}
	printf("\n****** TIPC Server hello program finished ******\n");
	return 0;
}

HMDBool HmdGetMsgQueue(int *msgqid){
	key_t key;

	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {		
		hmd_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		exit(1);
	}
	if ((*msgqid = msgget(key, 0644)) == -1) {
		hmd_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		exit(1);
	}
	return HMD_TRUE;
}

HMDBool HmdInitMsgQueue(int *msgqid){
	key_t key;
	FILE *dp = NULL;
	if(NULL == (dp=fopen(MSGQ_PATH,"w"))){
		hmd_syslog_crit("%s fopen %s",__func__,strerror(errno));
		perror("fopen");
		exit(1);	
	}
		
	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {
		hmd_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		fclose(dp);
		exit(1);
	}
	if ((*msgqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
		hmd_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		fclose(dp);
		exit(1);
	}
	fclose(dp);
	return HMD_TRUE;
}
int HmdDirectoryCheck(char *path){
	DIR *dp = NULL;
	dp = opendir(path);
	if(dp == NULL){
		if (mkdir(path, S_IRWXU | S_IRWXG) < 0) {
			return 0;
		}
	}else{
		closedir(dp);
	}
	return 1;
}
int HmdClearFile(FILE *fp){
	ftruncate(fileno(fp),0);
	return 1;
}
int HmdInstInit(int InstID, int islocaled, int op){
	char BasePath[] = "/var/run/hmd";
	char BaseLocalPath[] = "/var/run/hmd/local";
	char BaseRemotePath[] = "/var/run/hmd/remote";
	char InstPath[128] = {0};
	char cmd[128] = {0};
	int i = 0;
	if(HmdDirectoryCheck(BasePath) == 0){
		return -1; 
	}
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID] == NULL)
			return -1;
		if(HmdDirectoryCheck(BaseLocalPath) == 0){
			return -1; 
		}
		sprintf(InstPath,"%s/inst%d",BaseLocalPath,InstID);
		if(HmdDirectoryCheck(InstPath) == 0){
			return -1; 
		}
		if(op == HMD_HANSI_ENABLE){
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/InstID",InstID,InstPath);			
			system(cmd);
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/state",HOST_BOARD->Hmd_Local_Inst[InstID]->InstState,InstPath);			
			system(cmd);
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/neighborSlotID",HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1,InstPath);
			system(cmd);
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/uplink_num",HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_UNum,InstPath);
			system(cmd);
			for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_UNum; i++){
				memset(cmd, 0, 128);
				sprintf(cmd,"sudo echo %s > %s/uplink%d",HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].ifname,InstPath, i);
				system(cmd);	
			}
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/downlink_num",HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_DNum,InstPath);			
			system(cmd);
			for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_DNum; i++){
				memset(cmd, 0, 128);
				sprintf(cmd,"sudo echo %s > %s/uplink%d",HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].ifname,InstPath, i);
				system(cmd);	
			}
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/gateway_num",HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_GNum,InstPath);			
			system(cmd);
			for(i = 0; i < HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_GNum; i++){
				memset(cmd, 0, 128);
				sprintf(cmd,"sudo echo %s > %s/uplink%d",HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].ifname,InstPath, i);
				system(cmd);
			}
		}else if(op == HMD_DELETE){
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo rm -rf %s/inst%d",BaseLocalPath,InstID);
			system(cmd);
		}else if(op == HMD_STATE_SWITCH){
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/state",HOST_BOARD->Hmd_Local_Inst[InstID]->InstState,InstPath);			
			system(cmd);
		}
	}else{		
		if(HOST_BOARD->Hmd_Inst[InstID] == NULL)
			return -1;
		if(HmdDirectoryCheck(BaseRemotePath) == 0){
			return -1; 
		}
		sprintf(InstPath,"%s/inst%d",BaseRemotePath,InstID);
		if(HmdDirectoryCheck(InstPath) == 0){
			return -1; 
		}
		if(op == HMD_HANSI_ENABLE){
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/InstID",InstID,InstPath);			
			system(cmd);
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/state",HOST_BOARD->Hmd_Inst[InstID]->InstState,InstPath);			
			system(cmd);
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/uplink_num",HOST_BOARD->Hmd_Inst[InstID]->Inst_UNum,InstPath);
			system(cmd);
			for(i = 0; i < HOST_BOARD->Hmd_Inst[InstID]->Inst_UNum; i++){
				memset(cmd, 0, 128);
				sprintf(cmd,"sudo echo %s > %s/uplink%d",HOST_BOARD->Hmd_Inst[InstID]->Inst_Uplink[i].ifname,InstPath, i);
				system(cmd);	
			}
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/downlink_num",HOST_BOARD->Hmd_Inst[InstID]->Inst_DNum,InstPath);			
			system(cmd);
			for(i = 0; i < HOST_BOARD->Hmd_Inst[InstID]->Inst_DNum; i++){
				memset(cmd, 0, 128);
				sprintf(cmd,"sudo echo %s > %s/uplink%d",HOST_BOARD->Hmd_Inst[InstID]->Inst_Downlink[i].ifname,InstPath, i);
				system(cmd);	
			}
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/gateway_num",HOST_BOARD->Hmd_Inst[InstID]->Inst_GNum,InstPath);
			system(cmd);
			for(i = 0; i < HOST_BOARD->Hmd_Inst[InstID]->Inst_GNum; i++){
				memset(cmd, 0, 128);
				sprintf(cmd,"sudo echo %s > %s/uplink%d",HOST_BOARD->Hmd_Inst[InstID]->Inst_Gateway[i].ifname,InstPath, i);
				system(cmd);	
			}
		}else if(op == HMD_DELETE){
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo rm -rf %s/inst%d",BaseRemotePath,InstID);
			system(cmd);
		}else if(op == HMD_STATE_SWITCH){
			memset(cmd, 0, 128);
			sprintf(cmd,"sudo echo %d > %s/state",HOST_BOARD->Hmd_Inst[InstID]->InstState,InstPath);			
			system(cmd);
		}
	}
	return 0;
}
void InitPath(char *buf,int slotid){
	
	printf("%s,1\n",__func__);
	int len = strlen(buf);
	
	printf("%s,2\n",__func__);
	sprintf(buf+len,"%d",slotid);
	
	printf("%s,3\n",__func__);
} 

void CWHMDDbusPathInit()
{
	printf("%s,1\n",__func__);

	InitPath(HMD_DBUS_BUSNAME,HOST_SLOT_NO);
	
	printf("%s,2\n",__func__);
	InitPath(HMD_DBUS_OBJPATH,HOST_SLOT_NO);
	
	printf("%s,3\n",__func__);
	InitPath(HMD_DBUS_INTERFACE,HOST_SLOT_NO);
	
	printf("%s,4\n",__func__);
}
void HmdPathInitTmp(){
	char command[128] = {0};
	sprintf(command,"%s","mkdir -p /var/run/hmd");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","mkdir -p /dbm/board/");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","mkdir -p /dbm/product/");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","mkdir -p /dbm/product/");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","touch /dbm/board/slot_id");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","touch /dbm/board/is_master");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","touch /dbm/product/distributed");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","touch /dbm/product");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","echo 1 > /dbm/product/distributed");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","echo 1 > /dbm/board/is_master");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","echo 0 > /dbm/board/slot_id");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","echo 0 > /dbm/product/master_slot_id");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","ifconfig eth0-2 up");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","tipc-config -netid=128");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","tipc-config -addr=3.1.1");
	system(command);
	memset(command,0,128);
	sprintf(command,"%s","tipc-config -be=eth:eth0-2");
	system(command);
	memset(command,0,128);
}
void HmdStateInit(){
	char Board_Slot_Path[] = "/dbm/local_board/slot_id";
	int slot_id = 0;
	char Board_isMaster_Path[] = "/dbm/local_board/is_master";
	int	is_master = 0;
	char Board_isActive_Path[] = "/dbm/local_board/is_active_master";
	int is_active = 0;
	char Board_Func_Path[] = "/dbm/local_board/function_type";
	int functype = 0;
	int board_state = 0;
	char Base_Slot_Path[] = "/dbm/product/slot/slot";
	char Product_Distributed_Path[] = "/dbm/product/is_distributed";
	int is_distributed = 0;
	char Master_Slot_Path[] = "/dbm/product/active_master_slot_id";
	int master_slot_id = 0;
	char tmpbuf[DEFAULT_LEN] = {0};	
	char tmpbuf2[128] = {0};
	int i = 0;	
	DIR *dir = NULL;
	char HmdDir[] = "/var/run/hmd";
	char cmd[128] = {0};
	dir = opendir(HmdDir);
	if(dir == NULL){
		sprintf(cmd,"sudo mkdir %s",HmdDir);
		system(cmd);
	
		memset(cmd, 0, 128);
		sprintf(cmd,"sudo chmod 755 %s",HmdDir);
		system(cmd);
		memset(cmd, 0, 128);			
	}else{
		
		printf("ok\n");
		closedir(dir);
	}

	if(read_file_info(Product_Distributed_Path,tmpbuf) == 0){
		if(parse_int_ID(tmpbuf,&is_distributed) == 0){
			isDistributed = is_distributed;
		}	
	}

	if(isDistributed){
		HOST_BOARD = (struct Hmd_Board_Info *)malloc(sizeof(struct Hmd_Board_Info));
		memset(HOST_BOARD, 0, sizeof(struct Hmd_Board_Info));		
		HOST_BOARD->tipcfd = socket (AF_TIPC, SOCK_RDM,0);
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_Slot_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&slot_id) == 0){
				HOST_SLOT_NO = slot_id;
				HOST_BOARD->slot_no = slot_id;
			}	
		}
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Master_Slot_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&master_slot_id) == 0){
				MASTER_SLOT_NO = master_slot_id;
			}	
		}
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_isMaster_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&is_master) == 0){
				isMaster = is_master;
				HOST_BOARD->isMaster = is_master;
			}	
		}
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_isActive_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&is_active) == 0){
				isActive = is_active;
			}	
		}
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_Func_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&functype) == 0){
				HOST_BOARD->board_func_type = functype;
			}	
		}
		HOST_BOARD->tipcaddr.family = AF_TIPC;
		HOST_BOARD->tipcaddr.addrtype = TIPC_ADDR_NAME;
		HOST_BOARD->tipcaddr.addr.name.name.type = SERVER_TYPE;
		HOST_BOARD->tipcaddr.addr.name.name.instance = SERVER_BASE_INST+MASTER_SLOT_NO;
		HOST_BOARD->tipcaddr.addr.name.domain = 0;
		read_master_ap_max_counter(HOST_BOARD->slot_no,&(HOST_BOARD->sem_max_ap_num));  //fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter
		if(isMaster == 1){
			for(i = 0; i < MAX_SLOT_NUM; i++){
				if(i == HOST_SLOT_NO)
					continue;
				memset(tmpbuf, 0, DEFAULT_LEN);
				memset(tmpbuf2, 0, 128);
				sprintf(tmpbuf2,"%s%d/board_state",Base_Slot_Path,i);
				if(read_file_info(tmpbuf2,tmpbuf) == 0){
					if(parse_int_ID(tmpbuf,&board_state) == 0){
						if(board_state >= BOARD_READY){
							memset(tmpbuf, 0, DEFAULT_LEN);
							memset(tmpbuf2, 0, 128);
							sprintf(tmpbuf2,"%s%d/function_type",Base_Slot_Path,i);
							if(read_file_info(tmpbuf2,tmpbuf) == 0){
								if(parse_int_ID(tmpbuf,&functype) == 0){
									if(HMD_BOARD[i] == NULL){
										HMD_BOARD[i] = (struct Hmd_Board_Info *)malloc(sizeof(struct Hmd_Board_Info));
									}
									memset(HMD_BOARD[i], 0, sizeof(struct Hmd_Board_Info));
									HMD_BOARD[i]->board_func_type = functype;
									HMD_BOARD[i]->slot_no = i;
									HMD_BOARD[i]->isMaster = 0;
									HMD_BOARD[i]->tipcaddr.family = AF_TIPC;
									HMD_BOARD[i]->tipcaddr.addrtype = TIPC_ADDR_NAME;
									HMD_BOARD[i]->tipcaddr.addr.name.name.type = SERVER_TYPE;
									HMD_BOARD[i]->tipcaddr.addr.name.name.instance = SERVER_BASE_INST+i;
									HMD_BOARD[i]->tipcaddr.addr.name.domain = 0;				
									HMD_BOARD[i]->tipcfd = socket (AF_TIPC, SOCK_RDM,0);
									read_master_ap_max_counter(i,&(HMD_BOARD[i]->sem_max_ap_num));   //fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter
								}	
							}
						}
					}
				}
			}
		}
	}
	else{
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_Slot_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&slot_id) == 0){
				HOST_SLOT_NO = slot_id;
			}	
		}
		HOST_BOARD = (struct Hmd_Board_Info *)malloc(sizeof(struct Hmd_Board_Info));
		memset(HOST_BOARD, 0, sizeof(struct Hmd_Board_Info));
		HOST_BOARD->slot_no = HOST_SLOT_NO;
		
	}
	HMD_BOARD[HOST_SLOT_NO] = HOST_BOARD;	
}

int hmd_load_slot_config(char *dir, char *wildfile, int waiting)
{
	DIR *dp = NULL;
	struct dirent *dirp;
	char command[DEFAULT_LEN] = {0};
	int wildfilecount = 0;
	dp = opendir(dir);
	if(dp == NULL)
	{
		return 0;
	}
	while((dirp = readdir(dp)) != NULL)
	{
		if((memcmp(dirp->d_name,wildfile,strlen(wildfile))) ==  0)
		{
			if((strstr(dirp->d_name, "~"))||(strstr(dirp->d_name, ".sav")))
				continue;
			memset(command, 0, DEFAULT_LEN);
			if(waiting)
				sprintf(command,"vtysh -f %s/%s -b",dir,dirp->d_name);
			else
				sprintf(command,"vtysh -f %s/%s -b &",dir,dirp->d_name);
			system(command);
			hmd_syslog_info("%s %s\n",__func__,command);
			wildfilecount++;
		}
	}
	printf("last count = %d\n",wildfilecount);
	closedir(dp);
	return 1;
}

void HmdStateReInitSlot(int slotid){
	int j = 0;
	int functype = 0;
	int board_state = 0;
	char Base_Slot_Path[] = "/dbm/product/slot/slot";
	char tmpbuf[DEFAULT_LEN] = {0};	
	char tmpbuf2[128] = {0};
	memset(tmpbuf, 0, DEFAULT_LEN);
	memset(tmpbuf2, 0, 128);
	sprintf(tmpbuf2,"%s%d/board_state",Base_Slot_Path,slotid);
	if(read_file_info(tmpbuf2,tmpbuf) == 0){
		if(parse_int_ID(tmpbuf,&board_state) == 0){
			if(board_state >= BOARD_READY){
				memset(tmpbuf, 0, DEFAULT_LEN);
				memset(tmpbuf2, 0, 128);
				sprintf(tmpbuf2,"%s%d/function_type",Base_Slot_Path,slotid);
				if(read_file_info(tmpbuf2,tmpbuf) == 0){
					if(parse_int_ID(tmpbuf,&functype) == 0){
						if(HMD_BOARD[slotid] == NULL){
							HMD_BOARD[slotid] = (struct Hmd_Board_Info *)malloc(sizeof(struct Hmd_Board_Info));
						}else{
							return;
						}
						memset(HMD_BOARD[slotid], 0, sizeof(struct Hmd_Board_Info));
						HMD_BOARD[slotid]->board_func_type = functype;
						HMD_BOARD[slotid]->slot_no = slotid;
						HMD_BOARD[slotid]->isMaster = 0;
						HMD_BOARD[slotid]->tipcaddr.family = AF_TIPC;
						HMD_BOARD[slotid]->tipcaddr.addrtype = TIPC_ADDR_NAME;
						HMD_BOARD[slotid]->tipcaddr.addr.name.name.type = SERVER_TYPE;
						HMD_BOARD[slotid]->tipcaddr.addr.name.name.instance = SERVER_BASE_INST+slotid;
						HMD_BOARD[slotid]->tipcaddr.addr.name.domain = 0;				
						HMD_BOARD[slotid]->tipcfd = socket (AF_TIPC, SOCK_RDM,0);
						read_master_ap_max_counter(HMD_BOARD[slotid]->slot_no,&(HMD_BOARD[slotid]->sem_max_ap_num));  //fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter
						for(j = 0; j < MAX_INSTANCE; j++){
							HMD_BOARD[slotid]->L_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
							memset(HMD_BOARD[slotid]->L_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
							HMD_BOARD[slotid]->R_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
							memset(HMD_BOARD[slotid]->R_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
						}
					}
				}
			}
		}
	}
}


void HmdStateReInit(){
	char Board_Slot_Path[] = "/dbm/local_board/slot_id";
	int slot_id = 0;
	char Board_isMaster_Path[] = "/dbm/local_board/is_master";
	int	is_master = 0;
	char Board_isActive_Path[] = "/dbm/local_board/is_active_master";
	int is_active = 0;
	char Board_Func_Path[] = "/dbm/local_board/function_type";
	int functype = 0;
	int board_state = 0;
	char Base_Slot_Path[] = "/dbm/product/slot/slot";
	char Product_Distributed_Path[] = "/dbm/product/is_distributed";
	int is_distributed = 0;
	char Master_Slot_Path[] = "/dbm/product/active_master_slot_id";
	int master_slot_id = 0;
	char tmpbuf[DEFAULT_LEN] = {0};	
	char tmpbuf2[128] = {0};
	int i = 0;	
	int j = 0;
	DIR *dir = NULL;
	char HmdDir[] = "/var/run/hmd";
	char cmd[128] = {0};
	dir = opendir(HmdDir);
	if(dir == NULL){
		sprintf(cmd,"sudo mkdir %s",HmdDir);
		system(cmd);
	
		memset(cmd, 0, 128);
		sprintf(cmd,"sudo chmod 755 %s",HmdDir);
		system(cmd);
		memset(cmd, 0, 128);			
	}else{
		
		printf("ok\n");
		closedir(dir);
	}
	if(read_file_info(Product_Distributed_Path,tmpbuf) == 0){
		if(parse_int_ID(tmpbuf,&is_distributed) == 0){
			isDistributed = is_distributed;
		}	
	}

	if(isDistributed){
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_Slot_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&slot_id) == 0){
				HOST_SLOT_NO = slot_id;
				HOST_BOARD->slot_no = slot_id;
			}	
		}
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Master_Slot_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&master_slot_id) == 0){
				MASTER_SLOT_NO = master_slot_id;
			}	
		}
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_isMaster_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&is_master) == 0){
				isMaster = is_master;
				HOST_BOARD->isMaster = is_master;
			}	
		}
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_isActive_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&is_active) == 0){
				isActive = is_active;
			}	
		}
		memset(tmpbuf, 0, DEFAULT_LEN);
		if(read_file_info(Board_Func_Path,tmpbuf) == 0){
			if(parse_int_ID(tmpbuf,&functype) == 0){
				HOST_BOARD->board_func_type = functype;
			}	
		}
		HOST_BOARD->tipcaddr.family = AF_TIPC;
		HOST_BOARD->tipcaddr.addrtype = TIPC_ADDR_NAME;
		HOST_BOARD->tipcaddr.addr.name.name.type = SERVER_TYPE;
		HOST_BOARD->tipcaddr.addr.name.name.instance = SERVER_BASE_INST+MASTER_SLOT_NO;
		HOST_BOARD->tipcaddr.addr.name.domain = 0;
		read_master_ap_max_counter(HOST_BOARD->slot_no,&(HOST_BOARD->sem_max_ap_num));   //fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter
		hmd_syslog_info("%s,%d.addrtype:%d,name.type:%d,name.instance:%d,name.domain:%d.\n",__func__,__LINE__,HOST_BOARD->tipcaddr.addrtype,HOST_BOARD->tipcaddr.addr.name.name.type,HOST_BOARD->tipcaddr.addr.name.name.instance,HOST_BOARD->tipcaddr.addr.name.domain);
		if(isMaster == 1){
			for(i = 0; i < MAX_SLOT_NUM; i++){
				if(i == HOST_SLOT_NO)
					continue;
				memset(tmpbuf, 0, DEFAULT_LEN);
				memset(tmpbuf2, 0, 128);
				sprintf(tmpbuf2,"%s%d/board_state",Base_Slot_Path,i);
				if(read_file_info(tmpbuf2,tmpbuf) == 0){
					if(parse_int_ID(tmpbuf,&board_state) == 0){
						if(board_state >= BOARD_READY){
							memset(tmpbuf, 0, DEFAULT_LEN);
							memset(tmpbuf2, 0, 128);
							sprintf(tmpbuf2,"%s%d/function_type",Base_Slot_Path,i);
							if(read_file_info(tmpbuf2,tmpbuf) == 0){
								if(parse_int_ID(tmpbuf,&functype) == 0){
									if(HMD_BOARD[i] == NULL){
										HMD_BOARD[i] = (struct Hmd_Board_Info *)malloc(sizeof(struct Hmd_Board_Info));
									}else{
										continue;
									}
									memset(HMD_BOARD[i], 0, sizeof(struct Hmd_Board_Info));
									HMD_BOARD[i]->board_func_type = functype;
									HMD_BOARD[i]->slot_no = i;
									HMD_BOARD[i]->isMaster = 0;
									HMD_BOARD[i]->tipcaddr.family = AF_TIPC;
									HMD_BOARD[i]->tipcaddr.addrtype = TIPC_ADDR_NAME;
									HMD_BOARD[i]->tipcaddr.addr.name.name.type = SERVER_TYPE;
									HMD_BOARD[i]->tipcaddr.addr.name.name.instance = SERVER_BASE_INST+i;
									HMD_BOARD[i]->tipcaddr.addr.name.domain = 0;				
									hmd_syslog_info("%s,%d.i:%d,addrtype:%d,name.type:%d,name.instance:%d,name.domain:%d.\n",__func__,__LINE__,i,HOST_BOARD->tipcaddr.addrtype,HOST_BOARD->tipcaddr.addr.name.name.type,HOST_BOARD->tipcaddr.addr.name.name.instance,HOST_BOARD->tipcaddr.addr.name.domain);
									HMD_BOARD[i]->tipcfd = socket (AF_TIPC, SOCK_RDM,0);
									read_master_ap_max_counter(HMD_BOARD[i]->slot_no,&(HMD_BOARD[i]->sem_max_ap_num));   //fengwenchao add for read gMaxWTPs from  /dbm/local_board/board_ap_max_counter
									for(j = 0; j < MAX_INSTANCE; j++){
										HMD_BOARD[i]->L_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
										memset(HMD_BOARD[i]->L_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
										HMD_BOARD[i]->R_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
										memset(HMD_BOARD[i]->R_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
									}
								}	
							}
						}
					}
				}
			}
		}
	}
}


void HmdInit(){
	int ret = 0;
	int i = 0;
	int j = 0;
	char command[128];
	pthread_t HMDSL;
	pthread_t HMDBUS;
	pthread_t HMDMG;
//	HmdPathInitTmp();
	HmdStateInit();
	hmd_syslog_init();
	if(isDistributed){
		if(isMaster){
			hmd_license_init();
			for(i = 0; i < MAX_SLOT_NUM; i++){
				if(HMD_BOARD[i] != NULL){
					for(j = 0; j < MAX_INSTANCE; j++){
						HMD_BOARD[i]->L_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
						memset(HMD_BOARD[i]->L_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
						HMD_BOARD[i]->R_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
						memset(HMD_BOARD[i]->R_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
					}
				}
			}
		}else{
			for(j = 0; j < MAX_INSTANCE; j++){
				HOST_BOARD->L_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
				memset(HOST_BOARD->L_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
				HOST_BOARD->R_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
				memset(HOST_BOARD->R_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
			}
		}
	}else{
		hmd_license_init();
		for(i = 0; i < LicenseCount; i++){
			memset(command, 0, 128);
			if(LICENSE_MGMT[i].total_num > 1024){
				sprintf(command,"sudo echo %d > /var/run/wcpss/wtplicense%d",1024,i+1);
			}else{
				sprintf(command,"sudo echo %d > /var/run/wcpss/wtplicense%d",LICENSE_MGMT[i].total_num,i+1);
			}
		}
		for(j = 0; j < MAX_INSTANCE; j++){
			HOST_BOARD->L_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
			memset(HOST_BOARD->L_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
			HOST_BOARD->R_LicenseNum[j] = malloc((LicenseCount + 1)*sizeof(int));
			memset(HOST_BOARD->R_LicenseNum[j], 0, (LicenseCount + 1)*sizeof(int));
		}
	}

	//CWHMDDbusPathInit();
	hmd_pid_write(HOST_SLOT_NO);
	hmd_pid_write_v2("HmdMain",HOST_SLOT_NO);
	hmd_syslog_info("isDistributed %d,isMaster %d,isActive %d,MASTER_SLOT_NO %d,HOST_SLOT_NO %d\n",isDistributed,isMaster,isActive,MASTER_SLOT_NO,HOST_SLOT_NO);
	HmdInitMsgQueue(&HMDMsgqID);
	if (timer_init() == 0) {
		hmd_syslog_crit("Can't init timer module");
		exit(1);
	}
	ret = HmdCreateThread(&HMDSL, hmd_netlink_recv_thread, NULL, 0);
	if(ret != 1){
		exit(2);
	}	
	ret = HmdCreateThread(&HMDBUS, HMDDbus, NULL, 0);
	if(ret != 1){
		exit(2);
	}
	if(isDistributed){
		if(isMaster == 1){
			ret = HmdCreateThread(&HMDMG, HMDManagementS, NULL, 0);
			if(ret != 1){
				exit(2);
			}
		}else{
			ret = HmdCreateThread(&HMDMG, HMDManagementC, NULL, 0);
			if(ret != 1){
				exit(2);
			}
		}
	}
	if(isDistributed){
		if((isMaster)&&(!isActive)){
			for(i = 0;i < LicenseCount; i++){
				if(LICENSE_MGMT[i].total_num != 0){
					hmd_license_synchronize(MASTER_SLOT_NO,i,LICENSE_MGMT[i].total_num);
				}
			}
			hmd_hansi_synchronize_request(MASTER_SLOT_NO);
		}
		hmd_config_save_timer_init(0);//fengwenchao add for hmd timer config save
	}
	return;
}
void HmdRun(){
	while(1){
		sleep(100000000);
		printf("%s,2...\n",__func__);
	}
	return;
}
void HmdRelease(){
	return;
}

int main(int argc, const char * argv[]){

	HmdInit();
	HmdRun();
	HmdRelease();
	return 0;
}
