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
* dot11WtpWlanFaultTable.c
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
#include "dot11WtpWlanFaultTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sysinfo.h"
#include "ws_security.h"
#include "ws_sta.h"
#include "ws_init_dbus.h"
#include "mibs_public.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define WTPWLANFAULTTABLE  "1.5.1"

    /* Typical data structure for a row entry */
struct dot11WtpWlanFaultTable_entry {
    /* Index values */
	dbus_parameter parameter;
    long wtpCurrID;	
	char *wtpMacAddr;
	long local_wlanCurrID;
    long wlanCurrID;

    /* Column values */
    u_long wirelessCRCFault;
    u_long wirelessPHYReceiveFault;
    u_long wirelessMICFault;
    u_long wirelessKEYFault;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WtpWlanFaultTable_entry *next;
};

void dot11WtpWlanFaultTable_load();
void
dot11WtpWlanFaultTable_removeEntry( struct dot11WtpWlanFaultTable_entry *entry );

/** Initializes the dot11WtpWlanFaultTable module */
void
init_dot11WtpWlanFaultTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WtpWlanFaultTable();
}

/** Initialize the dot11WtpWlanFaultTable table by defining its contents and how it's structured */
void
initialize_table_dot11WtpWlanFaultTable(void)
{
    static oid dot11WtpWlanFaultTable_oid[128] = {0};
    size_t dot11WtpWlanFaultTable_oid_len   = 0;
	mad_dev_oid(dot11WtpWlanFaultTable_oid,WTPWLANFAULTTABLE,&dot11WtpWlanFaultTable_oid_len,enterprise_pvivate_oid);
 
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WtpWlanFaultTable",     dot11WtpWlanFaultTable_handler,
              dot11WtpWlanFaultTable_oid, dot11WtpWlanFaultTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: wtpCurrID */
                           ASN_INTEGER,  /* index: wlanCurrID */
                           0);
    table_info->min_column = WtpWlanFaultTableMIN;
    table_info->max_column = WtpWlanFaultTableMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WtpWlanFaultTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WtpWlanFaultTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WtpWlanFaultTable_load, dot11WtpWlanFaultTable_removeEntry,
							dot11WtpWlanFaultTable_oid, dot11WtpWlanFaultTable_oid_len));

    /* Initialise the contents of the table here */
}


struct dot11WtpWlanFaultTable_entry  *dot11WtpWlanFaultTable_head;

