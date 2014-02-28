#ifndef WEB_SNMP_DETAIL_CONFIG
#define WEB_SNMP_DETAIL_CONFIG

//#include <dbus/dbus.h>
//#include "dbus/wcpss/ACDbusDef.h"
//#include "dbus/asd/ASDDbusDef.h"

//#include "../ws_init_dbus.h"

// ap inner
#define wtpDownTrap    						"wtpDownTrap"						//0.1
#define wtpSysStartTrap 					"wtpSysStartTrap"					//0.2
#define wtpChannelModifiedTrap  			"APMtRdoChanlChgTrap"			//0.3
#define wtpIPChangeAlarmTrap				"wtpIPChangeAlarmTrap"				//0.4
#define wtpFlashWriteFailTrap				"wtpFlashWriteFailTrap"				//0.5
#define wtpColdStartTrap					"wtpColdStartTrap"					//0.6
#define wtpAuthModeChangeTrap				"wtpAuthModeChangeTrap"				//0.7
#define wtpPreSharedKeyChangeTrap			"wtpPreSharedKeyChangeTrap"			//0.8
#define wtpElectrifyRegisterCircleTrap		"wtpElectrifyRegisterCircleTrap"	//0.9
#define wtpAPUpdateTrap						"wtpAPUpdateTrap"					//0.10

#define wtpCoverholeTrap					"CoverHoleTrap"					//0.11
#define wtpWirePortExceptionTrap			"wtpWirePortExceptionTrap"			//0.12  not support now
#define CPUusageTooHighTrap					"APCPUusageTooHighTrap"				//0.13
#define CPUusageTooHighRecovTrap			"APCPUusageTooHighRecovTrap"			//0.14
#define MemUsageTooHighTrap					"APMemUsageTooHighTrap"				//0.15
#define MemUsageTooHighRecovTrap			"APMemUsageTooHighRecovTrap"			//0.16
#define TemperTooHighTrap					"APTemperatureTooHighTrap"					//0.17
#define TemperTooHighRecoverTrap			"APTemperTooHighRecovTrap"			//0.18
#define APMtWorkModeChgTrap					"APMtWorkModeChgTrap"				//0.19
#define APswitchBetweenACTrap				"APswitchBetweenACTrap"				//0.20 not support now

#define SSIDkeyConflictTrap					"SSIDkeyConflictTrap"				//0.21
#define wtpOnlineTrap						"APOnlineTrap"						//0.22
#define wtpOfflineTrap						"APOfflineTrap"					//0.23
#define wtpCoverHoleClearTrap				"CoverHoleRecovTrap"				//0.24

//ap app
#define wtpChannelObstructionTrap			"wtpChannelObstructionTrap"				//1.1
#define wtpAPInterferenceDetectedTrap		"CoChAPInterfDetectedTrap" 		//1.2
#define wtpStaInterferenceDetectedTrap		"StaInterfDetectedTrap"		//1.3
#define wtpDeviceInterferenceDetectedTrap	"wtpDeviceInterferenceDetectedTrap"		//1.4
#define wtpSubscriberDatabaseFullTrap		"APStaFullTrap"			//1.5
#define wtpDFSFreeCountBelowThresholdTrap	"wtpDFSFreeCountBelowThresholdTrap"		//1.6
#define wtpFileTransferTrap					"wtpFileTransferTrap"					//1.7
#define wtpStationOffLineTrap				"wtpStationOffLineTrap"					//1.8
#define wtpSolveLinkVarifiedTrap			"wtpSolveLinkVarifiedTrap"				//1.9
#define wtpLinkVarifyFailedTrap				"wtpLinkVarifyFailedTrap"				//1.10

