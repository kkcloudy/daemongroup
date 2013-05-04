/* Port Role Transitions state machine : 17.24 */
#ifndef _STP_ROLES_TRANSIT_H__
#define _STP_ROLES_TRANSIT_H__

void
MSTP_roletrns_enter_state (STATE_MACH_T* s);

Bool
MSTP_roletrns_check_conditions (STATE_MACH_T* s);

char* STP_roletrns_get_state_name (int state);

#endif /* _STP_ROLES_TRANSIT_H__ */

