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
* dot11WPA2ESecurityTable.c
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
#include "dot11WPA2ESecurityTable.h"
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

#define DOT11WPA2ETABLE "2.14.7"

/** Initializes the dot11WPA2ESecurityTable module */
void
init_dot11WPA2ESecurityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WPA2ESecurityTable();
}

/** Initialize the dot11WPA2ESecurityTable table by defining its contents and how it's structured */
void
initialize_table_dot11WPA2ESecurityTable(void)
{
    static oid dot11WPA2ESecurityTable_oid[128] = {0};
    size_t dot11WPA2ESecurityTable_oid_len   = 0;
	
	mad_dev_oid(dot11WPA2ESecurityTable_oid,DOT11WPA2ETABLE,&dot11WPA2ESecurityTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WPA2ESecurityTable",     dot11WPA2ESecurityTable_handler,
              dot11WPA2ESecurityTable_oid, dot11WPA2ESecurityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: securityWPA2EIndex */
                           0);
    table_info->min_column = SECURITYWPA2EMIN;
    table_info->max_column = SECURITUWPA2EMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WPA2ESecurityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WPA2ESecurityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct dot11WPA2ESecurityTable_entry {
    /* Index values */
    long securityWPA2EIndex;

    /* Column values */
    long securityWPA2EEncryType;
    long old_securityWPA2EEncryType;
    char *securityWPA2EAuthIP;
    char *old_securityWPA2EAuthIP;
    long securityWPA2EAuthPort;
    char *securityWPA2EAuthKey;
    char *old_securityWPA2EAuthKey;
    char *securityWPA2ERadiusIP;
    char *old_securityWPA2ERadiusIP;
    long securityWPA2ERadiusPort;
    char *securityWPA2ERadiusKey;
    char *old_securityWPA2ERadiusKey;
    char * securityWPA2EHostIP;
    char *old_securityWPA2EHostIP;
    long securityWPA2ERadiusServer;
    long old_securityWPA2ERadiusServer;
    long securityWPA2ERadiusInfoUpdateTime;
    long old_securityWPA2ERadiusInfoUpdateTime;
    long securityWPA2EReauthTime;
    long old_securityWPA2EReauthTime;
    long securityWPA2EID;
    long old_securityWPA2EID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WPA2ESecurityTable_entry *next;
};

struct dot11WPA2ESecurityTable_entry  *dot11WPA2ESecurityTable_head;

/* create a new row in the (unsorted) table */
struct dot11WPA2ESecurityTable_entry *
dot11WPA2ESecurityTable_createEntry(
                 long  securityWPA2EIndex,
                 long securityWPA2EEncryType,
			    char * securityWPA2EAuthIP,
			    long securityWPA2EAuthPort,
			    char *securityWPA2EAuthKey,
			   char * securityWPA2ERadiusIP,
			    long securityWPA2ERadiusPort,
			    char *securityWPA2ERadiusKey,
			    char * securityWPA2EHostIP,
			    long securityWPA2ERadiusServer,
			    long securityWPA2ERadiusInfoUpdateTime,
			    long securityWPA2EReauthTime,
			    long securityWPA2EID
                ) {
    struct dot11WPA2ESecurityTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WPA2ESecurityTable_entry);
    if (!entry)
        return NULL;

    entry->securityWPA2EIndex = securityWPA2EIndex;
	 entry->securityWPA2EEncryType = securityWPA2EEncryType;
    entry->securityWPA2EAuthIP = strdup(securityWPA2EAuthIP);
    entry->securityWPA2EAuthPort = securityWPA2EAuthPort;
    entry->securityWPA2EAuthKey  = strdup(securityWPA2EAuthKey);
    entry->securityWPA2ERadiusIP = strdup(securityWPA2ERadiusIP);
    entry->securityWPA2ERadiusPort = securityWPA2ERadiusPort;
    entry->securityWPA2ERadiusKey = strdup(securityWPA2ERadiusKey);
    entry->securityWPA2EHostIP = strdup(securityWPA2EHostIP);
    entry->securityWPA2ERadiusServer = securityWPA2ERadiusServer;
    entry->securityWPA2ERadiusInfoUpdateTime = securityWPA2ERadiusInfoUpdateTime;
    entry->securityWPA2EReauthTime = securityWPA2EReauthTime;
    entry->securityWPA2EID = securityWPA2EID;
    entry->next = dot11WPA2ESecurityTable_head;
    dot11WPA2ESecurityTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WPA2ESecurityTable_removeEntry( struct dot11WPA2ESecurityTable_entry *entry ) {
    struct dot11WPA2ESecurityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WPA2ESecurityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WPA2ESecurityTable_head = ptr->next;
    else
        prev->next = ptr->next;
	free(entry->securityWPA2EAuthKey);
	free(entry->securityWPA2ERadiusKey);
	free(entry->securityWPA2EAuthIP);
	free(entry->securityWPA2EHostIP);
	free(entry->securityWPA2ERadiusIP);
	
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WPA2ESecurityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{

	static int flag = 0;
		if(flag%3==0)
			{
				struct dot11WPA2ESecurityTable_entry *temp;	
				 while( dot11WPA2ESecurityTable_head ){
								  temp=dot11WPA2ESecurityTable_head->next;
								  dot11WPA2ESecurityTable_removeEntry(dot11WPA2ESecurityTable_head);
								  dot11WPA2ESecurityTable_head=temp;
				 }


	///////////////////
	           {
							int up_value = 0;
							int sec_num = 0;
							int result1 = 0;
					  		struct dcli_security *head,*q;          
							int i = 0;
							//security
							struct dcli_security *security;
							int result2 = 0;

							result1= show_security_list(0,&head,&sec_num);
							if(result1 == 1)
							{
								q=head;
								for(i=0;i<sec_num;i++)
								{
									if(q->SecurityType==WPA2_E)
									{
										result2 = show_security_one(0,q->SecurityID,&security);
										if(result2 == 1)
										{
											dot11WPA2ESecurityTable_createEntry(++up_value,
																				(security->EncryptionType)-1,
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

	///////////////////
			//	{
				
			//		 dot11WPA2ESecurityTable_createEntry(1,2,3,4,"5",6,7,"8",9,10,11,12,13);
			
			//    }

	/////////////////////
	
				flag = 0;
			}
		++flag;
		
	*my_data_context = dot11WPA2ESecurityTable_head;
    *my_loop_context = dot11WPA2ESecurityTable_head;
    return dot11WPA2ESecurityTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WPA2ESecurityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WPA2ESecurityTable_entry *entry = (struct dot11WPA2ESecurityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->securityWPA2EIndex, sizeof(entry->securityWPA2EIndex) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WPA2ESecurityTable table */
int
dot11WPA2ESecurityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WPA2ESecurityTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPA2ESecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);


		if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPA2EINDEX:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2EIndex,
                                          sizeof(table_entry->securityWPA2EIndex));
                break;
            case COLUMN_SECURITYWPA2EENCRYTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2EEncryType,
                                          sizeof(table_entry->securityWPA2EEncryType));
                break;
            case COLUMN_SECURITYWPA2EAUTHIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPA2EAuthIP,
                                          strlen(table_entry->securityWPA2EAuthIP));
                break;
            case COLUMN_SECURITYWPA2EAUTHPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2EAuthPort,
                                          sizeof(table_entry->securityWPA2EAuthPort));
                break;
            case COLUMN_SECURITYWPA2EAUTHKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPA2EAuthKey,
                                          strlen(table_entry->securityWPA2EAuthKey));
                break;
            case COLUMN_SECURITYWPA2ERADIUSIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPA2ERadiusIP,
                                          strlen(table_entry->securityWPA2ERadiusIP));
                break;
            case COLUMN_SECURITYWPA2ERADIUSPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2ERadiusPort,
                                          sizeof(table_entry->securityWPA2ERadiusPort));
                break;
            case COLUMN_SECURITYWPA2ERADIUSKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPA2ERadiusKey,
                                          strlen(table_entry->securityWPA2ERadiusKey));
                break;
            case COLUMN_SECURITYWPA2EHOSTIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPA2EHostIP,
                                          strlen(table_entry->securityWPA2EHostIP));
                break;
            case COLUMN_SECURITYWPA2ERADIUSSERVER:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2ERadiusServer,
                                          sizeof(table_entry->securityWPA2ERadiusServer));
                break;
            case COLUMN_SECURITYWPA2ERADIUSINFOUPDATETIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2ERadiusInfoUpdateTime,
                                          sizeof(table_entry->securityWPA2ERadiusInfoUpdateTime));
                break;
            case COLUMN_SECURITYWPA2EREAUTHTIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2EReauthTime,
                                          sizeof(table_entry->securityWPA2EReauthTime));
                break;
            case COLUMN_SECURITYWPA2EID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPA2EID,
                                          sizeof(table_entry->securityWPA2EID));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPA2ESecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPA2EENCRYTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2EAUTHIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2EAUTHKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2ERADIUSIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2ERADIUSKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2EHOSTIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2ERADIUSSERVER:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2ERADIUSINFOUPDATETIME:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2EREAUTHTIME:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPA2EID:
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
            table_entry = (struct dot11WPA2ESecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPA2EENCRYTYPE:
               {
							int ret = 0;
				if(*request->requestvb->val.integer==1)
                	ret = encryption_type(0,table_entry->securityWPA2EID,"AES");
				else
					{
						ret = encryption_type(0,table_entry->securityWPA2EID,"TKIP");
					}
				if(ret == 1)
					{
						table_entry->securityWPA2EEncryType= *request->requestvb->val.integer;
					}
            	}
                break;
            case COLUMN_SECURITYWPA2EAUTHIP:
               {
					int ret = 0;
         
              
				ret = security_auth_acct(0,1,table_entry->securityWPA2EID,request->requestvb->val.string,1812,table_entry->securityWPA2EAuthKey);
				if(ret ==1)
					{
					  table_entry->securityWPA2EAuthIP= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYWPA2EAUTHKEY:
               {
				int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,1,table_entry->securityWPA2EID,table_entry->securityWPA2EAuthIP,1812,request->requestvb->val.string);
				if(ret ==1)
					{
					  table_entry->securityWPA2EAuthKey= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYWPA2ERADIUSIP:
                {
				int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,0,table_entry->securityWPA2EID,request->requestvb->val.string,1813,table_entry->securityWPA2ERadiusKey);
				if(ret ==1)
					{
					  table_entry->securityWPA2ERadiusIP= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYWPA2ERADIUSKEY:
                {
				int ret = 0;
                /* Need to save old 'table_entry->security8021xAuthIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_security8021xAuthIP = strdup(table_entry->security8021xAuthIP);
              
				ret = security_auth_acct(0,0,table_entry->securityWPA2EID,table_entry->securityWPA2ERadiusIP,1813,request->requestvb->val.string);
				if(ret ==1)
					{
					  table_entry->securityWPA2ERadiusKey= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYWPA2EHOSTIP:
               {
						int ret = 0;
						ret = security_host_ip(0,table_entry->securityWPA2EID,request->requestvb->val.string);
						if(ret == 1)
							{
								table_entry->securityWPA2EHostIP= strdup(request->requestvb->val.string);
							}
					}
                break;
            case COLUMN_SECURITYWPA2ERADIUSSERVER:
                 {
						int ret = 0;
               			ret = radius_server(0,table_entry->securityWPA2EID,*request->requestvb->val.integer);
						if(ret == 1)
							{
							 table_entry->securityWPA2ERadiusServer= *request->requestvb->val.integer;
							}
					}
                break;
            case COLUMN_SECURITYWPA2ERADIUSINFOUPDATETIME:
               {
					int ret = 0;
					ret = set_acct_interim_interval(0,table_entry->securityWPA2EID,*request->requestvb->val.integer);
					if(ret == 1)
						{	
							table_entry->securityWPA2ERadiusInfoUpdateTime= *request->requestvb->val.integer;
						}
				}
                break;
            case COLUMN_SECURITYWPA2EREAUTHTIME:
              {
						int ret = 0;
						ret = set_eap_reauth_period_cmd(0,table_entry->securityWPA2EID,*request->requestvb->val.integer);
						if(ret ==1)
							{
								table_entry->securityWPA2EID= *request->requestvb->val.integer;
							}
					}
                break;
            case COLUMN_SECURITYWPA2EID:
                /* Need to save old 'table_entry->securityWPA2EID' value.
                   May need to use 'memcpy' */
                table_entry->old_securityWPA2EID = table_entry->securityWPA2EID;
               // table_entry->securityWPA2EID     = request->requestvb->val.YYY;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPA2ESecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPA2EENCRYTYPE:
                /* Need to restore old 'table_entry->securityWPA2EEncryType' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2EEncryType = table_entry->old_securityWPA2EEncryType;
                break;
            case COLUMN_SECURITYWPA2EAUTHIP:
                /* Need to restore old 'table_entry->securityWPA2EAuthIP' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2EAuthIP = table_entry->old_securityWPA2EAuthIP;
                break;
            case COLUMN_SECURITYWPA2EAUTHKEY:
                /* Need to restore old 'table_entry->securityWPA2EAuthKey' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2EAuthKey = table_entry->old_securityWPA2EAuthKey;
                break;
            case COLUMN_SECURITYWPA2ERADIUSIP:
                /* Need to restore old 'table_entry->securityWPA2ERadiusIP' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2ERadiusIP = table_entry->old_securityWPA2ERadiusIP;
                break;
            case COLUMN_SECURITYWPA2ERADIUSKEY:
                /* Need to restore old 'table_entry->securityWPA2ERadiusKey' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2ERadiusKey = table_entry->old_securityWPA2ERadiusKey;
                break;
            case COLUMN_SECURITYWPA2EHOSTIP:
                /* Need to restore old 'table_entry->securityWPA2EHostIP' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2EHostIP = table_entry->old_securityWPA2EHostIP;
                break;
            case COLUMN_SECURITYWPA2ERADIUSSERVER:
                /* Need to restore old 'table_entry->securityWPA2ERadiusServer' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2ERadiusServer = table_entry->old_securityWPA2ERadiusServer;
                break;
            case COLUMN_SECURITYWPA2ERADIUSINFOUPDATETIME:
                /* Need to restore old 'table_entry->securityWPA2ERadiusInfoUpdateTime' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2ERadiusInfoUpdateTime = table_entry->old_securityWPA2ERadiusInfoUpdateTime;
                break;
            case COLUMN_SECURITYWPA2EREAUTHTIME:
                /* Need to restore old 'table_entry->securityWPA2EReauthTime' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2EReauthTime = table_entry->old_securityWPA2EReauthTime;
                break;
            case COLUMN_SECURITYWPA2EID:
                /* Need to restore old 'table_entry->securityWPA2EID' value.
                   May need to use 'memcpy' */
                table_entry->securityWPA2EID = table_entry->old_securityWPA2EID;
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
