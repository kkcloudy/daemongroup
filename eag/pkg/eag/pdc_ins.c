/* pdc_ins.c */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "nm_list.h"
#include "hashtable.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_blkmem.h"
#include "eag_thread.h"
#include "pdc_interface.h"
#include "eag_dbus.h"
#include "eag_hansi.h"
#include "eag_util.h"


#include "pdc_ins.h"
#include "pdc_server.h"
#include "pdc_client.h"
#include "pdc_userconn.h"
#include "pdc_usermap.h"
#include "portal_packet.h"

#define PDC_DBUS_NAME_FMT		"aw.pdc_%s_%d"
#define PDC_DBUS_OBJPATH_FMT	"/aw/pdc_%s_%d"
#define PDC_DBUS_INTERFACE_FMT	"aw.pdc_%s_%d"

#define SLOT_ID_FILE			"/dbm/local_board/slot_id"
#define IS_DISTRIBUTED_FILE		"/dbm/product/is_distributed"

struct pdc_ins {
	int status;
	pdc_server_t *server;
	pdc_client_t *client;
	eag_thread_master_t *master;
	eag_dbus_t *eagdbus;
	eag_hansi_t *eaghansi;
	pdc_userconn_db_t *userdb;
	pdc_usermap_t *usermap;
	uint8_t slot_id;
	uint8_t hansi_type;
	uint8_t hansi_id;
	int is_distributed;
};

static char PDC_DBUS_NAME[MAX_DBUS_BUSNAME_LEN]="";
static char PDC_DBUS_OBJPATH[MAX_DBUS_BUSNAME_LEN]="";
static char PDC_DBUS_INTERFACE[MAX_DBUS_BUSNAME_LEN]="";


#define PDC_HANSI_BACKUP_PORT	2003

static void
pdc_register_all_dbus_method(pdc_ins_t *pdcins);

void 
pdc_dbus_path_init(int type,int insid)
{
	snprintf(PDC_DBUS_NAME, sizeof(PDC_DBUS_NAME),
				PDC_DBUS_NAME_FMT, 
				(HANSI_LOCAL==type)?"l":"r",insid);
	snprintf(PDC_DBUS_OBJPATH, sizeof(PDC_DBUS_OBJPATH),
				PDC_DBUS_OBJPATH_FMT,
				(HANSI_LOCAL==type)?"l":"r",insid);
	snprintf(PDC_DBUS_INTERFACE, sizeof(PDC_DBUS_INTERFACE),
				PDC_DBUS_INTERFACE_FMT,
				(HANSI_LOCAL==type)?"l":"r",insid);
	return;
}

int
pdc_packet_minsize(void)
{
	int protocol_type = PORTAL_PROTOCOL_MOBILE;

	protocol_type = portal_get_protocol_type();
	if (PORTAL_PROTOCOL_MOBILE == protocol_type) {
		return PDC_PACKET_HEADSIZE + PORTAL_PACKET_MOBILE_HEADSIZE;
	}
	
	return PDC_PACKET_HEADSIZE + PORTAL_PACKET_TELECOM_HEADSIZE;
}


