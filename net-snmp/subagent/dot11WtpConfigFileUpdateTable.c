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
* dot11WtpConfigFileUpdateTable.c
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
#include "dot11WtpConfigFileUpdateTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "autelanWtpGroup.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

#define FILEOID	"1.1.3"
/** Initializes the dot11WtpConfigFileUpdateTable module */

struct dot11WtpConfigFileUpdateTable_entry {
    /* Index values */
    dbus_parameter parameter;
    
    long wtpCurrID;
	
	char *wtpMacAddr;
    /* Column values */
    char *wtpConfigFileUpdate;
   // char *old_wtpConfigFileUpdate;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WtpConfigFileUpdateTable_entry *next;
};


void dot11WtpConfigFileUpdateTable_load(void);
void dot11WtpConfigFileUpdateTable_removeEntry( struct dot11WtpConfigFileUpdateTable_entry *entry );


void
init_dot11WtpConfigFileUpdateTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WtpConfigFileUpdateTable();
}

/** Initialize the dot11WtpConfigFileUpdateTable table by defining its contents and how it's structured */
void
initialize_table_dot11WtpConfigFileUpdateTable(void)
{
    static oid dot11WtpConfigFileUpdateTable_oid[128] = {0};
    size_t dot11WtpConfigFileUpdateTable_oid_len   = 0;
	mad_dev_oid(dot11WtpConfigFileUpdateTable_oid,FILEOID,&dot11WtpConfigFileUpdateTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WtpConfigFileUpdateTable",     dot11WtpConfigFileUpdateTable_handler,
              dot11WtpConfigFileUpdateTable_oid, dot11WtpConfigFileUpdateTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: wtpCurrID */
                           0);
    table_info->min_column = COLUMN_FILEMIN;
    table_info->max_column = COLUMN_FILEMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WtpConfigFileUpdateTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WtpConfigFileUpdateTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WtpConfigFileUpdateTable_load, dot11WtpConfigFileUpdateTable_removeEntry,dot11WtpConfigFileUpdateTable_oid, dot11WtpConfigFileUpdateTable_oid_len));


    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11WtpConfigFileUpdateTable_entry  *dot11WtpConfigFileUpdateTable_head;

/* create a new row in the (unsorted) table */
struct dot11WtpConfigFileUpdateTable_entry *
dot11WtpConfigFileUpdateTable_createEntry(
                 dbus_parameter parameter,
                 long  wtpCurrID,
                 char *wtpMacAddr,
		    char *wtpConfigFileUpdate
                ) {
    struct dot11WtpConfigFileUpdateTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WtpConfigFileUpdateTable_entry);
    if (!entry)
        return NULL;
    memcpy(&(entry->parameter), &parameter, sizeof(dbus_parameter));
//	entry->instanceID = instanceID;
    entry->wtpCurrID = wtpCurrID;
	entry->wtpMacAddr	= strdup(wtpMacAddr);    	
	entry->wtpConfigFileUpdate = strdup(wtpConfigFileUpdate);
    entry->next = dot11WtpConfigFileUpdateTable_head;
    dot11WtpConfigFileUpdateTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WtpConfigFileUpdateTable_removeEntry( struct dot11WtpConfigFileUpdateTable_entry *entry ) {
    struct dot11WtpConfigFileUpdateTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WtpConfigFileUpdateTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WtpConfigFileUpdateTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->wtpMacAddr);
	FREE_OBJECT(entry->wtpConfigFileUpdate);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11WtpConfigFileUpdateTable_load()
{	
	snmp_log(LOG_DEBUG, "enter dot11WtpConfigFileUpdateTable_load\n");

	struct dot11WtpConfigFileUpdateTable_entry *temp = NULL;
	while( dot11WtpConfigFileUpdateTable_head )
	{
		temp=dot11WtpConfigFileUpdateTable_head->next;
		dot11WtpConfigFileUpdateTable_removeEntry(dot11WtpConfigFileUpdateTable_head);
		dot11WtpConfigFileUpdateTable_head=temp;
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
    		    int i = 0;
    		    WID_WTP *q=NULL;
    			for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q=q->next)
    			{
    			    
    				memset(temp_mac, 0, sizeof(temp_mac));
                    if(q->WTPMAC) {
    				    snprintf(temp_mac, sizeof(temp_mac) - 1, "%02X:%02X:%02X:%02X:%02X:%02X",
    				                q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
                    }
    				dot11WtpConfigFileUpdateTable_createEntry(messageNode->parameter,
    														q->WTPID,
    														temp_mac,
    														"1");
					FREE_OBJECT(q->WTPMAC);
    			}
    		}
        }
        free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
    }
	snmp_log(LOG_DEBUG, "exit dot11WtpConfigFileUpdateTable_load\n");
}


