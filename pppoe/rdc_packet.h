/* rdc_packet.h */

#ifndef _RDC_PACKET_H
#define _RDC_PACKET_H

struct rdc_packet_t {
	uint32	ip;
	uint16	port;
	uint16	length;
	uint16	coa_port;
	uint8	slotid;
	uint8	hansi_type; 	/* 0 remote, 1 local or none */
	uint8	hansi_id;
	uint8	client_type;  	/* 0 rdc, 1 eag, 2 ... */
	uint8	rsv[2];
	uint8	desc[32];
	uint8	data[0];
} __attribute__((packed));


#endif        /* _RDC_PACKET_H */

