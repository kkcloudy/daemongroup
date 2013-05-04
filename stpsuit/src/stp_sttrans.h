#ifndef _STP_STATE_TRANSIT_H__
#define _STP_STATE_TRANSIT_H__

void
MSTP_sttrans_enter_state (STATE_MACH_T* s);
/*STP_sttrans_enter_state (STATE_MACH_T* s);*/

Bool
MSTP_sttrans_check_conditions (STATE_MACH_T* s);
/*STP_sttrans_check_conditions (STATE_MACH_T* s);*/

char*
STP_sttrans_get_state_name (int state);

#endif /* _STP_STATE_TRANSIT_H__ */

