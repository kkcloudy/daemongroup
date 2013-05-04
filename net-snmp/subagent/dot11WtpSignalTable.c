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
* dot11WtpSignalTable.c
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
#include "dot11WtpSignalTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sta.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define WTPSIGNALTABLE "1.2.1"

    /* Typical data structure for a row entry */
struct dot11WtpSignalTable_entry {
    /* Index values */
	dbus_parameter parameter;
    long  wtpCurrID;
    char *wtpMacAddr;

    /* Column values */
    unsigned long wtpAllSignalPack;
    unsigned long wtpRxSignalPack;
    unsigned long wtpTxSignalPack;
	char *wtpDataType;
    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WtpSignalTable_entry *next;
};

void dot11WtpSignalTable_load();
void
dot11WtpSignalTable_removeEntry( struct dot11WtpSignalTable_entry *entry );

/** Initializes the dot11WtpSignalTable module */
void
init_dot11WtpSignalTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WtpSignalTable();
}

/** Initialize the dot11WtpSignalTable table by defining its contents and how it's structured */
void
initialize_table_dot11WtpSignalTable(void)
{
    static oid dot11WtpSignalTable_oid[128] = {0};
	size_t dot11WtpSignalTable_oid_len   = 0;

	mad_dev_oid(dot11WtpSignalTable_oid,WTPSIGNALTABLE,&dot11WtpSignalTable_oid_len,enterprise_pvivate_oid);

    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WtpSignalTable",     dot11WtpSignalTable_handler,
              dot11WtpSignalTable_oid, dot11WtpSignalTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: wtpCurrID */
                           0);
    table_info->min_column = WTPSIGNALMIN;
    table_info->max_column = WTPSIGNALMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WtpSignalTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WtpSignalTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WtpSignalTable_load, dot11WtpSignalTable_removeEntry,
							dot11WtpSignalTable_oid, dot11WtpSignalTable_oid_len));

    /* Initialise the contents of the table here */
}


struct dot11WtpSignalTable_entry  *dot11WtpSignalTable_head;

/* create a new row in the (unsorted) table */
struct dot11WtpSignalTable_entry *
dot11WtpSignalTable_createEntry(
				 dbus_parameter parameter,
                 long wtpCurrID,
                 char *wtpMacAddr,
                 unsigned long wtpAllSignalPack,
                 unsigned long wtpRxSignalPack,
                 unsigned long wtpTxSignalPack,
                 char *wtpDataType
                ) {
    struct dot11WtpSignalTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WtpSignalTable_entry);
    if (!entry)
        return NULL;
	memcpy(&(entry->parameter), &parameter, sizeof(dbus_parameter));
	entry->wtpCurrID	= wtpCurrID;
    entry->wtpMacAddr 	= strdup(wtpMacAddr);
	entry->wtpAllSignalPack = wtpAllSignalPack;
	entry->wtpRxSignalPack = wtpRxSignalPack;
	entry->wtpTxSignalPack = wtpTxSignalPack;
	entry->wtpDataType = strdup(wtpDataType);
    entry->next = dot11WtpSignalTable_head;
    dot11WtpSignalTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WtpSignalTable_removeEntry( struct dot11WtpSignalTable_entry *entry ) {
    struct dot11WtpSignalTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WtpSignalTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WtpSignalTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->wtpDataType);
	FREE_OBJECT(entry->wtpMacAddr);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11WtpSignalTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11WtpSignalTable_load\n");

	struct dot11WtpSignalTable_entry *temp = NULL;	
	while( dot11WtpSignalTable_head ) {
		temp=dot11WtpSignalTable_head->next;
		dot11WtpSignalTable_removeEntry(dot11WtpSignalTable_head);
		dot11WtpSignalTable_head=temp;
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
            if((head)&&(head->WTP_INFO)&&(head->WTP_INFO->WTP_LIST))
        	{
        	    void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(messageNode->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    continue;
                    
        	    int i = 0;
        	    WID_WTP *q = NULL;
        		for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q=q->next)
        		{
        		    int ret = 0;
        		    DCLI_WTP_API_GROUP_TWO *WTPINFO = NULL;  
                    
    				unsigned long wtpAllSignalPack = 0;
    				unsigned long wtpRxSignalPack = 0;
    				unsigned long wtpTxSignalPack = 0;
    				snmp_log(LOG_DEBUG, "enter show_wtp_extension_information_v3\n");
    				ret = show_wtp_extension_information_v3(messageNode->parameter, connection, q->WTPID, &WTPINFO);
    				snmp_log(LOG_DEBUG, "exit show_wtp_extension_information_v3, ret=%d\n", ret);
    				if(1 == ret && WTPINFO) {
						if(WTPINFO->WTP[0])
						{
							wtpAllSignalPack = WTPINFO->WTP[0]->wifi_extension_info.rx_mgmt+WTPINFO->WTP[0]->wifi_extension_info.tx_mgmt;
							wtpRxSignalPack = WTPINFO->WTP[0]->wifi_extension_info.rx_mgmt;
							wtpTxSignalPack = WTPINFO->WTP[0]->wifi_extension_info.tx_mgmt;	
						}
    					free_how_wtp_extension_information_v3(WTPINFO);
    				}
    				else if(SNMPD_CONNECTION_ERROR == ret) {
    				    close_slot_dbus_connection(messageNode->parameter.slot_id);
    				    break;
    				}
    				
    				memset(temp_mac,0,20);
					if(q->WTPMAC)
					{
						snprintf(temp_mac,sizeof(temp_mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
					}
    				dot11WtpSignalTable_createEntry(messageNode->parameter,
    												q->WTPID,
    												temp_mac,
    												wtpAllSignalPack,
    												wtpRxSignalPack,
    												wtpTxSignalPack,
    												"Management Frame");
    				FREE_OBJECT(q->WTPMAC);	
				}
			}
		}
		free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
	}

	snmp_log(LOG_DEBUG, "exit dot11WtpSignalTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WtpSignalTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{

	if(dot11WtpSignalTable_head==NULL)
		{
			return NULL;
		}
	*my_loop_context = dot11WtpSignalTable_head;
	*my_data_context = dot11WtpSignalTable_head;
    return dot11WtpSignalTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WtpSignalTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WtpSignalTable_entry *entry = (struct dot11WtpSignalTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx,(u_char *)entry->wtpMacAddr, strlen(entry->wtpMacAddr) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	 return put_index_data;
}


/** handles requests for the dot11WtpSignalTable table */
int
dot11WtpSignalTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WtpSignalTable_entry          *table_entry;
	void *data_context = NULL;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpSignalTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			 if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }    
	
            switch (table_info->colnum) {
            case COLUMN_WTPALLSIGNALPACK:
			{   
				
				snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
										  (u_char*)&table_entry->wtpAllSignalPack,
										  sizeof(table_entry->wtpAllSignalPack));
			}
                break;
            case COLUMN_WTPRXSIGNALPACK:
			{   
			    snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
										(u_char*)&table_entry->wtpRxSignalPack,
										sizeof(table_entry->wtpRxSignalPack));
			}
                break;
            case COLUMN_WTPTXSIGNALPACK:
			{   
			    snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
										(u_char*)&table_entry->wtpTxSignalPack,
										sizeof(table_entry->wtpTxSignalPack));
			}
                break;
			case COLUMN_WTPDATATYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->wtpDataType,
                                          strlen(table_entry->wtpDataType));
                break;
            }
        }
        break;

    }
    return SNMP_ERR_NOERROR;
}
