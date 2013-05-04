/* pdc_client.c */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "nm_list.h"
#include "hashtable.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_blkmem.h"
#include "eag_thread.h"
#include "pdc_interface.h"
#include "eag_dbus.h"
#include "portal_packet.h"
#include "eag_util.h"

#include "pdc_ins.h"
#include "pdc_server.h"
#include "pdc_client.h"

#define PORTAL_PORT_DEFAULT		2000

static unsigned short pdc_req_id = 1;

typedef int (*pdc_client_proc_func_t)(pdc_client_t *,
		uint32_t,
		uint16_t,
		struct pdc_packet_t *);

struct pdc_client {
	int sockfd;
	uint32_t nasip;
	uint16_t port;
	eag_thread_t *t_read;
	eag_thread_master_t *master;
	pdc_ins_t *pdcins;
	pdc_client_proc_func_t proc_func;
};

typedef enum {
	PDC_CLIENT_READ,
} pdc_client_event_t;

static void
pdc_client_event(pdc_client_event_t event,
					pdc_client_t *client);

static int
client_process_packet(pdc_client_t *client,
					uint32_t ip,
					uint16_t port,
					struct pdc_packet_t *pdc_packet);

pdc_client_t *
pdc_client_new(void)
{
	pdc_client_t *client = NULL;

	client = eag_malloc(sizeof(*client));
	if (NULL == client) {
		eag_log_err("pdc_client_new eag_malloc failed");
		return NULL;
	}

	memset(client, 0, sizeof(*client));
	client->sockfd = -1;
	client->port = PORTAL_PORT_DEFAULT;
	client->proc_func = client_process_packet;

	eag_log_debug("client", "client new ok");
	return client;	
}

int
pdc_client_free(pdc_client_t *client)
{
	if (NULL == client) {
		eag_log_err("pdc_client_free input error");
		return -1;
	}

	if (client->sockfd >= 0) {
		close(client->sockfd);
		client->sockfd = -1;
	}
	eag_free(client);

	eag_log_debug("client", "client free ok");
	return 0;
}

