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
* dot11WtpWirelessCapStatTable.c
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
#include "dot11WtpWirelessCapStatTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "ws_init_dbus.h"
#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "autelanWtpGroup.h"
#include "ws_snmpd_engine.h"
#include "ws_sta.h"
#include "ws_dbus_list_interface.h"

#define WTPCAPSTATTABLE "1.3.7"

#define __DEBUG	1


struct dot11WtpWirelessCapStatTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long wtpCurrID;
    char *wtpMacAddr;
    long wtpWirelessIfIndex;

    /* Column values */
    char *AvgRxSignalStrength;
    char *HighestRxSignalStrength;
    char *LowestRxSignalStrength;
    u_long ChstatsPhyerrPkts;
    u_long ChstatsFrameRetryCnt;
    u_long ChstatsFrameErrorCnt;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WtpWirelessCapStatTable_entry *next;
};

void dot11WtpWirelessCapStatTable_load(void);
void dot11WtpWirelessCapStatTable_removeEntry( struct dot11WtpWirelessCapStatTable_entry *entry );


/** Initializes the dot11WtpWirelessCapStatTable module */
void
init_dot11WtpWirelessCapStatTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WtpWirelessCapStatTable();
}

/** Initialize the dot11WtpWirelessCapStatTable table by defining its contents and how it's structured */
void
initialize_table_dot11WtpWirelessCapStatTable(void)
{
    static oid dot11WtpWirelessCapStatTable_oid[128] = {0};
    size_t dot11WtpWirelessCapStatTable_oid_len   = 0;	
	mad_dev_oid(dot11WtpWirelessCapStatTable_oid,WTPCAPSTATTABLE,&dot11WtpWirelessCapStatTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WtpWirelessCapStatTable",     dot11WtpWirelessCapStatTable_handler,
              dot11WtpWirelessCapStatTable_oid, dot11WtpWirelessCapStatTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: wtpMacAddr */
                           ASN_INTEGER,  /* index: wtpWirelessIfIndex */
                           0);
    table_info->min_column = WTPCAPSTATTABLE_MIN_COLUMN;
    table_info->max_column = WTPCAPSTATTABLE_MAX_COLUMN;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WtpWirelessCapStatTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WtpWirelessCapStatTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WtpWirelessCapStatTable_load, dot11WtpWirelessCapStatTable_removeEntry,dot11WtpWirelessCapStatTable_oid, dot11WtpWirelessCapStatTable_oid_len));
    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11WtpWirelessCapStatTable_entry  *dot11WtpWirelessCapStatTable_head;

/* create a new row in the (unsorted) table */
struct dot11WtpWirelessCapStatTable_entry *
dot11WtpWirelessCapStatTable_createEntry(
				dbus_parameter parameter,
                long  wtpCurrID,
                char *wtpMacAddr,
                long  wtpWirelessIfIndex,
                char * AvgRxSignalStrength,
				char * HighestRxSignalStrength,
				char * LowestRxSignalStrength,
				u_long ChstatsPhyerrPkts,
				u_long ChstatsFrameRetryCnt,
				u_long ChstatsFrameErrorCnt
                ) {
    struct dot11WtpWirelessCapStatTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WtpWirelessCapStatTable_entry);
    if (!entry)
        return NULL;
    
	memset( entry, 0, sizeof(struct dot11WtpWirelessCapStatTable_entry) );
	memcpy(&(entry->parameter), &parameter, sizeof(dbus_parameter));
	entry->wtpCurrID = wtpCurrID;
    entry->wtpMacAddr = strdup(wtpMacAddr);
    entry->wtpWirelessIfIndex = wtpWirelessIfIndex;
	entry->AvgRxSignalStrength = strdup(AvgRxSignalStrength);
	entry->HighestRxSignalStrength = strdup(HighestRxSignalStrength);
	entry->LowestRxSignalStrength = strdup(LowestRxSignalStrength);
	entry->ChstatsPhyerrPkts = ChstatsPhyerrPkts;
	entry->ChstatsFrameRetryCnt = ChstatsFrameRetryCnt;
	entry->ChstatsFrameErrorCnt = ChstatsFrameErrorCnt;
    entry->next = dot11WtpWirelessCapStatTable_head;
    dot11WtpWirelessCapStatTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WtpWirelessCapStatTable_removeEntry( struct dot11WtpWirelessCapStatTable_entry *entry ) {
    struct dot11WtpWirelessCapStatTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WtpWirelessCapStatTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WtpWirelessCapStatTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->wtpMacAddr);
	FREE_OBJECT(entry->AvgRxSignalStrength);
	FREE_OBJECT(entry->HighestRxSignalStrength);
	FREE_OBJECT(entry->LowestRxSignalStrength);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}


