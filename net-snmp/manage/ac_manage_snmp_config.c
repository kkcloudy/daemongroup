#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dbus/dbus.h>
#include <syslog.h>
#include <sys/wait.h>
#include "board/board_define.h"
#include "ws_dbus_def.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"

#include "ws_dbus_list_interface.h"
#include "ac_manage_public.h"

#include "ac_manage_snmp_config.h"


STSNMPSummary snmpSummary = { 0 };
STTRAPSummary trapSummary = { 0 };

unsigned int snmpDebugLevel = 0;
unsigned int trapDebugLevel = 0;

unsigned int manual_master = 0;

PRODUCT_OID_SERIAL	snmp_product_serial[] = {
		{ 0x7,  "7", "605" 	},
		{ 0x8,  "8", "610"  }
};

BORAD_OID_TYPE	snmp_borad_type[] = {
		{ 0x0,  "BOARD_TYPE_AX81_SMU" 	},
		{ 0x1,  "BOARD_TYPE_AX81_AC12C" },
		{ 0x2,  "BOARD_TYPE_AX81_AC8C" 	},
		{ 0x4,  "BOARD_TYPE_AX71_2X12G12S" },
		{ 0x80, "BOARD_TYPE_AX71_CRSMU" }
};



TRAP_DETAIL_CONFIG ALL_TRAP[] = {
/* ap inner */
//0.1
	{wtpDownTrap, 								"Ap关机告警",					"AP is down", 						0},
	{wtpSysStartTrap, 							"AP重启告警",					"AP reboot", 						0},
	{wtpChannelModifiedTrap, 					"AP信道变更通告",				"channel is changed", 				0},
	{wtpIPChangeAlarmTrap, 						"AP的IP地址改变告警",			"AP ip address changed", 			0},
	{wtpFlashWriteFailTrap,						"AP Flash写失败通告",			"AP flash write failed", 				0},
	{wtpColdStartTrap, 							"AP冷启动告警",					"AP cold boot", 					0},
	{wtpAuthModeChangeTrap, 					"AP加密方式改变告警",			"AP auth mode changed",				0},
	{wtpPreSharedKeyChangeTrap, 				"预共享密钥密码改变告警",		"Pre-share-key is changed", 		0},
	{wtpElectrifyRegisterCircleTrap,			"上电注册周期告警", 			"Electricity-reg-circle is on",		0},
	{wtpAPUpdateTrap,	 						"AP升级告警",					"ap update", 						0},
//0.11
	{wtpCoverholeTrap, 							"覆盖漏洞告警",					"AP cover hole", 					0},
	// 0.12 不支持
	{CPUusageTooHighTrap, 						"AP CPU利用率过高告警", 		"AP CPU usage is over threshold", 	0},
	{CPUusageTooHighRecovTrap, 					"AP CPU利用率过高告警清除",		"AP CPU usage recovery", 			0},
	{MemUsageTooHighTrap, 						"AP 内存利用率过高告警",		"AP MEM usage is over threshold", 	0},
	{MemUsageTooHighRecovTrap, 					"AP 内存利用率过高告警清除",	"AP MEM usage recovery", 			0},
	{TemperTooHighTrap, 						"AP温度过高告警",				"AP temperature is too high", 		0},
	{TemperTooHighRecoverTrap, 					"AP温度过高告警清除",			"AP temperature is recover", 		0},
	{APMtWorkModeChgTrap, 						"AP无线监视工作模式变更通告",	"AP MTwork mode is changed", 		0},
	// 0.20 不支持
//0.21
	{SSIDkeyConflictTrap, 						"SSID密钥冲突通告",				"SSID key conflict", 				0},
	{wtpOnlineTrap, 							"AP上线告警",					"AP status is running", 			0},
	{wtpOfflineTrap, 							"AP下线告警",					"AP status is quit", 				0},
	{wtpCoverHoleClearTrap, 					"覆盖漏洞告警清除",				"AP cover hole clear", 				0},
	{wtpSoftWareUpdateSucceed,				    "AP软件升级成功通告", 			"AP software update succeed",			0},
	{wtpSoftWareUpdateFailed,					"AP软件升级失败通告",			"AP software update failed",		0},
/* ap app */
//1.1
	{wtpChannelObstructionTrap,					"信道干扰告警", 				"AP channel obstruction", 			0},
	{wtpAPInterferenceDetectedTrap, 			"同频AP干扰告警", 				"AP interfere has detected", 		0},
	{wtpStaInterferenceDetectedTrap, 			"终端干扰告警",					"Station interfere has detected", 	0},
	{wtpDeviceInterferenceDetectedTrap,			"设备干扰告警",					"Device interfere has detected", 	0},
	{wtpSubscriberDatabaseFullTrap, 			"AP无法增加新的移动用户告警",	"AP can not add station", 			0},
	{wtpDFSFreeCountBelowThresholdTrap,			"可供使用的信道数过低告警",	 	"Channel count minor", 	 0},
	{wtpFileTransferTrap, 						"文件传送告警",					"File is transfered", 				0},
	{wtpStationOffLineTrap, 					"STATION下线告警",				"wtp station offline", 				0},
	{wtpSolveLinkVarifiedTrap, 					"链路认证失败告警清除",			"Link varified fail clear", 		0},
	{wtpLinkVarifyFailedTrap, 					"链路认证失败告警",				"Link varified fail", 				0},
//1.11
	{wtpStationOnLineTrap, 						"STATION上线告警",				"station has been on line", 		0},
	{APInterfClearTrap, 						"AP干扰告警清除",				"AP interfere has been clear", 		0},
	{APStaInterfClearTrap, 						"终端干扰告警清除",				"Station interfere has been clear", 0},
	// 1.14 不支持
	{APOtherDevInterfClearTrap, 				"设备干扰告警清除"	,			"device interfere has been clear", 	0},
	{APModuleTroubleTrap, 						"无线模块故障告警",				"Wireless moudle show errors", 		0},
	{APModuleTroubleClearTrap, 					"无线模块故障告警清除",			"Wireless moudle show errors clear",0},
	{APRadioDownTrap, 							"无线链路中断告警",				"Wireless link show errors", 		0},
	{APRadioDownRecovTrap, 						"无线链路中断告警清除",			"Wireless link show errors clear", 	0},
	// 1.20 不支持
//1.21
	{APStaAuthErrorTrap, 						"端站鉴权失败通告",				"Station auth fail", 				0},
	{APStAssociationFailTrap, 					"端站关联失败通告",				"Station assoc fail", 				0},
	{APUserWithInvalidCerInbreakNetTrap, 		"非法证书用户侵入网络通告",		"Invalid certification user attack",0},
	{APStationRepititiveAttackTrap, 			"客户端重放攻击通告",			"Client Re-attack", 				0},
	{APTamperAttackTrap, 						"篡改攻击通告",					"tamper attack", 					0},
	{APLowSafeLevelAttackTrap, 					"安全等级降低攻击通告",			"low safe level attack", 			0},
	{APAddressRedirectionAttackTrap, 			"地址重定向攻击通告",			"address redirect attack", 			0},
//1.28-1.36不支持

//1.37
	{APAddUserFailClearTrap, 					"AP无法增加新的移动用户告警清除","AP can not add station clear",	0},	
	{APChannelTooLowClearTrap, 					"可供使用的信道数过低告警清除",	"Channel count minor clear", 		0},
	{APFindUnsafeESSID, 						"AP发现未授权SSID通告",			"AP find unsafe SSID", 				0},
	{APFindSYNAttack, 							"AP发现攻击通告",				"AP find syn attack", 				0},
	{ApNeighborChannelInterfTrap, 				"临频AP干扰告警",				"AP neighbor channel interfere", 	0},
	{ApNeighborChannelInterfTrapClear, 			"临频AP干扰告警清除", 			"AP neighbor channel interfere clear", 0},
/*ac inner */	
	//0.1
	{acSystemRebootTrap, 						"AC重启动告警",					"AC reboot", 						0},
	{acAPACTimeSynchroFailureTrap, 				"AC与AP间系统时钟同步失败通告",	"AC synoc time fail with ap", 		0},
	{acChangedIPTrap, 							"AC IP地址变更通告",			"AC ip address has changed", 		0},
	{acTurntoBackupDeviceTrap, 					"AC主备切换告警",				"AC instance vrrp state change ",   0},
	{acConfigurationErrorTrap, 					"AC解析配置文件出错告警",		"AC configuration error", 			0},
	{acSysColdStartTrap, 						"AC系统冷启动告警",				"AC system cold start", 			0},
	// 0.7 不支持 

	{acAPACTimeSynchroFailureTrapClear, 		"AC与AP间系统时钟同步失败通告清除",	"AC synoc time success with ap", 		0},
/*ac app*/
	//1.1
	{acDiscoveryDangerAPTrap, 					"AP发现可疑设备通告",			"AC discover danger ap", 			0},
	{acRadiusAuthenticationServerNotReachTrap,	"Radius认证服务器不可达告警",	"Radius auth server can not reach", 0},
	{acRadiusAccountServerNotLinkTrap,			"Radius计费服务器不可达告警",	"Radius accounting server can not reach", 0},
	{acPortalServerNotReachTrap, 				"Portal服务器不可达告警",		"Portal server can not reach",		 0},
	{acAPLostNetTrap, 							"AP脱离网络告警",				"AP has been lost network", 		0},
	{acCPUUtilizationOverThresholdTrap, 		"AC CPU利用率过高告警",			"AC CPU usage is over threshold", 	0},
	{acMemUtilizationOverThresholdTrap, 		"AC 内存利用率过高告警",		"AC MEM usage is over threshold", 	0},
	{acBandwithOverThresholdTrap, 				"AC带宽利用率超过阀值告警",		"AC bandwith is too high", 			0},
	{acLostPackRateOverThresholdTrap, 			"AC丢包率超过阀值告警",			"AC drop pkg rate is too high", 	0},
	{acMaxUsrNumOverThresholdTrap, 				"AC最大在线用户数过高告警",		"AC Max Online user is too high", 	0},

//1.11
	// 1.11-1.13 不支持
	{acAuthSucRateBelowThresholdTrap, 			"AC RADIUS认证成功率过低告警",	"AC radius auth suc rate is too low", 0},
	{acAveIPPoolOverThresholdTrap, 				"AC IP地址池平均使用率过高告警","AC IP pool average rate is too high", 0},
	{acMaxIPPoolOverThresholdTrap, 				"AC IP地址池最大使用率过高告警","AC IP pool max rate is too high", 	0},
	//1.17 不支持
	{acPowerOffTrap, 							"AC电源掉电告警",				"AC power is turned", 				0},
	{acPowerOffRecovTrap, 						"AC电源掉电告警恢复",			"AC power is recovery", 			0},
	{acCPUusageTooHighRecovTrap, 				"AC CPU利用率过高告警清除",		"AC CPU usage is over threshold clear", 0},
	
//1.21
	{acMemUsageTooHighRecovTrap, 				"AC 内存利用率过高告警清除",	"AC MEM usage is over threshold clear", 0},
	{acTemperTooHighTrap, 						"AC温度超过阀值告警",			"AC temperature is too high", 		0},
	{acTemperTooHighRecovTrap, 					"AC温度超过阀值告警清除",		"AC temperature high recovered", 	0},
	{acDHCPAddressExhaustTrap, 					"AC的DHCP可分配地址耗尽告警",	"AC DHCP pool exhaust", 			0},
	{acDHCPAddressExhaustRecovTrap, 			"AC的DHCP可分配地址耗尽告警清除","AC DHCP pool exhaust clear", 		0},
	{acRadiusAuthServerAvailableTrap,			"Radius认证服务器不可达告警清除","radius auth server can reached", 	0},
	{acRadiusAccServerAvailableTrap,			"Radius计费服务器不可达告警清除","radius account server can reached", 0},
	{acPortalServerAvailableTrap, 				"Portal服务器不可达告警清除",	"Portal server can reached", 		0},
	{acFindAttackTrap, 							"AC 发现攻击告警",				"AC find attack", 				0},						
	
//1.50
	{acHeartTimePackageTrap, 					"AC 心跳周期通告",				"AC heart time break", 				0},

//    {acVersionUpdateFailedTrap,                 "AC 版本升级失败通告",          "AC Version Update failed",              0},
//    {acConfigfileErrorTrap,                     "AC 配置文件错误通告",          "AC Config File Error",              0},
	{acMaxUsrNumOverThresholdTrapClear,			"AC最大在线用户数过高告警清除",	"AC Max Online user is too high clear", 	0},
	{wtpStationOffLineAbnormalTrap, 			"station 异常断开通告", 		"station leave abnormal",					0},
	{wtpUserLogoffAbnormalTrap,             	"用户异常下线通告",         	"user logoff abnormal",           			0},
	{wtpConfigurationErrorTrap, 				"ap 配置文件错误告警", 			"ap configuration error", 					0},
	{wtpUserTrafficOverloadTrap,             	"用户收发流量超限告警",         "users to send and receive traffic overload",0},
	{wtpUnauthorizedStaMacTrap,             	"未授权station mac告警",        "Unauthorized Station Mac",           		0},
	{acBoardExtractTrap,						"板卡拔出告警",					"ac board pull out",						0},
	{acBoardInsertTrap,							"板卡插入告警",					"ac board insert",							0},
	{acPortDownTrap,							"端口down告警",					"ac port down",								0},
	{acPortUpTrap,								"端口up告警",					"ac port up",								0}
};


TRAPParameter  Trap_para[] = {
/*
    {STATISTICS_INTERVAL, 300},
    {SAMPLING_INTERVAL, 50},
*/    
    {HEARTBEAT_INTERVAL, 30},
    {HEARTBEAT_MODE, 0},
    
    {RESEND_INTERVAL, 300},
    {RESEND_TIMES, 0},
/*    
    {CPU_THRESHOLD, 90},
    {MEMORY_THRESHOLD, 90},
    {TEMPERATURE_THRESHOLD, 90},
*/
};



static int 
CHECK_INPUT_DIGIT(char * src) {

	if( src == NULL ) {
		return -1;
	}

	char * str = src;
	int i = 0;
	
	while( str[i] != '\0') {
		if( isspace(str[i]) ){
			i++;
		}
		
		if( !isdigit(str[i]) ){
			return -2;
		}
		i++;
		
	}
	return 0;
}


static char *
get_sys_description(void) {

    char *buf = (char *)malloc(HOST_NAME_LENTH);
    if(NULL == buf) {
        return NULL;
    }
    
    FILE *fp = NULL;
	fp = popen("hostname -v", "r");	
	if(NULL == fp) {
	    free(buf);
	    return NULL;
	}
	
	memset(buf, 0, sizeof(HOST_NAME_LENTH));
    fgets(buf, HOST_NAME_LENTH, fp);
    pclose(fp);

    if('\n' == buf[strlen(buf) - 1]) {
        buf[strlen(buf) - 1] = '\0';
    }
    
    return buf;
}


