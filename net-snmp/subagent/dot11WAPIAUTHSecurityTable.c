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
* dot11WAPIAUTHSecurityTable.c
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
#include "dot11WAPIAUTHSecurityTable.h"
#include "wcpss/asd/asd.h"
#include "ws_security.h"
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

#define DOT11WAPIAUTHTABLE "2.14.11"
    /* Typical data structure for a row entry */
struct dot11WAPIAUTHSecurityTable_entry {
    /* Index values */
    long securityWAPIAUTHIndex;

    /* Column values */
    char *securityWAPIAUTHEncryType;
    char * securityWAPIAUTHServerIP;
    char * old_securityWAPIAUTHServerIP;
    long securityWAPIAUTHServerType;
    long old_securityWAPIAUTHServerType;
    char *securityWAPIAUTHServerCertificateRoute;
    char *old_securityWAPIAUTHServerCertificateRoute;
    char *securityWAPIAUTHUsrCertificateRoute;
    char *old_securityWAPIAUTHUsrCertificateRoute;
    long securityWAPIAUTHID;
    long old_securityWAPIAUTHID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WAPIAUTHSecurityTable_entry *next;
};

void dot11WAPIAUTHSecurityTable_load();
void
dot11WAPIAUTHSecurityTable_removeEntry( struct dot11WAPIAUTHSecurityTable_entry *entry );

/** Initializes the dot11WAPIAUTHSecurityTable module */
void
init_dot11WAPIAUTHSecurityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WAPIAUTHSecurityTable();
}

/** Initialize the dot11WAPIAUTHSecurityTable table by defining its contents and how it's structured */
void
initialize_table_dot11WAPIAUTHSecurityTable(void)
{
    static oid dot11WAPIAUTHSecurityTable_oid[128] = {0};
    size_t dot11WAPIAUTHSecurityTable_oid_len   = 0;
	mad_dev_oid(dot11WAPIAUTHSecurityTable_oid,DOT11WAPIAUTHTABLE,&dot11WAPIAUTHSecurityTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WAPIAUTHSecurityTable",     dot11WAPIAUTHSecurityTable_handler,
              dot11WAPIAUTHSecurityTable_oid, dot11WAPIAUTHSecurityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: securityWAPIAUTHIndex */
                           0);
    table_info->min_column = SECURITYWAPIUTHMIN;
    table_info->max_column = SECURITYWAPIUTHMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WAPIAUTHSecurityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WAPIAUTHSecurityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
 	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WAPIAUTHSecurityTable_load, dot11WAPIAUTHSecurityTable_removeEntry,
 							dot11WAPIAUTHSecurityTable_oid, dot11WAPIAUTHSecurityTable_oid_len));

    /* Initialise the contents of the table here */
}


struct dot11WAPIAUTHSecurityTable_entry  *dot11WAPIAUTHSecurityTable_head;

