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
* dot11WtpExtensionTable.c
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
#include "dot11WtpExtensionTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_init_dbus.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"


#define EXTENOID	"1.12.3"

    /* Typical data structure for a row entry */
struct dot11WtpExtensionTable_entry {
    /* Index values */
	dbus_parameter parameter;
    long wtpCurrID;
     char *wtpMacAddr;

    /* Column values */
    long wtpExtensionUsed;
    //long old_wtpExtensionUsed;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WtpExtensionTable_entry *next;
};

void dot11WtpExtensionTable_load();
void
dot11WtpExtensionTable_removeEntry( struct dot11WtpExtensionTable_entry *entry );


/** Initializes the dot11WtpExtensionTable module */
void
init_dot11WtpExtensionTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WtpExtensionTable();
}

/** Initialize the dot11WtpExtensionTable table by defining its contents and how it's structured */
void
initialize_table_dot11WtpExtensionTable(void)
{
    static oid dot11WtpExtensionTable_oid[128] = {0};
    size_t dot11WtpExtensionTable_oid_len   = 0;
	mad_dev_oid(dot11WtpExtensionTable_oid,EXTENOID,&dot11WtpExtensionTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WtpExtensionTable",     dot11WtpExtensionTable_handler,
              dot11WtpExtensionTable_oid, dot11WtpExtensionTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: wtpCurrID */
                           0);
    table_info->min_column = COLUMN_WTPEXTENSIONUSED;
    table_info->max_column = COLUMN_WTPEXTENSIONUSED;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WtpExtensionTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WtpExtensionTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
 	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WtpExtensionTable_load, dot11WtpExtensionTable_removeEntry,
 							dot11WtpExtensionTable_oid, dot11WtpExtensionTable_oid_len));

    /* Initialise the contents of the table here */
}



struct dot11WtpExtensionTable_entry  *dot11WtpExtensionTable_head;

/* create a new row in the (unsorted) table */
struct dot11WtpExtensionTable_entry *
dot11WtpExtensionTable_createEntry(
				 dbus_parameter parameter,
                 long  wtpCurrID,
                 char *wtpMacAddr,
                 long wtpExtensionUsed
                ) {
    struct dot11WtpExtensionTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WtpExtensionTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
    entry->wtpCurrID = wtpCurrID;
	entry->wtpMacAddr	= strdup(wtpMacAddr);    	
    entry->wtpExtensionUsed = wtpExtensionUsed;
    entry->next = dot11WtpExtensionTable_head;
    dot11WtpExtensionTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WtpExtensionTable_removeEntry( struct dot11WtpExtensionTable_entry *entry ) {
    struct dot11WtpExtensionTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WtpExtensionTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WtpExtensionTable_head = ptr->next;
    else
        prev->next = ptr->next;
    FREE_OBJECT(entry->wtpMacAddr);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11WtpExtensionTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11WtpExtensionTable_load\n");

	struct dot11WtpExtensionTable_entry *temp = NULL;	
    while( dot11WtpExtensionTable_head ){
        temp=dot11WtpExtensionTable_head->next;
        dot11WtpExtensionTable_removeEntry(dot11WtpExtensionTable_head);
        dot11WtpExtensionTable_head=temp;
    }

	char temp_mac[20] = { 0 };
	
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wtp_list_by_mac_cmd_func\n");
    messageHead = list_connection_call_dbus_method(show_wtp_list_by_mac_cmd_func, SHOW_ALL_WTP_TABLE_METHOD);
	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wtp_list_by_mac_cmd_func,messageHead=%p\n", messageHead);

	if(messageHead)
	{
		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
		{
		    DCLI_WTP_API_GROUP_ONE *head = messageNode->message;
    		if((head)&&(head->WTP_INFO))
    		{
    		    int i = 0;
    		    WID_WTP *q = NULL;
    			for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q = q->next)
    			{
    				memset(temp_mac,0,20);
					if(q->WTPMAC)
					{
						snprintf(temp_mac,sizeof(temp_mac)-1,
								 "%02X:%02X:%02X:%02X:%02X:%02X",
								 q->WTPMAC[0],q->WTPMAC[1],
								 q->WTPMAC[2],q->WTPMAC[3],
								 q->WTPMAC[4],q->WTPMAC[5]);
					}

    				dot11WtpExtensionTable_createEntry(messageNode->parameter,
    												   q->WTPID,
    												   temp_mac,
    												   0);
					FREE_OBJECT(q->WTPMAC);
    			}
    		}
        }	
        free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
	}

	snmp_log(LOG_DEBUG, "exit dot11WtpExtensionTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WtpExtensionTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11WtpExtensionTable_head==NULL)
		return NULL;
	*my_data_context = dot11WtpExtensionTable_head;
	*my_loop_context = dot11WtpExtensionTable_head;
	return dot11WtpExtensionTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WtpExtensionTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WtpExtensionTable_entry *entry = (struct dot11WtpExtensionTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx,(u_char *)entry->wtpMacAddr, strlen(entry->wtpMacAddr));
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WtpExtensionTable table */
int
dot11WtpExtensionTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WtpExtensionTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpExtensionTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

		 if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }    
	
            switch (table_info->colnum) {
            case COLUMN_WTPEXTENSIONUSED:
			{
			    void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret  = 0;
				int value = 3;
				DCLI_WTP_API_GROUP_TWO *WTPINFO = NULL; 
				
				ret  = show_wtp_extension_information_v3(table_entry->parameter, connection,table_entry->wtpCurrID,&WTPINFO); 
				if(ret == -2)
				{
					value = 2;
				} 
				else if((ret == 1)&&(WTPINFO->WTP[0]))
				{
                	value = (WTPINFO->WTP[0]->wifi_extension_reportswitch==1)?1:2;
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
        	    }
        	    
				table_entry->wtpExtensionUsed = value;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->wtpExtensionUsed,
                                          sizeof(long));
				if(ret==1)
				{
					free_how_wtp_extension_information_v3(WTPINFO);
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
            table_entry = (struct dot11WtpExtensionTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WTPEXTENSIONUSED:
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
            table_entry = (struct dot11WtpExtensionTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
			
		if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}
    
            switch (table_info->colnum) {
		case COLUMN_WTPEXTENSIONUSED:
		{
            void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
            {
                netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
                break;
            }
            
			int flag = 1;
			char state[10] = { 0 };
			memset(state,0,10);
			int ret  = 0;
			/* Need to save old 'table_entry->wtpExtensionUsed' value.
			May need to use 'memcpy' */
			if(*request->requestvb->val.integer==1)
			{				
				strncpy(state,"enable",sizeof(state)-1);
			}
			else if(*request->requestvb->val.integer==2)
			{
				strncpy(state,"disable",sizeof(state)-1);
			}
			else
			{
				flag = 0;
				netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
			}

			if(flag == 1)
			{
				ret = set_ap_extension_infomation_enable(table_entry->parameter, connection,table_entry->wtpCurrID,state);
				if(ret == 1)
				{
					table_entry->wtpExtensionUsed = *request->requestvb->val.integer;
				}
				else
				{
				    if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(table_entry->parameter.slot_id);
            	    }
					netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
				}
			}
		}
		break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpExtensionTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WTPEXTENSIONUSED:
                /* Need to restore old 'table_entry->wtpExtensionUsed' value.
                   May need to use 'memcpy' */
                //table_entry->wtpExtensionUsed = table_entry->old_wtpExtensionUsed;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
