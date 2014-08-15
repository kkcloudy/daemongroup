#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "eag_errcode.h"
#include "limits2.h"
#include "nm_list.h"
#include "pdc_interface.h"
#include "eag_log.h"

#define MAX_DBUS_OBJPATH_LEN	128
#define MAX_DBUS_BUSNAME_LEN	128
#define MAX_DBUS_INTERFACE_LEN	128
#define MAX_DBUS_METHOD_LEN		128

/* pdc */
#define PDC_DBUS_METHOD_SET_NASIP			"pdc_dbus_method_set_nasip"
#define PDC_DBUS_METHOD_SET_PORTAL_PORT		"pdc_dbus_method_set_port"
#define PDC_DBUS_METHOD_SET_STATUS			"pdc_dbus_method_set_status"
#define PDC_DBUS_METHOD_GET_BASE_CONF		"pdc_dbus_method_get_base_conf"
#define PDC_DBUS_METHOD_ADD_MAP			 	"pdc_dbus_method_add_map"
#define PDC_DBUS_METHOD_DEL_MAP				"pdc_dbus_method_del_map"
#define PDC_DBUS_METHOD_MODIFY_MAP			"pdc_dbus_method_modify_map"
#define PDC_DBUS_METHOD_SHOW_MAPS			"pdc_dbus_method_show_maps"
#define PDC_DBUS_METHOD_SET_PORTAL_PROTOCOL	"pdc_dbus_method_set_portal_protocol"

#define PDC_DBUS_METHOD_ADD_DEBUG_FILTER 	"pdc_dbus_method_add_debug_filter"
#define PDC_DBUS_METHOD_DEL_DEBUG_FILTER  	"pdc_dbus_method_del_debug_filter"

#define PDC_DBUS_METHOD_LOG_ALL_BLKMEN		"pdc_dbus_method_log_all_blkmen"
#define PDC_DBUS_METHOD_LOG_ALL_THREAD	  	"pdc_dbus_method_log_all_thread"

#define PDC_DBUS_METHOD_SET_USERCONN		"pdc_dbus_method_set_userconn"
#define PDC_DBUS_METHOD_LOG_USERCONN		"pdc_dbus_method_log_userconn"


#define PDC_DBUS_METHOD_ADD_IPV6_MAP		"pdc_dbus_method_add_ipv6_map"
#define PDC_DBUS_METHOD_DEL_IPV6_MAP		"pdc_dbus_method_del_ipv6_map"
#define PDC_DBUS_METHOD_MODIFY_ipv6_MAP		"pdc_dbus_method_modify_ipv6_map"
#define PDC_DBUS_METHOD_SHOW_IPV6_MAPS		"pdc_dbus_method_show_ipv6_maps"


static char PDC_DBUS_NAME[MAX_DBUS_BUSNAME_LEN]="";
static char PDC_DBUS_OBJPATH[MAX_DBUS_BUSNAME_LEN]="";
static char PDC_DBUS_INTERFACE[MAX_DBUS_BUSNAME_LEN]="";

#define PDC_DBUS_NAME_FMT		"aw.pdc_%s_%d"
#define PDC_DBUS_OBJPATH_FMT	"/aw/pdc_%s_%d"
#define PDC_DBUS_INTERFACE_FMT	"aw.pdc_%s_%d"


#define INTERFACE_IP_CHECK_SUCCESS		0
#define INTERFACE_IP_CHECK_FAILURE		1

#define INTERFACE_MAC_CHECK_SUCESS		0
#define INTERFACE_MAC_CHECK_FAILURE		1


