/* rdc_ins.c */

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
#include "rdc_interface.h"
#include "eag_dbus.h"
#include "eag_hansi.h"
#include "eag_time.h"
#include "eag_util.h"
#include "rdc_ins.h"
#include "rdc_server.h"
#include "rdc_client.h"
#include "rdc_coa.h"
#include "rdc_pktconn.h"
#include "rdc_userconn.h"
#include "rdc_coaconn.h"

/* prime number */
#define RDC_PKTCONN_DB_HASHSIZE		1009
#define RDC_COACONN_DB_HASHSIZE		251

#define RDC_DBUS_NAME_FMT		"aw.rdc_%s_%d"
#define RDC_DBUS_OBJPATH_FMT	"/aw/rdc_%s_%d"
#define RDC_DBUS_INTERFACE_FMT	"aw.rdc_%s_%d"

#define SLOT_ID_FILE			"/dbm/local_board/slot_id"
#define IS_DISTRIBUTED_FILE		"/dbm/product/is_distributed"

#define RDC_HANSI_BACKUP_PORT	2004

#define RDC_DEFUALT_STOP_TIMEOUT		60
#define RDC_DEFAULT_INTERVAL_TIMEOUT	3600*8

struct rdc_ins {
	int status;
	rdc_server_t *server;
	rdc_client_t *client;
	rdc_coa_t *coa;
	
	rdc_pktconn_db_t *db;
	rdc_coaconn_db_t *coadb;
	rdc_userconn_db_t *userdb;
	
	eag_thread_master_t *master;
	eag_dbus_t *eagdbus;
	eag_hansi_t *eaghansi;
	int timeout;
	uint8_t slot_id;
	uint8_t hansi_type;
	uint8_t hansi_id;
	int is_distributed;
	uint32_t stop_timeout;// acct stop session will be deleted after this time
	uint32_t interval_timeout;// acct interval session will be deleted after this time.
	rdc_coa_radius_conf_t   radius_conf;
};

static char RDC_DBUS_NAME[MAX_DBUS_BUSNAME_LEN]="";
static char RDC_DBUS_OBJPATH[MAX_DBUS_BUSNAME_LEN]="";
static char RDC_DBUS_INTERFACE[MAX_DBUS_BUSNAME_LEN]="";

static void
rdc_register_all_dbus_method(rdc_ins_t *rdcins);

void 
rdc_dbus_path_init(int type,int insid)
{
	snprintf(RDC_DBUS_NAME, sizeof(RDC_DBUS_NAME),
				RDC_DBUS_NAME_FMT, 
				(HANSI_LOCAL==type)?"l":"r",insid);
	snprintf(RDC_DBUS_OBJPATH, sizeof(RDC_DBUS_OBJPATH),
				RDC_DBUS_OBJPATH_FMT,
				(HANSI_LOCAL==type)?"l":"r",insid);
	snprintf(RDC_DBUS_INTERFACE, sizeof(RDC_DBUS_INTERFACE),
				RDC_DBUS_INTERFACE_FMT,
				(HANSI_LOCAL==type)?"l":"r",insid);
	return;
}

