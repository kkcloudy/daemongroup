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
* dot11WlanProfileTable.c
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
#include "dot11WlanProfileTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define 	HIDDENWLAN	"6.1.3"
    /* Typical data structure for a row entry */
struct dot11WlanProfileTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalWlanID;
    long WlanID;

    /* Column values */
    char* WlanEssid;
    long WlanState;
    long WlanBindSecurityID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WlanProfileTable_entry *next;
};
void dot11WlanProfileTable_load(void);
void dot11WlanProfileTable_removeEntry( struct dot11WlanProfileTable_entry *entry );

/** Initializes the dot11WlanProfileTable module */
void
init_dot11WlanProfileTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WlanProfileTable();
}

/** Initialize the dot11WlanProfileTable table by defining its contents and how it's structured */
void
initialize_table_dot11WlanProfileTable(void)
{
    static oid dot11WlanProfileTable_oid[128] = {0};
    size_t dot11WlanProfileTable_oid_len   =   0;	
	mad_dev_oid(dot11WlanProfileTable_oid,HIDDENWLAN,&dot11WlanProfileTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WlanProfileTable",     dot11WlanProfileTable_handler,
              dot11WlanProfileTable_oid, dot11WlanProfileTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalWlanID */
                           0);
    table_info->min_column = COLUMN_HIDDENWLANMIN;
    table_info->max_column = COLUMN_HIDDENWLANMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WlanProfileTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WlanProfileTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WlanProfileTable_load, dot11WlanProfileTable_removeEntry,dot11WlanProfileTable_oid, dot11WlanProfileTable_oid_len));
    /* Initialise the contents of the table here */
}



struct dot11WlanProfileTable_entry  *dot11WlanProfileTable_head;

/* create a new row in the (unsorted) table */
struct dot11WlanProfileTable_entry *
dot11WlanProfileTable_createEntry(
				 dbus_parameter parameter,
				 long globalWlanID,
                 long  WlanID,
                 char* WlanEssid,
			     long WlanState,
			     long WlanBindSecurityID
                ) {
    struct dot11WlanProfileTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WlanProfileTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->globalWlanID = globalWlanID;
    entry->WlanID = WlanID;
	entry->WlanEssid = strdup(WlanEssid);
	entry->WlanState = WlanState;
	entry->WlanBindSecurityID = WlanBindSecurityID;
    entry->next = dot11WlanProfileTable_head;
    dot11WlanProfileTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WlanProfileTable_removeEntry( struct dot11WlanProfileTable_entry *entry ) {
    struct dot11WlanProfileTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WlanProfileTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WlanProfileTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->WlanEssid);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
void dot11WlanProfileTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11WlanProfileTable_load\n");

	struct dot11WlanProfileTable_entry *temp = NULL; 
	while( dot11WlanProfileTable_head ){
		temp=dot11WlanProfileTable_head->next;
		dot11WlanProfileTable_removeEntry(dot11WlanProfileTable_head);
		dot11WlanProfileTable_head=temp;
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
			char essid[255] = { 0 };
		    if(WLANINFO)
		    {
		        int i = 0;
		        for(i=0;i<WLANINFO->wlan_num;i++)
        		{        		    
					if(WLANINFO->WLAN[i])
					{
						unsigned long globalWlanID = local_to_global_ID(messageNode->parameter, 
																		WLANINFO->WLAN[i]->WlanID, 
																		WIRELESS_MAX_NUM);
						
						memset(essid,0,sizeof(essid));
						if(WLANINFO->WLAN[i]->ESSID)
						{
							strncpy(essid,WLANINFO->WLAN[i]->ESSID,sizeof(essid)-1);
						}
																							
						dot11WlanProfileTable_createEntry(messageNode->parameter,
														  globalWlanID,
														  WLANINFO->WLAN[i]->WlanID,
														  essid,
														  !WLANINFO->WLAN[i]->Status,
														  WLANINFO->WLAN[i]->SecurityID);
					}
        		}
        	}
        }
        free_dbus_message_list(&messageHead, Free_wlan_head);
    }    
		        
	snmp_log(LOG_DEBUG, "exit dot11WlanProfileTable_load\n");
}
netsnmp_variable_list *
dot11WlanProfileTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11WlanProfileTable_head == NULL)
	{
		return NULL;
	}
    *my_loop_context = dot11WlanProfileTable_head;
    return dot11WlanProfileTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WlanProfileTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WlanProfileTable_entry *entry = (struct dot11WlanProfileTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char *)&entry->globalWlanID, sizeof(long));
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }	
	return put_index_data;
}


/** handles requests for the dot11WlanProfileTable table */
int
dot11WlanProfileTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WlanProfileTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WlanProfileTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
			if( !table_entry ){
				netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
			}	  
    
            switch (table_info->colnum) {
            case COLUMN_WLANID:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->globalWlanID,
                                          sizeof(long));
                break;
            }
            case COLUMN_WLANESSID:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->WlanEssid,
                                          strlen(table_entry->WlanEssid));
                break;
            }
            case COLUMN_WLANSTATE:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->WlanState,
                                          sizeof(long));
                break;
            }
            case COLUMN_WLANBINDSECURITYID:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->WlanBindSecurityID,
                                          sizeof(long));
                break;
            }
            }
        }
        break;

    }
    return SNMP_ERR_NOERROR;
}
