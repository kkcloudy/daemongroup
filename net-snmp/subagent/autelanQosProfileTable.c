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
*autelanQosProfileTable.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* complete wireless qos infomation table
*
*
*******************************************************************************/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "autelanQosProfileTable.h"
#include "wcpss/wid/WID.h"
#include "ws_dcli_wqos.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define QOSPROFILETABLE "3.2.1"
struct autelanQosProfileTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalQosProfileIndex;
    long qosProfileIndex;

    /* Column values */
    char *qosProfileName;
    long qosRadioBestEffortDepth;
    long qosRadioBackGroundDepth;
    long qosClientBestEffortDepth;
    long qosClientBackGroundDepth;
    long qosWmmBestEffortMapPriority;
    long qosWmmBackGroundMapPriority;

    /* Illustrate using a simple linked list */
    int   valid;
    struct autelanQosProfileTable_entry *next;
};

void autelanQosProfileTable_load();
void
autelanQosProfileTable_removeEntry( struct autelanQosProfileTable_entry *entry );

/** Initializes the autelanQosProfileTable module */
void
init_autelanQosProfileTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_autelanQosProfileTable();
}

/** Initialize the autelanQosProfileTable table by defining its contents and how it's structured */
void
initialize_table_autelanQosProfileTable(void)
{
    static oid autelanQosProfileTable_oid[128] = {0};
    size_t autelanQosProfileTable_oid_len   = 0;
	
	mad_dev_oid(autelanQosProfileTable_oid,QOSPROFILETABLE,&autelanQosProfileTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "autelanQosProfileTable",     autelanQosProfileTable_handler,
              autelanQosProfileTable_oid, autelanQosProfileTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalQosProfileIndex */
                           0);
    table_info->min_column = QOSPROFILETABLE_MIN_COLUMN;
    table_info->max_column = QOSPROFILETABLE_MAX_COLUMN;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = autelanQosProfileTable_get_first_data_point;
    iinfo->get_next_data_point  = autelanQosProfileTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,autelanQosProfileTable_load, autelanQosProfileTable_removeEntry,
							autelanQosProfileTable_oid, autelanQosProfileTable_oid_len));

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct autelanQosProfileTable_entry  *autelanQosProfileTable_head;