#define wtpStationOnLineTrap				"wtpStationOnLineTrap"					//1.11				
#define APInterfClearTrap					"CoChAPInterfClearTrap"						//1.12
#define APStaInterfClearTrap				"StaInterfClearTrap"					//1.13
#define APOtherDeviceInterfDetectedTrap		"OtherDeviceInterfDetectedTrap"		//1.14 not support now
#define APOtherDevInterfClearTrap			"OtherDevInterfClearTrap"				//1.15
#define APModuleTroubleTrap					"ModuleTroubleTrap"					//1.16
#define APModuleTroubleClearTrap			"ModuleTroubleClearTrap"				//1.17
#define APRadioDownTrap						"RadioDownTrap"						//1.18
#define APRadioDownRecovTrap				"RadioDownRecovTrap"					//1.19
#define APTLAuthenticationFailedTrap		"APTLAuthenticationFailedTrap"			//1.20

#define	APStaAuthErrorTrap					"StaAuthErrorTrap"					//1.21
#define APStAssociationFailTrap				"StAssociationFailTrap"				//1.22
#define APUserWithInvalidCerInbreakNetTrap	"UserWithInvalidCerficationInbreakNetworkTrap"	//1.23
#define APStationRepititiveAttackTrap		"StationRepititiveAttackTrap"			//1.24
#define APTamperAttackTrap					"TamperAttackTrap"					//1.25
#define APLowSafeLevelAttackTrap			"LowSafeLevelAttackTrap"				//1.26
#define APAddressRedirectionAttackTrap		"AddressRedirectionAttackTrap"		//1.27
#define APMeshAuthFailureTrap				"APMeshAuthFailureTrap"					//1.28
#define APChildExcludedParentTrap			"APChildExcludedParentTrap"				//1.29
#define APParentChangeTrap					"APParentChangeTrap"					//1.30
	
#define APChildMovedTrap					"APChildMovedTrap"						//1.31
#define	APExcessiveParentChangeTrap			"APExcessiveParentChangeTrap"			//1.32
#define APMeshOnsetSNRTrap					"APMeshOnsetSNRTrap"					//1.33
#define APMeshAbateSNRTrap					"APMeshAbateSNRTrap"					//1.34
#define APConsoleLoginTrap					"APConsoleLoginTrap"					//1.35
#define APQueueOverflowTrap					"APQueueOverflowTrap"					//1.36
#define APAddUserFailClearTrap				"APStaFullRecoverTrap"				//1.37
#define APChannelTooLowClearTrap			"APChannelTooLowClearTrap"				//1.38
#define APFindUnsafeESSID					"WIDSUnauthorSSIDTrap"						//1.39
#define APFindSYNAttack						"WIDSDetectAttack"						//1.40
#define ApNeighborChannelInterfTrap			"AdjacentChAPInterfDetectedTrap"			//1.41
#define ApNeighborChannelInterfTrapClear	"AdjacentChAPInterfClearTrap"		//1.42

//ac inner
#define acSystemRebootTrap					"SystemWarmStartTrap"					//0.1
#define acAPACTimeSynchroFailureTrap		"APACTimeSyncFailureTrap"			//0.2
#define acChangedIPTrap						"IPAddChangeTrap"						//0.3
#define acTurntoBackupDeviceTrap			"ACTurnToBackupDeviceTrap"				//0.4
#define acConfigurationErrorTrap			"ConfigurationErrorTrap"				//0.5
#define acSysColdStartTrap					"SystmColdStartTrap"					//0.6
#define acHeartbeatTrap						"ACHeartbeatTrap"						//0.7

#define acAPACTimeSynchroFailureTrapClear   "acAPACTimeSynchroFailureTrapClear"		//0.22

#define acBoardExtractTrap					"acBoardExtractTrap"					//0.24
#define acBoardInsertTrap					"acBoardInsertTrap"						//0.25
#define acPortDownTrap						"acPortDownTrap"						//0.26
#define acPortUpTrap						"acPortUpTrap"							//0.27


