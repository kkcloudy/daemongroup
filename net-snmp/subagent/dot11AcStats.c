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
* dot11AcStats.c
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
#include "dot11AcStats.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "nm/app/eag/eag_conf.h"
#include "nm/public/nm_list.h"
#include "nm/app/eag/eag_interface.h"
#include "ws_dcli_wlans.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"
#include "ws_dcli_portconf.h"
#include "ws_dcli_ac_roaming.h"
#include "mibs_public.h"
#include "ws_sta.h"
#include "autelanWtpGroup.h"
#include "ws_user_manage.h"
#include "ws_public.h"
#include "ws_dbus_list_interface.h"
#include <sys/time.h>
/** Initializes the dot11AcStats module */

#define ACNUMAPINAC  				"2.3.1.1"
#define	ACONLINENUMAPINAC			"2.3.1.2"
#define	ACUSERSONAC					"2.3.1.3"
#define	ACUPLINKDATATHROUGHPUT		"2.3.1.4"
#define	ACDOWNLINKDATATHROUGHPUT	"2.3.1.5"
#define	ACPORTTHROUGHPUT			"2.3.1.6"
#define	ACINACROAMINGSUCCRATE		"2.3.1.7"
#define	ACOUTACROAMINGSUCCRATE		"2.3.1.8"
#define	ACAUTHREQCNT				"2.3.1.9"
#define	ACAUTHSUCCNT				"2.3.1.10"
#define	ACAUTHFAILCNT				"2.3.1.11"
#define	ACAUTHREJECTCNT				"2.3.1.12"
#define	ACAUTHATTEMPTCNT			"2.3.1.13"
#define	ACSTAALLREASSOC				"2.3.1.14"
#define ACSTATUS					"2.3.1.15"
#define ACSTANDBYSWITCH				"2.3.1.16"
#define ACBANDWIDTHAVGUTILIZATION	"2.3.1.17"
#define ACLOSTPKTSAVGUTILIZATION	"2.3.1.18"
#define ACMAXSTANUM					"2.3.1.19"
#define ACRADIUSREQSUCCRATE			"2.3.1.20"
#define    ACINACROAMINGINRATE        "2.3.1.21"
#define    ACINACROAMINGOUTRATE        "2.3.1.22"
#define ACPORTALLOGOFFNUM          "2.3.1.23"
#define    ACCHALLENGERECREQNUM        "2.3.1.24"
#define    ACCHALLENGERESREQNUM        "2.3.1.25"
#define    ACAUTHRECREQNUM             "2.3.1.26"
#define    ACSENDRADIUSREQNUM          "2.3.1.27"
#define    ACRECRADIUSRESREQNUM        "2.3.1.28"
#define    ACRECRADIUSREQSUCCNUM       "2.3.1.29"
#define    ACAUTHRESREQNUM             "2.3.1.30"
#define    ACSENDRADIUSOFFLINEREQNUM   "2.3.1.31"
#define    ACRECRADIUSOFFLINERESNUM    "2.3.1.32"
#define    ACACCTREQNUM                "2.3.1.33"
#define    ACACCTREQSUCCNUM            "2.3.1.34"
#define    ACLOGOFFRATE                "2.3.1.35"
#define    ACASSOCIATEDTOTALUSERNUM	   "2.3.1.36"
#define    ACAUTHTIMEOUTNUM            "2.3.1.37"
#define    ACCHALLENGETIMEOUTNUM       "2.3.1.38"
#define    ACCHALLENGEREJECTNUM        "2.3.1.39"
#define    ACCHALLENGEBUSYNUM      	   "2.3.1.40"
#define    ACAUTHPASSWORDMISSINGNUM    "2.3.1.41"
#define    ACAUTHUNKNOWNTYPENUM        "2.3.1.42"
#define    ACAUTHBUSYNUM               "2.3.1.43"
#define    ACAUTHDISORDERNUM           "2.3.1.44"
#define    ACPORTALERRCODE0NUM         "2.3.1.45"
#define    ACPORTALERRCODE1NUM         "2.3.1.46"
#define    ACPORTALERRCODE2NUM         "2.3.1.47"
#define    ACPORTALERRCODE3NUM         "2.3.1.48"
#define    ACPORTALERRCODE4NUM         "2.3.1.49"
#define    ACMACAUTHCURUSERNUM           "2.3.1.50"
#define    ACMACAUTHERRLOGOFFNUM         "2.3.1.51"
#define    ACMACAUTHREQNUM         "2.3.1.52"
#define    ACMACAUTHREQSUCCNUM         "2.3.1.53"
#define    ACMACAUTHREQFAILNUM         "2.3.1.54"
#define    ACAUTHFREECURUSERNUM        "2.3.1.55"
#define    ACAUTHFREEERRLOGOFFNUM      "2.3.1.56"
#define    ACASSOCAUTHCURUSERNUM       "2.3.1.57"
#define    ACASSOCAUTHERRLOGOFFNUM     "2.3.1.58"
#define    ACASSOCAUTHREQNUM           "2.3.1.59"
#define    ACASSOCAUTHREQSUCCNUM       "2.3.1.60"
#define    ACASSOCAUTHREQFAILNUM       "2.3.1.61"
#define   ASSOCAUTHONLINEUSERNUM        "2.3.1.69"
#define   ASSOCAUTHUSERLOSTCONNECTIONCNT	   "2.3.1.70"
#define   ASSOCAUTHREQCNT                      "2.3.1.71"
#define   ASSOCAUTHSUCCNT                      "2.3.1.72"
#define   ASSOCAUTHREQFAILCNT                  "2.3.1.73"
#define   AUTOAUTHONLINEUSERNUM                "2.3.1.74"
#define   AUTOAUTHUSERLOSTCONNECTIONCNT	       "2.3.1.75"

#define ACMAXNUM_XML 	"/opt/services/option/acmaxnum_option.option"
#define ACMAXNUM_STATUS "/opt/services/status/acmaxnum_status.status"
#define ACMAXNUM_XML_S  "/opt/www/htdocs/acmaxnum.xml"

#define ACMAX_NODE  "acmax"
#define ACMAX_AP    "acmaxap"
#define ACMAX_STAT  "acmaxsta"
#define ACMAX_PROID "acproid"

static unsigned int acUsersonAC = 0;
static unsigned int acAuthRejectCnt = 0;
static unsigned int acAuthAttemptCnt = 0;
static unsigned int wtpStaAllReassoc = 0;
static long update_time_show_station_list_by_group = 0;
static void update_data_for_show_station_list_by_group();

static unsigned long long acUplinkDataThroughput = 0;
static unsigned long long acDownlinkDataThroughput = 0;
static unsigned long long acPortThroughput = 0;
static long update_time_show_ethport_list = 0;
static void update_data_for_show_ethport_list();

static int acOutACRoamingSuccRate = 0;
static int acOutACRoamingInSuccRate = 0;
static int acOutACRoamingOutSuccRate = 0;
static long update_time_show_inter_ac_roaming_count_cmd = 0;
static void update_data_for_show_inter_ac_roaming_count_cmd();

static unsigned long acAuthReqCnt = 0;
static unsigned long acAuthSucCnt = 0;
static unsigned long acAuthFailCnt = 0;
static unsigned long acPortalLogoffnum = 0;
static unsigned long acChallengeRecReqnum = 0;
static unsigned long acChallengeResReqnum = 0;
static unsigned long acAuthRecReqnum = 0;
static unsigned long acSendRadiusReqnum = 0;
static unsigned long access_response_count_portal = 0;
static unsigned long acRecRadiusReqSuccnum = 0;
static unsigned long acAuthResReqnum = 0;
static unsigned long acSendRadiusOfflineReqnum = 0;
static unsigned long acRecRadiusOfflineResnum = 0;
static unsigned long acAcctReqnum = 0;
static unsigned long acAcctReqSuccnum = 0;
static unsigned long acLogoffRate = 0;
static unsigned long acAuthTimeoutNum = 0;
static unsigned long acChallengeTimeoutNum = 0;
static unsigned long acChallengeRejectNum = 0;
static unsigned long acChallengeBusyNum = 0;
static unsigned long acAuthPasswordMissingNum = 0;
static unsigned long acAuthUnknownTypeNum = 0;
static unsigned long acAuthBusyNum = 0;
static unsigned long acAuthDisorderNum = 0;
static unsigned long acPortalErrcode0Num = 0;
static unsigned long acPortalErrcode1Num = 0;
static unsigned long acPortalErrcode2Num = 0;
static unsigned long acPortalErrcode3Num = 0;
static unsigned long acPortalErrcode4Num = 0;
static long update_time_eag_get_eag_statistics = 0;

static unsigned long acMacAuthCuruserNum  = 0;
static unsigned long acMacAuthErrLogoffNum  = 0;
static unsigned long acMacAuthReqNum = 0;
static unsigned long acMacAuthReqSuccNum  = 0;
static unsigned long acMacAuthReqFailNum  = 0;

static void update_data_for_eag_get_eag_statistics();

static unsigned long access_response_count_SIM = 0;
static unsigned long acAuthFreeCuruserNum = 0;
static unsigned long acAuthFreeErrLogoffNum = 0;
static unsigned long acAssocAuthCuruserNum = 0;
static unsigned long acAssocAuthErrLogoffNum = 0;
static unsigned long acAssocAuthReqNum = 0;
static unsigned long acAssocAuthReqSuccNum = 0;
static unsigned long acAssocAuthReqFailNum = 0;
static unsigned long assocAuthOnlineUserNum = 0;
static unsigned long assocAuthUserLostConnectionCnt = 0;
static unsigned long assocAuthReqCnt = 0;
static unsigned long assocAuthSucCnt = 0;
static unsigned long assocAuthReqFailCnt = 0;
static unsigned long autoAuthOnlineUserNum = 0;
static unsigned long autoAuthUserLostConnectionCnt = 0;
static long update_time_show_ac_station_information_cmd = 0;
static void update_data_for_show_ac_station_information_cmd();

