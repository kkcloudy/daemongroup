/* rdc_client.h */

#ifndef _RDC_CLIENT_H
#define _RDC_CLIENT_H

#define RDC_TYPE_PPPOE				3
#define SLOT_IPV4_BASE  			0XA9FE0200 /* 169.254.2.0 */
#define RDC_RADIUS_PORT_BASE 		4150

typedef struct rdc_client rdc_client_t;

int rdc_sendto(rdc_client_t *client, int sk, struct pppoe_buf *pbuf, 
					struct sockaddr_in *addr, socklen_t len);		         
int rdc_recvfrom(rdc_client_t *client, int sk, struct pppoe_buf *pbuf, 
					struct sockaddr_in *addr, socklen_t *len);


void rdc_client_setup(rdc_client_t *client, uint16 coa_port);
rdc_client_t *rdc_client_init(uint8 slotid, uint8 local_id, 
							uint8 instance_id, uint8 client_type,
							uint8 s_slotid, uint8 s_insid, char *module_desc);
void rdc_client_exit(rdc_client_t **client);

#endif
