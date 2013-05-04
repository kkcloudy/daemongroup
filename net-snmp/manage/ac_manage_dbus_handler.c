#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dbus/dbus.h>

#include "manage_log.h"

#include "ws_dbus_def.h"
#include "ac_manage_def.h"

#include "ws_dbus_list_interface.h"
#include "ac_manage_public.h"

#include "ws_snmpd_engine.h"
#include "ac_manage_snmp_config.h"

#include "ws_snmpd_manual.h"
#include "ws_intf.h"
#include "ac_manage_mib_manual.h"

#include "eag_errcode.h"
#include "eag_conf.h"
#include "eag_interface.h"
#include "ac_manage_sample_interface.h"
#include "ac_manage_sample.h"

#include "ac_manage_extend.h"

#include "ws_tcrule.h"
#include "ac_manage_tcrule.h"

#include "ws_firewall.h"
#include "ac_manage_firewall.h"

#include "ws_log_conf.h"
#include "ws_webservice_conf.h"
#include "ac_manage_ntpsyslog.h"

#include "ac_manage_dbus_handler.h"
#include "ws_dbus_list.h"
#include "ws_acinfo.h"
#include "board/board_define.h"

unsigned int snmp_collect_mode = 0;

DBusMessage *
ac_manage_dbus_config_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int state = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
									DBUS_TYPE_UINT32, &state,
									DBUS_TYPE_INVALID))){

		manage_log(LOG_WARNING, "Unable to get input args\n");

		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(1 == state) {
		setlogmask(LOG_ON_DEBUG);
		manage_debug_config(MANAGE_LOG_DEBUG_ON);
		manage_log(LOG_INFO, "Ac Manage log on debug\n");
	}
	else if(0 == state) {
		setlogmask(LOG_ON_INFO);
		manage_debug_config(MANAGE_LOG_DEBUG_OFF);
		manage_log(LOG_INFO, "Ac Manage log off debug\n");
	}
	else {
		ret = AC_MANAGE_INPUT_TYPE_ERROR;
	}

	reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}


DBusMessage *
ac_manage_dbus_config_token_debug(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int token = 0;
	unsigned int state = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
									DBUS_TYPE_UINT32, &token,
									DBUS_TYPE_UINT32, &state,
									DBUS_TYPE_INVALID))){

		manage_log(LOG_WARNING, "Unable to get input args\n");

		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(1 == state) {
		ret = manage_token_register(token);
		manage_log(LOG_INFO, "manage register token %d\n", token);
	}
	else if(0 == state) {
		ret = manage_token_unregister(token);
		manage_log(LOG_INFO, "manage unregister token %d\n", token);
	}
	else {
		ret = AC_MANAGE_INPUT_TYPE_ERROR;
	}

	reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}


DBusMessage *
ac_manage_dbus_config_snmp_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int debugLevel = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &debugLevel,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    ret = snmp_set_debug_level(debugLevel);

    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}


DBusMessage *
ac_manage_dbus_config_trap_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int debugLevel = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &debugLevel,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    ret = trap_set_debug_level(debugLevel);

    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}

DBusMessage *
ac_manage_dbus_show_snmp_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int debugLevel = 0;

    snmp_show_debug_level(&debugLevel);

    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &debugLevel); 

	return reply;
}


DBusMessage *
ac_manage_dbus_show_trap_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int debugLevel = 0;

    trap_show_debug_level(&debugLevel);

    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &debugLevel); 

	return reply;
}


DBusMessage *
ac_manage_dbus_config_snmp_manual_instance(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    unsigned int local_id = 0;
    unsigned int instance_id = 0;
    unsigned int status = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &local_id,
                                DBUS_TYPE_UINT32, &instance_id,
                                DBUS_TYPE_UINT32, &status,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    ret = snmp_manual_set_instance_status(local_id, instance_id, status);	

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}

DBusMessage *
ac_manage_dbus_show_snmp_manual_instance(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    unsigned int manual_instance = 0;

    int ret = snmp_show_manual_set_instance_master(&manual_instance);
        
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append(reply, &iter);   

    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret); 
    
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &manual_instance);
    
    return reply;        
}


DBusMessage *
ac_manage_dbus_config_snmp_service(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int state = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    ret = snmp_config_service(state);
	

    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}

DBusMessage *
ac_manage_dbus_config_snmp_collection_mode(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    unsigned int collection_mode;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &collection_mode,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    STSNMPSysInfo *snmp_info = NULL;
    ret = snmp_show_sysinfo(&snmp_info);
    if(AC_MANAGE_SUCCESS == ret && snmp_info) {

        if(collection_mode == snmp_info->collection_mode) {
            ret = AC_MANAGE_CONFIG_EXIST;
        }
        else {
            snmp_info->collection_mode = collection_mode;
            ret = snmp_set_sysinfo(snmp_info);

            if(snmp_show_service_state()) { /*if snmp service enable */
                snmp_config_service(0);  /*disable snmp service */
            }		

            uninit_snmp_config(); /*uninit snmp config */
        }
    }
    
    MANAGE_FREE(snmp_info);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}

DBusMessage *
ac_manage_dbus_proxy_pfm_config(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    struct pfmOptParameter pfmParameter = { 0 };
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_INT32,  &pfmParameter.pfm_opt,    /*0:add , 1:delete*/
                                DBUS_TYPE_INT32,  &pfmParameter.pfm_opt_para,
                                DBUS_TYPE_UINT16, &pfmParameter.pfm_protocol,
                                DBUS_TYPE_STRING, &pfmParameter.ifName,
                                DBUS_TYPE_UINT32, &pfmParameter.src_port,
                                DBUS_TYPE_UINT32, &pfmParameter.dest_port,
                                DBUS_TYPE_STRING, &pfmParameter.src_ipaddr,
                                DBUS_TYPE_STRING, &pfmParameter.dest_ipaddr,
                                DBUS_TYPE_INT32,  &pfmParameter.slot_id, 
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "ac_manage_dbus_proxy_pfm_config: Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    manage_log(LOG_DEBUG, "ac_manage_dbus_proxy_pfm_config: src_port = %d, dest_port = %d, ifName = %s, slot_id = %d, state = %d\n", 
                        pfmParameter.src_port, pfmParameter.dest_port, pfmParameter.ifName, pfmParameter.slot_id, pfmParameter.pfm_opt);

    ret = manage_config_pfm_table_entry(&pfmParameter);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;

}

