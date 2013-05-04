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
* dot11VlanAbilityTable.c
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
#include "dot11VlanAbilityTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_init_dbus.h"
#include "ws_sysinfo.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define VLANABILITYTABLE "1.5.5"

    /* Typical data structure for a row entry */
struct dot11VlanAbilityTable_entry {
    /* Index values */
   // long wtpCurrID;
   	dbus_parameter parameter;
	char *wtpMacAddr;
	long local_wlanCurrID;
	long wlanCurrID;

    /* Column values */
    long vlanID;
    //long old_vlanID;
    long Priority;
    //long old_Priority;
    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11VlanAbilityTable_entry *next;
};

void dot11VlanAbilityTable_load();
void
dot11VlanAbilityTable_removeEntry( struct dot11VlanAbilityTable_entry *entry );

/** Initializes the dot11VlanAbilityTable module */
void
init_dot11VlanAbilityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11VlanAbilityTable();
}

/** Initialize the dot11VlanAbilityTable table by defining its contents and how it's structured */
void
initialize_table_dot11VlanAbilityTable(void)
{
    static oid dot11VlanAbilityTable_oid[128] = {0};
	size_t dot11VlanAbilityTable_oid_len   = 0;
	mad_dev_oid(dot11VlanAbilityTable_oid,VLANABILITYTABLE,&dot11VlanAbilityTable_oid_len,enterprise_pvivate_oid);
    
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11VlanAbilityTable",     dot11VlanAbilityTable_handler,
              dot11VlanAbilityTable_oid, dot11VlanAbilityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: wtpCurrID */
                           ASN_INTEGER,  /* index: wlanCurrID */
                           0);
    table_info->min_column = VLANABILITITY_MIN;
    table_info->max_column = VLANABITLITY_MAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11VlanAbilityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11VlanAbilityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11VlanAbilityTable_load, dot11VlanAbilityTable_removeEntry,
							dot11VlanAbilityTable_oid, dot11VlanAbilityTable_oid_len));

    /* Initialise the contents of the table here */
}



struct dot11VlanAbilityTable_entry  *dot11VlanAbilityTable_head;

/* create a new row in the (unsorted) table */
struct dot11VlanAbilityTable_entry *
dot11VlanAbilityTable_createEntry(
				 dbus_parameter parameter,
				 char *wtpMacAddr,                                 
				 long local_wlanCurrID,
                 long  wlanCurrID,
                   long vlanID,
                   long Priority
                ) 
{
	struct dot11VlanAbilityTable_entry *entry;

	entry = SNMP_MALLOC_TYPEDEF(struct dot11VlanAbilityTable_entry);
	if (!entry)
	return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->wtpMacAddr	= strdup(wtpMacAddr);	
	entry->local_wlanCurrID = local_wlanCurrID;
	entry->wlanCurrID = wlanCurrID;
	entry->vlanID = vlanID;
	entry->Priority = Priority;
	entry->next = dot11VlanAbilityTable_head;
	dot11VlanAbilityTable_head = entry;
	return entry;
}

