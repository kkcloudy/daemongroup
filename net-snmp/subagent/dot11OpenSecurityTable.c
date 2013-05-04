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
* dot11OpenSecurityTable.c
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
#include "dot11OpenSecurityTable.h"
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

#define DOT11OPENSECURITYTABLE "2.14.3"

/** Initializes the dot11OpenSecurityTable module */
void
init_dot11OpenSecurityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11OpenSecurityTable();
}

/** Initialize the dot11OpenSecurityTable table by defining its contents and how it's structured */
void
initialize_table_dot11OpenSecurityTable(void)
{
    static oid dot11OpenSecurityTable_oid[128] = {0};
    size_t dot11OpenSecurityTable_oid_len   = 0;
	mad_dev_oid(dot11OpenSecurityTable_oid,DOT11OPENSECURITYTABLE,&dot11OpenSecurityTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11OpenSecurityTable",     dot11OpenSecurityTable_handler,
              dot11OpenSecurityTable_oid, dot11OpenSecurityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: securityOpenIndex */
                           0);
    table_info->min_column = SECURITYOPENMIN;
    table_info->max_column = SECURITYOPENMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11OpenSecurityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11OpenSecurityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct dot11OpenSecurityTable_entry {
    /* Index values */
    long securityOpenIndex;

    /* Column values */
    long securityOpenEncryType;
    long old_securityOpenEncryType;
    long securityOpenKeyInputType;
    long old_securityOpenKeyInputType;
    char *securityOpenKey;
    char *old_securityOpenKey;
    long securityOpenExtensibleAuth;
    long old_securityOpenExtensibleAuth;
    char *securityOpenAuthIP;
    char *old_securityOpenAuthIP;
    long securityOpenAuthPort;
    char *securityOpenAuthKey;
    char *old_securityOpenAuthKey;
    char *securityOpenRadiusIP;
    char *old_securityOpenRadiusIP;
    long securityOpenRadiusPort;
    char *securityOpenRadiusKey;
    char *old_securityOpenRadiusKey;
    char *securityOpenHostIP;
    char *old_securityOpenHostIP;
    long securityOpenRadiusServer;
    long old_securityOpenRadiusServer;
    long securityOpenRadiusInfoUpdateTime;
    long old_securityOpenRadiusInfoUpdateTime;
    long securityOpenReauthTime;
    long old_securityOpenReauthTime;
    long securityOpenID;
    long old_securityOpenID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11OpenSecurityTable_entry *next;
};

struct dot11OpenSecurityTable_entry  *dot11OpenSecurityTable_head;

/* create a new row in the (unsorted) table */
struct dot11OpenSecurityTable_entry *
dot11OpenSecurityTable_createEntry(
                 long  securityOpenIndex,
                 long securityOpenEncryType,
		   long securityOpenKeyInputType,
		   char *securityOpenKey,
			    long securityOpenExtensibleAuth,
		   char *securityOpenAuthIP,
		   long securityOpenAuthPort,
		   char *securityOpenAuthKey,
		   char *securityOpenRadiusIP,
		   long securityOpenRadiusPort,
		   char *securityOpenRadiusKey,
		   char *securityOpenHostIP,
		   long securityOpenRadiusServer,
		   long securityOpenRadiusInfoUpdateTime,
		   long securityOpenReauthTime,
		   long securityOpenID
                ) {
    struct dot11OpenSecurityTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11OpenSecurityTable_entry);
    if (!entry)
        return NULL;

    entry->securityOpenIndex = securityOpenIndex;
    entry->securityOpenEncryType = securityOpenEncryType;
    entry->securityOpenKeyInputType = securityOpenKeyInputType;
    entry->securityOpenKey = strdup(securityOpenKey);
	entry->securityOpenExtensibleAuth = securityOpenExtensibleAuth;
    entry->securityOpenAuthIP = strdup(securityOpenAuthIP);
    entry->securityOpenAuthPort = securityOpenAuthPort;
    entry->securityOpenAuthKey = strdup(securityOpenAuthKey);
    entry->securityOpenRadiusIP = strdup(securityOpenRadiusIP);
    entry->securityOpenRadiusPort = securityOpenRadiusPort;
    entry->securityOpenRadiusKey =strdup(securityOpenRadiusKey);
    entry->securityOpenHostIP = strdup(securityOpenHostIP);
    entry->securityOpenRadiusServer = securityOpenRadiusServer;
    entry->securityOpenRadiusInfoUpdateTime =securityOpenRadiusInfoUpdateTime;
    entry->securityOpenReauthTime =  securityOpenReauthTime;
	entry->securityOpenID = securityOpenID;
    entry->next = dot11OpenSecurityTable_head;
    dot11OpenSecurityTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11OpenSecurityTable_removeEntry( struct dot11OpenSecurityTable_entry *entry ) {
    struct dot11OpenSecurityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11OpenSecurityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11OpenSecurityTable_head = ptr->next;
    else
        prev->next = ptr->next;
	free(entry->securityOpenKey);
	free(entry->securityOpenAuthKey);
	free(entry->securityOpenRadiusKey);
	free(entry->securityOpenAuthIP);
	free(entry->securityOpenRadiusIP);
	free(entry->securityOpenHostIP);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11OpenSecurityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{


		static int flag = 0;
		if(flag%3==0)
			{
				struct dot11OpenSecurityTable_entry *temp;	
				 while( dot11OpenSecurityTable_head ){
								  temp=dot11OpenSecurityTable_head->next;
								  dot11OpenSecurityTable_removeEntry(dot11OpenSecurityTable_head);
								  dot11OpenSecurityTable_head=temp;
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
    		                 if(strcmp(SecurityType,"open")==0)
						 	{
    						 	IfIndex++;        						 	
        						    ret_one=show_security_one(0,(int)(q->SecurityID),&security);
								if(ret_one==1)
								{
    								dot11OpenSecurityTable_createEntry(IfIndex,
																	   security->EncryptionType,
																	   security->keyInputType,
																	   security->SecurityKey,
																	   security->extensible_auth,
																	   security->auth.auth_ip,
																	   security->auth.auth_port,
																	   security->auth.auth_shared_secret,
																	   security->acct.acct_ip,
																	   security->acct.acct_port,
																	   security->acct.acct_shared_secret,
																	   security->host_ip,
																	   security->wired_radius,
																	   security->acct_interim_interval,
																	   security->eap_reauth_period,
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

				flag = 0;
			}
		++flag;
	
	*my_data_context = dot11OpenSecurityTable_head;
    *my_loop_context = dot11OpenSecurityTable_head;
    return dot11OpenSecurityTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11OpenSecurityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11OpenSecurityTable_entry *entry = (struct dot11OpenSecurityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->securityOpenIndex, sizeof(entry->securityOpenIndex) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11OpenSecurityTable table */
int
dot11OpenSecurityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11OpenSecurityTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11OpenSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
	
            switch (table_info->colnum) {
            case COLUMN_SECURITYOPENINDEX:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenIndex,
                                          sizeof(table_entry->securityOpenIndex));
                break;
            case COLUMN_SECURITYOPENENCRYTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenEncryType,
                                          sizeof(table_entry->securityOpenEncryType));
                break;
            case COLUMN_SECURITYOPENKEYINPUTTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenKeyInputType,
                                          sizeof(table_entry->securityOpenKeyInputType));
                break;
            case COLUMN_SECURITYOPENKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityOpenKey,
                                          strlen(table_entry->securityOpenKey));
                break;
            case COLUMN_SECURITYOPENEXTENSIBLEAUTH:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenExtensibleAuth,
                                          sizeof(table_entry->securityOpenExtensibleAuth));
                break;
            case COLUMN_SECURITYOPENAUTHIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityOpenAuthIP,
                                          strlen(table_entry->securityOpenAuthIP));
                break;
            case COLUMN_SECURITYOPENAUTHPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenAuthPort,
                                          sizeof(table_entry->securityOpenAuthPort));
                break;
            case COLUMN_SECURITYOPENAUTHKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityOpenAuthKey,
                                          strlen(table_entry->securityOpenAuthKey));
                break;
            case COLUMN_SECURITYOPENRADIUSIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityOpenRadiusIP,
                                          strlen(table_entry->securityOpenRadiusIP));
                break;
            case COLUMN_SECURITYOPENRADIUSPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenRadiusPort,
                                          sizeof(table_entry->securityOpenRadiusPort));
                break;
            case COLUMN_SECURITYOPENRADIUSKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityOpenRadiusKey,
                                          strlen(table_entry->securityOpenRadiusKey));
                break;
            case COLUMN_SECURITYOPENHOSTIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityOpenHostIP,
                                          strlen(table_entry->securityOpenHostIP));
                break;
            case COLUMN_SECURITYOPENRADIUSSERVER:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenRadiusServer,
                                          sizeof(table_entry->securityOpenRadiusServer));
                break;
            case COLUMN_SECURITYOPENRADIUSINFOUPDATETIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenRadiusInfoUpdateTime,
                                          sizeof(table_entry->securityOpenRadiusInfoUpdateTime));
                break;
            case COLUMN_SECURITYOPENREAUTHTIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenReauthTime,
                                          sizeof(table_entry->securityOpenReauthTime));
                break;
            case COLUMN_SECURITYOPENID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityOpenID,
                                          sizeof(table_entry->securityOpenID));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11OpenSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYOPENENCRYTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENKEYINPUTTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENEXTENSIBLEAUTH:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENAUTHIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENAUTHKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENRADIUSIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENRADIUSKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENHOSTIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENRADIUSSERVER:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENRADIUSINFOUPDATETIME:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYOPENREAUTHTIME:
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
            table_entry = (struct dot11OpenSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
            switch (table_info->colnum) {
            case COLUMN_SECURITYOPENENCRYTYPE:
               			{
							int ret = 0;
				if(*request->requestvb->val.integer==1)
                	ret = encryption_type(0,table_entry->securityOpenID,"WEP");
				else
					{
						ret = encryption_type(0,table_entry->securityOpenID,"none");
					}
				if(ret == 1)
					{
						table_entry->securityOpenEncryType = *request->requestvb->val.integer;
					}
            	}
                break;
            case COLUMN_SECURITYOPENKEYINPUTTYPE:
               	{
				int ret = 0;
				char *type = (char*)malloc(10); 
				memset(type,0,10);
                if(*request->requestvb->val.integer==1)
					strcpy(type,"ASCII");
                	
				else
					{
                  	 	strcpy(type,"HEX");					
					}	
				ret = security_key(0,table_entry->securityOpenID,table_entry->securityOpenKey,type);
				if(ret == 1)
					{
						table_entry->securityOpenKeyInputType = *request->requestvb->val.integer;
					}
				free(type);
			    }		
                break;
            case COLUMN_SECURITYOPENKEY:
                {
					
				int ret = 0;
				char *type = (char*)malloc(10); 
				memset(type,0,10);
                if(table_entry->securityOpenKeyInputType==1)
					strcpy(type,"ASCII");
                	
				else
					{
                  	 	strcpy(type,"HEX");					
					}	
                ret = security_key(0,table_entry->securityOpenID,request->requestvb->val.string,type);
				if(ret == 1)
					{
						table_entry->securityOpenKey= strdup(request->requestvb->val.string);
					}
				free(type);
			    }
                break;
            case COLUMN_SECURITYOPENEXTENSIBLEAUTH:
               {
				int ret = 0;
                if(*request->requestvb->val.integer==1)
                	ret = extensible_authentication(0,table_entry->securityOpenID,1);
				else
					{
                  	 	ret =  extensible_authentication(0,table_entry->securityOpenID,0);					
					}	
				if(ret == 1)
					{
						table_entry->securityOpenExtensibleAuth= *request->requestvb->val.integer;
					}
			    }
                break;
            case COLUMN_SECURITYOPENAUTHIP:
                {
					int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,1,table_entry->securityOpenID,request->requestvb->val.string,1812,table_entry->securityOpenAuthKey);
				if(ret ==1)
					{
					  table_entry->securityOpenAuthIP= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYOPENAUTHKEY:
                {
				int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,1,table_entry->securityOpenID,table_entry->securityOpenAuthIP,1812,request->requestvb->val.string);
				if(ret ==1)
					{
					  table_entry->securityOpenAuthKey= strdup(request->requestvb->val.string);
					}
				}
			break;
            case COLUMN_SECURITYOPENRADIUSIP:
                {
				int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,0,table_entry->securityOpenID,request->requestvb->val.string,1813,table_entry->securityOpenRadiusKey);
				if(ret ==1)
					{
					  table_entry->securityOpenRadiusIP= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYOPENRADIUSKEY:
                {
				int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,0,table_entry->securityOpenID,table_entry->securityOpenRadiusIP,1813,request->requestvb->val.string);
				if(ret ==1)
					{
					  table_entry->securityOpenRadiusKey= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYOPENHOSTIP:
                {
						int ret = 0;
						ret = security_host_ip(0,table_entry->securityOpenID,request->requestvb->val.string);
						if(ret == 1)
							{
								table_entry->securityOpenHostIP= strdup(request->requestvb->val.string);
							}
					}
                break;
            case COLUMN_SECURITYOPENRADIUSSERVER:
               {
						int ret = 0;
               			ret = radius_server(0,table_entry->securityOpenID,*request->requestvb->val.integer);
						if(ret == 1)
							{
							 table_entry->securityOpenRadiusServer= *request->requestvb->val.integer;
							}
					}
                break;
            case COLUMN_SECURITYOPENRADIUSINFOUPDATETIME:
               {
					int ret = 0;
					ret = set_acct_interim_interval(0,table_entry->securityOpenID,*request->requestvb->val.integer);
					if(ret == 1)
						{	
							table_entry->securityOpenRadiusInfoUpdateTime= *request->requestvb->val.integer;
						}
				}
                break;
            case COLUMN_SECURITYOPENREAUTHTIME:
                {
						int ret = 0;
						ret = set_eap_reauth_period_cmd(0,table_entry->securityOpenID,*request->requestvb->val.integer);
						if(ret ==1)
							{
								table_entry->securityOpenReauthTime= *request->requestvb->val.integer;
							}
					}
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11OpenSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYOPENENCRYTYPE:
                /* Need to restore old 'table_entry->securityOpenEncryType' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenEncryType = table_entry->old_securityOpenEncryType;
                break;
            case COLUMN_SECURITYOPENKEYINPUTTYPE:
                /* Need to restore old 'table_entry->securityOpenKeyInputType' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenKeyInputType = table_entry->old_securityOpenKeyInputType;
                break;
            case COLUMN_SECURITYOPENKEY:
                /* Need to restore old 'table_entry->securityOpenKey' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenKey = table_entry->old_securityOpenKey;
                break;
            case COLUMN_SECURITYOPENEXTENSIBLEAUTH:
                /* Need to restore old 'table_entry->securityOpenExtensibleAuth' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenExtensibleAuth = table_entry->old_securityOpenExtensibleAuth;
                break;
            case COLUMN_SECURITYOPENAUTHIP:
                /* Need to restore old 'table_entry->securityOpenAuthIP' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenAuthIP = table_entry->old_securityOpenAuthIP;
                break;
            case COLUMN_SECURITYOPENAUTHKEY:
                /* Need to restore old 'table_entry->securityOpenAuthKey' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenAuthKey = table_entry->old_securityOpenAuthKey;
                break;
            case COLUMN_SECURITYOPENRADIUSIP:
                /* Need to restore old 'table_entry->securityOpenRadiusIP' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenRadiusIP = table_entry->old_securityOpenRadiusIP;
                break;
            case COLUMN_SECURITYOPENRADIUSKEY:
                /* Need to restore old 'table_entry->securityOpenRadiusKey' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenRadiusKey = table_entry->old_securityOpenRadiusKey;
                break;
            case COLUMN_SECURITYOPENHOSTIP:
                /* Need to restore old 'table_entry->securityOpenHostIP' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenHostIP = table_entry->old_securityOpenHostIP;
                break;
            case COLUMN_SECURITYOPENRADIUSSERVER:
                /* Need to restore old 'table_entry->securityOpenRadiusServer' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenRadiusServer = table_entry->old_securityOpenRadiusServer;
                break;
            case COLUMN_SECURITYOPENRADIUSINFOUPDATETIME:
                /* Need to restore old 'table_entry->securityOpenRadiusInfoUpdateTime' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenRadiusInfoUpdateTime = table_entry->old_securityOpenRadiusInfoUpdateTime;
                break;
            case COLUMN_SECURITYOPENREAUTHTIME:
                /* Need to restore old 'table_entry->securityOpenReauthTime' value.
                   May need to use 'memcpy' */
                table_entry->securityOpenReauthTime = table_entry->old_securityOpenReauthTime;
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