DBusMessage *
ac_manage_dbus_config_snmp_pfm_requestpkts(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    unsigned int port = 0;
    unsigned int state = 0;
    char *ifName = NULL;
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &ifName, 
								DBUS_TYPE_UINT32, &port,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    manage_log(LOG_DEBUG, "ac_manage_dbus_config_snmp_pfm_requestpkts: port = %d, ifName = %s, state = %d\n", port, ifName, state);
    if(!port || port > 65535) {
        ret = AC_MANAGE_INPUT_TYPE_ERROR;
    }    
    else if(!snmp_show_service_state()) {
        
    	STSNMPSysInfo *snmp_info = NULL;
    	ret = snmp_show_sysinfo(&snmp_info);
    	if(AC_MANAGE_SUCCESS == ret && snmp_info) {
    	    if(state) { /*add pfm interface*/
        	    if(snmp_info->agent_port){
                    if(snmp_info->agent_port != port) {
                        ret = AC_MANAGE_CONFIG_FAIL;
                        goto END_REPLY;
                    }    
                }
                
                ret = snmp_add_pfm_interface(ifName, port);
                if(AC_MANAGE_SUCCESS == ret) {
                    snmp_info->agent_port = port;                    
                    snmp_set_sysinfo(snmp_info);       
                }
            }  
            else { /*delete pfm interface*/
        	    if(snmp_info->agent_port){
        	        if(snmp_info->agent_port != port) {
                        ret = AC_MANAGE_CONFIG_FAIL;
                        goto END_REPLY;
        	        }
        	        else {
                        ret = snmp_del_pfm_interface(ifName, port);
                    }        
        	    }
        	    else {
                    ret = AC_MANAGE_CONFIG_NONEXIST;
                    goto END_REPLY;
        	    }
            }
    	}
    	
    END_REPLY:
    	MANAGE_FREE(snmp_info);
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}



DBusMessage *
ac_manage_dbus_config_snmp_version_mode(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    int mode, state;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_UINT32, &mode,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {
        
    	STSNMPSysInfo *snmp_info = NULL;
    	ret = snmp_show_sysinfo(&snmp_info);
    	if(AC_MANAGE_SUCCESS == ret && snmp_info) {
    	
        	if(1 == mode) {
        		snmp_info->v1_status = state;
        	}
        	else if(2 == mode) {
        		snmp_info->v2c_status = state;
        	}
        	else if(3 == mode) {
        		snmp_info->v3_status = state;
        	}	
        	
            ret = snmp_set_sysinfo(snmp_info);
    	}

    	MANAGE_FREE(snmp_info);
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}

DBusMessage *
ac_manage_dbus_config_snmp_update_sysinfo(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	int ret = AC_MANAGE_SUCCESS;

    if(!snmp_show_service_state()) {
        update_snmp_sysinfo();
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }

    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}


DBusMessage *
ac_manage_dbus_config_snmp_cachetime(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    unsigned int cachetime = 300;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &cachetime,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {
        
    	STSNMPSysInfo *snmp_info = NULL;
    	ret = snmp_show_sysinfo(&snmp_info);
    	if(AC_MANAGE_SUCCESS == ret && snmp_info) {
    	
            snmp_info->cache_time = cachetime;
            ret = snmp_set_sysinfo(snmp_info);
    	}

    	MANAGE_FREE(snmp_info);
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}

DBusMessage *
ac_manage_dbus_config_snmp_add_community(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *community = NULL;
    char *ip_addr = NULL;
    char *ip_mask = NULL;
    unsigned int access_mode = 1;
    unsigned int state = 1;
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &community,
	                            DBUS_TYPE_STRING, &ip_addr,   
	                            DBUS_TYPE_STRING, &ip_mask,
								DBUS_TYPE_UINT32, &access_mode,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {
        
        if(NULL == community || 0 == strcmp(community, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            
            STCommunity pcommunity;
            memset(&pcommunity, 0, sizeof(STCommunity));
            
            strncpy(pcommunity.community, community, sizeof(pcommunity.community) - 1);
            strncpy(pcommunity.ip_addr, ip_addr, sizeof(pcommunity.ip_addr) - 1);
            strncpy(pcommunity.ip_mask, ip_mask, sizeof(pcommunity.ip_mask) - 1);
                   
            if(access_mode) {
                pcommunity.access_mode = ACCESS_MODE_RW; //community mode rw
            }
            else {
                pcommunity.access_mode = ACCESS_MODE_RO; //community mode ro
            }

            if(state) {
                pcommunity.status = RULE_ENABLE; // community enable
            }
            else {
                pcommunity.status = RULE_DISABLE; // community disable
            }
            
            ret = snmp_add_community(pcommunity);
                        
        }  
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;        
}



DBusMessage *
ac_manage_dbus_config_snmp_set_community(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *old_community = NULL;
    char *community = NULL;
    char *ip_addr = NULL;
    char *ip_mask = NULL;
    unsigned int access_mode = 1;
    unsigned int state = 1;
    
	int ret = AC_MANAGE_SUCCESS;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &old_community,
	                            DBUS_TYPE_STRING, &community,
	                            DBUS_TYPE_STRING, &ip_addr,   
	                            DBUS_TYPE_STRING, &ip_mask,
								DBUS_TYPE_UINT32, &access_mode,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    
//    if(!snmp_show_service_state()) {
        
        if(NULL == old_community || 0 == strcmp(old_community, "") ||
            NULL == community || 0 == strcmp(community, "")) {
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            STCommunity pcommunity;
            memset(&pcommunity, 0, sizeof(STCommunity));
            
            strncpy(pcommunity.community, community, sizeof(pcommunity.community) - 1);
            strncpy(pcommunity.ip_addr, ip_addr, sizeof(pcommunity.ip_addr) - 1);
            strncpy(pcommunity.ip_mask, ip_mask, sizeof(pcommunity.ip_mask) - 1);            
                   
            if(access_mode) {
                pcommunity.access_mode = ACCESS_MODE_RW; //community mode rw
            }
            else {
                pcommunity.access_mode = ACCESS_MODE_RO; //community mode ro
            }

            if(state) {
                pcommunity.status = RULE_ENABLE; // community enable
            }
            else {
                pcommunity.status = RULE_DISABLE; // community disable
            }
            
            ret = snmp_set_community(old_community, pcommunity);      
        }  
//    }
//    else {
//        ret = AC_MANAGE_SERVICE_ENABLE;
//    }
        
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;        
}

DBusMessage *
ac_manage_dbus_config_snmp_del_community(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *community = NULL;
    
	int ret = AC_MANAGE_SUCCESS;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &community,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {

        if(NULL == community || 0 == strcmp(community, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            ret = snmp_del_community(community);
        }        
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
        
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_config_snmp_view(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *view_name = NULL;
    unsigned int mode = 0;
	int ret = AC_MANAGE_SUCCESS;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &view_name,
	                            DBUS_TYPE_UINT32, &mode,    /*create or delete view*/
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {

        if(NULL == view_name || 0 == strcmp(view_name, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            if(1 == mode) {
                if(0 == strcmp(view_name, "all")) {
                    ret = AC_MANAGE_INPUT_TYPE_ERROR;
                }
                
                ret = snmp_create_view(view_name);
            }
            else if( 0 == mode){
                ret = snmp_del_view(view_name);
            }
            else {
                ret = snmp_check_view(view_name);
            }
        }        
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
        
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_check_snmp_view(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *view_name = NULL;
	int ret = AC_MANAGE_SUCCESS;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &view_name,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {

        if(NULL == view_name || 0 == strcmp(view_name, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {

            ret = snmp_check_view(view_name);
        }        
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
        
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_config_snmp_view_oid(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *view_name = NULL;
    char *oid = NULL;
    unsigned int oid_type = 0;
    unsigned int mode =  0;
    
	int ret = AC_MANAGE_SUCCESS;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &view_name,
	                            DBUS_TYPE_STRING, &oid,
	                            DBUS_TYPE_UINT32, &oid_type,    /* include or exclude oid */
	                            DBUS_TYPE_UINT32, &mode,    /*delete or add oid */
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    if(!snmp_show_service_state()) {

        if(NULL == view_name || 0 == strcmp(view_name, "") 
            || NULL == oid || 0 == strcmp(oid, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            if(mode) {
                ret = snmp_add_view_oid(view_name, oid, oid_type); /*mode : 1 include ; 0 exclude*/
            }
            else {
                ret = snmp_del_view_oid(view_name, oid, oid_type); /*mode : 1 include ; 0 exclude*/
            }
        }        
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
        
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_config_snmp_add_group(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;
    char *group_name = NULL;
    char *view_name = NULL;
    unsigned int access_mode = 0;
    unsigned int sec_level = 0;
    
	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &group_name,
	                            DBUS_TYPE_STRING, &view_name,
	                            DBUS_TYPE_UINT32, &access_mode,
	                            DBUS_TYPE_UINT32, &sec_level,    
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {
        if(NULL == group_name || NULL == view_name || 0 == strcmp(group_name, "") || 0 == strcmp(view_name, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            ret = snmp_add_group(group_name, view_name, access_mode, sec_level);
        }    
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }    

    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}

DBusMessage *
ac_manage_dbus_config_snmp_del_group(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;
    char *group_name = NULL;
    
	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &group_name,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {
        if(NULL == group_name || 0 == strcmp(group_name, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            ret = snmp_del_group(group_name);
        }    
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }    

    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}

DBusMessage *
ac_manage_dbus_config_snmp_add_v3user(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;
    char *v3user_name = NULL;
    char *group_name = NULL;
    char *auth_key = NULL;
    char *priv_key = NULL;
    unsigned int auth_type = 0;
    unsigned int priv_type = 0;
    unsigned int state = 0;
    
	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &v3user_name,
                                DBUS_TYPE_UINT32, &auth_type,
                                DBUS_TYPE_STRING, &auth_key,
                                DBUS_TYPE_UINT32, &priv_type,
                                DBUS_TYPE_STRING, &priv_key,
                                DBUS_TYPE_STRING, &group_name,
                                DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {
        if(NULL == v3user_name || 0 == strlen(v3user_name) || NULL == group_name || 0 == strlen(group_name)
            || NULL == auth_key || NULL == priv_key){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            ret = snmp_add_v3user(v3user_name, auth_type, auth_key, priv_type, priv_key, group_name, state);
        }    
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }    

    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}

DBusMessage *
ac_manage_dbus_config_snmp_del_v3user(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;
    char *v3user_name = NULL;
    
	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &v3user_name,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!snmp_show_service_state()) {
        if(NULL == v3user_name || 0 == strcmp(v3user_name, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            ret = snmp_del_v3user(v3user_name);
        }    
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }    

    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}


DBusMessage *
ac_manage_dbus_show_snmp_state(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

    int ret = AC_MANAGE_SUCCESS;

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);

    unsigned int snmp_state = 0;
    if(snmp_show_service_state()) {
        snmp_state = 1;
    }

	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &snmp_state);
    								     								     
    return reply;        
}



DBusMessage *
ac_manage_dbus_show_snmp_base_info(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;

    int ret = AC_MANAGE_SUCCESS;

    STSNMPSysInfo *snmp_info = NULL;
    ret = snmp_show_sysinfo(&snmp_info);
    manage_log(LOG_DEBUG, "after snmp_show_sysinfo, ret = %d\n", ret);

    char *sys_name = (char *)malloc(MAX_SYSTEM_NAME_LEN);
    char *sys_description = (char *)malloc(MAX_SYSTEM_DESCRIPTION);
    char *sys_oid = (char *)malloc(MAX_OID_LEN);

    if(!sys_name || !sys_description || !sys_oid) {
        ret = AC_MANAGE_MALLOC_ERROR;
    }

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    if(AC_MANAGE_SUCCESS == ret) {

        memset(sys_name, 0, MAX_SYSTEM_NAME_LEN);
        memcpy(sys_name, snmp_info->sys_name, MAX_SYSTEM_NAME_LEN);

        memset(sys_description, 0, MAX_SYSTEM_DESCRIPTION);
        memcpy(sys_description, snmp_info->sys_description, MAX_SYSTEM_DESCRIPTION);

        
        memset(sys_oid, 0, MAX_OID_LEN);
        memcpy(sys_oid, snmp_info->sys_oid, MAX_OID_LEN);
          
        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_STRING,
        								 &sys_name);
    
        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_STRING,
        								 &sys_description);
        
        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_STRING,
        								 &sys_oid);
        								 
        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_UINT32,
        								 &snmp_info->agent_port);
        								 
        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_UINT32,
        								 &snmp_info->collection_mode);
        								 
        dbus_message_iter_append_basic(&iter,
                                         DBUS_TYPE_UINT32,
                                         &snmp_info->cache_time);
                                         
        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_UINT32,
        								 &snmp_info->v1_status);

        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_UINT32,
        								 &snmp_info->v2c_status);

        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_UINT32,
        								 &snmp_info->v3_status); 								 

    }

    MANAGE_FREE(sys_name);
    MANAGE_FREE(sys_description);
    MANAGE_FREE(sys_oid);
    MANAGE_FREE(snmp_info);
    
    return reply;        
}


DBusMessage *
ac_manage_dbus_show_snmp_pfm_interface(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;
    
    int ret = AC_MANAGE_SUCCESS;
    
    STSNMPSysInfo *snmp_info = NULL;
    SNMPINTERFACE *interface_array = NULL;
    unsigned int interface_num = 0;
        
    char *ifName = NULL;
    unsigned int agent_port = 0;
    
    ret = snmp_show_sysinfo(&snmp_info);
    if(AC_MANAGE_SUCCESS == ret) {

        agent_port = snmp_info->agent_port;
        manage_log(LOG_DEBUG, "after snmp_show_sysinfo, agent_port = %d\n", agent_port);
        
        ret = snmp_show_pfm_interface(&interface_array, &interface_num);
        manage_log(LOG_DEBUG, "after snmp_show_pfm_interface, ret = %d, interface_num = %d\n", ret, interface_num);

        ifName = (char *)malloc(MAX_INTERFACE_NAME_LEN);

        if(NULL == ifName) {
            ret = AC_MANAGE_MALLOC_ERROR;
        }
    }    
    
    
    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &agent_port); 								 
    
    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &interface_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                    
                                        DBUS_TYPE_STRING_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
	if(AC_MANAGE_SUCCESS == ret) {   

        int i = 0;
        for(i = 0; i < interface_num; i++) {

            memset(ifName, 0, MAX_INTERFACE_NAME_LEN);
            strncpy(ifName, interface_array[i].ifName, MAX_INTERFACE_NAME_LEN - 1);


    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &ifName);

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array); 
    
    MANAGE_FREE(ifName);
    MANAGE_FREE(interface_array);
    MANAGE_FREE(snmp_info);
    
    return reply;        
}


DBusMessage *
ac_manage_dbus_show_snmp_community(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;

    int ret = AC_MANAGE_SUCCESS;
    
    STCommunity *community_array = NULL;
    unsigned int community_num = 0;
    ret = snmp_show_community(&community_array, &community_num);
    manage_log(LOG_DEBUG, "after snmp_show_community, ret = %d, community_num = %d\n", ret, community_num);

    char *community = (char *)malloc(MAX_SNMP_NAME_LEN);
    char *ip_addr = (char *)malloc(MAX_IP_ADDR_LEN);
    char *ip_mask = (char *)malloc(MAX_IP_ADDR_LEN);

    if(!community || !ip_addr || !ip_mask) {
        ret = AC_MANAGE_MALLOC_ERROR;
    }
    
    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &community_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                    
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
	if(AC_MANAGE_SUCCESS == ret) {   

        int i = 0;
        for(i = 0; i < community_num; i++) {

            memset(community, 0, MAX_SNMP_NAME_LEN);
            memcpy(community, community_array[i].community, MAX_SNMP_NAME_LEN);

            memset(ip_addr, 0, MAX_IP_ADDR_LEN);
            memcpy(ip_addr, community_array[i].ip_addr, MAX_IP_ADDR_LEN);

            memset(ip_mask, 0, MAX_IP_ADDR_LEN);        
            memcpy(ip_mask, community_array[i].ip_mask, MAX_IP_ADDR_LEN);

    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &community);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &ip_addr);  

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &ip_mask); 

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &community_array[i].access_mode);  

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &community_array[i].status);

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);    

    MANAGE_FREE(community);
    MANAGE_FREE(ip_addr);
    MANAGE_FREE(ip_mask);
    MANAGE_FREE(community_array);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_show_snmp_view(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	

    int ret = AC_MANAGE_SUCCESS;

    unsigned int mode = 0;
    char *view_name = NULL;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
                                DBUS_TYPE_UINT32, &mode,    /*mode : 0 all, 1 view_name */
	                            DBUS_TYPE_STRING, &view_name,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    STSNMPView *view_array = NULL;
    unsigned int view_num = 0;

    if(mode) {
        ret = snmp_show_view(&view_array, &view_num, view_name);
    }
    else {
        ret = snmp_show_view(&view_array, &view_num, NULL);
    }

    char *temp_name = (char *)malloc(MAX_VIEW_NAME_LEN);
    char *oid = (char *)malloc(MAX_oid);
    if(NULL == temp_name || NULL == oid)
        ret = AC_MANAGE_MALLOC_ERROR;
    

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &view_num);

	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING

                                        DBUS_TYPE_ARRAY_AS_STRING
                                        DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                            DBUS_TYPE_STRING_AS_STRING
                                        DBUS_STRUCT_END_CHAR_AS_STRING
                                        
                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);

	if(AC_MANAGE_SUCCESS == ret && view_num > 0) {   
        DBusMessageIter  iter_struct;
        DBusMessageIter  iter_sub_array;
        
        int i = 0;
        for(i = 0; i < view_num; i++) {

            memset(temp_name, 0, MAX_VIEW_NAME_LEN);
            memcpy(temp_name, view_array[i].name, MAX_VIEW_NAME_LEN);

    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &temp_name);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(view_array[i].view_included.oid_num)); 

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(view_array[i].view_excluded.oid_num));  

            dbus_message_iter_open_container (&iter_struct,
                                               DBUS_TYPE_ARRAY,
                                               DBUS_STRUCT_BEGIN_CHAR_AS_STRING  
                                                  DBUS_TYPE_BYTE_AS_STRING
                                               DBUS_STRUCT_END_CHAR_AS_STRING, 
                                               &iter_sub_array);

            int j = 0;
            struct oid_list *oid_node = NULL;
            for(j = 0, oid_node = view_array[i].view_included.oidHead;
                j < view_array[i].view_included.oid_num && NULL != oid_node; 
                j++, oid_node = oid_node->next) {
            
                DBusMessageIter iter_sub_struct;
                dbus_message_iter_open_container(&iter_sub_array,
                                                    DBUS_TYPE_STRUCT,
                                                    NULL,
                                                    &iter_sub_struct);


                memset(oid, 0, MAX_oid);
                memcpy(oid, oid_node->oid, MAX_oid);
                                                    
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_STRING,
                                                &oid);

                dbus_message_iter_close_container(&iter_sub_array, &iter_sub_struct);
            }
            
            for(j = 0, oid_node = view_array[i].view_excluded.oidHead;
                j < view_array[i].view_excluded.oid_num && NULL != oid_node; 
                j++, oid_node = oid_node->next) {
            
                DBusMessageIter iter_sub_struct;
                dbus_message_iter_open_container(&iter_sub_array,
                                                    DBUS_TYPE_STRUCT,
                                                    NULL,
                                                    &iter_sub_struct);


                memset(oid, 0, MAX_oid);
                memcpy(oid, oid_node->oid, MAX_oid);
                                                    
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_STRING,
                                                &oid);

                dbus_message_iter_close_container(&iter_sub_array, &iter_sub_struct);
            }

            dbus_message_iter_close_container(&iter_struct, &iter_sub_array);

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);  
    
    MANAGE_FREE(temp_name);
    MANAGE_FREE(oid);
    free_snmp_show_view(&view_array, view_num);
    
    return reply;        
}


DBusMessage *
ac_manage_dbus_show_snmp_group(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	

    int ret = AC_MANAGE_SUCCESS;

    unsigned int mode = 0;
    char *group_name = NULL;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
                                DBUS_TYPE_UINT32, &mode,    /*mode : 0 all, 1 group_name */
	                            DBUS_TYPE_STRING, &group_name,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    STSNMPGroup *group_array = NULL;
    unsigned int group_num = 0;

    if(mode) {
        ret = snmp_show_group(&group_array, &group_num, group_name);
    }
    else {
        ret = snmp_show_group(&group_array, &group_num, NULL);
    }

    char *temp_group_name = (char *)malloc(MAX_GROUP_NAME_LEN);
    char *temp_view_name = (char *)malloc(MAX_VIEW_NAME_LEN);
    if(NULL == temp_group_name || NULL == temp_view_name)
        ret = AC_MANAGE_MALLOC_ERROR;
    

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &group_num);

	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);

	if(AC_MANAGE_SUCCESS == ret && group_num) {   	    
        DBusMessageIter  iter_struct;    
        
        int i = 0;
        for(i = 0; i < group_num; i++) {

            memset(temp_group_name, 0, MAX_GROUP_NAME_LEN);
            memcpy(temp_group_name, group_array[i].group_name, MAX_GROUP_NAME_LEN);

            memset(temp_view_name, 0, MAX_VIEW_NAME_LEN);
            memcpy(temp_view_name, group_array[i].group_view, MAX_VIEW_NAME_LEN);

    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &temp_group_name);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &temp_view_name);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(group_array[i].access_mode)); 

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(group_array[i].sec_level));  

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);  
    
    MANAGE_FREE(temp_group_name);
    MANAGE_FREE(temp_view_name);
    MANAGE_FREE(group_array);
    
    return reply;        
}


