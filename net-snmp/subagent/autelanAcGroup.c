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
* autelanAcGroup.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* Initialization all ac infomation  table
*
*
*******************************************************************************/


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "autelanWtpGroup.h"



/** Initializes the autelanWtpGroup module */
void
init_autelanAcGroup(void)
{
  /* here we initialize all the tables we're planning on supporting */
  init_dot11AcInfo();
  init_dot11AcDeviceInfo();
  init_dot11AcPara();
  init_dot11AcStats();
  init_dot11RogueTable();
  init_dot11SnmpServiceTable();
  init_dot11AdministratorTable();
  init_dot11VlanConfigTable();
  init_dot11LegalDeviceTable();
  init_dot11PermitConfigTable();
  init_dot11PermitSSIDTable();
  init_dot11AttackMACTable();
  init_dot11CreateWlanTable();
  init_dot11ConfigWlanTable();
  init_dot11WlanStationTable();
  init_dot11WlanStatsTable();
  init_dot11WlanDataTable();
  init_dot11CreateSecurityTable();
  
  #if 0
  init_dot11OpenSecurityTable();
  init_dot11SharedSecurityTable();
  init_dot118021xSecurityTable();
  init_dot11WPAESecurityTable();
  init_dot11WPA2ESecurityTable();
  init_dot11WPAPSecurityTable();
  init_dot11WPA2PSecurityTable();
  init_dot11WAPIPSKSecurityTable();
  init_dot11WAPIAUTHSecurityTable();
  init_dot11SecurityTypeTable();
  #endif
  init_dot11QosBasicConfigTable();
  init_autelanQosProfileTable();
  init_dot11AcQosTable();  
  init_dot11QosWirelessTable();
  init_dot11QosWirelessBasicConfigTable();
  init_dot11BackgroundQosTable();
  init_dot11BestEffortQosTable();
  init_dot11VoiceQosTable();
  init_dot11VideoQosTable();
  init_dot11AcInterface();
  init_dot11AcVersionAndConfigTable();
 //init_dot11StationinfoTable();
  init_dot11DHCP();
  init_dot11CreateWtpTable();
  init_dot11ConfigWtpTable();
  init_dot11AcStorageInfo();
  init_dot11RogueStaTable();
  init_dot11IllegalDeviceTable();
  init_dot11NewIllegalDeviceTable();
  init_dot11TerminalBlackListTable();   
  init_dot11ConfigSecurityTable();
  init_dot11ConfigPskTable();
  init_dot11ConfigWepTable();
  init_dot11ConfigWapiTable();
  init_dot11ConfigRadiusAuthServerTable();
  init_dot11ConfigRadiusAccServerTable();
  init_dot11StationinfoTable();
  init_dot11SSIDBindTable();
//  init_dot11AcIfTable();// in zebra to process it   shaojunwu 20100920;
  init_dot11AcIfCapabilityTable();
  init_dot11AcInterfaceTable();
//init_dot11TrapTable();
	init_dot11SpeedLimitTable();
	init_dot11SpeedLimit();
	init_dot11WIDPolicy();
	init_dot11ApProfileTable();
	init_dot11WlanProfileTable();
	init_dot11RadioProfileTable();
	init_dot11InformationInfo();	
	init_dot11AlarmConfiguration();
	//init_dot11TrapDesIPAddGroupTable();
	init_dot11ConfigTrapGroupTable();
	init_dot11ConfTotalTrapGroupTable();
	init_dot11CreateQosTable();
	//init_dot11AcVersionFile();/*new*/
	init_dot11VersionFile();
	init_dot11ConfigNasGroupTable();
	//init_dot11AcConfigurationFile();/*new*/
	init_dot11ConfigurationFile();
	init_dot11AcConfigurationFile();
	init_dot11WtpVersionFileGroupUpdate();
	init_dot11AcVersionFile();
	init_dot11TotalNasIDGroup();
	//init_dot11WlanDataPktsTable();
	init_dot11ConfigPortalServerGroupTable();
	init_dot11ConfigIpGroupTable();
	init_dot11ConfigVlanGroupTable();
	init_dot11ConfigSysLogServerTable();
	init_dot11ConfigPortalRadiusAuthServerTable();
	init_dot11PermitBSSIDTable();
	init_dot11ConfigTrapSwitchTable();
	init_dot11HansiTable();
	init_dot11StaticRouteTable();
	init_dot11ConfigSyslogGlobal();
	init_dot11TerminalWhiteListTable();		
	init_dot11DHCPUseageTable();
	init_dot11NasPortTable();
	init_dot11RWVlanTable();
	init_dot11NewNasPortTable();
	init_dot11PortalWhiteListTable();
	init_dot11PortalBlackListTable();
	init_dot11NewConfigVlanGroupTable();
	init_dot11ConfigEagServerGroupTable();
	init_dot11PortalNoAuthenticatedTable();
	init_dot11AcPhyPortTable();
	init_dot11AcPhyPortCapabilityTable();
}



