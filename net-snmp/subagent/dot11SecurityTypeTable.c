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
* dot11SecurityTypeTable.c
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
#if 0

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "dot11SecurityTypeTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "ws_security.h"
#include "ws_sta.h"
#include "mibs_public.h"
#include "autelanWtpGroup.h"

/** Initializes the dot11SecurityTypeTable module */
#define DOT11SECURITYTYPE "2.14.2"

void
init_dot11SecurityTypeTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11SecurityTypeTable();
}

/** Initialize the dot11SecurityTypeTable table by defining its contents and how it's structured */
void
initialize_table_dot11SecurityTypeTable(void)
{
    static oid dot11SecurityTypeTable_oid[128] = {0};
    size_t dot11SecurityTypeTable_oid_len   = 0;
	
	mad_dev_oid(dot11SecurityTypeTable_oid,DOT11SECURITYTYPE,&dot11SecurityTypeTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11SecurityTypeTable",     dot11SecurityTypeTable_handler,
              dot11SecurityTypeTable_oid, dot11SecurityTypeTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: securityID */
                           0);
    table_info->min_column = SECURITYTYPEMIN;
    table_info->max_column = SECURITYTYPEMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11SecurityTypeTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11SecurityTypeTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct dot11SecurityTypeTable_entry {
    /* Index values */
    long securityID;

    /* Column values */
    long securityType;
    long old_securityType;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11SecurityTypeTable_entry *next;
};

struct dot11SecurityTypeTable_entry  *dot11SecurityTypeTable_head;

/* create a new row in the (unsorted) table */
struct dot11SecurityTypeTable_entry *
dot11SecurityTypeTable_createEntry(
                 long  securityID,
                 long securityType
                ) {
    struct dot11SecurityTypeTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11SecurityTypeTable_entry);
    if (!entry)
        return NULL;

    entry->securityID = securityID;
	entry->securityType = securityType;
    entry->next = dot11SecurityTypeTable_head;
    dot11SecurityTypeTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11SecurityTypeTable_removeEntry( struct dot11SecurityTypeTable_entry *entry ) {
    struct dot11SecurityTypeTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11SecurityTypeTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11SecurityTypeTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11SecurityTypeTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{

	static int flag = 0;
	if(flag%3==0)
	{
		struct dot11SecurityTypeTable_entry *temp; 
		while( dot11SecurityTypeTable_head ){
		temp=dot11SecurityTypeTable_head->next;
		dot11SecurityTypeTable_removeEntry(dot11SecurityTypeTable_head);
		dot11SecurityTypeTable_head=temp;
	}

	{
		int result = 0;
		int sec_num = 0;
  		struct dcli_security *head,*q;          /*存放security信息的链表头*/    
		int i = 0;

		result = show_security_list(0,&head,&sec_num);
		if(result  == 1)
		{
			q=head;
			for(i=0;i<sec_num;i++)
			{		
				dot11SecurityTypeTable_createEntry(q->SecurityID,q->SecurityType);
				q = q->next;
			}
		}

		if(result == 1)
		{
			Free_security_head(head);
		}
	}

	flag = 0;
	}
	++flag;

	*my_data_context = dot11SecurityTypeTable_head;

	*my_loop_context = dot11SecurityTypeTable_head;
	return dot11SecurityTypeTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11SecurityTypeTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11SecurityTypeTable_entry *entry = (struct dot11SecurityTypeTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->securityID, sizeof(entry->securityID) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11SecurityTypeTable table */
int
dot11SecurityTypeTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11SecurityTypeTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SecurityTypeTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);


			if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
            switch (table_info->colnum) {
            case COLUMN_SECURITYTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityType,
                                          sizeof(table_entry->securityType));
                break;
            }
        }
        break;
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SharedSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYTYPE:
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
            table_entry = (struct dot11SharedSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYTYPE:
		{
			char sec_type[10];
                /* Need to save old 'table_entry->securitySharedID' value.
                   May need to use 'memcpy' */
                table_entry->old_securityType = table_entry->securityType;
                table_entry->securityType     = *request->requestvb->val.integer;
		switch(table_entry->securityType)
		{
			case OPEN :
			{
				strcpy(sec_type,"open");
			}
			break;
			case SHARED:
			{
				strcpy(sec_type,"shared");
			}
				break;
			case IEEE8021X:
			{
				strcpy(sec_type,"802.1x");
			}
				break;
			case WPA_P:
			{
				strcpy(sec_type,"WPA_P");
			}
				break;
			case WPA_E :
			{
				strcpy(sec_type,"WPA_E");
			}
				break;
			case WPA2_P :
			{
				strcpy(sec_type,"WPA2_P");
			}
				break;
			case WPA2_E :
			{
				strcpy(sec_type,"WPA2_E");
			}
				break;
			case WAPI_PSK:
			{
				strcpy(sec_type,"WAPI_PSK");
			}
				break;
			case WAPI_AUTH:
			{
				strcpy(sec_type,"WAPI_AUTH");
			}
				break;
		}
		security_type(0,table_entry->securityID,sec_type);
            	}
                break;
            }
        }
        break;
    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SharedSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYTYPE:
                /* Need to restore old 'table_entry->securityType' value.
                   May need to use 'memcpy' */
                table_entry->securityType= table_entry->old_securityType;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
	
    return SNMP_ERR_NOERROR;
}

#endif
