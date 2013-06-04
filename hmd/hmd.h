#ifndef _HMD_H_
#define	_HMD_H_
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
#include "hmd/hmdpub.h"

/*fengwenchao copy from 1318 for AXSSZFI-839*/
#define IFF_BINDING_FLAG  0x2
#define SIOCGIFUDFFLAGS	0x893d		/* get udf_flags			*/
#define SIOCSIFUDFFLAGS	0x893e		/* set udf_flags			*/
/*fengwenchao copy end*/

typedef enum {
	HMD_FALSE = 0,
	HMD_TRUE = 1
} HMDBool;

typedef struct {
	int QID;
	int islocaled;
	int InstID;
} HMDThreadArg; // argument passed to the thread func

extern int HOST_SLOT_NO;
extern int isMaster;
extern int MASTER_SLOT_NO;
extern int HMDMsgqID;
extern int	isDistributed;
extern int LicenseCount;
extern int isActive;
extern int HANSI_CHECK_OP;
extern int HANSI_TIMER_CONFIG_SAVE;//fengwenchao add 20130412 for hmd timer config save
extern int  HANSI_TIMER;//fengwenchao add 20130412 for hmd timer config save
extern int MASTER_BACKUP_SLOT_NO;
extern unsigned int service_tftp_state;//service tftp (0--disable;1--enable)
extern unsigned int service_ftp_state;//service ftp  (0--disable;1--enable)

#define 	MAX_SLOTNO 16
#define		HMD_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
#define		HMD_FREE_OBJECTS_ARRAY(ar_name, ar_size)			{int _i = 0; for(_i = ((ar_size)-1); _i >= 0; _i--) {if(((ar_name)[_i]) != NULL){ free((ar_name)[_i]);}} free(ar_name); (ar_name) = NULL; }
#define		HMD_BUF_LEN		2048
// custom error
#define		HMD_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}
#define		HMD_CREATE_OBJECT_SIZE_ERR(obj_name, obj_size,on_err)	{obj_name = (malloc(obj_size)); if(!(obj_name)) {on_err}}
#define		HMD_CREATE_ARRAY_ERR(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (malloc(sizeof(ar_type) * (ar_size))); if(!(ar_name)) {on_err}}
#define		HMD_CREATE_STRING_ERR(str_name, str_length, on_err)	{str_name = (char*) (malloc(sizeof(char) * ((str_length)+1) ) ); if(!(str_name)) {on_err}}
#define		HMD_CREATE_STRING_FROM_STRING_ERR(str_name, str, on_err)	{HMD_CREATE_STRING_ERR(str_name, strlen(str), on_err); strcpy((str_name), str);}
#define 	DEFAULT_LEN	256
#define THREAD_NUM	16
void HmdStateReInit();
int read_ac_file(char *FILENAME,char *buff,int blen);
HMDBool HmdGetMsgQueue(int *msgqid);
void hmd_pid_write_v3(int slot_num,int instID,int islocal);
int HmdNoticeToClient(int slotid,int InstID,int localid,int op);
int configuration_server_to_client(int slotid,int profile, int neighbor_slotid);
int HmdSetAssambleHansiMsg(int slotid,int InstID,int localid,int op);
void hmd_pid_write_v2(char *name,int slot_num);
int hmd_load_slot_config(char *dir, char *wildfile, int waiting);
void HmdStateReInitSlot(int slotid);
int Set_Interface_binding_Info(char * ifname,char flag);//fengwenchao copy from 1318 for AXSSZFI-839
int HmdNoticeToClient_ForClearIfname(int slotid,char *ifname,int op);//fengwenchao copy from 1318 for AXSSZFI-839
int HmdNoticeToClient_ForBakForeverConfig(struct HmdMsg *hmdmsg,int op);
int Set_hmd_bakup_foreve_config(struct HmdMsg *hmdmsg);
int hmd_config_save_timer_init(int method_flag);//fengwenchao add for hmd timer config save
#endif
