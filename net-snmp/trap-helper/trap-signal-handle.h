/* trap-signal-handle.h */

#ifndef TRAP_SIGNAL_HANDLE_H
#define TRAP_SIGNAL_HANDLE_H


#ifdef  __cplusplus
extern "C" {
#endif

extern TrapList gSignalList;


#define TRAP_SIGNAL_HANDLE_SEND_TRAP_OK				0
#define TRAP_SIGNAL_HANDLE_GET_ARGS_ERROR			1
#define TRAP_SIGNAL_HANDLE_GET_DESCR_ERROR			2
#define TRAP_SIGNAL_HANDLE_DESCR_SWITCH_OFF			3
#define TRAP_SIGNAL_HANDLE_HANSI_BACKUP				4
#define TRAP_SIGNAL_HANDLE_HANSI_MASTER       			5
#define TRAP_SIGNAL_HANDLE_HANSI_OTHERS       			6
#define TRAP_SIGNAL_HANDLE_AC_IS_BACKUP       			7
#define TRAP_SIGNAL_HANDLE_AC_IS_MASTER       			8
#define TRAP_SIGNAL_HANDLE_AC_STATE_ERROR       		9
#define TRAP_SIGNAL_HANDLE_TRAP_SENDED				10
#define TRAP_SIGNAL_HANDLE_RESEND_TRAP_OK			11
#define TRAP_SIGNAL_HANDLE_RESEND_TRAP_ERROR		12
#define TRAP_SIGNAL_HANDLE_AC_IS_NOT_ACTIVE_MASTER	13
#define TRAP_SIGNAL_HANDLE_IF_IS_NOT_LOCAL			14


#define DISTRIBUTED_ACTIVE_MASTER_FILE		"/dbm/local_board/is_active_master"
#define IS_NOT_ACTIVE_MASTER	0

void trap_descr_register_all(TrapList *list, hashtable *ht);

#if 0
void trap_descr_register_all(TrapList *list);
#endif

void trap_signal_register_all(TrapList *list, hashtable *ht);

#if 0
void trap_signal_register_all(TrapList *list);
#endif

void trap_signal_test(void);

void trap_descr_test(void);

char * get_ac_mac_str(char *mac_str, int str_len, unsigned char *mac);
char * get_ac_mac_oid(char *mac_oid, int oid_len, char *mac_str);

#ifdef  __cplusplus
}
#endif

#endif /* TRAP_SIGNAL_HANDLE_H */

