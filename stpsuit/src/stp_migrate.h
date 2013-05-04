/* Port Protocol Migration state machine : 17.26 */
 
#ifndef _STP_MIGRATE_H__
#define _STP_MIGRATE_H__

void
/*STP_migrate_enter_state (STATE_MACH_T* s);*/
MSTP_migrate_enter_state (STATE_MACH_T* s);
Bool
/*STP_migrate_check_conditions (STATE_MACH_T* s);*/
MSTP_migrate_check_conditions (STATE_MACH_T* s);
char*
STP_migrate_get_state_name (int state);

#endif /* _STP_MIGRATE_H__ */
