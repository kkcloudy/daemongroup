#include <stdio.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "board/board_define.h"
#include "trap-util.h"
#include "trap-list.h"
#include "trap-def.h"
#include "trap-receiver.h"
#include "nm_list.h"
#include "hashtable.h"
#include "trap-descr.h"
#include "trap-data.h"
#include "trap-resend.h"
#include "ws_dcli_vrrp.h"
#include "trap-instance.h"
#include "ws_dbus_def.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "hmd/hmdpub.h"
#include "ws_local_hansi.h"
#include "ws_acinfo.h"

TrapInsVrrpState gInsVrrpState = {0};

#if 0
int trap_init_vrrp_state(TrapInsVrrpState *ins_vrrp_state)
{
	int ret = 0, i;
	int hansi_state=0;
	int ac_vrrp_statistic=0;
	
	if (NULL == ins_vrrp_state)
		return 0;
	
	for (i=1; i<=16; i++)
	{
		ret = snmp_get_vrrp_state(i,&hansi_state);
		
		if (-1 == ret ||0 == ret )
		{
			
			0 == ret? trap_syslog(LOG_DEBUG,"get_extra_switch_by_vrrp: snmp_get_vrrp_state instance failed!")\
				: trap_syslog (LOG_DEBUG,"get_extra_switch_by_vrrp: snmp_get_vrrp_state instance disabled!"),ac_vrrp_statistic++;
			continue;
		}
		
		trap_set_vrrp_state(&gInsVrrpState, i, hansi_state);
		
		trap_syslog(LOG_INFO, "instance_id=%d, state=%d\n", i, ins_vrrp_state->instance[i].vrrp_state);
	}
	ins_vrrp_state->is_ac_trap_enabled=AC_IS_MASTER; //for ac trap normally send but there is no ac backup state
/*	if (16==ac_vrrp_statistic)
	{
		vrrpState->is_ac_trap_enabled=1;
		trap_syslog(LOG_DEBUG,"no instance is configured ac is main");
	}
*/
	return 0;
}
#endif

int is_ac_trap (int flag)
{
	if (1 == flag)	
		return 1;
	
	return 0;
}

int trap_renew_master_instance(TrapInsVrrpState *ins_vrrp_state)
{
    memset(ins_vrrp_state->instance_master, 0, sizeof(ins_vrrp_state->instance_master));
    
	int i = 0, j = 0;;
	for(i = 0; i < VRRP_TYPE_NUM; i++) {
        unsigned int index = 0;
    	for(j = 1; j <= INSTANCE_NUM; j++ ) {
    		if(1 == ins_vrrp_state->instance[i][j].vrrp_state) {
                ins_vrrp_state->instance_master[i][index++] = j;
    		}
    	}
    }	
		
	return 0;
}

int
trap_is_ac_trap_enabled(TrapInsVrrpState *ins_vrrp_state) {
    if(NULL == ins_vrrp_state) {
        return AC_IS_BACKUP;
    }

    int i = 0;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        if(ins_vrrp_state->instance_master[i][0]) {
            return AC_IS_MASTER;
        }
    }
    
    return AC_IS_BACKUP;
}

int trap_is_ap_trap_enabled(TrapInsVrrpState *ins_vrrp_state, unsigned int local_id,unsigned int instance_id)
{
	if (NULL == ins_vrrp_state || 0 == instance_id ||  instance_id > 16 || local_id > 1)
		return INSTANCE_BACKUP;

	return 1 == ins_vrrp_state->instance[local_id][instance_id].vrrp_state;
}

int trap_set_instance_vrrp_state(TrapInsVrrpState *ins_vrrp_state, unsigned int local_id, unsigned int instance_id, int state)
{
	if (NULL == ins_vrrp_state || 0 == instance_id ||  instance_id > 16 || local_id > 1)
		return 1;

	ins_vrrp_state->instance[local_id][instance_id].vrrp_state = state;

	trap_renew_master_instance(ins_vrrp_state);
	
	return 0;
}

