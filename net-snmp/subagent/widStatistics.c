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
* widStatistics.c
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
#include "widStatistics.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "autelanWtpGroup.h"
#include "ws_dcli_ac.h"
#include "ws_dbus_list_interface.h"

/** Initializes the widStatistics module */


#define FLOODTIMES	"1.13.1"
#define SPOOFTIMES 	"1.13.2"
#define WEAKIVTIMES "1.13.3"

static unsigned int FloodAttackTimes = 0;
static unsigned int SpoofingAttackTimes = 0;
static unsigned int WeakIVAttackTimes = 0;
static long update_time_show_wids_statistics_list_cmd_func = 0;
static void update_data_for_show_wids_statistics_list_cmd_func();

void
init_widStatistics(void)
{
    static oid FloodAttackTimes_oid[128] = {0};
    static oid SpoofingAttackTimes_oid[128] = {0};
    static oid WeakIVAttackTimes_oid[128] = {0};
	size_t public_oid_len   = 0;
	mad_dev_oid(FloodAttackTimes_oid,FLOODTIMES,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(SpoofingAttackTimes_oid,SPOOFTIMES,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(WeakIVAttackTimes_oid,WEAKIVTIMES,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("widStatistics", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("FloodAttackTimes", handle_FloodAttackTimes,
                               FloodAttackTimes_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SpoofingAttackTimes", handle_SpoofingAttackTimes,
                               SpoofingAttackTimes_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("WeakIVAttackTimes", handle_WeakIVAttackTimes,
                               WeakIVAttackTimes_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_FloodAttackTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_FloodAttackTimes\n");

    
    
    switch(reqinfo->mode) {

		case MODE_GET:
		{
			//int FloodAttackTimes = 0;
			instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
            	#if 0
    			int ret = 0;
    			DCLI_AC_API_GROUP_THREE *widsstatis = NULL;

    			snmp_log(LOG_DEBUG, "enter show_wids_statistics_list_cmd_func\n");
    			ret = show_wids_statistics_list_cmd_func(paraHead->parameter, paraHead->connection,&widsstatis);
    			snmp_log(LOG_DEBUG, "exit show_wids_statistics_list_cmd_func,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				FloodAttackTimes = widsstatis->floodingcount;
    				Free_wids_statistics_list_head(widsstatis);
    			}
				#endif

				update_data_for_show_wids_statistics_list_cmd_func(paraHead);
    		}	
            free_instance_parameter_list(&paraHead);
            
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
											(u_char *) &FloodAttackTimes,
											sizeof(FloodAttackTimes));
		}
		break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_FloodAttackTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_FloodAttackTimes\n");
    return SNMP_ERR_NOERROR;
}
int
handle_SpoofingAttackTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_SpoofingAttackTimes\n");

    dbus_parameter parameter;
    void *connection = NULL;
    
    
    switch(reqinfo->mode) {

	    case MODE_GET:
		{
			//int SpoofingAttackTimes = 0;
			instance_parameter *paraHead = NULL;
			if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
			{
				#if 0
    			int ret = 0;
    			DCLI_AC_API_GROUP_THREE *widsstatis = NULL;

    			snmp_log(LOG_DEBUG, "enter show_wids_statistics_list_cmd_func\n");
    			ret = show_wids_statistics_list_cmd_func(paraHead->parameter, paraHead->connection,&widsstatis);
    			snmp_log(LOG_DEBUG, "exit show_wids_statistics_list_cmd_func,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				SpoofingAttackTimes = widsstatis->sproofcount;
    				Free_wids_statistics_list_head(widsstatis);
    			}
				#endif

				update_data_for_show_wids_statistics_list_cmd_func(paraHead);
    		}	
            free_instance_parameter_list(&paraHead);
            
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
									 (u_char *)&SpoofingAttackTimes,
									 sizeof(SpoofingAttackTimes));
	    }
	        break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SpoofingAttackTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_SpoofingAttackTimes\n");
    return SNMP_ERR_NOERROR;
}
int
handle_WeakIVAttackTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_WeakIVAttackTimes\n");
    
    switch(reqinfo->mode) {

	    case MODE_GET:
		{
			//int WeakIVAttackTimes = 0;
			instance_parameter *paraHead = NULL;
			if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
			{
				#if 0
    			int ret = 0;
    			DCLI_AC_API_GROUP_THREE *widsstatis = NULL;

    			snmp_log(LOG_DEBUG, "enter show_wids_statistics_list_cmd_func\n");
    			ret = show_wids_statistics_list_cmd_func(paraHead->parameter, paraHead->connection,&widsstatis);
    			snmp_log(LOG_DEBUG, "exit show_wids_statistics_list_cmd_func,ret=%d\n", ret);
    			
    			if(ret == 1)
    			{
    				WeakIVAttackTimes = widsstatis->weakivcount;
    				Free_wids_statistics_list_head(widsstatis);
    			}
				#endif

				update_data_for_show_wids_statistics_list_cmd_func(paraHead);
			}
            free_instance_parameter_list(&paraHead);
            
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
									 (u_char *)&WeakIVAttackTimes,
									 sizeof(WeakIVAttackTimes));
	    }
	        break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_WeakIVAttackTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_WeakIVAttackTimes\n");
    return SNMP_ERR_NOERROR;
}

static void update_data_for_show_wids_statistics_list_cmd_func(instance_parameter *paraHead)
{
	struct sysinfo info;
	
	if(0 != update_time_show_wids_statistics_list_cmd_func)
	{
		sysinfo(&info); 	
		if(info.uptime - update_time_show_wids_statistics_list_cmd_func < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_wids_statistics_list_cmd_func\n");
	
	/*update cache data*/
	DCLI_AC_API_GROUP_THREE *widsstatis = NULL;
	int ret = 0;

	FloodAttackTimes = 0;
	SpoofingAttackTimes = 0;
	WeakIVAttackTimes = 0;
	
	snmp_log(LOG_DEBUG, "enter show_wids_statistics_list_cmd_func\n");
	ret = show_wids_statistics_list_cmd_func(paraHead->parameter, paraHead->connection, &widsstatis);
	snmp_log(LOG_DEBUG, "exit show_wids_statistics_list_cmd_func,ret=%d\n",ret);
	if((ret == 1)&&(widsstatis))
	{
		FloodAttackTimes = widsstatis->floodingcount;
		SpoofingAttackTimes = widsstatis->sproofcount;
		WeakIVAttackTimes = widsstatis->weakivcount;
	}
	if(ret == 1)
	{
		Free_wids_statistics_list_head(widsstatis);
	}
	
	sysinfo(&info); 		
	update_time_show_wids_statistics_list_cmd_func = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_wids_statistics_list_cmd_func\n");
}


