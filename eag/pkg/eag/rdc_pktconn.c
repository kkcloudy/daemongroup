/* rdc_pktconn.c */

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
#include "rdc_pktconn.h"

#define RDC_PKTCONN_BLKMEM_NAME 		"rdc_pktconn_blkmem"
#define RDC_PKTCONN_BLKMEM_ITEMNUM		1024

struct rdc_pktconn_db {
	int num;
	hashtable *htable;
	eag_blk_mem_t *conn_blkmem;
	eag_thread_master_t *master;
	int timeout;
};

struct rdc_pktconn {
	struct hlist_node hnode;
	rdc_pktconn_db_t *db;
	rdc_sockclient_t *sockclient;
	uint32_t ip;
	uint16_t port;
	uint8_t id;
	eag_thread_master_t *master;
	eag_thread_t *t_timeout;
	int timeout;
	int request;
	int response;
};

typedef enum {
	RDC_PKTCONN_TIMEOUT,
} rdc_pktconn_event_t;

struct rdc_pktconn_key {
	uint32_t ip;
	uint16_t port;
	uint8_t id;
} __attribute__ ((packed));

static void
rdc_pktconn_event(rdc_pktconn_event_t event,
						rdc_pktconn_t *pktconn);

rdc_pktconn_db_t *
rdc_pktconn_db_create(uint32_t size)
{
	rdc_pktconn_db_t *db = NULL;
	
	if (0 == size) {
		eag_log_err("rdc_pktconn_db_create input error");
		return NULL;
	}

	db = eag_malloc(sizeof(*db));
	if (NULL == db) {
		eag_log_err("rdc_pktconn_db_create malloc failed");
		goto failed_0;
	}

	memset(db, 0, sizeof(*db));
	if (EAG_RETURN_OK != hashtable_create_table(&(db->htable), size)) {
		eag_log_err("rdc_pktconn_db_create hashtable_create failed");
		goto failed_1;
	}

	if (EAG_RETURN_OK != eag_blkmem_create(&(db->conn_blkmem),
							RDC_PKTCONN_BLKMEM_NAME,
							sizeof(rdc_pktconn_t),
							RDC_PKTCONN_BLKMEM_ITEMNUM, MAX_BLK_NUM)) {
		eag_log_err("rdc_pktconn_db_create blkmem_create failed");
		goto failed_2;
	}

	eag_log_debug("pktconn", "pktconn db create ok");
	return db;

failed_2:
	hashtable_destroy_table(&(db->htable));
failed_1:
	eag_free(db);
failed_0:
	return NULL;
}

int
rdc_pktconn_db_destroy(rdc_pktconn_db_t *db)
{
	if (NULL == db) {
		eag_log_err("rdc_pktconn_db_destroy input error");
		return -1;
	}
	
	if (NULL != db->conn_blkmem) {
		eag_blkmem_destroy(&(db->conn_blkmem));
	}
	if (NULL != db->htable) {
		hashtable_destroy_table(&(db->htable));
	}
	eag_free(db);

	eag_log_debug("pktconn", "pktconn db destroy ok");
	return 0;
}

int
rdc_pktconn_db_add(rdc_pktconn_db_t *db,
						rdc_pktconn_t *pktconn)
{
	struct rdc_pktconn_key key = {0};
	char ipstr[32] = "";
	
	if (NULL == db || NULL == pktconn)
	{
		eag_log_err("rdc_pktconn_db_add input error");
		return -1;
	}

	key.ip = pktconn->ip;
	key.port = pktconn->port;
	key.id = pktconn->id;
	hashtable_check_add_node(db->htable, &key, sizeof(key), 
						&(pktconn->hnode));
	db->num++;

	eag_log_debug("pktconn", "pktconn db add pktconn(%s,%u,%u) ok",
		ip2str(key.ip, ipstr, sizeof(ipstr)), 
		key.port, key.id);
	
	return 0;
}

int 
rdc_pktconn_db_del(rdc_pktconn_db_t *db,
						rdc_pktconn_t *pktconn)
{
	char ipstr[32] = "";
	
	if (NULL == db || NULL == pktconn)
	{
		eag_log_err("rdc_pktconn_db_del input error");
		return -1;
	}

	hlist_del(&(pktconn->hnode));
	db->num--;
	
	eag_log_debug("pktconn", "pktconn db del pktconn(%s,%u,%u) ok",
		ip2str(pktconn->ip, ipstr, sizeof(ipstr)), 
		pktconn->port, pktconn->id);
	
	return 0;
}

