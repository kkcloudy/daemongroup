/* pdc_server.c */

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
#include "eag_util.h"
#include "portal_packet.h"

#include "pdc_ins.h"
#include "pdc_client.h"
#include "pdc_server.h"
#include "pdc_userconn.h"


#define PDC_MAP_BLKMEM_NAME 		"pdc_map_blkmem"
#define PDC_MAP_BLKMEM_ITEMNUM		128

typedef int (*pdc_server_proc_func_t)(pdc_server_t *,
		uint32_t,
		uint16_t,
		struct pdc_packet_t *);

struct pdc_map {
	struct list_head node;	//must be the first member
	uint32_t userip;
	uint32_t usermask;
	uint8_t eag_slotid;
	uint8_t eag_hansitype;
	uint8_t eag_hansiid;
	uint32_t eag_ip;
	uint16_t eag_port;
};

struct pdc_ipv6_map{
	struct list_head node;	//must be the first member
	struct in6_addr useripv6;
	int prefix_length;
	uint8_t eag_slotid;
	uint8_t eag_hansitype;
	uint8_t eag_hansiid;
	uint32_t eag_ip;
	uint16_t eag_port;
};


struct pdc_server {
	int sockfd;
	uint32_t ip;
	uint16_t port;
	eag_thread_t *t_read;
	eag_thread_master_t *master;
	uint32_t cur_map_num;
	struct list_head map_list;
	eag_blk_mem_t *map_blkmem;
	uint32_t cur_ipv6_map_num;
	struct list_head ipv6_map_list;
	eag_blk_mem_t *ipv6_map_blkmem;
	pdc_ins_t *pdcins;
	pdc_server_proc_func_t proc_func;
};

typedef enum {
	PDC_SERVER_READ,
} pdc_server_event_t;

static void
pdc_server_event(pdc_server_event_t event,
					pdc_server_t *server);

static int
server_process_packet(pdc_server_t *server,
					uint32_t ip,
					uint16_t port,
					struct pdc_packet_t *pdc_packet);

static struct pdc_map *
pdc_map_new(pdc_server_t *server)
{
	struct pdc_map *map = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_map_new input error");
		return NULL;
	}
	
	map = eag_blkmem_malloc_item(server->map_blkmem);
	if (NULL == map) {
		eag_log_err("pdc_map_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(map, 0, sizeof(*map));
	INIT_LIST_HEAD(&(map->node));

	eag_log_debug("server", "map new ok");
	
	return map;	
}

static int
pdc_map_free(pdc_server_t *server, struct pdc_map *map)
{
	if ( NULL == server || NULL == map){
		eag_log_err("pdc_map_free input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	return eag_blkmem_free_item(server->map_blkmem, map);	
}

static struct pdc_map *
pdc_server_lookup_map(pdc_server_t *server, 	
							uint32_t userip, uint32_t usermask)
{
	struct pdc_map *map = NULL;
	struct pdc_map *next = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_server_lookup_map input param error!");
		return NULL;
	}
	
	list_for_each_entry_safe(map, next, &(server->map_list), node){
		if( userip == map->userip 
			&& usermask == map->usermask ){
			return map;
		}
	}

	return NULL;
}

static struct pdc_map *
pdc_server_find_map_overlap( pdc_server_t *server,
							uint32_t userip, uint32_t usermask)
{
	struct pdc_map *map = NULL;
	struct pdc_map *next = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_server_find_map_overlap input param error!");
		return NULL;
	}
	
	list_for_each_entry_safe(map, next, &(server->map_list), node){
		if( is_net_overlap(userip, usermask,
							map->userip,
							map->usermask )){
			return map;
		}
	}

	return NULL;
}

static int
pdc_server_add_map(pdc_server_t *server, 
							uint32_t userip, uint32_t usermask, 
							int eag_slotid, int eag_hansitype, int eag_insid)
{
	struct pdc_map *map = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_server_add_map input param error!");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if ( server->cur_map_num >= MAX_PDC_MAP_NUM ){
		return EAG_ERR_PDC_MAP_NUM_OVERSTEP;
	}

	if (NULL != pdc_server_find_map_overlap(server, userip, usermask)){
		eag_log_err("pdc_server_add_map: pdc_server_find_map_overlap"
						" map conflict!");
		return EAG_ERR_PDC_MAP_CONFLICT;
	}

	map = pdc_map_new(server);
	if ( NULL == map ){
		eag_log_err("pdc_server_add_map: pdc_map_new error!");
		return EAG_ERR_PDC_MAP_NULL;
	}

	map->userip = userip;
	map->usermask = usermask;
	map->eag_slotid = eag_slotid;
	map->eag_hansitype = eag_hansitype;
	map->eag_hansiid = eag_insid;
//	map->eag_ip = SLOT_IPV4_BASE + eag_slotid;
	if( HANSI_LOCAL == eag_hansitype ){
		map->eag_ip = SLOT_IPV4_BASE + 100 + eag_insid;
		map->eag_port = EAG_PORTAL_PORT_BASE + eag_insid;
	}else{
		map->eag_ip = SLOT_IPV4_BASE + eag_slotid;
		map->eag_port = EAG_PORTAL_PORT_BASE + MAX_HANSI_ID + eag_insid;
	}
	list_add_tail(&(map->node), &(server->map_list));
	server->cur_map_num += 1;

	return 0;
}

