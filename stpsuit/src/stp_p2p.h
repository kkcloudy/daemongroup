/* Point To Point MAC mode selection machine : 6.4.3, 6.5.1 */
 
#ifndef _STP_P2P_H__
#define _STP_P2P_H__

void
/*STP_p2p_enter_state (STATE_MACH_T* s);*/
MSTP_p2p_enter_state (STATE_MACH_T* s);

Bool
MSTP_p2p_check_conditions (STATE_MACH_T* s);
/*STP_p2p_check_conditions (STATE_MACH_T* s);*/

char*
STP_p2p_get_state_name (int state);

#endif /* _STP_P2P_H__ */