/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WtpConfigFileUpdateTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{

	if(dot11WtpConfigFileUpdateTable_head==NULL)
		{
			return NULL;
		}
    *my_data_context = dot11WtpConfigFileUpdateTable_head;
    *my_loop_context = dot11WtpConfigFileUpdateTable_head;
    return dot11WtpConfigFileUpdateTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WtpConfigFileUpdateTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WtpConfigFileUpdateTable_entry *entry = (struct dot11WtpConfigFileUpdateTable_entry *)*my_loop_context;
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


/** handles requests for the dot11WtpConfigFileUpdateTable table */
int
dot11WtpConfigFileUpdateTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WtpConfigFileUpdateTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpConfigFileUpdateTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
		 if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }    
	
            switch (table_info->colnum) {
            case COLUMN_WTPCONFIGFILEUPDATE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->wtpConfigFileUpdate,
                                          strlen(table_entry->wtpConfigFileUpdate));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpConfigFileUpdateTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WTPCONFIGFILEUPDATE:
                if ( request->requestvb->type != ASN_OCTET_STR) {
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
            table_entry = (struct dot11WtpConfigFileUpdateTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
			
    	 if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }    
		 
            switch (table_info->colnum) {
				#if 0
            case COLUMN_WTPVERSIONFILEUPDATE:
		{
			char *ip  = (char *)malloc(20);
			char *filename = (char *)malloc(100);
			memset(ip,0,20);
			memset(filename,0,100);
			int ret =  0;
			int ret_down = 0;
			ret  = sscanf(request->requestvb->val.string,"%s %s",ip,filename);
			if(ret==3)
			{
				ret_down = download_ap_version(url,uname,pwd);
				if(ret_down ==-1)
				{
					netsnmp_set_request_error( reqinfo, request,SNMP_ERR_COMMITFAILED);
				}
			}
			else
			{
				netsnmp_set_request_error( reqinfo, request,SNMP_ERR_BADVALUE);
			}
			free(ip);
			free(filename);
            	}
                break;
				#endif
		case COLUMN_WTPCONFIGFILEUPDATE:
		{
			int ret = 0;
            void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
            {
                netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
                break;
            }
            
			char input_string[256] = { 0 };
			memset(input_string,0,sizeof(input_string));
			strncpy(input_string, request->requestvb->val.string, sizeof(input_string) - 1);
			
			ret = set_ap_config_update_func(table_entry->parameter, connection,table_entry->wtpCurrID,input_string);
			if(ret != 1)
			{   
			    if (SNMPD_CONNECTION_ERROR == ret) {
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
            table_entry = (struct dot11WtpConfigFileUpdateTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
				#if 0
            case COLUMN_WTPVERSIONFILEUPDATE:
                /* Need to restore old 'table_entry->wtpVersionFileUpdate' value.
                   May need to use 'memcpy' */
                table_entry->wtpVersionFileUpdate =strdup(table_entry->old_wtpVersionFileUpdate);
                break;
				#endif
            case COLUMN_WTPCONFIGFILEUPDATE:
                /* Need to restore old 'table_entry->wtpConfigFileUpdate' value.
                   May need to use 'memcpy' */
               // table_entry->wtpConfigFileUpdate = strdup(table_entry->old_wtpConfigFileUpdate);
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