static int
pdc_server_del_map(pdc_server_t *server, 
						uint32_t userip, uint32_t usermask)
{
	struct pdc_map *map = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_server_del_map input param error!");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	map = pdc_server_lookup_map(server, userip, usermask);
	if( NULL == map ){
		eag_log_err("pdc_server_del_map not find map error!");
		return EAG_ERR_PDC_MAP_NOT_FOUND;
	}

	list_del(&(map->node));
	server->cur_map_num -=1;
	
	return pdc_map_free(server, map);
}

static int
pdc_server_modify_map(pdc_server_t *server, 
							uint32_t userip, uint32_t usermask, 
							int eag_slotid, int eag_hansitype, int eag_insid)
{
	struct pdc_map *map = NULL;
	
	if ( NULL == server ){
		eag_log_err("pdc_server_del_map input param error!");
		return EAG_ERR_PDC_SERVER_NULL;
	}
	
	map = pdc_server_lookup_map(server, userip, usermask);
	if ( NULL == map ){
		eag_log_err("pdc_server_modify_map not find map error!");
		return EAG_ERR_PDC_MAP_NOT_FOUND;
	}

	map->userip = userip;
	map->usermask = usermask;
	map->eag_slotid = eag_slotid;
	map->eag_hansitype = eag_hansitype;
	map->eag_hansiid = eag_insid;
//	map->eag_ip = SLOT_IPV4_BASE + eag_slotid;
	if( HANSI_LOCAL == eag_hansitype ){
		map->eag_ip = SLOT_IPV4_BASE + 100 + eag_insid;
		map->eag_port = EAG_PORTAL_PORT_BASE + eag_insid;
	}else{
		map->eag_ip = SLOT_IPV4_BASE + eag_slotid;
		map->eag_port = EAG_PORTAL_PORT_BASE + MAX_HANSI_ID + eag_insid;
	}

	return 0;
}

