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
* dot11InformationInfo.c
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
#include "ws_user_manage.h"
#include "dot11InformationInfo.h"
#include "autelanWtpGroup.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

/** Initializes the InformationInfo module */
#define     	InforM		"2.17.2"
void
init_dot11InformationInfo(void)
{
    static oid informationinfo_oid[128] = {0};
	size_t public_oid_len   = 0;
    mad_dev_oid(informationinfo_oid,InforM,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11InformationInfo", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("informationinfo", handle_informationinfo,
                               informationinfo_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
//	printf("sadfadf");
}

int
handle_informationinfo(netsnmp_mib_handler *handler,
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
        char value[20] = { 0 };
		memset(value,0,20);
        snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char*)value,
                                     strlen(value));
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
            /* XXX malloc "undo" storage buffer */
           // if (/* XXX if malloc, or whatever, failed: */) {
            //    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
          //  }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

	case MODE_SET_ACTION:
	{
	
        int value = 2;
		int length = 0;
	
		
		
		value = send_msg(requests->requestvb->val.string, requests->requestvb->val_len );
	}
	break;



	

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
        //    if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
         //       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
         //   }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
         //   if (/* XXX: error? */) {
         //       /* try _really_really_ hard to never get to this point */
         //       netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
         //   }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_informationinfo\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }


    return SNMP_ERR_NOERROR;
}
