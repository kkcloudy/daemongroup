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
* dot11VlanConfigTable.c
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
#include "dot11VlanConfigTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sysinfo.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

/** Initializes the dot11VlanConfigTable module */

#define DOT11VLANCONFIGTABLE "2.7"

    /* Typical data structure for a row entry */
struct dot11VlanConfigTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long wlanGlobalID;
    long wlanCurrID;

    /* Column values */
    long vlanID;
    //long old_vlanID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11VlanConfigTable_entry *next;
};


void dot11VlanConfigTable_load();
void
dot11VlanConfigTable_removeEntry( struct dot11VlanConfigTable_entry *entry );

void
init_dot11VlanConfigTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11VlanConfigTable();
}

/** Initialize the dot11VlanConfigTable table by defining its contents and how it's structured */
void
initialize_table_dot11VlanConfigTable(void)
{
    static oid dot11VlanConfigTable_oid[128] = {0};

	
    size_t dot11VlanConfigTable_oid_len   = 0;
	
	mad_dev_oid(dot11VlanConfigTable_oid,DOT11VLANCONFIGTABLE,&dot11VlanConfigTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11VlanConfigTable",     dot11VlanConfigTable_handler,
              dot11VlanConfigTable_oid, dot11VlanConfigTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: wlanGlobalID */
                           0);
    table_info->min_column = WLANVLAN_MIN;
    table_info->max_column = WLANVLAN_MAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11VlanConfigTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11VlanConfigTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11VlanConfigTable_load, dot11VlanConfigTable_removeEntry,
							dot11VlanConfigTable_oid, dot11VlanConfigTable_oid_len));

    /* Initialise the contents of the table here */
}



struct dot11VlanConfigTable_entry  *dot11VlanConfigTable_head;

/* create a new row in the (unsorted) table */
struct dot11VlanConfigTable_entry *
dot11VlanConfigTable_createEntry(
				 dbus_parameter parameter,
				 long wlanGlobalID,
                 long  wlanCurrID,
                  long vlanID
                ) {
    struct dot11VlanConfigTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11VlanConfigTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->wlanGlobalID = wlanGlobalID;
    entry->wlanCurrID = wlanCurrID;
	entry->vlanID = vlanID;
    entry->next = dot11VlanConfigTable_head;
    dot11VlanConfigTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11VlanConfigTable_removeEntry( struct dot11VlanConfigTable_entry *entry ) {
    struct dot11VlanConfigTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11VlanConfigTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11VlanConfigTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11VlanConfigTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11VlanConfigTable_load\n");

	struct dot11VlanConfigTable_entry *temp = NULL;
	while( dot11VlanConfigTable_head )
	{
		temp=dot11VlanConfigTable_head->next;
		dot11VlanConfigTable_removeEntry(dot11VlanConfigTable_head);
		dot11VlanConfigTable_head=temp;
	}
	
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wlan_list\n");
    messageHead = list_connection_call_dbus_method(show_wlan_list, SHOW_ALL_WTP_TABLE_METHOD);
	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wlan_list,messageHead=%p\n", messageHead);

	if(messageHead)
	{
		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
		{
		    DCLI_WLAN_API_GROUP *WLANINFO = messageNode->message;
		    void *connection = NULL;
		    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(messageNode->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
		        continue;
		        
		    if(WLANINFO)
		    {
		        int i = 0;
    			for(i=0;i<WLANINFO->wlan_num;i++)
    			{
    				int vlan_id = 0;
    				int ret = 0;
    				DCLI_WLAN_API_GROUP *head = NULL; 
    				ret = show_wlan_vlan_info(messageNode->parameter, connection,WLANINFO->WLAN[i]->WlanID,&head);
    				if(ret == 1)
    				{
    					vlan_id = head->WLAN[0]->vlanid;
    				}
    				if(ret == 1)
    				{
    					Free_wlan_vlan_info(head);
    			   	}

                    long wlanGlobalID = local_to_global_ID(messageNode->parameter, WLANINFO->WLAN[i]->WlanID, WIRELESS_MAX_NUM);
    				
    				dot11VlanConfigTable_createEntry(messageNode->parameter,
    				                                 wlanGlobalID,
    												 WLANINFO->WLAN[i]->WlanID,
    												 vlan_id);
    			}
    		}	
		}
		free_dbus_message_list(&messageHead, Free_wlan_head);
	}
	snmp_log(LOG_DEBUG, "exit dot11VlanConfigTable_load\n");
}


/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11VlanConfigTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11VlanConfigTable_head==NULL)
			return NULL;
	*my_data_context = dot11VlanConfigTable_head;
	*my_loop_context = dot11VlanConfigTable_head;
	return dot11VlanConfigTable_get_next_data_point(my_loop_context, my_data_context,	put_index_data,  mydata );
}

netsnmp_variable_list *
dot11VlanConfigTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11VlanConfigTable_entry *entry = (struct dot11VlanConfigTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->wlanGlobalID, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11VlanConfigTable table */
int
dot11VlanConfigTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11VlanConfigTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11VlanConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

		 if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}     
	
	
            switch (table_info->colnum) {
            case COLUMN_WLANCURRID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->wlanGlobalID,
                                          sizeof(long));
                break;
            case COLUMN_VLANID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->vlanID,
                                          sizeof(long));
                break;
			 default:
                netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
			break; 
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11VlanConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_VLANID:
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
            table_entry = (struct dot11VlanConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}
    
            switch (table_info->colnum) {
            case COLUMN_VLANID:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                        return MFD_ERROR;
                        
					int ret1=0,ret2=0;
					char vlanid[10] = { 0 };
					memset(vlanid, 0, sizeof(vlanid));
					ret1=config_wlan_service(table_entry->parameter, connection,table_entry->wlanCurrID,"disable");
					if(ret1==1)
					{
						snprintf(vlanid, sizeof(vlanid) - 1, "%d", *request->requestvb->val.integer);
						ret2=set_wlan_vlan_id(table_entry->parameter, connection,table_entry->wlanCurrID,vlanid);
						if(ret2==1)
						{
							table_entry->vlanID = *request->requestvb->val.integer;
						}
					}

					if((ret1!=1)||(ret2!=1))
					{	
					    if(SNMPD_CONNECTION_ERROR == ret1 || SNMPD_CONNECTION_ERROR == ret2) {
                            close_slot_dbus_connection(table_entry->parameter.slot_id);
                	    }
						netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
					}
					
            	}
                break;
            }
        }
    	break;
    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11VlanConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_VLANID:
                /* Need to restore old 'table_entry->vlanID' value.
                   May need to use 'memcpy' */
            //    table_entry->vlanID = table_entry->old_vlanID;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