#if 0
int
pdc_server_get_map(struct pdc_map *map, 
							uint32_t *userip, uint32_t *usermask, 
							uint32_t *eag_ip, uint16_t *eag_port)
{
	if (NULL == map 
		|| NULL == userip
		|| NULL == usermask
		|| NULL == eag_ip
		|| NULL == eag_port){
		eag_log_err("pdc_server_get_map input param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	*userip = map->userip;
	*usermask = map->usermask;
	*eag_ip = map->eag_ip;
	*eag_port = map->eag_port;

	return 0;
}
#endif

static struct pdc_map *
pdc_server_find_map_by_userip(pdc_server_t *server, uint32_t userip)
{
	
	struct pdc_map *map = NULL;
	struct pdc_map *next = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_server_find_map_by_userip input param error!");
		return NULL;
	}
	
	list_for_each_entry_safe(map, next, &(server->map_list), node){
		if((map->userip & map->usermask) == (userip & map->usermask)){
			return map;
		}
	}

	return NULL;
}

static struct pdc_ipv6_map *
pdc_ipv6_map_new(pdc_server_t *server)
{
	struct pdc_ipv6_map *map = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_ipv6_map_new input error");
		return NULL;
	}
	
	map = eag_blkmem_malloc_item(server->ipv6_map_blkmem);
	if (NULL == map) {
		eag_log_err("pdc_ipv6_map_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(map, 0, sizeof(*map));
	INIT_LIST_HEAD(&(map->node));

	eag_log_debug("server", "ipv6 map new ok");
	
	return map;	
}

static int
pdc_ipv6_map_free(pdc_server_t *server, struct pdc_ipv6_map *map)
{
	if ( NULL == server || NULL == map){
		eag_log_err("pdc_ipv6_map_free input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	return eag_blkmem_free_item(server->ipv6_map_blkmem, map);	
}


static struct pdc_ipv6_map *
pdc_server_find_ipv6_map_overlap( pdc_server_t *server,
							uint32_t useripv6[4], int prefix_length)
{
	struct pdc_ipv6_map *map = NULL;
	struct pdc_ipv6_map *next = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_server_find_ipv6_map_overlap input param error!");
		return NULL;
	}
	
	list_for_each_entry_safe(map, next, &(server->ipv6_map_list), node){
		if(is_ipv6net_overlap(useripv6, prefix_length, 
							(uint32_t *)&(map->useripv6),
							map->prefix_length )){
			return map;
		}
	}

	return NULL;
}

static struct pdc_ipv6_map *
pdc_server_lookup_ipv6_map(pdc_server_t *server, 	
							uint32_t useripv6[4], int prefix_length)
{
	struct pdc_ipv6_map *map = NULL;
	struct pdc_ipv6_map *next = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_server_lookup_ipv6_map input param error!");
		return NULL;
	}

	list_for_each_entry_safe(map, next, &(server->ipv6_map_list), node){
		if( !memcmp(useripv6, &(map->useripv6), sizeof(struct in6_addr))
			&& prefix_length == map->prefix_length ){
			return map;
		}
	}

	return NULL;
}

static struct pdc_ipv6_map *
pdc_server_find_ipv6_map_by_userip(pdc_server_t *server, uint32_t useripv6[4])
{
	int i = 0;
	uint32_t ipv6[4] = {0};
	uint32_t prefix[4] = {0};
	struct pdc_ipv6_map *map = NULL;
	struct pdc_ipv6_map *next = NULL;
	

	if ( NULL == server ){
		eag_log_err("pdc_server_find_ipv6_map_by_userip input param error!");
		return NULL;
	}
	
	list_for_each_entry_safe(map, next, &(server->ipv6_map_list), node){
		memset(ipv6, 0 , sizeof(ipv6));
		memset(prefix, 0 , sizeof(prefix));
		memcpy(ipv6, &(map->useripv6), sizeof(struct in6_addr));
		ipv6mask2binary(map->prefix_length, prefix);
		for (i = 0; i < 4; i++) {
			if ((ipv6[i] & prefix[i]) != (useripv6[i] & prefix[i]))	{
				break;
			}
		}
		if (4 == i) {
			return map;	
		}
	}

	return NULL;
}

static int
pdc_server_add_ipv6_map(pdc_server_t *server, 
							uint32_t useripv6[4], int prefix_length,
							int eag_slotid, int eag_hansitype, int eag_insid)
{
	struct pdc_ipv6_map *map = NULL;
	
	if ( NULL == server ){
		eag_log_err("pdc_server_add_ipv6_map input param error!");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if ( server->cur_ipv6_map_num >= MAX_PDC_IPV6_MAP_NUM ){
		return EAG_ERR_PDC_MAP_NUM_OVERSTEP;
	}

	if (NULL != pdc_server_find_ipv6_map_overlap(server, useripv6, prefix_length)){
		eag_log_err("pdc_server_add_ipv6_map: pdc_server_find_ipv6_map_overlap"
						" map conflict!");
		return EAG_ERR_PDC_MAP_CONFLICT;
	}

	map = pdc_ipv6_map_new(server);
	if ( NULL == map ){
		eag_log_err("pdc_server_add_ipv6_map: pdc_ipv6_map_new error!");
		return EAG_ERR_PDC_MAP_NULL;
	}

	memcpy(&(map->useripv6), useripv6, sizeof(struct in6_addr));
	map->prefix_length = prefix_length;
	map->eag_slotid = eag_slotid;
	map->eag_hansitype = eag_hansitype;
	map->eag_hansiid = eag_insid;
//	map->eag_ip = SLOT_IPV4_BASE + eag_slotid;
	if( HANSI_LOCAL == eag_hansitype ){
		map->eag_ip = SLOT_IPV4_BASE + 100 + eag_insid;
		map->eag_port = EAG_PORTAL_PORT_BASE + eag_insid;
	}else{
		map->eag_ip = SLOT_IPV4_BASE + eag_slotid;
		map->eag_port = EAG_PORTAL_PORT_BASE + MAX_HANSI_ID + eag_insid;
	}
	list_add_tail(&(map->node), &(server->ipv6_map_list));
	server->cur_ipv6_map_num += 1;

	return 0;
}

static int
pdc_server_del_ipv6_map(pdc_server_t *server, 
						uint32_t useripv6[4], int prefix_length)
{
	struct pdc_ipv6_map *map = NULL;

	if ( NULL == server ){
		eag_log_err("pdc_server_del_ipv6_map input param error!");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	map = pdc_server_lookup_ipv6_map(server, useripv6, prefix_length);
	if( NULL == map ){
		eag_log_err("pdc_server_del_ipv6_map not find map error!");
		return EAG_ERR_PDC_MAP_NOT_FOUND;
	}

	list_del(&(map->node));
	server->cur_ipv6_map_num -= 1;
	
	return pdc_ipv6_map_free(server, map);
}

static int
pdc_server_modify_ipv6_map(pdc_server_t *server, 
							uint32_t useripv6[4], int prefix_length, 
							int eag_slotid, int eag_hansitype, int eag_insid)
{
	struct pdc_ipv6_map *map = NULL;
	
	if ( NULL == server ){
		eag_log_err("pdc_server_modify_ipv6_map input param error!");
		return EAG_ERR_PDC_SERVER_NULL;
	}
	
	map = pdc_server_lookup_ipv6_map(server, useripv6, prefix_length);
	if ( NULL == map ){
		eag_log_err("pdc_server_modify_ipv6_map not find map error!");
		return EAG_ERR_PDC_MAP_NOT_FOUND;
	}

	memcpy(&(map->useripv6), useripv6, sizeof(struct in6_addr));
	map->prefix_length = prefix_length;
	map->eag_slotid = eag_slotid;
	map->eag_hansitype = eag_hansitype;
	map->eag_hansiid = eag_insid;
//	map->eag_ip = SLOT_IPV4_BASE + eag_slotid;
	if( HANSI_LOCAL == eag_hansitype ){
		map->eag_ip = SLOT_IPV4_BASE + 100 + eag_insid;
		map->eag_port = EAG_PORTAL_PORT_BASE + eag_insid;
	}else{
		map->eag_ip = SLOT_IPV4_BASE + eag_slotid;
		map->eag_port = EAG_PORTAL_PORT_BASE + MAX_HANSI_ID + eag_insid;
	}

	return 0;
}

pdc_server_t *
pdc_server_new(void)
{
	pdc_server_t *server = NULL;

	server = eag_malloc(sizeof(*server));
	if (NULL == server) {
		eag_log_err("pdc_server_new eag_malloc failed");
		return NULL;
	}

	memset(server, 0, sizeof(*server));
	if (EAG_RETURN_OK != eag_blkmem_create(&(server->map_blkmem),
					       PDC_MAP_BLKMEM_NAME,
					       sizeof(struct pdc_map),
					       PDC_MAP_BLKMEM_ITEMNUM,
					       MAX_BLK_NUM)) {
		eag_log_err("pdc_server_new blkmem_create failed");
		eag_free(server);
		server = NULL;
		return NULL;
	}
	INIT_LIST_HEAD(&(server->map_list));

	if (EAG_RETURN_OK != eag_blkmem_create(&(server->ipv6_map_blkmem),
					       PDC_MAP_BLKMEM_NAME,
					       sizeof(struct pdc_ipv6_map),
					       PDC_MAP_BLKMEM_ITEMNUM,
					       MAX_BLK_NUM)) {
		eag_log_err("pdc_server_new ipv6 blkmem_create failed");
		eag_free(server);
		server = NULL;
		return NULL;
	}
	INIT_LIST_HEAD(&(server->ipv6_map_list));
	server->sockfd = -1;
	server->cur_map_num = 0;
	server->cur_ipv6_map_num = 0;
	server->proc_func = server_process_packet;

	eag_log_debug("server", "server new ok");
	return server;
}

int
pdc_server_free(pdc_server_t *server)
{
	struct pdc_map *map = NULL;
	struct pdc_map *next = NULL;

	struct pdc_ipv6_map *ipv6_map = NULL;
	struct pdc_ipv6_map *ipv6_next = NULL;

	if (NULL == server) {
		eag_log_err("pdc_server_free input error");
		return EAG_ERR_PDC_SERVER_NULL;
	}
	
	list_for_each_entry_safe(map, next, &(server->map_list), node) {
		pdc_map_free(server, map);
	}
	if (NULL != server->map_blkmem) {
		eag_blkmem_destroy(&(server->map_blkmem));
	}

	list_for_each_entry_safe(ipv6_map, ipv6_next, &(server->map_list), node) {
		pdc_map_free(server, ipv6_map);
	}
	if (NULL != server->ipv6_map_blkmem) {
		eag_blkmem_destroy(&(server->ipv6_map_blkmem));
	}
	eag_free(server);

	eag_log_debug("server", "server free ok");
	
	return 0;
}

int
pdc_server_start(pdc_server_t *server)
{
	int ret = 0;
	struct sockaddr_in addr = {0};
	char ipstr[32] = "";
	
	if (NULL == server) {
		eag_log_err("pdc_server_start input error");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if (server->sockfd >= 0) {
		eag_log_err("pdc_server_start already start fd(%d)", 
			server->sockfd);
		return EAG_RETURN_OK;
	}

	server->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server->sockfd  < 0) {
		eag_log_err("Can't create server dgram socket: %s",
			safe_strerror(errno));
		server->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(server->port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(server->ip);
	#if 0
	ret = setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR,
					&opt, sizeof(opt));
	if (0 != ret) {
		eag_log_err("Can't set REUSEADDR to server socket(%d): %s",
			server->sockfd, safe_strerror(errno));
	}
	#endif
	ret  = bind(server->sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		eag_log_err("Can't bind to server socket(%d): %s",
			server->sockfd, safe_strerror(errno));
		close(server->sockfd);
		server->sockfd = -1;
		return EAG_ERR_SOCKET_BIND_FAILED;
	}

	pdc_server_event(PDC_SERVER_READ, server);
	
	eag_log_info("server(%s:%u) fd(%d) start ok", 
			ip2str(server->ip, ipstr, sizeof(ipstr)),
			server->port,
			server->sockfd);

	return EAG_RETURN_OK;
}

int
pdc_server_stop(pdc_server_t *server)
{
	char ipstr[32] = "";

	if (NULL == server) {
		eag_log_err("pdc_server_stop input error");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if (NULL != server->t_read) {
		eag_thread_cancel(server->t_read);
		server->t_read = NULL;
	}
	if (server->sockfd >= 0)
	{
		close(server->sockfd);
		server->sockfd = -1;
	}
	eag_log_info("server(%s:%u) stop ok",
			ip2str(server->ip, ipstr, sizeof(ipstr)),
			server->port);

	return EAG_RETURN_OK;
}

static int
server_receive(eag_thread_t *thread)
{
	pdc_server_t *server = NULL;
	struct sockaddr_in addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	struct pdc_packet_t pdc_packet = {0};
	struct portal_packet_t *portal_packet = NULL;
	int packet_len = 0;
	char ipstr[32] = "";
	uint32_t ip = 0;
	uint16_t port = 0;
	int pdc_minsize = 0;
	
	if (NULL == thread) {
		eag_log_err("server_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	server = eag_thread_get_arg(thread);
	if (NULL == server) {
		eag_log_err("server_receive server null");
		return EAG_ERR_NULL_POINTER;
	}

	len = sizeof(addr);
	nbyte = recvfrom(server->sockfd, &pdc_packet, sizeof(pdc_packet), 0,
					(struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("server_receive fd:%d recvfrom failed: %s",
					server->sockfd, safe_strerror (errno));
		return EAG_ERR_SOCKET_RECV_FAILED;
	}

	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	eag_log_debug("server", "server fd(%d) receive %d bytes from %s:%u",
		server->sockfd,
		nbyte,
		ip2str(ip, ipstr, sizeof(ipstr)),
		port);

	pdc_minsize = pdc_packet_minsize();
	if (nbyte < pdc_minsize) {
		eag_log_warning("server_receive packet size %d < min %d",
			nbyte, pdc_minsize);
		return -1;
	}

	if (nbyte > PDC_PACKET_SIZE) {
		eag_log_warning("server_receive packet size %d > max %d",
			nbyte, PDC_PACKET_SIZE);
		return -1;
	}
	
	portal_packet = (struct portal_packet_t *)(&(pdc_packet.data));
	packet_len = portal_packet_get_length(portal_packet);
	if (nbyte != packet_len + PDC_PACKET_HEADSIZE) {
		eag_log_warning("server_receive packet size %d != len %d",
			nbyte, packet_len + PDC_PACKET_HEADSIZE);
		return -1;
	}
	
	if (ACK_CHALLENGE == portal_packet->type
		|| ACK_AUTH == portal_packet->type
		|| ACK_LOGOUT == portal_packet->type
		|| NTF_LOGOUT == portal_packet->type
		|| ACK_INFO == portal_packet->type 
		|| REQ_MACBINDING_INFO == portal_packet->type
		|| NTF_USER_LOGON == portal_packet->type
		|| NTF_USER_LOGOUT == portal_packet->type)
	{
		if (NULL != server->proc_func) {
			server->proc_func(server, ip, port, &pdc_packet);
		}
		else {
			eag_log_warning("server_receive proc_func null");
		}
	}
	else {
		eag_log_warning("server_receive unexpected packet type %X",
			portal_packet->type);
		return -1;
	}

	return EAG_RETURN_OK;
}

static int
server_process_packet(pdc_server_t *server,
							uint32_t ip,
							uint16_t port,
							struct pdc_packet_t *pdc_packet)
{
	struct portal_packet_t *portal_packet = NULL;
	uint32_t toip = 0;
	uint16_t toport = 0;
	char ipstr[32] = "";
	char toipstr[32] = "";
	pdc_ins_t *pdcins = NULL;
	pdc_client_t *client = NULL;
	char desc[64] = "";
	
	if (NULL == server || NULL == pdc_packet) {
		eag_log_err("server_process_packet input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	toip = ntohl(pdc_packet->ip);
	toport = ntohs(pdc_packet->port);
	portal_packet = (struct portal_packet_t *)(&(pdc_packet->data));

	memcpy(desc, pdc_packet->desc, sizeof(pdc_packet->desc));
	eag_log_debug("server", "server process packet "
		"eag(%s:%u) portal(%s:%u) "
		"length(%u) slotid(%u) hansiType(%u) hansiId(%u) "
		"clientType(%d) desc(%s)",
		ip2str(ip, ipstr, sizeof(ipstr)),
		port,
		ip2str(toip, toipstr, sizeof(toipstr)),
		toport,
		ntohs(pdc_packet->length),
		pdc_packet->slotid,
		pdc_packet->hansi_type,
		pdc_packet->hansi_id,
		pdc_packet->client_type,
		desc);
	
	pdcins = server->pdcins;
	if (NULL == pdcins) {
		eag_log_err("server_process_packet pdcins null");
		return -1;
	}

	client = pdc_ins_get_client(pdcins);
	if (NULL == client) {
		eag_log_err("server_process_packet client null");
		return -1;
	}
	
	pdc_client_send_packet(client, pdc_packet);
	
	eag_log_debug("server", "server process packet ok");
	return EAG_RETURN_OK;
}

int
pdc_server_send_packet(pdc_server_t *server,
								struct pdc_packet_t *pdc_packet)
{
	struct portal_packet_t *portal_packet = NULL;
	struct portal_packet_attr *attr = NULL;
	struct sockaddr_in addr = {0};
	size_t length = 0;
	ssize_t nbyte = 0;
	uint32_t userip = 0;
	char ipstr[32] = "";
	uint32_t useripv6[4] = {0};
	char ipv6str[48] = "";
	int ret = 0;
	struct pdc_map *map = NULL;
	struct pdc_ipv6_map *ipv6map = NULL;
	pdc_client_t *client = NULL;
	
	if (NULL == server || NULL == pdc_packet) {
		eag_log_err("pdc_server_send_packet input error");
		return -1;
	}

	portal_packet = (struct portal_packet_t *)(&(pdc_packet->data));
	userip = ntohl(portal_packet->user_ip);
	attr = portal_packet_get_attr(portal_packet, ATTR_USER_IPV6);	
	if (0 != userip) {
		ip2str(userip, ipstr, sizeof(ipstr));
		eag_log_debug("server","pdc_server_send_packet ipv4 = %s", ipstr);
		map = pdc_server_find_map_by_userip(server, userip);
		if (NULL != map) {			
			memset(&addr, 0, sizeof(struct sockaddr_in));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(map->eag_port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
			addr.sin_len = sizeof(struct sockaddr_in);
#endif
			addr.sin_addr.s_addr = htonl(map->eag_ip);		
		}
	} else if (NULL != attr) {
		memcpy(useripv6, attr->value, attr->len - 2);
		ipv6tostr((struct in6_addr *)useripv6, ipv6str, sizeof(ipv6str));
		eag_log_debug("server","pdc_server_send_packet ipv6 = %s", ipv6str);
		ipv6map = pdc_server_find_ipv6_map_by_userip(server, useripv6);
		if (NULL != ipv6map) {			
			memset(&addr, 0, sizeof(struct sockaddr_in));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(ipv6map->eag_port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
			addr.sin_len = sizeof(struct sockaddr_in);
#endif
			addr.sin_addr.s_addr = htonl(ipv6map->eag_ip);		
		}
	} else {
		eag_log_err("pdc_server_send_packet userip and useripv6 is 0!");
		return -1;
	}

	if (NULL == map && NULL == ipv6map) {
		uint32_t eagip = 0;
		uint16_t eagport = 0;
		pdc_userconn_t *userconn = NULL;
		pdc_userconn_db_t *userdb = NULL;

		userdb = pdc_ins_get_userconn_db( server->pdcins );
		if( NULL == userdb ){
			eag_log_err("pdc_server_send_packet get userdb error!");
			return -1;
		}
		userconn = pdc_userconn_db_find_user( userdb, userip );
		if( NULL == userconn ){
			eag_log_warning("pdc_server_send_packet failed ! get userconn error! "\
						"and not find user map config!");
			
			client = pdc_ins_get_client(server->pdcins);
			ret = pdc_client_not_find_map_proc(client, pdc_packet);
			return ret;
		}
		pdc_userconn_get_eaginfo( userconn, &eagip, &eagport );
				
		memset(&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(eagport);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
		addr.sin_len = sizeof(struct sockaddr_in);
#endif
		addr.sin_addr.s_addr = htonl(eagip);
	}

	length = ntohs(pdc_packet->length);
	nbyte = sendto(server->sockfd, pdc_packet, length,
				0, (struct sockaddr *)&addr, sizeof(addr));
	if (nbyte < 0) {
		eag_log_err("pdc_server_send_packet sendto failed: %s",
			safe_strerror(errno));
		return -1;
	}

	eag_log_debug("server", "server send packet ok");
	return EAG_RETURN_OK;
}

int 
pdc_server_set_thread_master(pdc_server_t *server,
								eag_thread_master_t *master)
{
	if (NULL == server) {
		eag_log_err("pdc_server_set_thread_master input error");
		return -1;
	}

	server->master = master;

	return 0;
}

int 
pdc_server_set_pdcins(pdc_server_t *server,
						pdc_ins_t *pdcins)
{
	if (NULL == server) {
		eag_log_err("pdc_server_set_pdcins input error");
		return -1;
	}

	server->pdcins = pdcins;

	return 0;
}

int 
pdc_server_set_ip(pdc_server_t *server,
						uint32_t ip)
{
	char ipstr[32] = "";
	
	if (NULL == server) {
		eag_log_err("pdc_server_set_ip input error");
		return -1;
	}

	server->ip = ip;

	eag_log_info("server set ip %s",
		ip2str(ip, ipstr, sizeof(ipstr)));
	return 0;
}

int 
pdc_server_set_port(pdc_server_t *server,
						uint16_t port)
{
	if (NULL == server) {
		eag_log_err("pdc_server_set_port input error");
		return -1;
	}

	server->port = port;
	
	eag_log_info("server set port %u", port);
	return 0;
}

static void
pdc_server_event(pdc_server_event_t event,
					pdc_server_t *server)
{
	if (NULL == server) {
		eag_log_err("pdc_server_event input error");
		return;
	}
	
	switch (event) {
	case PDC_SERVER_READ:
		server->t_read =
		    eag_thread_add_read(server->master, server_receive,
					server, server->sockfd);
		break;
	default:
		break;
	}
}

DBusMessage *
pdc_dbus_method_add_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_server_t *server = NULL;
	uint32_t userip = 0;
	uint32_t usermask = 0;
	int eag_slotid = 0;
	int eag_hansitype = 0;
	int eag_insid = 0;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_add_map "
					"DBUS new reply message error!");
		return NULL;
	}

	server = (pdc_server_t *)user_data;
	if (NULL == server) {
		eag_log_err("pdc_dbus_method_add_map user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_UINT32, &usermask,
								DBUS_TYPE_INT32, &eag_slotid,
								DBUS_TYPE_INT32, &eag_hansitype,
								DBUS_TYPE_INT32, &eag_insid,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_add_map "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_add_map %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = pdc_server_add_map(server, userip, usermask, eag_slotid, eag_hansitype, eag_insid);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *
pdc_dbus_method_modify_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_server_t *server = NULL;
	uint32_t userip = 0;
	uint32_t usermask = 0;
	int eag_slotid = 0;
	int eag_hansitype = 0;
	int eag_insid = 0;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_add_map "
					"DBUS new reply message error!");
		return NULL;
	}

	server = (pdc_server_t *)user_data;
	if (NULL == server) {
		eag_log_err("pdc_dbus_method_add_map user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_UINT32, &usermask,
								DBUS_TYPE_INT32, &eag_slotid,
								DBUS_TYPE_INT32, &eag_hansitype,
								DBUS_TYPE_INT32, &eag_insid,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_add_map "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_add_map %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = pdc_server_modify_map(server, userip, usermask, eag_slotid, eag_hansitype, eag_insid);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *
pdc_dbus_method_del_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_server_t *server = NULL;
	uint32_t userip = 0;
	uint32_t usermask = 0;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_add_map "
					"DBUS new reply message error!");
		return NULL;
	}

	server = (pdc_server_t *)user_data;
	if (NULL == server) {
		eag_log_err("pdc_dbus_method_add_map user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_UINT32, &usermask,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_add_map "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_add_map %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = pdc_server_del_map(server, userip, usermask);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	return reply;
}


DBusMessage *
pdc_dbus_method_show_maps(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_server_t *server = NULL;
	struct pdc_map *map = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = EAG_RETURN_OK;
	int i = 0;
	int eag_slotid = 0;
	int eag_hansitype = 0;
	int eag_hansiid = 0;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_show_map "
					"DBUS new reply message error!");
		return NULL;
	}

	server = (pdc_server_t *)user_data;
	if (NULL == server) {
		eag_log_err("pdc_dbus_method_show_map user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	if (EAG_RETURN_OK == ret){
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &(server->cur_map_num));
		i = 0;
		if (server->cur_map_num > 0){
			DBusMessageIter  iter_array;
			dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												 DBUS_TYPE_UINT32_AS_STRING
												 DBUS_TYPE_UINT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_UINT32_AS_STRING
											     	 DBUS_TYPE_UINT16_AS_STRING																				
											DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);
			
			list_for_each_entry(map, &(server->map_list), node){
				DBusMessageIter iter_struct;
				dbus_message_iter_open_container (&iter_array,
												DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct);
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32, &(map->userip));
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32, &(map->usermask));
				eag_slotid = map->eag_slotid;
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_INT32, &eag_slotid);
				eag_hansitype = map->eag_hansitype;
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_INT32, &eag_hansitype);
				eag_hansiid = map->eag_hansiid;
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_INT32, &eag_hansiid);
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32, &(map->eag_ip));
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT16, &(map->eag_port));
				dbus_message_iter_close_container (&iter_array, &iter_struct);
				i+=1;
			}
			dbus_message_iter_close_container (&iter, &iter_array);
		}

		if (i != server->cur_map_num){
			eag_log_err("cur_map_num is not equal to real fetched map num error!");
			if (reply){
				dbus_message_unref (reply);
			}
			return NULL;
		}
	}

	return reply;
	
}

DBusMessage *
pdc_dbus_method_add_ipv6_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_server_t *server = NULL;
	uint32_t useripv6[4] = {0};
	int prefix_length = 0;
	int eag_slotid = 0;
	int eag_hansitype = 0;
	int eag_insid = 0;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_add_ipv6_map "
					"DBUS new reply message error!");
		return NULL;
	}
	
	server = (pdc_server_t *)user_data;
	if (NULL == server) {
		eag_log_err("pdc_dbus_method_add_ipv6_map user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &useripv6[0],
								DBUS_TYPE_UINT32, &useripv6[1],
								DBUS_TYPE_UINT32, &useripv6[2],
								DBUS_TYPE_UINT32, &useripv6[3],
								DBUS_TYPE_INT32, &prefix_length,
								DBUS_TYPE_INT32, &eag_slotid,
								DBUS_TYPE_INT32, &eag_hansitype,
								DBUS_TYPE_INT32, &eag_insid,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_add_ipv6_map "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_add_ipv6_map %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = pdc_server_add_ipv6_map(server, useripv6, prefix_length, eag_slotid, eag_hansitype, eag_insid);
replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	return reply;
}


DBusMessage *
pdc_dbus_method_del_ipv6_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_server_t *server = NULL;
	uint32_t useripv6[4] = {0};
	int prefix_length = 0;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_del_ipv6_map "
					"DBUS new reply message error!");
		return NULL;
	}

	server = (pdc_server_t *)user_data;
	if (NULL == server) {
		eag_log_err("pdc_dbus_method_del_ipv6_map user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &useripv6[0],
								DBUS_TYPE_UINT32, &useripv6[1],
								DBUS_TYPE_UINT32, &useripv6[2],
								DBUS_TYPE_UINT32, &useripv6[3],
								DBUS_TYPE_INT32, &prefix_length,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_del_ipv6_map "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_del_ipv6_map %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = pdc_server_del_ipv6_map(server, useripv6, prefix_length);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	return reply;
}


DBusMessage *
pdc_dbus_method_modify_ipv6_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_server_t *server = NULL;
	uint32_t useripv6[4] = {0};
	int prefix_length = 0;
	int eag_slotid = 0;
	int eag_hansitype = 0;
	int eag_insid = 0;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_modify_ipv6_map "
					"DBUS new reply message error!");
		return NULL;
	}

	server = (pdc_server_t *)user_data;
	if (NULL == server) {
		eag_log_err("pdc_dbus_method_modify_ipv6_map user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &useripv6[0],
								DBUS_TYPE_UINT32, &useripv6[1],
								DBUS_TYPE_UINT32, &useripv6[2],
								DBUS_TYPE_UINT32, &useripv6[3],
								DBUS_TYPE_INT32, &prefix_length,
								DBUS_TYPE_INT32, &eag_slotid,
								DBUS_TYPE_INT32, &eag_hansitype,
								DBUS_TYPE_INT32, &eag_insid,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_modify_ipv6_map "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_modify_ipv6_map %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = pdc_server_modify_ipv6_map(server, useripv6, prefix_length, eag_slotid, eag_hansitype, eag_insid);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	return reply;
}


DBusMessage *
pdc_dbus_method_show_ipv6_maps(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_server_t *server = NULL;
	struct pdc_ipv6_map *map = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	uint32_t userip[4] = {0};
	int ret = EAG_RETURN_OK;
	int i = 0;
	int eag_slotid = 0;
	int eag_hansitype = 0;
	int eag_hansiid = 0;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_show_ipv6_maps "
					"DBUS new reply message error!");
		return NULL;
	}
	
	server = (pdc_server_t *)user_data;
	if (NULL == server) {
		eag_log_err("pdc_dbus_method_show_ipv6_maps user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	if (EAG_RETURN_OK == ret){
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &(server->cur_ipv6_map_num));
		i = 0;
		if (server->cur_ipv6_map_num > 0){
			DBusMessageIter  iter_array;
			dbus_message_iter_open_container (&iter,
										DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												 DBUS_TYPE_UINT32_AS_STRING
												 DBUS_TYPE_UINT32_AS_STRING
												 DBUS_TYPE_UINT32_AS_STRING
												 DBUS_TYPE_UINT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_INT32_AS_STRING
												 DBUS_TYPE_UINT32_AS_STRING
											     DBUS_TYPE_UINT16_AS_STRING																				
											DBUS_STRUCT_END_CHAR_AS_STRING,
										&iter_array);
			
			list_for_each_entry(map, &(server->ipv6_map_list), node){
				memset(userip , 0, sizeof(userip));
				memcpy(userip, &(map->useripv6), sizeof(struct in6_addr));
				DBusMessageIter iter_struct;
				dbus_message_iter_open_container (&iter_array,
												DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct);				
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32, &(userip[0]));
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32, &(userip[1]));
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32, &(userip[2]));
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32, &(userip[3]));
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_INT32, &(map->prefix_length));
				
				eag_slotid = map->eag_slotid;				
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_INT32, &eag_slotid);
				eag_hansitype = map->eag_hansitype;
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_INT32, &eag_hansitype);
				eag_hansiid = map->eag_hansiid;
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_INT32, &eag_hansiid);
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT32, &(map->eag_ip));
				dbus_message_iter_append_basic(&iter_struct,
												DBUS_TYPE_UINT16, &(map->eag_port));
				dbus_message_iter_close_container (&iter_array, &iter_struct);
				i+=1;
			}
			dbus_message_iter_close_container (&iter, &iter_array);
		}

		if (i != server->cur_ipv6_map_num){
			eag_log_err("cur_map_num is not equal to real fetched map num error!");
			if (reply){
				dbus_message_unref (reply);
			}
			return NULL;
		}
	}

	return reply;
	
}

