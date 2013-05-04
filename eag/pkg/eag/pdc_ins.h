/* pdc_ins.h */

#ifndef _PDC_INS_H
#define _PDC_INS_H

#include "stdint.h"
#include "pdc_def.h"

#define MAX_HANSI_ID 16
#define SLOT_IPV4_BASE  0XA9FE0200 /* 169.254.2.0 */
//#define SLOT_IPV4_BASE  0XC0A807ED /* 192.168.7.238 */

#define PDC_USERMAP_PORT_BASE		4200
#define PDC_PORTAL_PORT_BASE 		4100
#define EAG_PORTAL_PORT_BASE 		4050

typedef enum {
	PDC_SELF = 0,
	PDC_EAG,
} PDC_CLIENT_TYPE;

//#define PDC_PACKET_MINSIZE		PDC_PACKET_HEADSIZE+PORTAL_PACKET_HEADSIZE

pdc_ins_t *
pdc_ins_new(uint8_t hansitype, uint8_t insid);

int
pdc_ins_free(pdc_ins_t *pdcins);

int
pdc_ins_start(pdc_ins_t *pdcins);

int
pdc_ins_stop(pdc_ins_t *pdcins);

int 
pdc_ins_dispatch(pdc_ins_t *pdcins);

int
pdc_ins_is_running(pdc_ins_t *pdcins);

pdc_server_t *
pdc_ins_get_server(pdc_ins_t *rdcins);

pdc_client_t *
pdc_ins_get_client(pdc_ins_t *rdcins);
pdc_usermap_t *
pdc_ins_get_usermap( pdc_ins_t *pdcins );

pdc_userconn_db_t *
pdc_ins_get_userconn_db( pdc_ins_t *pdcins );

uint8_t
pdc_ins_get_slot_id(pdc_ins_t *pdcins);

uint8_t
pdc_ins_get_hansi_type(pdc_ins_t *pdcins);

uint8_t
pdc_ins_get_hansi_id(pdc_ins_t *pdcins);

int
pdc_packet_minsize(void);

#endif        /* _PDC_INS_H */