/* create a new row in the (unsorted) table */
struct dot11WAPIAUTHSecurityTable_entry *
dot11WAPIAUTHSecurityTable_createEntry(
                 long  securityWAPIAUTHIndex,
                  char *securityWAPIAUTHEncryType,
			    char *securityWAPIAUTHServerIP,
			    long securityWAPIAUTHServerType,
			    char *securityWAPIAUTHServerCertificateRoute,
			    char *securityWAPIAUTHUsrCertificateRoute,
			    long securityWAPIAUTHID
                ) {
    struct dot11WAPIAUTHSecurityTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WAPIAUTHSecurityTable_entry);
    if (!entry)
        return NULL;

    entry->securityWAPIAUTHIndex = securityWAPIAUTHIndex;
	entry->securityWAPIAUTHEncryType = strdup(securityWAPIAUTHEncryType);
	entry->securityWAPIAUTHServerIP = strdup(securityWAPIAUTHServerIP);
	entry->securityWAPIAUTHServerType = securityWAPIAUTHServerType;
	entry->securityWAPIAUTHServerCertificateRoute = strdup(securityWAPIAUTHServerCertificateRoute);
	entry->securityWAPIAUTHUsrCertificateRoute = strdup(securityWAPIAUTHUsrCertificateRoute);
	entry->securityWAPIAUTHID = securityWAPIAUTHID;
    entry->next = dot11WAPIAUTHSecurityTable_head;
    dot11WAPIAUTHSecurityTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WAPIAUTHSecurityTable_removeEntry( struct dot11WAPIAUTHSecurityTable_entry *entry ) {
    struct dot11WAPIAUTHSecurityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WAPIAUTHSecurityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WAPIAUTHSecurityTable_head = ptr->next;
    else
        prev->next = ptr->next;

	free(entry->securityWAPIAUTHEncryType);
	free(entry->securityWAPIAUTHServerCertificateRoute);
	free(entry->securityWAPIAUTHUsrCertificateRoute);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11WAPIAUTHSecurityTable_load()
{


				struct dot11WAPIAUTHSecurityTable_entry *temp; 
				 while( dot11WAPIAUTHSecurityTable_head ){
								  temp=dot11WAPIAUTHSecurityTable_head->next;
								  dot11WAPIAUTHSecurityTable_removeEntry(dot11WAPIAUTHSecurityTable_head);
								  dot11WAPIAUTHSecurityTable_head=temp;
				 }

				 
            {
				  struct dcli_security *security; 
				  
				char *endptr = NULL;  
				struct dcli_security *head,*q; 
				char *SecurityType=(char *)malloc(20); 
				int snum = 0; 
				int sec_id,op_ret=0,i=0,ret_one;
				long IfIndex = 0;				
				
				     op_ret=show_security_list(0,&head,&snum);						
					 if(op_ret == 1)
    				 {                    
					       q=head;
    						for(i=0;i<snum;i++)
    		                 {        		                 
								 memset(SecurityType, 0, 20);
								 CheckSecurityType(SecurityType, q->SecurityType);
        		                 if(strcmp(SecurityType,"wapi_auth")==0)
        						 {
        						 	IfIndex++;        						 	
        						    ret_one=show_security_one(0,(int)(q->SecurityID),&security);									
									if(ret_one==1)  
									{
        							    dot11WAPIAUTHSecurityTable_createEntry(IfIndex,
																			   "SMS4",
																			   security->wapi_as.as_ip,
																			   security->wapi_as.certification_type,
																			   security->wapi_as.certification_path,
																			   security->wapi_as.ae_cert_path,
																			   security->SecurityID);
									}
									if(ret_one==1)
									{
										Free_security_one(security);
									}
        		                 }						 
        						 
        						 q=q->next;
    						 }   					 
    				 }	
			    free(SecurityType);
				if(op_ret == 1)
				{
					Free_security_head(head);				   
				}
			    }


}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WAPIAUTHSecurityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11WAPIAUTHSecurityTable_head==NULL)
		return NULL;
	*my_data_context = dot11WAPIAUTHSecurityTable_head;
    *my_loop_context = dot11WAPIAUTHSecurityTable_head;
    return dot11WAPIAUTHSecurityTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WAPIAUTHSecurityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WAPIAUTHSecurityTable_entry *entry = (struct dot11WAPIAUTHSecurityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->securityWAPIAUTHIndex, sizeof(entry->securityWAPIAUTHIndex) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WAPIAUTHSecurityTable table */
int
dot11WAPIAUTHSecurityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WAPIAUTHSecurityTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WAPIAUTHSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
	
            switch (table_info->colnum) {
            case COLUMN_SECURITYWAPIAUTHINDEX:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWAPIAUTHIndex,
                                          sizeof(table_entry->securityWAPIAUTHIndex));
                break;
            case COLUMN_SECURITYWAPIAUTHENCRYTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWAPIAUTHEncryType,
                                          strlen(table_entry->securityWAPIAUTHEncryType));
                break;
            case COLUMN_SECURITYWAPIAUTHSERVERIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWAPIAUTHServerIP,
                                          strlen(table_entry->securityWAPIAUTHServerIP));
                break;
            case COLUMN_SECURITYWAPIAUTHSERVERTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWAPIAUTHServerType,
                                          sizeof(table_entry->securityWAPIAUTHServerType));
                break;
            case COLUMN_SECURITYWAPIAUTHSERVERCERTIFICATEROUTE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWAPIAUTHServerCertificateRoute,
                                          strlen(table_entry->securityWAPIAUTHServerCertificateRoute));
                break;
            case COLUMN_SECURITYWAPIAUTHUSRCERTIFICATEROUTE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWAPIAUTHUsrCertificateRoute,
                                          strlen(table_entry->securityWAPIAUTHUsrCertificateRoute));
                break;
            case COLUMN_SECURITYWAPIAUTHID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWAPIAUTHID,
                                          sizeof(table_entry->securityWAPIAUTHID));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WAPIAUTHSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWAPIAUTHSERVERIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWAPIAUTHSERVERTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWAPIAUTHSERVERCERTIFICATEROUTE:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWAPIAUTHUSRCERTIFICATEROUTE:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWAPIAUTHID:
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


