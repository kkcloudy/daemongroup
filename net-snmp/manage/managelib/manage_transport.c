
/** include glibc **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "manage_log.h"
#include "manage_type.h"
#include "manage_tipc.h"

#include "manage_transport.h"

static manage_tdomain *domain_list = NULL;


static manage_tdomain *
_find_tdomain(const char *name) {
	manage_tdomain *d = domain_list;
	for(; d; d = d->next) {
		if(0 == strcmp(d->name, name)) {
			manage_token_log(LOG_TOKEN_TRANSPORT, "_find_tdomain: Found domain \"%s\" from name \"%s\"\n", 
													d->name, name);
			return d;
		}		
	}
	manage_log(LOG_WARNING, "_find_tdomain: Found no domain from name \"%s\"\n", name);
	return NULL;
}


void
manage_transport_free(manage_transport *t) {
	if (NULL == t) {
		return ;
	}
	
	MANAGE_FREE(t->addr);
	MANAGE_FREE(t);

	return ;
}

int
manage_tdomain_register(manage_tdomain *n) {
	manage_tdomain **prevNext = &domain_list, *d = domain_list;

	if(NULL != n) {
		for(; d; d = d->next) {
			if(0 == strcmp(n->name, d->name)) {
				/*
				* Already registered.  
				*/
				manage_log(LOG_WARNING, "manage_tdomain_register: %s is already registered\n", n->name);
				return -1;
			}
			prevNext = &(d->next);
		}
		n->next = NULL;
		*prevNext = n;
		return 0;
	} 

	manage_log(LOG_WARNING, "manage_tdomain_register: input para n is NULL\n");
	return -1;
}

static void
manage_tdomain_dump(void) {
	manage_tdomain *d = domain_list;
	manage_token_log(LOG_TOKEN_TRANSPORT, "domain_list ->");
	for(; d; d = d->next) {
		manage_token_log(LOG_TOKEN_TRANSPORT, "{ %s } -> ", d->name);
	}
	manage_token_log(LOG_TOKEN_TRANSPORT, "[NIL]\n");
	
	return ;
}

void
manage_tdomain_init(void) {
	manage_token_log(LOG_TOKEN_TRANSPORT, "manage tdomain init\n");
	
	manage_tipc_ctor();
	//manage_netlink_ctor();
	manage_tdomain_dump();
	
	return ;
}

manage_transport *
manage_transport_open(const char *name, u_long flags,
								const u_char *chunk, const size_t chunk_size) {
	manage_token_log(LOG_TOKEN_TRANSPORT, "enter manage_transport_open\n");
	
	if(NULL == name || NULL == chunk) {
		return NULL;
	}
	
	manage_tdomain *match = _find_tdomain(name);
	if(NULL != match) {
		manage_transport *t = match->f_create_from_chunk(chunk, chunk_size, flags);
		manage_token_log(LOG_TOKEN_TRANSPORT, "create manage_transport %p\n", t);
		return t;	
	}
	return NULL;
}

