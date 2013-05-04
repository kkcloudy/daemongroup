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
* dot11ConfigWepTable.c
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
#include "dot11ConfigWepTable.h"
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
#include <dbus/dbus.h>
#include "ws_security.h"
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"
//#include "autelanWtpGroup.h"

#define CONFIGWEPTABLE "2.14.4"

#define __DEBUG	1


struct dot11ConfigWepTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalWepSSID;
    long WepSSID;
    long globalWebSecID;
    long WepSecID;
    /* Column values */
    char *WepCipherKeyValue;
    long WepCipherKeyCharType;
    long WepSSIDIndex;
    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11ConfigWepTable_entry *next;
};

void dot11ConfigWepTable_load(void);
void dot11ConfigWepTable_removeEntry( struct dot11ConfigWepTable_entry *entry );

/** Initializes the dot11ConfigWepTable module */
void
init_dot11ConfigWepTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11ConfigWepTable();
}

/** Initialize the dot11ConfigWepTable table by defining its contents and how it's structured */
void
initialize_table_dot11ConfigWepTable(void)
{
    static oid dot11ConfigWepTable_oid[64] = {0};
    size_t dot11ConfigWepTable_oid_len   = 0;	
	mad_dev_oid(dot11ConfigWepTable_oid,CONFIGWEPTABLE,&dot11ConfigWepTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11ConfigWepTable",     dot11ConfigWepTable_handler,
              dot11ConfigWepTable_oid, dot11ConfigWepTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalWepSSID */
                           0);
    table_info->min_column = CONFIGWEPMIN;
    table_info->max_column = CONFIGWEPMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11ConfigWepTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11ConfigWepTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11ConfigWepTable_load, dot11ConfigWepTable_removeEntry,dot11ConfigWepTable_oid, dot11ConfigWepTable_oid_len));

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11ConfigWepTable_entry  *dot11ConfigWepTable_head;

