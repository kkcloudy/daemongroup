#ifndef _TRAP_RESEND_H_
#define _TRAP_RESEND_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "ws_dbus_def.h"

#define TRAP_RESEND_INTERVAL 	300
#define TRAP_RESEND_TIMES		3


typedef struct TrapResendList_t{
	struct list_head resend_list_head;
	int interval;
	int resend_times;
}TrapResendList;

typedef struct TrapResendNode_t{
	struct list_head resend_list_node;
	unsigned int local_id;
	unsigned int instance_id;
	unsigned char flag;	// 1 stand for ac trap 0 is default
	int resend_no;
	time_t last_resend_time;
	void *private;
	struct TrapResendItem_t *ptr;
	TrapData *tData;
}TrapResendNode;

extern TrapResendList gTrapResendList;

/****trap resend check***/
typedef struct TrapResendCheckNode_t{
	struct list_head check_list_node;
	void *check_data;
	void *ptr;
	void (*free_check_data)(void *);
	
}TrapResendCheckNode;

typedef struct TrapResendItem_t{
	TrapDescr *Descr;
	TrapResendNode *resend_node;
}TrapResendItem;

typedef struct ApTrap_t{
	unsigned int num;
	unsigned int local_id;
	unsigned int instance_id;
	TrapResendItem aptrap_item[0];
}ApTrap;

typedef struct Wtp_t{
	unsigned int num;		//max wtp num
	ApTrap *ap[0];	//depands wtp id
}Wtp;

typedef struct AcTrap_t{
	unsigned int num;
	TrapResendItem actrap_item[0];
}AcTrap;

typedef struct Ins_trap_t{
	//int instance_id;
	Wtp *wtp;
	AcTrap *actrap;
}Ins_trap;

typedef struct Global_t{
	TrapList gSignalList;
	TrapList gDescrList;
	TrapList gProxyList;
	//TrapResendList gTrapResendList;
	hashtable *gSignalList_hash;
	hashtable *gDescrList_hash;
	hashtable *gProxyList_hash;
	Wtp *wtp[VRRP_TYPE_NUM][INSTANCE_NUM + 1];
	AcTrap *actrap;
	//Ins_trap Ins_trap[MAXINSTANCE+1];
}Global;

extern Global global;



#define RNTRAP(p)    ((TrapResendNode *)p)


int trap_init_resend ( TrapResendList *trap_resend_list );

int trap_init_resend_node(TrapResendNode **resend_node, TrapData **tData, unsigned int local_id, unsigned int instance_id);

int trap_resend_enabled (TrapResendList *trap_resend_list);

int trap_resend_remove_node(TrapResendNode *resend_node);

int trap_resend_append(TrapResendNode *resend_node,struct list_head *head);

void trap_resend_free_node(TrapResendNode **resend_node);

void trap_resend_node_update(TrapResendNode *resend_node,int interval);

int trap_resend(TrapResendList *trap_resend_list);

TrapResendCheckNode *trap_check_in_resend_list(struct list_head *check_head, int (*check_function)(void *,void*),void *check_data);

int trap_resend_check_remove_node(TrapResendCheckNode *check_node);

void trap_resend_check_free_node(TrapResendCheckNode **check_node);

int trap_init_resend_check_node(TrapResendCheckNode *check_node ,void **check_data, void *ptr, void (*check_data_free)(void *) );

void trap_destory_wtp_by_instance( struct Global_t *global, unsigned int local_id, unsigned int instance_id );


#define TRAP_SIGNAL_AP_RESEND_UPPER_MACRO(wtpindex, tDescr, local_id, instance_id) do { \
    if(local_id >= VRRP_TYPE_NUM || 0 == instance_id || instance_id > INSTANCE_NUM)    \
        return TRAP_ERROR;  \
	if ( NULL == global.wtp[local_id][instance_id]  || wtpindex > global.wtp[local_id][instance_id]->num || wtpindex <= 0 )	\
		return TRAP_ERROR;	\
	if ( NULL != global.wtp[local_id][instance_id]->ap[wtpindex] && tDescr == global.wtp[local_id][instance_id]->ap[wtpindex]->aptrap_item[tDescr->trap_index].Descr )	\
		return TRAP_SIGNAL_HANDLE_TRAP_SENDED;		\
}while (0)