int trap_get_instance_vrrp_state(TrapInsVrrpState *ins_vrrp_state, unsigned int local_id, unsigned int instance_id)
{
	if(NULL == ins_vrrp_state || 0 == instance_id ||  instance_id > 16 || local_id > 1)
		return 0;
		
	return ins_vrrp_state->instance[local_id][instance_id].vrrp_state;
}

int trap_instance_is_master(TrapInsVrrpState *ins_vrrp_state, unsigned int local_id, unsigned int instance_id)
{		
	if(NULL == ins_vrrp_state || 0 == instance_id || instance_id > 16 || local_id > 1)
		return 0;
	
	return 1 == ins_vrrp_state->instance[local_id][instance_id].vrrp_state;
}

int trap_get_instance_info( Z_VRRP *zvrrp, unsigned int instance_id )
{
	int ret = 0;
	memset(zvrrp, 0, sizeof(zvrrp)); 

	ret = ccgi_show_hansi_profile(zvrrp, instance_id );
	if ( DCLI_VRRP_RETURN_CODE_OK == ret ) {
		goto OUT; 
	}else {
		trap_syslog(LOG_INFO,"ccgi_show_hansi_profile error ret=%d\n", ret);
		goto ERROR;
	}
	
OUT:
	return 1;
ERROR:
	return 0;
}

#if 0
int trap_clear_local_ip() //we must clear local ip first then set local ip
{
	char *local_ip=NULL;
	local_ip = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CLIENT_ADDR); //add local bind ip and port
	if ( NULL == local_ip ){
		return 0;
	}else {
		free(local_ip);
		netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CLIENT_ADDR, NULL); 
	}
	return 0;
}

int trap_set_local_ip(char *localname) //we must clear local ip first then set local ip strdup localname for free netsnmp_ds_get_string
{
	char *local_ip=NULL;
	char *tmp_localname=NULL;
	local_ip = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CLIENT_ADDR); //add local bind ip and port
	if( NULL == local_ip ){
		tmp_localname=strdup(localname);
		if ( NULL != tmp_localname ){
			netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CLIENT_ADDR, tmp_localname); //add local bind ip and port
		}else {
			trap_syslog(LOG_INFO,"trap_set_local_ip: strdup(localname) error!");
		}
	}
	trap_syslog(LOG_DEBUG,"netsnmp_ds_get_string(%s) netsnmp_ds_set_string(%s)", local_ip, tmp_localname);
	return 0;
}
#endif

