/* rdc_coaconn.h */

#ifndef _RDC_COACONN_H
#define _RDC_COACONN_H

#include <stdint.h>
#include "rdc_def.h"

rdc_coaconn_db_t *
rdc_coaconn_db_create(uint32_t size);

int
rdc_coaconn_db_destroy(rdc_coaconn_db_t *db);

int
rdc_coaconn_db_add(rdc_coaconn_db_t *db,
						rdc_coaconn_t *coaconn);

int 
rdc_coaconn_db_del(rdc_coaconn_db_t *db,
						rdc_coaconn_t *coaconn);

rdc_coaconn_t *
rdc_coaconn_db_lookup(rdc_coaconn_db_t *db,
							uint32_t ip,
							uint16_t port,
							uint8_t id);

int
rdc_coaconn_db_clear(rdc_coaconn_db_t *db);

int 
rdc_coaconn_db_set_timeout(rdc_coaconn_db_t *db,
								uint32_t timeout);
int 
rdc_coaconn_db_set_thread_master(rdc_coaconn_db_t *db,
								eag_thread_master_t *master);

int
rdc_coaconn_db_log_all(rdc_coaconn_db_t *db);

rdc_coaconn_t *
rdc_coaconn_new(rdc_coaconn_db_t *db,
					uint32_t ip,
					uint16_t port,
					uint8_t id);

int
rdc_coaconn_free(rdc_coaconn_t *coaconn);

int
rdc_coaconn_wait(rdc_coaconn_t *coaconn);

uint32_t 
rdc_coaconn_get_ip(rdc_coaconn_t *coaconn);

uint16_t 
rdc_coaconn_get_port(rdc_coaconn_t *coaconn);

int
rdc_coaconn_set_coaip(rdc_coaconn_t *coaconn, uint32_t coaip);

uint32_t 
rdc_coaconn_get_coaip(rdc_coaconn_t *coaconn);

int
rdc_coaconn_set_coaport(rdc_coaconn_t *coaconn, uint16_t coaport);

uint16_t 
rdc_coaconn_get_coaport(rdc_coaconn_t *coaconn);


uint8_t 
rdc_coaconn_get_id(rdc_coaconn_t *coaconn);

int 
rdc_coaconn_get_timeout(rdc_coaconn_t *coaconn);



int
rdc_coaconn_set_server(rdc_coaconn_t *coaconn,
								rdc_server_t *server);

rdc_server_t *
rdc_coaconn_get_server(rdc_coaconn_t *coaconn);

int
rdc_coaconn_inc_req(rdc_coaconn_t *coaconn);

int
rdc_coaconn_inc_res(rdc_coaconn_t *coaconn);

#endif        /* _RDC_COACONN_H */
