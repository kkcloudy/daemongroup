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
* dot11QosBasicConfigTable.c
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
#include "dot11QosBasicConfigTable.h"
#include "wcpss/wid/WID.h"
#include "ws_dcli_wqos.h"
#include "ws_dbus_list_interface.h"
#include "autelanWtpGroup.h"

#define QOSBASICONFIGTABLE "3.1.1"
struct dot11QosBasicConfigTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalQosID;
    long QosID;

    /* Column values */
    //long old_QosID;
    long dot11QosTotalBW;
    //long old_dot11QosTotalBW;
    long dot11QosResScale;
    //long old_dot11QosResScale;
    long dot11QosShareBW;
    //long old_dot11QosShareBW;
    long dot11QosResShareScale;
    //long old_dot11QosResShareScale;
    char *dot11OperAtpArithmetic;
    //char *old_dot11OperAtpArithmetic;
    long dot11UseResGrabPolicy;
    //long old_dot11UseResGrabPolicy;
    char *dot11ResPolicyName;
    //char *old_dot11ResPolicyName;
    long dot11UseResShovePolicy;
    //long old_dot11UseResShovePolicy;
    char *dot11ResShoveName;
    //char *old_dot11ResShoveName;
	long  QosEnabled;
	//long  old_QosEnabled;
    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11QosBasicConfigTable_entry *next;
};

void dot11QosBasicConfigTable_load();
void
dot11QosBasicConfigTable_removeEntry( struct dot11QosBasicConfigTable_entry *entry );

/** Initializes the dot11QosBasicConfigTable module */
void
init_dot11QosBasicConfigTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11QosBasicConfigTable();
}

/** Initialize the dot11QosBasicConfigTable table by defining its contents and how it's structured */
void
initialize_table_dot11QosBasicConfigTable(void)
{
    static oid dot11QosBasicConfigTable_oid[128] = {0};
    size_t dot11QosBasicConfigTable_oid_len   = 0;	
	mad_dev_oid(dot11QosBasicConfigTable_oid,QOSBASICONFIGTABLE,&dot11QosBasicConfigTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11QosBasicConfigTable",     dot11QosBasicConfigTable_handler,
              dot11QosBasicConfigTable_oid, dot11QosBasicConfigTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalQosID */
                           0);
    table_info->min_column = DOT11QOSCONFMIN;
    table_info->max_column = DOT11QOSCONFMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11QosBasicConfigTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11QosBasicConfigTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11QosBasicConfigTable_load, dot11QosBasicConfigTable_removeEntry,
							dot11QosBasicConfigTable_oid, dot11QosBasicConfigTable_oid_len));

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11QosBasicConfigTable_entry  *dot11QosBasicConfigTable_head = NULL;

