#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include "trap-util.h"
#include "nm_list.h"
#include "trap-list.h"
#include "hashtable.h"
#include "trap-descr.h" //must include "hashtable.h"
#include "trap-data.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "ac_manage_def.h"
#include "trap-receiver.h"
#include "trap-instance.h"
#include "trap-resend.h"

TrapResendList gTrapResendList = {0};
Global global={0};

int trap_init_resend ( TrapResendList *trap_resend_list ) 
{
	//TRAP_RESEND_INTERVAL, TRAP_RESEND_TIMES set default
	int interval = TRAP_RESEND_INTERVAL;
	int resend_times = TRAP_RESEND_TIMES;
	
    TRAPParameter *parameter_array = NULL;
    unsigned int parameter_num = 0;
    int ret = ac_manage_show_trap_parameter(ccgi_dbus_connection, &parameter_array, &parameter_num);
	if(AC_MANAGE_SUCCESS == ret) {
        int i = 0;
        for(i = 0; i < parameter_num; i++) {
            if(0 == strcmp(RESEND_INTERVAL, parameter_array[i].paraStr)) {
                interval = parameter_array[i].data;
            }
            else if(0 == strcmp(RESEND_TIMES, parameter_array[i].paraStr)) {
                resend_times = parameter_array[i].data;
            }
        }
	}

		
	trap_resend_list->interval = interval;
	trap_resend_list->resend_times = resend_times;
	INIT_LIST_HEAD(&trap_resend_list->resend_list_head);

	trap_syslog(LOG_DEBUG,"trap_init_resend: trap resend interval is (%d), trap resend times is (%d).\n", interval ,resend_times );
	return 0;
}

int trap_init_resend_node(TrapResendNode **resend_node, TrapData **tData, unsigned int local_id, unsigned int instance_id)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	if (NULL == resend_node )
		return TRAP_ERROR;
	*resend_node=(TrapResendNode *)TRAP_MALLOC(sizeof(TrapResendNode));

	if (NULL == *resend_node)
		return TRAP_MALLOC_ERROR;
	
	TrapResendNode *resend_node_real=*resend_node;
	memset(resend_node_real, 0, sizeof(TrapResendNode));
	
	if(local_id >= VRRP_TYPE_NUM || 0 == instance_id || instance_id > INSTANCE_NUM)
		return TRAP_ERROR;
		
    resend_node_real->local_id = local_id;
	resend_node_real->instance_id = instance_id;

	resend_node_real->flag = 0;
	
	if (NULL == tData){
		TRAP_TRACE_LOG(LOG_INFO,"tData is NULL\n");
		resend_node_real->tData=NULL;
	}else if(NULL != *tData){
		resend_node_real->tData=*tData;
		*tData=NULL;
	}else{
		TRAP_TRACE_LOG(LOG_INFO,"tData is NULL\n");
		resend_node_real->tData=NULL;
	}
	
	INIT_LIST_HEAD(&resend_node_real->resend_list_node);
	resend_node_real->resend_no=0;
	resend_node_real->last_resend_time=time(NULL);
	return TRAP_OK;
}

int trap_resend_enabled (TrapResendList *trap_resend_list)
{
	//TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	
	if(0 == trap_resend_list->resend_times ||0 == trap_resend_list->interval)
		return TRAP_FALSE;
	
	//TRAP_TRACE_LOG(LOG_DEBUG,"resend interval is %d resend times is %d\n", trap_resend_list->interval, trap_resend_list->resend_times);
	
	return TRAP_TRUE;
}

int trap_resend_remove_node(TrapResendNode *resend_node)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	if (NULL== resend_node)
		return TRAP_ERROR;
	list_del(&(resend_node->resend_list_node));
	TRAP_TRACE_LOG(LOG_DEBUG,"resend list delete succeed.\n");
	return TRAP_OK;
}

int trap_resend_append(TrapResendNode *resend_node,struct list_head *head)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	if (NULL== resend_node||NULL==head)
		return TRAP_ERROR;
	TRAP_TRACE_LOG(LOG_DEBUG,"resend_node=%p",resend_node);
	list_add_tail(LPNLNODE(resend_node), head);
	TRAP_TRACE_LOG(LOG_DEBUG,"resend_node append list head %p\n",head);
	return TRAP_OK;
}

