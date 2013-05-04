/* trap-signal.h */

#ifndef TRAP_SIGNAL_H
#define TRAP_SIGNAL_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef int (* TrapSignalHandleFunc)(DBusMessage *message);

typedef struct TrapSignal_t {
	const char *signal_name;
	TrapSignalHandleFunc signal_handle_func;
	struct hlist_node signal_hash_node;
} TrapSignal;

#define INIT_SIGNAL_HASH_LIST( tSignal_tmp , ht ) do{		\
	trap_init_signal_hash_node(tSignal_tmp);	\
	trap_signal_hashtable_add_node(ht,tSignal_tmp->signal_name,strlen(tSignal_tmp->signal_name),&(tSignal_tmp->signal_hash_node)); \
}while (0)

TrapSignal *trap_signal_new(const char *signal_name,
							TrapSignalHandleFunc func);

void trap_signal_free(TrapSignal *tSignal);

void trap_signal_list_init(TrapList *list);

void trap_signal_list_destroy(TrapList *list);

TrapSignal *trap_signal_list_register(TrapList *list,
							const char *signal_name,
							TrapSignalHandleFunc func);
#if 0
void trap_signal_list_register(TrapList *list,
							const char *signal_name,
							TrapSignalHandleFunc func);
#endif

int trap_init_signal_hash_node(TrapSignal *signal);

int trap_signal_hashtable_add_node(hashtable *ht, const void *key, unsigned int len, struct hlist_node *node);

TrapSignal *trap_signal_hashtable_get_item(hashtable *ht, const char *signal_name);


#ifdef  __cplusplus
}
#endif

#endif /* TRAP_SIGNAL_H */

