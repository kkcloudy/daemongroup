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
* dot11DHCP.c
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
#include "ws_dbus_list_interface.h"
#include "ws_dcli_dhcp.h"
#include "dot11DHCP.h"
#include "mibs_public.h"
#include "autelanWtpGroup.h"
#include "ac_sample_def.h"
#include "ac_sample_err.h"
/** Initializes the dot11DHCP module */
#define DHCPIPPOOLUSAGE	"2.6.3.1"
#define DHCPREQTIMES		"2.6.3.2"
#define DHCPREQSUCTIMES	"2.6.3.3"
#define DHCPAVGUSAGE		"2.6.3.4"
#define DHCPPEAKUSAGE		"2.6.3.5"
#define DHCPIPNUM			"2.6.3.6"
#define DHCPDISCOVERTIMES	"2.6.3.7"
#define DHCPOFFERTIMES		"2.6.3.8"
#define Ipv6DHCPREQTIMES	"2.6.3.9"   /*wangchao add*/
#define Ipv6DHCPREQSUCTIMES		"2.6.3.10" /*wangchao add*/

#define DHCP_WHOLE				 "/var/run/apache2/dhcp_whole"
#define DHCP_SEGMENT 			 "/var/run/apache2/dhcp_segment"
#define DHCP_REQUESTTIMES 		 "/var/run/apache2/dhcp_request"
#define DHCP_RESPONSETIMES	     "/var/run/apache2/dhcp_response"

static unsigned int active_lease = 0;
static unsigned int DHCPReqTimes = 0;
static unsigned int DHCPReqSucTimes = 0;
static unsigned int DHCPIpNum = 0;
static unsigned int DHCPDiscoverTimes = 0;
static unsigned int DHCPOfferTimes = 0;
/**wangchao add**/
static unsigned int DHCPv6ReqTimes = 0;
static unsigned int DHCPv6ReplyTimes = 0;
static long update_time_ccgi_show_dhcpv6_statistic_info = 0;
static void update_data_for_ccgi_show_dhcpv6_statistic_info();


static long update_time_ccgi_show_dhcp_statistic_info = 0;
static void update_data_for_ccgi_show_dhcp_statistic_info();

#define DEFAULT_REQSUCC_MULTIPLIER	1

static unsigned long
mib_get_reqSucc_multiplier(void) {
	unsigned long multiplier = DEFAULT_REQSUCC_MULTIPLIER;	/*default*/
	FILE *fp = NULL;    	
	
	fp = fopen("/var/run/DHCPReqSucTimes_multiplier.txt", "r");
	if(NULL == fp) {
		snmp_log(LOG_DEBUG, "Open DHCPReqSucTimes_multiplier.txt error\n");
		goto FAIL_GET;
	}

	if(fscanf(fp, "%lu", &multiplier) <= 0) {
		snmp_log(LOG_DEBUG, "Fscanf DHCPReqSucTimes_multiplier.txt error\n");
		fclose(fp);
		goto FAIL_GET;
	}

	fclose(fp);

	if(!multiplier) {
		goto FAIL_GET;
	}

	return multiplier;
	
FAIL_GET:
	return DEFAULT_REQSUCC_MULTIPLIER;	/*default*/
}


