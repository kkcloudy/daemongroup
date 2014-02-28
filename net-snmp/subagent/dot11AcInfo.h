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
* dot11AcInfo.h
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

#ifndef DOT11ACINFO_H
#define DOT11ACINFO_H

/* function declarations */
void init_dot11AcInfo(void);
Netsnmp_Node_Handler handle_acSoftwareName;
Netsnmp_Node_Handler handle_acSoftwareVersion;
Netsnmp_Node_Handler handle_acRunningTime;
Netsnmp_Node_Handler handle_acSampleTime;
Netsnmp_Node_Handler handle_acSystemTime;
Netsnmp_Node_Handler handle_acSoftwareVendor;
Netsnmp_Node_Handler handle_acContactInfo;
Netsnmp_Node_Handler handle_acProductName;
Netsnmp_Node_Handler handle_acLocationInfo;
Netsnmp_Node_Handler handle_acDomain;
Netsnmp_Node_Handler handle_acLatestPollDevTime;
Netsnmp_Node_Handler handle_acLogServerAddr;
Netsnmp_Node_Handler handle_acSyslogServerPort;
Netsnmp_Node_Handler handle_acNTPConfig;
Netsnmp_Node_Handler handle_acTimeAfterNTPCal;
Netsnmp_Node_Handler handle_acSTP;
Netsnmp_Node_Handler handle_acSnmpTrapConfig;
Netsnmp_Node_Handler handle_acStatWindowTime;
Netsnmp_Node_Handler handle_acHeartbeatPeriod;
Netsnmp_Node_Handler handle_acSysRestart;
Netsnmp_Node_Handler handle_acSysReset;
Netsnmp_Node_Handler handle_TimeSynPeriod;
Netsnmp_Node_Handler handle_SyslogSvcEnable;
Netsnmp_Node_Handler handle_SyslogReportEventLevel;
Netsnmp_Node_Handler handle_SysObjectId;
Netsnmp_Node_Handler handle_acHeartBeatEnable;
Netsnmp_Node_Handler handle_NormalCollectCycle;
Netsnmp_Node_Handler handle_RtCollectCycle;
Netsnmp_Node_Handler handle_acSoftwareHwSpecr;
Netsnmp_Node_Handler handle_acSoftwareBigVersion;
Netsnmp_Node_Handler handle_acBootImg;
Netsnmp_Node_Handler handle_acWriteConfig;
Netsnmp_Node_Handler handle_sysBackupIdentity;
Netsnmp_Node_Handler handle_sysBackupMode;
Netsnmp_Node_Handler handle_sysBackupStatus;
Netsnmp_Node_Handler handle_sysBackupNetworManageIp;
Netsnmp_Node_Handler handle_sysBackupSwitchTimes;
Netsnmp_Node_Handler handle_sysBackupNetworManageIpv6;
Netsnmp_Node_Handler handle_acslotinfo;

#endif /* DOT11ACINFO_H */