DBusMessage *
ac_manage_dbus_show_snmp_v3user(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	

    int ret = AC_MANAGE_SUCCESS;

    unsigned int mode = 0;
    char *v3user_name = NULL;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
                                DBUS_TYPE_UINT32, &mode,    /*mode : 0 all, 1 v3user_name */
	                            DBUS_TYPE_STRING, &v3user_name,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    STSNMPV3User *v3user_array = NULL;
    unsigned int v3user_num = 0;

    if(mode) {
        ret = snmp_show_v3user(&v3user_array, &v3user_num, v3user_name);
    }
    else {
        ret = snmp_show_v3user(&v3user_array, &v3user_num, NULL);
    }

    char *temp_v3user_name = (char *)malloc(MAX_SNMP_NAME_LEN);
    char *temp_group_name = (char *)malloc(MAX_GROUP_NAME_LEN);
    char *temp_auth_passwd = (char *)malloc(MAX_SNMP_PASSWORD_LEN);
    char *temp_priv_passwd = (char *)malloc(MAX_SNMP_PASSWORD_LEN);
    if(NULL == temp_v3user_name || NULL == temp_group_name || NULL == temp_auth_passwd || NULL == temp_priv_passwd)
        ret = AC_MANAGE_MALLOC_ERROR;
    

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &v3user_num);

	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);

	if(AC_MANAGE_SUCCESS == ret && v3user_num) {   	    
        DBusMessageIter  iter_struct;    
        
        int i = 0;
        for(i = 0; i < v3user_num; i++) {

            memset(temp_v3user_name, 0, MAX_SNMP_NAME_LEN);
            memcpy(temp_v3user_name, v3user_array[i].name, MAX_SNMP_NAME_LEN);

            memset(temp_group_name, 0, MAX_GROUP_NAME_LEN);
            memcpy(temp_group_name, v3user_array[i].group_name, MAX_GROUP_NAME_LEN);

            memset(temp_auth_passwd, 0, MAX_SNMP_PASSWORD_LEN);
            memcpy(temp_auth_passwd, v3user_array[i].authentication.passwd, MAX_SNMP_PASSWORD_LEN);

            memset(temp_priv_passwd, 0, MAX_SNMP_PASSWORD_LEN);
            memcpy(temp_priv_passwd, v3user_array[i].privacy.passwd, MAX_SNMP_PASSWORD_LEN);
            

    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &temp_v3user_name);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(v3user_array[i].authentication.protocal)); 
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &temp_auth_passwd);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(v3user_array[i].privacy.protocal)); 
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &temp_priv_passwd);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &temp_group_name);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(v3user_array[i].status));  

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);  
    
    MANAGE_FREE(temp_v3user_name);
    MANAGE_FREE(temp_group_name);
    MANAGE_FREE(temp_auth_passwd);
    MANAGE_FREE(temp_priv_passwd);
    MANAGE_FREE(v3user_array);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_show_snmp_running_config(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

    unsigned int mode = 0;
    int ret = AC_MANAGE_SUCCESS;
    unsigned int moreConfig = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_UINT32, &mode,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    struct running_config *configHead = NULL;
    ret = snmp_show_running_config(&configHead, mode);
    
    char *showStr = (char *)malloc(256);
    if(NULL == showStr) {
        ret = AC_MANAGE_MALLOC_ERROR;
    }    

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    if(AC_MANAGE_SUCCESS == ret && configHead) {
    
        moreConfig = 1;
        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_UINT32,
        								 &moreConfig);

        struct running_config *configNode = NULL;
        for(configNode = configHead; NULL != configNode; configNode = configNode->next) {

            memset(showStr, 0, 256);
            memcpy(showStr, configNode->showStr, 256);

    	    dbus_message_iter_append_basic(&iter,
            								 DBUS_TYPE_STRING,
            								 &showStr);

            if(NULL != configNode->next) {
                dbus_message_iter_append_basic(&iter,
                								 DBUS_TYPE_UINT32,
                								 &moreConfig);  
            }        								 
        }        								 
        moreConfig = 0;
    }

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &moreConfig);

    MANAGE_FREE(showStr);
    manage_free_running_config(&configHead);
    
    return reply;        
}




DBusMessage *
ac_manage_dbus_config_trap_service(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int state = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    ret = trap_config_service(state);
	

    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}


DBusMessage *
ac_manage_dbus_config_trap_config_receiver(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    unsigned int local_id = 0;
    unsigned int instance_id = 0;
    unsigned int dest_port = 162;
    unsigned int version = 1;
    unsigned int state = 1;
    unsigned int type = 0;

    char *receName = NULL;
    char *sour_ipAddr = NULL;
    char *dest_ipAddr = NULL;
    char *trapcom = NULL;
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
                                DBUS_TYPE_UINT32, &local_id,
                                DBUS_TYPE_UINT32, &instance_id,
                                DBUS_TYPE_UINT32, &version,
                                DBUS_TYPE_STRING, &receName,
                                DBUS_TYPE_STRING, &sour_ipAddr,
                                DBUS_TYPE_STRING, &dest_ipAddr,   
                                DBUS_TYPE_UINT32, &dest_port,
                                DBUS_TYPE_STRING, &trapcom,
                                DBUS_TYPE_UINT32, &state,
                                DBUS_TYPE_UINT32, &type,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!trap_show_service_state()) {

        if(local_id > 1 || 0 == instance_id || instance_id > 16 || dest_port > 65535)
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        else if(NULL == receName || 0 == strcmp(receName, "") || NULL == sour_ipAddr || NULL == dest_ipAddr || 
                0 == strcmp(dest_ipAddr, "") || NULL == trapcom || (3 != version && 0 == strcmp(trapcom, ""))){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            
            STSNMPTrapReceiver receiver;
            memset(&receiver, 0, sizeof(STSNMPTrapReceiver));
            
            strncpy(receiver.name, receName, sizeof(receiver.name) - 1);
            strncpy(receiver.sour_ipAddr, sour_ipAddr, sizeof(receiver.sour_ipAddr) - 1);
            strncpy(receiver.dest_ipAddr, dest_ipAddr, sizeof(receiver.dest_ipAddr) - 1);
            strncpy(receiver.trapcom, trapcom, sizeof(receiver.trapcom) - 1);

            receiver.local_id = local_id;
            receiver.instance_id = instance_id;
            receiver.dest_port = dest_port ? dest_port : 162;
            receiver.version = version;
            receiver.status = state;

            if(type) {
                ret = trap_add_receiver(&receiver);
            }
            else {
                ret = trap_set_receiver(&receiver);
            }
                        
        }  
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;        
}

DBusMessage *
ac_manage_dbus_config_trap_del_receiver(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *name = NULL;
    
	int ret = AC_MANAGE_SUCCESS;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &name,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!trap_show_service_state()) {

        if(NULL == name || 0 == strcmp(name, "")){
            ret = AC_MANAGE_INPUT_TYPE_ERROR;
        }
        else {
            ret = trap_del_receiver(name);
        }        
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
        
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_config_trap_switch(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	unsigned int index = 0;
	char *trapName = NULL;
	char *trapEDes = NULL;
	unsigned int state = 1;
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_STRING, &trapName,
								DBUS_TYPE_STRING, &trapEDes,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

		manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(!trap_show_service_state()) {
		ret = trap_set_switch(index, trapName, trapEDes, state);
	} else{
		ret = AC_MANAGE_SERVICE_ENABLE;
	}

	reply = dbus_message_new_method_return(message);

	dbus_message_iter_init_append (reply, &iter);   
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;        
}

DBusMessage *
ac_manage_dbus_config_trap_group_switch(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    struct trap_group_switch group_switch = { 0 };
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_UINT64, &group_switch.low_switch,
	                            DBUS_TYPE_UINT64, &group_switch.high_switch,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!trap_show_service_state()) {
        ret = trap_set_group_switch(&group_switch);
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;        
}



DBusMessage *
ac_manage_dbus_config_trap_instance_heartbeat(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    unsigned int local_id = 0;
    unsigned int instance_id = 0;
    char *ipAddr = NULL;
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_UINT32, &local_id,
	                            DBUS_TYPE_UINT32, &instance_id,
	                            DBUS_TYPE_STRING, &ipAddr,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(local_id >= VRRP_TYPE_NUM || instance_id > INSTANCE_NUM || 0 == instance_id
        || NULL == ipAddr || 0 == strlen(ipAddr)) {
        ret = AC_MANAGE_INPUT_TYPE_ERROR;
    }
    else {
        if(!trap_show_service_state()) {
            TRAPHeartbeatIP tempHeartbeat = { 0 };
            tempHeartbeat.local_id = local_id;
            tempHeartbeat.instance_id = instance_id;
            strncpy(tempHeartbeat.ipAddr, ipAddr, sizeof(tempHeartbeat.ipAddr) - 1);
            
            ret = trap_set_heartbeat_ip(&tempHeartbeat);
        }
        else {
            ret = AC_MANAGE_SERVICE_ENABLE;
        }
    }
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;        
}

DBusMessage *
ac_manage_dbus_clear_trap_instance_heartbeat(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    unsigned int local_id = 0;
    unsigned int instance_id = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &local_id,
                                DBUS_TYPE_UINT32, &instance_id,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(local_id >= VRRP_TYPE_NUM || instance_id > INSTANCE_NUM || 0 == instance_id) {
        ret = AC_MANAGE_INPUT_TYPE_ERROR;
    }
    else {
        if(!trap_show_service_state()) {
            ret = trap_clear_heartbeat_ip(local_id, instance_id);
        }
        else {
            ret = AC_MANAGE_SERVICE_ENABLE;
        }
    }

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}


DBusMessage *
ac_manage_dbus_config_trap_parameter(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *paraStr = NULL;
    unsigned int paraData = 0;
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &paraStr,
								DBUS_TYPE_UINT32, &paraData,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    if(!trap_show_service_state()) {
        if(NULL == paraStr || 0 == strlen(paraStr)) {
            return AC_MANAGE_INPUT_TYPE_ERROR;
        }
        ret = trap_set_parameter(paraStr, paraData);
    }
    else {
        ret = AC_MANAGE_SERVICE_ENABLE;
    }
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;        
}

DBusMessage *
ac_manage_dbus_show_trap_state(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

    int ret = AC_MANAGE_SUCCESS;

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);

	unsigned int trap_state = 0;
    if(trap_show_service_state()) {
        trap_state = 1;
    }
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &trap_state);

    return reply;        
}



DBusMessage *
ac_manage_dbus_show_trap_receiver(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;

    int ret = AC_MANAGE_SUCCESS;
    
    STSNMPTrapReceiver *receiver_array = NULL;
    unsigned int receiver_num = 0;
    ret = trap_show_receiver(&receiver_array, &receiver_num);
    manage_log(LOG_DEBUG, "after trap_show_receiver, ret = %d, receiver_num = %d\n", ret, receiver_num);

    char *name = (char *)malloc(MAX_SNMP_NAME_LEN);
    char *sour_ipAddr = (char *)malloc(MAX_IP_ADDR_LEN);
    char *dest_ipAddr = (char *)malloc(MAX_IP_ADDR_LEN);
    char *community = (char *)malloc(MAX_SNMP_NAME_LEN);

    if(!name || !sour_ipAddr || !dest_ipAddr || !community) {
        ret = AC_MANAGE_MALLOC_ERROR;
    }
    
    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &receiver_num);

	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
	if(AC_MANAGE_SUCCESS == ret) {   

        int i = 0;
        for(i = 0; i < receiver_num; i++) {
            
            memset(name, 0, MAX_SNMP_NAME_LEN);        
            memcpy(name, receiver_array[i].name, MAX_SNMP_NAME_LEN);
            
            memset(sour_ipAddr, 0, MAX_IP_ADDR_LEN);
            memcpy(sour_ipAddr, receiver_array[i].sour_ipAddr, MAX_IP_ADDR_LEN);

            memset(dest_ipAddr, 0, MAX_IP_ADDR_LEN);
            memcpy(dest_ipAddr, receiver_array[i].dest_ipAddr, MAX_IP_ADDR_LEN);
            
            memset(community, 0, MAX_SNMP_NAME_LEN);
            memcpy(community, receiver_array[i].trapcom, MAX_SNMP_NAME_LEN);


    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);
         										
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &receiver_array[i].local_id); 
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &receiver_array[i].instance_id);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &receiver_array[i].version);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &name);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &sour_ipAddr);  

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &dest_ipAddr);  

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &receiver_array[i].dest_port);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &community); 

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &receiver_array[i].status);  

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);    

    MANAGE_FREE(name);
    MANAGE_FREE(sour_ipAddr);
    MANAGE_FREE(dest_ipAddr);
    MANAGE_FREE(community);
    MANAGE_FREE(receiver_array);
    
    return reply;        
}


DBusMessage *
ac_manage_dbus_show_trap_switch(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;

    int ret = AC_MANAGE_SUCCESS;
    
    TRAP_DETAIL_CONFIG *trapDetail_array = NULL;
    unsigned int trapDetail_num = 0;
    ret = trap_show_switch(&trapDetail_array, &trapDetail_num);
    manage_log(LOG_DEBUG, "after trap_show_switch, ret = %d, trapDetail_num = %d\n", ret, trapDetail_num);

    char *trapName = (char *)malloc(128);
    char *trapEDes = (char *)malloc(128);

    if(!trapName || !trapEDes) {
        ret = AC_MANAGE_MALLOC_ERROR;
    }
    
    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &trapDetail_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
	if(AC_MANAGE_SUCCESS == ret) {   

        int i = 0;
        for(i = 0; i < trapDetail_num; i++) {
            
            memset(trapName, 0, 128);        
            memcpy(trapName, trapDetail_array[i].trapName, 128);
            manage_log(LOG_DEBUG, "trapDetail_array[i].trapName = %s\n", trapDetail_array[i].trapName);
            
            memset(trapEDes, 0, 128);
            memcpy(trapEDes, trapDetail_array[i].trapEDes, 128);
            manage_log(LOG_DEBUG, "trapDetail_array[i].trapEDes = %s\n", trapDetail_array[i].trapEDes);
            
    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);
 
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &trapName);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &trapEDes);
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(trapDetail_array[i].trapSwitch));   

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);    

    MANAGE_FREE(trapName);
    MANAGE_FREE(trapEDes);
    MANAGE_FREE(trapDetail_array);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_show_trap_instance_heartbeat(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;

    int ret = AC_MANAGE_SUCCESS;
    
    TRAPHeartbeatIP *heartbeat_array = NULL;
    unsigned int heartbeat_num = 0;
    ret = trap_show_heartbeat_ip(&heartbeat_array, &heartbeat_num);
    manage_log(LOG_DEBUG, "after trap_show_heartbeat_ip, ret = %d, heartbeat_num = %d\n", ret, heartbeat_num);

    char *ipAddr = (char *)malloc(MAX_IP_ADDR_LEN);
    if(NULL == ipAddr) {
        ret = AC_MANAGE_MALLOC_ERROR;
    }
    
    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &heartbeat_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
	if(AC_MANAGE_SUCCESS == ret) {   

        int i = 0;
        for(i = 0; i < heartbeat_num; i++) {
            
            memset(ipAddr, 0, MAX_IP_ADDR_LEN);        
            memcpy(ipAddr, heartbeat_array[i].ipAddr, MAX_IP_ADDR_LEN);
            manage_log(LOG_DEBUG, "heartbeat_array[i].ipAddr = %s\n", heartbeat_array[i].ipAddr);
            
    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);
 
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(heartbeat_array[i].local_id));

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(heartbeat_array[i].instance_id));
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &ipAddr);   

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);    

    MANAGE_FREE(ipAddr);
    MANAGE_FREE(heartbeat_array);
    
    return reply;        
}



