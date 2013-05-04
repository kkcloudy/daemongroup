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
* wapiBasicInfo.c
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
#include <sys/sysinfo.h>
#include "wapiBasicInfo.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sta.h"
#include "autelanWtpGroup.h"
/** Initializes the wapiBasicInfo module */
#define WAPICONFIGVERSION						"1.10.1.1"
#define WAPIOPTIONIMPLEMENTED					"1.10.1.2"
#define WAPIPREAUTHIMPLEMENTED				"1.10.1.3"
#define WAPIPREAUTHENABLED					"1.10.1.4"
#define WAPIUNICASTKEYSSUPPORTED				"1.10.1.5"
#define WAPIMULTICASTCIPHER					"1.10.1.6"
#define WAPIMULTICASTREKEYSTRICT				"1.10.1.7"
#define WAPIMULTICASTCIPHERSIZE				"1.10.1.8"
#define WAPIBKLIFETIME							"1.10.1.9"
#define WAPIBKREAUTHTHRESHOLD				"1.10.1.10"
#define WAPISATIMEOUT							"1.10.1.11"
#define WAPIUNICASTCIPHERSELECTED				"1.10.1.12"
#define WAPIMULTICASTCIPHERSELECTED			"1.10.1.13"
#define WAPIUNICASTCIPHERREQUESTED			"1.10.1.14"
#define WAPIMULTICASTCIPHERREQUESTED			"1.10.1.15"

static unsigned int wapiConfigVersion = 0;
static int wapiPreauthEnabled = 0;
static unsigned int wapiUnicastKeysSupported = 0;
static char wapiMulticastCipher[30] = { 0 };
static unsigned int wapiMulticastRekeyStrict = 0;
static unsigned int wapiMulticastCipherSize = 0;
static unsigned int wapiBKLifetime = 0;
static unsigned int wapiBKReauthThreshold = 0;
static unsigned int wapiSATimeout = 0;
static char wapiUnicastCipherSelected[50] = { 0 };
static char wapiMulticastCipherSelected[50] = { 0 };
static char wapiUnicastCipherRequested[50] = { 0 };
static char wapiMulticastCipherRequested[50] = { 0 };
static long update_time_show_wapi_protocol_info_cmd_func = 0;
static void update_data_for_show_wapi_protocol_info_cmd_func();


