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
* dot11MaxWtpIDInfo.c
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
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_init_dbus.h"
#include "dot11MaxWtpIDInfo.h"
#include "autelanWtpGroup.h"
#include "ws_dbus_list_interface.h"

#define     	MAXWTPIDINFO 		"6.1.4"
void
init_dot11MaxWtpIDInfo(void)
{
    static oid MaxInfo_oid[128] = {0};
	size_t public_oid_len   = 0;
	mad_dev_oid(MaxInfo_oid,MAXWTPIDINFO ,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11MaxWtpIDInfo", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("MaxInfo", handle_MaxInfo,
                               MaxInfo_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_MaxInfo(netsnmp_mib_handler *handler,
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
        unsigned long maxwtpid = 0;
        snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    
        snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wtp_list_by_mac_cmd_func\n");
        messageHead = list_connection_call_dbus_method(show_wtp_list_by_mac_cmd_func, SHOW_ALL_WTP_TABLE_METHOD);
    	snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wtp_list_by_mac_cmd_func,messageHead=%p\n", messageHead);

    	if(messageHead)
    	{
    		for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
    		{
    		    int temp_wtpid = 0;
    		    DCLI_WTP_API_GROUP_ONE *head = messageNode->message;
    		    if((head)&&(head->WTP_INFO))
		  	    { 
		  	        int i = 0;
		  	        WID_WTP *q = NULL;
                    for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q->next); i++,q = q->next)
		  	        {
						temp_wtpid = local_to_global_ID(messageNode->parameter, q->WTPID, WIRELESS_MAX_NUM);
						if(temp_wtpid > maxwtpid)
						{
							maxwtpid = temp_wtpid;
						}
						FREE_OBJECT(q->WTPMAC);
		  	        }
		  	    }
		  	}
		  	free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
		 }
		  	        
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char*)&maxwtpid,
                                     sizeof(u_long));	
	}
    break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_informationinfo\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}
