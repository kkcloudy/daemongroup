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
* dot11AcInfo.c
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
#include "dot11AcInfo.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"
#include "mibs_public.h"
#include "ws_log_conf.h"
#include "ws_stp.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "board/board_define.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_ac.h"
#include "ws_log_conf.h"
#include "ws_dhcp_conf.h"
#include "ws_dcli_boot.h"
#include "ac_sample_def.h"
#include "ac_sample_err.h"
#include "autelanWtpGroup.h"
#include <stdlib.h>
#include <unistd.h>
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "ws_dbus_list.h"
#include "ws_acinfo.h"
#include <string.h> 
#include <stdio.h>
#include "ws_dcli_vrrp.h"

#define SYS_LOCATION_CONFIG_FILE "/var/run/sys_location" 
#define AC_CONTACT_INFO_FILE "/var/run/ac_contact_info"


 static CPUS_STU st_cpus_status;
 static CPU_TEM st_cpu_temperature;
 static MEM_STU st_mem_status;
 static SYS_INFO st_sys_info={ &st_cpus_status, 
                              &st_cpu_temperature,  
                              &st_mem_status};
/** Initializes the dot11AcInfo module */

#define     ACSOFTWARENAME	 				"2.1.1.1"
#define		ACSOFTWAREVERSION   			"2.1.1.2"
#define		ACRUNNINGTIME					"2.1.1.3"
#define		ACSAMPLETIME					"2.1.1.4"
#define		ACSYSTEMTIME					"2.1.1.5"
#define		ACSOFTWAREVENDOR    			"2.1.1.6"
#define		ACCONTACTACTINFO				"2.1.1.7"
#define		ACPRODUCTNAME					"2.1.1.8"
#define		ACLOCATIONINFO   				"2.1.1.9"
#define		ACDOMAIN   						"2.1.1.10"
#define		ACLATESTPOLLDEVTIME 			"2.1.1.11"
#define		ACLOGSERVERADDR 				"2.1.1.12"
#define		ACSYSLOGSERVERPORT				"2.1.1.13"
#define		ACNTPCONFIG 					"2.1.1.14"
#define		ACTIMEAFTERNTPCAL				"2.1.1.15"
#define		ACSTP 							"2.1.1.16"
#define		ACSNMPTRAPCONF 					"2.1.1.17"
#define		ACSTATWINDOWTIME 				"2.1.1.18"
#define		ACHEARTBEATPERIOD 				"2.1.1.19"
#define		ACSYSRESTART					"2.1.1.20"
#define		ACSYSRESET						"2.1.1.21"
#define		TIMESYNPERIOD					"2.1.1.22"
#define     SYSLOGSVCENABLE                 "2.1.1.23"
#define     SYSLOGREPORTEVENTLEVEL          "2.1.1.24"
#define		SYSOBJECTID						"2.1.1.25"
#define		ACHEARTBEATENABLE				"2.1.1.26"
#define     NORMALCOLLECTCYCLE              "2.1.1.27"
#define     RTCOLLECTCYCLE                  "2.1.1.28"
#define     SOFTWAREHWSPECR                 "2.1.1.29"
#define		ACSOFTWAREBIGVERSION   			"2.1.1.33"
#define		ACBOOTIMG			   			"2.1.1.34"
#define		ACWRITECONFIG					"2.1.1.35"
#define		SYSBACKUPIDENTITY				"2.1.1.36"
#define		SYSBACKMODE						"2.1.1.37"
#define 	SYSBACKSTATUS					"2.1.1.38"
#define		SYSBACKNETIP					"2.1.1.39"
#define		SYSBACKSWITCHTIME 				"2.1.1.40"
#define		SYSBACKNETIPV6 				        "2.1.1.41"
#define		ACSLOTINFO				"2.1.1.42"


static char acSoftwareName[50] = { 0 };
static char acSoftwareVersion[50] = { 0 };
static char acProductName[50] = { 0 };
static long update_time_show_sys_ver = 0;
static void update_data_for_show_sys_ver();

static unsigned int acSampleTime = 0;
static int acStatWindowTime = 0;
static unsigned int NormalCollectCycle = 0;
static unsigned int RtCollectCycle = 0;
static long update_time_show_ap_moment_information_reportinterval_cmd = 0;
static void update_data_for_show_ap_moment_information_reportinterval_cmd();

static int identi = 0;
static int mode = 0;
static int status = 0;
static unsigned long ipaddr = 0;
static char ipv6address[128] = { 0 };
static long update_time_show_backupidentity_information = 0;
static void update_data_for_show_backupidentity_information();

static char acslotinfo[35] = { 0 };
static void ac_slot_information();

