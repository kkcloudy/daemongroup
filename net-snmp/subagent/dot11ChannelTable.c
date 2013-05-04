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
* dot11ChannelTable.c
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
#include "dot11ChannelTable.h"
#include "autelanWtpGroup.h"
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "ws_dbus_list_interface.h"

#define CHANNELTABLE	"1.7.3"
    /* Typical data structure for a row entry */
struct dot11ChannelTable_entry {
    /* Index values */
    long channelID;

    /* Column values */
    u_long monitorTime;
    //u_long old_monitorTime;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11ChannelTable_entry *next;
};

void dot11ChannelTable_load();
void
dot11ChannelTable_removeEntry( struct dot11ChannelTable_entry *entry );

/** Initializes the dot11ChannelTable module */
void
init_dot11ChannelTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11ChannelTable();
}

/** Initialize the dot11ChannelTable table by defining its contents and how it's structured */
void
initialize_table_dot11ChannelTable(void)
{
    static oid dot11ChannelTable_oid[128] = {0};
    size_t dot11ChannelTable_oid_len   = 0;
	mad_dev_oid(dot11ChannelTable_oid,CHANNELTABLE,&dot11ChannelTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11ChannelTable",     dot11ChannelTable_handler,
              dot11ChannelTable_oid, dot11ChannelTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: channelID */
                           0);
    table_info->min_column = CHANNEL_MIN;
    table_info->max_column = CHANNEL_MAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11ChannelTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11ChannelTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11ChannelTable_load, dot11ChannelTable_removeEntry,
							dot11ChannelTable_oid, dot11ChannelTable_oid_len));

    /* Initialise the contents of the table here */
}


struct dot11ChannelTable_entry  *dot11ChannelTable_head;

/* create a new row in the (unsorted) table */
struct dot11ChannelTable_entry *
dot11ChannelTable_createEntry(
                 long  channelID,
    		   u_long monitorTime
                ) {
    struct dot11ChannelTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11ChannelTable_entry);
    if (!entry)
        return NULL;

    entry->channelID = channelID;
    entry->monitorTime = monitorTime;
    entry->next = dot11ChannelTable_head;
    dot11ChannelTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11ChannelTable_removeEntry( struct dot11ChannelTable_entry *entry ) {
    struct dot11ChannelTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11ChannelTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11ChannelTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11ChannelTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11ChannelTable_load\n");

	struct dot11ChannelTable_entry *temp = NULL;
	while( dot11ChannelTable_head ) {
		temp=dot11ChannelTable_head->next;
		dot11ChannelTable_removeEntry(dot11ChannelTable_head);
		dot11ChannelTable_head=temp;
	}
	unsigned long monitorTime[20] = { 0 };
	memset(monitorTime, 0 ,sizeof(monitorTime));
	
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_channel_access_time_cmd\n");
    messageHead = list_connection_call_dbus_method(show_channel_access_time_cmd, SHOW_ALL_WTP_TABLE_METHOD);
	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_channel_access_time_cmd,messageHead=%p\n", messageHead);

	if(messageHead)
	{
		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
		{
		    struct dcli_channel_info *channel = messageNode->message;
		    if(channel)
		    {
		        struct dcli_channel_info *channel_node = NULL;
		        for(channel_node = channel->channel_list; NULL != channel_node; channel_node = channel_node->next)
                {
        			monitorTime[channel_node->channel_id] += channel_node->StaTime;
				}
		    }
		}
		printf("before free_dbus_message_list\n");
		free_dbus_message_list(&messageHead, Free_channel_access_time_head);
		printf("after free_dbus_message_list\n");
	}
	
	int i;
	for(i = 1; i < 14; i++)
	{
	    dot11ChannelTable_createEntry(i,
        							  monitorTime[i]*100);
    }
    
	snmp_log(LOG_DEBUG, "exit dot11ChannelTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11ChannelTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11ChannelTable_head==NULL)
			return NULL;
    *my_data_context = dot11ChannelTable_head;
    *my_loop_context = dot11ChannelTable_head;
    return dot11ChannelTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11ChannelTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11ChannelTable_entry *entry = (struct dot11ChannelTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx,(u_char*)&entry->channelID, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11ChannelTable table */
int
dot11ChannelTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11ChannelTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ChannelTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
		if( !table_entry )
			{
				netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
			}     
            switch (table_info->colnum) {
            case COLUMN_CHANNELID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->channelID,
                                          sizeof(long));
                break;
            case COLUMN_MONITORTIME:
                snmp_set_var_typed_value( request->requestvb, ASN_TIMETICKS,
                                          (u_char *)&table_entry->monitorTime,
                                          sizeof(long));
                break;
            }
        }
        break;
	#if 0
        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ChannelTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_MONITORTIME:
                if ( request->requestvb->type != ASN_TIMETICKS ) {
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
            table_entry = (struct dot11ChannelTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_MONITORTIME:
                /* Need to save old 'table_entry->monitorTime' value.
                   May need to use 'memcpy' */
                table_entry->old_monitorTime = table_entry->monitorTime;
                table_entry->monitorTime     = request->requestvb->val.YYY;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ChannelTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_MONITORTIME:
                /* Need to restore old 'table_entry->monitorTime' value.
                   May need to use 'memcpy' */
                table_entry->monitorTime = table_entry->old_monitorTime;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
	#endif
    }
    return SNMP_ERR_NOERROR;
}