void dot11WtpWirelessCapStatTable_load()
{
    snmp_log(LOG_DEBUG, "enter dot11WtpWirelessCapStatTable_load\n");

    struct dot11WtpWirelessCapStatTable_entry *temp = NULL; 
	while( dot11WtpWirelessCapStatTable_head ) {
    	temp=dot11WtpWirelessCapStatTable_head->next;
    	dot11WtpWirelessCapStatTable_removeEntry(dot11WtpWirelessCapStatTable_head);
    	dot11WtpWirelessCapStatTable_head=temp;
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
                    memset(temp_mac,0,20);
					if(q->WTPMAC)
					{
						snprintf(temp_mac,sizeof(temp_mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);	
					}
                    
                    int result2 = 0;
                    DCLI_WTP_API_GROUP_TWO *WTPINFO = NULL; 
                    result2 = show_ap_if_info_func(messageNode->parameter, connection,q->WTPID,&WTPINFO);
                    if(result2 == 1 && WTPINFO)
                    {
                        int j = 0;
						if(WTPINFO->WTP[0])
						{
							for(j=0;j<WTPINFO->WTP[0]->apifinfo.wifi_num && j<WTPINFO->WTP[0]->RadioCount;j++)
							{
								dot11WtpWirelessCapStatTable_createEntry(messageNode->parameter,
																		q->WTPID,
																		temp_mac,
																		j+1,
																		"",
																		"", 				
																		"", 													
																		0,
																		0,
																		0); 
							}
                    	}
                    	free_show_ap_if_info(WTPINFO);
                    }
                    else if(SNMPD_CONNECTION_ERROR == result2) {
    				    close_slot_dbus_connection(messageNode->parameter.slot_id);
    				}
					FREE_OBJECT(q->WTPMAC);
        		}
        	}
        }	
		free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
    }

    snmp_log(LOG_DEBUG, "exit dot11WtpWirelessCapStatTable_load\n");
}
/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WtpWirelessCapStatTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{   
    
if(dot11WtpWirelessCapStatTable_head == NULL)
		{
			return NULL;
		}
    *my_loop_context = dot11WtpWirelessCapStatTable_head;
    return dot11WtpWirelessCapStatTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WtpWirelessCapStatTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{   
    struct dot11WtpWirelessCapStatTable_entry *entry = (struct dot11WtpWirelessCapStatTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)entry->wtpMacAddr, strlen(entry->wtpMacAddr));
        idx = idx->next_variable;
        snmp_set_var_value( idx, (u_char*)&entry->wtpWirelessIfIndex, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
		
    } else {
        return NULL;
    }
	
	return put_index_data; 
}