rdc_pktconn_t *
rdc_pktconn_db_lookup(rdc_pktconn_db_t *db,
							uint32_t ip,
							uint16_t port,
							uint8_t id)
{
	struct rdc_pktconn_key key = {ip, port, id};
	rdc_pktconn_t *pktconn = NULL;
	struct hlist_head *head = NULL;
	struct hlist_node *node = NULL;
	char ipstr[32] = "";
	
	head = hashtable_get_hash_list(db->htable, &key, sizeof(key));
	if (NULL == head) {
		eag_log_warning("pktconn db lookup head null");
		return NULL;
	}

	hlist_for_each_entry(pktconn, node, head, hnode) {
		if (key.ip == pktconn->ip
			&& key.port == pktconn->port
			&& key.id == pktconn->id) {
			eag_log_debug("pktconn", "pktconn db lookup, "
				"find pktconn(%s,%u,%u)",
				ip2str(ip, ipstr, sizeof(ipstr)),
				port, id);
			return pktconn;
		}
	}

	eag_log_debug("pktconn", "pktconn db lookup, not find pktconn(%s,%u,%u)",
				ip2str(ip, ipstr, sizeof(ipstr)),
				port, id);
	
	return NULL;
}

int
rdc_pktconn_db_clear(rdc_pktconn_db_t *db)
{
	hashtable *htable = NULL;
	struct hlist_node *node = NULL;
	struct hlist_node *next = NULL;
	rdc_pktconn_t *pktconn = NULL;
	int i = 0;

	if (NULL == db) {
		eag_log_err("rdc_pktconn_db_clear input error");
		return -1;
	}

	htable = db->htable;
	if (NULL == htable) {
		eag_log_err("rdc_pktconn_db_clear htable null");
		return -1;
	}

	for (i = 0; i < htable->hash_num; i++) {
		hlist_for_each_entry_safe(pktconn, node, next,
				&(htable->head_nodes[i]), hnode) {
			if (NULL != pktconn->t_timeout) {
				eag_thread_cancel(pktconn->t_timeout);
				pktconn->t_timeout = NULL;
			}
			hlist_del(&(pktconn->hnode));
			rdc_pktconn_free(pktconn);
		}
	}
	db->num = 0;
	
	eag_log_debug("pktconn", "pktconn db clear ok");
	return 0;
}

int 
rdc_pktconn_db_set_timeout(rdc_pktconn_db_t *db,
								uint32_t timeout)
{
	if (NULL == db) {
		eag_log_err("rdc_pktconn_db_set_timeout input error");
		return -1;
	}

	db->timeout = timeout;

	return 0;
}

int 
rdc_pktconn_db_set_thread_master(rdc_pktconn_db_t *db,
								eag_thread_master_t *master)
{
	if (NULL == db) {
		eag_log_err("rdc_pktconn_db_set_thread_master input error");
		return -1;
	}

	db->master = master;

	return 0;
}

int
rdc_pktconn_db_log_all(rdc_pktconn_db_t *db)
{
	hashtable *htable = NULL;
	struct hlist_node *node = NULL;
	rdc_pktconn_t *pktconn = NULL;
	int i = 0;
	int num = 0;
	char ipstr[32] = "";
	
	if (NULL == db) {
		eag_log_err("rdc_pktconn_db_log_all input error");
		return -1;
	}

	htable = db->htable;
	if (NULL == htable) {
		eag_log_err("rdc_pktconn_db_log_all htable null");
		return -1;
	}
	eag_log_info("rdc log all packetconn begin");
	for (i = 0; i < htable->hash_num; i++) {
		hlist_for_each_entry(pktconn, node, 
				&(htable->head_nodes[i]), hnode) {
			num++;
			eag_log_info("%-5d packetconn:%s,%u,%u request:%d response:%d",
				num,
				ip2str(pktconn->ip, ipstr, sizeof(ipstr)),
				pktconn->port,
				pktconn->id,
				pktconn->request,
				pktconn->response);
		}
	}

	eag_log_info("rdc log all packetconn end, num: %d", num);
	return 0;
}

