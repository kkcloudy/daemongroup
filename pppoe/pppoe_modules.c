#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "pppoe_dbus_def.h"
#include "pppoe_interface_def.h"
#include "kernel/if_pppoe.h"

#include "pppoe_log.h"
#include "pppoe_thread.h"
#include "pppoe_buf.h"
#include "pppoe_ppp.h"

#include "pppoe_lcp.h"
#include "pppoe_chap.h"
#include "pppoe_ipcp.h"
#include "pppoe_ccp.h"

#include "pppoe_method.h"
#include "pppoe_method_handler.h"

#include "pppoe_modules.h"


static int
module_ppp_proto_init(void) {
	int ret;

	ppp_proto_init();
	ret = lcp_proto_init();
	if (ret) {
		pppoe_log(LOG_ERR, "LCP protocol init fail\n");
		goto error;
	}

	ret = chap_proto_init();
	if (ret) {
		pppoe_log(LOG_ERR, "CHAP protocol init fail\n");
		goto error;
	}
	
	ret = ipcp_proto_init();
	if (ret) {
		pppoe_log(LOG_ERR, "IPCP protocol init fail\n");
		goto error;
	}

	ret = ccp_proto_init();
	if (ret) {
		pppoe_log(LOG_ERR, "CCP protocol init fail\n");
		goto error;
	}

	pppoe_log(LOG_INFO, "PPP protocols init success\n");
	return PPPOEERR_SUCCESS;

error:
	pppoe_log(LOG_ERR, "PPP protocols init fail\n");
	ppp_proto_exit();
	return ret;
}

static void
module_ppp_proto_exit(void) {
	lcp_proto_exit();
	chap_proto_exit();
	ipcp_proto_exit();
	ccp_proto_exit();
	ppp_proto_exit();
}

static void
module_method_init(void) {
	pppoe_method_init();
	pppoe_method_handler_init();
}

static void
module_method_exit(void) {
	pppoe_method_handler_exit();
	pppoe_method_exit();
}


int
pppoe_modules_init(uint32 slot_id, uint32 local_id, uint32 instance_id) {
	int ret;

	module_method_init();
	ret = module_ppp_proto_init();
	if (unlikely(ret)) {
		pppoe_log(LOG_ERR, "module ppp proto init failed, ret %d\n", ret);
		goto error;
	}
	
	pppoe_log(LOG_INFO, "PPPOE modules init success\n");
	return PPPOEERR_SUCCESS;

error:
	module_method_exit();
	return ret;
}

void
pppoe_modules_exit(void) {
	module_method_exit();
	module_ppp_proto_exit();
}
