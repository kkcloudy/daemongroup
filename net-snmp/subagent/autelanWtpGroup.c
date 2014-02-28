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
* Initialization all ap infomation  table
*
*
*******************************************************************************/


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "autelanWtpGroup.h"
#include <string.h>
/** Initializes the autelanWtpGroup module */
void
init_autelanWtpGroup(void)
{
  /* here we initialize all the tables we're planning on supporting */

    get_dev_oid(enterprise_pvivate_oid);
    strcat(enterprise_pvivate_oid,"6.1");
    init_dot11WtpInfoTable();
    init_dot11WtpRealTimeCollectTable();
    init_dot11WtpSignalTable();
    init_dot11WtpEthPortTable();
    init_dot11WtpDataPktsTable();
    init_dot11WtpStatsTable();
    init_dot11RadioStatsTable();
    init_dot11WtpParaTable();
    init_dot11WtpChannelTable();
    init_dot11WtpIfTable();
    init_dot11WtpWirelessIfTable();
    init_dot11WtpWiredIfStatsTable();
    init_dot11WtpWirelessIfConfigTable();
    init_dot11WtpWirelessIfstatsTable();
    init_dot11NewWtpWirelessIfstatsTable();
    init_dot11NewWtpWirelessIfConfigTable();
    init_dot11NewWtpIfTable();
    init_dot11NewWtpWiredIfStatsTable();
    init_dot11WtpWirelessCapStatTable();
    init_dot11WtpBssTable();
    init_dot11WtpStaTable();
    init_dot11WtpEndStaInfoTable();
    init_dot11WtpWlanFaultTable();
   // init_dot11WtpWlanDataPktsTable();
    init_dot11WtpWlanStatsTable();  	 
    init_dot11SSIDStatsTable();
    init_dot11RadioParaTable();  	 
    init_dot11SSIDInfoTable();
    init_dot11SecurityMechTable();
    init_dot11UsrLinkTable();
    init_dot11ConjunctionTable();
    init_dot11DistinguishTable();    	 
    init_dot11VlanAbilityTable();  	 
    init_dot11WtpDeviceInfoTable();
    //init_dot11WtpKeyConfigTable();
    init_dot11RogueAPTable();	 
    init_dot11ChannelTable();
    init_dot11ConfigRadioTable();
    init_wtpSwitch();
    init_dot11RadioStatisticsTable();
    init_dot11WtpConfigFileUpdateTable();
    init_dot11WtpVersionFileUpdateTable();
    init_dot11WtpExtensionTable();
    init_widStatistics();
    init_dot11WtpWidStatisticsTable();
    init_dot11WtpClearWidStatisticsTable();
    init_dot11WidDetectHistoryTable();
    init_dot11WtpWiredIfMulticastTable();
    init_dot11WtpWAPIPerformanceStatsTable();
    init_dot11BssWAPIPerformanceStatsTable();
    init_dot11BSSIDWAPIProtocolConfigTable();
    init_dot11StaWAPIProtocolConfigTable();
    init_wapiBasicInfo();
    init_dot11UnicastEncryption();
    init_dot11AKMSuite();
    init_dot11AKMConfigTable();
    init_dot11UnicastTable();
    init_dot11WtpWAPIConfigTable();
    init_dot11DHCPIpAddressPoolTable();
    init_dot11MaxWtpIDInfo();	
    init_dot11RadioWlanTable();
    init_dot11SSIDExternStatsTable();
    init_dot11SSIDRateStatsTable();
    init_dot11SSIDConfigTable();
    init_dot11SSIDVlanTable();
    //init_dot11VlanConfigTable();	
    init_dot11WtpStatisticsTable();
    init_dot11WtpNetworkaddrTable();
	init_dot11DHCPIpPoolConfigTable();
	init_dot11WtpBssIDNumTable();
	init_dot11NeighborAPTable();
	init_dot11WlanDataPktsTable();
	init_dot11WlanRadioDataTable();
	init_dot11APInfoTable();	
	init_dot11WtpTeminalTable();
	init_dot11PortalUserInfoTable();
	init_dot11WtpBssidTeminalTable();
	init_dot11WtpWirelessAuthTable();
	init_dot11NewWtpVersionFileUpdateTable();
	init_dot11NewNeighborAPTable();
	init_dot11BssStaInfoTable();
	init_dot11QuitWtpInfoTable();	
	init_dot11RunWtpInfoTable();
	init_dot11APSummaryInfoTable();
	init_dot11ApInfoTableByInterface();
	init_dot11WtpIfTeminalTable();
	init_dot11SsidTeminalTable();
	init_dot11DHCPIpv6PoolConfigTable();
	init_dot11DHCPIpv6UseageTable();
}






