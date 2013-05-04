#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <syslog.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include "wbmd.h"
#include "wbmd/wbmdpub.h"
#include "wbmd/wbmdmibdef.h"
#include "dbus/wbmd/WbmdDbusDef.h"
#include "wbmd_thread.h"
#include "wbmd_dbus.h"
#include "wbmd_check.h"
#include "wbmd_manage.h"
#include "wbmd_dbushandle.h"
#include <net-snmp/library/tools.h>
#include <net-snmp/library/system.h>
#include <net-snmp/library/default_store.h>    /* for "internal" definitions */

int wbmd_oid_response_check(netsnmp_variable_list **vars){
	if(*vars){
		if(((*vars)->type == SNMP_ENDOFMIBVIEW) ||
           ((*vars)->type == SNMP_NOSUCHOBJECT) ||
           ((*vars)->type == SNMP_NOSUCHINSTANCE)){
			(*vars) = (*vars)->next_variable;
			return 0;
		}
	}else{
		return 0;
	}
	return 1;
}

int wbmd_get_wbridge_rm_property_info(int WBID){
	netsnmp_session *ss;
	netsnmp_pdu    *pdu;
	netsnmp_pdu    *response;
	netsnmp_variable_list *vars;
	oid name[MAX_OID_LEN];
	size_t	name_length;
	int status;
	int exitval = 0;		
	int i = 0;
	int j = 0;
	int failures = 0;
	char tmp[PATH_LEN];
	wbmd_syslog_info("wbid %d get info\n",WBID);
	if(wbmd_id_check(WBID) == WBMD_TRUE)
	{
		SOCK_STARTUP;
		ss = snmp_open(&(wBridge[WBID]->session));
		if (ss == NULL) {
			wbmd_syslog_info("11111111111111wbid %d get info\n",WBID);
			snmp_sess_perror("snmpget", &(wBridge[WBID]->session));
			SOCK_CLEANUP;
			return 0;
		}
		wbmd_syslog_info("%s 222222222222222wbid %d get info\n",__func__,WBID);
		i = 3;
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		for(j = 1; j <= 21 ; j ++){							
			name_length = MAX_OID_LEN;
			memset(tmp,0,PATH_LEN);
			sprintf(tmp,"%s.%d.%d",WBRIDGE_RM_PROPERTY_INFO_OID,j,i);
			if(!snmp_parse_oid(tmp, name, &name_length)) {
				snmp_perror(tmp);
				failures++;
			}else{
				snmp_add_null_var(pdu, name, name_length);
			}
		}
		wbmd_syslog_info("%s 33333333333333333wbid %d get info\n",__func__,WBID);
		status = snmp_synch_response(ss, pdu, &response);
		if (status == STAT_SUCCESS){			
			wbmd_syslog_info("%s 44444444444444wbid %d get info\n",__func__,WBID);
			wBridge[WBID]->GetRfInfoTimes = 1;
			vars = response->variables;
			if(wbmd_oid_response_check(&vars)){
                print_variable(vars->name, vars->name_length, vars);
				wBridge[WBID]->WBRmProperty.rmPropertiesIfIndex = *(vars->val.integer);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmPropertiesIfIndex);
			}
			if(wbmd_oid_response_check(&vars)){
				strcpy(wBridge[WBID]->WBRmProperty.rmType , (char *)(vars->val.string));
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%s\n",wBridge[WBID]->WBRmProperty.rmType);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmFrequency = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmFrequency);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmBitRate = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmBitRate);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmSid = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmSid);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmCurPowerLevel = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmCurPowerLevel);
			}						
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmModulation = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmModulation);
			}						
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmAntenna = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmAntenna);
			}						
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmDistance= *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmDistance);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmBurst = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmBurst);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmLongRange = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmLongRange);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmPowerCtl = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmPowerCtl);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmTXRT = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmTXRT);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmTXVRT= *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmTXVRT);
			}						
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmPTP = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmPTP);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmWOCD = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmWOCD);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmBCsid = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmBCsid);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmDistanceAuto = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmDistanceAuto);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmNoiseFloor= *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmNoiseFloor);
			}			
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmBandwidth= *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmBandwidth);
			}			
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBRmProperty.rmChainMode = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBRmProperty.rmChainMode);
			}			
		}
		else if (status == STAT_TIMEOUT) {			
			if(wBridge[WBID]->GetRfInfoTimes < 10)
				wBridge[WBID]->GetRfInfoTimes += 1;
			wbmd_syslog_info("44444444444444wbid %d get info\n",WBID);
			fprintf(stderr, "Timeout: No Response from %s.\n",
					wBridge[WBID]->session.peername);
			exitval = 1;
		} else {			
			if(wBridge[WBID]->GetRfInfoTimes < 10)
				wBridge[WBID]->GetRfInfoTimes += 1;
			wbmd_syslog_info("5555555555555wbid %d get info\n",WBID);
			snmp_sess_perror("snmpget", ss);
			exitval = 1;

		}
		wbmd_syslog_info("%s 55555555555555555555555 wbid %d get info\n",__func__,WBID);
		if (response)
			snmp_free_pdu(response);
		wbmd_syslog_info("%s 666666666666666666666 wbid %d get info\n",__func__,WBID);
		snmp_close(ss);
		SOCK_CLEANUP;				
	}
	return 1;
}

