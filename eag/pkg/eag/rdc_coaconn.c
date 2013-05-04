/* rdc_coaconn.c */

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

#include "rdc_ins.h"
#include "rdc_server.h"
#include "rdc_client.h"
//#include "rdc_userconn.h"
#include "rdc_coaconn.h"

#define RDC_COACONN_BLKMEM_NAME 		  "rdc_coaconn_blkmem"
#define RDC_COACONN_BLKMEM_ITEMNUM		64

struct rdc_coaconn_db {
	int num;
	hashtable *htable;
	eag_blk_mem_t *conn_blkmem;
	eag_thread_master_t *master;
	int timeout;
};

struct rdc_coaconn {
	struct hlist_node hnode;
	rdc_coaconn_db_t *coadb;
	rdc_server_t *server;
	uint32_t ip;
	uint16_t port;
	uint8_t id;
	uint32_t coaip;
	uint16_t coaport;
	eag_thread_master_t *master;
	eag_thread_t *t_timeout;
//	rdc_userconn_t *userconn;
	int timeout;
	int request;
	int response;
};

typedef enum {
	RDC_COACONN_TIMEOUT,
} rdc_coaconn_event_t;

struct rdc_coaconn_key {
	uint32_t ip;
	uint16_t port;
	uint8_t id;
} __attribute__ ((packed));

static void
rdc_coaconn_event(rdc_coaconn_event_t event,
						rdc_coaconn_t *coaconn);

rdc_coaconn_db_t *
rdc_coaconn_db_create(uint32_t size)
{
	rdc_coaconn_db_t *db = NULL;
	
	if (0 == size) {
		eag_log_err("rdc_coaconn_db_create input error");
		return NULL;
	}

	db = eag_malloc(sizeof(*db));
	if (NULL == db) {
		eag_log_err("rdc_coaconn_db_create malloc failed");
		goto failed_0;
	}

	memset(db, 0, sizeof(*db));
	if (EAG_RETURN_OK != hashtable_create_table(&(db->htable), size)) {
		eag_log_err("rdc_coaconn_db_create hashtable_create failed");
		goto failed_1;
	}

	if (EAG_RETURN_OK != eag_blkmem_create(&(db->conn_blkmem),
							RDC_COACONN_BLKMEM_NAME,
							sizeof(rdc_coaconn_t),
							RDC_COACONN_BLKMEM_ITEMNUM, MAX_BLK_NUM)) {
		eag_log_err("rdc_coaconn_db_create blkmem_create failed");
		goto failed_2;
	}

	eag_log_debug("coaconn", "coaconn db create ok");
	return db;

failed_2:
	hashtable_destroy_table(&(db->htable));
failed_1:
	eag_free(db);
failed_0:
	return NULL;
}

int
rdc_coaconn_db_destroy(rdc_coaconn_db_t *db)
{
	if (NULL == db) {
		eag_log_err("rdc_coaconn_db_destroy input error");
		return -1;
	}
	
	if (NULL != db->conn_blkmem) {
		eag_blkmem_destroy(&(db->conn_blkmem));
	}
	if (NULL != db->htable) {
		hashtable_destroy_table(&(db->htable));
	}
	eag_free(db);

	eag_log_debug("coaconn", "coaconn db destroy ok");
	return 0;
}

int
rdc_coaconn_db_add(rdc_coaconn_db_t *db,
						rdc_coaconn_t *coaconn)
{
	struct rdc_coaconn_key key = {0};
	char ipstr[32] = "";
	
	if (NULL == db || NULL == coaconn)
	{
		eag_log_err("rdc_coaconn_db_add input error");
		return -1;
	}

	key.ip = coaconn->ip;
	key.port = coaconn->port;
	key.id = coaconn->id;
	hashtable_check_add_node(db->htable, &key, sizeof(key), 
						&(coaconn->hnode));
	db->num++;

	eag_log_debug("coaconn", "coaconn db add coaconn(%s,%u,%u) ok",
		ip2str(key.ip, ipstr, sizeof(ipstr)), 
		key.port, key.id);
	
	return 0;
}

int 
rdc_coaconn_db_del(rdc_coaconn_db_t *db,
						rdc_coaconn_t *coaconn)
{
	char ipstr[32] = "";
	
	if (NULL == db || NULL == coaconn)
	{
		eag_log_err("rdc_coaconn_db_del input error");
		return -1;
	}

	hlist_del(&(coaconn->hnode));
	db->num--;
	
	eag_log_debug("coaconn", "coaconn db del coaconn(%s,%u,%u) ok",
		ip2str(coaconn->ip, ipstr, sizeof(ipstr)), 
		coaconn->port, coaconn->id);
	
	return 0;
}

