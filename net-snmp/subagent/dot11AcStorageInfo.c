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
* dot11AcStorageInfo.c
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
#include "dot11AcStorageInfo.h"
#include "autelanWtpGroup.h"
#include "ws_usrinfo.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_ac.h"
#include "ws_dbus_list_interface.h"

#define  	MAXAP 							"2.3.2.1"
#define 	MAXSTA							"2.3.2.2"
#define 	LOADBALANCESTATEBASEONFLOW		"2.3.2.3"
#define 	LOADBALANCESTATEBASEONUSR		"2.3.2.4"
#define 	LOADBALANCETRIGERBASEFLOW		"2.3.2.5"
#define 	LOADBALANCETRIGERBASEUSR		"2.3.2.6"

static int acLoadBalanceStatusBaseOnFlow = 0;
static int acLoadBalanceStatusBaseOnUsr = 0;
static unsigned int acLoadBalanceTrigerBaseFlow = 0;
static unsigned int acLoadBalanceTrigerBaseUsr = 0;
static long update_time_show_ac_balance_cmd = 0;
static void update_data_for_show_ac_balance_cmd();

/** Initializes the dot11AcStorageInfo module */
void
init_dot11AcStorageInfo(void)
{
    static oid acMaxAPNumPermitted_oid[128] = {0};
    static oid acMaxStationNumPermitted_oid[128] = {0};
	static oid acLoadBalanceStatusBaseOnFlow_oid[128] = {0};
	static oid acLoadBalanceStatusBaseOnUsr_oid[128] = {0};
	static oid acLoadBalanceTrigerBaseFlow_oid[128] = {0};
	static oid acLoadBalanceTrigerBaseUsr_oid[128] = {0};
	size_t public_oid_len = 0;
	mad_dev_oid(acMaxAPNumPermitted_oid,MAXAP,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMaxStationNumPermitted_oid,MAXSTA,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLoadBalanceStatusBaseOnFlow_oid,LOADBALANCESTATEBASEONFLOW,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLoadBalanceStatusBaseOnUsr_oid,LOADBALANCESTATEBASEONUSR,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLoadBalanceTrigerBaseFlow_oid,LOADBALANCETRIGERBASEFLOW,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLoadBalanceTrigerBaseUsr_oid,LOADBALANCETRIGERBASEUSR,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11AcStorageInfo", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMaxAPNumPermitted", handle_acMaxAPNumPermitted,
                               acMaxAPNumPermitted_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMaxStationNumPermitted", handle_acMaxStationNumPermitted,
                               acMaxStationNumPermitted_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLoadBalanceStatusBaseOnFlow", handle_acLoadBalanceStatusBaseOnFlow,
                               acLoadBalanceStatusBaseOnFlow_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLoadBalanceStatusBaseOnUsr", handle_acLoadBalanceStatusBaseOnUsr,
                               acLoadBalanceStatusBaseOnUsr_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLoadBalanceTrigerBaseFlow", handle_acLoadBalanceTrigerBaseFlow,
                               acLoadBalanceTrigerBaseFlow_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLoadBalanceTrigerBaseUsr", handle_acLoadBalanceTrigerBaseUsr,
                               acLoadBalanceTrigerBaseUsr_oid,public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
}

int
handle_acMaxAPNumPermitted(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */    

	snmp_log(LOG_DEBUG, "enter handle_acMaxAPNumPermitted\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acMaxAPNumPermitted = 0;

			instance_parameter *paraHead = NULL, *paraNode = NULL;
		    list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);			
			int ret = 0;
			struct acAccessWtpCount *count_head = NULL, *count_node = NULL;
			struct acAccessBindLicCount *bind_lic_head = NULL;
			
    		for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) 
    		{
    			snmp_log(LOG_DEBUG, "enter show_ac_access_wtp_vendor_count\n");
    			ret = show_ac_access_wtp_vendor_count(paraNode->parameter, paraNode->connection, &count_head, &bind_lic_head);
				snmp_log(LOG_DEBUG, "exit show_ac_access_wtp_vendor_count,ret=%d\n", ret);
				if(1 == ret)
				{
					for(count_node = count_head; count_node; count_node = count_node->next)
					{
                        acMaxAPNumPermitted += count_node->license_count[1];
                    }
					Free_show_ac_access_wtp_vendor_count(count_head, bind_lic_head);
				}
    		}
			free_instance_parameter_list(&paraHead);
			
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acMaxAPNumPermitted,
                                     sizeof(acMaxAPNumPermitted));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMaxAPNumPermitted\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acMaxAPNumPermitted\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acMaxStationNumPermitted(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */    

	snmp_log(LOG_DEBUG, "enter handle_acMaxStationNumPermitted\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acMaxStationNumPermitted = 0;
            snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
            
            snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wc_config\n");
            messageHead = list_connection_call_dbus_method(show_wc_config, SHOW_ALL_WTP_TABLE_METHOD);
            snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wc_config,messageHead=%p\n", messageHead);
            
            if(messageHead)
            {
                for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
                {
                    int result = 0;
                    DCLI_AC_API_GROUP_FIVE *wirelessconfig = messageNode->message;
                    if((wirelessconfig)&&(wirelessconfig->wireless_control))
                    {
                        acMaxStationNumPermitted += wirelessconfig->wireless_control->sta_count;
                    }
                } 
                free_dbus_message_list(&messageHead, Free_wc_config);
            }
			
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acMaxStationNumPermitted,
                                     sizeof(acMaxStationNumPermitted));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMaxStationNumPermitted\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acMaxStationNumPermitted\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acLoadBalanceStatusBaseOnFlow(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acLoadBalanceStatusBaseOnFlow\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int acLoadBalanceStatusBaseOnFlow = 0;
			
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
    			int ret = 0;
    			DCLI_AC_API_GROUP_FIVE *balance = NULL;
    			
    			snmp_log(LOG_DEBUG, "enter show_ac_balance_cmd\n");
    			ret = show_ac_balance_cmd(paraHead->parameter, paraHead->connection,&balance);
    			snmp_log(LOG_DEBUG, "exit show_ac_balance_cmd,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				if(balance->state == 2)
    					acLoadBalanceStatusBaseOnFlow = 1;
    				else
    					acLoadBalanceStatusBaseOnFlow = 0;

    				Free_ac_balance(balance);	
    			}
    		}	
            free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_balance_cmd();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acLoadBalanceStatusBaseOnFlow,
                                     sizeof(acLoadBalanceStatusBaseOnFlow));	
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
			int ret1 = 0,ret2 = 0;
			if(*requests->requestvb->val.integer == 0)
			{
				instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
				    DCLI_AC_API_GROUP_FIVE *balance = NULL; 
                    snmp_log(LOG_DEBUG, "enter show_ac_balance_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
                                        paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret1 = show_ac_balance_cmd(paraNode->parameter, paraNode->connection,&balance);
    				snmp_log(LOG_DEBUG, "exit show_ac_balance_cmd,ret1=%d\n", ret1);
    				
    				if(ret1 == 1)
    				{
    					if(balance->state == 1)
    						;
    					else
    						ret2=ac_load_balance_cmd(paraNode->parameter, paraNode->connection,"disable");
    				}
    				if(ret1 == 1)
    				{
    					Free_ac_balance(balance);
    				}
    				else if(SNMPD_CONNECTION_ERROR == ret1) {
                        close_slot_dbus_connection(paraNode->parameter.slot_id);
            	    }
				}
				free_instance_parameter_list(&paraHead);
			}
			else if(*requests->requestvb->val.integer == 1)
			{
			    instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                    snmp_log(LOG_DEBUG, "enter ac_load_balance_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
                                        paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret2=ac_load_balance_cmd(paraNode->parameter, paraNode->connection,"flow");
    				snmp_log(LOG_DEBUG, "exit ac_load_balance_cmd,ret2=%d\n", ret2);
				}
				free_instance_parameter_list(&paraHead);
			}			
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}	

			if(1 == ret2)
			{
				acLoadBalanceStatusBaseOnFlow = *requests->requestvb->val.integer;
			}
			else
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acLoadBalanceStatusBaseOnFlow\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLoadBalanceStatusBaseOnFlow\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acLoadBalanceStatusBaseOnUsr(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acLoadBalanceStatusBaseOnUsr\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int acLoadBalanceStatusBaseOnUsr = 0;
			
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
    			int ret = 0;
    			DCLI_AC_API_GROUP_FIVE *balance = NULL;

    			snmp_log(LOG_DEBUG, "enter show_ac_balance_cmd\n");
    			ret = show_ac_balance_cmd(paraHead->parameter, paraHead->connection,&balance);
    			snmp_log(LOG_DEBUG, "exit show_ac_balance_cmd,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				if(balance->state == 1)
    					acLoadBalanceStatusBaseOnUsr = 1;
    				else
    					acLoadBalanceStatusBaseOnUsr = 0;

    			    Free_ac_balance(balance);
    			}	
    		}	
            free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_balance_cmd();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acLoadBalanceStatusBaseOnUsr,
                                     sizeof(acLoadBalanceStatusBaseOnUsr));	
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
			int ret1 = 0,ret2 = 0;
			if(*requests->requestvb->val.integer == 0)
			{
			    
				instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
				    DCLI_AC_API_GROUP_FIVE *balance = NULL; 
                    snmp_log(LOG_DEBUG, "enter show_ac_balance_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
                                        paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret1 = show_ac_balance_cmd(paraNode->parameter, paraNode->connection,&balance);
    				snmp_log(LOG_DEBUG, "exit show_ac_balance_cmd,ret1=%d\n", ret1);
    				
    				if(ret1 == 1)
    				{
    					if(balance->state == 2)
    						;
    					else
    						ret2=ac_load_balance_cmd(paraNode->parameter, paraNode->connection,"disable");
    				}
    				if(ret1 == 1)
    				{
    					Free_ac_balance(balance);
    				}
    				else if(SNMPD_CONNECTION_ERROR == ret1) {
                        close_slot_dbus_connection(paraNode->parameter.slot_id);
            	    }
				}
				free_instance_parameter_list(&paraHead);
			}
			else if(*requests->requestvb->val.integer == 1)
			{
			    instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                    snmp_log(LOG_DEBUG, "enter ac_load_balance_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
                                        paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret2=ac_load_balance_cmd(paraNode->parameter, paraNode->connection,"number");
    				snmp_log(LOG_DEBUG, "exit ac_load_balance_cmd,ret2=%d\n", ret2);
				}
				free_instance_parameter_list(&paraHead);
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}	

			if(1 == ret2)
			{
				acLoadBalanceStatusBaseOnUsr = *requests->requestvb->val.integer;
			}
			else
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acLoadBalanceStatusBaseOnUsr\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLoadBalanceStatusBaseOnUsr\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acLoadBalanceTrigerBaseFlow(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acLoadBalanceTrigerBaseFlow\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int acLoadBalanceTrigerBaseFlow = 0;
			
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
    			int ret = 0;
    			DCLI_AC_API_GROUP_FIVE *balance = NULL;

    			snmp_log(LOG_DEBUG, "enter show_ac_balance_cmd\n");
    			ret = show_ac_balance_cmd(paraHead->parameter, paraHead->connection,&balance);
    			snmp_log(LOG_DEBUG, "exit show_ac_balance_cmd,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				acLoadBalanceTrigerBaseFlow = balance->flow;
    				Free_ac_balance(balance);
    			}
    		}	
            free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_balance_cmd();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acLoadBalanceTrigerBaseFlow,
                                     sizeof(acLoadBalanceTrigerBaseFlow));
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
			int ret = 0;
			char para[10] = { 0 };

			if((*requests->requestvb->val.integer > 0)&&(*requests->requestvb->val.integer < 31))
			{
				memset(para,0,10);
				snprintf(para,sizeof(para)-1,"%d",*requests->requestvb->val.integer);
                
				instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                    snmp_log(LOG_DEBUG, "enter ac_balance_parameter_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
                                        paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret = ac_balance_parameter_cmd(paraNode->parameter, paraNode->connection,"flow",para);
    				snmp_log(LOG_DEBUG, "exit ac_balance_parameter_cmd,ret=%d\n", ret);
    				
					if(1 == ret)
					{
						acLoadBalanceTrigerBaseFlow = *requests->requestvb->val.integer;
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
			else
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acLoadBalanceStatusBaseOnUsr\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLoadBalanceTrigerBaseFlow\n");
    return SNMP_ERR_NOERROR;
}


int
handle_acLoadBalanceTrigerBaseUsr(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acLoadBalanceTrigerBaseUsr\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int acLoadBalanceTrigerBaseUsr = 0;
			
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
    			int ret = 0;
    			DCLI_AC_API_GROUP_FIVE *balance = NULL;

    			snmp_log(LOG_DEBUG, "enter show_ac_balance_cmd\n");
    			ret = show_ac_balance_cmd(paraHead->parameter, paraHead->connection,&balance);
    			snmp_log(LOG_DEBUG, "exit show_ac_balance_cmd,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				acLoadBalanceTrigerBaseUsr = balance->number;
    				Free_ac_balance(balance);
    			}	
    		}	
            free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_balance_cmd();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acLoadBalanceTrigerBaseUsr,
                                     sizeof(acLoadBalanceTrigerBaseUsr));
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
			int ret = 0;
			char para[10] = { 0 };

			if((*requests->requestvb->val.integer > 0)&&(*requests->requestvb->val.integer < 11))
			{
				memset(para,0,10);
				snprintf(para,sizeof(para)-1,"%d",*requests->requestvb->val.integer);

				instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                    snmp_log(LOG_DEBUG, "enter ac_balance_parameter_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
                                        paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret = ac_balance_parameter_cmd(paraNode->parameter, paraNode->connection,"number",para);
    				snmp_log(LOG_DEBUG, "exit ac_balance_parameter_cmd,ret=%d\n", ret);
    				
					if(1 == ret)
					{
						acLoadBalanceTrigerBaseUsr = *requests->requestvb->val.integer;
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
			else
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acLoadBalanceStatusBaseOnUsr\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLoadBalanceTrigerBaseUsr\n");
    return SNMP_ERR_NOERROR;
}

static void update_data_for_show_ac_balance_cmd()
{
	struct sysinfo info;
	
	if(0 != update_time_show_ac_balance_cmd)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_show_ac_balance_cmd < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_ac_balance_cmd\n");
	
	/*update cache data*/	
	acLoadBalanceStatusBaseOnFlow = 0;
	acLoadBalanceStatusBaseOnUsr = 0;
	acLoadBalanceTrigerBaseFlow = 0;
	acLoadBalanceTrigerBaseUsr = 0;
	
	instance_parameter *paraHead = NULL;
    if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
    {
		int ret = 0;
		DCLI_AC_API_GROUP_FIVE *balance = NULL;
		
		snmp_log(LOG_DEBUG, "enter show_ac_balance_cmd\n");
		ret = show_ac_balance_cmd(paraHead->parameter, paraHead->connection,&balance);
		snmp_log(LOG_DEBUG, "exit show_ac_balance_cmd,ret=%d\n", ret);
		
		if(ret == 1)
		{
			if(balance)
			{
				if(balance->state == 2)
					acLoadBalanceStatusBaseOnFlow = 1;
				else
					acLoadBalanceStatusBaseOnFlow = 0;
			
				if(balance->state == 1)
					acLoadBalanceStatusBaseOnUsr = 1;
				else
					acLoadBalanceStatusBaseOnUsr = 0;
			
				acLoadBalanceTrigerBaseFlow = balance->flow;
			
				acLoadBalanceTrigerBaseUsr = balance->number;
			}
			Free_ac_balance(balance);	
		}
	}	
    free_instance_parameter_list(&paraHead);
	
	sysinfo(&info); 		
	update_time_show_ac_balance_cmd = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_ac_balance_cmd\n");
}

