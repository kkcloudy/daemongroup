/* pdc_packet.h */

#ifndef _PDC_PACKET_H
#define _PDC_PACKET_H

#include <stdint.h>

#define PDC_PACKET_HEADSIZE		48
#define PDC_PACKET_SIZE			4096
//#define PDC_PACKET_MINSIZE 	PDC_PACKET_HEADSIZE+PORTAL_HEADRSIZE

struct pdc_packet_t {
	uint32_t ip;
	uint16_t port;
	uint16_t length;
	uint8_t slotid;
	uint8_t hansi_type; //0 remote, 1 local or none
	uint8_t hansi_id;
	uint8_t client_type; // 0 pdc, 1 eag
	uint8_t rsv[4];
	uint8_t desc[32];
	uint8_t data[PDC_PACKET_SIZE-PDC_PACKET_HEADSIZE]; 
};

struct pdc_usermap_pkt_t {
	uint8_t slot_id;
	uint8_t hansi_type;
	uint8_t hansi_id;
	uint8_t client_type;
	uint8_t desc[32];
	uint32_t user_ip;
	uint8_t user_mac[6];
};


#endif        /* _PDC_PACKET_H */