static int
pdc_vrrp_status_switch_proc(eag_hansi_t *eaghansi,  void *cbp)
{
	int ret;
	pdc_ins_t *pdcins = (pdc_ins_t *)cbp;
	
	if( NULL == eaghansi || NULL==pdcins ){
		eag_log_err("pdc_vrrp_status_switch_proc param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( 1 != pdcins->status ){
		eag_log_info("pdc_vrrp_status_switch_proc get status change but pdc not start!");
		return EAG_RETURN_OK;
	}
	pdc_server_stop(pdcins->server);
	pdc_client_stop(pdcins->client);
	if( eag_hansi_is_master(eaghansi) ){
		if ( (ret = pdc_server_start(pdcins->server)) != 0) {
			return ret;
		}
		if ( (ret = pdc_client_start(pdcins->client)) != 0) {
			pdc_server_stop(pdcins->server);
			return ret;
		}			
	}
/*
	if(eag_hansi_is_enable(eaghansi)){
		if( eag_hansi_is_master(eaghansi) ){
			if ( (ret = pdc_server_start(pdcins->server)) != 0) {
				return ret;
			}
			if ( (ret = pdc_client_start(pdcins->client)) != 0) {
				pdc_server_stop(pdcins->server);
				return ret;
			}			
		}else{
			pdc_server_stop(pdcins->server);
			pdc_client_stop(pdcins->client);
		}
	}else{
		pdc_server_stop(pdcins->server);
		pdc_client_stop(pdcins->client);
	}
*/
	return EAG_RETURN_OK;
}

pdc_ins_t *
pdc_ins_new(uint8_t hansitype, uint8_t insid)
{
	pdc_ins_t *pdcins = NULL;
	char buf[64] = "";
	
	pdcins = eag_malloc(sizeof(*pdcins));
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_new eag_malloc failed");
		goto failed_0;
	}

	memset(pdcins, 0, sizeof(*pdcins));

	pdcins->server = pdc_server_new();
	if (NULL == pdcins->server) {
		eag_log_err("pdc_ins_new pdc_server_new failed");
		goto failed_1;
	}
	
	pdcins->client = pdc_client_new();
	if (NULL == pdcins->client) {
		eag_log_err("pdc_ins_new pdc_client_new failed");
		goto failed_2;
	}
	
	pdcins->master = eag_thread_master_new();
	if (NULL == pdcins->master) {
		eag_log_err("pdc_ins_new eag_thread_master_new failed");
		goto failed_3;
	}

	pdc_dbus_path_init(hansitype, insid);
	pdcins->eagdbus = eag_dbus_new(PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH);
	if (NULL == pdcins->eagdbus) {
		eag_log_err("pdc_ins_new eag_dbus_new failed");
		goto failed_4;
	}

	pdcins->eaghansi = eag_hansi_new( pdcins->eagdbus, 
					insid,hansitype, pdcins->master);
	if( NULL == pdcins->eaghansi ){
		eag_log_err("pdc_ins_new eag_hansi_new failed!");
		goto failed_5;
	}
	eag_hansi_set_backup_port(pdcins->eaghansi,PDC_HANSI_BACKUP_PORT);
	eag_hansi_set_on_status_change_cb(pdcins->eaghansi,pdc_vrrp_status_switch_proc, pdcins );

	pdcins->userdb = pdc_userconn_db_create();
	if( NULL == pdcins->userdb ){
		eag_log_err("pdc_ins_new pdc_userconn_db_create failed!");
		goto failed_6;
	}
	
	
	pdcins->hansi_type = hansitype;
	pdcins->hansi_id = insid;
	read_file(IS_DISTRIBUTED_FILE, buf, sizeof(buf));
	pdcins->is_distributed = atoi(buf);
	read_file(SLOT_ID_FILE, buf, sizeof(buf));
	pdcins->slot_id = atoi(buf);

	pdcins->usermap = pdc_usermap_new();
	if( NULL == pdcins->usermap ){
		eag_log_err("pdc_ins_new pdc_usermap_new failed!");
		goto failed_7;
	}
	pdc_usermap_set_pdcins (pdcins->usermap, pdcins);/*it will use pdcins  hansitype hand insid.*/
	pdc_usermap_set_thread_master (pdcins->usermap, pdcins->master);
	pdc_usermap_set_userconn_db (pdcins->usermap, pdcins->userdb);

	eag_log_info("pdcins new hansiType %u, HansiId %u, "
		"isDistributed %d, slotId %u",
		pdcins->hansi_type,
		pdcins->hansi_id,
		pdcins->is_distributed,
		pdcins->slot_id);
	pdc_server_set_thread_master(pdcins->server, pdcins->master);
	pdc_server_set_pdcins(pdcins->server, pdcins);
//	pdc_server_set_ip(pdcins->server, 
//			SLOT_IPV4_BASE + pdcins->slot_id);
	if (HANSI_LOCAL == pdcins->hansi_type) {
		pdc_server_set_ip(pdcins->server, 
			SLOT_IPV4_BASE + 100 + pdcins->hansi_id );		
		pdc_server_set_port(pdcins->server,
			PDC_PORTAL_PORT_BASE + pdcins->hansi_id);
	}
	else {
		pdc_server_set_ip(pdcins->server, 
			SLOT_IPV4_BASE + pdcins->slot_id);		
		pdc_server_set_port(pdcins->server,
			PDC_PORTAL_PORT_BASE + MAX_HANSI_ID + pdcins->hansi_id);
	}
	
	pdc_client_set_thread_master(pdcins->client, pdcins->master);
	pdc_client_set_pdcins(pdcins->client, pdcins);
	
	pdc_register_all_dbus_method(pdcins);
	
	eag_log_debug("pdcins", "pdcins new ok");
	return pdcins;
	
failed_7:
	pdc_userconn_db_destroy(pdcins->userdb);
failed_6:
	eag_hansi_free(pdcins->eaghansi);
failed_5:
	eag_dbus_free(pdcins->eagdbus);
failed_4:
	eag_thread_master_free(pdcins->master);
failed_3:
	pdc_client_free(pdcins->client);
failed_2:
	pdc_server_free(pdcins->server);
failed_1:
	eag_free(pdcins);
failed_0:
	return NULL;
}

