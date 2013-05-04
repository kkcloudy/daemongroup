#ifndef _WBMD_H_
#define	_WBMD_H_
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
#include "wbmd/wbmdpub.h"
#include "wbmd_log.h"
typedef enum {
	WBMD_FALSE = 0,
	WBMD_TRUE = 1
} WBMDBool;

typedef struct {
	int QID;
	int islocaled;
	int InstID;
} WBMDThreadArg; // argument passed to the thread func

#define 	MAX_SLOTNO 16
#define		WBMD_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#define		WBMD_FREE_OBJECTS_ARRAY(ar_name, ar_size)			{int _i = 0; for(_i = ((ar_size)-1); _i >= 0; _i--) {if(((ar_name)[_i]) != NULL){ free((ar_name)[_i]);}} free(ar_name); (ar_name) = NULL; }
#define		WBMD_BUF_LEN		2048
// custom error
#define		WBMD_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define		WBMD_CREATE_OBJECT_SIZE_ERR(obj_name, obj_size,on_err)	{obj_name = (malloc(obj_size)); if(!(obj_name)) {on_err}}
#define		WBMD_CREATE_ARRAY_ERR(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (malloc(sizeof(ar_type) * (ar_size))); if(!(ar_name)) {on_err}}
#define		WBMD_CREATE_STRING_ERR(str_name, str_length, on_err)	{str_name = (char*) (malloc(sizeof(char) * ((str_length)+1) ) ); if(!(str_name)) {on_err}}
#define		WBMD_CREATE_STRING_FROM_STRING_ERR(str_name, str, on_err)	{WBMD_CREATE_STRING_ERR(str_name, strlen(str), on_err); strcpy((str_name), str);}
#define 	DEFAULT_LEN	256
#define THREAD_NUM	16
int read_ac_file(char *FILENAME,char *buff,int blen);
void InitPath(char *buf);
void wbmd_pid_write_v2(char *name);
WBMDBool WbmdGetMsgQueue(int *msgqid);
extern int Checkfd; 
extern unsigned int vrrid;
extern unsigned int local;
extern struct wbridge_info * wBridge[WBRIDGE_NUM];
extern struct wbridge_ip_hash wb_hash;

#endif

