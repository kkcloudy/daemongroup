/* Note: this state mashine distinkts from described in P802.1t Clause 18. */
/* I am ready to discuss it                                                */
 
#ifndef _STP_EDGE_H__
#define _STP_EDGE_H__

void

/*STP_edge_enter_state (STATE_MACH_T* s);*/
MSTP_edge_enter_state (STATE_MACH_T* s);
Bool
MSTP_edge_check_conditions (STATE_MACH_T* s);
/*STP_edge_check_conditions (STATE_MACH_T* s);*/

char*
STP_edge_get_state_name (int state);

#endif /* _STP_EDGE_H__ */
