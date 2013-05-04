/* rdc_server.h */

#ifndef _RDC_SERVER_H
#define _RDC_SERVER_H

#include <stdint.h>
#include "rdc_def.h"
#include "rdc_packet.h"

rdc_server_t *
rdc_server_new(void);

int
rdc_server_free(rdc_server_t *server);

int
rdc_server_start(rdc_server_t *server);

int
rdc_server_stop(rdc_server_t *server);

int
rdc_server_send_response(rdc_server_t *server,
						rdc_pktconn_t *pktconn,
						struct rdc_packet_t *rdcpkt);

int
rdc_server_send_coa_request( rdc_server_t *server,
						rdc_coaconn_t *coaconn,
						struct rdc_packet_t *rdcpkt );

int 
rdc_server_set_thread_master(rdc_server_t *server,
								eag_thread_master_t *master);

int 
rdc_server_set_rdcins(rdc_server_t *server,
						rdc_ins_t *rdcins);

int 
rdc_server_set_ip(rdc_server_t *server,
						uint32_t ip);

int 
rdc_server_set_port(rdc_server_t *server,
						uint16_t port);

int
rdc_server_set_userdb( rdc_server_t *server, rdc_userconn_db_t *userdb );

#endif        /* _RDC_SERVER_H */