#define TRAP_SIGNAL_AP_RESEND_LOWER_MACRO(wtpindex , tDescr, tData, local_id, instance_id) do {	\
		if (!trap_resend_enabled(&gTrapResendList)){	\
			trap_data_destroy(tData);		\
			break;  \
		}	\
		if ( NULL == global.wtp[local_id][instance_id]->ap[wtpindex]){		\
			if(TRAP_OK != trap_init_aptrap(&(global.wtp[local_id][instance_id]->ap[wtpindex]), MAXAPTRAPNUM, local_id, instance_id), TRAP_ERROR ,LOG_INFO)\
				TRAP_TRACE_LOG(LOG_DEBUG,"trap init aptrap failed!\n");	\
				break;  \
		}	\
		if ( tDescr != global.wtp[local_id][instance_id]->ap[wtpindex]->aptrap_item[tDescr->trap_index].Descr){	\
			if ( NULL != global.wtp[local_id][instance_id]->ap[wtpindex]->aptrap_item[tDescr->trap_index].resend_node){	\
				trap_resend_remove_node(global.wtp[local_id][instance_id]->ap[wtpindex]->aptrap_item[tDescr->trap_index].resend_node);	\
				trap_resend_free_node(&(global.wtp[local_id][instance_id]->ap[wtpindex]->aptrap_item[tDescr->trap_index].resend_node));	\
				TRAP_TRACE_LOG(LOG_INFO,"trap resend remove and free node!\n");	\
				TrapResendNode *resend_node=NULL;	\
				trap_init_resend_node(&resend_node, &tData, local_id, instance_id);	\
				trap_set_resend_item ( &(global.wtp[local_id][instance_id]->ap[wtpindex]->aptrap_item[tDescr->trap_index]), tDescr, resend_node);	\
				trap_resend_append( resend_node, &(gTrapResendList.resend_list_head) );	\
			}else if (NULL == global.wtp[local_id][instance_id]->ap[wtpindex]->aptrap_item[tDescr->trap_index].resend_node)	\
			{	\
				TrapResendNode *resend_node=NULL;	\
				trap_init_resend_node(&resend_node, &tData, local_id, instance_id);	\
				trap_set_resend_item( &(global.wtp[local_id][instance_id]->ap[wtpindex]->aptrap_item[tDescr->trap_index]), tDescr, resend_node);	\
				trap_resend_append( resend_node, &(gTrapResendList.resend_list_head) );	\
			}else {	\
				TRAP_TRACE_LOG(LOG_DEBUG,"Unknown error!\n");	\
				break;  \
			}	\
		}else {		\
			TRAP_TRACE_LOG(LOG_DEBUG,"Unknown error!\n");	\
			break;  \
		}	\
}while (0)


#define TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr) do {	\
	if ( NULL != global.actrap && tDescr == global.actrap->actrap_item[tDescr->trap_index].Descr ){	\
			return TRAP_SIGNAL_HANDLE_TRAP_SENDED;		\
	}	\
}while (0)


#define TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, local_id, instance_id) do {	\
		if (!trap_resend_enabled(&gTrapResendList)){	\
			trap_data_destroy(tData);		\
			break; \
		}	\
		if ( NULL == global.actrap){ 	\
			TRAP_TRACE_LOG(LOG_INFO,"global.actrap is empty!");	\
			/*TRAP_RETURN_VAL_IF_FAILED(TRAP_OK == trap_init_ac_array(&(global.actrap), MAXACTRAPNUM), TRAP_ERROR ,LOG_INFO);*/\
			break;	\
		}	\
        if(local_id >= VRRP_TYPE_NUM || 0 == instance_id || instance_id > INSTANCE_NUM)    \
            break;  \
		if ( tDescr != global.actrap->actrap_item[tDescr->trap_index].Descr){	\
			if ( NULL != global.actrap->actrap_item[tDescr->trap_index].resend_node){	\
				trap_resend_remove_node(global.actrap->actrap_item[tDescr->trap_index].resend_node); \
				trap_resend_free_node(&(global.actrap->actrap_item[tDescr->trap_index].resend_node));	\
				TRAP_TRACE_LOG(LOG_INFO,"trap resend remove and free node!\n"); \
				TrapResendNode *resend_node = NULL;	\
				trap_init_resend_node(&resend_node, &tData, local_id, instance_id);	\
				trap_set_ac_flag(resend_node);	\
				trap_set_resend_item ( &(global.actrap->actrap_item[tDescr->trap_index]), tDescr, resend_node);	\
				trap_resend_append( resend_node, &(gTrapResendList.resend_list_head) ); \
			}else if (NULL == global.actrap->actrap_item[tDescr->trap_index].resend_node)	\
			{	\
				TrapResendNode *resend_node=NULL;	\
				trap_init_resend_node(&resend_node, &tData, local_id, instance_id);	\
				trap_set_ac_flag(resend_node);	\
				trap_set_resend_item( &(global.actrap->actrap_item[tDescr->trap_index]), tDescr, resend_node);	\
				trap_resend_append( resend_node, &(gTrapResendList.resend_list_head) ); \
			}else { \
				TRAP_TRACE_LOG(LOG_DEBUG,"Unknown error!\n");	\
				break;	\
			}	\
		}else { 	\
			TRAP_TRACE_LOG(LOG_DEBUG,"Unknown error!\n");	\
			break;	\
		}	\
}while (0)


#ifdef  __cplusplus
}
#endif

#endif

