#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eag_errcode.h"
#include "limits2.h"
#include "nm_list.h"
#include "rdc_interface.h"

#define MAX_DBUS_OBJPATH_LEN	128
#define MAX_DBUS_BUSNAME_LEN	128
#define MAX_DBUS_INTERFACE_LEN	128
#define MAX_DBUS_METHOD_LEN		128

/* rdc */
#define RDC_DBUS_METHOD_SET_NASIP			"rdc_dbus_method_set_nasip"
#define RDC_DBUS_METHOD_SET_TIMEOUT			"rdc_dbus_method_set_timeout"
#define RDC_DBUS_METHOD_ADD_RADIUS			"rdc_dbus_method_add_radius"
#define RDC_DBUS_METHOD_DELETE_RADIUS		"rdc_dbus_method_delete_radius"
#define RDC_DBUS_METHOD_GET_RADIUS_CONF	"rdc_dbus_method_get_radius_conf"
#define RDC_DBUS_METHOD_SET_STATUS			"rdc_dbus_method_set_status"
#define RDC_DBUS_METHOD_GET_BASE_CONF		"rdc_dbus_method_get_base_conf"
#define RDC_DBUS_METHOD_ADD_DEBUG_FILTER 	"rdc_dbus_method_add_debug_filter"
#define RDC_DBUS_METHOD_DEL_DEBUG_FILTER  	"rdc_dbus_method_del_debug_filter"
#define RDC_DBUS_METHOD_LOG_ALL_PACKETCONN 	"rdc_dbus_method_log_all_packetconn"
#define RDC_DBUS_METHOD_LOG_ALL_USERCONN 	"rdc_dbus_method_log_all_userconn"
#define RDC_DBUS_METHOD_LOG_ALL_SOCKCLIENT 	"rdc_dbus_method_log_all_sockclient"
#define RDC_DBUS_METHOD_LOG_ALL_THREAD      "rdc_dbus_method_log_all_thread"
#define RDC_DBUS_METHOD_LOG_ALL_BLKMEM_INFO "rdc_dbus_method_log_all_blkmem_info"

static char RDC_DBUS_NAME[MAX_DBUS_BUSNAME_LEN]="";
static char RDC_DBUS_OBJPATH[MAX_DBUS_BUSNAME_LEN]="";
static char RDC_DBUS_INTERFACE[MAX_DBUS_BUSNAME_LEN]="";

#define RDC_DBUS_NAME_FMT		"aw.rdc_%s_%d"
#define RDC_DBUS_OBJPATH_FMT	"/aw/rdc_%s_%d"
#define RDC_DBUS_INTERFACE_FMT	"aw.rdc_%s_%d"

#define INTERFACE_IP_CHECK_SUCCESS		0
#define INTERFACE_IP_CHECK_FAILURE		1

#define INTERFACE_MAC_CHECK_SUCESS		0
#define INTERFACE_MAC_CHECK_FAILURE		1


static void 
rdc_dbus_path_reinit(int type,int insid)
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

int
rdc_intf_set_nasip(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t nasip)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_SET_NASIP);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &nasip,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
rdc_intf_set_timeout(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t timeout)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_SET_TIMEOUT);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &timeout,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
rdc_add_radius( DBusConnection *connection, 
							int hansitype, int insid, 					
							unsigned long auth_ip,
							unsigned short auth_port,
							char *auth_secret )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if(0 == auth_ip || 0 == auth_port){
		return EAG_ERR_RADIUS_PARAM_ERR;
	}
	if(NULL == auth_secret || strlen(auth_secret)>RADIUS_SECRETSIZE-1){
        return EAG_ERR_RADIUS_SECRET_LENTH_OUTOFF_SIZE;
	}
	
	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_ADD_RADIUS);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,							
							DBUS_TYPE_UINT32, &auth_ip,
							DBUS_TYPE_UINT16, &auth_port,
							DBUS_TYPE_STRING, &auth_secret,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
