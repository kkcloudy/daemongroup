/* Path Cost monitoring state machine  */
 
#ifndef _STP_PCOST_H__
#define _STP_PCOST_H__

void

MSTP_pcost_enter_state (STATE_MACH_T* s);
/*STP_pcost_enter_state (STATE_MACH_T* s);*/

Bool
MSTP_pcost_check_conditions (STATE_MACH_T* s);
/*STP_pcost_check_conditions (STATE_MACH_T* s);*/

char*
STP_pcost_get_state_name (int state);

#endif /* _STP_PCOST_H__ */
