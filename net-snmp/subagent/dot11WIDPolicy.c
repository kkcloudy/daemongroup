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
* dot11WIDPolicy.c
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
#include "dot11WIDPolicy.h"
#include "autelanWtpGroup.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_ac.h"
#include "ws_dbus_list_interface.h"

/** Initializes the dot11WIDPolicy module */
#define	WIDSFLOODINTERVAL				"2.11.2.1"
#define	WIDSBLACKLISTTHRESHOLD			"2.11.2.2"
#define	WIDSBLACKLISTDURATION			"2.11.2.3"
#define	WIDSFLOODDETECTONOFF			"2.11.2.4"
#define	SSIDFILTERSWITCH				"2.11.2.5"
#define	BSSIDFILTERSWITCH				"2.11.2.6"
#define	ROGUEAPCOUNTERMEASURESWITCH		"2.11.2.7"
#define	ROGUEAPCOUNTERMEASUREMODE		"2.11.2.8"

static int widsFloodInterval = 0;
static int widsBlackListThreshold = 0;
static unsigned int widsBlackListDuration = 0;
static int widsFloodDetectOnOff = 0;
static long update_time_show_ap_wids_set_cmd_func = 0;
static void update_data_for_show_ap_wids_set_cmd_func();


void
init_dot11WIDPolicy(void)
{
    static oid widsFloodInterval_oid[128] = {0};
    static oid widsBlackListThreshold_oid[128] = {0};
    static oid widsBlackListDuration_oid[128] = {0};
    static oid widsFloodDetectOnOff_oid[128] = {0};
	static oid SSIDFilterSwitch_oid[128] = {0};
	static oid BSSIDFilterSwitch_oid[128] = {0};
	static oid RogueAPCountermeasureSwitch_oid[128] = {0};
	static oid RogueAPCountermeasureMode_oid[128] = {0};
	size_t public_oid_len   = 0;
	mad_dev_oid(widsFloodInterval_oid,WIDSFLOODINTERVAL,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(widsBlackListThreshold_oid,WIDSBLACKLISTTHRESHOLD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(widsBlackListDuration_oid,WIDSBLACKLISTDURATION,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(widsFloodDetectOnOff_oid,WIDSFLOODDETECTONOFF,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(SSIDFilterSwitch_oid,SSIDFILTERSWITCH,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(BSSIDFilterSwitch_oid,BSSIDFILTERSWITCH,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(RogueAPCountermeasureSwitch_oid,ROGUEAPCOUNTERMEASURESWITCH,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(RogueAPCountermeasureMode_oid,ROGUEAPCOUNTERMEASUREMODE,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11WIDPolicy", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("widsFloodInterval", handle_widsFloodInterval,
                               widsFloodInterval_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("widsBlackListThreshold", handle_widsBlackListThreshold,
                               widsBlackListThreshold_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("widsBlackListDuration", handle_widsBlackListDuration,
                               widsBlackListDuration_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("widsFloodDetectOnOff", handle_widsFloodDetectOnOff,
                               widsFloodDetectOnOff_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SSIDFilterSwitch", handle_SSIDFilterSwitch,
                               SSIDFilterSwitch_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("BSSIDFilterSwitch", handle_BSSIDFilterSwitch,
                               BSSIDFilterSwitch_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("RogueAPCountermeasureSwitch", handle_RogueAPCountermeasureSwitch,
                               RogueAPCountermeasureSwitch_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("RogueAPCountermeasureMode", handle_RogueAPCountermeasureMode,
                               RogueAPCountermeasureMode_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
}

int
handle_widsFloodInterval(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_widsFloodInterval\n");
    
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
		    int widsFloodInterval = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
    			int ret = 0;
    			DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;  

    			snmp_log(LOG_DEBUG, "enter show_ap_wids_set_cmd_func\n");
    			ret=show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,&WTPINFO);
    			snmp_log(LOG_DEBUG, "exit show_ap_wids_set_cmd_func,ret=%d\n", ret);
    			
    			if(ret==1)
    			{
    				widsFloodInterval = WTPINFO->interval;	
    				free_show_ap_wids_set_cmd(WTPINFO);	
    			}
    			else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(paraHead->parameter.slot_id);
                }
    	    }		
    		free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ap_wids_set_cmd_func();
            snmp_set_var_typed_value(requests->requestvb, ASN_UNSIGNED,
                                     (u_char *)&widsFloodInterval,
                                     sizeof(widsFloodInterval));
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
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{	
            instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
            {
    			int ret = 0;
    			char para[50] = { 0 };
				memset(para,0,50);
				snprintf(para,sizeof(para)-1,"%d",*requests->requestvb->val.integer);

    			snmp_log(LOG_DEBUG, "enter set_wtp_wids_interval_cmd_func\n");
    			ret = set_wtp_wids_interval_cmd_func(paraNode->parameter, paraNode->connection, para);
    			snmp_log(LOG_DEBUG, "exit set_wtp_wids_interval_cmd_func,ret=%d\n", ret);
    			
				if(1 == ret)
				{
					widsFloodInterval = *requests->requestvb->val.integer;
				}
				else
    			{
    			    if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(paraNode->parameter.slot_id);
            	    }
    				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    			}
    		}	
    		free_instance_parameter_list(&paraHead);
        }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_widsFloodInterval\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_widsFloodInterval\n");
    return SNMP_ERR_NOERROR;
}
int
handle_widsBlackListThreshold(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_widsBlackListThreshold\n");
    
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int widsBlackListThreshold = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
                int ret = 0;
    			DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;  

    			snmp_log(LOG_DEBUG, "enter show_ap_wids_set_cmd_func\n");
    			ret=show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,&WTPINFO);
    			snmp_log(LOG_DEBUG, "exit show_ap_wids_set_cmd_func,ret=%d\n", ret);
    			
    			if(ret==1)
    			{
    				widsBlackListThreshold = WTPINFO->otherthreshold;	
    				free_show_ap_wids_set_cmd(WTPINFO);	
    			}
    		}	
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ap_wids_set_cmd_func();
            snmp_set_var_typed_value(requests->requestvb, ASN_UNSIGNED,
                                     (u_char *)&widsBlackListThreshold,
                                     sizeof(widsBlackListThreshold));


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
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
        {
            instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
            {
    			int ret = 0;
    			char para[50] = { 0 };
				memset(para,0,50);
				snprintf(para,sizeof(para)-1,"%d",*requests->requestvb->val.integer);

    			snmp_log(LOG_DEBUG, "enter set_wtp_wids_threshold_cmd_func\n");
    			ret = set_wtp_wids_threshold_cmd_func(paraNode->parameter, paraNode->connection,"other",para);
    			snmp_log(LOG_DEBUG, "exit set_wtp_wids_threshold_cmd_func,ret=%d\n", ret);
    			
				if(1 == ret)
				{
					widsBlackListThreshold = *requests->requestvb->val.integer;
				}
				else
    			{
    			    if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(paraNode->parameter.slot_id);
            	    }
    				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    			}
    		}	
			
			free_instance_parameter_list(&paraHead);
        }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_widsBlackListThreshold\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_widsBlackListThreshold\n");
    return SNMP_ERR_NOERROR;
}
int
handle_widsBlackListDuration(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_widsBlackListDuration\n");
    
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int widsBlackListDuration = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
                int ret = 0;
    			DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL; 

    			snmp_log(LOG_DEBUG, "enter show_ap_wids_set_cmd_func\n");
    			ret=show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,&WTPINFO);
    			snmp_log(LOG_DEBUG, "exit show_ap_wids_set_cmd_func,ret=%d\n", ret);
    			
    			if(ret==1)
    			{
    				widsBlackListDuration = WTPINFO->lasttime;	
    				free_show_ap_wids_set_cmd(WTPINFO);		
    			}
    		}	
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ap_wids_set_cmd_func();
            snmp_set_var_typed_value(requests->requestvb, ASN_UNSIGNED,
                                     (u_char *)&widsBlackListDuration,
                                     sizeof(widsBlackListDuration));
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
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{	
            instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
            {
    			int ret = 0;
    			char para[50] = { 0 };
				memset(para,0,50);
				snprintf(para,sizeof(para)-1,"%d",*requests->requestvb->val.integer);

    			snmp_log(LOG_DEBUG, "enter set_wtp_wids_lasttime_cmd_func\n");
    			ret = set_wtp_wids_lasttime_cmd_func(paraNode->parameter, paraNode->connection,para);
    			snmp_log(LOG_DEBUG, "exit set_wtp_wids_lasttime_cmd_func,ret=%d\n", ret);
    			
				if(1 == ret)
				{
					widsBlackListDuration = *requests->requestvb->val.integer;
				}
				else
    			{
    			    if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(paraNode->parameter.slot_id);
            	    }
    				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    			}
    	    }	
			free_instance_parameter_list(&paraHead);
        }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_widsBlackListDuration\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_widsBlackListDuration\n");
    return SNMP_ERR_NOERROR;
}
int
handle_widsFloodDetectOnOff(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_widsFloodDetectOnOff\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int widsFloodDetectOnOff = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
			    int ret = 0;
    			DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;   
    			
    			snmp_log(LOG_DEBUG, "enter show_ap_wids_set_cmd_func\n");
    			ret=show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,&WTPINFO);
    			snmp_log(LOG_DEBUG, "exit show_ap_wids_set_cmd_func,ret=%d\n", ret);
    			
    			if(ret==1)
    			{
    				if(WTPINFO->wids.flooding==1)
    					widsFloodDetectOnOff = 1;
    				else
    					widsFloodDetectOnOff = 0;
    				free_show_ap_wids_set_cmd(WTPINFO);	
    			}
    		}				
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ap_wids_set_cmd_func();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&widsFloodDetectOnOff,
                                     sizeof(widsFloodDetectOnOff));
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
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{	
		    
            instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
            {
    			int ret = 0,flag = 1;
				char state[10] = { 0 };
    			memset(state,0,10);

    			if(*requests->requestvb->val.integer==0)
					strncpy(state,"disable",sizeof(state)-1);
    			else if(*requests->requestvb->val.integer==1)
					strncpy(state,"enable",sizeof(state)-1);
    			else
    			{
    				flag = 0;
    				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    			}
    			
    			if(flag==1)
    			{
    				snmp_log(LOG_DEBUG, "enter set_ap_wids_set_cmd_func\n");
    				ret=set_ap_wids_set_cmd_func(paraNode->parameter, paraNode->connection,"flooding","","",state);
    				snmp_log(LOG_DEBUG, "exit set_ap_wids_set_cmd_func,ret=%d\n", ret);

					if(1 == ret)
					{
						widsFloodDetectOnOff = *requests->requestvb->val.integer;
					}
					else
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    				}
    			}
    		}	
			free_instance_parameter_list(&paraHead);

        }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_widsFloodDetectOnOff\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_widsFloodDetectOnOff\n");
    return SNMP_ERR_NOERROR;
}
int
handle_SSIDFilterSwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_SSIDFilterSwitch\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int SSIDFilterOnOff = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
			    int ret = 0;
    			DCLI_AC_API_GROUP_FIVE *wirelessconfig = NULL;
    			wireless_config *head = NULL;

    			snmp_log(LOG_DEBUG, "enter show_wc_config\n");
    			ret = show_wc_config(paraHead->parameter, paraHead->connection, &wirelessconfig);
    			snmp_log(LOG_DEBUG, "exit show_wc_config,ret=%d\n", ret);
    			
    			if((ret == 1)&&(wirelessconfig->wireless_control != NULL))
    			{
    				head = wirelessconfig->wireless_control;
    				SSIDFilterOnOff = (head->essidfiltrflag==1)?1:0;
    			}
    			if(1 ==  ret) {
    			    Free_wc_config(wirelessconfig);
    			}
    		}	
			free_instance_parameter_list(&paraHead);
			
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&SSIDFilterOnOff,
                                     sizeof(SSIDFilterOnOff));
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
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{			
			int ret = 0;

			if(*requests->requestvb->val.integer==0)
			{
                instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
                {
    				snmp_log(LOG_DEBUG, "enter set_wid_essid_whitelist_cmd\n");
    				ret = set_wid_essid_whitelist_cmd(paraNode->parameter, paraNode->connection,"disable");
    				snmp_log(LOG_DEBUG, "exit set_wid_essid_whitelist_cmd,ret=%d\n", ret);
    				
    				if(ret != 1)
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    				}
    			}
    			free_instance_parameter_list(&paraHead);
			}
			else if(*requests->requestvb->val.integer==1)
			{
			    
                instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
                {
    				snmp_log(LOG_DEBUG, "enter set_wid_essid_whitelist_cmd\n");
    				ret = set_wid_essid_whitelist_cmd(paraNode->parameter, paraNode->connection,"enable");		
    				snmp_log(LOG_DEBUG, "exit set_wid_essid_whitelist_cmd,ret=%d\n", ret);
    				
    				if(ret != 1)
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    				}
    			}
    			free_instance_parameter_list(&paraHead);
			}
			else
			{	
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
        }
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
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_SSIDFilterSwitch\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_SSIDFilterSwitch\n");
    return SNMP_ERR_NOERROR;
}
int
handle_BSSIDFilterSwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_BSSIDFilterSwitch\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int BSSIDFilterOnOff = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
                int ret = 0;
    			DCLI_AC_API_GROUP_FIVE *wirelessconfig = NULL;
    			wireless_config *head = NULL;

    			snmp_log(LOG_DEBUG, "enter show_wc_config\n");
    			ret = show_wc_config(paraHead->parameter, paraHead->connection,&wirelessconfig);
    			snmp_log(LOG_DEBUG, "exit show_wc_config,ret=%d\n", ret);
    			
    			if((ret == 1)&&(wirelessconfig->wireless_control != NULL))
    			{
    				head = wirelessconfig->wireless_control;
    				BSSIDFilterOnOff = (head->macfiltrflag==1)?1:0;
    			}
			    if(ret == 1)
    			{
    				Free_wc_config(wirelessconfig);
    			}
			}
            free_instance_parameter_list(&paraHead);
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&BSSIDFilterOnOff,
                                     sizeof(BSSIDFilterOnOff));

			
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
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{			
			int ret = 0;

			if(*requests->requestvb->val.integer==0)
			{
                instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
                {
    				snmp_log(LOG_DEBUG, "enter set_wid_mac_whitelist_cmd\n");
    				ret = set_wid_mac_whitelist_cmd(paraNode->parameter, paraNode->connection,"disable");
    				snmp_log(LOG_DEBUG, "exit set_wid_mac_whitelist_cmd,ret=%d\n", ret);
    				
    				if(ret != 1)
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    				}
    			}	
                free_instance_parameter_list(&paraHead);
			}
			else if(*requests->requestvb->val.integer==1)
			{
                instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
                {
    				snmp_log(LOG_DEBUG, "enter set_wid_mac_whitelist_cmd\n");
    				ret = set_wid_mac_whitelist_cmd(paraNode->parameter, paraNode->connection,"enable");		
    				snmp_log(LOG_DEBUG, "exit set_wid_mac_whitelist_cmd,ret=%d\n", ret);
    				
    				if(ret != 1)
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    				}
    			}	
                free_instance_parameter_list(&paraHead);
			}
			else
			{	
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
        }
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
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_BSSIDFilterSwitch\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_BSSIDFilterSwitch\n");
    return SNMP_ERR_NOERROR;
}

