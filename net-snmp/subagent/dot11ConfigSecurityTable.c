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
* dot11ConfigSecurityTable.c
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
#include "dot11ConfigSecurityTable.h"
#include "autelanWtpGroup.h"
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "wcpss/asd/asd.h"
#include "ws_security.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"

#define CONFIGSECURITY	"2.14.2"

    /* Typical data structure for a row entry */
struct dot11ConfigSecurityTable_entry {
    /* Index values */
	dbus_parameter parameter;
	long globalSecurityID;
    long securityID;

    /* Column values */
    long securityType;
    //long old_securityType;
    long encryptionType;
    //long old_encryptionType;
    long encryptionInputTypeKey;
    //long old_encryptionInputTypeKey;
    char *securityKEY;
    //char *old_securityKEY;
    long extensibleAuth;
    //long old_extensibleAuth;
    char *authIP;
    //char *old_authIP;
    long authPort;
	//long old_authPort;
    char *authSharedSecret;
    //char *old_authSharedSecret;
    char *acctIP;
    //char *old_acctIP;
    long acctPort;
	//long old_acctPort;
    char *acctSharedSecret;
    //char *old_acctSharedSecret;
    char *hostIP;
    //char *old_hostIP;
    long radiusServer;
    //long old_radiusServer;
    long acctInterimInterval;
    //long old_acctInterimInterval;
    long eapReauthPeriod;
    //long old_eapReauthPeriod;
    char *wapiASIP;
    //char *old_wapiASIP;
    long wapiASType;
    //long old_wapiASType;
    char *wapiASCertificationPath;
    //char *old_wapiASCertificationPath;
    long wapiIsInstalledASCer;
    //long old_wapiIsInstalledASCer;
    long wapiUnicastRekeyMethod;
    //long old_wapiUnicastRekeyMethod;
    long wapiMulticastRekeyMethod;
    //long old_wapiMulticastRekeyMethod;
    u_long wapiUnicastRekeyTime;
    //u_long old_wapiUnicastRekeyTime;
    u_long wapiUnicastRekeyPackets;
    //u_long old_wapiUnicastRekeyPackets;
    u_long wapiMulticastRekeyTime;
    //u_long old_wapiMulticastRekeyTime;
    u_long wapiMulticastRekeyPackets;
    //u_long old_wapiMulticastRekeyPackets;
	char *PSKValue;
	//char *old_PSKValue;
	char *PSKPassPhrase;
	//char *old_PSKPassPhrase;
	char *RadiusAuthServerIPAdd;
	//char *old_RadiusAuthServerIPAdd;
	long RadiusAuthServerPort;
	//long old_RadiusAuthServerPort;
	char *RadiusAuthServerSharedKey;
	//char *old_RadiusAuthServerSharedKey;
	char *RadiusAccServerIPAdd;
	//char *old_RadiusAccServerIPAdd;
	long RadiusAccServerPort;
	//long old_RadiusAccServerPort;
	char *RadiusAccServerSharedKey;
	//char *old_RadiusAccServerSharedKey;
    char *wapiAECertificationPath;
    //long old_wapiAECertificationPath;
    long HybridAuthSwitch;
	//long old_HybridAuthSwitch;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11ConfigSecurityTable_entry *next;
};

void dot11ConfigSecurityTable_load();
void
dot11ConfigSecurityTable_removeEntry( struct dot11ConfigSecurityTable_entry *entry );

/** Initializes the dot11ConfigSecurityTable module */
void
init_dot11ConfigSecurityTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11ConfigSecurityTable();
}

/** Initialize the dot11ConfigSecurityTable table by defining its contents and how it's structured */
void
initialize_table_dot11ConfigSecurityTable(void)
{
    static oid dot11ConfigSecurityTable_oid[128] = {0};
    size_t dot11ConfigSecurityTable_oid_len   = 0;
	mad_dev_oid(dot11ConfigSecurityTable_oid,CONFIGSECURITY,&dot11ConfigSecurityTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11ConfigSecurityTable",     dot11ConfigSecurityTable_handler,
              dot11ConfigSecurityTable_oid, dot11ConfigSecurityTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: globalSecurityID */
                           0);
    table_info->min_column = COLUMN_SEMIN;
    table_info->max_column = COLUMN_SEMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11ConfigSecurityTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11ConfigSecurityTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11ConfigSecurityTable_load, dot11ConfigSecurityTable_removeEntry,
							dot11ConfigSecurityTable_oid, dot11ConfigSecurityTable_oid_len));

    /* Initialise the contents of the table here */
}



struct dot11ConfigSecurityTable_entry  *dot11ConfigSecurityTable_head;

