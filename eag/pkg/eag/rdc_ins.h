/* rdc_ins.h */

#ifndef _RDC_INS_H
#define _RDC_INS_H

#include <stdint.h>
#include "rdc_def.h"

#define MAX_HANSI_ID 16
#define SLOT_IPV4_BASE  0XA9FE0200 /* 169.254.1.0 */
//#define SLOT_IPV4_BASE  0XC0A807ED /* 192.168.7.238 */

#define RDC_RADIUS_PORT_BASE 4150

typedef enum {
	RDC_SELF = 0,
	RDC_EAG,
} RDC_CLIENT_TYPE;

#define RDC_PACKET_MINSIZE 		RDC_PACKET_HEADSIZE+RADIUS_HDRSIZE

rdc_ins_t *
rdc_ins_new(uint8_t hansi_type, 
				uint8_t hansi_id);

int
rdc_ins_free(rdc_ins_t *rdcins);

int
rdc_ins_start(rdc_ins_t *rdcins);

int
rdc_ins_stop(rdc_ins_t *rdcins);

int
rdc_ins_set_timeout(rdc_ins_t *rdcins,
						uint32_t timeout);

int 
rdc_ins_dispatch(rdc_ins_t *rdcins);

int
rdc_ins_is_running(rdc_ins_t *rdcins);

rdc_server_t *
rdc_ins_get_server(rdc_ins_t *rdcins);

rdc_client_t *
rdc_ins_get_client(rdc_ins_t *rdcins);

rdc_coa_t *
rdc_ins_get_coa(rdc_ins_t *rdcins);


rdc_pktconn_db_t *
rdc_ins_get_pktconn_db(rdc_ins_t *rdcins);

rdc_coaconn_db_t *
rdc_ins_get_coaconn_db(rdc_ins_t *rdcins);
rdc_userconn_db_t *
rdc_ins_get_userconn_db( rdc_ins_t *rdcins );


uint8_t
rdc_ins_get_slot_id(rdc_ins_t *rdcins);

uint8_t
rdc_ins_get_hansi_type(rdc_ins_t *rdcins);

uint8_t
rdc_ins_get_hansi_id(rdc_ins_t *rdcins);

#endif        /* _RDC_INS_H */