//ac app 
#define acDiscoveryDangerAPTrap				"WIDSDetectRogueTrap"				//1.1
#define acRadiusAuthenticationServerNotReachTrap		"RadiusAuthServerUnavailableTrap"	//1.2
#define acRadiusAccountServerNotLinkTrap	"RadioAccServerUnavailableTrap"		//1.3
#define acPortalServerNotReachTrap			"PortalServerUnavaibleTrap"			//1.4
#define acAPLostNetTrap						"acAPLostNetTrap"						//1.5
#define acCPUUtilizationOverThresholdTrap	"ACCPUusageTooHighTrap"		//1.6
#define acMemUtilizationOverThresholdTrap	"ACMemUsageTooHighTrap"		//1.7
#define acBandwithOverThresholdTrap			"acBandwithOverThresholdTrap"			//1.8
#define acLostPackRateOverThresholdTrap		"acLostPackRateOverThresholdTrap"		//1.9
#define acMaxUsrNumOverThresholdTrap		"acMaxUsrNumOverThresholdTrap"			//1.10

#define acDisconnectNumOverThresholdTrap	"acDisconnectNumOverThresholdTrap"		//1.11
#define acDropRateOverThresholdTrap			"acDropRateOverThresholdTrap"			//1.12
#define acOfflineSucRateBelowThresholdTrap	"acOfflineSucRateBelowThresholdTrap"	//1.13
#define acAuthSucRateBelowThresholdTrap		"acAuthSucRateBelowThresholdTrap"		//1.14
#define acAveIPPoolOverThresholdTrap		"acAveIPPoolOverThresholdTrap"			//1.15
#define acMaxIPPoolOverThresholdTrap		"acMaxIPPoolOverThresholdTrap"			//1.16
#define acDHCPSucRateOverThresholdTrap		"acDHCPSucRateOverThresholdTrap"		//1.17
#define acPowerOffTrap						"PowerOffTrap"						//1.18
#define acPowerOffRecovTrap					"PowerOffRecovTrap"					//1.19
#define acCPUusageTooHighRecovTrap			"ACCPUusageTooHighRecovTrap"			//1.20

#define acMemUsageTooHighRecovTrap			"ACMemUsageTooHighRecovTrap"			//1.21
#define acTemperTooHighTrap					"ACTemperatureTooHighTrap"					//1.22
#define acTemperTooHighRecovTrap			"ACTemperTooHighRecovTrap"				//1.23
#define acDHCPAddressExhaustTrap			"ACDHCPAddressExhaustTrap"				//1.24
#define acDHCPAddressExhaustRecovTrap		"ACDHCPAddressExhaustRecovTrap"			//1.25
#define acRadiusAuthServerAvailableTrap		"RadiusAuthServerAvailableTrap"		//1.26
#define acRadiusAccServerAvailableTrap		"RadiusAccServerAvailableTrap"		//1.27
#define acRorterServerAvailableTrap			"PortalServerAvalibleTrap"			//1.28
#define acFindAttackTrap					"acFindAttackTrap"						//1.29

#define acHeartTimePackageTrap				"ACHeartbeatTrap"				//1.50


/**********************trap discr des******************************/
#define PRIVATE_TRAP_DESCR_AP_CPU_OVER_THRESHOLD 			"AP CPU利用率过高告警"
#define PRIVATE_TRAP_DESCR_AP_CPU_OVER_THRESHOLD_CLEAR 		"AP CPU利用率过高告警清除"

#define PRIVATE_TRAP_DESCR_AP_MEM_OVER_THRESHOLD			"AP 内存利用率过高告警"
#define PRIVATE_TRAP_DESCR_AP_MEM_OVER_THRESHOLD_CLEAR		"AP 内存利用率过高告警清除"

#define PRIVATE_TRAP_DESCR_AP_OFF_LINE						"AP下线告警"
#define PRIVATE_TRAP_DESCR_AP_ON_LINE						"AP上线告警"