rdc_pktconn_t *
rdc_pktconn_new(rdc_pktconn_db_t *db,
					uint32_t ip,
					uint16_t port,
					uint8_t id)
{
	rdc_pktconn_t *pktconn = NULL;
	char ipstr[32] = "";
	
	if (NULL == db) {
		eag_log_err("rdc_pktconn_new input error");
		return NULL;
	}

	pktconn = eag_blkmem_malloc_item(db->conn_blkmem);
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(pktconn, 0, sizeof(*pktconn));
	pktconn->db = db;
	pktconn->ip = ip;
	pktconn->port = port;
	pktconn->id = id;
	pktconn->master = db->master;
	pktconn->timeout = db->timeout;

	eag_log_debug("pktconn", "pktconn(%s,%u,%u) new ok",
		ip2str(ip, ipstr, sizeof(ipstr)), 
		port, id);
	
	return pktconn;
}

int
rdc_pktconn_free(rdc_pktconn_t *pktconn)
{
	rdc_pktconn_db_t *db = NULL;

	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_free input error");
		return -1;
	}
	db = pktconn->db;
	if (NULL == db) {
		eag_log_err("rdc_pktconn_free db null");
		return -1;
	}
	
	eag_blkmem_free_item(db->conn_blkmem, pktconn);

	eag_log_debug("pktconn", "pktconn free ok");
	return 0;
}

static int
pktconn_timeout(eag_thread_t *thread)
{
	rdc_pktconn_t *pktconn = NULL;
	char ipstr[32] = "";
	
	if (NULL == thread) {
		eag_log_err("pktconn_timeout input error");
		return -1; 
	}
	
	pktconn = eag_thread_get_arg(thread);	
	if (NULL == pktconn) {
		eag_log_err("pktconn_timeout pktconn null");
		return -1;
	}
	eag_log_debug("pktconn", "pktconn(%s,%u,%u) timeout",
				ip2str(pktconn->ip, ipstr, sizeof(ipstr)),
				pktconn->port, pktconn->id);
	if (NULL != pktconn->t_timeout) {
		eag_thread_cancel(pktconn->t_timeout);
		pktconn->t_timeout = NULL;
	}

	if (NULL != pktconn->sockclient) {
		rdc_sockclient_id_wait(pktconn->sockclient, pktconn->id);
	}
	else {
		eag_log_warning("pktconn timeout sockclient null");
	}
	if (NULL != pktconn->db) {
		rdc_pktconn_db_del(pktconn->db, pktconn);
	}
	else {
		eag_log_warning("pktconn timeout db null");
	}
	
	rdc_pktconn_free(pktconn);
	
	return 0;
}

int
rdc_pktconn_wait(rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_wait input error");
		return -1;
	}
	rdc_pktconn_event(RDC_PKTCONN_TIMEOUT, pktconn);

	return 0;
}

uint32_t 
rdc_pktconn_get_ip(rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_get_ip input error");
		return 0;
	}

	return pktconn->ip;
}

uint16_t 
rdc_pktconn_get_port(rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_get_port input error");
		return 0;
	}

	return pktconn->port;
}

uint8_t 
rdc_pktconn_get_id(rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_get_id input error");
		return 0;
	}

	return pktconn->id;
}

int 
rdc_pktconn_get_timeout(rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_get_timeout input error");
		return 0;
	}

	return pktconn->timeout;
}

int
rdc_pktconn_set_sockclient(rdc_pktconn_t *pktconn,
								rdc_sockclient_t *sockclient)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_set_sockclient input error");
		return -1;
	}

	pktconn->sockclient = sockclient;

	return 0;

}

rdc_sockclient_t *
rdc_pktconn_get_sockclient(rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_get_sockclient input error");
		return NULL;
	}

	return pktconn->sockclient;
}

int
rdc_pktconn_inc_req(rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_inc_req input error");
		return -1;
	}

	pktconn->request++;

	return 0;
}

int
rdc_pktconn_inc_res(rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_inc_res input error");
		return -1;
	}

	pktconn->response++;

	return 0;
}

static void
rdc_pktconn_event(rdc_pktconn_event_t event,
						rdc_pktconn_t *pktconn)
{
	if (NULL == pktconn) {
		eag_log_err("rdc_pktconn_event input error");
		return;
	}
	
	switch (event) {
	case RDC_PKTCONN_TIMEOUT:
		pktconn->t_timeout =
		    eag_thread_add_timer(pktconn->master, pktconn_timeout,
					pktconn, pktconn->timeout);
		break;
	default:
		break;
	}
}