DBusMessage *
ac_manage_dbus_show_trap_parameter(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;

    int ret = AC_MANAGE_SUCCESS;
    
    TRAPParameter *parameter_array = NULL;
    unsigned int parameter_num = 0;
    ret = trap_show_parameter(&parameter_array, &parameter_num);
    manage_log(LOG_DEBUG, "after trap_show_parameter, ret = %d, trapDetail_num = %d\n", ret, parameter_num);

    char *paraStr = (char *)malloc(128);
    if(NULL == paraStr) {
        ret = AC_MANAGE_MALLOC_ERROR;
    }
    
    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &parameter_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
	if(AC_MANAGE_SUCCESS == ret) {   

        int i = 0;
        for(i = 0; i < parameter_num; i++) {
            manage_log(LOG_DEBUG, "parameter_array[i].paraStr = %s\n", parameter_array[i].paraStr);
            manage_log(LOG_DEBUG, "parameter_array[i].data = %d\n", parameter_array[i].data);
            
            memset(paraStr, 0, 128);        
            memcpy(paraStr, parameter_array[i].paraStr, 128);
            

    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);
 
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &paraStr);
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &parameter_array[i].data);   

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);    

    MANAGE_FREE(paraStr);
    MANAGE_FREE(parameter_array);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_manual_mib_acif_stats(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

    char *if_name = NULL;
    struct mib_acif_stats acif_stats = { 0 };
    
	int ret = AC_MANAGE_SUCCESS;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &if_name,
	                            DBUS_TYPE_UINT32, &acif_stats.acIfInNUcastPkts,
	                            DBUS_TYPE_UINT32, &acif_stats.acIfInDiscardPkts,
                                DBUS_TYPE_UINT32, &acif_stats.acIfInErrors,
                                DBUS_TYPE_UINT32, &acif_stats.acIfInMulticastPkts,
                                DBUS_TYPE_UINT32, &acif_stats.acIfOutDiscardPkts,
                                DBUS_TYPE_UINT32, &acif_stats.acIfOutErrors,
                                DBUS_TYPE_UINT32, &acif_stats.acIfOutNUcastPkts,
                                DBUS_TYPE_UINT32, &acif_stats.acIfOutMulticastPkts,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    if(NULL == if_name || 0 == strcmp(if_name, "")) {
        ret = AC_MANAGE_INPUT_TYPE_ERROR;
    }
    else {        
        manage_log(LOG_DEBUG, "ac_manage_dbus_manual_mib_acif_stats: if_name = %s\n", if_name);
        strncpy(acif_stats.ifname, if_name, sizeof(acif_stats.ifname) - 1);
        ret = mib_manual_set_acif_stats(&acif_stats);
    }
        
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_show_mib_acif_stats(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;	
	
    
	int ret = AC_MANAGE_SUCCESS;
	
    unsigned int acif_num = 0;
	struct mib_acif_stats *acif_array = NULL;

    char *ifname = (char *)malloc(MIB_IFNAME_SIZE);
    if(NULL == ifname) {
        manage_log(LOG_WARNING, "ac_manage_dbus_show_mib_acif_stats: malloc ifname error!\n");
        ret = AC_MANAGE_MALLOC_ERROR;
    }
	else {
    	ret = mib_show_manual_acif_stats(&acif_array, &acif_num);
        manage_log(LOG_DEBUG, "ac_manage_dbus_show_mib_acif_stats: mib_show_manual_acif_stats, ret = %d\n", ret);
    }     
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter); 
    
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &acif_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING
                                        
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
    if(AC_MANAGE_SUCCESS == ret && acif_num > 0) {
        DBusMessageIter  iter_struct;

        int i = 0;
        for(i = 0; i < acif_num; i++) {

            
            memset(ifname, 0, MIB_IFNAME_SIZE);
            memcpy(ifname, acif_array[i].ifname, MIB_IFNAME_SIZE);

            manage_log(LOG_DEBUG, "ac_manage_dbus_show_mib_acif_stats: acif_array[%d].ifname = %s, ifname = %s\n", 
                                i, acif_array[i].ifname, ifname);
            
            dbus_message_iter_open_container(&iter_array,
                                                DBUS_TYPE_STRUCT,
                                                NULL,
                                                &iter_struct);
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &ifname);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &acif_array[i].acIfInNUcastPkts);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &acif_array[i].acIfInDiscardPkts);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &acif_array[i].acIfInErrors);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &acif_array[i].acIfInMulticastPkts); 
                                            
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &acif_array[i].acIfOutDiscardPkts);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &acif_array[i].acIfOutErrors);  
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &acif_array[i].acIfOutNUcastPkts);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &acif_array[i].acIfOutMulticastPkts);   
            
            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }        
    }

    dbus_message_iter_close_container(&iter, &iter_array);  

    MANAGE_FREE(ifname);
    MANAGE_FREE(acif_array);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_get_mib_localslot_acif_stats(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;	
	
    
	int ret = AC_MANAGE_SUCCESS;
	
    unsigned int acif_num = 0;
	struct if_stats_list *acifHead = NULL;

    char *ifname = (char *)malloc(MIB_IFNAME_SIZE);
    if(NULL == ifname) {
        manage_log(LOG_WARNING, "ac_manage_dbus_get_mib_localslot_acif_stats: malloc ifname error!\n");
        ret = AC_MANAGE_MALLOC_ERROR;
    }
	else {
    	ret = mib_get_localslot_acif_stats(&acifHead, &acif_num);
        manage_log(LOG_DEBUG, "ac_manage_dbus_get_mib_localslot_acif_stats: mib_get_localslot_acif_stats, ret = %d\n", ret);
    }     
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter); 
    
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &acif_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING      /*ac interface name*/
                                        
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING

                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
    if(AC_MANAGE_SUCCESS == ret && acif_num > 0) {
        DBusMessageIter  iter_struct;

        
        int i = 0;
        struct if_stats_list *ifNode = NULL;
        for(i = 0, ifNode = acifHead; i < acif_num && ifNode; i++, ifNode = ifNode->next) {
                        
            memset(ifname, 0, MIB_IFNAME_SIZE);
            memcpy(ifname, ifNode->ifname, MIB_IFNAME_SIZE);

            manage_log(LOG_DEBUG, "ac_manage_dbus_show_mib_acif_stats: ifNode->ifname = %s, ifname = %s\n", 
                                ifNode->ifname, ifname);
            
            dbus_message_iter_open_container(&iter_array,
                                                DBUS_TYPE_STRUCT,
                                                NULL,
                                                &iter_struct);
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &ifname);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_packets);     /* total packets received       */
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_packets);     /* total packets transmitted    */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_bytes);        /* total bytes received         */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_bytes);       /* total bytes transmitted      */
                                                     
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_errors);      /* bad packets received         */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_errors);      /* packet transmit problems     */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_dropped);     /* no space in linux buffers    */ 
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_dropped);      /* no space available in linux  */ 

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_multicast);      /* multicast packets received   */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_multicast);      /* multicast packets transmitted   */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_compressed);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_compressed);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.collisions);   

            /* detailed rx_errors: */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_length_errors);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_over_errors);      /* receiver ring buff overflow  */
                                                                               
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_crc_errors);      /* recved pkt with crc error    */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_frame_errors);     /* recv'd frame alignment error */ 

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_fifo_errors);      /* recv'r fifo overrun          */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_missed_errors);    /* receiver missed packet     */

            
            /* detailed tx_errors */
              
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_aborted_errors);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_carrier_errors);  

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_fifo_errors);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_heartbeat_errors);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_window_errors);   
                                            
            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }        
    }

    dbus_message_iter_close_container(&iter, &iter_array);  

    MANAGE_FREE(ifname);
    free_mib_acif_stats_list(&acifHead);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_show_mib_accumulate_acif_stats(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;	

    unsigned int slot_id = 0;
    
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    manage_log(LOG_DEBUG, "ac_manage_dbus_show_mib_accumulate_acif_stats: slot_id = %d\n", slot_id);
    
	int ret = AC_MANAGE_SUCCESS;
	
    unsigned int acif_num = 0;
	struct if_stats_list *acifHead = NULL;

    char *ifname = (char *)malloc(MIB_IFNAME_SIZE);
    if(NULL == ifname) {
        manage_log(LOG_WARNING, "ac_manage_dbus_get_mib_localslot_acif_stats: malloc ifname error!\n");
        ret = AC_MANAGE_MALLOC_ERROR;
    }
	else {
    	ret = mib_accumulate_acif_stats(&acifHead, &acif_num, slot_id);
        manage_log(LOG_DEBUG, "ac_manage_dbus_show_mib_accumulate_acif_stats: mib_accumulate_acif_stats, ret = %d\n", ret);
    }     
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter); 
    
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &acif_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING      /*ac interface name*/
                                        
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING

                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING
                                        DBUS_TYPE_UINT64_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
                                    
    if(AC_MANAGE_SUCCESS == ret && acif_num > 0) {
        DBusMessageIter  iter_struct;

        
        int i = 0;
        struct if_stats_list *ifNode = NULL;
        for(i = 0, ifNode = acifHead; i < acif_num && ifNode; i++, ifNode = ifNode->next) {
                        
            memset(ifname, 0, MIB_IFNAME_SIZE);
            memcpy(ifname, ifNode->ifname, MIB_IFNAME_SIZE);

            manage_log(LOG_DEBUG, "ac_manage_dbus_show_mib_acif_stats: ifNode->ifname = %s, ifname = %s\n", 
                                ifNode->ifname, ifname);
            
            dbus_message_iter_open_container(&iter_array,
                                                DBUS_TYPE_STRUCT,
                                                NULL,
                                                &iter_struct);
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_STRING,
                                            &ifname);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_packets);     /* total packets received       */
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_packets);     /* total packets transmitted    */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_bytes);        /* total bytes received         */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_bytes);       /* total bytes transmitted      */
                                                     
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_errors);      /* bad packets received         */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_errors);      /* packet transmit problems     */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_dropped);     /* no space in linux buffers    */ 
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_dropped);      /* no space available in linux  */ 

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_multicast);      /* multicast packets received   */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_multicast);      /* multicast packets transmitted   */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_compressed);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_compressed);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.collisions);   

            /* detailed rx_errors: */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_length_errors);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_over_errors);      /* receiver ring buff overflow  */
                                                                               
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_crc_errors);      /* recved pkt with crc error    */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_frame_errors);     /* recv'd frame alignment error */ 

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_fifo_errors);      /* recv'r fifo overrun          */
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.rx_missed_errors);    /* receiver missed packet     */

            
            /* detailed tx_errors */
              
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_aborted_errors);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_carrier_errors);  

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_fifo_errors);   
            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_heartbeat_errors);  
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT64,
                                            &ifNode->stats.tx_window_errors);   
                                            
            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }        
    }

    dbus_message_iter_close_container(&iter, &iter_array);  

    MANAGE_FREE(ifname);
    free_mib_acif_stats_list(&acifHead);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_show_radius_config(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;

    int ret = AC_MANAGE_SUCCESS;
    
    struct instance_radius_config *configHead = NULL;
    unsigned int config_num = 0;
    ret = manage_get_master_radius_config(&configHead, &config_num);
    manage_log(LOG_DEBUG, "after manage_get_master_radius_config, ret = %d, config_num = %d\n", ret, config_num);

    char *domain = (char *)malloc(128);
    char *auth_secret = (char *)malloc(128);
    char *acct_secret = (char *)malloc(128);
    char *backup_auth_secret = (char *)malloc(128);
    char *backup_acct_secret = (char *)malloc(128);
    if(NULL == domain || NULL == auth_secret || NULL == acct_secret || NULL == backup_auth_secret || NULL == backup_acct_secret) {
        manage_log(LOG_WARNING, "ac_manage_dbus_show_radius_config, malloc para failed!\n");
        MANAGE_FREE(domain);
        MANAGE_FREE(auth_secret);
        MANAGE_FREE(acct_secret);
        MANAGE_FREE(backup_auth_secret);
        MANAGE_FREE(backup_acct_secret);
        ret = AC_MANAGE_MALLOC_ERROR;
    }
    
    reply = dbus_message_new_method_return(message);

	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &config_num);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        
                                        DBUS_TYPE_ARRAY_AS_STRING
                                        DBUS_STRUCT_BEGIN_CHAR_AS_STRING

                                            DBUS_TYPE_STRING_AS_STRING          //domain
                                            
                                            DBUS_TYPE_UINT32_AS_STRING          //auth_ip
                                            DBUS_TYPE_UINT16_AS_STRING          //auth_port
                                            DBUS_TYPE_STRING_AS_STRING          //auth_secret
                                            DBUS_TYPE_UINT32_AS_STRING          //auth_secretlen
        
                                            DBUS_TYPE_UINT32_AS_STRING          //acct_ip
                                            DBUS_TYPE_UINT16_AS_STRING          //acct_port
                                            DBUS_TYPE_STRING_AS_STRING          //acct_secret
                                            DBUS_TYPE_UINT32_AS_STRING          //acct_secretlen
    
                                            DBUS_TYPE_UINT32_AS_STRING          //backup_auth_ip
                                            DBUS_TYPE_UINT16_AS_STRING          //backup_auth_port
                                            DBUS_TYPE_STRING_AS_STRING          //backup_auth_secret
                                            DBUS_TYPE_UINT32_AS_STRING          //backup_auth_secretlen
                                            
                                            DBUS_TYPE_UINT32_AS_STRING          //backup_acct_ip
                                            DBUS_TYPE_UINT16_AS_STRING          //backup_acct_port
                                            DBUS_TYPE_STRING_AS_STRING          //backup_acct_secret
                                            DBUS_TYPE_UINT32_AS_STRING          //backup_acct_secretlen
                                            
                                        DBUS_STRUCT_END_CHAR_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);

	if(AC_MANAGE_SUCCESS == ret) {   

        struct instance_radius_config *configNode = NULL;
        for(configNode = configHead; NULL != configNode; configNode = configNode->next) {

            DBusMessageIter iter_sub_array;
            DBusMessageIter iter_sub_struct;

    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);
         										
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &configNode->slot_id);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &configNode->local_id);
 
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &configNode->instance_id);
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(configNode->radiusconf.current_num));   


    		dbus_message_iter_open_container(&iter_struct,
                                            DBUS_TYPE_ARRAY,
                                            DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                            
                                                DBUS_TYPE_STRING_AS_STRING          //domain
                                                
                                                DBUS_TYPE_UINT32_AS_STRING          //auth_ip
                                                DBUS_TYPE_UINT16_AS_STRING          //auth_port
                                                DBUS_TYPE_STRING_AS_STRING          //auth_secret
                                                DBUS_TYPE_UINT32_AS_STRING          //auth_secretlen
                                                
                                                DBUS_TYPE_UINT32_AS_STRING          //acct_ip
                                                DBUS_TYPE_UINT16_AS_STRING          //acct_port
                                                DBUS_TYPE_STRING_AS_STRING          //acct_secret
                                                DBUS_TYPE_UINT32_AS_STRING          //acct_secretlen
                                                
                                                DBUS_TYPE_UINT32_AS_STRING          //backup_auth_ip
                                                DBUS_TYPE_UINT16_AS_STRING          //backup_auth_port
                                                DBUS_TYPE_STRING_AS_STRING          //backup_auth_secret
                                                DBUS_TYPE_UINT32_AS_STRING          //backup_auth_secretlen
                                                
                                                DBUS_TYPE_UINT32_AS_STRING          //backup_acct_ip
                                                DBUS_TYPE_UINT16_AS_STRING          //backup_acct_port
                                                DBUS_TYPE_STRING_AS_STRING          //backup_acct_secret
                                                DBUS_TYPE_UINT32_AS_STRING          //backup_acct_secretlen
                                            
                                            DBUS_STRUCT_END_CHAR_AS_STRING,
                                            &iter_sub_array);
            
            int i = 0;
            for(i = 0; i < configNode->radiusconf.current_num; i++) {

                dbus_message_iter_open_container(&iter_sub_array,
                                                    DBUS_TYPE_STRUCT,
                                                    NULL,
                                                    &iter_sub_struct);

                memset(domain, 0, 128);
                memcpy(domain, configNode->radiusconf.radius_srv[i].domain, 128);

                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_STRING,
                                                &domain);             //domain


                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->radiusconf.radius_srv[i].auth_ip));

                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT16,
                                                &(configNode->radiusconf.radius_srv[i].auth_port));

                memset(auth_secret, 0,128);
                memcpy(auth_secret, configNode->radiusconf.radius_srv[i].auth_secret, 128);

                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_STRING,
                                                &auth_secret);             //auth_secret                                

                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->radiusconf.radius_srv[i].auth_secretlen));



                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->radiusconf.radius_srv[i].acct_ip));
                                                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT16,
                                                &(configNode->radiusconf.radius_srv[i].acct_port));

                memset(acct_secret, 0, 128);
                memcpy(acct_secret, configNode->radiusconf.radius_srv[i].acct_secret, 128);
                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_STRING,
                                                &acct_secret);             //acct_secret
                                                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->radiusconf.radius_srv[i].acct_secretlen));


                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->radiusconf.radius_srv[i].backup_auth_ip));
                                                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT16,
                                                &(configNode->radiusconf.radius_srv[i].backup_auth_port));
                                                
                memset(backup_auth_secret, 0, 128);
                memcpy(backup_auth_secret, configNode->radiusconf.radius_srv[i].backup_auth_secret, 128);
                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_STRING,
                                                &backup_auth_secret);             //backup_auth_secret
                                                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->radiusconf.radius_srv[i].backup_auth_secretlen));



                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->radiusconf.radius_srv[i].backup_acct_ip));
                                                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT16,
                                                &(configNode->radiusconf.radius_srv[i].backup_acct_port));
                                                
                memset(backup_acct_secret, 0, 128);
                memcpy(backup_acct_secret, configNode->radiusconf.radius_srv[i].backup_acct_secret, 128);
                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_STRING,
                                                &backup_acct_secret);             //backup_acct_secret
                                                
                dbus_message_iter_append_basic(&iter_sub_struct,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->radiusconf.radius_srv[i].backup_acct_secretlen));


                dbus_message_iter_close_container(&iter_sub_array, &iter_sub_struct);
            }
    		dbus_message_iter_close_container(&iter_struct, &iter_sub_array);
            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
    }
    
    dbus_message_iter_close_container(&iter, &iter_array);    

    MANAGE_FREE(domain);
    MANAGE_FREE(auth_secret);
    MANAGE_FREE(acct_secret);
    MANAGE_FREE(backup_auth_secret);
    MANAGE_FREE(backup_acct_secret);
    manage_free_radius_config(&configHead);
    
    return reply;        
}


