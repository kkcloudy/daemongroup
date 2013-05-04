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
* dot11WPAESecurityTable.c
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
#include "dot11WPAESecurityTable.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_security.h"


#include "autelanWtpGroup.h"

#define DOT11WPAETABLE "2.14.6"

/** Initializes the dot11WPAESecurityTable module */
void
init_dot11WPAESecurityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WPAESecurityTable();
}

/** Initialize the dot11WPAESecurityTable table by defining its contents and how it's structured */
void
initialize_table_dot11WPAESecurityTable(void)
{
    static oid dot11WPAESecurityTable_oid[128] = {0};
    size_t dot11WPAESecurityTable_oid_len   = 0;
	
	mad_dev_oid(dot11WPAESecurityTable_oid,DOT11WPAETABLE,&dot11WPAESecurityTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WPAESecurityTable",     dot11WPAESecurityTable_handler,
              dot11WPAESecurityTable_oid, dot11WPAESecurityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: securityWPAEIndex */
                           0);
    table_info->min_column = SECURITYWPAEMIN;
    table_info->max_column = SECURITYWPAEMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WPAESecurityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WPAESecurityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct dot11WPAESecurityTable_entry {
    /* Index values */
    long securityWPAEIndex;

    /* Column values */
    long securityWPAEEncryType;
    long old_securityWPAEEncryType;
    char *securityWPAEAuthIP;
    char *old_securityWPAEAuthIP;
    long securityWPAEAuthPort;
    char *securityWPAEAuthKey;
    char *old_securityWPAEAuthKey;
    char *securityWPAERadiusIP;
    char *old_securityWPAERadiusIP;
    long securityWPAERadiusPort;
    char *securityWPAERadiusKey;
    char *old_securityWPAERadiusKey;
    char *securityWPAEHostIP;
    char *old_securityWPAEHostIP;
    long securityWPAERadiusServer;
    long old_securityWPAERadiusServer;
    long securityWPAERadiusInfoUpdateTime;
    long old_securityWPAERadiusInfoUpdateTime;
    long securityWPAEReauthTime;
    long old_securityWPAEReauthTime;
    long securityWPAEID;
    long old_securityWPAEID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WPAESecurityTable_entry *next;
};

struct dot11WPAESecurityTable_entry  *dot11WPAESecurityTable_head;

/* create a new row in the (unsorted) table */
struct dot11WPAESecurityTable_entry *
dot11WPAESecurityTable_createEntry(
                 long  securityWPAEIndex,
                 long securityWPAEEncryType,
			     char *securityWPAEAuthIP,
			     long securityWPAEAuthPort,
			     char *securityWPAEAuthKey,
			     char *securityWPAERadiusIP,
			     long securityWPAERadiusPort,
			     char *securityWPAERadiusKey,
			     char *securityWPAEHostIP,
			     long securityWPAERadiusServer,
			     long securityWPAERadiusInfoUpdateTime,
			     long securityWPAEReauthTime,
			     long securityWPAEID
                ) {
    struct dot11WPAESecurityTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WPAESecurityTable_entry);
    if (!entry)
        return NULL;

    entry->securityWPAEIndex = securityWPAEIndex;
	entry->securityWPAEEncryType = securityWPAEEncryType;
	entry->securityWPAEAuthIP = strdup(securityWPAEAuthIP);
	entry->securityWPAEAuthPort = securityWPAEAuthPort;
	entry->securityWPAEAuthKey = strdup(securityWPAEAuthKey);
	entry->securityWPAERadiusIP = strdup(securityWPAERadiusIP);
	entry->securityWPAERadiusPort = securityWPAERadiusPort;
	entry->securityWPAERadiusKey = strdup(securityWPAERadiusKey);
	entry->securityWPAEHostIP = strdup(securityWPAEHostIP);
	entry->securityWPAERadiusServer = securityWPAERadiusServer;
	entry->securityWPAERadiusInfoUpdateTime = securityWPAERadiusInfoUpdateTime;
	entry->securityWPAEReauthTime = securityWPAEReauthTime;
	entry->securityWPAEID = securityWPAEID;
    entry->next = dot11WPAESecurityTable_head;
    dot11WPAESecurityTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WPAESecurityTable_removeEntry( struct dot11WPAESecurityTable_entry *entry ) {
    struct dot11WPAESecurityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WPAESecurityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WPAESecurityTable_head = ptr->next;
    else
        prev->next = ptr->next;

   free(entry->securityWPAEAuthKey);
   free(entry->securityWPAERadiusKey);
   free(entry->securityWPAEAuthIP);
   free(entry->securityWPAEHostIP);
   free(entry->securityWPAERadiusIP);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WPAESecurityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{


	static int flag = 0;
			if(flag%3==0)
				{
					struct dot11WPAESecurityTable_entry *temp;	
					 while( dot11WPAESecurityTable_head ){
									  temp=dot11WPAESecurityTable_head->next;
									  dot11WPAESecurityTable_removeEntry(dot11WPAESecurityTable_head);
									  dot11WPAESecurityTable_head=temp;
					 }
					 {
							int up_value = 0;
							int sec_num = 0;
							int result1 = 0;
					  		struct dcli_security *head,*q;          /*存放security信息的链表头*/    
							int i = 0;
							//security
							struct dcli_security *security;
							int result2 = 0;

							result1 = show_security_list(0,&head,&sec_num);
							if(result1 == 1)
							{
								q=head;
								for(i=0;i<sec_num;i++)
								{
									if(q->SecurityType==WPA_E)
									{
										result2 = show_security_one(0,q->SecurityID,&security);
										if(result2 == 1)
										{
											dot11WPAESecurityTable_createEntry(++up_value,
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
		
		
					flag = 0;
				}
			++flag;
			
	*my_data_context = dot11WPAESecurityTable_head;
    *my_loop_context = dot11WPAESecurityTable_head;
    return dot11WPAESecurityTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WPAESecurityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WPAESecurityTable_entry *entry = (struct dot11WPAESecurityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->securityWPAEIndex, sizeof(entry->securityWPAEIndex) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WPAESecurityTable table */
int
dot11WPAESecurityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WPAESecurityTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPAESecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

		if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
	
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPAEINDEX:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPAEIndex,
                                          sizeof(table_entry->securityWPAEIndex));
                break;
            case COLUMN_SECURITYWPAEENCRYTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPAEEncryType,
                                          sizeof(table_entry->securityWPAEEncryType));
                break;
            case COLUMN_SECURITYWPAEAUTHIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPAEAuthIP,
                                          strlen(table_entry->securityWPAEAuthIP));
                break;
            case COLUMN_SECURITYWPAEAUTHPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPAEAuthPort,
                                          sizeof(table_entry->securityWPAEAuthPort));
                break;
            case COLUMN_SECURITYWPAEAUTHKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPAEAuthKey,
                                          strlen(table_entry->securityWPAEAuthKey));
                break;
            case COLUMN_SECURITYWPAERADIUSIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPAERadiusIP,
                                          strlen(table_entry->securityWPAERadiusIP));
                break;
            case COLUMN_SECURITYWPAERADIUSPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPAERadiusPort,
                                          sizeof(table_entry->securityWPAERadiusPort));
                break;
            case COLUMN_SECURITYWPAERADIUSKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPAERadiusKey,
                                          strlen(table_entry->securityWPAERadiusKey));
                break;
            case COLUMN_SECURITYWPAEHOSTIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securityWPAEHostIP,
                                          strlen(table_entry->securityWPAEHostIP));
                break;
            case COLUMN_SECURITYWPAERADIUSSERVER:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPAERadiusServer,
                                          sizeof(table_entry->securityWPAERadiusServer));
                break;
            case COLUMN_SECURITYWPAERADIUSINFOUPDATETIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPAERadiusInfoUpdateTime,
                                          sizeof(table_entry->securityWPAERadiusInfoUpdateTime));
                break;
            case COLUMN_SECURITYWPAEREAUTHTIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPAEReauthTime,
                                          sizeof(table_entry->securityWPAEReauthTime));
                break;
            case COLUMN_SECURITYWPAEID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securityWPAEID,
                                          sizeof(table_entry->securityWPAEID));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPAESecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPAEENCRYTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAEAUTHIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAEAUTHKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAERADIUSIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAERADIUSKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAEHOSTIP:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAERADIUSSERVER:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAERADIUSINFOUPDATETIME:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAEREAUTHTIME:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYWPAEID:
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
            table_entry = (struct dot11WPAESecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPAEENCRYTYPE:
				{
							int ret = 0;
				if(*request->requestvb->val.integer==1)
                	ret = encryption_type(0,table_entry->securityWPAEID,"AES");
				else
					{
						ret = encryption_type(0,table_entry->securityWPAEID,"TKIP");
					}
				if(ret == 1)
					{
						table_entry->securityWPAEEncryType = *request->requestvb->val.integer;
					}
            	}
                break;
            case COLUMN_SECURITYWPAEAUTHIP:
                {
					int ret = 0;              
				ret = security_auth_acct(0,1,table_entry->securityWPAEID,request->requestvb->val.string,1812,table_entry->securityWPAEAuthKey);
				if(ret ==1)
					{
					  table_entry->securityWPAEAuthIP= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYWPAEAUTHKEY:
                {
					int ret = 0;
              
				ret = security_auth_acct(0,1,table_entry->securityWPAEID,table_entry->securityWPAEAuthIP,1812,request->requestvb->val.string);
				if(ret ==1)
					{
					  table_entry->securityWPAEAuthKey= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYWPAERADIUSIP:
                {
					int ret = 0;
              
				ret = security_auth_acct(0,0,table_entry->securityWPAEID,request->requestvb->val.string,1813,table_entry->securityWPAERadiusKey);
				if(ret ==1)
					{
					  table_entry->securityWPAERadiusIP= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYWPAERADIUSKEY:
                {
					int ret = 0;
              
				ret = security_auth_acct(0,0,table_entry->securityWPAEID,table_entry->securityWPAERadiusIP,1813,request->requestvb->val.string);
				if(ret ==1)
					{
					  table_entry->securityWPAERadiusKey= strdup(request->requestvb->val.string);
					}
				}
                break;
            case COLUMN_SECURITYWPAEHOSTIP:
                {
					int ret = 0;
					ret = security_host_ip(0,table_entry->securityWPAEID,request->requestvb->val.string);
					if(ret == 1)
						{
							table_entry->securityWPAEHostIP= strdup(request->requestvb->val.string);
						}
				}
                break;
            case COLUMN_SECURITYWPAERADIUSSERVER:
				{
					int ret = 0;
					ret = radius_server(0,table_entry->securityWPAEID,*request->requestvb->val.integer);
					if(ret == 1)
						{
						 table_entry->securityWPAERadiusServer= *request->requestvb->val.integer;
						}
				}
                break;
            case COLUMN_SECURITYWPAERADIUSINFOUPDATETIME:
				{
					int ret = 0;
					ret = set_acct_interim_interval(0,table_entry->securityWPAEID,*request->requestvb->val.integer);
					if(ret == 1)
						{	
							table_entry->securityWPAERadiusInfoUpdateTime= *request->requestvb->val.integer;
						}
				}
                break;
            case COLUMN_SECURITYWPAEREAUTHTIME:
				{
					int ret = 0;
					ret = set_eap_reauth_period_cmd(0,table_entry->securityWPAEID,*request->requestvb->val.integer);
					if(ret ==1)
						{
							table_entry->securityWPAEReauthTime= *request->requestvb->val.integer;
						}
				}
                break;
            case COLUMN_SECURITYWPAEID:
                /* Need to save old 'table_entry->securityWPAEID' value.
                   May need to use 'memcpy' */
                table_entry->old_securityWPAEID = table_entry->securityWPAEID;
             //   table_entry->securityWPAEID     = request->requestvb->val.YYY;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WPAESecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYWPAEENCRYTYPE:
                /* Need to restore old 'table_entry->securityWPAEEncryType' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAEEncryType = table_entry->old_securityWPAEEncryType;
                break;
            case COLUMN_SECURITYWPAEAUTHIP:
                /* Need to restore old 'table_entry->securityWPAEAuthIP' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAEAuthIP = table_entry->old_securityWPAEAuthIP;
                break;
            case COLUMN_SECURITYWPAEAUTHKEY:
                /* Need to restore old 'table_entry->securityWPAEAuthKey' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAEAuthKey = table_entry->old_securityWPAEAuthKey;
                break;
            case COLUMN_SECURITYWPAERADIUSIP:
                /* Need to restore old 'table_entry->securityWPAERadiusIP' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAERadiusIP = table_entry->old_securityWPAERadiusIP;
                break;
            case COLUMN_SECURITYWPAERADIUSKEY:
                /* Need to restore old 'table_entry->securityWPAERadiusKey' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAERadiusKey = table_entry->old_securityWPAERadiusKey;
                break;
            case COLUMN_SECURITYWPAEHOSTIP:
                /* Need to restore old 'table_entry->securityWPAEHostIP' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAEHostIP = table_entry->old_securityWPAEHostIP;
                break;
            case COLUMN_SECURITYWPAERADIUSSERVER:
                /* Need to restore old 'table_entry->securityWPAERadiusServer' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAERadiusServer = table_entry->old_securityWPAERadiusServer;
                break;
            case COLUMN_SECURITYWPAERADIUSINFOUPDATETIME:
                /* Need to restore old 'table_entry->securityWPAERadiusInfoUpdateTime' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAERadiusInfoUpdateTime = table_entry->old_securityWPAERadiusInfoUpdateTime;
                break;
            case COLUMN_SECURITYWPAEREAUTHTIME:
                /* Need to restore old 'table_entry->securityWPAEReauthTime' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAEReauthTime = table_entry->old_securityWPAEReauthTime;
                break;
            case COLUMN_SECURITYWPAEID:
                /* Need to restore old 'table_entry->securityWPAEID' value.
                   May need to use 'memcpy' */
                table_entry->securityWPAEID = table_entry->old_securityWPAEID;
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
