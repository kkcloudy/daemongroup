/* trap-list.h */

#ifndef TRAP_LIST_H
#define TRAP_LIST_H

#ifdef  __cplusplus
extern "C" {
#endif

#if 0
typedef int (* TrapCompareFunc) (void *data1, const void *data2);
#endif

typedef void (* TrapFreeFunc) (void *data);

typedef struct TrapNode {
	void *data;
	struct TrapNode *next;
} TrapNode;

typedef struct TrapList {
	int num;
	TrapNode *first;
	TrapNode *last;
} TrapList;

#if 0
typedef struct TrapListIter {
	TrapList *list;
	TrapNode *node;
	int index;
} TrapListIter;
#endif

void trap_list_init(TrapList *list);

void trap_list_destroy(TrapList *list, TrapFreeFunc free_func);

void trap_list_append(TrapList *list, void *data);

#if 0
void *trap_list_find_custom(TrapList *list, const void *data, TrapCompareFunc func);

void trap_list_iter_first(TrapList *list, TrapListIter *iter);

void trap_list_iter_next(TrapListIter *iter);

int trap_list_iter_is_end(TrapListIter *iter);

void *trap_list_iter_get_value(TrapListIter *iter);
#endif

#ifdef  __cplusplus
}
#endif

#endif /* TRAP_LIST_H */