/* create a new row in the (unsorted) table */
struct dot11ConfigWepTable_entry *
dot11ConfigWepTable_createEntry(				
				dbus_parameter parameter,
				long globalWepSSID,
                long  WepSSID,
                long globalWebSecID,
                long WepSecID,
                char *WepCipherKeyValue,
                long WepCipherKeyCharType,
                long WepSSIDIndex
                ) {
    struct dot11ConfigWepTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11ConfigWepTable_entry);
    if (!entry)
        return NULL;
    memset(entry,0,sizeof(struct dot11ConfigWepTable_entry));
	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->globalWepSSID = globalWepSSID;
    entry->WepSSID = WepSSID;
    entry->globalWebSecID = globalWebSecID;
	entry->WepSecID = WepSecID;
	entry->WepCipherKeyValue =strdup(WepCipherKeyValue);
	entry->WepCipherKeyCharType = WepCipherKeyCharType;
	entry->WepSSIDIndex  = WepSSIDIndex;

    entry->next = dot11ConfigWepTable_head;
    dot11ConfigWepTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11ConfigWepTable_removeEntry( struct dot11ConfigWepTable_entry *entry ) {
    struct dot11ConfigWepTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11ConfigWepTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11ConfigWepTable_head = ptr->next;
    else
        prev->next = ptr->next;
    FREE_OBJECT(entry->WepCipherKeyValue);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11ConfigWepTable_load()
{
				
	snmp_log(LOG_DEBUG, "enter dot11ConfigWepTable_load\n");
	
	struct dot11ConfigWepTable_entry *temp = NULL; 
	while( dot11ConfigWepTable_head ){
		temp=dot11ConfigWepTable_head->next;
		dot11ConfigWepTable_removeEntry(dot11ConfigWepTable_head);
		dot11ConfigWepTable_head=temp;
	}
	char key[20] = { 0 };
	
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wlan_list\n");
    messageHead = list_connection_call_dbus_method(show_wlan_list, SHOW_ALL_WTP_TABLE_METHOD);
	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wlan_list,messageHead=%p\n", messageHead);

	if(messageHead)
	{
		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
		{
		    DCLI_WLAN_API_GROUP *WLANINFO = messageNode->message;
		    if(WLANINFO)
		    {
		        void *connection = NULL;
		        if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(messageNode->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
		            continue;

		        int i = 0;
		        for(i=0;i<WLANINFO->wlan_num;i++)
        		{   
        			memset(key,0,20);
        			int ret_wlan = 0;
        			DCLI_WLAN_API_GROUP *wlan = NULL;

        			ret_wlan = show_wlan_one(messageNode->parameter, connection,WLANINFO->WLAN[i]->WlanID,&wlan);
        			if(ret_wlan==1)
        			{
                        unsigned long globalWepSSID = local_to_global_ID(messageNode->parameter, WLANINFO->WLAN[i]->WlanID, WIRELESS_MAX_NUM);
                        unsigned long globalWebSecID = local_to_global_ID(messageNode->parameter, wlan->WLAN[0]->SecurityID, WIRELESS_MAX_NUM);
						dot11ConfigWepTable_createEntry(messageNode->parameter,
        				                                globalWepSSID,
        												WLANINFO->WLAN[i]->WlanID,
        												globalWebSecID,
        												wlan->WLAN[0]->SecurityID,
        												"",
        												0,
        												0);
						Free_one_wlan_head(wlan);
        			}
        	    }	
        	}
        }
        free_dbus_message_list(&messageHead, Free_wlan_head);
    }   		

	snmp_log(LOG_DEBUG, "exit dot11ConfigWepTable_load\n");
}


/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11ConfigWepTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{   
   if(dot11ConfigWepTable_head == NULL)
	   {
		   return NULL;
	   }
    *my_loop_context = dot11ConfigWepTable_head;
    return dot11ConfigWepTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11ConfigWepTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11ConfigWepTable_entry *entry = (struct dot11ConfigWepTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->globalWepSSID, sizeof(entry->globalWepSSID) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }	
	return put_index_data; 
}


/** handles requests for the dot11ConfigWepTable table */
int
dot11ConfigWepTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11ConfigWepTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ConfigWepTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);			
			if( !table_entry )
			{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}	   

            void *connection = NULL;
		    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3)) {
                break;
		    }
			
            switch (table_info->colnum) {
            case COLUMN_WEPSSID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->globalWepSSID,
                                          sizeof(long));
                break;
            case COLUMN_WEPCIPHERKEYVALUE:
			{   
				int ret=0;
				char key[128] = { 0 };
				memset(key,0,128);
				struct dcli_security *security = NULL; 

				ret = show_security_one(table_entry->parameter, connection,table_entry->WepSecID,&security);	
				if(ret == 1)
				{
					if((security->EncryptionType== 1)&&(security->SecurityType== 0))
					{
						 strncpy(key,security->SecurityKey,sizeof(key)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)(key),
                                          strlen(key));
				
				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WEPCIPHERKEYCHARTYPE:
			{   
				int ret=0;
				int type = 0;
				struct dcli_security *security; 

				ret = show_security_one(table_entry->parameter, connection,table_entry->WepSecID,&security);
				if(ret == 1)
				{
					if(security->keyInputType == 2)
					{
						type =2;
					}
					else 
					{
						type =1;
					}
					if((security->EncryptionType == 1)&&(security->SecurityType == 0))
					{
						table_entry->WepCipherKeyCharType = type;
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&(table_entry->WepCipherKeyCharType),
                                          sizeof(long));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WEPSSIDINDEX:
			{   
				int ret=0;
				struct dcli_security *security = NULL; 

				ret = show_security_one(table_entry->parameter, connection,table_entry->WepSecID,&security);
				if(ret == 1)
				{
					table_entry->WepSSIDIndex = security->security_index;
				}
								
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&(table_entry->WepSSIDIndex),
                                          sizeof(long));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ConfigWepTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WEPCIPHERKEYVALUE:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WEPCIPHERKEYCHARTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WEPSSIDINDEX:
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
            table_entry = (struct dot11ConfigWepTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
            
		    if( !table_entry )
			{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}	
            
            void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                break;
			
            switch (table_info->colnum) {
            case COLUMN_WEPCIPHERKEYVALUE:
           {	
			 	int value = -1;
			 	int ret_wlan = 0;
			 	int ret = 2;
				int ret_one = 2;
				int ret_two = 2;
			    int ret_three = 3;
				char input_string[256] = {0};
				memset(input_string,0,256);
				char type[256] = {0};
				memset(type,0,256);
				DCLI_WLAN_API_GROUP *wlan;
				
				ret_wlan = show_wlan_one(table_entry->parameter, connection,table_entry->WepSSID,&wlan);
				if(ret_wlan == 1)
				{
					if(0 == wlan->WLAN[0]->asic_hex)
					{
						strncpy(type,"ASCII",sizeof(type)-1);
					}
					else if(1 == wlan->WLAN[0]->asic_hex)
					{
						strncpy(type,"HEX",sizeof(type)-1);
					}
				}
				strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
                ret_three = security_type(table_entry->parameter, connection,table_entry->WepSecID,"open");
				ret_one = encryption_type(table_entry->parameter, connection,table_entry->WepSecID,"WEP");
				ret = security_key(table_entry->parameter, connection,table_entry->WepSecID,input_string,type);
				if((ret_wlan == 1)&&(ret == 1)&&(ret_three == 1) &&(ret_one == 1))
				{
                   	 ret_two = apply_wlanID(table_entry->parameter, connection,table_entry->WepSecID,table_entry->WepSSID);
					 if (ret_two == 1)
				 	 {					 	
						FREE_OBJECT(table_entry->WepCipherKeyValue);
						table_entry->WepCipherKeyValue= strdup(input_string);						    
				 	 }	
				  else
				 	 {
	 	             	netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
 	                 }
				}			      
				else
				{   
					netsnmp_set_request_error( reqinfo, request,SNMP_ERR_WRONGTYPE);
				}
				if(ret_wlan==1)
				{
					Free_one_wlan_head(wlan);
				} 
            }
                break;
            case COLUMN_WEPCIPHERKEYCHARTYPE:
            {	
			 	int ret = 2;
				int ret_three = 2;
				int ret_one = 2;
				int ret_two = 2;
				char type[256] = {0};
				memset(type,0,256);
				char value[256] = {0};
				memset(value,0,256);
				if(1 == *request->requestvb->val.integer)
				{
					strncpy(type,"ASCII",sizeof(type)-1);
					strncpy(value,"11111",sizeof(value)-1);
				}
				else if(2 == *request->requestvb->val.integer)
				{
					strncpy(type,"HEX",sizeof(type)-1);
					strncpy(value,"1111111111",sizeof(value)-1);
				}
				ret_three = security_type(table_entry->parameter, connection,table_entry->WepSecID,"open");
				ret_one = encryption_type(table_entry->parameter, connection,table_entry->WepSecID,"WEP");
				ret = security_key(table_entry->parameter, connection,table_entry->WepSecID,value,type);
				if((ret_one == 1)&&(ret == 1)&&(ret_three == 1))
				{
                   	ret_two = apply_wlanID(table_entry->parameter, connection,table_entry->WepSecID,table_entry->WepSSID);
					 if (ret_two == 1)
				 	 {
					 	
					    table_entry->WepCipherKeyCharType= *request->requestvb->val.integer;
						    
				 	 }	
				  else
				 	 {
	 	             	netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
 	                 }
				}			      
				else
				{
					netsnmp_set_request_error( reqinfo, request,SNMP_ERR_WRONGTYPE);
				}
            }
                break;
			case COLUMN_WEPSSIDINDEX:
			{	
				if(*request->requestvb->val.integer >  0 && *request->requestvb->val.integer < 5) 
				{
					int ret = 0;
					char input_index[128] = { 0 };
					snprintf(input_index, sizeof(input_index) - 1, "%d", *request->requestvb->val.integer);
					
					config_wlan_service(table_entry->parameter, connection, table_entry->WepSSID, "disable");
					usleep(100);

					ret = set_security_wep_index_cmd(table_entry->parameter, connection, table_entry->WepSecID, input_index);
					if(1 == ret) {	  
						table_entry->WepSSIDIndex= *request->requestvb->val.integer;
					}								
					else {	 
						netsnmp_set_request_error( reqinfo, request, SNMP_ERR_WRONGTYPE);
					}
					
					apply_wlanID(table_entry->parameter, connection, table_entry->WepSecID, table_entry->WepSSID);
					config_wlan_service(table_entry->parameter, connection, table_entry->WepSSID, "enable");
				}
				
			}
			break;				
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ConfigWepTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WEPCIPHERKEYVALUE:
                /* Need to restore old 'table_entry->WepCipherKeyValue' value.
                   May need to use 'memcpy' */
                break;
            case COLUMN_WEPCIPHERKEYCHARTYPE:
                /* Need to restore old 'table_entry->WepCipherKeyCharType' value.
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
