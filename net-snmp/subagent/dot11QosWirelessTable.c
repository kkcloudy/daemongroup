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
* dot11QosWirelessTable.c
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
#include "dot11QosWirelessTable.h"
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
#include "ws_dbus_list_interface.h"
//#include "autelanWtpGroup.h"

#define QOSWIRELESSTABLE "3.4.1"

#define __DEBUG	1


struct dot11QosWirelessTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalWtpID;
    long WtpID;
    long RadioLocalID;
    long QosType;

    /* Column values */
    long QosWirelessAIFS;
    long QosWirelessCWmin;
    long QoSWirelessCWmax;
    long QoSWirelessTXOPLim;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11QosWirelessTable_entry *next;
};

void dot11QosWirelessTable_load(void);
void dot11QosWirelessTable_removeEntry( struct dot11QosWirelessTable_entry *entry );

/** Initializes the dot11QosWirelessTable module */
void
init_dot11QosWirelessTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11QosWirelessTable();
}

/** Initialize the dot11QosWirelessTable table by defining its contents and how it's structured */
void
initialize_table_dot11QosWirelessTable(void)
{
    static oid dot11QosWirelessTable_oid[128] = {0};
    size_t dot11QosWirelessTable_oid_len   = 0;	
	mad_dev_oid(dot11QosWirelessTable_oid,QOSWIRELESSTABLE,&dot11QosWirelessTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11QosWirelessTable",     dot11QosWirelessTable_handler,
              dot11QosWirelessTable_oid, dot11QosWirelessTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalWtpID */
                           ASN_INTEGER,  /* index: RadioLocalID */
                           ASN_INTEGER,  /* index: QosType */
                           0);
    table_info->min_column = QOSWIRELESSMIN;
    table_info->max_column = QOSWIRELESSMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11QosWirelessTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11QosWirelessTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
    
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11QosWirelessTable_load, dot11QosWirelessTable_removeEntry,dot11QosWirelessTable_oid, dot11QosWirelessTable_oid_len));
    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11QosWirelessTable_entry  *dot11QosWirelessTable_head = NULL;

/* create a new row in the (unsorted) table */
struct dot11QosWirelessTable_entry *
dot11QosWirelessTable_createEntry(
				dbus_parameter parameter,
				long globalWtpID,
                long  WtpID,
                long  RadioLocalID,
                long  QosType,
			    long QosWirelessAIFS,
			    long QosWirelessCWmin,
			    long QoSWirelessCWmax,
			    long QoSWirelessTXOPLim
                ) {
    struct dot11QosWirelessTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11QosWirelessTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->globalWtpID = globalWtpID;
    entry->WtpID = WtpID;
    entry->RadioLocalID = RadioLocalID;
    entry->QosType = QosType;
	entry->QosWirelessAIFS = QosWirelessAIFS;
	entry->QosWirelessCWmin = QosWirelessCWmin;
	entry->QoSWirelessCWmax = QoSWirelessCWmax;
	entry->QoSWirelessTXOPLim = QoSWirelessTXOPLim;
    entry->next = dot11QosWirelessTable_head;
    dot11QosWirelessTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11QosWirelessTable_removeEntry( struct dot11QosWirelessTable_entry *entry ) {
    struct dot11QosWirelessTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11QosWirelessTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11QosWirelessTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11QosWirelessTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11QosWirelessTable_load\n");

	struct dot11QosWirelessTable_entry *temp = NULL; 
	while( dot11QosWirelessTable_head ){
		temp=dot11QosWirelessTable_head->next;
		dot11QosWirelessTable_removeEntry(dot11QosWirelessTable_head);
		dot11QosWirelessTable_head=temp;
	}
	
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
    		    int i = 0;
    		    WID_WTP *q = NULL;
    			for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q=q->next)
    			{
    			    
                    unsigned long globalWtpID = local_to_global_ID(messageNode->parameter, 
                                                                   q->WTPID, 
                                                                   WIRELESS_MAX_NUM);

    			    int j = 0;
    				for(j=0;j<q->RadioCount;j++)
    				{
    				    int k = 1;
    					for(k=1;k<5;k++)
    					{
    						dot11QosWirelessTable_createEntry(messageNode->parameter,
    						                                  globalWtpID,
    														  q->WTPID,
    														  j,
    														  k,
    														  0,
    														  0,
    														  0,
    														  0); 
    					}
    				}
					FREE_OBJECT(q->WTPMAC);
    			}
    		}
    	}	
    	free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);	
	}

	snmp_log(LOG_DEBUG, "exit dot11QosWirelessTable_load\n");
}		


