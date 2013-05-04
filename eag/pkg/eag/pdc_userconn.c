/* pdc_userconn.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eag_errcode.h"
#include "nm_list.h"
#include "eag_log.h"
#include "eag_mem.h"
#include "eag_blkmem.h"
#include "limits2.h"
#include "eag_util.h"
#include "hashtable.h"


#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <error.h>
#include <mcheck.h>

#include "pdc_interface.h"
#include "eag_dbus.h"

#include "eag_time.h"
#include "pdc_ins.h"
#include "pdc_userconn.h"


#define PDC_USERCONN_BLKMEM_SIZE		(512)
#define PDC_USERCONN_HASH_SIZE			(8192)



struct pdc_userconn_db {
	int num;
	struct list_head head;
	hashtable *ht;
	eag_blk_mem_t *userblkmem;
};

struct pdc_userconn {
	struct list_head node;
	struct hlist_node hnode;
	pdc_userconn_db_t *db;
	uint32_t user_ip;
	uint8_t  user_mac[6];
	uint8_t slot_id;
	uint8_t hansi_type;
	uint8_t hansi_id;
	uint32_t eag_ip;
	uint16_t eag_port;
	uint32_t start_time;
	uint32_t last_time;
};

#define eag_malloc malloc
#define eag_free free

pdc_userconn_db_t * 
pdc_userconn_db_create()
{
	pdc_userconn_db_t *new_db = NULL;
	int iret = EAG_ERR_UNKNOWN;

	new_db = (pdc_userconn_db_t *) eag_malloc(sizeof (pdc_userconn_db_t));
	if (NULL == new_db) {
		eag_log_err("pdc_userconn_db_create malloc error!\n");
		return NULL;
	}
	memset(new_db, 0, sizeof (pdc_userconn_db_t));
	INIT_LIST_HEAD(&(new_db->head));

	iret = eag_blkmem_create(&(new_db->userblkmem),
				 "pdc_userconn_db_blk", sizeof (struct pdc_userconn),
				 PDC_USERCONN_BLKMEM_SIZE, MAX_BLK_NUM);
	if (EAG_RETURN_OK != iret) {
		eag_log_err("pdc_userconn_db create blkmem failed!");
		eag_free(new_db);
		return NULL;
	}

	iret = hashtable_create_table(&(new_db->ht), PDC_USERCONN_HASH_SIZE);
	if (EAG_RETURN_OK != iret) {
		eag_log_err("pdc_userconn_db create hashtable failed!");
		eag_blkmem_destroy(&(new_db->userblkmem));
		eag_free(new_db);
		return NULL;
	}

	return new_db;
}

int
pdc_userconn_db_destroy(pdc_userconn_db_t *db)
{

	if (NULL == db) {
		eag_log_err("pdc_userconn_db_destroy error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (NULL != db->ht) {
		hashtable_destroy_table(&(db->ht));
	}
	
	if (NULL != db->userblkmem) {
		eag_blkmem_destroy(&(db->userblkmem));
	}
	eag_free(db);
	return 0;
}


int
pdc_userconn_db_clear(pdc_userconn_db_t *userdb)
{
	pdc_userconn_t *userconn=NULL;
	pdc_userconn_t *n=NULL;
	
	if (NULL == userdb) {
		eag_log_err("pdc_userconn_db_clear error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	list_for_each_entry_safe(userconn, n, &(userdb->head), node){
		pdc_userconn_db_del (userdb, userconn);
		pdc_userconn_free (userdb, userconn);
	}
	return 0;
}


int
pdc_userconn_db_add(pdc_userconn_db_t *db,
						pdc_userconn_t *userconn)
{	
	char ip_str[32];

	if( NULL == db || NULL == userconn ){
		eag_log_err("pdc_userconn_db_add input param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
		
	list_add_tail(&(userconn->node), &(db->head));
	hashtable_add_node( db->ht, &(userconn->user_ip),
					sizeof(userconn->user_ip), &(userconn->hnode) );
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn","pdc_userconn_db_add  ip=%s slotid=%u hansi_id=%u", 
						ip_str, userconn->slot_id, userconn->hansi_id );
	return EAG_RETURN_OK;
}

int 
pdc_userconn_db_del(pdc_userconn_db_t *db,
						pdc_userconn_t *userconn)
{
	char ip_str[32];
	if( NULL == db || NULL == userconn ){
		eag_log_err("pdc_userconn_db_del input param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn","pdc_userconn_db_del ip=%s slot_id=%u hansi_id=%u ", 
						ip_str, userconn->slot_id, userconn->hansi_id );
	hashtable_del_node( &(userconn->hnode) );
	list_del(&(userconn->node));
	return EAG_RETURN_OK;
}


pdc_userconn_t *
pdc_userconn_db_find_user(pdc_userconn_db_t *db,
							uint32_t ip )
{	
	char ip_str[32];
	pdc_userconn_t *userconn = NULL;
	struct hlist_head *hhead;
	struct hlist_node *hash_node = NULL;

	ip2str( ip, ip_str, sizeof(ip_str));
	hhead = hashtable_get_hash_list( db->ht, &ip, sizeof(ip) );
	if( NULL != hhead ){
		hlist_for_each_entry(userconn,hash_node,hhead,hnode){
			if( userconn->user_ip == ip ){
				eag_log_debug("userconn","pdc_userconn_db_find_user success "\
								" ip=%s",  ip_str );
				return userconn;
			}
		}
	}
	
	eag_log_debug("userconn","pdc_userconn_db_find_user failed "\
					" ip=%s",  ip_str );
	return NULL;
}


pdc_userconn_t *
pdc_userconn_new(pdc_userconn_db_t * db, uint32_t userip )
{
	char ip_str[32];
	pdc_userconn_t *userconn = eag_blkmem_malloc_item(db->userblkmem);
	if (NULL == userconn) {
		eag_log_err("pdc_new_userconn return NULL");
		return NULL;
	}
	userconn->db = db;
	userconn->user_ip = userip;
	
	ip2str( userip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn", "pdc_userconn_new ret=%p ip=%s",
					userconn, ip_str );
	return userconn;
}

int
pdc_userconn_set_hansi( pdc_userconn_t *userconn, uint8_t slot_id,
							uint8_t hansi_type, uint8_t hansi_id )
{
	char ip_str[32];

	if( NULL==userconn || 0==slot_id || 0==hansi_id ){
		eag_log_err("pdc_userconn_set_hansi input error userconn=%p slot_id=%u hansi_id=%u",
					userconn, slot_id, hansi_id );
		return EAG_ERR_NULL_POINTER;
	}

	userconn->slot_id = slot_id;
	userconn->hansi_type = hansi_type;
	userconn->hansi_id = hansi_id;


	if( HANSI_LOCAL == hansi_type ){
		userconn->eag_ip = SLOT_IPV4_BASE + 100 + hansi_id;
		userconn->eag_port = EAG_PORTAL_PORT_BASE + hansi_id;
	}else{
		userconn->eag_ip = SLOT_IPV4_BASE + slot_id;
		userconn->eag_port = EAG_PORTAL_PORT_BASE + MAX_HANSI_ID + hansi_id;
	}

	ip2str( userconn->user_ip, ip_str, sizeof(ip_str) );
	eag_log_debug("userconn", "pdc_userconn_set_hansi set %s hansi--%u:%u",
					ip_str, slot_id, hansi_id );
	return EAG_RETURN_OK;
}


int
pdc_userconn_get_hansi( pdc_userconn_t *userconn, uint8_t *slot_id,
							uint8_t *hansi_id )
{
	char ip_str[32];
	if( NULL == userconn || NULL == slot_id || NULL == hansi_id ){
		eag_log_err("pdc_userconn_get_client userconn=%p slot_id=%p "\
					"hansi_id=%p", userconn, slot_id, hansi_id );
		return EAG_ERR_NULL_POINTER;
	}

	*slot_id = userconn->slot_id;
	*hansi_id = userconn->hansi_id;
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn", "pdc_userconn_get_client %s hansi--%u:%u",
						ip_str, userconn->slot_id, userconn->hansi_id );
	return EAG_RETURN_OK;
}

int
pdc_userconn_get_eaginfo( pdc_userconn_t *userconn, 
								uint32_t *eagip, uint16_t *eagport )
{
	char ip_str[32];
	char eagip_str[32];
	if( NULL == userconn || NULL == eagip || NULL == eagport ){
		eag_log_err("pdc_userconn_get_eaginfo userconn=%p peagip=%p peagport=%p",
						userconn, eagip, eagport);
		return EAG_ERR_NULL_POINTER;
	}

	*eagip = userconn->eag_ip;
	*eagport = userconn->eag_port;
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	ip2str( userconn->eag_ip, eagip_str, sizeof(eagip_str));
	
	eag_log_debug("userconn", "pdc_userconn_get_eaginfo %s hansi--%u:%u  eag:--%s:%u",
						ip_str, userconn->slot_id, userconn->hansi_id,
						eagip_str, userconn->eag_port );	
	return EAG_RETURN_OK;
}

int
pdc_userconn_set_start_time( pdc_userconn_t *userconn, uint32_t start_time )
{
	char ip_str[32];
	if( NULL == userconn ){
		eag_log_err("pdc_userconn_set_start_time userconn is NULL");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	userconn->start_time = start_time;
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn","pdc_userconn_set_start_time %s %ul",
					ip_str, start_time );
	return EAG_RETURN_OK;
}

int
pdc_userconn_set_last_time( pdc_userconn_t *userconn, uint32_t last_time )
{
	char ip_str[32];
	if( NULL == userconn ){
		eag_log_err("pdc_userconn_set_last_time userconn is NULL");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	userconn->last_time = last_time;
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn","pdc_userconn_set_last_time %s %ul",
					ip_str, last_time );
	return EAG_RETURN_OK;
}

int
pdc_userconn_set_usermac( pdc_userconn_t *userconn, uint8_t *mac )
{
	if( NULL == userconn || NULL == mac ){
		eag_log_err("pdc_userconn_set_usermac usercon=%p mac=%p", userconn, mac );
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	memcpy( userconn->user_mac, mac, sizeof(userconn->user_mac) );
	return EAG_RETURN_OK;
}

int
pdc_userconn_get_usermac( pdc_userconn_t *userconn, uint8_t *mac )
{
	if( NULL == userconn || NULL == mac ){
		eag_log_err("pdc_userconn_get_usermac userconn=%p mac=%p", userconn, mac );
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	memcpy( mac, userconn->user_mac, sizeof(userconn->user_mac) );

	return EAG_RETURN_OK;
}

int
pdc_userconn_free(pdc_userconn_db_t * db, pdc_userconn_t *userconn)
{
	char ip_str[32];
	if (NULL == db || NULL == db->userblkmem || NULL == userconn) {
		eag_log_err("pdc_userconn_free error db=%p blkmem=%p userconn=%p",
						db, (db==NULL)?NULL:db->userblkmem, userconn);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn", "pdc_userconn_free %p  %s", userconn, ip_str );
	return eag_blkmem_free_item(db->userblkmem, userconn);
}

int
pdc_userconn_db_log_all( pdc_userconn_db_t *userdb )
{	
	char ip_str[32];
	pdc_userconn_t *userconn = NULL;

	eag_log_info("pdc_userconn_db_log_all begin");
	list_for_each_entry(userconn, &(userdb->head), node){
		ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
		eag_log_info( "pdc_userconn "\
						" ip=%s hans:%u-%u start_time=%u last_time=%u", 
						ip_str, userconn->slot_id, userconn->hansi_id,
						userconn->start_time, userconn->last_time );
	}
	eag_log_info("pdc_userconn_db_log_all end");	
	return 0;
}


int
pdc_userconn_db_check_timeout( pdc_userconn_db_t *userdb, 
				uint32_t timeout )
{
	char ip_str[32]="";
	pdc_userconn_t *userconn = NULL;
	pdc_userconn_t *next = NULL;
	uint32_t timenow = 0;
	struct timeval tv;

	if( NULL == userdb ){
		eag_log_err("pdc_userconn_db_check_timeout input userdb is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	eag_time_gettimeofday( &tv, NULL);
	timenow = tv.tv_sec;
	
	list_for_each_entry_safe(userconn, next, &(userdb->head), node){
		if( timenow < userconn->last_time ){
			eag_log_err("pdc check userconn time error last(%ul) > timenow(%ul)",
						userconn->last_time, timenow);
			userconn->last_time = timenow;
			continue;
		} 
		if( timenow - userconn->last_time > timeout ){
			eag_log_err("pdc del userconn not get stop: %s last:%ul now:%ul", 
						ip_str,	userconn->last_time, timenow );		
			pdc_userconn_db_del( userdb, userconn );
			pdc_userconn_free( userdb, userconn );
		}
	}
	return EAG_RETURN_OK;
}



DBusMessage *
pdc_dbus_method_set_userconn(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_userconn_db_t *userdb = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = EAG_RETURN_OK;
	uint8_t eag_slotid = 0;
	uint8_t eag_hansitype = 0;
	uint8_t eag_hansiid = 0;
	uint32_t userip = 0;
	pdc_userconn_t *userconn = NULL;
	
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_set_userconn "
					"DBUS new reply message error!");
		return NULL;
	}

	userdb = (pdc_userconn_db_t *)user_data;
	if (NULL == userdb) {
		eag_log_err("pdc_dbus_method_set_userconn user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_BYTE, &eag_slotid,
								DBUS_TYPE_BYTE, &eag_hansitype,
								DBUS_TYPE_BYTE, &eag_hansiid,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_set_userconn "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_set_userconn %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	userconn = pdc_userconn_db_find_user( userdb, userip );
	if( NULL == userconn ){
		struct timeval tv;
		userconn = pdc_userconn_new( userdb, userip );
		if( NULL == userconn ){
			eag_log_err("pdc_dbus_method_set_userconn pdc_userconn_new failed!");
			ret = EAG_ERR_MALLOC_FAILED;
			goto replyx;
		}
		eag_time_gettimeofday( &tv, NULL );
		pdc_userconn_set_start_time( userconn, tv.tv_sec );
		pdc_userconn_set_last_time( userconn, tv.tv_sec );
		pdc_userconn_db_add( userdb, userconn );
	}
	
	ret = pdc_userconn_set_hansi(userconn, eag_slotid, eag_hansitype, eag_hansiid);
	if( EAG_RETURN_OK != ret ){
		pdc_userconn_db_del( userdb, userconn );
		pdc_userconn_free( userdb, userconn );
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	return reply;
	
}



DBusMessage *
pdc_dbus_method_log_userconn(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_userconn_db_t *userdb = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = EAG_RETURN_OK;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_log_userconn "
					"DBUS new reply message error!");
		return NULL;
	}

	userdb = (pdc_userconn_db_t *)user_data;
	if (NULL == userdb) {
		eag_log_err("pdc_dbus_method_log_userconn user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);

	pdc_userconn_db_log_all(userdb);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	
	return reply;
	
}

#ifdef pdc_userconn_test

#include "eag_errcode.c"
#include "eag_log.c"
#include "eag_mem.c"
#include "eag_blkmem.c"
#include "eag_util.c"
#include "eag_time.c"
#include "hashtable.c"

int main(void)
{
	printf("start\n");
	eag_log_init("userconn");
	setenv("MALLOC_TRACE", "log123", 1);
	mtrace();
	
//	int ret = -1;
	pdc_userconn_db_t *db = NULL;
	pdc_userconn_t *findconn;

	malloc(123);

	db = pdc_userconn_db_create();
	if (NULL == db){
		printf("pdc_userconn_db_create err\n");
	};
	
	struct in_addr addr1;
	struct in_addr addr1_ser;
	char *userip1 = "11.22.33.44";
	char *serip1 = "11.12.33.55";
	inet_aton(userip1, &addr1 );
	inet_aton(serip1, &addr1_ser );

	pdc_userconn_t *userconn1 = 
			pdc_userconn_new(db,ntohl(addr1.s_addr));
	pdc_userconn_set_hansi( userconn1, 1, 2);
	pdc_userconn_db_add(db, userconn1);


	struct in_addr addr2;
	struct in_addr addr2_ser;
	char *userip2 = "123.222.33.44";
	char *serip2 = "123.132.33.55";
	inet_aton(userip2, &addr2 );
	inet_aton(serip2, &addr2_ser );

	pdc_userconn_t *userconn2 = 
			pdc_userconn_new(db, ntohl(addr2.s_addr));
	pdc_userconn_set_hansi( userconn2, 3, 4);
	pdc_userconn_db_add(db, userconn2);

	
	findconn = pdc_userconn_db_find_user( db, ntohl(addr1.s_addr) );
	if( findconn != userconn1 ){
		printf("find userconn1 failed!");
	}

	findconn = pdc_userconn_db_find_user( db, ntohl(addr2.s_addr) );
	if( findconn != userconn2 ){
		printf("find userconn2 failed!");
	}

	pdc_userconn_db_log_all(db);

    pdc_userconn_db_del(db, userconn2);
	findconn = pdc_userconn_db_find_user(db,ntohl(addr2.s_addr) );
	if( findconn != NULL ){
		printf("userconn2 del failed!");
	}
	pdc_userconn_free(db,userconn2);


	pdc_userconn_db_del(db, userconn1);
	findconn = pdc_userconn_db_find_user(db,ntohl(addr1.s_addr) );
	if( findconn != NULL ){
		printf("userconn1 del failed!");
	}

	
	pdc_userconn_free(db,userconn1);
	

	if (EAG_RETURN_OK != pdc_userconn_db_destroy(db)){
		printf("pdc_userconn_db_destroy err\n");
	}
	
	muntrace();
	eag_log_uninit();
	printf("end\n");
	return 0;
}

#endif