static void 
pdc_dbus_path_reinit(int type,int insid)
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
pdc_intf_set_nasip(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t nasip)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_SET_NASIP);
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
pdc_intf_set_port(DBusConnection *connection,
							int hansitype, int insid,
							uint16_t port)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_SET_PORTAL_PORT);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16, &port,
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
pdc_intf_set_status(DBusConnection *connection,
							int hansitype, int insid,
							int status)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_SET_STATUS);
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
pdc_intf_get_base_conf(DBusConnection *connection,
						int hansitype, int insid,
						struct pdc_base_conf *baseconf)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	if (NULL == baseconf){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_GET_BASE_CONF);
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
			dbus_message_iter_get_basic(&iter, &(baseconf->port));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &(baseconf->portal_protocol));
		}
	}

	dbus_message_unref(reply);
	
	return iRet;
}

int
pdc_intf_add_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t userip, uint32_t usermask,
							int  eag_slotid, int eag_insid)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_ADD_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_UINT32, &usermask,
								DBUS_TYPE_INT32, &eag_slotid,
								DBUS_TYPE_INT32, &hansitype,
								DBUS_TYPE_INT32, &eag_insid,
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
pdc_intf_del_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t userip, uint32_t usermask)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_DEL_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_UINT32, &usermask,
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
pdc_intf_modify_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t userip, uint32_t usermask,
							int  eag_slotid, int eag_insid)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_MODIFY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_UINT32, &usermask,
								DBUS_TYPE_INT32, &eag_slotid,
								DBUS_TYPE_INT32, &hansitype,
								DBUS_TYPE_INT32, &eag_insid,
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
pdc_intf_show_maps(DBusConnection *connection,
							int hansitype, int insid,
							struct pdc_map_conf *map_conf)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	int iRet = 0;
	uint32_t num = 0;
	int i = 0;
	int eag_slotid = 0;
	int eag_hansitype = 0;
	int eag_hansiid = 0;

	if (NULL == map_conf){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	memset(map_conf, 0, sizeof(struct pdc_map_conf));
	
	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_SHOW_MAPS);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		
		if( EAG_RETURN_OK == iRet ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			
			if( num > MAX_PDC_MAP_NUM ){
				num = MAX_PDC_MAP_NUM;
			}
			map_conf->num = num;
			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			
			
				for( i=0; i<num; i++ ){
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
									&(map_conf->map[i].userip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&(map_conf->map[i].usermask));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&eag_slotid);
					map_conf->map[i].eag_slotid = eag_slotid;
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&eag_hansitype);
					map_conf->map[i].eag_hansitype = eag_hansitype;
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&eag_hansiid);
					map_conf->map[i].eag_hansiid = eag_hansiid;
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&(map_conf->map[i].eag_ip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&(map_conf->map[i].eag_port));
					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return iRet;
	
}

