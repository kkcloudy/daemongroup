#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dbus/dbus.h>

#include "ws_log_conf.h"
#include "ac_manage_def.h"
#include "ac_manage_ntpsyslog_interface.h"
#include "manage_log.h"


static void
ntp_rule_copy(struct clientz_st *new_rule, struct clientz_st *rule) {
	if(NULL == new_rule || NULL == rule) {
		return ;
	}

	memset(new_rule, 0, sizeof(struct clientz_st));
	
	memcpy(new_rule->clitipz, rule->clitipz,sizeof(new_rule->clitipz));
	memcpy(new_rule->ifper, rule->ifper,sizeof(new_rule->ifper));
	memcpy(new_rule->timeflag, rule->timeflag,sizeof(new_rule->timeflag));
	memcpy(new_rule->slotid, rule->slotid,sizeof(new_rule->slotid));

	return ;
}


static void
ntp_syslog_copy(struct syslogrule_st *new_rule, struct syslogrule_st *rule) {
	if(NULL == new_rule || NULL == rule) {
		return ;
	}

	memset(new_rule, 0, sizeof(struct syslogrule_st));
	
	memcpy(new_rule->udpstr, rule->udpstr,sizeof(new_rule->udpstr));
	memcpy(new_rule->ipstr, rule->ipstr,sizeof(new_rule->ipstr));
	memcpy(new_rule->portstr, rule->portstr,sizeof(new_rule->portstr));
	memcpy(new_rule->flevel, rule->flevel,sizeof(new_rule->flevel));
	memcpy(new_rule->id, rule->id,sizeof(new_rule->id));
	return ;
}

static void
time_rule_copy(struct timez_st *new_rule, struct timez_st *rule) {
	if(NULL == new_rule || NULL == rule) {
		return ;
	}

	memset(new_rule, 0, sizeof(struct timez_st));
	
	memcpy(new_rule->nowstr, rule->nowstr,sizeof(new_rule->nowstr));
	memcpy(new_rule->utcstr, rule->utcstr,sizeof(new_rule->utcstr));
	memcpy(new_rule->slotid, rule->slotid,sizeof(new_rule->slotid));

	return ;
}

