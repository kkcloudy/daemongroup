/* trap-descr.c */

#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include "trap-util.h"
#include "nm_list.h"
#include "hashtable.h"
#include "trap-list.h"
#include "trap-def.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "trap-descr.h"


TrapList gDescrList = {0};

TrapDescr *trap_descr_new(char *trap_name,
							int trap_index,
							int event_source,
							char *trap_oid,
							char *trap_type,
							char *trap_level,
							char *title,
							char *content)
{
	TrapDescr *tDescr = NULL;
	
	tDescr = malloc(sizeof(*tDescr));
	if (NULL == tDescr)
		return NULL;

	memset(tDescr,0,sizeof(*tDescr));
	
	tDescr->trap_index = trap_index;
	tDescr->trap_name = trap_name;
	tDescr->event_source = event_source;
	tDescr->trap_oid = trap_oid;
	tDescr->trap_type = trap_type;
	tDescr->trap_level = trap_level;
	tDescr->trap_status = 1;
	tDescr->title = title;
	tDescr->content = content;
	tDescr->switch_status = 1;
	
	return tDescr;
}

#if 0
TrapDescr *trap_descr_new(char *trap_name,
							int event_source,
							char *trap_oid,
							char *trap_type,
							char *trap_level,
							char *title,
							char *content)
{
	TrapDescr *tDescr = NULL;
	
	tDescr = malloc(sizeof(*tDescr));
	if (NULL == tDescr)
		return NULL;

	tDescr->trap_name = trap_name;
	tDescr->event_source = event_source;
	tDescr->trap_oid = trap_oid;
	tDescr->trap_type = trap_type;
	tDescr->trap_level = trap_level;
	tDescr->trap_status = 1;
	tDescr->title = title;
	tDescr->content = content;
	tDescr->switch_status = 1;
	
	return tDescr;
}
#endif

void trap_descr_free(TrapDescr *tDescr)
{
	if (tDescr != NULL)
		free(tDescr);
}

void trap_descr_list_init(TrapList *list)
{
	trap_list_init(list);
}

void trap_descr_list_destroy(TrapList *list)
{
	trap_list_destroy(list, (TrapFreeFunc)trap_descr_free);
}

TrapDescr *trap_descr_list_register(TrapList *list,
							int trap_index,
							char *trap_name,
							int event_source,
							char *trap_oid,
							char *trap_type,
							char *trap_level,
							char *title,
							char *content)
{
	TrapDescr *tDescr = NULL;

	tDescr = trap_descr_new(trap_name, trap_index, event_source, trap_oid,
							trap_type, trap_level, title, content);
	if (tDescr == NULL)
		return NULL;
	
	trap_list_append(list, tDescr);
	
	return tDescr;
	
}

#if 0
TrapDescr *trap_descr_list_register(TrapList *list,
							char *trap_name,
							int event_source,
							char *trap_oid,
							char *trap_type,
							char *trap_level,
							char *title,
							char *content)
{
	TrapDescr *tDescr = NULL;

	tDescr = trap_descr_new(trap_name, event_source, trap_oid,
							trap_type, trap_level, title, content);
	if (tDescr == NULL)
		return NULL;
	
	trap_list_append(list, tDescr);
	
	return tDescr;
	
}
#endif 

#if 0 
int trap_descr_list_register(TrapList *list,
							char *trap_name,
							int event_source,
							char *trap_oid,
							char *trap_type,
							char *trap_level,
							char *title,
							char *content)
{
	TrapDescr *tDescr = NULL;

	tDescr = trap_descr_new(trap_name, event_source, trap_oid,
							trap_type, trap_level, title, content);
	if (tDescr != NULL)
		trap_list_append(list, tDescr);
	
}

#endif


TrapDescr *trap_descr_list_get_item(hashtable *ht,
								const char *trap_name)
{
	TrapDescr *tDescr = NULL;
	struct hlist_head *hash_head=NULL;
	struct hlist_node *hlist_node=NULL;
	
	hash_head = hashtable_get_hash_list(ht, trap_name, strlen(trap_name));
	
	hlist_for_each_entry(tDescr , hlist_node, hash_head, descr_hash_node ){
		if (strcmp(tDescr->trap_name, trap_name) == 0){
			trap_syslog(LOG_DEBUG,"find in hash list trap_name is %s\n",tDescr->trap_name);
			return tDescr;
		}
	}
	return NULL;
}

#if 0
TrapDescr *trap_descr_list_get_item(TrapList *list,
								const char *trap_name)
{
	TrapNode *tNode=NULL;
	TrapDescr *tDescr = NULL;

	for (tNode = list->first; tNode; tNode = tNode->next) {
		tDescr = tNode->data;
		if (strcmp(tDescr->trap_name, trap_name) == 0)
			return tDescr;
	}

	return NULL;
}
#endif

void trap_descr_load_switch(TrapList *list)
{    
	TRAP_DETAIL_CONFIG *trapConf_array = NULL;
	unsigned int trapConf_num = 0;

    int ret = ac_manage_show_trap_switch(ccgi_dbus_connection, &trapConf_array, &trapConf_num);
    trap_syslog(LOG_INFO,"trap_descr_load_switch:ac_manage_show_trap_switch  ret (%d), trapConf_num (%d)\n", ret, trapConf_num);

    if(AC_MANAGE_SUCCESS == ret && trapConf_array) {

	    TrapNode *tNode = NULL;
	    TrapDescr *tDescr = NULL;
        
    	for (tNode = list->first; tNode; tNode = tNode->next){
    		tDescr = tNode->data;
    		if (NULL != tDescr) {
    		    int i = 0;
    			for (i = 0; i < trapConf_num; i++) {
    				if(0 == strcmp(trapConf_array[i].trapName, tDescr->trap_name)){
    					tDescr->switch_status = trapConf_array[i].trapSwitch;
                        trap_syslog(LOG_INFO,"trap_descr_load_switch:tDescr->trap_name = %s, switch is %s\n", 
                                                tDescr->trap_name, tDescr->switch_status ? "on" : "off");
    					break;
    				}
    			}
    		}	
    	}
    }

    MANAGE_FREE(trapConf_array);
    return ;
}

int trap_init_descr_hash_node ( TrapDescr *describe )
{
	if (NULL == describe){
		TRAP_TRACE_LOG(LOG_INFO,"describe is NULL error!\n");
		return TRAP_ERROR;
	}
	
	INIT_HLIST_NODE(&(describe->descr_hash_node));
}

int trap_descr_hashtable_add_node(hashtable *ht, const void *key, unsigned int len, struct hlist_node *node)
{
	TRAP_RETURN_VAL_IF_FAILED(TRAP_OK==hashtable_add_node(ht, key, len, node),TRAP_ERROR,LOG_INFO);
	return TRAP_OK;
}