DBusMessage *
ac_manage_dbus_show_portal_config(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

    int ret = AC_MANAGE_SUCCESS;
    
    struct instance_portal_config *configHead = NULL;
    unsigned int config_num = 0;
    ret = manage_get_master_portal_config(&configHead, &config_num);
    manage_log(LOG_DEBUG, "after manage_get_master_portal_config, ret = %d, config_num = %d\n", ret, config_num);

    char *temp_buf = (char *)malloc(256);
    char *portal_url = (char *)malloc(256);
    char *domain = (char *)malloc(256);
    char *acname = (char *)malloc(256);
    if(NULL == temp_buf || NULL == portal_url || NULL == domain || NULL == acname) {
        manage_log(LOG_WARNING, "ac_manage_dbus_show_portal_config, malloc para failed!\n");
        MANAGE_FREE(temp_buf);
        MANAGE_FREE(portal_url);
        MANAGE_FREE(domain);
        MANAGE_FREE(acname);
        ret = AC_MANAGE_MALLOC_ERROR;
    }
    
    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &config_num);

                                        
	if(AC_MANAGE_SUCCESS == ret) {   

        struct instance_portal_config *configNode = NULL;
        for(configNode = configHead; NULL != configNode; configNode = configNode->next) {

         										
            dbus_message_iter_append_basic(&iter,
                                            DBUS_TYPE_UINT32,
                                            &configNode->slot_id);
                                            
            dbus_message_iter_append_basic(&iter,
                                            DBUS_TYPE_UINT32,
                                            &configNode->local_id);
 
            dbus_message_iter_append_basic(&iter,
                                            DBUS_TYPE_UINT32,
                                            &configNode->instance_id);
                                            
            dbus_message_iter_append_basic(&iter,
                                            DBUS_TYPE_UINT32,
                                            &configNode->portalconf.current_num);   
            
            int i = 0;
            for(i = 0; i < configNode->portalconf.current_num; i++) {
                
                dbus_message_iter_append_basic(&iter,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->portalconf.portal_srv[i].key_type)); 

                switch(configNode->portalconf.portal_srv[i].key_type) {
                    case PORTAL_KEYTYPE_ESSID:
                        memset(temp_buf, 0, 256);
                        strncpy(temp_buf, configNode->portalconf.portal_srv[i].key.essid, 256 - 1);
                        dbus_message_iter_append_basic(&iter,
                                                        DBUS_TYPE_STRING,
                                                        &temp_buf); 
                        break;          
                        
                    case PORTAL_KEYTYPE_WLANID:
                        dbus_message_iter_append_basic(&iter,
                                                        DBUS_TYPE_UINT32,
                                                        &(configNode->portalconf.portal_srv[i].key.wlanid)); 
                        break;
                        
                    case PORTAL_KEYTYPE_VLANID:
                        dbus_message_iter_append_basic(&iter,
                                                        DBUS_TYPE_UINT32,
                                                        &(configNode->portalconf.portal_srv[i].key.vlanid)); 
                        break;
                        
                    case PORTAL_KEYTYPE_WTPID:
                        dbus_message_iter_append_basic(&iter,
                                                        DBUS_TYPE_UINT32,
                                                        &(configNode->portalconf.portal_srv[i].key.wtpid)); 
                        break;
                        
                    case PORTAL_KEYTYPE_INTF:                        
                        memset(temp_buf, 0, 256);
                        strncpy(temp_buf, configNode->portalconf.portal_srv[i].key.intf, 256 - 1);
                        dbus_message_iter_append_basic(&iter,
                                                        DBUS_TYPE_STRING,
                                                        &temp_buf); 
                        break;
                        
                    default:
                        manage_log(LOG_WARNING, "ac_manage_dbus_show_portal_config: switch %d unknow\n", configNode->portalconf.portal_srv[i].key_type);
                        break;
                }
                                                
                memset(portal_url, 0, 256);
                strncpy(portal_url, configNode->portalconf.portal_srv[i].portal_url, 256 - 1);
                dbus_message_iter_append_basic(&iter,
                                                DBUS_TYPE_STRING,
                                                &portal_url); 
                                                
                dbus_message_iter_append_basic(&iter,
                                                DBUS_TYPE_UINT16,
                                                &(configNode->portalconf.portal_srv[i].ntf_port));
                
                memset(domain, 0, 256);
                strncpy(domain, configNode->portalconf.portal_srv[i].domain, 256 - 1);
                dbus_message_iter_append_basic(&iter,
                                                DBUS_TYPE_STRING,
                                                &domain);             //domain                                

                memset(acname, 0, 256);
                strncpy(acname, configNode->portalconf.portal_srv[i].acname, 256 - 1);
                dbus_message_iter_append_basic(&iter,
                                                DBUS_TYPE_STRING,
                                                &acname);             //acname                                

                dbus_message_iter_append_basic(&iter,
                                                DBUS_TYPE_UINT32,
                                                &(configNode->portalconf.portal_srv[i].ip));
                                                
            }
        }
    }

    MANAGE_FREE(temp_buf);
    MANAGE_FREE(portal_url);
    MANAGE_FREE(domain);
    MANAGE_FREE(acname);
    manage_free_portal_config(&configHead);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_web_edit(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	char *address = NULL;
    char *name = NULL;
    char *ifname = NULL;
	unsigned int port = 0;
	unsigned int type = 0;
	unsigned int edit = 0;
	unsigned int slot = 0;

	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
    dbus_message_iter_init(message, &iter);
    dbus_message_iter_get_basic(&iter, &edit);

    switch(edit)
    {
        case HOST_ADD:
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &address);
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &port);
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &name);
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &type);
            LOG("get %s %s %d %d",name, address, port, type);
            break;

        case HOST_DEL:
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &name);
            LOG("get %s", name);
            break;

        case IFNAME_ADD:
        case IFNAME_DEL:
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &name);
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &ifname);
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &slot);
            LOG("get %s %s %d", name, ifname, slot);
            break;
    }


    switch(edit)
    {
        case HOST_ADD:
		    ret = web_host_add(name, address, port, type);
            break;
        case HOST_DEL:
		    ret = web_host_del(name);
            break;
        case IFNAME_ADD:
            ret = web_interface_add(name, ifname, slot);
            break;
        case IFNAME_DEL:
            ret = web_interface_del(name, ifname);
            break;
        default:
            exit(0);
            break;
    }
    
    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}

DBusMessage *
ac_manage_dbus_web_show(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusMessageIter	iter; 
    DBusMessageIter iter_array, iter_struct; 
    DBusMessageIter iter_sub_array, iter_sub_struct;
	DBusError err;

	struct webHostHead head;
	webHostPtr vh = NULL;	
    webIfPtr in = NULL;

    unsigned int n1 = 0;
    unsigned int n2 = 0;

    unsigned int ref = 0; 
	int sum, i = 0, j = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args hello world\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	LINK_INIT(&head);
	
    sum = web_host_show(&head, &n1, &n2);

    LOG("reply %d - ret  = %d ", ref++, sum);
    LOG("reply %d - stat = %d ", ref++, n1);
    LOG("reply %d - flag = %d ", ref++, n2);
	
    reply = dbus_message_new_method_return(message);

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32,&sum);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32,&n1);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32,&n2);

    if(sum > 0)
    {
        dbus_message_iter_open_container(&iter, 
                DBUS_TYPE_ARRAY,
                DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                    DBUS_TYPE_STRING_AS_STRING
                    DBUS_TYPE_STRING_AS_STRING
                    DBUS_TYPE_UINT32_AS_STRING
                    DBUS_TYPE_UINT32_AS_STRING
                    DBUS_TYPE_UINT32_AS_STRING

                    DBUS_TYPE_ARRAY_AS_STRING
                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                        DBUS_TYPE_STRING_AS_STRING
                        DBUS_TYPE_STRING_AS_STRING
                        DBUS_TYPE_UINT32_AS_STRING
                        DBUS_TYPE_UINT32_AS_STRING
                    DBUS_STRUCT_END_CHAR_AS_STRING

                DBUS_STRUCT_END_CHAR_AS_STRING,
                &iter_array);

        vh = head.lh_first;
        while(i++ <sum)
        {
            LOG("reply %d - %s %s %d %d %d",ref++, vh->name, vh->address, vh->port, vh->type, vh->count);
            dbus_message_iter_open_container(&iter_array, DBUS_TYPE_STRUCT, NULL, &iter_struct);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &(vh->name));
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &(vh->address));
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(vh->port));
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(vh->type));
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(vh->count));

                dbus_message_iter_open_container(&iter_struct, 
                        DBUS_TYPE_ARRAY,
                        DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                            DBUS_TYPE_STRING_AS_STRING
                            DBUS_TYPE_STRING_AS_STRING
                            DBUS_TYPE_UINT32_AS_STRING
                            DBUS_TYPE_UINT32_AS_STRING
                        DBUS_STRUCT_END_CHAR_AS_STRING,
                        &iter_sub_array);
                
                in = vh->head.lh_first;          
                j = 0; 
                while(j++ < vh->count)
                {
                    LOG("reply %d - %s %s %d %d",ref++ , in->name, in->ifname, in->slot, in->opt);
                    dbus_message_iter_open_container(&iter_sub_array, DBUS_TYPE_STRUCT, NULL, &iter_sub_struct);
                    dbus_message_iter_append_basic(&iter_sub_struct, DBUS_TYPE_STRING, &(in->name));
                    dbus_message_iter_append_basic(&iter_sub_struct, DBUS_TYPE_STRING, &(in->ifname));
                    dbus_message_iter_append_basic(&iter_sub_struct, DBUS_TYPE_UINT32, &(in->slot));
                    dbus_message_iter_append_basic(&iter_sub_struct, DBUS_TYPE_UINT32, &(in->opt));
                    dbus_message_iter_close_container(&iter_sub_array, &iter_sub_struct);
                    in = LINK_NEXT(in, entries);
                }
                dbus_message_iter_close_container(&iter_struct, &iter_sub_array);
            dbus_message_iter_close_container(&iter_array, &iter_struct); 
            vh = LINK_NEXT(vh, entries);
        }
        dbus_message_iter_close_container(&iter, &iter_array); 
    }

    LOG("----reply end----");
	return reply;
}

DBusMessage *
ac_manage_dbus_web_conf(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusMessageIter	 iter;	
	DBusError err;
	
	int ret, operate;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &operate,
								DBUS_TYPE_INVALID))){

    manage_log(LOG_WARNING, "Unable to get input args\n");
	    
	if (dbus_error_is_set(&err)) {
        manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
		dbus_error_free(&err);
		}
	
		return NULL;
	}

	switch(operate)
	{
		case WEB_START:
			LOG("WEB_START");
			break;
		case WEB_STOP:
			LOG("WEB_STOP");
			break;
		case PORTAL_START:
			LOG("PORTAL_START");
			break;
		case PORTAL_STOP:
			LOG("PORTAL_STOP");
			break;
		default:
			break;
	}
	
	ret = web_host_service(operate);

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}

DBusMessage *ac_manage_dbus_web_download(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	char *addr, *path, *usr, *pass;
	int ret = 1;
	
	dbus_error_init(&err);	

	dbus_message_iter_init(message, &iter);
	dbus_message_iter_get_basic(&iter, &addr);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &path);		
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &usr);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &pass);	
	
	ret = web_dir_download(addr, path, usr, pass);

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}

DBusMessage *ac_manage_dbus_web_show_pages(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *dir[4];
	int count, i;
	
	dbus_error_init(&err);	

	dbus_message_iter_init(message, &iter);

	web_dir_show(dir, &count);

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&count);
    for(i = 0; i < count; i++)
    {
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&(dir[i]));
    }
    
    return reply;        	
	
}

DBusMessage *ac_manage_dbus_web_del_pages(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *dir;
	int ret;
	
	dbus_error_init(&err);	

	dbus_message_iter_init(message, &iter);
	dbus_message_iter_get_basic(&iter, &dir);	

	ret = web_dir_del(dir);

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
 
    return reply;        	
	
}


