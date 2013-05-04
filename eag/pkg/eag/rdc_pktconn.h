/* rdc_pktconn.h */

#ifndef _RDC_PKTCONN_H
#define _RDC_PKTCONN_H

#include <stdint.h>
#include "rdc_def.h"

rdc_pktconn_db_t *
rdc_pktconn_db_create(uint32_t size);

int
rdc_pktconn_db_destroy(rdc_pktconn_db_t *db);

int
rdc_pktconn_db_add(rdc_pktconn_db_t *db,
						rdc_pktconn_t *pktconn);

int 
rdc_pktconn_db_del(rdc_pktconn_db_t *db,
						rdc_pktconn_t *pktconn);

rdc_pktconn_t *
rdc_pktconn_db_lookup(rdc_pktconn_db_t *db,
							uint32_t ip,
							uint16_t port,
							uint8_t id);

int
rdc_pktconn_db_clear(rdc_pktconn_db_t *db);

int 
rdc_pktconn_db_set_timeout(rdc_pktconn_db_t *db,
								uint32_t timeout);

int 
rdc_pktconn_db_set_thread_master(rdc_pktconn_db_t *db,
								eag_thread_master_t *master);

int
rdc_pktconn_db_log_all(rdc_pktconn_db_t *db);

rdc_pktconn_t *
rdc_pktconn_new(rdc_pktconn_db_t *db,
					uint32_t ip,
					uint16_t port,
					uint8_t id);

int
rdc_pktconn_free(rdc_pktconn_t *pktconn);

int
rdc_pktconn_wait(rdc_pktconn_t *pktconn);

uint32_t 
rdc_pktconn_get_ip(rdc_pktconn_t *pktconn);

uint16_t 
rdc_pktconn_get_port(rdc_pktconn_t *pktconn);

uint8_t 
rdc_pktconn_get_id(rdc_pktconn_t *pktconn);

int 
rdc_pktconn_get_timeout(rdc_pktconn_t *pktconn);

int
rdc_pktconn_set_sockclient(rdc_pktconn_t *pktconn,
								rdc_sockclient_t *sockclient);

rdc_sockclient_t *
rdc_pktconn_get_sockclient(rdc_pktconn_t *pktconn);

int
rdc_pktconn_inc_req(rdc_pktconn_t *pktconn);

int
rdc_pktconn_inc_res(rdc_pktconn_t *pktconn);

#endif        /* _RDC_PKTCONN_H */