#define PRIVATE_TRAP_DESCR_AP_MTWORK_CHANGE					"AP无线监视工作模式变更通告"
#define PRIVATE_TRAP_DESCR_AP_FLASH_WIRTE_FAIL				"AP软件升级失败通告"
#define PRIVATE_TRAP_DESCR_AP_SSID_KEY_CONFLICT				"SSID密钥冲突通告"
#define PRIVATE_TRAP_DESCR_AP_COVER_HOLE					"覆盖漏洞告警"
#define PRIVATE_TRAP_DESCR_AP_COVER_HOLE_CLEAR				"覆盖漏洞告警清除"

#define PRIVATE_TRAP_DESCR_AP_FILE_TRANSFERED				"文件传送告警"
#define PRIVATE_TRAP_DESCR_AP_LINK_VARIFIED_FAIL			"链路认证失败告警"
#define PRIVATE_TRAP_DESCR_AP_LINK_VARIFIED_FAIL_CLEAR		"链路认证失败告警清除"

#define PRIVATE_TRAP_DESCR_AP_DOWN							"Ap关机告警"
#define PRIVATE_TRAP_DESCR_AP_REBOOT						"AP重启告警"
#define PRIVATE_TRAP_DESCR_AP_COLD_REBOOT					"AP冷启动告警"
#define PRIVATE_TRAP_DESCR_AP_SHAREKEY_CHANGE				"预共享密钥密码改变告警"
#define PRIVATE_TRAP_DESCR_AP_ELECTRIFY_REG_CIRCLE			"上电注册周期告警"
#define PRIVATE_TRAP_DESCR_AP_IP_CHANGE						"AP的IP地址改变告警"
#define PRIVATE_TRAP_DESCR_AP_AUTH_MODE_CHANGE				"AP的认证方式改变告警"



#define PRIVATE_TRAP_DESCR_AP_INTERFERE_DETECT				"邻近AP干扰告警"
#define PRIVATE_TRAP_DESCR_AP_STA_INTERFERE_DETECT			"终端干扰告警"
#define PRIVATE_TRAP_DESCR_AP_DEVICE_INTERFERE_DETECT		"其他设备干扰告警"
#define PRIVATE_TRAP_DESCR_AP_ATH_MOUDLE_ERROR				"无线模块故障告警"
#define PRIVATE_TRAP_DESCR_AP_ATH_MOUDLE_ERROR_CLEAR		"无线模块故障告警清除"

#define PRIVATE_TRAP_DESCR_AP_LINKDOWN_ERROR				"无线链路中断告警"
#define PRIVATE_TRAP_DESCR_AP_LINKDOWN_ERROR_CLEAR			"无线链路中断告警清除"

#define PRIVATE_TRAP_DESCR_AP_DENY_STA						"AP无法增加新的移动用户告警"
#define PRIVATE_TRAP_DESCR_AP_DENY_STA_CLEAR				"AP无法增加新的移动用户告警清除"

#define PRIVATE_TRAP_DESCR_AP_CHANNEL_COUNT_MINOR			"可供使用的信道数过低告警"
#define PRIVATE_TRAP_DESCR_AP_CHANNEL_COUNT_MINOR_CLEAR		"可供使用的信道数过低告警清除"

#define PRIVATE_TRAP_DESCR_AP_CHANNEL_CHANGE				"无线信道变更通告"



#define PRIVATE_TRAP_DESCR_AP_STATION_AUTH_FAIL				"端站鉴权失败通告"
#define PRIVATE_TRAP_DESCR_AP_STATION_ASSOC_FAIL			"端站关联失败通告"



#define PRIVATE_TRAP_DESCR_AP_INVALID_CER_USR_ATTACK		"非法证书用户侵入网络通告"
#define PRIVATE_TRAP_DESCR_AP_CLIENT_RE_ATTACK				"客户端重放攻击通告"
#define PRIVATE_TRAP_DESCR_AP_TAMPER_ATTACK					"篡改攻击通告"
#define PRIVATE_TRAP_DESCR_AP_LOW_SAFE_ATTACK				"安全等级降低攻击通告"
#define PRIVATE_TRAP_DESCR_AP_ADDR_REDIR_ATTACK				"地址重定向攻击通告"



