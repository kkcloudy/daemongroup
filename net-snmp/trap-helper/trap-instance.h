#ifndef _TRAP_INSTANCE_H_
#define _TRAP_INSTANCE_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define INSTANCE_BACKUP 0
#define INSTANCE_MASTER 1

#define AC_IS_BACKUP 0
#define AC_IS_MASTER 1

typedef struct Instance_info_t {
	int vrrp_state;
	TrapList *receivelist;
	char trap_instance_ip[128];
	char backup_network_ip[128];
	int pre_vrrp_state;
}Instance_info;

typedef struct TrapInsVrrpState_t {
	Instance_info instance[2][17];
    unsigned int instance_master[2][16];
}TrapInsVrrpState;

extern TrapInsVrrpState gInsVrrpState;

#if 0
int trap_init_vrrp_state(TrapInsVrrpState *ins_vrrp_state);
#endif

int trap_set_instance_vrrp_state(TrapInsVrrpState *ins_vrrp_state, unsigned int local_id, unsigned int instance_id, int state);

//int trap_get_instance_info ( Z_VRRP  *zvrrp, int instance_id );

int trap_init_instance_info ( TrapInsVrrpState *ins_vrrp_state); //add for multi instance init snmpssion receive_list and bind local instance socket

#ifdef  __cplusplus
}
#endif

#endif
