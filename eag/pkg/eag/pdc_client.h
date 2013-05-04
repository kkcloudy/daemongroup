/* pdc_client.h */

#ifndef _PDC_CLIENT_H
#define _PDC_CLIENT_H

#include "stdint.h"
#include "pdc_def.h"
#include "pdc_packet.h"

pdc_client_t *
pdc_client_new(void);

int
pdc_client_free(pdc_client_t *client);

int
pdc_client_start(pdc_client_t *client);

int
pdc_client_stop(pdc_client_t *client);

int
pdc_client_set_nasip(pdc_client_t *client,
							uint32_t nasip);

uint32_t
pdc_client_get_nasip(pdc_client_t *client);

int
pdc_client_set_port(pdc_client_t *client,
							uint16_t port);

uint32_t
pdc_client_get_port(pdc_client_t *client);

int 
pdc_client_set_thread_master(pdc_client_t *client,
								eag_thread_master_t *master);

int 
pdc_client_set_pdcins(pdc_client_t *client,
						pdc_ins_t *pdcins);

int
pdc_client_send_packet(pdc_client_t *client,
								struct pdc_packet_t *pdc_packet);

int
pdc_client_not_find_map_proc(pdc_client_t *client, 
								struct pdc_packet_t *pdc_req_packet);

#endif        /* _PDC_CLIENT_H */