/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11QosWirelessTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{   
	if(dot11QosWirelessTable_head == NULL)
		{
			return NULL;
		}
    *my_loop_context = dot11QosWirelessTable_head;
    return dot11QosWirelessTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11QosWirelessTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11QosWirelessTable_entry *entry = (struct dot11QosWirelessTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->globalWtpID, sizeof(entry->globalWtpID) );
        idx = idx->next_variable;
        snmp_set_var_value( idx, (u_char*)&entry->RadioLocalID, sizeof(entry->RadioLocalID) );
        idx = idx->next_variable;
        snmp_set_var_value( idx, (u_char*)&entry->QosType, sizeof(entry->QosType) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	
	return put_index_data; 
}


/** handles requests for the dot11QosWirelessTable table */
int
dot11QosWirelessTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11QosWirelessTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11QosWirelessTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);			
			if( !table_entry )
			{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}	
				
            void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                break;
    
            switch (table_info->colnum) {
            case COLUMN_WTPID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->globalWtpID,
                                          sizeof(long));
                break;
            case COLUMN_RADIOLOCALID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->RadioLocalID,
                                          sizeof(long));
                break;
            case COLUMN_QOSTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->QosType,
                                          sizeof(long));
                break;
            case COLUMN_QOSWIRELESSAIFS:
            {   
			    char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char type[20] = { 0 };
				memset(type,0,20);
				char radioID[10] = { 0 };
                memset(radioID,0,10);
				int ret = 0;
				DCLI_WQOS *WQOS = NULL;
				snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->WtpID);
                snprintf(radioID,sizeof(radioID)-1,"%d",table_entry->RadioLocalID);

				if(table_entry->QosType == 1)
				{
					memset(type,0,20);
					strncpy(type,"BESTEFFORT",sizeof(type)-1);
				}
				else if(table_entry->QosType == 2)
				{
					memset(type,0,20);
	                strncpy(type,"BACKGROUND",sizeof(type)-1);
				}
				else if(table_entry->QosType == 3)
				{
					memset(type,0,20);
	                strncpy(type,"VIDEO",sizeof(type)-1);
				}
				else if(table_entry->QosType == 4)
				{
					memset(type,0,20);
	                strncpy(type,"VOICE",sizeof(type)-1);
				}				

				ret = wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS);
				if((ret == 1)&&(WQOS->qos[0])&&(WQOS->qos[0]->radio_qos[table_entry->QosType-1]))
				{
					table_entry->QosWirelessAIFS = WQOS->qos[0]->radio_qos[table_entry->QosType-1]->AIFS;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->QosWirelessAIFS,
                                          sizeof(long));

				if(ret == 1)
				{
					Free_qos_one(WQOS);
				}
            }
                break;
            case COLUMN_QOSWIRELESSCWMIN:
			 {   
			    char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char type[20] = { 0 };
				memset(type,0,20);
				char radioID[10] = { 0 };
                memset(radioID,0,10);
				int ret = 0;
				DCLI_WQOS *WQOS = NULL;
				snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->WtpID);
                snprintf(radioID,sizeof(radioID)-1,"%d",table_entry->RadioLocalID);

				if(table_entry->QosType == 1)
				{
					memset(type,0,20);
					strncpy(type,"BESTEFFORT",sizeof(type)-1);
				}
				else if(table_entry->QosType == 2)
				{
					memset(type,0,20);
	                strncpy(type,"BACKGROUND",sizeof(type)-1);
				}
				else if(table_entry->QosType == 3)
				{
					memset(type,0,20);
	                strncpy(type,"VIDEO",sizeof(type)-1);
				}
				else if(table_entry->QosType == 4)
				{
					memset(type,0,20);
	                strncpy(type,"VOICE",sizeof(type)-1);
				}
				
				ret = wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS);
				if((ret == 1)&&(WQOS->qos[0])&&(WQOS->qos[0]->radio_qos[table_entry->QosType-1]))
				{
					table_entry->QosWirelessCWmin = WQOS->qos[0]->radio_qos[table_entry->QosType-1]->CWMin;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->QosWirelessCWmin,
                                          sizeof(long));

				if(ret == 1)
				{
					Free_qos_one(WQOS);
				}
            }
                break;
            case COLUMN_QOSWIRELESSCWMAX:
			{   
			    char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char type[20] = { 0 };
				memset(type,0,20);
				char radioID[10] = { 0 };
                memset(radioID,0,10);
				int ret = 0;
				DCLI_WQOS *WQOS = NULL;
				snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->WtpID);
                snprintf(radioID,sizeof(radioID)-1,"%d",table_entry->RadioLocalID);

				if(table_entry->QosType == 1)
				{
					memset(type,0,20);
					strncpy(type,"BESTEFFORT",sizeof(type)-1);
				}
				else if(table_entry->QosType == 2)
				{
					memset(type,0,20);
	                strncpy(type,"BACKGROUND",sizeof(type)-1);
				}
				else if(table_entry->QosType == 3)
				{
					memset(type,0,20);
	                strncpy(type,"VIDEO",sizeof(type)-1);
				}
				else if(table_entry->QosType == 4)
				{
					memset(type,0,20);
	                strncpy(type,"VOICE",sizeof(type)-1);
				}
				
				ret = wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS);
				if((ret == 1)&&(WQOS->qos[0])&&(WQOS->qos[0]->radio_qos[table_entry->QosType-1]))
				{
					table_entry->QoSWirelessCWmax = WQOS->qos[0]->radio_qos[table_entry->QosType-1]->CWMax;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->QoSWirelessCWmax,
                                          sizeof(long));

				if(ret == 1)
				{
					Free_qos_one(WQOS);
				}
            }
                break;
            case COLUMN_QOSWIRELESSTXOPLIM:
			{   
			    char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char type[20] = { 0 };
				memset(type,0,20);
				char radioID[10] = { 0 };
                memset(radioID,0,10);
				int ret = 0;
				DCLI_WQOS *WQOS = NULL;
				snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->WtpID);
                snprintf(radioID,sizeof(radioID)-1,"%d",table_entry->RadioLocalID);

				if(table_entry->QosType == 1)
				{
					memset(type,0,20);
					strncpy(type,"BESTEFFORT",sizeof(type)-1);
				}
				else if(table_entry->QosType == 2)
				{
					memset(type,0,20);
	                strncpy(type,"BACKGROUND",sizeof(type)-1);
				}
				else if(table_entry->QosType == 3)
				{
					memset(type,0,20);
	                strncpy(type,"VIDEO",sizeof(type)-1);
				}
				else if(table_entry->QosType == 4)
				{
					memset(type,0,20);
	                strncpy(type,"VOICE",sizeof(type)-1);
				}
				
				ret = wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS);
				if((ret == 1)&&(WQOS->qos[0])&&(WQOS->qos[0]->radio_qos[table_entry->QosType-1]))
				{
					table_entry->QoSWirelessTXOPLim = WQOS->qos[0]->radio_qos[table_entry->QosType-1]->TXOPlimit;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->QoSWirelessTXOPLim,                                          
                                          sizeof(long));

				if(ret == 1)
				{
					Free_qos_one(WQOS);
				}
            }
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11QosWirelessTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_QOSWIRELESSAIFS:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_QOSWIRELESSCWMIN:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_QOSWIRELESSCWMAX:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_QOSWIRELESSTXOPLIM:
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
            table_entry = (struct dot11QosWirelessTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info( request);

			if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }   

			void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                break;
            
            switch (table_info->colnum) {

			 case COLUMN_QOSWIRELESSAIFS:
			{	
				int result1 = 0,result2 = 0;
				char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char type[20] = { 0 };
				memset(type,0,20);
				char radioID[10] = { 0 };
				memset(radioID,0,10);
				char cwMin[20] = { 0 };
				char cwMax[20] = { 0 };
				char aifs[20] = { 0 };
				char txopLimit[20] = { 0 };
				char ack[20] = { 0 };
				int radioGId;
				char radio_g_id[10] = { 0 };
				memset(radio_g_id,0,10);
				char qos_name[10] = { 0 };
				memset(qos_name,0,10);
				int ret = 0;
				DCLI_WQOS *WQOS1 = NULL,*WQOS2 = NULL;
				snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->WtpID);
				snprintf(radioID,sizeof(radioID)-1,"%d",table_entry->RadioLocalID);

				if(table_entry->QosType == 1)
				{
					memset(type,0,20);
					strncpy(type,"besteffort",sizeof(type)-1);//"BESTEFFORT");
				}
				else if(table_entry->QosType == 2)
				{
					memset(type,0,20);
					strncpy(type,"background",sizeof(type)-1);//BACKGROUND");
				}
				else if(table_entry->QosType == 3)
				{
					memset(type,0,20);
					strncpy(type,"video",sizeof(type)-1);//VIDEO");
				}
				else if(table_entry->QosType == 4)
				{
					memset(type,0,20);
					strncpy(type,"voice",sizeof(type)-1);//VOICE");
				}
				
				result1 = wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS1);
				if(( 1 == result1 )&&(WQOS1->qos[0])&&(WQOS1->qos[0]->radio_qos[0]))
				{
					memset(cwMin,0,20);
					memset(cwMax,0,20);
					memset(aifs,0,20);
					memset(txopLimit,0,20);
					memset(ack,0,20);
					
					snprintf( cwMin, sizeof(cwMin)-1, "%u", WQOS1->qos[0]->radio_qos[0]->CWMin);
					snprintf( cwMax, sizeof(cwMax)-1, "%u", WQOS1->qos[0]->radio_qos[0]->CWMax);
					snprintf( aifs, sizeof(aifs)-1, "%u", *request->requestvb->val.integer );
					snprintf( txopLimit, sizeof(txopLimit)-1, "%u",WQOS1->qos[0]->radio_qos[0]->TXOPlimit);
					if( 1 == WQOS1->qos[0]->radio_qos[0]->ACK)
					{
						snprintf( ack, sizeof(ack)-1, "ack");
					}
					else
					{
						snprintf( ack, sizeof(ack)-1, "noack");
					}
				
					ret = 0;
					ret = config_radio_qos_service(table_entry->parameter, connection,WQOS1->qos[0]->QosID, type,
										cwMin, cwMax, aifs, txopLimit, ack );	/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
				}
				else
				{
					radioGId = table_entry->WtpID * 4 + table_entry->RadioLocalID;
					snprintf(radio_g_id,sizeof(radio_g_id)-1,"%d",radioGId);
					snprintf(qos_name,sizeof(qos_name)-1,"wqos%d",radioGId);

					create_qos(table_entry->parameter, connection,radio_g_id,qos_name);
					
					ret = 0;
					ret = radio_apply_qos(table_entry->parameter, connection,radioGId,radio_g_id);
					if(ret == 1)
					{
						result2=wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS2);
						if(( 1 == result2 )&&(WQOS2->qos[0])&&(WQOS2->qos[0]->radio_qos[0]))
						{
							memset(cwMin,0,20);
							memset(cwMax,0,20);
							memset(aifs,0,20);
							memset(txopLimit,0,20);
							memset(ack,0,20);
							
							snprintf( cwMin, sizeof(cwMin)-1, "%u", WQOS2->qos[0]->radio_qos[0]->CWMin);
							snprintf( cwMax, sizeof(cwMax)-1, "%u", WQOS2->qos[0]->radio_qos[0]->CWMax);
							snprintf( aifs, sizeof(aifs)-1, "%u", *request->requestvb->val.integer );
							snprintf( txopLimit, sizeof(txopLimit)-1, "%u",WQOS2->qos[0]->radio_qos[0]->TXOPlimit);
							if( 1 == WQOS2->qos[0]->radio_qos[0]->ACK)
							{
								snprintf( ack, sizeof(ack)-1, "ack");
							}
							else
							{
								snprintf( ack, sizeof(ack)-1, "noack");
							}
						
							ret = 0;
							ret = config_radio_qos_service(table_entry->parameter, connection,WQOS2->qos[0]->QosID, type,
												cwMin, cwMax, aifs, txopLimit, ack );	/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
						}

						if( result2 == 1 )
						{
							Free_qos_one(WQOS2);
						}
					}
				}

				if( 1 != ret )
				{
					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
				}

				if( result1 == 1 )
				{
					Free_qos_one(WQOS1);
				}
			}
			 	break;
		    case COLUMN_QOSWIRELESSCWMIN:
			{	
				int result1 = 0,result2 = 0;
				char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char type[20] = { 0 };
				memset(type,0,20);
				char radioID[10] = { 0 };
				memset(radioID,0,10);
				char cwMin[20] = { 0 };
				char cwMax[20] = { 0 };
				char aifs[20] = { 0 };
				char txopLimit[20] = { 0 };
				char ack[20] = { 0 };
				int radioGId;
				char radio_g_id[10] = { 0 };
				memset(radio_g_id,0,10);
				char qos_name[10] = { 0 };
				memset(qos_name,0,10);
				int ret = 0;
				DCLI_WQOS *WQOS1 = NULL,*WQOS2 = NULL;
				snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->WtpID);
				snprintf(radioID,sizeof(radioID)-1,"%d",table_entry->RadioLocalID);

				if(table_entry->QosType == 1)
				{
					memset(type,0,20);
					strncpy(type,"besteffort",sizeof(type)-1);//"BESTEFFORT");
				}
				else if(table_entry->QosType == 2)
				{
					memset(type,0,20);
					strncpy(type,"background",sizeof(type)-1);//BACKGROUND");
				}
				else if(table_entry->QosType == 3)
				{
					memset(type,0,20);
					strncpy(type,"video",sizeof(type)-1);//VIDEO");
				}
				else if(table_entry->QosType == 4)
				{
					memset(type,0,20);
					strncpy(type,"voice",sizeof(type)-1);//VOICE");
				}
				
				result1= wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS1);				
				if(( 1 == result1 )&&(WQOS1->qos[0])&&(WQOS1->qos[0]->radio_qos[0]))
				{
					memset(cwMin,0,20);
					memset(cwMax,0,20);
					memset(aifs,0,20);
					memset(txopLimit,0,20);
					memset(ack,0,20);
					
					snprintf( cwMin, sizeof(cwMin)-1, "%u", *request->requestvb->val.integer );
					snprintf( cwMax, sizeof(cwMax)-1, "%u", WQOS1->qos[0]->radio_qos[0]->CWMax);
					snprintf( aifs, sizeof(aifs)-1, "%u", WQOS1->qos[0]->radio_qos[0]->AIFS);
					snprintf( txopLimit, sizeof(txopLimit)-1, "%u", WQOS1->qos[0]->radio_qos[0]->TXOPlimit);
					if( 1 == WQOS1->qos[0]->radio_qos[0]->ACK )
					{
						snprintf( ack, sizeof(ack)-1, "ack");
					}
					else
					{
						snprintf( ack, sizeof(ack)-1, "noack");
					}
				
					ret = 0;
					ret = config_radio_qos_service(table_entry->parameter, connection, WQOS1->qos[0]->QosID, type,
										cwMin, cwMax, aifs, txopLimit, ack );	/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
				}
				else
				{
					radioGId = table_entry->WtpID * 4 + table_entry->RadioLocalID;
					snprintf(radio_g_id,sizeof(radio_g_id)-1,"%d",radioGId);
					snprintf(qos_name,sizeof(qos_name)-1,"wqos%d",radioGId);
					create_qos(table_entry->parameter, connection,radio_g_id,qos_name);
					ret = 0;
					ret = radio_apply_qos(table_entry->parameter, connection,radioGId,radio_g_id);
					if(ret == 1)
					{
						result2=wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS2);
						if(( 1 == result2 )&&(WQOS2->qos[0])&&(WQOS2->qos[0]->radio_qos[0]))
						{
							memset(cwMin,0,20);
							memset(cwMax,0,20);
							memset(aifs,0,20);
							memset(txopLimit,0,20);
							memset(ack,0,20);
							
							snprintf( cwMin, sizeof(cwMin)-1, "%u", *request->requestvb->val.integer );
							snprintf( cwMax, sizeof(cwMax)-1, "%u", WQOS2->qos[0]->radio_qos[0]->CWMax );
							snprintf( aifs, sizeof(aifs)-1, "%u", WQOS2->qos[0]->radio_qos[0]->AIFS );
							snprintf( txopLimit, sizeof(txopLimit)-1, "%u", WQOS2->qos[0]->radio_qos[0]->TXOPlimit);
							if( 1 == WQOS2->qos[0]->radio_qos[0]->ACK )
							{
								snprintf( ack, sizeof(ack)-1, "ack");
							}
							else
							{
								snprintf( ack, sizeof(ack)-1, "noack");
							}
						
							ret = 0;
							ret = config_radio_qos_service(table_entry->parameter, connection, WQOS2->qos[0]->QosID, type,
												cwMin, cwMax, aifs, txopLimit, ack );	/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
						}

						if( result2 == 1 )
						{
							Free_qos_one(WQOS2);
						}
					}
				}

				if( 1 != ret )
				{
					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
				}

				if( result1 == 1 )
				{
					Free_qos_one(WQOS1);
				}
			}
				break;
			case COLUMN_QOSWIRELESSCWMAX:
			{	
				int result1=0,result2=0;
				char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char type[20] = { 0 };
				memset(type,0,20);
				char radioID[10] = { 0 };
				memset(radioID,0,10);
				char cwMin[20] = { 0 };
				char cwMax[20] = { 0 };
				char aifs[20] = { 0 };
				char txopLimit[20] = { 0 };
				char ack[20] = { 0 };
				int radioGId;
				char radio_g_id[10] = { 0 };
				memset(radio_g_id,0,10);
				char qos_name[10] = { 0 };
				memset(qos_name,0,10);
				int ret = 0;
				DCLI_WQOS *WQOS1 = NULL,*WQOS2 = NULL;
				snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->WtpID);
				snprintf(radioID,sizeof(radioID)-1,"%d",table_entry->RadioLocalID);
				if(table_entry->QosType == 1)
				{
					memset(type,0,20);
					strncpy(type,"besteffort",sizeof(type)-1);//"BESTEFFORT");
				}
				else if(table_entry->QosType == 2)
				{
					memset(type,0,20);
					strncpy(type,"background",sizeof(type)-1);//BACKGROUND");
				}
				else if(table_entry->QosType == 3)
				{
					memset(type,0,20);
					strncpy(type,"video",sizeof(type)-1);//VIDEO");
				}
				else if(table_entry->QosType == 4)
				{
					memset(type,0,20);
					strncpy(type,"voice",sizeof(type)-1);//VOICE");
				}
				result1 = wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS1);
				
				if(( 1 == result1 )&&(WQOS1->qos[0])&&(WQOS1->qos[0]->radio_qos[0]))
				{
					memset(cwMin,0,20);
					memset(cwMax,0,20);
					memset(aifs,0,20);
					memset(txopLimit,0,20);
					memset(ack,0,20);
					
					snprintf( cwMin, sizeof(cwMin)-1, "%u", WQOS1->qos[0]->radio_qos[0]->CWMin );
					snprintf( cwMax, sizeof(cwMax)-1, "%u", *request->requestvb->val.integer);
					snprintf( aifs, sizeof(aifs)-1, "%u", WQOS1->qos[0]->radio_qos[0]->AIFS );
					snprintf( txopLimit, sizeof(txopLimit)-1, "%u",WQOS1->qos[0]->radio_qos[0]->TXOPlimit);
					if( 1 == WQOS1->qos[0]->radio_qos[0]->ACK )
					{
						snprintf( ack, sizeof(ack)-1, "ack");
					}
					else
					{
						snprintf( ack, sizeof(ack)-1, "noack");
					}
				
					ret = 0;
					ret = config_radio_qos_service(table_entry->parameter, connection,WQOS1->qos[0]->QosID, type,
										cwMin, cwMax, aifs, txopLimit, ack );	/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
				}
				else
				{
					radioGId = table_entry->WtpID * 4 + table_entry->RadioLocalID;
					snprintf(radio_g_id,sizeof(radio_g_id)-1,"%d",radioGId);
					snprintf(qos_name,sizeof(qos_name)-1,"wqos%d",radioGId);

					create_qos(table_entry->parameter, connection,radio_g_id,qos_name);
					
					ret = 0;
					ret = radio_apply_qos(table_entry->parameter, connection,radioGId,radio_g_id);
					if(ret == 1)
					{
						result2=wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS2);
						if(( 1 == result2 )&&(WQOS2->qos[0])&&(WQOS2->qos[0]->radio_qos[0]))
						{
							memset(cwMin,0,20);
							memset(cwMax,0,20);
							memset(aifs,0,20);
							memset(txopLimit,0,20);
							memset(ack,0,20);
							
							snprintf( cwMin, sizeof(cwMin)-1, "%u", WQOS2->qos[0]->radio_qos[0]->CWMin );
							snprintf( cwMax, sizeof(cwMax)-1, "%u", *request->requestvb->val.integer);
							snprintf( aifs, sizeof(aifs)-1, "%u", WQOS2->qos[0]->radio_qos[0]->AIFS );
							snprintf( txopLimit, sizeof(txopLimit)-1, "%u",WQOS2->qos[0]->radio_qos[0]->TXOPlimit);
							if( 1 == WQOS2->qos[0]->radio_qos[0]->ACK )
							{
								snprintf( ack, sizeof(ack)-1, "ack");
							}
							else
							{
								snprintf( ack, sizeof(ack)-1, "noack");
							}
						
							ret = 0;
							ret = config_radio_qos_service(table_entry->parameter, connection,WQOS2->qos[0]->QosID, type,
												cwMin, cwMax, aifs, txopLimit, ack );	/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
						}

						if( result2 == 1 )
						{
							Free_qos_one(WQOS2);
						}
					}
				}

				if( 1 != ret )
				{
					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
				}


				if( result1 == 1 )
				{
					Free_qos_one(WQOS1);
				}
			}
				break;
			case COLUMN_QOSWIRELESSTXOPLIM:
			{	
				int result1 = 0,result2 = 0;
				char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char type[20] = { 0 };
				memset(type,0,20);
				char radioID[10] = { 0 };
				memset(radioID,0,10);
				char cwMin[20] = { 0 };
				char cwMax[20] = { 0 };
				char aifs[20] = { 0 };
				char txopLimit[20] = { 0 };
				char ack[20] = { 0 };
				int radioGId;
				char radio_g_id[10] = { 0 };
				memset(radio_g_id,0,10);
				char qos_name[10] = { 0 };
				memset(qos_name,0,10);
				int ret = 0;
				DCLI_WQOS *WQOS1 = NULL,*WQOS2 = NULL;
				snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->WtpID);
				snprintf(radioID,sizeof(radioID)-1,"%d",table_entry->RadioLocalID);

				if(table_entry->QosType == 1)
				{
					memset(type,0,20);
					strncpy(type,"besteffort",sizeof(type)-1);//"BESTEFFORT");
				}
				else if(table_entry->QosType == 2)
				{
					memset(type,0,20);
					strncpy(type,"background",sizeof(type)-1);//BACKGROUND");
				}
				else if(table_entry->QosType == 3)
				{
					memset(type,0,20);
					strncpy(type,"video",sizeof(type)-1);//VIDEO");
				}
				else if(table_entry->QosType == 4)
				{
					memset(type,0,20);
					strncpy(type,"voice",sizeof(type)-1);//VOICE");
				}
				
				result1= wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS1);				
				if(( 1 == result1 )&&(WQOS1->qos[0])&&(WQOS1->qos[0]->radio_qos[0]))
				{
					memset(cwMin,0,20);
					memset(cwMax,0,20);
					memset(aifs,0,20);
					memset(txopLimit,0,20);
					memset(ack,0,20);
					
					snprintf( cwMin, sizeof(cwMin)-1, "%u", WQOS1->qos[0]->radio_qos[0]->CWMin );
					snprintf( cwMax, sizeof(cwMax)-1, "%u", WQOS1->qos[0]->radio_qos[0]->CWMax );
					snprintf( aifs, sizeof(aifs)-1, "%u", WQOS1->qos[0]->radio_qos[0]->AIFS );
					snprintf( txopLimit, sizeof(txopLimit)-1, "%u", *request->requestvb->val.integer);
					if( 1 == WQOS1->qos[0]->radio_qos[0]->ACK )
					{
						snprintf( ack, sizeof(ack)-1, "ack");
					}
					else
					{
						snprintf( ack, sizeof(ack)-1, "noack");
					}

					ret = 0;				
					ret = config_radio_qos_service(table_entry->parameter, connection, WQOS1->qos[0]->QosID, type,
										cwMin, cwMax, aifs, txopLimit, ack );	/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/

				}
				else
				{
					radioGId = table_entry->WtpID * 4 + table_entry->RadioLocalID;
					snprintf(radio_g_id,sizeof(radio_g_id)-1,"%d",radioGId);
					snprintf(qos_name,sizeof(qos_name)-1,"wqos%d",radioGId);

					create_qos(table_entry->parameter, connection,radio_g_id,qos_name);
					
					ret = 0;
					ret = radio_apply_qos(table_entry->parameter, connection,radioGId,radio_g_id);
					if(ret == 1)
					{
						result2=wid_show_qos_radio_cmd(table_entry->parameter, connection,wtpID,radioID,type,&WQOS2);
						if(( 1 == result2 )&&(WQOS2->qos[0])&&(WQOS2->qos[0]->radio_qos[0]))
						{
							memset(cwMin,0,20);
							memset(cwMax,0,20);
							memset(aifs,0,20);
							memset(txopLimit,0,20);
							memset(ack,0,20);
							
							snprintf( cwMin, sizeof(cwMin)-1, "%u", WQOS2->qos[0]->radio_qos[0]->CWMin );
							snprintf( cwMax, sizeof(cwMax)-1, "%u", WQOS2->qos[0]->radio_qos[0]->CWMax );
							snprintf( aifs, sizeof(aifs)-1, "%u", WQOS2->qos[0]->radio_qos[0]->AIFS );
							snprintf( txopLimit, sizeof(txopLimit)-1, "%u", *request->requestvb->val.integer);
							if( 1 == WQOS2->qos[0]->radio_qos[0]->ACK )
							{
								snprintf( ack, sizeof(ack)-1, "ack");
							}
							else
							{
								snprintf( ack, sizeof(ack)-1, "noack");
							}
						
							ret = 0;
							ret = config_radio_qos_service(table_entry->parameter, connection, WQOS2->qos[0]->QosID, type,
												cwMin, cwMax, aifs, txopLimit, ack );	/*返回0表示失败，返回1表示成功，返回-1表示unknown qos type，返回-2表示unknown id format*/
						
						}

						if( result2 == 1 )
						{
							Free_qos_one(WQOS2);
						}
					}
				}

				if( 1 != ret )
				{
					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
				}

				if( result1 == 1 )
				{
					Free_qos_one(WQOS1);
				}
			}
			break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11QosWirelessTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_QOSWIRELESSAIFS:
                /* Need to restore old 'table_entry->QosWirelessAIFS' value.
                   May need to use 'memcpy' */
                break;
            case COLUMN_QOSWIRELESSCWMIN:
                /* Need to restore old 'table_entry->QosWirelessCWmin' value.
                   May need to use 'memcpy' */
                break;
            case COLUMN_QOSWIRELESSCWMAX:
                /* Need to restore old 'table_entry->QoSWirelessCWmax' value.
                   May need to use 'memcpy' */
                break;
            case COLUMN_QOSWIRELESSTXOPLIM:
                /* Need to restore old 'table_entry->QoSWirelessTXOPLim' value.
                   May need to use 'memcpy' */
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
