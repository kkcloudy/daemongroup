/* Port Transmit state machine : 17.27 */
#ifndef _STP_TRANSMIT_H__
#define _STP_TRANSMIT_H__

void
MSTP_transmit_enter_state (STATE_MACH_T* s);
/*STP_transmit_enter_state (STATE_MACH_T* s);*/
  
Bool
MSTP_transmit_check_conditions (STATE_MACH_T* s);
/*STP_transmit_check_conditions (STATE_MACH_T* s);*/

char*
STP_transmit_get_state_name (int state);


#endif /* _STP_TRANSMIT_H__ */

