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
* dot11WidDetectHistoryTable.c
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
#include "dot11WidDetectHistoryTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_init_dbus.h"
#include "autelanWtpGroup.h"
#include "ws_dcli_ac.h"
#include "ws_dbus_list_interface.h"


#define WIDDETECT	"1.13.5"

    /* Typical data structure for a row entry */
struct dot11WidDetectHistoryTable_entry {
    /* Index values */
	dbus_parameter parameter;
    long deviceID;

    /* Column values */
    char *deviceMac;
    char *attackType;
    char *frameType;
    u_long attackCount;
    char *firstAttack;
    char *lastAttack;
	long RogStaMonApNum;
	char *RogStaAccBSSID;
	long RogStaMaxSigStrength;
	long RogStaChannel;
	long RogStaAdHocStatus;
	long RogStaAttackStatus;
	long old_RogStaAttackStatus;
	long RogStaToIgnore;
	long old_RogStaToIgnore;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WidDetectHistoryTable_entry *next;
};

void dot11WidDetectHistoryTable_load();
void
dot11WidDetectHistoryTable_removeEntry( struct dot11WidDetectHistoryTable_entry *entry );


/** Initializes the dot11WidDetectHistoryTable module */
void
init_dot11WidDetectHistoryTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WidDetectHistoryTable();
}

/** Initialize the dot11WidDetectHistoryTable table by defining its contents and how it's structured */
void
initialize_table_dot11WidDetectHistoryTable(void)
{
    static oid dot11WidDetectHistoryTable_oid[128] = {0};
    size_t dot11WidDetectHistoryTable_oid_len   = 0;
	mad_dev_oid(dot11WidDetectHistoryTable_oid,WIDDETECT,&dot11WidDetectHistoryTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WidDetectHistoryTable",     dot11WidDetectHistoryTable_handler,
              dot11WidDetectHistoryTable_oid, dot11WidDetectHistoryTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: deviceID */
                           0);
    table_info->min_column = COLUMN_WIDDETECTMIN;
    table_info->max_column = COLUMN_WIDDETECTMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WidDetectHistoryTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WidDetectHistoryTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WidDetectHistoryTable_load, dot11WidDetectHistoryTable_removeEntry,
						dot11WidDetectHistoryTable_oid, dot11WidDetectHistoryTable_oid_len));

    /* Initialise the contents of the table here */
}



struct dot11WidDetectHistoryTable_entry  *dot11WidDetectHistoryTable_head;

