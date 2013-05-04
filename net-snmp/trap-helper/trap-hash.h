#ifndef _TRAP_HASH_H_
#define _TRAP_HASH_H_


void trap_init_hashtable();


#define TRAP_SHOW_HASHTABLE(ht, type, member, content) 	do{	\
	int i=0;		\
	struct hlist_node *hlist_node=NULL;	\
	for (i=0; i < ht->hash_num; i++){	\
		hlist_for_each_entry(type , hlist_node, &(ht->head_nodes[i]), member ){		\
		trap_syslog(LOG_DEBUG,"hash_num is %d content is %s\n",i,type->content);	\
		}	\
	}	\
}while (0)

#endif