int
handle_RogueAPCountermeasureSwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_RogueAPCountermeasureSwitch\n");
    
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int RogueAPCountermeasureSwitch = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
                int ret = 0;
    			DCLI_AC_API_GROUP_FIVE *resource_mg = NULL;

    			snmp_log(LOG_DEBUG, "enter show_ap_rrm_config_func\n");
    			ret = show_ap_rrm_config_func(paraHead->parameter, paraHead->connection,&resource_mg);
    			snmp_log(LOG_DEBUG, "exit show_ap_rrm_config_func,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				if(resource_mg->countermeasures_switch==1)
    					RogueAPCountermeasureSwitch = 1;
    				else
    					RogueAPCountermeasureSwitch = 2;

    				Free_ap_rrm_config(resource_mg);
    			}
    		}	
            free_instance_parameter_list(&paraHead);
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&RogueAPCountermeasureSwitch,
                                     sizeof(RogueAPCountermeasureSwitch));
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
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{			
			int ret = 0;
			int flag = 1;
			char state[10] = { 0 };

			if(*requests->requestvb->val.integer==1)
			{
				memset(state,0,10);
				strncpy(state,"enable",sizeof(state)-1);					
			}
			else if(*requests->requestvb->val.integer==2)
			{
				memset(state,0,10);
				strncpy(state,"disable",sizeof(state)-1);			
			}
			else
			{
				flag = 0;
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}	

			if(flag == 1)
			{
                instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
                {
    				snmp_log(LOG_DEBUG, "enter set_ap_countermeasures_cmd\n");
    				ret = set_ap_countermeasures_cmd(paraNode->parameter, paraNode->connection,state);
    				snmp_log(LOG_DEBUG, "exit set_ap_countermeasures_cmd,ret=%d\n", ret);
    				
    				if(ret != 1)
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    				}
    			}	
                free_instance_parameter_list(&paraHead);
			}
        }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_RogueAPCountermeasureSwitch\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_RogueAPCountermeasureSwitch\n");
    return SNMP_ERR_NOERROR;
}