static int
rdc_vrrp_status_switch_proc(eag_hansi_t *eaghansi,  void *cbp)
{
	int ret;
	rdc_ins_t *rdcins = (rdc_ins_t *)cbp;
	
	if( NULL == eaghansi || NULL==rdcins ){
		eag_log_err("rdc_vrrp_status_switch_proc param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( 1 != rdcins->status ){
		eag_log_info("pdc_vrrp_status_switch_proc get status change but pdc not start!");
		return EAG_RETURN_OK;
	}
	rdc_server_stop(rdcins->server);
	rdc_client_stop(rdcins->client);
	rdc_coa_stop(rdcins->coa);
	rdc_pktconn_db_clear(rdcins->db);
	if( eag_hansi_is_master(eaghansi) ){
		if ( (ret = rdc_server_start(rdcins->server)) != 0) {
			goto failed_0;
		}
		if ( (ret = rdc_client_start(rdcins->client)) != 0) {
			goto failed_1;
		}
		if ( (ret = rdc_coa_start(rdcins->coa)) != 0) {
			goto failed_2;
		}
	}
/*
	if(eag_hansi_is_enable(eaghansi)){
		if( eag_hansi_is_master(eaghansi) ){
			if ( (ret = rdc_server_start(rdcins->server)) != 0) {
				return ret;
			}
			if ( (ret = rdc_client_start(rdcins->client)) != 0) {
				rdc_server_stop(rdcins->server);
				return ret;
			}			
		}else{
			rdc_server_stop(rdcins->server);
			rdc_client_stop(rdcins->client);
		}
	}else{
		rdc_server_stop(rdcins->server);
		rdc_client_stop(rdcins->client);
	}
*/
	return EAG_RETURN_OK;
failed_2:
	rdc_client_stop(rdcins->client);
failed_1:
	rdc_server_stop(rdcins->server);
failed_0:
	return ret;
}


rdc_ins_t *
rdc_ins_new(uint8_t hansitype, uint8_t insid)
{
	rdc_ins_t *rdcins = NULL;
	char buf[64] = "";
	
	rdcins = eag_malloc(sizeof(*rdcins));
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_new eag_malloc failed");
		goto failed_0;
	}

	memset(rdcins, 0, sizeof(*rdcins));

	rdcins->server = rdc_server_new();
	if (NULL == rdcins->server) {
		eag_log_err("rdc_ins_new rdc_server_new failed");
		goto failed_1;
	}
	
	rdcins->client = rdc_client_new();
	if (NULL == rdcins->client) {
		eag_log_err("rdc_ins_new rdc_client_new failed");
		goto failed_2;
	}

	rdcins->coa = rdc_coa_new();
	if( NULL == rdcins->coa ){
		eag_log_err("rdc_ins_new rdc_coa_new failed");
		goto failed_3;
	}


	rdcins->db = 
		rdc_pktconn_db_create(RDC_PKTCONN_DB_HASHSIZE);
	if (NULL == rdcins->db) {
		eag_log_err("rdc_ins_new rdc_pktconn_db_create failed");
		goto failed_4;
	}

	rdcins->userdb = 
		rdc_userconn_db_create();
	if( NULL == rdcins->userdb ){
		eag_log_err("rdc_ins_new rdc_userconn_db_create failed");
		goto failed_5;
	}

	rdcins->coadb = 
		rdc_coaconn_db_create(RDC_COACONN_DB_HASHSIZE);
	if( NULL == rdcins->coadb ){
		eag_log_err("rdc_ins_new rdc_coaconn_db_create failed");
		goto failed_6;
	}
	rdcins->master = eag_thread_master_new();
	if (NULL == rdcins->master) {
		eag_log_err("rdc_ins_new eag_thread_master_new failed");
		goto failed_7;
	}

	rdc_dbus_path_init(hansitype, insid);
	rdcins->eagdbus = eag_dbus_new(RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH);
	if (NULL == rdcins->eagdbus) {
		eag_log_err("rdc_ins_new eag_dbus_new failed");
		goto failed_8;
	}

	rdcins->eaghansi = eag_hansi_new( rdcins->eagdbus, 
					insid,hansitype, rdcins->master);
	if( NULL == rdcins->eaghansi ){
		eag_log_err("rdc_ins_new eag_hansi_new failed!");
		goto failed_9;
	}
	eag_hansi_set_backup_port(rdcins->eaghansi,RDC_HANSI_BACKUP_PORT);
	eag_hansi_set_on_status_change_cb(rdcins->eaghansi,rdc_vrrp_status_switch_proc, rdcins );

	
	rdcins->timeout = RDC_TIMEOUT_DEFAULT;
	rdcins->hansi_type = hansitype;
	rdcins->hansi_id = insid;
	read_file(IS_DISTRIBUTED_FILE, buf, sizeof(buf));
	rdcins->is_distributed = atoi(buf);
	read_file(SLOT_ID_FILE, buf, sizeof(buf));
	rdcins->slot_id = atoi(buf);

	rdcins->stop_timeout = RDC_DEFUALT_STOP_TIMEOUT;
	rdcins->interval_timeout = RDC_DEFAULT_INTERVAL_TIMEOUT;

	eag_log_info("rdcins new hansiType %u, HansiId %u, "
		"isDistributed %d, slotId %u",
		rdcins->hansi_type,
		rdcins->hansi_id,
		rdcins->is_distributed,
		rdcins->slot_id);
	rdc_server_set_thread_master(rdcins->server, rdcins->master);
	rdc_server_set_rdcins(rdcins->server, rdcins);
//	rdc_server_set_ip(rdcins->server, 
//			SLOT_IPV4_BASE + rdcins->slot_id);
	if (HANSI_LOCAL == rdcins->hansi_type) {
		rdc_server_set_ip(rdcins->server, 
				SLOT_IPV4_BASE + 100 + rdcins->hansi_id);		
		rdc_server_set_port(rdcins->server,
			RDC_RADIUS_PORT_BASE + rdcins->hansi_id);
	}
	else {
		rdc_server_set_ip(rdcins->server, 
					SLOT_IPV4_BASE + rdcins->slot_id);
		rdc_server_set_port(rdcins->server,
			RDC_RADIUS_PORT_BASE + MAX_HANSI_ID + rdcins->hansi_id);
	}

	
	rdc_client_set_thread_master(rdcins->client, rdcins->master);
	rdc_client_set_rdcins(rdcins->client, rdcins);

	rdc_pktconn_db_set_timeout(rdcins->db, rdcins->timeout);
	rdc_pktconn_db_set_thread_master(rdcins->db, rdcins->master);

	rdc_server_set_userdb( rdcins->server, rdcins->userdb );

	rdc_coa_set_port(rdcins->coa,3799);
	rdc_coa_set_rdcins(rdcins->coa,rdcins);
	rdc_coa_set_thread_master(rdcins->coa,rdcins->master);
	rdc_coa_set_userdb(rdcins->coa,rdcins->userdb);
	rdc_coa_set_coadb(rdcins->coa,rdcins->coadb);
	rdc_coa_set_server(rdcins->coa,rdcins->server);
	rdc_coa_set_radiusconf(rdcins->coa,&(rdcins->radius_conf));

	rdc_coaconn_db_set_thread_master(rdcins->coadb,rdcins->master);
	rdc_coaconn_db_set_timeout(rdcins->coadb, rdcins->timeout );
	
	rdc_register_all_dbus_method(rdcins);
	
	eag_log_debug("rdcins", "rdcins new ok");
	return rdcins;
failed_9:
	eag_dbus_free(rdcins->eagdbus);
failed_8:
	eag_thread_master_free(rdcins->master);
failed_7:
	rdc_coaconn_db_destroy(rdcins->coadb);
failed_6:
	rdc_userconn_db_destroy(rdcins->userdb);
failed_5:
	rdc_pktconn_db_destroy(rdcins->db);
failed_4:
	rdc_coa_free(rdcins->coa);
failed_3:
	rdc_client_free(rdcins->client);
failed_2:
	rdc_server_free(rdcins->server);
failed_1:
	eag_free(rdcins);
failed_0:
	return NULL;
}