static char * 
get_porduct_oid(void) {

	char *sysOid = (char *)malloc(MAX_OID_LENTH);
	if(NULL == sysOid) {
        return NULL;    
	}
	
	memset(sysOid, 0, MAX_OID_LENTH);
	
    unsigned int status = 1;
    unsigned int product_serial = 0;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		product_serial = manage_get_product_info(PRODUCT_LOCAL_SERIAL);
	}
    //unsigned int borad_type = manage_get_product_info(BORAD_LOCAL_TYPE);
    
    int i = 0;
	for( i = 0; i < (sizeof(snmp_product_serial) / sizeof(snmp_product_serial[0])); i++ ) {
		if(product_serial == snmp_product_serial[i].value) {
			snprintf(sysOid, MAX_OID_LENTH - 1, "%d.%s.%s.%d", status, snmp_product_serial[i].product_type, \
														snmp_product_serial[i].product_node, \
														snmpSummary.snmp_sysinfo.sysoid_boardtype);
			break;
		}
	}
	
	return sysOid;
}



static char * 
get_sys_oid(void)
{    
	char *ret_oid = (char *)malloc(TOTAL_OID_LENGTH);
	if(NULL == ret_oid) {
        return NULL;
	}
	memset(ret_oid, 0, TOTAL_OID_LENGTH);
    
	char temp[50] = { 0 };
	char enterprise_part[ENTERPRISE_OID_LENGTH] = { 0 };
	char product_OID[PRODUCT_OID_LENGTH] = { 0 };
	char *start_oid = ".1.3.6.1.4.1";
    char *product_form = NULL;
    int len;
    
    
    FILE *fp = NULL;
	if(NULL != (fp = popen(ENTERPRISE_NODE_SH,"r")) ) {
		fgets(enterprise_part, ENTERPRISE_OID_LENGTH, fp);
		pclose(fp);
	}
	
	len = strlen(enterprise_part);
	if(enterprise_part[len-1] == '\n')
		enterprise_part[len-1] = '\0';
	
	if((!strcmp(enterprise_part,"0")) || (!strcmp(enterprise_part,"")) || (CHECK_INPUT_DIGIT(enterprise_part) == -2) ) {
		strcpy(enterprise_part,ENTERPRISE_OID);
	}

	if(NULL != (fp = popen(PRODUCT_NODE_SH,"r")) ) {
		fgets(product_OID, ENTERPRISE_OID_LENGTH, fp);
		pclose(fp);
	}
	
	len = strlen(product_OID);
	if(product_OID[len-1] == '\n')
		product_OID[len-1] = '\0';

    product_form = get_porduct_oid();

	if((!strcmp(product_OID, "0")) || (!strcmp(product_OID,"")))
		snprintf(ret_oid, TOTAL_OID_LENGTH - 1, "%s.%s.2.%s", start_oid, enterprise_part, product_form ? product_form : "1.8.610.0");
	else
		snprintf(ret_oid, TOTAL_OID_LENGTH - 1, "%s.%s.%s.2.%s", start_oid, enterprise_part, product_OID, product_form ? product_form : "1.8.610.0");

	MANAGE_FREE(product_form);

	return ret_oid;
}

static char *
get_product_name(void) {

    char *product_name = (char *)malloc(MAX_SYSTEM_NAME_LEN);
    if(NULL == product_name) {
        syslog(LOG_INFO, "get_product_name: MALLOC product_name error!\n");
        return NULL;
    }

	memset(product_name, 0, MAX_SYSTEM_NAME_LEN);

    FILE *fp = NULL;
    fp = fopen(PRODUCT_LOCAL_NAME, "r");
    if(NULL == fp) {
        syslog(LOG_INFO, "get_product_name: open %s failed!\n", PRODUCT_LOCAL_NAME);
        strncpy(product_name, "PRODUCT", MAX_SYSTEM_NAME_LEN - 1);
        return product_name;
    }

    if(NULL == fgets(product_name, MAX_SYSTEM_NAME_LEN, fp)) {
        syslog(LOG_INFO, "get_product_name: fgets %s failed!\n", PRODUCT_LOCAL_NAME);
        strncpy(product_name, "PRODUCT", MAX_SYSTEM_NAME_LEN - 1);
        fclose(fp);
        return product_name;
    }

    if('\n' == product_name[strlen(product_name) - 1]) {
        product_name[strlen(product_name) - 1] = '\0';
    }
    
    fclose(fp);
    return product_name;
}


static void
free_snmp_view_oid_list(struct oid_list **oidHead){
    if(NULL == oidHead)
        return ;

    while(*oidHead) {
        struct oid_list *temp_node = (*oidHead)->next;
        MANAGE_FREE(*oidHead);

        *oidHead = temp_node;
    }   
    *oidHead = NULL;

    return ;
}