int
pdc_ins_free(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_free input error");
		return -1;
	}
	
	if (NULL != pdcins->eagdbus) {
		eag_dbus_free(pdcins->eagdbus);
	}
	if (NULL != pdcins->master) {
		eag_thread_master_free(pdcins->master);
	}
	if (NULL != pdcins->client) {
		pdc_client_free(pdcins->client);
	}
	if (NULL != pdcins->server) {
		pdc_server_free(pdcins->server);
	}
	if (NULL != pdcins->userdb) {
		pdc_userconn_db_destroy(pdcins->userdb);
	}
	if (NULL != pdcins->usermap) {
		pdc_usermap_free( pdcins->usermap );
	}
	eag_free(pdcins);

	eag_log_debug("pdcins", "pdcins free ok");
	return 0;
}



int
pdc_ins_start(pdc_ins_t *pdcins)
{
	int ret = 0;
	
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_start input error");
		return EAG_ERR_PDC_PDCINS_NULL;
	}

	if (NULL == pdcins->server) {
		eag_log_err("pdc_ins_start server null");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if (NULL == pdcins->client) {
		eag_log_err("pdc_ins_start client null");
		return EAG_ERR_PDC_CLIENT_NULL;
	}
	
	if (0 == pdcins->is_distributed) {
		eag_log_warning("pdc start, not distributed");
		return EAG_ERR_PDC_PRODUCT_NOT_DISTRIBUTED;
	}
	
	if (1 == pdcins->status) {
		eag_log_warning("pdc start, already started");
		return EAG_ERR_PDC_SERVICE_ALREADY_ENABLE;
	}

	if( EAG_RETURN_OK != eag_hansi_start(pdcins->eaghansi) ){
		eag_log_err("pdc_ins_start  eag_hansi_start failed!");
	}

	ret = pdc_usermap_start( pdcins->usermap );
	if( EAG_RETURN_OK != ret ){
		eag_log_err("pdc_usermap_start start failed!");
		return ret;
	}
#if 0
	if((!eag_hansi_is_enable(pdcins->eaghansi))
		  || eag_hansi_is_master(pdcins->eaghansi))
#else
	extern int if_has_ipaddr( unsigned long ipaddr );

	if (eag_hansi_is_master(pdcins->eaghansi)
		|| (if_has_ipaddr(pdc_client_get_nasip(pdcins->client)) 
			&& !eag_hansi_is_enable(pdcins->eaghansi)) )  /*shaojunwu 20110922. */
#endif
	{
		if ( (ret = pdc_server_start(pdcins->server)) != 0) {
			pdc_usermap_stop( pdcins->usermap );
			return ret;
		}
		if ( (ret = pdc_client_start(pdcins->client)) != 0) {
			pdc_usermap_stop( pdcins->usermap );
			pdc_server_stop(pdcins->server);
			return ret;
		}
	}else{
		eag_log_info("pdc_ins_start vrrp status is not master. do not listen nasip!");
	}
	pdcins->status = 1;

	eag_log_info("pdc start ok");
	return EAG_RETURN_OK;
}

int
pdc_ins_stop(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_stop input error");
		return EAG_ERR_PDC_PDCINS_NULL;
	}

	if (NULL == pdcins->server) {
		eag_log_err("pdc_ins_stop server null");
		return EAG_ERR_PDC_SERVER_NULL;
	}

	if (NULL == pdcins->client) {
		eag_log_err("pdc_ins_stop client null");
		return EAG_ERR_PDC_CLIENT_NULL;
	}

	if (0 == pdcins->status) {
		eag_log_warning("pdc stop, already stoped");
		return EAG_ERR_PDC_SERVICE_ALREADY_DISABLE;
	}

	pdc_server_stop(pdcins->server);
	pdc_client_stop(pdcins->client);
	pdc_usermap_stop(pdcins->usermap);
	if( EAG_RETURN_OK != eag_hansi_stop(pdcins->eaghansi) ){
		eag_log_err("pdc_ins_stop  eag_hansi_start failed!");
	}
	pdcins->status = 0;

	eag_log_info("pdc stop ok");
	return 0;
}

