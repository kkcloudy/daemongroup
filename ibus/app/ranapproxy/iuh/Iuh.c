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
* Iuh.c
*
*
* CREATOR:
* zhanglei@autelan.com
*
* DESCRIPTION:
* Femto Iuh module
*
*
*******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <dirent.h>
#include <unistd.h>
#include <linux/tipc.h>
#include "iuh/Iuh.h"
#include "Iuh_Thread.h"
#include "Iuh_SockOP.h"
#include "Iuh_IFListen.h"
#include "Iuh_DBus.h"
#include "Iuh_log.h"
#include "Iuh_IuRecv.h"
#include "Iuh_ManageHNB.h"
#include "dbus/iuh/IuhDBusPath.h"
#include "Iuh_License.h"

#define PID_BUF_LEN 128
#define CMD_BUF_LEN 128

int vrrid = 0;
int local = 1;
int slotid = 0;
int sockPerThread[SOCK_NUM] ={0};
int sockpsPerThread[SOCK_NUM] ={0};//book add, 2011-10-27
char MSGQ_PATH[PATH_LEN] = "/var/run/femto/iuhmsgq";
unixAddr toIU;
unixAddr toIUPS;//book add, 2011-10-27
int IuSocket = 0;
int IupsSocket = 0;//book add, 2011-10-27
int IuhMsgQid;
int IuhAllQid;
IuhThread IThread[THREAD_NUM];
IuhMultiHomedSocket gIuhSocket;
Iuh_HNB  **IUH_HNB;
Iuh_HNB_UE  **IUH_HNB_UE;
Iu_RAB **IU_RAB;
iuh_auto_hnb_info g_auto_hnb_login;
struct ifi *IUH_IF = NULL;
int HNB_NUM = 0;
int UE_NUM = 0;
int glicensecount = 0;
int gStaticHNBs = 0;
int gStaticUEs = 0;
unsigned short int gRncId = 5;   
struct g_switch gSwitch;
int IuTipcSocket = 0;	//xiaodw add for tipc
struct sockaddr_tipc Iuh2Iu_tipcaddr;
int TipcsockPerThread[SOCK_NUM] ={0};
int gIuhTipcSockFlag = 0;

/*****************************************************
** DISCRIPTION:
**          init femto path
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-17
*****************************************************/
void Iuh_path_init()
{
    DIR *dir = NULL;
	char FemtoDir[] = "/var/run/femto";
	char cmd[CMD_BUF_LEN] = {0};
	dir = opendir(FemtoDir);
	if(dir == NULL){
		sprintf(cmd,"sudo mkdir -p %s",FemtoDir);
		system(cmd);
	    //printf("mkdir over\n");
		memset(cmd, 0, CMD_BUF_LEN);
		sprintf(cmd,"sudo chmod 755 %s",FemtoDir);
		system(cmd);
		memset(cmd, 0, CMD_BUF_LEN);
	}else{
		//printf("dir is not null\n");
		closedir(dir);
	}   
}


/*****************************************************
** DISCRIPTION:
**          get the iuh process id.
**          create file iuh.pid
**          write the process id into iuh.pid
** INPUT:
**          vrrid
** OUTPUT:
**          void
** RETURN:
**          void
*****************************************************/
void iuh_pid_write(int vrrid)
{
	char pidBuf[PID_BUF_LEN] = {0}, pidPath[PID_BUF_LEN] = {0};
	pid_t myPid = 0;
	int fd;
#ifndef _DISTRIBUTION_
	sprintf(pidPath,"%s%d%s", "/var/run/femto/iuh", \
				vrrid, ".pid");
#else
	sprintf(pidPath,"%s%d_%d%s", "/var/run/femto/iuh", \
				local,vrrid, ".pid");
#endif
		
	fd = open(pidPath, O_RDWR|O_CREAT);
	myPid = getpid();	
	sprintf(pidBuf,"%d\n", myPid);
	write(fd, pidBuf, strnlen(pidBuf,PID_BUF_LEN));
	close(fd);
	return;
}

