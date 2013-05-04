/* eag_fastfwd.h */
#ifndef _EAG_FASTFWD_H
#define _EAG_FASTFWD_H

#include "eag_def.h"
#include "se_agent/se_agent_def.h"

eag_fastfwd_t *
eag_fastfwd_new(uint8_t hansi_type, 
		uint8_t hansi_id);

int
eag_fastfwd_free(eag_fastfwd_t *fastfwd);

int
eag_fastfwd_start(eag_fastfwd_t *fastfwd);

int
eag_fastfwd_stop(eag_fastfwd_t *fastfwd);

int
eag_fastfwd_send(eag_fastfwd_t *fastfwd,
		uint32_t user_ip,
		const char *hand_cmd);

int
eag_fastfwd_set_slot_id(eag_fastfwd_t *fastfwd,
		uint8_t slot_id);

void 
eag_fastfwd_set_thread_master(eag_fastfwd_t *fastfwd,
		eag_thread_master_t *master);

int
eag_fastfwd_set_appdb(eag_fastfwd_t *fastfwd,
		appconn_db_t *appdb);

int
eag_fastfwd_set_macauth(eag_fastfwd_t *fastfwd,
		eag_macauth_t *macauth);

#endif

