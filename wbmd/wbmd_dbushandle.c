
#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <dirent.h> 
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/utilities.h>
#include <net-snmp/net-snmp-includes.h>
#include "wbmd.h"
#include "wbmd/wbmdpub.h"
#include "dbus/wbmd/WbmdDbusDef.h"
#include "wbmd_thread.h"
#include "wbmd_dbus.h"
#include "wbmd_check.h"
#include "wbmd_manage.h"

struct wbridge_info * wbridge_get(unsigned int ip)
{
	struct wbridge_info *s;

	s = wb_hash.wBridge_hash[WB_HASH(ip)];
	while (s != NULL && s->IP != ip)
		s = s->hnext;
	return s;
}

void wbridge_hash_add(struct wbridge_info *wb)
{
	wb->hnext = wb_hash.wBridge_hash[WB_HASH(wb->IP)];
	wb_hash.wBridge_hash[WB_HASH(wb->IP)] = wb;
	wb_hash.wb_num += 1;
	return ;
}


void wbridge_hash_del(struct wbridge_info *wb)
{
	struct wbridge_info *s;

	s = wb_hash.wBridge_hash[WB_HASH(wb->IP)];
	if (s == NULL) return;
	if (wb->IP == s->IP) {
		wb_hash.wBridge_hash[WB_HASH(wb->IP)] = s->hnext;
		return;
	}

	while (s->hnext != NULL &&
	       wb->IP != s->hnext->IP)
		s = s->hnext;
	if (s->hnext != NULL){
		s->hnext = s->hnext->hnext;
		wb_hash.wb_num -= 1;
	}
	return ;
}

WBMDBool wbmd_id_check(int WBID){
	if(WBID < WBRIDGE_NUM && WBID > 0){
		if(wBridge[WBID] != NULL){
			return WBMD_TRUE;
		}
	}
	return WBMD_FALSE;
}

static void
optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'f':
                netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID, 
					  0);
                break;
            default:
                fprintf(stderr, "Unknown flag passed to -C: %c\n",
                        optarg[-1]);
                exit(1);
            }
        }
        break;
    }
}

void wbridge_init_session(int ID, int IP){	
	int i = 0;
	int ret = 0;
	for(i = 0; i < wBridge[ID]->argn; i++){
		wBridge[ID]->argv[i] = (char*)malloc(16);
		memset(wBridge[ID]->argv[i], 0, 16);
	}
	memcpy(wBridge[ID]->argv[0], "wbinit", 6);
	memcpy(wBridge[ID]->argv[1], "-v", 2);
	memcpy(wBridge[ID]->argv[2], "3", 1);
	memcpy(wBridge[ID]->argv[3], "-u", 2);
	memcpy(wBridge[ID]->argv[4], "test", 4);
	memcpy(wBridge[ID]->argv[5], "-l", 2);
	memcpy(wBridge[ID]->argv[6], "authNoPriv", 10);
	memcpy(wBridge[ID]->argv[7], "-a", 2);
	memcpy(wBridge[ID]->argv[8], "MD5", 3);
	memcpy(wBridge[ID]->argv[9], "-A", 2);
	memcpy(wBridge[ID]->argv[10], "12345678", 8);
	strcpy(wBridge[ID]->argv[11],inet_ntoa((wBridge[ID]->wbaddr.sin_addr)));
	ret = snmp_parse_args(wBridge[ID]->argn, wBridge[ID]->argv, &(wBridge[ID]->session), "C:", optProc);
	if(ret < 0 ){
		wbmd_syslog_crit("wbridge %d snmp session init failed",ID);
	}
}

int wbmd_wbridge_create(int ID, int IP){
	if(wbridge_get(IP) != NULL){
		return WBMD_DBUS_ID_EXIST;
	}
	wBridge[ID] = (struct wbridge_info *)malloc(sizeof(struct wbridge_info));
	if(wBridge[ID] == NULL){
		return WBMD_DBUS_OUT_OF_MEMORY;
	}
	memset(wBridge[ID], 0, sizeof(struct wbridge_info));
	wBridge[ID]->WBID = ID;
	wBridge[ID]->IP = IP;
	wBridge[ID]->wbaddr.sin_family=AF_INET;
	wBridge[ID]->wbaddr.sin_addr.s_addr = IP;
	wBridge[ID]->WBState = 0;//0 quit 1 run
	wBridge[ID]->PackNo = 100+ID;
	wBridge[ID]->argn = 0;
	wBridge[ID]->GetIfInfoTimes = 1;
	wBridge[ID]->GetMintInfoTimes = 1;
	wBridge[ID]->GetRfInfoTimes = 1;
	wbridge_hash_add(wBridge[ID]);
	//wbridge_init_session(ID,IP);
	WBMDTimerRequest(5,&(wBridge[ID]->CheckTimerID),WBMD_CHECKING,ID);
	return WBMD_DBUS_SUCCESS;
}

