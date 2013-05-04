#ifndef _STP_PORTINFO_H__
#define _STP_PORTINFO_H__

void
MSTP_info_enter_state (STATE_MACH_T* s);

Bool
MSTP_info_check_conditions (STATE_MACH_T* s);
/*STP_info_check_conditions (STATE_MACH_T* s);*/

int
stp_portinfo_rx_bpdu (PORT_T* this, struct stp_bpdu_t* bpdu, size_t len);

char*
STP_info_get_state_name (int state);

void 
stp_portinfo_send_tcn2npd(PORT_T *port);


#endif /* _STP_PORTINFO_H__ */