int
handle_RogueAPCountermeasureMode(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_RogueAPCountermeasureMode\n");
    
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int RogueAPCountermeasureMode = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
                int ret = 0;
    			DCLI_AC_API_GROUP_FIVE *resource_mg = NULL;

    			snmp_log(LOG_DEBUG, "enter show_ap_rrm_config_func\n");
    			ret = show_ap_rrm_config_func(paraHead->parameter, paraHead->connection,&resource_mg);
    			snmp_log(LOG_DEBUG, "exit show_ap_rrm_config_func,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				if(resource_mg->countermeasures_mode==0)
    					RogueAPCountermeasureMode = 1;
    				else if(resource_mg->countermeasures_mode==1)
    					RogueAPCountermeasureMode = 2;
    				else
    					RogueAPCountermeasureMode = 3;

    				Free_ap_rrm_config(resource_mg);
    			}
    		}	
            free_instance_parameter_list(&paraHead);
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&RogueAPCountermeasureMode,
                                     sizeof(RogueAPCountermeasureMode));
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
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{			
			int ret = 0;
			int flag = 1;
			char mode[10] = { 0 };

			if(*requests->requestvb->val.integer==1)
			{
				memset(mode,0,10);
				strncpy(mode,"ap",sizeof(mode)-1);					
			}
			else if(*requests->requestvb->val.integer==2)
			{
				memset(mode,0,10);
				strncpy(mode,"adhoc",sizeof(mode)-1);			
			}
			else if(*requests->requestvb->val.integer==3)
			{
				memset(mode,0,10);
				strncpy(mode,"all",sizeof(mode)-1);			
			}
			else
			{
				flag = 0;
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}	

			if(flag == 1)
			{
                instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next)
                {
    				snmp_log(LOG_DEBUG, "enter set_ap_countermeasures_mode_cmd\n");
    				ret = set_ap_countermeasures_mode_cmd(paraNode->parameter, paraNode->connection,mode);
    				snmp_log(LOG_DEBUG, "exit set_ap_countermeasures_mode_cmd,ret=%d\n", ret);
    				
    				if(ret != 1)
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    				}
    			}	
                free_instance_parameter_list(&paraHead);
			}
        }
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_RogueAPCountermeasureMode\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_RogueAPCountermeasureMode\n");
    return SNMP_ERR_NOERROR;
}

