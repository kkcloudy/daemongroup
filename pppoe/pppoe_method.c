#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_method_def.h"

#include "pppoe_log.h"
#include "pppoe_method.h"


static methodFunc pppoeMethod[PPPOE_METHOD_NUMS];

int
pppoe_method_perform(uint32 method_id, void *data, void *para) {
	if (unlikely(method_id >= PPPOE_METHOD_NUMS))
		return PPPOEERR_EINVAL;

	if (unlikely(!pppoeMethod[method_id]))
		return PPPOEERR_ENOEXIST;

	return pppoeMethod[method_id](data, para);
}


int
pppoe_method_register(uint32 method_id, methodFunc method) {
	if (unlikely(method_id >= PPPOE_METHOD_NUMS || !method))
		return PPPOEERR_EINVAL;
	
	if (unlikely(pppoeMethod[method_id])) {
		pppoe_log(LOG_WARNING, "pppoe method register: method %u is already exist\n", method_id);
		return PPPOEERR_EEXIST;
	}
	
	pppoeMethod[method_id] = method;
	pppoe_log(LOG_DEBUG, "pppoe method %u register success\n", method_id);
	return PPPOEERR_SUCCESS;
}

int
pppoe_method_unregister(uint32 method_id) {
	if (unlikely(method_id >= PPPOE_METHOD_NUMS))
		return PPPOEERR_EINVAL;

	if (unlikely(!pppoeMethod[method_id])){
		pppoe_log(LOG_WARNING, "pppoe method unregister: method %u is not exist\n", method_id);
		return PPPOEERR_ENOEXIST;
	}

	pppoeMethod[method_id] = NULL;
	pppoe_log(LOG_DEBUG, "pppoe method %u unregister success\n", method_id);
	return PPPOEERR_SUCCESS;
}


void
pppoe_method_init(void) {
	memset(pppoeMethod, 0, sizeof(pppoeMethod));
}

void
pppoe_method_exit(void) {
	memset(pppoeMethod, 0, sizeof(pppoeMethod));
}