int
rdc_ins_free(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_free input error");
		return -1;
	}
	
	if (NULL != rdcins->eagdbus) {
		eag_dbus_free(rdcins->eagdbus);
	}
	if (NULL != rdcins->master) {
		eag_thread_master_free(rdcins->master);
	}
	if (NULL != rdcins->db) {
		rdc_pktconn_db_destroy(rdcins->db);
	}
	if( NULL != rdcins->userdb){
		rdc_userconn_db_destroy(rdcins->userdb);
	}
	if (NULL != rdcins->client) {
		rdc_client_free(rdcins->client);
	}
	if (NULL != rdcins->server) {
		rdc_server_free(rdcins->server);
	}
	eag_free(rdcins);

	eag_log_debug("rdcins", "rdcins free ok");
	return 0;
}

int
rdc_ins_start(rdc_ins_t *rdcins)
{
	int ret = 0;
	
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_start input error");
		return EAG_ERR_RDC_RDCINS_NULL;
	}

	if (NULL == rdcins->server) {
		eag_log_err("rdc_ins_start server null");
		return EAG_ERR_RDC_SERVER_NULL;
	}

	if (NULL == rdcins->client) {
		eag_log_err("rdc_ins_start client null");
		return EAG_ERR_RDC_CLIENT_NULL;
	}
	
	if (0 == rdcins->is_distributed) {
		eag_log_warning("rdc start, not distributed");
		return EAG_ERR_RDC_PRODUCT_NOT_DISTRIBUTED;
	}
	
	if (1 == rdcins->status) {
		eag_log_warning("rdc start, already started");
		return EAG_ERR_RDC_SERVICE_ALREADY_ENABLE;
	}

	if( EAG_RETURN_OK != eag_hansi_start(rdcins->eaghansi) ){
		eag_log_err("rdc_ins_start  eag_hansi_start failed!");
	}
	
