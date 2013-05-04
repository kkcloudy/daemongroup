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
* dot11SnmpServiceTable.c
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
#include "dot11SnmpServiceTable.h"
#include "autelanWtpGroup.h"

#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "ws_init_dbus.h"

/** Initializes the dot11SnmpServiceTable module */

#define DOT11SNMPSERVICESTABLE "2.6.1"
    /* Typical data structure for a row entry */
struct dot11SnmpServiceTable_entry {
    /* Index values */
    long communityID;

    /* Column values */
    char *communityName;
    //char *old_communityName;
    long communityPermission;

    STCommunity communityNode;
    //long old_communityPermission;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11SnmpServiceTable_entry *next;
};

void dot11SnmpServiceTable_load();
void
dot11SnmpServiceTable_removeEntry( struct dot11SnmpServiceTable_entry *entry );

void
init_dot11SnmpServiceTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11SnmpServiceTable();
}

/** Initialize the dot11SnmpServiceTable table by defining its contents and how it's structured */
void
initialize_table_dot11SnmpServiceTable(void)
{
    static oid dot11SnmpServiceTable_oid[128] = {0};
    size_t dot11SnmpServiceTable_oid_len   = 0;
	mad_dev_oid(dot11SnmpServiceTable_oid,DOT11SNMPSERVICESTABLE,&dot11SnmpServiceTable_oid_len,enterprise_pvivate_oid);

    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;
	
    reg = netsnmp_create_handler_registration(
              "dot11SnmpServiceTable",     dot11SnmpServiceTable_handler,
              dot11SnmpServiceTable_oid, dot11SnmpServiceTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: communityID */
                           0);
    table_info->min_column = SNMPSERVICETABLE_MIN;
    table_info->max_column = SNMPSERVICETABLE_MAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11SnmpServiceTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11SnmpServiceTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11SnmpServiceTable_load, dot11SnmpServiceTable_removeEntry,
							dot11SnmpServiceTable_oid, dot11SnmpServiceTable_oid_len));

    /* Initialise the contents of the table here */
}


struct dot11SnmpServiceTable_entry  *dot11SnmpServiceTable_head;

/* create a new row in the (unsorted) table */
struct dot11SnmpServiceTable_entry *
dot11SnmpServiceTable_createEntry(
                 long  communityID,
                  char *communityName,
                  STCommunity *communityNode,
  			    long communityPermission
                ) {
    struct dot11SnmpServiceTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11SnmpServiceTable_entry);
    if (!entry)
        return NULL;

    entry->communityID = communityID;
	entry->communityName = strdup(communityName);

	if(communityNode) {
        memcpy(&entry->communityNode, communityNode, sizeof(STCommunity));
	}
	
	entry->communityPermission = communityPermission;
    entry->next = dot11SnmpServiceTable_head;
    dot11SnmpServiceTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11SnmpServiceTable_removeEntry( struct dot11SnmpServiceTable_entry *entry ) {
    struct dot11SnmpServiceTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11SnmpServiceTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11SnmpServiceTable_head = ptr->next;
    else
        prev->next = ptr->next;
   FREE_OBJECT(entry->communityName);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11SnmpServiceTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11SnmpServiceTable_load\n");

	struct dot11SnmpServiceTable_entry *temp = NULL;	
	while(dot11SnmpServiceTable_head) {
		  temp=dot11SnmpServiceTable_head->next;
		  dot11SnmpServiceTable_removeEntry(dot11SnmpServiceTable_head);
		  dot11SnmpServiceTable_head=temp;
	}

    int ret = AC_MANAGE_SUCCESS;

    STCommunity *community_array = NULL;
    unsigned int community_num = 0;
    
    ret = ac_manage_show_snmp_community(ccgi_dbus_connection, &community_array, &community_num);
    if(AC_MANAGE_SUCCESS == ret) {
        int i = 0;
        for (i = 0; i < community_num; i++) {

            dot11SnmpServiceTable_createEntry( i + 1,
                                                community_array[i].community,
                                                &(community_array[i]),
                                                community_array[i].access_mode);
        }
    }
    SNMP_FREE(community_array);
    
	snmp_log(LOG_DEBUG, "exit dot11SnmpServiceTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11SnmpServiceTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{

	if(dot11SnmpServiceTable_head==NULL)
			return NULL;
	*my_data_context = dot11SnmpServiceTable_head;
    *my_loop_context = dot11SnmpServiceTable_head;
    return dot11SnmpServiceTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11SnmpServiceTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11SnmpServiceTable_entry *entry = (struct dot11SnmpServiceTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx,(u_char*)&entry->communityID, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11SnmpServiceTable table */
int
dot11SnmpServiceTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11SnmpServiceTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SnmpServiceTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }   
	
            switch (table_info->colnum) {
            case COLUMN_COMMUNITYID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->communityID,
                                          sizeof(long));
                break;
            case COLUMN_COMMUNITYNAME:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->communityName,
                                          strlen(table_entry->communityName));
                break;
            case COLUMN_COMMUNITYPERMISSION:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->communityPermission,
                                          sizeof(long));
                break;
			 default:	
                netsnmp_set_request_error( reqinfo, request,
                                           SNMP_ERR_NOTWRITABLE );
                return SNMP_ERR_NOERROR;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
		#if 0
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SnmpServiceTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_COMMUNITYNAME:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_COMMUNITYPERMISSION:
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
		#endif
        break;

    case MODE_SET_RESERVE2:
        break;

    case MODE_SET_FREE:
        break;

    case MODE_SET_ACTION:
    { 

        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SnmpServiceTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
    		
            switch (table_info->colnum) {
            case COLUMN_COMMUNITYNAME:
    			{
                    char * input_string = (char *)malloc(request->requestvb->val_len + 1);
    				memset(input_string, 0, request->requestvb->val_len+1);
    				strncpy(input_string, request->requestvb->val.string, request->requestvb->val_len);
    				
                  	if( strlen(input_string) >= sizeof(table_entry->communityNode.community)) {
            			netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
                        SNMP_FREE(input_string);
            			break;//超过了最大长度，
            		}
            		if(input_string)
                  		strncpy(table_entry->communityNode.community, input_string, sizeof(table_entry->communityNode.community) - 1 );

                  	int ret = ac_manage_config_snmp_set_community(ccgi_dbus_connection, table_entry->communityName, &table_entry->communityNode);
                  	if(AC_MANAGE_SUCCESS == ret) {
                        SNMP_FREE(table_entry->communityName);
                        table_entry->communityName = strdup(table_entry->communityNode.community);
                  	}
                    else {
            			netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
                    }
                    
    				SNMP_FREE(input_string);
                }
                break;
            case COLUMN_COMMUNITYPERMISSION:
                {
                    table_entry->communityNode.access_mode = *request->requestvb->val.integer;
                  	int ret = ac_manage_config_snmp_set_community(ccgi_dbus_connection, table_entry->communityName, &table_entry->communityNode);
                  	if(AC_MANAGE_SUCCESS == ret) {
                  	    table_entry->communityPermission = table_entry->communityNode.access_mode;
                  	}
                    else {
            			netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
                    }
                }    
                break;
            default:
            	
            	break;
            }
            
        }
   }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SnmpServiceTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_COMMUNITYNAME:
                /* Need to restore old 'table_entry->communityName' value.
                   May need to use 'memcpy' */
             //   table_entry->communityName = table_entry->old_communityName;
                break;
            case COLUMN_COMMUNITYPERMISSION:
                /* Need to restore old 'table_entry->communityPermission' value.
                   May need to use 'memcpy' */
             //   table_entry->communityPermission = table_entry->old_communityPermission;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
