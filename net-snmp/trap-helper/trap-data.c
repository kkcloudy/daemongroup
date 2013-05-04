/* trap-data.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "trap-util.h" //must include <stdarg.h> before
#include "nm_list.h"
#include "hashtable.h"
#include "trap-list.h"
#include "trap-descr.h"
#include "trap-receiver.h"
#include "trap-def.h"
#include "trap-data.h"

oid objid_sysuptime[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
oid objid_snmptrap[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};

#if 0
int
snmp_input(int operation,
           netsnmp_session * session,
           int reqid, netsnmp_pdu *pdu, void *magic)
{
    return 1;
}
#endif

TrapData *trap_data_new(const char *full_oid)
{
	if (NULL == full_oid)
		return NULL;

	TrapData *tData = NULL;
	
	tData = malloc(sizeof(*tData));
	if (NULL == tData)
		return NULL;

	memset(tData, 0, sizeof(*tData));
	tData->oid = strdup(full_oid);
	trap_list_init(&tData->paramList);
	
	return tData;
}

TrapData *trap_data_new_from_descr(TrapDescr *tDescr)
{
	if (NULL == tDescr)
		return NULL;

	TrapData *tData = NULL;
	int len = 0;
	char *full_oid;

	len += strlen(TRAP_BEFORE_ENTERPRISES_OID);
	len += strlen(gSysInfo.enterprise_oid);
	len += strlen(gSysInfo.product_oid);
	if (TRAP_SRC_AC == tDescr->event_source)
		len += strlen(TRAP_AC_OID);
	else
		len += strlen(TRAP_AP_OID);
	len += strlen(tDescr->trap_oid);

	full_oid = malloc(len+1);
	memset(full_oid, 0, len+1);
	snprintf(full_oid, len+1, "%s%s%s%s%s", TRAP_BEFORE_ENTERPRISES_OID,
			gSysInfo.enterprise_oid, gSysInfo.product_oid, 
			TRAP_SRC_AC == tDescr->event_source?TRAP_AC_OID:TRAP_AP_OID,
			tDescr->trap_oid);

	tData = trap_data_new(full_oid);

	free(full_oid);

	return tData;
	
}

TrapParam *trap_param_new(const char *full_oid,
								char type,
								const char *value)
{
	if (NULL == full_oid || NULL == value)
		return NULL;

	TrapParam *tParam = NULL;

	tParam = malloc(sizeof(*tParam));
	if (NULL == tParam)
		return NULL;

	memset(tParam, 0, sizeof(*tParam));
	tParam->oid = strdup(full_oid);
	tParam->type = type;
	tParam->value = strdup(value);

	return tParam;
}

void trap_param_free(TrapParam *tParam)
{
	if (NULL != tParam){
		if (NULL != tParam->oid)
			free(tParam->oid);
		if (NULL != tParam->value)
			free(tParam->value);

		free(tParam);
	}
}

void trap_data_append_param(TrapData *tData,
								const char *full_oid,
								char type,
								const char *value)
{
	TrapParam *tParam = NULL;

	tParam = trap_param_new(full_oid, type, value);
	
	trap_list_append(&tData->paramList, tParam);
}


void trap_data_append_param_str(TrapData *tData,
								const char *format, ...)
{
	char buf[1024];
	char *full_oid, *type, *value;

	va_list args;
	va_start (args, format);

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%s%s%s", TRAP_BEFORE_ENTERPRISES_OID,
			gSysInfo.enterprise_oid, gSysInfo.product_oid);
	vsnprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), format, args);

	va_end (args);
		
	full_oid = strtok(buf, " \t");
	if (NULL == full_oid)
		return;

	type = strtok(NULL, " \t");
	if (NULL == type)
		return;

	value = strtok(NULL, " \t");
	if (NULL == value)
		return;

	trap_data_append_param(tData, full_oid, type[0], value);
}

void trap_data_append_common_param(TrapData *tData, TrapDescr *tDescr)
{
	if (NULL == tData || NULL == tDescr)
		return;

	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_FREQUENCY_OID, tDescr->frequency);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TYPE_OID, tDescr->trap_type);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_LEVEL_OID, tDescr->trap_level);
	char time_str[32];
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_EVENT_TIME_OID, 
								trap_get_time_str(time_str, sizeof(time_str)));
	trap_data_append_param_str(tData, "%s%s s %d", TRAP_AC_OID, TRAP_STATUS_OID, tDescr->trap_status);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_TITLE_OID, tDescr->title);
	trap_data_append_param_str(tData, "%s%s s %s", TRAP_AC_OID, TRAP_CONTENT_OID, tDescr->content);
}

void trap_data_destroy(TrapData *tData)
{
	if (NULL != tData){
		trap_list_destroy(&tData->paramList, (TrapFreeFunc)trap_param_free);
		if (NULL != tData->oid)
			free(tData->oid);
		free(tData);
	}
	trap_syslog(LOG_DEBUG,"trap_data_destroy(tData)\n");
}

int trap_send_v1(TrapReceiver *tRcv, TrapData *tData)
{
	netsnmp_pdu *pdu=NULL;
	oid name[MAX_OID_LEN];
	size_t name_length;
	in_addr_t *pdu_in_addr_t=NULL;
	int ret = 0;
	TrapNode *tNode=NULL;
	TrapParam *tParam=NULL;


	pdu = snmp_pdu_create(SNMP_MSG_TRAP);
	if (NULL == pdu) {
		trap_syslog(LOG_WARNING, "Failed to create snmp pdu: snmp_msg_trap\n");
		SOCK_CLEANUP;
		return 1;
	}

	name_length = MAX_OID_LEN;
	if (!snmp_parse_oid(tData->oid, name, &name_length)) {
		trap_syslog(LOG_WARNING, "Failed to parse oid: %s\n", tData->oid);
		SOCK_CLEANUP;
		snmp_free_pdu(pdu);
		return 1;
	}
	pdu->enterprise = (oid *)malloc(name_length * sizeof(oid));
	memcpy(pdu->enterprise, name, name_length * sizeof(oid));
	pdu->enterprise_length = name_length;

	pdu_in_addr_t = (in_addr_t *)pdu->agent_addr;
	*pdu_in_addr_t = get_myaddr();

	pdu->trap_type = 6;
	pdu->specific_type = 17;
	pdu->time = get_uptime();

	for (tNode = tData->paramList.first; tNode; tNode = tNode->next) {
		tParam = tNode->data;
		name_length = MAX_OID_LEN;
		if (!snmp_parse_oid(tParam->oid, name, &name_length)) {
			trap_syslog(LOG_WARNING, "Failed to parse oid: %s\n", tParam->oid);
			SOCK_CLEANUP;
			snmp_free_pdu(pdu);
			return 1;
		}
		if (snmp_add_var(pdu, name, name_length, tParam->type, tParam->value) != 0) {
			trap_syslog(LOG_WARNING, "Failed to add var: %c, %s\n", tParam->type, tParam->value);
			SOCK_CLEANUP;
			snmp_free_pdu(pdu);
			return 1;
		}
	}
	
	if (snmp_send(tRcv->ss, pdu) == 0) {
        trap_syslog(LOG_WARNING, "Failed to send trap pdu\n");
		snmp_free_pdu(pdu);
		ret = 1;
	}

    return ret;
}

int trap_send_v2(TrapReceiver *tRcv, TrapData *tData)
{
	netsnmp_pdu *pdu=NULL;
	oid name[MAX_OID_LEN];
	size_t name_length;
	long sysuptime;
	char csysuptime[20];
	int ret = 0;
	TrapNode *tNode=NULL;
	TrapParam *tParam=NULL;

	pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
	if (NULL == pdu) {
		trap_syslog(LOG_WARNING, "Failed to create snmp pdu: snmp_msg_trap2\n");
		SOCK_CLEANUP;
		return 1;
	}

	sysuptime = get_uptime();
	sprintf(csysuptime, "%ld", sysuptime);
	if (snmp_add_var(pdu, objid_sysuptime, 
					sizeof(objid_sysuptime)/sizeof(oid), 't', csysuptime) != 0) {
		trap_syslog(LOG_WARNING, "Failed to add var: %c %s\n", 't', csysuptime);
		SOCK_CLEANUP;
		snmp_free_pdu(pdu);
		return 1;
	}

	if (snmp_add_var(pdu, objid_snmptrap,
					sizeof(objid_snmptrap)/sizeof(oid), 'o', tData->oid) != 0) {
		trap_syslog(LOG_WARNING, "Failed to add var: %c %s\n", 'o', tData->oid);
		SOCK_CLEANUP;
		snmp_free_pdu(pdu);
		return 1;
	}

	for (tNode = tData->paramList.first; tNode; tNode = tNode->next) {
		tParam = tNode->data;
		name_length = MAX_OID_LEN;
		if (snmp_parse_oid(tParam->oid, name, &name_length) == 0) {
			trap_syslog(LOG_WARNING, "Failed to parse oid: %s\n", tParam->oid);
			SOCK_CLEANUP;
			snmp_free_pdu(pdu);
			return 1;
		}
		
		if (snmp_add_var(pdu, name, name_length, tParam->type, tParam->value) != 0) {
			trap_syslog(LOG_WARNING, "Failed to add var: %c %s\n", tParam->type, tParam->value);
			SOCK_CLEANUP;
			snmp_free_pdu(pdu);
			return 1;
		}
	}


	if( NULL != tRcv->ss ){
		if (snmp_send(tRcv->ss, pdu) == 0) {
			trap_syslog(LOG_WARNING, "Failed to send trap pdu\n");
			snmp_free_pdu(pdu);
			ret = 1;
		}
	}
	
	SOCK_CLEANUP;

	return ret;
}

int trap_send_v3(TrapReceiver *tRcv, TrapList *tV3UserList, TrapData *tData)
{
	#if 0
	struct arg arg;
	TrapNode *tV3UserNode, *tParamNode;
	TrapParam *tParam;
	TrapV3User *tV3User;

	for (tV3UserNode = tV3UserList->first; tV3UserNode; tV3UserNode = tV3UserNode->next) {
		tV3User = tV3UserNode->data;
		arg_init(&arg, 100, 20);
		arg_append(&arg, "snmptrap");
		arg_append(&arg, "-v");
		arg_append(&arg, "3");
		arg_append(&arg, "-u");
		arg_append(&arg, tV3User->username);
		if (strlen(tV3User->auth_type) > 0 && strlen(tV3User->auth_passwd) > 0){
			arg_append(&arg, "-a");
			arg_append(&arg, tV3User->auth_type);
			arg_append(&arg, "-A");
			arg_append(&arg, tV3User->auth_passwd);
		}
		if (strlen(tV3User->priv_type) > 0 && strlen(tV3User->priv_passwd) > 0){
			arg_append(&arg, "-x");
			arg_append(&arg, tV3User->priv_type);
			arg_append(&arg, "-X");
			arg_append(&arg, tV3User->priv_passwd);
		}
		char addr[100];
		snprintf(addr, sizeof(addr), "%s:%hd", tRcv->rcv_ip, 162);
		arg_append(&arg, addr);
		
		char tm[100];
		snprintf(tm, sizeof(tm), "%ul", time(0)*100);
		arg_append(&arg, tm);
		arg_append(&arg, tData->oid);

		for (tParamNode = tData->paramList.first; tParamNode; tParamNode = tParamNode->next){
			tParam = tParamNode->data;
			arg_append(&arg, tParam->oid);
			char buf[10];
			sprintf(buf, "%c", tParam->type);
			arg_append(&arg, buf);
			arg_append(&arg, tParam->value);
		}
	
		trap_exec(arg.argc, arg.argv);

		arg_destroy(&arg);
	}
	#endif
	return 0;
}


void trap_send(TrapList *tRcvList, TrapList *tV3UserList, TrapData *tData)
{
	TrapReceiver *tRcv = NULL;
	TrapNode *tNode = NULL;
	int version=0;
    
	if ( NULL == tRcvList || NULL == tV3UserList || NULL == tData ) {
		return;
	}

	for (tNode = tRcvList->first; tNode; tNode = tNode->next) {
		tRcv = tNode->data;

		if (NULL != tRcv) {
		    int ret = 0;
			if (SNMP_VERSION_1 == tRcv->version){
				ret = trap_send_v1(tRcv, tData);
				version = V1;
			}
			else if (SNMP_VERSION_2c == tRcv->version){
				ret = trap_send_v2(tRcv, tData);
				version = V2;
			}
			else if (SNMP_VERSION_3 == tRcv->version){
				ret = trap_send_v3(tRcv, tV3UserList, tData);
				version = V3;
			}			

            if(0 == ret) {
				trap_syslog(LOG_INFO, "send trap: instance id = %d-%d, trap version = %d, ip = %s, port = %d, trap_oid = %s\n",
										tRcv->local_id, tRcv->instance_id, version, 
										tRcv->dest_ipAddr, tRcv->dest_port ? tRcv->dest_port : 162,tData->oid);
				
                trap_heartbeat_update_last_time(&gHeartbeatInfo[tRcv->local_id][tRcv->instance_id], 0);
            }
		}
	}

}


#if 0
void trap_data_test(void)
{
	trap_receiver_list_parse(&gReceiverList);
	trap_v3user_list_parse(&gV3UserList);
	trap_system_info_get(&gSysInfo);

	TrapData *tData = NULL;
	TrapDescr *tDescr = NULL;

	tDescr = trap_descr_new("acCPUUtilizationOverThresholdTrap", 0, 
									".1.6", "device", "major",
									"ac CPU Over Threshold", "ac CPU Over Threshold");
	
	tData = trap_data_new_from_descr(tDescr);
	trap_data_append_param_str(tData, "%s x %02X%02X%02X%02X%02X%02X", EI_MAC_TRAP_DES,
						gSysInfo.ac_mac[0], gSysInfo.ac_mac[1], gSysInfo.ac_mac[2], 
						gSysInfo.ac_mac[3], gSysInfo.ac_mac[4], gSysInfo.ac_mac[5]);
	trap_data_append_param_str(tData, "%s s %s", AC_NET_ELEMENT_CODE_OID, gSysInfo.hostname);
	trap_data_append_common_param(tData, tDescr);
	trap_send(&gReceiverList, &gV3UserList, tData);
		
	trap_data_destroy(tData);
}
#endif