IuhBool IuhInitMsgQueue(int *msgqid){
	key_t key;
	InitFemtoPath(vrrid,MSGQ_PATH);
	//printf("%s\n",MSGQ_PATH);
	if(fopen(MSGQ_PATH,"w") == NULL){		
		iuh_syslog_crit("%s fopen %s",__func__,strerror(errno));
		perror("fopen");
		exit(1);	
	}
		
	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {
		iuh_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		exit(1);
	}
	if ((*msgqid = msgget(key, 0666 | IPC_CREAT)) == -1) {		
		iuh_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		exit(1);
	}
	//printf("*msgqid %d\n",*msgqid);
	return Iuh_TRUE;
}

IuhBool IuhGetMsgQueue(int *msgqid){
	key_t key;

	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {		
		iuh_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		exit(1);
	}
	if ((*msgqid = msgget(key, 0644)) == -1) {
		iuh_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		exit(1);
	}
	return Iuh_TRUE;
}

void FemtoIuhInit(){
	int i = 0;
	iuh_syslog_init();
	Iuh_License_Init();	
	Iuh_DbusPath_Init();
	Iuh_Interface_Listen_init();
	if (timer_init() == 0) {
		iuh_syslog_crit("Can't init timer module");
		exit(1);
	}
	if(//!(Iuh_IU_InitSocket(&IuSocket))||
	    //!(Iuh_IUPS_InitSocket(&IupsSocket))||
	 	!(IuhInitMsgQueue(&IuhMsgQid))||
	 	!(IuhGetMsgQueue(&IuhAllQid))
	)
	{
		exit(1);
	}
	
	IuhThread thread_dbus;	
	if(!(IuhCreateThread(&thread_dbus, iuh_dbus_thread, NULL,0))) {
		iuh_syslog_crit("Error starting Dbus Thread");
		exit(1);
	}
	while(gIuhTipcSockFlag == 0)
	{
		sleep(2);
	}

	IuhThread thread_iu;
	if(!(IuhCreateThread(&thread_iu, IuRecv, NULL,0))) {
		iuh_syslog_crit("Error starting Interface Thread");
		exit(1);
	}

    /*
	IuhThread thread_iups;
	if(!(IuhCreateThread(&thread_iups, IupsRecv, NULL,0))) {
		iuh_syslog_crit("Error starting Interface Thread");
		exit(1);
	}
	*/
	
	for(i = 0; i < THREAD_NUM; i++){	
		IuhThreadArg*argPtr;		
		IUH_CREATE_OBJECT_ERR(argPtr, IuhThreadArg, { iuh_syslog_crit("Out Of Memory"); return; });
		argPtr->index = i+1;
		if(!(IuhCreateThread(&(IThread[i]), IuhManageHNB, argPtr,0))) {
			IUH_FREE_OBJECT(argPtr);
			return;
		}
	}
	gIuhSocket.count = 0;
	
	//auto hnb area
	{
		g_auto_hnb_login.auto_hnb_switch =0;
		g_auto_hnb_login.save_switch = 0;
		g_auto_hnb_login.ifnum = 0;
		g_auto_hnb_login.auto_hnb_if = NULL;
	}

    /* initiate global switch */
	gSwitch.paging_imsi = 0;
	gSwitch.paging_lai = 0;
	
}


int main(int argc, const char * argv[]){
	if(argc > 2){
		local =  atoi(argv[1]);
		vrrid =  atoi(argv[2]);
	}else if(argc != 1){
		printf("iuh argc %d, something wrong!!\n", argc);
		return 0;
	}
	//iuh_syslog_debug_debug(IUH_DEFAULT,"main function\n");
	Iuh_path_init();
	iuh_pid_write(vrrid);
	FemtoIuhInit();
	FemtoIuhRun();

	return 0;
}

