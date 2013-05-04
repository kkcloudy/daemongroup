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
* dot11AlarmConfiguration.c
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
#include "dot11AlarmConfiguration.h"
#include "autelanWtpGroup.h"
#include "wcpss/wid/WID.h"
#include "ws_dcli_ac.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ac_sample_def.h"
#include "ac_sample_err.h"
#include "ws_init_dbus.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"


/** Initializes the dot11AlarmConfiguration module */
#define     APINTERFNUMTHRESHHD 			"2.18.1.1"
#define		STAINTERFNUMTHRESHHD   			"2.18.1.2"
#define		TRAPRESENDWAITINGTIME			"2.18.1.3"
#define		TRAPRESENDWAITINGTIMES			"2.18.1.4"

void
init_dot11AlarmConfiguration(void)
{
    static oid APInterfNumThreshhd_oid[128] = {0};
    static oid StaInterfNumThreshhd_oid[128] = {0};
    static oid TrapResendWaitingTime_oid[128] = {0};
	static oid TrapResendWaitingTimes_oid[128] = {0};
	size_t public_oid_len   = 0;
	mad_dev_oid(APInterfNumThreshhd_oid,APINTERFNUMTHRESHHD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(StaInterfNumThreshhd_oid,STAINTERFNUMTHRESHHD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(TrapResendWaitingTime_oid,TRAPRESENDWAITINGTIME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(TrapResendWaitingTimes_oid,TRAPRESENDWAITINGTIMES,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11AlarmConfiguration", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("APInterfNumThreshhd", handle_APInterfNumThreshhd,
                               APInterfNumThreshhd_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("StaInterfNumThreshhd", handle_StaInterfNumThreshhd,
                               StaInterfNumThreshhd_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("TrapResendWaitingTime", handle_TrapResendWaitingTime,
                               TrapResendWaitingTime_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("TrapResendWaitingTimes", handle_TrapResendWaitingTimes,
                               TrapResendWaitingTimes_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
}

int
handle_APInterfNumThreshhd(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_APInterfNumThreshhd\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int APInterfNumThreshhd = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
                int ret = 0;
			    DCLI_AC_API_GROUP_FIVE *rogue_trap = NULL;
    			snmp_log(LOG_DEBUG, "enter show_rogue_ap_trap_threshold_func\n");
    			ret=show_rogue_ap_trap_threshold_func(paraHead->parameter, paraHead->connection,&rogue_trap);
    			snmp_log(LOG_DEBUG, "exit show_rogue_ap_trap_threshold_func,ret=%d\n", ret);
    			
    			if(ret == 1) {
    				APInterfNumThreshhd = rogue_trap->num;
    				Free_rogue_ap_trap_threshold(rogue_trap);
    			}
    		}	
			free_instance_parameter_list(&paraHead);
			
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&APInterfNumThreshhd,
                                     sizeof(APInterfNumThreshhd));
			
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
			#if 0
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
			char APInterfNumThreshhd[10] = { 0 };
			if((*requests->requestvb->val.integer>0)&&(*requests->requestvb->val.integer<201))
			{
				memset(APInterfNumThreshhd,0,10);
				snprintf(APInterfNumThreshhd,sizeof(APInterfNumThreshhd)-1,"%d",*requests->requestvb->val.integer);
                instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                
                    int ret = 0;
    				snmp_log(LOG_DEBUG, "enter set_rogue_ap_trap_threshold_func, slot %d, local_id = %d, instanec_id = %d\n", 
    				                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret=set_rogue_ap_trap_threshold_func(paraNode->parameter, paraNode->connection,APInterfNumThreshhd);
    				snmp_log(LOG_DEBUG, "exit set_rogue_ap_trap_threshold_func,ret=%d\n", ret);
    				
    				if(ret != 1)
        			{
                        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                    }
    		    }	
    		    free_instance_parameter_list(&paraHead);
			} 
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
			#if 0
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
			#endif
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
			#if 0
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
			#endif
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_APInterfNumThreshhd\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_APInterfNumThreshhd\n");
    return SNMP_ERR_NOERROR;
}
int
handle_StaInterfNumThreshhd(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_StaInterfNumThreshhd\n");
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int StaInterfNumThreshhd = 0;
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
    			int ret = 0;
    			WID_TRAP_THRESHOLD *info;

    			snmp_log(LOG_DEBUG, "enter show_ap_trap_rogue_ap_ter_cpu_mem_threshold_cmd\n");
    	//		ret=show_ap_trap_rogue_ap_ter_cpu_mem_threshold_cmd(paraHead->parameter, paraHead->connection,0,&info);
    			snmp_log(LOG_DEBUG, "exit show_ap_trap_rogue_ap_ter_cpu_mem_threshold_cmd,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				StaInterfNumThreshhd = info->rogue_termi_threshold;
    				Free_show_ap_trap_threshold(info);
    			}
    		}	
			free_instance_parameter_list(&paraHead);
			
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&StaInterfNumThreshhd,
                                     sizeof(StaInterfNumThreshhd));
			
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
			#if 0
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
		    char StaInterfNumThreshhd[10] = { 0 };
			snprintf(StaInterfNumThreshhd,sizeof(StaInterfNumThreshhd)-1,"%d",*requests->requestvb->val.integer);
			
            instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                int ret = 0;
                snmp_log(LOG_DEBUG, "enter set_wtp_trap_threshold_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
                                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    //			ret=set_wtp_trap_threshold_cmd(paraNode->parameter, paraNode->connection,0,"rogueterminal",StaInterfNumThreshhd);	
    			snmp_log(LOG_DEBUG, "exit set_wtp_trap_threshold_cmd,ret=%d\n", ret);
    			
    			if(ret != 1)
    			{
                    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
                }
            }
            free_instance_parameter_list(&paraHead);
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
			#if 0
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
			#endif
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
			#if 0
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
			#endif
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_StaInterfNumThreshhd\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    
	snmp_log(LOG_DEBUG, "exit handle_StaInterfNumThreshhd\n");
    return SNMP_ERR_NOERROR;
}
int
handle_TrapResendWaitingTime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_TrapResendWaitingTime\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int TrapResendWaitingTime = 0;
			TRAPParameter *parameter_array = NULL;
			unsigned int parameter_num = 0;
			int i = 0;

			unsigned int ret = AC_MANAGE_DBUS_ERROR;
			snmp_log(LOG_DEBUG, "enter ac_manage_show_trap_parameter\n");
			ret = ac_manage_show_trap_parameter(ccgi_dbus_connection, &parameter_array, &parameter_num);
			snmp_log(LOG_DEBUG, "exit ac_manage_show_trap_parameter,ret=%d\n", ret);
			if(AC_MANAGE_SUCCESS == ret) 
			{		 
				if(parameter_num) 
				{
		            for(i = 0; i < parameter_num; i++) 
					{
						if(strcmp(parameter_array[i].paraStr,RESEND_INTERVAL)==0)
						{
							TrapResendWaitingTime = parameter_array[i].data;
							break;
						}
		            }
		        }
		
		        MANAGE_FREE(parameter_array);
			}	
			
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&TrapResendWaitingTime,
                                     sizeof(TrapResendWaitingTime));
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
			#if 0
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
			int ret2 = AS_RTN_DBUS_ERR;
			
			if((*requests->requestvb->val.integer >= 0)&&(*requests->requestvb->val.integer<=65535))
			{
				instance_parameter *slotConnect_head = NULL, *slotConnect_node = NULL;	
			    unsigned int inter_data = *requests->requestvb->val.integer;
				int ret = AC_MANAGE_DBUS_ERROR;

				list_instance_parameter(&slotConnect_head, SNMPD_SLOT_CONNECT);
				for(slotConnect_node = slotConnect_head; NULL != slotConnect_node; slotConnect_node = slotConnect_node->next)
				{
					ret = ac_manage_config_trap_parameter(slotConnect_node->connection, RESEND_INTERVAL, inter_data);
					if(AC_MANAGE_SUCCESS != ret) 
					{
						netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
					}				 
				}
				free_instance_parameter_list(&slotConnect_head);
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
			}
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
			#if 0
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
			#endif
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
			#if 0
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
			#endif
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_TrapResendWaitingTime\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_TrapResendWaitingTime\n");
    return SNMP_ERR_NOERROR;
}

int
handle_TrapResendWaitingTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_TrapResendWaitingTimes\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int TrapResendWaitingTimes = 0;
			TRAPParameter *parameter_array = NULL;
			unsigned int parameter_num = 0;
			int i = 0;

			unsigned int ret = AC_MANAGE_DBUS_ERROR;
			snmp_log(LOG_DEBUG, "enter ac_manage_show_trap_parameter\n");
			ret = ac_manage_show_trap_parameter(ccgi_dbus_connection, &parameter_array, &parameter_num);
			snmp_log(LOG_DEBUG, "exit ac_manage_show_trap_parameter,ret=%d\n", ret);
			if(AC_MANAGE_SUCCESS == ret) 
			{		 
				if(parameter_num) 
				{
		            for(i = 0; i < parameter_num; i++) 
					{
						if(strcmp(parameter_array[i].paraStr,RESEND_TIMES)==0)
						{
							TrapResendWaitingTimes = parameter_array[i].data;
							break;
						}
		            }
		        }
		
		        MANAGE_FREE(parameter_array);
			}	
			
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&TrapResendWaitingTimes,
                                     sizeof(TrapResendWaitingTimes));
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
			#if 0
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
			int ret2 = AS_RTN_DBUS_ERR;
			
			if((*requests->requestvb->val.integer >= 0)&&(*requests->requestvb->val.integer<=50))
			{
				instance_parameter *slotConnect_head = NULL, *slotConnect_node = NULL;	
			    unsigned int times_data = *requests->requestvb->val.integer;
				int ret = AC_MANAGE_DBUS_ERROR;

				list_instance_parameter(&slotConnect_head, SNMPD_SLOT_CONNECT);
				for(slotConnect_node = slotConnect_head; NULL != slotConnect_node; slotConnect_node = slotConnect_node->next)
				{
					ret = ac_manage_config_trap_parameter(slotConnect_node->connection, RESEND_TIMES, times_data);
					if(AC_MANAGE_SUCCESS != ret) 
					{
						netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
					}				 
				}
				free_instance_parameter_list(&slotConnect_head);
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
			}
        }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
			#if 0
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
			#endif
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
			#if 0
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
			#endif
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_TrapResendWaitingTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_TrapResendWaitingTimes\n");
    return SNMP_ERR_NOERROR;
}