void trap_resend_free_node(TrapResendNode **resend_node)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry\n");
	if (NULL == resend_node || NULL == *resend_node)
		return;
	TrapResendNode *resend_node_real=*resend_node;
	TrapResendNode **resend_node_ptr=NULL;
	if (NULL != resend_node_real->ptr){
		
		TRAP_TRACE_LOG(LOG_DEBUG,"resend_node->ptr=%p\n",resend_node_real->ptr);
		resend_node_ptr=&(resend_node_real->ptr->resend_node);
	}
	TRAP_TRACE_LOG(LOG_DEBUG,"resend_node=%p\n",resend_node_real);
	TRAP_TRACE_LOG(LOG_DEBUG,"resend_node->tData=%p\n",resend_node_real->tData);
	if (NULL != resend_node_real->tData){
		trap_data_destroy(resend_node_real->tData);
		resend_node_real->tData=NULL;
	}
	TRAP_TRACE_LOG(LOG_DEBUG,"resend_node->tData=%p\n",resend_node_real->tData);
	TRAP_FREE(*resend_node);
	*resend_node_ptr=NULL;
	TRAP_TRACE_LOG(LOG_DEBUG,"resend_node=%p\n",*resend_node);
	return;
}

void trap_resend_node_update(TrapResendNode *resend_node,int interval)
{
	resend_node->last_resend_time+=interval;
	resend_node->resend_no++;
}

int trap_resend(TrapResendList *trap_resend_list)
{
	TrapResendNode *ResendNode=NULL;
	//TrapResendNode *pos_tmp =NULL;
	time_t diff_time=0;
	unsigned int ins_id=0;

	if (NULL == trap_resend_list)
		return TRAP_ERROR;
#if 0
	if (list_empty(&trap_resend_list->resend_list_head)){
		TRAP_TRACE_LOG(LOG_DEBUG,"resend list empty!\n");
		return TRAP_OK;
	}
#endif

	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	
	list_for_each_safe(pos, next, LPNLNODE(trap_resend_list))
	{	
		ResendNode = RNTRAP(pos);
		
		//if ( NULL==ResendNode )
		//	return TRAP_ERROR;
		
		if(!trap_instance_is_master(&gInsVrrpState, ResendNode->local_id, ResendNode->instance_id))
		{
			TRAP_TRACE_LOG(LOG_DEBUG,"trap instance is not master, ResendNode->local_id = %d, ResendNode->instance_id = %d\n",
			                            ResendNode->local_id, ResendNode->instance_id);
			
			trap_resend_remove_node(ResendNode);
			trap_resend_free_node(&ResendNode);
			continue;
		}

		diff_time=difftime(time(NULL),ResendNode->last_resend_time);
		TRAP_TRACE_LOG(LOG_INFO,"difftime of resend is %ld interval is %d resend num is %d resend times is %d\n",diff_time,trap_resend_list->interval,ResendNode->resend_no,trap_resend_list->resend_times);
		
		if (diff_time > trap_resend_list->interval ||diff_time < 0) //if system time changed resend maybe crashed
		{
			if ( ResendNode->resend_no <= trap_resend_list->resend_times)
			{
				TRAP_TRACE_LOG(LOG_DEBUG,"resend_list->resend=%p\n",ResendNode->ptr);
				
				trap_send(gInsVrrpState.instance[ResendNode->local_id][ResendNode->instance_id].receivelist, &gV3UserList, ResendNode->tData);
				trap_resend_node_update(pos, trap_resend_list->interval);
				
				if(ResendNode->resend_no == trap_resend_list->resend_times)
				{
					trap_resend_remove_node(ResendNode);
					trap_resend_free_node(&ResendNode);
					TRAP_TRACE_LOG(LOG_INFO,"trap resend remove and free nodes!\n");
					
				}else{
					trap_resend_remove_node(ResendNode);
					trap_resend_append(ResendNode,LPNLNODE(trap_resend_list));					
				}
			}else{
			
				trap_resend_remove_node(ResendNode);
				trap_resend_free_node(&ResendNode);
				TRAP_TRACE_LOG(LOG_INFO,"trap resend remove and free nodes!\n");
			}
			TRAP_TRACE_LOG(LOG_INFO,"resend\n");
		}else{
			return TRAP_OK;
		}
	}
	return TRAP_OK;

}