#define PRIVATE_TRAP_DESCR_AC_CONFIG_ERROR					"AC配置错误告警"
#define PRIVATE_TRAP_DESCR_AC_DOWN							"AC电源掉电告警"
#define PRIVATE_TRAP_DESCR_AC_DOWN_CLEAR					"AC电源掉电告警恢复"

#define PRIVATE_TRAP_DESCR_AC_CPU_OVER_THRESHOLD			"AC CPU利用率过高告警"
#define PRIVATE_TRAP_DESCR_AC_CPU_OVER_THRESHOLD_CLEAR		"AC CPU利用率过高告警清除"

#define PRIVATE_TRAP_DESCR_AC_MEM_OVER_THRESHOLD			"AC 内存利用率过高告警"
#define PRIVATE_TRAP_DESCR_AC_MEM_OVER_THRESHOLD_CLEAR		"AC 内存利用率过高告警清除"

#define PRIVATE_TRAP_DESCR_AC_TURNTO_BACKUP					"AC发生主备切换告警"
#define PRIVATE_TRAP_DESCR_AC_DHCP_EXHAUST					"AC的DHCP可分配地址耗尽告警"
#define PRIVATE_TRAP_DESCR_AC_DHCP_EXHAUST_CLEAR			"AC的DHCP可分配地址耗尽告警清除"

#define PRIVATE_TRAP_DESCR_AC_TIME_SYNC_FAIL				"AC与AP间系统时钟同步失败通告"
#define PRIVATE_TRAP_DESCR_AC_TIME_SYNC_FAIL_CLEAR			"AC与AP间系统时钟同步失败通告清除"
#define PRIVATE_TRAP_DESCR_AC_COLD_REBOOT					"AC系统冷启动通告"
#define PRIVATE_TRAP_DESCR_AC_REBOOT						"AC系统热启动通告"
#define PRIVATE_TRAP_DESCR_AC_IP_CHANGE						"AC IP地址变更通告"
#define PRIVATE_TRAP_DESCR_AC_HEART_TIME					"AC 心跳周期通告"



#define PRIVATE_TRAP_DESCR_AP_DISCOVER_DANGER_AP			"AP发现可疑设备通告"
#define PRIVATE_TRAP_DESCR_AP_FIND_UNSAFE_SSID				"AP发现未授权SSID通告"
#define PRIVATE_TRAP_DESCR_AP_SYN_ATTACK					"AP发现攻击通告"



#define PRIVATE_TRAP_DESCR_AC_RADIUS_AUTH_ERROR				"Radius认证服务器不可达告警"
#define PRIVATE_TRAP_DESCR_AC_RADIUS_ACCOUNT_ERROR			"Radius计费服务器不可达告警"
#define PRIVATE_TRAP_DESCR_AC_RADIUS_AUTH_ERROR_CLEAR		"Radius认证服务器不可达告警清除"
#define PRIVATE_TRAP_DESCR_AC_RADIUS_ACCOUNT_ERROR_CLEAR	"Radius计费服务器不可达告警清除"

#define PRIVATE_TRAP_DESCR_AC_PORTAL_ERROR					"Portal服务器不可达告警"
#define PRIVATE_TRAP_DESCR_AC_PORTAL_ERROR_CLEAR			"Portal服务器不可达告警清除"


