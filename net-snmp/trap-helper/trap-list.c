/* trap-list.c */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "trap-util.h"
#include "trap-list.h"

static TrapNode *trap_node_new(void *data,
									TrapNode *next)
{
	TrapNode *node = NULL;
	
	node = malloc(sizeof(*node));
	if (NULL == node)
		return NULL;
	node->data = data;
	node->next = next;

	return node;
}

static void trap_node_free(TrapNode *node, TrapFreeFunc free_func)
{
	if (node != NULL){
		if (free_func != NULL && node->data != NULL)
			(* free_func)(node->data);
		free(node);
	}
}

void trap_list_init(TrapList *list)
{
	if (list != NULL){
		list->first = NULL;
		list->last = NULL;
		list->num = 0;
	}
}

void trap_list_destroy(TrapList *list, TrapFreeFunc free_func)
{
	TrapNode *p, *tmp;

	for (p = list->first; p; p = p->next, trap_node_free(tmp, free_func))
		tmp = p;
	
	list->first = NULL;
	list->last = NULL;
	list->num = 0;
}

void trap_list_append(TrapList *list, void *data)
{
	if (NULL == list->first)
		list->first = list->last = trap_node_new(data, NULL);
	else 
		list->last = list->last->next = trap_node_new(data, NULL);

	list->num++;
}

#if 0
void *trap_list_find_custom(TrapList *list, const void *data, TrapCompareFunc func)
{
	TrapNode *p;
	
	for (p = list->first; p; p = p->next)
		if (func != NULL && func(p->data, data) == 0)
			return p->data;

	return NULL;
}

void trap_list_iter_first(TrapList *list, TrapListIter *iter)
{
	memset(iter, 0, sizeof(*iter));
	if (NULL == list)
		return;
	iter->list = list;
	iter->node = list->first;
	iter->index = 0;
}

void trap_list_iter_next(TrapListIter *iter)
{
	if (NULL != iter->node)
		iter->node = iter->node->next;

	iter->index++;
}

int trap_list_iter_is_end(TrapListIter *iter)
{
	return NULL == iter->node;
}

void *trap_list_iter_get_value(TrapListIter *iter)
{
	if (NULL != iter->node)
		return iter->node->data;
	
	return NULL;
}
#endif