rdc_coaconn_t *
rdc_coaconn_db_lookup(rdc_coaconn_db_t *db,
							uint32_t ip,
							uint16_t port,
							uint8_t id)
{
	struct rdc_coaconn_key key = {ip, port, id};
	rdc_coaconn_t *coaconn = NULL;
	struct hlist_head *head = NULL;
	struct hlist_node *node = NULL;
	char ipstr[32] = "";
	
	head = hashtable_get_hash_list(db->htable, &key, sizeof(key));
	if (NULL == head) {
		eag_log_warning("coaconn db lookup head null");
		return NULL;
	}

	hlist_for_each_entry(coaconn, node, head, hnode) {
		if (key.ip == coaconn->ip
			&& key.port == coaconn->port
			&& key.id == coaconn->id) {
			eag_log_debug("coaconn", "coaconn db lookup, "
				"find coaconn(%s,%u,%u)",
				ip2str(ip, ipstr, sizeof(ipstr)),
				port, id);
			return coaconn;
		}
	}

	eag_log_debug("coaconn", "coaconn db lookup, not find coaconn(%s,%u,%u)",
				ip2str(ip, ipstr, sizeof(ipstr)),
				port, id);
	
	return NULL;
}

int
rdc_coaconn_db_clear(rdc_coaconn_db_t *db)
{
	hashtable *htable = NULL;
	struct hlist_node *node = NULL;
	struct hlist_node *next = NULL;
	rdc_coaconn_t *coaconn = NULL;
	int i = 0;

	if (NULL == db) {
		eag_log_err("rdc_coaconn_db_clear input error");
		return -1;
	}

	htable = db->htable;
	if (NULL == htable) {
		eag_log_err("rdc_coaconn_db_clear htable null");
		return -1;
	}

	for (i = 0; i < htable->hash_num; i++) {
		hlist_for_each_entry_safe(coaconn, node, next,
				&(htable->head_nodes[i]), hnode) {
			if (NULL != coaconn->t_timeout) {
				eag_thread_cancel(coaconn->t_timeout);
				coaconn->t_timeout = NULL;
			}
			hlist_del(&(coaconn->hnode));
			rdc_coaconn_free(coaconn);
		}
	}
	db->num = 0;
	
	eag_log_debug("coaconn", "coaconn db clear ok");
	return 0;
}

int 
rdc_coaconn_db_set_timeout(rdc_coaconn_db_t *db,
								uint32_t timeout)
{
	if (NULL == db) {
		eag_log_err("rdc_coaconn_db_set_timeout input error");
		return -1;
	}

	db->timeout = timeout;

	return 0;
}

int 
rdc_coaconn_db_set_thread_master(rdc_coaconn_db_t *db,
								eag_thread_master_t *master)
{
	if (NULL == db) {
		eag_log_err("rdc_coaconn_db_set_thread_master input error");
		return -1;
	}

	db->master = master;

	return 0;
}

int
rdc_coaconn_db_log_all(rdc_coaconn_db_t *db)
{
	hashtable *htable = NULL;
	struct hlist_node *node = NULL;
	rdc_coaconn_t *coaconn = NULL;
	int i = 0;
	int num = 0;
	char ipstr[32] = "";
	
	if (NULL == db) {
		eag_log_err("rdc_coaconn_db_log_all input error");
		return -1;
	}

	htable = db->htable;
	if (NULL == htable) {
		eag_log_err("rdc_coaconn_db_log_all htable null");
		return -1;
	}
	eag_log_info("rdc log all packetconn begin");
	for (i = 0; i < htable->hash_num; i++) {
		hlist_for_each_entry(coaconn, node, 
				&(htable->head_nodes[i]), hnode) {
			num++;
			eag_log_info("%-5d packetconn:%s,%u,%u request:%d response:%d",
				num,
				ip2str(coaconn->ip, ipstr, sizeof(ipstr)),
				coaconn->port,
				coaconn->id,
				coaconn->request,
				coaconn->response);
		}
	}

	eag_log_info("rdc log all packetconn end, num: %d", num);
	return 0;
}

rdc_coaconn_t *
rdc_coaconn_new(rdc_coaconn_db_t *coadb,
					uint32_t ip,
					uint16_t port,
					uint8_t id)
{
	rdc_coaconn_t *coaconn = NULL;
	char ipstr[32] = "";
	
	if (NULL == coadb) {
		eag_log_err("rdc_coaconn_new input error");
		return NULL;
	}

	coaconn = eag_blkmem_malloc_item(coadb->conn_blkmem);
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(coaconn, 0, sizeof(*coaconn));
	coaconn->coadb = coadb;
	coaconn->ip = ip;
	coaconn->port = port;
	coaconn->id = id;
	coaconn->master = coadb->master;
	coaconn->timeout = coadb->timeout;

	eag_log_debug("coaconn", "coaconn(%s,%u,%u) new ok",
		ip2str(ip, ipstr, sizeof(ipstr)), 
		port, id);
	
	return coaconn;
}

