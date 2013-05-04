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
 * dot11StationinfoTable.c
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
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sta.h"  
#include "ws_user_manage.h"
#include "dot11StationinfoTable.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#include "nm/app/eag/eag_conf.h"
#include "nm/public/nm_list.h"
#include "nm/app/eag/eag_interface.h"
#include "nm/app/eag/eag_errcode.h"


#define   STATIONINFOTABLE "2.17.1"


struct dot11StationinfoTable_entry {
	/* Index values */
	dbus_parameter parameter;
	char *StationMac;
	/* Column values */
	long StationOnline;

	/* Illustrate using a simple linked list */
	int   valid;
	struct dot11StationinfoTable_entry *next;
};
void dot11StationinfoTable_load();
void dot11StationinfoTable_removeEntry( struct dot11StationinfoTable_entry *entry );
/** Initializes the dot11StationinfoTable module */
void
init_dot11StationinfoTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
  initialize_table_dot11StationinfoTable();
}

/** Initialize the dot11StationinfoTable table by defining its contents and how it's structured */
void
initialize_table_dot11StationinfoTable(void)
{   
    static oid dot11StationinfoTable_oid[128] = {0};
    size_t dot11StationinfoTable_oid_len   = 0;
	mad_dev_oid(dot11StationinfoTable_oid,STATIONINFOTABLE,&dot11StationinfoTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration( 
              "dot11StationinfoTable",     dot11StationinfoTable_handler,
              dot11StationinfoTable_oid, dot11StationinfoTable_oid_len,
             HANDLER_CAN_RONLY
              );
    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: StationMac */
                           0);
	
    table_info->min_column = STATIONINFOTABLE_MIN_COLUMN;
    table_info->max_column = STATIONINFOTABLE_MAX_COLUMN;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11StationinfoTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11StationinfoTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
	
    netsnmp_register_table_iterator( reg, iinfo );

	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11StationinfoTable_load, dot11StationinfoTable_removeEntry,dot11StationinfoTable_oid, dot11StationinfoTable_oid_len));
	
    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct dot11StationinfoTable_entry  *dot11StationinfoTable_head;

