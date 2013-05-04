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
#include "eag_time.h"
#include "eag_util.h"

#include "portal_packet.h"
#include "pdc_packet.h"
#include "pdc_ins.h"
#include "pdc_userconn.h"
#include "pdc_usermap.h"

struct pdc_usermap {
	int sockfd;
	uint32_t ip;
	uint16_t port;
	eag_thread_t *t_read;
	eag_thread_t *t_timer;
	eag_thread_master_t *master;
	pdc_ins_t *pdcins;
	pdc_userconn_db_t *db;
//	pdc_usermap_proc_func_t proc_func;
};

typedef enum {
	PDC_USERMAP_READ,
	PDC_USERMAP_CHECK_TIMEOUT,
} pdc_usermap_event_t;

#define PDC_USERMAP_CHECK_INTERVAL			(60*60)
#define PDC_USERMAP_USERCONN_TIMEOUT		(7*24*60*60)


static void
pdc_usermap_event(pdc_usermap_event_t event,
					pdc_usermap_t *map);


pdc_usermap_t *
pdc_usermap_new(void)
{
	pdc_usermap_t *map = NULL;

	map = eag_malloc(sizeof(*map));
	if (NULL == map) {
		eag_log_err("pdc_usermap_new eag_malloc failed");
		return NULL;
	}

	memset(map, 0, sizeof(*map));
	map->sockfd = -1;
//	map->ip = 
//	map->port = PDC_USERMAP_PORT_BASE+;
//	map->proc_func = client_process_packet;

	eag_log_debug("map", "map new ok");
	return map;	
}

