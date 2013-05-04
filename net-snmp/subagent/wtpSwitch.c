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
* wtpSwitch.c
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
#include "wtpSwitch.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "autelanWtpGroup.h"
#include "ws_dcli_ac.h"
/** Initializes the wtpSwitch module */
#define    	ROGUE 				"1.12.1"
#define		STATICS   			"1.12.2"
#define		FLOOD				"1.12.4"
#define		SPOOF				"1.12.5"
#define		WEAKIV				"1.12.6"
#define		CLEANATTACKHIS		"1.12.7"
#define		CLEANATTACKSTA		"1.12.8"

static int wtpFloodAttackDetectSwitch = 0;
static int wtpSpoofAttackDetectSwitch = 0;
static int wtpWeakIVAttackDetectSwitch = 0;
static long update_time_show_ap_wids_set_cmd_func = 0;
static void update_data_for_show_ap_wids_set_cmd_func();

void
init_wtpSwitch(void)
{
	static oid wtpRogueSwtich_oid[128] = {0};
	static oid wtpStatisticsSwitch_oid[128] = {0};
	static oid wtpFloodAttackDetectSwitch_oid[128] = {0};
	static oid wtpSpoofAttackDetectSwitch_oid[128] = {0};
	static oid wtpWeakIVAttackDetectSwitch_oid[128] = {0};
	static oid wtpCleanAttackHistorySwitch_oid[128] = {0};
	static oid wtpCleanAttackStatisticalSwitch_oid[128] = {0};
	size_t public_oid_len   = 0;
	mad_dev_oid(wtpRogueSwtich_oid,ROGUE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wtpStatisticsSwitch_oid,STATICS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wtpFloodAttackDetectSwitch_oid,FLOOD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wtpSpoofAttackDetectSwitch_oid,SPOOF,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wtpWeakIVAttackDetectSwitch_oid,WEAKIV,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wtpCleanAttackHistorySwitch_oid,CLEANATTACKHIS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(wtpCleanAttackStatisticalSwitch_oid,CLEANATTACKSTA,&public_oid_len,enterprise_pvivate_oid);
  DEBUGMSGTL(("wtpSwitch", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpRogueSwtich", handle_wtpRogueSwtich,
                               wtpRogueSwtich_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpStatisticsSwitch", handle_wtpStatisticsSwitch,
                               wtpStatisticsSwitch_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpFloodAttackDetectSwitch", handle_wtpFloodAttackDetectSwitch,
                               wtpFloodAttackDetectSwitch_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpSpoofAttackDetectSwitch", handle_wtpSpoofAttackDetectSwitch,
                               wtpSpoofAttackDetectSwitch_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpWeakIVAttackDetectSwitch", handle_wtpWeakIVAttackDetectSwitch,
                               wtpWeakIVAttackDetectSwitch_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpCleanAttackHistorySwitch", handle_wtpCleanAttackHistorySwitch,
                               wtpCleanAttackHistorySwitch_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("wtpCleanAttackStatisticalSwitch", handle_wtpCleanAttackStatisticalSwitch,
                               wtpCleanAttackStatisticalSwitch_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
}

int
handle_wtpRogueSwtich(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_wtpRogueSwtich\n");
	instance_parameter *paraHead = NULL;
	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
		return SNMP_ERR_GENERR;
        
    switch(reqinfo->mode) {
    case MODE_GET:
	{
	    
		int state = 0;
		int ret = 0;
		DCLI_AC_API_GROUP_FIVE *resource_mg = NULL;

		snmp_log(LOG_DEBUG, "enter show_ap_rrm_config_func\n");
		ret = show_ap_rrm_config_func(paraHead->parameter, paraHead->connection,&resource_mg);
		snmp_log(LOG_DEBUG, "exit show_ap_rrm_config_func,ret=%d\n", ret);
		
		if(ret == 1)
		{			
			state = (resource_mg->rrm_state==1)?1:2;
		}
		
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
								(u_char *)&state,
								sizeof(long));

		if(ret == 1)
		{
			Free_ap_rrm_config(resource_mg);
		}
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
         //   if (/* XXX if malloc, or whatever, failed: */) {
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
        //    }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

	case MODE_SET_ACTION:
	{
		int state = 0;
		int ret = 0;
		if(*requests->requestvb->val.integer==1)
		{
			state  =1;
		}
		else if(*requests->requestvb->val.integer==2)
		{
			state = 0;
		}
		
		snmp_log(LOG_DEBUG, "enter set_radio_resource_management\n");
		ret = set_radio_resource_management(paraHead->parameter, paraHead->connection,state);
		snmp_log(LOG_DEBUG, "exit set_radio_resource_management,ret=%d\n", ret);
		
		if(ret != 1)
		{	
			netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
		}
		
		
	}
	break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
           // if (/* XXX: error? */) {
               // /* try _really_really_ hard to never get to this point */
                //netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
           // }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
           // if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
              //  netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            //}
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpRogueSwtich\n", reqinfo->mode );
            free_instance_parameter_list(&paraHead);
            return SNMP_ERR_GENERR;
    }
	free_instance_parameter_list(&paraHead);
	snmp_log(LOG_DEBUG, "exit handle_wtpRogueSwtich\n");
	return SNMP_ERR_NOERROR;
}
int
handle_wtpStatisticsSwitch(netsnmp_mib_handler *handler,
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
		int state = 1;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&state,
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
            //    netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
         //   }
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
    	{
    	    instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                break;
            }
    		int state = 0;
    		int ret = -1;
    		if(*requests->requestvb->val.integer==1)
    		{
    			state = 1;
    		}
    		else if(*requests->requestvb->val.integer==2)
    		{ 
    			state = 0;
    		}
    		ret = set_ap_statistics(paraHead->parameter, paraHead->connection,state);
    		if(ret != 1)
    		{
    			netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    		}
    		free_instance_parameter_list(&paraHead);
        }
          break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
          //  if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
          //  }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpStatisticsSwitch\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


int
handle_wtpFloodAttackDetectSwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_wtpFloodAttackDetectSwitch\n");

    instance_parameter *paraHead = NULL;
    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
        return SNMP_ERR_GENERR;
    
    switch(reqinfo->mode) {

        case MODE_GET:
	{	
		#if 0
		int state = 0;
		DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;
		int ret = 0;
		ret  = show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,&WTPINFO);
		if(ret == 1)
		{
			state = (WTPINFO->wids.flooding == 1)?1:2;
		}
		#endif

		update_data_for_show_ap_wids_set_cmd_func(paraHead);
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
								(u_char *)&wtpFloodAttackDetectSwitch,
								sizeof(long));
		#if 0
		if(ret == 1)
		{
			free_show_ap_wids_set_cmd(WTPINFO);
		}
		#endif
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
            //    netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
         //   }
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
	{
		int ret = 0;
		ret = set_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,"flooding","","",(*requests->requestvb->val.integer == 1)?"enable":"disable");
		if(1 == ret)
		{
			wtpFloodAttackDetectSwitch = (*requests->requestvb->val.integer == 1)?1:2;
		}
		else
		{
			netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		}
		
	}
	break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
          //  if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
          //  }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpStatisticsSwitch\n", reqinfo->mode );
            free_instance_parameter_list(&paraHead);
            return SNMP_ERR_GENERR;
    }
    free_instance_parameter_list(&paraHead);
	snmp_log(LOG_DEBUG, "exit handle_wtpFloodAttackDetectSwitch\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wtpSpoofAttackDetectSwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_wtpSpoofAttackDetectSwitch\n");

    instance_parameter *paraHead = NULL;
    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
        return SNMP_ERR_GENERR;
    
    switch(reqinfo->mode) {

        case MODE_GET:
	{	
		#if 0
		int state = 0;
		DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;
		int ret = 0;

		snmp_log(LOG_DEBUG, "enter show_ap_wids_set_cmd_func\n");
		ret  = show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,&WTPINFO);
		snmp_log(LOG_DEBUG, "exit show_ap_wids_set_cmd_func,ret=%d\n", ret);
		
		if(ret == 1)
		{
			state = (WTPINFO->wids.sproof == 1)?1:2;
		}
		#endif

		update_data_for_show_ap_wids_set_cmd_func(paraHead);
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 (u_char *)&wtpSpoofAttackDetectSwitch,
                                 sizeof(long));

		#if 0
		if(ret == 1)
		{
			free_show_ap_wids_set_cmd(WTPINFO);
		}
		#endif
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
            //    netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
         //   }
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
	{
		int ret = 0;

		snmp_log(LOG_DEBUG, "enter set_ap_wids_set_cmd_func\n");
		ret = set_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,"spoofing","","",(*requests->requestvb->val.integer == 1)?"enable":"disable");
		snmp_log(LOG_DEBUG, "exit set_ap_wids_set_cmd_func,ret=%d\n", ret);
		
		if(1 == ret)
		{
			wtpSpoofAttackDetectSwitch = (*requests->requestvb->val.integer == 1)?1:2;
		}
		else
		{
			netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		}
		
	}
          break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
          //  if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
          //  }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpStatisticsSwitch\n", reqinfo->mode );
            free_instance_parameter_list(&paraHead);
            return SNMP_ERR_GENERR;
    }
    free_instance_parameter_list(&paraHead);
	snmp_log(LOG_DEBUG, "exit handle_wtpSpoofAttackDetectSwitch\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wtpWeakIVAttackDetectSwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_wtpWeakIVAttackDetectSwitch\n");

    instance_parameter *paraHead = NULL;
    if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
        return SNMP_ERR_GENERR;
    
    switch(reqinfo->mode) {

        case MODE_GET:
	{	
		#if 0
		int state = 0;
		DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;   
		int ret = 0;

		snmp_log(LOG_DEBUG, "enter show_ap_wids_set_cmd_func\n");
		ret  = show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,&WTPINFO);
		snmp_log(LOG_DEBUG, "exit show_ap_wids_set_cmd_func,ret=%d\n", ret);
		
		if(ret == 1)
		{
			state = (WTPINFO->wids.weakiv == 1)?1:2;
		}
		#endif

		update_data_for_show_ap_wids_set_cmd_func(paraHead);
        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                 (u_char *)&wtpWeakIVAttackDetectSwitch,
                                 sizeof(long));

		#if 0
		if(ret == 1)
		{
			free_show_ap_wids_set_cmd(WTPINFO);
		}
		#endif
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
            //    netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
         //   }
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
	{
		int ret = 0;

		snmp_log(LOG_DEBUG, "enter set_ap_wids_set_cmd_func\n");
		ret = set_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,"weakiv","","",(*requests->requestvb->val.integer == 1)?"enable":"disable");
		snmp_log(LOG_DEBUG, "exit set_ap_wids_set_cmd_func,ret=%d\n", ret);
		
		if(1 == ret)
		{
			wtpWeakIVAttackDetectSwitch = (*requests->requestvb->val.integer == 1)?1:2;
		}
		else
		{
			netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		}
		
	}
          break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
          //  if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
          //  }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpStatisticsSwitch\n", reqinfo->mode );
            free_instance_parameter_list(&paraHead);
            return SNMP_ERR_GENERR;
    }
    free_instance_parameter_list(&paraHead);
	snmp_log(LOG_DEBUG, "exit handle_wtpWeakIVAttackDetectSwitch\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wtpCleanAttackHistorySwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_wtpCleanAttackHistorySwitch\n");
    
    switch(reqinfo->mode) {

        case MODE_GET:
	{	
		int state = 1;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&state,
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
            //    netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
         //   }
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
	{
        instance_parameter *paraHead = NULL;
        if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
        {    
            netsnmp_set_request_error(reqinfo,requests,SNMP_ERR_COMMITFAILED);
            break;
        }    
		int ret =0;
		if(*requests->requestvb->val.integer == 1)
		{
			snmp_log(LOG_DEBUG, "enter clear_wids_device_list_cmd_func\n");
			ret = clear_wids_device_list_cmd_func(paraHead->parameter, paraHead->connection);
			snmp_log(LOG_DEBUG, "exit clear_wids_device_list_cmd_func,ret=%d\n", ret);
			
			if(ret != 1)
			{
				netsnmp_set_request_error(reqinfo,requests,SNMP_ERR_COMMITFAILED);
			}
		}
		free_instance_parameter_list(&paraHead);
	}
	break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
          //  if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
          //  }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpStatisticsSwitch\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
	snmp_log(LOG_DEBUG, "exit handle_wtpCleanAttackHistorySwitch\n");
    return SNMP_ERR_NOERROR;
}
int
handle_wtpCleanAttackStatisticalSwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_wtpCleanAttackStatisticalSwitch\n");

    switch(reqinfo->mode) {

        case MODE_GET:
	{	
		int state = 1;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&state,
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
            //    netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
         //   }
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
	{
		instance_parameter *paraHead = NULL;
        if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
        {    
            netsnmp_set_request_error(reqinfo,requests,SNMP_ERR_COMMITFAILED);
            break;
        } 
		
		int ret =0;
		if(*requests->requestvb->val.integer == 1)
		{
			snmp_log(LOG_DEBUG, "enter clear_wids_statistics_list_cmd_func\n");
			ret = clear_wids_statistics_list_cmd_func(paraHead->parameter, paraHead->connection);
			snmp_log(LOG_DEBUG, "exit clear_wids_statistics_list_cmd_func,ret=%d\n", ret);
			
			if(ret != 1)
			{
				netsnmp_set_request_error(reqinfo,requests,SNMP_ERR_COMMITFAILED);
			}
		}
		free_instance_parameter_list(&paraHead);
	}
	break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
          //  if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
          //      netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
          //  }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_wtpStatisticsSwitch\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
	snmp_log(LOG_DEBUG, "exit handle_wtpCleanAttackStatisticalSwitch\n");
    return SNMP_ERR_NOERROR;
}

static void update_data_for_show_ap_wids_set_cmd_func(instance_parameter *paraHead)
{
	struct sysinfo info;
	
	if(0 != update_time_show_ap_wids_set_cmd_func)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_show_ap_wids_set_cmd_func < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_ap_wids_set_cmd_func\n");
	
	/*update cache data*/
	DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;
	int ret = 0;

	wtpFloodAttackDetectSwitch = 0;
	wtpSpoofAttackDetectSwitch = 0;
	wtpWeakIVAttackDetectSwitch = 0;
	
	snmp_log(LOG_DEBUG, "enter show_ap_wids_set_cmd_func\n");
	ret = show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection, &WTPINFO);
	snmp_log(LOG_DEBUG, "exit show_ap_wids_set_cmd_func,ret=%d\n",ret);
	if((ret == 1)&&(WTPINFO))
	{
		wtpFloodAttackDetectSwitch = (WTPINFO->wids.flooding == 1)?1:2;
		wtpSpoofAttackDetectSwitch = (WTPINFO->wids.sproof == 1)?1:2;
		wtpWeakIVAttackDetectSwitch = (WTPINFO->wids.weakiv == 1)?1:2;
	}
	if(ret == 1)
	{
		free_show_ap_wids_set_cmd(WTPINFO);
	}
	
	sysinfo(&info); 		
	update_time_show_ap_wids_set_cmd_func = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_ap_wids_set_cmd_func\n");
}