int wbmd_wbridge_delete(int ID){
	if(wBridge[ID] == NULL){
		return WBMD_DBUS_ID_NO_EXIST;
	}
	WbmdTimerCancel(&(wBridge[ID]->CheckTimerID),1);
	WbmdTimerCancel(&(wBridge[ID]->GetIfInfoTimerID),1);
	WbmdTimerCancel(&(wBridge[ID]->GetMintInfoTimerID),1);
	WbmdTimerCancel(&(wBridge[ID]->GetRfInfoTimerID),1);
	wbridge_hash_del(wBridge[ID]);
	free(wBridge[ID]);
	wBridge[ID] = NULL;
	return WBMD_DBUS_SUCCESS;
}


void clean_wbridge_snmp_info(int ID){
	int i = 0;
	if(wBridge[ID]->argn != 0){
		for(i = 0; i< wBridge[ID]->argn; i++){
			if(wBridge[ID]->argv[i] != NULL){
				free(wBridge[ID]->argv[i]);
				wBridge[ID]->argv[i]=NULL;
			}
		}
		wBridge[ID]->argn  = 0;
	}
}

int wbmd_wbridge_snmp_init(int ID, char ** argv, int argvn){
	int i = 0;
	int ret = 0;	
	clean_wbridge_snmp_info(ID);
	
	if(wBridge[ID]->WBState){
		WbmdTimerCancel(&(wBridge[ID]->GetIfInfoTimerID),1);
		WbmdTimerCancel(&(wBridge[ID]->GetMintInfoTimerID),1);
		WbmdTimerCancel(&(wBridge[ID]->GetRfInfoTimerID),1);					
		wBridge[ID]->GetIfInfoTimes = 1;
		wBridge[ID]->GetMintInfoTimes = 1;
		wBridge[ID]->GetRfInfoTimes = 1;
	}	
	wBridge[ID]->argn = argvn+2;	
	wBridge[ID]->argv[0] = (char*)malloc(16);
	memset(wBridge[ID]->argv[0], 0, 16);	
	memcpy(wBridge[ID]->argv[0], "wbinit", 6);
	for(i = 1; i < argvn+1; i++){
		wBridge[ID]->argv[i] = (char*)malloc(strlen(argv[i])+1);
		memset(wBridge[ID]->argv[i], 0, strlen(argv[i])+1);
		strcpy(wBridge[ID]->argv[i],argv[i]);
	}
	wBridge[ID]->argv[i] = (char*)malloc(16);
	memset(wBridge[ID]->argv[i], 0, 16);	
	strcpy(wBridge[ID]->argv[i],inet_ntoa((wBridge[ID]->wbaddr.sin_addr)));
	ret = snmp_parse_args(wBridge[ID]->argn, wBridge[ID]->argv, &(wBridge[ID]->session), "C:", optProc);
	if(ret < 0 ){
		clean_wbridge_snmp_info(ID);
		wbmd_syslog_crit("wbridge %d snmp session init failed",ID);
		return WBMD_DBUS_ERROR;
	}	
	for(i = 1; i < argvn+1; i++){
		memset(wBridge[ID]->argv[i], 0, strlen(argv[i])+1);
		strcpy(wBridge[ID]->argv[i],argv[i]);
	}
	if(wBridge[ID]->WBState){		
		WbmdTimerCancel(&(wBridge[ID]->GetIfInfoTimerID),1);
		WbmdTimerCancel(&(wBridge[ID]->GetMintInfoTimerID),1);
		WbmdTimerCancel(&(wBridge[ID]->GetRfInfoTimerID),1);					
		WBMDTimerRequest(5,&(wBridge[ID]->GetIfInfoTimerID),WBMD_GETIFINFO,ID);
		WBMDTimerRequest(6,&(wBridge[ID]->GetMintInfoTimerID),WBMD_GETMINTINFO,ID);
		WBMDTimerRequest(7,&(wBridge[ID]->GetRfInfoTimerID),WBMD_GETRFINFO,ID);
	}
	return WBMD_DBUS_SUCCESS;
}

