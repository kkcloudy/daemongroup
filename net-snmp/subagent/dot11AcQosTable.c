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
* dot11AcQosTable.c
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
#include "dot11AcQosTable.h"
#include "wcpss/wid/WID.h"
#include "ws_dcli_wqos.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define QOSTABLE "3.2.2"
struct dot11AcQosTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalQosID;
    long QosID;
    long dot11QosTrafficClass;

    /* Column values */
    long dot11QosAIFS;
    //long old_dot11QosAIFS;
    long dot11QoSCWmin;
    //long old_dot11QoSCWmin;
    long dot11QoSCWmax;
    //long old_dot11QoSCWmax;
    long dot11QoSTXOPLim;
    //long old_dot11QoSTXOPLim;
    long dot11AvgRate;
    //long old_dot11AvgRate;
    long dot11MaxDegree;
    //long old_dot11MaxDegree;
    long dot11PolicyPRI;
    //long old_dot11PolicyPRI;
    long dot11ResShovePRI;
    //long old_dot11ResShovePRI;
    long dot11ResGrabPRI;
    //long old_dot11ResGrabPRI;
    long dot11MaxPallel;
    //long old_dot11MaxPallel;
    long dot11Bandwidth;
    //long old_dot11Bandwidth;
    long dot11BandwidthScale;
    //long old_dot11BandwidthScale;
    long dot11UseFlowEqtQueue;
    //long old_dot11UseFlowEqtQueue;
    long dot11SingleFlowMaxQueue;
    //long old_dot11SingleFlowMaxQueue;
    long dot11FlowAvgRate;
    //long old_dot11FlowAvgRate;
    long dot11FlowMaxDegree;
    //long old_dot11FlowMaxDegree;
    long dot11UseWREDPolicy;
    //long old_dot11UseWREDPolicy;
    long dot11UseTrafficShaping;
    //long old_dot11UseTrafficShaping;
	long QoSSvcPktLossRatio;
	//long old_QoSSvcPktLossRatio;
	long PktLossRatio;
	//long old_PktLossRatio;
	long SvcLoss;
	//long old_SvcLoss;
	long QueAvgLen;
	//long old_QueAvgLen;
	long PutThroughRatio;
	//long old_PutThroughRatio;
	long DropRatio;
	//long old_DropRatio;
	long VoiceExceedMaxUsersRequest;
	//long old_VoiceExceedMaxUsersRequest;
	long VideoExceedMaxUsersRequest;
	//long old_VideoExceedMaxUsersRequest;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11AcQosTable_entry *next;
};

void dot11AcQosTable_load();
void
dot11AcQosTable_removeEntry( struct dot11AcQosTable_entry *entry );

/** Initializes the dot11AcQosTable module */
void
init_dot11AcQosTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11AcQosTable();
}

/** Initialize the dot11AcQosTable table by defining its contents and how it's structured */
void
initialize_table_dot11AcQosTable(void)
{
    static oid dot11AcQosTable_oid[128] = {0};
    size_t dot11AcQosTable_oid_len   = 0;
	
	mad_dev_oid(dot11AcQosTable_oid,QOSTABLE,&dot11AcQosTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11AcQosTable",     dot11AcQosTable_handler,
              dot11AcQosTable_oid, dot11AcQosTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalQosID */
                           ASN_INTEGER,  /* index: dot11QosTrafficClass */
                           0);
    table_info->min_column = COLUMN_DOT11QOSTRAFFICCLASS;
    table_info->max_column = COLUMN_VIDEOEXCEEDMAXUSERSREQUEST;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11AcQosTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11AcQosTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11AcQosTable_load, dot11AcQosTable_removeEntry,
							dot11AcQosTable_oid, dot11AcQosTable_oid_len));

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11AcQosTable_entry  *dot11AcQosTable_head;

