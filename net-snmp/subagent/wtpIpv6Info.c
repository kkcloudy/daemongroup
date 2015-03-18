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
#include "wtpIpv6Info.h"

#define NET_ELEMENT_CONFIG_FILE "/var/run/net_elemnet"
#define MAX_ITEM_NUM	64
#define EAG_CONF_FILE_PATH "/opt/services/conf/eag_conf.conf"   
dbus_parameter wtpIpv6Info_parameter = { 0 };
/** Initializes the wtpIpv6Info module */
#define WTPIPV6PREFIX				"1.18.1"
#define	WTPIPV6TYPE				"1.18.2"
void
init_wtpIpv6Info(void)
{
	static oid wtpIpv6Prefix_oid[128] 			= { 0};
	static oid wtpIpv6Type_oid[128] 				= { 0 };
	size_t public_oid_len   = 0;
	mad_dev_oid(wtpIpv6Prefix_oid,WTPIPV6PREFIX,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wtpIpv6Type_oid,WTPIPV6TYPE,&public_oid_len,enterprise_pvivate_oid);
    

  DEBUGMSGTL(("wtpIpv6Info", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpIpv6Prefix", handle_wtpIpv6Prefix,
                               wtpIpv6Prefix_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpIpv6Type", handle_wtpIpv6Type,
                               wtpIpv6Type_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_wtpIpv6Prefix(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
		{
		    char *wtpipv6prefix = "fe80:0:0:0";
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)wtpipv6prefix, 
                                      strlen(wtpipv6prefix));
		}
        break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpIpv6Prefix\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_wtpIpv6Type(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
        {   
			char *wtpipv6type = "link-local address";
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)wtpipv6type, 
                                    strlen(wtpipv6type));
		}
		
        break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpIpv6Type\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