/* remove a row from the table */
void
dot11VlanAbilityTable_removeEntry( struct dot11VlanAbilityTable_entry *entry ) {
    struct dot11VlanAbilityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11VlanAbilityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11VlanAbilityTable_head = ptr->next;
    else
        prev->next = ptr->next;

    FREE_OBJECT(entry->wtpMacAddr);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11VlanAbilityTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11VlanAbilityTable_load\n");

	struct dot11VlanAbilityTable_entry *temp = NULL;
	while( dot11VlanAbilityTable_head ) {
		temp=dot11VlanAbilityTable_head->next;
		dot11VlanAbilityTable_removeEntry(dot11VlanAbilityTable_head);
		dot11VlanAbilityTable_head=temp;
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
		    void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(messageNode->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                continue;
                
		    if((head)&&(head->WTP_INFO)&&(head->WTP_INFO->WTP_LIST))
			{
			    int i = 0;
		        WID_WTP *q = NULL;
				for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q=q->next)
				{
					memset(temp_mac,0,20);
					if(q->WTPMAC)
					{
						snprintf(temp_mac,sizeof(temp_mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
					}

                    int ret = 0;
					DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;   
					ret = show_wtp_wlan_vlan_information(messageNode->parameter, connection,q->WTPID,&WTPINFO);
					if(ret == 1 && WTPINFO)
					{
                        int j = 0;
						for(j=0;j<WTPINFO->wlan_num;j++)
						{
							if(WTPINFO->WLAN[j])
							{
								unsigned long wlanid = local_to_global_ID(messageNode->parameter, WTPINFO->WLAN[j]->WlanID, WIRELESS_MAX_NUM);
								
								dot11VlanAbilityTable_createEntry(messageNode->parameter,
																	temp_mac,
																	WTPINFO->WLAN[j]->WlanID,
																	wlanid,
																	WTPINFO->WLAN[j]->vlanid,
																	WTPINFO->WLAN[j]->wlan_1p_priority);
							}
						}
						free_show_wtp_wlan_vlan_information(WTPINFO);
					}
                    else if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(messageNode->parameter.slot_id);
						FREE_OBJECT(q->WTPMAC);
                        break;
            	    }
					FREE_OBJECT(q->WTPMAC);
				}
			}
		}
		free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
	}

	snmp_log(LOG_DEBUG, "exit dot11VlanAbilityTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11VlanAbilityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{

	if(dot11VlanAbilityTable_head==NULL)
		return NULL;
	*my_data_context = dot11VlanAbilityTable_head;
	*my_loop_context = dot11VlanAbilityTable_head;
	return dot11VlanAbilityTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11VlanAbilityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11VlanAbilityTable_entry *entry = (struct dot11VlanAbilityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char *)entry->wtpMacAddr, strlen(entry->wtpMacAddr) );
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


/** handles requests for the dot11VlanAbilityTable table */
int
dot11VlanAbilityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) 
{

	netsnmp_request_info       *request;
	netsnmp_table_request_info *table_info;
	struct dot11VlanAbilityTable_entry          *table_entry;

	switch (reqinfo->mode) 
	{
		/*
		* Read-support (also covers GetNext requests)
		*/
		case MODE_GET:
		{
			for (request=requests; request; request=request->next) 
			{
				table_entry = (struct dot11VlanAbilityTable_entry *)
				netsnmp_extract_iterator_context(request);
				table_info  =     netsnmp_extract_table_info(request);


				if( !table_entry )
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}     

				switch (table_info->colnum) 
				{
					case COLUMN_VLANID:
					{
						snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
						(u_char*)&table_entry->vlanID,
						sizeof(long));
					
						break;
					}
					case COLUMN_PRIORITY:
					{
						snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
						(u_char*)&table_entry->Priority,
						sizeof(long));
					
						break;
					}
				default:
				                netsnmp_set_request_error( reqinfo, request,
				                                           SNMP_ERR_NOTWRITABLE );
				                return SNMP_ERR_NOERROR;	
				}
			}
		}
		break;

		/*
		* Write-support
		*/
		case MODE_SET_RESERVE1:
		{
			for (request=requests; request; request=request->next) 
			{
				table_entry = (struct dot11VlanAbilityTable_entry *)
				netsnmp_extract_iterator_context(request);
				table_info  =     netsnmp_extract_table_info(request);

				switch (table_info->colnum) 
				{
					case COLUMN_VLANID:
					{
						if ( request->requestvb->type != ASN_INTEGER ) 
						{
							netsnmp_set_request_error( reqinfo, request,SNMP_ERR_WRONGTYPE );
							return SNMP_ERR_NOERROR;
						}
					}
					/* Also may need to check size/value */
					break;

					case COLUMN_PRIORITY:
					{
						if ( request->requestvb->type != ASN_INTEGER ) 
						{
							netsnmp_set_request_error( reqinfo, request,SNMP_ERR_WRONGTYPE );
							return SNMP_ERR_NOERROR;
						}
					}
					/* Also may need to check size/value */
					break;

					
					default:
					netsnmp_set_request_error( reqinfo, request,SNMP_ERR_NOTWRITABLE );
					return SNMP_ERR_NOERROR;
				}
			}
		}
		break;

		case MODE_SET_RESERVE2:
		break;

		case MODE_SET_FREE:
		break;

		case MODE_SET_ACTION:
		{
			for (request=requests; request; request=request->next) 
			{
				table_entry = (struct dot11VlanAbilityTable_entry *)
				netsnmp_extract_iterator_context(request);
				table_info  =     netsnmp_extract_table_info(      request);

				if( !table_entry )
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}  
				switch (table_info->colnum) 
				{
					case COLUMN_VLANID:
					{
                        void *connection = NULL;
                        if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                            return MFD_ERROR;
        
						int ret1=0,ret2=0;
						char vlanid[10] = { 0 };
						memset(vlanid,0,10);
						/* Need to save old 'table_entry->vlanID' value.
						May need to use 'memcpy' */
						//table_entry->old_vlanID = table_entry->vlanID;
						table_entry->vlanID     = *request->requestvb->val.integer;
						ret1=config_wlan_service(table_entry->parameter, connection,table_entry->local_wlanCurrID,"disable");
						if(ret1!=1)
						{	
						    if(SNMPD_CONNECTION_ERROR == ret1) {
                                close_slot_dbus_connection(table_entry->parameter.slot_id);
                    	    }
							netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
							break;
						}
						
						snprintf(vlanid,sizeof(vlanid)-1,"%d",table_entry->vlanID);
						ret2=set_wlan_vlan_id(table_entry->parameter, connection,table_entry->local_wlanCurrID,vlanid);
						if(ret2!=1)
						{	
						    if(SNMPD_CONNECTION_ERROR == ret2) {
                                close_slot_dbus_connection(table_entry->parameter.slot_id);
                    	    }
							netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
						}						
					}
					break;

					case COLUMN_PRIORITY:
					{
                        void *connection = NULL;
                        if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                            return MFD_ERROR;
                            
						int ret1=0,ret2=0;
						char pro[10] = { 0 };
						memset(pro,0,10);
						/* Need to save old 'table_entry->Priority' value.
						May need to use 'memcpy' */
						//table_entry->old_Priority = table_entry->Priority;
						table_entry->Priority = *request->requestvb->val.integer;
						ret1=config_wlan_service(table_entry->parameter, connection,table_entry->local_wlanCurrID,"disable");
						if(ret1!=1)
						{	
						    if(SNMPD_CONNECTION_ERROR == ret1) {
                                close_slot_dbus_connection(table_entry->parameter.slot_id);
                    	    }
							netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
							break;
						}
						
						snprintf(pro,sizeof(pro)-1,"%d",table_entry->Priority);
						ret2=set_wlan_vlan_priority(table_entry->parameter, connection,table_entry->local_wlanCurrID,pro);
						if(ret2!=1)
						{	
						    if(SNMPD_CONNECTION_ERROR == ret2) {
                                close_slot_dbus_connection(table_entry->parameter.slot_id);
                    	    }
							netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
						}
					}
					break;

				}
			}
		}
		break;

		case MODE_SET_UNDO:
		{
			for (request=requests; request; request=request->next) 
			{
				table_entry = (struct dot11VlanAbilityTable_entry *)
				netsnmp_extract_iterator_context(request);
				table_info  =     netsnmp_extract_table_info(request);

				switch (table_info->colnum) 
				{
					case COLUMN_VLANID:
					{
						/* Need to restore old 'table_entry->vlanID' value.
						May need to use 'memcpy' */
						//  table_entry->vlanID = table_entry->old_vlanID;
					}
					break;

					case COLUMN_PRIORITY:
					{
						/* Need to restore old 'table_entry->Priority' value.
						May need to use 'memcpy' */
						//  table_entry->Priority = table_entry->old_Priority;
					}
					break;
				}
			}
		}
		break;

		case MODE_SET_COMMIT:
		break;
	}
return SNMP_ERR_NOERROR;
}
