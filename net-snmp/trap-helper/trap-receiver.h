/* trap-receiver.h */

#ifndef TRAP_RECEIVER_H
#define TRAP_RECEIVER_H


#ifdef  __cplusplus
extern "C" {
#endif

#include "trap-instance.h"

#define V1      1
#define V2      2
#define V3      3

#define U_INT16  unsigned short

typedef struct TrapReceiver {
    int local_id;
	int instance_id;
	int version; // v1, v2c, v3
	
	char dest_ipAddr[136];  //ip(32)+port(6)
	U_INT16 dest_port;
	
	char sour_ipAddr[136];
	U_INT16 sour_port;

	char community[128];
	netsnmp_session *ss;
} TrapReceiver;

void init_trap_instance_receiver_list(TrapInsVrrpState *ins_vrrp_state);

void trap_receiver_list_destroy(TrapList *tRcvList);

typedef struct TrapV3User {
	char username[32];
	char auth_type[8];
	char auth_passwd[21];
	char priv_type[8];
	char priv_passwd[21];
} TrapV3User;

void trap_v3user_free(TrapV3User *tV3User);

void trap_v3user_list_parse(TrapList *tV3UserList);

void trap_v3user_list_destroy(TrapList *tV3UserList);

extern TrapList gReceiverList;
extern TrapList gV3UserList;

void trap_receiver_test(void);

#ifdef  __cplusplus
}
#endif

#endif /* TRAP_RECEIVER_H */

