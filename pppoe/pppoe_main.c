
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dbus/dbus.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "pppoe_log.h"
#include "pppoe_util.h"

#include "pppoe_dbus.h"
#include "pppoe_modules.h"
#include "pppoe_instance.h"


static int keep_running = 1;

static void
pppoe_print_usage(const char *file) {
	fprintf(stderr, "usage: %s [<type> <id>]\n", file);
	fprintf(stderr, "type=0, remote hansi, id 1-16\n");
	fprintf(stderr, "type=1, local hansi, id 1-16\n");
}



static void 
pppoe_signal_handler(int signal) {
	switch (signal) {
		case SIGTERM:
		case SIGINT:
		case SIGUSR2:
			pppoe_log(LOG_INFO, "Received terminate signal %d\n", signal);
			keep_running = 0;
			break;
			
		default:
			break;
	}
}


static inline void
pppoe_log_pid(const char *pidfile) {
	FILE *fp = fopen(pidfile, "w+");
	if (!fp) {
		fprintf(stderr, "log pid file %s fail\n", pidfile);
		return;
	}
	
	fprintf(fp, "%d\n", getpid());
	fclose(fp);
}


int 
main(int argc, char **argv) {
	unsigned int slot_id, local_id, instance_id;
	instance_struct_t *instance;
	struct sigaction act;
	char daemon_name[32];
	char pid_file[64];

	if (argc == 3) {
		local_id = atoi(argv[1]);
		instance_id = atoi(argv[2]);
		switch (local_id) {
			case HANSI_REMOTE:
			case HANSI_LOCAL:
				if (!instance_id || instance_id > HANSI_MAX_ID) {
					pppoe_print_usage(argv[0]);
					goto arg_error;
				}
				break;
				
			default:
				pppoe_print_usage(argv[0]);
				goto arg_error;
		}
	} else goto arg_error;

	/* pppoe daemon*/
	if (daemon(0, 0)){
		pppoe_log(LOG_ERR, "pppoe daemon fail\n");
		goto error;
	}

	/* init pppoe log*/
	memset(daemon_name, 0, sizeof(daemon_name));
	snprintf(daemon_name, sizeof(daemon_name), 
			"pppoed_%d_%d", local_id, instance_id);
	pppoe_log_init(daemon_name);

	/* log pppoe pid */
	memset(pid_file, 0, sizeof(pid_file));
	snprintf(pid_file, sizeof(pid_file), "/var/run/%s.pid", daemon_name);
	pppoe_log_pid(pid_file);

	/* init pppoe signal*/
	memset(&act, 0, sizeof(act));
	act.sa_handler = pppoe_signal_handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGUSR2, &act, NULL);

	slot_id = get_product_info(LOCAL_SLOTID_FILE);
	if (!slot_id) {
		pppoe_log(LOG_ERR, "pppoe get local slot id failed\n");
		goto out;
	}
	
	/* init pppoe dbus */	
	if (pppoe_dbus_init(local_id, instance_id)) {
		pppoe_log(LOG_ERR, "pppoe dbus init failed\n");
		goto out1;
	}
	
	/* init pppoe modules */
	if (pppoe_modules_init(slot_id, local_id, instance_id)) {
		pppoe_log(LOG_ERR, "pppoe modules init failed\n");
		goto out;
	}	

	/* init pppoe instance */
	instance = pppoe_instance_init(slot_id, local_id, instance_id);
	if (unlikely(!instance)) {
		pppoe_log(LOG_ERR, "pppoe instance init failed\n");
		goto out2;
	}

	while (keep_running) {
		pppoe_dbus_dispatch(0);
		pppoe_instance_dispatch(instance, 100);
	}

	pppoe_instance_exit(&instance);
out2:
	pppoe_dbus_exit();
out1:
	pppoe_modules_exit();
out:
	pppoe_log_exit();	
	return PPPOEERR_SUCCESS;
	
arg_error:
	pppoe_print_usage(argv[0]);
error:	
	return PPPOEERR_EINVAL;
}
