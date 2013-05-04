/* rdc_client.h */

#ifndef _RDC_CLIENT_H
#define _RDC_CLIENT_H

#include <stdint.h>
#include "rdc_def.h"

typedef int (*rdc_sockcli_proc_res_func_t)(rdc_sockclient_t *,
											uint32_t,
											uint16_t,
											struct rdc_packet_t *);

int 
rdc_sockclient_id_wait(rdc_sockclient_t *sockclient,
						uint8_t id);

rdc_client_t *
rdc_client_new(void);

int
rdc_client_free(rdc_client_t *client);

int
rdc_client_start(rdc_client_t *client);

int
rdc_client_stop(rdc_client_t *client);

int
rdc_client_send_request(rdc_client_t *client,
						rdc_pktconn_t *pktconn,
						struct rdc_packet_t *rdcpkt);

int
rdc_client_set_nasip(rdc_client_t *client,
							uint32_t nasip);

uint32_t
rdc_client_get_nasip(rdc_client_t *client);

int 
rdc_client_set_thread_master(rdc_client_t *client,
							eag_thread_master_t *master);

int 
rdc_client_set_rdcins(rdc_client_t *client,
						rdc_ins_t *rdcins);

int 
rdc_client_log_all(rdc_client_t *client);

#endif        /* _RDC_CLIENT_H */