/* create a new row in the (unsorted) table */
struct autelanQosProfileTable_entry *
autelanQosProfileTable_createEntry(
					dbus_parameter parameter,
					long globalQosProfileIndex,
                 	long  qosProfileIndex,
                 	char *qosProfileName,
				    long qosRadioBestEffortDepth,
				    long qosRadioBackGroundDepth,
				    long qosClientBestEffortDepth,
				    long qosClientBackGroundDepth,
				    long qosWmmBestEffortMapPriority,
				    long qosWmmBackGroundMapPriority                 
                ) {
    struct autelanQosProfileTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct autelanQosProfileTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
    entry->qosProfileIndex = qosProfileIndex;
    entry->globalQosProfileIndex = globalQosProfileIndex;
    entry->qosProfileName = strdup(qosProfileName);
    entry->qosRadioBestEffortDepth = qosRadioBestEffortDepth;
    entry->qosRadioBackGroundDepth = qosRadioBackGroundDepth;
    entry->qosClientBestEffortDepth = qosClientBestEffortDepth;
    entry->qosClientBackGroundDepth = qosClientBackGroundDepth;
    entry->qosWmmBestEffortMapPriority = qosWmmBestEffortMapPriority;
    entry->qosWmmBackGroundMapPriority = qosWmmBackGroundMapPriority;
    
    entry->valid = 1;
    
    entry->next = autelanQosProfileTable_head;
    autelanQosProfileTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
autelanQosProfileTable_removeEntry( struct autelanQosProfileTable_entry *entry ) {
    struct autelanQosProfileTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = autelanQosProfileTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        autelanQosProfileTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void autelanQosProfileTable_load()
{	
    snmp_log(LOG_DEBUG, "enter autelanQosProfileTable_load\n");
    
	struct autelanQosProfileTable_entry *temp;	
	while( autelanQosProfileTable_head )
	{
		temp=autelanQosProfileTable_head->next;
		autelanQosProfileTable_removeEntry(autelanQosProfileTable_head);
		autelanQosProfileTable_head=temp;
	}
	
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wireless_qos_profile_list\n");
    messageHead = list_connection_call_dbus_method(show_wireless_qos_profile_list, SHOW_ALL_WTP_TABLE_METHOD);
    snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wireless_qos_profile_list,messageHead=%p\n", messageHead);
    
    if(messageHead)
    {
        for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
        {
            DCLI_WQOS *wqos = messageNode->message;			
			char name[255] = { 0 };
            if(wqos)
            {
                int i = 0;
                for(i=0;i<wqos->qos_num;i++)
    			{    
    				unsigned long globalQosProfileIndex = 0;
					if(wqos->qos[i])
					{
						globalQosProfileIndex = local_to_global_ID(messageNode->parameter, 
																   wqos->qos[i]->QosID, 
																   WIRELESS_MAX_NUM);
						
						memset(name,0,sizeof(name));
						if(wqos->qos[i]->name)
						{
							strncpy(name,wqos->qos[i]->name,sizeof(name)-1);
						}

						autelanQosProfileTable_createEntry(messageNode->parameter,
														   globalQosProfileIndex,
														   wqos->qos[i]->QosID,
														   name,
														   0,
														   0,
														   0,
														   0,
														   0,
														   0);				
					}
    			}
    		}
    	}
    	free_dbus_message_list(&messageHead, Free_qos_head);
    }	
    
    snmp_log(LOG_DEBUG, "exit autelanQosProfileTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
autelanQosProfileTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{


	if(autelanQosProfileTable_head==NULL)
		return NULL;
	*my_data_context = autelanQosProfileTable_head;
	*my_loop_context = autelanQosProfileTable_head;
	return autelanQosProfileTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
autelanQosProfileTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct autelanQosProfileTable_entry *entry = (struct autelanQosProfileTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->globalQosProfileIndex, sizeof(entry->globalQosProfileIndex) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
    
    return put_index_data;
}


/** handles requests for the autelanQosProfileTable table */
int
autelanQosProfileTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct autelanQosProfileTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct autelanQosProfileTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
	   		if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}    		

            void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                break;
            
            switch (table_info->colnum) {
            case COLUMN_QOSPROFILEINDEX:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->globalQosProfileIndex,
                                          sizeof(table_entry->globalQosProfileIndex));
            }
                break;
            case COLUMN_QOSPROFILENAME:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->qosProfileName,
                                          strlen(table_entry->qosProfileName));
            }
                break;
            case COLUMN_QOSRADIOBESTEFFORTDEPTH:
			{		
				int qosRadioBestEffortDepth = 0;
				int ret = 0;
				char id[20] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(id,0,20);
				snprintf(id,sizeof(id)-1,"%d",table_entry->qosProfileIndex);
				ret=show_qos_one(table_entry->parameter, connection,id,&wqos);							
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[0]))
				{
					qosRadioBestEffortDepth = wqos->qos[0]->radio_qos[0]->QueueDepth;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&qosRadioBestEffortDepth,
                                          sizeof(qosRadioBestEffortDepth));
				
				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_QOSRADIOBACKGROUNDDEPTH:
			{
				int qosRadioBackGroundDepth = 0;
				int ret = 0;
				char id[20] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(id,0,20);
				snprintf(id,sizeof(id)-1,"%d",table_entry->qosProfileIndex);
				ret=show_qos_one(table_entry->parameter, connection,id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[1]))
				{
					qosRadioBackGroundDepth = wqos->qos[0]->radio_qos[1]->QueueDepth;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&qosRadioBackGroundDepth,
                                          sizeof(qosRadioBackGroundDepth));

				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_QOSCLIENTBESTEFFORTDEPTH:
			{
				int qosClientBestEffortDepth = 0;
				int ret = 0;
				char id[20] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(id,0,20);
				snprintf(id,sizeof(id)-1,"%d",table_entry->qosProfileIndex);
				ret=show_qos_one(table_entry->parameter, connection,id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->client_qos[0]))
				{
					qosClientBestEffortDepth = wqos->qos[0]->client_qos[0]->QueueDepth;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&qosClientBestEffortDepth,
                                          sizeof(qosClientBestEffortDepth));

				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_QOSCLIENTBACKGROUNDDEPTH:
			{
				int qosClientBackGroundDepth = 0;
				int ret = 0;
				char id[20] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(id,0,20);
				snprintf(id,sizeof(id)-1,"%d",table_entry->qosProfileIndex);
				ret=show_qos_one(table_entry->parameter, connection,id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->client_qos[1]))
				{
					qosClientBackGroundDepth = wqos->qos[0]->client_qos[1]->QueueDepth;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&qosClientBackGroundDepth,
                                          sizeof(qosClientBackGroundDepth));

				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_QOSWMMBESTEFFORTMAPPRIORITY:
			{
				int qosWmmBestEffortMapPriority = 0;
				int ret = 0;
				char id[20] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(id,0,20);
				snprintf(id,sizeof(id)-1,"%d",table_entry->qosProfileIndex);
				ret=show_qos_one(table_entry->parameter, connection,id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[0]))
				{
					if(wqos->qos[0]->radio_qos[0]->mapstate==1)
					{
						qosWmmBestEffortMapPriority=wqos->qos[0]->radio_qos[0]->wmm_map_dot1p;
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&qosWmmBestEffortMapPriority,
                                          sizeof(qosWmmBestEffortMapPriority));

				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_QOSWMMBACKGROUNDMAPPRIORITY:
			{
				int qosWmmBackGroundMapPriority = 0;
				int ret = 0;
				char id[20] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(id,0,20);
				snprintf(id,sizeof(id)-1,"%d",table_entry->qosProfileIndex);
				ret=show_qos_one(table_entry->parameter, connection,id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[1]))
				{
					if(wqos->qos[0]->radio_qos[1]->mapstate==1)
					{
						qosWmmBackGroundMapPriority=wqos->qos[0]->radio_qos[1]->wmm_map_dot1p;
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&qosWmmBackGroundMapPriority,
                                          sizeof(qosWmmBackGroundMapPriority));

				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            default:
			{
                netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
            }
				break;                
            }
        }
        break;

    }
    return SNMP_ERR_NOERROR;
}
