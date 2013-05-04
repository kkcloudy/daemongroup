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
* bsd.c
*
*
* CREATOR:
* zhangshu@autelan.com
*
* DESCRIPTION:
* Board Synchronization module
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
#include "bsd.h"
#include "dbus/bsd/BsdDbusPath.h"
#include "dbus/bsd/BsdDbusDef.h"
#include "bsd_management.h"
#include "bsd_dbus.h"
#include "bsd_log.h"
#include "bsd_timerLib.h"
#include "bsd_tipc.h"
#include "bsd_tcp.h"
#include "bsd_monitor.h"
#include "bsd_thread.h"

#define PID_BUF_LEN     128
#define PID_PATH_LEN    128

int HOST_SLOT_NO = 0; //slot no
int isActive = 0;
char MSGQ_PATH[PATH_LEN] = "/var/run/bsd/bmsgq";
int BSDMsgqID = 0;
unsigned short g_unEventId = 0;
struct Bsd_Board_Info *HOST_BOARD;
struct Bsd_Board_Info *BSD_BOARD[MAX_SLOT_NUM];

BsdThreadCondition fileStateCondition[MAX_SLOT_NUM];	
BsdThreadMutex fileStateMutex[MAX_SLOT_NUM];


int parse_int_ID(char* str,unsigned int* ID)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
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
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
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
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
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


void bsd_pid_write(int slot_num)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	char pidBuf[PID_BUF_LEN] = {0}, pidPath[PID_PATH_LEN] = {0};
	pid_t myPid = 0;
	int fd;
	sprintf(pidPath,"%s%d%s", "/var/run/bsd/bsd", \
				slot_num, ".pid");
		
	fd = open(pidPath, O_RDWR|O_CREAT);
	myPid = getpid();	
	sprintf(pidBuf,"%d\n", myPid);
	write(fd, pidBuf, strnlen(pidBuf, PID_BUF_LEN));
	close(fd);
	return;
}


void bsd_pid_write_v2(const char *name,int slot_num)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	char pidBuf[PID_BUF_LEN] = {0}, pidPath[PID_PATH_LEN] = {0};
	char idbuf[PID_BUF_LEN] = {0};
	pid_t myPid = 0;
	static int fd;
	static unsigned char opened = 0;
	sprintf(pidPath,"%s%d%s", "/var/run/bsd/bsd_thread", \
				slot_num, ".pid");
	if(!opened){
		
		fd = open(pidPath, O_RDWR|O_CREAT);
		bsd_syslog_info("fd=%d,%s,path:%s\n",fd,__func__,pidPath);
		if(fd < 0){
			bsd_syslog_err("<err>,%s\n",__func__);
			return;
		}
		opened = 1;
	}
		
	sprintf(idbuf,"BSD[%d]%s",slot_num,name);
	myPid = getpid();
	sprintf(pidBuf,"%s-%d\n",idbuf,myPid);
	write(fd, pidBuf, strnlen(pidBuf, PID_BUF_LEN));
	return;
}


void bsd_pid_write_v3(int slot_num,int instID,int islocal)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	char pidBuf[PID_BUF_LEN] = {0}, pidPath[PID_PATH_LEN] = {0};
	pid_t myPid = 0;
	int fd;
	sprintf(pidPath,"%s%d-%d-%d%s", "/var/run/bsd/bsd", \
				islocal,slot_num,instID, ".pid");
		
	fd = open(pidPath, O_RDWR|O_CREAT);
	if(fd > 0) {
    	myPid = getpid();	
    	sprintf(pidBuf,"%d\n", myPid);
    	write(fd, pidBuf, strnlen(pidBuf, PID_BUF_LEN));
    	close(fd);
	}
	return;
}


int read_file_info(char *FILENAME,char *buff)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
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


/*****************************************************
** DISCRIPTION:
**          get dir of the file_path
** INPUT:
**          file path
** OUTPUT:
**          file dir
** RETURN:
**          void
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-15
*****************************************************/
int BSDGetFileDir(const char *file_path, char file_dir[PATH_LEN])
{
    //bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    char *fileName = strrchr(file_path, '/');
    int ret = 0;
    memset(file_dir, 0, PATH_LEN);
    if(fileName != NULL)
	    memcpy(file_dir, file_path, fileName-file_path);
	else {
	    bsd_syslog_err("Error: failed to get file dir in BSDGetFileDir().\n");
	    ret = -1;
	}
    return ret;
}


/*****************************************************
** DISCRIPTION:
**          get name of the file from file_path
** INPUT:
**          file path
** OUTPUT:
**          file name
** RETURN:
**          void
**CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-16
*****************************************************/
int BSDGetFileName(const char *file_path, char file_name[NAME_LEN])
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    char *fileName = strrchr(file_path, '/');
    int ret = 0;
    if(fileName == NULL) {
        bsd_syslog_err("Error: failed to get file name from %s.\n", file_path);
        ret = -1;
    }
    memset(file_name, 0, NAME_LEN);
    if(fileName != NULL)
	    memcpy(file_name, fileName+1, strnlen(fileName, NAME_LEN));
	else {
	    bsd_syslog_err("Error: failed to get file name.\n");
	    ret = -1;
	}
    return ret;
}