int trap_init_ap_item (TrapResendItem **aptrapitem , TrapDescr *tDescr, TrapResendNode *resend_node) //malloc ApTrapItem init TrapDescr TrapResendNode and TrapResendNode->ptr point to ApTrapItem
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	
	if (NULL == aptrapitem )
		return TRAP_ERROR;

	*aptrapitem = (TrapResendItem *)TRAP_MALLOC(sizeof(TrapResendItem));

	if (NULL == *aptrapitem)
		return TRAP_MALLOC_ERROR;
	
	TrapResendItem *aptrapitem_real=NULL;
	aptrapitem_real=*aptrapitem;
	
	memset (aptrapitem_real, 0, sizeof(TrapResendItem));
	
	if (tDescr)
		aptrapitem_real->Descr=tDescr;

	if (resend_node){
		aptrapitem_real->resend_node=resend_node;
		resend_node->ptr=aptrapitem_real;
	}
	return TRAP_OK;

}

int trap_set_resend_item (TrapResendItem *trap_item, TrapDescr *tDescr, TrapResendNode *resend_node)
{
	if (NULL == trap_item)
		return TRAP_ERROR;

	if (tDescr)
		trap_item->Descr=tDescr;
	
	if (resend_node){
		trap_item->resend_node=resend_node;
		resend_node->ptr=trap_item;
	}
	
	return TRAP_OK;
}

int trap_init_aptrap (ApTrap **aptrap , unsigned int aptrap_max_num , unsigned int local_id, unsigned int instance_id)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	
	if (NULL == aptrap)
		return TRAP_ERROR;
	
	*aptrap=(ApTrap *)TRAP_MALLOC(sizeof(ApTrap) + (aptrap_max_num+1) * sizeof(TrapResendItem));
	
	if (NULL == *aptrap)
		return TRAP_MALLOC_ERROR;

	memset (*aptrap,0,sizeof(ApTrap) + (aptrap_max_num+1) * sizeof(TrapResendItem));

	(*aptrap)->num=aptrap_max_num;
	
	(*aptrap)->local_id = local_id;
	(*aptrap)->instance_id = instance_id;

	return TRAP_OK;
}

int trap_init_wtp_array (Wtp **wtp , unsigned int wtp_max_num)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	
	if (NULL == wtp)
		return TRAP_ERROR;

	*wtp=TRAP_MALLOC(sizeof(Wtp) + (wtp_max_num+1) * sizeof(ApTrap *));
	
	if (NULL == *wtp)
		return TRAP_MALLOC_ERROR;

	memset (*wtp, 0, sizeof(Wtp) + (wtp_max_num+1) * sizeof(ApTrap *));
	
	(*wtp)->num = wtp_max_num;

	TRAP_TRACE_LOG(LOG_DEBUG,"(*wtp)->num=%d",(*wtp)->num);
	
	return TRAP_OK;
}

int trap_init_ac_array (AcTrap **actrap, unsigned int max_ac_trap_num)
{
	if (NULL == actrap)
		return TRAP_ERROR;

	*actrap= (AcTrap *)TRAP_MALLOC(sizeof(AcTrap) +(max_ac_trap_num +1)*sizeof(TrapResendItem));

	if (NULL == *actrap)
		return TRAP_MALLOC_ERROR;

	memset (*actrap, 0, sizeof(AcTrap) + (max_ac_trap_num+1) * sizeof(TrapResendItem));
	
	(*actrap)->num=max_ac_trap_num;

	return TRAP_OK;
}

int trap_init_instance_array(Ins_trap *ins_trap)
{
	if ( NULL==ins_trap )
	{
		TRAP_TRACE_LOG(LOG_ERR,"ins_trap is NULL");
		return TRAP_RTN_NULL_POINT;
	}

	trap_init_ac_array ( &(ins_trap->actrap), MAXACTRAPNUM );
	trap_init_wtp_array ( &(ins_trap->wtp), MAXWTPID );

	return TRAP_RTN_OK;
}

void	trap_set_ac_flag (TrapResendNode *resend_node)
{
	if ( NULL == resend_node )
		return;

	resend_node->flag=1;

	return;
}

void trap_clear_resend_list(TrapResendList *trap_resend_list )
{	
	TrapResendNode *ResendNode=NULL;
	//TrapResendNode *pos_tmp =NULL;
	struct list_head *pos=NULL;
	struct list_head *next=NULL;
		
	list_for_each_safe(pos, next, LPNLNODE(trap_resend_list))
	{
		ResendNode=RNTRAP(pos);
		//if ( NULL==ResendNode )
		//	return ;
		
		trap_resend_remove_node(ResendNode);
		trap_resend_free_node(&ResendNode);
	}
	
	return ;
}