#if 0	
	if((!eag_hansi_is_enable(rdcins->eaghansi))
		  || eag_hansi_is_master(rdcins->eaghansi))
#else
	extern int if_has_ipaddr( unsigned long ipaddr );

	if(eag_hansi_is_master(rdcins->eaghansi)
		|| (if_has_ipaddr(rdc_client_get_nasip(rdcins->client)) 
			&& !eag_hansi_is_enable(rdcins->eaghansi)) )  /*shaojunwu 20110922. */
#endif
	{
		if ( (ret = rdc_server_start(rdcins->server)) != 0) {
			goto failed_0;
		}
		if ( (ret = rdc_client_start(rdcins->client)) != 0) {
			goto failed_1;
		}
		if ( (ret = rdc_coa_start(rdcins->coa)) != 0) {
			goto failed_2;
		}
	}else{
		eag_log_info("rdc_ins_start vrrp status is not master. do not listen nasip!");
	}
	rdcins->status = 1;

	eag_log_info("rdc start ok");
	return EAG_RETURN_OK;
failed_2:
	rdc_client_stop(rdcins->client);
failed_1:
	rdc_server_stop(rdcins->server);
failed_0:
	return ret;
}

int
rdc_ins_stop(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_stop input error");
		return EAG_ERR_RDC_RDCINS_NULL;
	}

	if (NULL == rdcins->server) {
		eag_log_err("rdc_ins_stop server null");
		return EAG_ERR_RDC_SERVER_NULL;
	}

	if (NULL == rdcins->client) {
		eag_log_err("rdc_ins_stop client null");
		return EAG_ERR_RDC_CLIENT_NULL;
	}
	if (NULL == rdcins->coa) {
		eag_log_err("rdc_ins_stop coa null");
		return EAG_ERR_RDC_COA_NULL;
	}
	if (NULL == rdcins->db) {
		eag_log_err("rdc_ins_stop pktdb null");
		return EAG_ERR_RDC_PKTCONNDB_NULL;
	}
	if( NULL == rdcins->userdb ){
		eag_log_err("rdc_ins_stop userdb null");
		return EAG_ERR_RDC_USERCONNDB_NULL;
	}
	if( NULL == rdcins->coadb ){
		eag_log_err("rdc_ins_stop coadb null");
		return EAG_ERR_RDC_COACONNDB_NULL;
	}

	if (0 == rdcins->status) {
		eag_log_warning("rdc stop, already stoped");
		return EAG_ERR_RDC_SERVICE_ALREADY_DISABLE;
	}

	rdc_server_stop(rdcins->server);
	rdc_client_stop(rdcins->client);
	rdc_coa_stop(rdcins->coa);
	
	rdc_pktconn_db_clear(rdcins->db);
	rdc_userconn_db_clear(rdcins->userdb);
	rdc_coaconn_db_clear(rdcins->coadb);
	
	if( EAG_RETURN_OK != eag_hansi_stop(rdcins->eaghansi) ){
		eag_log_err("rdc_ins_stop  eag_hansi_start failed!");
	}
	rdcins->status = 0;

	eag_log_info("rdc stop ok");
	return 0;
}

