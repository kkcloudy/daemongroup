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
* dot11WtpBssTable.c
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
#include "dot11WtpBssTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define WTPBSSTABLE "1.7.2"
    /* Typical data structure for a row entry */
struct dot11WtpBssTable_entry {
    /* Index values */
	dbus_parameter parameter;
    long wtpCurrID;
	long BSSindex;
	long WlanID;
	char *wtpMacAddr;

    char *wtpBssCurrID;

    /* Column values */
    long wtpDstBssidCurConfMaxConcurUser;
   // long old_wtpDstBssidCurConfMaxConcurUser;
    long wtpDstBssidCurConfMaxRate;
   // long old_wtpDstBssidCurConfMaxRate;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WtpBssTable_entry *next;
};

void dot11WtpBssTable_load();
void
dot11WtpBssTable_removeEntry( struct dot11WtpBssTable_entry *entry );

/** Initializes the dot11WtpBssTable module */
void
init_dot11WtpBssTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WtpBssTable();
}

/** Initialize the dot11WtpBssTable table by defining its contents and how it's structured */
void
initialize_table_dot11WtpBssTable(void)
{
    static oid dot11WtpBssTable_oid[128] = {0};
	size_t dot11WtpBssTable_oid_len   = 0;
	mad_dev_oid(dot11WtpBssTable_oid,WTPBSSTABLE,&dot11WtpBssTable_oid_len,enterprise_pvivate_oid);
    
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WtpBssTable",     dot11WtpBssTable_handler,
              dot11WtpBssTable_oid, dot11WtpBssTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: wtpCurrID */
                           ASN_OCTET_STR,  /* index: wtpBssCurrID */
                           0);
    table_info->min_column = WTPBSSTABLE_MIN_COLUMN;
    table_info->max_column = WTPBSSTABLE_MAX_COLUMN;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WtpBssTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WtpBssTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WtpBssTable_load, dot11WtpBssTable_removeEntry,
							dot11WtpBssTable_oid, dot11WtpBssTable_oid_len));

    /* Initialise the contents of the table here */
}


struct dot11WtpBssTable_entry  *dot11WtpBssTable_head;