/* create a new row in the (unsorted) table */
struct dot11StationinfoTable_entry *
dot11StationinfoTable_createEntry(
				 dbus_parameter parameter,
                 char *StationMac,
                 long StationOnline
                ) {
    struct dot11StationinfoTable_entry *entry;
    entry = SNMP_MALLOC_TYPEDEF(struct dot11StationinfoTable_entry);
    if (!entry)
        return NULL;

   	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
    entry->StationMac = strdup(StationMac);
	entry->StationOnline = StationOnline;
    entry->next = dot11StationinfoTable_head;
    dot11StationinfoTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11StationinfoTable_removeEntry( struct dot11StationinfoTable_entry *entry ) {
    struct dot11StationinfoTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11StationinfoTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11StationinfoTable_head = ptr->next;
    else
        prev->next = ptr->next;    
	FREE_OBJECT(entry->StationMac);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11StationinfoTable_load()
{   
	snmp_log(LOG_DEBUG, "enter dot11StationinfoTable_load\n");

 	struct dot11StationinfoTable_entry *temp = NULL; 
	while( dot11StationinfoTable_head ){
		temp=dot11StationinfoTable_head->next;
		dot11StationinfoTable_removeEntry(dot11StationinfoTable_head);
		dot11StationinfoTable_head=temp;
	}	
	
	 char mac[30] = { 0 };
     snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
     
     snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_station_list_by_group\n");
     messageHead = list_connection_call_dbus_method(show_station_list_by_group, SHOW_ALL_WTP_TABLE_METHOD);
     snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_station_list_by_group,messageHead=%p\n", messageHead);
     
     if(messageHead)
     {
         for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
         {
            struct dcli_ac_info *ac = messageNode->message;
            if(ac && ac->num_bss_wireless > 0)
            {
                struct dcli_bss_info *bss = NULL;
                for(bss = ac->bss_list; NULL != bss; bss = bss->next)
                {
                    struct dcli_sta_info *sta = NULL;
                    for(sta = bss->sta_list; NULL != sta; sta = sta->next)
                    {
                        int is1xuser = 2; /*2表示不在线*/
                        memset(mac,0,sizeof(mac));
                        snprintf(mac,sizeof(mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STRZ(sta->addr));

                        unsigned char ieee80211_state[20], PAE[20], BACKEND[20];
                        memset(ieee80211_state, 0, sizeof(ieee80211_state));
                        memset(PAE, 0, sizeof(PAE));
                        memset(BACKEND, 0, sizeof(BACKEND));
                        asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);			
                        if(strcmp(PAE,"PAE_AUTHENTICATED")==0) {   
                            is1xuser = 1; /*1表示在线*/
                        }
                        
                        dot11StationinfoTable_createEntry(messageNode->parameter,
                                         				  mac,
                                         				  is1xuser); 
                                             				  
                       
                    }	
                }
	        }
        }
        free_dbus_message_list(&messageHead, Free_sta_summary);
    }

     	instance_parameter *paraHead = NULL;
     	instance_parameter *pq = NULL;
	int ret = 0;
	struct eag_userdb userdb = {0};
	struct eag_user *user = NULL;
	int PortalOnlineUsers = 0;
	char webmac[20] = {0};

	list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
	for(pq = paraHead; (NULL != pq); pq = pq->next)
	{
		eag_userdb_init(&userdb);
		ret = eag_show_user_all(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&userdb);
		if(EAG_RETURN_OK == ret)
		{
			list_for_each_entry(user, &(userdb.head), node)
			{
				snprintf(webmac,sizeof(webmac)-1,"%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", 
					user->usermac[0], user->usermac[1], user->usermac[2], user->usermac[3], user->usermac[4], user->usermac[5]);
		 		dbus_parameter parameter;
		 		memset(&parameter, 0, sizeof(dbus_parameter));
			 	dot11StationinfoTable_createEntry(parameter, webmac, 1); 
			}	
		}
		eag_userdb_destroy(&userdb);
	}
	free_instance_parameter_list(&paraHead);
    	#if 0
	 /*web portal all mac*/	
	 char webmac[20] = { 0 };
	 int  start_f = 0;
	 int  end_f = 9; 
	 int flag = 0;
	 while(!flag)
	 {
		 STUserManagePkg * tableUserInfo = show_auth_users_info( &start_f , &end_f );
		 if( tableUserInfo == NULL )
		 	break;

		 if( tableUserInfo->data.ack.user_num_in_pkg == 0 )
		 {
			flag = 1;
			FREE_OBJECT(tableUserInfo);
			break;
		 }
		 int i = 0;
		 for( i=0; i<tableUserInfo->data.ack.user_num_in_pkg; i++ )
		 {  
		    memset(webmac,0,sizeof(webmac));
		 	snprintf(webmac,sizeof(webmac)-1,"%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",tableUserInfo->data.ack.users[i].usermac[0],tableUserInfo->data.ack.users[i].usermac[1],tableUserInfo->data.ack.users[i].usermac[2],tableUserInfo->data.ack.users[i].usermac[3],tableUserInfo->data.ack.users[i].usermac[4],tableUserInfo->data.ack.users[i].usermac[5] );
		 	dbus_parameter parameter;
		 	memset(&parameter, 0, sizeof(dbus_parameter));
			 dot11StationinfoTable_createEntry(parameter,
                                			 	webmac,
                                			 	1); 
		 }
		 if( tableUserInfo->data.ack.user_num_in_pkg < 10 )
		 {
		 	break;
		 }
		 start_f += 10;
		 end_f   += 10;
		 FREE_OBJECT(tableUserInfo);
	 }
	#endif
	 snmp_log(LOG_DEBUG, "exit dot11StationinfoTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11StationinfoTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{   
	if (dot11StationinfoTable_head == NULL) 
			return NULL;
	    *my_data_context = dot11StationinfoTable_head;
	    *my_loop_context = dot11StationinfoTable_head;
    return dot11StationinfoTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11StationinfoTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11StationinfoTable_entry *entry = (struct dot11StationinfoTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)entry->StationMac, strlen(entry->StationMac));
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
    	return put_index_data; 
}


/** handles requests for the dot11StationinfoTable table */

int
dot11StationinfoTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {
    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11StationinfoTable_entry          *table_entry; 
    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET: 

        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11StationinfoTable_entry *)
                              netsnmp_extract_iterator_context(request);

            table_info  =     netsnmp_extract_table_info(      request);

		 if( !table_entry )
			{  
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
				}
            switch (table_info->colnum) {
            case COLUMN_STATIONMAC:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->StationMac,
                                          strlen(table_entry->StationMac));
                break;
            case COLUMN_STATIONONLINE:	
			   snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
		                                          (u_char*)&table_entry->StationOnline,
		                                          sizeof(long));		
                break;
            }    
        }
        break;
    }
    return SNMP_ERR_NOERROR; 
}

