/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* dot11AKMConfigTable.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* 
*
*
*******************************************************************************/


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "dot11AKMConfigTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "ws_security.h"
#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"
//#include "autelanWtpGroup.h"

#define AKMCONFIG "1.10.8"

#define __DEBUG	1




struct dot11AKMConfigTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long SecurityID;
	long local_WlanID;
    long AKMWlanID;

    /* Column values */
    char *AKMAuthenticationSuite;
    long AKMAuthenticationSuiteEnabled;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11AKMConfigTable_entry *next;
};
void dot11AKMConfigTable_load(void);
void dot11AKMConfigTable_removeEntry( struct dot11AKMConfigTable_entry *entry );


/** Initializes the dot11AKMConfigTable module */
void
init_dot11AKMConfigTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11AKMConfigTable();
}

/** Initialize the dot11AKMConfigTable table by defining its contents and how it's structured */
void
initialize_table_dot11AKMConfigTable(void)
{
    static oid dot11AKMConfigTable_oid[128] = {0};
    size_t dot11AKMConfigTable_oid_len   = 0;
	mad_dev_oid(dot11AKMConfigTable_oid,AKMCONFIG,&dot11AKMConfigTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11AKMConfigTable",     dot11AKMConfigTable_handler,
              dot11AKMConfigTable_oid, dot11AKMConfigTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: AKMWlanID */
                           0);
    table_info->min_column = AKMCONFIGMIN;
    table_info->max_column = AKMCONFIGMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11AKMConfigTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11AKMConfigTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11AKMConfigTable_load, dot11AKMConfigTable_removeEntry,dot11AKMConfigTable_oid, dot11AKMConfigTable_oid_len));
    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11AKMConfigTable_entry  *dot11AKMConfigTable_head;

/* create a new row in the (unsorted) table */
struct dot11AKMConfigTable_entry *
dot11AKMConfigTable_createEntry(
				 dbus_parameter parameter,
                 long  SecurityID,
                 long local_WlanID,
                 long  AKMWlanID,
                 char *AKMAuthenticationSuite,
			    long AKMAuthenticationSuiteEnabled
                ) {
    struct dot11AKMConfigTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11AKMConfigTable_entry);
    if (!entry)
        return NULL;
    
	memset( entry, 0, sizeof(struct dot11AKMConfigTable_entry) );
	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->SecurityID = SecurityID;
	entry->local_WlanID = local_WlanID;
    entry->AKMWlanID = AKMWlanID;
	entry->AKMAuthenticationSuite = strdup(AKMAuthenticationSuite);
	entry->AKMAuthenticationSuiteEnabled = AKMAuthenticationSuiteEnabled;
    entry->next = dot11AKMConfigTable_head;
    dot11AKMConfigTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11AKMConfigTable_removeEntry( struct dot11AKMConfigTable_entry *entry ) {
    struct dot11AKMConfigTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11AKMConfigTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11AKMConfigTable_head = ptr->next;
    else
        prev->next = ptr->next;
    FREE_OBJECT(entry->AKMAuthenticationSuite);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}