int
rdc_ins_set_timeout(rdc_ins_t *rdcins,
						uint32_t timeout)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_set_timeout input error");
		return -1;
	}
	
	rdcins->timeout = timeout;
	if (NULL != rdcins->db) {
		rdc_pktconn_db_set_timeout(rdcins->db, timeout);
	}
	if (NULL != rdcins->coadb) {
		rdc_coaconn_db_set_timeout(rdcins->coadb, timeout);
	}

	return 0;
}

int 
rdc_ins_dispatch(rdc_ins_t *rdcins)
{
	struct timeval timer_wait = {0};
	struct timeval tv={0};
	uint32_t timenow=0;
	static uint32_t timeprv = 0;

	if (NULL == rdcins)
	{
		eag_log_err("rdc_ins_dispatch input error");
		return -1;
	}
	
	timer_wait.tv_sec = 0;
	timer_wait.tv_usec = 10000;
	eag_thread_dispatch(rdcins->master, &timer_wait);
	eag_dbus_dispach(rdcins->eagdbus, 0);

	eag_time_gettimeofday( &tv, NULL );
	timenow = tv.tv_sec;
	if( timenow < timeprv
		|| timenow - timeprv >=1 )
	{
		rdc_userconn_db_check_timeout(rdcins->userdb,timenow,
					rdcins->interval_timeout, rdcins->stop_timeout );
		timeprv = timenow;
	}

	return 0;
}

int
rdc_ins_is_running(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_is_running input error");
		return 0;
	}

	return rdcins->status;
}

rdc_server_t *
rdc_ins_get_server(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_get_server input error");
		return NULL;
	}

	return rdcins->server;
}

rdc_client_t *
rdc_ins_get_client(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_get_client input error");
		return NULL;
	}

	return rdcins->client;
}

rdc_coa_t *
rdc_ins_get_coa(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_get_coa input error");
		return NULL;
	}

	return rdcins->coa;
}


rdc_pktconn_db_t *
rdc_ins_get_pktconn_db(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_get_pktconn_db input error");
		return NULL;
	}

	return rdcins->db;
}

rdc_coaconn_db_t *
rdc_ins_get_coaconn_db(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_get_pktconn_db input error");
		return NULL;
	}

	return rdcins->coadb;
}


uint8_t
rdc_ins_get_slot_id(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_get_slot_id input error");
		return 0;
	}

	return rdcins->slot_id;
}

uint8_t
rdc_ins_get_hansi_type(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_get_hansi_type input error");
		return 0;
	}

	return rdcins->hansi_type;
}

uint8_t
rdc_ins_get_hansi_id(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_ins_get_hansi_id input error");
		return 0;
	}

	return rdcins->hansi_id;
}