DBusMessage *
ac_manage_dbus_extend_command_exec(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		
	DBusMessageIter	 iter_array;	

    int ret = AC_MANAGE_SUCCESS;

    unsigned int command_type;
    char *command = NULL;

    FILE *fp = NULL;
    char buf[AC_MANAGE_LINE_SIZE] = { 0 };

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
                                DBUS_TYPE_UINT32, &command_type,       
	                            DBUS_TYPE_STRING, &command,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    ret = manage_command_exec(command_type, command, &fp);   
    
    manage_log(LOG_DEBUG, "ac_manage_dbus_extend_command_exec: manage_command_exec, ret = %d, fp = %p\n", ret, fp);

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret); 

    unsigned int moreReturn = 0;
    
    manage_log(LOG_DEBUG, "ac_manage_dbus_extend_command_exec: start return\n");
    
    if(AC_MANAGE_SUCCESS == ret && fp) {
        
        while(NULL != fgets(buf, sizeof(buf), fp)) {
            
            manage_log(LOG_DEBUG, "%s\n", buf);

            moreReturn = 1;
            
            dbus_message_iter_append_basic(&iter,
                                             DBUS_TYPE_UINT32,
                                             &moreReturn);
            
            char *returnStr = buf;
    	    dbus_message_iter_append_basic(&iter,
            								 DBUS_TYPE_STRING,
            								 &returnStr);
        }

        pclose(fp);
        
        moreReturn = 0;
    }
    
    manage_log(LOG_DEBUG, "ac_manage_dbus_extend_command_exec: end return\n");
    
    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &moreReturn);

    return reply;        
}

DBusMessage *
ac_manage_dbus_config_flow_control_service(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;

    unsigned int status = 0;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_UINT32, &status,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
            
    ret = manage_config_flow_control_service(status);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}


DBusMessage *
ac_manage_dbus_add_tcrule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;

    char *name          = NULL;    
    unsigned int enable     = 0;
    unsigned int ruleIndex  = 0;
    char *comment       = NULL;
    
    char *interface     = NULL;
    char *up_interface  = NULL;
    char *protocol      = NULL;
    
    char *p2p_detail    = NULL;
    
    char *addrtype      = NULL;
    char *addr_begin    = NULL;
    char *addr_end      = NULL;
    char *mode          = NULL;

    char *uplink_speed   = NULL;
    char *downlink_speed = NULL;
    
    unsigned int useP2P      = 0;
    char *p2p_uplink_speed   = NULL;
    char *p2p_downlink_speed = NULL;

    unsigned int time_begin  = 0;
    unsigned int time_end    = 0;

    char *limit_speed    = NULL;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &name,
	                            DBUS_TYPE_UINT32, &enable,
	                            DBUS_TYPE_UINT32, &ruleIndex,
	                            DBUS_TYPE_STRING, &comment,
	                            DBUS_TYPE_STRING, &interface,
	                            DBUS_TYPE_STRING, &up_interface,
	                            DBUS_TYPE_STRING, &protocol,
	                            DBUS_TYPE_STRING, &p2p_detail,
	                            DBUS_TYPE_STRING, &addrtype,
	                            DBUS_TYPE_STRING, &addr_begin,
	                            DBUS_TYPE_STRING, &addr_end,
	                            DBUS_TYPE_STRING, &mode,
	                            DBUS_TYPE_STRING, &uplink_speed,
	                            DBUS_TYPE_STRING, &downlink_speed,
	                            DBUS_TYPE_UINT32, &useP2P,
	                            DBUS_TYPE_STRING, &p2p_uplink_speed,
	                            DBUS_TYPE_STRING, &p2p_downlink_speed,
	                            DBUS_TYPE_UINT32, &time_begin,
	                            DBUS_TYPE_UINT32, &time_end,    
	                            DBUS_TYPE_STRING, &limit_speed,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    
    TCRule *tcRuleNew = (TCRule *)malloc(sizeof(TCRule));
    if(tcRuleNew) {
    
        memset(tcRuleNew, 0, sizeof(TCRule));

        if(name && name[0]) {
            tcRuleNew->name = strdup(name);
            if(NULL == tcRuleNew->name) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }    
        }

        tcRuleNew->enable = enable;
        tcRuleNew->ruleIndex = ruleIndex;
        
        if(interface && interface[0]) {
            
            /*check the down interface is legal*/
            if(AC_MANAGE_SUCCESS != manage_ifname_is_legal_input(interface)) {
                ret = AC_MANAGE_INPUT_TYPE_ERROR;
                goto FREE_TCRULE;
            }

            tcRuleNew->interface = strdup(interface);
            if(NULL == tcRuleNew->interface) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }
        }
        
        if(up_interface && up_interface[0]) {

            /*check the up interface is legal*/
            if(AC_MANAGE_SUCCESS != manage_ifname_is_legal_input(up_interface)) {
                ret = AC_MANAGE_INPUT_TYPE_ERROR;
                goto FREE_TCRULE;
            }
            
            tcRuleNew->up_interface = strdup(up_interface);
            if(NULL == tcRuleNew->up_interface) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }
        
        if(protocol && protocol[0]) {
            tcRuleNew->protocol = strdup(protocol);
            if(NULL == tcRuleNew->protocol) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }
        
        if(p2p_detail && p2p_detail[0]) {
            tcRuleNew->p2p_detail = strdup(p2p_detail);
            if(NULL == tcRuleNew->p2p_detail) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }

        if(addrtype && addrtype[0]) {
            tcRuleNew->addrtype = strdup(addrtype);
            if(NULL == tcRuleNew->addrtype) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }

        if(addr_begin && addr_begin[0]) {
            tcRuleNew->addr_begin = strdup(addr_begin);
            if(NULL == tcRuleNew->addr_begin) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }
        }
        
        if(addr_end && addr_end[0]) {
            tcRuleNew->addr_end = strdup(addr_end);
            if(NULL == tcRuleNew->addr_end) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }
        
        if(mode && mode[0]) {
            tcRuleNew->mode = strdup(mode);
            if(NULL == tcRuleNew->mode) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }
        
        if(uplink_speed && uplink_speed[0]) {
            tcRuleNew->uplink_speed = strdup(uplink_speed);
            if(NULL == tcRuleNew->uplink_speed) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }

        if(downlink_speed && downlink_speed[0]) {
            tcRuleNew->downlink_speed = strdup(downlink_speed);
            if(NULL == tcRuleNew->downlink_speed) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }

        tcRuleNew->useP2P = useP2P;

        if(p2p_uplink_speed && p2p_uplink_speed[0]) {
            tcRuleNew->p2p_uplink_speed = strdup(p2p_uplink_speed);
            if(NULL == tcRuleNew->p2p_uplink_speed) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }
        }

        if(p2p_downlink_speed && p2p_downlink_speed[0]) {
            tcRuleNew->p2p_downlink_speed = strdup(p2p_downlink_speed);
            if(NULL == tcRuleNew->p2p_downlink_speed) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }

        tcRuleNew->time_begin = time_begin;
        tcRuleNew->time_begin = time_begin;
         
        if(limit_speed && limit_speed[0]) {
            tcRuleNew->limit_speed = strdup(limit_speed);
            if(NULL == tcRuleNew->limit_speed) {
                ret = AC_MANAGE_MALLOC_ERROR;
                goto FREE_TCRULE;
            }        
        }
        manage_log(LOG_DEBUG, "ac_manage_dbus_add_tcrule: before manage_add_tcrule\n");
        ret = manage_add_tcrule(tcRuleNew);
        manage_log(LOG_DEBUG, "ac_manage_dbus_add_tcrule: manage_add_tcrule, ret = %d\n", ret);
        
    FREE_TCRULE:
        if(AC_MANAGE_SUCCESS != ret) {
            tcFreeRule(tcRuleNew);
        }    
        
    }
    else {
        ret = AC_MANAGE_MALLOC_ERROR;
    }
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}

DBusMessage *
ac_manage_dbus_offset_tcrule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;

    struct tcrule_offset_s offset = { 0 };

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_UINT32, &offset.ruleIndex,
	                            DBUS_TYPE_UINT32, &offset.uplink_offset,
	                            DBUS_TYPE_UINT32, &offset.downlink_offset,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
            
    ret = manage_set_tcrule_offset(&offset);    
    manage_log(LOG_DEBUG, "ac_manage_dbus_offset_tcrule: manage_set_tcrule_offset, ret = %d\n", ret);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append(reply, &iter);   
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}


DBusMessage *
ac_manage_dbus_delete_tcrule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;

    unsigned int ruleIndex = 0;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_UINT32, &ruleIndex,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
            
    ret = manage_delete_tcrule(ruleIndex);    
    manage_log(LOG_DEBUG, "ac_manage_dbus_delete_tcrule: manage_delete_tcrule, ret = %d\n", ret);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}


DBusMessage *
ac_manage_dbus_show_flow_control_service(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
    
	dbus_error_init(&err);

    unsigned int status = 0;
	
    status = manage_show_flow_control_service();
    manage_log(LOG_DEBUG, "ac_manage_dbus_show_tcrule: after manage_show_flow_control_service, status = %d\n", status);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append(reply, &iter);   
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &status);

    return reply;
}


DBusMessage *
ac_manage_dbus_show_tcrule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;
	
	dbus_error_init(&err);

	int ret = AC_MANAGE_SUCCESS;
	
    TCRule *rule_array = NULL;
    unsigned int count = 0;
    ret = manage_show_tcrule(&rule_array, &count);
    manage_log(LOG_DEBUG, "after manage_show_tcrule, ret = %d, rule_array = %p, count = %d\n", ret, rule_array, count);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append(reply, &iter);  
    
    dbus_message_iter_append_basic(&iter, 
                                    DBUS_TYPE_UINT32, 
                                    &ret);
    
    dbus_message_iter_append_basic(&iter, 
                                    DBUS_TYPE_UINT32, 
                                    &count);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                    
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING

                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING
                                        DBUS_TYPE_STRING_AS_STRING

                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING

                                        DBUS_TYPE_STRING_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);

    if(AC_MANAGE_SUCCESS == ret && rule_array && count) {

        int i = 0;
        for(i = 0; i < count; i++) {

            char *name = rule_array[i].name ? rule_array[i].name : "";
            char *comment = rule_array[i].comment ? rule_array[i].comment : "";
            char *interface = rule_array[i].interface ? rule_array[i].interface : "";
            char *up_interface = rule_array[i].up_interface ? rule_array[i].up_interface : "";
            char *protocol = rule_array[i].protocol ? rule_array[i].protocol : "";
            char *p2p_detail = rule_array[i].p2p_detail ? rule_array[i].p2p_detail : "";
            char *addrtype = rule_array[i].addrtype ? rule_array[i].addrtype : "";
            char *addr_begin = rule_array[i].addr_begin ? rule_array[i].addr_begin : "";
            char *addr_end = rule_array[i].addr_end ? rule_array[i].addr_end : "";
            char *mode = rule_array[i].mode ? rule_array[i].mode : "";
            char *uplink_speed = rule_array[i].uplink_speed ? rule_array[i].uplink_speed : "";
            char *downlink_speed = rule_array[i].downlink_speed ? rule_array[i].downlink_speed : "";    
            char *p2p_uplink_speed = rule_array[i].p2p_uplink_speed ? rule_array[i].p2p_uplink_speed : "";
            char *p2p_downlink_speed = rule_array[i].p2p_downlink_speed ? rule_array[i].p2p_downlink_speed : "";
            char *limit_speed = rule_array[i].limit_speed ? rule_array[i].limit_speed : "";
            
    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);
            
            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &name);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_UINT32,
                                             &rule_array[i].enable);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_UINT32,
                                             &rule_array[i].ruleIndex);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &comment);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &interface);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &up_interface);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &protocol);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &p2p_detail);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &addrtype);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &addr_begin);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &addr_end);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &mode);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &uplink_speed);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &downlink_speed);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_UINT32,
                                             &rule_array[i].useP2P);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &p2p_uplink_speed);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &p2p_downlink_speed);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_UINT32,
                                             &rule_array[i].time_begin);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_UINT32,
                                             &rule_array[i].time_end);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_STRING,
                                             &limit_speed);

            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }

        tcFreeArray(&rule_array, count);
    }

    dbus_message_iter_close_container(&iter, &iter_array);

    return reply;        	
}

DBusMessage *
ac_manage_dbus_show_tcrule_offset(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;	
	DBusMessageIter  iter_struct;
	
	dbus_error_init(&err);

	int ret = AC_MANAGE_SUCCESS;
	
    struct tcrule_offset_s *offset_array = NULL;
    unsigned int count = 0;
    ret = manage_show_tcrule_offset(&offset_array, &count);
    manage_log(LOG_DEBUG, "after ac_manage_dbus_show_tcrule_offset, ret = %d, offset_array = %p, count = %d\n", ret, offset_array, count);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append(reply, &iter);  
    
    dbus_message_iter_append_basic(&iter, 
                                    DBUS_TYPE_UINT32, 
                                    &ret);
    
    dbus_message_iter_append_basic(&iter, 
                                    DBUS_TYPE_UINT32, 
                                    &count);

    
	dbus_message_iter_open_container(&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                    
                                        DBUS_TYPE_UINT32_AS_STRING
                                        
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING

                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);

    if(AC_MANAGE_SUCCESS == ret && offset_array && count) {

        int i = 0;
        for(i = 0; i < count; i++) {
            
    		dbus_message_iter_open_container(&iter_array,
     										    DBUS_TYPE_STRUCT,
         										NULL,
         										&iter_struct);
            
            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_UINT32,
                                             &offset_array[i].ruleIndex);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_UINT32,
                                             &offset_array[i].uplink_offset);

            dbus_message_iter_append_basic(&iter_struct,
                                             DBUS_TYPE_UINT32,
                                             &offset_array[i].downlink_offset);


            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }
        MANAGE_FREE(offset_array);
    }

    dbus_message_iter_close_container(&iter, &iter_array);
	MANAGE_FREE(offset_array);

    return reply;        	
}

DBusMessage *
ac_manage_dbus_show_tcrule_running_config(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusMessageIter	 iter;		

    int ret = AC_MANAGE_SUCCESS;
    
    char *showStr = NULL;
    unsigned int moreConfig = 0;    

    struct running_config *configHead = NULL;
    ret = manage_show_tcrule_running_config(&configHead);
    manage_log(LOG_DEBUG, "ac_manage_dbus_show_tcrule_running_config: manage_show_tcrule_running_config : configHead = %p, ret = %d", configHead, ret);

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    if(AC_MANAGE_SUCCESS == ret && configHead) {
    
        struct running_config *configNode = NULL;
        for(configNode = configHead; NULL != configNode; configNode = configNode->next) {
            
            moreConfig = 1;
            dbus_message_iter_append_basic(&iter,
                                             DBUS_TYPE_UINT32,
                                             &moreConfig);

            showStr = configNode->showStr;
            manage_log(LOG_DEBUG, "ac_manage_dbus_show_tcrule_running_config: showstr = %s\n", showStr);
    	    dbus_message_iter_append_basic(&iter,
            								 DBUS_TYPE_STRING,
            								 &showStr);
        }            
        moreConfig = 0;
    }

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &moreConfig);

    manage_free_running_config(&configHead);
    
    return reply;        
}

DBusMessage *
ac_manage_dbus_config_firewall_service(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int state = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){
		manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = manage_config_firewall_service(state);

	reply = dbus_message_new_method_return(message);
				
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}

