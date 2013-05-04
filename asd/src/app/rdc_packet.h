/* rdc_packet.h */

#ifndef _RDC_PACKET_H
#define _RDC_PACKET_H

#include <stdint.h>

typedef enum {
	RDC_SELF = 0,
	RDC_EAG,
	RDC_ASD,
} RDC_CLIENT_TYPE;

#define RDC_PACKET_HEADSIZE		48
#define RDC_PACKET_SIZE			4096
//#define RDC_PACKET_MINSIZE 		RDC_PACKET_HEADSIZE+RADIUS_HDRSIZE

struct rdc_packet_t {
	uint32_t ip;
	uint16_t port;
	uint16_t length;
	uint16_t coa_port;
	uint8_t slotid;
	uint8_t hansi_type; //0 remote, 1 local or none
	uint8_t hansi_id;
	uint8_t client_type;  // 0 rdc, 1 eag, 2 ...
	uint8_t rsv[2];
	uint8_t desc[32];
	uint8_t data[RDC_PACKET_SIZE-RDC_PACKET_HEADSIZE];
} __attribute__((packed));

#endif        /* _RDC_PACKET_H */
