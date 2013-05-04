#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "pppoe_log.h"
#include "pppoe_list.h"

#include "notifier_chain.h"



struct notifier_chain {
	struct list_head list;
};

int
notifier_chain_event(notifier_chain_t *chain, uint32 event) {
	struct notifier_struct *node;
	struct list_head *pos, *n;

	if (unlikely(!chain))
		return PPPOEERR_EINVAL;

	list_for_each_safe(pos, n, &chain->list) {
		node = list_entry(pos, struct notifier_struct, next);
		node->call(node, event, NULL);
	}

	return PPPOEERR_SUCCESS;
}

int
notifier_chain_register(notifier_chain_t *chain, struct notifier_struct *notifier) {
	struct notifier_struct *node;
	struct list_head *pos;

	if (unlikely(!chain || !notifier || !notifier->call)) {
		return PPPOEERR_EINVAL;
	}

	list_for_each(pos, &chain->list) {
		node = list_entry(pos, struct notifier_struct, next);
		if (node->priority > notifier->priority) {
			__list_add(&notifier->next, pos->prev, pos);
			return PPPOEERR_SUCCESS;
		}
	}

	list_add_tail(&notifier->next, &chain->list);
	return PPPOEERR_SUCCESS;
}

int
notifier_chain_unregister(notifier_chain_t *chain, struct notifier_struct *notifier) {
	struct list_head *pos;

	if (unlikely(!chain || !notifier)) {
		return PPPOEERR_EINVAL;
	}

	list_for_each(pos, &chain->list) {
		if (&notifier->next == pos) {
			list_del(&notifier->next);
			return PPPOEERR_SUCCESS;
		}
	}

	return PPPOEERR_ENOEXIST;
}

notifier_chain_t *
notifier_chain_create(void) {
	notifier_chain_t *chain;

	chain = (notifier_chain_t *)malloc(sizeof(notifier_chain_t));
	if (unlikely(!chain)) {
		pppoe_log(LOG_ERR, "chain malloc failed\n");
		return NULL;
	}

	memset(chain, 0, sizeof(*chain));
	INIT_LIST_HEAD(&chain->list);

	return chain;
}

int
notifier_chain_destroy(notifier_chain_t **chain) {
	if (unlikely(!chain || !(*chain)))
		return PPPOEERR_EINVAL;

	if (unlikely(!list_empty(&(*chain)->list))) {
		pppoe_log(LOG_ERR, "chain already have notifier which not unregister\n");
		return PPPOEERR_EEXIST;
	}

	PPPOE_FREE(*chain);
	return PPPOEERR_SUCCESS;
}