void trap_clear_actrap_items (AcTrap *actrap)
{
	int trap_index=0;
	int i=0;

	if ( NULL == actrap)
		return ;
	
	for (i=0 ; i <= actrap->num; i++){
		
		if (actrap->actrap_item[i].Descr){
			actrap->actrap_item[i].Descr=NULL;
		}
		
		if (NULL != actrap->actrap_item[i].resend_node){
			
			trap_resend_remove_node(actrap->actrap_item[i].resend_node);
			trap_resend_free_node(&(actrap->actrap_item[i].resend_node));
		}
	}

	return ;
}

void trap_vrrp_clear_actrap_items (hashtable *ht, AcTrap *actrap)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.");
	
	int trap_index=0;
	int i=0;
	TrapDescr *tDescr = NULL;

	if ( NULL == ht || NULL == actrap)
		return ;
	
	tDescr = trap_descr_list_get_item(ht, acTurntoBackupDeviceTrap);
	trap_index = tDescr->trap_index;
	
	for (i=0 ; i <= actrap->num; i++){

		if ( i == trap_index )
				continue;
		
		if (NULL != actrap->actrap_item[i].Descr){
			actrap->actrap_item[i].Descr=NULL;
		}

		if (NULL != actrap->actrap_item[i].resend_node){
			trap_resend_remove_node(actrap->actrap_item[i].resend_node);
			trap_resend_free_node(&(actrap->actrap_item[i].resend_node));
		}
	}

	return ;
}

void trap_ap_offline_clear_aptrap_items (hashtable *ht, ApTrap *aptrap)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n", ht , aptrap);
	
	int trap_index=0;
	int i=0;
	TrapDescr *tDescr = NULL;

	if (NULL == ht || NULL == aptrap)
		return ;

	tDescr = trap_descr_list_get_item(ht, wtpOfflineTrap);
	trap_index = tDescr->trap_index;
	
	for (i=0 ; i <= aptrap->num ; i++){
		
		if (NULL != aptrap->aptrap_item[i].Descr){

			if (trap_index == i)
				continue;

			aptrap->aptrap_item[i].Descr=NULL;
		}

		if (NULL != aptrap->aptrap_item[i].resend_node){
			
			trap_resend_remove_node(aptrap->aptrap_item[i].resend_node);
			trap_resend_free_node(&(aptrap->aptrap_item[i].resend_node));
		}
	}

	return ;
}


void trap_clear_aptrap_items ( ApTrap *ap )
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	
	int trap_index=0;
	int i=0;
	TrapDescr *tDescr = NULL;

	if ( NULL == ap)
		return ;
	
	for (i=0 ; i <= ap->num ; i++)
	{
		if (NULL != ap->aptrap_item[i].Descr){

			ap->aptrap_item[i].Descr=NULL;
		}

		if (NULL != ap->aptrap_item[i].resend_node){
			
			trap_resend_remove_node(ap->aptrap_item[i].resend_node);
			trap_resend_free_node(&(ap->aptrap_item[i].resend_node));
		}
	}

	return ;
}

void trap_destroy_ap (Wtp *wtp)
{
	int i=0;
	
	if ( NULL == wtp )
		return ;

	for( i=0; i<=wtp->num ;i++ )
	{
		if ( NULL!=wtp->ap[i] )
		{
			TRAP_FREE( wtp->ap[i] );
		}
	}

	return ;
}

static void trap_clear_wtp ( Wtp *wtp )
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	
	int wtp_id=0;
	
	if ( NULL == wtp )
		return ;

	for( wtp_id=0; wtp_id<=wtp->num; wtp_id++ )
	{
		if ( NULL != wtp->ap[wtp_id] )
		{
			trap_clear_aptrap_items(wtp->ap[wtp_id]);
			TRAP_FREE(wtp->ap[wtp_id]);
		}
	}

	return ;
}

void trap_destory_wtp_by_instance ( Global *global, unsigned int local_id, unsigned int instance_id )
{
	if ( NULL == global || local_id >= VRRP_TYPE_NUM || 0 == instance_id || 
	    instance_id > INSTANCE_NUM || NULL == global->wtp[local_id][instance_id]) {
	    
		TRAP_TRACE_LOG(LOG_INFO,"%s", NULL==global? "global is NULL":"wtp is NULL");
		return ;
	}

	trap_clear_wtp(global->wtp[local_id][instance_id] );
	TRAP_FREE(global->wtp[local_id][instance_id]);

	return;	
}

