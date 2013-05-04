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
* dot11WtpKeyConfigTable.c
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
#include "dot11WtpKeyConfigTable.h"
#include "ws_dbus_list_interface.h"
#include "autelanWtpGroup.h"

#define WTPKEYCONFIGTABLE "1.9.2"
    /* Typical data structure for a row entry */
struct dot11WtpKeyConfigTable_entry {
    /* Index values */
    long CipherKeyIndex;

    /* Column values */
    char *CipherKeyValue;
    char *old_CipherKeyValue;
    long CipherKeyCharType;
    long old_CipherKeyCharType;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WtpKeyConfigTable_entry *next;
};

void dot11WtpKeyConfigTable_load();
void
dot11WtpKeyConfigTable_removeEntry( struct dot11WtpKeyConfigTable_entry *entry );

/** Initializes the dot11WtpKeyConfigTable module */
void
init_dot11WtpKeyConfigTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WtpKeyConfigTable();
}

/** Initialize the dot11WtpKeyConfigTable table by defining its contents and how it's structured */
void
initialize_table_dot11WtpKeyConfigTable(void)
{
    static oid dot11WtpKeyConfigTable_oid[128] = {0};
    size_t dot11WtpKeyConfigTable_oid_len   = 0;
	mad_dev_oid(dot11WtpKeyConfigTable_oid,WTPKEYCONFIGTABLE,&dot11WtpKeyConfigTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WtpKeyConfigTable",     dot11WtpKeyConfigTable_handler,
              dot11WtpKeyConfigTable_oid, dot11WtpKeyConfigTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: CipherKeyIndex */
                           0);
    table_info->min_column = WTPKEYCONFMIN;
    table_info->max_column = WTPKEYCONFMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WtpKeyConfigTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WtpKeyConfigTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WtpKeyConfigTable_load, dot11WtpKeyConfigTable_removeEntry,
							dot11WtpKeyConfigTable_oid, dot11WtpKeyConfigTable_oid_len));

    /* Initialise the contents of the table here */
}



struct dot11WtpKeyConfigTable_entry  *dot11WtpKeyConfigTable_head;

/* create a new row in the (unsorted) table */
struct dot11WtpKeyConfigTable_entry *
dot11WtpKeyConfigTable_createEntry(
                 long  CipherKeyIndex,
                  char *CipherKeyValue,
			    long CipherKeyCharType
                ) {
    struct dot11WtpKeyConfigTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WtpKeyConfigTable_entry);
    if (!entry)
        return NULL;

    entry->CipherKeyIndex = CipherKeyIndex;
	entry->CipherKeyValue = strdup(CipherKeyValue);
	entry->CipherKeyCharType = CipherKeyCharType;
    entry->next = dot11WtpKeyConfigTable_head;
    dot11WtpKeyConfigTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WtpKeyConfigTable_removeEntry( struct dot11WtpKeyConfigTable_entry *entry ) {
    struct dot11WtpKeyConfigTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WtpKeyConfigTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WtpKeyConfigTable_head = ptr->next;
    else
        prev->next = ptr->next;
	
	free(entry->CipherKeyValue);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11WtpKeyConfigTable_load()
{

	struct dot11WtpKeyConfigTable_entry *temp;
	while( dot11WtpKeyConfigTable_head )
	{
		temp=dot11WtpKeyConfigTable_head->next;
		dot11WtpKeyConfigTable_removeEntry(dot11WtpKeyConfigTable_head);
		dot11WtpKeyConfigTable_head=temp;
	}

	{
		dot11WtpKeyConfigTable_createEntry(1,"2",3);
	}	

}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WtpKeyConfigTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{

		if(dot11WtpKeyConfigTable_head==NULL)
			{
				return NULL;
		}
	*my_loop_context = dot11WtpKeyConfigTable_head;

    *my_loop_context = dot11WtpKeyConfigTable_head;
    return dot11WtpKeyConfigTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WtpKeyConfigTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WtpKeyConfigTable_entry *entry = (struct dot11WtpKeyConfigTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->CipherKeyIndex, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WtpKeyConfigTable table */
int
dot11WtpKeyConfigTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WtpKeyConfigTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpKeyConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);


			if( !table_entry ){
								netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
								continue;
							}  

	
            switch (table_info->colnum) {
            case COLUMN_CIPHERKEYINDEX:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->CipherKeyIndex,
                                          sizeof(long));
                break;
            case COLUMN_CIPHERKEYVALUE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->CipherKeyValue,
                                          strlen(table_entry->CipherKeyValue));
                break;
            case COLUMN_CIPHERKEYCHARTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->CipherKeyCharType,
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
            table_entry = (struct dot11WtpKeyConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_CIPHERKEYVALUE:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_CIPHERKEYCHARTYPE:
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
            table_entry = (struct dot11WtpKeyConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_CIPHERKEYVALUE:
                /* Need to save old 'table_entry->CipherKeyValue' value.
                   May need to use 'memcpy' */
                table_entry->old_CipherKeyValue = table_entry->CipherKeyValue;
             //   table_entry->CipherKeyValue     = request->requestvb->val.YYY;
                break;
            case COLUMN_CIPHERKEYCHARTYPE:
                /* Need to save old 'table_entry->CipherKeyCharType' value.
                   May need to use 'memcpy' */
                table_entry->old_CipherKeyCharType = table_entry->CipherKeyCharType;
             //   table_entry->CipherKeyCharType     = request->requestvb->val.YYY;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpKeyConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_CIPHERKEYVALUE:
                /* Need to restore old 'table_entry->CipherKeyValue' value.
                   May need to use 'memcpy' */
                table_entry->CipherKeyValue = table_entry->old_CipherKeyValue;
                break;
            case COLUMN_CIPHERKEYCHARTYPE:
                /* Need to restore old 'table_entry->CipherKeyCharType' value.
                   May need to use 'memcpy' */
                table_entry->CipherKeyCharType = table_entry->old_CipherKeyCharType;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