int 
pdc_ins_dispatch(pdc_ins_t *pdcins)
{
	struct timeval timer_wait = {0};

	if (NULL == pdcins)
	{
		eag_log_err("pdc_ins_dispatch input error");
		return -1;
	}
	
	timer_wait.tv_sec = 0;
	timer_wait.tv_usec = 10000;
	eag_thread_dispatch(pdcins->master, &timer_wait);
	eag_dbus_dispach(pdcins->eagdbus, 0);
	eag_hansi_check_connect_state(pdcins->eaghansi);
	
	return 0;
}

int
pdc_ins_is_running(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_is_running input error");
		return 0;
	}

	return pdcins->status;
}

pdc_server_t *
pdc_ins_get_server(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_get_server input error");
		return NULL;
	}

	return pdcins->server;
}

pdc_client_t *
pdc_ins_get_client(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_get_client input error");
		return NULL;
	}

	return pdcins->client;
}

pdc_usermap_t *
pdc_ins_get_usermap( pdc_ins_t *pdcins )
{
	if( NULL == pdcins ){
		eag_log_err("pdc_ins_get_usermap input error!");
		return NULL;
	}
	return pdcins->usermap;
}

pdc_userconn_db_t *
pdc_ins_get_userconn_db( pdc_ins_t *pdcins )
{
	if( NULL == pdcins ){
		eag_log_err("pdc_ins_get_userconn_db input error!");
		return NULL;
	}

	return pdcins->userdb;
}

uint8_t
pdc_ins_get_slot_id(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_get_slot_id input error");
		return 0;
	}

	return pdcins->slot_id;
}

uint8_t
pdc_ins_get_hansi_type(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_get_hansi_type input error");
		return 0;
	}

	return pdcins->hansi_type;
}

uint8_t
pdc_ins_get_hansi_id(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_ins_get_hansi_id input error");
		return 0;
	}

	return pdcins->hansi_id;
}

static DBusMessage *
pdc_dbus_method_set_nasip(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_client_t *client = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	uint32_t nasip = 0;
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_set_nasip "
					"DBUS new reply message error!");
		return NULL;
	}

	client = (pdc_client_t *)user_data;
	if (NULL == client) {
		eag_log_err("pdc_dbus_method_set_nasip user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &nasip,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_set_nasip "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_set_nasip %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = pdc_client_set_nasip(client, nasip);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	
	return reply;
}

static DBusMessage *
pdc_dbus_method_set_port(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_client_t *client = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	uint16_t port = 0;
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_set_port "
					"DBUS new reply message error!");
		return NULL;
	}

	client = (pdc_client_t *)user_data;
	if (NULL == client) {
		eag_log_err("pdc_dbus_method_set_port user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &port,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_set_port "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_set_port %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = pdc_client_set_port(client, port);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	
	return reply;
}

static DBusMessage *
pdc_dbus_method_set_status(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_ins_t *pdcins = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int status = 0;
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_set_status "
					"DBUS new reply message error!");
		return NULL;
	}

	pdcins = (pdc_ins_t *)user_data;
	if (NULL == pdcins) {
		eag_log_err("pdc_dbus_method_set_status user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_set_status "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_set_status %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	if (status) {
		ret = pdc_ins_start(pdcins);
	}
	else {
		ret = pdc_ins_stop(pdcins);
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *
pdc_dbus_method_check_status(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data )
{
	pdc_ins_t *pdcins = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	//int status = 0;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_check_status "
					"DBUS new reply message error");
		return NULL;
	}

	pdcins = (pdc_ins_t *)user_data;
	if (NULL == pdcins) {
		eag_log_err("pdc_dbus_method_check_status user_data error");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	//status = pdcins->status;
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32, &ret);
	/*
	if (EAG_RETURN_OK == ret) {
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &status);
	}
	*/
	return reply;
}

static DBusMessage *
pdc_dbus_method_get_base_conf(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_ins_t *pdcins = NULL;
	pdc_client_t *client = NULL;
	uint32_t nasip = 0;
	uint16_t port = 0;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	int portal_protocol = 0;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_get_base_conf "
					"DBUS new reply message error!");
		return NULL;
	}

	pdcins = (pdc_ins_t *)user_data;
	if (NULL == pdcins) {
		eag_log_err("pdc_dbus_method_get_base_conf user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	client = pdcins->client;
	if (NULL == client) {
		eag_log_err("pdc_dbus_method_get_base_conf client null");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	nasip = pdc_client_get_nasip(client);
	port = pdc_client_get_port(client);
	
	dbus_error_init(&err);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	if (EAG_RETURN_OK == ret) {
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_INT32, &(pdcins->status));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &(nasip));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT16, &(port));
		portal_protocol = portal_get_protocol_type();
		dbus_message_iter_append_basic(&iter, 
								DBUS_TYPE_INT32, &portal_protocol);
	}

	return reply;
}