/** 
  * @brief  create all file dirs
  * @param  sPathName   
  * @return  
  * @author  zhangshu
  * @date  2012/06/11
  */
int bsdCreateDir(const char *sPathName)  
{
    char DirName[256] = {0};
    strcpy(DirName, sPathName);
    int i = 0;
    int len = strlen(DirName);
    if(DirName[len-1]!='/')
        strcat(DirName, "/");
    
    len = strlen(DirName);
    
    for(i = 1; i < len; i++) {
        if(DirName[i]=='/') {
            DirName[i] = 0;
            if(access(DirName, 0) != 0) {
                if(mkdir(DirName, 0755)==-1) {
                    perror("mkdir error");
                    return -1;
                }
            }
            DirName[i] = '/';
        }
    }
    
    return 0;
}




int BsdGetMsgQueue(int *msgqid)
{
	key_t key;

	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {		
		bsd_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		exit(1);
	}
	if ((*msgqid = msgget(key, 0644)) == -1) {
		bsd_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		exit(1);
	}
	return BSD_TRUE;
}


/** 
  * @brief  get specific file/folder size
  * @param  nFileSize   
  * @param  pFilePath 
  * @param  iType
  * @return  
  * @author  zhangshu
  * @date  2012/05/30
  */
int bsdGetFileSize(unsigned int *piFileSize, const char *pFilePath, const int iType)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int iReturnValue = BSD_SUCCESS;
    FILE *pFilePointer = NULL;
    char cmdBuf[1024] = {0};
    char sizeBuf[PATH_LEN] = {0};
    int len = 0;
    system("sudo mount /blk");
    bsd_syslog_debug_debug(BSD_DEFAULT, "type = %d\n", iType);
    /* make command buffer */
    switch(iType) {
        case BSD_FILE_FOLDER_SIZE:
            sprintf((char*)cmdBuf, "du -s %s |awk '{print $1}'", pFilePath);
            break;
        case BSD_BLK_FREE_MEMERY:
            sprintf((char*)cmdBuf, "df -l /blk |grep blk |awk '{print $4}'");
            break;
        case BSD_SYSTEM_FREE_MEMERY:
            sprintf((char*)cmdBuf, "free |grep + |awk '{print $4}'");
            break;
        default:
            bsd_syslog_err("Error : illegal type.\n");
            iReturnValue = BSD_GET_FILE_SIZE_ERROR;
    }
    /* get size | memery */
    pFilePointer = popen((char*)cmdBuf, "r");
    if(!pFilePointer) {
        bsd_syslog_err("Error : get file size failed.\n");
        iReturnValue = BSD_GET_FILE_SIZE_ERROR;
    } else {
        len = fread(sizeBuf, sizeof(char), sizeof(sizeBuf), pFilePointer);
        pclose(pFilePointer);
        if(len < 0) {
            bsd_syslog_err("Error : read file size failed!\n");
            iReturnValue =  BSD_GET_FILE_SIZE_ERROR;
        }
        else if(-1 == parse_int_ID((char*)sizeBuf, piFileSize)) {
            bsd_syslog_err("Error : get file size failed!\n");
            iReturnValue =  BSD_GET_FILE_SIZE_ERROR;
        }
    }
    bsd_syslog_debug_debug(BSD_DEFAULT, "@@@@@@@@ Size = %d\n", *piFileSize);
    system("sudo umount /blk");
    return iReturnValue;
}



int BsdInitMsgQueue(int *msgqid)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	key_t key;
	FILE *fp = NULL;
	if((fp = fopen(MSGQ_PATH,"w")) == NULL){
		bsd_syslog_crit("%s fopen %s",__func__,strerror(errno));
		perror("fopen");
		exit(1);	
	}
		
	if ((key = ftok(MSGQ_PATH, 'W')) == -1) {
		bsd_syslog_crit("%s ftok %s",__func__,strerror(errno));
		perror("ftok");
		fclose(fp);
		fp = NULL;
		exit(1);
	}
	if ((*msgqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
		bsd_syslog_crit("%s msgget %s",__func__,strerror(errno));
		perror("msgget");
		fclose(fp);
		fp = NULL;
		exit(1);
	}
	fclose(fp);
	fp = NULL;
	return BSD_TRUE;
}


int BsdDirectoryCheck(char *path)
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
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


int BsdClearFile(FILE *fp)
{
	ftruncate(fileno(fp),0);
	return 1;
}


void InitPath(char *buf,int slotid)
{
	
	printf("%s,1\n",__func__);
	int len = strnlen(buf, PATH_LEN);
	
	printf("%s,2\n",__func__);
	sprintf(buf+len,"%d",slotid);
	
	printf("%s,3\n",__func__);
	return;
} 