void
init_dot11AcInfo(void)
{
    static oid acSoftwareName_oid[128] 		= 	{0};
    static oid acSoftwareVersion_oid[128] 	= 	{0};
    static oid acRunningTime_oid[128] 		= 	{0};
    static oid acSampleTime_oid[128]		=	{0};
    static oid acSystemTime_oid[128]		=	{0};
    static oid acSoftwareVendor_oid[128] 	= 	{0};
    static oid acContactInfo_oid[128] 		= 	{0};
    static oid acProductName_oid[128] 		= 	{0};
    static oid acLocationInfo_oid[128] 		=	{0};
    static oid acDomain_oid[128] 			=	{0};
    static oid acLatestPollDevTime_oid[128]	= 	{0};
    static oid acLogServerAddr_oid[128] 	=	{0};
    static oid acSyslogServerPort_oid[128]	= 	{0};
    static oid acNTPConfig_oid[128]			=	{0};
    static oid acTimeAfterNTPCal_oid[128]	=	{0};
    static oid acSTP_oid[128] 				=	{0};
    static oid acSnmpTrapConfig_oid[128]	=	{0};
    static oid acStatWindowTime_oid[128] 	= 	{0};
    static oid acHeartbeatPeriod_oid[128] 	=	{0};
    static oid acSysRestart_oid[128]  		=	{0};
    static oid acSysReset_oid[128]			=	{0};
	static oid TimeSynPeriod_oid[128]		=	{0};
	static oid SyslogSvcEnable_oid[128]     =   {0};
	static oid SyslogReportEventLevel_oid[128]  =   {0};
	static oid SysObjectId_oid[128]				=	{0};
	static oid acHeartBeatEnable_oid[128]		=	{0};
	static oid NormalCollectCycle_oid[128]      =   {0};
	static oid RtCollectCycle_oid[128]      = {0};
	static oid acSoftwareHwspecr_oid[128] = {0};
    static oid acSoftwareBigVersion_oid[128] 	= 	{0};
	static oid acBootImg_oid[128] 	= 	{0};
	static oid acWriteConfig[128] 	= 	{0};
    static oid sysBackupIdentity_oid[128] =          {0};
    static oid sysBackupMode_oid[128] =        {0};
    static oid sysBackupStatus_oid[128] =     {0};
    static oid sysBackupNetworManageIp_oid[128] =  {0};
    static oid sysBackupSwitchTimes_oid[128]  =  {0};
    static oid sysBackupNetworManageIpv6_oid[128] = { 0};
	static oid acslotinfo_oid[128] = {0};

	
	size_t public_oid_len   = 0;
	
	mad_dev_oid(acSoftwareName_oid,ACSOFTWARENAME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSoftwareVersion_oid,ACSOFTWAREVERSION,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acRunningTime_oid,ACRUNNINGTIME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSampleTime_oid,ACSAMPLETIME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSystemTime_oid,ACSYSTEMTIME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSoftwareVendor_oid,ACSOFTWAREVENDOR,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acContactInfo_oid,ACCONTACTACTINFO,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acProductName_oid,ACPRODUCTNAME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLocationInfo_oid,ACLOCATIONINFO,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acDomain_oid,ACDOMAIN,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLatestPollDevTime_oid,ACLATESTPOLLDEVTIME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLogServerAddr_oid,ACLOGSERVERADDR,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSyslogServerPort_oid,ACSYSLOGSERVERPORT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acNTPConfig_oid,ACNTPCONFIG,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acTimeAfterNTPCal_oid,ACTIMEAFTERNTPCAL,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSTP_oid,ACSTP,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSnmpTrapConfig_oid,ACSNMPTRAPCONF,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acStatWindowTime_oid,ACSTATWINDOWTIME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acHeartbeatPeriod_oid,ACHEARTBEATPERIOD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSysRestart_oid,ACSYSRESTART,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSysReset_oid,ACSYSRESET,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(TimeSynPeriod_oid,TIMESYNPERIOD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(SyslogSvcEnable_oid,SYSLOGSVCENABLE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(SyslogReportEventLevel_oid,SYSLOGREPORTEVENTLEVEL,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(SysObjectId_oid,SYSOBJECTID,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acHeartBeatEnable_oid,ACHEARTBEATENABLE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(NormalCollectCycle_oid,NORMALCOLLECTCYCLE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(RtCollectCycle_oid,RTCOLLECTCYCLE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSoftwareHwspecr_oid,SOFTWAREHWSPECR,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSoftwareBigVersion_oid,ACSOFTWAREBIGVERSION,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acBootImg_oid,ACBOOTIMG,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acWriteConfig,ACWRITECONFIG,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(sysBackupIdentity_oid,SYSBACKUPIDENTITY,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(sysBackupMode_oid,SYSBACKMODE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(sysBackupStatus_oid,SYSBACKSTATUS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(sysBackupNetworManageIp_oid,SYSBACKNETIP,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(sysBackupSwitchTimes_oid,SYSBACKSWITCHTIME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(sysBackupNetworManageIpv6_oid,SYSBACKNETIPV6,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acslotinfo_oid,ACSLOTINFO,&public_oid_len,enterprise_pvivate_oid);
	
  DEBUGMSGTL(("dot11AcInfo", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSoftwareName", handle_acSoftwareName,
                               acSoftwareName_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSoftwareVersion", handle_acSoftwareVersion,
                               acSoftwareVersion_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acRunningTime", handle_acRunningTime,
                               acRunningTime_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSampleTime", handle_acSampleTime,
                               acSampleTime_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSystemTime", handle_acSystemTime,
                               acSystemTime_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSoftwareVendor", handle_acSoftwareVendor,
                               acSoftwareVendor_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acContactInfo", handle_acContactInfo,
                               acContactInfo_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acProductName", handle_acProductName,
                               acProductName_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLocationInfo", handle_acLocationInfo,
                               acLocationInfo_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acDomain", handle_acDomain,
                               acDomain_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLatestPollDevTime", handle_acLatestPollDevTime,
                               acLatestPollDevTime_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLogServerAddr", handle_acLogServerAddr,
                               acLogServerAddr_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSyslogServerPort", handle_acSyslogServerPort,
                               acSyslogServerPort_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acNTPConfig", handle_acNTPConfig,
                               acNTPConfig_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acTimeAfterNTPCal", handle_acTimeAfterNTPCal,
                               acTimeAfterNTPCal_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSTP", handle_acSTP,
                               acSTP_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSnmpTrapConfig", handle_acSnmpTrapConfig,
                               acSnmpTrapConfig_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));


    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acStatWindowTime", handle_acStatWindowTime,
                               acStatWindowTime_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acHeartbeatPeriod", handle_acHeartbeatPeriod,
                               acHeartbeatPeriod_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSysRestart", handle_acSysRestart,
                               acSysRestart_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSysReset", handle_acSysReset,
                               acSysReset_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("TimeSynPeriod", handle_TimeSynPeriod,
                               TimeSynPeriod_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("SyslogSvcEnable", handle_SyslogSvcEnable,
                               SyslogSvcEnable_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("SyslogReportEventLevel", handle_SyslogReportEventLevel,
                               SyslogReportEventLevel_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("SysObjectId", handle_SysObjectId,
                               SysObjectId_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
	netsnmp_register_scalar(
    netsnmp_create_handler_registration("acHeartBeatEnable", handle_acHeartBeatEnable,
                           acHeartBeatEnable_oid, public_oid_len,
                           HANDLER_CAN_RWRITE
    ));
	netsnmp_register_scalar(		    
    netsnmp_create_handler_registration("NormalCollectCycle", handle_NormalCollectCycle,
                           NormalCollectCycle_oid, public_oid_len,
                           HANDLER_CAN_RWRITE
    ));

	netsnmp_register_scalar(
    netsnmp_create_handler_registration("RtCollectCycle", handle_RtCollectCycle,
                           RtCollectCycle_oid, public_oid_len,
                           HANDLER_CAN_RWRITE
    ));

	netsnmp_register_scalar(
    netsnmp_create_handler_registration("acSoftwareHwSpecr", handle_acSoftwareHwSpecr,
                           acSoftwareHwspecr_oid, public_oid_len,
                           HANDLER_CAN_RONLY
    ));

    netsnmp_register_scalar(
	netsnmp_create_handler_registration("acSoftwareBigVersion", handle_acSoftwareBigVersion,
						   acSoftwareBigVersion_oid, public_oid_len,
						   HANDLER_CAN_RONLY
	));
	netsnmp_register_scalar(
	netsnmp_create_handler_registration("acBootImg", handle_acBootImg,
						   acBootImg_oid, public_oid_len,
						   HANDLER_CAN_RWRITE
	));

	netsnmp_register_scalar(
	netsnmp_create_handler_registration("acWriteConfig", handle_acWriteConfig,
						   acWriteConfig, public_oid_len,
						   HANDLER_CAN_RWRITE
	));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("sysBackupIdentity", handle_sysBackupIdentity,
                               sysBackupIdentity_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("sysBackupMode", handle_sysBackupMode,
                               sysBackupMode_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("sysBackupStatus", handle_sysBackupStatus,
                               sysBackupStatus_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("sysBackupNetworManageIp", handle_sysBackupNetworManageIp,
                               sysBackupNetworManageIp_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("sysBackupSwitchTimes", handle_sysBackupSwitchTimes,
                               sysBackupSwitchTimes_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("sysBackupNetworManageIpv6", handle_sysBackupNetworManageIpv6,
                               sysBackupNetworManageIpv6_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acslotinfo", handle_acslotinfo,
                               acslotinfo_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_acSoftwareName(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acSoftwareName\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int ret = -1;
			struct sys_ver ptrsysver;/*产品系统信息*/
			char name[50] = { 0 };
			memset(name,0,50);

			snmp_log(LOG_DEBUG, "enter show_sys_ver\n");
			ret = show_sys_ver(&ptrsysver);
			snmp_log(LOG_DEBUG, "exit show_sys_ver,ret=%d\n", ret);
			
			if(ret==0)
			{
				memset(name,0,50);
				snprintf(name,sizeof(name)-1,"%s",ptrsysver.sw_name);
			}
			delete_enter(name);
			#endif
			
			update_data_for_show_sys_ver();
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)acSoftwareName,
                                     strlen(acSoftwareName));

			#if 0
			FREE_OBJECT(ptrsysver.product_name);
			FREE_OBJECT(ptrsysver.base_mac);
			FREE_OBJECT(ptrsysver.serial_no);
			FREE_OBJECT(ptrsysver.swname);
			#endif
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSoftwareName\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSoftwareName\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSoftwareVersion(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acSoftwareVersion\n");

    switch(reqinfo->mode) {

       case MODE_GET:
	   {
	   		#if 0
			int ret = -1;
			struct sys_ver ptrsysver;/*产品系统信息*/
			char version[50] = { 0 };
			memset(version,0,50);			
			memset(ptrsysver.sw_version_str,0,128);

			snmp_log(LOG_DEBUG, "enter show_sys_ver\n");
			ret = show_sys_ver(&ptrsysver);
			snmp_log(LOG_DEBUG, "exit show_sys_ver,ret=%d\n", ret);
			
			if(ret==0)
			{
				memset(version,0,50);
				snprintf(version,sizeof(version)-1,ptrsysver.sw_version_str);
			}
			delete_enter(version);
			#endif

			update_data_for_show_sys_ver();
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)acSoftwareVersion,
                                     strlen(acSoftwareVersion));

			#if 0
			FREE_OBJECT(ptrsysver.product_name);
			FREE_OBJECT(ptrsysver.base_mac);
			FREE_OBJECT(ptrsysver.serial_no);
			FREE_OBJECT(ptrsysver.swname);
			#endif
       }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSoftwareVersion\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSoftwareVersion\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acRunningTime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acRunningTime\n");

	switch(reqinfo->mode) {
	
        case MODE_GET:
		{
			int time = 0;
			FILE *get_uptime = NULL;
			char temp[255] = { 0 };
			memset(temp,0,255);
			char *uptime = NULL;
			
			get_uptime = popen("cat /proc/uptime","r");

			if(get_uptime != NULL)
			{
				fgets(temp,255,get_uptime);
				delete_enter(temp);
				uptime=strtok(temp," ");
				if(uptime)
				{
					time = strtol(uptime,NULL,10) * 100;
				}
				pclose(get_uptime);
			}

            snmp_set_var_typed_value(requests->requestvb, ASN_TIMETICKS,
                                     (u_char *)&time,
                                     sizeof(time));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acRunningTime\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acRunningTime\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSampleTime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acSampleTime\n");
    
	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			#if 0
			int ret = 0;
			int time = 5;
			struct ap_reportinterval_profile reportinterval_info;
			#endif
			
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
            	#if 0
    			snmp_log(LOG_DEBUG, "enter show_ap_moment_information_reportinterval_cmd\n");
    			ret = show_ap_moment_information_reportinterval_cmd(paraHead->parameter, paraHead->connection,&reportinterval_info);
    			snmp_log(LOG_DEBUG, "exit show_ap_moment_information_reportinterval_cmd,ret=%d\n", ret);
    			
    			if((ret == 1) && (reportinterval_info.sample_time> 1) && (reportinterval_info.sample_time <= 900))
    			{
    				time = reportinterval_info.sample_time;
    			}
    			else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(paraHead->parameter.slot_id);
        	    }
				#endif

				update_data_for_show_ap_moment_information_reportinterval_cmd(paraHead);
            }
            free_instance_parameter_list(&paraHead);
            
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&acSampleTime,
									sizeof(acSampleTime));
		}
		break;
		
		case MODE_SET_RESERVE1:
		//   if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
		//  }
		break;

		case MODE_SET_RESERVE2:
		/* XXX malloc "undo" storage buffer */
		//  if (/* XXX if malloc, or whatever, failed: */) {

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{
			int ret1 = 0;
			int ret2 = AS_RTN_DBUS_ERR;
			char value[10] = { 0 };

			if((*requests->requestvb->val.integer > 1) && (*requests->requestvb->val.integer <= 900))
			{					
				memset(value,0,10);
				snprintf(value,sizeof(value)-1,"%d",*requests->requestvb->val.integer);		
				
				instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {

    				snmp_log(LOG_DEBUG, "enter set_ap_sample_infomation_reportinterval_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
    				                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret1 = set_ap_sample_infomation_reportinterval_cmd(paraNode->parameter, paraNode->connection,value);
    				snmp_log(LOG_DEBUG, "exit set_ap_sample_infomation_reportinterval_cmd,ret1=%d\n", ret1);
    				
					if(1 == ret1)
					{
						acSampleTime = *requests->requestvb->val.integer;
					}
					else
    				{
    				    if(SNMPD_CONNECTION_ERROR == ret1) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
    				}	
                }
                free_instance_parameter_list(&paraHead);
                
				snmp_log(LOG_DEBUG, "enter dbus_set_sample_param\n");
				ret2 = dbus_set_sample_param( ccgi_dbus_connection , AC_SAMPLE_PARAM_TYPE_INTERVAL, *requests->requestvb->val.integer );
				snmp_log(LOG_DEBUG, "exit dbus_set_sample_param,ret2=%d\n", ret2);
				
				if ( AS_RTN_OK != ret2 )
				{
					netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
				}
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
			}		
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSoftwareVendor\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSampleTime\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSystemTime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */
	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acSystemTime\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			FILE *info = NULL;
			char time[50] = { 0 };
			memset(time,0,50);
			info = popen("date +%F+%X","r");

			if(info != NULL)
			{
				fgets(time,50,info);
				delete_enter(time);
				char * tmp = NULL;
				int i = 0;
				tmp = time;
				while(tmp[i] != '+')
				{
					tmp++;
					i++;
				}
				tmp[i] = ' ';
				pclose(info);
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *)time,
									strlen(time));
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		/* XXX: perform the value change here */
		//  if (/* XXX: error? */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* some error */);
		//  }
		{
			FILE *p1 = NULL,*p2 = NULL,*p3 = NULL;
			char * time_info = (char *)malloc(requests->requestvb->val_len+1);
			char date[20] = { 0 };
			char time[20] = { 0 };
			char set_date[50] = { 0 };
			char set_time[50] = { 0 };
			memset(time,0,20);
			memset(date,0,20);
			memset(set_date,0,50);
			memset(set_time,0,50);
			if(time_info)
			{
				memset(time_info,0,requests->requestvb->val_len+1);
				strncpy(time_info,requests->requestvb->val.string,requests->requestvb->val_len);
				strncpy(date,strtok(time_info," "),sizeof(date)-1);
			}
			strncpy(time,strtok(NULL," "),sizeof(time)-1);
			strncat(set_date,"sudo date -s ",sizeof(set_date)-strlen(set_date)-1);
			strncat(set_date,date,sizeof(set_date)-strlen(set_date)-1);
			strncat(set_time,"sudo date -s ",sizeof(set_time)-strlen(set_time)-1);
			strncat(set_time,time,sizeof(set_time)-strlen(set_time)-1);
			p1 = popen(set_date,"r");
			if(p1 != NULL)
			{
				pclose(p1);
			}
			p2 = popen(set_time,"r");
			if(p2 != NULL)
			{
				pclose(p2);
			}
			//deleted by LT,do not notify hwclock
			//p3 = popen("sudo hwclock --systohc","r");
			//if(p3 != NULL)
			//{
			//	pclose(p3);
			//}
			FREE_OBJECT(time_info);
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;

		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acContactInfo\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acSystemTime\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acSoftwareVendor(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acSoftwareVendor\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			FILE * get_logo = NULL;
			char logo[100] = { 0 };
			memset(logo,0,100);
			get_logo = popen("cat /devinfo/enterprise_name | sed \"2,200d\"","r");
			if(get_logo != NULL)
			{
				fgets(logo,100,get_logo);
				delete_enter(logo);
				pclose(get_logo);
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *)logo,
									strlen(logo));
		}
		break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSoftwareVendor\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSoftwareVendor\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acContactInfo(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */
	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acContactInfo\n");

	switch(reqinfo->mode) 
	{
		case MODE_GET:
		{
			char contact_info[512]="";

			snmp_log(LOG_DEBUG, "enter fopen\n");
			FILE *contact_fp=fopen(AC_CONTACT_INFO_FILE, "r+");
			snmp_log(LOG_DEBUG, "exit fopen,contact_fp=%p\n", contact_fp);
			
			if (NULL == contact_fp)
			{
				contact_info[0]='\0';
			}
			else
			{
				fread(contact_info, sizeof(char), sizeof(contact_info)-1, contact_fp);
				fclose(contact_fp);
				contact_info[strlen(contact_info)-1] = '\0';
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *)contact_info,
									strlen(contact_info));
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{
			char * input_string = (char *)malloc(requests->requestvb->val_len+1);
			if(input_string)
			{
				memset(input_string,0,requests->requestvb->val_len+1);
				strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);
			
				instance_parameter *paraHead = NULL, *paraNode = NULL;
				list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
				for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
				{
					ac_manage_set_acinfo_rule(paraNode->connection, CON_TYPE,input_string,OPT_ADD); 				
				}
			}
			
			FREE_OBJECT(input_string);
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;

		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acContactInfo\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acContactInfo\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acProductName(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acVersionInfo\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			struct sys_ver ptrsysver;/*产品系统信息*/
			char dvId[50] = { 0 };
			memset(dvId,0,50);
			
			memset(ptrsysver.sw_product_name,0,128);

			snmp_log(LOG_DEBUG, "enter show_sys_ver\n");
			if(show_sys_ver(&ptrsysver)==0)
			{
				snprintf(dvId,sizeof(dvId)-1,"%s",ptrsysver.sw_product_name);
			}
			snmp_log(LOG_DEBUG, "exit show_sys_ver\n");
			
			delete_enter(dvId);
			#endif

			update_data_for_show_sys_ver();
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) acProductName,
                                     strlen(acProductName));

			#if 0
			FREE_OBJECT(ptrsysver.product_name);
			FREE_OBJECT(ptrsysver.base_mac);
			FREE_OBJECT(ptrsysver.serial_no);
			FREE_OBJECT(ptrsysver.swname);
			#endif
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acProductName\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acVersionInfo\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acLocationInfo(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acLocationInfo\n");

    switch(reqinfo->mode) {

    case MODE_GET:
	{
		FILE *fp = NULL;
		char result[256] = { 0 };
		memset(result,0,256);
		char acLocationInfo[256] = { 0 };
		memset(acLocationInfo,0,256);
		char *temp = NULL;
		
		fp = fopen(SYS_LOCATION_CONFIG_FILE,"r");
		if(fp)
		{
			memset(result,0,256);
			fgets(result,256,fp);
			temp=strchr(result,':');
			memset(acLocationInfo,0,256);
			if(temp)
			{
				strncpy(acLocationInfo,temp+1,sizeof(acLocationInfo)-1);
			}
			fclose(fp);
		}		
        delete_enter(acLocationInfo);
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *)acLocationInfo,
									strlen(acLocationInfo));
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
		char * input_string = (char *)malloc(requests->requestvb->val_len+1);
		if(input_string)
		{
			memset(input_string,0,requests->requestvb->val_len+1);
			strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);
		
			instance_parameter *paraHead = NULL, *paraNode = NULL;
			list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
			for(paraNode = paraHead; paraNode; paraNode = paraNode->next) 
			{
				ac_manage_set_acinfo_rule(paraNode->connection, LOC_TYPE,input_string,OPT_ADD); 				
			}
		}
		
		FREE_OBJECT(input_string);
	}
		break;

	case MODE_SET_COMMIT:
		break;

	case MODE_SET_UNDO:
		break;


    default:
        /* we should never get here, so this is a really bad error */
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_acLocationInfo\n", reqinfo->mode );
        return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLocationInfo\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acDomain(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	snmp_log(LOG_DEBUG, "enter handle_acDomain\n");

    switch(reqinfo->mode) {

        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)"CHINA",
                                     strlen("CHINA"));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDomain\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acDomain\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acLatestPollDevTime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acLatestPollDevTime\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char last_time[30] = { 0 };
			memset(last_time,0,30);
			FILE * fp = NULL;
			fp = fopen("/var/run/apache2/lastpoll.txt","r");
			if(fp != NULL)
			{
				memset(last_time,0,30);
				fgets(last_time,30,fp);
				fclose(fp);
			}
			else
			{
				memset(last_time,0,30);
				strncpy(last_time,"nerver",sizeof(last_time)-1);
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *) last_time,
									strlen(last_time));
		}
			break;

		case MODE_SET_RESERVE1:
		 //   if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
		  //	  netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
		  //  }
			break;
	
		case MODE_SET_RESERVE2:
			/* XXX malloc "undo" storage buffer */
		  //  if (/* XXX if malloc, or whatever, failed: */) {
	
		 // 	  netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
	
		  //  }
			break;
	
		case MODE_SET_FREE:
			/* XXX: free resources allocated in RESERVE1 and/or
			   RESERVE2.  Something failed somewhere, and the states
			   below won't be called. */
			break;
		
		case MODE_SET_ACTION:
		{
			#if 1
	
			time_t start_time;
			time(&start_time);
			char time_last[30] = { 0 }; 
			memset(time_last,0,30);
			FILE *fd = NULL;
			char * input_string = (char *)malloc(requests->requestvb->val_len+1);
			if(input_string)
			{
				memset(input_string,0,requests->requestvb->val_len+1);
				strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);
				if(strcmp(input_string,"1")==0)
				{
					memcpy(time_last,ctime(&start_time),30);
				}
			}

			snmp_log(LOG_DEBUG, "enter fopen\n");
			fd = fopen("/var/run/apache2/lastpoll.txt","w+");
			snmp_log(LOG_DEBUG, "exit fopen,fd=%p\n", fd);
			
			if(fd != NULL)
			{
				fputs(time_last,fd); 
				fclose(fd);
			}			
			FREE_OBJECT(input_string);
			#endif
		}	
		break;
		
		case MODE_SET_COMMIT:
			/* XXX: delete temporary storage */
		   // if (/* XXX: error? */) {
				/* try _really_really_ hard to never get to this point */
			//	  netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		   // }
			break;
	
		case MODE_SET_UNDO:
			/* XXX: UNDO and return to previous value for the object */
		   // if (/* XXX: error? */) {
				/* try _really_really_ hard to never get to this point */
		   //	  netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		   // }
			break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acLatestPollDevTime\n",reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLatestPollDevTime\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acLogServerAddr(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */
//#define LOG_CMD_FORMAT	"echo '%s' | sed 's/\\(.*\\)\\\"\\([0-9]\\{1,3\\}\\.[0-9]\\{1,3\\}\\.[0-9]\\{1,3\\}\\.[0-9]\\{1,3\\}\\)\\(.*\\)/%s%s%s/g'"
    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acLogServerAddr\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			//读取xml文件中第一个"NODE_ENABLES =1"项所对应的值
			u_long  ip_addr = 0;
			int flagz=0;
			char gets[50]={0};
			find_second_xmlnode(XML_FPATH,NODE_LOG,NODE_ENABLES,"1",&flagz);
			get_second_xmlnode(XML_FPATH,NODE_LOG,NODE_SYSIP,gets,flagz);
			snmp_log(LOG_DEBUG,"gets=%s\n",gets);
			
			INET_ATON(ip_addr,gets);
			snmp_log(LOG_DEBUG,"ip_addr=%d\n",ip_addr);
			
			snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS,
												(u_char *)&ip_addr,
												sizeof(ip_addr));		
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

         //       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

          //  }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

	case MODE_SET_ACTION:
	{
		//设置xml文件中第一个"NODE_ENABLES =1"项所对应的值
		SYSLOGALL_ST sysall;			
		memset(&sysall,0,sizeof(sysall));
		int flagz=0;
		char sysip[32]={0};		
		char gets[50]={0},tmp[50]={0};//tmp为des节点下value的值，如df_server_1294046665
		char porz[10]={0};
		char proipport[50]={0};
		memset(proipport,0,sizeof(proipport));
		memset(sysip,0,sizeof(sysip));
		
		int ip_addr;
		int port=0;
		ip_addr = *requests->requestvb->val.integer;
		INET_NTOA( ip_addr, sysip);
		
		//设置log
		find_second_xmlnode(XML_FPATH,NODE_LOG,NODE_ENABLES,"1",&flagz);
		get_second_xmlnode(XML_FPATH,NODE_LOG,CH_DEST,tmp,flagz);
		mod_second_xmlnode(XML_FPATH,NODE_LOG,NODE_SYSIP,sysip,flagz);

		//设置des
		find_second_xmlnode(XML_FPATH,NODE_DES,NODE_VALUE,tmp,&flagz);
		mod_second_xmlnode(XML_FPATH,NODE_DES,NODE_SYSIP,sysip,flagz);
		get_second_xmlnode(XML_FPATH,NODE_DES,NODE_PROTOCOLZ,porz,flagz);

		//设置content
		if(porz!=NULL)
			port=atoi(porz);
		snprintf(proipport,sizeof(proipport)-1,"%s(\"%s\" port(%d));",porz,sysip,port);
		mod_second_xmlnode(XML_FPATH,NODE_DES,NODE_CONTENT,proipport,flagz);
		 
		//读取xml文件，写入配置文件，并重启syslog
		save_syslog_file();
		restart_syslog();
	}   
	break;
        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
           // if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
            //    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
           // }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
           // if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
           //     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
           // }
            break;
		
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acLogServerAddr\n",reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLogServerAddr\n");
    return SNMP_ERR_NOERROR;
}
int 
handle_acSyslogServerPort(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acSyslogServerPort\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			//读取xml文件中第一个"NODE_ENABLES =1"项所对应的值
			int flagz=0,port=0;
			char gets[50]={0};
			find_second_xmlnode(XML_FPATH,NODE_LOG,NODE_ENABLES,"1",&flagz);
			get_second_xmlnode(XML_FPATH,NODE_LOG,NODE_SYSPORT,gets,flagz);
			if(gets!=NULL);
				port=atoi(gets);
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
											(u_char *)&port,
											sizeof(port));

		}
		break;

		/*
		* SET REQUEST
		*
		* multiple states in the transaction.  See:
		* http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
		*/
		case MODE_SET_RESERVE1:
		//    if (/* XXX: check incoming data in requests->requestvb->val.XXX forfailures, like an incorrect type or an illegal value or ... */) {
		//       netsnmp_set_request_error(reqinfo, requests, /* XXX: set errorcode depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ...*/);
		//   }
		break;

		case MODE_SET_RESERVE2:
		/* XXX malloc "undo" storage buffer */
		//  if (/* XXX if malloc, or whatever, failed: */) {
		//      netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
		// }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{	
			//设置xml文件中第一个"NODE_ENABLES =1"项所对应的值
			SYSLOGALL_ST sysall;			
			memset(&sysall,0,sizeof(sysall));
			int flagz=0;
			char port[10]={0},porz[10]={0};
			char ip[50]={0},tmp[50]={0};
			char proipport[50]={0};
			memset(proipport,0,sizeof(proipport));
			memset(port,0,sizeof(port));
			if((*requests->requestvb->val.integer > 0)&&(*requests->requestvb->val.integer < 65536))
			{
				snprintf(port,sizeof(port)-1,"%d",*requests->requestvb->val.integer);
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
			
			//设置log
			find_second_xmlnode(XML_FPATH,NODE_LOG,NODE_ENABLES,"1",&flagz);
			get_second_xmlnode(XML_FPATH,NODE_LOG,CH_DEST,tmp,flagz);
			mod_second_xmlnode(XML_FPATH,NODE_LOG,NODE_SYSPORT,port,flagz);

			//设置des
			find_second_xmlnode(XML_FPATH,NODE_DES,NODE_VALUE,tmp,&flagz);
			get_second_xmlnode(XML_FPATH,NODE_DES,NODE_PROTOCOLZ,porz,flagz);
			mod_second_xmlnode(XML_FPATH,NODE_DES,NODE_SYSPORT,port,flagz);
			
			//设置content
			get_second_xmlnode(XML_FPATH,NODE_DES,NODE_SYSIP,ip,flagz);
			snprintf(proipport,sizeof(proipport)-1,"%s(\"%s\" port(%d));",porz,ip,*requests->requestvb->val.integer);
			mod_second_xmlnode(XML_FPATH,NODE_DES,NODE_CONTENT,proipport,flagz);

			
			 
			//读取xml文件，写入配置文件，并重启syslog
			save_syslog_file();
			snmp_log(LOG_DEBUG,"restart\n");
			restart_syslog();
		}
		break;

		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		//  if (/* XXX: error? */) {
		//    /* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		//if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_UNDOFAILED);
		// }
		break;

		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNTPConfig\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
		}

	snmp_log(LOG_DEBUG, "exit handle_acSyslogServerPort\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acNTPConfig(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acNTPConfig\n");

	if(access(NTP_XML_FPATH,0)!=0)/*文件不存在*/
	{
		restart_ntp();
	}
	switch(reqinfo->mode) 
	{
		case MODE_GET:
		{
			int acNTPConfig = 0;
			int ret = 0;
			char sname[20] = {0};
			int flagz = 1;
       		if(access(NTP_XML_FPATH,0)==0)
       		{
				get_second_xmlnode(NTP_XML_FPATH,NTP_CLIZ,NTP_CIPZ,&sname,flagz);
				if (0 == strcmp(sname,""))
				{	
					acNTPConfig = 0;
					
				}
				else
				{
					INET_ATON(acNTPConfig, sname);
				}
       		}
			else
			{
				acNTPConfig = 0;
			}

			snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS,
										(u_char *)&acNTPConfig,
										sizeof(acNTPConfig));
			
		}
		break;

		/*
		* SET REQUEST
		*
		* multiple states in the transaction.  See:
		* http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
		*/
		case MODE_SET_RESERVE1:
		//    if (/* XXX: check incoming data in requests->requestvb->val.XXX forfailures, like an incorrect type or an illegal value or ... */) {
		//       netsnmp_set_request_error(reqinfo, requests, /* XXX: set errorcode depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ...*/);
		//   }
		break;

		case MODE_SET_RESERVE2:
		/* XXX malloc "undo" storage buffer */
		//  if (/* XXX if malloc, or whatever, failed: */) {
		//      netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
		// }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{
		/* XXX: perform the value change here */
		// if (/* XXX: error? */) {
		//    netsnmp_set_request_error(reqinfo, requests, /* some error */);
		// }
			int ret=-1,flagz = 1;
			ST_SYS_ALL sysall;
			memset(&sysall,0,sizeof(sysall));
			char fine_name[20] = {0};
			char sname[20] = {0};
			INET_NTOA(*requests->requestvb->val.integer,sname);

			get_second_xmlnode(NTP_XML_FPATH,NTP_CLIZ,NTP_CIPZ,&fine_name,flagz);
			
       		if (0 != strcmp(fine_name,""))
   			{
	       		mod_second_xmlnode(NTP_XML_FPATH,NTP_CLIZ,NTP_CIPZ,sname,flagz);
   			}
			else
			{
					snmp_log(LOG_DEBUG,"flagz=%d\n",flagz);
					add_ntp_client(NTP_XML_FPATH,sname, "");
			}
  			save_ntp_conf();
			reset_sigmask();
			restart_ntp();
		}
		break;

		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		//  if (/* XXX: error? */) {
		//    /* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		//if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_UNDOFAILED);
		// }
		break;

		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNTPConfig\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
		}

	snmp_log(LOG_DEBUG, "exit handle_acNTPConfig\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acTimeAfterNTPCal(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */
	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acTimeAfterNTPCal\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			time_t now;
			time(&now);					
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char*)&now,
									sizeof(long));
		}
		break;
		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acTimeAfterNTPCal\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acTimeAfterNTPCal\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acSTP(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acSTP\n");

    int acSTP =0 ;
    switch(reqinfo->mode) {

	case MODE_GET:
	{
		int state = 0;
		int stpmode = 0;
		unsigned int dist_slot = 0; 
		int function_type = -1;
		char file_path[64] = {0};
		DBusConnection *connection = NULL;
		int i = 0;
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			for(i = 1;i<16; i++)
			{
				memset(file_path,0,sizeof(file_path));
				sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
				function_type = get_product_info(file_path);
				if (function_type == 4)
				{
					dist_slot = i;
					break;
				}
			}
			
			get_slot_dbus_connection(dist_slot, &connection, SNMPD_INSTANCE_MASTER_V3);
				
			//state = ccgi_get_brg_g_state(&stpmode);
			state = ccgi_get_brg_g_state_slot(&stpmode,connection);
		}
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&state,
									sizeof(state));
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
          //      netsnmp_set_request_error(reqinfo, requests, /* XXX: set errorcode depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
          //  }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
           // if (/* XXX if malloc, or whatever, failed: */) {
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
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
		int state = 0;
		int stpmode = 0;
		unsigned int dist_slot = 0; 
		int function_type = -1;
		char file_path[64] = {0};
		DBusConnection *connection = NULL;
		unsigned int isEnable = 0;
		int i = 0;
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			for(i = 1;i<16; i++)
			{
				memset(file_path,0,sizeof(file_path));
				sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
				function_type = get_product_info(file_path);
				if (function_type == 4)
				{
					dist_slot = i;
					break;
				}
			}
			get_slot_dbus_connection(dist_slot, &connection, SNMPD_INSTANCE_MASTER_V3);
			if(*requests->requestvb->val.integer == 1)
			{
				snmp_log(LOG_DEBUG, "enter config_spanning_tree\n");
				isEnable = 1;
				//ret = config_spanning_tree("enable",connection);			
				ret = ccgi_enable_g_stp_to_protocol(DCLI_STP_M,isEnable,connection);
				ret = ccgi_set_bridge_force_version(STP_FORCE_VERS,connection);
				ccgi_enable_g_stp_to_protocol(DCLI_STP_M,isEnable,connection);
				
				snmp_log(LOG_DEBUG, "exit config_spanning_tree,ret=%d\n", ret);
			}
			else if(*requests->requestvb->val.integer == 0)
			{
				snmp_log(LOG_DEBUG, "enter config_spanning_tree\n");
				isEnable = 0;
				ret = ccgi_enable_g_stp_to_protocol(DCLI_STP_M,isEnable,connection);
				//ret =config_spanning_tree("disable",connection);
				snmp_log(LOG_DEBUG, "exit config_spanning_tree,ret=%d\n", ret);
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
			
			if(ret == -1)
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
		}
            /* XXX: perform the value change here */
          //  if (/* XXX: error? */) {
           //     netsnmp_set_request_error(reqinfo, requests, /* some error */);
           // }
           
       }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
           // if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_COMMITFAILED);
          //  }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
           // if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_UNDOFAILED);
           // }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSTP\n", reqinfo->mode);
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSTP\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSnmpTrapConfig(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acSnmpTrapConfig\n");
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char acSnmpTrapConfig[20] = { 0 };
			memset(acSnmpTrapConfig,0,20);
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)acSnmpTrapConfig,
										strlen(acSnmpTrapConfig));
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
           // }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
           // if (/* XXX if malloc, or whatever, failed: */) {
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
           // }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
            /* XXX: perform the value change here */
           // if (/* XXX: error? */) {
             //   netsnmp_set_request_error(reqinfo, requests, /* some error */);
           // }
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
           // if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_COMMITFAILED);
           // }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
            //if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
            //    netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_UNDOFAILED);
           // }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSnmpTrapConfig\n",reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSnmpTrapConfig\n");
    return SNMP_ERR_NOERROR;
}


int
handle_acStatWindowTime(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acStatWindowTime\n");
	
    switch(reqinfo->mode) {

       case MODE_GET:
	   {
			//只显示AP统计时长
			#if 0
			int ret = 0;
			int time = 900;
			struct ap_reportinterval_profile info;
			info.collect_time = 0;
			#endif
			
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
            	#if 0
    			snmp_log(LOG_DEBUG, "enter show_ap_moment_information_reportinterval_cmd\n");
    			ret = show_ap_moment_information_reportinterval_cmd(paraHead->parameter, paraHead->connection,&info);
    			snmp_log(LOG_DEBUG, "exit show_ap_moment_information_reportinterval_cmd,ret=%d\n", ret);
                if(( 1 == ret) && (info.collect_time > 0) && (info.collect_time <= 900))
    			{
    				time = info.collect_time;
    			}
    			else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(paraHead->parameter.slot_id);
        	    }
				#endif

				update_data_for_show_ap_moment_information_reportinterval_cmd(paraHead);
			}
            free_instance_parameter_list(&paraHead);

			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
								   (u_char *)&acStatWindowTime,
								   sizeof(acStatWindowTime));
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		/* XXX: perform the value change here */
		//  if (/* XXX: error? */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* some error */);
		//  }
		{
			int ret1 = AS_RTN_ERR, ret2 = 0;
			char value[10] = { 10 };

			if((*requests->requestvb->val.integer >= 5)&&(*requests->requestvb->val.integer <= 900))
			{
				
				snmp_log(LOG_DEBUG, "enter dbus_set_sample_param\n");
				ret1 = dbus_set_sample_param( ccgi_dbus_connection , AC_SAMPLE_PARAM_TYPE_STATISTICS, *requests->requestvb->val.integer );
				snmp_log(LOG_DEBUG, "exit dbus_set_sample_param,ret1=%d\n", ret1);
				
				memset(value,0,10);
				snprintf(value,sizeof(value)-1,"%d",*requests->requestvb->val.integer);
				instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
    				snmp_log(LOG_DEBUG, "enter set_ap_cpu_collect_time_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
    				                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret2 = set_ap_cpu_collect_time_cmd(paraNode->parameter, paraNode->connection, 0, value);
    				snmp_log(LOG_DEBUG, "exit set_ap_cpu_collect_time_cmd,ret2=%d\n", ret2);
				
    				if ( (AS_RTN_OK != ret1) || (1 != ret2))
    				{
    				    if(SNMPD_CONNECTION_ERROR == ret2) {
                            close_slot_dbus_connection(paraNode->parameter.slot_id);
                	    }
    					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
    				}
					else
					{
						acStatWindowTime = *requests->requestvb->val.integer;
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
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSoftwareVersion\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acStatWindowTime\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acHeartbeatPeriod(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acHeartbeatPeriod\n");
	
	switch(reqinfo->mode) {

        case MODE_GET:
	    {
			int acHeartbeatPeriod = 0;
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
						if(strcmp(parameter_array[i].paraStr,HEARTBEAT_INTERVAL)==0)
						{
							acHeartbeatPeriod = parameter_array[i].data;
							break;
						}
		            }
		        }
		
		        MANAGE_FREE(parameter_array);
			}
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&acHeartbeatPeriod,
									sizeof(acHeartbeatPeriod));
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{
			if((*requests->requestvb->val.integer>=0)&&(*requests->requestvb->val.integer<=3600))
			{
				instance_parameter *slotConnect_head = NULL, *slotConnect_node = NULL;				
				unsigned int data = *requests->requestvb->val.integer;
				int ret = AC_MANAGE_DBUS_ERROR;
				
				list_instance_parameter(&slotConnect_head, SNMPD_SLOT_CONNECT);
				for(slotConnect_node = slotConnect_head; NULL != slotConnect_node; slotConnect_node = slotConnect_node->next)
				{
					ret = ac_manage_config_trap_parameter(slotConnect_node->connection, HEARTBEAT_INTERVAL, data);					
					if(AC_MANAGE_SUCCESS != ret) 
					{
						netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
					}
				}
				free_instance_parameter_list(&slotConnect_head);
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSoftwareVersion\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acHeartbeatPeriod\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSysRestart(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */
	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acSysRestart\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			int number = 1;
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&number,
										sizeof(number));
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{
			int reset = 0;
			
			reset = *requests->requestvb->val.integer;
			if(reset == 2)
			{
			    if(0 == snmp_cllection_mode(ccgi_dbus_connection)) {
				    system("reboot.sh > /dev/null");
                }
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
						
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;

		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSysRestart\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acSysRestart\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acSysReset(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */
	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acSysReset\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			int number = 1;
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&number,
									sizeof(number));
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		/* XXX: perform the value change here */
		//  if (/* XXX: error? */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* some error */);
		//  }
		{
			int reset = 0;
			int status_d = 1;

			reset = *requests->requestvb->val.integer;
			if(reset == 2)
			{
				status_d=system("sudo earse.sh  > /dev/null");
								
				if(status_d!=0)
				{
					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
				}
				else
				{
					system("reboot.sh > /dev/null");
				}
			}
			/*else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}*/			
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;

		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acContactInfo\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acSysReset\n");
	return SNMP_ERR_NOERROR;
}

int
handle_TimeSynPeriod(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_TimeSynPeriod\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int day = 0;
			char *p = NULL;
			char *time = NULL;
			char gets_cronttab[10] = {0};
			if_ntp_exist();
			get_first_xmlnode(NTP_XML_FPATH, "cront",gets_cronttab);
				p = strtok(gets_cronttab, "^"); 
				time = p;
				p = strtok(NULL, "^"); 
				if(p)
				{
					if(strcmp(p,"days") == 0)
					{
						day = atoi(time);
					}
					else
						day = 0;
				}

			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&day,
									sizeof(day));
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{
			long day = 0;
			char time_t[10] = {0};	
			char cmd[128]  = {0};
			if_ntp_exist();
			day = *requests->requestvb->val.integer;
			if(day > 0 && day < 32){
				sprintf(time_t,"%d^%s",day,"days");
				mod_first_xmlnode(NTP_XML_FPATH, "cront",time_t);
				save_ntp_conf ();
				strcat(cmd,"sudo cronntp.sh > /dev/null");
				int status = system(cmd);
				if(status == -1)
				{
					return SNMP_ERR_GENERR;
				}
			}
			else
				return SNMP_ERR_GENERR;
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_TimeSynPeriod\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_TimeSynPeriod\n");
    return SNMP_ERR_NOERROR;
}

int
handle_SyslogSvcEnable(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */
	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_SyslogSvcEnable\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{  
			int sedis=0;int value = 1;
			sedis=if_syslog_enable();
			if(sedis==0)
			{
				value=2;
			}			
		    snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&value,
									sizeof(value));			
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{
			FILE *fp=NULL;
			char syslog_status[10]={0};
			if(( fp = fopen( STATUS_FPATH ,"w+"))!=NULL)
			{			
				char cmd[1024]={0};
				if (*requests->requestvb->val.integer==1)
				{
					strncpy(cmd, "sudo /opt/services/init/syslog_init start",sizeof(cmd)-1);
					strncpy( syslog_status, "start" ,sizeof(syslog_status)-1);
				}
				else if(*requests->requestvb->val.integer==2)
				{
					strncpy(cmd, "sudo /opt/services/init/syslog_init stop",sizeof(cmd)-1);
					strncpy( syslog_status, "stop" ,sizeof(syslog_status)-1);
				}
				int status = system(cmd);	 
			    int ret = WEXITSTATUS(status);
				if (0 != ret)					
					
				strncpy( syslog_status, (*requests->requestvb->val.integer==2)?"stop":"start",sizeof(syslog_status)-1);
				fwrite( syslog_status, strlen(syslog_status), 1, fp );
				mod_first_xmlnode(XML_FPATH, NODE_LSTATUS, syslog_status);
									
				fflush(fp);
				if(fp != NULL)
				{
					fclose(fp);
					fp = NULL;
				}
			}				
		}		   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;
		
		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in SyslogSvcEnable\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_SyslogSvcEnable\n");
	return SNMP_ERR_NOERROR;
}

int
handle_SyslogReportEventLevel(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */
	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_SyslogReportEventLevel\n");

	if_syslog_exist();
	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{  		
		   int value =0,flagz = 0;
	
		   char gets[50]={0};
		   find_second_xmlnode(XML_FPATH,NODE_LOG,NODE_ENABLES,"1",&flagz);
		   get_second_xmlnode(XML_FPATH,NODE_LOG,CH_DEST,gets,flagz);
		   snmp_log(LOG_DEBUG,"gets=%s,flagz=%d\n",gets,flagz);
		   if(gets)
		   {
				strtok(gets,";");
		   }
		   find_second_xmlnode(XML_FPATH,NODE_DES,NODE_VALUE,gets,&flagz);
		   memset(gets,0,sizeof(gets)-1);
		   get_second_xmlnode(XML_FPATH,NODE_DES,NODE_FLEVEL,gets,flagz);
		   snmp_log(LOG_DEBUG,"gets=%s\n",gets);

		   if(gets!=NULL)
		   		value = atoi(gets);
		
		   
		   snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&value,
									sizeof(value));		   
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

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		{
			SYSLOGALL_ST sysall;			
			memset(&sysall,0,sizeof(sysall));
			int flagz = 0;
			char gets[50]={0};
			char level[5]={0};
			snprintf(level,sizeof(level)-1,"%d",*requests->requestvb->val.integer);
			snmp_log(LOG_DEBUG,"level=%d\n",level);
			if((*requests->requestvb->val.integer>0)&&(*requests->requestvb->val.integer<7))
			{
				find_second_xmlnode(XML_FPATH,NODE_LOG,NODE_ENABLES,"1",&flagz);
		  	    get_second_xmlnode(XML_FPATH,NODE_LOG,CH_DEST,gets,flagz);
		 	    snmp_log(LOG_DEBUG,"gets=%s,flagz=%d\n",gets,flagz);							
				
				
				switch(*requests->requestvb->val.integer)
				{	
					case 6:mod_second_xmlnode(XML_FPATH,NODE_LOG,NODE_FILTER,"f_debug",flagz);
						   break;
					case 5:mod_second_xmlnode(XML_FPATH,NODE_LOG,NODE_FILTER,"f_at_least_crit",flagz);
						   break;
					case 4:mod_second_xmlnode(XML_FPATH,NODE_LOG,NODE_FILTER,"f_at_least_err",flagz);
						   break;
					case 3:mod_second_xmlnode(XML_FPATH,NODE_LOG,NODE_FILTER,"f_at_least_warn",flagz);
						   break;
					case 2:mod_second_xmlnode(XML_FPATH,NODE_LOG,NODE_FILTER,"f_at_least_notice",flagz);
						   break;
					case 1:mod_second_xmlnode(XML_FPATH,NODE_LOG,NODE_FILTER,"f_at_least_info",flagz);
						   break;
				}
				
				find_second_xmlnode(XML_FPATH,NODE_DES,NODE_VALUE,gets,&flagz);				
		   		mod_second_xmlnode(XML_FPATH,NODE_DES,NODE_FLEVEL,level,flagz);
				
				read_syslogall_st(XML_FPATH, &sysall);
				write_config_syslogallst(&sysall, CONF_FPATH);
				Free_read_syslogall_st(&sysall);
				restart_syslog();
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;
		
		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_SyslogReportEventLevel\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_SyslogReportEventLevel\n");
	return SNMP_ERR_NOERROR;
}

int
handle_SysObjectId(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */
	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_SysObjectId\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			STSNMPSysInfo snmpInfo = { 0 };
			int ret = ac_manage_show_snmp_base_info(ccgi_dbus_connection, &snmpInfo);
			if(AC_MANAGE_SUCCESS == ret)
			{
				snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)snmpInfo.sys_oid,
										strlen(snmpInfo.sys_oid));
			}
			else
			{
				snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)"",
										strlen(""));
			}
		}
		break;
		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_SysObjectId\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_SysObjectId\n");
	return SNMP_ERR_NOERROR;
}
int handle_acHeartBeatEnable(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_acHeartBeatEnable\n");

	switch(reqinfo->mode) 
		{

		case MODE_GET:
		{		    
			int heart_switch = 0;
			TRAP_DETAIL_CONFIG *trapDetail_array = NULL;
		    unsigned int trapDetail_num = 0;
	
			int ret = ac_manage_show_trap_switch(ccgi_dbus_connection, &trapDetail_array, &trapDetail_num);
			if(AC_MANAGE_SUCCESS == ret) {
				heart_switch = trapDetail_array[87].trapSwitch;
				MANAGE_FREE(trapDetail_array);
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&heart_switch,
									sizeof(heart_switch));
		}
		break;

		  case MODE_SET_RESERVE1:
			  //  if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
			  //	  netsnmp_set_request_error(reqinfo, requests, /* XXX: set errorcode depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
			  //  }
				break;
		
			case MODE_SET_RESERVE2:
				/* XXX malloc "undo" storage buffer */
			   // if (/* XXX if malloc, or whatever, failed: */) {
			   //	  netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
			  //  }
				break;
		
			case MODE_SET_FREE:
				/* XXX: free resources allocated in RESERVE1 and/or
				   RESERVE2.  Something failed somewhere, and the states
				   below won't be called. */
				break;
		
			case MODE_SET_ACTION:
			{
				TRAP_DETAIL_CONFIG trapConf = { 0 };
			    if(1 == *requests->requestvb->val.integer) {
			        trapConf.trapSwitch = 1;
			    }
			    else {
			        trapConf.trapSwitch = 0;
			    }

			    int ret = ac_manage_config_trap_switch(ccgi_dbus_connection, 87, &trapConf);
			    if(AC_MANAGE_SUCCESS != ret) 
				{
					netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			    }							   
			   }
				break;
		
			case MODE_SET_COMMIT:
				/* XXX: delete temporary storage */
			   // if (/* XXX: error? */) {
					/* try _really_really_ hard to never get to this point */
			   //	  netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_COMMITFAILED);
			  //  }
				break;
		
			case MODE_SET_UNDO:
				/* XXX: UNDO and return to previous value for the object */
			   // if (/* XXX: error? */) {
					/* try _really_really_ hard to never get to this point */
			   //	  netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_UNDOFAILED);
			   // }
				break;


		
		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_SysObjectId\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acHeartBeatEnable\n");
	return SNMP_ERR_NOERROR;

}
int
handle_NormalCollectCycle(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_NormalCollectCycle\n");
	
	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{       
			#if 0
			int ret = 0;
			int time = 120;
			struct ap_reportinterval_profile info;
			info.routine_report_value = 0;
			#endif
			
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {    
        		#if 0    
    			snmp_log(LOG_DEBUG, "enter show_ap_moment_information_reportinterval_cmd\n");
    			ret = show_ap_moment_information_reportinterval_cmd(paraHead->parameter, paraHead->connection,&info);
    			snmp_log(LOG_DEBUG, "exit show_ap_moment_information_reportinterval_cmd,ret=%d\n", ret);
    			
    			//当time值超过120之后，显示120
    			if((1 == ret)&&(9 < info.routine_report_value)&&(info.routine_report_value < 121))
    			{
    				time = info.routine_report_value;
    			}
    			else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(paraHead->parameter.slot_id);
        	    }
				#endif

				update_data_for_show_ap_moment_information_reportinterval_cmd(paraHead);
    		}	
            free_instance_parameter_list(&paraHead);
            
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
												(u_char *)&NormalCollectCycle,
												sizeof(NormalCollectCycle));
		}
		break;
	    case MODE_SET_RESERVE1:

		break;

		case MODE_SET_RESERVE2:

		break;

		case MODE_SET_FREE:
		break;

		case MODE_SET_ACTION:
		{
			int ret = 0;
            struct WtpCollectInfo *head = NULL;			
			struct WtpCollectInfo *ShowNode = NULL;
			
			int time = 0;
			int ret1 = 0;
			char value[10] = { 0 }; 
			time = *requests->requestvb->val.integer;
			
		    if((time > 9) && (time < 121))
		    {
				memset(value,0,10);
				snprintf(value,sizeof(value)-1,"%d",time);
                
				instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                    
    				snmp_log(LOG_DEBUG, "enter show_all_wtp_collect_information_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
    				                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret = show_all_wtp_collect_information_cmd(paraNode->parameter, paraNode->connection,&head);
    				snmp_log(LOG_DEBUG, "exit show_all_wtp_collect_information_cmd,ret=%d\n", ret);
				
    				if((NULL != head) && ( 1 == ret ))
    				{
    					for(ShowNode = head->WtpCollectInfo_list; NULL != ShowNode; ShowNode = ShowNode->next)
    					{
    						if(0 == ShowNode->wtpMomentCollectSwith)//判断事实采集开关是否关闭，如果关闭，修改exation的值
    						{
    							set_ap_extension_infomation_reportinterval(paraNode->parameter, paraNode->connection,ShowNode->wtpCurrID,value);
    							
    						}
    					}
    				}
    				Free_show_all_wtp_collect_information_cmd(head);

    				snmp_log(LOG_DEBUG, "enter set_ap_routine_infomation_reportinterval_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
    				                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    			    ret1 = set_ap_routine_infomation_reportinterval_cmd(paraNode->parameter, paraNode->connection,value);
    				snmp_log(LOG_DEBUG, "exit set_ap_routine_infomation_reportinterval_cmd,ret1=%d\n", ret1);
    				
					if( 1 == ret1)
					{
						NormalCollectCycle = time;
					}
					else
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret1) {
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
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_NormalCollectCycle\n");
	return SNMP_ERR_NOERROR;
}

int
handle_RtCollectCycle (netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{ 
	snmp_log(LOG_DEBUG, "enter handle_RtCollectCycle\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			#if 0
			int ret = 0;
			int time = 5;
			struct ap_reportinterval_profile info;
			info.moment_report_value = 0;
			#endif
            
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {    
            	#if 0
    			snmp_log(LOG_DEBUG, "enter show_ap_moment_information_reportinterval_cmd\n");
    			ret = show_ap_moment_information_reportinterval_cmd(paraHead->parameter, paraHead->connection,&info);
    			snmp_log(LOG_DEBUG, "exit show_ap_moment_information_reportinterval_cmd,ret=%d\n", ret);
    			
    			//当time值超过5之后，显示5
    			if((1 == ret) && (1 < info.moment_report_value) && (info.moment_report_value < 6))
    			{
    				time = info.moment_report_value;
    			}
				#endif

				update_data_for_show_ap_moment_information_reportinterval_cmd(paraHead);
            }
            free_instance_parameter_list(&paraHead);
            
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&RtCollectCycle,
									sizeof(RtCollectCycle));
		}
		break;
		case MODE_SET_RESERVE1:

		break;

		case MODE_SET_RESERVE2:

		break;

		case MODE_SET_FREE:
		break;
		case MODE_SET_ACTION:
		{
	        struct WtpCollectInfo *head = NULL;			
			struct WtpCollectInfo *ShowNode = NULL;
			int num = 0;
			int ret = 0;

			
			int time = 0;
			int ret1 = 0;
			int test = 0;
			char value[10] = { 0 }; 
			time = *requests->requestvb->val.integer;
			
		    if((time > 1) && (time < 6))
		    {
		    	memset(value,0,10);
				snprintf(value,sizeof(value)-1,"%d",time);
				
				instance_parameter *paraHead = NULL, *paraNode = NULL;
                list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
                for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                    
    				snmp_log(LOG_DEBUG, "enter show_all_wtp_collect_information_cmd, slot %d, local_id = %d, instanec_id = %d\n", 
    				                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
    				ret = show_all_wtp_collect_information_cmd(paraNode->parameter, paraNode->connection,&head);
    				snmp_log(LOG_DEBUG, "exit show_all_wtp_collect_information_cmd,ret=%d\n", ret);

				if((NULL != head) && ( 1 == ret ))
    				{
    					for(ShowNode = head->WtpCollectInfo_list; NULL != ShowNode; ShowNode = ShowNode->next)
    					{
    						if(1 == ShowNode->wtpMomentCollectSwith)//判断实时采集开关是否打开，如果打开，修改exation的值
    						{
    							set_ap_extension_infomation_reportinterval(paraNode->parameter, paraNode->connection,ShowNode->wtpCurrID,value);
    						}
    					}
    				}
    				Free_show_all_wtp_collect_information_cmd(head);
    				
    				ret1 = set_ap_moment_infomation_reportinterval_cmd(paraNode->parameter, paraNode->connection,value);
					if( 1 == ret1)
					{
						RtCollectCycle = time;
					}
					else
    				{	
    				    if(SNMPD_CONNECTION_ERROR == ret1) {
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
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_SysObjectId\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_RtCollectCycle\n");
	return SNMP_ERR_NOERROR;
}

int
handle_acSoftwareHwSpecr(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acSoftwareHwSpecr\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{   
			char value[256] = { 0 };
			memset(value,0,256);
			char fun[30] = { 0 };
			memset(fun,0,30);
			char sun[30] = { 0 };
			memset(sun,0,30);
			char pun[100] = { 0 };
			memset(pun,0,100);
			char mun[30] = { 0 };
			memset(mun,0,30);
			FILE *fp = NULL;
			fp = fopen("/devinfo/software_name","r");
			if(fp != NULL)
			{
				fgets(fun,30,fp);
				fclose(fp);
			}
			FILE *sp = NULL;
			sp = fopen("/etc/version/verstring","r");
			if(sp != NULL)
			{
				fgets(sun,30,sp);
				fclose(sp);
			}
			FILE *pp =NULL;
			pp = fopen("/devinfo/product_name","r");
			if (pp !=NULL)
			{
				fgets(pun,100,pp);
				fclose(pp);
			}
			FILE *mp = NULL;
			mp = fopen("/devinfo/local_mac","r");
				if (mp !=NULL)
			{
				fgets(mun,30,mp);
				fclose(mp);
			}
			delete_enter(fun);
			delete_enter(sun);
			delete_enter(pun);
			delete_enter(mun);
			strncpy(value,"Software:",sizeof(value)-1);
			strncat(value,fun,sizeof(value)-strlen(value)-1);
			strncat(value," ",sizeof(value)-strlen(value)-1);
			strncat(value,sun,sizeof(value)-strlen(value)-1);
			strncat(value," Hardware:",sizeof(value)-strlen(value)-1);
			strncat(value,pun,sizeof(value)-strlen(value)-1);
			if(strcmp(mun,"")!=0)
			{
				strncat(value," MAC:",sizeof(value)-strlen(value)-1);
				strncat(value,mun,sizeof(value)-strlen(value)-1);				
			}
			delete_enter(value);
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)value,
                                     strlen(value));			
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSoftwareName\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSoftwareHwSpecr\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSoftwareBigVersion(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acSoftwareBigVersion\n");

    switch(reqinfo->mode) {

       case MODE_GET:
	   {
			char bigversion[20];
			memset(bigversion,0,sizeof(bigversion));
			FILE *fver = NULL;
			fver = fopen("/etc/version/version","r");
			if( NULL != fver )
			{
				fgets(bigversion,4,fver);
				fclose(fver);
			}

			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
			                         (u_char *)bigversion,
			                         strlen(bigversion));
			
       }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSoftwareBigVersion\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSoftwareBigVersion\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acBootImg(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acBootImg\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char acBootImg[128] = { 0 };
			char imgname[128];
			int ret;
			memset(imgname,0,128);

			snmp_log(LOG_DEBUG, "enter get_boot_img_name\n");
			ret = get_boot_img_name(imgname);
			snmp_log(LOG_DEBUG, "exit get_boot_img_name,ret=%d\n", ret);
			
			if(0==ret)
			{
				memset(acBootImg,0,128);
				strncpy(acBootImg,imgname,sizeof(acBootImg)-1);
				delete_enter(acBootImg);
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)acBootImg,
										strlen(acBootImg));
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
           // }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
           // if (/* XXX if malloc, or whatever, failed: */) {
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
           // }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{
			int ret = 0;

			snmp_log(LOG_DEBUG, "enter config_boot_img_func_cmd\n");
			ret  = config_boot_img_func_cmd(requests->requestvb->val.string);
			snmp_log(LOG_DEBUG, "exit config_boot_img_func_cmd,ret=%d\n", ret);
			
			if(ret != 1)
		    {
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
		    }
		}
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
           // if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_COMMITFAILED);
           // }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
            //if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
            //    netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_UNDOFAILED);
           // }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acBootImg\n",reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acBootImg\n");
    return SNMP_ERR_NOERROR;
}
handle_acWriteConfig(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acWriteConfig\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{	
			char config[50];
			memset(config,0,sizeof(config));
			if(access("/var/run/writeconfig.tmp",0) == 0)
				{
					FILE *fp_config = NULL;
			   
					fp_config = fopen("/var/run/writeconfig.tmp","r");
					if(NULL != fp_config)
					{
						fgets(config,sizeof(config),fp_config);						
						fclose(fp_config);
					}
				}
			else
				{
					strncpy(config,"/blk/conf_xml.conf",sizeof(config)-1);
				}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)config,
										strlen(config));
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
           // }
            break;

        case MODE_SET_RESERVE2:
            /* XXX malloc "undo" storage buffer */
           // if (/* XXX if malloc, or whatever, failed: */) {
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);
           // }
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{
			FILE *info = NULL;
			char time[50] = {0};
			memset(time,0,50);
			info = popen("date +%X","r");

			if(info != NULL)
			{
				fgets(time,50,info);
				delete_enter(time);
				char * tmp = NULL;
				int i = 0;
				tmp = time;
				while(tmp[i] != '+')
				{
					tmp++;
					i++;
				}
				tmp[i] = ' ';
				pclose(info);
			}		
			snmp_log(LOG_DEBUG,"time=%s\n",time);
			
			unsigned char cmd[DEFAULT_LEN];
			if(strcmp(requests->requestvb->val.string,"1")!=0)
		    {
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
		    }
			else
			{
				int status = system("save_config.sh > /dev/null");
				int ret = WEXITSTATUS(status);
				if(ret==0)
				{			
					snprintf(cmd,sizeof(cmd)-1,"sudo echo %s%s > /var/run/writeconfig.tmp","config has saved by mib at ",time);
					system(cmd);
				}
			}

		}
            break;

        case MODE_SET_COMMIT:
            /* XXX: delete temporary storage */
           // if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
           //     netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_COMMITFAILED);
           // }
            break;

        case MODE_SET_UNDO:
            /* XXX: UNDO and return to previous value for the object */
            //if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
            //    netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_UNDOFAILED);
           // }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acBootImg\n",reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acWriteConfig\n");
    return SNMP_ERR_NOERROR;
}

int
handle_sysBackupIdentity(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    
    switch(reqinfo->mode) {
		
        case MODE_GET:
			{
				update_data_for_show_backupidentity_information();	
	            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
	                                     (u_char *) &identi,
	                                     sizeof(identi));
				//Free_read_acinfo_xml(&ahead);
	        }
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_sysBackupIdentity\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_sysBackupMode(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    
    switch(reqinfo->mode) {

        case MODE_GET:
				{
					update_data_for_show_backupidentity_information();
		            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
		                                     (u_char *) &mode,
		                                     sizeof(mode));
		        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_sysBackupMode\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_sysBackupStatus(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    
    switch(reqinfo->mode) {
        case MODE_GET:
			{
				update_data_for_show_backupidentity_information();
	            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
	                                     (u_char *) &status,
	                                     sizeof(status));
			}
            	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_sysBackupStatus\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    	}

    return SNMP_ERR_NOERROR;
}
int
handle_sysBackupNetworManageIp(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    
    switch(reqinfo->mode) {

        case MODE_GET:
			{
				update_data_for_show_backupidentity_information();
	            snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS,
	                                     (u_char *) &ipaddr,
	                                     sizeof(ipaddr));
	        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_sysBackupNetworManageIp\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_sysBackupSwitchTimes(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    
	snmp_log(LOG_DEBUG, "enter handle_sysBackupSwitchTimes\n");
    switch(reqinfo->mode) {

        case MODE_GET:
			{
				unsigned long times = 0;
				unsigned int vrrp_id = 1;
				for(vrrp_id = 1; vrrp_id < 17; vrrp_id++) 
				{
					unsigned long switch_times  = 0;
					if(0 == show_vrrp_switch_times(vrrp_id, &switch_times)) 
					{
						times += switch_times;
					}
				}

            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                     (u_char *) &times,
                                    sizeof(times));
			}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_sysBackupSwitchTimes\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}
int
handle_sysBackupNetworManageIpv6(netsnmp_mib_handler *handler,
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
		update_data_for_show_backupidentity_information();
           	snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) ipv6address,
                                     strlen(ipv6address));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_sysBackupNetworManageIpv6\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acslotinfo(netsnmp_mib_handler *handler,
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
		ac_slot_information();
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) acslotinfo,
                                     strlen(acslotinfo));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acslotinfo\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

static void update_data_for_show_sys_ver()
{
	struct sysinfo info;
	
	if(0 != update_time_show_sys_ver)
	{
		sysinfo(&info); 	
		if(info.uptime - update_time_show_sys_ver < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_sys_ver\n");
	
	/*update cache data*/
	int ret = -1;
	struct sys_ver ptrsysver;/*产品系统信息*/

	memset(acSoftwareName,0,sizeof(acSoftwareName));
	memset(acSoftwareVersion,0,sizeof(acSoftwareVersion));
	memset(acProductName,0,sizeof(acProductName));
	
	snmp_log(LOG_DEBUG, "enter show_sys_ver\n");
	ret = show_sys_ver(&ptrsysver);
	snmp_log(LOG_DEBUG, "exit show_sys_ver,ret=%d\n",ret);
	if(ret==0)
	{
		snprintf(acSoftwareName, sizeof(acSoftwareName)-1, "%s", ptrsysver.sw_name);
		delete_enter(acSoftwareName);
		strncpy(acSoftwareVersion, ptrsysver.sw_version_str, sizeof(acSoftwareVersion)-1);
		delete_enter(acSoftwareVersion);
		snprintf(acProductName, sizeof(acProductName)-1, "%s", ptrsysver.sw_product_name);
		delete_enter(acProductName);
		
		FREE_OBJECT(ptrsysver.product_name);
		FREE_OBJECT(ptrsysver.base_mac);
		FREE_OBJECT(ptrsysver.serial_no);
		FREE_OBJECT(ptrsysver.swname);
	}
	
	sysinfo(&info); 		
	update_time_show_sys_ver = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_sys_ver\n");
}

static void update_data_for_show_ap_moment_information_reportinterval_cmd(instance_parameter *paraHead)
{
	struct sysinfo info;
	
	if(0 != update_time_show_ap_moment_information_reportinterval_cmd)
	{
		sysinfo(&info); 	
		if(info.uptime - update_time_show_ap_moment_information_reportinterval_cmd < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_ap_moment_information_reportinterval_cmd\n");
	
	/*update cache data*/
	int ret = 0;
	struct ap_reportinterval_profile reportinterval_info;

	acSampleTime = 5;
	acStatWindowTime = 900;
	NormalCollectCycle = 120;
	RtCollectCycle = 5;
	
	snmp_log(LOG_DEBUG, "enter show_ap_moment_information_reportinterval_cmd\n");
	ret = show_ap_moment_information_reportinterval_cmd(paraHead->parameter, paraHead->connection, &reportinterval_info);
	snmp_log(LOG_DEBUG, "exit show_ap_moment_information_reportinterval_cmd,ret=%d\n",ret);
	if(ret == 1)
	{
		if((reportinterval_info.sample_time> 1) && (reportinterval_info.sample_time <= 900))
		{
			acSampleTime = reportinterval_info.sample_time;
		}
		if((reportinterval_info.collect_time > 0) && (reportinterval_info.collect_time <= 900))
		{
			acStatWindowTime = reportinterval_info.collect_time;
		}
		if((9 < reportinterval_info.routine_report_value)&&(reportinterval_info.routine_report_value < 121))
		{
			NormalCollectCycle = reportinterval_info.routine_report_value;
		}
		if((1 < reportinterval_info.moment_report_value) && (reportinterval_info.moment_report_value < 6))
		{
			RtCollectCycle = reportinterval_info.moment_report_value;
		}
	}
	else if(SNMPD_CONNECTION_ERROR == ret) 
	{
        close_slot_dbus_connection(paraHead->parameter.slot_id);
    }
	
	sysinfo(&info); 		
	update_time_show_ap_moment_information_reportinterval_cmd = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_ap_moment_information_reportinterval_cmd\n");
}

static void update_data_for_show_backupidentity_information()
{

	struct sysinfo info;
	if(0 != update_time_show_backupidentity_information)
	{
		sysinfo(&info); 	
		if(info.uptime - update_time_show_backupidentity_information < cache_time)
		{
			return;
		}
	}
	snmp_log(LOG_DEBUG, "enter update_data_for_show_backupidentity_information\n");

	char *ip = NULL;
	int confnum = 0;
	int retu = 0;
	char hansi_id[10] = {10};
	struct acbackup_st ahead;
	struct netipbk_st *aq = NULL;
	
	if(access(ACBACKUPFILE,0) != 0)
	{
//		new_xml_file(ACBACKUPFILE);
		return 0;
	}
	memset(&ahead,0,sizeof(struct acbackup_st));
	retu = read_acinfo_xml(&ahead,&confnum);
	if(0 == retu)
	{
		if(strcmp("master",ahead.identity) == 0)
			identi = 2;
		else if(strcmp("slave", ahead.identity) == 0)
			identi = 1;
		else
			identi = 0;
		if(strcmp("hot-backup",ahead.mode) == 0)
			mode = 2;
		else if(strcmp("cold-backup", ahead.mode) == 0)
			mode = 1;
		else
			mode = 0;
		if(strcmp("enable",ahead.status) == 0)
			status = 2;
		else if(strcmp("disable", ahead.status) == 0)
			status = 1;
		else
			status = 0;
		if(2 == status)
		{	
			instance_parameter *para_head = NULL, *para_node = NULL;
			list_instance_parameter(&para_head, SNMPD_INSTANCE_MASTER);
			if(para_head)
			{
				for(para_node = para_head; NULL != para_node; para_node = para_node->next)
				{
					snprintf(hansi_id,sizeof(hansi_id)-1,"%d-%d-%d",para_node->parameter.slot_id,para_node->parameter.local_id,para_node->parameter.instance_id);
					get_ip_by_active_instance(hansi_id,&ipaddr);
					if(ipaddr != 0)
					{
						break;
					}
				}
				for(para_node = para_head; NULL != para_node; para_node = para_node->next)
				{
					snprintf(hansi_id,sizeof(hansi_id)-1,"%d-%d-%d",para_node->parameter.slot_id,para_node->parameter.local_id,para_node->parameter.instance_id);
					get_ipv6_by_active_instance(hansi_id,ipv6address);
					if(strlen(ipv6address) != 0)
					{
						break;
					}
				}
			}
			free_instance_parameter_list(&para_head);
		}
		else
		{
			ipaddr = 0;
			strcpy(ipv6address,"::");
		}
	}
	Free_read_acinfo_xml(&ahead);
	sysinfo(&info); 		
	update_time_show_backupidentity_information = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for update_data_for_show_backupidentity_information\n");


}

static void ac_slot_information()
{

	struct sysinfo info;
	FILE *fd_count = NULL;
	FILE *fd = NULL;
	int slot_int=0;
	int slot_count=0;
	char up[35]={0};
	char down[35]={0};
	if(0 != update_time_show_backupidentity_information)
	{
		sysinfo(&info); 	
		if(info.uptime - update_time_show_backupidentity_information < cache_time)
		{
			return;
		}
	}
	snmp_log(LOG_DEBUG, "enter ac_slot_information\n");

	memset(acslotinfo,0,sizeof(acslotinfo))	;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		fd = fopen("/dbm/product/board_on_mask", "r");
		fd_count = fopen("/dbm/product/slotcount", "r");
	}
	if ((fd == NULL)||(fd_count == NULL))
	{
		snmp_log(LOG_DEBUG,"Get production information [2] error\n");
	}
	if((fd)&&(fd_count))
	{
		int i=1;
		int j=1;
		int h=0;
		char tmp[6]={0};
		fscanf(fd, "%d", &slot_int);
		fclose(fd);
		fscanf(fd_count, "%d", &slot_count);
		fclose(fd_count);
		for(i=1;i<(slot_count+1);i++)
		{
			if(slot_int&j)
			{
				if(h==0)
				{
					sprintf(tmp,"%d",i);
				}
				else
				{
					sprintf(tmp,",%d",i);

				}
				strcat(up,tmp);
				h=h+1;
			}
			else
			{
				sprintf(tmp,",%d",i);
				strcat(down,tmp);
			}
			j=j<<1;
		}
		strcat(acslotinfo,up);
		strcat(acslotinfo," is up  ");
		if(strlen(down)!=0)
		{
			strcat(acslotinfo,down);
			strcat(acslotinfo," is down");
		}
	}

	sysinfo(&info); 		
	update_time_show_backupidentity_information = info.uptime;

	snmp_log(LOG_DEBUG, "exit  ac_slot_information\n");


}