static DBusMessage *
pdc_dbus_method_set_portal_protocol(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	pdc_ins_t *pdcins = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int32_t portal_protocol = 0;
	int ret = EAG_RETURN_OK;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_set_portal_protocol "
					"DBUS new reply message error!");
		return NULL;
	}

	pdcins = (pdc_ins_t *)user_data;
	if (NULL == pdcins) {
		eag_log_err("pdc_dbus_method_set_portal_protocol user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_INT32, &portal_protocol,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_set_portal_protocol "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_set_portal_protocol %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	portal_set_protocol_type(portal_protocol);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	
	return reply;
}

static DBusMessage *
pdc_dbus_method_add_debug_filter(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	char *filter = NULL;
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_add_debug_filter "
					"DBUS new reply message error!");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_add_debug_filter "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_add_debug_filter %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}	

	eag_log_add_filter(filter);
	ret = EAG_RETURN_OK;
	
replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	
	return reply;
}

static DBusMessage *
pdc_dbus_method_log_all_blkmen(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	struct pdc_ins_t *pdcins = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	int ret = -1;

	eag_log_info("pdc_dbus_method_log_all_blkmem_info");
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_log_all_blkmem_info DBUS "\
					"new reply message error!\n");
		return NULL;
	}

	pdcins = (struct pdc_ins_t *)user_data;
	if( NULL == pdcins	){
		eag_log_err("pdc_dbus_method_log_all_blkmem_info user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	eag_blkmem_log_all_blkmem(NULL);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_INT32, &ret);
	return reply;
}



static DBusMessage *
pdc_dbus_method_log_all_thread(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	
	pdc_ins_t *pdcins = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	int ret = -1;

	eag_log_info("pdc_dbus_method_log_all_thread");
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_log_all_thread DBUS "\
					"new reply message error!\n");
		return NULL;
	}

	pdcins = (pdc_ins_t *)user_data;
	if( NULL == pdcins ){
		eag_log_err("pdc_dbus_method_log_all_thread user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	eag_thread_log_all_thread(pdcins->master);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_INT32, &ret);
	return reply;
}



static DBusMessage *
pdc_dbus_method_del_debug_filter(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	char *filter = NULL;
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("pdc_dbus_method_del_debug_filter "
					"DBUS new reply message error!");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_INVALID))) {
		eag_log_err("pdc_dbus_method_del_debug_filter "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("pdc_dbus_method_del_debug_filter %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}	

	eag_log_del_filter(filter);
	ret = EAG_RETURN_OK;
	
replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	
	return reply;
}

static void
pdc_register_all_dbus_method(pdc_ins_t *pdcins)
{
	if (NULL == pdcins) {
		eag_log_err("pdc_register_all_dbus_method input error");
		return;
	}
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_set_nasip, pdcins->client);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_set_port, pdcins->client);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_set_status, pdcins);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_check_status, pdcins);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_get_base_conf, pdcins);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_add_map, pdcins->server);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_del_map, pdcins->server);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_modify_map, pdcins->server);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_show_maps, pdcins->server);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_add_ipv6_map, pdcins->server);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_del_ipv6_map, pdcins->server);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_modify_ipv6_map, pdcins->server);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_show_ipv6_maps, pdcins->server);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_set_portal_protocol, pdcins);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_add_debug_filter, NULL);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_del_debug_filter, NULL);

	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_log_all_blkmen, pdcins);
	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_log_all_thread, pdcins);

	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_log_all_thread, pdcins);

	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_set_userconn, pdcins->userdb);

	eag_dbus_register_method(pdcins->eagdbus,
		PDC_DBUS_INTERFACE, pdc_dbus_method_log_userconn, pdcins->userdb);
	/*vrrp*/
	eag_dbus_register_method(pdcins->eagdbus,
					PDC_DBUS_INTERFACE,
					eag_hansi_dbus_vrrp_state_change_func,
					pdcins->eaghansi );
}

