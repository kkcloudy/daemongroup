/* trap-signal.c */

#include <stdlib.h>
#include <dbus/dbus.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>
#include "trap-util.h"
#include "trap-def.h"
#include "nm_list.h"
#include "trap-list.h"
#include "hashtable.h"
#include "trap-descr.h"
#include "trap-data.h"
#include "ws_snmpd_engine.h"
#include "trap-resend.h"
#include "trap-hash.h"
#include "trap-signal.h"

TrapSignal *trap_signal_new(const char *signal_name,
								TrapSignalHandleFunc func)
{
	TrapSignal *tSignal = NULL;
	
	tSignal = malloc(sizeof(*tSignal));
	if (NULL == tSignal)
		return NULL;
	
	tSignal->signal_name = signal_name;
	tSignal->signal_handle_func = func;

	return tSignal;
}

void trap_signal_free(TrapSignal *tSignal)
{
	if (tSignal != NULL)
		free(tSignal);
}

void trap_signal_list_init(TrapList *list)
{
	trap_list_init(list);
}

void trap_signal_list_destroy(TrapList *list)
{
	trap_list_destroy(list, (TrapFreeFunc)trap_signal_free);
}


TrapSignal *trap_signal_list_register(TrapList *list,
							const char *signal_name,
							TrapSignalHandleFunc func)
{
	TrapSignal *tSignal = NULL;

	tSignal = trap_signal_new(signal_name, func);
	
	if (tSignal == NULL)
		return NULL;
	
	trap_list_append(list, tSignal);
	
	return tSignal;
}

#if 0
void trap_signal_list_register(TrapList *list,
							const char *signal_name,
							TrapSignalHandleFunc func)
{
	TrapSignal *tSignal = NULL;

	tSignal = trap_signal_new(signal_name, func);
	if (tSignal != NULL)
		trap_list_append(list, tSignal);
}

#endif 

int trap_init_signal_hash_node(TrapSignal *signal)
{	
	if (NULL == signal){
		TRAP_TRACE_LOG(LOG_INFO,"describe is NULL error!\n");
		return TRAP_ERROR;
	}
	
	INIT_HLIST_NODE(&(signal->signal_hash_node));
}

int trap_signal_hashtable_add_node(hashtable *ht, const void *key, unsigned int len, struct hlist_node *node)
{
	TRAP_RETURN_VAL_IF_FAILED(TRAP_OK==hashtable_add_node(ht, key, len, node),TRAP_ERROR,LOG_INFO);
	return TRAP_OK;
}

TrapSignal *trap_signal_hashtable_get_item(hashtable *ht,
								const char *signal_name)
{
	TrapSignal *tSignal = NULL;
	struct hlist_head *hash_head=NULL;
	struct hlist_node *hlist_node=NULL;
	
	hash_head = hashtable_get_hash_list(ht, signal_name, strlen(signal_name));
	
	hlist_for_each_entry(tSignal , hlist_node, hash_head, signal_hash_node ){
		if (strcmp(tSignal->signal_name, signal_name) == 0){
			trap_syslog(LOG_DEBUG,"find in hash list signal_name is %s\n",tSignal->signal_name);
			return tSignal;
		}
	}
	return NULL;
}

TrapSignal *
trap_proxy_hashtable_get_item(hashtable *ht, const char *signal_name) {

    TrapSignal *tSignal = NULL;
	struct hlist_head *hash_head=NULL;
	struct hlist_node *hlist_node=NULL;
	
	hash_head = hashtable_get_hash_list(ht, signal_name, strlen(signal_name));
	
	hlist_for_each_entry(tSignal , hlist_node, hash_head, signal_hash_node ){
		if (strcmp(tSignal->signal_name, signal_name) == 0){
			trap_syslog(LOG_DEBUG,"find in hash proxy list signal_name is %s\n", tSignal->signal_name);
			return tSignal;
		}
	}
	return NULL;
}