///////////////进行set的操作


    case MODE_SET_ACTION:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WAPIAUTHSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWAPIAUTHSERVERIP:
					{
                /* Need to save old 'table_entry->securityWAPIAUTHServerIP' value.
                   May need to use 'memcpy' */
                //table_entry->old_securityWAPIAUTHServerIP = table_entry->securityWAPIAUTHServerIP;
               // table_entry->securityWAPIAUTHServerIP     = request->requestvb->val.YYY;

			
				if(table_entry->securityWAPIAUTHServerType==1)					
                  config_wapi_auth(0,table_entry->securityWAPIAUTHID,request->requestvb->val.string,"x509");
				else
					{
                   config_wapi_auth(0,table_entry->securityWAPIAUTHID,request->requestvb->val.string,"gbw");
					}

			}
			   break;
            case COLUMN_SECURITYWAPIAUTHSERVERTYPE:
					{
                /* Need to save old 'table_entry->securityWAPIAUTHServerType' value.
                   May need to use 'memcpy' */
               // table_entry->old_securityWAPIAUTHServerType = table_entry->securityWAPIAUTHServerType;
              //  table_entry->securityWAPIAUTHServerType     = request->requestvb->val.YYY;

				
                if(*request->requestvb->val.integer==1)					
                  config_wapi_auth(0,table_entry->securityWAPIAUTHID,table_entry->securityWAPIAUTHServerIP,"x509");
				else
					{
                   config_wapi_auth(0,table_entry->securityWAPIAUTHID,table_entry->securityWAPIAUTHServerIP,"gbw");
					}


			     }

			  break;
            case COLUMN_SECURITYWAPIAUTHSERVERCERTIFICATEROUTE:
					{
                /* Need to save old 'table_entry->securityWAPIAUTHServerCertificateRoute' value.
                   May need to use 'memcpy' */
                //table_entry->old_securityWAPIAUTHServerCertificateRoute = table_entry->securityWAPIAUTHServerCertificateRoute;
               // table_entry->securityWAPIAUTHServerCertificateRoute     = request->requestvb->val.YYY;

                config_wapi_auth_path(0,table_entry->securityWAPIAUTHID,"as",request->requestvb->val.string);

			    }
			   break;
            case COLUMN_SECURITYWAPIAUTHUSRCERTIFICATEROUTE:
					{
                /* Need to save old 'table_entry->securityWAPIAUTHUsrCertificateRoute' value.
                   May need to use 'memcpy' */
               // table_entry->old_securityWAPIAUTHUsrCertificateRoute = table_entry->securityWAPIAUTHUsrCertificateRoute;
              //  table_entry->securityWAPIAUTHUsrCertificateRoute     = request->requestvb->val.YYY;

               config_wapi_auth_path(0,table_entry->securityWAPIAUTHID,"ae",request->requestvb->val.string);

			      }
			   break;
            case COLUMN_SECURITYWAPIAUTHID:
					{
                /* Need to save old 'table_entry->securityWAPIAUTHID' value.
                   May need to use 'memcpy' */
                table_entry->old_securityWAPIAUTHID = table_entry->securityWAPIAUTHID;
              //  table_entry->securityWAPIAUTHID     = request->requestvb->val.YYY;
			    }
			  break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WAPIAUTHSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWAPIAUTHSERVERIP:
                /* Need to restore old 'table_entry->securityWAPIAUTHServerIP' value.
                   May need to use 'memcpy' */
                table_entry->securityWAPIAUTHServerIP = table_entry->old_securityWAPIAUTHServerIP;
                break;
            case COLUMN_SECURITYWAPIAUTHSERVERTYPE:
                /* Need to restore old 'table_entry->securityWAPIAUTHServerType' value.
                   May need to use 'memcpy' */
                table_entry->securityWAPIAUTHServerType = table_entry->old_securityWAPIAUTHServerType;
                break;
            case COLUMN_SECURITYWAPIAUTHSERVERCERTIFICATEROUTE:
                /* Need to restore old 'table_entry->securityWAPIAUTHServerCertificateRoute' value.
                   May need to use 'memcpy' */
                table_entry->securityWAPIAUTHServerCertificateRoute = table_entry->old_securityWAPIAUTHServerCertificateRoute;
                break;
            case COLUMN_SECURITYWAPIAUTHUSRCERTIFICATEROUTE:
                /* Need to restore old 'table_entry->securityWAPIAUTHUsrCertificateRoute' value.
                   May need to use 'memcpy' */
                table_entry->securityWAPIAUTHUsrCertificateRoute = table_entry->old_securityWAPIAUTHUsrCertificateRoute;
                break;
            case COLUMN_SECURITYWAPIAUTHID:
                /* Need to restore old 'table_entry->securityWAPIAUTHID' value.
                   May need to use 'memcpy' */
                table_entry->securityWAPIAUTHID = table_entry->old_securityWAPIAUTHID;
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
