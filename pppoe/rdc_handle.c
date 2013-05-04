
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"
#include "kernel/if_pppoe.h"

#include "pppoe_buf.h"
#include "pppoe_log.h"

#include "rdc_packet.h"
#include "rdc_handle.h"


struct rdc_client {
	uint8 slotid;
	uint8 local_id;
	uint8 instance_id;
	uint8 client_type;
	
	uint8 s_slotid;
	uint8 s_insid;

	uint16 coa_port;
	uint16 server_port;
	uint32 server_ip;
	char module_desc[32];
};

//static struct rdc_client_struct rdc_client;

int
rdc_sendto(rdc_client_t *client, int sk, struct pppoe_buf *pbuf, 
				struct sockaddr_in *addr, socklen_t len) {
	struct rdc_packet_t *rdc_packet;
	char errbuf[128];		
	ssize_t length;
	
	if (pbuf_headroom(pbuf) < sizeof(struct rdc_packet_t)) {
		pppoe_log(LOG_WARNING, "pbuf headroom lack space\n");
		return PPPOEERR_ENOMEM;
	}

	rdc_packet = (struct rdc_packet_t *)pbuf_push(pbuf, sizeof(struct rdc_packet_t));
	rdc_packet->ip = addr->sin_addr.s_addr;
	rdc_packet->port = addr->sin_port;
	rdc_packet->length = htons(pbuf->len);
	rdc_packet->coa_port = htons(client->coa_port);
	rdc_packet->slotid = client->slotid;
	rdc_packet->hansi_type = client->local_id;
	rdc_packet->hansi_id = client->instance_id;
	rdc_packet->client_type = client->client_type;
	memcpy(rdc_packet->desc, client->module_desc, sizeof(rdc_packet->desc));
	
	addr->sin_addr.s_addr = htonl(client->server_ip);
	addr->sin_port = htons(client->server_port);

	do {
		length = sendto(sk, rdc_packet, pbuf->len, 
						0, (struct sockaddr *)addr, len);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "socket %d sendto %u.%u.%u.%u:%u failed: %s\n", 
									sk, HIPQUAD(client->server_ip), client->server_port, 
									strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ESEND;			
		}
	} while (length < 0);

	if (length != pbuf->len){
		pppoe_log(LOG_WARNING, "rdc sendto %u.%u.%u.%u:%u, length %d != buf len %u\n", 
							HIPQUAD(client->server_ip), client->server_port, length, pbuf->len);
		return PPPOEERR_ELENGTH;
	}
	
	return PPPOEERR_SUCCESS;
}


int
rdc_recvfrom(rdc_client_t *client, int sk, struct pppoe_buf *pbuf, 
					struct sockaddr_in *addr, socklen_t *len) {
	struct rdc_packet_t *rdc_packet;
	char errbuf[128];
	ssize_t length;

	do {
		length = recvfrom(sk, pbuf->data, pbuf->end - pbuf->data, 
						0, addr, len);
		if (length < 0) {
			if (EINTR == errno) 
				continue;

			if (EAGAIN == errno || EWOULDBLOCK == errno) 
				return PPPOEERR_EAGAIN;

			pppoe_log(LOG_WARNING, "socket %d recvfrom failed: %s\n", 
							sk, strerror_r(errno, errbuf, sizeof(errbuf)));
			return PPPOEERR_ERECV;			
		}
	} while (length < 0);

	if (length <= sizeof(struct rdc_packet_t)) {
		pppoe_log(LOG_WARNING, "packet length %d "
								"<= sizoef(struct rdc_packet_t)\n", length);
		return PPPOEERR_ELENGTH;
	}

	pbuf->len = length;
	pbuf->tail = pbuf->data + pbuf->len;

	rdc_packet = (struct rdc_packet_t *)pbuf->data;
	if (pbuf_trim(pbuf, ntohs(rdc_packet->length))) {
		pppoe_log(LOG_WARNING, "packet length error\n");
		return PPPOEERR_ELENGTH;
	}

	pbuf_pull(pbuf, sizeof(struct rdc_packet_t));
	addr->sin_addr.s_addr = rdc_packet->ip;
	addr->sin_port = rdc_packet->port;
	return PPPOEERR_SUCCESS;
}

void
rdc_client_setup(rdc_client_t *client, uint16 coa_port) {
	client->coa_port = coa_port;
}

rdc_client_t *
rdc_client_init(uint8 slotid, uint8 local_id, 
				uint8 instance_id, uint8 client_type,
				uint8 s_slotid, uint8 s_insid, char *module_desc) {
	rdc_client_t *client
		= (rdc_client_t *)malloc(sizeof(rdc_client_t));
	if (unlikely(!client)) {
		pppoe_log(LOG_ERR, "malloc rdc client failed");
		return NULL;
	}
		
	memset(client, 0, sizeof(*client));
	client->slotid = slotid;
	client->local_id = local_id;
	client->instance_id = instance_id;
	client->client_type = client_type;
	client->s_slotid = s_slotid;
	client->s_insid = s_insid;

	pppoe_log(LOG_INFO, "rdc server slot %u, instance %u\n", s_slotid, s_insid);
	
	if (module_desc && strlen(module_desc)){
		strncpy(client->module_desc, module_desc, sizeof(client->module_desc) - 1);
	} else {
		snprintf(client->module_desc, sizeof(client->module_desc),
					"%u_%u_%u %u", client_type, local_id, instance_id, slotid);
	}

	if (HANSI_LOCAL == local_id) {
		client->server_ip = SLOT_IPV4_BASE + 100 + s_insid;
		client->server_port = RDC_RADIUS_PORT_BASE + s_insid;
	} else {
		client->server_ip = SLOT_IPV4_BASE + s_slotid;
		client->server_port = RDC_RADIUS_PORT_BASE + HANSI_MAX_ID + s_insid;
	}
	
	pppoe_log(LOG_INFO, "rdc server ipaddr %u.%u.%u.%u:%u",
						HIPQUAD(client->server_ip), client->server_port);	
	return client;
}

void
rdc_client_exit(rdc_client_t **client) {
	if (unlikely(client || *client))
		return;

	PPPOE_FREE(*client);
}

