/* pdc_client.h */

#ifndef _PDC_USERMAP_H
#define _PDC_USERMAP_H

#include "stdint.h"
#include "pdc_def.h"

pdc_usermap_t *
pdc_usermap_new(void);


int
pdc_usermap_free(pdc_usermap_t *usermap);

int
pdc_usermap_start(pdc_usermap_t *usermap);

int
pdc_usermap_stop(pdc_usermap_t *usermap);


int 
pdc_usermap_set_thread_master(pdc_usermap_t *usermap,
								eag_thread_master_t *master);

int 
pdc_usermap_set_pdcins(pdc_usermap_t *usermap,
						pdc_ins_t *pdcins);

int
pdc_usermap_set_userconn_db( pdc_usermap_t *map, 
						pdc_userconn_db_t *db );


#endif        /* _PDC_CLIENT_H */
