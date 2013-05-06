#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>

#include "ac_manage_def.h"

#include "eag_errcode.h"
#include "eag_conf.h"
#include "eag_interface.h"
#include "ac_manage_sample_interface.h"


int
ac_manage_show_radius_config(DBusConnection *connection, struct instance_radius_config **config_array, unsigned int *config_num) {
    syslog(LOG_DEBUG, "enter ac_manage_show_radius_config");

    if(NULL == connection || NULL == config_array || NULL == config_num) {
        syslog(LOG_DEBUG, "ac_manage_show_radius_config: input para error!\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *config_array = NULL;
    *config_num = 0;

    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_struct;
	DBusMessageIter  iter_sub_array;
	DBusMessageIter  iter_sub_struct;
	

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_RADIUS_CONFIG);

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
	dbus_message_iter_get_basic(&iter, &temp_num);
	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);  

    if(AC_MANAGE_SUCCESS == ret && temp_num) {
        struct instance_radius_config *temp_array = (struct instance_radius_config *)calloc(temp_num, sizeof(struct instance_radius_config));
        if(NULL == temp_array) {
            syslog(LOG_WARNING, "ac_manage_show_radius_config: calloc temp_array fail!\n");
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {
            
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].slot_id));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].local_id));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].instance_id));
                        
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(temp_array[i].radiusconf.current_num));

            dbus_message_iter_next(&iter_struct);  
            dbus_message_iter_recurse(&iter_struct,&iter_sub_array); 
            

            int j = 0;
            for(j = 0; j < temp_array[i].radiusconf.current_num; j++) {
                char *domain = NULL;
                char *auth_secret = NULL;
                char *acct_secret = NULL;
                char *backup_auth_secret = NULL;
                char *backup_acct_secret = NULL;
                
                dbus_message_iter_recurse(&iter_sub_array, &iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &domain);


                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].auth_ip));
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].auth_port));

                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &auth_secret);

                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].auth_secretlen));


                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].acct_ip));
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].acct_port));
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &acct_secret);
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].acct_secretlen));


                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].backup_auth_ip));
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].backup_auth_port));
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &backup_auth_secret);
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].backup_auth_secretlen));
                
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].backup_acct_ip));
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].backup_acct_port));
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &backup_acct_secret);
                
                dbus_message_iter_next(&iter_sub_struct);
                dbus_message_iter_get_basic(&iter_sub_struct, &(temp_array[i].radiusconf.radius_srv[j].backup_acct_secretlen));


                strncpy(temp_array[i].radiusconf.radius_srv[j].domain, domain, sizeof(temp_array[i].radiusconf.radius_srv[j].domain) - 1);
                strncpy(temp_array[i].radiusconf.radius_srv[j].auth_secret, auth_secret, sizeof(temp_array[i].radiusconf.radius_srv[j].auth_secret) - 1);
                strncpy(temp_array[i].radiusconf.radius_srv[j].acct_secret, acct_secret, sizeof(temp_array[i].radiusconf.radius_srv[j].acct_secret) - 1);
                strncpy(temp_array[i].radiusconf.radius_srv[j].backup_auth_secret, backup_auth_secret, sizeof(temp_array[i].radiusconf.radius_srv[j].backup_auth_secret) - 1);
                strncpy(temp_array[i].radiusconf.radius_srv[j].backup_acct_secret, backup_acct_secret, sizeof(temp_array[i].radiusconf.radius_srv[j].backup_acct_secret) - 1);

                dbus_message_iter_next(&iter_sub_array);
            }

            dbus_message_iter_next(&iter_array);
        }
        
        *config_array = temp_array;
        *config_num = temp_num;
    }
    
    dbus_message_unref(reply);

    syslog(LOG_DEBUG, "exit ac_manage_show_radius_config");
    return ret;
}



