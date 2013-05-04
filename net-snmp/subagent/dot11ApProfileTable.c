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
* dot11ApProfileTable.c
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
#include "dot11ApProfileTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_init_dbus.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define 	HIDDENAP	"6.1.1"
    /* Typical data structure for a row entry */
struct dot11ApProfileTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalApID;
    long ApID;

    /* Column values */
    char *ApSN;
    char *ApName;
    char *ApModel;
    char *ApManufacturer;
    char *ApLocation;
    char *ApSoftwareVersion;
    char *ApHardwareVersion;
    u_long ApStartTime;
    u_long ApWorkTime;
    char *ApIPAddress;
    char *ApMACAddress;
    long ApRunState;
    long ApWorkModel;
    char *ApBindRadioID;
    char *ApBindWLANID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11ApProfileTable_entry *next;
};
void dot11ApProfileTable_load(void);
void dot11ApProfileTable_removeEntry( struct dot11ApProfileTable_entry *entry );
/** Initializes the dot11ApProfileTable module */
void
init_dot11ApProfileTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11ApProfileTable();
}

/** Initialize the dot11ApProfileTable table by defining its contents and how it's structured */
void
initialize_table_dot11ApProfileTable(void)
{
    static oid dot11ApProfileTable_oid[128] = {0};
    size_t dot11ApProfileTable_oid_len   = 0;
	mad_dev_oid(dot11ApProfileTable_oid,HIDDENAP,&dot11ApProfileTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11ApProfileTable",     dot11ApProfileTable_handler,
              dot11ApProfileTable_oid, dot11ApProfileTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalApID */
                           0);
    table_info->min_column = COLUMN_HIDDENAPMIN;
    table_info->max_column = COLUMN_HIDDENAPMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11ApProfileTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11ApProfileTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11ApProfileTable_load, dot11ApProfileTable_removeEntry,dot11ApProfileTable_oid, dot11ApProfileTable_oid_len));
    /* Initialise the contents of the table here */
}


struct dot11ApProfileTable_entry  *dot11ApProfileTable_head;

/* create a new row in the (unsorted) table */
struct dot11ApProfileTable_entry *
dot11ApProfileTable_createEntry(
						dbus_parameter parameter,
						long globalApID,
						long  ApID,
						char *ApSN,
						char *ApName,
						char *ApModel,
						char *ApManufacturer,
						char *ApLocation,
						char *ApSoftwareVersion,
						char *ApHardwareVersion,
						u_long ApStartTime,
						u_long ApWorkTime,
						char *ApIPAddress,
						char *ApMACAddress,
						long ApRunState,
						long ApWorkModel,
						char *ApBindRadioID,
						char *ApBindWLANID
                ) {
    struct dot11ApProfileTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11ApProfileTable_entry);
    if (!entry)
        return NULL;
    memcpy(&(entry->parameter), &parameter, sizeof(dbus_parameter));
    entry->globalApID = globalApID;
	entry->ApID = ApID;
	entry->ApSN = strdup(ApSN);
	entry->ApName = strdup(ApName);
	entry->ApModel = strdup(ApModel);
	entry->ApManufacturer = strdup(ApManufacturer);
	entry->ApLocation = strdup(ApLocation);
	entry->ApSoftwareVersion = strdup(ApSoftwareVersion);
	entry->ApHardwareVersion = strdup(ApHardwareVersion);
	entry->ApStartTime = ApStartTime;
	entry->ApWorkTime = ApWorkTime;
	entry->ApIPAddress = strdup(ApIPAddress);
	entry->ApMACAddress = strdup(ApMACAddress);
	entry->ApRunState = ApRunState;
	entry->ApWorkModel = ApWorkModel;
	entry->ApBindRadioID = strdup(ApBindRadioID);
	entry->ApBindWLANID = strdup(ApBindWLANID);
    entry->next = dot11ApProfileTable_head;
    dot11ApProfileTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11ApProfileTable_removeEntry( struct dot11ApProfileTable_entry *entry ) {
    struct dot11ApProfileTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11ApProfileTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11ApProfileTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->ApSN);
	FREE_OBJECT(entry->ApName);
	FREE_OBJECT(entry->ApModel);
	FREE_OBJECT(entry->ApManufacturer);
	FREE_OBJECT(entry->ApLocation);
	FREE_OBJECT(entry->ApSoftwareVersion);
	FREE_OBJECT(entry->ApHardwareVersion);
	FREE_OBJECT(entry->ApIPAddress);
	FREE_OBJECT(entry->ApMACAddress);
	FREE_OBJECT(entry->ApBindRadioID);
	FREE_OBJECT(entry->ApBindWLANID);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