/* create a new row in the (unsorted) table */
struct dot11QosBasicConfigTable_entry *
dot11QosBasicConfigTable_createEntry(
				dbus_parameter parameter,
				long globalQosID,
                long  QosID,
			    long dot11QosTotalBW,
			    long dot11QosResScale,
			    long dot11QosShareBW,
			    long dot11QosResShareScale,
			    char *dot11OperAtpArithmetic,
			    long dot11UseResGrabPolicy,
			    char *dot11ResPolicyName,
			    long dot11UseResShovePolicy,
			    char *dot11ResShoveName,
			    long QosEnabled
                ) {
    struct dot11QosBasicConfigTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11QosBasicConfigTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->globalQosID = globalQosID;
    entry->QosID = QosID;
	entry->dot11QosTotalBW = dot11QosTotalBW,
    entry->dot11QosResScale = dot11QosResScale,
    entry->dot11QosShareBW = dot11QosShareBW,
    entry->dot11QosResShareScale = dot11QosResShareScale,
    entry->dot11OperAtpArithmetic = strdup(dot11OperAtpArithmetic),
    entry->dot11UseResGrabPolicy = dot11UseResGrabPolicy,
    entry->dot11ResPolicyName = strdup(dot11ResPolicyName),
    entry->dot11UseResShovePolicy = dot11UseResShovePolicy,
    entry->dot11ResShoveName = strdup(dot11ResShoveName);
	entry->QosEnabled= QosEnabled;
    entry->next = dot11QosBasicConfigTable_head;
    dot11QosBasicConfigTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11QosBasicConfigTable_removeEntry( struct dot11QosBasicConfigTable_entry *entry ) {
    struct dot11QosBasicConfigTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11QosBasicConfigTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11QosBasicConfigTable_head = ptr->next;
    else
        prev->next = ptr->next;

	if(NULL != entry->dot11OperAtpArithmetic)
	{
		free(entry->dot11OperAtpArithmetic);
	}
	if(NULL != entry->dot11ResPolicyName)
	{
		free(entry->dot11ResPolicyName);
	}
	if(NULL != entry->dot11ResShoveName)
	{
		free(entry->dot11ResShoveName);
	}

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11QosBasicConfigTable_load()
{	
	snmp_log(LOG_DEBUG, "enter dot11QosBasicConfigTable_load\n");

	 struct dot11QosBasicConfigTable_entry *temp;	
	 while( dot11QosBasicConfigTable_head )
	 {
		temp=dot11QosBasicConfigTable_head->next;
		dot11QosBasicConfigTable_removeEntry(dot11QosBasicConfigTable_head);
		dot11QosBasicConfigTable_head=temp;
	 }

	char wqos_id[10] = { 0 };
	
	char dot11OperAtpArithmetic[WID_QOS_ARITHMETIC_NAME_LEN] = { 0 };
	char dot11ResPolicyName[WID_QOS_ARITHMETIC_NAME_LEN] = { 0 };
	char dot11ResShoveName[WID_QOS_ARITHMETIC_NAME_LEN] = { 0 };
		
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wireless_qos_profile_list\n");
    messageHead = list_connection_call_dbus_method(show_wireless_qos_profile_list, SHOW_ALL_WTP_TABLE_METHOD);
    snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wireless_qos_profile_list,messageHead=%p\n", messageHead);
    
    
    if(messageHead)
    {
        for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
        {
            DCLI_WQOS *wireless_qos = messageNode->message;
            if(wireless_qos)
            {
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(messageNode->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    continue;
                
                int i = 0;
			    for(i=0;i<wireless_qos->qos_num;i++)
    			{
    			    int dot11QosTotalBW = 0;
    				int dot11QosResScale = 0;				
    				int dot11QosShareBW = 0;
    				int dot11QosResShareScale = 0;
    				int dot11UseResGrabPolicy = 0;
    				int dot11UseResShovePolicy = 0;
    				
    				memset(wqos_id,0,10);    				
					if(wireless_qos->qos[i])
					{
						snprintf(wqos_id,sizeof(wqos_id)-1,"%d",wireless_qos->qos[i]->QosID);
					}
    				
                    unsigned long globalQosID = 0;
					if(wireless_qos->qos[i])
					{
						globalQosID = local_to_global_ID(messageNode->parameter, 
													     wireless_qos->qos[i]->QosID, 
													     WIRELESS_MAX_NUM);
					}
					
                    int ret = 0;
                    DCLI_WQOS *wqos = NULL;
    				ret=show_qos_extension_info(messageNode->parameter, connection,wqos_id,&wqos);
					if((ret == 1)&&(wqos->qos[0]))
    				{
    					dot11QosTotalBW = wqos->qos[0]->qos_total_bandwidth;
    					dot11QosResScale = wqos->qos[0]->qos_res_scale;
    					dot11QosShareBW = wqos->qos[0]->qos_share_bandwidth;
    					dot11QosResShareScale = wqos->qos[0]->qos_res_share_scale;
    					memset(dot11OperAtpArithmetic,0,WID_QOS_ARITHMETIC_NAME_LEN);
						strncpy(dot11OperAtpArithmetic,wqos->qos[0]->qos_manage_arithmetic,sizeof(dot11OperAtpArithmetic)-1);
    					if(wqos->qos[0]->qos_use_res_grab==1)
    					{
    						dot11UseResGrabPolicy = 1;
    					}
    					else
    					{
    						dot11UseResGrabPolicy = 0;
    					}
    					memset(dot11ResPolicyName,0,WID_QOS_ARITHMETIC_NAME_LEN);
						strncpy(dot11ResPolicyName,wqos->qos[0]->qos_res_grab_arithmetic,sizeof(dot11ResPolicyName)-1);
    					if(wqos->qos[0]->qos_use_res_shove == 1)
    					{
    						dot11UseResShovePolicy = 1;
    					}
    					else
    					{
    						dot11UseResShovePolicy = 0;
    					}
    					memset(dot11ResShoveName,0,WID_QOS_ARITHMETIC_NAME_LEN);
						strncpy(dot11ResShoveName,wqos->qos[0]->qos_res_shove_arithmetic,sizeof(dot11ResShoveName)-1);

                        Free_qos_extension_info(wqos);
    				}
					
					if(wireless_qos->qos[i])
					{
						dot11QosBasicConfigTable_createEntry(messageNode->parameter,
															 globalQosID,
															 wireless_qos->qos[i]->QosID,
															 dot11QosTotalBW,
															 dot11QosResScale,
															 dot11QosShareBW,
															 dot11QosResShareScale,
															 dot11OperAtpArithmetic,
															 dot11UseResGrabPolicy,
															 dot11ResPolicyName,
															 dot11UseResShovePolicy,
															 dot11ResShoveName,
															 2);
					}
    				
    			}
    		}	
		}		
		free_dbus_message_list(&messageHead, Free_qos_head);
	}

	snmp_log(LOG_DEBUG, "exit dot11QosBasicConfigTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11QosBasicConfigTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{


	if(dot11QosBasicConfigTable_head==NULL)
		return NULL;
	*my_data_context = dot11QosBasicConfigTable_head;
	*my_loop_context = dot11QosBasicConfigTable_head;
	return dot11QosBasicConfigTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11QosBasicConfigTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11QosBasicConfigTable_entry *entry = (struct dot11QosBasicConfigTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->globalQosID, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11QosBasicConfigTable table */
int
dot11QosBasicConfigTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11QosBasicConfigTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11QosBasicConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);


			if( !table_entry ){
							netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
							continue;
						} 
				

	
            switch (table_info->colnum) {
            case COLUMN_QOSID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->globalQosID,
                                          sizeof(long));
                break;
            case COLUMN_DOT11QOSTOTALBW:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->dot11QosTotalBW,
                                          sizeof(long));
                break;
            case COLUMN_DOT11QOSRESSCALE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->dot11QosResScale,
                                          sizeof(long));
                break;
            case COLUMN_DOT11QOSSHAREBW:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->dot11QosShareBW,
                                          sizeof(long));
                break;
            case COLUMN_DOT11QOSRESSHARESCALE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->dot11QosResShareScale,
                                          sizeof(long));
                break;
            case COLUMN_DOT11OPERATPARITHMETIC:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->dot11OperAtpArithmetic,
                                          strlen(table_entry->dot11OperAtpArithmetic));
                break;
            case COLUMN_DOT11USERESGRABPOLICY:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->dot11UseResGrabPolicy,
                                          sizeof(long));
                break;
            case COLUMN_DOT11RESPOLICYNAME:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->dot11ResPolicyName,
                                          strlen(table_entry->dot11ResPolicyName));
                break;
            case COLUMN_DOT11USERESSHOVEPOLICY:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->dot11UseResShovePolicy,
                                          sizeof(long));
                break;
            case COLUMN_DOT11RESSHOVENAME:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->dot11ResShoveName,
                                          strlen(table_entry->dot11ResShoveName));
                break;
			case COLUMN_QOSENABLED:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->QosEnabled,
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
            table_entry = (struct dot11QosBasicConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_QOSID:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11QOSTOTALBW:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11QOSRESSCALE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11QOSSHAREBW:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11QOSRESSHARESCALE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11OPERATPARITHMETIC:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11USERESGRABPOLICY:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11RESPOLICYNAME:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11USERESSHOVEPOLICY:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11RESSHOVENAME:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }

                /* Also may need to check size/value */
                break;
			case COLUMN_QOSENABLED:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
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
            table_entry = (struct dot11QosBasicConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
				netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
			} 

            void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                break;
                
            switch (table_info->colnum) {
            case COLUMN_QOSID:
                /* Need to save old 'table_entry->QosID' value.
                   May need to use 'memcpy' */
                //table_entry->old_QosID = table_entry->QosID;
             //   table_entry->QosID     = request->requestvb->val.YYY;
                break;
            case COLUMN_DOT11QOSTOTALBW:
				{
					int ret = 0;
					char total_bandwidth[10]= { 0 };
					memset(total_bandwidth,0,10);
					snprintf(total_bandwidth,sizeof(total_bandwidth)-1,"%d",*request->requestvb->val.integer);

					ret=wid_config_set_qos_parameter(table_entry->parameter, connection,table_entry->QosID,"totalbandwidth",total_bandwidth);

					if(ret == 1)
					{
						table_entry->dot11QosTotalBW = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11QOSRESSCALE:
				{
					int ret = 0;
					char resource_scale[10]= { 0 };
					memset(resource_scale,0,10);
					snprintf(resource_scale,sizeof(resource_scale)-1,"%d",*request->requestvb->val.integer);

					ret=wid_config_set_qos_parameter(table_entry->parameter, connection,table_entry->QosID,"resourcescale",resource_scale);

					if(ret == 1)
					{
						table_entry->dot11QosResScale = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11QOSSHAREBW:
				{
					int ret = 0;
					char share_bandwidth[10]= { 0 };
					memset(share_bandwidth,0,10);
					snprintf(share_bandwidth,sizeof(share_bandwidth)-1,"%d",*request->requestvb->val.integer);

					ret=wid_config_set_qos_parameter(table_entry->parameter, connection,table_entry->QosID,"sharebandwidth",share_bandwidth);

					if(ret == 1)
					{
						table_entry->dot11QosShareBW = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11QOSRESSHARESCALE:
				{
					int ret = 0;
					char resource_share_scale[10]= { 0 };
					memset(resource_share_scale,0,10);
					snprintf(resource_share_scale,sizeof(resource_share_scale)-1,"%d",*request->requestvb->val.integer);

					ret=wid_config_set_qos_parameter(table_entry->parameter, connection,table_entry->QosID,"resourcesharescale",resource_share_scale);

					if(ret == 1)
					{
						table_entry->dot11QosResShareScale = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11OPERATPARITHMETIC:
				{
					int ret = 0;
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}

					ret=wid_config_set_qos_manage_arithmetic_name(table_entry->parameter, connection,table_entry->QosID,input_string);

					if(ret == 1)
					{
						if(table_entry->dot11OperAtpArithmetic!= NULL)
						{
							free(table_entry->dot11OperAtpArithmetic);
						}
						if(input_string)
						{
							table_entry->dot11OperAtpArithmetic = strdup(input_string);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
            	}
                break;
            case COLUMN_DOT11USERESGRABPOLICY:
				{
					int ret = 0;

					if(*request->requestvb->val.integer==1)
						ret=wid_config_set_qos_policy_used(table_entry->parameter, connection,table_entry->QosID,"grab","used");
					else
						ret=wid_config_set_qos_policy_used(table_entry->parameter, connection,table_entry->QosID,"grab","unused");
									
					if(ret == 1)
					{
						table_entry->dot11UseResGrabPolicy = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
            	}
                break;
            case COLUMN_DOT11RESPOLICYNAME:
				{
					int ret = 0;					
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}

					ret=wid_config_set_qos_policy_name(table_entry->parameter, connection,table_entry->QosID,"grab",input_string);
									
					if(ret == 1)
					{
						if(table_entry->dot11ResPolicyName!= NULL)
						{
							free(table_entry->dot11ResPolicyName);
						}
						if(input_string)
						{
							table_entry->dot11ResPolicyName = strdup(input_string);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
            	}
                break;
            case COLUMN_DOT11USERESSHOVEPOLICY:
				{
					int ret = 0;

					if(*request->requestvb->val.integer==1)
						ret=wid_config_set_qos_policy_used(table_entry->parameter, connection,table_entry->QosID,"shove","used");
					else
						ret=wid_config_set_qos_policy_used(table_entry->parameter, connection,table_entry->QosID,"shove","unused");
									
					if(ret == 1)
					{
						table_entry->dot11UseResShovePolicy = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
            	}
                break;
            case COLUMN_DOT11RESSHOVENAME:
				{
					int ret = 0;
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}

					ret=wid_config_set_qos_policy_name(table_entry->parameter, connection,table_entry->QosID,"shove",input_string);
									
					if(ret == 1)
					{
						if(table_entry->dot11ResShoveName!= NULL)
						{
							free(table_entry->dot11ResShoveName);
						}
						if(input_string)
						{
							table_entry->dot11ResShoveName = strdup(input_string);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
            	}
                break;
			case COLUMN_QOSENABLED:
				{
				}
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11QosBasicConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_QOSID:
                /* Need to restore old 'table_entry->QosID' value.
                   May need to use 'memcpy' */
                //table_entry->QosID = table_entry->old_QosID;
                break;
            case COLUMN_DOT11QOSTOTALBW:
                /* Need to restore old 'table_entry->dot11QosTotalBW' value.
                   May need to use 'memcpy' */
                //table_entry->dot11QosTotalBW = table_entry->old_dot11QosTotalBW;
                break;
            case COLUMN_DOT11QOSRESSCALE:
                /* Need to restore old 'table_entry->dot11QosResScale' value.
                   May need to use 'memcpy' */
                //table_entry->dot11QosResScale = table_entry->old_dot11QosResScale;
                break;
            case COLUMN_DOT11QOSSHAREBW:
                /* Need to restore old 'table_entry->dot11QosShareBW' value.
                   May need to use 'memcpy' */
                //table_entry->dot11QosShareBW = table_entry->old_dot11QosShareBW;
                break;
            case COLUMN_DOT11QOSRESSHARESCALE:
                /* Need to restore old 'table_entry->dot11QosResShareScale' value.
                   May need to use 'memcpy' */
                //table_entry->dot11QosResShareScale = table_entry->old_dot11QosResShareScale;
                break;
            case COLUMN_DOT11OPERATPARITHMETIC:
                /* Need to restore old 'table_entry->dot11OperAtpArithmetic' value.
                   May need to use 'memcpy' */
                //table_entry->dot11OperAtpArithmetic = table_entry->old_dot11OperAtpArithmetic;
                break;
            case COLUMN_DOT11USERESGRABPOLICY:
                /* Need to restore old 'table_entry->dot11UseResGrabPolicy' value.
                   May need to use 'memcpy' */
                //table_entry->dot11UseResGrabPolicy = table_entry->old_dot11UseResGrabPolicy;
                break;
            case COLUMN_DOT11RESPOLICYNAME:
                /* Need to restore old 'table_entry->dot11ResPolicyName' value.
                   May need to use 'memcpy' */
                //table_entry->dot11ResPolicyName = table_entry->old_dot11ResPolicyName;
                break;
            case COLUMN_DOT11USERESSHOVEPOLICY:
                /* Need to restore old 'table_entry->dot11UseResShovePolicy' value.
                   May need to use 'memcpy' */
                //table_entry->dot11UseResShovePolicy = table_entry->old_dot11UseResShovePolicy;
                break;
			case COLUMN_DOT11RESSHOVENAME:
                /* Need to restore old 'table_entry->dot11ResShoveName' value.
                   May need to use 'memcpy' */
                //table_entry->dot11ResShoveName = table_entry->old_dot11ResShoveName;
                break;
		    case COLUMN_QOSENABLED:
                /* Need to restore old 'table_entry->dot11ResShoveName' value.
                   May need to use 'memcpy' */
                //table_entry->QosEnabled= table_entry->old_QosEnabled;
                break;

            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
