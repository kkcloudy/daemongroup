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
* dot11UnicastEncryption.c
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
#include "dot11UnicastEncryption.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sta.h"
#include "autelanWtpGroup.h"
/** Initializes the dot11UnicastEncryption module */
#define UNICASTCIPHER   			"1.10.6.1"
#define UNICASTCIPHERENABLED   	"1.10.6.2"
#define UNICASTCIPHERSIZE   		"1.10.6.3"

static char UnicastCipher[50] = { 0 };
static int UnicastCipherEnabled = 0;
static unsigned int UnicastCipherSize = 0;
static long update_time_show_wapi_protocol_info_cmd_func = 0;
static void update_data_for_show_wapi_protocol_info_cmd_func();

void
init_dot11UnicastEncryption(void)
{
    static oid UnicastCipher_oid[128] = {0};
    static oid UnicastCipherEnabled_oid[128] = {0};
    static oid UnicastCipherSize_oid[128] = {0};
	size_t public_oid_len   = 0;
	mad_dev_oid(UnicastCipher_oid,UNICASTCIPHER,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(UnicastCipherEnabled_oid,UNICASTCIPHERENABLED,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(UnicastCipherSize_oid,UNICASTCIPHERSIZE,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11UnicastEncryption", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("UnicastCipher", handle_UnicastCipher,
                               UnicastCipher_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("UnicastCipherEnabled", handle_UnicastCipherEnabled,
                               UnicastCipherEnabled_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("UnicastCipherSize", handle_UnicastCipherSize,
                               UnicastCipherSize_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_UnicastCipher(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_UnicastCipher\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			#if 0
			struct wapi_protocol_info_profile wapi_protocol_info;
			memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));
			char UnicastCipher[50] = { 0 };

			snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
			show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
			snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
			
			snprintf(UnicastCipher, sizeof(UnicastCipher) - 1,"%02X-%02X-%02X-%02X\n",
			            wapi_protocol_info.UnicastCipher[0],wapi_protocol_info.UnicastCipher[1],
			            wapi_protocol_info.UnicastCipher[2],wapi_protocol_info.UnicastCipher[3]);
			            
			delete_enter(UnicastCipher);
			#endif

			update_data_for_show_wapi_protocol_info_cmd_func();
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)UnicastCipher,
										strlen(UnicastCipher));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_UnicastCipher\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_UnicastCipher\n");
	return SNMP_ERR_NOERROR;
}
int
handle_UnicastCipherEnabled(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_UnicastCipherEnabled\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			#if 0
			struct wapi_protocol_info_profile wapi_protocol_info;
			memset(&wapi_protocol_info,0,sizeof(struct wapi_protocol_info_profile));

			snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
			show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
			snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");
			
			int UnicastCipherEnabled = 1;
			UnicastCipherEnabled = (wapi_protocol_info.UnicastCipherEnabled==1)?1:2;
			#endif

			update_data_for_show_wapi_protocol_info_cmd_func();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&UnicastCipherEnabled,
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
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_UnicastCipherEnabled\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_UnicastCipherEnabled\n");
	return SNMP_ERR_NOERROR;
}
int
handle_UnicastCipherSize(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_UnicastCipherSize\n");

	switch(reqinfo->mode) 
	{

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
								(u_char *)&UnicastCipherSize,
								sizeof(u_long));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_UnicastCipherSize\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_UnicastCipherSize\n");
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

	memset(UnicastCipher,0,sizeof(UnicastCipher));
	UnicastCipherEnabled = 0;
	UnicastCipherSize = 0;
	
	snmp_log(LOG_DEBUG, "enter show_wapi_protocol_info_cmd_func\n");
	show_wapi_protocol_info_cmd_func(&wapi_protocol_info);
	snmp_log(LOG_DEBUG, "exit show_wapi_protocol_info_cmd_func\n");

	snprintf(UnicastCipher,sizeof(UnicastCipher)-1,"%02X-%02X-%02X-%02X\n",wapi_protocol_info.UnicastCipher[0],wapi_protocol_info.UnicastCipher[1],wapi_protocol_info.UnicastCipher[2],wapi_protocol_info.UnicastCipher[3]);
	delete_enter(UnicastCipher);
	UnicastCipherEnabled = (wapi_protocol_info.UnicastCipherEnabled==1)?1:2;
	UnicastCipherSize = wapi_protocol_info.UnicastCipherSize;
	
	sysinfo(&info); 		
	update_time_show_wapi_protocol_info_cmd_func = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_wapi_protocol_info_cmd_func\n");
}

