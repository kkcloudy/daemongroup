/* trap-helper.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <dbus/dbus.h>
#include <linux/kernel.h>
#include <mcheck.h>     

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "ws_dbus_def.h"
#include "ws_init_dbus.h"
#include "ws_snmpd_engine.h"
#include "ws_dcli_vrrp.h"
#include "dbus/npd/npd_dbus_def.h"
#include "board/board_define.h"
#include "trap-util.h"
#include "trap-def.h"
#include "nm_list.h"
#include "trap-list.h"
#include "hashtable.h"
#include "trap-signal.h"
#include "trap-descr.h"
#include "trap-data.h"
#include "trap-resend.h"
#include "trap-hash.h"
#include "trap-receiver.h"
#include "trap-signal-handle.h"
#include "trap-instance.h"
#include "trap-dbus.h"

int ac_heartbeat_send_func(HeartbeatInfo *info);
//int ac_system_start_func(void);
int ac_config_parse_fail_func(void);
int get_extra_switch_by_vrrp(void);

void trap_descr_list_debug(TrapList *tDescrList);
void trap_helper_set_started_flag(void);


#define TRAP_PID_FILE	"/var/run/trap-helper.pid"

static void signal_handler (int sig)
{
	switch (sig){
		case SIGUSR2:
		case SIGTERM:
			trap_syslog(LOG_DEBUG, "process exit, received signal %d\n", sig);
			gGlobalInfo.keep_loop = 0;
			//exit(0);
			break;
	}			
}

void trap_init_global(Global *global)
{
	TRAP_TRACE_LOG(LOG_DEBUG,"entry.\n");
	int hash_num=129;
	memset(global,0,sizeof(Global));
	global->gDescrList_hash = NULL;
	global->gSignalList_hash = NULL;
	global->gProxyList_hash = NULL;
	trap_init_hashtable(&(global->gDescrList_hash),hash_num, &hash_value_count);
	trap_init_hashtable(&(global->gSignalList_hash),hash_num, &hash_value_count);
	trap_init_hashtable(&(global->gProxyList_hash), hash_num, &hash_value_count);
	trap_descr_register_all(&(global->gDescrList), global->gDescrList_hash);
	trap_descr_load_switch(&(global->gDescrList));
	trap_signal_register_all(&(global->gSignalList), global->gSignalList_hash);
	trap_signal_register_proxy(&(global->gProxyList), global->gProxyList_hash);
	trap_init_ac_array (&(global->actrap), MAXACTRAPNUM);
	//trap_init_wtp_array (&(global->wtp[0]), MAXWTPID);
	//trap_init_instance_array(&(global->Ins_trap[0]));
}

static
void update_trap_heartbeat(void) {
    
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
            unsigned int instance_id = gInsVrrpState.instance_master[i][j];            
            if(gHeartbeatInfo[i][instance_id].ac_ip[0] && trap_heartbeat_is_demanded(&gHeartbeatInfo[i][instance_id])) {
                ac_heartbeat_send_func(&gHeartbeatInfo[i][instance_id]);
                trap_heartbeat_update_last_time(&gHeartbeatInfo[i][instance_id], 1);
            }
        }
    }  

    return ;
}

int main(int argc, char *argv[])
{
	#if 1
	//trap_become_daemon();
	trap_openlog();
	snmp_debug_init();//for snmp debug informations
    
	ccgi_local_dbus_init();
	    
	trap_enable_debug();
	trap_write_pid_file(TRAP_PID_FILE);

	trap_set_system_signal_handler(SIGUSR2, signal_handler);
	trap_set_system_signal_handler(SIGTERM, signal_handler);
    
	trap_wait_ac_configure_down(10);
	
	trap_init_global(&global);
	trap_system_info_get(&gSysInfo);
	
#if 0

	trap_signal_register_all(&gSignalList);
	trap_descr_register_all(&gDescrList);
	trap_descr_load_switch(&gDescrList);
#endif 

	trap_dbus_init();
	trap_dbus_add_rules();

	trap_init_tipc_connection();

	trap_init_instance_info(&gInsVrrpState); //changed for instance receive list 2010-12-17
	//trap_show_instance_info(&gInsVrrpState);//for test 
	//trap_v3user_list_parse(&gV3UserList);

	trap_init_resend(&gTrapResendList );

	trap_descr_list_debug(&(global.gDescrList));	
	
	trap_syslog(LOG_INFO, "**********trap-helper begin working**********\n");
	if (!trap_helper_is_started()) {
		//trap_syslog(LOG_DEBUG, "**********sleep 10 seconds**********\n");
		//sleep(10); 
		trap_helper_set_started_flag();
	}

    //ac_system_start_func();
    ac_config_parse_fail_func();
	
	gGlobalInfo.keep_loop = 1;
	
	while (gGlobalInfo.keep_loop) 
	{
		trap_dbus_dispatch (500);

		update_trap_heartbeat();
		
		if( trap_resend_enabled(&gTrapResendList))
		{
			trap_resend(&gTrapResendList);
			trap_syslog(LOG_DEBUG,"trap resend enabled!\n");
		}else 
		{
			trap_syslog(LOG_DEBUG,"trap resend disabled!\n");
		}
		
	}


	trap_signal_list_destroy(&gSignalList);
	trap_descr_list_destroy(&gDescrList);
	trap_receiver_list_destroy(&gReceiverList);
	trap_v3user_list_destroy(&gV3UserList);
	#endif
	
	#if 0
	trap_util_test();
	trap_receiver_test();
	trap_signal_test();
	trap_descr_test();
	trap_data_test();
	#endif
	
	return 0;
}

#if 0
int ac_heartbeat_send_func(void)
{
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	
	tDescr = trap_descr_list_get_item(&gDescrList, acHeartTimePackageTrap);

	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	tDescr->frequency++;
	tData = trap_data_new_from_descr(tDescr);

	char mac_str[MAC_STR_LEN];
	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);
	
	char mac_oid[MAX_MAC_OID];
	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

	trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
	trap_data_append_common_param(tData, tDescr);
	
	trap_send(&gReceiverList, &gV3UserList, tData);
	
	trap_data_destroy(tData);

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
#endif

int ac_heartbeat_send_func(HeartbeatInfo *info)
{
	TrapDescr *tDescr = NULL;
    trap_syslog(LOG_DEBUG, "enter ac_heartbeat_send_func\n");
    
	if (!trap_is_ac_trap_enabled(&gInsVrrpState)){
		return TRAP_SIGNAL_HANDLE_AC_IS_BACKUP;
	}
        
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, acHeartTimePackageTrap);
	if(NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	tDescr->frequency++;

    TrapData *tData = trap_data_new_from_descr(tDescr);
    if(NULL == tData) {
        trap_syslog(LOG_INFO, "ac_heartbeat_send_func: trap_data_new_from_descr malloc tData error!\n");
        return TRAP_SIGNAL_HANDLE_GET_DESCR_ERROR;
    }
	
    char mac_str[MAC_STR_LEN];
    get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);
    
    char mac_oid[MAX_MAC_OID];
    get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);
    
    trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
    trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
    trap_data_append_param_str(tData, "%s s %s",TRAP_AC_IP_OID, info->ac_ip); 
    trap_data_append_common_param(tData, tDescr);
    
    trap_send(gInsVrrpState.instance[info->local_id][info->instance_id].receivelist, &gV3UserList, tData);

    trap_data_destroy(tData);
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

#if 0
int ac_start_trap_has_sent(void)
{
	return access("/var/run/sendstarttrap_flag", 0) == 0;
}

// 0 cold, 1 reboot
int ac_start_trap_get_type(void)
{
	FILE *fp = NULL;
	char buf[64];
	
	if ( (fp = fopen("/var/run/softreboot", "r")) == NULL)
		return 0;

	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);
		return 0;
	}
	fclose(fp);

	return strncmp(buf, "1", strlen("1")) == 0;
}

void ac_start_trap_set_sent_flag(int start_type)
{
	FILE *fp = NULL;

	if ( (fp = fopen("/var/run/sendstarttrap_flag", "w")) != NULL) {
		fprintf(fp, "%d", start_type);
		fclose(fp);
	}
}

int ac_system_start_func(void)
{
	TrapDescr *tDescr = NULL;
	int start_type = 0;
	int instance_id = 0;

	if(1 == ac_trap_get_flag("/var/run/ac_restart_trap_flag"))
	{
		trap_syslog(LOG_INFO,"ac_system_start\n");
		return -1;
	}
	
	if (ac_start_trap_has_sent()) {
		return -1;
    }
    
#if 1
	do {
		struct sysinfo info;
    	sysinfo(&info);
		//fprintf(stderr, "uptime=%ld\n", info.uptime);
		if (info.uptime > 30*60)
			return -1;
	} while (0);
#endif

	start_type = ac_start_trap_get_type();
	
    ac_start_trap_set_sent_flag(start_type);

	if (0 == start_type)
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acSysColdStartTrap);
	else if (1 == start_type)
		tDescr = trap_descr_list_get_item(global.gDescrList_hash, acSystemRebootTrap);

	if (!trap_is_ac_trap_enabled(&gInsVrrpState)){
		return TRAP_SIGNAL_HANDLE_AC_IS_BACKUP;
	}
	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	tDescr->frequency++;

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_system_start_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}
        	
        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);
        	
        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
        	trap_data_append_common_param(tData, tDescr);
        	
            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
    	}
    }	

	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}
#endif

int ac_config_parse_is_failed(void)
{
	FILE *fp = NULL;
	char buf[64];
	
	if ( (fp = fopen("/var/run/conf.trap", "r")) == NULL)
		return 0;
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);
		return 0;
	}
	fclose(fp);

	return strncmp(buf, "0", strlen("0")) != 0;
}

int ac_config_parse_fail_func(void)
{
	TrapDescr *tDescr = NULL;
	TrapData *tData = NULL;
	unsigned int is_active_master = 0;
	
	if (!ac_config_parse_is_failed())
		return -1;

	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		is_active_master = get_product_info(DISTRIBUTED_ACTIVE_MASTER_FILE);
	}
	
	if(IS_NOT_ACTIVE_MASTER == is_active_master)
	{
		trap_syslog(LOG_INFO, "return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER in ac_config_parse_fail_func fail\n");
		return TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER;
	}

    if (!trap_is_ac_trap_enabled(&gInsVrrpState)){
		return TRAP_SIGNAL_HANDLE_AC_IS_BACKUP;
	}
    
	tDescr = trap_descr_list_get_item(global.gDescrList_hash, acConfigurationErrorTrap);
	
	if (NULL == tDescr || 0 == tDescr->switch_status)
		return TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF;

	tDescr->frequency++;

	TRAP_SIGNAL_AC_RESEND_UPPER_MACRO(tDescr);
	
    int i = 0, j = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        for(j = 0; j < INSTANCE_NUM && gInsVrrpState.instance_master[i][j]; j++) {  
        	TrapData *tData = trap_data_new_from_descr(tDescr);
        	if(NULL == tData) {
                trap_syslog(LOG_INFO, "ac_config_parse_fail_func: trap_data_new_from_descr malloc tData error!\n");
                continue;
        	}

        	char mac_str[MAC_STR_LEN];
        	get_ac_mac_str(mac_str, sizeof(mac_str), gSysInfo.ac_mac);
        	
        	char mac_oid[MAX_MAC_OID];
        	get_ac_mac_oid(mac_oid, sizeof(mac_oid), mac_str);

        	trap_data_append_param_str(tData, "%s s %s", EI_AC_MAC_TRAP_DES, mac_str);
        	trap_data_append_param_str(tData, "%s%s s %s", AC_NET_ELEMENT_CODE_OID, mac_oid, gSysInfo.hostname);
        	trap_data_append_common_param(tData, tDescr);

            trap_send(gInsVrrpState.instance[i][gInsVrrpState.instance_master[i][j]].receivelist, &gV3UserList, tData);
            TRAP_SIGNAL_AC_RESEND_LOWER_MACRO(tDescr, tData, i, gInsVrrpState.instance_master[i][j]); 
        }
    }
    
	return TRAP_SIGNAL_HANDLE_SEND_TRAP_OK;
}

int get_extra_switch_by_vrrp(void)
{
	int i, count = 0, ret = -1;
	Z_VRRP zvrrp;
	memset(&zvrrp, 0, sizeof(zvrrp)); 

	for (i = 1; i < 17; i++){
		memset(&zvrrp, 0, sizeof(zvrrp));
		ret = ccgi_show_hansi_profile(&zvrrp, i);
		if (DCLI_VRRP_RETURN_CODE_OK == ret) {
			count++;
			if (strcmp(zvrrp.state, "MASTER") == 0)
				return 1;
		}
	}
	
	return 0 == count;
}

void trap_descr_list_debug(TrapList *tDescrList)
{
	TrapNode *tNode;
	TrapDescr *tDescr;
	
	for (tNode = tDescrList->first; tNode; tNode = tNode->next) {
		tDescr = tNode->data;
		trap_syslog(LOG_DEBUG, "trap_name=%s, event_source=%d, trap_oid=%s, switch_status=%d, "
			"trap_type=%s, trap_level=%s, title=%s, content=%s\n", 
			tDescr->trap_name, tDescr->event_source, tDescr->trap_oid, tDescr->switch_status,
			tDescr->trap_type, tDescr->trap_level, tDescr->title, tDescr->content);
	}
}

int trap_helper_is_started(void)
{
	return access("/var/run/trap_start_flag", 0) == 0;	
}

void trap_helper_set_started_flag(void)
{	
	FILE *fp = NULL;

	if ( (fp = fopen("/var/run/trap_start_flag", "w")) == NULL)
		return;
	fprintf(fp, "%d", 1);
	fclose(fp);
}