/* create a new row in the (unsorted) table */
struct dot11ConfigSecurityTable_entry *
dot11ConfigSecurityTable_createEntry(
								dbus_parameter parameter,
								long globalSecurityID,
								long  securityID,
								long securityType,
								long encryptionType,
								long encryptionInputTypeKey,
								char *securityKEY,
								long extensibleAuth,
								char *authIP,
								long authPort,
								char *authSharedSecret,
								char *acctIP,
								long acctPort,
								char *acctSharedSecret,
								char *hostIP,
								long radiusServer,
								long acctInterimInterval,
								long eapReauthPeriod,
								char *wapiASIP,
								long wapiASType,
								char *wapiASCertificationPath,
								long wapiIsInstalledASCer,
								long wapiUnicastRekeyMethod,
								long wapiMulticastRekeyMethod,
								u_long wapiUnicastRekeyTime,
								u_long wapiUnicastRekeyPackets,
								u_long wapiMulticastRekeyTime,
								u_long wapiMulticastRekeyPackets,
								char *PSKValue,
								char *PSKPassPhrase,
								char *RadiusAuthServerIPAdd,
								long RadiusAuthServerPort,
								char *RadiusAuthServerSharedKey,
								char *RadiusAccServerIPAdd,
								long RadiusAccServerPort,
								char *RadiusAccServerSharedKey,
								char *wapiAECertificationPath,
								long HybridAuthSwitch)
    {
    struct dot11ConfigSecurityTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11ConfigSecurityTable_entry);
    if (!entry)
        return NULL;

	memcpy(&entry->parameter, &parameter, sizeof(dbus_parameter));
	entry->globalSecurityID = globalSecurityID;
    entry->securityID = securityID;
	entry->securityType = securityType;
	entry->encryptionType =encryptionType;
	entry->encryptionInputTypeKey = encryptionInputTypeKey;
	entry->securityKEY = strdup(securityKEY);
	entry->extensibleAuth = extensibleAuth;
	entry->authIP = strdup(authIP);
	entry->authPort = authPort;
	entry->authSharedSecret = strdup(authSharedSecret);
	entry->acctIP = strdup(acctIP);
	entry->acctPort = acctPort;
	entry->acctSharedSecret = strdup(acctSharedSecret);
	entry->hostIP = strdup(hostIP);
	entry->radiusServer = radiusServer;
	entry->acctInterimInterval= acctInterimInterval;
	entry->eapReauthPeriod = eapReauthPeriod;
	entry->wapiASIP = strdup(wapiASIP);
	entry->wapiASType = wapiASType;
	entry->wapiASCertificationPath = strdup(wapiASCertificationPath);
	entry->wapiIsInstalledASCer = wapiIsInstalledASCer;
	entry->wapiUnicastRekeyMethod = wapiUnicastRekeyMethod;
	entry->wapiMulticastRekeyMethod = wapiMulticastRekeyMethod;
	entry->wapiUnicastRekeyTime = wapiUnicastRekeyTime;
	entry->wapiUnicastRekeyPackets = wapiUnicastRekeyPackets;
	entry->wapiMulticastRekeyTime = wapiMulticastRekeyTime;
	entry->wapiMulticastRekeyPackets = wapiMulticastRekeyPackets;
	entry->PSKValue = strdup(PSKValue);
	entry->PSKPassPhrase = strdup(PSKPassPhrase);
	entry->RadiusAuthServerIPAdd = strdup(RadiusAuthServerIPAdd);
	entry->RadiusAuthServerPort = RadiusAuthServerPort;
	entry->RadiusAuthServerSharedKey = strdup(RadiusAuthServerSharedKey);
	entry->RadiusAccServerIPAdd = strdup(RadiusAccServerIPAdd);
	entry->RadiusAccServerPort = RadiusAccServerPort;
	entry->RadiusAccServerSharedKey = strdup(RadiusAccServerSharedKey);
	entry->wapiAECertificationPath = strdup(wapiAECertificationPath);
	entry->HybridAuthSwitch = HybridAuthSwitch;
    entry->next = dot11ConfigSecurityTable_head;
    dot11ConfigSecurityTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11ConfigSecurityTable_removeEntry( struct dot11ConfigSecurityTable_entry *entry ) {
    struct dot11ConfigSecurityTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11ConfigSecurityTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11ConfigSecurityTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->securityKEY);
	FREE_OBJECT(entry->acctIP);
	FREE_OBJECT(entry->authIP);
	FREE_OBJECT(entry->hostIP);
	FREE_OBJECT(entry->authSharedSecret);
	FREE_OBJECT(entry->acctSharedSecret);
	FREE_OBJECT(entry->wapiASIP);
	FREE_OBJECT(entry->wapiASCertificationPath);
	FREE_OBJECT(entry->PSKValue);
	FREE_OBJECT(entry->PSKPassPhrase);
	FREE_OBJECT(entry->RadiusAuthServerIPAdd);
	FREE_OBJECT(entry->RadiusAuthServerSharedKey);
	FREE_OBJECT(entry->RadiusAccServerIPAdd);
	FREE_OBJECT(entry->RadiusAccServerSharedKey);
	FREE_OBJECT(entry->wapiAECertificationPath);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11ConfigSecurityTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11ConfigSecurityTable_load\n");

	struct dot11ConfigSecurityTable_entry *temp;
	while( dot11ConfigSecurityTable_head )
	{
		temp=dot11ConfigSecurityTable_head->next;
		dot11ConfigSecurityTable_removeEntry(dot11ConfigSecurityTable_head);
		dot11ConfigSecurityTable_head=temp;
	}
	
    instance_parameter *paraHead = NULL, *paraNode = NULL;
    list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
    for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) 
	{
		struct dcli_security *head = NULL, *q = NULL;		   /*存放security信息的链表头*/ 	  
		int result = 0;
		int snum = 0;
		int i=0;

		snmp_log(LOG_DEBUG, "enter show_security_list\n");
		result=show_security_list(paraNode->parameter, paraNode->connection,&head,&snum);
		snmp_log(LOG_DEBUG, "exit show_security_list,result=%d\n", result);
		
		if(result == 1)
		{
			for(i=0,q = head; ((i<snum)&&(NULL != q)); i++,q = q->next)
			{
    		    unsigned long globalSecurityID = local_to_global_ID(paraNode->parameter, q->SecurityID, WIRELESS_MAX_NUM);
				dot11ConfigSecurityTable_createEntry(paraNode->parameter,
				                                    globalSecurityID,
													q->SecurityID,
													0,
													0,
													0,
													"",
													0,
													"",
													0,
													"",
													"",
													0,
													"",
													"",
													0,
													0,
													0,
													"",
													0,
													"",
													0,
													0,
													0,
													0,
													0,
													0,
													0,
													"",
													"",
													"",
													0,
													"",
													"",
													0,
													"",
													"",
													0);
			}
			Free_security_head(head);
		}
	}
    free_instance_parameter_list(&paraHead);
    
	snmp_log(LOG_DEBUG, "exit dot11ConfigSecurityTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11ConfigSecurityTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11ConfigSecurityTable_head==NULL)
		return NULL;
	*my_data_context = dot11ConfigSecurityTable_head;
	*my_loop_context = dot11ConfigSecurityTable_head;
	return dot11ConfigSecurityTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11ConfigSecurityTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11ConfigSecurityTable_entry *entry = (struct dot11ConfigSecurityTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->globalSecurityID, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11ConfigSecurityTable table */
int
dot11ConfigSecurityTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11ConfigSecurityTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ConfigSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
		if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    } 
		    void *connection = NULL;
		    if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3)) {
                break;
		    }
		    
            switch (table_info->colnum) {
            case COLUMN_SECURITYTYPE:
			{
				int ret=0;
				int securityType=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					securityType=security->SecurityType;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&securityType,
                                          sizeof(securityType));

				if(ret == 1)
				{
					Free_security_one(security);
				}								
            }
                break;
            case COLUMN_ENCRYPTIONTYPE:
			{
				int ret=0;
				int encryptionType=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					encryptionType=security->EncryptionType;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&encryptionType,
                                          sizeof(encryptionType));

				if(ret == 1)
				{
					Free_security_one(security);
				}				
            }
                break;
            case COLUMN_ENCRYPTIONINPUTTYPEKEY:
			{
				int ret=0;
				int encryptionInputTypeKey=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					encryptionInputTypeKey=security->keyInputType;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&encryptionInputTypeKey,
                                          sizeof(encryptionInputTypeKey));

				if(ret == 1)
				{
					Free_security_one(security);
				}
           	}
                break;
            case COLUMN_SECURITYKEY:
			{
				int ret=0;
				char securityKEY[128] = { 0 };
				memset(securityKEY,0,128);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(securityKEY,0,128);
					if(security->SecurityKey)
					{
						strncpy(securityKEY,security->SecurityKey,sizeof(securityKEY)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)securityKEY,
                                          strlen(securityKEY));
				
				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_EXTENSIBLEAUTH:
			{
				int ret=0;
				int extensibleAuth=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					extensibleAuth=security->extensible_auth;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&extensibleAuth,
                                          sizeof(extensibleAuth));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_AUTHIP:
			{
				int ret=0;
				char authIP[20] = { 0 };
				memset(authIP,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(authIP,0,20);
					if(security->auth.auth_ip)
					{
						strncpy(authIP,security->auth.auth_ip,sizeof(authIP)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)authIP,
                                          strlen(authIP));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_AUTHPORT:
			{
				int ret=0;
				int authPort=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					authPort=security->auth.auth_port;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&authPort,
                                          sizeof(authPort));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_AUTHSHAREDSECRET:
			{
				int ret=0;
				char authSharedSecret[20] = { 0 };
				memset(authSharedSecret,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(authSharedSecret,0,20);
					if(security->auth.auth_shared_secret)
					{
						strncpy(authSharedSecret,security->auth.auth_shared_secret,sizeof(authSharedSecret)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)authSharedSecret,
                                          strlen(authSharedSecret));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_ACCTIP:
			{
				int ret=0;
				char acctIP[20] = { 0 };
				memset(acctIP,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(acctIP,0,20);
					if(security->acct.acct_ip)
					{
						strncpy(acctIP,security->acct.acct_ip,sizeof(acctIP)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)acctIP,
                                          strlen(acctIP));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
                break;
            case COLUMN_ACCTPORT:
			{
				int ret=0;
				int acctPort=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					acctPort=security->acct.acct_port;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&acctPort,
                                          sizeof(acctPort));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_ACCTSHAREDSECRET:
			{
				int ret=0;
				char acctSharedSecret[20] = { 0 }; 
				memset(acctSharedSecret,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(acctSharedSecret,0,20);
					if(security->acct.acct_shared_secret)
					{
						strncpy(acctSharedSecret,security->acct.acct_shared_secret,sizeof(acctSharedSecret)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)acctSharedSecret,
                                          strlen(acctSharedSecret));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_HOSTIP:
			{
				int ret=0;
				char hostIP[20] = { 0 };
				memset(hostIP,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(hostIP,0,20);
					if(security->host_ip)
					{
						strncpy(hostIP,security->host_ip,sizeof(hostIP)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)hostIP,
                                          strlen(hostIP));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_RADIUSSERVER:
			{
				int ret=0;
				int radiusServer=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					if(security->wired_radius==1)
						radiusServer=1;
					else
						radiusServer=0;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&radiusServer,
                                          sizeof(radiusServer));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_ACCTINTERIMINTERVAL:
			{
				int ret=0;
				int acctInterimInterval=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					acctInterimInterval=security->acct_interim_interval;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&acctInterimInterval,
                                          sizeof(acctInterimInterval));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_EAPREAUTHPERIOD:
			{
				int ret=0;
				int eapReauthPeriod=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					eapReauthPeriod=security->eap_reauth_period;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&eapReauthPeriod,
                                          sizeof(eapReauthPeriod));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIASIP:
			{
				int ret=0;
				char wapiASIP[20] = { 0 };
				memset(wapiASIP,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(wapiASIP,0,20);
					if(security->wapi_as.as_ip)
					{
						strncpy(wapiASIP,security->wapi_as.as_ip,sizeof(wapiASIP)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)wapiASIP,
                                          strlen(wapiASIP));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIASTYPE:
			{
				int ret=0;
				int wapiASType=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
					wapiASType=security->wapi_as.certification_type;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&wapiASType,
                                          sizeof(wapiASType));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIASCERTIFICATIONPATH:
			{
				int ret=0;
				char wapiASCertificationPath[DEFAULT_LEN] = { 0 };
				memset(wapiASCertificationPath,0,DEFAULT_LEN);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(wapiASCertificationPath,0,DEFAULT_LEN);
					if(security->wapi_as.certification_path)
					{
						strncpy(wapiASCertificationPath,security->wapi_as.certification_path,sizeof(wapiASCertificationPath)-1);
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)wapiASCertificationPath,
                                          strlen(wapiASCertificationPath));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIISINSTALLEDASCER:
			{
				int ret=0;
				int flag = 2;
				char wapiIsInstalledASCer[DEFAULT_LEN] = { 0 };
				memset(wapiIsInstalledASCer,0,DEFAULT_LEN);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection, table_entry->securityID, &security);
				if(ret==1)
				{
					memset(wapiIsInstalledASCer,0,DEFAULT_LEN);
					if(security->wapi_as.certification_path)
					{
						strncpy(wapiIsInstalledASCer,security->wapi_as.certification_path,sizeof(wapiIsInstalledASCer)-1);
					}
				}

				if((strcmp(wapiIsInstalledASCer,"")!=0)&&(strchr(wapiIsInstalledASCer,' ')==NULL))
					flag=1;
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&flag,
                                          sizeof(flag));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIUNICASTREKEYMETHOD:
			{
				int ret=0;
				int wapiUnicastRekeyMethod=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					if(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK)
					{
						wapiUnicastRekeyMethod=security->wapi_ucast_rekey_method;
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&wapiUnicastRekeyMethod,
                                          sizeof(wapiUnicastRekeyMethod));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIMULTICASTREKEYMETHOD:
			{
				int ret=0;
				int wapiMulticastRekeyMethod=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					if(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK)
					{
						wapiMulticastRekeyMethod=security->wapi_mcast_rekey_method;
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&wapiMulticastRekeyMethod,
                                          sizeof(wapiMulticastRekeyMethod));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIUNICASTREKEYTIME:
			{
				int ret=0;
				unsigned int wapiUnicastRekeyTime=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					if(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK)
					{
						if((security->wapi_ucast_rekey_method==1)||(security->wapi_ucast_rekey_method==3))
						{
							wapiUnicastRekeyTime=security->wapi_ucast_rekey_para_t;
						}
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_UNSIGNED,
                                          (u_char*)&wapiUnicastRekeyTime,
                                          sizeof(wapiUnicastRekeyTime));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIUNICASTREKEYPACKETS:
			{
				int ret=0;
				unsigned int wapiUnicastRekeyPackets=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					if(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK)
					{
						if((security->wapi_ucast_rekey_method==2)||(security->wapi_ucast_rekey_method==3))
						{
							wapiUnicastRekeyPackets=security->wapi_ucast_rekey_para_p;
						}
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_UNSIGNED,
                                          (u_char*)&wapiUnicastRekeyPackets,
                                          sizeof(wapiUnicastRekeyPackets));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIMULTICASTREKEYTIME:
			{
				int ret=0;
				unsigned int wapiMulticastRekeyTime=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					if(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK)
					{
						if((security->wapi_mcast_rekey_method==1)||(security->wapi_mcast_rekey_method==3))
						{
							wapiMulticastRekeyTime=security->wapi_mcast_rekey_para_t;
						}
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_UNSIGNED,
                                          (u_char*)&wapiMulticastRekeyTime,
                                          sizeof(wapiMulticastRekeyTime));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            case COLUMN_WAPIMULTICASTREKEYPACKETS:
			{
				int ret=0;
				unsigned int wapiMulticastRekeyPackets=0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					if(security->SecurityType==WAPI_AUTH||security->SecurityType==WAPI_PSK)
					{
						if((security->wapi_mcast_rekey_method==2)||(security->wapi_mcast_rekey_method==3))
						{
							wapiMulticastRekeyPackets=security->wapi_mcast_rekey_para_p;
						}
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_UNSIGNED,
                                          (u_char*)&wapiMulticastRekeyPackets,
                                          sizeof(wapiMulticastRekeyPackets));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
			case COLUMN_PSKVALUE:
			{
				int ret=0;
				char PSKValue[128] = { 0 };
				memset(PSKValue,0,128);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if((ret==1)&&(security->SecurityType==WAPI_PSK))
				{
					memset(PSKValue,0,128);
					if(security->SecurityKey)
					{
						strncpy(PSKValue,security->SecurityKey,sizeof(PSKValue)-1);
					}
				}
				
				snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)PSKValue,
                                          strlen(PSKValue));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
				break;
			case COLUMN_PSKPASSPHRASE:				
			{
				int ret=0;
				char PSKPassPhrase[128] = { 0 };
				memset(PSKPassPhrase,0,128);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if((ret==1)&&(security->SecurityType==WAPI_PSK))
				{
					memset(PSKPassPhrase,0,128);
					if(security->SecurityKey)
					{
						strncpy(PSKPassPhrase,security->SecurityKey,sizeof(PSKPassPhrase)-1);
					}
				}
				
				snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)PSKPassPhrase,
                                          strlen(PSKPassPhrase));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
				break;
			case COLUMN_RADIUSAUTHSERVERIPADD:				
			{
				int ret=0;
				char RadiusAuthServerIPAdd[20] = { 0 };
				memset(RadiusAuthServerIPAdd,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(RadiusAuthServerIPAdd,0,20);
					if(security->auth.secondary_auth_ip)
					{
						strncpy(RadiusAuthServerIPAdd,security->auth.secondary_auth_ip,sizeof(RadiusAuthServerIPAdd)-1);
					}
				}
				
				snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)RadiusAuthServerIPAdd,
                                          strlen(RadiusAuthServerIPAdd));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
				break;
			case COLUMN_RADIUSAUTHSERVERPORT:				
			{
				int ret=0;
				int RadiusAuthServerPort = 0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					RadiusAuthServerPort=security->auth.secondary_auth_port;
				}
				
				snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&RadiusAuthServerPort,
                                          sizeof(RadiusAuthServerPort));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
				break;
			case COLUMN_RADIUSAUTHSERVERSHAREDKEY:				
			{
				int ret=0;
				char RadiusAuthServerSharedKey[20] = { 0 };
				memset(RadiusAuthServerSharedKey,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(RadiusAuthServerSharedKey,0,20);
					if(security->auth.secondary_auth_shared_secret)
					{
						strncpy(RadiusAuthServerSharedKey,security->auth.secondary_auth_shared_secret,sizeof(RadiusAuthServerSharedKey)-1);
					}
				}
				
				snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)RadiusAuthServerSharedKey,
                                          strlen(RadiusAuthServerSharedKey));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
				break;
			case COLUMN_RADIUSACCSERVERIPADD:				
			{
				int ret=0;
				char RadiusAccServerIPAdd[20] = { 0 };
				memset(RadiusAccServerIPAdd,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(RadiusAccServerIPAdd,0,20);
					if(security->acct.secondary_acct_ip)
					{
						strncpy(RadiusAccServerIPAdd,security->acct.secondary_acct_ip,sizeof(RadiusAccServerIPAdd)-1);
					}
				}
				
				snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)RadiusAccServerIPAdd,
                                          strlen(RadiusAccServerIPAdd));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
				break;
			case COLUMN_RADIUSACCSERVERPORT:				
			{
				int ret=0;
				int RadiusAccServerPort = 0;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					RadiusAccServerPort=security->acct.secondary_acct_port;
				}
				
				snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&RadiusAccServerPort,
                                          sizeof(RadiusAccServerPort));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
				break;
			case COLUMN_RADIUSACCSERVERSHAREDKEY:				
			{
				int ret=0;
				char RadiusAccServerSharedKey[20] = { 0 };
				memset(RadiusAccServerSharedKey,0,20);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(RadiusAccServerSharedKey,0,20);
					if(security->acct.secondary_acct_shared_secret)
					{
						strncpy(RadiusAccServerSharedKey,security->acct.secondary_acct_shared_secret,sizeof(RadiusAccServerSharedKey)-1);
					}
				}
				
				snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)RadiusAccServerSharedKey,
                                          strlen(RadiusAccServerSharedKey));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
				break;
			
            case COLUMN_WAPIAECERTIFICATIONPATH:
            {
				int ret=0;
				char wapiAECertificationPath[DEFAULT_LEN] = { 0 };
				memset(wapiAECertificationPath,0,DEFAULT_LEN);
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					memset(wapiAECertificationPath,0,DEFAULT_LEN);
					if(security->wapi_as.ae_cert_path)
					{
						strncpy(wapiAECertificationPath,security->wapi_as.ae_cert_path,sizeof(wapiAECertificationPath)-1);
					}
				}

				snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
										  (u_char*)wapiAECertificationPath,
										  strlen(wapiAECertificationPath));

				if(ret == 1)
				{
					Free_security_one(security);
				}
			}
                break;
			case COLUMN_HYBRIDAUTHSWITCH:
			{
				int ret=0;
				int HybridAuthSwitch=2;
				struct dcli_security *security = NULL; 
				
				ret = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
				if(ret==1)
				{
					if(security->hybrid_auth == 1)
						HybridAuthSwitch=1;
					else
						HybridAuthSwitch=2;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&HybridAuthSwitch,
                                          sizeof(HybridAuthSwitch));

				if(ret == 1)
				{
					Free_security_one(security);
				}
            }
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ConfigSecurityTable_entry *)
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
            case COLUMN_ENCRYPTIONTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_ENCRYPTIONINPUTTYPEKEY:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SECURITYKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_EXTENSIBLEAUTH:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_AUTHIP:
                if ( request->requestvb->type != ASN_OCTET_STR) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_AUTHPORT:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_AUTHSHAREDSECRET:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_ACCTIP:
                if ( request->requestvb->type != ASN_OCTET_STR) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_ACCTPORT:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_ACCTSHAREDSECRET:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_HOSTIP:
                if ( request->requestvb->type != ASN_OCTET_STR) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_RADIUSSERVER:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_ACCTINTERIMINTERVAL:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_EAPREAUTHPERIOD:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIASIP:
                if ( request->requestvb->type != ASN_OCTET_STR) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIASTYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIASCERTIFICATIONPATH:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIUNICASTREKEYMETHOD:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIMULTICASTREKEYMETHOD:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIUNICASTREKEYTIME:
                if ( request->requestvb->type != ASN_UNSIGNED ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIUNICASTREKEYPACKETS:
                if ( request->requestvb->type != ASN_UNSIGNED ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIMULTICASTREKEYTIME:
                if ( request->requestvb->type != ASN_UNSIGNED ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIMULTICASTREKEYPACKETS:
                if ( request->requestvb->type != ASN_UNSIGNED ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_PSKVALUE:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_PSKPASSPHRASE:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_RADIUSAUTHSERVERIPADD:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_RADIUSAUTHSERVERPORT:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_RADIUSAUTHSERVERSHAREDKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_RADIUSACCSERVERIPADD:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_RADIUSACCSERVERPORT:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_RADIUSACCSERVERSHAREDKEY:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WAPIAECERTIFICATIONPATH:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
			case COLUMN_HYBRIDAUTHSWITCH:
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
	{
		for (request=requests; request; request=request->next) 
		{
			table_entry = (struct dot11ConfigSecurityTable_entry *)
			netsnmp_extract_iterator_context(request);
			table_info  =     netsnmp_extract_table_info(      request);

			if( !table_entry ){
	        	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }

            void *connection = NULL;
            if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3))
                break;
                
			switch (table_info->colnum) 
			{
				case COLUMN_SECURITYTYPE:
				{
					int ret = 0;
					char type[10] = { 0 };
					memset(type,0,10);
					/* Need to save old 'table_entry->securityType' value.
					May need to use 'memcpy' */
					switch(*request->requestvb->val.integer)
					{
						case OPEN:
						{
							strncpy(type,"open",sizeof(type)-1);
						}
						break;
						case SHARED:
						{
							strncpy(type,"shared",sizeof(type)-1);
						}
						break;
						case IEEE8021X:
						{
							strncpy(type,"802.1x",sizeof(type)-1);
						}
						break;
						case WPA_P:
						{
							strncpy(type,"WPA_P",sizeof(type)-1);
						}
						break;
						case WPA2_P:
						{
							strncpy(type,"WPA2_P",sizeof(type)-1);
						}
						break;
						case WPA_E:
						{
							strncpy(type,"WPA_E",sizeof(type)-1);
						}
						break;
						case WPA2_E:
						{
							strncpy(type,"WPA2_E",sizeof(type)-1);
						}
						break;
						case MD5:
						{
							strncpy(type,"MD5",sizeof(type)-1);
						}
						break;
						case WAPI_PSK:
						{
							strncpy(type,"WAPI_PSK",sizeof(type)-1);
						}
						break;
						case WAPI_AUTH:
						{
							strncpy(type,"WAPI_AUTH",sizeof(type)-1);
						}
						break;
					}
					ret = security_type(table_entry->parameter, connection,table_entry->securityID,type);
					if(ret == 1)
					{						
						table_entry->securityType = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				
				case COLUMN_ENCRYPTIONTYPE:
				{
					int ret = 0;
					char type[10] = { 0 };
					memset(type,0,10);
					/* Need to save old 'table_entry->encryptionType' value.
					May need to use 'memcpy' */
					switch(*request->requestvb->val.integer)
					{
						case NONE:
						{
							strncpy(type,"none",sizeof(type)-1);
						}
						break;
						case WEP:
						{
							strncpy(type,"WEP",sizeof(type)-1);
						}
						break;
						case AES:
						{
							strncpy(type,"AES",sizeof(type)-1);
						}
						break;
						case TKIP:
						{
							strncpy(type,"TKIP",sizeof(type)-1);
						}
						break;
						case SMS4:
						{
							strncpy(type,"SMS4",sizeof(type)-1);
						}
						break;
					}

					ret = encryption_type(table_entry->parameter, connection,table_entry->securityID,type);
					if(ret == 1)
					{
						table_entry->encryptionType = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				case COLUMN_ENCRYPTIONINPUTTYPEKEY:
				{
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					char type[10] = { 0 };
					memset(type,0,10);
					char input_key[70] = { 0 };
					memset(input_key,0,70);

					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						if(((security->SecurityType==OPEN)&&(security->EncryptionType==WEP))||(security->SecurityType==SHARED)||(security->SecurityType==WPA_P)||(security->SecurityType==WPA2_P)||(security->SecurityType==WAPI_PSK))
						{
							if(*request->requestvb->val.integer==1)/*密钥输入类型为ASCII*/
							{
								if(security->EncryptionType==WEP)
								{
									memset(input_key,0,70);/*5位密钥*/
									strncpy(input_key,"00000",sizeof(input_key)-1);
								}
								else
								{
									memset(input_key,0,70);/*10位密钥*/
									strncpy(input_key,"0000000000",sizeof(input_key)-1);
								}
							}
							else if(*request->requestvb->val.integer==2)  /*密钥输入类型为HEX*/
							{
								if((security->SecurityType==WPA_P)||(security->SecurityType==WPA2_P))
								{
									memset(input_key,0,70);/*64位密钥*/
									strncpy(input_key,"0000000000000000000000000000000000000000000000000000000000000000",sizeof(input_key)-1);
								}
								else
								{
									memset(input_key,0,70);/*26位密钥*/
									strncpy(input_key,"00000000000000000000000000",sizeof(input_key)-1);
								}
							}
							else
							{
								netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							}
							
							
							/* Need to save old 'table_entry->encryptionInputTypeKey' value.
							May need to use 'memcpy' */
							if(*request->requestvb->val.integer == 1)
							{
								memset(type,0,10);
								strncpy(type,"ASCII",sizeof(type)-1);
							}
							else if(*request->requestvb->val.integer == 2)
							{
								memset(type,0,10);
								strncpy(type,"HEX",sizeof(type)-1);
							}
							
							ret2 = security_key(table_entry->parameter, connection,table_entry->securityID,input_key,type);
							if(ret2 == 1)
							{
								table_entry->encryptionInputTypeKey = *request->requestvb->val.integer;
							}
							else
							{
								netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
				break;
				case COLUMN_SECURITYKEY:
				{
					/* Need to save old 'table_entry->securityKEY' value.
					May need to use 'memcpy' */					
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					char type[10] = { 0 };
					memset(type,0,10);
					
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						if(security->keyInputType == 1)
						{
							memset(type,0,10);
							strncpy(type,"ASCII",sizeof(type)-1);
						}
						else if(security->keyInputType == 2)
						{
							memset(type,0,10);
							strncpy(type,"HEX",sizeof(type)-1);
						}
						
						ret2 = security_key(table_entry->parameter, connection,table_entry->securityID,input_string,type);
						if(ret2 == 1)
						{
							if(table_entry->securityKEY!= NULL)
							{
								free(table_entry->securityKEY);
							}
							if(input_string)
							{
								table_entry->securityKEY = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
				break;
				
				case COLUMN_EXTENSIBLEAUTH:
				{
					int ret = 0;
					/* Need to save old 'table_entry->extensibleAuth' value.
					May need to use 'memcpy' */
					ret = extensible_authentication(table_entry->parameter, connection,table_entry->securityID,*request->requestvb->val.integer);
					if(ret == 1)
					{
						table_entry->extensibleAuth = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				
				case COLUMN_AUTHIP:
				{
					/* Need to save old 'table_entry->authIP' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = security_auth_acct(table_entry->parameter, connection,1,table_entry->securityID,input_string,1812,security->auth.auth_shared_secret);
						if(ret2 == 1)
						{
							if(table_entry->authIP!= NULL)
							{
								free(table_entry->authIP);
							}
							if(input_string)
							{
								table_entry->authIP = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
				break;

				case COLUMN_AUTHPORT:
				{
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = security_auth_acct(table_entry->parameter, connection,1,table_entry->securityID,security->auth.auth_ip,*request->requestvb->val.integer,security->auth.auth_shared_secret);
						if(ret2 == 1)
						{
							table_entry->authPort= *request->requestvb->val.integer;
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				
				case COLUMN_AUTHSHAREDSECRET:
				{
					/* Need to save old 'table_entry->authSharedSecret' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = security_auth_acct(table_entry->parameter, connection,1,table_entry->securityID,security->auth.auth_ip,1812,input_string);
						if(ret2 == 1)
						{
							if(table_entry->authSharedSecret!= NULL)
							{
								free(table_entry->authSharedSecret);
							}
							if(input_string)
							{
								table_entry->authSharedSecret = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
				break;
				
				case COLUMN_ACCTIP:
				{
					/* Need to save old 'table_entry->acctIP' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 

					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = security_auth_acct(table_entry->parameter, connection,0,table_entry->securityID,input_string,1812,security->acct.acct_shared_secret);
						if(ret2 == 1)
						{
							if(table_entry->acctIP!= NULL)
							{
								free(table_entry->acctIP);
							}
							if(input_string)
							{
								table_entry->acctIP = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
				break;
				case COLUMN_ACCTPORT:
				{
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = security_auth_acct(table_entry->parameter, connection,0,table_entry->securityID,security->acct.acct_ip,*request->requestvb->val.integer,security->acct.acct_shared_secret);
						if(ret2 == 1)
						{
							table_entry->acctPort= *request->requestvb->val.integer;
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				case COLUMN_ACCTSHAREDSECRET:
				{
					/* Need to save old 'table_entry->acctSharedSecret' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = security_auth_acct(table_entry->parameter, connection,0,table_entry->securityID,security->acct.acct_ip,1812,input_string);
						if(ret2 == 1)
						{
							if(table_entry->acctSharedSecret!= NULL)
							{
								free(table_entry->acctSharedSecret);
							}
							if(input_string)
							{
								table_entry->acctSharedSecret = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
				break;
				case COLUMN_HOSTIP:
				{
					int ret = 0;
					/* Need to save old 'table_entry->hostIP' value.
					May need to use 'memcpy' */
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret = security_host_ip(table_entry->parameter, connection,table_entry->securityID,input_string);
					if(ret == 1)
					{
						if(table_entry->hostIP!= NULL)
						{
							free(table_entry->hostIP);
						}
						if(input_string)
						{
							table_entry->hostIP = strdup(input_string);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
				}
				break;
				
				case COLUMN_RADIUSSERVER:
				{
					int ret = 0;
					/* Need to save old 'table_entry->radiusServer' value.
					May need to use 'memcpy' */
					ret = radius_server(table_entry->parameter, connection,table_entry->securityID,*request->requestvb->val.integer);
					if(ret == 1)
					{
						table_entry->radiusServer = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

				}
				break;
				
				case COLUMN_ACCTINTERIMINTERVAL:
				{
					int ret =0;
					/* Need to save old 'table_entry->acctInterimInterval' value.
					May need to use 'memcpy' */
					ret = set_acct_interim_interval(table_entry->parameter, connection,table_entry->securityID,*request->requestvb->val.integer);
					if(ret == 1)
					{
						table_entry->acctInterimInterval = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				
				case COLUMN_EAPREAUTHPERIOD:
				{
					int ret =-1;
					/* Need to save old 'table_entry->eapReauthPeriod' value.
					May need to use 'memcpy' */
					if((*request->requestvb->val.integer<0)||(*request->requestvb->val.integer>32767))
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					ret = set_eap_reauth_period_cmd(table_entry->parameter, connection,table_entry->securityID,*request->requestvb->val.integer);
					if(ret == 1)
					{
						table_entry->eapReauthPeriod = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

				}
				break;
				case COLUMN_WAPIASIP:
				{
					/* Need to save old 'table_entry->wapiASIP' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = 0;
					char type[10] = { 0 };
					memset(type,0,10);
					struct dcli_security *security = NULL; 

					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
				
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						if(security->SecurityType == WAPI_AUTH)
						{							
							if(security->wapi_as.certification_type == 2)
							{
								memset(type,0,10);
								strncpy(type,"GBW",sizeof(type)-1);
							}
							else
							{
								memset(type,0,10);
								strncpy(type,"X.509",sizeof(type)-1);
							}
							ret2 =config_wapi_auth(table_entry->parameter, connection,table_entry->securityID,input_string,type);
							if(ret2 == 1)
							{
								if(table_entry->wapiASIP!= NULL)
								{
									free(table_entry->wapiASIP);
								}
								if(input_string)
								{
									table_entry->wapiASIP = strdup(input_string);
								}
							}
							else
							{
								netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
				break;
				case COLUMN_WAPIASTYPE:
				{
					/* Need to save old 'table_entry->wapiASType' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = 0;
					char type[10] = { 0 };
					memset(type,0,10);
					struct dcli_security *security = NULL; 
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						if(security->SecurityType == WAPI_AUTH)
						{
							if(*request->requestvb->val.integer == 1)
							{
								memset(type,0,10);
								strncpy(type,"X.509",sizeof(type)-1);
							}
							else if(*request->requestvb->val.integer == 2)
							{
								memset(type,0,10);
								strncpy(type,"GBW",sizeof(type)-1);
							}
							if((security->wapi_as.as_ip)&&(strcmp(security->wapi_as.as_ip,"")==0))
							{
								ret2 =config_wapi_auth(table_entry->parameter, connection,table_entry->securityID,"0.0.0.0",type);
							}
							else
							{
								ret2 =config_wapi_auth(table_entry->parameter, connection,table_entry->securityID,security->wapi_as.as_ip,type);
							}
							if(ret2 == 1)
							{
								table_entry->wapiASType = *request->requestvb->val.integer;
							}
							else
							{
								netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
				break;
				case COLUMN_WAPIASCERTIFICATIONPATH:
				{
					int ret = 0;
					/* Need to save old 'table_entry->wapiASCertificationPath' value.
					May need to use 'memcpy' */					
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}

					if(table_entry->securityType!=WAPI_AUTH)
					{
						ret = config_wapi_auth_path(table_entry->parameter, connection,table_entry->securityID,"as",input_string);
						if(ret == 1)
						{
							if(table_entry->wapiASCertificationPath!= NULL)
							{
								free(table_entry->wapiASCertificationPath);
							}
							if(input_string)
							{
								table_entry->wapiASCertificationPath = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
				}
				break;
				/*case COLUMN_WAPIAECERTIFICATIONPATH:
				{
					int ret = 0;
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					memset(input_string,0,request->requestvb->val_len+1);
					strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					
					if(table_entry->securityType!=WAPI_AUTH)
					{
						ret = config_wapi_auth_path(table_entry->parameter, connection,table_entry->securityID,"ae",input_string);
						if(ret ==1 )
						{
							if(table_entry->wapiAECertificationPath!= NULL)
							{
								free(table_entry->wapiAECertificationPath);
							}
							table_entry->wapiAECertificationPath = strdup(input_string);
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					free(input_string);
				}
				break;*/
				case COLUMN_WAPIUNICASTREKEYMETHOD:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyMethod' value.
					May need to use 'memcpy' */
					int ret = 0;
					char method[20] = { 0 };
					memset(method,0,20);
					CheckRekeyMethod(method,*request->requestvb->val.integer);
					ret = set_wapi_ucast_rekey_method_cmd_func(table_entry->parameter, connection,table_entry->securityID,"unicast",method);
					if(ret == 1)
					{
						table_entry->wapiUnicastRekeyMethod = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				case COLUMN_WAPIMULTICASTREKEYMETHOD:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyMethod' value.
					May need to use 'memcpy' */
					int ret = 0;
					char method[20] = { 0 };
					memset(method,0,20);
					CheckRekeyMethod(method,*request->requestvb->val.integer);
					ret = set_wapi_ucast_rekey_method_cmd_func(table_entry->parameter, connection,table_entry->securityID,"multicast",method);
					if(ret == 1)
					{
						table_entry->wapiMulticastRekeyMethod = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				case COLUMN_WAPIUNICASTREKEYTIME:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret = 0;
					char wapiUnicastRekeyTime[20] = { 0 };
					memset(wapiUnicastRekeyTime,0,20);
					snprintf(wapiUnicastRekeyTime,sizeof(wapiUnicastRekeyTime)-1,"%d",*request->requestvb->val.integer);

					ret = set_wapi_rekey_para_cmd_func(table_entry->parameter, connection,table_entry->securityID,"unicast","time",wapiUnicastRekeyTime);
					if(ret == 1)
					{
						table_entry->wapiUnicastRekeyTime = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				case COLUMN_WAPIUNICASTREKEYPACKETS:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret = 0;
					char wapiUnicastRekeyPackets[20] = { 0 };
					memset(wapiUnicastRekeyPackets,0,20);
					snprintf(wapiUnicastRekeyPackets,sizeof(wapiUnicastRekeyPackets)-1,"%d",*request->requestvb->val.integer);

					ret = set_wapi_rekey_para_cmd_func(table_entry->parameter, connection,table_entry->securityID,"unicast","packet",wapiUnicastRekeyPackets);
					if(ret == 1)
					{
						table_entry->wapiUnicastRekeyPackets = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				case COLUMN_WAPIMULTICASTREKEYTIME:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret = 0;
					char wapiMulticastRekeyTime[20] = { 0 };
					memset(wapiMulticastRekeyTime,0,20);
					snprintf(wapiMulticastRekeyTime,sizeof(wapiMulticastRekeyTime)-1,"%d",*request->requestvb->val.integer);
					
					ret = set_wapi_rekey_para_cmd_func(table_entry->parameter, connection,table_entry->securityID,"multicast","time",wapiMulticastRekeyTime);
					if(ret == 1)
					{
						table_entry->wapiMulticastRekeyTime = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				case COLUMN_WAPIMULTICASTREKEYPACKETS:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret = 0;
					char wapiMulticastRekeyPackets[20] = { 0 };
					memset(wapiMulticastRekeyPackets,0,20);
					snprintf(wapiMulticastRekeyPackets,sizeof(wapiMulticastRekeyPackets)-1,"%d",*request->requestvb->val.integer);
					
					ret = set_wapi_rekey_para_cmd_func(table_entry->parameter, connection,table_entry->securityID,"multicast","packet",wapiMulticastRekeyPackets);
					if(ret == 1)
					{
						table_entry->wapiMulticastRekeyPackets = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
				case COLUMN_PSKVALUE:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					char type[10] = { 0 };
					memset(type,0,10);
					strncpy(type,"ASCII",sizeof(type)-1);
					
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						if(security->keyInputType == 1)
						{
							memset(type,0,10);
							strncpy(type,"ASCII",sizeof(type)-1);
						}
						else if(security->keyInputType == 2)
						{
							memset(type,0,10);
							strncpy(type,"HEX",sizeof(type)-1);
						}
						
						if(security->SecurityType==WAPI_PSK)
						{
							ret2 = security_key(table_entry->parameter, connection,table_entry->securityID,input_string,type);
							if(ret2 == 1)
							{
								if(table_entry->PSKValue!= NULL)
								{
									free(table_entry->PSKValue);
								}					
								if(input_string)
								{
									table_entry->PSKValue = strdup(input_string);
								}
							}
							else
							{
								netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							}
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				case COLUMN_PSKPASSPHRASE:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = 0;
					struct dcli_security *security = NULL; 
					char type[10] = { 0 };
					memset(type,0,10);
					strncpy(type,"ASCII",sizeof(type)-1);

					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						if(security->keyInputType == 1)
						{
							memset(type,0,10);
							strncpy(type,"ASCII",sizeof(type)-1);
						}
						else if(security->keyInputType == 2)
						{
							memset(type,0,10);
							strncpy(type,"HEX",sizeof(type)-1);
						}
						
						if(security->SecurityType==WAPI_PSK)
						{
							ret2 = security_key(table_entry->parameter, connection,table_entry->securityID,input_string,type);
							if(ret2 == 1)
							{
								if(table_entry->PSKPassPhrase!= NULL)
								{
									free(table_entry->PSKPassPhrase);
								}		
								if(input_string)
								{
									table_entry->PSKPassPhrase = strdup(input_string);
								}
							}
							else
							{
								netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							}
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
					
					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				case COLUMN_RADIUSAUTHSERVERIPADD:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = -1;
					struct dcli_security *security = NULL; 

					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = secondary_radius_auth(table_entry->parameter, connection,table_entry->securityID,input_string,security->auth.secondary_auth_port,security->auth.secondary_auth_shared_secret);
						if(ret2 == 1)
						{
							if(table_entry->RadiusAuthServerIPAdd!= NULL)
							{
								free(table_entry->RadiusAuthServerIPAdd);
							}				
							if(input_string)
							{
								table_entry->RadiusAuthServerIPAdd = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				case COLUMN_RADIUSAUTHSERVERPORT:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = -1;
					struct dcli_security *security = NULL; 
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = secondary_radius_auth(table_entry->parameter, connection,table_entry->securityID,security->auth.secondary_auth_ip,*request->requestvb->val.integer,security->auth.secondary_auth_shared_secret);
						if(ret2 == 1)
						{
							table_entry->RadiusAuthServerPort = *request->requestvb->val.integer;
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}		
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				case COLUMN_RADIUSAUTHSERVERSHAREDKEY:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = -1;
					struct dcli_security *security = NULL; 

					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
				
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = secondary_radius_auth(table_entry->parameter, connection,table_entry->securityID,security->auth.secondary_auth_ip,security->auth.secondary_auth_port,input_string);
						if(ret2 == 1)
						{
							if(table_entry->RadiusAuthServerSharedKey!= NULL)
							{
								free(table_entry->RadiusAuthServerSharedKey);
							}			
							if(input_string)
							{
								table_entry->RadiusAuthServerSharedKey = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}		
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				case COLUMN_RADIUSACCSERVERIPADD:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = -1;
					struct dcli_security *security = NULL; 

					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
				
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = secondary_radius_acct(table_entry->parameter, connection,table_entry->securityID,input_string,security->acct.secondary_acct_port,security->acct.secondary_acct_shared_secret);
						if(ret2 == 1)
						{
							if(table_entry->RadiusAccServerIPAdd!= NULL)
							{
								free(table_entry->RadiusAccServerIPAdd);
							}			
							if(input_string)
							{
								table_entry->RadiusAccServerIPAdd = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				case COLUMN_RADIUSACCSERVERPORT:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = -1;
					struct dcli_security *security = NULL; 
					
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = secondary_radius_acct(table_entry->parameter, connection,table_entry->securityID,security->acct.secondary_acct_ip,*request->requestvb->val.integer,security->acct.secondary_acct_shared_secret);
						if(ret2 == 1)
						{
							table_entry->RadiusAccServerPort = *request->requestvb->val.integer;
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				case COLUMN_RADIUSACCSERVERSHAREDKEY:
				{
					/* Need to save old 'table_entry->wapiUnicastRekeyTime' value.
					May need to use 'memcpy' */
					int ret1 = 0,ret2 = -1;
					struct dcli_security *security = NULL; 

					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
				
					ret1 = show_security_one(table_entry->parameter, connection,table_entry->securityID,&security);
					if(ret1 == 1)
					{
						ret2 = secondary_radius_acct(table_entry->parameter, connection,table_entry->securityID,security->acct.secondary_acct_ip,security->acct.secondary_acct_port,input_string);
						if(ret2 == 1)
						{
							if(table_entry->RadiusAccServerSharedKey!= NULL)
							{
								free(table_entry->RadiusAccServerSharedKey);
							}				
							if(input_string)
							{
								table_entry->RadiusAccServerSharedKey = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
					if(ret1 == 1)
					{
						Free_security_one(security);
					}
				}
                break;
				
				case COLUMN_WAPIAECERTIFICATIONPATH:
				{
				    /* Need to save old 'table_entry->wapiAECertificationPath' value.
					   May need to use 'memcpy' */
					//table_entry->old_wapiAECertificationPath = table_entry->wapiAECertificationPath;
					//table_entry->wapiAECertificationPath	 = request->requestvb->val.YYY;
					
					int ret = 0;
					char * input_string = (char *)malloc(request->requestvb->val_len+1);
					if(input_string)
					{
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					}
					
					if(table_entry->securityType!=WAPI_AUTH)
					{
						ret = config_wapi_auth_path(table_entry->parameter, connection,table_entry->securityID,"ae",input_string);
						if(ret ==1 )
						{
							if(table_entry->wapiAECertificationPath!= NULL)
							{
								free(table_entry->wapiAECertificationPath);
							}
							if(input_string)
							{
								table_entry->wapiAECertificationPath = strdup(input_string);
							}
						}
						else
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}

					FREE_OBJECT(input_string);
				}	
				break;

				case COLUMN_HYBRIDAUTHSWITCH:
				{
					/* Need to save old 'table_entry->HybridAuthSwitch' value.
					May need to use 'memcpy' */
					int ret = 0;
					char state[10] = { 0 };
					memset(state,0,sizeof(state));
					if(1 == *request->requestvb->val.integer)
						strncpy(state,"enable",sizeof(state)-1);
					else
						strncpy(state,"disable",sizeof(state)-1);

					ret = config_hybrid_auth_cmd(table_entry->parameter, connection, table_entry->securityID, state);
					if(ret == 1)
					{
						table_entry->HybridAuthSwitch = *request->requestvb->val.integer;
					}
					else
					{
						netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
					}
				}
				break;
			}
		}
	}
	break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11ConfigSecurityTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_SECURITYTYPE:
                /* Need to restore old 'table_entry->securityType' value.
                   May need to use 'memcpy' */
                //table_entry->securityType = table_entry->old_securityType;
                break;
            case COLUMN_ENCRYPTIONTYPE:
                /* Need to restore old 'table_entry->encryptionType' value.
                   May need to use 'memcpy' */
                //table_entry->encryptionType = table_entry->old_encryptionType;
                break;
            case COLUMN_ENCRYPTIONINPUTTYPEKEY:
                /* Need to restore old 'table_entry->encryptionInputTypeKey' value.
                   May need to use 'memcpy' */
                //table_entry->encryptionInputTypeKey = table_entry->old_encryptionInputTypeKey;
                break;
            case COLUMN_SECURITYKEY:
                /* Need to restore old 'table_entry->securityKEY' value.
                   May need to use 'memcpy' */
                //table_entry->securityKEY = table_entry->old_securityKEY;
                break;
            case COLUMN_EXTENSIBLEAUTH:
                /* Need to restore old 'table_entry->extensibleAuth' value.
                   May need to use 'memcpy' */
                //table_entry->extensibleAuth = table_entry->old_extensibleAuth;
                break;
            case COLUMN_AUTHIP:
                /* Need to restore old 'table_entry->authIP' value.
                   May need to use 'memcpy' */
                //table_entry->authIP = strdup(table_entry->old_authIP);
                break;
			case COLUMN_AUTHPORT:
				/* Need to restore old 'table_entry->authPort' value.
                   May need to use 'memcpy' */
                //table_entry->authPort= table_entry->old_authPort;
                break;
            case COLUMN_AUTHSHAREDSECRET:
                /* Need to restore old 'table_entry->authSharedSecret' value.
                   May need to use 'memcpy' */
                //table_entry->authSharedSecret = table_entry->old_authSharedSecret;
                break;
            case COLUMN_ACCTIP:
                /* Need to restore old 'table_entry->acctIP' value.
                   May need to use 'memcpy' */
                //table_entry->acctIP = strdup(table_entry->old_acctIP);
                break;			
			case COLUMN_ACCTPORT:
				/* Need to restore old 'table_entry->acctPort' value.
                   May need to use 'memcpy' */
                //table_entry->acctPort= table_entry->old_acctPort;
                break;
            case COLUMN_ACCTSHAREDSECRET:
                /* Need to restore old 'table_entry->acctSharedSecret' value.
                   May need to use 'memcpy' */
                //table_entry->acctSharedSecret = table_entry->old_acctSharedSecret;
                break;
            case COLUMN_HOSTIP:
                /* Need to restore old 'table_entry->hostIP' value.
                   May need to use 'memcpy' */
                //table_entry->hostIP = strdup(table_entry->old_hostIP);
                break;
            case COLUMN_RADIUSSERVER:
                /* Need to restore old 'table_entry->radiusServer' value.
                   May need to use 'memcpy' */
               // table_entry->radiusServer = table_entry->old_radiusServer;
                break;
            case COLUMN_ACCTINTERIMINTERVAL:
                /* Need to restore old 'table_entry->acctInterimInterval' value.
                   May need to use 'memcpy' */
                //table_entry->acctInterimInterval = table_entry->old_acctInterimInterval;
                break;
            case COLUMN_EAPREAUTHPERIOD:
                /* Need to restore old 'table_entry->eapReauthPeriod' value.
                   May need to use 'memcpy' */
                //table_entry->eapReauthPeriod = table_entry->old_eapReauthPeriod;
                break;
            case COLUMN_WAPIASIP:
                /* Need to restore old 'table_entry->wapiASIP' value.
                   May need to use 'memcpy' */
                //table_entry->wapiASIP = strdup(table_entry->old_wapiASIP);
                break;
            case COLUMN_WAPIASTYPE:
                /* Need to restore old 'table_entry->wapiASType' value.
                   May need to use 'memcpy' */
                //table_entry->wapiASType = table_entry->old_wapiASType;
                break;
            case COLUMN_WAPIASCERTIFICATIONPATH:
                /* Need to restore old 'table_entry->wapiASCertificationPath' value.
                   May need to use 'memcpy' */
                //table_entry->wapiASCertificationPath = table_entry->old_wapiASCertificationPath;
                break;
            case COLUMN_WAPIISINSTALLEDASCER:
                /* Need to restore old 'table_entry->wapiIsInstalledASCer' value.
                   May need to use 'memcpy' */
                //table_entry->wapiIsInstalledASCer = table_entry->old_wapiIsInstalledASCer;
                break;
            case COLUMN_WAPIAECERTIFICATIONPATH:
                /* Need to restore old 'table_entry->wapiAECertificationPath' value.
                   May need to use 'memcpy' */
                //table_entry->wapiAECertificationPath = table_entry->old_wapiAECertificationPath;
                break;
            case COLUMN_WAPIUNICASTREKEYMETHOD:
                /* Need to restore old 'table_entry->wapiUnicastRekeyMethod' value.
                   May need to use 'memcpy' */
                //table_entry->wapiUnicastRekeyMethod = table_entry->old_wapiUnicastRekeyMethod;
                break;
            case COLUMN_WAPIMULTICASTREKEYMETHOD:
                /* Need to restore old 'table_entry->wapiMulticastRekeyMethod' value.
                   May need to use 'memcpy' */
                //table_entry->wapiMulticastRekeyMethod = table_entry->old_wapiMulticastRekeyMethod;
                break;
            case COLUMN_WAPIUNICASTREKEYTIME:
                /* Need to restore old 'table_entry->wapiUnicastRekeyTime' value.
                   May need to use 'memcpy' */
                //table_entry->wapiUnicastRekeyTime = table_entry->old_wapiUnicastRekeyTime;
                break;
            case COLUMN_WAPIUNICASTREKEYPACKETS:
                /* Need to restore old 'table_entry->wapiUnicastRekeyPackets' value.
                   May need to use 'memcpy' */
                //table_entry->wapiUnicastRekeyPackets = table_entry->old_wapiUnicastRekeyPackets;
                break;
            case COLUMN_WAPIMULTICASTREKEYTIME:
                /* Need to restore old 'table_entry->wapiMulticastRekeyTime' value.
                   May need to use 'memcpy' */
                //table_entry->wapiMulticastRekeyTime = table_entry->old_wapiMulticastRekeyTime;
                break;
            case COLUMN_WAPIMULTICASTREKEYPACKETS:
                /* Need to restore old 'table_entry->wapiMulticastRekeyPackets' value.
                   May need to use 'memcpy' */
                //table_entry->wapiMulticastRekeyPackets = table_entry->old_wapiMulticastRekeyPackets;
                break;
			case COLUMN_PSKVALUE:
				/* Need to restore old 'table_entry->PSKValue' value.
                   May need to use 'memcpy' */
                //table_entry->PSKValue= strdup(table_entry->old_PSKValue);
                break;
			case COLUMN_PSKPASSPHRASE:
				/* Need to restore old 'table_entry->PSKPassPhrase' value.
                   May need to use 'memcpy' */
                //table_entry->PSKPassPhrase= strdup(table_entry->old_PSKPassPhrase);
                break;
			case COLUMN_RADIUSAUTHSERVERIPADD:
				/* Need to restore old 'table_entry->RadiusAuthServerIPAdd' value.
                   May need to use 'memcpy' */
                //table_entry->RadiusAuthServerIPAdd= strdup(table_entry->old_RadiusAuthServerIPAdd);
                break;
			case COLUMN_RADIUSAUTHSERVERPORT:
				/* Need to restore old 'table_entry->RadiusAuthServerPort' value.
                   May need to use 'memcpy' */
                //table_entry->RadiusAuthServerPort= table_entry->old_RadiusAuthServerPort;	
                break;
			case COLUMN_RADIUSAUTHSERVERSHAREDKEY:
				/* Need to restore old 'table_entry->RadiusAuthServerSharedKey' value.
                   May need to use 'memcpy' */
                //table_entry->RadiusAuthServerSharedKey= strdup(table_entry->old_RadiusAuthServerSharedKey);
                break;
			case COLUMN_RADIUSACCSERVERIPADD:
				/* Need to restore old 'table_entry->RadiusAccServerIPAdd' value.
                   May need to use 'memcpy' */
                //table_entry->RadiusAccServerIPAdd= strdup(table_entry->old_RadiusAccServerIPAdd);
                break;
			case COLUMN_RADIUSACCSERVERPORT:
				/* Need to restore old 'table_entry->RadiusAccServerPort' value.
                   May need to use 'memcpy' */
                //table_entry->RadiusAccServerPort= table_entry->old_RadiusAccServerPort;	
                break;
			case COLUMN_RADIUSACCSERVERSHAREDKEY:
				/* Need to restore old 'table_entry->RadiusAccServerSharedKey' value.
                   May need to use 'memcpy' */
                //table_entry->RadiusAccServerSharedKey= strdup(table_entry->old_RadiusAccServerSharedKey);
                break;
			case COLUMN_HYBRIDAUTHSWITCH:
                /* Need to restore old 'table_entry->HybridAuthSwitch' value.
                   May need to use 'memcpy' */
                //table_entry->HybridAuthSwitch = table_entry->old_HybridAuthSwitch;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