/* create a new row in the (unsorted) table */
struct dot11WtpWlanFaultTable_entry *
dot11WtpWlanFaultTable_createEntry(
				 dbus_parameter parameter,
				 long wtpCurrID,
				 char *wtpMacAddr,                 
				 long local_wlanCurrID,
                 long  wlanCurrID,
				 u_long wirelessCRCFault,
			     u_long wirelessPHYReceiveFault,
			     u_long wirelessMICFault,
			     u_long wirelessKEYFault			
                ) {
    struct dot11WtpWlanFaultTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WtpWlanFaultTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->wtpCurrID = wtpCurrID;
	entry->wtpMacAddr	= strdup(wtpMacAddr);	
	entry->local_wlanCurrID = local_wlanCurrID;
    entry->wlanCurrID = wlanCurrID;
	entry->wirelessCRCFault = wirelessCRCFault;
	entry->wirelessPHYReceiveFault = wirelessPHYReceiveFault;
	entry->wirelessMICFault = wirelessMICFault;
	entry->wirelessKEYFault = wirelessKEYFault;

    entry->next = dot11WtpWlanFaultTable_head;
    dot11WtpWlanFaultTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WtpWlanFaultTable_removeEntry( struct dot11WtpWlanFaultTable_entry *entry ) {
    struct dot11WtpWlanFaultTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WtpWlanFaultTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WtpWlanFaultTable_head = ptr->next;
    else
        prev->next = ptr->next;
	
	FREE_OBJECT(entry->wtpMacAddr);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11WtpWlanFaultTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11WtpWlanFaultTable_load\n");

	struct dot11WtpWlanFaultTable_entry *temp = NULL;

	while( dot11WtpWlanFaultTable_head ) {
		temp=dot11WtpWlanFaultTable_head->next;
		dot11WtpWlanFaultTable_removeEntry(dot11WtpWlanFaultTable_head);
		dot11WtpWlanFaultTable_head=temp;
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
			    void *connection = NULL;
	            if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(messageNode->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    continue;
			    int i = 0;
			    WID_WTP *q = NULL;
				for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q=q->next)
				{
            		int ret_one = 0;
            		DCLI_WTP_API_GROUP_ONE *wtp = NULL;
            		
					memset(temp_mac,0,20);
					if(q->WTPMAC)
					{
						snprintf(temp_mac,sizeof(temp_mac)-1,
								 "%02X:%02X:%02X:%02X:%02X:%02X",
								 q->WTPMAC[0],q->WTPMAC[1],
								 q->WTPMAC[2],q->WTPMAC[3],
								 q->WTPMAC[4],q->WTPMAC[5]);
					}

					ret_one = show_wtp_one(messageNode->parameter, connection,q->WTPID,&wtp);
					if(ret_one == 1)
					{
						if((wtp->WTP[0]))
						{
							int j = 0;
							for(j=0;j<wtp->WTP[0]->apply_wlan_num;j++)
							{	
								unsigned long wlanid = local_to_global_ID(messageNode->parameter, wtp->WTP[0]->apply_wlanid[j], WIRELESS_MAX_NUM);
								
								dot11WtpWlanFaultTable_createEntry(messageNode->parameter,
																   q->WTPID,
																   temp_mac,
																   wtp->WTP[0]->apply_wlanid[j],
																   wlanid,
																   0,
																   0,
																   0,
																   0);
							}
						}
						Free_one_wtp_head(wtp);
					}
					else if(SNMPD_CONNECTION_ERROR == ret_one) {
                        close_slot_dbus_connection(messageNode->parameter.slot_id);
            	    }
					FREE_OBJECT(q->WTPMAC);
				}
			}
		}
        free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
	}	
	snmp_log(LOG_DEBUG, "exit dot11WtpWlanFaultTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WtpWlanFaultTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11WtpWlanFaultTable_head==NULL)
			return NULL;
	*my_data_context = dot11WtpWlanFaultTable_head;
	*my_loop_context = dot11WtpWlanFaultTable_head;
	return dot11WtpWlanFaultTable_get_next_data_point(my_loop_context, my_data_context,
	put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WtpWlanFaultTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WtpWlanFaultTable_entry *entry = (struct dot11WtpWlanFaultTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char *)entry->wtpMacAddr, strlen(entry->wtpMacAddr));
        idx = idx->next_variable;
        snmp_set_var_value( idx, (u_char*)&entry->wlanCurrID, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WtpWlanFaultTable table */
int
dot11WtpWlanFaultTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WtpWlanFaultTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpWlanFaultTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	} 
	
            switch (table_info->colnum) {
            case COLUMN_WIRELESSCRCFAULT:
			{	
			    void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret=0,i;
				DCLI_AC_API_GROUP_THREE *statics = NULL;		
				wlan_stats_info *head = NULL;
				int CRC_pack = 0;
				
				ret=show_ap_statistics_list_bywtp(table_entry->parameter, connection,table_entry->wtpCurrID,&statics);
				if(ret==1)
				{
					if(statics->ap_statics_list)
					{						
						for(head = statics->ap_statics_list->ap_statics_ele; (NULL != head); head = head->next)
						{	
							if((head->type == 0)&&((head->wlanId == table_entry->local_wlanCurrID)))
							{
								CRC_pack += head->ast_rx_crcerr;
							}
						}
					}
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
        	    }
        	    
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
                                          (u_char*)&CRC_pack,
                                          sizeof(CRC_pack));

				if(ret==1)
				{
					Free_ap_statistics_head(statics);
				}
            }
                break;
            case COLUMN_WIRELESSPHYRECEIVEFAULT:
			{
			    void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret=0,i;
				DCLI_AC_API_GROUP_THREE *statics = NULL;			
				wlan_stats_info *head = NULL;
				int PHY_pack = 0;
				
				
				ret=show_ap_statistics_list_bywtp(table_entry->parameter, connection,table_entry->wtpCurrID,&statics);
				if(ret==1)
				{
					if(statics->ap_statics_list)
					{						
						for(head = statics->ap_statics_list->ap_statics_ele; (NULL != head); head = head->next)
						{	
							if((head->type == 0)&&((head->wlanId == table_entry->local_wlanCurrID)))
							{
								PHY_pack += head->ast_rx_phyerr;
							}
						}
					}
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
        	    }
        	    
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
                                          (u_char*)&PHY_pack,
                                          sizeof(PHY_pack));

				if(ret==1)
				{
					Free_ap_statistics_head(statics);
				}
            }
                break;
            case COLUMN_WIRELESSMICFAULT:
			{
			    void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret=0,i;
				DCLI_AC_API_GROUP_THREE *statics = NULL;	
				wlan_stats_info *head = NULL;
				int MIC_pack = 0;
				
				
				ret=show_ap_statistics_list_bywtp(table_entry->parameter, connection,table_entry->wtpCurrID,&statics);
				if(ret==1)
				{
					if(statics->ap_statics_list)
					{						
						for(head = statics->ap_statics_list->ap_statics_ele; (NULL != head); head = head->next)
						{	
							if((head->type == 0)&&((head->wlanId == table_entry->local_wlanCurrID)))
							{
								MIC_pack += head->ast_rx_badmic;
							}
						}
					}
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
        	    }
        	    
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
                                          (u_char*)&MIC_pack,
                                          sizeof(MIC_pack));

				if(ret==1)
				{
					Free_ap_statistics_head(statics);
				}
            }
                break;
            case COLUMN_WIRELESSKEYFAULT:
			{
			    void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret=0,i;
				DCLI_AC_API_GROUP_THREE *statics = NULL;	
				wlan_stats_info *head = NULL;
				int Key_pack = 0;
				
				
				ret=show_ap_statistics_list_bywtp(table_entry->parameter, connection,table_entry->wtpCurrID,&statics);
				if(ret==1)
				{
					if(statics->ap_statics_list)
					{						
						for(head = statics->ap_statics_list->ap_statics_ele; (NULL != head); head = head->next)
						{	
							if((head->type == 0)&&((head->wlanId == table_entry->local_wlanCurrID)))
							{
								Key_pack += head->ast_rx_badcrypt;
							}
						}
					}
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
        	    }
        	    
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
                                         (u_char*)&Key_pack,
                                          sizeof(Key_pack));

				if(ret==1)
				{
					Free_ap_statistics_head(statics);
				}
            }
                break;
			default:
			{
                netsnmp_set_request_error( reqinfo, request,
                                           SNMP_ERR_NOTWRITABLE );
			}
                break;
            }
        }
        break;

    }
    return SNMP_ERR_NOERROR;
}
