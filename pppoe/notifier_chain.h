#ifndef _NOTIFIER_CHAIN_H
#define _NOTIFIER_CHAIN_H

typedef struct notifier_chain notifier_chain_t;

struct notifier_struct {
	int priority;
	int (*call) (struct notifier_struct *, uint32, void *);	/* event call func */
	void *arg;	/* event argument */
	struct list_head next;
};

/* this func is not thread safe */
int notifier_chain_event(notifier_chain_t *chain, uint32 event);

int notifier_chain_register(notifier_chain_t *chain, struct notifier_struct *notifier);
int notifier_chain_unregister(notifier_chain_t *chain, struct notifier_struct *notifier);

notifier_chain_t *notifier_chain_create(void);
int notifier_chain_destroy(notifier_chain_t **chain);

#endif
