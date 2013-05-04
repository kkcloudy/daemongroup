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
* dot11AcPara.c
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
#include "dot11AcPara.h"
#include "autelanWtpGroup.h"
#include "ws_snmpd_engine.h"
#include "ws_usrinfo.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "nm/app/eag/eag_conf.h"
#include "nm/public/nm_list.h"
#include "nm/app/eag/eag_interface.h"
#include "nm/app/eag/eag_errcode.h"
#include "board/board_define.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_ac.h"
#include "ws_sta.h"
#include "ws_public.h"
#include "ws_user_manage.h"
#include "ws_conf_engine.h"
#include "ws_eag_conf.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"
#include "ws_acinfo.h"
#include "ws_log_conf.h"

#define NET_ELEMENT_CONFIG_FILE "/var/run/net_elemnet"
#define MAX_ITEM_NUM	64
#define EAG_CONF_FILE_PATH "/opt/services/conf/eag_conf.conf"       

dbus_parameter dot11AcPara_parameter = { 0 };

/** Initializes the dot11AcPara module */

#define ACIPADDRESS				"2.2.1"
#define	ACNETMASK				"2.2.2"
#define	ACNETELEMENT			"2.2.3"
#define	ACAUTHENMOTHED			"2.2.4"
#define	ACMACADDRESS			"2.2.5"
#define	ACCONPORTALURL			"2.2.6"
#define	ACPORTALSERVERPORT		"2.2.7"
#define	ACMAXPORTALONLINEUSERS	"2.2.8"
#define	ACPORTALONLINEUSERS		"2.2.9"
#define	ACMAXPPPOEONLINEUSERS	"2.2.10"
#define	ACPPPOEONLINEUSERS		"2.2.11"
#define	ACCONASSERVERIP			"2.2.13"
#define	ACCERTIFSERVERTYPE		"2.2.12"
#define	ACFIRSTCONDNSSERVER 	"2.2.14"
#define	ACSECONCONDNSSERVER		"2.2.15"
#define	ACTRAPDESIPADDRESS		"2.2.16"
#define	ACSNMPPORT				"2.2.17"
#define SYSGWADDR				"2.2.18"
#define TRAPPORT                "2.2.19"
void
init_dot11AcPara(void)
{
	static oid acIpAddress_oid[128] 			= { 0};
	static oid acNetMask_oid[128] 				= { 0 };
	static oid acNetElementCode_oid[128] 		= { 0 };
	static oid acAuthenMothedsupp_oid[128] 		= { 0 };
	static oid acMacAddress_oid[128] 			= { 0 };
	static oid acConPortalURL_oid[128] 			= { 0 };
	static oid acPortalServerPort_oid[128] 		= { 0 };
	static oid acMaxPortalOnlineUsers_oid[128] 	= { 0 };
	static oid acPortalOnlineUsers_oid[128] 	= { 0 };
	static oid acMaxPPPoEOnlineUsers_oid[128] 	= { 0 };
	static oid acPPPoEOnlineUsers_oid[128] 		= { 0 };
	static oid acConASServerIP_oid[128] 		= { 0 };
	static oid acCertifServerType_oid[128] 		= { 0 };
	static oid acFirstConDNSServer_oid[128] 	= { 0 };
	static oid acSeconConDNSServer_oid[128] 	= { 0 };
	static oid acTrapDesIPAddress_oid[128] 		= { 0 };
	static oid acSNMPPort_oid[128]				= { 0 };
	static oid SysGWAddr_oid[128] 				= { 0};
	static oid acTrapPort_oid[128]                  = { 0 };

    size_t public_oid_len   = 0;
	mad_dev_oid(acIpAddress_oid,ACIPADDRESS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acNetMask_oid,ACNETMASK,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acNetElementCode_oid,ACNETELEMENT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthenMothedsupp_oid,ACAUTHENMOTHED,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMacAddress_oid,ACMACADDRESS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acConPortalURL_oid,ACCONPORTALURL,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortalServerPort_oid,ACPORTALSERVERPORT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMaxPortalOnlineUsers_oid,ACMAXPORTALONLINEUSERS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortalOnlineUsers_oid,ACPORTALONLINEUSERS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMaxPPPoEOnlineUsers_oid,ACMAXPPPOEONLINEUSERS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPPPoEOnlineUsers_oid,ACPPPOEONLINEUSERS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acConASServerIP_oid,ACCONASSERVERIP,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acCertifServerType_oid,ACCERTIFSERVERTYPE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acFirstConDNSServer_oid,ACFIRSTCONDNSSERVER,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSeconConDNSServer_oid,ACSECONCONDNSSERVER,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acTrapDesIPAddress_oid,ACTRAPDESIPADDRESS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSNMPPort_oid,ACSNMPPORT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(SysGWAddr_oid,SYSGWADDR,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acTrapPort_oid,TRAPPORT,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11AcPara", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acIpAddress", handle_acIpAddress,
                               acIpAddress_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acNetMask", handle_acNetMask,
                               acNetMask_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acNetElementCode", handle_acNetElementCode,
                               acNetElementCode_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthenMothedsupp", handle_acAuthenMothedsupp,
                               acAuthenMothedsupp_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMacAddress", handle_acMacAddress,
                               acMacAddress_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acConPortalURL", handle_acConPortalURL,
                               acConPortalURL_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortalServerPort", handle_acPortalServerPort,
                               acPortalServerPort_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMaxPortalOnlineUsers", handle_acMaxPortalOnlineUsers,
                               acMaxPortalOnlineUsers_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortalOnlineUsers", handle_acPortalOnlineUsers,
                               acPortalOnlineUsers_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMaxPPPoEOnlineUsers", handle_acMaxPPPoEOnlineUsers,
                               acMaxPPPoEOnlineUsers_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPPPoEOnlineUsers", handle_acPPPoEOnlineUsers,
                               acPPPoEOnlineUsers_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acConASServerIP", handle_acConASServerIP,
                               acConASServerIP_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acCertifServerType", handle_acCertifServerType,
                               acCertifServerType_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acFirstConDNSServer", handle_acFirstConDNSServer,
                               acFirstConDNSServer_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSeconConDNSServer", handle_acSeconConDNSServer,
                               acSeconConDNSServer_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acTrapDesIPAddress", handle_acTrapDesIPAddress,
                               acTrapDesIPAddress_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSNMPPort", handle_acSNMPPort,
                               acSNMPPort_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("SysGWAddr", handle_SysGWAddr,
                               SysGWAddr_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acTrapPort", handle_acTrapPort,
                               acTrapPort_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_acIpAddress(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acIpAddress\n");

    int acIpAddress = 0;
    switch(reqinfo->mode) {

        case MODE_GET:
		{		
			int acIpAddress = 0;
			infi  interf,*q = NULL;

			interface_list_ioctl(0, &interf);
			for(q = interf.next; NULL != q; q = q->next) {
				if((strcmp(q->if_name,"lo")!=0)&&(strncmp(q->if_addr,"169.254.",8)!=0)){
					INET_ATON(acIpAddress,q->if_addr);
					break;
				}
			}
			free_inf(&interf);
			
			snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS,
										(u_char *) &acIpAddress,
										sizeof(acIpAddress));
										
		}
		break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acIpAddress\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acIpAddress\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acNetMask(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acNetMask\n");

    int acNetMask = 0;
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acNetMask = 0;
			infi  interf,*q = NULL;
			
			interface_list_ioctl(0, &interf);
			for(q = interf.next; NULL != q; q = q->next) {
				if(strcmp(q->if_name,"lo")) {
					INET_ATON(acNetMask,q->if_mask);
					break;
				}
			}
			free_inf(&interf);
			
			snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS,
										(u_char *)&acNetMask,
										sizeof(acNetMask));

		}
		break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNetMask\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acNetMask\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acNetElementCode(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acNetElementCode\n");

    switch(reqinfo->mode) {

    case MODE_GET:
	{
		FILE *fp = NULL;
		char result[256] = { 0 };
		memset(result,0,256);
		char acNetElementCode[256] = { 0 };
		memset(acNetElementCode,0,256);
		char *temp = NULL;

		snmp_log(LOG_DEBUG, "enter fopen\n");
		fp = fopen(NET_ELEMENT_CONFIG_FILE,"r");
		snmp_log(LOG_DEBUG, "exit fopen,fp=%p\n", fp);
		
		if(fp)
		{
			memset(result,0,256);
			fgets(result,256,fp);
			temp=strchr(result,':');
			memset(acNetElementCode,0,256);
			if(temp)
			{
				strncpy(acNetElementCode,temp+1,sizeof(acNetElementCode)-1);
			}
			fclose(fp);
		}		
        delete_enter(acNetElementCode);
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *)acNetElementCode,
									strlen(acNetElementCode));
	}
        break;

	case MODE_SET_RESERVE1:
		break;

	case MODE_SET_RESERVE2:
	/* XXX malloc "undo" storage buffer */
		break;

	case MODE_SET_FREE:
	/* XXX: free resources allocated in RESERVE1 and/or
	RESERVE2.  Something failed somewhere, and the states
	below won't be called. */
		break;

	case MODE_SET_ACTION:
	{
		char * input_string = (char *)malloc(requests->requestvb->val_len+1);
		if(input_string)
		{
			memset(input_string,0,requests->requestvb->val_len+1);
			strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);

			instance_parameter *paraHead = NULL, *paraNode = NULL;
			list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
			for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
			{
				ac_manage_set_acinfo_rule(paraNode->connection, NET_TYPE,input_string,OPT_ADD); 				
			}
		}
	
		FREE_OBJECT(input_string);
	}
		break;

	case MODE_SET_COMMIT:
		break;

	case MODE_SET_UNDO:
		break;


    default:
        /* we should never get here, so this is a really bad error */
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNetElementCode\n", reqinfo->mode );
        return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acNetElementCode\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAuthenMothedsupp(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acAuthenMothedsupp\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acAuthenMothedsupp = 4;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acAuthenMothedsupp,
                                     sizeof(acAuthenMothedsupp));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthenMothedsupp\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAuthenMothedsupp\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acMacAddress(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acMacAddress\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			FILE *get_mac = NULL;
			char mac[50] = { 0 };
			memset(mac,0,50);
			char teammac[6][3] = {0};
			unsigned char ac_mac[6] = { 0 };
			int i = 0;

			snmp_log(LOG_DEBUG, "enter fopen\n");
			get_mac = fopen("/devinfo/local_mac","r");
			snmp_log(LOG_DEBUG, "exit fopen,get_mac=%p\n", get_mac);
			
			if(get_mac != NULL)
			{
				fgets(mac,50,get_mac);
				sscanf(mac,"%2s%2s%2s%2s%2s%2s",teammac[0],teammac[1],teammac[2],teammac[3],teammac[4],teammac[5]);

				for(i=0;i<6;i++)
				{
					ac_mac[i] = (unsigned char) strtoul(teammac[i], NULL, 16);
				}
				
				fclose(get_mac);
			}
		
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *)ac_mac,
									6);
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMacAddress\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acMacAddress\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acConPortalURL(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acConPortalURL\n");

    switch(reqinfo->mode) {

	case MODE_GET:
	{
		instance_parameter *paraHead = NULL;
		int ret = -1;
		struct portal_conf portalconf;
		memset( &portalconf, 0, sizeof(struct portal_conf) );
		char content[DEFAULT_LEN] = { 0 };
		memset(content,0,sizeof(content));

		list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
		if(paraHead)
		{
			memcpy(&dot11AcPara_parameter,&(paraHead->parameter),sizeof(dot11AcPara_parameter));
			
			ret = eag_get_portal_conf( paraHead->connection, paraHead->parameter.local_id,paraHead->parameter.instance_id, &portalconf );
			if( EAG_RETURN_OK == ret )
			{		
				strncpy(content,portalconf.portal_srv[0].portal_url,sizeof(content)-1);
			}
		}
		free_instance_parameter_list(&paraHead);
		
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *) content,
									strlen(content));
	}
	break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */

        case MODE_SET_RESERVE1:
            if ( 0/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
                netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
            }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
            if ( 0/* XXX if malloc, or whatever, failed: */) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
			/*http://192.168.9.204:3990/www/index.html*/
		{
			int result = SNMPD_DBUS_ERROR;
			void *connection = NULL;
			int ret1 = -1;
			struct portal_conf portalconf;
			memset( &portalconf, 0, sizeof(struct portal_conf) );
			int ret2 = EAG_ERR_DBUS_FAILED;
			unsigned long keyid = 0;
			char keystr[255] = { 0 };
			char * input_string = (char *)malloc(requests->requestvb->val_len+1);
			if(input_string)
			{
				memset(input_string,0,requests->requestvb->val_len+1);
				strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);
			}
			else
			{
				break;
			}

			result = get_instance_dbus_connection(dot11AcPara_parameter, &connection, SNMPD_INSTANCE_MASTER_V3);
			if(SNMPD_DBUS_SUCCESS == result)
			{
				ret1 = eag_get_portal_conf( connection, dot11AcPara_parameter.local_id,dot11AcPara_parameter.instance_id, &portalconf );
				if( EAG_RETURN_OK == ret1 )
				{		
					keyid = 0;
					memset(keystr,0,sizeof(keystr));
					
					switch(portalconf.portal_srv[0].key_type){						
					case PORTAL_KEYTYPE_ESSID:
						strncpy(keystr,portalconf.portal_srv[0].key.essid,sizeof(keystr)-1);
						break;
					case PORTAL_KEYTYPE_WLANID:
						keyid = portalconf.portal_srv[0].key.wlanid;
						break;
					case PORTAL_KEYTYPE_VLANID:
						keyid = portalconf.portal_srv[0].key.vlanid;
						break;
					case PORTAL_KEYTYPE_WTPID:
						keyid = portalconf.portal_srv[0].key.wtpid;
						break;
					case PORTAL_KEYTYPE_INTF:
						strncpy(keystr,portalconf.portal_srv[0].key.intf,sizeof(keystr)-1);
						break;
					default:
						break;
					}				

					ret2 = eag_modify_portal_server( connection,
												dot11AcPara_parameter.local_id,
												dot11AcPara_parameter.instance_id,					
												portalconf.portal_srv[0].key_type,
												keyid,
												keystr,
												input_string, 
												portalconf.portal_srv[0].ntf_port,
												portalconf.portal_srv[0].domain,
												0, 0);

				}
			}

	        if(ret2 != EAG_RETURN_OK)
	        {
	            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
	        }
			FREE_OBJECT(input_string);
		}
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
            if (0/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
            if (0/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acConPortalURL\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
			
    }

	snmp_log(LOG_DEBUG, "exit handle_acConPortalURL\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acPortalServerPort (netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acPortalServerPort\n");

	if(access(MULTI_PORTAL_F,0)!=0)/*文件不存在*/
	{
		create_eag_xml(MULTI_PORTAL_F);
	}
    switch(reqinfo->mode) {

		case MODE_GET:
		{
			instance_parameter *paraHead = NULL;
			int ret = -1;
			struct portal_conf portalconf;
			memset( &portalconf, 0, sizeof(struct portal_conf) );
			int port = 0;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			if(paraHead)
			{
				memcpy(&dot11AcPara_parameter,&(paraHead->parameter),sizeof(dot11AcPara_parameter));
				
				ret = eag_get_portal_conf( paraHead->connection, paraHead->parameter.local_id,paraHead->parameter.instance_id, &portalconf );
				if( EAG_RETURN_OK == ret )
				{		
					port = portalconf.portal_srv[0].ntf_port;
				}
			}
			free_instance_parameter_list(&paraHead);
			
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&port,
										sizeof(port));			
		}
			break;
		
		case MODE_SET_RESERVE1:
			break;

		case MODE_SET_RESERVE2:
			/* XXX malloc "undo" storage buffer */
			break;

		case MODE_SET_FREE:
			/* XXX: free resources allocated in RESERVE1 and/or
			   RESERVE2.  Something failed somewhere, and the states
			   below won't be called. */
			break;

		case MODE_SET_ACTION:
		{
			int result = SNMPD_DBUS_ERROR;
			void *connection = NULL;
			int ret1 = -1;
			struct portal_conf portalconf;
			memset( &portalconf, 0, sizeof(struct portal_conf) );
			int ret2 = EAG_ERR_DBUS_FAILED;
			unsigned long keyid = 0;
			char keystr[255] = { 0 };

			result = get_instance_dbus_connection(dot11AcPara_parameter, &connection, SNMPD_INSTANCE_MASTER_V3);
			if(SNMPD_DBUS_SUCCESS == result)
			{
				ret1 = eag_get_portal_conf( connection, dot11AcPara_parameter.local_id,dot11AcPara_parameter.instance_id, &portalconf );
				if( EAG_RETURN_OK == ret1 )
				{		
					keyid = 0;
					memset(keystr,0,sizeof(keystr));
					
					switch(portalconf.portal_srv[0].key_type){						
					case PORTAL_KEYTYPE_ESSID:
						strncpy(keystr,portalconf.portal_srv[0].key.essid,sizeof(keystr)-1);
						break;
					case PORTAL_KEYTYPE_WLANID:
						keyid = portalconf.portal_srv[0].key.wlanid;
						break;
					case PORTAL_KEYTYPE_VLANID:
						keyid = portalconf.portal_srv[0].key.vlanid;
						break;
					case PORTAL_KEYTYPE_WTPID:
						keyid = portalconf.portal_srv[0].key.wtpid;
						break;
					case PORTAL_KEYTYPE_INTF:
						strncpy(keystr,portalconf.portal_srv[0].key.intf,sizeof(keystr)-1);
						break;
					default:
						break;
					}				
	
					ret2 = eag_modify_portal_server( connection,
												dot11AcPara_parameter.local_id,
												dot11AcPara_parameter.instance_id,					
												portalconf.portal_srv[0].key_type,
												keyid,
												keystr,
												portalconf.portal_srv[0].portal_url, 
												*requests->requestvb->val.integer,
												portalconf.portal_srv[0].domain,
												0, 0);

				}
			}

	        if(ret2 != EAG_RETURN_OK)
	        {
	            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
	        }
		}
			break;

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNetMask\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acPortalServerPort\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acMaxPortalOnlineUsers(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acMaxPortalOnlineUsers\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acMaxPortalOnlineUsers = 0;
			
            snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
            snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wc_config\n");
            messageHead = list_connection_call_dbus_method(show_wc_config, SHOW_ALL_WTP_TABLE_METHOD);
        	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wc_config,messageHead=%p\n", messageHead);
        	
        	if(messageHead)
        	{
        		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
        		{
        		    DCLI_AC_API_GROUP_FIVE *wirelessconfig = messageNode->message;
        		    if((wirelessconfig != NULL)&&(wirelessconfig->wireless_control != NULL))
			        {
			            acMaxPortalOnlineUsers += wirelessconfig->wireless_control->max_wtp;
        		    }
        		} 
        		free_dbus_message_list(&messageHead, Free_wc_config);
			}
			
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acMaxPortalOnlineUsers,
                                     sizeof(acMaxPortalOnlineUsers));
                                     
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNetMask\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acMaxPortalOnlineUsers\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acPortalOnlineUsers(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acPortalOnlineUsers\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			int ret = 0;
			struct eag_userdb userdb = {0};
			struct eag_user *user = NULL;
			int PortalOnlineUsers = 0;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_userdb_init(&userdb);
				ret = eag_show_user_all(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&userdb);
				if(EAG_RETURN_OK == ret)
				{
					list_for_each_entry(user, &(userdb.head), node)
					{
						if (0 != user->sta_state) {
							continue;
						}
						PortalOnlineUsers++;
					}	
				}
				eag_userdb_destroy(&userdb);
			}
			free_instance_parameter_list(&paraHead);

			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&PortalOnlineUsers,
										sizeof(PortalOnlineUsers));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPortalOnlineUsers\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acPortalOnlineUsers\n");
	return SNMP_ERR_NOERROR;
}
int
 handle_acMaxPPPoEOnlineUsers (netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acMaxPPPoEOnlineUsers\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acMaxPPPoEOnlineUsers = 0;
			
            snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
            snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wc_config\n");
            messageHead = list_connection_call_dbus_method(show_wc_config, SHOW_ALL_WTP_TABLE_METHOD);
        	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wc_config,messageHead=%p\n", messageHead);
        	
        	if(messageHead)
        	{
        		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
        		{
        		    DCLI_AC_API_GROUP_FIVE *wirelessconfig = messageNode->message;
        		    if((wirelessconfig != NULL)&&(wirelessconfig->wireless_control != NULL))
			        {
			            acMaxPPPoEOnlineUsers += wirelessconfig->wireless_control->max_wtp;
        		    }
        		} 
        		free_dbus_message_list(&messageHead, Free_wc_config);
			}
			
	        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
	                                 (u_char *)&acMaxPPPoEOnlineUsers,
	                                 sizeof(acMaxPPPoEOnlineUsers));
        }
        break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNetMask\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acMaxPPPoEOnlineUsers\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acPPPoEOnlineUsers(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acPPPoEOnlineUsers\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int usrnum = 0;
			FILE *fp = NULL;
			char temp[20] = { 0 };
			memset(temp,0,20);
			char *endptr = NULL;  
			
			fp = popen("ps -ef |grep pppoe |grep -v pppoe-server |grep -v grep |wc -l","r");
			if(fp != NULL)
			{
				fgets(temp,20,fp);
				if(strcmp(temp,"")!=0)
				{
					usrnum= strtoul(temp,&endptr,10);	 /*char转成int，10代表十进制*/
				}
				pclose(fp);
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
	                                     (u_char *) &usrnum,
	                                     sizeof(usrnum));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNetMask\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acPPPoEOnlineUsers\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acCertifServerType(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acCertifServerType\n");

    switch(reqinfo->mode) {

        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)"IWN A2401(541)",
                                     strlen("IWN A2410(541)"));
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acCertifServerType\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acCertifServerType\n");
    return SNMP_ERR_NOERROR;
}int
handle_acConASServerIP(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acConASServerIP\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acConASServerIP = 0;
            snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS,
                                     (u_char *) &acConASServerIP,
                                     sizeof(int));
        }
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
         #if 0
        case MODE_SET_RESERVE1:
            if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
                netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
            }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
            if (/* XXX if malloc, or whatever, failed: */) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
            if (/* XXX: error? */) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
            break;
  		#endif
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acConASServerIP\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acConASServerIP\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acFirstConDNSServer(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acFirstConDNSServer\n");

	switch(reqinfo->mode) {

	case MODE_GET:
	{
		int ip = 0;
		FILE * first_dns = NULL;
		char dns[20] = { 0 };
		memset(dns,0,20);
		first_dns = popen("cat /etc/resolv.conf | grep nameserver | sed '2,200d' | awk '{print $2}'","r");

		if(first_dns != NULL)
		{
			fgets(dns,20,first_dns);
			if(strcmp(dns,"")==0)
			{
				memset(dns,0,20);
				strncpy(dns,"0.0.0.0",sizeof(dns)-1);
			}
			INET_ATON(ip,dns);
			pclose(first_dns);
		}
		snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS,
									(u_char *)&ip,
									sizeof(long));
	}
	break;
	case MODE_SET_RESERVE1:
		break;

	case MODE_SET_RESERVE2:
	/* XXX malloc "undo" storage buffer */
	break;

	case MODE_SET_FREE:
	/* XXX: free resources allocated in RESERVE1 and/or
	RESERVE2.  Something failed somewhere, and the states
	below won't be called. */
	break;

	case MODE_SET_ACTION:
	{
		int status;
		int value = *requests->requestvb->val.integer;
		char route[150] = { 0 };
		memset(route,0,150);
		char next_hop[20] = { 0 };
		memset(next_hop,0,20);
		INET_NTOA(value,next_hop);		
		memset(route,0,150);
		snprintf(route,sizeof(route)-1,"sudo set_dns_pro_ip.sh /etc/resolv.conf %s",next_hop);
		status=system(route);
		
		if(status!=0)
        {
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
        }
	}
	break;

	case MODE_SET_COMMIT:
	break;

	case MODE_SET_UNDO:
	break;

	default:
	/* we should never get here, so this is a really bad error */
	snmp_log(LOG_ERR, "unknown mode (%d) in handle_acConDNSServer\n", reqinfo->mode );
	return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acFirstConDNSServer\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acSeconConDNSServer(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acSeconConDNSServer\n");

    switch(reqinfo->mode) {

    case MODE_GET:
	{
		long ip = 0;
		FILE * first_dns = NULL;
		char dns[50] = { 0 };
		memset(dns,0,50);
		first_dns = popen("cat /etc/resolv.conf | grep nameserver | sed '1,1d' |sed '2,200d'| awk '{print $2}'","r");

		if(first_dns != NULL)
		{
			fgets(dns,50,first_dns);
			if(strcmp(dns,"")==0)
			{
				memset(dns,0,50);
				strncpy(dns,"0.0.0.0",sizeof(dns)-1);
			}
			INET_ATON(ip,dns);
			pclose(first_dns);
		}
        snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS,
                                 (u_char*)&ip,
                                 sizeof(long));
    }
            break;
	case MODE_SET_RESERVE1:
		break;

	case MODE_SET_RESERVE2:
	/* XXX malloc "undo" storage buffer */
	break;

	case MODE_SET_FREE:
	/* XXX: free resources allocated in RESERVE1 and/or
	RESERVE2.  Something failed somewhere, and the states
	below won't be called. */
	break;

	case MODE_SET_ACTION:
	{
		int status;
		int value = *requests->requestvb->val.integer;
		char route[150] = { 0 };
		memset(route,0,150);
		char next_hop[20] = { 0 };
		memset(next_hop,0,20);
		INET_NTOA(value,next_hop);
		syslog(LOG_NOTICE,"ip=%s",next_hop);
		snprintf(route,sizeof(route)-1,"sudo set_dns_backup_ip.sh /etc/resolv.conf %s",next_hop);		
		status=system(route);
		syslog(LOG_NOTICE,"status=%d",status);
		if(status!=0)
        {
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
        }
	}
	break;

	case MODE_SET_COMMIT:
	break;

	case MODE_SET_UNDO:
	break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acConDNSServer\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSeconConDNSServer\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acTrapDesIPAddress(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */ 
    snmp_log(LOG_DEBUG, "enter handle_acTrapDesIPAddress\n");

    switch(reqinfo->mode) {

	case MODE_GET:
	{	
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *)"",
									strlen(""));
	}
	break;
	  	case MODE_SET_RESERVE1:
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            {

        	}
            break;

        case MODE_SET_COMMIT:
            break;

        case MODE_SET_UNDO:
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acTrapDesIPAddress\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acTrapDesIPAddress\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSNMPPort(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_acSNMPPort\n");
     switch(reqinfo->mode)
	{
        case MODE_GET:
		{
			int cllection_mode = 0;/*0表示关闭分板采集，1表示开启分板采集*/  
			FILE *fd = NULL;
			int master_slot = 0;
			int acSNMPPort = 0;
			int flag=1;
			DBusConnection *select_connection = NULL;
			STSNMPSysInfo snmp_info;
			int ret1 = AC_MANAGE_SUCCESS, ret2 = AC_MANAGE_SUCCESS, ret3 = AC_MANAGE_SUCCESS;
			SNMPINTERFACE *interface_array = NULL;
			unsigned int port = 0;
			unsigned int snmp_state = 0;  
			unsigned int interface_num = 0;  


			if(snmp_cllection_mode(ccgi_dbus_connection))
			{	
				cllection_mode = 1;
				if(VALID_DBM_FLAG == get_dbm_effective_flag())
				{
					fd = fopen("/dbm/local_board/slot_id", "r");
				}
				if (fd == NULL)
				{
					snmp_log(LOG_DEBUG,"Get production information [2] error\n");
				  	flag = 0;
					acSNMPPort = 0;
				}
				if(fd)
				{
					fscanf(fd, "%d", &master_slot);
					fclose(fd);
				}
			}
			else
			{
				cllection_mode = 0;
				if(VALID_DBM_FLAG == get_dbm_effective_flag())
				{
					fd = fopen("/dbm/product/active_master_slot_id", "r");
				}
				if (fd == NULL)
				{
					snmp_log(LOG_DEBUG,"Get production information [2] error\n");
					flag = 0;
					acSNMPPort = 0;
				}
				if(fd)
				{
					fscanf(fd, "%d", &master_slot);
					fclose(fd);
				}
			}

			if(flag == 1)
			{
				if(1 == cllection_mode)/*开启分板采集*/
				{
					get_slot_dbus_connection(master_slot,&select_connection,SNMPD_INSTANCE_MASTER_V3);
					ret1 = ac_manage_show_snmp_base_info(select_connection, &snmp_info);
					ret2 = ac_manage_show_snmp_state(select_connection, &snmp_state);
					if(AC_MANAGE_SUCCESS == ret1) 
					{
					  ret3 = ac_manage_show_snmp_pfm_interface(select_connection, &interface_array, &interface_num, &port);
					}
					if(AC_MANAGE_SUCCESS == ret1 && AC_MANAGE_SUCCESS == ret2) 
					{
						if(0 == interface_num) 
							acSNMPPort = 0;
						else
							acSNMPPort = snmp_info.agent_port ? snmp_info.agent_port : 161;
		 			}
		 		}
				else
				{
					ret1 = ac_manage_show_snmp_base_info(ccgi_dbus_connection, &snmp_info);
					ret2 = ac_manage_show_snmp_state(ccgi_dbus_connection, &snmp_state);
		 			if(AC_MANAGE_SUCCESS == ret1 && AC_MANAGE_SUCCESS == ret2) 
					{
						acSNMPPort = snmp_info.agent_port ? snmp_info.agent_port : 161;
		 			}
		 		}
		 	}
			 			
		    snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
		                                     (u_char *)&acSNMPPort,
		                                     sizeof(acSNMPPort));
		}
			break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSNMPPort\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSNMPPort\n");
    return SNMP_ERR_NOERROR;
}

int
handle_SysGWAddr(netsnmp_mib_handler *handler,
								  netsnmp_handler_registration *reginfo,
								  netsnmp_agent_request_info   *reqinfo,
								  netsnmp_request_info		   *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	   "instance", as it's "magically" handled for us.	*/

	/* a instance handler also only hands us one request at a time, so
	   we don't need to loop over a list of requests; we'll only get one. */ 
	snmp_log(LOG_DEBUG, "enter handle_SysGWAddr\n");

	switch(reqinfo->mode) {

		case MODE_GET:
		{
			FILE *fd = NULL;
			char temp[256] = { 0 };
			memset(temp,0,256);
			char  SysGWAddr[20];
			memset(SysGWAddr,0,20);
			strncpy(temp,"show_route.sh | awk '{if($2~/0.0.0.0\\/0/){if($1~/K/) print $4 ;else print $5}}' | sed 's/,//g' | tail -1",sizeof(temp)-1);

			if((fd=popen(temp,"r"))!=NULL)
			{
				memset(SysGWAddr,0,20);
				fgets(SysGWAddr,20,fd);
			}
			delete_enter(SysGWAddr);
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *) SysGWAddr,
										strlen(SysGWAddr));

			if(fd != NULL)
			{
				pclose(fd);
			}
		}
		break;
		case MODE_SET_RESERVE1:
			break;

		case MODE_SET_RESERVE2:
			/* XXX malloc "undo" storage buffer */
			break;

		case MODE_SET_FREE:
			/* XXX: free resources allocated in RESERVE1 and/or
			   RESERVE2.  Something failed somewhere, and the states
			   below won't be called. */
			break;

		case MODE_SET_ACTION:
		{
			char command[256] = { 0 };
			int ip_addr;
			char next_hop[20] = { 0 };
			memset(next_hop,0,20);
			int status;
			int ret=1;
			
			memset(command,0,256);
			strncpy(command,"set_static_route.sh 0.0.0.0 0.0.0.0 ",sizeof(command)-1);
			ip_addr = *requests->requestvb->val.integer;
			INET_NTOA(ip_addr,next_hop);
			strncat(command,next_hop,sizeof(command)-strlen(command)-1);
			strncat(command," 1",sizeof(command)-strlen(command)-1);

			status = system(command);	

			snmp_log(LOG_DEBUG, "enter WEXITSTATUS\n");
           	ret = WEXITSTATUS(status);			
			snmp_log(LOG_DEBUG, "exit WEXITSTATUS,ret=%d\n", ret);
			
			if(ret!=0)
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
		}
			break;

		case MODE_SET_COMMIT:
			break;

		case MODE_SET_UNDO:
			break;

		default:
			/* we should never get here, so this is a really bad error */
			snmp_log(LOG_ERR, "unknown mode (%d) in handle_SysGWAddr\n", reqinfo->mode );
			return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_SysGWAddr\n");
	return SNMP_ERR_NOERROR;
}

int
handle_acTrapPort(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acTrapPort\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{  
			int value = 162;
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char*)&value,
									sizeof(value));
			
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMacAddress\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acTrapPort\n");
	return SNMP_ERR_NOERROR;
}