int
pdc_usermap_set_userconn_db( pdc_usermap_t *map, pdc_userconn_db_t *db )
{
	if( NULL == map || NULL == db ){
		eag_log_err("pdc_usermap_set_userconn_db input error map =%p db=%p",
						map, db );
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	map->db = db;
	return EAG_RETURN_OK;
}

int
pdc_usermap_free(pdc_usermap_t *map)
{
	if (NULL == map) {
		eag_log_err("pdc_usermap_free input error");
		return -1;
	}

	if (map->sockfd >= 0) {
		close(map->sockfd);
		map->sockfd = -1;
	}
	eag_free(map);

	eag_log_debug("map", "map free ok");
	return 0;
}

int
pdc_usermap_start(pdc_usermap_t *map)
{
	int ret = 0;
	struct sockaddr_in addr = {0};
	char ipstr[32] = "";
	
	if (NULL == map) {
		eag_log_err("pdc_usermap_start input error");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if (map->sockfd >= 0) {
		eag_log_err("pdc_usermap_start already start fd(%d)", 
			map->sockfd);
		return EAG_RETURN_OK;
	}

	map->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (map->sockfd  < 0) {
		eag_log_err("Can't create map dgram socket: %s",
			safe_strerror(errno));
		map->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(map->port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(map->ip);

#if 0
	ret = setsockopt(map->sockfd, SOL_SOCKET, SO_REUSEADDR,
					&opt, sizeof(opt));
	if (0 != ret) {
		eag_log_err("Can't set REUSEADDR to map socket(%d): %s",
			map->sockfd, safe_strerror(errno));
	}
#endif

	ret  = bind(map->sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		eag_log_err("Can't bind to map socket(%d): %s",
			map->sockfd, safe_strerror(errno));
		close(map->sockfd);
		map->sockfd = -1;
		return EAG_ERR_SOCKET_BIND_FAILED;
	}

	pdc_usermap_event(PDC_USERMAP_READ, map);
	pdc_usermap_event(PDC_USERMAP_CHECK_TIMEOUT, map);
	
	eag_log_info("map(%s:%u) fd(%d) start ok", 
			ip2str(map->ip, ipstr, sizeof(ipstr)),
			map->port,
			map->sockfd);

	return EAG_RETURN_OK;
}

int
pdc_usermap_stop(pdc_usermap_t *map)
{
	char ipstr[32] = "";

	if (NULL == map) {
		eag_log_err("pdc_usermap_stop input error");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if (NULL != map->t_read) {
		eag_thread_cancel(map->t_read);
		map->t_read = NULL;
	}
	
	if (NULL != map->t_timer) {
		eag_thread_cancel(map->t_timer);
		map->t_timer = NULL;
	}
	
	if (map->sockfd >= 0)
	{
		close(map->sockfd);
		map->sockfd = -1;
	}
	eag_log_info("map(%s:%u) stop ok",
			ip2str(map->ip, ipstr, sizeof(ipstr)),
			map->port);

	return EAG_RETURN_OK;
}


static int
client_receive(eag_thread_t *thread)
{
	pdc_usermap_t *map = NULL;
	ssize_t nbyte = 0;
	struct pdc_usermap_pkt_t map_pkt = {0};
	struct sockaddr_in addr = {0};
	socklen_t len = 0;
	uint32_t ip = 0;
	uint16_t port = 0;
	uint32_t userip = 0;
	char ipstr[32] = "";
	pdc_userconn_t *userconn=NULL;
	struct timeval tv;
	
	if (NULL == thread) {
		eag_log_err("client_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;	
	}

	map = eag_thread_get_arg(thread);
	if (NULL == map) {
		eag_log_err("client_receive map null");
		return EAG_ERR_NULL_POINTER;
	}

	len = sizeof(addr);
	nbyte = recvfrom(map->sockfd, &(map_pkt),
			sizeof(map_pkt), 0, (struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("client_receive fd:%d recvfrom failed: %s",
				map->sockfd, safe_strerror(errno));
		return EAG_ERR_SOCKET_RECV_FAILED;
	}

	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	eag_log_debug("userconn", "map fd(%d) receive %d bytes from %s:%u",
				map->sockfd,
				nbyte,
				ip2str(ip, ipstr, sizeof(ipstr)),
				port);
		
	if (nbyte < sizeof(map_pkt)) {
		eag_log_warning("map receive packet size %d < min %d",
			nbyte, sizeof(map_pkt));
		return -1;
	}

	if (nbyte > sizeof(map_pkt)) {
		eag_log_warning("map receive packet size %d > max %d",
			nbyte, sizeof(map_pkt));
		return -1;
	}
	if (map_pkt.hansi_type != pdc_ins_get_hansi_type(map->pdcins) ){
		eag_log_err("get pdc map_pkt but hansi type is error! "\
					"map_pkt.hansi_type=%u ins hansi_type=%u",
					map_pkt.hansi_type, pdc_ins_get_hansi_type(map->pdcins) );
		return -1;
	}

	eag_time_gettimeofday (&tv, NULL);

	userip = ntohl(map_pkt.user_ip);
	ip2str (userip, ipstr, sizeof(ipstr));
//	pdc_userconn_db_log_all( map->db );
	userconn = pdc_userconn_db_find_user (map->db, userip);
	if( NULL != userconn ){
		uint8_t user_slot_id;
		uint8_t user_hansi_id;
		pdc_userconn_get_hansi( userconn, &user_slot_id, &user_hansi_id );
		eag_log_debug("userconn","user %s map already exist! prev:%u-%u,new:%u-%u",
						ipstr, user_slot_id, user_hansi_id,
						map_pkt.slot_id, map_pkt.hansi_id );
	}else{
		userconn = pdc_userconn_new (map->db, userip);
		if( NULL == userconn ){
			return -1;
		}
		pdc_userconn_set_start_time (userconn, tv.tv_sec);
		pdc_userconn_db_add (map->db, userconn);
	}
	pdc_userconn_set_hansi (userconn, map_pkt.slot_id, map_pkt.hansi_type, map_pkt.hansi_id);
	pdc_userconn_set_last_time (userconn, tv.tv_sec);
	pdc_userconn_set_usermac (userconn, map_pkt.user_mac);

	eag_log_debug ("userconn", "set user %s map to %u-%u", 
						ipstr, map_pkt.slot_id, map_pkt.hansi_id );

	return EAG_RETURN_OK;
}


#if 0
int
pdc_usermap_set_ip(pdc_usermap_t *map,
							uint32_t ip)
{
	pdc_ins_t *pdcins = NULL;
	char ipstr[32] = "";
	
	if (NULL == map) {
		eag_log_err("pdc_usermap_set_nasip input error");
		return EAG_ERR_PDC_CLIENT_NULL;
	}

	pdcins = map->pdcins;
	if (NULL == pdcins) {
		eag_log_err("pdc_usermap_set_nasip pdcins null");
		return EAG_ERR_PDC_PDCINS_NULL;
	}

	if (pdc_ins_is_running(pdcins)) {
		eag_log_warning("pdc_usermap_set_nasip pdc already started");
		return EAG_ERR_PDC_SERVICE_ALREADY_ENABLE;
	}
	
	map->ip = ip;

	eag_log_info("map set ip %s",
			ip2str(ip, ipstr, sizeof(ipstr)));
	return EAG_RETURN_OK;
}

uint32_t
pdc_usermap_get_ip(pdc_usermap_t *map)
{
	if (NULL == map) {
		eag_log_err("pdc_usermap_get_nasip input error");
		return 0;
	}

	return map->ip;
}

int
pdc_usermap_set_port(pdc_usermap_t *map,
							uint16_t port)
{
	pdc_ins_t *pdcins = NULL;
	
	if (NULL == map) {
		eag_log_err("pdc_usermap_set_port input error");
		return EAG_ERR_PDC_CLIENT_NULL;
	}

	pdcins = map->pdcins;
	if (NULL == pdcins) {
		eag_log_err("pdc_usermap_set_port pdcins null");
		return EAG_ERR_PDC_PDCINS_NULL;
	}

	if (pdc_ins_is_running(pdcins)) {
		eag_log_warning("pdc_usermap_set_port pdc already started");
		return EAG_ERR_PDC_SERVICE_ALREADY_ENABLE;
	}
	
	map->port = port;

	eag_log_info("map set port %u", port);
	return EAG_RETURN_OK;
}

uint32_t
pdc_usermap_get_port(pdc_usermap_t *map)
{
	if (NULL == map) {
		eag_log_err("pdc_usermap_get_port input error");
		return 0;
	}

	return map->port;
}
#endif
int 
pdc_usermap_set_thread_master(pdc_usermap_t *map,
								eag_thread_master_t *master)
{
	if (NULL == map) {
		eag_log_err("pdc_usermap_set_thread_master input error");
		return -1;
	}

	map->master = master;

	return 0;
}

int 
pdc_usermap_set_pdcins(pdc_usermap_t *map,
						pdc_ins_t *pdcins)
{
	uint8_t slotid = 0;
	uint8_t hansi_id = 0;
	
	if (NULL == map) {
		eag_log_err("pdc_usermap_set_pdcins input error");
		return -1;
	}

	map->pdcins = pdcins;

	slotid = pdc_ins_get_slot_id (pdcins);
	hansi_id = pdc_ins_get_hansi_id(pdcins);
	map->ip = SLOT_IPV4_BASE+slotid;
	map->port = PDC_USERMAP_PORT_BASE+hansi_id;
	return 0;
}


static int
pdc_usermap_check_timeout(eag_thread_t *thread)
{
	pdc_usermap_t *map = NULL;

	map = eag_thread_get_arg(thread);
	if( NULL == map ){
		eag_log_err("pdc_usermap_check_timeout map is NULL!");
		eag_thread_cancel(thread);
		return -1;
	}
	eag_thread_cancel( map->t_timer );
	map->t_timer = NULL;

	pdc_userconn_db_check_timeout( map->db, PDC_USERMAP_USERCONN_TIMEOUT );
		
	map->t_timer = eag_thread_add_timer(map->master,pdc_usermap_check_timeout,
						map, PDC_USERMAP_CHECK_INTERVAL);

	return EAG_RETURN_OK;
}


static void
pdc_usermap_event(pdc_usermap_event_t event,
					pdc_usermap_t *map)
{
	if (NULL == map) {
		eag_log_err("pdc_usermap_event input error");
		return;
	}
	
	switch (event) {
	case PDC_USERMAP_READ:
		map->t_read =
		    eag_thread_add_read(map->master, client_receive,
					map, map->sockfd);
		break;
	case PDC_USERMAP_CHECK_TIMEOUT:
		map->t_timer = 
			eag_thread_add_timer(map->master,pdc_usermap_check_timeout,
					map, PDC_USERMAP_CHECK_INTERVAL);
		break;
	default:
		break;
	}
}

