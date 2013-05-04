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
* dot11RadioProfileTable.c
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
#include "dot11RadioProfileTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "autelanWtpGroup.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

#define 	HIDDENRadio	"6.1.2"

    /* Typical data structure for a row entry */
struct dot11RadioProfileTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalRadioID;
    long RadioID;

    /* Column values */
    long RadioChannel;
    long RadioTxpower;
    long RadioRxpower;
    char* RadioRate;
    char* RadioType;
    char* RadioBindWLANID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11RadioProfileTable_entry *next;
};
void dot11RadioProfileTable_load(void);
void dot11RadioProfileTable_removeEntry( struct dot11RadioProfileTable_entry *entry );

/** Initializes the dot11RadioProfileTable module */
void
init_dot11RadioProfileTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11RadioProfileTable();
}

/** Initialize the dot11RadioProfileTable table by defining its contents and how it's structured */
void
initialize_table_dot11RadioProfileTable(void)
{
    static oid dot11RadioProfileTable_oid[128] = {0};
    size_t dot11RadioProfileTable_oid_len   = 	0;
	mad_dev_oid(dot11RadioProfileTable_oid,HIDDENRadio,&dot11RadioProfileTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11RadioProfileTable",     dot11RadioProfileTable_handler,
              dot11RadioProfileTable_oid, dot11RadioProfileTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalRadioID */
                           0);
    table_info->min_column = COLUMN_HIDDENRADIOMIN;
    table_info->max_column = COLUMN_HIDDENRADIOMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11RadioProfileTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11RadioProfileTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11RadioProfileTable_load, dot11RadioProfileTable_removeEntry,dot11RadioProfileTable_oid, dot11RadioProfileTable_oid_len));

    /* Initialise the contents of the table here */
}


struct dot11RadioProfileTable_entry  *dot11RadioProfileTable_head;

