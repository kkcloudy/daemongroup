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
* dot11AKMSuite.c
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
#include "dot11AKMSuite.h"
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "autelanWtpGroup.h"
/** Initializes the dot11AKMSuite module */
#define AUTHENTICATIONSUITE    "1.10.7.1"
#define AUTHENTICATIONSUITEENABLED    "1.10.7.2"
void
init_dot11AKMSuite(void)
{
    static oid AuthenticationSuite_oid[128] = {0};
    static oid AuthenticationSuiteEnabled_oid[128] = {0};
	size_t	public_oid_len = 0;
	mad_dev_oid(AuthenticationSuite_oid,AUTHENTICATIONSUITE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(AuthenticationSuiteEnabled_oid,AUTHENTICATIONSUITEENABLED,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11AKMSuite", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("AuthenticationSuite", handle_AuthenticationSuite,
                               AuthenticationSuite_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("AuthenticationSuiteEnabled", handle_AuthenticationSuiteEnabled,
                               AuthenticationSuiteEnabled_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
}

int
handle_AuthenticationSuite(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_AuthenticationSuite\n");

	switch(reqinfo->mode) 
	{
		case MODE_GET:
		{
			char AuthenticationSuite[50] = { 0 };
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)AuthenticationSuite,
										strlen(AuthenticationSuite));
		}
		break;
		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_AuthenticationSuite\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_AuthenticationSuite\n");
	return SNMP_ERR_NOERROR;
}
int
handle_AuthenticationSuiteEnabled(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_AuthenticationSuiteEnabled\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			struct wapi_protocol_info_profile wapi_protocol_info;
			memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

			snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
			show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
			snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
			
			int AuthenticationSuiteEnabled = (wapi_protocol_info.AuthenticationSuiteEnabled==1)?1:2;
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&AuthenticationSuiteEnabled,
										sizeof(long));
		}
		break;

		/*
		* SET REQUEST
		*
		* multiple states in the transaction.  See:
		* http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
		*/
		case MODE_SET_RESERVE1:
		#if 0
		if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
		netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
		}
		#endif
		break;

		case MODE_SET_RESERVE2:
		#if 0
		/* XXX malloc "undo" storage buffer */
		if (/* XXX if malloc, or whatever, failed: */) {
		netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
		}
		#endif
		break;

		case MODE_SET_FREE:
		#if 0
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		#endif
		break;

		case MODE_SET_ACTION:
		#if 0
		/* XXX: perform the value change here */
		if (/* XXX: error? */) {
		netsnmp_set_request_error(reqinfo, requests, /* some error */);
		}
		#endif
		break;

		case MODE_SET_COMMIT:
		#if 0
		/* XXX: delete temporary storage */
		if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		}
		#endif
		break;

		case MODE_SET_UNDO:
		#if 0
		/* XXX: UNDO and return to previous value for the object */
		if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		}
		#endif
		break;

		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_AuthenticationSuiteEnabled\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_AuthenticationSuiteEnabled\n");
	return SNMP_ERR_NOERROR;
}
