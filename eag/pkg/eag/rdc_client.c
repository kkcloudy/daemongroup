/* rdc_client.c */

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
#include "rdc_interface.h"
#include "eag_dbus.h"
#include "eag_util.h"

#include "radius_packet.h"
#include "rdc_packet.h"
#include "rdc_ins.h"
#include "rdc_server.h"
#include "rdc_client.h"
#include "rdc_pktconn.h"

#define RDC_SOCKCLI_BLKMEM_NAME 		"rdc_sockcli_blkmem"
#define RDC_SOCKCLI_BLKMEM_ITEMNUM		32

#define RDC_ID_NUM	256 

struct rdc_client {
	//int status;
	uint32_t nasip;
	struct list_head sockcli_list;
	eag_blk_mem_t *sockcli_blkmem;
	eag_thread_master_t *master;
	rdc_ins_t *rdcins;
};

typedef enum {
	RDC_ID_FREE = 0,
	RDC_ID_USED,
	RDC_ID_WAIT,
} rdc_id_status_t;

struct rdc_id {
	uint8_t index;
	rdc_id_status_t status;
	rdc_pktconn_t *pktconn;
	rdc_sockclient_t *sockclient;
	eag_thread_t *t_timeout;
	eag_thread_master_t *master;
	int timeout;
};

struct rdc_sockclient {
	struct list_head node;
	int sockfd;
	uint32_t nasip;
	struct rdc_id id[RDC_ID_NUM];
	rdc_client_t *client;
	eag_thread_t *t_read;
	eag_thread_master_t *master;
	rdc_sockcli_proc_res_func_t proc_res;
};

typedef enum {
	RDC_SOCKCLI_READ,
} rdc_sockcli_event_t;

typedef enum {
	RDC_ID_STATUS_TIMEOUT,
} rdc_id_status_event_t;

static void
rdc_sockclient_event(rdc_sockcli_event_t event,
						rdc_sockclient_t *sockclient);

static void
rdc_id_status_event(rdc_id_status_event_t event,
						struct rdc_id *id);

static int
sockclient_process_response(rdc_sockclient_t *sockclient,
							uint32_t ip,
							uint16_t port,
							struct rdc_packet_t *rdcpkt);