static void update_data_for_show_ap_wids_set_cmd_func()
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
	widsFloodInterval = 0;
	widsBlackListThreshold = 0;
	widsBlackListDuration = 0;
	widsFloodDetectOnOff = 0;

	instance_parameter *paraHead = NULL;
    if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
    {
		int ret = 0;
		DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;  

		snmp_log(LOG_DEBUG, "enter show_ap_wids_set_cmd_func\n");
		ret=show_ap_wids_set_cmd_func(paraHead->parameter, paraHead->connection,&WTPINFO);
		snmp_log(LOG_DEBUG, "exit show_ap_wids_set_cmd_func,ret=%d\n", ret);
		
		if(ret==1)
		{
			{
				widsFloodInterval = WTPINFO->interval;
				widsBlackListThreshold = WTPINFO->otherthreshold;
				widsBlackListDuration = WTPINFO->lasttime;
				
				if(WTPINFO->wids.flooding==1)
					widsFloodDetectOnOff = 1;
				else
					widsFloodDetectOnOff = 0;
			}
			free_show_ap_wids_set_cmd(WTPINFO);	
		}
		else if(SNMPD_CONNECTION_ERROR == ret) {
            close_slot_dbus_connection(paraHead->parameter.slot_id);
        }
    }		
	free_instance_parameter_list(&paraHead);
	
	sysinfo(&info); 		
	update_time_show_ap_wids_set_cmd_func = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_ap_wids_set_cmd_func\n");
}