static DBusMessage *
rdc_dbus_method_set_nasip(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	rdc_ins_t *rdcins = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	uint32_t nasip = 0;
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_set_nasip "
					"DBUS new reply message error!");
		return NULL;
	}

	rdcins = (rdc_ins_t *)user_data;
	if (NULL == rdcins) {
		eag_log_err("rdc_dbus_method_set_nasip user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &nasip,
								DBUS_TYPE_INVALID))) {
		eag_log_err("rdc_dbus_method_set_nasip "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("rdc_dbus_method_set_nasip %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	ret = rdc_client_set_nasip(rdcins->client, nasip);
	if( 0 == ret ){
		ret = rdc_coa_set_nasip(rdcins->coa,nasip);
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	
	return reply;
}

static DBusMessage *
rdc_dbus_method_set_timeout(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	rdc_ins_t *rdcins = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	uint32_t timeout = 0;
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_set_timeout "
					"DBUS new reply message error!");
		return NULL;
	}

	rdcins = (rdc_ins_t *)user_data;
	if (NULL == rdcins) {
		eag_log_err("rdc_dbus_method_set_timeout user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &timeout,
								DBUS_TYPE_INVALID))) {
		eag_log_err("rdc_dbus_method_set_timeout "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("rdc_dbus_method_set_timeout %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	rdc_ins_set_timeout(rdcins, timeout);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	
	return reply;
}
	
static    DBusMessage *
rdc_dbus_method_add_radius(
					DBusConnection *conn, 
					DBusMessage *msg, 
					void *user_data )
{
		rdc_ins_t *rdcins = NULL;
		DBusMessage* reply = NULL;
		DBusMessageIter iter = {0};
		DBusError		err = {0};
		struct radius_srv_coa *radius_srv = NULL;
		
		unsigned long auth_ip=0;
		unsigned short auth_port=0;
		char *auth_secret=NULL;
		
		int ret = -1;
	
		reply = dbus_message_new_method_return(msg);
		if (NULL == reply) {
			eag_log_err("rdc_dbus_method_add_radius "\
						"DBUS new reply message error!\n");
			return NULL;
		}
	
		rdcins = (rdc_ins_t *)user_data;
		if (NULL == rdcins){
			eag_log_err("rdc_dbus_method_add_radius user_data error!");
			//return reply;
			ret = EAG_ERR_UNKNOWN;
			goto replyx;
		}
		
		dbus_error_init(&err);
		if (!(dbus_message_get_args(msg ,&err,
									DBUS_TYPE_UINT32, &auth_ip,
									DBUS_TYPE_UINT16, &auth_port,
									DBUS_TYPE_STRING, &auth_secret,							
									DBUS_TYPE_INVALID))){
			eag_log_err("rdc_dbus_method_add_radius "\
						"unable to get input args\n");
			if (dbus_error_is_set(&err)) {
				eag_log_err("rdc_dbus_method_add_radius %s raised:%s\n",
								err.name, err.message);
				dbus_error_free(&err);
			}
			ret = EAG_ERR_DBUS_FAILED;
			goto replyx;
		}
	
		radius_srv = rdc_coa_check_radius_srv(&(rdcins->radius_conf),auth_ip,auth_port,auth_secret);
		if( NULL != radius_srv ){
			ret = EAG_ERR_RADIUS_DOAMIN_AREADY_EXIST;
			eag_log_err("eag_dbus_method_add_radius domain already exist!");
		}
		else if( MAX_RADIUS_SRV_NUM == rdcins->radius_conf.current_num ){
			ret = EAG_ERR_RADIUS_MAX_NUM_LIMITE;
			eag_log_err("eag_dbus_method_add_radius failed because max num limite!");
		}
		else{
			radius_srv = rdc_coa_conf_radius_srv(&(rdcins->radius_conf));
			rdc_coa_set_radius_srv(radius_srv,auth_ip,auth_port,
								auth_secret,strlen(auth_secret));
			ret = EAG_RETURN_OK;
		}
	
	replyx:
		dbus_message_iter_init_append(reply, &iter);
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_INT32, &ret);
		return reply;
}

DBusMessage *
rdc_dbus_method_delete_radius(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data )
{
	rdc_ins_t *rdcins = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	struct radius_srv_coa *radius_srv = NULL;
	
	unsigned long auth_ip=0;
	unsigned short auth_port=0;
	char *auth_secret=NULL;
	
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_set_portal_port "\
					"DBUS new reply message error!\n");
		return NULL;
	}

	rdcins = ( rdc_ins_t *)user_data;
	if( NULL == rdcins ){
		eag_log_err("rdc_dbus_method_add_radius  user_data  error!");
		//return reply;
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
									DBUS_TYPE_UINT32, &auth_ip,
									DBUS_TYPE_UINT16, &auth_port,
									DBUS_TYPE_STRING, &auth_secret,							
									DBUS_TYPE_INVALID))){
		eag_log_err("rdc_dbus_method_del_radius "\
					"unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			eag_log_err("rdc_dbus_method_del_radius %s raised:%s\n",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	radius_srv = rdc_coa_check_radius_srv(&(rdcins->radius_conf),auth_ip,auth_port,auth_secret);
	if( NULL == radius_srv ){
		ret = EAG_ERR_RADIUS_DOAMIN_NOT_EXIST;
		eag_log_err("rdc_dbus_method_del_radius ip,port,secret not exist!");
	}
	else{
		ret = rdc_coa_del_radius_srv( &(rdcins->radius_conf),auth_ip,auth_port,auth_secret);
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	return reply;
}

static DBusMessage *
rdc_dbus_method_get_radius_conf(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data )
{
	rdc_ins_t *rdcins = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError  err = {0};
	struct  radius_srv_coa * radius_srv = NULL;
	int num=0;
	
	char *domain=NULL;
	char *auth_secret=NULL;
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_get_radius_conf "\
					"DBUS new reply message error!\n");
		return NULL;
	}

	rdcins = ( rdc_ins_t *)user_data;
	if( NULL == rdcins ){
		eag_log_err("rdc_dbus_method_get_radius_conf user_data error!");
		//return reply;
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
								DBUS_TYPE_STRING, &domain,						
								DBUS_TYPE_INVALID))){
		eag_log_err("rdc_dbus_method_get_radius_conf "\
					"unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			eag_log_err("rdc_dbus_method_get_radius_conf %s raised:%s\n",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	eag_log_info("rdc_dbus_method_get_radius_conf domain=%s,end", domain );
	if( 0 == strlen(domain)){
		/*get all radius configuration*/
		num = rdcins->radius_conf.current_num;
		ret = EAG_RETURN_OK;
		radius_srv = &(rdcins->radius_conf.radius_srv[0]);
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	if( EAG_RETURN_OK == ret ){
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &num);
	}
	if( EAG_RETURN_OK == ret && num > 0 ){
		int i;
		DBusMessageIter  iter_array;
		
		dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING											
											 DBUS_TYPE_UINT32_AS_STRING	
											 DBUS_TYPE_UINT16_AS_STRING
											 DBUS_TYPE_STRING_AS_STRING																
										DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

		for( i=0; i<num; i++ ){
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			/*domain = radius_srv[i].domain;
			eag_log_info("eag_dbus_method_get_radius_conf add domain %s", domain );
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_STRING, &domain);
			*/
			/*auth*/
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32, &radius_srv[i].auth_ip);			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT16, &radius_srv[i].auth_port);
			auth_secret = radius_srv[i].auth_secret;
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_STRING, &auth_secret);

			dbus_message_iter_close_container (&iter_array, &iter_struct);

		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	
	return reply;
}