rdc_sockclient_t *
rdc_sockclient_new(rdc_client_t *client)
{
	rdc_sockclient_t *sockclient = NULL;
	int i = 0;

	if (NULL == client) {
		eag_log_err("rdc_sockclient_new input error");
		return NULL;
	}

	sockclient = eag_blkmem_malloc_item(client->sockcli_blkmem);
	if (NULL == sockclient) {
		eag_log_err("rdc_sockclient_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(sockclient, 0, sizeof(*sockclient));
	list_add_tail(&(sockclient->node), &(client->sockcli_list));
	sockclient->sockfd = -1;
	sockclient->nasip = client->nasip;
	sockclient->client = client;
	sockclient->master = client->master;
	sockclient->proc_res = sockclient_process_response;

	for (i = 0; i < RDC_ID_NUM; i++) {
		sockclient->id[i].index = i;
		sockclient->id[i].sockclient = sockclient;
		sockclient->id[i].master = sockclient->master;
	}

	eag_log_debug("client", "sockclient new ok");
	return sockclient;
}

int
rdc_sockclient_free(rdc_sockclient_t *sockclient)
{
	rdc_client_t *client = NULL;

	if (NULL == sockclient) {
		eag_log_err("rdc_sockclient_free input error");
		return -1;
	}
	client = sockclient->client;
	if (NULL == client) {
		eag_log_err("rdc_sockclient_free client null");
		return -1;
	}
		
	list_del(&(sockclient->node));
	if (sockclient->sockfd >= 0) {
		close(sockclient->sockfd);
		sockclient->sockfd = -1;
	}
	eag_blkmem_free_item(client->sockcli_blkmem, sockclient);

	eag_log_debug("client", "sockclient free ok");
	
	return 0;
}

int
rdc_sockclient_start(rdc_sockclient_t *sockclient)
{
	int ret = 0;
	struct sockaddr_in addr = {0};

	if (NULL == sockclient) {
		eag_log_err("rdc_sockclient_start input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (sockclient->sockfd >= 0) {
		eag_log_err("rdc_sockclient_start already start fd(%d)", 
			sockclient->sockfd);
		return EAG_RETURN_OK;
	}

	sockclient->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockclient->sockfd  < 0) {
		eag_log_err("Can't create sockclient dgram socket: %s",
			safe_strerror(errno));
		sockclient->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(sockclient->nasip);

	#if 0
	ret = setsockopt(sockclient->sockfd, SOL_SOCKET, SO_REUSEADDR, 
					&opt, sizeof(opt));
	if (0 != ret) {
		eag_log_err("Can't set REUSEADDR to sockclient socket(%d): %s",
			sockclient->sockfd, safe_strerror(errno));
	}
	#endif
	ret  = bind(sockclient->sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		eag_log_err("Can't bind to sockclient socket(%d): %s",
			sockclient->sockfd, safe_strerror(errno));
		close(sockclient->sockfd);
		sockclient->sockfd = -1;
		return EAG_ERR_SOCKET_BIND_FAILED;
	}

	rdc_sockclient_event(RDC_SOCKCLI_READ, sockclient);

	eag_log_info("sockclient fd(%d) start ok", 
				sockclient->sockfd);
	
	return EAG_RETURN_OK;
}

int
rdc_sockclient_stop(rdc_sockclient_t *sockclient)
{
	int i = 0;

	if (NULL == sockclient) {
		eag_log_err("rdc_sockclient_stop input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if (NULL != sockclient->t_read) {
		eag_thread_cancel(sockclient->t_read);
		sockclient->t_read = NULL;
	}
	
	eag_log_info("sockclient fd(%d) stop ok", 
				sockclient->sockfd);
	if (sockclient->sockfd >= 0) {
		close(sockclient->sockfd);
		sockclient->sockfd = -1;
	}
	for (i = 0; i < RDC_ID_NUM; i++) {
		if (NULL != sockclient->id[i].t_timeout) {
			eag_thread_cancel(sockclient->id[i].t_timeout);
			sockclient->id[i].t_timeout = NULL;
		}
		sockclient->id[i].status = RDC_ID_FREE;
		sockclient->id[i].pktconn = NULL;
		sockclient->id[i].timeout = 0;
	}

	//eag_log_debug("client", "sockclient stop ok");
	
	return 0;
}

int
rdc_sockclient_send_request(rdc_sockclient_t *sockclient,
								rdc_pktconn_t *pktconn,
								struct rdc_packet_t *rdcpkt)
{
	uint32_t ip = 0;
	uint16_t port = 0;
	struct radius_packet_t *radpkt = NULL;
	size_t length = 0;
	struct sockaddr_in addr = {0};
	ssize_t nbyte = 0;
	char ipstr[32] = "";

	if (NULL == sockclient || NULL == pktconn || NULL == rdcpkt) {
		eag_log_err("rdc_sockclient_send_request input error");
		return -1;
	}

	ip = ntohl(rdcpkt->ip);
	port = ntohs(rdcpkt->port);
	radpkt = (struct radius_packet_t *)(&(rdcpkt->data));
	length = ntohs(radpkt->length);
	
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(ip);

	nbyte = sendto(sockclient->sockfd, radpkt, length,
				0, (struct sockaddr *)&addr, sizeof(addr));
	if (nbyte < 0) {
		eag_log_err("rdc_sockclient_send_request sendto failed: %s",
			safe_strerror(errno));
		return -1;
	}

	eag_log_debug("client", "sockclient(%d) send request to radius(%s:%d) ok",
		sockclient->sockfd,
		ip2str(ip, ipstr, sizeof(ipstr)),
		port);
	return EAG_RETURN_OK;
}

static int
sockclient_receive(eag_thread_t *thread)
{
	rdc_sockclient_t *sockclient = NULL;
	ssize_t nbyte = 0;
	struct rdc_packet_t rdcpkt = {0};
	struct radius_packet_t *radpkt = NULL;
	struct sockaddr_in addr = {0};
	socklen_t len = 0;
	uint32_t ip = 0;
	uint16_t port = 0;
	char ipstr[32] = "";

	if (NULL == thread) {
		eag_log_err("sockclient_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;	
	}

	sockclient = eag_thread_get_arg(thread);
	if (NULL == sockclient) {
		eag_log_err("sockclient_receive sockclient null");
		return EAG_ERR_NULL_POINTER;
	}

	len = sizeof(addr);
	nbyte = recvfrom(sockclient->sockfd, &(rdcpkt.data),
			sizeof(rdcpkt.data), 0, (struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("sockclient_receive recvfrom failed: %s",
			safe_strerror(errno));
		return EAG_ERR_SOCKET_RECV_FAILED;
	}

	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	eag_log_debug("client", "sockclient fd(%d) receive %d bytes from %s:%u",
				sockclient->sockfd,
				nbyte,
				ip2str(ip, ipstr, sizeof(ipstr)),
				port);
		
	if (nbyte < RADIUS_PACKET_HEADSIZE) {
		eag_log_warning("sockclient_receive packet size %d < min %d",
			nbyte, RADIUS_PACKET_HEADSIZE);
		return -1;
	}

	if (nbyte > sizeof(rdcpkt.data)) {
		eag_log_warning("sockclient_receive packet size %d > max %d",
			nbyte, sizeof(rdcpkt.data));
		return -1;
	}
	
	radpkt = (struct radius_packet_t *)(&(rdcpkt.data));
	if (nbyte != ntohs(radpkt->length)) {
		eag_log_warning("sockclient_receive packet size %d != len %d",
			nbyte, ntohs(radpkt->length));
		return -1;
	}

	if (NULL != sockclient->proc_res) {
		sockclient->proc_res(sockclient, ip, port, &rdcpkt);
	}
	else {
		eag_log_warning("sockclient_receive proc_res null");
	}

	return EAG_RETURN_OK;
}

static int
sockclient_process_response(rdc_sockclient_t *sockclient,
							uint32_t ip,
							uint16_t port,
							struct rdc_packet_t *rdcpkt)
{
	struct radius_packet_t *radpkt = NULL;
	rdc_pktconn_t *pktconn = NULL;
	rdc_client_t *client = NULL;
	rdc_ins_t *rdcins = NULL;
	rdc_server_t *server = NULL;
	char ipstr[32] = "";
			
	if (NULL == sockclient || NULL == rdcpkt) {
		eag_log_err("sockclient_process_response input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	radpkt = (struct radius_packet_t *)(&(rdcpkt->data));
/*
	if (radpkt->code != RADIUS_CODE_ACCESS_ACCEPT
		&& radpkt->code != RADIUS_CODE_ACCESS_REJECT
		&& radpkt->code != RADIUS_CODE_ACCOUNTING_RESPONSE)
	{
		eag_log_warning("sockclient process response, "
			"unexpected packet code %d",
			radpkt->code);
		return -1;
	}
*/
	eag_log_debug("client", "sockclient fd(%d) process response "
			"packet id(%u), radius(%s:%u)",
			sockclient->sockfd,
			radpkt->id,
			ip2str(ip, ipstr, sizeof(ipstr)),
			port);

	pktconn = sockclient->id[radpkt->id].pktconn;
	if (NULL == pktconn) {
		eag_log_warning("sockclient process response, pktconn null");
		return -1;
	}
	
	client = sockclient->client;
	if (NULL == client) {
		eag_log_err("sockclient_process_response client null");
		return -1;
	}
	rdcins = client->rdcins;
	if (NULL == rdcins) {
		eag_log_err("sockclient_process_response rdcins null");
		return -1;
	}
	server = rdc_ins_get_server(rdcins);
	if (NULL == server) {
		eag_log_err("sockclient_process_response server null");
		return -1;
	}

	rdcpkt->ip = htonl(ip);
	rdcpkt->port = htons(port);
	rdcpkt->length = htons(ntohs(radpkt->length) + RDC_PACKET_HEADSIZE);
	rdcpkt->slotid = rdc_ins_get_slot_id(rdcins);
	rdcpkt->hansi_type = rdc_ins_get_hansi_type(rdcins);
	rdcpkt->hansi_id = rdc_ins_get_hansi_id(rdcins);
	rdcpkt->client_type = RDC_SELF;
	
	snprintf((char *)(rdcpkt->desc), sizeof(rdcpkt->desc),
		"SLOT%u,%sHansi%uRDC",
		rdcpkt->slotid,
		HANSI_LOCAL==rdcpkt->hansi_type?"Local":"Remote",
		rdcpkt->hansi_id);
	
	rdc_pktconn_inc_res(pktconn);
	rdc_server_send_response(server, pktconn, rdcpkt);

	eag_log_debug("client", "sockclient process response ok");
	return EAG_RETURN_OK;
}

int
rdc_sockclient_set_nasip(rdc_sockclient_t *sockclient,
							uint32_t nasip)
{
	if (NULL == sockclient) {
		eag_log_err("rdc_sockclient_set_nasip input error");
		return -1;
	}

	sockclient->nasip = nasip;

	return 0;
}

static int
id_status_timeout(eag_thread_t *thread)
{
	struct rdc_id *id = NULL;
	rdc_sockclient_t *sockclient = NULL;

	if (NULL == thread) {
		eag_log_err("id_status_timeout input error");
		return -1;
	}

	id = eag_thread_get_arg(thread);
	if (NULL == id) {
		eag_log_err("id_status_timeout id null");
		return -1;
	}

	sockclient = id->sockclient;
	if (NULL == sockclient) {
		eag_log_err("id_status_timeout sockclient null");
		return -1;
	}
	eag_log_debug("client", "sockclient fd(%d) id(%u) timeout",
				sockclient->sockfd, id->index);
	
	if (NULL != id->t_timeout) {
		eag_thread_cancel(id->t_timeout);
		id->t_timeout = NULL;
	}
	id->status = RDC_ID_FREE;
	id->pktconn = NULL;
	id->timeout = 0;
	
	return 0;
}

int 
rdc_sockclient_id_wait(rdc_sockclient_t *sockclient,
							uint8_t id)
{
	if (NULL == sockclient) {
		eag_log_err("rdc_sockclient_id_wait input error");
		return -1;
	}

	sockclient->id[id].status = RDC_ID_WAIT;
	sockclient->id[id].pktconn = NULL;
	rdc_id_status_event(RDC_ID_STATUS_TIMEOUT,
					&(sockclient->id[id]));

	return 0;
}

#if 0
/* client */
#endif

rdc_client_t *
rdc_client_new(void)
{
	rdc_client_t *client = NULL;

	client = eag_malloc(sizeof(*client));
	if (NULL == client) {
		eag_log_err("rdc_client_new eag_malloc failed");
		return NULL;
	}

	memset(client, 0, sizeof(*client));
	if (EAG_RETURN_OK != eag_blkmem_create(&(client->sockcli_blkmem),
					       RDC_SOCKCLI_BLKMEM_NAME,
					       sizeof(rdc_sockclient_t),
					       RDC_SOCKCLI_BLKMEM_ITEMNUM, MAX_BLK_NUM)) {
		eag_log_err("rdc_client_new blkmem_create failed");
		eag_free(client);
		client = NULL;
		return NULL;
	}
	INIT_LIST_HEAD(&(client->sockcli_list));

	eag_log_debug("client", "client new ok");
	return client;
}

int
rdc_client_free(rdc_client_t *client)
{
	rdc_sockclient_t *sockclient = NULL;
	rdc_sockclient_t *next = NULL;

	if (NULL == client) {
		eag_log_err("rdc_client_free input error");
		return -1;
	}
	
	list_for_each_entry_safe(sockclient, next, 
		&(client->sockcli_list), node) {
		rdc_sockclient_free(sockclient);
	}
	if (NULL != client->sockcli_blkmem) {
		eag_blkmem_destroy(&(client->sockcli_blkmem));
	}
	eag_free(client);

	eag_log_debug("client", "client free ok");
	
	return 0;
}

int
rdc_client_start(rdc_client_t *client)
{
	rdc_sockclient_t *sockclient = NULL;
	int ret = 0;
	
	if (NULL == client) {
		eag_log_err("rdc_client_start input error");
		return EAG_ERR_RDC_CLIENT_NULL;
	}
	/*
	if (1 == client->status) {
		eag_log_err("rdc_client_start already started");
		return -1;
	}
	*/
	list_for_each_entry(sockclient, &(client->sockcli_list), node) {
		if ( (ret = rdc_sockclient_start(sockclient)) != EAG_RETURN_OK) {
			break;
		}
	}

	if (EAG_RETURN_OK != ret) {
		list_for_each_entry(sockclient, &(client->sockcli_list), node) {
			rdc_sockclient_stop(sockclient);
		}
		return ret;
	}

	//client->status = 1;

	eag_log_debug("client", "client start ok");
	return EAG_RETURN_OK;
}

int
rdc_client_stop(rdc_client_t *client)
{
	rdc_sockclient_t *sockclient = NULL;
		
	if (NULL == client) {
		eag_log_err("rdc_client_stop input error");
		return EAG_ERR_RDC_CLIENT_NULL;
	}
	/*
	if (0 == client->status) {
		eag_log_err("rdc_client_stop already stoped");
		return -1;
	}
	*/
	list_for_each_entry(sockclient, &(client->sockcli_list), node) {
		rdc_sockclient_stop(sockclient);
	}

	//client->status = 0;

	eag_log_debug("client", "client stop ok");
	return EAG_RETURN_OK;
}

int
rdc_client_send_request(rdc_client_t *client,
							rdc_pktconn_t *pktconn,
							struct rdc_packet_t *rdcpkt)
{
	rdc_sockclient_t *sockclient = NULL;
	uint8_t id = 0;
	int timeout = 0;
	int found = 0;
	
	if (NULL == client || NULL == pktconn || NULL == rdcpkt) {
		eag_log_err("rdc_client_send_request input error");
		return -1;
	}
	/*
	if (0 == client->status) {
		eag_log_err("rdc_client_send_request client stoped");
		return -1;
	}
	*/
	sockclient = rdc_pktconn_get_sockclient(pktconn);
	id = rdc_pktconn_get_id(pktconn);
	timeout = rdc_pktconn_get_timeout(pktconn);
	if (NULL != sockclient) {
		rdc_sockclient_send_request(sockclient, pktconn, rdcpkt);
		eag_log_debug("client", "client send request ok");
		return EAG_RETURN_OK;
	}

	list_for_each_entry(sockclient, &(client->sockcli_list), node) {
		if (RDC_ID_FREE == sockclient->id[id].status) {
			sockclient->id[id].status = RDC_ID_USED;
			sockclient->id[id].pktconn = pktconn;
			sockclient->id[id].timeout = timeout;
			rdc_pktconn_set_sockclient(pktconn, sockclient);
			found = 1;
			break;
		}
	}
	if (found) {
		rdc_sockclient_send_request(sockclient, pktconn, rdcpkt);
		eag_log_debug("client", "client send request ok");
		return EAG_RETURN_OK;
	}

	sockclient = rdc_sockclient_new(client);
	if (NULL == sockclient) {
		eag_log_err("rdc_client_send_request sockclient_new failed");
		return -1;
	}
	rdc_sockclient_start(sockclient);
	sockclient->id[id].status = RDC_ID_USED;
	sockclient->id[id].pktconn = pktconn;
	sockclient->id[id].timeout = timeout;
	rdc_pktconn_set_sockclient(pktconn, sockclient);
	rdc_sockclient_send_request(sockclient, pktconn, rdcpkt);
	eag_log_debug("client", "client send request ok");
	
	return EAG_RETURN_OK;
}

int
rdc_client_set_nasip(rdc_client_t *client,
							uint32_t nasip)
{
	rdc_sockclient_t *sockclient = NULL;
	rdc_ins_t *rdcins = NULL;
	char ipstr[32] = "";
	
	if (NULL == client) {
		eag_log_err("rdc_client_set_nasip input error");
		return EAG_ERR_RDC_CLIENT_NULL;
	}

	rdcins = client->rdcins;
	if (NULL == rdcins) {
		eag_log_err("rdc_client_set_nasip rdcins null");
		return EAG_ERR_RDC_RDCINS_NULL;
	}
	/*
	if (0 == client->status) {
		eag_log_warning("rdc_client_set_nasip client already started");
		return EAG_ERR_RDC_SERVICE_ALREADY_ENABLE;
	}
	*/
	if (rdc_ins_is_running(rdcins)) {
		eag_log_warning("rdc_client_set_nasip rdc already started");
		return EAG_ERR_RDC_SERVICE_ALREADY_ENABLE;
	}
	
	client->nasip = nasip;
	list_for_each_entry(sockclient, &(client->sockcli_list), node) {
		rdc_sockclient_set_nasip(sockclient, nasip);
	}

	eag_log_info("client set nasip %s",
			ip2str(nasip, ipstr, sizeof(ipstr)));
	return EAG_RETURN_OK;
}

uint32_t
rdc_client_get_nasip(rdc_client_t *client)
{
	if (NULL == client) {
		eag_log_err("rdc_client_get_nasip input error");
		return 0;
	}

	return client->nasip;
}

int 
rdc_client_set_thread_master(rdc_client_t *client,
								eag_thread_master_t *master)
{
	if (NULL == client) {
		eag_log_err("rdc_client_set_thread_master input error");
		return -1;
	}

	client->master = master;

	return 0;
}

int 
rdc_client_set_rdcins(rdc_client_t *client,
						rdc_ins_t *rdcins)
{
	if (NULL == client) {
		eag_log_err("rdc_client_set_rdcins input error");
		return -1;
	}

	client->rdcins = rdcins;

	return 0;
}

int 
rdc_client_log_all(rdc_client_t *client)
{
	rdc_sockclient_t *sockclient = NULL;
	int num = 0;
	int free = 0;
	int used = 0;
	int wait = 0;
	int i = 0;
	
	if (NULL == client) {
		eag_log_err("rdc_client_log_all input error");
		return -1;
	}
	eag_log_info("rdc log all client begin");
	list_for_each_entry(sockclient, &(client->sockcli_list), node) {
		num++;
		free = 0;
		used = 0;
		wait = 0;
		for (i = 0; i < RDC_ID_NUM; i++) {
			if (RDC_ID_FREE == sockclient->id[i].status) {
				free++;
			}
			else if (RDC_ID_USED == sockclient->id[i].status) {
				used++;
			}
			else {
				wait++;
			}
		}
		eag_log_info("%-5d sockclient:%d free:%d used:%d wait:%d",
				num,
				sockclient->sockfd,
				free,
				used,
				wait);
	}

	eag_log_info("rdc log all client end, num: %d", num);
	return 0;
}

static void
rdc_sockclient_event(rdc_sockcli_event_t event,
						rdc_sockclient_t *sockclient)
{
	if (NULL == sockclient) {
		eag_log_err("rdc_sockclient_event input error");
		return;
	}
	
	switch (event) {
	case RDC_SOCKCLI_READ:
		sockclient->t_read =
		    eag_thread_add_read(sockclient->master, sockclient_receive,
					sockclient, sockclient->sockfd);
		break;
	default:
		break;
	}
}

static void
rdc_id_status_event(rdc_id_status_event_t event,
								struct rdc_id *id)
{
	if (NULL == id) {
		eag_log_err("rdc_id_status_event input error");
		return;
	}
	
	switch (event) {
	case RDC_ID_STATUS_TIMEOUT:
		id->t_timeout =
		    eag_thread_add_timer(id->master, id_status_timeout,
					id, id->timeout);
		break;
	default:
		break;
	}
}