void dot11ApProfileTable_load()
{	
	snmp_log(LOG_DEBUG, "enter dot11ApProfileTable_load\n");

	struct dot11ApProfileTable_entry *temp = NULL;
	
	while( dot11ApProfileTable_head ) {
    	temp=dot11ApProfileTable_head->next;
    	dot11ApProfileTable_removeEntry(dot11ApProfileTable_head);
    	dot11ApProfileTable_head=temp;
    }

	char temp_mac[20] = { 0 };
	memset(temp_mac,0,20);
	char wtp_name[255] = { 0 };
	char wtp_model[255] = { 0 };

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
    		    WID_WTP *q=NULL;
    		    int i=0;
    			for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q=q->next)
    			{
                    unsigned long globalApID = local_to_global_ID(messageNode->parameter,
                                                                  q->WTPID,
                                                                  WIRELESS_MAX_NUM);
                                                                  
    				memset(temp_mac,0,20);
					if(q->WTPMAC)
					{
						snprintf(temp_mac,sizeof(temp_mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
					}
					memset(wtp_name,0,sizeof(wtp_name));
					memset(wtp_model,0,sizeof(wtp_model));
					if(q->WTPNAME)
					{
						strncpy(wtp_name,q->WTPNAME,sizeof(wtp_name)-1);
					}
					if(q->WTPModel)
					{
						strncpy(wtp_model,q->WTPModel,sizeof(wtp_model)-1);
					}
				
    				dot11ApProfileTable_createEntry(messageNode->parameter,
    				                                globalApID,
    												q->WTPID,
    												"",
													wtp_name,
													wtp_model,
    												"",
    												"",
    												"",
    												"",
    												0,
    												0,
    												"0.0.0.0",
    												temp_mac,
    												q->WTPStat,
    												2,
    												"",
    												"");
					FREE_OBJECT(q->WTPMAC);
    			}
    		}
		}
        free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
	}
	
	snmp_log(LOG_DEBUG, "exit dot11ApProfileTable_load\n");
}
netsnmp_variable_list *
dot11ApProfileTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
		if(dot11ApProfileTable_head == NULL)
			{
				return NULL;
			}
    *my_loop_context = dot11ApProfileTable_head;
    return dot11ApProfileTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11ApProfileTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11ApProfileTable_entry *entry = (struct dot11ApProfileTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx,(u_char *)&entry->globalApID, sizeof(long));
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11ApProfileTable table */
int
dot11ApProfileTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11ApProfileTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
	case MODE_GET:
	{
		for (request=requests; request; request=request->next) 
		{
			table_entry = (struct dot11ApProfileTable_entry *)
			netsnmp_extract_iterator_context(request);
			table_info  =     netsnmp_extract_table_info(      request);
    if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}     

			switch (table_info->colnum) 
			{
				case COLUMN_APID:
				{
					snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
					(u_char *)&table_entry->globalApID,
					sizeof(long));
				}
				break;
				case COLUMN_APSN:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					int ret_one=0;
					DCLI_WTP_API_GROUP_ONE *wtp = NULL;
					char wtp_sn[50] = { 0 };
					memset(wtp_sn,0,50);
					
					ret_one=show_wtp_one(table_entry->parameter, connection,table_entry->ApID,&wtp);
					if((ret_one == 1)&&(wtp->WTP[0]))
					{
						memset(wtp_sn,0,50);
						if(wtp->WTP[0]->WTPSN)
						{
							memcpy(wtp_sn,wtp->WTP[0]->WTPSN,50);
						}
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
										(u_char*)wtp_sn,
										strlen(wtp_sn));
					
					if(ret_one==1)
					{
						Free_one_wtp_head(wtp);
					}
				}
				break;
				case COLUMN_APNAME:
				{
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
											(u_char *)table_entry->ApName,
											strlen(table_entry->ApName));
				}
				break;
				case COLUMN_APMODEL:
				{
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
											(u_char *)table_entry->ApModel,
											strlen(table_entry->ApModel));
				}
				break;
				case COLUMN_APMANUFACTURER:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					char ApManufacturer[DEFAULT_LEN] = { 0 };
					memset(ApManufacturer,0,DEFAULT_LEN);
					int ret_one=0;
					DCLI_WTP_API_GROUP_ONE *wtp = NULL;
					int ret_two = 0;
					DCLI_AC_API_GROUP_FOUR *codeinfo = NULL;
					wid_code_infomation *head = NULL;
					
					ret_one=show_wtp_one(table_entry->parameter, connection, table_entry->ApID,&wtp);
					if((ret_one == 1)&&(wtp->WTP[0]))
					{
						if(wtp->WTP[0]->WTPModel)
						{
							ret_two = show_ap_model_code_func(table_entry->parameter, connection, wtp->WTP[0]->WTPModel,&codeinfo);
							if(ret_two == 1)
							{
								if(codeinfo != NULL)
									head = codeinfo->code_info;
								if(head != NULL)
								{
									memset(ApManufacturer,0,DEFAULT_LEN);
									if(head->supplier)
									{
										strncpy(ApManufacturer,head->supplier,sizeof(ApManufacturer)-1);
									}
								}
							}
							if(ret_two == 1)
							{
								Free_ap_model_code(codeinfo);
							}
						}
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
									      (u_char*)ApManufacturer,
									      strlen(ApManufacturer));

					if(ret_one==1)
					{
						Free_one_wtp_head(wtp);
					}
				}
				break;
				case COLUMN_APLOCATION:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					char Location[50] = { 0 };
					memset(Location,0,50);
					int ret = 0;
					DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;   
					
					ret = show_wtp_location(table_entry->parameter, connection,table_entry->ApID,&WTPINFO);					
					if(ret == 1)
					{
						if(WTPINFO->wtp_location)
						{
							strncpy(Location,WTPINFO->wtp_location,sizeof(Location)-1);
						}
						else 
						{
							strncpy(Location,"UNKNOWN",sizeof(Location)-1);
						}
					}
					else 
					{
						strncpy(Location,"UNKNOWN",sizeof(Location)-1);
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
											(u_char*)Location,
											strlen(Location));
					if(ret == 1)
					{
						free_show_wtp_location(WTPINFO);
					}
				}
				break;
				case COLUMN_APSOFTWAREVERSION:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					char ApSoftwareVersion[DEFAULT_LEN] = { 0 };
					memset(ApSoftwareVersion,0,DEFAULT_LEN);
					int ret_one=0;
					DCLI_WTP_API_GROUP_ONE *wtp = NULL;
					
					ret_one=show_wtp_one(table_entry->parameter, connection, table_entry->ApID,&wtp);
					if((ret_one == 1)&&(wtp->WTP[0]))
					{
						if(wtp->WTP[0]->ver)
						{
							strncpy(ApSoftwareVersion,wtp->WTP[0]->ver,sizeof(ApSoftwareVersion)-1);
						}
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
											(u_char*)ApSoftwareVersion,
											strlen(ApSoftwareVersion));

					if(ret_one==1)
					{
						Free_one_wtp_head(wtp);
					}
				}
				break;
				case COLUMN_APHARDWAREVERSION:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					char ApHardwareVersion[DEFAULT_LEN] = { 0 };
					memset(ApHardwareVersion,0,DEFAULT_LEN);
					int ret_one=0;
					DCLI_WTP_API_GROUP_ONE *wtp = NULL;
					
					ret_one=show_wtp_one(table_entry->parameter, connection,table_entry->ApID,&wtp);
					if((ret_one == 1)&&(wtp->WTP[0]))
					{
						if(wtp->WTP[0]->sysver)
						{
							strncpy(ApHardwareVersion,wtp->WTP[0]->sysver,sizeof(ApHardwareVersion)-1);
						}
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
											(u_char*)ApHardwareVersion,
											strlen(ApHardwareVersion));

					if(ret_one==1)
					{
						Free_one_wtp_head(wtp);
					}
				}
				break;
				case COLUMN_APSTARTTIME:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					int ret = 0;
					int time_run = 0;
					DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;
					
					ret=show_wtp_runtime(table_entry->parameter, connection,table_entry->ApID,&WTPINFO);
					if(ret == 1)
					{
						if((WTPINFO->addtime - WTPINFO->ElectrifyRegisterCircle)== 0)
						{
							time_run = 0;
						}
						else
						{
							time_t now,online_time;
							time(&now);							
							online_time = now - (WTPINFO->addtime - WTPINFO->ElectrifyRegisterCircle);							
						    time_run = online_time*100;
						}					
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_TIMETICKS,
											(u_char*)&time_run,
											sizeof(long));
					if(ret == 1)
					{
						 free_show_wtp_runtime(WTPINFO);					
					}
				}
				break;
				case COLUMN_APWORKTIME:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					int ret = 0;
					int time_run = 0;
					DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;
					
					ret=show_wtp_runtime(table_entry->parameter, connection, table_entry->ApID,&WTPINFO);
					if(ret == 1)
					{
						if(WTPINFO->addtime == 0)
						{
							time_run = 0;
						}
						else
						{
							time_t now,online_time;
							time(&now);							
							online_time = now - (WTPINFO->addtime);							
						    time_run = online_time*100;
						}					
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_TIMETICKS,
											(u_char*)&time_run,
											sizeof(long));
					if(ret == 1)
					{
						 free_show_wtp_runtime(WTPINFO);					
					}
				}
				break;
				case COLUMN_APIPADDRESS:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					char ip_add[20] = { 0 };
					char *pos=NULL;
					int ret_one=0;
					DCLI_WTP_API_GROUP_ONE *wtp = NULL;

					ret_one=show_wtp_one(table_entry->parameter, connection,table_entry->ApID,&wtp);
					if((ret_one == 1)&&(wtp->WTP[0]))
					{
						memset(ip_add,0,20);
						if(wtp->WTP[0]->WTPIP)
						{
							if(!strcmp(wtp->WTP[0]->WTPIP,""))
							{
								strncpy(ip_add,"0.0.0.0",sizeof(ip_add)-1);
							}
							else
							{
								pos = strchr(wtp->WTP[0]->WTPIP,':');
								if(pos)
								{
									strncpy( ip_add, wtp->WTP[0]->WTPIP, pos - wtp->WTP[0]->WTPIP);
								}
							}
						}
					}
					else
					{
						memset(ip_add,0,20);
						strncpy(ip_add,"0.0.0.0",sizeof(ip_add)-1);
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
											(u_char *)ip_add,
											strlen(ip_add));

					if(ret_one==1)
					{
						Free_one_wtp_head(wtp);
					}
				}
				break;
				case COLUMN_APMACADDRESS:
				{
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
											(u_char *)table_entry->ApMACAddress,
											strlen(table_entry->ApMACAddress));
				}
				break;
				case COLUMN_APRUNSTATE:
				{
					snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
											(u_char *)&table_entry->ApRunState,
											sizeof(table_entry->ApRunState));
				}
				break;
				case COLUMN_APWORKMODEL:
				{
					snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
											(u_char *)&table_entry->ApWorkModel,
											sizeof(table_entry->ApWorkModel));
				}
				break;
				case COLUMN_APBINDRADIOID:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					int ret_one=0;
					DCLI_WTP_API_GROUP_ONE *wtp = NULL;
					int radio_num=0;
					char wtp_radio_list[50] = { 0 };
					memset(wtp_radio_list,0,50);
					int m = 0;
					char temp[5] = { 0 };
					memset(temp,0,5);
					
					ret_one=show_wtp_one(table_entry->parameter, connection,table_entry->ApID,&wtp);
					if((ret_one == 1)&&(wtp->WTP[0]))
					{
						radio_num = wtp->WTP[0]->radio_num;
						for(m = 0;m<radio_num;m++)
						{
							memset(temp,0,5);
							if(wtp->WTP[0]->WTP_Radio[m])
							{
								snprintf(temp,sizeof(temp)-1,"%d",wtp->WTP[0]->WTP_Radio[m]->Radio_G_ID);
							}
							strncat(wtp_radio_list,temp,sizeof(wtp_radio_list)-strlen(wtp_radio_list)-1);
							if(m!=radio_num-1)
							{
								strncat(wtp_radio_list,",",sizeof(wtp_radio_list)-strlen(wtp_radio_list)-1);
							}
						}
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
										(u_char*)wtp_radio_list,
										strlen(wtp_radio_list));
					
					if(ret_one==1)
					{
						Free_one_wtp_head(wtp);
					}
				}
				break;
				case COLUMN_APBINDWLANID:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                        break;
                        
					int ret_one=0;
					DCLI_WTP_API_GROUP_ONE *wtp = NULL;
					int bindedwlannum=0;
					char wtp_wlan_list[50] = { 0 };
					memset(wtp_wlan_list,0,50);
					int m = 0;
					char temp[5] = { 0 };
					memset(temp,0,5);
					
					ret_one=show_wtp_one(table_entry->parameter, connection,table_entry->ApID,&wtp);
					if((ret_one == 1)&&(wtp->WTP[0]))
					{
						bindedwlannum = wtp->WTP[0]->apply_wlan_num;
						for(m = 0;m<bindedwlannum;m++)
						{
							memset(temp,0,5);
							snprintf(temp,sizeof(temp)-1,"%d",wtp->WTP[0]->apply_wlanid[m]);
							strncat(wtp_wlan_list,temp,sizeof(wtp_wlan_list)-strlen(wtp_wlan_list)-1);
							if(m!=bindedwlannum-1)
							{
								strncat(wtp_wlan_list,",",sizeof(wtp_wlan_list)-strlen(wtp_wlan_list)-1);
							}
						}
					}
					
					snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
										(u_char*)wtp_wlan_list,
										strlen(wtp_wlan_list));
					
					if(ret_one==1)
					{
						Free_one_wtp_head(wtp);
					}
				}
				break;
			}
		}
	}
        break;

    }
    return SNMP_ERR_NOERROR;
}
