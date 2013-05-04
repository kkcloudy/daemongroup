/* trap-descr.h */

#ifndef TRAP_DESCR_H
#define TRAP_DESCR_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
	TRAP_SRC_AC,
	TRAP_SRC_AP,
} TrapSource;

typedef struct TrapDescr_t {
	int trap_index;
	char *trap_name;
	int event_source;
	char *trap_oid;
	int switch_status;
	int frequency;
	char *trap_type;
	char *trap_level;
	char *event_time;
	int trap_status;
	char *title;
	char *content;
	struct hlist_node descr_hash_node;
} TrapDescr;
typedef struct VrrpValue_t {
	int interval;
	int profile;
} VrrpValue;

extern TrapList gDescrList;

#define INIT_DESCR_HASH_LIST( descr_tmp , ht ) do{		\
	trap_init_descr_hash_node(descr_tmp);	\
	trap_descr_hashtable_add_node(ht,descr_tmp->trap_name,strlen(descr_tmp->trap_name),&(descr_tmp->descr_hash_node)); \
}while (0)

TrapDescr *trap_descr_new(char *trap_name,
							int trap_index,
							int event_source,
							char *trap_oid,
							char *trap_type,
							char *trap_level,
							char *title,
							char *content);

void trap_descr_free(TrapDescr *tDescr);

void trap_descr_list_init(TrapList *list);

void trap_descr_list_destroy(TrapList *list);

TrapDescr *trap_descr_list_register(TrapList *list,
							int trap_index,
							char *trap_name,
							int event_source,
							char *trap_oid,
							char *trap_type,
							char *trap_level,
							char *title,
							char *content);

#if 0
int trap_descr_list_register(TrapList *list,
							char *trap_name,
							int event_source,
							char *trap_oid,
							char *trap_type,
							char *trap_level,
							char *title,
							char *content);
#endif

TrapDescr *trap_descr_list_get_item(hashtable *ht,
								const char *trap_name);

#if 0
TrapDescr *trap_descr_list_get_item(TrapList *list,
								const char *trap_name);

#endif

void trap_descr_load_switch(TrapList *list);

int trap_init_descr_hash_node ( TrapDescr *describe );

int trap_descr_hashtable_add_node(hashtable *ht, const void *key, unsigned int len, struct hlist_node *node);


#ifdef  __cplusplus
}
#endif

#endif /* TRAP_DESCR_H */