int 
ac_manage_show_ntp_rule(DBusConnection *connection, u_long *service_status, struct clientz_st **rule_array, u_long *rule_num) {


	if(NULL == connection || NULL == rule_array || NULL == rule_num) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	*rule_array = NULL;
	*rule_num = 0;

	struct clientz_st *temp_array = NULL;
	u_long temp_num = 0;
	int ret = AC_MANAGE_DBUS_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SHOW_NTP_RULE);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);  
	if(service_status) {
		dbus_message_iter_get_basic(&iter, service_status);
	}	

	dbus_message_iter_next(&iter);  
	dbus_message_iter_get_basic(&iter, &temp_num);

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   
	
	if(AC_MANAGE_SUCCESS == ret && temp_num) {
		temp_array = (struct clientz_st *)calloc(temp_num, sizeof(struct clientz_st));
		if(NULL == temp_array) {
			dbus_message_unref(reply);
			return AC_MANAGE_MALLOC_ERROR;
		}

		int i = 0;
		for(i = 0; i < temp_num; i++) {

	            struct clientz_st tempRule = { 0 };
				char *clitipz = NULL;
				char *ifper = NULL;
				char *timeflag = NULL;
				char *slotid = NULL;
	                
	            dbus_message_iter_recurse(&iter_array, &iter_struct);
	            dbus_message_iter_get_basic(&iter_struct, &clitipz);

				strncpy(tempRule.clitipz,clitipz,sizeof(tempRule.clitipz)-1);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &ifper);

				strncpy(tempRule.ifper,ifper,sizeof(tempRule.ifper)-1);
				

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &timeflag);

				strncpy(tempRule.timeflag,timeflag,sizeof(tempRule.timeflag)-1);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &slotid);

				strncpy(tempRule.slotid,slotid,sizeof(tempRule.slotid)-1);

	            ntp_rule_copy(&temp_array[i], &tempRule);
					
	            dbus_message_iter_next(&iter_array);
		}
        
		*rule_array = temp_array;
		*rule_num = temp_num;
	}

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_add_ntpserver_rule(DBusConnection *connection, struct clientz_st *rule,int config_type) {
	if(NULL == connection || NULL == rule) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	//char *natport = rule->natport ? rule->natport : "";

	char *serverip = rule->clitipz;
	char *serverpri =  rule->ifper;
	char *serversid =  rule->slotid;
	int opt_type = 0;
	opt_type = config_type;
	dbus_error_init(&err);

	

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_ADD_NTPSERVER);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &serverip,
							DBUS_TYPE_STRING, &serverpri,
							DBUS_TYPE_STRING, &serversid,
							DBUS_TYPE_INT32,&opt_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_inside_ntp_rule(DBusConnection *connection) {
	if(NULL == connection) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	//char *natport = rule->natport ? rule->natport : "";

	
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_INSIDE_NTP);
	dbus_message_append_args(query,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_set_ntpstatus_rule(DBusConnection *connection,char *status) {
	if(NULL == connection || NULL == status) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SET_NTP_STATUS);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &status,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_clean_ntp_rule(DBusConnection *connection) {
	if(NULL == connection) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	//char *natport = rule->natport ? rule->natport : "";

	
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_CLENA_NTP);
	dbus_message_append_args(query,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_set_timezone(DBusConnection *connection,char *area,char *city) {
	if(NULL == connection || NULL == city || NULL == area) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SET_TIMEZONE);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &area,
							DBUS_TYPE_STRING, &city,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_add_ntpclient_rule(DBusConnection *connection, struct serverz_st *rule,int opt_type) {

	if(NULL == connection || NULL == rule) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	char *clientip = rule->servipz;
	char *clientmask =  rule->maskz;
	char *clientsid =  rule->slotid;
	int opt = 0;
	opt = opt_type;
	dbus_error_init(&err);
	
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_ADD_NTPCLIENT);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &clientip,
							DBUS_TYPE_STRING, &clientmask,
							DBUS_TYPE_STRING, &clientsid,
							DBUS_TYPE_INT32,&opt,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int 
ac_manage_show_time(DBusConnection *connection, struct timez_st *rule_array) {


	if(NULL == connection) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	int ret = AC_MANAGE_DBUS_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SHOW_TIME);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   
	
	if(AC_MANAGE_SUCCESS == ret) {

		char *nowstr = NULL;
		char *utcstr = NULL;
		char *slotid = NULL;
            
        dbus_message_iter_recurse(&iter_array, &iter_struct);
        dbus_message_iter_get_basic(&iter_struct, &nowstr);

		strncpy(rule_array->nowstr,nowstr,sizeof(rule_array->nowstr)-1);

        dbus_message_iter_next(&iter_struct);  
        dbus_message_iter_get_basic(&iter_struct, &utcstr);

		strncpy(rule_array->utcstr,utcstr,sizeof(rule_array->utcstr)-1);
        
        dbus_message_iter_next(&iter_struct);  
        dbus_message_iter_get_basic(&iter_struct, &slotid);

		strncpy(rule_array->slotid,slotid,sizeof(rule_array->slotid)-1);

        //time_rule_copy(&temp_array, &tempRule);
			
        dbus_message_iter_next(&iter_array);
        
	}

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_set_time(DBusConnection *connection,char *timestr) {
	if(NULL == connection || NULL == timestr) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SET_TIME);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &timestr,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_add_syslog_rule(DBusConnection *connection, struct syslogrule_st *rule,int opt_type) {

	if(NULL == connection || NULL == rule) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	char *udpstr = rule->udpstr;	
	char *ipstr =  rule->ipstr;
	char *portstr =  rule->portstr;		
	char *filter =  rule->filter;
	char *flevel =  rule->flevel;	
	char *id =  rule->id;

	int opt = 0;
	opt = opt_type;
	dbus_error_init(&err);
	
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_ADD_SYSLOGRULE);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &udpstr,
							DBUS_TYPE_STRING, &ipstr,
							DBUS_TYPE_STRING, &portstr,
							DBUS_TYPE_STRING, &filter,
							DBUS_TYPE_STRING, &flevel,
							DBUS_TYPE_STRING, &id,
							DBUS_TYPE_INT32,&opt,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_set_syslogstatus_rule(DBusConnection *connection,char *status,char *opt_type) {
	if(NULL == connection || NULL == status) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SET_SYSLOG_STATUS);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &status,
							DBUS_TYPE_STRING, &opt_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

int 
ac_manage_show_syslog_rule(DBusConnection *connection, u_long *service_status, struct syslogrule_st **rule_array, u_long *rule_num) {


	if(NULL == connection || NULL == rule_array || NULL == rule_num) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	*rule_array = NULL;
	*rule_num = 0;

	struct syslogrule_st *temp_array = NULL;
	u_long temp_num = 0;
	int ret = AC_MANAGE_DBUS_ERROR;
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SHOW_SYSLOG_RULE);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_iter_next(&iter);  
	if(service_status) {
		dbus_message_iter_get_basic(&iter, service_status);
	}	

	dbus_message_iter_next(&iter);  
	dbus_message_iter_get_basic(&iter, &temp_num);

	dbus_message_iter_next(&iter);  
	dbus_message_iter_recurse(&iter,&iter_array);   
	
	if(AC_MANAGE_SUCCESS == ret && temp_num) {
		temp_array = (struct syslogrule_st *)calloc(temp_num, sizeof(struct syslogrule_st));
		if(NULL == temp_array) {
			dbus_message_unref(reply);
			return AC_MANAGE_MALLOC_ERROR;
		}

		int i = 0;
		for(i = 0; i < temp_num; i++) {

	            struct syslogrule_st tempRule = { 0 };
			    char *udpstr = NULL;
				char *ipstr = NULL;
				char *portstr = NULL;
				char *flevel = NULL;
				char *id = NULL;
	                
	            dbus_message_iter_recurse(&iter_array, &iter_struct);
	            dbus_message_iter_get_basic(&iter_struct, &udpstr);

				strncpy(tempRule.udpstr,udpstr,sizeof(tempRule.udpstr)-1);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &ipstr);

				strncpy(tempRule.ipstr,ipstr,sizeof(tempRule.ipstr)-1);
				

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &portstr);

				strncpy(tempRule.portstr,portstr,sizeof(tempRule.portstr)-1);
	            
	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &flevel);

				strncpy(tempRule.flevel,flevel,sizeof(tempRule.flevel)-1);

	            dbus_message_iter_next(&iter_struct);  
	            dbus_message_iter_get_basic(&iter_struct, &id);

				strncpy(tempRule.id,id,sizeof(tempRule.id)-1);

	            ntp_syslog_copy(&temp_array[i], &tempRule);
					
	            dbus_message_iter_next(&iter_array);
		}
        
		*rule_array = temp_array;
		*rule_num = temp_num;
	}

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_save_syslog_rule(DBusConnection *connection,char *status,char *opt_type) {
	if(NULL == connection || NULL == status || NULL == opt_type) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;
	dbus_error_init(&err);

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SAVE_SYSLOG);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &status,
							DBUS_TYPE_STRING, &opt_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}


int
ac_manage_config_syslog_rule(DBusConnection *connection, struct syslogrule_st *rule,int oper,int opt_type) {

	if(NULL == connection || NULL == rule) {
		return  AC_MANAGE_INPUT_TYPE_ERROR;
	}

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	char *ipstr =  rule->ipstr? rule->ipstr : "";
	char *portstr =  rule->portstr? rule->portstr: "";		
	char *filter =  rule->filter? rule->filter : "";
	char *rulename = rule->rulename? rule->rulename : "";
	char *keyword = rule->keyword? rule->keyword : "";
	dbus_error_init(&err);
	
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_CONFIG_SYSLOGRULE);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &rulename,
							DBUS_TYPE_STRING, &ipstr,
							DBUS_TYPE_STRING, &portstr,
							DBUS_TYPE_STRING, &filter,
							DBUS_TYPE_STRING, &keyword,
							DBUS_TYPE_INT32,&oper,
							DBUS_TYPE_INT32,&opt_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	dbus_message_unref(query);

	if(NULL == reply) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return AC_MANAGE_DBUS_ERROR;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	dbus_message_unref(reply);

	return ret;
}