DBusMessage *
ac_manage_dbus_config_firewall_rule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	int ret = AC_MANAGE_SUCCESS;

	fwRule rule;
	u_long config_type;

	memset(&rule, 0, sizeof(fwRule));
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &config_type,
								
								DBUS_TYPE_UINT32, &(rule.type),
								DBUS_TYPE_UINT32, &(rule.id),
								DBUS_TYPE_UINT32, &(rule.enable),
								
								DBUS_TYPE_STRING, &(rule.ineth),
								DBUS_TYPE_STRING, &(rule.outeth),
								DBUS_TYPE_UINT32, &(rule.srctype),
								DBUS_TYPE_STRING, &(rule.srcadd),
								DBUS_TYPE_UINT32, &(rule.dsttype),
								DBUS_TYPE_STRING, &(rule.dstadd),
								
								DBUS_TYPE_UINT32, &(rule.protocl),
								DBUS_TYPE_UINT32, &(rule.sptype),
								DBUS_TYPE_STRING, &(rule.sport),
								DBUS_TYPE_UINT32, &(rule.dptype),
								DBUS_TYPE_STRING, &(rule.dport),

								DBUS_TYPE_STRING, &(rule.connlimit),

								DBUS_TYPE_UINT32, &(rule.act),
								DBUS_TYPE_STRING, &(rule.tcpmss_var),

								DBUS_TYPE_STRING, &(rule.pkg_state),
								DBUS_TYPE_STRING, &(rule.string_filter),

								DBUS_TYPE_UINT32, &(rule.natiptype),
								DBUS_TYPE_STRING, &(rule.natipadd),
								DBUS_TYPE_UINT32, &(rule.natpttype),
								DBUS_TYPE_STRING, &(rule.natport),
								DBUS_TYPE_INVALID))){
		manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	if(config_type) {
		ret = manage_modify_firewall_rule(&rule);
	} else {
		ret = manage_add_firewall_rule(&rule);
	}

	reply = dbus_message_new_method_return(message);
				
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret); 

	return reply;
}


DBusMessage *
ac_manage_dbus_change_firewall_index(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	int ret = AC_MANAGE_SUCCESS;
	u_long rule_type, index, new_index;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &rule_type,
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_UINT32, &new_index,
								DBUS_TYPE_INVALID))){
		manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = manage_chanage_firewall_rule_index(new_index, rule_type, index);

	reply = dbus_message_new_method_return(message);
				
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}

DBusMessage *
ac_manage_dbus_del_firewall_rule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	int ret = AC_MANAGE_SUCCESS;
	u_long rule_type, index;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &rule_type,
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_INVALID))){
		manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = manage_del_firewall_rule(rule_type, index);

	reply = dbus_message_new_method_return(message);
				
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}

DBusMessage *
ac_manage_dbus_config_nat_udp_timeout(DBusConnection *connection, DBusMessage *message, void *user_data) {
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int timeout;
	int ret;
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &timeout,
								DBUS_TYPE_INVALID))){
		manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = manage_modify_nat_udp_timeout(timeout);
	manage_log(LOG_DEBUG, "after manage_modify_nat_udp_timeout, ret %d\n", ret);

	reply = dbus_message_new_method_return(message);		
	dbus_message_iter_init_append(reply, &iter);	
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret); 
	return reply;
}


DBusMessage *
ac_manage_dbus_show_firewall_rule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	iter;	
	DBusMessageIter	iter_array;	
	DBusMessageIter	iter_struct;
	
	dbus_error_init(&err);

	int ret = AC_MANAGE_SUCCESS;

	u_long service_status = 0;
	fwRule *rule_array = NULL;
	u_long rule_num = 0;
	unsigned int timeout;

	service_status = manage_show_firewall_service();
	manage_log(LOG_DEBUG, "after manage_show_firewall_service, service_status = %d\n", service_status);

	timeout = manage_show_nat_udp_timeout();
	manage_log(LOG_DEBUG, "after manage_show_nat_udp_timeout, timeout = %u\n", timeout);
	
	ret = manage_show_firewall_rule(&rule_array, &rule_num);
	manage_log(LOG_DEBUG, "after manage_show_firewall_rule, ret = %d, rule_array = %p, rule_num = %d\n", ret, rule_array, rule_num);
    
	reply = dbus_message_new_method_return(message);
	                
	dbus_message_iter_init_append(reply, &iter);  

	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, 
									&ret);
	
	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, 
									&service_status);

	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, 
									&timeout);
    
	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_UINT32, 
									&rule_num);

    
	dbus_message_iter_open_container(&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING

										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING

										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_STRING_AS_STRING

										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_STRING_AS_STRING

										DBUS_TYPE_STRING_AS_STRING	//connlimit

										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_STRING_AS_STRING

										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING

										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_STRING_AS_STRING

									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	if(AC_MANAGE_SUCCESS == ret && rule_array && rule_num) {

		int i = 0;
		for(i = 0; i < rule_num; i++) {
			char *ineth = rule_array[i].ineth ? : "";
			char *outeth = rule_array[i].outeth ? : "";
			char *srcadd = rule_array[i].srcadd ? : "";
			char *dstadd = rule_array[i].dstadd ? : "";
			char *sport = rule_array[i].sport ? : "";
			char *dport = rule_array[i].dport ? : "";
			char *connlimit = rule_array[i].connlimit ? : "";
			char *tcpmss_var = rule_array[i].tcpmss_var ? : "";
			char *pkg_state = rule_array[i].pkg_state ? : "";
			char *string_filter = rule_array[i].string_filter ? : "";
			char *natipadd = rule_array[i].natipadd ? : "";
			char *natport = rule_array[i].natport ? : "";
			
			dbus_message_iter_open_container(&iter_array,
											    DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct);
            
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].type);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].id);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].enable);
			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &ineth);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &outeth);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].srctype);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &srcadd);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].dsttype);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &dstadd);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].protocl);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].sptype);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &sport);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].dptype);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &dport);
			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &connlimit);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].act);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &tcpmss_var);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &pkg_state);
			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &string_filter);
			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].natiptype);
			
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_STRING,
											&natipadd);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_UINT32,
						                                 &rule_array[i].natpttype);

			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_STRING,
											&natport);

			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
		firewall_free_array(&rule_array, rule_num);
	}

	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;        	
}

/*********************************************************/

DBusMessage *
ac_manage_dbus_show_ntp_rule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	iter;	
	DBusMessageIter	iter_array;	
	DBusMessageIter	iter_struct;
	
	dbus_error_init(&err);

	int ret = AC_MANAGE_SUCCESS;

	u_long service_status = 0;
	u_long rule_num = 0;
	struct clientz_st *rule_array = NULL;

	ret = manage_show_ntp_configure(&rule_array,&rule_num);
    
	reply = dbus_message_new_method_return(message);
	                
	dbus_message_iter_init_append(reply, &iter);  


	dbus_message_iter_append_basic(&iter, 
				                                DBUS_TYPE_UINT32, 
				                                &ret);
	dbus_message_iter_append_basic(&iter, 
				                                DBUS_TYPE_UINT32, 
				                                &service_status);	
	dbus_message_iter_append_basic(&iter, 
				                                DBUS_TYPE_UINT32, 
				                                &rule_num);
    
	dbus_message_iter_open_container(&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING

										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING

									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	if(AC_MANAGE_SUCCESS == ret && rule_array ) {

		int i = 0;
		for(i = 0; i < rule_num; i++) {

			char *clitipz = rule_array[i].clitipz;
			char *ifper = rule_array[i].ifper;	
			char *timeflag = rule_array[i].timeflag;
			char *slotid = rule_array[i].slotid;

			dbus_message_iter_open_container(&iter_array,
											    DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct);            
			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &clitipz);

			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &ifper);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &timeflag);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &slotid);			

			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
	}
    MANAGE_FREE(rule_array);
	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;        	
}


DBusMessage *
ac_manage_dbus_add_ntpserver(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;

	char *serverip = NULL;
	char *serverpri = NULL;
	char *serversid = NULL;
	int opt_type = 0;
	int flagz = 0;

	dbus_error_init(&err);
	if_ntp_exist();
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &serverip,
	                            DBUS_TYPE_STRING, &serverpri,
	                            DBUS_TYPE_STRING, &serversid,
								DBUS_TYPE_INT32,&opt_type,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    //manage_add_ntp_server(struct clientz_st rule_st);
    if(opt_type == OPT_ADD)
	{
		if(ntp_upperserverip_duplication(serverip))
		{
			goto end_ntp;
		}
		ret = add_upper_ntp(NTP_XML_FPATH, serverip, serverpri, serversid);
	} 
	else
	{
		find_second_xmlnode(NTP_XML_FPATH,  NTP_UPCLIZ, NTP_CIPZ,serverip,&flagz);
		if(0 != flagz)
		{
			ret = del_second_xmlnode(NTP_XML_FPATH,  NTP_UPCLIZ , flagz);
		}
	}
    save_ntp_conf ();
    end_ntp:
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}

	

DBusMessage *
ac_manage_dbus_inside_ntp(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;
	int p_masterid = 0;
	
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
	}
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	set_default_inside_client_func(p_masterid);
    
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}
	

DBusMessage *
ac_manage_dbus_set_ntp_status(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *status = NULL;
	int ret = AC_MANAGE_SUCCESS;
	int p_masterid = 0;
	
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
	}
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &status,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(0 == strcmp(status,"enable"))
	{
		start_ntp();
	}
	if(0 == strcmp(status,"disable"))
	{
		stop_ntp();
	}
	if(0 == strcmp(status,"reload"))
	{
		restart_ntp();
	}
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}
DBusMessage *
ac_manage_dbus_clean_ntp(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *status = NULL;
	int ret = AC_MANAGE_SUCCESS;
	int p_masterid = 0;
	
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
	}
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	char cmd[128] = {0};
	snprintf(cmd,sizeof(cmd)-1,"sudo rm /opt/services/conf/ntp_conf.conf /opt/services/option/ntp_option");
	system(cmd);
	if_ntp_exist();
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}


DBusMessage *
ac_manage_dbus_config_ntp_pfm_requestpkts(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	unsigned int state = 0;
	char *ifName = NULL;
	char *ipstr = NULL;
	
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &ifName, 
								DBUS_TYPE_STRING, &ipstr,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
	if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
   	manage_log(LOG_DEBUG, "ac_manage_dbus_config_ntp_pfm_requestpkts:  ifName = %s, state = %d\n",  ifName, state);
	if(state)
	{ /*add pfm interface*/

		ret = ntp_config_pfm_table_entry(ifName, ipstr,1);
	}  
	else 
	{ /*delete pfm interface*/

	    	ret = ntp_config_pfm_table_entry(ifName, ipstr,0);
	}        

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}
	

DBusMessage *
ac_manage_dbus_set_timezone(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	char *area = NULL;
	char *city = NULL;
	int ret = AC_MANAGE_SUCCESS;
	char cmd[128] = {0};
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &area,
								DBUS_TYPE_STRING, &city,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	snprintf(cmd,sizeof(cmd)-1,"/usr/sbin/tzconfig %s %s",area,city);		
	system(cmd);
	
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}

DBusMessage *
ac_manage_dbus_config_snmp_sysoid_boardtype(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

	unsigned int sysoid = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &sysoid,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    config_snmp_sysoid_boardtype(sysoid);

    reply = dbus_message_new_method_return(message);
					
	return reply;
}

DBusMessage *
ac_manage_dbus_config_strict_access_level(DBusConnection *connection, DBusMessage *message, void *user_data) 
{
    
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;

	int level = 0;
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_INT32, &level,
								DBUS_TYPE_INVALID))){
		manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = manage_config_strict_access_level(level);

	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_INT32, &ret);

	return reply;
}

DBusMessage *
ac_manage_dbus_show_strict_access(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	iter;	
	int ret = AC_MANAGE_SUCCESS;
	int level = 0;

	level = manage_show_strict_access_level();
	dbus_error_init(&err);

	manage_log(LOG_DEBUG, "after show_strict_access, strict_access_level = %d\n", level);
	
	reply = dbus_message_new_method_return(message);
	                
	dbus_message_iter_init_append(reply, &iter);  

	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_INT32, &ret);
	
	dbus_message_iter_append_basic(&iter, 
									DBUS_TYPE_INT32, &level);
    
	return reply;        	
}


DBusMessage *
ac_manage_dbus_add_ntpclient(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	int ret = AC_MANAGE_SUCCESS;

	char *clientip = NULL;
	char *clientmask = NULL;
	char *clientsid = NULL;
	int opt_type = 0;
	int flagz = 0;
	dbus_error_init(&err);
	if_ntp_exist();
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_STRING, &clientip,
	                            DBUS_TYPE_STRING, &clientmask,
	                            DBUS_TYPE_STRING, &clientsid,
								DBUS_TYPE_INT32,&opt_type,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	
	
	if(opt_type == OPT_ADD)	
	{
		if(ntp_serverip_duplication(clientip) )
		{
			//return NULL;
			goto end_conf;
		}
		ret = add_ntp_server(NTP_XML_FPATH, clientip, clientmask);
	}
	else	
	{
		find_second_xmlnode(NTP_XML_FPATH,  NTP_SERVZ, NTP_SIPZ,clientip,&flagz);
		if(0 != flagz)
		{
			ret = del_second_xmlnode(NTP_XML_FPATH,  NTP_SERVZ , flagz);
		}
	}
    save_ntp_conf ();

    end_conf:
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}

/*********************************************************/

DBusMessage *
ac_manage_dbus_show_time(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	iter;	
	DBusMessageIter	iter_array;	
	DBusMessageIter	iter_struct;
	
	dbus_error_init(&err);

	int ret = AC_MANAGE_SUCCESS;

	u_long service_status = 0;
	struct timez_st rule_array;
	memset(&rule_array,0,sizeof(rule_array));

	ret = manage_show_time(&rule_array);
    
	reply = dbus_message_new_method_return(message);
	                
	dbus_message_iter_init_append(reply, &iter);  


	dbus_message_iter_append_basic(&iter, 
				                                DBUS_TYPE_UINT32, 
				                                &ret);
	dbus_message_iter_open_container(&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING

										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING

									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	if(AC_MANAGE_SUCCESS == ret ) {
			char *nowstr = rule_array.nowstr;
			char *utcstr = rule_array.utcstr;	
			char *slotid = rule_array.slotid;

			dbus_message_iter_open_container(&iter_array,
											    DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct);            
			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &nowstr);

			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &utcstr);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &slotid);			

			dbus_message_iter_close_container(&iter_array, &iter_struct);
	}
	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;        	
}
	

DBusMessage *
ac_manage_dbus_set_time(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	char *timestr = NULL;
	int ret = AC_MANAGE_SUCCESS;
	char cmd[128] = {0};
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &timestr,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	snprintf(cmd,sizeof(cmd)-1,"sudo date -u -s \"%s\" > /dev/null",timestr);		
	system(cmd);
	
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}