int wbmd_get_wbridge_mint_info(int WBID){
	netsnmp_session *ss;
	netsnmp_pdu    *pdu;
	netsnmp_pdu    *response;
	netsnmp_variable_list *vars;
	oid name[MAX_OID_LEN];
	size_t	name_length;
	int status;
	int exitval = 0;		
	int i = 0;
	int j = 0;
	int failures = 0;
	char tmp[PATH_LEN];
	wbmd_syslog_info("wbid %d get info\n",WBID);
	if(wbmd_id_check(WBID) == WBMD_TRUE)
	{
		SOCK_STARTUP;
		ss = snmp_open(&(wBridge[WBID]->session));
		if (ss == NULL) {
			wbmd_syslog_info("11111111111111wbid %d get info\n",WBID);
			snmp_sess_perror("snmpget", &(wBridge[WBID]->session));
			SOCK_CLEANUP;
			return 0;
		}
		wbmd_syslog_info("%s 22222222222wbid %d get info\n",__func__,WBID);
		i = 3;
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		for(j = 1; j <= 27 ; j ++){							
			name_length = MAX_OID_LEN;
			memset(tmp,0,PATH_LEN);
			sprintf(tmp,"%s.%d.%d",WBRIDGE_MINT_INFO_OID,j,i);
			if(!snmp_parse_oid(tmp, name, &name_length)) {
				snmp_perror(tmp);
				failures++;
			}else{
				snmp_add_null_var(pdu, name, name_length);
			}
		}
		wbmd_syslog_info("%s 3333333333333333333333333wbid %d get info\n",__func__,WBID);
		status = snmp_synch_response(ss, pdu, &response);
		if (status == STAT_SUCCESS){			
			wbmd_syslog_info("%s 44444444444444444444444444wbid %d get info\n",__func__,WBID);
			wBridge[WBID]->GetMintInfoTimes = 1;
			vars = response->variables;
			if(wbmd_oid_response_check(&vars)){
				memcpy(wBridge[WBID]->WBMintNode.netAddress, (char *)(vars->val.string),MAC_LEN);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%s\n",wBridge[WBID]->WBMintNode.netAddress);
			}
			if(wbmd_oid_response_check(&vars)){
                print_variable(vars->name, vars->name_length, vars);
				wBridge[WBID]->WBMintNode.nodeType = *(vars->val.integer);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.nodeType);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.nodeMode = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.nodeMode);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.linksCount = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.linksCount);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.nodesCount = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.nodesCount);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.nodeInterfaceId = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.nodeInterfaceId);
			}						
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.protocolEnabled = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.protocolEnabled);
			}						
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				strcpy(wBridge[WBID]->WBMintNode.nodeName, (char *)(vars->val.string));
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%s\n",wBridge[WBID]->WBMintNode.nodeName);
			}						
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.autoBitrateEnable = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.autoBitrateEnable);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.autoBitrateAddition = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.autoBitrateAddition);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.autoBitrateMinLevel= *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.autoBitrateMinLevel);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.extraCost = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.extraCost);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.fixedCost = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.fixedCost);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.nodeID= *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.nodeID);
			}						
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.ampLow = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.ampLow);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.ampHigh = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.ampHigh);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.authMode = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.authMode);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.authRelay= *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.authRelay);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.crypt = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.crypt);
			}			
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.compress = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.compress);
			}			
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.overTheAirUpgradeEnable = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.overTheAirUpgradeEnable);
			}			
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.overTheAirUpgradeSpeed = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.overTheAirUpgradeSpeed);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.roaming = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.roaming);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.polling = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.polling);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.mintBroadcastRate = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.mintBroadcastRate);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				wBridge[WBID]->WBMintNode.noiseFloor = *(vars->val.integer);
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%d\n",wBridge[WBID]->WBMintNode.noiseFloor);
			}
			if(wbmd_oid_response_check(&vars)){
				//vars = vars->next_variable;
				strcpy(wBridge[WBID]->WBMintNode.secretKey, (char *)(vars->val.string));
                print_variable(vars->name, vars->name_length, vars);
				vars = vars->next_variable;
				printf("%s\n",wBridge[WBID]->WBMintNode.secretKey);
			}

		} 

		 else if (status == STAT_TIMEOUT) {		 	
			if(wBridge[WBID]->GetMintInfoTimes < 10)
				wBridge[WBID]->GetMintInfoTimes += 1;
			wbmd_syslog_info("44444444444444wbid %d get info\n",WBID);
			fprintf(stderr, "Timeout: No Response from %s.\n",
					wBridge[WBID]->session.peername);
			exitval = 1;
		} else {
			if(wBridge[WBID]->GetMintInfoTimes < 10)
				 wBridge[WBID]->GetMintInfoTimes += 1;
			wbmd_syslog_info("5555555555555wbid %d get info\n",WBID);
			snmp_sess_perror("snmpget", ss);
			exitval = 1;

		}
		wbmd_syslog_info("%s 55555555555555555555555wbid %d get info\n",__func__,WBID);
		if (response)
			snmp_free_pdu(response);
		wbmd_syslog_info("%s 66666666666666666666666666wbid %d get info\n",__func__,WBID);
		snmp_close(ss);
		SOCK_CLEANUP;				
	}
	return 1;
}