init_wapiBasicInfo(void)
{
	static oid wapiConfigVersion_oid[128] = {0};
	static oid wapiOptionImplemented_oid[128] = {0};
	static oid wapiPreauthImplemented_oid[128] = {0};
	static oid wapiPreauthEnabled_oid[128] = {0};
	static oid wapiUnicastKeysSupported_oid[128] = {0};
	static oid wapiMulticastCipher_oid[128] = {0};
	static oid wapiMulticastRekeyStrict_oid[128] = {0};
	static oid wapiMulticastCipherSize_oid[128] = {0};
	static oid wapiBKLifetime_oid[128] = {0};
	static oid wapiBKReauthThreshold_oid[128] = {0};
	static oid wapiSATimeout_oid[128] = {0};
	static oid wapiUnicastCipherSelected_oid[128] = {0};
	static oid wapiMulticastCipherSelected_oid[128] = {0};
	static oid wapiUnicastCipherRequested_oid[128] = {0};
	static oid wapiMulticastCipherRequested_oid[128] = {0};
	size_t public_oid_len   = 0;
	mad_dev_oid(wapiConfigVersion_oid,WAPICONFIGVERSION,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiOptionImplemented_oid,WAPIOPTIONIMPLEMENTED ,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiPreauthImplemented_oid,WAPIPREAUTHIMPLEMENTED ,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiPreauthEnabled_oid,WAPIPREAUTHENABLED,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiUnicastKeysSupported_oid,WAPIUNICASTKEYSSUPPORTED,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiMulticastCipher_oid,WAPIMULTICASTCIPHER,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiMulticastRekeyStrict_oid,WAPIMULTICASTREKEYSTRICT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiMulticastCipherSize_oid,WAPIMULTICASTCIPHERSIZE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiBKLifetime_oid,WAPIBKLIFETIME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiBKReauthThreshold_oid,WAPIBKREAUTHTHRESHOLD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiSATimeout_oid,WAPISATIMEOUT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiUnicastCipherSelected_oid,WAPIUNICASTCIPHERSELECTED,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiMulticastCipherSelected_oid,WAPIMULTICASTCIPHERSELECTED,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiUnicastCipherRequested_oid,WAPIUNICASTCIPHERREQUESTED,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wapiMulticastCipherRequested_oid,WAPIMULTICASTCIPHERREQUESTED,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("wapiBasicInfo", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiConfigVersion", handle_wapiConfigVersion,
                               wapiConfigVersion_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiOptionImplemented", handle_wapiOptionImplemented,
                               wapiOptionImplemented_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiPreauthImplemented", handle_wapiPreauthImplemented,
                               wapiPreauthImplemented_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiPreauthEnabled", handle_wapiPreauthEnabled,
                               wapiPreauthEnabled_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiUnicastKeysSupported", handle_wapiUnicastKeysSupported,
                               wapiUnicastKeysSupported_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiMulticastCipher", handle_wapiMulticastCipher,
                               wapiMulticastCipher_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiMulticastRekeyStrict", handle_wapiMulticastRekeyStrict,
                               wapiMulticastRekeyStrict_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiMulticastCipherSize", handle_wapiMulticastCipherSize,
                               wapiMulticastCipherSize_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiBKLifetime", handle_wapiBKLifetime,
                               wapiBKLifetime_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiBKReauthThreshold", handle_wapiBKReauthThreshold,
                               wapiBKReauthThreshold_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiSATimeout", handle_wapiSATimeout,
                               wapiSATimeout_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiUnicastCipherSelected", handle_wapiUnicastCipherSelected,
                               wapiUnicastCipherSelected_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiMulticastCipherSelected", handle_wapiMulticastCipherSelected,
                               wapiMulticastCipherSelected_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiUnicastCipherRequested", handle_wapiUnicastCipherRequested,
                               wapiUnicastCipherRequested_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wapiMulticastCipherRequested", handle_wapiMulticastCipherRequested,
                               wapiMulticastCipherRequested_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_wapiConfigVersion(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiConfigVersion\n");

    switch(reqinfo->mode) {

        case MODE_GET:
	{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
					(u_char *)&wapiConfigVersion,
					sizeof(long));
	}
	break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiConfigVersion\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
	
	snmp_log(LOG_DEBUG, "exit handle_wapiConfigVersion\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiOptionImplemented(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiOptionImplemented\n");

    switch(reqinfo->mode) {

	case MODE_GET:
	{
		int val = 1;
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
								(u_char *)&val,
								sizeof(long));
	}
	break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiOptionImplemented\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiOptionImplemented\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiPreauthImplemented(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiPreauthImplemented\n");

    switch(reqinfo->mode) {

        case MODE_GET:
	{
		int val = 2;
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
								(u_char *)&val,
								sizeof(long));
	}
	break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiPreauthImplemented\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiPreauthImplemented\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiPreauthEnabled(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	snmp_log(LOG_DEBUG, "enter handle_wapiPreauthEnabled\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
	{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		
		int val = (wapi_protocol_info.WapiPreauthEnabled==1)?1:2;
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
					(u_char *)&wapiPreauthEnabled,
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
          ///  if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
            //    netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
           // }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
        //    if (/* XXX if malloc, or whatever, failed: */) {
        //        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
        //    }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
         //   break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
         //   if (/* XXX: error? */) {
         //       netsnmp_set_request_error(reqinfo, requests, /* some error */);
          //  }
        //    break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
         //   if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
         //       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
        //    }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
       //     if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
       //         netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
       //     }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiPreauthEnabled\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiPreauthEnabled\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiUnicastKeysSupported(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiUnicastKeysSupported\n");

    switch(reqinfo->mode) {

        case MODE_GET:
	{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		int tmp = 0;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));
		wapi_protocol_info.UnicastKeysSupported = 0;

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		
		tmp = wapi_protocol_info.UnicastKeysSupported;
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
					(u_char *)&wapiUnicastKeysSupported,
					sizeof(long));
	}            
		break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiUnicastKeysSupported\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiUnicastKeysSupported\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiMulticastCipher(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiMulticastCipher\n");

    switch(reqinfo->mode) {

        case MODE_GET:
				{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		
		char val[50] = { 0 };
		memset(val,0,sizeof(val));
		snprintf(val, sizeof(val) - 1, "%02X-%02X-%02X-%02X\n",
		        wapi_protocol_info.MulticastCipher[0], wapi_protocol_info.MulticastCipher[1],
		        wapi_protocol_info.MulticastCipher[2], wapi_protocol_info.MulticastCipher[3]);

		delete_enter(val);
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
					(u_char *)wapiMulticastCipher,
					strlen(wapiMulticastCipher));
		}
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
         //   if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
          //      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
          //  }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
          //  if (/* XXX if malloc, or whatever, failed: */) {
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
          //  }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        //    if (/* XXX: error? */) {
        //        netsnmp_set_request_error(reqinfo, requests, /* some error */);
        //    }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
        //    if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
           //     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
         //   }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
         //   if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
         //       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
         //   }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiMulticastCipher\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiMulticastCipher\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiMulticastRekeyStrict(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiMulticastRekeyStrict\n");

    switch(reqinfo->mode) {

        case MODE_GET:
	{
		#if 0
		int wapiMulticastRekeyStrict = 0;
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		
		if(wapi_protocol_info.MulticastRekeyStrict==1)
			wapiMulticastRekeyStrict = 1;
		else
			wapiMulticastRekeyStrict = 2;
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&wapiMulticastRekeyStrict,
									sizeof(wapiMulticastRekeyStrict));
	}
	break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
          //  if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
          //      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
          //  }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
          //  if (/* XXX if malloc, or whatever, failed: */) {
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
          ///  }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
          //  if (/* XXX: error? */) {
           //     netsnmp_set_request_error(reqinfo, requests, /* some error */);
          //  }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
         //   if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
         //   }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
        ///    if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
         //       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
        //    }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiMulticastRekeyStrict\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiMulticastRekeyStrict\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiMulticastCipherSize(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiMulticastCipherSize\n");

    switch(reqinfo->mode) {

        case MODE_GET:
	{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_UNSIGNED,
									(u_char *)&wapiMulticastCipherSize,
									sizeof(long));
	}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiMulticastCipherSize\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiMulticastCipherSize\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiBKLifetime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiBKLifetime\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
					(u_char *)&wapiBKLifetime,
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
       //     if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
         //       netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
       //     }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
         //   if (/* XXX if malloc, or whatever, failed: */) {
         //       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
         //   }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
        //    if (/* XXX: error? */) {
        //        netsnmp_set_request_error(reqinfo, requests, /* some error */);
        //    }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
       //     if (/* XXX: error? */) {
       //         /* try _really_really_ hard to never get to this point */
       //         netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
      //      }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
        //    if (/* XXX: error? */) {
        //        /* try _really_really_ hard to never get to this point */
         ///       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
        //    }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiBKLifetime\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiBKLifetime\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiBKReauthThreshold(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiBKReauthThreshold\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
					(u_char *)&wapiBKReauthThreshold,
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
           // if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
           //     netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
          //  }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
          //  if (/* XXX if malloc, or whatever, failed: */) {
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
          //  }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
         //   if (/* XXX: error? */) {
         //       netsnmp_set_request_error(reqinfo, requests, /* some error */);
         //   }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
          //  if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
         //       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
         //   }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
            //if (/* XXX: error? */) {
            //    /* try _really_really_ hard to never get to this point */
           //     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
         //   }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiBKReauthThreshold\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiBKReauthThreshold\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiSATimeout(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiSATimeout\n");

    switch(reqinfo->mode) {

        case MODE_GET:
				{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
					(u_char *)&wapiSATimeout,
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
          //  if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
          //      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
         //   }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
        //    if (/* XXX if malloc, or whatever, failed: */) {
        //        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
        //    }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
         //   if (/* XXX: error? */) {
         //       netsnmp_set_request_error(reqinfo, requests, /* some error */);
         //   }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
         //   if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
         //       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
         //   }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
          //  if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
         //   }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiSATimeout\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiSATimeout\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiUnicastCipherSelected(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiUnicastCipherSelected\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		
		char val[50] = { 0 };
		memset(val,0,sizeof(val));
		snprintf(val, sizeof(val) - 1, "%02X-%02X-%02X-%02X\n",
		        wapi_protocol_info.UnicastCipherSelected[0], wapi_protocol_info.UnicastCipherSelected[1],
		        wapi_protocol_info.UnicastCipherSelected[2], wapi_protocol_info.UnicastCipherSelected[3]);
		        
		delete_enter(val);
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
					(u_char *)wapiUnicastCipherSelected,
					strlen(wapiUnicastCipherSelected));
	}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiUnicastCipherSelected\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiUnicastCipherSelected\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiMulticastCipherSelected(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiMulticastCipherSelected\n");

    switch(reqinfo->mode) {

        case MODE_GET:
	{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		
		char val[50] = { 0 };
		memset(val,0,sizeof(val));
		snprintf(val, sizeof(val) - 1, "%02X-%02X-%02X-%02X\n",
		        wapi_protocol_info.MulticastCipherSelected[0], wapi_protocol_info.MulticastCipherSelected[1],
		        wapi_protocol_info.MulticastCipherSelected[2], wapi_protocol_info.MulticastCipherSelected[3]);
		        
		delete_enter(val);
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
					(u_char *)wapiMulticastCipherSelected,
					strlen(wapiMulticastCipherSelected));
	}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiMulticastCipherSelected\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiMulticastCipherSelected\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiUnicastCipherRequested(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiUnicastCipherRequested\n");

    switch(reqinfo->mode) {

        case MODE_GET:
				{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		
		char val[50] = { 0 };
		memset(val,0,sizeof(val));
		snprintf(val, sizeof(val) - 1, "%02X-%02X-%02X-%02X\n",
		        wapi_protocol_info.UnicastCipherRequested[0], wapi_protocol_info.UnicastCipherRequested[1],
		        wapi_protocol_info.UnicastCipherRequested[2], wapi_protocol_info.UnicastCipherRequested[3]);

		delete_enter(val);
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
					(u_char *)wapiUnicastCipherRequested,
					strlen(wapiUnicastCipherRequested));
	}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiUnicastCipherRequested\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiUnicastCipherRequested\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wapiMulticastCipherRequested(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_wapiMulticastCipherRequested\n");

    switch(reqinfo->mode) {

        case MODE_GET:
				{
		#if 0
		struct wapi_protocol_info_profile wapi_protocol_info;
		memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

		snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
		show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
		snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
		
		char val[50] = { 0 };
		memset(val,0,sizeof(val));
		snprintf(val, sizeof(val) - 1, "%02X-%02X-%02X-%02X\n",
		        wapi_protocol_info.MulticastCipherRequested[0], wapi_protocol_info.MulticastCipherRequested[1],
		        wapi_protocol_info.MulticastCipherRequested[2], wapi_protocol_info.MulticastCipherRequested[3]);

		delete_enter(val);
		#endif

		update_data_for_show_wapi_protocol_info_cmd_func();
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
					(u_char *)wapiMulticastCipherRequested,
					strlen(wapiMulticastCipherRequested));
	}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wapiMulticastCipherRequested\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_wapiMulticastCipherRequested\n");
    return SNMP_ERR_NOERROR;
}

static void update_data_for_show_wapi_protocol_info_cmd_func()
{
	struct sysinfo info;
	
	if(0 != update_time_show_wapi_protocol_info_cmd_func)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_show_wapi_protocol_info_cmd_func < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_wapi_protocol_info_cmd_func\n");
	
	/*update cache data*/
	struct wapi_protocol_info_profile wapi_protocol_info;
	memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

	wapiConfigVersion = 0;	
	wapiPreauthEnabled = 0;
	wapiUnicastKeysSupported = 0;
	memset(wapiMulticastCipher,0,sizeof(wapiMulticastCipher));
	wapiMulticastRekeyStrict = 0;
	wapiMulticastCipherSize = 0;
	wapiBKLifetime = 0;
	wapiBKReauthThreshold = 0;
	wapiSATimeout = 0;
	memset(wapiUnicastCipherSelected,0,sizeof(wapiUnicastCipherSelected));
	memset(wapiMulticastCipherSelected,0,sizeof(wapiMulticastCipherSelected));
	memset(wapiUnicastCipherRequested,0,sizeof(wapiUnicastCipherRequested));
	memset(wapiMulticastCipherRequested,0,sizeof(wapiMulticastCipherRequested));
	
	snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
	show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
	snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");

	wapiConfigVersion = wapi_protocol_info.ConfigVersion;	
	wapiPreauthEnabled = (wapi_protocol_info.WapiPreauthEnabled==1)?1:2;
	wapiUnicastKeysSupported = wapi_protocol_info.UnicastKeysSupported;
	snprintf(wapiMulticastCipher,sizeof(wapiMulticastCipher)-1,"%02X-%02X-%02X-%02X\n",wapi_protocol_info.MulticastCipher[0],wapi_protocol_info.MulticastCipher[1],wapi_protocol_info.MulticastCipher[2],wapi_protocol_info.MulticastCipher[3]);
	delete_enter(wapiMulticastCipher);
	if(wapi_protocol_info.MulticastRekeyStrict==1)
		wapiMulticastRekeyStrict = 1;
	else
		wapiMulticastRekeyStrict = 2;
	wapiMulticastCipherSize = wapi_protocol_info.MulticastCipherSize;
	wapiBKLifetime = wapi_protocol_info.BKLifetime;
	wapiBKReauthThreshold = wapi_protocol_info.BKReauthThreshold;
	wapiSATimeout = wapi_protocol_info.SATimeout;
	snprintf(wapiUnicastCipherSelected,sizeof(wapiUnicastCipherSelected)-1,"%02X-%02X-%02X-%02X\n",wapi_protocol_info.UnicastCipherSelected[0],wapi_protocol_info.UnicastCipherSelected[1],wapi_protocol_info.UnicastCipherSelected[2],wapi_protocol_info.UnicastCipherSelected[3]);
	delete_enter(wapiUnicastCipherSelected);
	snprintf(wapiMulticastCipherSelected,sizeof(wapiMulticastCipherSelected)-1,"%02X-%02X-%02X-%02X\n",wapi_protocol_info.MulticastCipherSelected[0],wapi_protocol_info.MulticastCipherSelected[1],wapi_protocol_info.MulticastCipherSelected[2],wapi_protocol_info.MulticastCipherSelected[3]);
	delete_enter(wapiMulticastCipherSelected);
	snprintf(wapiUnicastCipherRequested,sizeof(wapiUnicastCipherRequested)-1,"%02X-%02X-%02X-%02X\n",wapi_protocol_info.UnicastCipherRequested[0],wapi_protocol_info.UnicastCipherRequested[1],wapi_protocol_info.UnicastCipherRequested[2],wapi_protocol_info.UnicastCipherRequested[3]);
	delete_enter(wapiUnicastCipherRequested);
	snprintf(wapiMulticastCipherRequested,sizeof(wapiMulticastCipherRequested)-1,"%02X-%02X-%02X-%02X\n",wapi_protocol_info.MulticastCipherRequested[0],wapi_protocol_info.MulticastCipherRequested[1],wapi_protocol_info.MulticastCipherRequested[2],wapi_protocol_info.MulticastCipherRequested[3]);
	delete_enter(wapiMulticastCipherRequested);
	
	sysinfo(&info); 		
	update_time_show_wapi_protocol_info_cmd_func = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_wapi_protocol_info_cmd_func\n");
}