/* create a new row in the (unsorted) table */
struct dot11WidDetectHistoryTable_entry *
dot11WidDetectHistoryTable_createEntry(
								dbus_parameter parameter,
								long  deviceID,
								char *deviceMac,
								char *attackType,
								char *frameType,
								u_long attackCount,
								char *firstAttack,
								char *lastAttack,
								long RogStaMonApNum,
								char *RogStaAccBSSID,
								long RogStaMaxSigStrength,
								long RogStaChannel,
								long RogStaAdHocStatus,
								long RogStaAttackStatus,
								long RogStaToIgnore) {
    struct dot11WidDetectHistoryTable_entry *entry;
    entry = SNMP_MALLOC_TYPEDEF(struct dot11WidDetectHistoryTable_entry);
    if (!entry)
        return NULL;
	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->deviceID = deviceID;
	entry->deviceMac = strdup(deviceMac);
	entry->attackType = strdup(attackType);
	entry->frameType = strdup(frameType);
	entry->attackCount = attackCount;
	entry->firstAttack = strdup(firstAttack);
	entry->lastAttack = strdup(lastAttack);
	entry->RogStaMonApNum = RogStaMonApNum;
	entry->RogStaAccBSSID = strdup(RogStaAccBSSID);
	entry->RogStaMaxSigStrength = RogStaMaxSigStrength;
	entry->RogStaChannel = RogStaChannel;
	entry->RogStaAdHocStatus = RogStaAdHocStatus;
	entry->RogStaAttackStatus = RogStaAttackStatus;
	entry->RogStaToIgnore = RogStaToIgnore;
    entry->next = dot11WidDetectHistoryTable_head;
    dot11WidDetectHistoryTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WidDetectHistoryTable_removeEntry( struct dot11WidDetectHistoryTable_entry *entry ) {
    struct dot11WidDetectHistoryTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WidDetectHistoryTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WidDetectHistoryTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->deviceMac);
	FREE_OBJECT(entry->firstAttack);
	FREE_OBJECT(entry->lastAttack);
	FREE_OBJECT(entry->attackType);
	FREE_OBJECT(entry->frameType);
	FREE_OBJECT(entry->RogStaAccBSSID);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11WidDetectHistoryTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11WidDetectHistoryTable_load\n");

	struct dot11WidDetectHistoryTable_entry *temp = NULL;
	while( dot11WidDetectHistoryTable_head )
	{
		temp=dot11WidDetectHistoryTable_head->next;
		dot11WidDetectHistoryTable_removeEntry(dot11WidDetectHistoryTable_head);
		dot11WidDetectHistoryTable_head=temp;
	}
	
	unsigned char firsttime[128] = { 0 };
	unsigned char lasttime[128] = { 0 };
	char mac[20] = { 0 };		
	char attackType[WIDS_TYPE_LEN] = { 0 };
	char frameType[WIDS_TYPE_LEN] = { 0 };
	char bssid[20] = { 0 };
    int i = 0;
		
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wids_device_list_cmd_func\n");
    messageHead = list_connection_call_dbus_method(show_wids_device_list_cmd_func, SHOW_ALL_WTP_TABLE_METHOD);
	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wids_device_list_cmd_func,messageHead=%p\n", messageHead);

	if(messageHead)
	{
		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
		{
		    DCLI_AC_API_GROUP_TWO *LIST = messageNode->message;
		    
			if((LIST) && (LIST->wids_device_list))
			{
				int j = 0;
				int len = LIST->wids_device_list->count;
				struct tag_wids_device_ele *head = NULL;
				for(j=0,head = LIST->wids_device_list->wids_device_info;
					((j<len)&&(NULL != head));
					j++,head = head->next)
				{
					memset(mac,0,20);
					snprintf(mac,sizeof(mac)-1,
							 "%02X:%02X:%02X:%02X:%02X:%02X",
							 head->bssid[0],head->bssid[1],
							 head->bssid[2],head->bssid[3],
							 head->bssid[4],head->bssid[5]);
					
					memset(attackType,0,WIDS_TYPE_LEN);
					memset(frameType,0,WIDS_TYPE_LEN);
					CheckWIDSType(attackType,
									frameType,
									head->attacktype,
									head->frametype);

					memset(firsttime,0,128);
					snprintf(firsttime,127,"%s",ctime(&head->fst_attack));
					delete_enter(firsttime);
					memset(lasttime,0,128);
					snprintf(lasttime,127,"%s",ctime(&head->lst_attack));
					delete_enter(lasttime);
					
					memset(bssid,0,20);
					snprintf(bssid,sizeof(bssid)-1,
							 "%02X:%02X:%02X:%02X:%02X:%02X",
							 head->vapbssid[0],head->vapbssid[1],
							 head->vapbssid[2],head->vapbssid[3],
							 head->vapbssid[4],head->vapbssid[5]);
					
					dot11WidDetectHistoryTable_createEntry(messageNode->parameter,
															++i,
															mac,
															attackType,
															frameType,
															head->attackcount,
															firsttime,
															lasttime,
															1,
															bssid,
															head->rssi,
															head->channel,
															2,
															0,
															2);
				}
			}			
		}
		free_dbus_message_list(&messageHead, Free_wids_device_head);
	}

	snmp_log(LOG_DEBUG, "exit dot11WidDetectHistoryTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WidDetectHistoryTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11WidDetectHistoryTable_head==NULL)
			return NULL;
	*my_loop_context = dot11WidDetectHistoryTable_head;
	*my_data_context = dot11WidDetectHistoryTable_head;
    return dot11WidDetectHistoryTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WidDetectHistoryTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WidDetectHistoryTable_entry *entry = (struct dot11WidDetectHistoryTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx,(u_char *)&entry->deviceID, sizeof(long));
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WidDetectHistoryTable table */
int
dot11WidDetectHistoryTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WidDetectHistoryTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WidDetectHistoryTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}     
            switch (table_info->colnum) {
            case COLUMN_DEVICEID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->deviceID,
                                          sizeof(long));
                break;
            case COLUMN_DEVICEMAC:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->deviceMac,
                                          strlen(table_entry->deviceMac));
                break;
            case COLUMN_ATTACKTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->attackType,
                                          strlen(table_entry->attackType));
                break;
            case COLUMN_FRAMETYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->frameType,
                                          strlen(table_entry->frameType));
                break;
            case COLUMN_ATTACKCOUNT:
                snmp_set_var_typed_value( request->requestvb, ASN_COUNTER,
                                          (u_char *)&table_entry->attackCount,
                                          sizeof(u_long));
                break;
            case COLUMN_FIRSTATTACK:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->firstAttack,
                                          strlen(table_entry->firstAttack));
                break;
            case COLUMN_LASTATTACK:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->lastAttack,
                                          strlen(table_entry->lastAttack));
                break;
			case COLUMN_ROGSTAMONAPNUM:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->RogStaMonApNum,
                                          sizeof(long));
                break;
			case COLUMN_ROGSTAACCBSSID:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char *)table_entry->RogStaAccBSSID,
                                          strlen(table_entry->RogStaAccBSSID));
                break;
			case COLUMN_ROGSTAMAXSIGSTRENGTH:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->RogStaMaxSigStrength,
                                          sizeof(long));
                break;
			case COLUMN_ROGSTACHANNEL:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->RogStaChannel,
                                          sizeof(long));
                break;
			case COLUMN_ROGSTAADHOCSTATUS:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->RogStaAdHocStatus,
                                          sizeof(long));
                break;
			case COLUMN_ROGSTAATTACKSTATUS:
			{
			    void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
                    break;
                    
				int RogStaAttackStatus = 2;
				int ret = 0;
				DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;   

				ret = show_ap_wids_set_cmd_func(table_entry->parameter, connection, &WTPINFO);
				if(ret==1)
				{
					if(WTPINFO->wids.weakiv==1)
						RogStaAttackStatus = 1; 
					else
						RogStaAttackStatus = 2;
				}
				else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
        	    }
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&RogStaAttackStatus,
                                          sizeof(RogStaAttackStatus));
				if(ret==1)
				{
					free_show_ap_wids_set_cmd(WTPINFO);
				}
			}
                break;
			case COLUMN_ROGSTATOIGNORE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char *)&table_entry->RogStaToIgnore,
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
            table_entry = (struct dot11WidDetectHistoryTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_ROGSTAATTACKSTATUS:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_ROGSTATOIGNORE:
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
            table_entry = (struct dot11WidDetectHistoryTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_ROGSTAATTACKSTATUS:
			{
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                    return MFD_ERROR;
                    
				int ret = 0;

				if(*request->requestvb->val.integer==1)
				{
					ret = set_wtp_wids_policy_cmd(table_entry->parameter, connection,"forbid");
					if(ret != 1)
					{
					    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(table_entry->parameter.slot_id);
                	    }
						netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
					}
				}
				else if(*request->requestvb->val.integer==2)
				{
					ret = set_wtp_wids_policy_cmd(table_entry->parameter, connection,"no");
					if(ret != 1)
					{
					    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(table_entry->parameter.slot_id);
                	    }
						netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
					}
				}
				else
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
				}

        	}
                break;
			case COLUMN_ROGSTATOIGNORE:
			{
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                    return MFD_ERROR;
                    
				int ret = 0;

				if(*request->requestvb->val.integer==1)
				{
					ret = add_wids_mac_cmd(table_entry->parameter, connection,table_entry->deviceMac);	
					if(ret != 1)
					{
					    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(table_entry->parameter.slot_id);
                	    }
						netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
					}
				}
				else if(*request->requestvb->val.integer!=2)
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
				}
        	}
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WidDetectHistoryTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_ROGSTAATTACKSTATUS:
                /* Need to restore old 'table_entry->RogStaAttackStatus' value.
                   May need to use 'memcpy' */
                table_entry->RogStaAttackStatus= table_entry->old_RogStaAttackStatus;
                break;
			case COLUMN_ROGSTATOIGNORE:
                /* Need to restore old 'table_entry->RogStaToIgnore' value.
                   May need to use 'memcpy' */
                table_entry->RogStaToIgnore= table_entry->old_RogStaToIgnore;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;

    }
    return SNMP_ERR_NOERROR;
}
