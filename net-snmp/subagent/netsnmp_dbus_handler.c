
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "ac_manage_def.h"

#include "ws_dbus_def.h"
#include "ws_dbus_list_interface.h"
#include "ws_snmpd_dbus_interface.h"

#include "netsnmp_private_interface.h"

#include "netsnmp_dbus_handler.h"


int 
netsnmp_dbus_had_master_advertise(DBusMessage * message) {    

	DBusError err;
	dbus_error_init(&err);
    
	unsigned int state = 0, hadID = 0, vrrID = 0;
	
	if (!(dbus_message_get_args ( message, &err,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_UINT32,&hadID,
								DBUS_TYPE_UINT32,&vrrID,
								DBUS_TYPE_INVALID))){
				
		if (dbus_error_is_set(&err)) {
			syslog(LOG_WARNING, "netsnmp_dbus_had_master_advertise: %s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return SNMPD_DBUS_ERROR;
	}

	syslog(LOG_NOTICE,"netsnmp_dbus_had_master_advertise:state = %d, vrrID = %d, hadID = %d\n", state, vrrID, hadID);
    had_hansi_message_handler(state);

    return SNMPD_DBUS_SUCCESS;
}


DBusMessage * 
netsnmp_dbus_interface_show_dbus_connection_list(DBusConnection *connection, DBusMessage *message, void *user_data){

	DBusMessage *reply = NULL;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusMessageIter  iter_struct;	
	DBusMessageIter	 iter_state_array;
	DBusMessageIter  iter_state_struct;

	DBusError err;
	dbus_error_init(&err);
	
	int ret = SNMPD_DBUS_SUCCESS;

	unsigned char WlanID = 0;	

    instance_parameter *paraHead = NULL, *paraNode = NULL;
    
    list_instance_parameter(&paraHead, SNMPD_INSTANCE_ALL);
    
    unsigned int slot_count = 0;
    unsigned int slot_id = 0;
    
    struct snmpdInstanceInfo *InstHead = NULL;
    struct snmpdInstanceInfo *temp = NULL;
    
    for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
    {
        if(slot_id != paraNode->parameter.slot_id){
        
            slot_id = paraNode->parameter.slot_id;
            
            temp = (struct snmpdInstanceInfo *)malloc(sizeof(struct snmpdInstanceInfo));
            if(NULL == temp){
                syslog(LOG_DEBUG, "netsnmp_dbus_interface_show_dbus_connection_list:malloc error\n");
                ret = SNMPD_DBUS_ERROR;
                break;
            }
            memset(temp, 0, sizeof(struct snmpdInstanceInfo));

            temp->slot_id = paraNode->parameter.slot_id;
            
            if(paraNode->connection)
                temp->dbus_connection_state = 1;

            if(SNMPD_VRRP_MASTER == paraNode->parameter.vrrp_state) {
                temp->master_count++;
            }

            temp->instance_state[paraNode->parameter.local_id][paraNode->parameter.instance_id] = paraNode->parameter.vrrp_state;
                  
            temp->next = InstHead;
            InstHead = temp;

            slot_count++;
        }
        else {
            if(SNMPD_VRRP_MASTER == paraNode->parameter.vrrp_state) {
                temp->master_count++;
            }
            temp->instance_state[paraNode->parameter.local_id][paraNode->parameter.instance_id] = paraNode->parameter.vrrp_state;
        }    
    }
    
    free_instance_parameter_list(&paraHead);
    
	reply = dbus_message_new_method_return(message);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &slot_count);	

    dbus_message_iter_open_container (&iter,
                                    DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                    DBUS_TYPE_UINT32_AS_STRING
                                    DBUS_TYPE_UINT32_AS_STRING
                                    DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_TYPE_ARRAY_AS_STRING
                                        DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                        DBUS_TYPE_UINT32_AS_STRING
                                        DBUS_STRUCT_END_CHAR_AS_STRING
                                    DBUS_STRUCT_END_CHAR_AS_STRING,
                                    &iter_array);
									 
    if(slot_count > 0) {

        temp = NULL;
        for(temp = InstHead; NULL != temp; temp = temp->next) {
            dbus_message_iter_open_container(&iter_array,
                                             DBUS_TYPE_STRUCT,
                                             NULL,
                                             &iter_struct);

            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(temp->slot_id));                                 
                                             
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(temp->dbus_connection_state));
                                            
            dbus_message_iter_append_basic(&iter_struct,
                                            DBUS_TYPE_UINT32,
                                            &(temp->master_count));

            
    		dbus_message_iter_open_container(&iter_struct,
    										 DBUS_TYPE_ARRAY,
		    							     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                             DBUS_TYPE_UINT32_AS_STRING
                                             DBUS_STRUCT_END_CHAR_AS_STRING,
    									     &iter_state_array);		

            
            int i =0, j = 0;
            for(i =0; i < VRRP_TYPE_NUM; i ++) {
                for(j = 0; j <= INSTANCE_NUM; j++) {
                    
                    dbus_message_iter_open_container(&iter_state_array,
                                                     DBUS_TYPE_STRUCT,
                                                     NULL,
                                                     &iter_state_struct);
                                                     
                    dbus_message_iter_append_basic(&iter_state_struct,
                                                    DBUS_TYPE_UINT32,
                                                    &(temp->instance_state[i][j]));

					dbus_message_iter_close_container (&iter_state_array, &iter_state_struct);
                }
            }
            
            dbus_message_iter_close_container (&iter_struct, &iter_state_array);
            dbus_message_iter_close_container (&iter_array, &iter_struct);
        }
        
        dbus_message_iter_close_container (&iter, &iter_array);
    }                                        

    free_snmpd_dbus_connection_list(&InstHead);
	return reply;
}

DBusMessage * 
netsnmp_dbus_interface_config_snmp_log_debug(DBusConnection *connection, DBusMessage *message, void *user_data){
    
	DBusMessage *reply = NULL;	
	DBusMessageIter	 iter;		
    
	DBusError err;
	dbus_error_init(&err);
	
	int ret = SNMPD_DBUS_SUCCESS;
	unsigned int debugLevel = 0;
	
	if(!(dbus_message_get_args(message, &err,
								DBUS_TYPE_UINT32, &debugLevel,
								DBUS_TYPE_INVALID))){

        syslog(LOG_WARNING, "netsnmp_dbus_interface_config_snmp_log_debug: Unable to get input args\n");
		if(dbus_error_is_set(&err)) {		
            syslog(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

    netsnmp_config_log_debug(debugLevel);

    reply = dbus_message_new_method_return(message);
					
	dbus_message_iter_init_append(reply, &iter);	
	dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&ret); 

	return reply;
}