rdc_del_radius( DBusConnection *connection, 
							int hansitype, int insid, 
							unsigned long auth_ip,
							unsigned short auth_port,
							char *auth_secret )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	if(0 == auth_ip || 0 == auth_port){
		return EAG_ERR_RADIUS_PARAM_ERR;
	}
	if(NULL == auth_secret || strlen(auth_secret)>RADIUS_SECRETSIZE-1){
        return EAG_ERR_RADIUS_SECRET_LENTH_OUTOFF_SIZE;
	}

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE, 
									RDC_DBUS_METHOD_DELETE_RADIUS );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
							DBUS_TYPE_UINT32, &auth_ip,
							DBUS_TYPE_UINT16, &auth_port,
							DBUS_TYPE_STRING, &auth_secret,
							DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
rdc_get_radius_conf(DBusConnection *connection, 
							int hansitype, int insid, 
							char *domain,struct rdc_coa_radius_conf *radiusconf )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	
	int ret=0,i=0;
	unsigned long num = 0;
	
	char *auth_secret=NULL;

	if( NULL == domain ){
		domain = "";
	}

	if( NULL == radiusconf ){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE, 
									RDC_DBUS_METHOD_GET_RADIUS_CONF );
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &domain,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}
	else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &ret);
		
		if( EAG_RETURN_OK == ret ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);

			if( num > MAX_RADIUS_SRV_NUM ){
				num = MAX_RADIUS_SRV_NUM;
			}
			radiusconf->current_num = num;
			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			
			
				for( i=0; i<num; i++ ){
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					/*domain*/
					/*dbus_message_iter_get_basic(&iter_struct, &domain );

					if( NULL != domain ){
						strncpy( radiusconf->radius_srv[i].domain, 
										domain, MAX_RADIUS_DOMAIN_LEN-1);
					}
					dbus_message_iter_next(&iter_struct);
					*/
					/*auth*/
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].auth_ip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
											&(radiusconf->radius_srv[i].auth_port));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&auth_secret);

					if( NULL != auth_secret ){
						strncpy(radiusconf->radius_srv[i].auth_secret, 
								auth_secret, RADIUS_SECRETSIZE-1);
					}
		
					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return ret;	
}

int 
rdc_intf_set_status(DBusConnection *connection,
							int hansitype, int insid,
							int status)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_SET_STATUS);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
rdc_intf_get_base_conf(DBusConnection *connection,
						int hansitype, int insid,
						struct rdc_base_conf *baseconf)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_GET_BASE_CONF);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		DBusMessageIter  iter;
		dbus_message_iter_init(reply, &iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		if ( EAG_RETURN_OK == iRet ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(baseconf->status));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(baseconf->nasip));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(baseconf->timeout));
		}
	}

	dbus_message_unref(reply);
	
	return iRet;
}

int
rdc_add_debug_filter(DBusConnection *connection,
							int hansitype, int insid,
							const char *filter)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_ADD_DEBUG_FILTER );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
rdc_del_debug_filter(DBusConnection *connection,
							int hansitype, int insid,
							const char *filter)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_DEL_DEBUG_FILTER );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);
	
	return iRet;
}

int
rdc_log_all_packetconn( DBusConnection *connection,
							int hansitype,int insid)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_LOG_ALL_PACKETCONN);

	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

int
rdc_log_all_userconn( DBusConnection *connection,
							int hansitype,int insid)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_LOG_ALL_USERCONN);

	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}


int
rdc_log_all_thread( DBusConnection *connection, 
							int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_LOG_ALL_THREAD );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}


int
rdc_log_all_blkmem( DBusConnection *connection, 
							int hansitype,int insid )
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_LOG_ALL_BLKMEM_INFO );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}


int
rdc_log_all_sockclient( DBusConnection *connection,
							int hansitype,int insid)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	rdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									RDC_DBUS_NAME,
									RDC_DBUS_OBJPATH,
									RDC_DBUS_INTERFACE,
									RDC_DBUS_METHOD_LOG_ALL_SOCKCLIENT);

	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}
	
	dbus_message_unref(reply);	
	return iRet;
}