int
pdc_intf_add_ipv6_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t useripv6[4], int prefix_length,
							int  eag_slotid, int eag_insid)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_ADD_IPV6_MAP);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &useripv6[0],
								DBUS_TYPE_UINT32, &useripv6[1],
								DBUS_TYPE_UINT32, &useripv6[2],
								DBUS_TYPE_UINT32, &useripv6[3],
								DBUS_TYPE_INT32, &prefix_length,
								DBUS_TYPE_INT32, &eag_slotid,
								DBUS_TYPE_INT32, &hansitype,
								DBUS_TYPE_INT32, &eag_insid,
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
pdc_intf_del_ipv6_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t useripv6[4], int prefix_length)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_DEL_IPV6_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &useripv6[0],
								DBUS_TYPE_UINT32, &useripv6[1],
								DBUS_TYPE_UINT32, &useripv6[2],
								DBUS_TYPE_UINT32, &useripv6[3],
								DBUS_TYPE_INT32, &prefix_length,
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
pdc_intf_modify_ipv6_map(DBusConnection *connection,
							int hansitype, int insid,
							uint32_t useripv6[4], int prefix_length,
							int  eag_slotid, int eag_insid)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_MODIFY_ipv6_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &useripv6[0],
								DBUS_TYPE_UINT32, &useripv6[1],
								DBUS_TYPE_UINT32, &useripv6[2],
								DBUS_TYPE_UINT32, &useripv6[3],
								DBUS_TYPE_INT32, &prefix_length,
								DBUS_TYPE_INT32, &eag_slotid,
								DBUS_TYPE_INT32, &hansitype,
								DBUS_TYPE_INT32, &eag_insid,
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
pdc_intf_show_ipv6_maps(DBusConnection *connection,
							int hansitype, int insid,
							struct pdc_ipv6_map_conf *map_conf)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	int iRet = 0;
	uint32_t num = 0;
	int i = 0;
	int eag_slotid = 0;
	int eag_hansitype = 0;
	int eag_hansiid = 0;

	if (NULL == map_conf){
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	memset(map_conf, 0, sizeof(struct pdc_ipv6_map_conf));
	
	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_SHOW_IPV6_MAPS);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (
						connection, query, -1, &err );
	
	dbus_message_unref(query);
	
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return EAG_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		
		if( EAG_RETURN_OK == iRet ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			
			if( num > MAX_PDC_IPV6_MAP_NUM ){
				num = MAX_PDC_IPV6_MAP_NUM;
			}
			map_conf->num = num;
			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			
			
				for( i=0; i<num; i++ ){					
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
												&(map_conf->map[i].userip[0]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
												&(map_conf->map[i].userip[1]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
												&(map_conf->map[i].userip[2]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, 
												&(map_conf->map[i].userip[3]));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&(map_conf->map[i].prefix_length));
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&eag_slotid);
					map_conf->map[i].eag_slotid = eag_slotid;					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&eag_hansitype);
					map_conf->map[i].eag_hansitype = eag_hansitype;					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&eag_hansiid);
					map_conf->map[i].eag_hansiid = eag_hansiid;					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&(map_conf->map[i].eag_ip));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,
									&(map_conf->map[i].eag_port));					
					dbus_message_iter_next(&iter_array);					
				}
			}
		}
	}
	
	dbus_message_unref(reply);
	
	return iRet;
	
}



int
pdc_intf_set_portal_protocol(DBusConnection *connection,
							int hansitype, int insid,
							int32_t portal_protocol)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_SET_PORTAL_PROTOCOL);
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32, &portal_protocol,
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
pdc_log_all_blkmem(DBusConnection *connection,
							int hansitype, int insid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet = -1;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_LOG_ALL_BLKMEN);
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
pdc_log_all_thread(DBusConnection *connection,
							int hansitype, int insid)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=-1;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
								PDC_DBUS_NAME,
								PDC_DBUS_OBJPATH,
								PDC_DBUS_INTERFACE,
								PDC_DBUS_METHOD_LOG_ALL_THREAD);
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
pdc_set_userconn( DBusConnection *connection,
							int hansitype, int insid,
							uint32_t userip,
							uint8_t eag_slotid,
							uint8_t eag_hansitype,
							uint8_t eag_hansiid )
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_SET_USERCONN );
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32, &userip,
								DBUS_TYPE_BYTE, &eag_slotid,
								DBUS_TYPE_BYTE, &eag_hansitype,
								DBUS_TYPE_BYTE, &eag_hansiid,
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
pdc_log_userconn( DBusConnection *connection,
							int hansitype, int insid )
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_LOG_USERCONN );
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
		dbus_message_get_args( reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);

	return iRet;
}

int
pdc_add_debug_filter(DBusConnection *connection,
							int hansitype, int insid,
							const char *filter)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_ADD_DEBUG_FILTER );
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
pdc_del_debug_filter(DBusConnection *connection,
							int hansitype, int insid,
							const char *filter)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	int iRet = 0;

	pdc_dbus_path_reinit(hansitype,insid);
	query = dbus_message_new_method_call(
									PDC_DBUS_NAME,
									PDC_DBUS_OBJPATH,
									PDC_DBUS_INTERFACE,
									PDC_DBUS_METHOD_DEL_DEBUG_FILTER );
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