void CWBSDDbusPathInit()
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	printf("%s,1\n",__func__);
	InitPath(BSD_DBUS_BUSNAME,HOST_SLOT_NO);
	printf("BSD_DBUS_BUSNAME = %s\n",BSD_DBUS_BUSNAME);
	
	printf("%s,2\n",__func__);
	InitPath(BSD_DBUS_OBJPATH,HOST_SLOT_NO);
	printf("BSD_DBUS_OBJPATH = %s\n",BSD_DBUS_OBJPATH);
	
	printf("%s,3\n",__func__);
	InitPath(BSD_DBUS_INTERFACE,HOST_SLOT_NO);
	printf("BSD_DBUS_INTERFACE = %s\n",BSD_DBUS_INTERFACE);
	
	printf("%s,4\n",__func__);
	return;
}


void BsdPathInitTmp()
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	char command[128] = {0};
	sprintf(command,"%s","mkdir -p /var/run/bsd");
	system(command);
	memset(command,0,128);
	return;
}


void BsdStateInit()
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    char Board_Slot_Path[] = "/dbm/local_board/slot_id";
	unsigned int slot_id = 0;
	char tmpbuf[256] = {0};
	DIR *dir = NULL;
	char BsdDir[] = "/var/run/bsd";
	//printf("bsddir = %s\n",BsdDir);
	char cmd[128] = {0};
	//printf("open dir\n");
	dir = opendir(BsdDir);
	if(dir == NULL){
	    //printf("dir is null\n");
		sprintf(cmd,"sudo mkdir -p %s",BsdDir);
		system(cmd);
	    //printf("mkdir over\n");
		memset(cmd, 0, 128);
		sprintf(cmd,"sudo chmod 755 %s",BsdDir);
		system(cmd);
		memset(cmd, 0, 128);
	}else{
		//printf("dir is not null\n");
		closedir(dir);
	}
	if(read_file_info(Board_Slot_Path,tmpbuf) == 0){
		if(parse_int_ID(tmpbuf,&slot_id) == 0){
			HOST_SLOT_NO = slot_id;
		}
	}
	return;
}


/*****************************************************
** DISCRIPTION:
**          init mutex
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-09-15
*****************************************************/
void BsdMutexInit()
{
    int i = 0;
    for(i = 0; i < MAX_SLOT_NUM; i++)
    {
    	if(BSD_FALSE == BsdCreateThreadMutex(&fileStateMutex[i])){
            bsd_syslog_err("Can't create thread mutex");
    	    exit(2);
        }
    }    
	return;
}


/*****************************************************
** DISCRIPTION:
**          init condition
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-09-15
*****************************************************/
void BsdConditionInit()
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
    int i = 0;
    for(i = 0; i < MAX_SLOT_NUM; i++)
    {
    	if(BSD_FALSE == BsdCreateThreadCondition(&fileStateCondition[i])){
    	    bsd_syslog_err("Can't create thread condition");
    	    exit(2);
    	}
    }
	return;
}

/*****************************************************
** DISCRIPTION:
**          init function
** INPUT:
**          void
** OUTPUT:
**          void
** RETURN:
**          void
** CREATOR:
**          <zhangshu@autelan.com>
** DATE:
**          2011-08-10
*****************************************************/
void BsdInit()
{
    bsd_syslog_debug_debug(BSD_DEFAULT, "%s  %d: %s\n",__FILE__, __LINE__, __func__);
	int ret = 0;
	pthread_t BSDDBUS;
	pthread_t BSDMG;
	pthread_t BSDMAIN;
	pthread_t BSDTCP;
	BsdStateInit();
	bsd_syslog_init();
	bsd_pid_write(HOST_SLOT_NO);
	bsd_pid_write_v2("BsdMain",HOST_SLOT_NO);
    BSDServerTipcInit(HOST_SLOT_NO);
	BSDClientTipcInit();
	//BSDClientGroupTipcInit();
	bsdInitiateTcpServer();
    
	BsdInitMsgQueue(&BSDMsgqID);
	if (timer_init() == 0) {
		bsd_syslog_crit("Can't init timer module");
		exit(1);
	}
    
	BsdMutexInit();
	BsdConditionInit();
	
	/* creat bsd tipc monitor thread */
	ret = BsdCreateThread(&BSDMAIN, bsdTipcMonitor, NULL, 0);
	if(ret != 1)
		exit(2);
	/* create bsd tipc management thread */
	ret = BsdCreateThread(&BSDMG, bsdTipcManagement, NULL, 0);
	if(ret != 1)
		exit(2);
	/* create bsd dbus thread */
	ret = BsdCreateThread(&BSDDBUS, bsdDbus, NULL, 0);
	if(ret != 1)
		exit(2);
	/* create bsd tcp thread */
    ret = BsdCreateThread(&BSDTCP, bsdTcpManagement, NULL, 0);
    if(ret != 1)
        exit(2);
    
    return;
}

void BsdRun(){
	while(1){
		sleep(100000000);
	}
	return;
}

void BsdRelease(){
    int i = 0;
    for(i = 0; i < MAX_SLOT_NUM; i++)
    {
        BsdDestroyThreadCondition(&fileStateCondition[i]);
        BsdDestroyThreadMutex(&fileStateMutex[i]);
    }
    return;
}

int main(int argc, const char * argv[]){

	BsdInit();
	BsdRun();
	BsdRelease();
	
	return 0;
}

