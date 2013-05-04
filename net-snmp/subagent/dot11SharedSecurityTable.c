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
* dot11SharedSecurityTable.c
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
#include "dot11SharedSecurityTable.h"
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
/** Initializes the dot11SharedSecurityTable module */

#define DOT11SHARESECURITYTABLE "2.14.4"

void
init_dot11SharedSecurityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11SharedSecurityTable();
}

/** Initialize the dot11SharedSecurityTable table by defining its contents and how it's structured */
void
initialize_table_dot11SharedSecurityTable(void)
{
    static oid dot11SharedSecurityTable_oid[128] = {0};
    size_t dot11SharedSecurityTable_oid_len   = 0;
	mad_dev_oid(dot11SharedSecurityTable_oid,DOT11SHARESECURITYTABLE,&dot11SharedSecurityTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11SharedSecurityTable",     dot11SharedSecurityTable_handler,
              dot11SharedSecurityTable_oid, dot11SharedSecurityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: securitySharedIndex */
                           0);
    table_info->min_column = SECURITYSHAREDMIN;
    table_info->max_column = SECURITYSHAREDMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11SharedSecurityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11SharedSecurityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct dot11SharedSecurityTable_entry {
    /* Index values */
    long securitySharedIndex;

    /* Column values */
    char *securitySharedEncryType;
    long securitySharedKeyInputType;
    long old_securitySharedKeyInputType;
    char *securitySharedKey;
    char *old_securitySharedKey;
    long securitySharedExtensibleAuth;
    long old_securitySharedExtensibleAuth;
    char *securitySharedAuthIP;
    char *old_securitySharedAuthIP;
    long securitySharedAuthPort;
    char *securitySharedAuthKey;
    char *old_securitySharedAuthKey;
    char *securitySharedRadiusIP;
    char *old_securitySharedRadiusIP;
    long securitySharedRadiusPort;
    char *securitySharedRadiusKey;
    char *old_securitySharedRadiusKey;
    char *securitySharedHostIP;
    char *old_securitySharedHostIP;
    long securitySharedRadiusServer;
    long old_securitySharedRadiusServer;
    long securitySharedRadiusInfoUpdateTime;
    long old_securitySharedRadiusInfoUpdateTime;
    long securitySharedReauthTime;
    long old_securitySharedReauthTime;
    long securitySharedID;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11SharedSecurityTable_entry *next;
};

struct dot11SharedSecurityTable_entry  *dot11SharedSecurityTable_head;

/* create a new row in the (unsorted) table */
struct dot11SharedSecurityTable_entry *
dot11SharedSecurityTable_createEntry(
                 long  securitySharedIndex,
			    char *securitySharedEncryType,
			    long securitySharedKeyInputType,
			    char *securitySharedKey,
			    long securitySharedExtensibleAuth,
			    char *securitySharedAuthIP,
			    long securitySharedAuthPort,
			    char *securitySharedAuthKey,
			    char *securitySharedRadiusIP,
			    long securitySharedRadiusPort,
			    char *securitySharedRadiusKey,
			    char *securitySharedHostIP,
			    long securitySharedRadiusServer,
			    long securitySharedRadiusInfoUpdateTime,
			    long securitySharedReauthTime,
			    long securitySharedID
                ) {
    struct dot11SharedSecurityTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11SharedSecurityTable_entry);
    if (!entry)
        return NULL;

    entry->securitySharedIndex = securitySharedIndex;
    entry->securitySharedEncryType = strdup(securitySharedEncryType);
    entry->securitySharedKeyInputType = securitySharedKeyInputType;
    entry->securitySharedKey = strdup(securitySharedKey);
    entry->securitySharedExtensibleAuth = securitySharedExtensibleAuth;
    entry->securitySharedAuthIP = strdup(securitySharedAuthIP);
    entry->securitySharedAuthPort = securitySharedAuthPort;
    entry->securitySharedAuthKey = strdup(securitySharedAuthKey);
    entry->securitySharedRadiusIP = strdup(securitySharedRadiusIP);
    entry->securitySharedRadiusPort = securitySharedRadiusPort;
    entry->securitySharedRadiusKey = strdup(securitySharedRadiusKey);
    entry->securitySharedHostIP = strdup(securitySharedHostIP);
    entry->securitySharedRadiusServer = securitySharedRadiusServer;
    entry->securitySharedRadiusInfoUpdateTime = securitySharedRadiusInfoUpdateTime;
    entry->securitySharedReauthTime = securitySharedReauthTime;
    entry->securitySharedID = securitySharedID;
    entry->next = dot11SharedSecurityTable_head;
    dot11SharedSecurityTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11SharedSecurityTable_removeEntry( struct dot11SharedSecurityTable_entry *entry ) {
    struct dot11SharedSecurityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11SharedSecurityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11SharedSecurityTable_head = ptr->next;
    else
        prev->next = ptr->next;
	free(entry->securitySharedAuthKey);
	free(entry->securitySharedRadiusKey);
	free(entry->securitySharedEncryType);
	free(entry->securitySharedKey);
	free(entry->old_securitySharedAuthIP);
	free(entry->securitySharedRadiusIP);
	free(entry->securitySharedHostIP);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}



/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11SharedSecurityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{


	static int flag = 0;
	if(flag%3==0)
	{
		struct dot11SharedSecurityTable_entry *temp;	
		while( dot11SharedSecurityTable_head )
		{
			temp=dot11SharedSecurityTable_head->next;
			dot11SharedSecurityTable_removeEntry(dot11SharedSecurityTable_head);
			dot11SharedSecurityTable_head=temp;
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

		result1= show_security_list(0,&head,&sec_num);
		if(result1 == 1)
		{
			q=head;
			for(i=0;i<sec_num;i++)
			{
				if(q->SecurityType==SHARED)
				{
					result2 = show_security_one(0,q->SecurityID,&security);
					if(result2 == 1)
					{
						dot11SharedSecurityTable_createEntry(++up_value,
															"WEP",
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
					if(result2==1)
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

	*my_data_context = dot11SharedSecurityTable_head;
	*my_loop_context = dot11SharedSecurityTable_head;
	return dot11SharedSecurityTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11SharedSecurityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11SharedSecurityTable_entry *entry = (struct dot11SharedSecurityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->securitySharedIndex, sizeof(entry->securitySharedIndex) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11SharedSecurityTable table */
int
dot11SharedSecurityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11SharedSecurityTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SharedSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

		if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
	
            switch (table_info->colnum) {
            case COLUMN_SECURITYSHAREDINDEX:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securitySharedIndex,
                                          sizeof(table_entry->securitySharedIndex));
                break;
            case COLUMN_SECURITYSHAREDENCRYTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securitySharedEncryType,
                                          strlen(table_entry->securitySharedEncryType));
                break;
            case COLUMN_SECURITYSHAREDKEYINPUTTYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securitySharedKeyInputType,
                                          sizeof(table_entry->securitySharedKeyInputType));
                break;
            case COLUMN_SECURITYSHAREDKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->securitySharedKey,
                                          strlen(table_entry->securitySharedKey));
                break;
            case COLUMN_SECURITYSHAREDEXTENSIBLEAUTH:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securitySharedExtensibleAuth,
                                          sizeof(table_entry->securitySharedExtensibleAuth));
                break;
            case COLUMN_SECURITYSHAREDAUTHIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char*)table_entry->securitySharedAuthIP,
                                          strlen(table_entry->securitySharedAuthIP));
                break;
            case COLUMN_SECURITYSHAREDAUTHPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char*)&table_entry->securitySharedAuthPort,
                                          sizeof(table_entry->securitySharedAuthPort));
                break;
            case COLUMN_SECURITYSHAREDAUTHKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char*)table_entry->securitySharedAuthKey,
                                          strlen(table_entry->securitySharedAuthKey));
                break;
            case COLUMN_SECURITYSHAREDRADIUSIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char*)table_entry->securitySharedRadiusIP,
                                          strlen(table_entry->securitySharedRadiusIP));
                break;
            case COLUMN_SECURITYSHAREDRADIUSPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char*)&table_entry->securitySharedRadiusPort,
                                          sizeof(table_entry->securitySharedRadiusPort));
                break;
            case COLUMN_SECURITYSHAREDRADIUSKEY:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char*)table_entry->securitySharedRadiusKey,
                                          strlen(table_entry->securitySharedRadiusKey));
                break;
            case COLUMN_SECURITYSHAREDHOSTIP:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char*)table_entry->securitySharedHostIP,
                                          strlen(table_entry->securitySharedHostIP));
                break;
            case COLUMN_SECURITYSHAREDRADIUSSERVER:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char*)&table_entry->securitySharedRadiusServer,
                                          sizeof(table_entry->securitySharedRadiusServer));
                break;
            case COLUMN_SECURITYSHAREDRADIUSINFOUPDATETIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char*)&table_entry->securitySharedRadiusInfoUpdateTime,
                                          sizeof(table_entry->securitySharedRadiusInfoUpdateTime));
                break;
            case COLUMN_SECURITYSHAREDREAUTHTIME:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char*)&table_entry->securitySharedReauthTime,
                                          sizeof(table_entry->securitySharedReauthTime));
                break;
            case COLUMN_SECURITYSHAREDID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->securitySharedID,
                                          sizeof(table_entry->securitySharedID));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11SharedSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYSHAREDKEYINPUTTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDEXTENSIBLEAUTH:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDAUTHIP:
                if ( request->requestvb->type != ASN_IPADDRESS ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDAUTHKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDRADIUSIP:
                if ( request->requestvb->type != ASN_IPADDRESS ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDRADIUSKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDHOSTIP:
                if ( request->requestvb->type != ASN_IPADDRESS ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDRADIUSSERVER:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDRADIUSINFOUPDATETIME:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYSHAREDREAUTHTIME:
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
            case COLUMN_SECURITYSHAREDKEYINPUTTYPE:
				{
					
                /* Need to save old 'table_entry->securitySharedKeyInputType' value.
                   May need to use 'memcpy' */
               // table_entry->old_securitySharedKeyInputType = table_entry->securitySharedKeyInputType;
                //table_entry->securitySharedKeyInputType     = *request->requestvb->val.integer;
                char * InputType = (char*) malloc(20);
				memset(InputType,0,20);
				if(*request->requestvb->val.integer==1)
					{
					 strcpy(InputType,"ASCII");
					}
				else
					{
					 strcpy(InputType,"HEX");
					}
				int ret = 0;
				ret = security_key(0,table_entry->securitySharedID,table_entry->securitySharedKey,InputType);
				free(InputType);
            	}
                break;
            case COLUMN_SECURITYSHAREDKEY:
					{
					
                /* Need to save old 'table_entry->securitySharedKeyInputType' value.
                   May need to use 'memcpy' */
               // table_entry->old_securitySharedKeyInputType = table_entry->securitySharedKeyInputType;
                //table_entry->securitySharedKeyInputType     = *request->requestvb->val.integer;
                char * InputType = (char*) malloc(20);
				memset(InputType,0,20);
				if(table_entry->securitySharedKeyInputType==1)
					{
					 strcpy(InputType,"ASCII");
					}
				else
					{
					 strcpy(InputType,"HEX");
					}
				security_key(0,table_entry->securitySharedID,request->requestvb->val.string,InputType);
				 table_entry->securitySharedKeyInputType   =strdup(request->requestvb->val.string);
				free(InputType);
            	}
                break;
            case COLUMN_SECURITYSHAREDEXTENSIBLEAUTH:
                /* Need to save old 'table_entry->securitySharedExtensibleAuth' value.
                   May need to use 'memcpy' */
                //table_entry->old_securitySharedExtensibleAuth = table_entry->securitySharedExtensibleAuth;
                //table_entry->securitySharedExtensibleAuth     = *request->requestvb->val.integer;
                break;
            case COLUMN_SECURITYSHAREDAUTHIP:
                /* Need to save old 'table_entry->securitySharedAuthIP' value.
                   May need to use 'memcpy' */
                //table_entry->old_securitySharedAuthIP = table_entry->securitySharedAuthIP;
                //table_entry->securitySharedAuthIP     = request->requestvb->val.YYY;
                break;
            case COLUMN_SECURITYSHAREDAUTHKEY:
                /* Need to save old 'table_entry->securitySharedAuthKey' value.
                   May need to use 'memcpy' */
                //table_entry->old_securitySharedAuthKey = table_entry->securitySharedAuthKey;
                //table_entry->securitySharedAuthKey     = request->requestvb->val.YYY;
                break;
            case COLUMN_SECURITYSHAREDRADIUSIP:
                /* Need to save old 'table_entry->securitySharedRadiusIP' value.
                   May need to use 'memcpy' */
                //table_entry->old_securitySharedRadiusIP = table_entry->securitySharedRadiusIP;
                //table_entry->securitySharedRadiusIP     = request->requestvb->val.YYY;
                break;
            case COLUMN_SECURITYSHAREDRADIUSKEY:
                /* Need to save old 'table_entry->securitySharedRadiusKey' value.
                   May need to use 'memcpy' */
               // table_entry->old_securitySharedRadiusKey = table_entry->securitySharedRadiusKey;
               // table_entry->securitySharedRadiusKey     = request->requestvb->val.YYY;
                break;
            case COLUMN_SECURITYSHAREDHOSTIP:
                /* Need to save old 'table_entry->securitySharedHostIP' value.
                   May need to use 'memcpy' */
               // table_entry->old_securitySharedHostIP = table_entry->securitySharedHostIP;
               // table_entry->securitySharedHostIP     = request->requestvb->val.YYY;
                break;
            case COLUMN_SECURITYSHAREDRADIUSSERVER:
                /* Need to save old 'table_entry->securitySharedRadiusServer' value.
                   May need to use 'memcpy' */
               // table_entry->old_securitySharedRadiusServer = table_entry->securitySharedRadiusServer;
               // table_entry->securitySharedRadiusServer     = request->requestvb->val.YYY;
                break;
            case COLUMN_SECURITYSHAREDRADIUSINFOUPDATETIME:
                /* Need to save old 'table_entry->securitySharedRadiusInfoUpdateTime' value.
                   May need to use 'memcpy' */
                //table_entry->old_securitySharedRadiusInfoUpdateTime = table_entry->securitySharedRadiusInfoUpdateTime;
                //table_entry->securitySharedRadiusInfoUpdateTime     = request->requestvb->val.YYY;
                break;
            case COLUMN_SECURITYSHAREDREAUTHTIME:
                /* Need to save old 'table_entry->securitySharedReauthTime' value.
                   May need to use 'memcpy' */
               // table_entry->old_securitySharedReauthTime = table_entry->securitySharedReauthTime;
               // table_entry->securitySharedReauthTime     = request->requestvb->val.YYY;
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
            case COLUMN_SECURITYSHAREDKEYINPUTTYPE:
                /* Need to restore old 'table_entry->securitySharedKeyInputType' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedKeyInputType = table_entry->old_securitySharedKeyInputType;
                break;
            case COLUMN_SECURITYSHAREDKEY:
                /* Need to restore old 'table_entry->securitySharedKey' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedKey = table_entry->old_securitySharedKey;
                break;
            case COLUMN_SECURITYSHAREDEXTENSIBLEAUTH:
                /* Need to restore old 'table_entry->securitySharedExtensibleAuth' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedExtensibleAuth = table_entry->old_securitySharedExtensibleAuth;
                break;
            case COLUMN_SECURITYSHAREDAUTHIP:
                /* Need to restore old 'table_entry->securitySharedAuthIP' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedAuthIP = table_entry->old_securitySharedAuthIP;
                break;
            case COLUMN_SECURITYSHAREDAUTHKEY:
                /* Need to restore old 'table_entry->securitySharedAuthKey' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedAuthKey = table_entry->old_securitySharedAuthKey;
                break;
            case COLUMN_SECURITYSHAREDRADIUSIP:
                /* Need to restore old 'table_entry->securitySharedRadiusIP' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedRadiusIP = table_entry->old_securitySharedRadiusIP;
                break;
            case COLUMN_SECURITYSHAREDRADIUSKEY:
                /* Need to restore old 'table_entry->securitySharedRadiusKey' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedRadiusKey = table_entry->old_securitySharedRadiusKey;
                break;
            case COLUMN_SECURITYSHAREDHOSTIP:
                /* Need to restore old 'table_entry->securitySharedHostIP' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedHostIP = table_entry->old_securitySharedHostIP;
                break;
            case COLUMN_SECURITYSHAREDRADIUSSERVER:
                /* Need to restore old 'table_entry->securitySharedRadiusServer' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedRadiusServer = table_entry->old_securitySharedRadiusServer;
                break;
            case COLUMN_SECURITYSHAREDRADIUSINFOUPDATETIME:
                /* Need to restore old 'table_entry->securitySharedRadiusInfoUpdateTime' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedRadiusInfoUpdateTime = table_entry->old_securitySharedRadiusInfoUpdateTime;
                break;
            case COLUMN_SECURITYSHAREDREAUTHTIME:
                /* Need to restore old 'table_entry->securitySharedReauthTime' value.
                   May need to use 'memcpy' */
                table_entry->securitySharedReauthTime = table_entry->old_securitySharedReauthTime;
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