static int
write_snmp_config(STSNMPSummary pstSummary, char *file_path ) {

    //char File_content[MAX_FILE_CONTENT] = { 0 };   

    char *File_content= NULL;
    File_content=(char *)malloc(MAX_FILE_CONTENT);
    memset(File_content , 0,  MAX_FILE_CONTENT);

    char syscommand[512] = { 0 };

    char tmp_view[MAX_oid] = { 0 };

    char sec_name[50] = { 0 };

    char group_name[50] = { 0 };

    char user_name[50] = { 0 };

    char auth[10] = { 0 };

    char no_auth[10] = { 0 };
    
    char privacy[10] = { 0 };

    char access_command[10] = { 0 };

    char comm_type[10] = { 0 };

    char mib_sys_name[MAX_MIB_SYSNAME] = { 0 };


    
    if( NULL == file_path) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    /*
        * open snmp config file
        */
    
    FILE *fp = NULL;
    if(NULL == (fp = fopen(file_path,"w+")))
        return AC_MANAGE_FILE_OPEN_FAIL;

    system("sudo chmod 666 "CONF_FILE_PATH);


    /*
        *  snmp bind udp port
        */
    if((pstSummary.snmp_sysinfo.agent_port) &&(pstSummary.snmp_sysinfo.agent_port_ipv6)){
        memset(syscommand, 0, sizeof(syscommand));
        snprintf(syscommand, sizeof(syscommand) - 1, "agentaddress udp:%d,udp6:%d\n\n", 
				pstSummary.snmp_sysinfo.agent_port,pstSummary.snmp_sysinfo.agent_port_ipv6);
        strcat(File_content,syscommand);    
    }
    else if((pstSummary.snmp_sysinfo.agent_port) &&(pstSummary.snmp_sysinfo.agent_port_ipv6 == 0)){
        memset(syscommand, 0, sizeof(syscommand));
        snprintf(syscommand, sizeof(syscommand) - 1, "agentaddress udp:%d,udp6:%d\n\n", 
				pstSummary.snmp_sysinfo.agent_port, 161);
        strcat(File_content,syscommand);    
    }
    else if((pstSummary.snmp_sysinfo.agent_port == 0) &&(pstSummary.snmp_sysinfo.agent_port_ipv6)){
        memset(syscommand, 0, sizeof(syscommand));
        snprintf(syscommand, sizeof(syscommand) - 1, "agentaddress udp:%d,udp6:%d\n\n", 
				161 , pstSummary.snmp_sysinfo.agent_port_ipv6);
        strcat(File_content,syscommand);    
    }
    else if((pstSummary.snmp_sysinfo.agent_port == 0) &&(pstSummary.snmp_sysinfo.agent_port_ipv6 == 0)){
        memset(syscommand, 0, sizeof(syscommand));
        snprintf(syscommand, sizeof(syscommand) - 1, "agentaddress udp:%d,udp6:%d\n\n", 161 , 161);
        strcat(File_content,syscommand);    
    }

    
    int i = 0;
    STCommunity *community_node = NULL;
    for(i = 0, community_node = pstSummary.communityHead; 
        i < pstSummary.community_num && NULL != community_node; 
        i++, community_node = community_node->next)
    {

        if(RULE_ENABLE == community_node->status) {
             /*
                    *  com2sec
                    */
            memset(sec_name, 0, sizeof(sec_name));
            if(ACCESS_MODE_RO == community_node->access_mode) {
                snprintf(sec_name, sizeof(sec_name) - 1, "%s_ReadOnly", community_node->community);
            }
            else if(ACCESS_MODE_RW == community_node->access_mode){
                snprintf(sec_name, sizeof(sec_name) - 1, "%s_ReadWrite", community_node->community);
            }

            memset(syscommand, 0, sizeof(syscommand));
            snprintf(syscommand, sizeof(syscommand) - 1, "com2sec %s %s/%s %s \n", sec_name, 
                    community_node->ip_addr, community_node->ip_mask, community_node->community);
            strcat(File_content, syscommand);


            /*
                    *  group
                    */        
            memset(group_name, 0, sizeof(group_name));
            if(ACCESS_MODE_RO == community_node->access_mode)
                strcpy(group_name , "MyROGroup");
            else if(ACCESS_MODE_RW == community_node->access_mode)
                strcpy(group_name , "MyRWGroup");

            if(RULE_ENABLE == pstSummary.snmp_sysinfo.v1_status) {
                memset(syscommand, 0, sizeof(syscommand));
                snprintf(syscommand, sizeof(syscommand) - 1, "group %s v1 %s \n", group_name, sec_name);
                strcat(File_content , syscommand);
            }
            
            if(RULE_ENABLE == pstSummary.snmp_sysinfo.v2c_status){
                memset(syscommand, 0, sizeof(syscommand));
                snprintf(syscommand, sizeof(syscommand) - 1, "group %s v2c %s \n", group_name, sec_name);
                strcat(File_content , syscommand);
            }
            strcat(File_content,"\n\n");
        }
        
    }

    	i = 0;
	IPV6STCommunity *ipv6_community_node = NULL;
	for(i = 0, ipv6_community_node = pstSummary.ipv6_communityHead; 
	    i < pstSummary.ipv6_community_num&& NULL != ipv6_community_node; 
	    i++, ipv6_community_node = ipv6_community_node->next)
	{
    
	    if(RULE_ENABLE == ipv6_community_node->status) {
		 /*
		*  com2sec6
		*/
		memset(sec_name, 0, sizeof(sec_name));
		if(ACCESS_MODE_RO == ipv6_community_node->access_mode) {
		    snprintf(sec_name, sizeof(sec_name) - 1, "%s_ReadOnly", ipv6_community_node->community);
		}
		else if(ACCESS_MODE_RW == ipv6_community_node->access_mode){
		    snprintf(sec_name, sizeof(sec_name) - 1, "%s_ReadWrite", ipv6_community_node->community);
		}
    
		memset(syscommand, 0, sizeof(syscommand));
		snprintf(syscommand, sizeof(syscommand) - 1, "com2sec6 %s %s/%d %s \n", sec_name, 
			ipv6_community_node->ip_addr, ipv6_community_node->prefix, ipv6_community_node->community);
		strcat(File_content, syscommand);
    
    
		/*
		*  group
		*/	  
		memset(group_name, 0, sizeof(group_name));
		if(ACCESS_MODE_RO == ipv6_community_node->access_mode)
		    strcpy(group_name , "MyROGroup6");
		else if(ACCESS_MODE_RW == ipv6_community_node->access_mode)
		    strcpy(group_name , "MyRWGroup6");
    
		if(RULE_ENABLE == pstSummary.snmp_sysinfo.v1_status) {
		    memset(syscommand, 0, sizeof(syscommand));
		    snprintf(syscommand, sizeof(syscommand) - 1, "group %s v1 %s \n", group_name, sec_name);
		    strcat(File_content , syscommand);
		}
		
		if(RULE_ENABLE == pstSummary.snmp_sysinfo.v2c_status){
		    memset(syscommand, 0, sizeof(syscommand));
		    snprintf(syscommand, sizeof(syscommand) - 1, "group %s v2c %s \n", group_name, sec_name);
		    strcat(File_content , syscommand);
		}
		strcat(File_content,"\n\n");
	    }
	}


    /*
        * 根据v3用户添加v3 group
        * v3group的安全体名为v3用户名
        */    
    STSNMPV3User *v3user_node = NULL;
    if(RULE_ENABLE == pstSummary.snmp_sysinfo.v3_status) { /* V1,V2,V3 的开关是总开关*/
    
        for(i = 0, v3user_node = pstSummary.v3userHead; 
            i < pstSummary.v3user_num && NULL != v3user_node; 
            i++, v3user_node = v3user_node->next) {
            
            memset(syscommand, 0, sizeof(syscommand));
            snprintf(syscommand, sizeof(syscommand) - 1,"group %s usm %s\n", v3user_node->group_name, v3user_node->name);
            strcat(File_content , syscommand);
        }
        strcat(File_content,"\n\n");
    }

    /*
        * 添加视图
        */
    strcat(File_content,"view  all  included  .1  80\n");
    STSNMPView *view_node = NULL;
    for(i = 0, view_node = pstSummary.viewHead; 
        i < pstSummary.view_num && NULL != view_node; 
        i++, view_node = view_node->next ) {
        
        int j = 0, m = 0;  
        struct oid_list *oid_node = NULL;
        for(j = 0, oid_node = view_node->view_included.oidHead;
            j < view_node->view_included.oid_num && NULL != oid_node; 
            j++, oid_node = oid_node->next) {

            memset(tmp_view, 0, sizeof(tmp_view));
            strncpy(tmp_view, oid_node->oid, sizeof(tmp_view) - 1);
            
            for(m = 0; m < MAX_oid; m++)
                if(tmp_view[m] == ':')  tmp_view[m]=' ';

            memset(syscommand, 0, sizeof(syscommand));
            snprintf(syscommand, sizeof(syscommand) - 1,"view  %s  included  %s\n", view_node->name, tmp_view);
            strcat(File_content,syscommand);
        }
        
        for(j = 0, oid_node = view_node->view_excluded.oidHead;
            j < view_node->view_excluded.oid_num && NULL != oid_node; 
            j++, oid_node = oid_node->next) {
            
            memset(tmp_view, 0, sizeof(tmp_view));
            strncpy(tmp_view, oid_node->oid, sizeof(tmp_view) - 1);
            
            for(m = 0; m < MAX_oid; m++)
                if(tmp_view[m] == ':')  tmp_view[m]=' ';

            memset(syscommand, 0, sizeof(syscommand));
            snprintf(syscommand, sizeof(syscommand) - 1,"view  %s  excluded  %s\n", view_node->name, tmp_view);
            strcat(File_content, syscommand);
        }
    }
    strcat(File_content,"\n");


     /*
        * ipv4 v1 v2c 向安全组授权相应的视图
        */   
    for(i = 0, community_node = pstSummary.communityHead;
        i < pstSummary.community_num && NULL != community_node; 
        i++, community_node = community_node->next ) {
    
        if(RULE_ENABLE == community_node->status) {
            if(ACCESS_MODE_RO == community_node->access_mode)
                strcat(File_content,"access  MyROGroup  \"\"  any  noauth  exact  all  none  none\n");
            else if(ACCESS_MODE_RW == community_node->access_mode)
                strcat(File_content,"access  MyRWGroup  \"\"  any  noauth  exact  all  all  all\n");
        }
    }
    strcat(File_content,"\n\n");

    /*
       * ipv6 v1 v2c 向安全组授权相应的视图
       */   
    for(i = 0, ipv6_community_node = pstSummary.ipv6_communityHead;
        i < pstSummary.ipv6_community_num && NULL != ipv6_community_node; 
        i++, ipv6_community_node = ipv6_community_node->next ) {
    
        if(RULE_ENABLE == ipv6_community_node->status) {
            if(ACCESS_MODE_RO == ipv6_community_node->access_mode)
                strcat(File_content,"access  MyROGroup6  \"\"  any  noauth  exact  all  none  none\n");
            else if(ACCESS_MODE_RW == ipv6_community_node->access_mode)
                strcat(File_content,"access  MyRWGroup6  \"\"  any  noauth  exact  all  all  all\n");
        }
    }
    strcat(File_content,"\n\n");


    /*
        * v3 向安全组授权相应的视图
        */
    STSNMPGroup *group_node = NULL;
    for(i = 0, group_node = pstSummary.groupHead; 
        i < pstSummary.group_num, NULL != group_node; 
        i++, group_node = group_node->next) {

        char level_str[10] = { 0 };
        if(SEC_PRIV == group_node->sec_level) {
            strncpy(level_str, "priv", sizeof(level_str) - 1);
        }
        else if(SEC_AUTH == group_node->sec_level) {
            strncpy(level_str, "auth", sizeof(level_str) - 1);
        }
        else {
            strncpy(level_str, "noauth", sizeof(level_str) - 1);
        }
        
        memset(syscommand, 0, sizeof(syscommand));
        if(ACCESS_MODE_RO == group_node->access_mode) {
            snprintf(syscommand, sizeof(syscommand) - 1,"access  %s  \"\"  usm  %s  exact  %s  none  none\n", 
                    group_node->group_name, level_str, group_node->group_view);
        }
        else if(ACCESS_MODE_RW == group_node->access_mode) {
            snprintf(syscommand, sizeof(syscommand) - 1,"access  %s  \"\"  usm  %s  exact  %s  %s  all\n",
                    group_node->group_name, level_str, group_node->group_view, group_node->group_view);
        }
        strcat(File_content,syscommand);    
    }
    strcat(File_content,"\n\n");

    /*
        * 建新v3  用户
        */
    if(RULE_ENABLE == pstSummary.snmp_sysinfo.v3_status) { /* V1,V2,V3 的开关是总开关*/
        for(i = 0, v3user_node = pstSummary.v3userHead; 
            i < pstSummary.v3user_num && NULL != v3user_node; 
            i++, v3user_node = v3user_node->next) {

            memset(user_name, 0, sizeof(user_name));
            strcpy(user_name, v3user_node->name);

            memset(access_command, 0, sizeof(access_command));
            if(ACCESS_MODE_RO == v3user_node->access_mode) {
                strcpy(access_command, "rouser");
            }
            else if(ACCESS_MODE_RW ==  v3user_node->access_mode) {
                strcpy(access_command, "rwuser");
            }
            else{
                strcpy(access_command, "rouser"); //默认rouser
            }

            memset(auth, 0, sizeof(auth));
            if(AUTH_PRO_NONE ==  v3user_node->authentication.protocal) {
                memset(auth, 0, sizeof(auth));
            }
            else if(AUTH_PRO_MD5 ==  v3user_node->authentication.protocal) {
                strcpy(auth,"MD5");
            }
            else if(AUTH_PRO_SHA ==  v3user_node->authentication.protocal){
                strcpy(auth,"SHA");
            }

            memset(privacy, 0, sizeof(privacy));
            if(PRIV_PRO_NONE == v3user_node->privacy.protocal){
                memset(privacy, 0, sizeof(privacy));
            }
            else if(PRIV_PRO_AES ==  v3user_node->privacy.protocal) {
                strcpy(privacy,"AES");
            }
            else if(PRIV_PRO_DES ==  v3user_node->privacy.protocal) {
                strcpy(privacy,"DES");
            }

            if(!( strcmp(auth,"") || strcmp(privacy,""))){
                strcpy(no_auth, "noauth");
            }

            memset(syscommand, 0, sizeof(syscommand));
            snprintf(syscommand, sizeof(syscommand) - 1,"createUser %s %s %s %s %s \n",  v3user_node->name, auth,
                     v3user_node->authentication.passwd, privacy, v3user_node->privacy.passwd);

            strcat(File_content, syscommand);
            strcat(File_content,"\n\n");
        }
    }

    /*
        * get_sys_oid
        */
    memset(syscommand, 0, sizeof(syscommand));
    snprintf(syscommand, sizeof(syscommand) - 1, "sysobjectid %s\n", pstSummary.snmp_sysinfo.sys_oid);
    strcat(File_content,syscommand);
    strcat(File_content,"sysdescr Networking Device \n");

    memset(syscommand, 0, sizeof(syscommand));
    strcpy(mib_sys_name, pstSummary.snmp_sysinfo.sys_description);
    snprintf(syscommand, sizeof(syscommand) - 1, "sysname %s\n", mib_sys_name);
    strcat(File_content,syscommand);
    strcat(File_content,"\n\n");

    strcat(File_content,"master  agentx \n");
    strcat(File_content,"\n\n");
        
#if 0        
    memset(syscommand, 0, sizeof(syscommand));
    STSNMPTrapReceiver *receiver_node = NULL;
    for(i = 0, receiver_node = pstSummary.receiverHead; 
        i < pstSummary.receiver_num, NULL != receiver_node; 
        i++, receiver_node = receiver_node->next) {

        if(RULE_ENABLE == receiver_node->status) {
        
            switch(receiver_node->version) {
			#if 0
                case V1V2V3: 
                    {   
                        sprintf(syscommand , "trapsink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                        memset(syscommand, 0, 500);
                        sprintf(syscommand , "trap2sink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                        memset(syscommand, 0, 500);
                        sprintf(syscommand , "informsink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                    
                    }break;
                case V1V2  :
                    {
                        sprintf(syscommand , "trapsink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                        memset(syscommand, 0, 500);
                        sprintf(syscommand , "trap2sink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                    }
                    break;
                case V1V3 :
                    {
                        sprintf(syscommand , "trapsink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                        memset(syscommand, 0, 500);
                        sprintf(syscommand , "informsink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                    }
                case V2V3 :
                    {
                        sprintf(syscommand , "trap2sink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                        memset(syscommand, 0, 500);
                        sprintf(syscommand , "informsink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                        strcat(File_content , syscommand);
                    }
			#endif
                case V1 : 
                    snprintf(syscommand, sizeof(syscommand) - 1,"#trapsink %s %s %s \n", receiver_node->dest_ipAddr, receiver_node->dest_port, receiver_node->trapcom);
                    strcat(File_content, syscommand);
                    break;
                case V2 :
                    snprintf(syscommand, sizeof(syscommand) - 1, "#trap2sink %s %s %s \n", receiver_node->dest_ipAddr, receiver_node->dest_port, receiver_node->trapcom);
                    strcat(File_content, syscommand);
                    break;
                case V3 :
                    snprintf(syscommand, sizeof(syscommand) - 1, "#informsink %s %s \n", receiver_node->dest_port, receiver_node->dest_port);
                    strcat(File_content, syscommand);
                    break;
            }
		#if 0	     		
            if( pstSummary.receiver[i].version  == V1V2)
            {
                sprintf(syscommand , "trap2sink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                strcat(File_content , syscommand);
            }
            if( pstSummary.receiver[i].version == RULE_ENABLE)
            {
                sprintf(syscommand , "informsink %s %s %s \n",pstSummary.receiver[i].ip_addr, pstSummary.receiver[i].portno, pstSummary.receiver[i].trapcom);
                strcat(File_content , syscommand);
            }
		#endif
            strcat(File_content,"\n\n");
        }
    }
#endif

    strcat(File_content,"\n\n");
    strcat(File_content,"agentSecName internal \n");
    strcat(File_content,"rouser internal \n");
    strcat(File_content,"dlmod subagent_plugin /opt/lib/subagent_plugin.so \n");
    strcat(File_content,"smuxpeer .1.3.6.1.6.3.1 mypasswordIsGreat \n");
    
#if 0/*del some trap which commer mib send*/
    strcat(File_content,"linkUpDownNotifications yes \n");
    strcat(File_content,"defaultMonitors yes \n");
#endif

    strcat(File_content,"\n\n");

    fwrite(File_content,strlen(File_content),1,fp);
    
    fclose(fp);
    return AC_MANAGE_SUCCESS;
}


static void
init_snmp_sysinfo(STSNMPSysInfo *sysinfo) {
    if(NULL == sysinfo) {
        return;
    }
    
    char *sys_oid = get_sys_oid();
    if(NULL == sys_oid) {
        syslog(LOG_INFO, "update_snmp_sysinfo: get sys oid faild, use default sys oid!\n");
        strncpy(sysinfo->sys_oid, ".1.3.6.1.4.1.31656.2.1.8.610.0", sizeof(sysinfo->sys_oid) - 1);
    }
    else {
        strncpy(sysinfo->sys_oid, sys_oid, sizeof(sysinfo->sys_oid) - 1);
        MANAGE_FREE(sys_oid);
    }

    char *sys_description = get_sys_description();
    if(NULL == sys_description) {        
        syslog(LOG_INFO, "update_snmp_sysinfo: get sys description faild, use default sys description!\n");
        strncpy(sysinfo->sys_description, "SYSTEM", sizeof(sysinfo->sys_description) - 1);
    }
    else {
        strncpy(sysinfo->sys_description, sys_description, sizeof(sysinfo->sys_description) - 1);
        MANAGE_FREE(sys_description);
    }

    char *sys_product_name = get_product_name();
    if(NULL == sys_product_name) {
        syslog(LOG_INFO, "update_snmp_sysinfo: get product name faild, use default product name!\n");
        strncpy(sysinfo->sys_name, "PRODUCT", sizeof(sysinfo->sys_name) - 1);
    }
    else {
        strncpy(sysinfo->sys_name, sys_product_name, sizeof(sysinfo->sys_name) - 1);
        MANAGE_FREE(sys_product_name);
    }
    
}



void
init_snmp_config(void) {

    memset(&snmpSummary, 0, sizeof(STSNMPSummary));
    
    snmpSummary.snmp_sysinfo.trap_port = 162;
    snmpSummary.snmp_sysinfo.cache_time = 300;  

    init_snmp_sysinfo(&snmpSummary.snmp_sysinfo);

    snmpSummary.snmp_sysinfo.v1_status = RULE_ENABLE;
    snmpSummary.snmp_sysinfo.v2c_status = RULE_ENABLE;
    snmpSummary.snmp_sysinfo.v3_status = RULE_ENABLE;
    
    write_snmp_config(snmpSummary, CONF_FILE_PATH);
    
    return ;
}

int 
snmp_show_service_state(void){   
    
    FILE *fp = NULL;	
    char buf[100] = { 0 };	
    
    fp = fopen(STATUS_FILE_PATH, "r");	
    if(NULL == fp) {        
        return 0;	
    }
    
    if(NULL == fgets(buf, 100, fp)){		
        fclose(fp);		
        return 0;	
    }
    
    fclose(fp);		
    return strncmp(buf, "start", strlen("start")) == 0;
}

int
snmp_show_sysinfo(STSNMPSysInfo **snmp_info) {
    if(NULL == snmp_info) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *snmp_info = NULL;
    
    STSNMPSysInfo *temp_info = (STSNMPSysInfo *)malloc(sizeof(STSNMPSysInfo));
    if(NULL == temp_info) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    memset(temp_info, 0, sizeof(STSNMPSysInfo));
    memcpy(temp_info, &snmpSummary.snmp_sysinfo, sizeof(STSNMPSysInfo));

    *snmp_info = temp_info;
    
    return AC_MANAGE_SUCCESS;
}

void
update_snmp_sysinfo(void) {

    init_snmp_sysinfo(&snmpSummary.snmp_sysinfo);
    
    write_snmp_config(snmpSummary, CONF_FILE_PATH);

    return ;
}

int 
snmp_show_community(STCommunity **community_array, unsigned int *community_num) {
    if(NULL == community_array || NULL == community_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *community_array = NULL;
    *community_num = 0;

    if(0 == snmpSummary.community_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    STCommunity *temp_community = (STCommunity *)calloc(snmpSummary.community_num, sizeof(STCommunity));
    if(NULL == temp_community){
        return AC_MANAGE_MALLOC_ERROR;
    }

    STCommunity *community_node = NULL;
    int i = 0;
    for(i = 0, community_node = snmpSummary.communityHead; 
        i < snmpSummary.community_num && NULL != community_node; 
        i++, community_node = community_node->next) {
        
        memcpy(&temp_community[i], community_node, sizeof(STCommunity));
    }

    *community_array = temp_community;
    *community_num = snmpSummary.community_num;
    
    return AC_MANAGE_SUCCESS;
}

int 
snmp_show_community_ipv6(IPV6STCommunity **community_array, unsigned int *community_num) {
    if(NULL == community_array || NULL == community_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *community_array = NULL;
    *community_num = 0;

    if(0 == snmpSummary.ipv6_community_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    IPV6STCommunity *temp_community = (IPV6STCommunity *)calloc(snmpSummary.ipv6_community_num, sizeof(IPV6STCommunity));
    if(NULL == temp_community){
        return AC_MANAGE_MALLOC_ERROR;
    }

    IPV6STCommunity *community_node = NULL;
    int i = 0;
    for(i = 0, community_node = snmpSummary.ipv6_communityHead; 
        i < snmpSummary.ipv6_community_num && NULL != community_node; 
        i++, community_node = community_node->next) {
        
        memcpy(&temp_community[i], community_node, sizeof(IPV6STCommunity));
    }

    *community_array = temp_community;
    *community_num = snmpSummary.ipv6_community_num;
    
    return AC_MANAGE_SUCCESS;
}


static void
copy_view_oid(STSNMPView *dest_view, STSNMPView *src_view) {
    if(NULL == dest_view || NULL == src_view) {
        return ;
    }
    
    int j = 0;
    struct oid_list *oid_node = NULL;
    for(j = 0, oid_node = src_view->view_included.oidHead;
        j < src_view->view_included.oid_num && NULL != oid_node;
        j++, oid_node = oid_node->next) {   
        
        struct oid_list *temp_oid = (struct oid_list *)malloc(sizeof(struct oid_list));
        if(NULL == temp_oid) {
            syslog(LOG_DEBUG, "copy_view_oid: %s include oid malloc error!\n", src_view->name);
            continue;
        }
    
        memset(temp_oid, 0, sizeof(struct oid_list));
        memcpy(temp_oid->oid, oid_node->oid, sizeof(temp_oid->oid));
    
        temp_oid->next = dest_view->view_included.oidHead;
        dest_view->view_included.oidHead = temp_oid;
    }
    
    for(j = 0, oid_node = src_view->view_excluded.oidHead;
        j < src_view->view_excluded.oid_num && NULL != oid_node;
        j++, oid_node = oid_node->next) {   
        
        struct oid_list *temp_oid = (struct oid_list *)malloc(sizeof(struct oid_list));
        if(NULL == temp_oid) {
            syslog(LOG_DEBUG, "copy_view_oid: %s exclude oid malloc error!\n", src_view->name);
            continue;
        }
    
        memset(temp_oid, 0, sizeof(struct oid_list));
        memcpy(temp_oid->oid, oid_node->oid, sizeof(temp_oid->oid));
    
        temp_oid->next = dest_view->view_excluded.oidHead;
        dest_view->view_excluded.oidHead = temp_oid;
    }

    memcpy(dest_view->name, src_view->name, sizeof(dest_view->name));
    dest_view->view_included.oid_num = src_view->view_included.oid_num;
    dest_view->view_excluded.oid_num = src_view->view_excluded.oid_num;

    return ;
}

void 
free_snmp_show_view(STSNMPView **view_array, unsigned int view_num){
    if(NULL == view_array || NULL == *view_array)
        return ;

    STSNMPView *temp_view = *view_array;
    int i = 0;
    for(i = 0; i < view_num; i++) {
        free_snmp_view_oid_list(&(temp_view[i].view_included.oidHead));
        free_snmp_view_oid_list(&(temp_view[i].view_excluded.oidHead));
    }

    free(temp_view);
    *view_array = NULL;
    
    return ;
}

int 
snmp_show_view(STSNMPView **view_array, unsigned int *view_num, char *view_name) {
    if(NULL == view_array || NULL == view_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *view_array = NULL;
    *view_num = 0;

    if(0 == snmpSummary.view_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }

    int i = 0;
    if(view_name) {
        STSNMPView *view_node = NULL;
        for(i = 0, view_node = snmpSummary.viewHead;
            i < snmpSummary.view_num && NULL != view_node;
            i++, view_node = view_node->next) {
            if(0 == strcmp(view_node->name, view_name)) {
                STSNMPView *temp_view = (STSNMPView *)calloc(1, sizeof(STSNMPView));
                if(NULL == temp_view){
                    return AC_MANAGE_MALLOC_ERROR;
                }
                
                copy_view_oid(temp_view, view_node);    /*copy view oid config */

                *view_array = temp_view;
                *view_num = 1;
                return AC_MANAGE_SUCCESS;
            }
        }  
        return AC_MANAGE_CONFIG_NONEXIST;
    }

    STSNMPView *temp_view = (STSNMPView *)calloc(snmpSummary.view_num, sizeof(STSNMPView));
    if(NULL == temp_view){
        return AC_MANAGE_MALLOC_ERROR;
    }

    STSNMPView *view_node = NULL;
    for(i = 0, view_node = snmpSummary.viewHead; 
        i < snmpSummary.view_num && NULL != view_node; 
        i++, view_node = view_node->next) {

        copy_view_oid(&temp_view[i], view_node);     /*copy view oid config */
    }

    *view_array = temp_view;
    *view_num = snmpSummary.view_num;
    syslog(LOG_DEBUG, "snmp_show_view:*view_num = %d\n", *view_num);
    return AC_MANAGE_SUCCESS;
}


int
snmp_config_service(unsigned int service_state) {

	int ret =  AC_MANAGE_SUCCESS;

	FILE *fp = NULL;
	fp = fopen(STATUS_FILE_PATH ,"w+");
	if(NULL == fp) {
		syslog(LOG_WARNING, "Open file %s failed!\n", STATUS_FILE_PATH);
		ret = AC_MANAGE_CONFIG_FAIL;
	}
	else {
	
		char cmd[1024] = { 0 };        
		char snmp_status[10] = { 0 };

		if(service_state){            
			strncpy(cmd, "sudo /opt/services/init/snmpd_init restart > /dev/null \n", sizeof(cmd) - 1);
			strncpy(snmp_status, "start", sizeof(snmp_status) - 1);
		} else{
			strncpy(cmd, "sudo /opt/services/init/snmpd_init stop > /dev/null \n", sizeof(cmd) - 1);
			strncpy(snmp_status, "stop", sizeof(snmp_status) - 1);
		}

		int status = system(cmd);	 
		int snmp_ret = WEXITSTATUS(status);

		fputs(snmp_status, fp);

		if (0 != snmp_ret) {
			syslog(LOG_WARNING, "snmp service %s failed.....\n", snmp_status);
			ret = AC_MANAGE_CONFIG_FAIL;
		}

		fclose(fp);
	}
	
	char command[100] = { 0 };
	snprintf(command, sizeof(command) - 1, "sudo chmod a+rw %s >/dev/null 2>&1", STATUS_FILE_PATH);
	system(command);

	return ret;
}


int 
snmp_set_sysinfo(STSNMPSysInfo *snmp_info) {
    if(NULL == snmp_info) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    memcpy(&snmpSummary.snmp_sysinfo, snmp_info, sizeof(STSNMPSysInfo));

    write_snmp_config(snmpSummary, CONF_FILE_PATH);
    
    return AC_MANAGE_SUCCESS;
}

static int
snmp_create_pfm_request(struct pfmOptParameter *pfmParameter, 
                                    char *ifName, 
                                    unsigned int dest_port,
                                    unsigned int state) {
    syslog(LOG_DEBUG, "enter snmp_create_pfm_request\n");
    
    if(NULL == pfmParameter || NULL == ifName) {
        syslog(LOG_WARNING, "snmp_create_pfm_request: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "snmp_create_pfm_request: get local slot id error\n");
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }
    
    memset(pfmParameter, 0, sizeof(*pfmParameter));
    if(state) {
        pfmParameter->pfm_opt = 0;
    }
    else {
        pfmParameter->pfm_opt = 1;
    }
    
    pfmParameter->pfm_opt_para = 0;
    pfmParameter->pfm_protocol = 17;
    
    pfmParameter->ifName = ifName;    
    
    pfmParameter->src_ipaddr = "all";
    pfmParameter->src_port = 0;

    pfmParameter->dest_ipaddr = "all";
    pfmParameter->dest_port = dest_port;
    
    pfmParameter->slot_id = local_slotID;
        
    syslog(LOG_DEBUG, "exit snmp_create_pfm_request\n");

    return AC_MANAGE_SUCCESS;
}


static int
snmp_config_pfm_table_entry(char *ifName, unsigned int dest_port, unsigned int state) {
    syslog(LOG_DEBUG, "enter snmp_config_pfm_table_entry\n");
    
    if(NULL == ifName) {
        syslog(LOG_WARNING, "snmp_config_pfm_table_entry: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    struct pfmOptParameter pfmParameter = { 0 };
    if(AC_MANAGE_SUCCESS != snmp_create_pfm_request(&pfmParameter, 
                                                      ifName, 
                                                      dest_port, 
                                                      state)) {
        syslog(LOG_WARNING, "snmp_config_pfm_table_entry: create snmp pfm request error\n");
        return AC_MANAGE_CONFIG_FAIL;
    }

    int ret = AC_MANAGE_SUCCESS;
    ret = manage_config_pfm_table_entry(&pfmParameter);
    syslog(LOG_DEBUG, "snmp_config_pfm_table_entry: call manage_config_pfm_table_entry , return %d\n", ret);

    syslog(LOG_DEBUG, "exit snmp_config_pfm_table_entry\n");
    return ret;
}

int
snmp_add_pfm_interface(char *ifName, unsigned int port) {
    syslog(LOG_DEBUG, "enter snmp_add_pfm_interface\n");
    
    if(NULL == ifName) {
        syslog(LOG_WARNING, "snmp_add_pfm_interface: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    syslog(LOG_DEBUG, "snmp_add_pfm_interface: ifName = %s\n", ifName);
    int i = 0;
    struct snmp_interface *interfaceNode = NULL;
    for(i = 0, interfaceNode = snmpSummary.interfaceHead; 
        i < snmpSummary.interface_num && NULL != interfaceNode; 
        i++, interfaceNode = interfaceNode->next) {

        if(0 == strcmp(ifName, interfaceNode->ifName)) {
            return AC_MANAGE_CONFIG_EXIST;
        }
    }

    if(snmpSummary.interface_num >= MAX_SNMP_PFM_INTERFACE_NUM) {
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }
    
    struct snmp_interface *tempNode = (struct snmp_interface *)malloc(sizeof(struct snmp_interface));
    if(NULL == tempNode){
        syslog(LOG_WARNING, "snmp_add_pfm_interface:malloc tempNode error\n!");
        return AC_MANAGE_MALLOC_ERROR;
    }

    int ret = snmp_config_pfm_table_entry(ifName, port, 1);
    if(AC_MANAGE_SUCCESS != ret) {
        MANAGE_FREE(tempNode);
        return ret;
    }
    
    memset(tempNode, 0, sizeof(struct snmp_interface));
    strncpy(tempNode->ifName, ifName, sizeof(tempNode->ifName) - 1);

    tempNode->next = snmpSummary.interfaceHead;
    snmpSummary.interfaceHead = tempNode;
    snmpSummary.interface_num++;
    
    syslog(LOG_DEBUG, "exit snmp_add_pfm_interface\n");
    return AC_MANAGE_SUCCESS;
}

int
snmp_del_pfm_interface(char *ifName, unsigned int port) {
    syslog(LOG_DEBUG, "enter snmp_del_pfm_interface\n");
    
    if(NULL == ifName) {
        syslog(LOG_WARNING, "snmp_del_pfm_interface: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    struct snmp_interface *interfaceNode = NULL, *prior = NULL;
    for(i = 0, interfaceNode = snmpSummary.interfaceHead, prior = interfaceNode; 
        i < snmpSummary.interface_num && NULL != interfaceNode; 
        i++, prior = interfaceNode, interfaceNode = interfaceNode->next) {

        if(0 == strcmp(ifName, interfaceNode->ifName)) {
                    
            int ret = snmp_config_pfm_table_entry(ifName, port, 0);
            if(AC_MANAGE_SUCCESS != ret) {
                return ret;
            }
            
            if(prior == interfaceNode) {
                snmpSummary.interfaceHead = interfaceNode->next;
            }
            else {
                prior->next = interfaceNode->next;
            }
            MANAGE_FREE(interfaceNode);
            snmpSummary.interface_num--;
            
            if(0 == snmpSummary.interface_num) {
                snmpSummary.snmp_sysinfo.agent_port = 0;
				write_snmp_config(snmpSummary, CONF_FILE_PATH); 
			}
                
            syslog(LOG_DEBUG, "exit snmp_del_pfm_interface\n");
            return  AC_MANAGE_SUCCESS;
        }
    }

    syslog(LOG_DEBUG, "snmp_del_pfm_interface: can`t find interface %s port %d\n", ifName, port);
    return AC_MANAGE_CONFIG_NONEXIST;
}

int
snmp_add_pfm_interface_ipv6(char *ifName, unsigned int port) {
    syslog(LOG_DEBUG, "enter snmp_add_pfm_interface\n");
    
    if(NULL == ifName) {
        syslog(LOG_WARNING, "snmp_add_pfm_interface: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    syslog(LOG_DEBUG, "snmp_add_pfm_interface: ifName = %s\n", ifName);
    int i = 0;
    struct snmp_interface *interfaceNode = NULL;
    for(i = 0, interfaceNode = snmpSummary.ipv6_interfaceHead; 
        i < snmpSummary.ipv6_interface_num && NULL != interfaceNode; 
        i++, interfaceNode = interfaceNode->next) {

        if(0 == strcmp(ifName, interfaceNode->ifName)) {
            return AC_MANAGE_CONFIG_EXIST;
        }
    }

    if(snmpSummary.ipv6_interface_num >= MAX_SNMP_PFM_INTERFACE_NUM) {
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }
    
    struct snmp_interface *tempNode = (struct snmp_interface *)malloc(sizeof(struct snmp_interface));
    if(NULL == tempNode){
        syslog(LOG_WARNING, "snmp_add_pfm_interface:malloc tempNode error\n!");
        return AC_MANAGE_MALLOC_ERROR;
    }

    int ret = snmp_config_pfm_table_entry(ifName, port, 1);
    if(AC_MANAGE_SUCCESS != ret) {
        MANAGE_FREE(tempNode);
        return ret;
    }
    
    memset(tempNode, 0, sizeof(struct snmp_interface));
    strncpy(tempNode->ifName, ifName, sizeof(tempNode->ifName) - 1);

    tempNode->next = snmpSummary.ipv6_interfaceHead;
    snmpSummary.ipv6_interfaceHead = tempNode;
    snmpSummary.ipv6_interface_num++;
    
    syslog(LOG_DEBUG, "exit snmp_add_pfm_interface\n");
    return AC_MANAGE_SUCCESS;
}


int
snmp_del_pfm_interface_ipv6(char *ifName, unsigned int port) {
    syslog(LOG_DEBUG, "enter snmp_del_pfm_interface\n");
    
    if(NULL == ifName) {
        syslog(LOG_WARNING, "snmp_del_pfm_interface: input para error\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    struct snmp_interface *interfaceNode = NULL, *prior = NULL;
    for(i = 0, interfaceNode = snmpSummary.ipv6_interfaceHead, prior = interfaceNode; 
        i < snmpSummary.ipv6_interface_num && NULL != interfaceNode; 
        i++, prior = interfaceNode, interfaceNode = interfaceNode->next) {

        if(0 == strcmp(ifName, interfaceNode->ifName)) {
                    
            int ret = snmp_config_pfm_table_entry(ifName, port, 0);
            if(AC_MANAGE_SUCCESS != ret) {
                return ret;
            }
            
            if(prior == interfaceNode) {
                snmpSummary.ipv6_interfaceHead = interfaceNode->next;
            }
            else {
                prior->next = interfaceNode->next;
            }
            MANAGE_FREE(interfaceNode);
            snmpSummary.ipv6_interface_num--;
            
            if(0 == snmpSummary.ipv6_interface_num) {
                snmpSummary.snmp_sysinfo.agent_port_ipv6 = 0;
				write_snmp_config(snmpSummary, CONF_FILE_PATH); 
			}
                
            syslog(LOG_DEBUG, "exit snmp_del_pfm_interface\n");
            return  AC_MANAGE_SUCCESS;
        }
    }

    syslog(LOG_DEBUG, "snmp_del_pfm_interface: can`t find interface %s port %d\n", ifName, port);
    return AC_MANAGE_CONFIG_NONEXIST;
}


void
snmp_clear_pfm_interface(void) {
    syslog(LOG_DEBUG, "enter snmp_clear_pfm_interface\n");
    
    while(snmpSummary.interfaceHead){
        struct snmp_interface *temp = snmpSummary.interfaceHead->next;
        snmp_config_pfm_table_entry(snmpSummary.interfaceHead->ifName, snmpSummary.snmp_sysinfo.agent_port, 0);
        MANAGE_FREE(snmpSummary.interfaceHead);
        snmpSummary.interfaceHead = temp;
    }

    snmpSummary.interface_num = 0;
    snmpSummary.snmp_sysinfo.agent_port = 0;


    
    while(snmpSummary.ipv6_interfaceHead){
        struct snmp_interface *temp = snmpSummary.ipv6_interfaceHead->next;
        snmp_config_pfm_table_entry(snmpSummary.ipv6_interfaceHead->ifName, snmpSummary.snmp_sysinfo.agent_port_ipv6, 0);
        MANAGE_FREE(snmpSummary.ipv6_interfaceHead);
        snmpSummary.ipv6_interfaceHead = temp;
    }

    snmpSummary.ipv6_interface_num = 0;
    snmpSummary.snmp_sysinfo.agent_port_ipv6 = 0;
    syslog(LOG_DEBUG, "exit snmp_clear_pfm_interface\n");
    return ;
}

int 
snmp_show_pfm_interface(SNMPINTERFACE **interface_array, unsigned int *interface_num) {
    if(NULL == interface_array || NULL == interface_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    *interface_array = NULL;
    *interface_num = 0;

    if(0 == snmpSummary.interface_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    SNMPINTERFACE *temp_interface = (SNMPINTERFACE *)calloc(snmpSummary.interface_num, sizeof(SNMPINTERFACE));
    if(NULL == temp_interface){
        return AC_MANAGE_MALLOC_ERROR;
    }
    
    int i = 0;
    SNMPINTERFACE *interface_node = NULL;
    for(i = 0, interface_node = snmpSummary.interfaceHead; 
        i < snmpSummary.interface_num && NULL != interface_node; 
        i++, interface_node = interface_node->next) {
        
        memcpy(&temp_interface[i].ifName, interface_node->ifName, sizeof(interface_node->ifName));
    }

    *interface_array = temp_interface;
    *interface_num = snmpSummary.interface_num;
    
    return AC_MANAGE_SUCCESS;
}

int 
snmp_show_pfm_interface_ipv6(SNMPINTERFACE **interface_array, unsigned int *interface_num) {
    if(NULL == interface_array || NULL == interface_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    *interface_array = NULL;
    *interface_num = 0;

    if(0 == snmpSummary.ipv6_interface_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    SNMPINTERFACE *temp_interface = (SNMPINTERFACE *)calloc(snmpSummary.ipv6_interface_num, sizeof(SNMPINTERFACE));
    if(NULL == temp_interface){
        return AC_MANAGE_MALLOC_ERROR;
    }
    
    int i = 0;
    SNMPINTERFACE *interface_node = NULL;
    for(i = 0, interface_node = snmpSummary.ipv6_interfaceHead; 
        i < snmpSummary.ipv6_interface_num && NULL != interface_node; 
        i++, interface_node = interface_node->next) {
	        memcpy(&temp_interface[i].ifName, interface_node->ifName, sizeof(interface_node->ifName));
    }

    *interface_array = temp_interface;
    *interface_num = snmpSummary.ipv6_interface_num;
    
    return AC_MANAGE_SUCCESS;
}


int 
snmp_add_community(STCommunity pstCommunity) {

    int i = 0;    
    STCommunity *temp = NULL;
    for(i = 0, temp = snmpSummary.communityHead; 
        i < snmpSummary.community_num && NULL != temp; i++, 
        temp = temp->next) {

        if(0 == strcmp(temp->community, pstCommunity.community)) {
            return AC_MANAGE_CONFIG_EXIST;
        }
    }

    if(snmpSummary.community_num >= MAX_COMMUNITY_NUM) {
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }

    STCommunity *temp_node = (STCommunity *)malloc(sizeof(STCommunity));
    if(NULL == temp_node) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    memcpy(temp_node, &pstCommunity, sizeof(STCommunity));

    temp_node->next = snmpSummary.communityHead;
    snmpSummary.communityHead = temp_node;
    snmpSummary.community_num++;

    write_snmp_config(snmpSummary, CONF_FILE_PATH); 
    
    return AC_MANAGE_SUCCESS;
}
int 
snmp_add_community_ipv6(IPV6STCommunity pstCommunity) {

    int i = 0;    
    IPV6STCommunity *temp = NULL;
    for(i = 0, temp = snmpSummary.ipv6_communityHead; 
        i < snmpSummary.ipv6_community_num && NULL != temp; i++, 
        temp = temp->next) {

        if(0 == strcmp(temp->community, pstCommunity.community)) {
            return AC_MANAGE_CONFIG_EXIST;
        }
    }

    if(snmpSummary.ipv6_community_num >= MAX_COMMUNITY_NUM) {
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }

    IPV6STCommunity *temp_node = (IPV6STCommunity *)malloc(sizeof(IPV6STCommunity));
    if(NULL == temp_node) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    memcpy(temp_node, &pstCommunity, sizeof(IPV6STCommunity));

    temp_node->next = snmpSummary.ipv6_communityHead;
    snmpSummary.ipv6_communityHead = temp_node;
    snmpSummary.ipv6_community_num++;

    write_snmp_config(snmpSummary, CONF_FILE_PATH); 
    
    return AC_MANAGE_SUCCESS;
}


int 
snmp_del_community(char *community) {
    if(NULL == community) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;    
    STCommunity *temp = NULL, *prior = NULL ;
    for(i = 0, temp = snmpSummary.communityHead, prior = temp; 
        i < snmpSummary.community_num && NULL != temp; 
        i++, prior = temp, temp = temp->next) {
        
        if(0 == strcmp(temp->community, community)) {
        
            if(prior == temp) {
                snmpSummary.communityHead = temp->next;
            }
            else {
                prior->next = temp->next;
            }
            MANAGE_FREE(temp);
            snmpSummary.community_num--;
            
            write_snmp_config(snmpSummary, CONF_FILE_PATH);
            
            return  AC_MANAGE_SUCCESS;
        }
    }

    return AC_MANAGE_CONFIG_NONEXIST;
}

int 
snmp_del_community_ipv6(char *community) {
    if(NULL == community) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;    
    IPV6STCommunity *temp = NULL, *prior = NULL ;
    for(i = 0, temp = snmpSummary.ipv6_communityHead, prior = temp; 
        i < snmpSummary.ipv6_community_num && NULL != temp; 
        i++, prior = temp, temp = temp->next) {
        
        if(0 == strcmp(temp->community, community)) {
        
            if(prior == temp) {
                snmpSummary.ipv6_communityHead = temp->next;
            }
            else {
                prior->next = temp->next;
            }
            MANAGE_FREE(temp);
            snmpSummary.ipv6_community_num--;
            
            write_snmp_config(snmpSummary, CONF_FILE_PATH);
            
            return  AC_MANAGE_SUCCESS;
        }
    }

    return AC_MANAGE_CONFIG_NONEXIST;
}


int 
snmp_set_community(char *old_community, STCommunity pstCommunity) {
    if(NULL == old_community) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;    
    STCommunity *temp = NULL;
    for(i = 0, temp = snmpSummary.communityHead; 
        i < snmpSummary.community_num && NULL != temp; i++, 
        temp = temp->next) {

        if(0 == strcmp(temp->community, old_community)) {

            pstCommunity.next = temp->next;
            memcpy(temp, &pstCommunity, sizeof(STCommunity));
            
            write_snmp_config(snmpSummary, CONF_FILE_PATH); 

            return AC_MANAGE_SUCCESS;
        }
    }

    return AC_MANAGE_CONFIG_NONEXIST;
}
int 
snmp_set_community_ipv6(char *old_community, IPV6STCommunity pstCommunity) {
    if(NULL == old_community) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;    
    IPV6STCommunity *temp = NULL;
    for(i = 0, temp = snmpSummary.ipv6_communityHead; 
        i < snmpSummary.ipv6_community_num && NULL != temp; i++, 
        temp = temp->next) {

        if(0 == strcmp(temp->community, old_community)) {

            pstCommunity.next = temp->next;
            memcpy(temp, &pstCommunity, sizeof(IPV6STCommunity));
            
            write_snmp_config(snmpSummary, CONF_FILE_PATH); 

            return AC_MANAGE_SUCCESS;
        }
    }

    return AC_MANAGE_CONFIG_NONEXIST;
}


void
snmp_clear_community(void) {
    while(snmpSummary.communityHead) {
        STCommunity *temp = snmpSummary.communityHead->next;
        MANAGE_FREE(snmpSummary.communityHead);
        snmpSummary.communityHead = temp;
    }
    snmpSummary.community_num = 0;

    while(snmpSummary.ipv6_communityHead) {
        IPV6STCommunity *temp_ipv6 = snmpSummary.ipv6_communityHead->next;
        MANAGE_FREE(snmpSummary.ipv6_communityHead);
        snmpSummary.ipv6_communityHead = temp_ipv6;
    }
    snmpSummary.ipv6_community_num = 0;
    
    return ;
}

int
snmp_create_view(char *view_name) {
    if(NULL == view_name) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    STSNMPView *view_node = NULL;
    for(i = 0, view_node = snmpSummary.viewHead;
        i < snmpSummary.view_num && NULL != view_node;
        i++, view_node = view_node->next) {

        if(0 == strcmp(view_node->name, view_name)) {
            return AC_MANAGE_CONFIG_EXIST;
        }
    }    

    if(snmpSummary.view_num >= MAX_VIEW_NUM) {
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }

    STSNMPView *temp_node = (STSNMPView *)malloc(sizeof(STSNMPView));
    if(NULL == temp_node) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    memset(temp_node, 0, sizeof(STSNMPView));
    strncpy(temp_node->name, view_name, sizeof(temp_node->name) - 1);
    
    temp_node->next = snmpSummary.viewHead;
    snmpSummary.viewHead = temp_node;
    snmpSummary.view_num++;

    return AC_MANAGE_SUCCESS;    
}

int
snmp_check_view(char *view_name) {
    if(NULL == view_name) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    STSNMPView *view_node = NULL;
    for(i = 0, view_node = snmpSummary.viewHead;
        i < snmpSummary.view_num && NULL != view_node;
        i++, view_node = view_node->next) {

        if(0 == strcmp(view_node->name, view_name)) {
            return AC_MANAGE_SUCCESS;
        }
    }    

    return AC_MANAGE_CONFIG_NONEXIST;    
}


int 
snmp_add_view_oid(char *view_name, char *oid, unsigned int mode) {

    if(NULL == view_name || NULL == oid) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    STSNMPView *view_node = NULL;
    for(i = 0, view_node = snmpSummary.viewHead;
        i < snmpSummary.view_num && NULL != view_node;
        i++, view_node = view_node->next) {

        if(0 == strcmp(view_node->name, view_name)) {
            if(mode) {  /* include */
                int j = 0;
                struct oid_list *oid_node = NULL;
                for(j = 0, oid_node = view_node->view_included.oidHead;
                    j < view_node->view_included.oid_num && NULL != oid_node;
                    j++, oid_node = oid_node->next) {

                    if(0 == strcmp(oid_node->oid, oid)) {
                        return AC_MANAGE_CONFIG_EXIST;
                    }
                }

                if(view_node->view_included.oid_num >= MAX_VIEW_INEX_NUM) {
                    return AC_MANAGE_CONFIG_REACH_MAX_NUM;
                }

                struct oid_list *temp_node = (struct oid_list *)malloc(sizeof(struct oid_list));
                if(NULL == temp_node) {
                    return AC_MANAGE_MALLOC_ERROR;
                }
                memset(temp_node, 0, sizeof(struct oid_list));
                strncpy(temp_node->oid, oid, sizeof(temp_node->oid) - 1);

                temp_node->next = view_node->view_included.oidHead;
                view_node->view_included.oidHead = temp_node;
                view_node->view_included.oid_num++;
            }
            else {  /* exclude */
                int j = 0;
                struct oid_list *oid_node = NULL;
                for(j = 0, oid_node = view_node->view_excluded.oidHead;
                    j < view_node->view_excluded.oid_num && NULL != oid_node;
                    j++, oid_node = oid_node->next) {

                    if(0 == strcmp(oid_node->oid, oid)) {
                        return AC_MANAGE_CONFIG_EXIST;
                    }
                }
                
                if(view_node->view_excluded.oid_num >= MAX_VIEW_INEX_NUM) {
                    return AC_MANAGE_CONFIG_REACH_MAX_NUM;
                }
                
                struct oid_list *temp_node = (struct oid_list *)malloc(sizeof(struct oid_list));
                if(NULL == temp_node) {
                    return AC_MANAGE_MALLOC_ERROR;
                }
                memset(temp_node, 0, sizeof(struct oid_list));
                strncpy(temp_node->oid, oid, sizeof(temp_node->oid) - 1);

                temp_node->next = view_node->view_excluded.oidHead;
                view_node->view_excluded.oidHead = temp_node;
                view_node->view_excluded.oid_num++;
            }
            
            write_snmp_config(snmpSummary, CONF_FILE_PATH); 

            return AC_MANAGE_SUCCESS;
        }
    }    

    return AC_MANAGE_CONFIG_NONEXIST;
}



int 
snmp_del_view_oid(char *view_name, char *oid, unsigned int mode) {

    if(NULL == view_name || NULL == oid) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    STSNMPView *view_node = NULL;
    for(i = 0, view_node = snmpSummary.viewHead;
        i < snmpSummary.view_num && NULL != view_node;
        i++, view_node = view_node->next) {

        if(0 == strcmp(view_node->name, view_name)) {
            if(mode) {  /* include */
                syslog(LOG_DEBUG, "enter delete include view oid\n");
                int j = 0;
                struct oid_list *oid_node = NULL, *prior = NULL;
                for(j = 0, oid_node = view_node->view_included.oidHead, prior = oid_node;
                    j < view_node->view_included.oid_num && NULL != oid_node;
                    j++, prior = oid_node, oid_node = oid_node->next) {
                    syslog(LOG_DEBUG, "include view oid:oid_node->oid = %s\n", oid_node->oid);
                    if(0 == strcmp(oid_node->oid, oid)) {
                        syslog(LOG_DEBUG, "include view oid:find oid = %s\n", oid_node->oid);
                        if(prior == oid_node) {
                            view_node->view_included.oidHead = oid_node->next;
                        }
                        else {
                            prior->next = oid_node->next;
                        }
                        MANAGE_FREE(oid_node);
                        view_node->view_included.oid_num--;

                        write_snmp_config(snmpSummary, CONF_FILE_PATH);
                        
                        return  AC_MANAGE_SUCCESS;
                    }
                }
            }
            else {
                syslog(LOG_DEBUG, "enter delete exclude view oid\n");
                int j = 0;
                struct oid_list *oid_node = NULL, *prior = NULL;
                for(j = 0, oid_node = view_node->view_excluded.oidHead, prior = oid_node;
                    j < view_node->view_excluded.oid_num && NULL != oid_node;
                    j++, prior = oid_node, oid_node = oid_node->next) {
                    syslog(LOG_DEBUG, "exclude view oid:oid_node->oid = %s\n", oid_node->oid);
                    if(0 == strcmp(oid_node->oid, oid)) {
                        syslog(LOG_DEBUG, "exclude view oid:find oid = %s\n", oid_node->oid);
                        if(prior == oid_node) {
                            view_node->view_excluded.oidHead = oid_node->next;
                        }
                        else {
                            prior->next = oid_node->next;
                        }
                        MANAGE_FREE(oid_node);
                        view_node->view_excluded.oid_num--;

                        write_snmp_config(snmpSummary, CONF_FILE_PATH);
                        
                        return  AC_MANAGE_SUCCESS;
                    }
                }
            }
            return AC_MANAGE_CONFIG_NONEXIST;
        }
    }    

    return AC_MANAGE_CONFIG_NONEXIST;
}

static int
check_view_relevance(char *view_name) {
    
    int i = 0;
    STSNMPGroup *group_node = NULL;
    for(i = 0, group_node = snmpSummary.groupHead;
        i < snmpSummary.group_num && NULL != group_node;
        i++, group_node = group_node->next) {

        if(0 == strcmp(view_name, group_node->group_view)) {
            return AC_MANAGE_CONFIG_RELEVANCE;
        }                
    }

    return AC_MANAGE_SUCCESS;
}

int 
snmp_del_view(char *view_name) {
    if(NULL == view_name) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    STSNMPView *view_node = NULL, *prior = NULL;
    for(i = 0, view_node = snmpSummary.viewHead, prior = view_node;
        i < snmpSummary.view_num && NULL != view_node;
        i++, prior = view_node, view_node = view_node->next) {

        if(0 == strcmp(view_node->name, view_name)) {
            
            if(AC_MANAGE_SUCCESS != check_view_relevance(view_name))
                return AC_MANAGE_CONFIG_RELEVANCE;
            
            if(prior == view_node) {
                snmpSummary.viewHead = view_node->next;
            }
            else {
                prior->next = view_node->next;
            }
            
            free_snmp_view_oid_list(&(view_node->view_included.oidHead));            
            free_snmp_view_oid_list(&(view_node->view_excluded.oidHead));
            MANAGE_FREE(view_node);
            
            snmpSummary.view_num--;
            
            write_snmp_config(snmpSummary, CONF_FILE_PATH);
            
            return  AC_MANAGE_SUCCESS;
        }
    } 

    return AC_MANAGE_CONFIG_NONEXIST;
}

void
snmp_clear_view(void) {
    while(snmpSummary.viewHead) {
        STSNMPView *temp = snmpSummary.viewHead->next;
        free_snmp_view_oid_list(&(snmpSummary.viewHead->view_included.oidHead));            
        free_snmp_view_oid_list(&(snmpSummary.viewHead->view_excluded.oidHead));
        MANAGE_FREE(snmpSummary.viewHead);
        snmpSummary.viewHead = temp;
    }
    snmpSummary.view_num = 0;
    return ;
}

int
snmp_add_group(char *group_name, char *view_name, unsigned int access_mode, unsigned int sec_level) {
    if(NULL == group_name || NULL == view_name)
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
    if(0 != strcmp(view_name, "all")) {
        if(AC_MANAGE_SUCCESS != snmp_check_view(view_name))
            return AC_MANAGE_CONFIG_NONEXIST;
    }

    int i = 0;
    STSNMPGroup *group_node = NULL;
    for(i = 0, group_node = snmpSummary.groupHead;
        i < snmpSummary.group_num && NULL != group_node;
        i++, group_node = group_node->next) {

        if(0 == strcmp(group_name, group_node->group_name)) {
            return AC_MANAGE_CONFIG_EXIST;
        }                
    }

    
    if(snmpSummary.group_num >= MAX_SNMP_GROUP_NUM) {
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }

    STSNMPGroup *temp_node = (STSNMPGroup *)malloc(sizeof(STSNMPGroup));
    if(NULL == temp_node) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    memset(temp_node, 0, sizeof(STSNMPGroup));
    strncpy(temp_node->group_name, group_name, sizeof(temp_node->group_name) - 1);
    strncpy(temp_node->group_view, view_name, sizeof(temp_node->group_view) - 1);
    temp_node->access_mode = access_mode;
    temp_node->sec_level = sec_level;
    
    temp_node->next = snmpSummary.groupHead;
    snmpSummary.groupHead = temp_node;
    snmpSummary.group_num++;

    write_snmp_config(snmpSummary, CONF_FILE_PATH);

    return AC_MANAGE_SUCCESS;    
}


static int
check_group_relevance(char *group_name) {
    
    int i = 0;
    STSNMPV3User *v3user_node = NULL;
    for(i = 0, v3user_node = snmpSummary.v3userHead;
        i < snmpSummary.v3user_num && NULL != v3user_node;
        i++, v3user_node = v3user_node->next) {

        if(0 == strcmp(group_name, v3user_node->group_name)) {
            return AC_MANAGE_CONFIG_RELEVANCE;
        }
    }  

    return AC_MANAGE_SUCCESS;
}

int
snmp_del_group(char *group_name) {
    if(NULL == group_name)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    int i = 0;
    STSNMPGroup *group_node = NULL, *prior = NULL;
    for(i = 0, group_node = snmpSummary.groupHead, prior = group_node;
        i < snmpSummary.group_num && NULL != group_node;
        i++, prior = group_node, group_node = group_node->next) {

        if(0 == strcmp(group_name, group_node->group_name)) {

            if(AC_MANAGE_SUCCESS != check_group_relevance(group_name))
                return AC_MANAGE_CONFIG_RELEVANCE;
            
            if(prior == group_node) {
                snmpSummary.groupHead = group_node->next;
            }
            else {
                prior->next = group_node->next;
            }
            
            MANAGE_FREE(group_node);
            
            snmpSummary.group_num--;
            
            write_snmp_config(snmpSummary, CONF_FILE_PATH);
            
            return  AC_MANAGE_SUCCESS;
        }                
    }
    
    return  AC_MANAGE_CONFIG_NONEXIST;
}

static void 
snmp_clear_group(void) {
    while(snmpSummary.groupHead) {
        STSNMPGroup *temp = snmpSummary.groupHead->next;
        MANAGE_FREE(snmpSummary.groupHead);
        snmpSummary.groupHead = temp;
    }
    snmpSummary.group_num = 0;
    return ;
}


int 
snmp_show_group(STSNMPGroup **group_array, unsigned int *group_num, char *group_name) {
    if(NULL == group_array || NULL == group_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *group_array = NULL;
    *group_num = 0;

    if(0 == snmpSummary.group_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }

    int i = 0;
    if(group_name) {
        STSNMPGroup *group_node = NULL;
        for(i = 0, group_node = snmpSummary.groupHead;
            i < snmpSummary.group_num && NULL != group_node;
            i++, group_node = group_node->next) {
            if(0 == strcmp(group_node->group_name, group_name)) {
                STSNMPGroup *temp_group = (STSNMPGroup *)calloc(1, sizeof(STSNMPGroup));
                if(NULL == temp_group){
                    return AC_MANAGE_MALLOC_ERROR;
                }

                memcpy(temp_group, group_node, sizeof(STSNMPGroup));
                temp_group->next = NULL;
                
                *group_array = temp_group;
                *group_num = 1;
                return AC_MANAGE_SUCCESS;
            }
        }  
        return AC_MANAGE_CONFIG_NONEXIST;
    }

    STSNMPGroup *temp_group = (STSNMPGroup *)calloc(snmpSummary.group_num, sizeof(STSNMPGroup));
    if(NULL == temp_group){
        return AC_MANAGE_MALLOC_ERROR;
    }

    STSNMPGroup *group_node = NULL;
    for(i = 0, group_node = snmpSummary.groupHead; 
        i < snmpSummary.group_num && NULL != group_node; 
        i++, group_node = group_node->next) {

        memcpy(&temp_group[i], group_node, sizeof(STSNMPGroup));
        temp_group[i].next = NULL;
    }

    *group_array = temp_group;
    *group_num = snmpSummary.group_num;
    syslog(LOG_DEBUG, "snmp_show_group:*group_num = %d\n", *group_num);
    
    return AC_MANAGE_SUCCESS;
}

static int 
check_v3user_dependent(char *group_name) {
    if(NULL == group_name) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    STSNMPGroup *group_node = NULL;
    for(i = 0, group_node = snmpSummary.groupHead;
        i < snmpSummary.group_num && NULL != group_node;
        i++, group_node = group_node->next) {

        if(0 == strcmp(group_node->group_name, group_name)) {
            return AC_MANAGE_SUCCESS;
        }
    }

    syslog(LOG_DEBUG, "check_v3user_dependent: Can`t find group %s\n", group_name);
    return AC_MANAGE_CONFIG_NONEXIST;
}

int
snmp_add_v3user(char *v3user_name, 
                        unsigned int auth_protocal, 
                        char *ath_passwd, 
                        unsigned int priv_protocal,
                        char *priv_passwd,
                        char *group_name,
                        unsigned int state) {

    if(NULL == v3user_name || NULL == ath_passwd || NULL == priv_passwd || NULL == group_name)
        return AC_MANAGE_INPUT_TYPE_ERROR;


    if(AC_MANAGE_SUCCESS != check_v3user_dependent(group_name)) {
        syslog(LOG_INFO, "snmp_add_v3user: check_v3user_dependent group %s failed!\n", group_name);
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    int i = 0;
    STSNMPV3User *v3user_node = NULL;
    for(i = 0, v3user_node = snmpSummary.v3userHead;
        i < snmpSummary.v3user_num && NULL != v3user_node;
        i++, v3user_node = v3user_node->next) {

        if(0 == strcmp(v3user_name, v3user_node->name)) {
            return AC_MANAGE_CONFIG_EXIST;
        }
    }                            

    
    if(snmpSummary.group_num >= MAX_SNMP_GROUP_NUM) {
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }

    STSNMPV3User *temp_node = (STSNMPV3User *)malloc(sizeof(STSNMPV3User));
    if(NULL == temp_node) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    memset(temp_node, 0, sizeof(STSNMPV3User));
    strncpy(temp_node->name, v3user_name, sizeof(temp_node->name) - 1);
    temp_node->authentication.protocal = auth_protocal;
    if(AUTH_PRO_NONE != auth_protocal) {
        strncpy(temp_node->authentication.passwd, ath_passwd, sizeof(temp_node->authentication.passwd) - 1);
        temp_node->privacy.protocal = priv_protocal;
        if(PRIV_PRO_NONE != priv_protocal) {
            strncpy(temp_node->privacy.passwd, priv_passwd, sizeof(temp_node->privacy.passwd) - 1);
        }
    }
    strncpy(temp_node->group_name, group_name, sizeof(temp_node->group_name) - 1);
    temp_node->status = state;
    
    temp_node->next = snmpSummary.v3userHead;
    snmpSummary.v3userHead = temp_node;
    snmpSummary.v3user_num++;

    write_snmp_config(snmpSummary, CONF_FILE_PATH);
        
    return AC_MANAGE_SUCCESS;
}

int 
snmp_del_v3user(char *v3user_name) {
    if(NULL == v3user_name)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    int i = 0;
    STSNMPV3User *v3user_node = NULL, *prior = NULL;
    for(i = 0, v3user_node = snmpSummary.v3userHead, prior = v3user_node;
        i < snmpSummary.v3user_num && NULL != v3user_node;
        i++, prior = v3user_node, v3user_node = v3user_node->next) {

        if(0 == strcmp(v3user_name, v3user_node->name)) {
        
            if(prior == v3user_node) {
                snmpSummary.v3userHead = v3user_node->next;
            }
            else {
                prior->next = v3user_node->next;
            }
            
            MANAGE_FREE(v3user_node);
            snmpSummary.v3user_num--;
            
            write_snmp_config(snmpSummary, CONF_FILE_PATH);
            
            return  AC_MANAGE_SUCCESS;
        }                
    }

    return AC_MANAGE_CONFIG_NONEXIST;
}
static void
snmp_clear_v3user(void) {
    while(snmpSummary.v3userHead) {
        STSNMPV3User *temp = snmpSummary.v3userHead->next;
        MANAGE_FREE(snmpSummary.v3userHead);
        snmpSummary.v3userHead = temp;
    }
    snmpSummary.v3user_num = 0;
    return ;
}


int 
snmp_show_v3user(STSNMPV3User **v3user_array, unsigned int *v3user_num, char *v3user_name) {
    if(NULL == v3user_array || NULL == v3user_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *v3user_array = NULL;
    *v3user_num = 0;

    if(0 == snmpSummary.v3user_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }

    int i = 0;
    if(v3user_name) {
        STSNMPV3User *v3user_node = NULL;
        for(i = 0, v3user_node = snmpSummary.v3userHead;
            i < snmpSummary.v3user_num && NULL != v3user_node;
            i++, v3user_node = v3user_node->next) {
            if(0 == strcmp(v3user_node->name, v3user_name)) {
                STSNMPV3User *temp_v3user = (STSNMPV3User *)calloc(1, sizeof(STSNMPV3User));
                if(NULL == temp_v3user){
                    return AC_MANAGE_MALLOC_ERROR;
                }

                memcpy(temp_v3user, v3user_node, sizeof(STSNMPV3User));
                temp_v3user->next = NULL;
                
                *v3user_array = temp_v3user;
                *v3user_num = 1;
                return AC_MANAGE_SUCCESS;
            }
        }  
        return AC_MANAGE_CONFIG_NONEXIST;
    }

    STSNMPV3User *temp_v3user = (STSNMPV3User *)calloc(snmpSummary.v3user_num, sizeof(STSNMPV3User));
    if(NULL == temp_v3user){
        return AC_MANAGE_MALLOC_ERROR;
    }

    STSNMPV3User *v3user_node = NULL;
    for(i = 0, v3user_node = snmpSummary.v3userHead; 
        i < snmpSummary.v3user_num && NULL != v3user_node; 
        i++, v3user_node = v3user_node->next) {

        memcpy(&temp_v3user[i], v3user_node, sizeof(STSNMPV3User));
        temp_v3user[i].next = NULL;
    }

    *v3user_array = temp_v3user;
    *v3user_num = snmpSummary.v3user_num;
    syslog(LOG_DEBUG, "snmp_show_v3user:*v3user_num = %d\n", *v3user_num);
    
    return AC_MANAGE_SUCCESS;
}

void
uninit_snmp_config(void) {
    
    snmpSummary.snmp_sysinfo.cache_time = 300;  
    init_snmp_sysinfo(&snmpSummary.snmp_sysinfo);
    snmpSummary.snmp_sysinfo.v1_status = RULE_ENABLE;
    snmpSummary.snmp_sysinfo.v2c_status = RULE_ENABLE;
    snmpSummary.snmp_sysinfo.v3_status = RULE_ENABLE;
	snmpSummary.snmp_sysinfo.sysoid_boardtype = 0;

    snmp_clear_pfm_interface();
    
    snmp_clear_v3user();
    snmp_clear_group();
    snmp_clear_view();
    snmp_clear_community();
    
    write_snmp_config(snmpSummary, CONF_FILE_PATH);
    
    return ;
}

void 
init_trap_config(void) {

    memset(&trapSummary, 0, sizeof(STTRAPSummary));

    trapSummary.trapDetail = ALL_TRAP;
    trapSummary.trapDetailNUM = (sizeof(ALL_TRAP) / sizeof(ALL_TRAP[0]));

    trapSummary.trapParameter = Trap_para;
    trapSummary.trapParaNUM = (sizeof(Trap_para) / sizeof(Trap_para[0]));

	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
	}

    return ;
}

int 
trap_show_service_state(void)
{
	int ret = 0;
	char cmd[200] = { 0 };
	memset(cmd, 0, sizeof(cmd));
	
	snprintf(cmd, sizeof(cmd) - 1, "check_process_status.sh %s", "trap-helper");
	ret = system(cmd);
	ret = WEXITSTATUS(ret);

	return 2 == ret;
}

static int 
trap_service_stop(void)
{
	char cmd[200] = { 0 };
	
	if(trap_show_service_state()) {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd) - 1,"sudo kill_process.sh %s", "trap-helper");
		system(cmd);
    }
    
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd) - 1, "sudo echo stop > %s", TRAP_STATUS_FILE_PATH);
	system(cmd);
	
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd) - 1, "sudo chmod a+rw %s >/dev/null 2>&1", TRAP_STATUS_FILE_PATH);
	system(cmd);
	
	return 0;
}

static int 
trap_service_start(void)
{

#define OPERATE_TRAP	"sudo /opt/bin/trap-helper >/dev/null 2>&1 &"

	char cmd[200] = { 0 };

    trap_service_stop();
    system(OPERATE_TRAP);
    
	memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd) - 1, "sudo echo start > %s", TRAP_STATUS_FILE_PATH);
    system(cmd);
    
	memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd) - 1, "sudo chmod a+rw %s >/dev/null 2>&1", TRAP_STATUS_FILE_PATH);
    system(cmd);

    return 0;
}


int
trap_config_service(unsigned int service_state) {
    if(service_state) {
         trap_service_start();         
    }
    else {
        trap_service_stop();
    }
    
    return  AC_MANAGE_SUCCESS;     
}


int
trap_add_receiver(STSNMPTrapReceiver *receiver) {
    if(NULL == receiver) 
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
    int i = 0;    
    STSNMPTrapReceiver *temp = NULL;
    for(i = 0, temp = trapSummary.receiverHead; 
        i < trapSummary.receiver_num && NULL != temp; i++, 
        temp = temp->next) {

        if(0 == strcmp(temp->name, receiver->name)) {
            return AC_MANAGE_CONFIG_EXIST;
        }
    }

    if(trapSummary.receiver_num >= 32) {
        return AC_MANAGE_CONFIG_REACH_MAX_NUM;
    }

    STSNMPTrapReceiver *temp_node = (STSNMPTrapReceiver *)malloc(sizeof(STSNMPTrapReceiver));
    if(NULL == temp_node) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    memcpy(temp_node, receiver, sizeof(STSNMPTrapReceiver));

    temp_node->next = trapSummary.receiverHead;
    trapSummary.receiverHead = temp_node;
    trapSummary.receiver_num++;

    return AC_MANAGE_SUCCESS;
}

int
trap_set_receiver(STSNMPTrapReceiver *receiver) {
    if(NULL == receiver) 
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
    int i = 0;
    STSNMPTrapReceiver *receiverNode = NULL;
    for(i = 0, receiverNode = trapSummary.receiverHead;
        i < trapSummary.receiver_num && NULL != receiverNode;
        i++, receiverNode = receiverNode->next) {

        if(0 == strcmp(receiverNode->name, receiver->name)) {

            STSNMPTrapReceiver *temp = receiverNode->next;
            memcpy(receiverNode, receiver, sizeof(STSNMPTrapReceiver));
            receiverNode->next = temp;
            
            return AC_MANAGE_SUCCESS;
        }
    }    

    return AC_MANAGE_CONFIG_NONEXIST;
}

int
trap_del_receiver(char *receiverName) {
    if(NULL == receiverName) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    int i = 0;
    STSNMPTrapReceiver *receiverNode = NULL, *priorNode = NULL;
    for(i = 0, receiverNode = trapSummary.receiverHead, priorNode = receiverNode;
        i < trapSummary.receiver_num && NULL != receiverNode;
        i++, priorNode = receiverNode, receiverNode = receiverNode->next) {
        
        if(0 == strcmp(receiverNode->name, receiverName)) {
            if(receiverNode == priorNode) {
                trapSummary.receiverHead = receiverNode->next;
            }
            else {
                priorNode->next = receiverNode->next;
            }
            
            MANAGE_FREE(receiverNode);
            trapSummary.receiver_num--;
            
            return AC_MANAGE_SUCCESS;
        }
    }

    return AC_MANAGE_CONFIG_NONEXIST;
}

int
trap_show_receiver(STSNMPTrapReceiver **receiver_array, unsigned int *receiver_num) {
    if(NULL == receiver_array || NULL == receiver_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *receiver_array = NULL;
    *receiver_num = 0;

    if(0 == trapSummary.receiver_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    STSNMPTrapReceiver *temp_receiver = (STSNMPTrapReceiver *)calloc(trapSummary.receiver_num, sizeof(STSNMPTrapReceiver));
    if(NULL == temp_receiver){
        return AC_MANAGE_MALLOC_ERROR;
    }

    STSNMPTrapReceiver *receiver_node = NULL;
    int i = 0;
    for(i = 0, receiver_node = trapSummary.receiverHead; 
        i < trapSummary.receiver_num && NULL != receiver_node; 
        i++, receiver_node = receiver_node->next) {
        
        memcpy(&temp_receiver[i], receiver_node, sizeof(STSNMPTrapReceiver));
    }

    *receiver_array = temp_receiver;
    *receiver_num = trapSummary.receiver_num;
    
    return AC_MANAGE_SUCCESS;
}


int
trap_set_switch(unsigned int trapIndex, char *trapName, char *trapEDes, unsigned int state) {
    if(trapIndex >= trapSummary.trapDetailNUM || NULL == trapName || NULL == trapEDes) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    if(0 != strcmp(trapName, ""))
        strncpy(trapSummary.trapDetail[trapIndex].trapName, trapName, sizeof(trapSummary.trapDetail[trapIndex].trapName) - 1);

    if(0 != strcmp(trapEDes, ""))
        strncpy(trapSummary.trapDetail[trapIndex].trapEDes, trapEDes, sizeof(trapSummary.trapDetail[trapIndex].trapEDes) - 1);
    
    trapSummary.trapDetail[trapIndex].trapSwitch = state;

    return AC_MANAGE_SUCCESS;
}

int
trap_set_group_switch(struct trap_group_switch *group_switch) {
    if(NULL == group_switch) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    for(i = 0; i < 64; i++) {
        trapSummary.trapDetail[i].trapSwitch = (group_switch->low_switch & ((unsigned long long)0x1 << i)) ? 1 : 0;
    }
    for(; i < trapSummary.trapDetailNUM; i++) {
        trapSummary.trapDetail[i].trapSwitch = (group_switch->high_switch & ((unsigned long long)0x1 << (i - 64))) ? 1 : 0;
    }

    return AC_MANAGE_SUCCESS;
}

int
trap_show_switch(TRAP_DETAIL_CONFIG **trapDetail_array, unsigned int *trapDetail_num) {

    if(NULL == trapDetail_array || NULL == trapDetail_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *trapDetail_array = NULL;
    *trapDetail_num = 0;

    if(0 == trapSummary.trapDetailNUM) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    TRAP_DETAIL_CONFIG *temp_trapDetail = (TRAP_DETAIL_CONFIG *)calloc(trapSummary.trapDetailNUM, sizeof(TRAP_DETAIL_CONFIG));
    if(NULL == temp_trapDetail){
        return AC_MANAGE_MALLOC_ERROR;
    }

    memcpy(temp_trapDetail, trapSummary.trapDetail, trapSummary.trapDetailNUM * sizeof(TRAP_DETAIL_CONFIG));

    *trapDetail_array = temp_trapDetail;
    *trapDetail_num = trapSummary.trapDetailNUM;
    
    return AC_MANAGE_SUCCESS;
}

int
trap_set_heartbeat_ip(TRAPHeartbeatIP *heartbeatIP) {
    if(NULL == heartbeatIP) 
        return AC_MANAGE_INPUT_TYPE_ERROR;
    
    int i = 0;
    TRAPHeartbeatIP *heartbeat_node = NULL;
    for(i = 0, heartbeat_node = trapSummary.heartbeatIPHead; 
        i < trapSummary.heartbeatIP_num&& NULL != heartbeat_node; 
        i++, heartbeat_node = heartbeat_node->next) {

        if(heartbeat_node->local_id == heartbeatIP->local_id && heartbeat_node->instance_id == heartbeatIP->instance_id) {
            memcpy(heartbeat_node->ipAddr, heartbeatIP->ipAddr, sizeof(heartbeat_node->ipAddr));
            return AC_MANAGE_SUCCESS;
        }
    }

    TRAPHeartbeatIP *temp_node = (TRAPHeartbeatIP *)malloc(sizeof(TRAPHeartbeatIP));
    if(NULL == temp_node) {
        return AC_MANAGE_MALLOC_ERROR;
    }

    memcpy(temp_node, heartbeatIP, sizeof(TRAPHeartbeatIP));

    temp_node->next = trapSummary.heartbeatIPHead;
    trapSummary.heartbeatIPHead = temp_node;
    trapSummary.heartbeatIP_num++;

    return AC_MANAGE_SUCCESS;
}

int
trap_clear_heartbeat_ip(unsigned int local_id, unsigned int instance_id) {
    if(local_id >= VRRP_TYPE_NUM || instance_id > INSTANCE_NUM || 0 == instance_id)
        return AC_MANAGE_INPUT_TYPE_ERROR;

    int i = 0;
    TRAPHeartbeatIP *heartbeatNode = NULL, *priorNode = NULL;
    for(i = 0, heartbeatNode = trapSummary.heartbeatIPHead, priorNode = heartbeatNode;
        i < trapSummary.heartbeatIP_num && NULL != heartbeatNode;
        i++, priorNode = heartbeatNode, heartbeatNode = heartbeatNode->next) {
        
        if(local_id == heartbeatNode->local_id && instance_id == heartbeatNode->instance_id) {
            if(heartbeatNode == priorNode) {
                trapSummary.heartbeatIPHead = heartbeatNode->next;
            }
            else {
                priorNode->next = heartbeatNode->next;
            }
            
            MANAGE_FREE(heartbeatNode);
            trapSummary.heartbeatIP_num--;
            
            return AC_MANAGE_SUCCESS;
        }
    }

    return AC_MANAGE_CONFIG_NONEXIST;
    
}

int 
trap_show_heartbeat_ip(TRAPHeartbeatIP **heartbeat_array, unsigned int *heartbeat_num) {
    if(NULL == heartbeat_array || NULL == heartbeat_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *heartbeat_array = NULL;
    *heartbeat_num = 0;

    if(0 == trapSummary.heartbeatIP_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    TRAPHeartbeatIP *temp_heartbeat = (TRAPHeartbeatIP *)calloc(trapSummary.heartbeatIP_num, sizeof(TRAPHeartbeatIP));
    if(NULL == temp_heartbeat){
        return AC_MANAGE_MALLOC_ERROR;
    }

    TRAPHeartbeatIP *heartbeat_node = NULL;
    int i = 0;
    for(i = 0, heartbeat_node = trapSummary.heartbeatIPHead; 
        i < trapSummary.heartbeatIP_num && NULL != heartbeat_node; 
        i++, heartbeat_node = heartbeat_node->next) {
        
        memcpy(&temp_heartbeat[i], heartbeat_node, sizeof(TRAPHeartbeatIP));
    }

    *heartbeat_array = temp_heartbeat;
    *heartbeat_num = trapSummary.heartbeatIP_num;
    
    return AC_MANAGE_SUCCESS;
}


int
trap_set_parameter(char *paraStr, unsigned int data) {
	if(NULL == paraStr) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	int i = 0;
	for(i = 0; i < trapSummary.trapParaNUM; i++){
		syslog(LOG_DEBUG, "trapSummary.trapParameter[i].paraStr %s compare paraStr %s\n", trapSummary.trapParameter[i].paraStr, paraStr);
		if(0 == strcmp(paraStr, trapSummary.trapParameter[i].paraStr)) {
			trapSummary.trapParameter[i].data = data;
			return AC_MANAGE_SUCCESS;
		}
	}

	return AC_MANAGE_CONFIG_NONEXIST;
}

int
trap_show_parameter(TRAPParameter **parameter_array, unsigned int *parameter_num){
    
    if(NULL == parameter_array || NULL == parameter_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *parameter_array = NULL;
    *parameter_num = 0;

    if(0 == trapSummary.trapParaNUM) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    TRAPParameter *temp_parameter = (TRAPParameter *)calloc(trapSummary.trapParaNUM, sizeof(TRAPParameter));
    if(NULL == temp_parameter){
        return AC_MANAGE_MALLOC_ERROR;
    }

    memcpy(temp_parameter, trapSummary.trapParameter, trapSummary.trapParaNUM * sizeof(TRAPParameter));

    *parameter_array = temp_parameter;
    *parameter_num = trapSummary.trapParaNUM;
    
    return AC_MANAGE_SUCCESS;
}

int
snmp_manual_set_instance_status(unsigned int local_id, unsigned int instance_id, unsigned int status) {

    if(local_id >= VRRP_TYPE_NUM || 0 == instance_id || instance_id > INSTANCE_NUM) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    if(status) {
        manual_master = manual_master | (0x1 << ((local_id * INSTANCE_NUM) + instance_id - 1));
    }
    else {
        manual_master = manual_master & ~(0x1 << ((local_id * INSTANCE_NUM) + instance_id - 1));
    }

    return AC_MANAGE_SUCCESS;
}

int
snmp_show_manual_set_instance_master(unsigned int *instance_state) {
    if(NULL == instance_state) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    *instance_state = manual_master;

    return AC_MANAGE_SUCCESS;
}

int
snmp_set_debug_level(unsigned int debugLevel) {

    switch(debugLevel) {
        case 0:
        case 1:
        case 2:
            snmpDebugLevel = debugLevel;
            break;
            
        default:
            syslog(LOG_INFO, "snmp_set_debug_level: unknow debuglevel %d\n", debugLevel);
            return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    return AC_MANAGE_SUCCESS;
}

int
snmp_show_debug_level(unsigned int *debugLevel) {
    return *debugLevel = snmpDebugLevel;
}


int
trap_set_debug_level(unsigned int debugLevel) {

    switch(debugLevel) {
        case 0:
        case 1:
        case 2:
            trapDebugLevel = debugLevel;
            break;
            
        default:
            syslog(LOG_INFO, "trap_set_debug_level: unknow debuglevel %d\n", debugLevel);
            return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
    return AC_MANAGE_SUCCESS;
}

int
trap_show_debug_level(unsigned int *debugLevel) {
    return *debugLevel = trapDebugLevel;
}

void
config_snmp_sysoid_boardtype(unsigned int sysoid) {
	snmpSummary.snmp_sysinfo.sysoid_boardtype = sysoid;
    update_snmp_sysinfo();
}


int 
snmp_show_running_config(struct running_config **configHead, unsigned int type) {
    if(NULL == configHead) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *configHead = NULL;
    struct running_config *configEnd = NULL;

    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "snmp_show_running_config: get loal slot id error\n");
            manage_free_running_config(configHead);
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }

    switch(type) {
        case SHOW_RUNNING_SNMP_MANUAL:
            if(manual_master) {
            
                int i = 0;
                for(i = 0; i < (INSTANCE_NUM * 2); i++) {
                    if(manual_master & (0x1 << i)) {
                
                        unsigned int local_id = i / INSTANCE_NUM;
                        unsigned int instance_id = i - (local_id * INSTANCE_NUM) + 1;
                        syslog(LOG_DEBUG, "local_id = %d, instance_id = %d\n", local_id, instance_id);
                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                            snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " snmp manual set %s %d-%d master", 
                                        local_id ? "local_hansi" : "remote_hansi", local_slotID, instance_id);
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                        }    
                    }
                }
            }  
            break;
            
        case SHOW_RUNNING_SNMP_CONFIG:
            {                
                char space_str[10] = { 0 };
                if(snmpSummary.snmp_sysinfo.collection_mode) {
                    strncpy(space_str, "  ", sizeof(space_str) - 1);        
                }
                else {
                    strncpy(space_str, " ", sizeof(space_str) - 1);        
                }
                
                if(snmpSummary.snmp_sysinfo.collection_mode) {
                
                    struct running_config *temp_config = manage_new_running_config();
                    if(temp_config) {
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " config snmp slot %d", local_slotID);
                        manage_insert_running_config(configHead, &configEnd, temp_config);
                    }    
                }    
                /*
                    *   snmp sysinfo
                    */    
                if(RULE_ENABLE != snmpSummary.snmp_sysinfo.v1_status) {
                    struct running_config *temp_config = manage_new_running_config();
                    if(temp_config) {
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sset snmp mode v1 disable", space_str);
                        manage_insert_running_config(configHead, &configEnd, temp_config);
                    }    
                }
                
                if(RULE_ENABLE != snmpSummary.snmp_sysinfo.v2c_status) {
                    struct running_config *temp_config = manage_new_running_config();
                    if(temp_config) {
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sset snmp mode v2 disable", space_str);
                        manage_insert_running_config(configHead, &configEnd, temp_config);
                    }
                }
                
                if(RULE_ENABLE != snmpSummary.snmp_sysinfo.v3_status) {
                    struct running_config *temp_config = manage_new_running_config();
                    if(temp_config) {
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sset snmp mode v3 disable", space_str);
                        manage_insert_running_config(configHead, &configEnd, temp_config);
                    }
                }

				if(snmpSummary.snmp_sysinfo.sysoid_boardtype){
                    struct running_config *temp_config = manage_new_running_config();
                    if(temp_config) {
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sconfig sysoid %d", space_str, snmpSummary.snmp_sysinfo.sysoid_boardtype);
                        manage_insert_running_config(configHead, &configEnd, temp_config);
                    }
				}
                
                if(snmpSummary.snmp_sysinfo.agent_port) {
                   int i = 0;
                   SNMPINTERFACE *interfaceNode = NULL;
                   for(i = 0, interfaceNode = snmpSummary.interfaceHead; 
                       i < snmpSummary.interface_num && NULL != interfaceNode; 
                       i++, interfaceNode = interfaceNode->next) {
                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                            snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%ssnmp apply interface %s udp port %d",
                                        space_str, interfaceNode->ifName, snmpSummary.snmp_sysinfo.agent_port);
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                        }
                    }
                }

		if(snmpSummary.snmp_sysinfo.agent_port_ipv6) {
		   int i = 0;
		   SNMPINTERFACE *interfaceNode = NULL;
		   for(i = 0, interfaceNode = snmpSummary.ipv6_interfaceHead; 
		       i < snmpSummary.ipv6_interface_num && NULL != interfaceNode; 
		       i++, interfaceNode = interfaceNode->next) {
			struct running_config *temp_config = manage_new_running_config();
			if(temp_config) {
			    snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%ssnmp ipv6  apply interface %s udp port %d",
					space_str, interfaceNode->ifName, snmpSummary.snmp_sysinfo.agent_port_ipv6);
			    manage_insert_running_config(configHead, &configEnd, temp_config);
			}
		    }
		}

			if(snmpSummary.snmp_sysinfo.cache_time > 0 && DEFAULT_SNMPD_CACHETIME != snmpSummary.snmp_sysinfo.cache_time) {                  
				struct running_config *temp_config = manage_new_running_config();
				if(temp_config) {
					snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sset cachetime %d",
												space_str, snmpSummary.snmp_sysinfo.cache_time);
					manage_insert_running_config(configHead, &configEnd, temp_config);
				}
			}
                
                /*
                    *   snmp community
                    */
                if(snmpSummary.community_num > 0) {
                    int i = 0;
                    STCommunity *community_node = NULL;
                    for(i = 0, community_node = snmpSummary.communityHead; 
                        i < snmpSummary.community_num && NULL != community_node; 
                        i++, community_node = community_node->next) {
                        
                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                            snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sadd community %s %s %s %s %s",
                                    space_str, community_node->community, community_node->ip_addr, community_node->ip_mask, 
                                    ACCESS_MODE_RO == community_node->access_mode ? "ro" : "rw",
                                    RULE_ENABLE == community_node->status ? "enable" : "disable");
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                        }
                        
                    }
                }
		
                /*
                    *   snmp ipv6 community
                    */
                if(snmpSummary.ipv6_community_num  > 0) {
                    int i = 0;
                    IPV6STCommunity *community_node = NULL;
                    for(i = 0, community_node = snmpSummary.ipv6_communityHead; 
                        i < snmpSummary.ipv6_community_num && NULL != community_node; 
                        i++, community_node = community_node->next) {
                        
                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                            snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sadd ipv6 community %s %s %d %s %s",
                                    space_str, community_node->community, community_node->ip_addr, community_node->prefix, 
                                    ACCESS_MODE_RO == community_node->access_mode ? "ro" : "rw",
                                    RULE_ENABLE == community_node->status ? "enable" : "disable");
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                        }
                        
                    }
                }
                
                /*
                   *   snmp view
                   */
                if(snmpSummary.view_num > 0) {
                    int i = 0;
                    STSNMPView *view_node = NULL;
                    for(i = 0, view_node = snmpSummary.viewHead; 
                        i < snmpSummary.view_num && NULL != view_node; 
                        i++, view_node = view_node->next) {
                
                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                            snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%screate view %s", space_str, view_node->name);
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                
                            struct running_config *next_config = manage_new_running_config();
                            if(next_config) {
                                snprintf(next_config->showStr, sizeof(next_config->showStr) - 1, "%sconfig view %s", space_str, view_node->name);
                                manage_insert_running_config(configHead, &configEnd, next_config);
                            }
                            
                            int j = 0;
                            struct oid_list *oid_node = NULL;
                            for(j = 0, oid_node = view_node->view_included.oidHead; 
                                j < view_node->view_included.oid_num && NULL != oid_node; 
                                j++, oid_node = oid_node->next) {
                                
                                struct running_config *temp_sub_config = manage_new_running_config();
                                if(temp_sub_config) {
                                    snprintf(temp_sub_config->showStr, sizeof(temp_sub_config->showStr) - 1, "%s add view-included %s", space_str, oid_node->oid);
                                    manage_insert_running_config(configHead, &configEnd, temp_sub_config);
                                }
                            }
                
                            for(j = 0, oid_node = view_node->view_excluded.oidHead; 
                                j < view_node->view_excluded.oid_num && NULL != oid_node; 
                                j++, oid_node = oid_node->next) {
                                
                                struct running_config *temp_sub_config = manage_new_running_config();
                                if(temp_sub_config) {
                                    snprintf(temp_sub_config->showStr, sizeof(temp_sub_config->showStr) - 1, "%s add view-excluded %s", space_str, oid_node->oid);
                                    manage_insert_running_config(configHead, &configEnd, temp_sub_config);
                                }
                            }
                            struct running_config *exit_config = manage_new_running_config();
                            if(exit_config) {
                                snprintf(exit_config->showStr, sizeof(exit_config->showStr) - 1, "%s exit", space_str);
                                manage_insert_running_config(configHead, &configEnd, exit_config);
                            }        
                        }
                    }
                }
                
                /*
                   *   snmp group
                   */
                if(snmpSummary.group_num > 0) {
                    int i = 0;
                    STSNMPGroup *group_node = NULL;
                    for(i = 0, group_node = snmpSummary.groupHead; 
                        i < snmpSummary.group_num && NULL != group_node; 
                        i++, group_node = group_node->next) {
                        
                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                
                            char level_str[10] = { 0 };
                            if(SEC_NOAUTH== group_node->sec_level) {
                                strncpy(level_str, "noauth", sizeof(level_str) - 1);
                            }
                            else if(SEC_AUTH == group_node->sec_level) {
                                strncpy(level_str, "auth", sizeof(level_str) - 1);
                            }
                            else if(SEC_PRIV == group_node->sec_level){
                                strncpy(level_str, "priv", sizeof(level_str) - 1);
                            }
                            else {
                                MANAGE_FREE(temp_config);
                                continue;
                            }
                
                            snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sadd group %s %s %s %s",space_str, 
                                    group_node->group_name, group_node->group_view, ACCESS_MODE_RO == group_node->access_mode ? "ro" : "rw", level_str);
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                        }
                        
                    }
                }
                
                 /*
                   *   snmp v3user
                   */
                if(snmpSummary.v3user_num > 0) {
                    int i = 0;
                    STSNMPV3User *v3user_node = NULL;
                    for(i = 0, v3user_node = snmpSummary.v3userHead; 
                        i < snmpSummary.v3user_num && NULL != v3user_node; 
                        i++, v3user_node = v3user_node->next) {
                        
                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                
                            if(AUTH_PRO_NONE == v3user_node->authentication.protocal) {
                                snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sadd v3user %s %s %s %s", space_str,
                                            v3user_node->name, "none", v3user_node->group_name, v3user_node->status ? "enable" : "disable");
                            }
                            else {
                            
                                char auth_str[10] = { 0 };
                                if(AUTH_PRO_MD5 == v3user_node->authentication.protocal) {
                                    strncpy(auth_str, "md5", sizeof(auth_str) - 1);
                                }
                                else if(AUTH_PRO_SHA == v3user_node->authentication.protocal) {
                                    strncpy(auth_str, "sha", sizeof(auth_str) - 1);
                                }
                                else {
                                    MANAGE_FREE(temp_config);
                                    continue;
                                }
                                
                                if(PRIV_PRO_NONE == v3user_node->privacy.protocal) {
                                    snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sadd v3user %s %s %s %s %s %s", space_str,
                                                v3user_node->name, auth_str, v3user_node->authentication.passwd, "none", v3user_node->group_name,
                                                v3user_node->status ? "enable" : "disable");
                                }
                                else if(PRIV_PRO_DES == v3user_node->privacy.protocal){
                                    snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sadd v3user %s %s %s %s %s %s %s", space_str,
                                                v3user_node->name, auth_str, v3user_node->authentication.passwd, "des", v3user_node->privacy.passwd,
                                                v3user_node->group_name, v3user_node->status ? "enable" : "disable");
                                }
                                else {
                                    MANAGE_FREE(temp_config);
                                    continue;
                                }
                 
                            }            
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                        }
                        
                    }
                }
                
                /*
                   *   snmp service state
                   */
                if(snmp_show_service_state()) {
                    struct running_config *temp_config = manage_new_running_config();
                    if(temp_config) {
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%ssnmp service enable", space_str);
                        manage_insert_running_config(configHead, &configEnd, temp_config);
                    }
                }
                
                if(snmpSummary.snmp_sysinfo.collection_mode) {
                    struct running_config *temp_config = manage_new_running_config();
                    if(temp_config) {
                        snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, "%sexit", space_str);
                        manage_insert_running_config(configHead, &configEnd, temp_config);
                    }
                }
            }
            break;
            
        case SHOW_RUNNING_TRAP_RECEIVER:
            {
                if(trapSummary.receiver_num) {
                    
                    int i = 0;
                    STSNMPTrapReceiver *receiver_node = NULL;
                    for(i = 0, receiver_node = trapSummary.receiverHead;
                        i < trapSummary.receiver_num && NULL != receiver_node;
                        i++, receiver_node = receiver_node->next) {
                
                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                            char version[10] = { 0 };
                            if(3 == receiver_node->version) {
                                strncpy(version, "v3", sizeof(version) - 1);
                            }
                            else if(2 == receiver_node->version) {
                                strncpy(version, "v2c", sizeof(version) - 1);
                            }
                            else {
                                strncpy(version, "v1", sizeof(version) - 1);
                            }
                            
                            if(0 == receiver_node->sour_ipAddr[0]) {
                                snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " add trap %s %s %d-%d %s destaddr %s %s %s %d",
                                            receiver_node->name, receiver_node->local_id ? "local-hansi" : "remote-hansi", local_slotID, receiver_node->instance_id,
                                            version, receiver_node->dest_ipAddr, receiver_node->trapcom, receiver_node->status ? "enable" : "disable", 
                                            receiver_node->dest_port ? receiver_node->dest_port : 162);
                            }
                            else {
                                snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " add trap %s %s %d-%d %s souraddr %s destaddr %s %s %s %d",
                                            receiver_node->name, receiver_node->local_id ? "local-hansi" : "remote-hansi", local_slotID, receiver_node->instance_id,
                                            version, receiver_node->sour_ipAddr, receiver_node->dest_ipAddr, receiver_node->trapcom, 
                                            receiver_node->status ? "enable" : "disable", receiver_node->dest_port ? receiver_node->dest_port : 162);
                            }
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                        }
                                
                    }
                }

                if(trapSummary.heartbeatIP_num) {
                    int i = 0;
                    TRAPHeartbeatIP *heartbeat_node = NULL;
                    for(i = 0, heartbeat_node = trapSummary.heartbeatIPHead;
                        i < trapSummary.heartbeatIP_num && NULL != heartbeat_node;
                        i++, heartbeat_node = heartbeat_node->next) {

                        struct running_config *temp_config = manage_new_running_config();
                        if(temp_config) {
                            snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " set trap %s %d-%d heartbeat %s",
                                      heartbeat_node->local_id ? "local-hansi" : "remote-hansi", local_slotID, 
                                      heartbeat_node->instance_id, heartbeat_node->ipAddr);
                            manage_insert_running_config(configHead, &configEnd, temp_config);
                        }
                    }
                }
            }
            break;
		case SHOW_RUNNING_TRAP_CONFIG:
			{
				if(trapSummary.trapParaNUM) {

					unsigned int resendTimes = 0;
					unsigned int resendInterval = 0;

					int i = 0;
					for(i = 0; i < trapSummary.trapParaNUM; i++) {

						syslog(LOG_DEBUG, "trapSummary.trapParameter[i].paraStr = %s, data = %d\n", 
											trapSummary.trapParameter[i].paraStr, trapSummary.trapParameter[i].data);

						if(0 == strcmp(RESEND_INTERVAL, trapSummary.trapParameter[i].paraStr)) {
							resendInterval = trapSummary.trapParameter[i].data;
							continue ;
						} else if(0 == strcmp(RESEND_TIMES, trapSummary.trapParameter[i].paraStr)) {
							resendTimes = trapSummary.trapParameter[i].data;
							continue ;
						} else if(0 == strcmp(HEARTBEAT_INTERVAL, trapSummary.trapParameter[i].paraStr)) {
							if(DEFAULT_TRAP_HEARTBEAT_INTERVAL == trapSummary.trapParameter[i].data) {
								continue ;
							}
						} else if(0 == strcmp(HEARTBEAT_MODE, trapSummary.trapParameter[i].paraStr)) {
							if(DEFAULT_TRAP_HEARTBEAT_MODE == trapSummary.trapParameter[i].data) {
								continue ;
							}
						} 

						struct running_config *temp_config = manage_new_running_config();
						if(temp_config) {
							snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " set %s %d", 
							trapSummary.trapParameter[i].paraStr, trapSummary.trapParameter[i].data);
							manage_insert_running_config(configHead, &configEnd, temp_config);
						}
					}    

					if(DEFAULT_TRAP_RESEND_INTERVAL != resendInterval || DEFAULT_TRAP_RESEND_TIMES != resendTimes) {
						struct running_config *temp_config = manage_new_running_config();
						if(temp_config) {
							snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " set trap resend interval %d times %d", resendInterval, resendTimes);
							manage_insert_running_config(configHead, &configEnd, temp_config);
						}
					}		
				}

			if(trapSummary.trapDetailNUM) {
				unsigned int config_flag = 0;
				char switch_buf[256] = { 0 };

				int i = 0;
				for(i = 0; i < trapSummary.trapDetailNUM; i++) {
					syslog(LOG_DEBUG, "trapSummary.trapDetail[%d].trapSwitch = %d\n", i, trapSummary.trapDetail[i].trapSwitch);
					if(trapSummary.trapDetail[i].trapSwitch) {
						switch_buf[i] = '1';
						config_flag = 1;
					} else {
						switch_buf[i] = '0';
					}
				} 
				
				if(config_flag) {
					struct running_config *temp_config = manage_new_running_config();
					if(temp_config) {
						snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " set trap group switch %s", switch_buf);
						manage_insert_running_config(configHead, &configEnd, temp_config);
					}
				}	
			}
                
                
			if(trap_show_service_state()) {
				struct running_config *temp_config = manage_new_running_config();
				if(temp_config) {
					snprintf(temp_config->showStr, sizeof(temp_config->showStr) - 1, " trap service enable");
					manage_insert_running_config(configHead, &configEnd, temp_config);
				}
			}
		}
		break;

		default:
			syslog(LOG_WARNING, "snmp_show_running_config: the %d type is unknow", type);
			break;
	}

	return AC_MANAGE_SUCCESS;
}

