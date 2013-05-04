#ifndef _BSD_H_
#define	_BSD_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include "bsd/bsdpub.h"


#define	BSD_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#define BSD_BIG_BUF_LENGTH 2048 

#define BSD_PATH_LEN (128)
#define BSD_NAME_LEN (96)
#define SHOW_RUNNING_LEN	(1024 * 1024)
#define MAX_EVENT_ID (65535)
#define BSD_FILE_HEAD_LEN (BSD_PATH_LEN*2 + 64) 
#define BSD_DATA_LEN (32768)
#define BSD_FILE_FOLDER_SIZE    1
#define BSD_SYSTEM_FREE_MEMERY  2
#define BSD_BLK_FREE_MEMERY     3

#define DIR_TPYE    (4)
#define FILE_TYPE   (8)


typedef enum {
	BSD_FALSE = 0,
	BSD_TRUE = 1
} BSDBool;

typedef enum bsd_file_state{
    BSD_FILE_UNKNOWN = 0,
    BSD_FILE_NORMAL = 1,
    BSD_FILE_LAST = 2,
    BSD_FILE_FINISH = 3,
    BSD_FILE_DES_PATH_CHECK = 4,
    BSD_FILE_DES_PATH_AVA = 5,
    BSD_FILE_DES_PATH_UNA = 6,
    BSD_FILE_MEMERY_CHECK = 7,
    BSD_FILE_MEMERY_OK = 8,
    BSD_FILE_MEMERY_NOT_ENOUGH = 9,
    BSD_FILE_FAILURE = 10
}bsd_file_state_t;


typedef struct bsd_file_head
{
    unsigned short event_id;
	unsigned int file_total_len;
	unsigned int file_already_len;
	unsigned int send_len;
	bsd_file_state_t file_state;
	bsd_file_type_t  file_type;
	int tar_flag;
	unsigned char uchFileName[BSD_PATH_LEN];
	unsigned char md5Result[BSD_PATH_LEN];
}bsd_file_head_t;


typedef struct bsd_file_info{
    unsigned int slot_no;
	bsd_file_head_t file_head;
	unsigned char uchData[BSD_DATA_LEN];
}bsd_file_info_t;


extern int HOST_SLOT_NO;
extern int isActive;
extern int BSDMsgqID;
extern int tarFlag;

int parse_int_ID(char* str,unsigned int* ID);
void bsd_pid_write_v2(const char *name,int slot_num);
int BsdGetMsgQueue(int *msgqid);
int BSDGetFileDir(const char *file_path, char file_dir[BSD_PATH_LEN]);
int BSDGetFileName(const char *file_path, char file_name[BSD_NAME_LEN]);
int bsdGetFileSize(unsigned int *iFileSize, const char *pFilePath, const int iType);
int bsdCreateDir(const char *sPathName);

#endif

