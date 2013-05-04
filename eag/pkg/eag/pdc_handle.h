/* pdc_handle.h */

#ifndef _PDC_HANDLE_H
#define _PDC_HANDLE_H

void
pdc_set_log_packet_switch( int status );

void
pdc_client_init( uint8_t slotid, 
			uint8_t hansi_type, uint8_t hansi_id, 
			uint8_t client_type, char *module_desc );

void
pdc_client_uninit();
			
ssize_t
pdc_sendto(int s, const void *buf, 
			         size_t len, int flags, 
			         const struct sockaddr *to, socklen_t tolen);
			         
ssize_t
pdc_recvfrom(int s, void *buf, size_t len, int flags,
					    struct sockaddr *from, socklen_t *fromlen);

int
pdc_get_server_hansi( int *slotid, int *insid );
int 
pdc_set_server_hansi( int slotid, int insid );

int
pdc_send_usermap( int fd, uint32_t userip, 
		uint8_t slot_id, uint8_t hansi_type, uint8_t hansi_id );

#endif