void
init_dot11AcStats(void)
{
	static oid acNumAPInAC_oid[128] 				= { 0 };
	static oid acOnlineNumAPInAC_oid[128] 			= { 0 };
	static oid acUsersonAC_oid[128] 				= { 0 };
	static oid acUplinkDataThroughput_oid[128] 		= { 0 };
	static oid acDownlinkDataThroughput_oid[128] 	= { 0 };
	static oid acPortThroughput_oid[128] 			= { 0 };
	static oid acInACRoamingSuccRate_oid[128] 		= { 0 };	
	static oid acOutACRoamingSuccRate_oid[128]   	= { 0 };	
	static oid acAuthReqCnt_oid[128] 				= { 0 };	
	static oid acAuthSucCnt_oid[128] 				= { 0 };
	static oid acAuthFailCnt_oid[128] 				= { 0 };
	static oid acAuthRejectCnt_oid[128] 			= { 0 };
	static oid acAuthAttemptCnt_oid[128] 			= { 0 };
	static oid acStaAllReassoc_oid[128] 			= { 0 };
	static oid acStatus_oid[128] 					= { 0 };
	static oid acStandbySwitch_oid[128] 			= { 0 };
	static oid acBandWidthAvgUtilization_oid[128] 	= { 0 };
	static oid acLostPktsAvgUtilization_oid[128] 	= { 0 };
	static oid acMaxStaNum_oid[128] 				= { 0 };
	static oid acRadiusReqSuccRate_oid[128] 		= { 0 };
	static oid acOutACRoamingInRate_oid[128] 		= { 0 };
	static oid acOutACRoamingOutRate_oid[128] 		= { 0 };
	static oid acPortalLogoffnum_oid[128]           = { 0 };
    static oid acChallengeRecReqnum_oid[128]        = { 0 };
    static oid acChallengeResReqnum_oid[128]        = { 0 };
    static oid acAuthRecReqnum_oid[128]             = { 0 };
    static oid acSendRadiusReqnum_oid[128]          = { 0 };
    static oid acRecRadiusResReqnum_oid[128]        = { 0 };
    static oid acRecRadiusReqSuccnum_oid[128]       = { 0 };
    static oid acAuthResReqnum_oid[128]             = { 0 };
    static oid acSendRadiusOfflineReqnum_oid[128]   = { 0 };
    static oid acRecRadiusOfflineResnum_oid[128]    = { 0 };
    static oid acAcctReqnum_oid[128]                = { 0 };
    static oid acAcctReqSuccnum_oid[128] 			= { 0 };
    static oid acLogoffRate_oid[128] 				= { 0 };
	static oid acAssociatedTotalUserNum_oid[128]	= { 0 };
    static oid acAuthTimeoutNum_oid[128] 			= { 0 };
    static oid acChallengeTimeoutNum_oid[128] 		= { 0 };
    static oid acChallengeRejectNum_oid[128] 		= { 0 };
    static oid acChallengeBusyNum_oid[128] 			= { 0 };
	static oid acAuthPasswordMissingNum_oid[128] 	= { 0 };
	static oid acAuthUnknownTypeNum_oid[128] 		= { 0 };
	static oid acAuthBusyNum_oid[128] 				= { 0 };
	static oid acAuthDisorderNum_oid[128] 			= { 0 };
	static oid acPortalErrcode0Num_oid[128]			= { 0 };
	static oid acPortalErrcode1Num_oid[128]			= { 0 };
	static oid acPortalErrcode2Num_oid[128]			= { 0 };
	static oid acPortalErrcode3Num_oid[128]			= { 0 };
	static oid acPortalErrcode4Num_oid[128]			= { 0 };
    static oid acMacAuthCuruserNum_oid[128] = { 0 };
    static oid acMacAuthErrLogoffNum_oid[128] = { 0 };
    static oid acMacAuthReqNum_oid[128] = { 0 };
    static oid acMacAuthReqSuccNum_oid[128] = { 0 };
    static oid acMacAuthReqFailNum_oid[128] = { 0 };
    static oid acAuthFreeCuruserNum_oid[128]        = { 0 };
    static oid acAuthFreeErrLogoffNum_oid[128]      = { 0 };
    static oid acAssocAuthCuruserNum_oid[128]       = { 0 };
    static oid acAssocAuthErrLogoffNum_oid[128]     = { 0 };
    static oid acAssocAuthReqNum_oid[128]           = { 0 };
    static oid acAssocAuthReqSuccNum_oid[128]       = { 0 };
    static oid acAssocAuthReqFailNum_oid[128]       = { 0 };
	static oid assocAuthOnlineUserNum_oid[128]		= { 0 };
	static oid assocAuthUserLostConnectionCnt_oid[128]	= { 0 };
	static oid assocAuthReqCnt_oid[128] 				= { 0 };
	static oid assocAuthSucCnt_oid[128] 				= { 0 };
	static oid assocAuthReqFailCnt_oid[128] 			= { 0 };
	static oid autoAuthOnlineUserNum_oid[128]			= { 0 };
	static oid autoAuthUserLostConnectionCnt_oid[128]	= { 0 };	
	
	size_t public_oid_len   = 0;
	mad_dev_oid(acNumAPInAC_oid,ACNUMAPINAC,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acOnlineNumAPInAC_oid,ACONLINENUMAPINAC,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acUsersonAC_oid,ACUSERSONAC,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acUplinkDataThroughput_oid,ACUPLINKDATATHROUGHPUT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acDownlinkDataThroughput_oid,ACDOWNLINKDATATHROUGHPUT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortThroughput_oid,ACPORTTHROUGHPUT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acInACRoamingSuccRate_oid,ACINACROAMINGSUCCRATE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acOutACRoamingSuccRate_oid,ACOUTACROAMINGSUCCRATE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthReqCnt_oid,ACAUTHREQCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthSucCnt_oid,ACAUTHSUCCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthFailCnt_oid,ACAUTHFAILCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthRejectCnt_oid,ACAUTHREJECTCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthAttemptCnt_oid,ACAUTHATTEMPTCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acStaAllReassoc_oid,ACSTAALLREASSOC,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acStatus_oid,ACSTATUS,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acStandbySwitch_oid,ACSTANDBYSWITCH,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acBandWidthAvgUtilization_oid,ACBANDWIDTHAVGUTILIZATION,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLostPktsAvgUtilization_oid,ACLOSTPKTSAVGUTILIZATION,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMaxStaNum_oid,ACMAXSTANUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acRadiusReqSuccRate_oid,ACRADIUSREQSUCCRATE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acOutACRoamingInRate_oid,ACINACROAMINGINRATE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acOutACRoamingOutRate_oid,ACINACROAMINGOUTRATE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortalLogoffnum_oid,ACPORTALLOGOFFNUM,&public_oid_len,enterprise_pvivate_oid);
    mad_dev_oid(acChallengeRecReqnum_oid,ACCHALLENGERECREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acChallengeResReqnum_oid,ACCHALLENGERESREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthRecReqnum_oid,ACAUTHRECREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSendRadiusReqnum_oid,ACSENDRADIUSREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acRecRadiusResReqnum_oid,ACRECRADIUSRESREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acRecRadiusReqSuccnum_oid,ACRECRADIUSREQSUCCNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthResReqnum_oid,ACAUTHRESREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acSendRadiusOfflineReqnum_oid,ACSENDRADIUSOFFLINEREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acRecRadiusOfflineResnum_oid,ACRECRADIUSOFFLINERESNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAcctReqnum_oid,ACACCTREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAcctReqSuccnum_oid,ACACCTREQSUCCNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acLogoffRate_oid,ACLOGOFFRATE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAssociatedTotalUserNum_oid,ACASSOCIATEDTOTALUSERNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthTimeoutNum_oid,ACAUTHTIMEOUTNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acChallengeTimeoutNum_oid,ACCHALLENGETIMEOUTNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acChallengeRejectNum_oid,ACCHALLENGEREJECTNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acChallengeBusyNum_oid,ACCHALLENGEBUSYNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthPasswordMissingNum_oid,ACAUTHPASSWORDMISSINGNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthUnknownTypeNum_oid,ACAUTHUNKNOWNTYPENUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthBusyNum_oid,ACAUTHBUSYNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthDisorderNum_oid,ACAUTHDISORDERNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortalErrcode0Num_oid,ACPORTALERRCODE0NUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortalErrcode1Num_oid,ACPORTALERRCODE1NUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortalErrcode2Num_oid,ACPORTALERRCODE2NUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortalErrcode3Num_oid,ACPORTALERRCODE3NUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPortalErrcode4Num_oid,ACPORTALERRCODE4NUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMacAuthCuruserNum_oid,ACMACAUTHCURUSERNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMacAuthErrLogoffNum_oid,ACMACAUTHERRLOGOFFNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMacAuthReqNum_oid,ACMACAUTHREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMacAuthReqSuccNum_oid,ACMACAUTHREQSUCCNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMacAuthReqFailNum_oid,ACMACAUTHREQFAILNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthFreeCuruserNum_oid,ACAUTHFREECURUSERNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAuthFreeErrLogoffNum_oid,ACAUTHFREEERRLOGOFFNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAssocAuthCuruserNum_oid,ACASSOCAUTHCURUSERNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAssocAuthErrLogoffNum_oid,ACASSOCAUTHERRLOGOFFNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAssocAuthReqNum_oid,ACASSOCAUTHREQNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAssocAuthReqSuccNum_oid,ACASSOCAUTHREQSUCCNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acAssocAuthReqFailNum_oid,ACASSOCAUTHREQFAILNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(assocAuthOnlineUserNum_oid, ASSOCAUTHONLINEUSERNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(assocAuthUserLostConnectionCnt_oid, ASSOCAUTHUSERLOSTCONNECTIONCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(assocAuthReqCnt_oid, ASSOCAUTHREQCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(assocAuthSucCnt_oid, ASSOCAUTHSUCCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(assocAuthReqFailCnt_oid, ASSOCAUTHREQFAILCNT,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(autoAuthOnlineUserNum_oid, AUTOAUTHONLINEUSERNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(autoAuthUserLostConnectionCnt_oid, AUTOAUTHUSERLOSTCONNECTIONCNT,&public_oid_len,enterprise_pvivate_oid);
	
	
  DEBUGMSGTL(("dot11AcStats", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acNumAPInAC", handle_acNumAPInAC,
                               acNumAPInAC_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acOnlineNumAPInAC", handle_acOnlineNumAPInAC,
                               acOnlineNumAPInAC_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acUsersonAC", handle_acUsersonAC,
                               acUsersonAC_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acUplinkDataThroughput", handle_acUplinkDataThroughput,
                               acUplinkDataThroughput_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acDownlinkDataThroughput", handle_acDownlinkDataThroughput,
                               acDownlinkDataThroughput_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortThroughput", handle_acPortThroughput,
                               acPortThroughput_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acInACRoamingSuccRate", handle_acInACRoamingSuccRate,
                               acInACRoamingSuccRate_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acOutACRoamingSuccRate", handle_acOutACRoamingSuccRate,
                               acOutACRoamingSuccRate_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthReqCnt", handle_acAuthReqCnt,
                               acAuthReqCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthSucCnt", handle_acAuthSucCnt,
                               acAuthSucCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthFailCnt", handle_acAuthFailCnt,
                               acAuthFailCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthRejectCnt", handle_acAuthRejectCnt,
                               acAuthRejectCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthAttemptCnt", handle_acAuthAttemptCnt,
                               acAuthAttemptCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acStaAllReassoc", handle_acStaAllReassoc,
                               acStaAllReassoc_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acStatus", handle_acStatus,
                               acStatus_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acStandbySwitch", handle_acStandbySwitch,
                               acStandbySwitch_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acBandWidthAvgUtilization", handle_acBandWidthAvgUtilization,
                               acBandWidthAvgUtilization_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLostPktsAvgUtilization", handle_acLostPktsAvgUtilization,
                               acLostPktsAvgUtilization_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMaxStaNum", handle_acMaxStaNum,
                               acMaxStaNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acRadiusReqSuccRate", handle_acRadiusReqSuccRate,
                               acRadiusReqSuccRate_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acOutACRoamingInRate", handle_acOutACRoamingInSuccRate,
                               acOutACRoamingInRate_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acOutACRoamingOutRate", handle_acOutACRoamingOutSuccRate,
                               acOutACRoamingOutRate_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortalLogoffnum", handle_acPortalLogoffnum,
                           	   acPortalLogoffnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acChallengeRecReqnum", handle_acChallengeRecReqnum,
                               acChallengeRecReqnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acChallengeResReqnum", handle_acChallengeResReqnum,
                               acChallengeResReqnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthRecReqnum", handle_acAuthRecReqnum,
                               acAuthRecReqnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSendRadiusReqnum", handle_acSendRadiusReqnum,
                               acSendRadiusReqnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acRecRadiusResReqnum", handle_acRecRadiusResReqnum,
                               acRecRadiusResReqnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acRecRadiusReqSuccnum", handle_acRecRadiusReqSuccnum,
                               acRecRadiusReqSuccnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthResReqnum", handle_acAuthResReqnum,
                               acAuthResReqnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acSendRadiusOfflineReqnum", handle_acSendRadiusOfflineReqnum,
                               acSendRadiusOfflineReqnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acRecRadiusOfflineResnum", handle_acRecRadiusOfflineResnum,
                               acRecRadiusOfflineResnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAcctReqnum", handle_acAcctReqnum,
                               acAcctReqnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAcctReqSuccnum", handle_acAcctReqSuccnum,
                               acAcctReqSuccnum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acLogoffRate", handle_acLogoffRate,
                               acLogoffRate_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAssociatedTotalUserNum", handle_acAssociatedTotalUserNum,
								acAssociatedTotalUserNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthTimeoutNum", handle_acAuthTimeoutNum,
                               acAuthTimeoutNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acChallengeTimeoutNum", handle_acChallengeTimeoutNum,
                               acChallengeTimeoutNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acChallengeRejectNum", handle_acChallengeRejectNum,
                               acChallengeRejectNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acChallengeBusyNum", handle_acChallengeBusyNum,
								acChallengeBusyNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthPasswordMissingNum", handle_acAuthPasswordMissingNum,
								acAuthPasswordMissingNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthUnknownTypeNum", handle_acAuthUnknownTypeNum,
								 acAuthUnknownTypeNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthBusyNum", handle_acAuthBusyNum,
								 acAuthBusyNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthDisorderNum", handle_acAuthDisorderNum,
								 acAuthDisorderNum_oid, public_oid_len,
								 HANDLER_CAN_RONLY
	));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortalErrcode0Num", handle_acPortalErrcode0Num,
                               acPortalErrcode0Num_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortalErrcode1Num", handle_acPortalErrcode1Num,
                               acPortalErrcode1Num_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortalErrcode2Num", handle_acPortalErrcode2Num,
                               acPortalErrcode2Num_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortalErrcode3Num", handle_acPortalErrcode3Num,
                               acPortalErrcode3Num_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acPortalErrcode4Num", handle_acPortalErrcode4Num,
                               acPortalErrcode4Num_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMacAuthCuruserNum", handle_acMacAuthCuruserNum,
                               acMacAuthCuruserNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMacAuthErrLogoffNum", handle_acMacAuthErrLogoffNum,
                               acMacAuthErrLogoffNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMacAuthReqNum", handle_acMacAuthReqNum,
                               acMacAuthReqNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMacAuthReqSuccNum", handle_acMacAuthReqSuccNum,
                               acMacAuthReqSuccNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMacAuthReqFailNum", handle_acMacAuthReqFailNum,
                               acMacAuthReqFailNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthFreeCuruserNum", handle_acAuthFreeCuruserNum,
                               acAuthFreeCuruserNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAuthFreeErrLogoffNum", handle_acAuthFreeErrLogoffNum,
                               acAuthFreeErrLogoffNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAssocAuthCuruserNum", handle_acAssocAuthCuruserNum,
                               acAssocAuthCuruserNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAssocAuthErrLogoffNum", handle_acAssocAuthErrLogoffNum,
                               acAssocAuthErrLogoffNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAssocAuthReqNum", handle_acAssocAuthReqNum,
                               acAssocAuthReqNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAssocAuthReqSuccNum", handle_acAssocAuthReqSuccNum,
                               acAssocAuthReqSuccNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acAssocAuthReqFailNum", handle_acAssocAuthReqFailNum,
                               acAssocAuthReqFailNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
    ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("assocAuthOnlineUserNum", handle_assocAuthOnlineUserNum,
                               assocAuthOnlineUserNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("assocAuthUserLostConnectionCnt", handle_assocAuthUserLostConnectionCnt,
                               assocAuthUserLostConnectionCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("assocAuthReqCnt", handle_assocAuthReqCnt,
                               assocAuthReqCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("assocAuthSucCnt", handle_assocAuthSucCnt,
                               assocAuthSucCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("assocAuthReqFailCnt", handle_assocAuthReqFailCnt,
                               assocAuthReqFailCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("autoAuthOnlineUserNum", handle_autoAuthOnlineUserNum,
                               autoAuthOnlineUserNum_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("autoAuthUserLostConnectionCnt", handle_autoAuthUserLostConnectionCnt,
                               autoAuthUserLostConnectionCnt_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_acNumAPInAC(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acNumAPInAC\n");

    switch(reqinfo->mode) {

	case MODE_GET:
	{
	    int wtp_num = 0;
        snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
        
        snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wtp_list_by_sn_cmd_func\n");
        messageHead = list_connection_call_dbus_method(show_wtp_list_by_sn_cmd_func, SHOW_ALL_WTP_TABLE_METHOD);
        snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wtp_list_by_sn_cmd_func,messageHead=%p\n", messageHead);
        
        if(messageHead)
        {
            for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
            {
                DCLI_WTP_API_GROUP_ONE *wtp_head = messageNode->message;
                if((wtp_head)&&(wtp_head->WTP_INFO))
			    {
                    wtp_num += wtp_head->WTP_INFO->list_len;
                }
            } 
            free_dbus_message_list(&messageHead, Free_wtp_list_by_sn_head);
        }

		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&wtp_num,
									sizeof(wtp_num));
		
	}
	break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acNumAPInAC\n", reqinfo->mode );

            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acNumAPInAC\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acOnlineNumAPInAC(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acOnlineNumAPInAC\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
		    int online_num = 0;
		    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
            
            snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wtp_list_by_sn_cmd_func\n");
            messageHead = list_connection_call_dbus_method(show_wtp_list_by_sn_cmd_func, SHOW_ALL_WTP_TABLE_METHOD);
            snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wtp_list_by_sn_cmd_func,messageHead=%p\n", messageHead);
            
            if(messageHead)
            {
                for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
                {
                    DCLI_WTP_API_GROUP_ONE *wtp_head = messageNode->message;
                    if(wtp_head)
    			    {
                        online_num += wtp_head->run_num;
                    }
                } 
                free_dbus_message_list(&messageHead, Free_wtp_list_by_sn_head);
            }

            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &online_num,
                                     sizeof(online_num));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acOnlineNumAPInAC\n", reqinfo->mode );

            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acOnlineNumAPInAC\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acUsersonAC(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acUsersonAC\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int acUsersonAC = 0;
			
		    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
            
            snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_station_list_by_group\n");
            messageHead = list_connection_call_dbus_method(show_station_list_by_group, SHOW_ALL_WTP_TABLE_METHOD);
            snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_station_list_by_group,messageHead=%p\n", messageHead);
            
            if(messageHead)
            {
                for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
                {
                    struct dcli_ac_info *ac = messageNode->message;
                    if(ac)
    			    {
                        acUsersonAC += ac->num_sta_all;
                    }
                } 
                free_dbus_message_list(&messageHead, Free_sta_summary);
            }
			#endif
			
			update_data_for_show_station_list_by_group();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &acUsersonAC,
                                     sizeof(acUsersonAC));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acUsersonAC\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acUsersonAC\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acUplinkDataThroughput(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acUplinkDataThroughput\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int number = 0,ret=1;
			ETH_SLOT_LIST  head,*p = NULL;
			memset(&head,0,sizeof(ETH_SLOT_LIST));
			ETH_PORT_LIST *pp = NULL;
			port_flow * flow = NULL;
			flow = (port_flow *)malloc(sizeof(port_flow));
			if(flow)
			{
				memset(flow,0,sizeof(port_flow));
			}
			unsigned long long upload = 0;

			
			snmp_log(LOG_DEBUG, "enter show_ethport_list\n");
			ret=show_ethport_list(&head,&number);
			snmp_log(LOG_DEBUG, "exit show_ethport_list,ret=%d\n", ret);
			
			for(p=head.next; (NULL != p); p=p->next)
			{
				for(pp=p->port.next; (NULL != pp); pp=pp->next)
				{
					if(flow)
					{
						ccgi_get_port_flow(0,0,flow,p->slot_no,pp->port_no);
						upload +=(unsigned long long)flow->tx_goodbytes;
					}
				}
			}
			#endif

			update_data_for_show_ethport_list();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64,
                                     (u_char *) &acUplinkDataThroughput,
                                     sizeof(acUplinkDataThroughput));

			#if 0
			FREE_OBJECT(flow);
			if((ret==0)&&(number>0))
			{
				Free_ethslot_head(&head);
			}
			#endif
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acUplinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acUplinkDataThroughput\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acDownlinkDataThroughput(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	
	snmp_log(LOG_DEBUG, "enter handle_acDownlinkDataThroughput\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int number = 0,ret=1;
			ETH_SLOT_LIST  head,*p = NULL;
			memset(&head,0,sizeof(ETH_SLOT_LIST));
			ETH_PORT_LIST *pp = NULL;
			port_flow * flow = NULL;
			flow = (port_flow *)malloc(sizeof(port_flow));
			if(flow)
			{
				memset(flow,0,sizeof(port_flow));
			}
			unsigned long long download=0;

			snmp_log(LOG_DEBUG, "enter show_ethport_list\n");
			ret=show_ethport_list(&head,&number);
			snmp_log(LOG_DEBUG, "exit show_ethport_list,ret=%d\n", ret);

			for(p=head.next; (NULL != p); p=p->next)
			{
				for(pp=p->port.next; (NULL != pp); pp=pp->next)
				{
					if(flow)
					{
						ccgi_get_port_flow(0,0,flow,p->slot_no,pp->port_no);
						download +=(unsigned long long)((flow->rx_goodbytes)+(flow->rx_badbytes));
					}
				}
			}
			#endif

			update_data_for_show_ethport_list();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64,
                                     (u_char *)&acDownlinkDataThroughput,
                                     sizeof(acDownlinkDataThroughput));

			#if 0
			FREE_OBJECT(flow);
			if((ret==0)&&(number>0))
			{
			  Free_ethslot_head(&head);
			}
			#endif
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );

            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acDownlinkDataThroughput\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acPortThroughput(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acPortThroughput\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			int number = 0,ret=1;
			ETH_SLOT_LIST  head,*p = NULL;
			memset(&head,0,sizeof(ETH_SLOT_LIST));
			ETH_PORT_LIST *pp = NULL;
			port_flow * flow = NULL;
			flow = (port_flow *)malloc(sizeof(port_flow));
			if(flow)
			{
				memset(flow,0,sizeof(port_flow));
			}
			unsigned long long int upload=0,download=0;
			unsigned long long allload=0;

			
			snmp_log(LOG_DEBUG, "enter show_ethport_list\n");
			ret=show_ethport_list(&head,&number);
			snmp_log(LOG_DEBUG, "exit show_ethport_list,ret=%d\n", ret);
			
	  		for(p=head.next; (NULL != p); p=p->next)
			{
				for(pp=p->port.next; (NULL != pp); pp=pp->next)
				{
					if(flow)
					{
						ccgi_get_port_flow(0,0,flow,p->slot_no,pp->port_no);
						upload +=(unsigned long long)(flow->tx_goodbytes);
						download +=(unsigned long long)((flow->rx_goodbytes)+(flow->rx_badbytes));
					}
			  	}
			}
			allload = upload + download;
			#endif

			update_data_for_show_ethport_list();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64,
                                     (u_char *) &acPortThroughput,
                                     sizeof(acPortThroughput));

			#if 0
			FREE_OBJECT(flow);
			if((ret==0)&&(number>0))
			{
				Free_ethslot_head(&head);
			}
			#endif
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPortThroughput\n", reqinfo->mode );


            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acPortThroughput\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acInACRoamingSuccRate(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acInACRoamingSuccRate\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{	
		    unsigned long acInACRoamingSuccRate = 0;
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			int ret = 0;
			struct dcli_ac_info *ac = NULL;
            
			snmp_log(LOG_DEBUG, "enter list_instance_parameter\n");
			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			snmp_log(LOG_DEBUG, "exit list_instance_parameter,paraHead=%p\n", paraHead);
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_sta_summary(pq->parameter, pq->connection, &ac, NULL);
				if((1 == ret)&&(ac))
				{
					acInACRoamingSuccRate += ac->num_local_roam;
					Free_sta_summary(ac);
				}
			}
			free_instance_parameter_list(&paraHead);
                        
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                     (u_char *)&acInACRoamingSuccRate,
                                     sizeof(acInACRoamingSuccRate));	    
        }
	        break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acInACRoamingSuccRate\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acOutACRoamingSuccRate(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acOutACRoamingSuccRate\n");

	switch(reqinfo->mode) {

        case MODE_GET:
		{	
			#if 0
			int acOutACRoamingSuccRate = 0;
			instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);            
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
                int ret = 0;
			    struct roaming_count_profile count_info;
        		snmp_log(LOG_DEBUG, "enter show_inter_ac_roaming_count_cmd\n");
        	    ret=show_inter_ac_roaming_count_cmd(paraNode->parameter, paraNode->connection, &count_info);
        		snmp_log(LOG_DEBUG, "exit show_inter_ac_roaming_count_cmd,ret=%d\n", ret);
        		
        	    if(ret == 1)
        	    {
        	    	acOutACRoamingSuccRate += count_info.total_count;
        	    }
			}
			free_instance_parameter_list(&paraHead);
			#endif
			
			update_data_for_show_inter_ac_roaming_count_cmd();	    		
	        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
	                                 (u_char *)&acOutACRoamingSuccRate,
	                                  sizeof(acOutACRoamingSuccRate));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acOutACRoamingSuccRate\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAuthReqCnt(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acAuthReqCnt\n");

	switch(reqinfo->mode) {

		case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			unsigned long auth_req_count = 0;
			struct timeval time_begin = {0};
			struct timeval time_end = {0};
			struct timeval time_res = {0};

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 


			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				gettimeofday(&time_begin, NULL);
				
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);
				
				auth_req_count += eag_stat.auth_req_count;

				gettimeofday(&time_end, NULL);
				timersub(&time_end, &time_begin, &time_res);

				snmp_log(LOG_DEBUG, "handle_acAuthReqCnt eag_get_eag_statistics time used %ds%dus\n", time_res.tv_sec, time_res.tv_usec);
				
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acAuthReqCnt,
										sizeof(acAuthReqCnt));
		}
		break;

	default:
	/* we should never get here, so this is a really bad error */
	snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthReqCnt\n", reqinfo->mode );
	return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acAuthReqCnt\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acAuthSucCnt(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acAuthSucCnt\n");
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0 
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			unsigned long auth_suc_count = 0;
			struct timeval time_begin = {0};
			struct timeval time_end = {0};
			struct timeval time_res = {0};

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				gettimeofday(&time_begin, NULL);

				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);
				auth_suc_count += eag_stat.auth_ack_0_count;

				gettimeofday(&time_end, NULL);
				timersub(&time_end, &time_begin, &time_res);

				snmp_log(LOG_DEBUG, "handle_acAuthSucCnt eag_get_eag_statistics time used %ds%dus\n", time_res.tv_sec, time_res.tv_usec);		
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
											(u_char *)&acAuthSucCnt,
											sizeof(acAuthSucCnt));
		}
		break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthSucCnt\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAuthSucCnt\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAuthFailCnt(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acAuthFailCnt\n");

	switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			//unsigned long auth_req_count = 0;
			//unsigned long auth_suc_count = 0;
			unsigned long auth_fail_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{	
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				auth_fail_count += eag_stat.auth_ack_1_count + eag_stat.auth_ack_2_count 
						+ eag_stat.auth_ack_3_count + eag_stat.auth_ack_4_count;
				
			}
			free_instance_parameter_list(&paraHead);

			//auth_fail_count = auth_req_count - auth_suc_count;
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
												(u_char *)&acAuthFailCnt,
												sizeof(acAuthFailCnt));
		}
		break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthFailCnt\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAuthFailCnt\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAuthRejectCnt(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acAuthRejectCnt\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			unsigned long acAuthFail = 0;
			    
            snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
            
            snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_station_list_by_group\n");
            messageHead = list_connection_call_dbus_method(show_station_list_by_group, SHOW_ALL_WTP_TABLE_METHOD);
            snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_station_list_by_group,messageHead=%p\n", messageHead);
            
            if(messageHead)
            {
                for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
                {
                    struct dcli_ac_info *ac = messageNode->message;
                    if(ac)
                    {
                        acAuthFail += ac->num_auth_fail;
                    }
                } 
                free_dbus_message_list(&messageHead, Free_sta_summary);
            }
			#endif

			update_data_for_show_station_list_by_group();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
	                                     (u_char *)&acAuthRejectCnt,
	                                     sizeof(acAuthRejectCnt));
			       
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAuthRejectCnt\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAuthAttemptCnt(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acAuthAttemptCnt\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			unsigned long acAuthNum = 0;
			    
            snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
            
            snmp_log(LOG_DEBUG, "handle_acAuthAttemptCnt:enter list_connection_call_dbus_method:show_station_list_by_group\n");
            messageHead = list_connection_call_dbus_method(show_station_list_by_group, SHOW_ALL_WTP_TABLE_METHOD);
            snmp_log(LOG_DEBUG, "handle_acAuthAttemptCnt:exit list_connection_call_dbus_method:show_station_list_by_group,messageHead=%p\n", messageHead);
            
            if(messageHead)
            {
                for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
                {
                    struct dcli_ac_info *ac = messageNode->message;
                    if(ac)
                    {
                        acAuthNum += ac->num_auth;
                    }
                } 
                free_dbus_message_list(&messageHead, Free_sta_summary);
            }
			#endif
			
			update_data_for_show_station_list_by_group();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
	                                     (u_char *)&acAuthAttemptCnt,
	                                     sizeof(acAuthAttemptCnt));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAuthAttemptCnt\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acStaAllReassoc(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acStaAllReassoc\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
		    unsigned long all_number = 0;
			    
            snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
            
            snmp_log(LOG_DEBUG, "handle_acAuthAttemptCnt:enter list_connection_call_dbus_method:show_station_list_by_group\n");
            messageHead = list_connection_call_dbus_method(show_station_list_by_group, SHOW_ALL_WTP_TABLE_METHOD);
            snmp_log(LOG_DEBUG, "handle_acAuthAttemptCnt:exit list_connection_call_dbus_method:show_station_list_by_group,messageHead=%p\n", messageHead);
            
            if(messageHead)
            {
                for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
                {
                    struct dcli_ac_info *ac = messageNode->message;
                    if(ac)
                    {
                        all_number += ac->num_reassoc;
                    }
                } 
                free_dbus_message_list(&messageHead, Free_sta_summary);
            }		
			#endif

			update_data_for_show_station_list_by_group();
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &wtpStaAllReassoc,
                                     sizeof(wtpStaAllReassoc));
			
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acStaAllReassoc\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acStatus(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acStatus\n");

	switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acStatus = 0;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acStatus,
                                     sizeof(acStatus));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acStatus\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acStandbySwitch(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acStandbySwitch\n");

	switch(reqinfo->mode) {

        case MODE_GET:
		{
			int acStandbySwitch = 0;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acStandbySwitch,
                                     sizeof(acStandbySwitch));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
	
	snmp_log(LOG_DEBUG, "exit handle_acStandbySwitch\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acBandWidthAvgUtilization(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acBandWidthAvgUtilization\n");

	switch(reqinfo->mode) {

        case MODE_GET:
		{
		    
			int BandWidth_usage = 0;
		    #if 0
			int ret  = 0;

			snmp_log(LOG_DEBUG, "enter trap_read\n");
			ret = trap_read(&_usage,"band_average");
			snmp_log(LOG_DEBUG, "exit trap_read,ret=%d\n", ret);
			
			if(ret != 0)
			{
				_usage = 0;
			}
			#endif 
	        snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
	                                 (u_char *)&BandWidth_usage,
	                                 sizeof(BandWidth_usage));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
	
	snmp_log(LOG_DEBUG, "exit handle_acBandWidthAvgUtilization\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acLostPktsAvgUtilization(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acLostPktsAvgUtilization\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int BandWidth_usage = 0;
			#if 0
			int ret = 0;

			snmp_log(LOG_DEBUG, "enter trap_read\n");
			ret =  trap_read(&_usage,"band_average");
			snmp_log(LOG_DEBUG, "exit trap_read,ret=%d\n", ret);
			
			if(ret != 0)
			{
				ret = 0;
			}
			#endif
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&BandWidth_usage,
                                     sizeof(BandWidth_usage));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLostPktsAvgUtilization\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acMaxStaNum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acMaxStaNum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{

			char tmp[64] = {0};
			int ret = 0;
			if(access(ACMAXNUM_XML,0)!=0)
			{
				memset(tmp,0,sizeof(tmp));
				snprintf(tmp,sizeof(tmp)-1,"sudo cp %s %s",ACMAXNUM_XML_S,ACMAXNUM_XML);
				system(tmp);
			}
			else
			{
				ret=if_xml_file(ACMAXNUM_XML);
				if(ret!=0)
				{
					memset(tmp,0,sizeof(tmp));
					snprintf(tmp,sizeof(tmp)-1,"sudo rm  %s > /dev/null",ACMAXNUM_XML);
					system(tmp);
					memset(tmp,0,sizeof(tmp));
					snprintf(tmp,sizeof(tmp)-1,"sudo cp %s %s",ACMAXNUM_XML_S,ACMAXNUM_XML);
					system(tmp);
				}
			}

			if(access(ACMAXNUM_STATUS,0)!=0)
			{
				memset(tmp,0,64);
				snprintf(tmp,sizeof(tmp)-1,"echo \"1\" > %s ",ACMAXNUM_STATUS);
				system(tmp);
			}

			int product_id  = 0;
			product_id = get_product_id();
			char product_str[50] = {0};
			snprintf(product_str,sizeof(product_str)-1,"%d",product_id);		
			char get_prostr[50] = {0};
			unsigned long long get_pronum = 0;
			int flag = -1;			
			find_second_xmlnode(ACMAXNUM_XML, ACMAX_NODE, ACMAX_PROID, product_str, &flag);
			if (0 != flag)
			{
				get_second_xmlnode(ACMAXNUM_XML, ACMAX_NODE,ACMAX_STAT, &get_prostr, flag);
				get_pronum = strtoul(get_prostr,0,10);
				
			}

            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER64,
                                     (u_char *)&get_pronum,
                                     sizeof(get_pronum));

        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acMaxStaNum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acRadiusReqSuccRate(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acRadiusReqSuccRate\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
            
            unsigned int access_accept_rate = 100;
 		    
		    instance_parameter *inst_para = NULL, *inst_node = NULL;
            list_instance_parameter(&inst_para, SNMPD_SLOT_CONNECT);	
            for(inst_node = inst_para; NULL != inst_node; inst_node = inst_node->next) {
                struct radius_req_rate *radiusReq_array = NULL;
                unsigned int radiusReq_num = 0;
                int ret = dbus_get_radius_req_info(inst_para->connection, &radiusReq_array, &radiusReq_num);
                if(0 == ret && radiusReq_num) {
                    access_accept_rate = radiusReq_array[0].access_accept_rate;
                    snmp_log(LOG_DEBUG, "handle_acRadiusReqSuccRate: get slot %d %s instance %d, access_accept_rate = %d\n",
                                         inst_node->parameter.slot_id, radiusReq_array[0].local_id ? "Local-hansi" : "Remote-hansi",
                                         radiusReq_array[0].instance_id, radiusReq_array[0].access_accept_rate);
                    break;
                }
            }
			free_instance_parameter_list(&inst_para);
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&access_accept_rate,
                                     sizeof(access_accept_rate));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acRadiusReqSuccRate\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acOutACRoamingInSuccRate(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acOutACRoamingInSuccRate\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{	
			#if 0
			unsigned long acOutACRoamingInSuccRate = 0;
			instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);            
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
    			int ret = 0;
    			struct roaming_count_profile count_info;
    			snmp_log(LOG_DEBUG, "enter show_inter_ac_roaming_count_cmd\n");
    		    ret=show_inter_ac_roaming_count_cmd(paraNode->parameter, paraNode->connection, &count_info);
    			snmp_log(LOG_DEBUG, "exit show_inter_ac_roaming_count_cmd,ret=%d\n", ret);
    			
    		    if(ret == 1)
    		    {
    		    	acOutACRoamingInSuccRate += count_info.in_count;
    		    }
    		}    
		    free_instance_parameter_list(&paraHead);		
			#endif

			update_data_for_show_inter_ac_roaming_count_cmd();
	        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
	                                 (u_char *)&acOutACRoamingInSuccRate,
	                                  sizeof(acOutACRoamingInSuccRate));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acOutACRoamingInSuccRate\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acOutACRoamingOutSuccRate(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acOutACRoamingOutSuccRate\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{	
			#if 0
			unsigned long acOutACRoamingOutSuccRate = 0;
			instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);            
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
    			int ret;
    			struct roaming_count_profile count_info;

    			snmp_log(LOG_DEBUG, "enter show_inter_ac_roaming_count_cmd\n");
    		    ret=show_inter_ac_roaming_count_cmd(paraNode->parameter, paraNode->connection, &count_info);
    			snmp_log(LOG_DEBUG, "exit show_inter_ac_roaming_count_cmd,ret=%d\n", ret);
    			
    		    if(ret == 1)
    		    {
    		    	acOutACRoamingOutSuccRate += count_info.out_count;
    		    }
    		}    
			free_instance_parameter_list(&paraHead);		    		
			#endif

			update_data_for_show_inter_ac_roaming_count_cmd();
	        snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
	                                 (u_char *)&acOutACRoamingOutSuccRate,
	                                  sizeof(acOutACRoamingOutSuccRate));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acOutACRoamingOutSuccRate\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acOutACRoamingOutRate(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acOutACRoamingOutRate\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int time  = 0;
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&time,
                                     sizeof(time));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDownlinkDataThroughput\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acOutACRoamingOutRate\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acPortalLogoffnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acPortalLogoffnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			unsigned long auth_logoff_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				auth_logoff_count += eag_stat.abnormal_logoff_count;
			}
			free_instance_parameter_list(&paraHead);			
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acPortalLogoffnum,
										sizeof(acPortalLogoffnum));
        }
        	break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPortalLogoffnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acPortalLogoffnum\n");
    return SNMP_ERR_NOERROR;
}



handle_acChallengeRecReqnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acChallengeRecReqnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int chal_req_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				chal_req_count += eag_stat.challenge_req_count; 
			}
			free_instance_parameter_list(&paraHead);
			#endif
			
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
									(u_char *)&acChallengeRecReqnum,
									sizeof(acChallengeRecReqnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acChallengeRecReqnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acChallengeRecReqnum\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acChallengeResReqnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acChallengeResReqnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int ack_challenge_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				ack_challenge_count += eag_stat.challenge_ack_0_count + eag_stat.challenge_ack_1_count
						+ eag_stat.challenge_ack_2_count + eag_stat.challenge_ack_3_count
						+ eag_stat.challenge_ack_4_count; 
			}
			free_instance_parameter_list(&paraHead);
			#endif
			
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acChallengeResReqnum,
										sizeof(acChallengeResReqnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acChallengeResReqnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acChallengeResReqnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAuthRecReqnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acAuthRecReqnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int auth_req_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
		
			for (pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				auth_req_count += eag_stat.auth_req_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif
				
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAuthRecReqnum,
										sizeof(acAuthRecReqnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthRecReqnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAuthRecReqnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSendRadiusReqnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acSendRadiusReqnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int access_request_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				access_request_count += eag_stat.access_request_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif
				
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acSendRadiusReqnum,
										sizeof(acSendRadiusReqnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSendRadiusReqnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSendRadiusReqnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acRecRadiusResReqnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acRecRadiusResReqnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int access_response_count_portal = 0;
			int access_response_count_sim = 0;
            int ret = 0;
			struct eag_all_stat eag_stat = {0};
            struct WtpStationinfo *acStationInfo = NULL;
			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				access_response_count_portal += eag_stat.access_accept_count + eag_stat.access_reject_count;
				
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				if(1 == ret && acStationInfo)
				{
					access_response_count_sim += acStationInfo->auto_auth_resp_cnt;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif			
			unsigned long access_response_count = 0;
			
			update_data_for_eag_get_eag_statistics();
			
			update_data_for_show_ac_station_information_cmd();
			
			access_response_count = access_response_count_portal + access_response_count_SIM;
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&access_response_count,
										sizeof(access_response_count));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acRecRadiusResReqnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acRecRadiusResReqnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acRecRadiusReqSuccnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acRecRadiusReqSuccnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int access_response_count = 0;
			int access_ack_failed_count = 0;
			int access_accept_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				access_accept_count += eag_stat.access_accept_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif
				
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acRecRadiusReqSuccnum,
										sizeof(acRecRadiusReqSuccnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acRecRadiusReqSuccnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acRecRadiusReqSuccnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAuthResReqnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acAuthResReqnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int ack_auth_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				ack_auth_count += eag_stat.auth_ack_0_count + eag_stat.auth_ack_1_count
							+ eag_stat.auth_ack_2_count + eag_stat.auth_ack_3_count
							+ eag_stat.auth_ack_4_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAuthResReqnum,
										sizeof(acAuthResReqnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthResReqnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAuthResReqnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acSendRadiusOfflineReqnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acSendRadiusOfflineReqnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int acct_request_stop_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				acct_request_stop_count += eag_stat.acct_request_stop_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif
				
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acSendRadiusOfflineReqnum,
										sizeof(acSendRadiusOfflineReqnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acSendRadiusOfflineReqnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acSendRadiusOfflineReqnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acRecRadiusOfflineResnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acRecRadiusOfflineResnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int acct_request_stop_response_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				acct_request_stop_response_count += eag_stat.acct_response_stop_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif
				
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acRecRadiusOfflineResnum,
										sizeof(acRecRadiusOfflineResnum));
		}
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acRecRadiusOfflineResnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acRecRadiusOfflineResnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAcctReqnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */

	snmp_log(LOG_DEBUG, "enter handle_acAcctReqnum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int acct_request_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				acct_request_count += eag_stat.acct_request_start_count + eag_stat.acct_request_start_retry_count
						+ eag_stat.acct_request_update_count + eag_stat.acct_request_update_retry_count
						+ eag_stat.acct_request_stop_count + eag_stat.acct_request_stop_retry_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif
				
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAcctReqnum,
										sizeof(acAcctReqnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAcctReqnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAcctReqnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acAcctReqSuccnum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	snmp_log(LOG_DEBUG, "enter handle_acAcctReqSuccnum\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int account_ack_count = 0;
			int account_ack_failed_count = 0;
			int acct_request_success_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				acct_request_success_count += eag_stat.acct_response_start_count
							+ eag_stat.acct_response_update_count
							+ eag_stat.acct_response_stop_count; 
			}
			free_instance_parameter_list(&paraHead);
			//acct_request_success_count = account_ack_count - account_ack_failed_count;
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAcctReqSuccnum,
										sizeof(acAcctReqSuccnum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAcctReqSuccnum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acAcctReqSuccnum\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acLogoffRate(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acLogoffRate\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int logoff_normal_count = 0;
			int logoff_abnormal_count = 0;
			int logoff_abnormal_rate = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				logoff_abnormal_count += eag_stat.abnormal_logoff_count; 
				logoff_normal_count += eag_stat.normal_logoff_count; 
			}
			free_instance_parameter_list(&paraHead);			
			if (0 == logoff_normal_count && 0 == logoff_abnormal_count)
				logoff_abnormal_rate = 0;
			else
				logoff_abnormal_rate = 100 * logoff_abnormal_count / (logoff_normal_count + logoff_abnormal_count);
			#endif
				
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acLogoffRate,
										sizeof(acLogoffRate));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acLogoffRate\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acLogoffRate\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acAssociatedTotalUserNum(netsnmp_mib_handler *handler,
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
			int acAssociatedTotalUserNum  = 0;	
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			int ret = 0;
			struct dcli_ac_info *ac = NULL;
            
			snmp_log(LOG_DEBUG, "enter list_instance_parameter\n");
			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			snmp_log(LOG_DEBUG, "exit list_instance_parameter,paraHead=%p\n", paraHead);
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_sta_summary(pq->parameter, pq->connection, &ac, NULL);
				if((1 == ret)&&(ac))
				{
					acAssociatedTotalUserNum += ac->num_sta;
					Free_sta_summary(ac);
				}
			}
			free_instance_parameter_list(&paraHead);		

            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&acAssociatedTotalUserNum,
                                     sizeof(acAssociatedTotalUserNum));
        }
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAssociatedTotalUserNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAuthTimeoutNum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acAuthTimeoutNum\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			#if 0
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int access_request_timeout_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				access_request_timeout_count += eag_stat.access_request_timeout_count;
			}
			free_instance_parameter_list(&paraHead);			
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAuthTimeoutNum,
										sizeof(acAuthTimeoutNum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthTimeoutNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
	snmp_log(LOG_DEBUG, "exit handle_acAuthTimeoutNum\n");
    return SNMP_ERR_NOERROR;
}

int
handle_acChallengeTimeoutNum(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int challenge_timeout_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				challenge_timeout_count += eag_stat.challenge_timeout_count;
			}
			free_instance_parameter_list(&paraHead);	
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acChallengeTimeoutNum,
										sizeof(acChallengeTimeoutNum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acChallengeTimeoutNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acChallengeRejectNum(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int challenge_reject_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				challenge_reject_count += eag_stat.challenge_ack_1_count; 
			}
			free_instance_parameter_list(&paraHead);	
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acChallengeRejectNum,
										sizeof(acChallengeRejectNum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acChallengeRejectNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acChallengeBusyNum(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int challenge_busy_count  = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				challenge_busy_count += eag_stat.challenge_busy_count;
			}
			free_instance_parameter_list(&paraHead);	
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acChallengeBusyNum,
										sizeof(acChallengeBusyNum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acChallengeBusyNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAuthPasswordMissingNum(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int req_auth_password_missing_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				req_auth_password_missing_count += eag_stat.req_auth_password_missing_count;
			}
			free_instance_parameter_list(&paraHead);	
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAuthPasswordMissingNum,
										sizeof(acAuthPasswordMissingNum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthPasswordMissingNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAuthUnknownTypeNum(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int unknown_type_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				unknown_type_count += eag_stat.req_auth_unknown_type_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAuthUnknownTypeNum,
										sizeof(acAuthUnknownTypeNum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthUnknownTypeNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAuthBusyNum(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int ack_auth_busy_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				ack_auth_busy_count += eag_stat.ack_auth_busy_count;
			}
			free_instance_parameter_list(&paraHead);
			#endif
				
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAuthBusyNum,
										sizeof(acAuthBusyNum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthBusyNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAuthDisorderNum(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			struct list_head ap_stat;
			struct eag_ap_stat *tmp = NULL;
			int auth_disorder_count = 0;

			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 

			
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);

				auth_disorder_count += eag_stat.auth_disorder_count; 
			}
			free_instance_parameter_list(&paraHead);
			#endif
	
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
										(u_char *)&acAuthDisorderNum,
										sizeof(acAuthDisorderNum));
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthDisorderNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


int
handle_acPortalErrcode0Num(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;			
			unsigned long acPortalErrcode0Num = 0;
			int ret = -1;
			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = eag_get_eag_statistics(pq->connection, pq->parameter.local_id, pq->parameter.instance_id, &eag_stat);
				if(0 == ret) 
				{
					acPortalErrcode0Num += (eag_stat.auth_ack_0_count + eag_stat.challenge_ack_0_count);
				}
			}
			free_instance_parameter_list(&paraHead);
			#endif
			
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acPortalErrcode0Num,
										sizeof(acPortalErrcode0Num));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPortalErrcode0Num\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acPortalErrcode1Num(netsnmp_mib_handler *handler,
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
            instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;            
			unsigned long acPortalErrcode1Num = 0;
			int ret = -1;
			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = eag_get_eag_statistics(pq->connection, pq->parameter.local_id, pq->parameter.instance_id, &eag_stat);
				if(0 == ret) 
				{					
					acPortalErrcode1Num += (eag_stat.auth_ack_1_count + eag_stat.challenge_ack_1_count);
				}
			}
			free_instance_parameter_list(&paraHead);

			#endif
			
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acPortalErrcode1Num,
										sizeof(acPortalErrcode1Num));

        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPortalErrcode1Num\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


int
handle_acPortalErrcode2Num(netsnmp_mib_handler *handler,
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
            instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			unsigned long acPortalErrcode2Num = 0;
			int ret = -1;
			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = eag_get_eag_statistics(pq->connection, pq->parameter.local_id, pq->parameter.instance_id, &eag_stat);
				if(0 == ret) 
				{					
					acPortalErrcode2Num += (eag_stat.auth_ack_2_count + eag_stat.challenge_ack_2_count);
				}
			}
			free_instance_parameter_list(&paraHead);

			#endif
			
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acPortalErrcode2Num,
										sizeof(acPortalErrcode2Num));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPortalErrcode2Num\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acPortalErrcode3Num(netsnmp_mib_handler *handler,
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
            instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			unsigned long acPortalErrcode3Num = 0;
			int ret = -1;
			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = eag_get_eag_statistics(pq->connection, pq->parameter.local_id, pq->parameter.instance_id, &eag_stat);
				if(0 == ret) 
				{					
					acPortalErrcode3Num += (eag_stat.auth_ack_3_count + eag_stat.challenge_ack_3_count);
				}
			}
			free_instance_parameter_list(&paraHead);
			#endif
			
			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acPortalErrcode3Num,
										sizeof(acPortalErrcode3Num));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPortalErrcode3Num\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acPortalErrcode4Num(netsnmp_mib_handler *handler,
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
            instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			unsigned long acPortalErrcode4Num = 0;
			int ret = -1;
			struct eag_all_stat eag_stat = {0};

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = eag_get_eag_statistics(pq->connection, pq->parameter.local_id, pq->parameter.instance_id, &eag_stat);
				if(0 == ret) 
				{					
					acPortalErrcode4Num += (eag_stat.auth_ack_4_count + eag_stat.challenge_ack_4_count);
				}
			}
			free_instance_parameter_list(&paraHead);

			#endif

			update_data_for_eag_get_eag_statistics();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acPortalErrcode4Num,
										sizeof(acPortalErrcode4Num));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPortalErrcode4Num\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_acMacAuthCuruserNum(netsnmp_mib_handler *handler,
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
			update_data_for_eag_get_eag_statistics();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                     (u_char *) &acMacAuthCuruserNum,
                                     sizeof(acMacAuthCuruserNum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMacAuthCuruserNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_acMacAuthErrLogoffNum(netsnmp_mib_handler *handler,
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
			update_data_for_eag_get_eag_statistics();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                     (u_char *) &acMacAuthErrLogoffNum,
                                     sizeof(acMacAuthErrLogoffNum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMacAuthErrLogoffNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_acMacAuthReqNum(netsnmp_mib_handler *handler,
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
			update_data_for_eag_get_eag_statistics();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                     (u_char *) &acMacAuthReqNum,
                                     sizeof(acMacAuthReqNum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMacAuthReqNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_acMacAuthReqSuccNum(netsnmp_mib_handler *handler,
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
			update_data_for_eag_get_eag_statistics();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                     (u_char *) &acMacAuthReqSuccNum,
                                     sizeof(acMacAuthReqSuccNum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMacAuthReqSuccNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_acMacAuthReqFailNum(netsnmp_mib_handler *handler,
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
			update_data_for_eag_get_eag_statistics();
            snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                                     (u_char *) &acMacAuthReqFailNum,
                                     sizeof(acMacAuthReqFailNum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acMacAuthReqFailNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_acAuthFreeCuruserNum(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long acAuthFreeCuruserNum = 0;
            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;
			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);				
				if(1 == ret && acStationInfo) {
					acAuthFreeCuruserNum += acStationInfo->no_auth_sta_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);            
            #endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acAuthFreeCuruserNum,
										sizeof(acAuthFreeCuruserNum));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthFreeCuruserNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAuthFreeErrLogoffNum(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;            
            unsigned long acAuthFreeErrLogoffNum = 0;
            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;
			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);				
				if(1 == ret && acStationInfo) {
					acAuthFreeErrLogoffNum += acStationInfo->no_auth_sta_abnormal_down_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acAuthFreeErrLogoffNum,
										sizeof(acAuthFreeErrLogoffNum));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAuthFreeErrLogoffNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAssocAuthCuruserNum(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long acAssocAuthCuruserNum = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				if(1 == ret && acStationInfo) {
					acAssocAuthCuruserNum += acStationInfo->assoc_auth_sta_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acAssocAuthCuruserNum,
										sizeof(acAssocAuthCuruserNum));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAssocAuthCuruserNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAssocAuthErrLogoffNum(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long acAssocAuthErrLogoffNum = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					acAssocAuthErrLogoffNum += acStationInfo->assoc_auth_sta_abnormal_down_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acAssocAuthErrLogoffNum,
										sizeof(acAssocAuthErrLogoffNum));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAssocAuthErrLogoffNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAssocAuthReqNum(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long acAssocAuthReqNum = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					acAssocAuthReqNum += acStationInfo->assoc_auth_req_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acAssocAuthReqNum,
										sizeof(acAssocAuthReqNum));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAssocAuthReqNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAssocAuthReqSuccNum(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long acAssocAuthReqSuccNum = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					acAssocAuthReqSuccNum += acStationInfo->assoc_auth_succ_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acAssocAuthReqSuccNum,
										sizeof(acAssocAuthReqSuccNum));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAssocAuthReqSuccNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_acAssocAuthReqFailNum(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long acAssocAuthReqFailNum = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					acAssocAuthReqFailNum += acStationInfo->assoc_auth_fail_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&acAssocAuthReqFailNum,
										sizeof(acAssocAuthReqFailNum));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acAssocAuthReqFailNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}

int
handle_assocAuthOnlineUserNum(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			unsigned long assocAuthOnlineUserNum = 0;			
			int ret = 0;
			struct WtpStationinfo *acStationInfo = NULL;
	
			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					assocAuthOnlineUserNum += acStationInfo->assoc_auth_online_sta_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&assocAuthOnlineUserNum,
										sizeof(assocAuthOnlineUserNum));
		}
			break;
	
		default:
			/* we should never get here, so this is a really bad error */
			snmp_log(LOG_ERR, "unknown mode (%d) in handle_assocAuthOnlineUserNum\n", reqinfo->mode );
			return SNMP_ERR_GENERR;
	}
	
	return SNMP_ERR_NOERROR;
}
int
handle_assocAuthUserLostConnectionCnt(netsnmp_mib_handler *handler,
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
			instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
			unsigned long assocAuthUserLostConnectionCnt = 0;			
			int ret = 0;
			struct WtpStationinfo *acStationInfo = NULL;
	
			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					assocAuthUserLostConnectionCnt += acStationInfo->assoc_auth_sta_drop_cnt;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&assocAuthUserLostConnectionCnt,
										sizeof(assocAuthUserLostConnectionCnt));
		}
			break;
	
		default:
			/* we should never get here, so this is a really bad error */
			snmp_log(LOG_ERR, "unknown mode (%d) in handle_assocAuthUserLostConnectionCnt\n", reqinfo->mode );
			return SNMP_ERR_GENERR;
	}
	
	return SNMP_ERR_NOERROR;
}
int
handle_assocAuthReqCnt(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long assocAuthReqCnt = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					assocAuthReqCnt += acStationInfo->weppsk_assoc_req_cnt;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&assocAuthReqCnt,
										sizeof(assocAuthReqCnt));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_assocAuthReqCnt\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_assocAuthSucCnt(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long assocAuthSucCnt = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					assocAuthSucCnt += acStationInfo->weppsk_assoc_succ_cnt;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&assocAuthSucCnt,
										sizeof(assocAuthSucCnt));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_assocAuthSucCnt\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_assocAuthReqFailCnt(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long assocAuthReqFailCnt = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					assocAuthReqFailCnt += acStationInfo->weppsk_assoc_fail_cnt;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&assocAuthReqFailCnt,
										sizeof(assocAuthReqFailCnt));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_assocAuthReqFailCnt\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_autoAuthOnlineUserNum(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long autoAuthOnlineUserNum = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					autoAuthOnlineUserNum += acStationInfo->auto_auth_online_sta_num;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&autoAuthOnlineUserNum,
										sizeof(autoAuthOnlineUserNum));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_autoAuthOnlineUserNum\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_autoAuthUserLostConnectionCnt(netsnmp_mib_handler *handler,
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
		    instance_parameter *paraHead = NULL;
			instance_parameter *pq = NULL;
            unsigned long autoAuthUserLostConnectionCnt = 0;            
            int ret = 0;
            struct WtpStationinfo *acStationInfo = NULL;

			list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
			for(pq = paraHead; (NULL != pq); pq = pq->next)
			{
				ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
				
				if(1 == ret && acStationInfo) {
					autoAuthUserLostConnectionCnt += acStationInfo->auto_auth_sta_drop_cnt;
				}
				Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
			}
			free_instance_parameter_list(&paraHead);
			#endif

			update_data_for_show_ac_station_information_cmd();
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
										(u_char *)&autoAuthUserLostConnectionCnt,
										sizeof(autoAuthUserLostConnectionCnt));
        }
        	break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_autoAuthUserLostConnectionCnt\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}


static void update_data_for_show_station_list_by_group()
{
	struct sysinfo info;
	
	if(0 != update_time_show_station_list_by_group)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_show_station_list_by_group < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_station_list_by_group\n");
	
	/*update cache data*/
	instance_parameter *paraHead = NULL;
	instance_parameter *pq = NULL;
	int ret = 0;
	struct dcli_ac_info *ac = NULL;
	
	acUsersonAC = 0;
	acAuthRejectCnt = 0;
	acAuthAttemptCnt = 0;
	wtpStaAllReassoc = 0;
	snmp_log(LOG_DEBUG, "enter list_instance_parameter\n");
	list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
	snmp_log(LOG_DEBUG, "exit list_instance_parameter,paraHead=%p\n",paraHead);
	for(pq = paraHead; (NULL != pq); pq = pq->next)
	{
		snmp_log(LOG_DEBUG, "enter show_station_list_by_group\n");
		ret = show_station_list_by_group(pq->parameter, pq->connection, &ac);
		snmp_log(LOG_DEBUG, "exit show_station_list_by_group,ret=%d\n", ret);
		if(1 == ret)
		{
			acUsersonAC += ac->num_sta_all;
			acAuthRejectCnt += ac->num_auth_fail;
			acAuthAttemptCnt += ac->num_auth;
			wtpStaAllReassoc += ac->num_reassoc;
			Free_sta_summary(ac);
		}
	}
	free_instance_parameter_list(&paraHead);	
	
	sysinfo(&info); 		
	update_time_show_station_list_by_group = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_station_list_by_group\n");
}

static void update_data_for_show_ethport_list()
{
	struct sysinfo info;
	
	if(0 != update_time_show_ethport_list)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_show_ethport_list < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_ethport_list\n");
	
	/*update cache data*/
	int number = 0,ret = 1;
	ETH_SLOT_LIST  head,*p = NULL;
	memset(&head,0,sizeof(ETH_SLOT_LIST));
	ETH_PORT_LIST *pp = NULL;
	port_flow * flow = NULL;
	flow = (port_flow *)malloc(sizeof(port_flow));
	if(flow)
	{
		memset(flow,0,sizeof(port_flow));
	}
	
	acUplinkDataThroughput = 0;
	acDownlinkDataThroughput = 0;
	acPortThroughput = 0;
	
	snmp_log(LOG_DEBUG, "enter show_ethport_list\n");
	ret = show_ethport_list(&head,&number);
	snmp_log(LOG_DEBUG, "exit show_ethport_list,ret=%d\n", ret);
	for(p=head.next; (NULL != p); p=p->next)
  	{
		for(pp=p->port.next; (NULL != pp); pp=pp->next)
		{
			if(flow)
			{
				ccgi_get_port_flow(0,0,flow,p->slot_no,pp->port_no);
				acUplinkDataThroughput += (unsigned long long)flow->tx_goodbytes;
				acDownlinkDataThroughput += (unsigned long long)((flow->rx_goodbytes)+(flow->rx_badbytes));
			}
		}
  	}
	acPortThroughput = acUplinkDataThroughput + acDownlinkDataThroughput;
	
	FREE_OBJECT(flow);
	if((ret==0)&&(number>0))
	{
		Free_ethslot_head(&head);
	}
	
	sysinfo(&info); 		
	update_time_show_ethport_list = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_ethport_list\n");
}

static void update_data_for_show_inter_ac_roaming_count_cmd()
{
	struct sysinfo info;
	
	if(0 != update_time_show_inter_ac_roaming_count_cmd)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_show_inter_ac_roaming_count_cmd < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_inter_ac_roaming_count_cmd\n");
	
	/*update cache data*/
	acOutACRoamingSuccRate = 0;
	acOutACRoamingInSuccRate = 0;
	acOutACRoamingOutSuccRate = 0;
	
	instance_parameter *paraHead = NULL, *paraNode = NULL;
    list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);            
    for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
        int ret = 0;
	    struct roaming_count_profile count_info;
		snmp_log(LOG_DEBUG, "enter show_inter_ac_roaming_count_cmd\n");
	    ret=show_inter_ac_roaming_count_cmd(paraNode->parameter, paraNode->connection, &count_info);
		snmp_log(LOG_DEBUG, "exit show_inter_ac_roaming_count_cmd,ret=%d\n", ret);
		
	    if(ret == 1)
	    {
	    	acOutACRoamingSuccRate += count_info.total_count;
			acOutACRoamingInSuccRate += count_info.in_count;
			acOutACRoamingOutSuccRate += count_info.out_count;
	    }
	}
	free_instance_parameter_list(&paraHead);
	
	sysinfo(&info); 		
	update_time_show_inter_ac_roaming_count_cmd = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_inter_ac_roaming_count_cmd\n");
}

static void update_data_for_eag_get_eag_statistics()
{
	struct sysinfo info;
	
	if(0 != update_time_eag_get_eag_statistics)
	{
		sysinfo(&info); 	
		if(info.uptime - update_time_eag_get_eag_statistics < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for eag_get_eag_statistics\n");
	
	/*update cache data*/	
	instance_parameter *paraHead = NULL;
	instance_parameter *pq = NULL;
	int ret = -1;
	struct eag_all_stat eag_stat = {0};	
	unsigned long logoff_normal_count = 0;

	acAuthReqCnt = 0;
	acAuthSucCnt = 0;	
	acAuthFailCnt = 0;
	acPortalLogoffnum = 0;
	acChallengeRecReqnum = 0;
	acChallengeResReqnum = 0;
	acAuthRecReqnum = 0;
	acSendRadiusReqnum = 0;
	access_response_count_portal = 0;
	acRecRadiusReqSuccnum = 0;
	acAuthResReqnum = 0;
	acSendRadiusOfflineReqnum = 0;
	acRecRadiusOfflineResnum = 0;
	acAcctReqnum = 0;
	acAcctReqSuccnum = 0;
	acLogoffRate = 0;
	acAuthTimeoutNum = 0;
	acChallengeTimeoutNum = 0;
	acChallengeRejectNum = 0;
	acChallengeBusyNum = 0;
	acAuthPasswordMissingNum = 0;
	acAuthUnknownTypeNum = 0;
	acAuthBusyNum = 0;
	acAuthDisorderNum = 0;
	acPortalErrcode0Num = 0;
	acPortalErrcode1Num = 0;
	acPortalErrcode2Num = 0;
	acPortalErrcode3Num = 0;
	acPortalErrcode4Num = 0;
	acMacAuthCuruserNum  = 0;
	acMacAuthErrLogoffNum	= 0;
	acMacAuthReqNum = 0;
	acMacAuthReqSuccNum  = 0;
	acMacAuthReqFailNum  = 0;

	snmp_log(LOG_DEBUG, "enter list_instance_parameter\n");
	list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
	snmp_log(LOG_DEBUG, "exit list_instance_parameter,paraHead=%p\n", paraHead);
	for(pq = paraHead; (NULL != pq); pq = pq->next)
	{
		ret = eag_get_eag_statistics(pq->connection,pq->parameter.local_id,pq->parameter.instance_id,&eag_stat);
		if (0 == ret) 
		{
			acAuthReqCnt += eag_stat.auth_req_count;
			acAuthSucCnt += eag_stat.auth_ack_0_count;
			acAuthFailCnt += (eag_stat.auth_ack_1_count + eag_stat.auth_ack_2_count + eag_stat.auth_ack_3_count + eag_stat.auth_ack_4_count);			
			acPortalLogoffnum += eag_stat.abnormal_logoff_count;
			acChallengeRecReqnum += eag_stat.challenge_req_count;
			acChallengeResReqnum += (eag_stat.challenge_ack_0_count + eag_stat.challenge_ack_1_count
							+ eag_stat.challenge_ack_2_count + eag_stat.challenge_ack_3_count + eag_stat.challenge_ack_4_count);
			acAuthRecReqnum += eag_stat.auth_req_count;
			acSendRadiusReqnum += eag_stat.access_request_count;
			access_response_count_portal += eag_stat.access_accept_count + eag_stat.access_reject_count;
			acRecRadiusReqSuccnum += eag_stat.access_accept_count;
			acAuthResReqnum += (eag_stat.auth_ack_0_count + eag_stat.auth_ack_1_count
							+ eag_stat.auth_ack_2_count + eag_stat.auth_ack_3_count + eag_stat.auth_ack_4_count);
			acSendRadiusOfflineReqnum += eag_stat.acct_request_stop_count;
			acRecRadiusOfflineResnum += eag_stat.acct_response_stop_count;
			acAcctReqnum += (eag_stat.acct_request_start_count + eag_stat.acct_request_start_retry_count
									+ eag_stat.acct_request_update_count + eag_stat.acct_request_update_retry_count
									+ eag_stat.acct_request_stop_count + eag_stat.acct_request_stop_retry_count);
			acAcctReqSuccnum += (eag_stat.acct_response_start_count + eag_stat.acct_response_update_count + eag_stat.acct_response_stop_count);
			
			logoff_normal_count += eag_stat.normal_logoff_count;
			if (0 == logoff_normal_count && 0 == acPortalLogoffnum)
			{
				acLogoffRate = 0;
			}
			else
			{
				acLogoffRate = 100 * acPortalLogoffnum / (logoff_normal_count + acPortalLogoffnum);
			}
			
			acAuthTimeoutNum += eag_stat.access_request_timeout_count;
			acChallengeTimeoutNum += eag_stat.challenge_timeout_count;
			acChallengeRejectNum += eag_stat.challenge_ack_1_count;
			acChallengeBusyNum += eag_stat.challenge_busy_count;
			acAuthPasswordMissingNum += eag_stat.req_auth_password_missing_count;
			acAuthUnknownTypeNum += eag_stat.req_auth_unknown_type_count;
			acAuthBusyNum += eag_stat.ack_auth_busy_count;
			acAuthDisorderNum += eag_stat.auth_disorder_count;
			acPortalErrcode0Num += (eag_stat.auth_ack_0_count + eag_stat.challenge_ack_0_count);
			acPortalErrcode1Num += (eag_stat.auth_ack_1_count + eag_stat.challenge_ack_1_count);
			acPortalErrcode2Num += (eag_stat.auth_ack_2_count + eag_stat.challenge_ack_2_count);
			acPortalErrcode3Num += (eag_stat.auth_ack_3_count + eag_stat.challenge_ack_3_count);
			acPortalErrcode4Num += (eag_stat.auth_ack_4_count + eag_stat.challenge_ack_4_count);
			acMacAuthCuruserNum  += eag_stat.macauth_online_user_num;
			acMacAuthErrLogoffNum	+= eag_stat.macauth_abnormal_logoff_count;
			acMacAuthReqNum += eag_stat.macauth_req_count;
			acMacAuthReqSuccNum  += eag_stat.macauth_ack_0_count;
			acMacAuthReqFailNum  += (eag_stat.macauth_ack_1_count + eag_stat.macauth_ack_2_count + eag_stat.macauth_ack_3_count + eag_stat.macauth_ack_4_count);
		}
	}
	free_instance_parameter_list(&paraHead);

	sysinfo(&info); 		
	update_time_eag_get_eag_statistics = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for eag_get_eag_statistics\n");
}

static void update_data_for_show_ac_station_information_cmd()
{
	struct sysinfo info;
	
	if(0 != update_time_show_ac_station_information_cmd)
	{
		sysinfo(&info);		
		if(info.uptime - update_time_show_ac_station_information_cmd < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for show_ac_station_information_cmd\n");
	
	/*update cache data*/
	instance_parameter *paraHead = NULL;
	instance_parameter *pq = NULL;
    int ret = 0;
    struct WtpStationinfo *acStationInfo = NULL;
	
	access_response_count_SIM = 0;
	acAuthFreeCuruserNum = 0;
	acAuthFreeErrLogoffNum = 0;
	acAssocAuthCuruserNum = 0;
	acAssocAuthErrLogoffNum = 0;
	acAssocAuthReqNum = 0;
	acAssocAuthReqSuccNum = 0;
	acAssocAuthReqFailNum = 0;
	assocAuthOnlineUserNum = 0;
	assocAuthUserLostConnectionCnt = 0;
	assocAuthReqCnt = 0;
	assocAuthSucCnt = 0;
	assocAuthReqFailCnt = 0;
	autoAuthOnlineUserNum = 0;
	autoAuthUserLostConnectionCnt = 0;
	
	snmp_log(LOG_DEBUG, "enter list_instance_parameter\n");
	list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER); 
	snmp_log(LOG_DEBUG, "exit list_instance_parameter,paraHead=%p\n", paraHead);
	for(pq = paraHead; (NULL != pq); pq = pq->next)
	{
		ret = show_ac_sta_information_cmd(pq->parameter, pq->connection, &acStationInfo);
		if(1 == ret)
		{
			if(acStationInfo)
			{
				access_response_count_SIM += acStationInfo->auto_auth_resp_cnt;
				acAuthFreeCuruserNum += acStationInfo->no_auth_sta_num;
				acAuthFreeErrLogoffNum += acStationInfo->no_auth_sta_abnormal_down_num;
				acAssocAuthCuruserNum += acStationInfo->assoc_auth_sta_num;
				acAssocAuthErrLogoffNum += acStationInfo->assoc_auth_sta_abnormal_down_num;
				acAssocAuthReqNum += acStationInfo->assoc_auth_req_num;
				acAssocAuthReqSuccNum += acStationInfo->assoc_auth_succ_num;
				acAssocAuthReqFailNum += acStationInfo->assoc_auth_fail_num;
				assocAuthOnlineUserNum += acStationInfo->assoc_auth_online_sta_num;
				assocAuthUserLostConnectionCnt += acStationInfo->assoc_auth_sta_drop_cnt;
				assocAuthReqCnt += acStationInfo->weppsk_assoc_req_cnt;
				assocAuthSucCnt += acStationInfo->weppsk_assoc_succ_cnt;
				assocAuthReqFailCnt += acStationInfo->weppsk_assoc_fail_cnt;
				autoAuthOnlineUserNum += acStationInfo->auto_auth_online_sta_num;
				autoAuthUserLostConnectionCnt += acStationInfo->auto_auth_sta_drop_cnt;
			}
			Free_show_all_wtp_station_statistic_information_cmd(acStationInfo);
		}
	}
	free_instance_parameter_list(&paraHead);
	
	sysinfo(&info); 		
	update_time_show_ac_station_information_cmd = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for show_ac_station_information_cmd\n");
}