/* create a new row in the (unsorted) table */
struct dot11RadioProfileTable_entry *
dot11RadioProfileTable_createEntry(				 
				 dbus_parameter parameter,
				 long globalRadioID,
                 long  RadioID,
			     long RadioChannel,
			     long RadioTxpower,
			     long RadioRxpower,
			     char* RadioRate,
			     char* RadioType,
			     char* RadioBindWLANID
                ) {
    struct dot11RadioProfileTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11RadioProfileTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->globalRadioID = globalRadioID;
    entry->RadioID = RadioID;
	entry->RadioChannel = RadioChannel;
	entry->RadioTxpower = RadioTxpower;
	entry->RadioRxpower = RadioRxpower;	
	entry->RadioRate = strdup(RadioRate);
	entry->RadioType = strdup(RadioType);
	entry->RadioBindWLANID = strdup(RadioBindWLANID);
    entry->next = dot11RadioProfileTable_head;
    dot11RadioProfileTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11RadioProfileTable_removeEntry( struct dot11RadioProfileTable_entry *entry ) {
    struct dot11RadioProfileTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11RadioProfileTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11RadioProfileTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->RadioRate);
	FREE_OBJECT(entry->RadioType);
	FREE_OBJECT(entry->RadioBindWLANID);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
void dot11RadioProfileTable_load()
{
	
	snmp_log(LOG_DEBUG, "enter dot11RadioProfileTable_load\n");
	
	struct dot11RadioProfileTable_entry *temp = NULL; 
	while( dot11RadioProfileTable_head ){
		temp=dot11RadioProfileTable_head->next;
		dot11RadioProfileTable_removeEntry(dot11RadioProfileTable_head);
		dot11RadioProfileTable_head=temp;
	}	

	snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_radio_list\n");
    messageHead = list_connection_call_dbus_method(show_radio_list, SHOW_ALL_WTP_TABLE_METHOD);
	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_radio_list,messageHead=%p\n", messageHead);

	
	if(messageHead)
	{
		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
		{
		    DCLI_RADIO_API_GROUP_ONE *radio = messageNode->message;
		    if(radio)
		    {
		        int i = 0;
		        for(i=0;i<radio->radio_num;i++)
        		{        		    
					if(radio->RADIO[i])
					{
						unsigned long globalRadioID = local_to_global_ID(messageNode->parameter, 
																		 radio->RADIO[i]->Radio_G_ID, 
																		 WIRELESS_MAX_NUM);
						
						char RType[10] = { 0 };						
						memset(RType,0,sizeof(RType));
						Radio_Type(radio->RADIO[i]->Radio_Type,RType);
						
						dot11RadioProfileTable_createEntry(messageNode->parameter,
														   globalRadioID,
														   radio->RADIO[i]->Radio_G_ID,
														   radio->RADIO[i]->Radio_Chan,
														   radio->RADIO[i]->Radio_TXP,
														   radio->RADIO[i]->Radio_TXP,
														   "",
														   RType,
														   ""
														   );
					}
        		}
        	}
        }
        free_dbus_message_list(&messageHead, Free_radio_head);
    }    

	snmp_log(LOG_DEBUG, "exit dot11RadioProfileTable_load\n");
}

netsnmp_variable_list *
dot11RadioProfileTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{	
	if(dot11RadioProfileTable_head == NULL)
	{
		return NULL;
	}
    *my_loop_context = dot11RadioProfileTable_head;
    return dot11RadioProfileTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11RadioProfileTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11RadioProfileTable_entry *entry = (struct dot11RadioProfileTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {		
        snmp_set_var_value( idx, (u_char *)&entry->globalRadioID, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }	
	return put_index_data;
}


/** handles requests for the dot11RadioProfileTable table */
int
dot11RadioProfileTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11RadioProfileTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11RadioProfileTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
			
			if( !table_entry ){
				netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
			}	  
    
            switch (table_info->colnum) {
            case COLUMN_RADIOID:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->globalRadioID,
                                          sizeof(long));
            }
				break;
            case COLUMN_RADIOCHANNEL:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->RadioChannel,
                                          sizeof(long));
            }
				break;
            case COLUMN_RADIOTXPOWER:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->RadioTxpower,
                                          sizeof(long));
            }
				break;
            case COLUMN_RADIORXPOWER:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->RadioRxpower,
                                          sizeof(long));
           	}
				break;
            case COLUMN_RADIORATE:
			{
				int i,ret=0;				
	            DCLI_RADIO_API_GROUP_ONE  *rad = NULL;
				char radio_rate[100] = { 0 };
				char rate[10] = { 0 };

                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                
				ret=show_radio_one(table_entry->parameter, connection,table_entry->RadioID,&rad);
				if((ret == 1)&&(rad->RADIO[0]))
				{
					memset(radio_rate,0,100);
					for (i=0;i<(rad->RADIO[0]->Support_Rate_Count);i++)
					{
						memset(rate,0,10);
						snprintf(rate,sizeof(rate)-1,"%0.1f",(*(rad->RADIO[0]->RadioRate[i]))/10.0);
						strncat(radio_rate,rate,sizeof(radio_rate)-strlen(radio_rate)-1);
						strncat(radio_rate,",",sizeof(radio_rate)-strlen(radio_rate)-1);
					}
				}
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)radio_rate,
                                          strlen(radio_rate));
				
				if(ret==1)
				{
					Free_radio_one_head(rad);
				}
            }
				break;
            case COLUMN_RADIOTYPE:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->RadioType,
                                          strlen(table_entry->RadioType));
            }
				break;
            case COLUMN_RADIOBINDWLANID:
			{
				int i,ret=0;
	            DCLI_RADIO_API_GROUP_ONE  *radio = NULL;
				char bwlanid[1024] = { 0 };
				char tembwid[5];
                
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;

				ret=show_radio_one(table_entry->parameter, connection,table_entry->RadioID,&radio);
				if(ret == 1)
				{
					memset(bwlanid,0,1024);
	    			for(i=0;i<radio->wlan_num;i++)			
	    			{
	    			  memset(tembwid,0,5);
					  if((radio->RADIO[0])&&(radio->RADIO[0]->WlanId != NULL))
					  {
						  if(i==radio->wlan_num-1)
							snprintf(tembwid,sizeof(tembwid)-1,"%d",radio->RADIO[0]->WlanId[i]);	/*int转成char*/
						  else
							snprintf(tembwid,sizeof(tembwid)-1,"%d,",radio->RADIO[0]->WlanId[i]);	/*int转成char*/
					  }
	    			  strncat(bwlanid,tembwid,sizeof(bwlanid)-strlen(bwlanid)-1);
	    			}
				}
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)bwlanid,
                                          strlen(bwlanid));
				
				if(ret==1)
				{
					Free_radio_one_head(radio);
				}
            }
				break;
            }
        }
        break;

    }
    return SNMP_ERR_NOERROR;
}