/** handles requests for the dot11WtpWirelessCapStatTable table */
int
dot11WtpWirelessCapStatTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {


    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WtpWirelessCapStatTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
			
            table_entry = (struct dot11WtpWirelessCapStatTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
           
			if( !table_entry )
			{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}	  
            switch (table_info->colnum) {
            case COLUMN_AVGRXSIGNALSTRENGTH:
			{   
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret = 0;
				int AvgRxSignalStrength = 0;
				DCLI_WTP_API_GROUP_TWO *WTPINFO;
				char value[256] = { 0 };
				memset(value,0,256);
				char snr_avg[256] = { 0 };
				memset(snr_avg,0,256);
				strncpy(value,"-",sizeof(value)-1);	
				ret=show_wtp_wifi_snr_func(table_entry->parameter, connection,table_entry->wtpCurrID,&WTPINFO);
				if(ret==1)
				{
					if((WTPINFO)&&(WTPINFO->WTP[0]))
					{
						AvgRxSignalStrength = WTPINFO->WTP[0]->wtp_wifi_snr_stats.snr_average;
					}
					snprintf(snr_avg,sizeof(snr_avg)-1,"%d",AvgRxSignalStrength);
					strncat(value,snr_avg,sizeof(value)-strlen(value)-1);
					strncat(value,"dB",sizeof(value)-strlen(value)-1);
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
    	        }
    	        
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)value,
                                          strlen(value));
				if(ret==1)
				{
					free_show_wtp_wifi_snr(WTPINFO);
				}
            }
                break;
            case COLUMN_HIGHESTRXSIGNALSTRENGTH:
			{
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret = 0;
				int HighestRxSignalStrength = 0;
				DCLI_WTP_API_GROUP_TWO *WTPINFO;
				char value[256] = { 0 };
				memset(value,0,256);
				char snr_max[256] = { 0 };
				memset(snr_max,0,256);
				strncpy(value,"-",sizeof(value)-1);				
				ret=show_wtp_wifi_snr_func(table_entry->parameter, connection,table_entry->wtpCurrID,&WTPINFO);
				if(ret==1)
				{
					if((WTPINFO)&&(WTPINFO->WTP[0]))
					{
						HighestRxSignalStrength = WTPINFO->WTP[0]->wtp_wifi_snr_stats.snr_max;
					}
					snprintf(snr_max,sizeof(snr_max)-1,"%d",HighestRxSignalStrength);
					strncat(value,snr_max,sizeof(value)-strlen(value)-1);
					strncat(value,"dB",sizeof(value)-strlen(value)-1);
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
    	        }
    	        
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)value,
                                          strlen(value));
				if(ret==1)
				{
					free_show_wtp_wifi_snr(WTPINFO);
				}
            }
                break;
            case COLUMN_LOWESTRXSIGNALSTRENGTH:
			{
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret = 0;
				int LowestRxSignalStrength=0;
				DCLI_WTP_API_GROUP_TWO *WTPINFO;
                char value[256] = { 0 };
				memset(value,0,256);
				char snr_min[256] = { 0 };
				memset(snr_min,0,256);
				strncpy(value,"-",sizeof(value)-1);		
				ret=show_wtp_wifi_snr_func(table_entry->parameter, connection,table_entry->wtpCurrID,&WTPINFO);
				if(ret==1)
				{
					if((WTPINFO)&&(WTPINFO->WTP[0]))
					{
						if((WTPINFO->WTP[0]->wtp_wifi_snr_stats.snr_min) == 100)
						{
							LowestRxSignalStrength=0;
						}
						else
						{
							LowestRxSignalStrength	=WTPINFO->WTP[0]->wtp_wifi_snr_stats.snr_min;
						}					
					}
				
					snprintf(snr_min,sizeof(snr_min)-1,"%d",LowestRxSignalStrength);
					strncat(value,snr_min,sizeof(value)-strlen(value)-1);
					strncat(value,"dB",sizeof(value)-strlen(value)-1);
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
    	        }
    	        
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)value,
                                          strlen(value));
				if(ret==1)
				{
					free_show_wtp_wifi_snr(WTPINFO);
				}
            }
                break;
            case COLUMN_CHSTATSPHYERRPKTS:
			{
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
			    int ret = 0;
				unsigned int PhyErrors = 0; 
				DCLI_AC_API_GROUP_THREE *statics = NULL;
				wlan_stats_info *head = NULL;

				ret=show_ap_statistics_list_bywtp(table_entry->parameter, connection,table_entry->wtpCurrID,&statics);
				if(ret==1)
				{
					if((statics)&&(statics->ap_statics_list)&&(statics->ap_statics_list->ap_statics_ele))
					{						
						head = statics->ap_statics_list->ap_statics_ele;
						while(head)
						{	
							if((head->type == 2)&&(head->wlanId == (table_entry->wtpWirelessIfIndex-1)))
							{
								PhyErrors = head->ast_rx_phyerr; 
								break;
							}
							head = head->next;
						}
					}
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
    	        }
    	        
				table_entry->ChstatsPhyerrPkts = PhyErrors;
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
                                          (u_char*)&table_entry->ChstatsPhyerrPkts,
                                          sizeof(long));
				     if(ret==1)
					{
						Free_ap_statistics_head(statics);
					}
			}
                break;
            case COLUMN_CHSTATSFRAMERETRYCNT:
			{
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int ret =  0;
				DCLI_WTP_API_GROUP_TWO *WTPINFO = NULL;
				ret  = show_wtp_extension_information_v4_func(table_entry->parameter, connection,table_entry->wtpCurrID,&WTPINFO);
				if((ret == 1)&&(WTPINFO)&&(WTPINFO->WTP[0]))
				{ 
				  table_entry->ChstatsFrameRetryCnt = WTPINFO->WTP[0]->wifi_extension_info.tx_retry;
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
    	        }
    	        
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
                                      (u_char*)&table_entry->ChstatsFrameRetryCnt,
                                      sizeof(long));
				if(ret == 1)
				{ 
				  free_show_wtp_extension_information_v4(WTPINFO);
				}
            }
                break;
            case COLUMN_CHSTATSFRAMEERRORCNT:
			{
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
			    int ret = 0;
				unsigned int RxErrors = 0; 
				DCLI_AC_API_GROUP_THREE *statics = NULL;
				wlan_stats_info *head = NULL;
				
				ret=show_ap_statistics_list_bywtp(table_entry->parameter, connection,table_entry->wtpCurrID,&statics);
				if(ret==1)
				{
					if((statics)&&(statics->ap_statics_list)&&(statics->ap_statics_list->ap_statics_ele))
					{						
						head = statics->ap_statics_list->ap_statics_ele;
						while(head)
						{	
							if((head->type == 2)&&(head->wlanId == (table_entry->wtpWirelessIfIndex-1)))
							{
								RxErrors = head->rx_errors; 
								break;
							}
							head = head->next;
						}
					}
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
    	        }
    	        
					
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
                                          (u_char*)&table_entry->ChstatsFrameErrorCnt,
                                          sizeof(long));
			    if(ret==1)
				{
					Free_ap_statistics_head(statics);
				}
			}
                break;
            }
        }
        break;

    }
    return SNMP_ERR_NOERROR;
}