void dot11AKMConfigTable_load()
{
    snmp_log(LOG_DEBUG, "enter dot11AKMConfigTable_load\n");

    struct dot11AKMConfigTable_entry *temp = NULL;	
    while( dot11AKMConfigTable_head )
    {
    	temp=dot11AKMConfigTable_head->next;
    	dot11AKMConfigTable_removeEntry(dot11AKMConfigTable_head);
    	dot11AKMConfigTable_head=temp;
    }
	char id[10] = { 0 };
	
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wlan_list\n");
    messageHead = list_connection_call_dbus_method(show_wlan_list, SHOW_ALL_WTP_TABLE_METHOD);
	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wlan_list,messageHead=%p\n", messageHead);

	if(messageHead)
	{
		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
		{
		    void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(messageNode->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                continue;
                
		    DCLI_WLAN_API_GROUP *WLANINFO = messageNode->message;
		    if(WLANINFO)
		    {
		        int i = 0;
    			for(i=0;i<WLANINFO->wlan_num;i++)
    			{  
    				memset(id,0,10);
					if(WLANINFO->WLAN[i])
					{
						snprintf(id,sizeof(id)-1,"%d",WLANINFO->WLAN[i]->WlanID);
					}
    				
    				int ret = 0;
    				struct dcli_security *sec = NULL;
                    ret = show_wlanid_security_wapi_config_cmd(messageNode->parameter, connection,id,&sec);
    			    if(ret == 1)
    			 	{
						if(WLANINFO->WLAN[i])
						{
							unsigned int wlanid = local_to_global_ID(messageNode->parameter, WLANINFO->WLAN[i]->WlanID, WIRELESS_MAX_NUM);

							int ret1 = 0;
							struct dcli_security *sec1 = NULL;
							char AKMAuthenticationSuite[20] = { 0 };
							ret1 = show_security_one(messageNode->parameter,connection,sec->SecurityID,&sec1);
							memset(AKMAuthenticationSuite,0,sizeof(AKMAuthenticationSuite));
							if(ret1 == 1)
							{
								if(sec1->SecurityType == WAPI_AUTH)
								{
									strncpy(AKMAuthenticationSuite,"00-14-72-01",sizeof(AKMAuthenticationSuite)-1);
								}
								else if(sec1->SecurityType==WAPI_PSK)
								{
									strncpy(AKMAuthenticationSuite,"00-14-72-02",sizeof(AKMAuthenticationSuite)-1);
								}
								Free_security_one(sec1);
							}
							
							dot11AKMConfigTable_createEntry(messageNode->parameter,
															sec->SecurityID,
															WLANINFO->WLAN[i]->WlanID,
															wlanid,
															AKMAuthenticationSuite,
															(sec->AuthenticationSuiteEnable==1)?1:2);
 						}
    			 	}
                    else if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(messageNode->parameter.slot_id);
                        break;
            	    } 
    				Free_wlanid_security_wapi_info(sec);
    			}
    	    }
		}
		free_dbus_message_list(&messageHead, Free_wlan_head);
	}

	snmp_log(LOG_DEBUG, "exit dot11AKMConfigTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11AKMConfigTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{   
    if(dot11AKMConfigTable_head == NULL)
		{
			return NULL;
		}
    *my_loop_context = dot11AKMConfigTable_head;
    return dot11AKMConfigTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11AKMConfigTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11AKMConfigTable_entry *entry = (struct dot11AKMConfigTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->AKMWlanID, sizeof(entry->AKMWlanID) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }	
	return put_index_data; 
}


/** handles requests for the dot11AKMConfigTable table */
int
dot11AKMConfigTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11AKMConfigTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AKMConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);            
			if( !table_entry )
			{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}	  
            switch (table_info->colnum) {
            case COLUMN_AKMWLANID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->AKMWlanID,
                                          sizeof(long));
                break;
            case COLUMN_AKMAUTHENTICATIONSUITE:

                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->AKMAuthenticationSuite,
                                         strlen(table_entry->AKMAuthenticationSuite));

                break;
            case COLUMN_AKMAUTHENTICATIONSUITEENABLED:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->AKMAuthenticationSuiteEnabled,
                                          sizeof(long));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AKMConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_AKMAUTHENTICATIONSUITEENABLED:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            default:
                netsnmp_set_request_error( reqinfo, request,
                                           SNMP_ERR_NOTWRITABLE );
                return SNMP_ERR_NOERROR;
            }
        }
        break;

    case MODE_SET_RESERVE2:
        break;

    case MODE_SET_FREE:
        break;

    case MODE_SET_ACTION:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AKMConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
            
			if( !table_entry )
			{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}	  
            switch (table_info->colnum) {
            case COLUMN_AKMAUTHENTICATIONSUITEENABLED:
            {   
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                {
                    netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
                    break;
                }
                int ret = 2;
                char input[DEFAULT_LEN] = { 0 };
                memset(input,0,DEFAULT_LEN);
                memset(input,0,DEFAULT_LEN);
                int ret_one = 2;
                int ret_two = 2;
                int ret_three = 2;									 
                int value = 0;
                ret = config_wlan_service(table_entry->parameter, connection,table_entry->local_WlanID,"disable");
                if(ret == 1)
                {
                    value = *request->requestvb->val.integer;
                    if ( value == 1 )
                    {
                        strncpy(input,"enable",sizeof(input)-1);
                    }
                    else if( value == 2)
                    	{
                        strncpy(input,"disable",sizeof(input)-1);
                    }
                    ret_one = set_wapi_sub_attr_unicastcipherenabled_cmd(table_entry->parameter, connection,table_entry->SecurityID,input);
                    if(ret_one == 1)
                    {						 
                        ret_two = apply_wlanID(table_entry->parameter, connection,table_entry->SecurityID,table_entry->local_WlanID);
                        if (ret_two == 1)
                        {
                            ret_three = config_wlan_service(table_entry->parameter, connection,table_entry->local_WlanID,"enable");
                            if (ret_three == 1)
                            {
                                table_entry->AKMAuthenticationSuiteEnabled= *request->requestvb->val.integer;
                            }
                            else
                            {
                                 if(SNMPD_CONNECTION_ERROR == ret_three) {
                                    close_slot_dbus_connection(table_entry->parameter.slot_id);
                                }
                            	netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
                            }
                        }	
                        else
                        {
                            if(SNMPD_CONNECTION_ERROR == ret_two) {
                                close_slot_dbus_connection(table_entry->parameter.slot_id);
                            }
                            netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
                        }								 	     
                    }
                    else 
                    {   
                        if(SNMPD_CONNECTION_ERROR == ret_one) {
                            close_slot_dbus_connection(table_entry->parameter.slot_id);
                        }
                        netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
                    }
                }
                else 
                {
                    if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
                    }
                    netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
                }
            }
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AKMConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_AKMAUTHENTICATIONSUITEENABLED:
                /* Need to restore old 'table_entry->AKMAuthenticationSuiteEnabled' value.
                   May need to use 'memcpy' */
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
