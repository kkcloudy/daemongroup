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
* WBMD.c
*
*
* CREATOR:
* zhanglei@autelan.com
*
* DESCRIPTION:
* Wireless Bridge Management module
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
#include "wbmd.h"
#include "wbmd/wbmdpub.h"
#include "wbmd/wbmdmib.h"
#include "dbus/wbmd/WbmdDbusPath.h"
#include "wbmd_thread.h"
#include "wbmd_dbus.h"
#include "wbmd_check.h"
#include "wbmd_manage.h"

unsigned int vrrid = 0;
unsigned int local = 1;
struct wbridge_info * wBridge[WBRIDGE_NUM];
struct wbridge_ip_hash wb_hash;

int Checkfd = 0; 
int parse_int_ID(char* str,unsigned int* ID){
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

void InitPath(char *buf){	
	int len = strlen(buf);	
	sprintf(buf+len,"%d_%d",local,vrrid);
} 

void wbmd_pid_write()
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	pid_t myPid = 0;
	int fd;
	sprintf(pidPath,"%s%d-%d%s", "/var/run/wbmd/wbmd", \
				local,vrrid, ".pid");
		
	fd = open(pidPath, O_RDWR|O_CREAT);
	myPid = getpid();	
	sprintf(pidBuf,"%d\n", myPid);
	write(fd, pidBuf, strlen(pidBuf));
	close(fd);
	return;
}

void wbmd_pid_write_v2(char *name)
{
	char pidBuf[128] = {0}, pidPath[128] = {0};
	char idbuf[128] = {0};
	pid_t myPid = 0;
	static int fd;
	static unsigned char opened = 0;
	sprintf(pidPath,"%s%d-%d%s", "/var/run/wbmd/wbmd_thread", \
				local,vrrid, ".pid");
	if(!opened){
		
		fd = open(pidPath, O_RDWR|O_CREAT);
		wbmd_syslog_info("fd=%d,%s,path:%s\n",fd,__func__,pidPath);
		if(fd < 0){
			wbmd_syslog_err("<err>,%s\n",__func__);
			return ;
		}
		opened = 1;
	}
		
	sprintf(idbuf,"WBMD[%d-%d]%s",local,vrrid,name);
	myPid = getpid();
	sprintf(pidBuf,"%s-%d\n",idbuf,myPid);
	write(fd, pidBuf, strlen(pidBuf));
	return;
}


WBMDBool WbmdGetMsgQueue(int *msgqid){
	key_t key;		

	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {		
		wbmd_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		exit(1);
	}
	if ((*msgqid = msgget(key, 0644)) == -1) {
		wbmd_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		exit(1);
	}
	return WBMD_TRUE;
}

int WbmdDirectoryCheck(char *path){
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
int WbmdClearFile(FILE *fp){
	ftruncate(fileno(fp),0);
	return 1;
}

void WBMDDbusPathInit()
{
	InitPath(WBMD_DBUS_BUSNAME);
	InitPath(WBMD_DBUS_OBJPATH);
	InitPath(WBMD_DBUS_INTERFACE);
}

WBMDBool WbmdInitMsgQueue(){
	key_t key;
	char BasePath[] = "/var/run/wbmd";
	if(WbmdDirectoryCheck(BasePath) == 0){
		return -1; 
	}
	InitPath(MSGQ_PATH);
	if(fopen(MSGQ_PATH,"w") == NULL){
		wbmd_syslog_crit("%s fopen %s",__func__,strerror(errno));
		perror("fopen");
		exit(1);	
	}
		
	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {
		wbmd_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		exit(1);
	}
	if ((msgget(key, 0666 | IPC_CREAT)) == -1) {
		wbmd_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		exit(1);
	}
	return WBMD_TRUE;
}

void WBInit(){
	memset(wBridge, 0, WBRIDGE_NUM*sizeof(struct wbridge_info*));
	memset(&wb_hash, 0, sizeof(struct wbridge_ip_hash));
}

void WbmdDbusPathInit()
{
	InitPath(WBMD_DBUS_BUSNAME);
	InitPath(WBMD_DBUS_OBJPATH);
	InitPath(WBMD_DBUS_INTERFACE);
}


void WbmdInit(){
	int i = 0;
	wbmd_syslog_init();
	WBInit();
	if (timer_init() == 0) {
		wbmd_syslog_crit("Can't init timer module");
		exit(1);
	}
	WbmdDbusPathInit();
	WbmdInitMsgQueue();
	Checkfd = wbmd_init_ping_check_sock();
	WbmdThread thread_dbus;	
	if(!(WbmdCreateThread(&thread_dbus, WBMDDbus, NULL,0))) {
		wbmd_syslog_crit("Error starting Dbus Thread");
		exit(1);
	}

	WbmdThread thread_check;
	if(!(WbmdCreateThread(&thread_check, WBMDCheck, NULL,0))) {
		wbmd_syslog_crit("Error starting wbridge check Thread");
		exit(1);
	}
	for(i = 0; i < THREAD_NUM; i++){
		WbmdThread thread_manage;
		WBMDThreadArg * argPtr = NULL;		
		WBMD_CREATE_OBJECT_ERR(argPtr, WBMDThreadArg, { wbmd_syslog_crit("Out Of Memory"); return; });
		argPtr->QID = i+1;
		argPtr->islocaled = local;
		argPtr->InstID = vrrid;
		if(!(WbmdCreateThread(&thread_manage, WBMDManage, argPtr,0))) {
			wbmd_syslog_crit("Error starting wbridge check Thread");
			exit(1);
		}		
	}
}
void WbmdRun(){	
	int MsgqID;	
	struct WbmdCheckMsgQ msg;
	WbmdGetMsgQueue(&MsgqID);
	while(1){
			fd_set fset;
			struct timeval timeout;
			int max = 0;
			char buf[DEFAULT_LEN];			
			int len = 0;
			FD_ZERO(&fset);
			FD_SET(Checkfd,&fset);
			max = Checkfd;
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;			
			struct sockaddr_in from;			
			int fromlen=sizeof(from);
			while(select(max+1, &fset, NULL, NULL, &timeout) < 0) {
				if(errno != EINTR) {
					break;
				}
			}
			if(FD_ISSET(Checkfd, &fset)) 
			{
				if((len=recvfrom(Checkfd,buf,sizeof(buf),0, (struct sockaddr *)&from,(socklen_t *)&fromlen)) < 0)
				{
					continue;
				}
				if(len > 0)
				{
					memset((char*)&msg, 0, sizeof(msg));
					msg.mqid = 99;
					msg.mqinfo.type = WBMD_DATA;
					msg.mqinfo.WBID = from.sin_addr.s_addr; 
					msg.mqinfo.Datalen = len;
					memcpy(msg.mqinfo.Data, buf, DEFAULT_LEN);
					if (msgsnd(MsgqID, (struct WbmdCheckMsgQ *)&msg, sizeof(msg.mqinfo), IPC_NOWAIT) == -1){
						wbmd_syslog_crit("%s msgsend %s",__func__,strerror(errno));
						perror("msgsnd");
					}
					
				}
			}
	}
}

int main (int argc, char * argv[]) {
	if(argc > 2){
		local =  atoi(argv[1]);
		vrrid =  atoi(argv[2]);
	}else if(argc != 1){
		printf("argc %d, something wrong\n",argc);
		return 0;
	}

	WbmdInit();
	WbmdRun();	
	return 0;
}