int
pdc_client_start(pdc_client_t *client)
{
	int ret = 0;
	struct sockaddr_in addr = {0};
	char ipstr[32] = "";
	
	if (NULL == client) {
		eag_log_err("pdc_client_start input error");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if (client->sockfd >= 0) {
		eag_log_err("pdc_client_start already start fd(%d)", 
			client->sockfd);
		return EAG_RETURN_OK;
	}

	client->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (client->sockfd  < 0) {
		eag_log_err("Can't create client dgram socket: %s",
			safe_strerror(errno));
		client->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(client->port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(client->nasip);

#if 0
	ret = setsockopt(client->sockfd, SOL_SOCKET, SO_REUSEADDR,
					&opt, sizeof(opt));
	if (0 != ret) {
		eag_log_err("Can't set REUSEADDR to client socket(%d): %s",
			client->sockfd, safe_strerror(errno));
	}
#endif

	ret  = bind(client->sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		eag_log_err("Can't bind to client socket(%d): %s",
			client->sockfd, safe_strerror(errno));
		close(client->sockfd);
		client->sockfd = -1;
		return EAG_ERR_SOCKET_BIND_FAILED;
	}

	pdc_client_event(PDC_CLIENT_READ, client);
	
	eag_log_info("client(%s:%u) fd(%d) start ok", 
			ip2str(client->nasip, ipstr, sizeof(ipstr)),
			client->port,
			client->sockfd);

	return EAG_RETURN_OK;
}

int
pdc_client_stop(pdc_client_t *client)
{
	char ipstr[32] = "";

	if (NULL == client) {
		eag_log_err("pdc_client_stop input error");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if (NULL != client->t_read) {
		eag_thread_cancel(client->t_read);
		client->t_read = NULL;
	}
	if (client->sockfd >= 0)
	{
		close(client->sockfd);
		client->sockfd = -1;
	}
	eag_log_info("client(%s:%u) stop ok",
			ip2str(client->nasip, ipstr, sizeof(ipstr)),
			client->port);

	return EAG_RETURN_OK;
}

int
pdc_client_send_packet(pdc_client_t *client,
								struct pdc_packet_t *pdc_packet)
{
	uint32_t ip = 0;
	uint16_t port = 0;
	struct portal_packet_t *portal_packet = NULL;
	size_t length = 0;
	struct sockaddr_in addr = {0};
	ssize_t nbyte = 0;
	char ipstr[32] = "";

	if (NULL == client || NULL == pdc_packet) {
		eag_log_err("pdc_client_send_packet input error");
		return -1;
	}

	ip = ntohl(pdc_packet->ip);
	port = ntohs(pdc_packet->port);
	portal_packet = (struct portal_packet_t *)(&(pdc_packet->data));
	length = portal_packet_get_length(portal_packet);
	
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(ip);

	nbyte = sendto(client->sockfd, portal_packet, length,
				0, (struct sockaddr *)&addr, sizeof(addr));
	if (nbyte < 0) {
		eag_log_err("pdc_client_send_packet sendto failed: %s",
			safe_strerror(errno));
		return -1;
	}

	eag_log_debug("client", "client(%d) send packet to portal(%s:%d) ok",
		client->sockfd,
		ip2str(ip, ipstr, sizeof(ipstr)),
		port);
	return EAG_RETURN_OK;
}

static int
client_receive(eag_thread_t *thread)
{
	pdc_client_t *client = NULL;
	ssize_t nbyte = 0;
	struct pdc_packet_t pdc_packet = {0};
	struct portal_packet_t *portal_packet = NULL;
	struct sockaddr_in addr = {0};
	socklen_t len = 0;
	uint32_t ip = 0;
	uint16_t port = 0;
	char ipstr[32] = "";
	int packet_len = 0;
	int portal_minsize = 0;
	
	if (NULL == thread) {
		eag_log_err("client_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;	
	}

	client = eag_thread_get_arg(thread);
	if (NULL == client) {
		eag_log_err("client_receive client null");
		return EAG_ERR_NULL_POINTER;
	}

	len = sizeof(addr);
	nbyte = recvfrom(client->sockfd, &(pdc_packet.data),
			sizeof(pdc_packet.data), 0, (struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("client_receive fd:%d recvfrom failed: %s",
				client->sockfd, safe_strerror(errno));
		return EAG_ERR_SOCKET_RECV_FAILED;
	}

	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	eag_log_debug("client", "client fd(%d) receive %d bytes from %s:%u",
				client->sockfd,
				nbyte,
				ip2str(ip, ipstr, sizeof(ipstr)),
				port);

	portal_minsize = portal_packet_minsize();
	if (nbyte < portal_minsize) {
		eag_log_warning("client receive packet size %d < min %d",
			nbyte, portal_minsize);
		return -1;
	}

	if (nbyte > sizeof(pdc_packet.data)) {
		eag_log_warning("client receive packet size %d > max %d",
			nbyte, sizeof(pdc_packet.data));
		return -1;
	}
	
	portal_packet = (struct portal_packet_t *)(&(pdc_packet.data));
	packet_len = portal_packet_get_length(portal_packet);
	if (nbyte != packet_len) {
		eag_log_warning("client receive packet size %d != len %d",
			nbyte, packet_len);
		return -1;
	}
	if (REQ_CHALLENGE == portal_packet->type
		|| REQ_AUTH == portal_packet->type
		|| REQ_LOGOUT == portal_packet->type
		|| ACK_LOGOUT == portal_packet->type
		|| AFF_ACK_AUTH == portal_packet->type
		|| REQ_INFO == portal_packet->type 
		|| ACK_MACBINDING_INFO == portal_packet->type
		|| REQ_USER_OFFLINE == portal_packet->type) 
	{
		if (NULL != client->proc_func) {
			client->proc_func(client, ip, port, &pdc_packet);
		}
		else {
			eag_log_warning("client receive proc_func null");
		}
	}
	else {
		eag_log_warning("client receive unexpected packet type %d",
			portal_packet->type);
		return -1;
	}
	

	return EAG_RETURN_OK;
}

static int
client_process_packet(pdc_client_t *client,
							uint32_t ip,
							uint16_t port,
							struct pdc_packet_t *pdc_packet)
{
	struct portal_packet_t *portal_packet = NULL;
	pdc_ins_t *pdcins = NULL;
	pdc_server_t *server = NULL;
	char ipstr[32] = "";
	char useripstr[32] = "";
	int packet_len = 0;
			
	if (NULL == client || NULL == pdc_packet) {
		eag_log_err("client_process_packet input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	portal_packet = (struct portal_packet_t *)(&(pdc_packet->data));

	ip2str(ntohl(portal_packet->user_ip), useripstr, sizeof(useripstr));
	eag_log_debug("client", "client fd(%d) process packet "
			"packet type(%u), userip(%s), portal(%s:%u)",
			client->sockfd,
			portal_packet->type,
			useripstr,
			ip2str(ip, ipstr, sizeof(ipstr)),
			port);
	
	pdcins = client->pdcins;
	if (NULL == pdcins) {
		eag_log_err("client_process_packet pdcins null");
		return -1;
	}
	server = pdc_ins_get_server(pdcins);
	if (NULL == server) {
		eag_log_err("client_process_packet server null");
		return -1;
	}

	pdc_packet->ip = htonl(ip);
	pdc_packet->port = htons(port);
	packet_len = portal_packet_get_length(portal_packet);
	pdc_packet->length = htons(packet_len + PDC_PACKET_HEADSIZE);
	pdc_packet->slotid = pdc_ins_get_slot_id(pdcins);
	pdc_packet->hansi_type = pdc_ins_get_hansi_type(pdcins);
	pdc_packet->hansi_id = pdc_ins_get_hansi_id(pdcins);
	pdc_packet->client_type = PDC_SELF;
	
	snprintf((char *)(pdc_packet->desc), sizeof(pdc_packet->desc),
		"SLOT%u,%sHansi%uPDC",
		pdc_packet->slotid,
		HANSI_LOCAL==pdc_packet->hansi_type?"Local":"Remote",
		pdc_packet->hansi_id);
	
	pdc_server_send_packet(server, pdc_packet);

	eag_log_debug("client", "client process packet ok");
	return EAG_RETURN_OK;
}

int
pdc_client_not_find_map_proc(pdc_client_t *client, struct pdc_packet_t *pdc_req_packet)
{
	char ipstr[32] = "";
	struct pdc_packet_t pdc_rsp_packet = {0};
	struct portal_packet_t *req = NULL;
	struct portal_packet_t *rsp = NULL;

	pdc_rsp_packet.ip = pdc_req_packet->ip;
	pdc_rsp_packet.port = pdc_req_packet->port;
#if 0
	pdc_rsp_packet.slotid = pdc_req_packet->slotid;
	pdc_rsp_packet.hansi_id = pdc_req_packet->hansi_id;
	pdc_rsp_packet.hansi_type = pdc_req_packet->hansi_type;
	pdc_rsp_packet.client_type = pdc_req_packet->client_type;
#endif	
	req = (struct portal_packet_t *)(&(pdc_req_packet->data));
	rsp = (struct portal_packet_t *)(&(pdc_rsp_packet.data));
	rsp->version = req->version;
	rsp->auth_type = req->auth_type;
	rsp->serial_no = req->serial_no;
	rsp->user_ip = req->user_ip;
	rsp->user_port = req->user_port;
	rsp->attr_num = 0;

	switch(req->type){
	case REQ_CHALLENGE:
		rsp->type = ACK_CHALLENGE;
		rsp->err_code = CHALLENGE_REJECT;
		rsp->req_id = pdc_req_id;
		pdc_req_id++;
		break;
	case REQ_AUTH:
		rsp->type = ACK_AUTH;
		rsp->err_code = PORTAL_AUTH_REJECT;
		rsp->req_id = req->req_id;
		break;
	case REQ_LOGOUT:
		rsp->type = ACK_LOGOUT;
		rsp->err_code = EC_ACK_LOGOUT_REJECT;
		rsp->req_id = req->req_id;
		break;
	case REQ_INFO:
		rsp->type = ACK_INFO;
		rsp->err_code = EC_ACK_INFO_NOTSUPPORT;
		rsp->req_id = req->req_id;
		break;
	default:
		eag_log_warning("pdc_client_not_find_map_proc req type is wrong");
		break;
	}
	pdc_rsp_packet.length = portal_packet_get_length(rsp) + PDC_PACKET_HEADSIZE;
	
	eag_log_debug("pdc_client", "req_type:%x,portal_ip:%s,port:%u", rsp->type,
		ip2str(pdc_rsp_packet.ip, ipstr, sizeof(ipstr)),
		pdc_rsp_packet.port);
	
	pdc_client_send_packet(client, &pdc_rsp_packet);
	
	return EAG_RETURN_OK;
}

int
pdc_client_set_nasip(pdc_client_t *client,
							uint32_t nasip)
{
	pdc_ins_t *pdcins = NULL;
	char ipstr[32] = "";
	
	if (NULL == client) {
		eag_log_err("pdc_client_set_nasip input error");
		return EAG_ERR_PDC_CLIENT_NULL;
	}

	pdcins = client->pdcins;
	if (NULL == pdcins) {
		eag_log_err("pdc_client_set_nasip pdcins null");
		return EAG_ERR_PDC_PDCINS_NULL;
	}

	if (pdc_ins_is_running(pdcins)) {
		eag_log_warning("pdc_client_set_nasip pdc already started");
		return EAG_ERR_PDC_SERVICE_ALREADY_ENABLE;
	}
	
	client->nasip = nasip;

	eag_log_info("client set nasip %s",
			ip2str(nasip, ipstr, sizeof(ipstr)));
	return EAG_RETURN_OK;
}

uint32_t
pdc_client_get_nasip(pdc_client_t *client)
{
	if (NULL == client) {
		eag_log_err("pdc_client_get_nasip input error");
		return 0;
	}

	return client->nasip;
}

int
pdc_client_set_port(pdc_client_t *client,
							uint16_t port)
{
	pdc_ins_t *pdcins = NULL;
	
	if (NULL == client) {
		eag_log_err("pdc_client_set_port input error");
		return EAG_ERR_PDC_CLIENT_NULL;
	}

	pdcins = client->pdcins;
	if (NULL == pdcins) {
		eag_log_err("pdc_client_set_port pdcins null");
		return EAG_ERR_PDC_PDCINS_NULL;
	}

	if (pdc_ins_is_running(pdcins)) {
		eag_log_warning("pdc_client_set_port pdc already started");
		return EAG_ERR_PDC_SERVICE_ALREADY_ENABLE;
	}
	
	client->port = port;

	eag_log_info("client set port %u", port);
	return EAG_RETURN_OK;
}

uint32_t
pdc_client_get_port(pdc_client_t *client)
{
	if (NULL == client) {
		eag_log_err("pdc_client_get_port input error");
		return 0;
	}

	return client->port;
}

int 
pdc_client_set_thread_master(pdc_client_t *client,
								eag_thread_master_t *master)
{
	if (NULL == client) {
		eag_log_err("pdc_client_set_thread_master input error");
		return -1;
	}

	client->master = master;

	return 0;
}

int 
pdc_client_set_pdcins(pdc_client_t *client,
						pdc_ins_t *pdcins)
{
	if (NULL == client) {
		eag_log_err("pdc_client_set_pdcins input error");
		return -1;
	}

	client->pdcins = pdcins;

	return 0;
}

static void
pdc_client_event(pdc_client_event_t event,
					pdc_client_t *client)
{
	if (NULL == client) {
		eag_log_err("pdc_client_event input error");
		return;
	}
	
	switch (event) {
	case PDC_CLIENT_READ:
		client->t_read =
		    eag_thread_add_read(client->master, client_receive,
					client, client->sockfd);
		break;
	default:
		break;
	}
}

