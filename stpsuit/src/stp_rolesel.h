/* Port Role Selection state machine : 17.22 */
#ifndef _STP_ROLES_SELECT_H
#define _STP_ROLES_SELECT_H

void
MSTP_rolesel_enter_state (STATE_MACH_T* s);
/*STP_rolesel_enter_state (STATE_MACH_T* s);*/

Bool
MSTP_rolesel_check_conditions (STATE_MACH_T* s);
/*STP_rolesel_check_conditions (STATE_MACH_T* s);*/

void
stp_rolesel_update_stpm (struct stpm_t* this);

char*
STP_rolesel_get_state_name (int state);

#endif /* _STP_ROLES_SELECT_H */