int
rdc_coaconn_free(rdc_coaconn_t *coaconn)
{
	rdc_coaconn_db_t *coadb = NULL;

	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_free input error");
		return -1;
	}
	coadb = coaconn->coadb;
	if (NULL == coadb) {
		eag_log_err("rdc_coaconn_free db null");
		return -1;
	}
	
	eag_blkmem_free_item(coadb->conn_blkmem, coaconn);

	eag_log_debug("coaconn", "coaconn free ok");
	return 0;
}

static int
coaconn_timeout(eag_thread_t *thread)
{
	rdc_coaconn_t *coaconn = NULL;
	char ipstr[32] = "";
	
	if (NULL == thread) {
		eag_log_err("coaconn_timeout input error");
		return -1; 
	}
	
	coaconn = eag_thread_get_arg(thread);	
	if (NULL == coaconn) {
		eag_log_err("coaconn_timeout coaconn null");
		return -1;
	}
	eag_log_debug("coaconn", "coaconn(%s,%u,%u) timeout",
				ip2str(coaconn->ip, ipstr, sizeof(ipstr)),
				coaconn->port, coaconn->id);
	if (NULL != coaconn->t_timeout) {
		eag_thread_cancel(coaconn->t_timeout);
		coaconn->t_timeout = NULL;
	}

#if 0
	if (NULL != coaconn->server) {
		//rdc_sockclient_id_wait(coaconn->server, coaconn->id);
	}
	else {
		eag_log_warning("coaconn timeout sockclient null");
	}
#endif	
	if (NULL != coaconn->coadb) {
		rdc_coaconn_db_del(coaconn->coadb, coaconn);
	}
	else {
		eag_log_warning("coaconn timeout db null");
	}
	
	rdc_coaconn_free(coaconn);
	
	return 0;
}

int
rdc_coaconn_wait(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_wait input error");
		return -1;
	}
	rdc_coaconn_event(RDC_COACONN_TIMEOUT, coaconn);

	return 0;
}

uint32_t 
rdc_coaconn_get_ip(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_ip input error");
		return 0;
	}

	return coaconn->ip;
}

uint16_t 
rdc_coaconn_get_port(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_port input error");
		return 0;
	}

	return coaconn->port;
}


uint32_t 
rdc_coaconn_get_coaip(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_ip input error");
		return 0;
	}

	return coaconn->coaip;
}

uint16_t 
rdc_coaconn_get_coaport(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_port input error");
		return 0;
	}

	return coaconn->coaport;
}



int
rdc_coaconn_set_coaip(rdc_coaconn_t *coaconn, uint32_t coaip)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_ip input error");
		return -1;
	}
	
	coaconn->coaip = coaip;
	return 0;
}

int 
rdc_coaconn_set_coaport(rdc_coaconn_t *coaconn, uint16_t coaport)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_port input error");
		return -1;
	}

	coaconn->coaport = coaport;
	return 0;
}


uint8_t 
rdc_coaconn_get_id(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_id input error");
		return 0;
	}

	return coaconn->id;
}

int 
rdc_coaconn_get_timeout(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_timeout input error");
		return 0;
	}

	return coaconn->timeout;
}

int
rdc_coaconn_set_server(rdc_coaconn_t *coaconn,
								rdc_server_t *server)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_set_sockclient input error");
		return -1;
	}

	coaconn->server = server;

	return 0;

}

rdc_server_t *
rdc_coaconn_get_server(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_get_sockclient input error");
		return NULL;
	}

	return coaconn->server;
}

int
rdc_coaconn_inc_req(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_inc_req input error");
		return -1;
	}

	coaconn->request++;
	eag_log_debug("coaconn","rdc_coaconn_inc_req request=%d",
					coaconn->request);
	return 0;
}

int
rdc_coaconn_inc_res(rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_inc_res input error");
		return -1;
	}

	coaconn->response++;

	return 0;
}

static void
rdc_coaconn_event(rdc_coaconn_event_t event,
						rdc_coaconn_t *coaconn)
{
	if (NULL == coaconn) {
		eag_log_err("rdc_coaconn_event input error");
		return;
	}
	
	switch (event) {
	case RDC_COACONN_TIMEOUT:
		if( NULL != coaconn->t_timeout ){
			eag_thread_cancel(coaconn->t_timeout);
			coaconn->t_timeout = NULL;
		}
		coaconn->t_timeout =
		    eag_thread_add_timer(coaconn->master, coaconn_timeout,
					coaconn, coaconn->timeout);
		break;
	default:
		break;
	}
}