static DBusMessage *
rdc_dbus_method_set_status(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	rdc_ins_t *rdcins = NULL;
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

	rdcins = (rdc_ins_t *)user_data;
	if (NULL == rdcins) {
		eag_log_err("rdc_dbus_method_set_status user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID))) {
		eag_log_err("rdc_dbus_method_set_status "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("rdc_dbus_method_set_status %s raised:%s",
							err.name, err.message);
			dbus_error_free(&err);
		}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}

	if (status) {
		ret = rdc_ins_start(rdcins);
	}
	else {
		ret = rdc_ins_stop(rdcins);
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *
rdc_dbus_method_check_status(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data )
{
	rdc_ins_t *rdcins = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;
	//int status = 0;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_check_status "
					"DBUS new reply message error");
		return NULL;
	}

	rdcins = (rdc_ins_t *)user_data;
	if (NULL == rdcins) {
		eag_log_err("rdc_dbus_method_check_status user_data error");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);
	//status = rdcins->status;
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
rdc_dbus_method_get_base_conf(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	rdc_ins_t *rdcins = NULL;
	rdc_client_t *client = NULL;
	uint32_t nasip = 0;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_get_base_conf "
					"DBUS new reply message error!");
		return NULL;
	}

	rdcins = (rdc_ins_t *)user_data;
	if (NULL == rdcins) {
		eag_log_err("rdc_dbus_method_get_base_conf user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	client = rdcins->client;
	if (NULL == client) {
		eag_log_err("rdc_dbus_method_get_base_conf client null");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	nasip = rdc_client_get_nasip(client);
	
	dbus_error_init(&err);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);
	if (EAG_RETURN_OK == ret) {
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_INT32, &(rdcins->status));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &(nasip));
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &(rdcins->timeout));
	}

	return reply;
}