#define PRIVATE_TRAP_DESCR_AC_BANDWITH_TOO_HIGH				"AC带宽利用率超过阀值告警"
#define PRIVATE_TRAP_DESCR_AC_LOSTPKG_TOO_HIGH				"AC丢包率超过阀值告警"
#define PRIVATE_TRAP_DESCR_AC_MAX_ONLINEUSR_TOO_HIGH		"AC最大在线用户数过高告警"
#define PRIVATE_TRAP_DESCR_AC_RADIUS_AUTH_TOO_LOW			"AC RADIUS认证成功率过低告警"
#define PRIVATE_TRAP_DESCR_AC_IPPOOL_AVE_RATE_TOO_HIGH		"AC IP地址池平均使用率过高告警"
#define PRIVATE_TRAP_DESCR_AC_IPPOOL_MAX_RATE_TOO_HIGH		"AC IP地址池最大使用率过高告警"
#define PRIVATE_TRAP_DESCR_AC_FIND_SYN_ATTACK				"AC SYN攻击告警"


#define PRIVATE_TRAP_DESCR_AC_TEMPERATUE_TOO_HIGH			"AC温度超过阀值告警"
#define PRIVATE_TRAP_DESCR_AC_TEMPERATUE_TOO_HIGH_CLEAR		"AC温度超过阀值告警清除"

#define PRIVATE_TRAP_DESCR_AP_LOST_NET						"AP脱离网络告警"
#define PRIVATE_TRAP_DESCR_STA_OFFLINE						"STATION下线告警"	
#define PRIVATE_TRAP_DESCR_STA_ONLINE						"STATION上线告警"
#define PRIVATE_TRAP_DESCR_DEVICE_INTF_CLEAR				"设备干扰告警清除"	
#define PRIVATE_TRAP_DESCR_AP_INTF_CLEAR					"AP干扰告警清除"
#define PRIVATE_TRAP_DESCR_STA_INTF_CLEAR					"station干扰告警清除"
#define PRIVATE_TRAP_DESCR_COVER_HOLE_CLEAR					"覆盖漏洞告警清除"
#define PRIVATE_TRAP_DESCR_AP_TEMP_TOO_HIGH					"AP温度过高告警"
#define PRIVATE_TRAP_DESCR_AP_TEMP_TOO_HIGH_CLEAR			"AP温度过高告警清除"

#define PRIVATE_TRAP_DESCR_AP_ONLINE						"AP上线告警"
#define PRIVATE_TRAP_DESCR_AP_OFFLINE						"AP下线告警"
#define PRIVATE_TRAP_DESCR_AP_WIDS_ATTACK					"AP发现WIDS攻击告警"

#define PRIVATE_TRAP_DESCR_AC_POWER_EXCHANGE				"AC主备电源切换告警"

/**************************END*********************************/

typedef struct {
	char trapName[128];
	char trapDes[128];
	char trapEDes[128];
	unsigned char  trapSwitch;
	unsigned char  trap_level;
}TRAP_DETAIL_CONFIG;


#if 0
#define TRAP_CONFIG_PATH	"/opt/www/htdocs/trap/all_trap_conf.xml"
#else
#define TRAP_CONFIG_PATH	"/opt/services/conf/trapconf_conf.conf"/*开关文件，自动创建*/
#define TRAP_STATUS_PATH	"/opt/services/status/trapconf_status.status"/*for srvload!*/

#endif

#define XML_ROOT_NODE_NAME	"trap_conf"

#define XML_CHAR			(const char *)

#define TRAP_RTN_OK		0
#define TRAP_RTN_ERR	-1


#define create_trap_xml()\
{\
	int ret_create=_general_trap_xml(TRAP_CONFIG_PATH);\
	if (ret_create < 0)\
	{\
		return TRAP_RTN_ERR;\
	}\
}

int init_trap_config();

TRAP_DETAIL_CONFIG * load_trap_config(int index);
int get_trap_config_num();
char *get_trapName_byindex(int num);
char *get_trapDes_byindex(int num);
char *get_trapDes_byindex(int num);
int get_trapSwitch_byindex(int num);
int mod_trap_conf(TRAP_DETAIL_CONFIG *conf, int index);
int write_conf_into_xml();
int _general_trap_xml(char *file_path);





#endif
