/* eag_trap.h */

#ifndef _EAG_TRAP_H
#define _EAG_TRAP_H

#include "eag_dbus.h"

#define EAG_TRAP 					1
#define EAG_TRAP_CLEAR				0

#define EAG_DEFAULT_ONLINEUSERNUM_THRESHOLD          1000

/* trap type */
#define EAG_TRAP_USER_LOGOFF_ABNORMAL				"eag_send_trap_user_logoff_abnormal"
#define EAG_ONLINE_USER_NUM_THRESHOLD_SIGNAL		"eag_online_user_num_threshold_trap"

int eag_trap_set_eagdbus( eag_dbus_t *eagdbus);

int eag_send_trap_signal(const char *signal_name,int first_arg_type,...);

#endif /* _EAG_TRAP_H */
