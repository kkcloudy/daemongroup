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
* dot118021xSecurityTable.c
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
#include "dot118021xSecurityTable.h"

#include "ws_init_dbus.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "wcpss/asd/asd.h"
#include "ws_security.h"
#include "dbus/asd/ASDDbusDef1.h"


#include "autelanWtpGroup.h"

#define DOT118021XTABLE "2.14.5"

/** Initializes the dot118021xSecurityTable module */
void
init_dot118021xSecurityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot118021xSecurityTable();
}

/** Initialize the dot118021xSecurityTable table by defining its contents and how it's structured */
void
initialize_table_dot118021xSecurityTable(void)
{
    static oid dot118021xSecurityTable_oid[128] = {0};
    size_t dot118021xSecurityTable_oid_len   = 0;
	
	mad_dev_oid(dot118021xSecurityTable_oid,DOT118021XTABLE,&dot118021xSecurityTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot118021xSecurityTable",     dot118021xSecurityTable_handler,
              dot118021xSecurityTable_oid, dot118021xSecurityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: security8021xIndex */
                           0);
    table_info->min_column = SECURITY802MIN;
    table_info->max_column = SECURITY802MAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot118021xSecurityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot118021xSecurityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct dot118021xSecurityTable_entry {
    /* Index values */
    long security8021xIndex;

    /* Column values */
    char *security8021xEncryType;
    char *security8021xAuthIP;
    char *old_security8021xAuthIP;
    long security8021xAuthPort;
    char *security8021xAuthKey;
    char *old_security8021xAuthKey;
    char *security8021xRadiusIP;
    char *old_security8021xRadiusIP;
    long security8021xRadiusPort;
    char *security8021xRadiusKey;
    char *old_security8021xRadiusKey;
    char *security8021xHostIP;
    char *old_security8021xHostIP;
    long security8021xRadiusServer;
    long old_security8021xRadiusServer;
    long security8021xRadiusInfoUpdateTime;
    long old_security8021xRadiusInfoUpdateTime;
    long security8021xReauthTime;
    long old_security8021xReauthTime;
    long security8021xID;
    long old_security8021xID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot118021xSecurityTable_entry *next;
};

struct dot118021xSecurityTable_entry  *dot118021xSecurityTable_head;

/* create a new row in the (unsorted) table */
struct dot118021xSecurityTable_entry *
dot118021xSecurityTable_createEntry(
                 long  security8021xIndex,
                 char *security8021xEncryType,
			    char *security8021xAuthIP,
			    long security8021xAuthPort,
			    char *security8021xAuthKey,
			    char *security8021xRadiusIP,
			    long security8021xRadiusPort,
			    char *security8021xRadiusKey,
			    char *security8021xHostIP,
			    long security8021xRadiusServer,
			    long security8021xRadiusInfoUpdateTime,
			    long security8021xReauthTime,
			    long security8021xID
                ) {
    struct dot118021xSecurityTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot118021xSecurityTable_entry);
    if (!entry)
        return NULL;

    entry->security8021xIndex = security8021xIndex;
	entry->security8021xEncryType = strdup(security8021xEncryType);
    entry->security8021xAuthIP = strdup(security8021xAuthIP);
    entry->security8021xAuthPort = security8021xAuthPort;
    entry->security8021xAuthKey = strdup(security8021xAuthKey);
    entry->security8021xRadiusIP = strdup(security8021xRadiusIP);
    entry->security8021xRadiusPort=security8021xRadiusPort;
    entry->security8021xRadiusKey=strdup(security8021xRadiusKey);
    entry->security8021xHostIP=strdup(security8021xHostIP);
    entry->security8021xRadiusServer=security8021xRadiusServer;
    entry->security8021xRadiusInfoUpdateTime=security8021xRadiusInfoUpdateTime;
    entry->security8021xReauthTime=security8021xReauthTime;
    entry->security8021xID = security8021xID;
    entry->next = dot118021xSecurityTable_head;
    dot118021xSecurityTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot118021xSecurityTable_removeEntry( struct dot118021xSecurityTable_entry *entry ) {
    struct dot118021xSecurityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot118021xSecurityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot118021xSecurityTable_head = ptr->next;
    else
        prev->next = ptr->next;

	free(entry->security8021xEncryType);
	free(entry->security8021xAuthKey);
	free(entry->security8021xRadiusKey);
	free(entry->security8021xAuthIP);
	free(entry->security8021xHostIP);
	free(entry->security8021xRadiusIP);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot118021xSecurityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{


	static int flag = 0;
				if(flag%3==0)
					{
						struct dot118021xSecurityTable_entry *temp;	
						 while( dot118021xSecurityTable_head ){
										  temp=dot118021xSecurityTable_head->next;
										  dot118021xSecurityTable_removeEntry(dot118021xSecurityTable_head);
										  dot118021xSecurityTable_head=temp;
						 }
			
						{
							int up_value = 0;
							int sec_num = 0;
							int result1 = 0;
					  		struct dcli_security *head,*q,*security;          /*存放security信息的链表头*/    
							int i = 0;
							int result2 = 0;

							result1= show_security_list(0,&head,&sec_num);
							if(result1 == 1)
							{
								q=head;
								for(i=0;i<sec_num;i++)
								{
									if(q->SecurityType==IEEE8021X)
									{
										result2 = show_security_one(0,q->SecurityID,&security);
										if(result2 == 1)
										{
											dot118021xSecurityTable_createEntry(++up_value,
																				"WEP",
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
			
						flag = 0;
					}
				++flag;
			
		*my_data_context = dot118021xSecurityTable_head;

    *my_loop_context = dot118021xSecurityTable_head;
    return dot118021xSecurityTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot118021xSecurityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot118021xSecurityTable_entry *entry = (struct dot118021xSecurityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->security8021xIndex, sizeof(entry->security8021xIndex) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot118021xSecurityTable table */
int
dot118021xSecurityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot118021xSecurityTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot118021xSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
	
            switch (table_info->colnum) {
            case COLUMN_SECURITY8021XINDEX:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->security8021xIndex,
                                          sizeof(table_entry->security8021xIndex));
                break;
            case COLUMN_SECURITY8021XENCRYTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->security8021xEncryType,
                                          strlen(table_entry->security8021xEncryType));
                break;
            case COLUMN_SECURITY8021XAUTHIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->security8021xAuthIP,
                                          strlen(table_entry->security8021xAuthIP));
                break;
            case COLUMN_SECURITY8021XAUTHPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->security8021xAuthPort,
                                          sizeof(table_entry->security8021xAuthPort));
                break;
            case COLUMN_SECURITY8021XAUTHKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->security8021xAuthKey,
                                          strlen(table_entry->security8021xAuthKey));
                break;
            case COLUMN_SECURITY8021XRADIUSIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->security8021xRadiusIP,
                                          strlen(table_entry->security8021xRadiusIP));
                break;
            case COLUMN_SECURITY8021XRADIUSPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->security8021xRadiusPort,
                                          sizeof(table_entry->security8021xRadiusPort));
                break;
            case COLUMN_SECURITY8021XRADIUSKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->security8021xRadiusKey,
                                          strlen(table_entry->security8021xRadiusKey));
                break;
            case COLUMN_SECURITY8021XHOSTIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->security8021xHostIP,
                                          strlen(table_entry->security8021xHostIP));
                break;
            case COLUMN_SECURITY8021XRADIUSSERVER:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->security8021xRadiusServer,
                                          sizeof(table_entry->security8021xRadiusServer));
                break;
            case COLUMN_SECURITY8021XRADIUSINFOUPDATETIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->security8021xRadiusInfoUpdateTime,
                                          sizeof(table_entry->security8021xRadiusInfoUpdateTime));
                break;
            case COLUMN_SECURITY8021XREAUTHTIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->security8021xReauthTime,
                                          sizeof(table_entry->security8021xReauthTime));
                break;
            case COLUMN_SECURITY8021XID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->security8021xID,
                                          sizeof(table_entry->security8021xID));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot118021xSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITY8021XAUTHIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITY8021XAUTHKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITY8021XRADIUSIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITY8021XRADIUSKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITY8021XHOSTIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITY8021XRADIUSSERVER:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITY8021XRADIUSINFOUPDATETIME:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITY8021XREAUTHTIME:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITY8021XID:
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
            table_entry = (struct dot118021xSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
            switch (table_info->colnum) {
            case COLUMN_SECURITY8021XAUTHIP:
					{
						int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,1,table_entry->security8021xID,request->requestvb->val.string,1812,table_entry->security8021xAuthKey);
				if(ret ==1)
					{
					  table_entry->security8021xAuthIP     = strdup(request->requestvb->val.string);
					}
				}
				break;
            case COLUMN_SECURITY8021XAUTHKEY:
                {
						int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,1,table_entry->security8021xID,table_entry->security8021xAuthIP,1812,request->requestvb->val.string);
				if(ret ==1)
					{
					  table_entry->security8021xAuthKey= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITY8021XRADIUSIP:
                {
				int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,0,table_entry->security8021xID,request->requestvb->val.string,1813,table_entry->security8021xRadiusKey);
				if(ret ==1)
					{
					  table_entry->security8021xRadiusIP = strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITY8021XRADIUSKEY:
                {
				int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,0,table_entry->security8021xID,table_entry->security8021xRadiusKey,1813,request->requestvb->val.string);
				if(ret ==1)
					{
					  table_entry->security8021xRadiusKey= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITY8021XHOSTIP:
                	{
						int ret = 0;
						ret = security_host_ip(0,table_entry->security8021xID,request->requestvb->val.string);
						if(ret == 1)
							{
								table_entry->security8021xHostIP = strdup(request->requestvb->val.string);
							}
					}
                break;
            case COLUMN_SECURITY8021XRADIUSSERVER:
					{
						int ret = 0;
               			ret = radius_server(0,table_entry->security8021xID,request->requestvb->val.integer);
						if(ret == 1)
							{
							 table_entry->security8021xRadiusServer = *request->requestvb->val.integer;
							}
					}
                break;
            case COLUMN_SECURITY8021XRADIUSINFOUPDATETIME:
               	{
					int ret = 0;
					ret = set_acct_interim_interval(0,table_entry->security8021xID,*request->requestvb->val.integer);
					if(ret == 1)
						{	
							table_entry->security8021xRadiusInfoUpdateTime = *request->requestvb->val.integer;
						}
				}
                break;
            case COLUMN_SECURITY8021XREAUTHTIME:
                	{
						int ret = 0;
						ret = set_eap_reauth_period_cmd(0,table_entry->security8021xID,*request->requestvb->val.integer);
						if(ret == 1)
						{
							table_entry->security8021xReauthTime = *request->requestvb->val.integer;
						}
					}
                break;
            case COLUMN_SECURITY8021XID:
                /* Need to save old 'table_entry->security8021xID' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xID = table_entry->security8021xID;
               // table_entry->security8021xID     = *request->requestvb->val.integer;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot118021xSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITY8021XAUTHIP:
                /* Need to restore old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
                table_entry->security8021xAuthIP = table_entry->old_security8021xAuthIP;
                break;
            case COLUMN_SECURITY8021XAUTHKEY:
                /* Need to restore old 'table_entry->security8021xAuthKey' value.
                   May need to use 'memcpy' */
                table_entry->security8021xAuthKey = table_entry->old_security8021xAuthKey;
                break;
            case COLUMN_SECURITY8021XRADIUSIP:
                /* Need to restore old 'table_entry->security8021xRadiusIP' value.
                   May need to use 'memcpy' */
                table_entry->security8021xRadiusIP = table_entry->old_security8021xRadiusIP;
                break;
            case COLUMN_SECURITY8021XRADIUSKEY:
                /* Need to restore old 'table_entry->security8021xRadiusKey' value.
                   May need to use 'memcpy' */
                table_entry->security8021xRadiusKey = table_entry->old_security8021xRadiusKey;
                break;
            case COLUMN_SECURITY8021XHOSTIP:
                /* Need to restore old 'table_entry->security8021xHostIP' value.
                   May need to use 'memcpy' */
                table_entry->security8021xHostIP = table_entry->old_security8021xHostIP;
                break;
            case COLUMN_SECURITY8021XRADIUSSERVER:
                /* Need to restore old 'table_entry->security8021xRadiusServer' value.
                   May need to use 'memcpy' */
                table_entry->security8021xRadiusServer = table_entry->old_security8021xRadiusServer;
                break;
            case COLUMN_SECURITY8021XRADIUSINFOUPDATETIME:
                /* Need to restore old 'table_entry->security8021xRadiusInfoUpdateTime' value.
                   May need to use 'memcpy' */
                table_entry->security8021xRadiusInfoUpdateTime = table_entry->old_security8021xRadiusInfoUpdateTime;
                break;
            case COLUMN_SECURITY8021XREAUTHTIME:
                /* Need to restore old 'table_entry->security8021xReauthTime' value.
                   May need to use 'memcpy' */
                table_entry->security8021xReauthTime = table_entry->old_security8021xReauthTime;
                break;
            case COLUMN_SECURITY8021XID:
                /* Need to restore old 'table_entry->security8021xID' value.
                   May need to use 'memcpy' */
                table_entry->security8021xID = table_entry->old_security8021xID;
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