/* create a new row in the (unsorted) table */
struct dot11AcQosTable_entry *
dot11AcQosTable_createEntry(
				dbus_parameter parameter,
				long globalQosID,
                long  QosID,
                long  dot11QosTrafficClass,
                long dot11QosAIFS,
			    long dot11QoSCWmin,
			    long dot11QoSCWmax,
			    long dot11QoSTXOPLim,
			    long dot11AvgRate,
			    long dot11MaxDegree,
			    long dot11PolicyPRI,
			    long dot11ResShovePRI,
			    long dot11ResGrabPRI,
			    long dot11MaxPallel,
			    long dot11Bandwidth,
			    long dot11BandwidthScale,
			    long dot11UseFlowEqtQueue,
			    long dot11SingleFlowMaxQueue,
			    long dot11FlowAvgRate,
			    long dot11FlowMaxDegree,
			    long dot11UseWREDPolicy,
			    long dot11UseTrafficShaping,
			    long QoSSvcPktLossRatio,
				long PktLossRatio,
				long SvcLoss,
				long QueAvgLen,
				long PutThroughRatio,
				long DropRatio,
				long VoiceExceedMaxUsersRequest,
				long VideoExceedMaxUsersRequest
                ) {
    struct dot11AcQosTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11AcQosTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->globalQosID = globalQosID;
    entry->QosID = QosID;
    entry->dot11QosTrafficClass = dot11QosTrafficClass;
	entry->dot11QosAIFS =dot11QosAIFS;
    entry->dot11QoSCWmin = dot11QoSCWmin;
    entry->dot11QoSCWmax = dot11QoSCWmax;
    entry->dot11QoSTXOPLim = dot11QoSTXOPLim;
    entry->dot11AvgRate = dot11AvgRate;
    entry->dot11MaxDegree = dot11MaxDegree;
    entry->dot11PolicyPRI = dot11PolicyPRI;
    entry->dot11ResShovePRI = dot11ResShovePRI;
    entry->dot11ResGrabPRI = dot11ResGrabPRI;
    entry->dot11MaxPallel = dot11MaxPallel;
    entry->dot11Bandwidth = dot11Bandwidth;
    entry->dot11BandwidthScale = dot11BandwidthScale;
    entry->dot11UseFlowEqtQueue = dot11UseFlowEqtQueue;
    entry->dot11SingleFlowMaxQueue = dot11SingleFlowMaxQueue;
    entry->dot11FlowAvgRate = dot11FlowAvgRate;
    entry->dot11FlowMaxDegree = dot11FlowMaxDegree;
    entry->dot11UseWREDPolicy = dot11UseWREDPolicy;
    entry->dot11UseTrafficShaping = dot11UseTrafficShaping;
	entry->QoSSvcPktLossRatio = QoSSvcPktLossRatio;
	entry->PktLossRatio = PktLossRatio;
	entry->SvcLoss = SvcLoss;
	entry->QueAvgLen = QueAvgLen;
	entry->PutThroughRatio = PutThroughRatio;
	entry->DropRatio = DropRatio;
	entry->VoiceExceedMaxUsersRequest = VoiceExceedMaxUsersRequest;
	entry->VideoExceedMaxUsersRequest = VideoExceedMaxUsersRequest;
    entry->next = dot11AcQosTable_head;
    dot11AcQosTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11AcQosTable_removeEntry( struct dot11AcQosTable_entry *entry ) {
    struct dot11AcQosTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11AcQosTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11AcQosTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11AcQosTable_load()
{	
	snmp_log(LOG_DEBUG, "enter dot11AcQosTable_load\n");

	struct dot11AcQosTable_entry *temp;	
	while( dot11AcQosTable_head )
	{
		temp=dot11AcQosTable_head->next;
		dot11AcQosTable_removeEntry(dot11AcQosTable_head);
		dot11AcQosTable_head=temp;
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
            if(wqos)
			{
                int i = 0;
                for(i=0;i<wqos->qos_num;i++)
                {                	
                	unsigned long globalQosID = 0;						
					if(wqos->qos[i])
					{
						globalQosID = local_to_global_ID(messageNode->parameter, 
														 wqos->qos[i]->QosID, 
														 WIRELESS_MAX_NUM);
						int j = 0;
						for(j=0;j<4;j++)
						{
							dot11AcQosTable_createEntry(messageNode->parameter,
														globalQosID,
														wqos->qos[i]->QosID,
														j+1,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
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
        }
        free_dbus_message_list(&messageHead, Free_qos_head);
    }        
	
	snmp_log(LOG_DEBUG, "exit dot11AcQosTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11AcQosTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{


	if(dot11AcQosTable_head==NULL)
		return NULL;
	*my_data_context = dot11AcQosTable_head;
	*my_loop_context = dot11AcQosTable_head;
	return dot11AcQosTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11AcQosTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11AcQosTable_entry *entry = (struct dot11AcQosTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->globalQosID, sizeof(long) );
        idx = idx->next_variable;
        snmp_set_var_value( idx, (u_char*)&entry->dot11QosTrafficClass, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11AcQosTable table */
int
dot11AcQosTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11AcQosTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AcQosTable_entry *)
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
            case COLUMN_DOT11QOSTRAFFICCLASS:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->dot11QosTrafficClass,
                                          sizeof(long));
            }
                break;
            case COLUMN_DOT11QOSAIFS:
			{
				int dot11QosAIFS = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_one(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11QosAIFS = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->AIFS;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11QosAIFS,
                                          sizeof(dot11QosAIFS));
				
				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_DOT11QOSCWMIN:
			{
				int dot11QoSCWmin = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_one(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11QoSCWmin = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->CWMin;
				}

                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11QoSCWmin,
                                          sizeof(dot11QoSCWmin));
				
				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_DOT11QOSCWMAX:
			{
				int dot11QoSCWmax = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_one(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11QoSCWmax = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->CWMax;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11QoSCWmax,
                                          sizeof(dot11QoSCWmax));
				
				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_DOT11QOSTXOPLIM:
			{
				int dot11QoSTXOPLim = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_one(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11QoSTXOPLim = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->TXOPlimit;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11QoSTXOPLim,
                                          sizeof(dot11QoSTXOPLim));
				
				if(ret==1)
				{
					Free_qos_one(wqos);
				}
            }
                break;
            case COLUMN_DOT11AVGRATE:
			{   
				int dot11AvgRate = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11AvgRate = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_average_rate;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11AvgRate,
                                          sizeof(dot11AvgRate));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11MAXDEGREE:
			{   
				int dot11MaxDegree = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11MaxDegree = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_max_degree;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11MaxDegree,
                                          sizeof(dot11MaxDegree));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11POLICYPRI:
			{   
				int dot11PolicyPRI = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11PolicyPRI = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_policy_pri;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11PolicyPRI,
                                          sizeof(dot11PolicyPRI));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11RESSHOVEPRI:
			{   
				int dot11ResShovePRI = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11ResShovePRI = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_res_shove_pri;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11ResShovePRI,
                                          sizeof(dot11ResShovePRI));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11RESGRABPRI:
			{   
				int dot11ResGrabPRI = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11ResGrabPRI = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_res_grab_pri;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11ResGrabPRI,
                                          sizeof(dot11ResGrabPRI));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11MAXPALLEL:
			{   
				int dot11MaxPallel = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11MaxPallel = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_max_parallel;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11MaxPallel,
                                          sizeof(dot11MaxPallel));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11BANDWIDTH:
			{   
				int dot11Bandwidth = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11Bandwidth = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_bandwidth;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11Bandwidth,
                                          sizeof(dot11Bandwidth));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11BANDWIDTHSCALE:
			{   
				int dot11BandwidthScale = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11BandwidthScale = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_bandwidth_scale;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11BandwidthScale,
                                          sizeof(dot11BandwidthScale));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11USEFLOWEQTQUEUE:
			{   
				int dot11UseFlowEqtQueue = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11UseFlowEqtQueue = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_use_flow_eq_queue;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11UseFlowEqtQueue,
                                          sizeof(dot11UseFlowEqtQueue));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11SINGLEFLOWMAXQUEUE:
			{   
				int dot11SingleFlowMaxQueue = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11SingleFlowMaxQueue = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_flow_max_queuedepth;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11SingleFlowMaxQueue,
                                          sizeof(dot11SingleFlowMaxQueue));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11FLOWAVGRATE:
			{   
				int dot11FlowAvgRate = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11FlowAvgRate = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_flow_average_rate;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                         (u_char*)&dot11FlowAvgRate,
                                          sizeof(dot11FlowAvgRate));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11FLOWMAXDEGREE:
			{   
				int dot11FlowMaxDegree = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11FlowMaxDegree = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_flow_max_degree;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11FlowMaxDegree,
                                          sizeof(dot11FlowMaxDegree));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
            case COLUMN_DOT11USEWREDPOLICY:
			{   
				int dot11UseWREDPolicy = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11UseWREDPolicy = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_use_wred;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11UseWREDPolicy,
                                          sizeof(dot11UseWREDPolicy));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
			case COLUMN_DOT11USETRAFFICSHAPING:
		    {   
				int dot11UseTrafficShaping = 0;
				int ret = 0;
				char wqos_id[10] = { 0 };
				DCLI_WQOS *wqos = NULL;
				
				memset(wqos_id,0,10);
				snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
				ret=show_qos_extension_info(table_entry->parameter, connection,wqos_id,&wqos);
				if((ret == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
				{
					dot11UseTrafficShaping = wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->qos_use_traffic_shaping;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&dot11UseTrafficShaping,
                                          sizeof(dot11UseTrafficShaping));
				
				if(ret == 1)
				{
					Free_qos_extension_info(wqos);
				}
            }
                break;
			case COLUMN_QOSSVCPKTLOSSRATIO:
			{       
				int value = 0;
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&value,
                                         sizeof(long));
			}
                break;
			case COLUMN_PKTLOSSRATIO:
			{
				int value = 0;
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&value,
                                        sizeof(long));
			}
                break;
			case COLUMN_SVCLOSS:
			{
			    int value = 0;
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&value,
                                          sizeof(long));
			}
                break;
			case COLUMN_QUEAVGLEN :
			{
				int value = 0;
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&value,
                                      sizeof(long));
			}
                break;
			case COLUMN_PUTTHROUGHRATIO:
			{
				int value =0;
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&value,
                                        sizeof(long));
			}
                break;
			case COLUMN_DROPRATIO :
			{
				int value = 0;
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&value,
                                       sizeof(long));
			}
                break;
			case COLUMN_VOICEEXCEEDMAXUSERSREQUEST :
			{
				int value = 0;
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&value,
                                          sizeof(long));
			}
			case COLUMN_VIDEOEXCEEDMAXUSERSREQUEST :
			{
				int value = 0;
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&value,
                                          sizeof(long));
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
            table_entry = (struct dot11AcQosTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_DOT11QOSAIFS:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11QOSCWMIN:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11QOSCWMAX:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11QOSTXOPLIM:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11AVGRATE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11MAXDEGREE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11POLICYPRI:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11RESSHOVEPRI:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11RESGRABPRI:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11MAXPALLEL:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11BANDWIDTH:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11BANDWIDTHSCALE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11USEFLOWEQTQUEUE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11SINGLEFLOWMAXQUEUE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11FLOWAVGRATE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11FLOWMAXDEGREE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11USEWREDPOLICY:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_DOT11USETRAFFICSHAPING:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
		    case COLUMN_QOSSVCPKTLOSSRATIO:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_PKTLOSSRATIO:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_SVCLOSS:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_QUEAVGLEN:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_PUTTHROUGHRATIO:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_DROPRATIO:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_VOICEEXCEEDMAXUSERSREQUEST:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_VIDEOEXCEEDMAXUSERSREQUEST:
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
            table_entry = (struct dot11AcQosTable_entry *)
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
            case COLUMN_DOT11QOSAIFS:
				{
					int ret1=0,ret2=0;
					
					char wqos_id[10] = { 0 };
                    DCLI_WQOS *wqos = NULL;
					memset(wqos_id,0,10);
					snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);

					ret1=show_qos_one(table_entry->parameter, connection,wqos_id,&wqos);
					if(ret1!=1)
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char cwmin[10] = { 0 };
					memset(cwmin,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(cwmin,sizeof(cwmin)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->CWMin);
					}
					
					char cwmax[10] = { 0 };
					memset(cwmax,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(cwmax,sizeof(cwmax)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->CWMax);
					}
					
					char aifs[10] = { 0 };
					memset(aifs,0,10);
					snprintf(aifs,sizeof(aifs)-1,"%d",*request->requestvb->val.integer);
					
					char txoplimit[10] = { 0 };
					memset(txoplimit,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(txoplimit,sizeof(txoplimit)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->TXOPlimit);
					}
					
					char ack[10] = { 0 };
					memset(ack,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						if(wqos->qos[0]->radio_qos[table_entry->dot11QosTrafficClass-1]->ACK==1)
							strncpy(ack,"ack",sizeof(ack)-1);
						else
							strncpy(ack,"noack",sizeof(ack)-1);
					}
					
					ret2=config_radio_qos_service(table_entry->parameter, connection,table_entry->QosID,TrafficClass,cwmin,cwmax,aifs,txoplimit,ack);

					if(ret2 == 1)
					{
						table_entry->dot11QosAIFS = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					if(ret1==1)
					{
						Free_qos_one(wqos);
					}
            	}
                break;
            case COLUMN_DOT11QOSCWMIN:
				{
					int ret1=0,ret2=0;
					
					char wqos_id[10] = { 0 };
                    DCLI_WQOS *wqos = NULL;
					memset(wqos_id,0,10);
					snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
					
					ret1=show_qos_one(table_entry->parameter, connection,wqos_id,&wqos);
					if(ret1!=1)
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char cwmin[10] = { 0 };
					memset(cwmin,0,10);
					snprintf(cwmin,sizeof(cwmin)-1,"%d",*request->requestvb->val.integer);
					
					char cwmax[10] = { 0 };
					memset(cwmax,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(cwmax,sizeof(cwmax)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->CWMax);
					}
					
					char aifs[10] = { 0 };
					memset(aifs,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(aifs,sizeof(aifs)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->AIFS);
					}
					
					char txoplimit[10] = { 0 };
					memset(txoplimit,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(txoplimit,sizeof(txoplimit)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->TXOPlimit);
					}
					
					char ack[10] = { 0 };
					memset(ack,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						if(wqos->qos[0]->radio_qos[table_entry->dot11QosTrafficClass-1]->ACK==1)
							strncpy(ack,"ack",sizeof(ack)-1);
						else
							strncpy(ack,"noack",sizeof(ack)-1);
					}
					
					ret2=config_radio_qos_service(table_entry->parameter, connection,table_entry->QosID,TrafficClass,cwmin,cwmax,aifs,txoplimit,ack);

					if(ret2 == 1)
					{
						table_entry->dot11QoSCWmin = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					if(ret1==1)
					{
						Free_qos_one(wqos);
					}
            	}
                break;
            case COLUMN_DOT11QOSCWMAX:
				{
					int ret1=0,ret2=0;
					
					char wqos_id[10] = { 0 };
                    DCLI_WQOS *wqos = NULL;
					memset(wqos_id,0,10);
					snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
					
					ret1=show_qos_one(table_entry->parameter, connection,wqos_id,&wqos);
					if(ret1!=1)
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char cwmin[10] = { 0 };
					memset(cwmin,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(cwmin,sizeof(cwmin)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->CWMin);
					}
					
					char cwmax[10] = { 0 };
					memset(cwmax,0,10);
					snprintf(cwmax,sizeof(cwmax)-1,"%d",*request->requestvb->val.integer);
					
					char aifs[10] = { 0 };
					memset(aifs,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(aifs,sizeof(aifs)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->AIFS);
					}
					
					char txoplimit[10] = { 0 };
					memset(txoplimit,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(txoplimit,sizeof(txoplimit)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->TXOPlimit);
					}
					
					char ack[10] = { 0 };
					memset(ack,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						if(wqos->qos[0]->radio_qos[table_entry->dot11QosTrafficClass-1]->ACK==1)
							strncpy(ack,"ack",sizeof(ack)-1);
						else
							strncpy(ack,"noack",sizeof(ack)-1);
					}
					
					ret2=config_radio_qos_service(table_entry->parameter, connection,table_entry->QosID,TrafficClass,cwmin,cwmax,aifs,txoplimit,ack);

					if(ret2 == 1)
					{
						table_entry->dot11QoSCWmax = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					if(ret1==1)
					{
						Free_qos_one(wqos);
					}
            	}
                break;
            case COLUMN_DOT11QOSTXOPLIM:
				{
					int ret1=0,ret2=0;
					
					char wqos_id[10] = { 0 };
                    DCLI_WQOS *wqos = NULL;
					memset(wqos_id,0,10);
					snprintf(wqos_id,sizeof(wqos_id)-1,"%d",table_entry->QosID);
					
					ret1=show_qos_one(table_entry->parameter, connection,wqos_id,&wqos);
					if(ret1!=1)
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char cwmin[10] = { 0 };
					memset(cwmin,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(cwmin,sizeof(cwmin)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->CWMin);
					}
					
					char cwmax[10] = { 0 };
					memset(cwmax,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(cwmax,sizeof(cwmax)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->CWMax);
					}
					
					char aifs[10] = { 0 };
					memset(aifs,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						snprintf(aifs,sizeof(aifs)-1,"%d",wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]->AIFS);
					}
					
					char txoplimit[10] = { 0 };
					memset(txoplimit,0,10);
					snprintf(txoplimit,sizeof(txoplimit)-1,"%d",*request->requestvb->val.integer);
					
					char ack[10] = { 0 };
					memset(ack,0,10);
					if((ret1 == 1)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[(table_entry->dot11QosTrafficClass)-1]))
					{
						if(wqos->qos[0]->radio_qos[table_entry->dot11QosTrafficClass-1]->ACK==1)
							strncpy(ack,"ack",sizeof(ack)-1);
						else
							strncpy(ack,"noack",sizeof(ack)-1);
					}
					
					ret2=config_radio_qos_service(table_entry->parameter, connection,table_entry->QosID,TrafficClass,cwmin,cwmax,aifs,txoplimit,ack);

					if(ret2 == 1)
					{
						table_entry->dot11QoSTXOPLim = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					if(ret1==1)
					{
						Free_qos_one(wqos);
					}
            	}
                break;
            case COLUMN_DOT11AVGRATE:
				{
					int ret = 0;					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char para_value[10] = { 0 };
					memset(para_value,0,10);
					snprintf(para_value,sizeof(para_value)-1,"%d",*request->requestvb->val.integer);
					
					ret=wid_config_set_qos_flow_parameter(table_entry->parameter, connection,table_entry->QosID,TrafficClass,"averagerate",para_value);

					if(ret == 1)
					{
						table_entry->dot11AvgRate = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11MAXDEGREE:
				{
					int ret = 0;					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char para_value[10] = { 0 };
					memset(para_value,0,10);
					snprintf(para_value,sizeof(para_value)-1,"%d",*request->requestvb->val.integer);
					
					ret=wid_config_set_qos_flow_parameter(table_entry->parameter, connection,table_entry->QosID,TrafficClass,"maxburstiness",para_value);

					if(ret == 1)
					{
						table_entry->dot11MaxDegree = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11POLICYPRI:
				{
					int ret = 0;					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char para_value[10] = { 0 };
					memset(para_value,0,10);
					snprintf(para_value,sizeof(para_value)-1,"%d",*request->requestvb->val.integer);
					
					ret=wid_config_set_qos_flow_parameter(table_entry->parameter, connection,table_entry->QosID,TrafficClass,"managepriority",para_value);

					if(ret == 1)
					{
						table_entry->dot11PolicyPRI = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11RESSHOVEPRI:
				{
					int ret = 0;					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char para_value[10] = { 0 };
					memset(para_value,0,10);
					snprintf(para_value,sizeof(para_value)-1,"%d",*request->requestvb->val.integer);
					
					ret=wid_config_set_qos_flow_parameter(table_entry->parameter, connection,table_entry->QosID,TrafficClass,"shovepriority",para_value);

					if(ret == 1)
					{
						table_entry->dot11ResShovePRI = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11RESGRABPRI:
				{
					int ret = 0;					
					char TrafficClass[15] = { 0 };
					memset(TrafficClass,0,15);
					switch(table_entry->dot11QosTrafficClass)
					{
						case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							   break;
						case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							   break;
						case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							   break;
						case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							   break;
					}
					
					char para_value[10] = { 0 };
					memset(para_value,0,10);
					snprintf(para_value,sizeof(para_value)-1,"%d",*request->requestvb->val.integer);
					
					ret=wid_config_set_qos_flow_parameter(table_entry->parameter, connection,table_entry->QosID,TrafficClass,"grabpriority",para_value);

					if(ret == 1)
					{
						table_entry->dot11ResGrabPRI = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
            	}
                break;
            case COLUMN_DOT11MAXPALLEL:
                {
				  int ret = 0;
				  char TrafficClass[15] = {0};
				  memset(TrafficClass,0,sizeof(TrafficClass));
				  switch(table_entry->dot11QosTrafficClass)
				  {
					  case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							 break;
					  case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							 break;
					  case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							 break;
					  case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							 break;
				  }
				  
				  char para_value[10] = {0};
				  memset(para_value,0,sizeof(TrafficClass));
				  snprintf(para_value,sizeof(para_value)-1,"%d",*request->requestvb->val.integer);
                  ret = wid_config_set_qos_flow_parameter(table_entry->parameter, connection,table_entry->QosID,TrafficClass,"maxparallel",para_value);
				  if(1 == ret)
				  {
					table_entry->dot11MaxPallel = *request->requestvb->val.integer;
				  }
				  else
				  {
					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
				  }
			    }
                break;
            case COLUMN_DOT11BANDWIDTH:
                {
				  int ret = 0;
				  char TrafficClass[15] = {0};
				  memset(TrafficClass,0,sizeof(TrafficClass));
				  switch(table_entry->dot11QosTrafficClass)
				  {
					  case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							 break;
					  case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							 break;
					  case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							 break;
					  case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							 break;
				  }
				  
				  char para_value[10] = {0};
				  memset(para_value,0,10);
				  snprintf(para_value,sizeof(para_value)-1,"%d",*request->requestvb->val.integer);
				  
                  ret = wid_config_set_qos_flow_parameter(table_entry->parameter, connection,table_entry->QosID,TrafficClass,"bandwidth",para_value);

				  if(1 == ret)
				  {
					table_entry->dot11Bandwidth = *request->requestvb->val.integer;
				  }
				  else
				  {
					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
				  }
			    }
                break;
            case COLUMN_DOT11BANDWIDTHSCALE:
                {
				  int ret = 0;
				  char TrafficClass[15] = {0};
				  memset(TrafficClass,0,sizeof(TrafficClass));
				  switch(table_entry->dot11QosTrafficClass)
				  {
					  case 1:strncpy(TrafficClass,"besteffort",sizeof(TrafficClass)-1);
							 break;
					  case 2:strncpy(TrafficClass,"background",sizeof(TrafficClass)-1);
							 break;
					  case 3:strncpy(TrafficClass,"video",sizeof(TrafficClass)-1);
							 break;
					  case 4:strncpy(TrafficClass,"voice",sizeof(TrafficClass)-1);
							 break;
				  }
				  
				  char para_value[10] = {0};
				  memset(para_value,0,10);
				  snprintf(para_value,sizeof(para_value)-1,"%d",*request->requestvb->val.integer);
				  
                  ret = wid_config_set_qos_flow_parameter(table_entry->parameter, connection,table_entry->QosID,TrafficClass,"bandwidthpercentage",para_value);

				  if(1 == ret)
				  {
					table_entry->dot11BandwidthScale = *request->requestvb->val.integer;
				  }
				  else
				  {
					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
				  }
			    }
                break;
            case COLUMN_DOT11USEFLOWEQTQUEUE:
                /* Need to save old 'table_entry->dot11UseFlowEqtQueue' value.
                   May need to use 'memcpy' */
                //table_entry->old_dot11UseFlowEqtQueue = table_entry->dot11UseFlowEqtQueue;
              //  table_entry->dot11UseFlowEqtQueue     = request->requestvb->val.YYY;
                break;
            case COLUMN_DOT11SINGLEFLOWMAXQUEUE:
                /* Need to save old 'table_entry->dot11SingleFlowMaxQueue' value.
                   May need to use 'memcpy' */
                //table_entry->old_dot11SingleFlowMaxQueue = table_entry->dot11SingleFlowMaxQueue;
             //   table_entry->dot11SingleFlowMaxQueue     = request->requestvb->val.YYY;
                break;
            case COLUMN_DOT11FLOWAVGRATE:
                /* Need to save old 'table_entry->dot11FlowAvgRate' value.
                   May need to use 'memcpy' */
                //table_entry->old_dot11FlowAvgRate = table_entry->dot11FlowAvgRate;
             //   table_entry->dot11FlowAvgRate     = request->requestvb->val.YYY;
                break;
            case COLUMN_DOT11FLOWMAXDEGREE:
                /* Need to save old 'table_entry->dot11FlowMaxDegree' value.
                   May need to use 'memcpy' */
                //table_entry->old_dot11FlowMaxDegree = table_entry->dot11FlowMaxDegree;
              //  table_entry->dot11FlowMaxDegree     = request->requestvb->val.YYY;
                break;
            case COLUMN_DOT11USEWREDPOLICY:
                /* Need to save old 'table_entry->dot11UseWREDPolicy' value.
                   May need to use 'memcpy' */
                //table_entry->old_dot11UseWREDPolicy = table_entry->dot11UseWREDPolicy;
              //  table_entry->dot11UseWREDPolicy     = request->requestvb->val.YYY;
                break;
            case COLUMN_DOT11USETRAFFICSHAPING:
                /* Need to save old 'table_entry->dot11UseTrafficShaping' value.
                   May need to use 'memcpy' */
                //table_entry->old_dot11UseTrafficShaping = table_entry->dot11UseTrafficShaping;
              //  table_entry->dot11UseTrafficShaping     = request->requestvb->val.YYY;
                break;
			case COLUMN_QOSSVCPKTLOSSRATIO:
                /* Need to save old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->old_QoSSvcPktLossRatio= table_entry->QoSSvcPktLossRatio;
              //  table_entry->QoSSvcPktLossRatio     = request->requestvb->val.YYY;
                break;
			case COLUMN_PKTLOSSRATIO:
                /* Need to save old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->old_PktLossRatio= table_entry->PktLossRatio;
              //  table_entry->QoSSvcPktLossRatio     = request->requestvb->val.YYY;
                break;
			 case COLUMN_SVCLOSS:
                /* Need to save old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->old_SvcLoss= table_entry->SvcLoss;
              //  table_entry->QoSSvcPktLossRatio     = request->requestvb->val.YYY;
                break;
			 case COLUMN_QUEAVGLEN:
                /* Need to save old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->old_QueAvgLen= table_entry->QueAvgLen;
              //  table_entry->QoSSvcPktLossRatio     = request->requestvb->val.YYY;
                break;
			 case COLUMN_PUTTHROUGHRATIO:
                /* Need to save old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->old_PutThroughRatio= table_entry->PutThroughRatio;
              //  table_entry->QoSSvcPktLossRatio     = request->requestvb->val.YYY;
                break;
			 case COLUMN_DROPRATIO :
                /* Need to save old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->old_DropRatio= table_entry->DropRatio;
              //  table_entry->QoSSvcPktLossRatio     = request->requestvb->val.YYY;
                break;
			 case COLUMN_VOICEEXCEEDMAXUSERSREQUEST:
                /* Need to save old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->old_VoiceExceedMaxUsersRequest= table_entry->VoiceExceedMaxUsersRequest;
              //  table_entry->QoSSvcPktLossRatio     = request->requestvb->val.YYY;
                break;
			 case COLUMN_VIDEOEXCEEDMAXUSERSREQUEST:
                /* Need to save old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->old_VideoExceedMaxUsersRequest= table_entry->VideoExceedMaxUsersRequest;
              //  table_entry->QoSSvcPktLossRatio     = request->requestvb->val.YYY;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AcQosTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_DOT11QOSAIFS:
                /* Need to restore old 'table_entry->dot11QosAIFS' value.
                   May need to use 'memcpy' */
                //table_entry->dot11QosAIFS = table_entry->old_dot11QosAIFS;
                break;
            case COLUMN_DOT11QOSCWMIN:
                /* Need to restore old 'table_entry->dot11QoSCWmin' value.
                   May need to use 'memcpy' */
                //table_entry->dot11QoSCWmin = table_entry->old_dot11QoSCWmin;
                break;
            case COLUMN_DOT11QOSCWMAX:
                /* Need to restore old 'table_entry->dot11QoSCWmax' value.
                   May need to use 'memcpy' */
                //table_entry->dot11QoSCWmax = table_entry->old_dot11QoSCWmax;
                break;
            case COLUMN_DOT11QOSTXOPLIM:
                /* Need to restore old 'table_entry->dot11QoSTXOPLim' value.
                   May need to use 'memcpy' */
                //table_entry->dot11QoSTXOPLim = table_entry->old_dot11QoSTXOPLim;
                break;
            case COLUMN_DOT11AVGRATE:
                /* Need to restore old 'table_entry->dot11AvgRate' value.
                   May need to use 'memcpy' */
                //table_entry->dot11AvgRate = table_entry->old_dot11AvgRate;
                break;
            case COLUMN_DOT11MAXDEGREE:
                /* Need to restore old 'table_entry->dot11MaxDegree' value.
                   May need to use 'memcpy' */
                //table_entry->dot11MaxDegree = table_entry->old_dot11MaxDegree;
                break;
            case COLUMN_DOT11POLICYPRI:
                /* Need to restore old 'table_entry->dot11PolicyPRI' value.
                   May need to use 'memcpy' */
                //table_entry->dot11PolicyPRI = table_entry->old_dot11PolicyPRI;
                break;
            case COLUMN_DOT11RESSHOVEPRI:
                /* Need to restore old 'table_entry->dot11ResShovePRI' value.
                   May need to use 'memcpy' */
                //table_entry->dot11ResShovePRI = table_entry->old_dot11ResShovePRI;
                break;
            case COLUMN_DOT11RESGRABPRI:
                /* Need to restore old 'table_entry->dot11ResGrabPRI' value.
                   May need to use 'memcpy' */
                //table_entry->dot11ResGrabPRI = table_entry->old_dot11ResGrabPRI;
                break;
            case COLUMN_DOT11MAXPALLEL:
                /* Need to restore old 'table_entry->dot11MaxPallel' value.
                   May need to use 'memcpy' */
                //table_entry->dot11MaxPallel = table_entry->old_dot11MaxPallel;
                break;
            case COLUMN_DOT11BANDWIDTH:
                /* Need to restore old 'table_entry->dot11Bandwidth' value.
                   May need to use 'memcpy' */
                //table_entry->dot11Bandwidth = table_entry->old_dot11Bandwidth;
                break;
            case COLUMN_DOT11BANDWIDTHSCALE:
                /* Need to restore old 'table_entry->dot11BandwidthScale' value.
                   May need to use 'memcpy' */
                //table_entry->dot11BandwidthScale = table_entry->old_dot11BandwidthScale;
                break;
            case COLUMN_DOT11USEFLOWEQTQUEUE:
                /* Need to restore old 'table_entry->dot11UseFlowEqtQueue' value.
                   May need to use 'memcpy' */
                //table_entry->dot11UseFlowEqtQueue = table_entry->old_dot11UseFlowEqtQueue;
                break;
            case COLUMN_DOT11SINGLEFLOWMAXQUEUE:
                /* Need to restore old 'table_entry->dot11SingleFlowMaxQueue' value.
                   May need to use 'memcpy' */
                //table_entry->dot11SingleFlowMaxQueue = table_entry->old_dot11SingleFlowMaxQueue;
                break;
            case COLUMN_DOT11FLOWAVGRATE:
                /* Need to restore old 'table_entry->dot11FlowAvgRate' value.
                   May need to use 'memcpy' */
                //table_entry->dot11FlowAvgRate = table_entry->old_dot11FlowAvgRate;
                break;
            case COLUMN_DOT11FLOWMAXDEGREE:
                /* Need to restore old 'table_entry->dot11FlowMaxDegree' value.
                   May need to use 'memcpy' */
                //table_entry->dot11FlowMaxDegree = table_entry->old_dot11FlowMaxDegree;
                break;
            case COLUMN_DOT11USEWREDPOLICY:
                /* Need to restore old 'table_entry->dot11UseWREDPolicy' value.
                   May need to use 'memcpy' */
                //table_entry->dot11UseWREDPolicy = table_entry->old_dot11UseWREDPolicy;
                break;
            case COLUMN_DOT11USETRAFFICSHAPING:
                /* Need to restore old 'table_entry->dot11UseTrafficShaping' value.
                   May need to use 'memcpy' */
                //table_entry->dot11UseTrafficShaping = table_entry->old_dot11UseTrafficShaping;
                break;
			case COLUMN_QOSSVCPKTLOSSRATIO:
                /* Need to restore old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->QoSSvcPktLossRatio = table_entry->old_QoSSvcPktLossRatio;
                break;
			case COLUMN_PKTLOSSRATIO:
                /* Need to restore old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->PktLossRatio= table_entry->old_PktLossRatio;
                break;
		   	case COLUMN_SVCLOSS:
                /* Need to restore old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->SvcLoss= table_entry->old_SvcLoss;
                break;
			case COLUMN_QUEAVGLEN:
                /* Need to restore old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->QueAvgLen= table_entry->old_QueAvgLen;
                break;
			case COLUMN_PUTTHROUGHRATIO:
                /* Need to restore old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->PutThroughRatio= table_entry->old_PutThroughRatio;
                break;
			case COLUMN_DROPRATIO:
                /* Need to restore old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->DropRatio= table_entry->old_DropRatio;
                break;
			case COLUMN_VOICEEXCEEDMAXUSERSREQUEST:
                /* Need to restore old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->VoiceExceedMaxUsersRequest= table_entry->old_VoiceExceedMaxUsersRequest;
                break;
			case COLUMN_VIDEOEXCEEDMAXUSERSREQUEST:
                /* Need to restore old 'table_entry->QoSSvcPktLossRatio' value.
                   May need to use 'memcpy' */
                //table_entry->VideoExceedMaxUsersRequest= table_entry->old_VideoExceedMaxUsersRequest;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