int wbmd_get_wbridge_if_basic_info(int WBID){
	netsnmp_session *ss;
	netsnmp_pdu    *pdu;
	netsnmp_pdu    *response;
	netsnmp_variable_list *vars;
	oid name[MAX_OID_LEN];
	size_t	name_length;
	int status;
	int exitval = 0;		
	int i = 0;
	int j = 0;
	int failures = 0;
	char tmp[PATH_LEN];
	wbmd_syslog_info("wbid %d get info\n",WBID);
	if(wbmd_id_check(WBID) == WBMD_TRUE)
	{
		SOCK_STARTUP;
		ss = snmp_open(&(wBridge[WBID]->session));
		if (ss == NULL) {
			wbmd_syslog_info("11111111111111wbid %d get info\n",WBID);
			snmp_sess_perror("snmpget", &(wBridge[WBID]->session));
			SOCK_CLEANUP;
			return 0;
		}
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		{
			name_length = MAX_OID_LEN;
			if (!snmp_parse_oid(WBRIDGE_IF_NUM_OID, name, &name_length)) {
				snmp_perror(WBRIDGE_IF_NUM_OID);
				failures++;
			} else{
				wbmd_syslog_info("name:%x%x%x%x%x%x\n",name[0],name[1],name[2],name[3],name[4],name[5]);						
				snmp_add_null_var(pdu, name, name_length);
			}
		}
		if (failures) {
			snmp_close(ss);
			SOCK_CLEANUP;
			wbmd_syslog_info("2222222222222222wbid %d get info\n",WBID);
			return 0;
		}
		wbmd_syslog_info("%s 222222222222222222wbid %d get info\n",__func__,WBID);
		status = snmp_synch_response(ss, pdu, &response);
		if (status == STAT_SUCCESS) {
		  wBridge[WBID]->GetIfInfoTimes = 1;
		  wbmd_syslog_info("%s 3333333333333333333333wbid %d get info\n",__func__,WBID);
		  vars = response->variables;
		  if(wbmd_oid_response_check(&vars)){
			  print_variable(vars->name, vars->name_length, vars);
			  wBridge[WBID]->if_num = *(vars->val.integer);	
		  }			  
		  if (response)
			  snmp_free_pdu(response);
		  wbmd_syslog_info("%s 444444444444444444444wbid %d get info\n",__func__,WBID);
		  if(wBridge[WBID]->if_num != 0){
			for(i = 0; i < wBridge[WBID]->if_num; i++){						
				pdu = snmp_pdu_create(SNMP_MSG_GET);
				for(j = 1; j < 23 ; j ++){							
					name_length = MAX_OID_LEN;
					memset(tmp,0,PATH_LEN);
					sprintf(tmp,"%s.%d.%d",WBRIDGE_IF_INFO_BASIC_OID,j,i+1);
					if(!snmp_parse_oid(tmp, name, &name_length)) {
						snmp_perror(tmp);
						failures++;
					}else{
						snmp_add_null_var(pdu, name, name_length);
					}
				}
				wbmd_syslog_info("%s 555555555555555555555555wbid %d get info\n",__func__,WBID);
				status = snmp_synch_response(ss, pdu, &response);
				if (status == STAT_SUCCESS){					
					wbmd_syslog_info("%s 666666666666666666666666wbid %d get info\n",__func__,WBID);
					wBridge[WBID]->GetIfInfoTimes = 1;
					vars = response->variables;
					if(wbmd_oid_response_check(&vars)){
		                print_variable(vars->name, vars->name_length, vars);
						wBridge[WBID]->WBIF[i].ifIndex = *(vars->val.integer);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifIndex);
					}
					if(wbmd_oid_response_check(&vars)){
						strcpy(wBridge[WBID]->WBIF[i].ifDescr, (char *)(vars->val.string));
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%s\n",wBridge[WBID]->WBIF[i].ifDescr);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifType= *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifType);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifMtu= *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifMtu);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifSpeed = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifSpeed);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						memcpy(wBridge[WBID]->WBIF[i].ifPhysAddress ,vars->val.string, 6);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%02X\n",wBridge[WBID]->WBIF[i].ifPhysAddress[5]);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifAdminStatus = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifAdminStatus);
					}						
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifOperStatus = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifOperStatus);
					}						
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifLastChange = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifLastChange);
					}						
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifInOctets = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifInOctets);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifInUcastPkts = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifInUcastPkts);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifInNUcastPkts = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifInNUcastPkts);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifInDiscards = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifInDiscards);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifInErrors = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifInErrors);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifInUnknownProtos = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifInUnknownProtos);
					}						
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifOutOctets= *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifOutOctets);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifOutUcastPkts= *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifOutUcastPkts);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifOutNUcastPkts= *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifOutNUcastPkts);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifOutDiscards = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifOutDiscards);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifOutErrors = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifOutErrors);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifOutQLen = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						vars = vars->next_variable;
						printf("%d\n",wBridge[WBID]->WBIF[i].ifOutQLen);
					}
					if(wbmd_oid_response_check(&vars)){
						//vars = vars->next_variable;
						wBridge[WBID]->WBIF[i].ifSpecific = *(vars->val.integer);
		                print_variable(vars->name, vars->name_length, vars);
						printf("%d\n",wBridge[WBID]->WBIF[i].ifSpecific);
					}
					
				}else{
					if(wBridge[WBID]->GetIfInfoTimes < 10)
						wBridge[WBID]->GetIfInfoTimes += 1;
				}
				if (response)
					snmp_free_pdu(response);
			}
		  }
				  
		}  else if (status == STAT_TIMEOUT) {
			wbmd_syslog_info("44444444444444wbid %d get info\n",WBID);
			if(wBridge[WBID]->GetIfInfoTimes < 10)
				wBridge[WBID]->GetIfInfoTimes += 1;
			fprintf(stderr, "Timeout: No Response from %s.\n",
					wBridge[WBID]->session.peername);
			exitval = 1;
		} else {
			wbmd_syslog_info("5555555555555wbid %d get info\n",WBID);
			if(wBridge[WBID]->GetIfInfoTimes < 10)
				wBridge[WBID]->GetIfInfoTimes += 1;
			snmp_sess_perror("snmpget", ss);
			exitval = 1;
		}
		wbmd_syslog_info("%s 777777777777777777777777777777wbid %d get info\n",__func__,WBID);

		snmp_close(ss);
		SOCK_CLEANUP;				
		wbmd_syslog_info("%s 88888888888888888888888wbid %d get info\n",__func__,WBID);
	}
	return 1;
}