int trap_init_instance_info ( TrapInsVrrpState *ins_vrrp_state) //add for multi instance init snmpssion receive_list and bind local instance socket
{
	Z_VRRP zvrrp;
	TrapNode *tNode = NULL;
	TrapReceiver *tRcv = NULL;

	if (NULL == ins_vrrp_state){
		trap_syslog(LOG_INFO,"ins_vrrp_state is NULL");
		return -1;
	}
    
	memset(ins_vrrp_state, 0, sizeof(TrapInsVrrpState));

    /*  trap get local slot id */
    unsigned int local_slotID = 0;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slotID = trap_get_product_info(PRODUCT_LOCAL_SLOTID);
	}
    if(0 == local_slotID) {
        syslog(LOG_DEBUG, "trap_init_instance_info: trap get local slot no error!\n");
        return -1;
    }
    


    /*  init hansi instance info */
    int inst_ret = 0;
    struct Hmd_Board_Info_show *instance_head = NULL;
    trap_syslog(LOG_INFO, "before show_broad_instance_info\n");
    inst_ret = show_broad_instance_info(ccgi_dbus_connection, &instance_head);
    trap_syslog(LOG_INFO, "after show_broad_instance_info, inst_ret = %d\n", inst_ret);

    if(1 == inst_ret && instance_head) {
    
        struct Hmd_Board_Info_show *instance_node = NULL;
        for(instance_node = instance_head->hmd_board_list; NULL != instance_node; instance_node = instance_node->next) {

            if(local_slotID != instance_node->slot_no) {
                continue;
            }

            int i = 0;
            for(i = 0; i < instance_node->InstNum; i++) {
                if(0 == instance_node->Hmd_Inst[i]->Inst_ID || instance_node->Hmd_Inst[i]->Inst_ID > 16) {
                    trap_syslog(LOG_INFO, "The remote instance id(%d) is not right!\n", instance_node->Hmd_Inst[i]->Inst_ID);
                    continue;
                }

				/*get backup network ipaddr*/
				char instance_id[10] = { 0 };
				unsigned long ip  = 0;
				
				memset(instance_id, 0, sizeof(instance_id));
				snprintf(instance_id, sizeof(instance_id)-1, "%d-%d-%d", local_slotID, 0, instance_node->Hmd_Inst[i]->Inst_ID);
		        get_ip_by_active_instance(instance_id, &ip);
		        if(0 != ip) {
					memset(ins_vrrp_state->instance[0][instance_node->Hmd_Inst[i]->Inst_ID].backup_network_ip, 0, sizeof(ins_vrrp_state->instance[0][instance_node->Hmd_Inst[i]->Inst_ID].backup_network_ip));
					INET_NTOA(ip, ins_vrrp_state->instance[0][instance_node->Hmd_Inst[i]->Inst_ID].backup_network_ip);

					trap_syslog(LOG_INFO, "trap_init_instance_info: get instance %s backup network ip %s", 
					                        instance_id, ins_vrrp_state->instance[0][instance_node->Hmd_Inst[i]->Inst_ID].backup_network_ip);
		        }
				
                trap_syslog(LOG_INFO, "This borad remote hansi instance %d is %s\n", 
                                    instance_node->Hmd_Inst[i]->Inst_ID, 
                                    instance_node->Hmd_Inst[i]->isActive? "master" : "back");

                if(1 == instance_node->Hmd_Inst[i]->isActive) {
                    ins_vrrp_state->instance[0][instance_node->Hmd_Inst[i]->Inst_ID].vrrp_state = 1;
                }
                else {
                    ins_vrrp_state->instance[0][instance_node->Hmd_Inst[i]->Inst_ID].vrrp_state = 0;
                }
				ins_vrrp_state->instance[0][instance_node->Hmd_Inst[i]->Inst_ID].pre_vrrp_state = instance_node->Hmd_Inst[i]->InstState;
                trap_syslog(LOG_INFO, "This borad remote hansi instance %d (set)pre state is %d.\n", 
                                    instance_node->Hmd_Inst[i]->Inst_ID, 
                                    ins_vrrp_state->instance[0][instance_node->Hmd_Inst[i]->Inst_ID].pre_vrrp_state);
            }
            
            for(i = 0; i < instance_node->LocalInstNum; i++) {      
                if(0 == instance_node->Hmd_Local_Inst[i]->Inst_ID || instance_node->Hmd_Local_Inst[i]->Inst_ID > 16) {
                    trap_syslog(LOG_INFO, "The local instance id(%d) is not right!\n", instance_node->Hmd_Local_Inst[i]->Inst_ID);
                    continue;
                }

				/*get backup network ipaddr*/
				char instance_id[10] = { 0 };
				unsigned long ip  = 0;
				
				memset(instance_id, 0, sizeof(instance_id));
				snprintf(instance_id, sizeof(instance_id)-1, "%d-%d-%d", local_slotID, 1, instance_node->Hmd_Local_Inst[i]->Inst_ID);
		        get_ip_by_active_instance(instance_id, &ip);
		        if(0 != ip) {
					memset(ins_vrrp_state->instance[1][instance_node->Hmd_Local_Inst[i]->Inst_ID].backup_network_ip, 0, sizeof(ins_vrrp_state->instance[1][instance_node->Hmd_Local_Inst[i]->Inst_ID].backup_network_ip));
					INET_NTOA(ip, ins_vrrp_state->instance[1][instance_node->Hmd_Local_Inst[i]->Inst_ID].backup_network_ip);

					trap_syslog(LOG_INFO, "trap_init_instance_info: get instance %s backup network ip %s", 
					                        instance_id, ins_vrrp_state->instance[1][instance_node->Hmd_Local_Inst[i]->Inst_ID].backup_network_ip);
		        }
				
                trap_syslog(LOG_INFO, "This borad local hansi instance %d is %s\n", 
                                   instance_node->Hmd_Local_Inst[i]->Inst_ID, 
                                   instance_node->Hmd_Local_Inst[i]->isActive? "master" : "back");

                if(1 == instance_node->Hmd_Local_Inst[i]->isActive) {
                    ins_vrrp_state->instance[1][instance_node->Hmd_Local_Inst[i]->Inst_ID].vrrp_state = 1;
                }
                else {
                    ins_vrrp_state->instance[1][instance_node->Hmd_Local_Inst[i]->Inst_ID].vrrp_state = 0;
                }
				ins_vrrp_state->instance[1][instance_node->Hmd_Local_Inst[i]->Inst_ID].pre_vrrp_state = instance_node->Hmd_Local_Inst[i]->InstState;
                trap_syslog(LOG_INFO, "This borad local hansi instance %d (set)pre state is %d.\n", 
                                    instance_node->Hmd_Local_Inst[i]->Inst_ID, 
                                    ins_vrrp_state->instance[1][instance_node->Hmd_Local_Inst[i]->Inst_ID].pre_vrrp_state);
            }    
        }
    }
    free_broad_instance_info(&instance_head);
    
    /*
        *   manual set instance master
        *   instance 0 is use for test trap
        */	
    unsigned int manual_instance = 0;
    int ret = ac_manage_show_snmp_manual_instance(ccgi_dbus_connection, &manual_instance);
    trap_syslog(LOG_INFO, "manual_instance = %d\n", manual_instance);
    if(AC_MANAGE_SUCCESS == ret && manual_instance) {
        int i = 0;
        for(i = 0; i < (INSTANCE_NUM * 2); i++) {
            if(manual_instance & (0x1 << i)) {
                unsigned int local_id = i / INSTANCE_NUM;
                unsigned int instance_id = i - (local_id * INSTANCE_NUM) + 1;
                
                ins_vrrp_state->instance[local_id][instance_id].vrrp_state = 1;
                trap_syslog(LOG_INFO, "Trap manual set %s %d Master\n", 
                                    local_id ? "Local-hansi" : "Remote-hansi", instance_id);
            }
        }
    }


    /*  trap init receiver list */
    init_trap_instance_receiver_list(ins_vrrp_state);
    

    /* trap init receiver list */
	trap_renew_master_instance(ins_vrrp_state);


    /* trap renew master instance array */
    int i = 0;        
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        int j = 0;
        for(j = 0; j < INSTANCE_NUM && ins_vrrp_state->instance_master[i][j]; j++) {
            unsigned int instance_id = ins_vrrp_state->instance_master[i][j];
            if(TRAP_OK != trap_init_wtp_array(&(global.wtp[i][instance_id]), MAXWTPID)) {
                trap_syslog(LOG_WARNING, "trap_init_wtp_array local id = %d, instance id = %d failed!\n", i, instance_id);
            }
            trap_syslog(LOG_INFO, "trap_init_wtp_array: global.wtp[%d][%d] = %p\n", i, instance_id, global.wtp[i][instance_id]);
            
            trap_instance_heartbeat_init(&(gHeartbeatInfo[i][instance_id]), i, instance_id);
        }    
    }
    
	return 0;	
}


int trap_show_instance_info(TrapInsVrrpState *ins_vrrp_state)
{
	int i = 0, j = 1;
	for(i = 0; i < VRRP_TYPE_NUM; i++) {
    	for(j = 1; j <= INSTANCE_NUM; j++) {
    	
    		trap_syslog(LOG_DEBUG,"Instance INFO: %s instance_id = %d, vrrp_state = %d, receivelist = %p, trap_instance_ip = %s\n", 
    		    i ? "Local-Hansi" : "Remote-Hansi", j, ins_vrrp_state->instance[i][j].vrrp_state, 
    		    ins_vrrp_state->instance[i][j].receivelist, ins_vrrp_state->instance[i][j].trap_instance_ip );
    	}
    }	
	return 0;

}