/* create a new row in the (unsorted) table */
struct dot11WtpBssTable_entry *
dot11WtpBssTable_createEntry(
				dbus_parameter parameter,
				long wtpCurrID,
				long BSSindex,
				long WlanID,
				char *wtpMacAddr,
                 char  *wtpBssCurrID,
				long wtpDstBssidCurConfMaxConcurUser,
				long wtpDstBssidCurConfMaxRate
                ) {
    struct dot11WtpBssTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WtpBssTable_entry);
    if (!entry)
        return NULL;
	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->wtpCurrID = wtpCurrID;
	entry->BSSindex = BSSindex;
	entry->WlanID = WlanID;
	entry->wtpMacAddr   = strdup(wtpMacAddr); 
    entry->wtpBssCurrID = strdup(wtpBssCurrID);
	 entry->wtpDstBssidCurConfMaxConcurUser = wtpDstBssidCurConfMaxConcurUser;
	 entry->wtpDstBssidCurConfMaxRate = wtpDstBssidCurConfMaxRate;
	 entry->valid = 1;
    entry->next = dot11WtpBssTable_head;
    dot11WtpBssTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WtpBssTable_removeEntry( struct dot11WtpBssTable_entry *entry ) {
    struct dot11WtpBssTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WtpBssTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WtpBssTable_head = ptr->next;
    else
        prev->next = ptr->next;
	
	FREE_OBJECT(entry->wtpMacAddr);
    FREE_OBJECT(entry->wtpBssCurrID);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11WtpBssTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11WtpBssTable_load\n");

	struct dot11WtpBssTable_entry *temp = NULL;

	while( dot11WtpBssTable_head ) {
		temp=dot11WtpBssTable_head->next;
		dot11WtpBssTable_removeEntry(dot11WtpBssTable_head);
		dot11WtpBssTable_head=temp;
	}
	
	char temp_mac[20] = { 0 };
    char bssid[20] = {0};
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
                        
		        int i  = 0;
		        WID_WTP *q = NULL;
			    for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q=q->next)
				{
					memset(temp_mac,0,20);
					if(q->WTPMAC)
					{
						snprintf(temp_mac,sizeof(temp_mac)-1,
								 "%02X:%02X:%02X:%02X:%02X:%02X",
								 q->WTPMAC[0],q->WTPMAC[1],
								 q->WTPMAC[2],q->WTPMAC[3],
								 q->WTPMAC[4],q->WTPMAC[5]);
					}
					
                    int k = 0;
					for(k=0;k<q->RadioCount;k++)
					{
						int radio_id = q->WFR_Index + k;
						DCLI_RADIO_API_GROUP_ONE *radio = NULL;
                        DCLI_RADIO_API_GROUP_ONE *RADIOINFO = NULL;
						int retu = 0, retu2 = 0;
						retu =show_radio_bss_max_throughput(messageNode->parameter, connection,radio_id,&radio);
                        retu2 = show_radio_one(messageNode->parameter, connection,radio_id,&RADIOINFO);
						if(retu == 1)
						{       
						    int j = 0;
							for(j=0;j<radio->bss_num_int;j++)
							{
								memset(bssid,0,20);
								if((radio->WTP[0])&&(radio->WTP[0]->WTP_Radio[0])&&(radio->WTP[0]->WTP_Radio[0]->BSS[j]))
								{
									if((radio->WTP[0]->WTP_Radio[0]->BSS[j]->BSSID))
									{
										snprintf(bssid,sizeof(bssid)-1,
												 "%02X:%02X:%02X:%02X:%02X:%02X",
												 radio->WTP[0]->WTP_Radio[0]->BSS[j]->BSSID[0],
												 radio->WTP[0]->WTP_Radio[0]->BSS[j]->BSSID[1],
												 radio->WTP[0]->WTP_Radio[0]->BSS[j]->BSSID[2],
												 radio->WTP[0]->WTP_Radio[0]->BSS[j]->BSSID[3],
												 radio->WTP[0]->WTP_Radio[0]->BSS[j]->BSSID[4],
												 radio->WTP[0]->WTP_Radio[0]->BSS[j]->BSSID[5]);
									}
									
									unsigned int wlanid = 0;
									if((1 == retu2)&&(RADIOINFO->RADIO[0])&&(RADIOINFO->RADIO[0]->BSS[j]))
									{
										wlanid = RADIOINFO->RADIO[0]->BSS[j]->WlanID;
									}
										
									dot11WtpBssTable_createEntry(messageNode->parameter,
																 q->WTPID,
																 j+1,
																 wlanid,
																 temp_mac,
																 bssid,
																 radio->WTP[0]->WTP_Radio[0]->BSS[j]->bss_max_allowed_sta_num,
																 radio->WTP[0]->WTP_Radio[0]->BSS[j]->band_width);
								}
						
							}
							Free_radio_bss_max_throughput_head(radio);
						}				
						else if(SNMPD_CONNECTION_ERROR == retu) {
                            close_slot_dbus_connection(messageNode->parameter.slot_id);
                            break;
                	    }

                        if(retu2 == 1){
                             Free_radio_one_head(RADIOINFO);
                        }
                    }
					FREE_OBJECT(q->WTPMAC);
				}
			}
		}
        free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
	}

	snmp_log(LOG_DEBUG, "exit dot11WtpBssTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WtpBssTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11WtpBssTable_head==NULL)
		return NULL;
	*my_loop_context = dot11WtpBssTable_head;
	*my_data_context = dot11WtpBssTable_head;
	return dot11WtpBssTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WtpBssTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WtpBssTable_entry *entry = (struct dot11WtpBssTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char *)entry->wtpMacAddr, strlen(entry->wtpMacAddr) );
        idx = idx->next_variable;
        snmp_set_var_value( idx, (u_char*)entry->wtpBssCurrID, strlen(entry->wtpBssCurrID) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WtpBssTable table */
int
dot11WtpBssTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WtpBssTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpBssTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
			
 	   if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
 	   	}
            switch (table_info->colnum) {
            case COLUMN_WTPBSSCURRID:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->wtpBssCurrID,
                                          strlen(table_entry->wtpBssCurrID));
                break;
            case COLUMN_WTPDSTBSSIDCURCONFMAXCONCURUSER:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->wtpDstBssidCurConfMaxConcurUser,
                                          sizeof(long));
                break;
            case COLUMN_WTPDSTBSSIDCURCONFMAXRATE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->wtpDstBssidCurConfMaxRate,
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
            table_entry = (struct dot11WtpBssTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WTPDSTBSSIDCURCONFMAXCONCURUSER:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WTPDSTBSSIDCURCONFMAXRATE:
                if ( request->requestvb->type != ASN_INTEGER ) {
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
            table_entry = (struct dot11WtpBssTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

	 	   if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
 	   		}
            switch (table_info->colnum) {
            case COLUMN_WTPDSTBSSIDCURCONFMAXCONCURUSER:
            {
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3)) {
                    netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
                    break;
                }
                
			    int ret=0;
			    int radioid = 0;
			    char Wlan_id[10] = { 0 };
			    char bssnum[20] = { 0 };			    
			    
			    radioid = (table_entry->wtpCurrID)*4;
			    memset(Wlan_id,0,10);
			    snprintf(Wlan_id,sizeof(Wlan_id)-1,"%d",table_entry->WlanID);
			    memset(bssnum,0,20);
			    snprintf(bssnum,sizeof(bssnum)-1,"%d",*request->requestvb->val.integer);

           	    ret=set_bss_max_sta_num(table_entry->parameter,connection,radioid,Wlan_id,bssnum);
			    if(ret==1)
			    {
			    	table_entry->wtpDstBssidCurConfMaxConcurUser = *request->requestvb->val.integer;
			    }
				else
			    {	
			        if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(table_entry->parameter.slot_id);
            	    }
			        netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
			    }
        	}
	            break;
            case COLUMN_WTPDSTBSSIDCURCONFMAXRATE:
           	{
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3)) {
                    netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
                    break;
                }
                
				int ret=0;
				int radioid = 0;
				char Wlan_id[10] = { 0 };
				char bssnum[20] = { 0 };
				 
				radioid = (table_entry->wtpCurrID)*4;
			  	memset(Wlan_id,0,10);
			  	snprintf(Wlan_id,sizeof(Wlan_id)-1,"%d",table_entry->WlanID);
			  	memset(bssnum,0,20);
				snprintf(bssnum,sizeof(bssnum)-1,"%d",*request->requestvb->val.integer);
				
				ret=set_bss_max_throughput(table_entry->parameter,connection,radioid, Wlan_id,bssnum);
				if(ret==1)
				{
					table_entry->wtpDstBssidCurConfMaxRate = *request->requestvb->val.integer;
				}
				else
			    {	
			        if(SNMPD_CONNECTION_ERROR == ret) {
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
            table_entry = (struct dot11WtpBssTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WTPDSTBSSIDCURCONFMAXCONCURUSER:
                /* Need to restore old 'table_entry->wtpDstBssidCurConfMaxConcurUser' value.
                   May need to use 'memcpy' */
              //  table_entry->wtpDstBssidCurConfMaxConcurUser = table_entry->old_wtpDstBssidCurConfMaxConcurUser;
                break;
            case COLUMN_WTPDSTBSSIDCURCONFMAXRATE:
                /* Need to restore old 'table_entry->wtpDstBssidCurConfMaxRate' value.
                   May need to use 'memcpy' */
             //   table_entry->wtpDstBssidCurConfMaxRate = table_entry->old_wtpDstBssidCurConfMaxRate;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