int
ac_manage_show_portal_config(DBusConnection *connection, struct instance_portal_config **config_array, unsigned int *config_num) {
    syslog(LOG_DEBUG, "enter ac_manage_show_portal_config");

    if(NULL == connection || NULL == config_array || NULL == config_num) {
        syslog(LOG_DEBUG, "ac_manage_show_portal_config: input para error!\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *config_array = NULL;
    *config_num = 0;

    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;	

    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_PORTAL_CONFIG);

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
	dbus_message_iter_get_basic(&iter, &temp_num);
	
    if(AC_MANAGE_SUCCESS == ret && temp_num) {
        struct instance_portal_config *temp_array = (struct instance_portal_config *)calloc(temp_num, sizeof(struct instance_portal_config));
        if(NULL == temp_array) {
            syslog(LOG_WARNING, "ac_manage_show_portal_config: calloc temp_array fail!\n");
            dbus_message_unref(reply);
            return AC_MANAGE_MALLOC_ERROR;
        }

        int i = 0;
        for(i = 0; i < temp_num; i++) {
            
            dbus_message_iter_next(&iter);  
            dbus_message_iter_get_basic(&iter, &(temp_array[i].slot_id));
            
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &(temp_array[i].local_id));
            
            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &(temp_array[i].instance_id));

            dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter, &(temp_array[i].portalconf.current_num));

            int j = 0;
            for(j = 0; j < temp_array[i].portalconf.current_num; j++) {

                char *essid = NULL;
                char *intf = NULL;
                char *portal_url = NULL;
                char *domain = NULL;
                char *acname = NULL;

                
                dbus_message_iter_next(&iter);
                dbus_message_iter_get_basic(&iter, &(temp_array[i].portalconf.portal_srv[j].key_type));

                switch(temp_array[i].portalconf.portal_srv[j].key_type) {
                    case PORTAL_KEYTYPE_ESSID:
                        dbus_message_iter_next(&iter);
                        dbus_message_iter_get_basic(&iter, &essid);
                        strncpy(temp_array[i].portalconf.portal_srv[j].key.essid, essid, sizeof(temp_array[i].portalconf.portal_srv[j].key.essid) - 1);
                        break;
                        
                    case PORTAL_KEYTYPE_WLANID:
                        dbus_message_iter_next(&iter);
                        dbus_message_iter_get_basic(&iter, &(temp_array[i].portalconf.portal_srv[j].key.wlanid));
                        break;

                    case PORTAL_KEYTYPE_VLANID:
                        dbus_message_iter_next(&iter);
                        dbus_message_iter_get_basic(&iter, &(temp_array[i].portalconf.portal_srv[j].key.vlanid));
                        break;
                        
                    case PORTAL_KEYTYPE_WTPID:
                        dbus_message_iter_next(&iter);
                        dbus_message_iter_get_basic(&iter, &(temp_array[i].portalconf.portal_srv[j].key.wtpid));
                        break;

                    case PORTAL_KEYTYPE_INTF:
                        dbus_message_iter_next(&iter);
                        dbus_message_iter_get_basic(&iter, &intf);                        
                        strncpy(temp_array[i].portalconf.portal_srv[j].key.intf, intf, sizeof(temp_array[i].portalconf.portal_srv[j].key.intf) - 1);
                        break;

                    default:
                        syslog(LOG_WARNING, "ac_manage_show_portal_config: switch %d unknow\n", temp_array[i].portalconf.portal_srv[j].key_type);
                        break;    
                }

                dbus_message_iter_next(&iter);
                dbus_message_iter_get_basic(&iter, &portal_url);
                
                dbus_message_iter_next(&iter);
                dbus_message_iter_get_basic(&iter, &(temp_array[i].portalconf.portal_srv[j].ntf_port));

                dbus_message_iter_next(&iter);
                dbus_message_iter_get_basic(&iter, &domain);

                dbus_message_iter_next(&iter);
                dbus_message_iter_get_basic(&iter, &acname);

                dbus_message_iter_next(&iter);
                dbus_message_iter_get_basic(&iter, &(temp_array[i].portalconf.portal_srv[j].ip));

                strncpy(temp_array[i].portalconf.portal_srv[j].portal_url, portal_url, sizeof(temp_array[i].portalconf.portal_srv[j].portal_url) - 1);
                strncpy(temp_array[i].portalconf.portal_srv[j].domain, domain, sizeof(temp_array[i].portalconf.portal_srv[j].domain) - 1);
                strncpy(temp_array[i].portalconf.portal_srv[j].acname, acname, sizeof(temp_array[i].portalconf.portal_srv[j].acname) - 1);
            }
        }
        
        *config_array = temp_array;
        *config_num = temp_num;
    }
    
    dbus_message_unref(reply);

    syslog(LOG_DEBUG, "exit ac_manage_show_portal_config");
    return ret;
}


