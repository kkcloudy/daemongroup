/* rdc_client.h */

#ifndef _RDC_CLIENT_H
#define _RDC_CLIENT_H

void
rdc_set_log_packet_switch( int status );

void
rdc_client_init( uint16_t coa_port, uint8_t slotid, 
			uint8_t hansi_type, uint8_t hansi_id, 
			uint8_t client_type, char *module_desc );

void
rdc_client_uninit();

ssize_t
rdc_sendto(int s, const void *buf, 
			         size_t len, int flags, 
			         const struct sockaddr *to, socklen_t tolen);
			         
ssize_t
rdc_recvfrom(int s, void *buf, size_t len, int flags,
					    struct sockaddr *from, socklen_t *fromlen);

int 
rdc_set_server_hansi( int slotid, int insid );

#endif