void * WBMDManage(void *arg) {	
	int QID = ((WBMDThreadArg*)arg)->QID;
	wbmd_pid_write_v2("WBMDManage");
	int WBMsgqID;
	struct WbmdMsgQ msg;
	WBMD_FREE_OBJECT(arg);	
	WbmdGetMsgQueue(&WBMsgqID);
	while(1){		
		int WBID = 0;	
		#if 0
		netsnmp_session *ss;
		netsnmp_pdu    *pdu;
		netsnmp_pdu    *response;
		netsnmp_variable_list *vars;
		int count;
		int current_name = 0;
		char *names[SNMP_MAX_CMDLINE_OIDS];
		oid name[MAX_OID_LEN];
		size_t	name_length;
		int status;
		int failures = 0;
		int exitval = 0;
		#endif
		memset((char*)&msg, 0, sizeof(msg));
		if (msgrcv(WBMsgqID, (struct WbmdMsgQ*)&msg, sizeof(msg.mqinfo), QID, 0) == -1) {
			wbmd_syslog_info("%s msgrcv %s",__func__,strerror(errno));
			perror("msgrcv");
			continue;
		}
		if(msg.mqinfo.op == WBMD_GET){
			if(msg.mqinfo.type == WBMD_IF){
				WBID = msg.mqinfo.WBID;			
				wbmd_syslog_info("wbid %d get info\n",WBID);
				wbmd_get_wbridge_if_basic_info(WBID);				
				if(wbmd_id_check(WBID) == WBMD_TRUE){			
					WbmdTimerCancel(&(wBridge[WBID]->GetIfInfoTimerID),1);
					WBMDTimerRequest(10*wBridge[WBID]->GetIfInfoTimes,&(wBridge[WBID]->GetIfInfoTimerID),WBMD_GETIFINFO,WBID);
				}
			}else if(msg.mqinfo.type == WBMD_MINT){
				WBID = msg.mqinfo.WBID;			
				wbmd_syslog_info("wbid %d get info\n",WBID);
				wbmd_get_wbridge_mint_info(WBID);				
				if(wbmd_id_check(WBID) == WBMD_TRUE){
					WbmdTimerCancel(&(wBridge[WBID]->GetMintInfoTimerID),1);			
					WBMDTimerRequest(11*wBridge[WBID]->GetMintInfoTimes,&(wBridge[WBID]->GetMintInfoTimerID),WBMD_GETMINTINFO,WBID);
				}			
			}else if(msg.mqinfo.type == WBMD_RF){
				WBID = msg.mqinfo.WBID;
				wbmd_syslog_info("wbid %d get info\n",WBID);
				wbmd_get_wbridge_rm_property_info(WBID);				
				if(wbmd_id_check(WBID) == WBMD_TRUE){
					WbmdTimerCancel(&(wBridge[WBID]->GetRfInfoTimerID),1);										
					WBMDTimerRequest(12*wBridge[WBID]->GetRfInfoTimes,&(wBridge[WBID]->GetRfInfoTimerID),WBMD_GETRFINFO,WBID);
				}
			}
			#if 0
			if(wbmd_id_check(WBID) == WBMD_TRUE){
			    names[current_name++] = WBRIDGE_ETH_MAC_OID;
			    SOCK_STARTUP;
				ss = snmp_open(&(wBridge[WBID]->session));
			    if (ss == NULL) {
					wbmd_syslog_info("11111111111111wbid %d get info\n",WBID);
			        snmp_sess_perror("snmpget", &(wBridge[WBID]->session));
			        SOCK_CLEANUP;
			        continue;
			    }
			    pdu = snmp_pdu_create(SNMP_MSG_GET);
			    for (count = 0; count < current_name; count++) {
			        name_length = MAX_OID_LEN;
			        if (!snmp_parse_oid(names[count], name, &name_length)) {
			            snmp_perror(names[count]);
			            failures++;
			        } else{
						wbmd_syslog_info("name:%x%x%x%x%x%x\n",name[0],name[1],name[2],name[3],name[4],name[5]);			        	
    			        snmp_add_null_var(pdu, name, name_length);
					}
			    }
			    if (failures) {
			        snmp_close(ss);
			        SOCK_CLEANUP;
					wbmd_syslog_info("2222222222222222wbid %d get info\n",WBID);
					continue;
			    }
			  retry:
			    status = snmp_synch_response(ss, pdu, &response);
			    if (status == STAT_SUCCESS) {
					wbmd_syslog_info("333333333333333wbid %d get info\n",WBID);
			        if (response->errstat == SNMP_ERR_NOERROR) {
			            for (vars = response->variables; vars;
			                 vars = vars->next_variable){
			                print_variable(vars->name, vars->name_length, vars);
							wbmd_syslog_info("get eth mac %s\n",vars->val.string);
							 }

			        } else {
						wbmd_syslog_info("wbid %d get info\n",WBID);
			            fprintf(stderr, "Error in packet\nReason: %s\n",
			                    snmp_errstring(response->errstat));
			            if (response->errindex != 0) {
			                fprintf(stderr, "Failed object: ");
			                for (count = 1, vars = response->variables;
			                     vars && count != response->errindex;
			                     vars = vars->next_variable, count++)
			                    /*EMPTY*/;
			                if (vars) {
								wbmd_syslog_info("get eth mac %s\n",vars->val.string);
			                    fprint_objid(stderr, vars->name, vars->name_length);
							}
			                fprintf(stderr, "\n");
			            }
			            exitval = 2;
			            if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
								0)) {
			                pdu = snmp_fix_pdu(response, SNMP_MSG_GET);
			                snmp_free_pdu(response);
			                response = NULL;
			                if (pdu != NULL) {
			                    goto retry;
							}
			            }
			        }

			    } else if (status == STAT_TIMEOUT) {
					wbmd_syslog_info("44444444444444wbid %d get info\n",WBID);
			        fprintf(stderr, "Timeout: No Response from %s.\n",
			                wBridge[WBID]->session.peername);
			        exitval = 1;
			    } else {
					wbmd_syslog_info("5555555555555wbid %d get info\n",WBID);
			        snmp_sess_perror("snmpget", ss);
			        exitval = 1;

			    }
			    if (response)
			        snmp_free_pdu(response);
			    snmp_close(ss);
			    SOCK_CLEANUP;				
				WBMDTimerRequest(10,&(wBridge[WBID]->GetInfoTimerID),WBMD_GETINFO,WBID);
			}
			#endif
		}
	}
	return NULL;
}

