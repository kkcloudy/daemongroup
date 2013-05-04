/* Topolgy Change state machine : 17.25 */
#ifndef _STP_TOPO_CHANGE_H__
#define _STP_TOPO_CHANGE_H__

void
MSTP_topoch_enter_state (STATE_MACH_T* s);
/*STP_topoch_enter_state (STATE_MACH_T* s);*/

Bool
MSTP_topoch_check_conditions (STATE_MACH_T* s);
/*STP_topoch_check_conditions (STATE_MACH_T* s);*/

char* STP_topoch_get_state_name (int state);

#endif /* _STP_TOPO_CHANGE_H__ */