static DBusMessage *
rdc_dbus_method_add_debug_filter(
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
		eag_log_err("rdc_dbus_method_add_debug_filter "
					"DBUS new reply message error!");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_INVALID))) {
		eag_log_err("rdc_dbus_method_add_debug_filter "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("rdc_dbus_method_add_debug_filter %s raised:%s",
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
rdc_dbus_method_del_debug_filter(
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
		eag_log_err("rdc_dbus_method_del_debug_filter "
					"DBUS new reply message error!");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_INVALID))) {
		eag_log_err("rdc_dbus_method_del_debug_filter "
					"unable to get input args");
		if (dbus_error_is_set(&err)) {
			eag_log_err("rdc_dbus_method_del_debug_filter %s raised:%s",
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

static DBusMessage *
rdc_dbus_method_log_all_packetconn(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	rdc_pktconn_db_t *db = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_log_all_packetconn "
					"DBUS new reply message error!");
		return NULL;
	}
	
	db = (rdc_pktconn_db_t *)user_data;
	if (NULL == db) {
		eag_log_err("rdc_dbus_method_log_all_packetconn "
					"user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	rdc_pktconn_db_log_all(db);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_INT32, &ret);

	return reply;
}

static DBusMessage *
rdc_dbus_method_log_all_userconn(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	rdc_userconn_db_t *db = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_log_all_userconn "
					"DBUS new reply message error!");
		return NULL;
	}
	
	db = (rdc_userconn_db_t *)user_data;
	if (NULL == db) {
		eag_log_err("rdc_dbus_method_log_all_userconn "
					"user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	rdc_userconn_db_log_all(db);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_INT32, &ret);

	return reply;
}

static DBusMessage *
rdc_dbus_method_log_all_sockclient(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data )
{
	rdc_client_t *client = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_log_all_sockclient "
					"DBUS new reply message error!");
		return NULL;
	}

	client = (rdc_client_t *)user_data;
	if (NULL == client) {
		eag_log_err("rdc_dbus_method_log_all_sockclient "
					"user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	rdc_client_log_all(client);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *
rdc_dbus_method_log_all_thread(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data )
{
	rdc_ins_t *rdcins = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	int ret = -1;

	eag_log_info("rdc_dbus_method_log_all_thread");
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_log_all_thread DBUS "\
					"new reply message error!\n");
		return NULL;
	}

	rdcins = (rdc_ins_t *)user_data;
	if( NULL == rdcins ){
		eag_log_err("rdc_dbus_method_log_all_thread user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}	
	
	eag_thread_log_all_thread(rdcins->master);
	ret = EAG_RETURN_OK;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_INT32, &ret);
	return reply;
}



DBusMessage *
rdc_dbus_method_log_all_blkmem_info(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data )
{
	rdc_ins_t *rdcins = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	int ret = -1;

	eag_log_info("rdc_dbus_method_log_all_blkmem_info");
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("rdc_dbus_method_log_all_blkmem_info DBUS "\
					"new reply message error!\n");
		return NULL;
	}

	rdcins = (rdc_ins_t *)user_data;
	if( NULL == rdcins  ){
		eag_log_err("rdc_dbus_method_log_all_blkmem_info user_data error!");
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



static void
rdc_register_all_dbus_method(rdc_ins_t *rdcins)
{
	if (NULL == rdcins) {
		eag_log_err("rdc_register_all_dbus_method input error");
		return;
	}
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_set_nasip, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_set_timeout, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_add_radius, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_delete_radius, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_get_radius_conf, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_set_status, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_check_status, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_get_base_conf, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_add_debug_filter, NULL);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_del_debug_filter, NULL);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_log_all_packetconn, rdcins->db);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_log_all_userconn, rdcins->userdb);	
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_log_all_sockclient, rdcins->client);
	eag_dbus_register_method(rdcins->eagdbus,
					RDC_DBUS_INTERFACE,
					eag_hansi_dbus_vrrp_state_change_func,
					rdcins->eaghansi );	
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_log_all_blkmem_info, rdcins);
	eag_dbus_register_method(rdcins->eagdbus,
		RDC_DBUS_INTERFACE, rdc_dbus_method_log_all_thread, rdcins);
}