void
init_dot11DHCP(void)
{
	static oid DHCPIPPoolUsage_oid[128] 			= {0};
	static oid DHCPReqTimes_oid[128] 				= {0};
	static oid DHCPReqSucTimes_oid[128] 			= {0};
	static oid DHCPAvgUsage_oid[128] 				= {0};
	static oid DHCPPeakUsage_oid[128] 				= {0};
	static oid DHCPIpNum_oid[128]					= {0};
	static oid DHCPDiscoverTimes_oid[128] 				= {0};
	static oid DHCPOfferTimes_oid[128] 			= {0};
	/*wangchao add*/
	static oid Ipv6DHCPReqTimes_oid[128] 				= {0};
	static oid Ipv6DHCPReqSucTimes_oid[128] 			= {0};	
	
	size_t	public_oid_len = 0;
	mad_dev_oid(DHCPIPPoolUsage_oid,DHCPIPPOOLUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(DHCPReqTimes_oid,DHCPREQTIMES,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(DHCPReqSucTimes_oid,DHCPREQSUCTIMES,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(DHCPAvgUsage_oid,DHCPAVGUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(DHCPPeakUsage_oid,DHCPPEAKUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(DHCPIpNum_oid,DHCPIPNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(DHCPDiscoverTimes_oid,DHCPDISCOVERTIMES,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(DHCPOfferTimes_oid,DHCPOFFERTIMES,&public_oid_len,enterprise_pvivate_oid);
	/*wangchao add*/
	mad_dev_oid(Ipv6DHCPReqTimes_oid,Ipv6DHCPREQTIMES,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(Ipv6DHCPReqSucTimes_oid,Ipv6DHCPREQSUCTIMES,&public_oid_len,enterprise_pvivate_oid);	
	
  DEBUGMSGTL(("dot11DHCP", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("DHCPIPPoolUsage", handle_DHCPIPPoolUsage,
                               DHCPIPPoolUsage_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("DHCPReqTimes", handle_DHCPReqTimes,
                               DHCPReqTimes_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("DHCPReqSucTimes", handle_DHCPReqSucTimes,
                               DHCPReqSucTimes_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("DHCPAvgUsage", handle_DHCPAvgUsage,
                               DHCPAvgUsage_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("DHCPPeakUsage", handle_DHCPPeakUsage,
                               DHCPPeakUsage_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
	
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("DHCPIpNum", handle_DHCPIpNum,
                               DHCPIpNum_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("DHCPDiscoverTime", handle_DHCPDiscoverTimes,
                               DHCPDiscoverTimes_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("DHCPOfferTimes", handle_DHCPOfferTimes,
                               DHCPOfferTimes_oid,public_oid_len,
                               HANDLER_CAN_RONLY
        ));
/*******wangchao add*****/		
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("Ipv6DHCPReqTimes", handle_Ipv6DHCPReqTimes,
                               Ipv6DHCPReqTimes_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("Ipv6DHCPReqSucTimes", handle_Ipv6DHCPReqSucTimes,
                               Ipv6DHCPReqSucTimes_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));		
		
}

int
handle_DHCPIPPoolUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_DHCPIPPoolUsage\n");

    switch(reqinfo->mode) {

	    case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL, *paraNode = NULL;
			int ret = 0;
			struct all_lease_state_p pool_statist = { 0 };
			unsigned int active_lease = 0;
			unsigned int total_lease = 0;
			unsigned int  DHCPIPPoolUsage = 0;
			
		    list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
		    for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
			{
		        ret = 0;
				memset(&pool_statist, 0, sizeof(struct all_lease_state_p));
				snmp_log(LOG_DEBUG, "enter ccgi_show_dhcp_pool_statistics\n");
		        ret = ccgi_show_dhcp_pool_statistics(paraNode->parameter.slot_id, &pool_statist);
				snmp_log(LOG_DEBUG, "exit ccgi_show_dhcp_pool_statistics, ret=%d\n",ret);
		        if(1 == ret) 
				{
					active_lease += pool_statist.all_lease_state.active_lease_num;
					total_lease += pool_statist.all_lease_state.total_lease_num;
		        }	    
			}		
		    free_instance_parameter_list(&paraHead);
			#endif
			unsigned int dhcp_usage = 0;
			
			update_data_for_ccgi_show_dhcp_statistic_info();

			if(0 != DHCPIpNum)
			{
				dhcp_usage = (active_lease*100)/DHCPIpNum;
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
	                                     (u_char *)&dhcp_usage,
	                                      sizeof(dhcp_usage));
        }
		break;
		
		default:
			/* we should never get here, so this is a really bad error */
			snmp_log(LOG_ERR, "unknown mode (%d) in handle_DHCPIPPoolUsage\n", reqinfo->mode );
			return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_DHCPIPPoolUsage\n");
    return SNMP_ERR_NOERROR;
}
int
handle_DHCPReqTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	
	snmp_log(LOG_DEBUG, "enter handle_DHCPReqTimes\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			unsigned int DHCPReqTimes = 0;
			instance_parameter *paraHead = NULL, *paraNode = NULL;
			int ret = 0;
			struct all_lease_state_p pool_statist = { 0 };
			int i = 0;
			
		    list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
		    for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
			{
		        ret = 0;
				memset(&pool_statist, 0, sizeof(struct all_lease_state_p));
				snmp_log(LOG_DEBUG, "enter ccgi_show_dhcp_pool_statistics\n");
		        ret = ccgi_show_dhcp_pool_statistics(paraNode->parameter.slot_id, &pool_statist);
				snmp_log(LOG_DEBUG, "exit ccgi_show_dhcp_pool_statistics, ret=%d\n",ret);
		        if(1 == ret) 
				{					
		            for(i = 0; i < pool_statist.subnet_num; i++) 
					{		                
		                DHCPReqTimes += pool_statist.sub_state[i].info.requested_times;		            
		    	    }
		        }	    
			}		
		    free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_ccgi_show_dhcp_statistic_info();
	        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
	                                     (u_char *)&DHCPReqTimes,
	                                     sizeof(DHCPReqTimes));
	    }
	    break;

	    default:
	        /* we should never get here, so this is a really bad error */
	        snmp_log(LOG_ERR, "unknown mode (%d) in handle_DHCPReqTimes\n", reqinfo->mode );
	        return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_DHCPReqTimes\n");
    return SNMP_ERR_NOERROR;
}
int
handle_DHCPReqSucTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_DHCPReqSucTimes\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			unsigned int DHCPReqSucTimes = 0;
			unsigned int DHCPReqTimes = 0;
			instance_parameter *paraHead = NULL, *paraNode = NULL;
			int ret = 0;
			struct all_lease_state_p pool_statist = { 0 };
			int i = 0;
			snmp_log(LOG_DEBUG, "DHCPReqSucTimes_multiplier = %lu\n", multiplier);
			
		    list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
		    for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
			{
		        ret = 0;
				memset(&pool_statist, 0, sizeof(struct all_lease_state_p));
				snmp_log(LOG_DEBUG, "enter ccgi_show_dhcp_pool_statistics\n");
		        ret = ccgi_show_dhcp_pool_statistics(paraNode->parameter.slot_id, &pool_statist);
				snmp_log(LOG_DEBUG, "exit ccgi_show_dhcp_pool_statistics, ret=%d\n",ret);
		        if(1 == ret) 
				{					
		            for(i = 0; i < pool_statist.subnet_num; i++) 
					{		                
		                DHCPReqSucTimes += pool_statist.sub_state[i].info.ack_times;		            
						DHCPReqTimes += pool_statist.sub_state[i].info.requested_times;
		    	    }
		        }	    
			}		
			free_instance_parameter_list(&paraHead);
			#endif
			unsigned int Times = 0;
			unsigned long multiplier = mib_get_reqSucc_multiplier();

			update_data_for_ccgi_show_dhcp_statistic_info();
			
			Times = ((DHCPReqSucTimes * multiplier) > DHCPReqTimes) ? DHCPReqTimes : (DHCPReqSucTimes * multiplier);
			 
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
	                                     (u_char *)&Times,
	                                     sizeof(Times));
        }
        break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DHCPReqSucTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_DHCPReqSucTimes\n");
    return SNMP_ERR_NOERROR;
}

int
handle_DHCPAvgUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_DHCPAvgUsage\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{

			snmp_log(LOG_DEBUG, "enter trap_read\n");

			unsigned int acDHCPRTUsage = 0;
			unsigned int acDHCPAvgUsage = 0;
			unsigned int acDHCPPeakUsage = 0;
			dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_DHCPUSE,&acDHCPRTUsage,&acDHCPAvgUsage, &acDHCPPeakUsage);
			snmp_log(LOG_DEBUG, "exit trap_read\n");
	
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acDHCPAvgUsage,
                                     sizeof(acDHCPAvgUsage));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DHCPAvgUsage\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_DHCPAvgUsage\n");
    return SNMP_ERR_NOERROR;
}

int
handle_DHCPPeakUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_DHCPPeakUsage\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{

			snmp_log(LOG_DEBUG, "enter trap_read\n");
			
			unsigned int acDHCPRTUsage = 0;
			unsigned int acDHCPAvgUsage = 0;
			unsigned int acDHCPPeakUsage = 0;
			dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_DHCPUSE,&acDHCPRTUsage,&acDHCPAvgUsage, &acDHCPPeakUsage);
			snmp_log(LOG_DEBUG, "exit trap_read\n");

            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acDHCPPeakUsage,
                                     sizeof(acDHCPPeakUsage));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DHCPPeakUsage\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_DHCPPeakUsage\n");
    return SNMP_ERR_NOERROR;
}


int
handle_DHCPIpNum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	
	snmp_log(LOG_DEBUG, "enter handle_DHCPIpNum\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			unsigned int dhcp_whole_count = 0;
			instance_parameter *paraHead = NULL, *paraNode = NULL;
			int ret = 0;
			struct all_lease_state_p pool_statist = { 0 };
			
		    list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
		    for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
			{
		        ret = 0;
				memset(&pool_statist, 0, sizeof(struct all_lease_state_p));
				snmp_log(LOG_DEBUG, "enter ccgi_show_dhcp_pool_statistics\n");
		        ret = ccgi_show_dhcp_pool_statistics(paraNode->parameter.slot_id, &pool_statist);
				snmp_log(LOG_DEBUG, "exit ccgi_show_dhcp_pool_statistics, ret=%d\n",ret);
		        if(1 == ret) 
				{
					dhcp_whole_count += pool_statist.all_lease_state.total_lease_num;
		        }	    
			}		
		    free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_ccgi_show_dhcp_statistic_info();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&DHCPIpNum,
                                     sizeof(DHCPIpNum));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DHCPIpNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_DHCPIpNum\n");
    return SNMP_ERR_NOERROR;
}

int
handle_DHCPDiscoverTimes(netsnmp_mib_handler *handler,
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
			#if 0
			unsigned int dhcp_whole_count = 0;
			instance_parameter *paraHead = NULL, *paraNode = NULL;
			int ret = 0;
			struct all_lease_state_p pool_statist = { 0 };
			int i = 0;
			
		    list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
		    for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
			{
		        ret = 0;
				memset(&pool_statist, 0, sizeof(struct all_lease_state_p));
				snmp_log(LOG_DEBUG, "enter ccgi_show_dhcp_pool_statistics\n");
		        ret = ccgi_show_dhcp_pool_statistics(paraNode->parameter.slot_id, &pool_statist);
				snmp_log(LOG_DEBUG, "exit ccgi_show_dhcp_pool_statistics, ret=%d\n",ret);
		        if(1 == ret) 
				{					
		            for(i = 0; i < pool_statist.subnet_num; i++) 
					{		                
		                dhcp_whole_count += pool_statist.sub_state[i].info.offer_times;		            
		    	    }
		        }	    
			}		
		    free_instance_parameter_list(&paraHead);
			#endif
			
			update_data_for_ccgi_show_dhcp_statistic_info();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                     (u_char *)&DHCPDiscoverTimes,
                                     sizeof(DHCPDiscoverTimes));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DHCPDiscoverTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_DHCPOfferTimes(netsnmp_mib_handler *handler,
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
			update_data_for_ccgi_show_dhcp_statistic_info();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
											(u_char *)&DHCPOfferTimes,
											sizeof(DHCPOfferTimes));
	    }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DHCPOfferTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
#if 0 
int dhcp_get(int *whole_count,int *seg_count)
{
	
	char buf[256];
	memset(buf,0,256);
	char seg[5] = { 0 };
	memset(seg,0,5);
	char *tmp = (char*)malloc(40);
	memset(tmp,0,40);
	char *tmp2 = (char*)malloc(40);
	memset(tmp2,0,40);
	unsigned long int whole = 0;
	char count1[40] = { 0 };
	memset(count1,0,40);
	char count2[40] = { 0 };
	memset(count2,0,40);
	int t = 0;
	int m = 0;
	unsigned long int t11 = 0;
	unsigned long int t12 = 0;
	unsigned long int t13 = 0;
	unsigned long int t14 = 0;
	unsigned long int t21 = 0;
	unsigned long int t22 = 0;
	unsigned long int t23 = 0;
	unsigned long int t24 = 0;
	int retu = system("ip_ser.sh");
	if(retu != 0)
	{
		return 1;
	}
	int ret=system("count_rn.sh");
	if(ret!=0)
	{
		return 2;
	}
	FILE *fd = NULL;
	fd=fopen("/var/run/apache2/dhcp_rang.tmp","r");
	if(fd==NULL)
	{
		return 3;
	}
	fseek(fd,0,0);
	while(fgets(buf,256,fd))
	{
		t =0;
		tmp = strtok(buf," ");
		while(tmp!=NULL)
		{
			t++;					
			if(t==1)
			{
				strncpy(count1,tmp,sizeof(count1)-1);
			}
			else if(t==2)
			{
				strncpy(count2,tmp,sizeof(count2)-1);
			}
			tmp=strtok(NULL," ");
		}
		tmp2 = strtok(count1,".");
		m = 0;
		while(tmp2!=NULL)
		{
			m++;
			if(m ==1)
			{
				t11 = strtol(tmp2,NULL,10);
			}
			else if(m==2)
			{
				t12 = strtol(tmp2,NULL,10);
			}
			else if(m==3)
			{
				t13 = strtol(tmp2,NULL,10);
			}
			else if(m==4)
			{
				t14 = strtol(tmp2,NULL,10);
			}
			tmp2 = strtok(NULL,".");
		}
		tmp2 = strtok(count2,".");
		m = 0;
		while(tmp2!=NULL)
		{
			m++;
			if(m ==1)
			{
				t21 = strtol(tmp2,NULL,10);
			}
			else if(m==2)
			{
				t22 = strtol(tmp2,NULL,10);		
			}
			else if(m==3)
			{
				t23 = strtol(tmp2,NULL,10);
			}
			else if(m==4)
			{
				t24 = strtol(tmp2,NULL,10);
			}
			tmp2 = strtok(NULL,".");
		}
		
		t21 = t21-t11;
		t22 = t22-t12;
		t23 = t23-t13;
		t24 = t24-t14+1;
		whole = whole + t21*255*255*255+t22*255*255+t23*255+t24;
	}
	FILE * fd2=fopen("/var/run/apache2/dhcp_stat.tmp","r");
	if(fd2==NULL)
	{
		fprintf(stderr,"can not open dhcp_stat!");
	}
	else
	{
		fgets(seg,5,fd2);
	}
	*seg_count=atoi(seg);
	*whole_count=whole;
	FREE_OBJECT(tmp);
	FREE_OBJECT(tmp2);
	fclose(fd);
	if(fd2 != NULL)
	{
		fclose(fd2);
	}	 
}
#endif

static void update_data_for_ccgi_show_dhcp_statistic_info()
{
	struct sysinfo info;
	
	if(0 != update_time_ccgi_show_dhcp_statistic_info)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_ccgi_show_dhcp_statistic_info < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for ccgi_show_dhcp_statistic_info\n");
	
	/*update cache data*/
	instance_parameter *paraHead = NULL, *paraNode = NULL;
	int ret = 0;
	struct all_lease_state_p pool_statist = { 0 };
	int i = 0;
	
	active_lease = 0;
	DHCPReqTimes = 0;
	DHCPReqSucTimes = 0;
	DHCPIpNum = 0;
	DHCPDiscoverTimes = 0;
	DHCPOfferTimes = 0;
	
	snmp_log(LOG_DEBUG, "enter list_instance_parameter\n");
    list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
	snmp_log(LOG_DEBUG, "exit list_instance_parameter,paraHead=%p\n",paraHead);
    for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
	{
        ret = 0;
		memset(&pool_statist, 0, sizeof(struct all_lease_state_p));
        ret = ccgi_show_dhcp_pool_statistics(paraNode->parameter.slot_id, &pool_statist);
        if(1 == ret) 
		{
			active_lease += pool_statist.all_lease_state.active_lease_num;
			for(i = 0; i < pool_statist.subnet_num; i++) 
			{		                
                DHCPReqTimes += pool_statist.sub_state[i].info.requested_times;		            
				DHCPReqSucTimes += pool_statist.sub_state[i].info.ack_times;
				DHCPDiscoverTimes += pool_statist.sub_state[i].info.discover_times;
				DHCPOfferTimes += pool_statist.sub_state[i].info.offer_times;
    	    }
			DHCPIpNum += pool_statist.all_lease_state.total_lease_num;
		}
	}		
    free_instance_parameter_list(&paraHead);
	
	sysinfo(&info); 		
	update_time_ccgi_show_dhcp_statistic_info = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for ccgi_show_dhcp_statistic_info\n");
}


/***wangchao add****/
#define DHCP_MAX_SLOT_NUM 16
static void update_data_for_ccgi_show_dhcpv6_statistic_info()
{
	struct sysinfo info;
	if(0 != update_time_ccgi_show_dhcpv6_statistic_info)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_ccgi_show_dhcpv6_statistic_info < cache_time)
		{
			return;
		}
	}
	
	/*update cache data*/
	instance_parameter *paraHead = NULL, *paraNode = NULL;
	int ret = 0;
	struct dhcpv6_statistics_info statistic_info[DHCP_MAX_SLOT_NUM] = {0};
	int i = 0;
	
	DHCPv6ReqTimes = 0;
	DHCPv6ReplyTimes = 0;

	snmp_log(LOG_DEBUG, "enter list_instance_parameter\n");
    list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
	snmp_log(LOG_DEBUG, "exit list_instance_parameter,paraHead=%p\n",paraHead);
    for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
	{
        ret = 0;
        ret = ccgi_show_dhcpv6_pool_statistics(paraNode->parameter.slot_id, statistic_info);
        if(1 == ret) 
		{	           
			 //snmp_log(LOG_DEBUG, "dhcpv6_request_times = %d\n",statistic_info[paraNode->parameter.slot_id].dhcpv6_request_times);
			 //snmp_log(LOG_DEBUG, "dhcpv6_reply_times = %d\n",statistic_info[paraNode->parameter.slot_id].dhcpv6_reply_times);
			 

             DHCPv6ReqTimes += statistic_info[paraNode->parameter.slot_id].dhcpv6_request_times;		            
		     DHCPv6ReplyTimes += statistic_info[paraNode->parameter.slot_id].dhcpv6_reply_times;
			 
		}
	}		
    free_instance_parameter_list(&paraHead);
	
	sysinfo(&info); 		
	update_time_ccgi_show_dhcpv6_statistic_info = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for ccgi_show_dhcp_statistic_info\n");
}


/***wangchao add***/
int
handle_Ipv6DHCPReqTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_Ipv6DHCPReqTimes\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			
		 	update_data_for_ccgi_show_dhcpv6_statistic_info();
	        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
	                                     (u_char *)&DHCPv6ReqTimes,
	                                     sizeof(DHCPv6ReqTimes));
	    }
	    break;

	    default:
	        /* we should never get here, so this is a really bad error */
	        snmp_log(LOG_DEBUG, "unknown mode (%d) in handle_DHCPReqTimes\n", reqinfo->mode );
	        return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_DHCPReqTimes\n");
    return SNMP_ERR_NOERROR;
}

/***wangchao add****/
int
handle_Ipv6DHCPReqSucTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_DHCPReqSucTimes\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			update_data_for_ccgi_show_dhcpv6_statistic_info();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
	                                     (u_char *)&DHCPv6ReplyTimes,
	                                     sizeof(DHCPv6ReplyTimes));
        }
        break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_DEBUG, "unknown mode (%d) in handle_DHCPReqSucTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_DHCPReqSucTimes\n");
    return SNMP_ERR_NOERROR;
}