DBusMessage *
ac_manage_dbus_show_ntp_running_config(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;		

    unsigned int mode = 0;
    int ret = AC_MANAGE_SUCCESS;
    unsigned int moreConfig = 0;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
	                            DBUS_TYPE_UINT32, &mode,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    struct running_config *configHead = NULL;
    ret = ntp_show_running_config(&configHead, mode);

    
    char *showStr = (char *)malloc(256);
    if(NULL == showStr) {
        ret = AC_MANAGE_MALLOC_ERROR;
    }    

    reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append(reply, &iter);
		
	dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &ret);

    if(AC_MANAGE_SUCCESS == ret && configHead) {
    
        moreConfig = 1;
        dbus_message_iter_append_basic(&iter,
        								 DBUS_TYPE_UINT32,
        								 &moreConfig);

        struct running_config *configNode = NULL;
        for(configNode = configHead; NULL != configNode; configNode = configNode->next) {

            memset(showStr, 0, 256);
            memcpy(showStr, configNode->showStr, 256);

    	    dbus_message_iter_append_basic(&iter,
            								 DBUS_TYPE_STRING,
            								 &showStr);

            if(NULL != configNode->next) {
                dbus_message_iter_append_basic(&iter,
                								 DBUS_TYPE_UINT32,
                								 &moreConfig);  
            }        								 
        }        								 
        moreConfig = 0;
    }

    dbus_message_iter_append_basic(&iter,
    								 DBUS_TYPE_UINT32,
    								 &moreConfig);

    MANAGE_FREE(showStr);
    manage_free_running_config(&configHead);
    
    return reply;        
}


DBusMessage *
ac_manage_dbus_add_syslogrule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	if_syslog_exist();
	int ret = AC_MANAGE_SUCCESS;

	char *udpstr = NULL;
	char *ipstr =  NULL;
	char *portstr =  NULL;
	char *filter =  NULL;
	char *flevel =  NULL;
	char *id = NULL;
	char timeflag[50]={0};
	int opt_type = 0;
	int ftime=-1,fid=0;	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &udpstr,
								DBUS_TYPE_STRING, &ipstr,
								DBUS_TYPE_STRING, &portstr,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_STRING, &flevel,
								DBUS_TYPE_STRING, &id,
								DBUS_TYPE_INT32,&opt_type,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	
	
	if(opt_type == OPT_ADD)	
	{
			memset(timeflag,0,sizeof(timeflag));
		    ftime=if_dup_info(udpstr, ipstr,portstr, flevel, &timeflag);
			find_second_xmlnode(XML_FPATH, NODE_LOG, NODE_INDEXZ, id, &fid);	
			if(fid!=0)
			{
				goto end_conf;
			}
	        else if(ftime==1)
			{
				goto end_conf;
			}
			else if((ftime==0)&&(fid==0))
			{
				add_syslog_serve_web(XML_FPATH, CONF_ENABLE, ipstr, portstr, filter, udpstr,flevel,id);
			}
		}
	else	
	{
		del_syslog_serve_web(XML_FPATH,ipstr, portstr, filter, udpstr,flevel);
	}
	save_syslog_file();	
    end_conf:
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}
	

DBusMessage *
ac_manage_dbus_set_syslog_status(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *status = NULL;
	char *opt_type = NULL;
	int ret = AC_MANAGE_SUCCESS;
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &status,
                                DBUS_TYPE_STRING, &opt_type,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if_syslog_exist();
	if(0 == strncmp(opt_type,"all",3))
	{
		mod_first_xmlnode(XML_FPATH, NODE_LSTATUS, status);
	}
	else if(0 == strncmp(opt_type,"syn",3))
	{
		mod_first_xmlnode(XML_FPATH, IF_SYNFLOOD, status);
	}
	else if(0 == strncmp(opt_type,"eag",3))
	{
		mod_first_xmlnode(XML_FPATH, IF_EAGLOG, status);
	}
	save_syslog_file();
	restart_syslog();

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}

/*********************************************************/

DBusMessage *
ac_manage_dbus_show_syslog_rule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	iter;	
	DBusMessageIter	iter_array;	
	DBusMessageIter	iter_struct;
	
	dbus_error_init(&err);

	int ret = AC_MANAGE_SUCCESS;

	u_long service_status = 0;
	u_long rule_num = 0;
	struct syslogrule_st *rule_array = NULL;

	ret = manage_show_syslog_configure(&rule_array,&rule_num);
    
	reply = dbus_message_new_method_return(message);
	                
	dbus_message_iter_init_append(reply, &iter);  


	dbus_message_iter_append_basic(&iter, 
				                                DBUS_TYPE_UINT32, 
				                                &ret);
	dbus_message_iter_append_basic(&iter, 
				                                DBUS_TYPE_UINT32, 
				                                &service_status);	
	dbus_message_iter_append_basic(&iter, 
				                                DBUS_TYPE_UINT32, 
				                                &rule_num);
    
	dbus_message_iter_open_container(&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING

										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING
										DBUS_TYPE_STRING_AS_STRING

									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	if(AC_MANAGE_SUCCESS == ret && rule_array ) {

		int i = 0;
		for(i = 0; i < rule_num; i++) {

			char *udpstr = rule_array[i].udpstr;
			char *ipstr = rule_array[i].ipstr;	
			char *portstr = rule_array[i].portstr;
			char *flevel = rule_array[i].flevel;
			char *id = rule_array[i].id;

			dbus_message_iter_open_container(&iter_array,
											    DBUS_TYPE_STRUCT,
												NULL,
												&iter_struct);            
			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &udpstr);

			
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &ipstr);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &portstr);

			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &flevel);	
			dbus_message_iter_append_basic(&iter_struct,
						                                 DBUS_TYPE_STRING,
						                                 &id);	

			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
	}
    MANAGE_FREE(rule_array);
	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;        	
}
	

DBusMessage *
ac_manage_dbus_save_syslog(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	char *status = NULL;
	char *opt_type = NULL;
	int ret = AC_MANAGE_SUCCESS;
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &status,
                                DBUS_TYPE_STRING, &opt_type,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	char cmdstr[128] = {0};
	char des_path[128] = {0};

	///1 save syslog to master borad
	if(0 == strcmp(status,"1"))
	{
		snprintf(cmdstr,sizeof(cmdstr)-1,"sudo /usr/bin/syslog_save.sh >/dev/null");
		system(cmdstr);	
		snprintf(des_path,sizeof(des_path)-1,"/var/run/systemlog%d.tar.gz",local_slotID);
		ret = dcli_bsd_copy_file_to_board_v2(connection,active_master_slotID,"/var/log/systemlog.tar.gz",des_path,1,0);
		memset(cmdstr,0,sizeof(cmdstr)-1);
		snprintf(cmdstr,sizeof(cmdstr)-1,"sudo rm /var/log/systemlog.tar* >/dev/null");
		system(cmdstr);
	}
	///2 savecycle
	else if(0 == strcmp(status,"2"))
	{
		mod_first_xmlnode(XML_FPATH, NODE_SAVECYCLE, opt_type);
		memset(cmdstr,0,sizeof(cmdstr)-1);
		snprintf(cmdstr,sizeof(cmdstr)-1,"sudo syslog_cron.sh >/dev/null");
		system(cmdstr);
	}
	///3 delete all logfile
	else if(0 == strcmp(status,"3"))
	{
		memset(cmdstr,0,sizeof(cmdstr)-1);
		snprintf(cmdstr,sizeof(cmdstr)-1,"sudo echo "" > /var/log/syslogservice.log");
		system(cmdstr);
	}
	
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}


DBusMessage *
ac_manage_dbus_config_syslogupload_pfm_requestpkts(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	unsigned int state = 0;
	unsigned int srcport = 0;
	char *ifName = NULL;
	char *ipstr = NULL;
	
    
	int ret = AC_MANAGE_SUCCESS;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &ifName, 
								DBUS_TYPE_STRING, &ipstr,
								DBUS_TYPE_UINT32, &srcport,
								DBUS_TYPE_UINT32, &state,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
	    
	if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
   	manage_log(LOG_DEBUG, "ac_manage_dbus_config_ntp_pfm_requestpkts:  ifName = %s, state = %d\n",  ifName, state);
	if(state)
	{ /*add pfm interface*/

		ret = syslog_config_pfm_table_entry(ifName, ipstr,srcport,1);
	}  
	else 
	{ /*delete pfm interface*/

	    	ret = syslog_config_pfm_table_entry(ifName, ipstr,srcport,0);
	}        

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 

    return reply;
}

DBusMessage *ac_manage_dbus_download_internal_portal(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	char *addr, *path, *usr, *pass;
	int ret = 1;
	
	dbus_error_init(&err);	

	dbus_message_iter_init(message, &iter);
	dbus_message_iter_get_basic(&iter, &addr);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &path);		
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &usr);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &pass);	
	
	ret = web_download_internal_portal(addr, path, usr, pass);

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}

DBusMessage *ac_manage_dbus_show_internal_portal(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *dir[4];
	int count, i;
	
	dbus_error_init(&err);	

	dbus_message_iter_init(message, &iter);

	web_show_internal_portal(dir, &count);

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&count);
    for(i = 0; i < count; i++)
    {
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&(dir[i]));
    }
    
    return reply;        	
	
}

DBusMessage *ac_manage_dbus_delete_internal_portal(DBusConnection *connection, DBusMessage *message, void *user_data) {

	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *dir;
	int ret;
	
	dbus_error_init(&err);	

	dbus_message_iter_init(message, &iter);
	dbus_message_iter_get_basic(&iter, &dir);	

	ret = web_del_internal_portal(dir);

    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
 
    return reply;        	
	
}


DBusMessage *
ac_manage_dbus_config_syslogrule(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	
	if_syslog_exist();
	int ret = AC_MANAGE_SUCCESS;

	char *rulename = NULL;
	char *ipstr =  NULL;
	char *portstr =  NULL;
	char *filter =  NULL;
	char *keyword = NULL;
	int oper = 0;
	int opt_type = 0;
	int ftime=-1,fid=0;	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &rulename,
								DBUS_TYPE_STRING, &ipstr,
								DBUS_TYPE_STRING, &portstr,
								DBUS_TYPE_STRING, &filter,
								DBUS_TYPE_STRING, &keyword,
								DBUS_TYPE_INT32,&oper,
								DBUS_TYPE_INT32,&opt_type,								
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {
		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	manage_log(LOG_WARNING, "value  rulename is: %s\n",rulename );
	manage_log(LOG_WARNING, "value  ipstr is: %s\n",ipstr );
	manage_log(LOG_WARNING, "value  portstr is: %s\n",portstr );
	manage_log(LOG_WARNING, "value  filter is: %s\n",filter );
	manage_log(LOG_WARNING, "value  keyword is: %s\n",keyword );
	manage_log(LOG_WARNING, "value  oper is: %d\n",oper );
	manage_log(LOG_WARNING, "value  opt_type is: %d\n",opt_type );


	if(FIL_OPT == oper)
	{
		if(opt_type == OPT_ADD)	
		{
			add_filter(rulename, keyword);
		}
		else if(opt_type == OPT_DEL)
		{
			del_syslogruler(XML_FPATH, NODE_FILTER,NODE_VALUE,rulename);
			del_syslogruler(XML_FPATH, NODE_LOG,NODE_FILTER,rulename);
		}
	}
	else if(DES_OPT == oper)
	{
		if(opt_type == OPT_ADD)	
		{
			add_destination_rule(rulename,ipstr,portstr);
		}
		else if(opt_type == OPT_DEL)	
		{	
			del_syslogruler(XML_FPATH, NODE_DES,NODE_VALUE,rulename);
			del_syslogruler(XML_FPATH, NODE_LOG,CH_DEST,rulename);
		}
	}
	else if(LOG_OPT == oper)
	{
		if(opt_type == OPT_ADD)	
		{
			add_log_rule(rulename, keyword);
		}
		else if(opt_type == OPT_DEL)	
		{
			del_syslogruler(XML_FPATH, NODE_LOG,NODE_MARKZ,rulename);
		}
	}	
	
	save_syslog_file();	
    end_conf:
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
}
	

DBusMessage *
ac_manage_dbus_set_acinfo_value(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *status = NULL;
	char *key = NULL;
	int opt_type = 0;
	int ret = AC_MANAGE_SUCCESS;
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &status,
								DBUS_TYPE_STRING, &key,
                                DBUS_TYPE_INT32, &opt_type,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	char cmd[512] = { 0 };
	if(OPT_ADD == opt_type)
	{
		if(0 == strcmp(status,CON_TYPE))
		{
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd, sizeof(cmd)-1, "echo -e \"%s\" > /var/run/ac_contact_info", key);
			system(cmd);
		}
		else if(0 == strcmp(status,LOC_TYPE))
		{
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd, sizeof(cmd)-1, "sudo sys_location.sh  %s >/dev/null", key);
			system(cmd);
		}
		else if(0 == strcmp(status,NET_TYPE))
		{
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd, sizeof(cmd)-1, "sudo net_elemnet.sh  %s >/dev/null", key);
			system(cmd);
		}

	}
	else if(OPT_DEL == opt_type)
	{
		if(0 == strcmp(status,CON_TYPE))
		{
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd, sizeof(cmd)-1, "sudo echo "" > /var/run/ac_contact_info");
			system(cmd);
		}
		else if(0 == strcmp(status,LOC_TYPE))
		{
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd, sizeof(cmd)-1, "sudo echo "" > /var/run/sys_location");
			system(cmd);
		}
		else if(0 == strcmp(status,NET_TYPE))
		{
		
		}
	}
	
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}
	

DBusMessage *
ac_manage_dbus_set_bkacinfo_value(DBusConnection *connection, DBusMessage *message, void *user_data) {
    
	DBusMessage *reply = NULL;	
	DBusError err;
	DBusMessageIter	 iter;	

	char *status = NULL;
	char *key = NULL;
	char *insid = NULL;
	char *netip = NULL;	
	int opt_type = 0;
	int ret = AC_MANAGE_SUCCESS;
	dbus_error_init(&err);	
	if (!(dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &status,
								DBUS_TYPE_STRING, &key,
								DBUS_TYPE_STRING, &insid,
								DBUS_TYPE_STRING, &netip,
                                DBUS_TYPE_INT32, &opt_type,
								DBUS_TYPE_INVALID))){

        manage_log(LOG_WARNING, "Unable to get input args\n");
		if (dbus_error_is_set(&err)) {		
            manage_log(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(access(ACBACKUPFILE,0) != 0)
	{
		new_xml_file(ACBACKUPFILE);
	}
	if(OPT_ADD == opt_type)
	{
		if(0 == strcmp(status,STATUS_TYPE))
		{
			mod_first_xmlnode(ACBACKUPFILE, AC_STATUS, key);
		}
		else if(0 == strcmp(status,MODE_TYPE))
		{
			mod_first_xmlnode(ACBACKUPFILE, AC_MODE, key);
		}
		else if(0 == strcmp(status,NETIP_TYPE))
		{
			mod_insbk_xmlnode(ACBACKUPFILE, AC_NETIP, insid, netip);		
		}
		else if(0 == strcmp(status,IDEN_TYPE))
		{
			mod_first_xmlnode(ACBACKUPFILE, AC_IDENTITY,key);
		}
	}
	else if(OPT_DEL == opt_type)
	{

	}
	
    reply = dbus_message_new_method_return(message);
                    
    dbus_message_iter_init_append (reply, &iter);   
    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret); 
    
    return reply;        	
	
}

