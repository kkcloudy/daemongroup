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
* dot11WPA2PSecurityTable.c
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
#include "dot11WPA2PSecurityTable.h"
#include "wcpss/asd/asd.h"
#include "ws_security.h"
#include "dbus/asd/ASDDbusDef1.h"
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

#define DOT11WPA2PTABLE "2.14.9"

/** Initializes the dot11WPA2PSecurityTable module */
void
init_dot11WPA2PSecurityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WPA2PSecurityTable();
}

/** Initialize the dot11WPA2PSecurityTable table by defining its contents and how it's structured */
void
initialize_table_dot11WPA2PSecurityTable(void)
{
    static oid dot11WPA2PSecurityTable_oid[128] = {0};
    size_t dot11WPA2PSecurityTable_oid_len   = 0;
	
	mad_dev_oid(dot11WPA2PSecurityTable_oid,DOT11WPA2PTABLE,&dot11WPA2PSecurityTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WPA2PSecurityTable",     dot11WPA2PSecurityTable_handler,
              dot11WPA2PSecurityTable_oid, dot11WPA2PSecurityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: securityWPA2PIndex */
                           0);
    table_info->min_column = SECURITYWPA2PMIN;
    table_info->max_column = SECURITYWPA2PMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WPA2PSecurityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WPA2PSecurityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct dot11WPA2PSecurityTable_entry {
    /* Index values */
    long securityWPA2PIndex;

    /* Column values */
    long securityWPA2PEncryType;
    long old_securityWPA2PEncryType;
    long securityWPA2PKeyInputType;
    long old_securityWPA2PKeyInputType;
    char *securityWPA2PKey;
    char old_securityWPA2PKey;
    long securityWPA2PID;
    long old_securityWPA2PID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WPA2PSecurityTable_entry *next;
};

struct dot11WPA2PSecurityTable_entry  *dot11WPA2PSecurityTable_head;

/* create a new row in the (unsorted) table */
struct dot11WPA2PSecurityTable_entry *
dot11WPA2PSecurityTable_createEntry(
                 long  securityWPA2PIndex,
                 long securityWPA2PEncryType,
			    long securityWPA2PKeyInputType,
			    char *securityWPA2PKey,
			    long securityWPA2PID
                ) {
    struct dot11WPA2PSecurityTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WPA2PSecurityTable_entry);
    if (!entry)
        return NULL;

    entry->securityWPA2PIndex = securityWPA2PIndex;
	entry->securityWPA2PEncryType = securityWPA2PEncryType;
	entry->securityWPA2PKeyInputType = securityWPA2PKeyInputType;
	entry->securityWPA2PKey = strdup(securityWPA2PKey);
	entry->securityWPA2PID = securityWPA2PID;
    entry->next = dot11WPA2PSecurityTable_head;
    dot11WPA2PSecurityTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WPA2PSecurityTable_removeEntry( struct dot11WPA2PSecurityTable_entry *entry ) {
    struct dot11WPA2PSecurityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WPA2PSecurityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WPA2PSecurityTable_head = ptr->next;
    else
        prev->next = ptr->next;
  	free(entry->securityWPA2PKey);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WPA2PSecurityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{


	static int flag = 0;
			if(flag%3==0)
				{
					struct dot11WPA2PSecurityTable_entry *temp; 
					 while( dot11WPA2PSecurityTable_head ){
									  temp=dot11WPA2PSecurityTable_head->next;
									  dot11WPA2PSecurityTable_removeEntry(dot11WPA2PSecurityTable_head);
									  dot11WPA2PSecurityTable_head=temp;
					 }
					 
        ////////////////

		               {
                          		int up_value = 0;
                          		int sec_num = 0;
                          		int result1 = 0;
                            	struct dcli_security *head,*q;          
								int i = 0;
                          		///////////////
                          		struct dcli_security *security;
								int result2 = 0;
                          
                          		result1= show_security_list(0,&head,&sec_num);
                          		if(result1 == 1)
                          		{
                      				q=head;
									for(i=0;i<sec_num;i++)
                      				{                             										
										if(q->SecurityType==WPA2_P)
                      					{                  
											result2 = show_security_one(0,q->SecurityID,&security);
											if(result2 == 1)
											{
												if(security->EncryptionType==3)
												   security->EncryptionType=1;
												
												dot11WPA2PSecurityTable_createEntry(++up_value,
																					security->EncryptionType,
																					security->keyInputType,
																					security->SecurityKey,
																					security->SecurityID); 	   
											}
											if(result2 == 1)
											{
												Free_security_one(security);
											}
										}
                      					q = q->next;
                      				}
                          		}

                          		if(result1 == 1)
                          		{
									Free_security_head(head);
                          		}
                          	}

		/////////////////////
					   // {				
					   // dot11WPA2PSecurityTable_createEntry(1,2,3,"4",5);					
				       // }

		//////////////////////
		
					flag = 0;
				}
			++flag;
			
		*my_data_context = dot11WPA2PSecurityTable_head;

    *my_loop_context = dot11WPA2PSecurityTable_head;
    return dot11WPA2PSecurityTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WPA2PSecurityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WPA2PSecurityTable_entry *entry = (struct dot11WPA2PSecurityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->securityWPA2PIndex, sizeof(entry->securityWPA2PIndex) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WPA2PSecurityTable table */
int
dot11WPA2PSecurityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WPA2PSecurityTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPA2PSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

		if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
	
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPA2PINDEX:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2PIndex,
                                          sizeof(table_entry->securityWPA2PIndex));
                break;
            case COLUMN_SECURITYWPA2PENCRYTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2PEncryType,
                                          sizeof(table_entry->securityWPA2PEncryType));
                break;
            case COLUMN_SECURITYWPA2PKEYINPUTTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2PKeyInputType,
                                          sizeof(table_entry->securityWPA2PKeyInputType));
                break;
            case COLUMN_SECURITYWPA2PKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPA2PKey,
                                          strlen(table_entry->securityWPA2PKey));
                break;
            case COLUMN_SECURITYWPA2PID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2PID,
                                          sizeof(table_entry->securityWPA2PID));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPA2PSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPA2PENCRYTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2PKEYINPUTTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2PKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2PID:
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
            table_entry = (struct dot11WPA2PSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPA2PENCRYTYPE:
                /* Need to save old 'table_entry->securityWPA2PEncryType' value.
                   May need to use 'memcpy' */
               // table_entry->old_securityWPA2PEncryType = table_entry->securityWPA2PEncryType;
               // table_entry->securityWPA2PEncryType     = request->requestvb->val.YYY;
               	

				{
				int ret = 0;
				if(*request->requestvb->val.integer==1)
                	ret = encryption_type(0,table_entry->securityWPA2PID,"TKIP");
				else
					{
						ret = encryption_type(0,table_entry->securityWPA2PID,"AES");
					}
            	}
                break;
            case COLUMN_SECURITYWPA2PKEYINPUTTYPE:
                /* Need to save old 'table_entry->securityWPA2PKeyInputType' value.
                   May need to use 'memcpy' */
                //table_entry->old_securityWPA2PKeyInputType = table_entry->securityWPA2PKeyInputType;
              //  table_entry->securityWPA2PKeyInputType     = request->requestvb->val.YYY;
              	{
                char * InputType = (char*) malloc(20);
				memset(InputType,0,20);
			
				if(*request->requestvb->val.integer==0)
					{
					 strcpy(InputType,"hex");
					}
				else
					{
					 strcpy(InputType,"ascii");
					}

				
				int ret = 0;
				ret=security_key(0,table_entry->securityWPA2PID,table_entry->securityWPA2PKey,InputType);
               
				free(InputType);
            	}
                break;
            case COLUMN_SECURITYWPA2PKEY:
                /* Need to save old 'table_entry->securityWPA2PKey' value.
                   May need to use 'memcpy' */
               // table_entry->old_securityWPA2PKey = table_entry->securityWPA2PKey;
              //  table_entry->securityWPA2PKey     = request->requestvb->val.YYY;

              	{
		
                char * InputType = (char*) malloc(20);
				memset(InputType,0,20);
				if(table_entry->securityWPA2PKeyInputType==1)
					{
					 strcpy(InputType,"ascii");
					}
				else
					{
					 strcpy(InputType,"hex");
					}
               
			    int ret;
				ret=security_key(0,table_entry->securityWPA2PID,request->requestvb->val.string,InputType);
                
				table_entry->securityWPA2PKey=strdup(request->requestvb->val.string);
                	
				free(InputType);
            	}
				
                break;
            case COLUMN_SECURITYWPA2PID:
                /* Need to save old 'table_entry->securityWPA2PID' value.
                   May need to use 'memcpy' */
                table_entry->old_securityWPA2PID = table_entry->securityWPA2PID;
              //  table_entry->securityWPA2PID     = request->requestvb->val.YYY;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPA2PSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPA2PENCRYTYPE:
                /* Need to restore old 'table_entry->securityWPA2PEncryType' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2PEncryType = table_entry->old_securityWPA2PEncryType;
                break;
            case COLUMN_SECURITYWPA2PKEYINPUTTYPE:
                /* Need to restore old 'table_entry->securityWPA2PKeyInputType' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2PKeyInputType = table_entry->old_securityWPA2PKeyInputType;
                break;
            case COLUMN_SECURITYWPA2PKEY:
                /* Need to restore old 'table_entry->securityWPA2PKey' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2PKey = table_entry->old_securityWPA2PKey;
                break;
            case COLUMN_SECURITYWPA2PID:
                /* Need to restore old 'table_entry->securityWPA2PID' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2PID = table_entry->old_securityWPA2PID;
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
