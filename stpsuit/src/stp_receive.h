#ifndef __STP_RECEIVE_H__
#define __STP_RECEIVE_H__
#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_in.h"

#define STATES { \
  CHOOSE(INIT),             \
  CHOOSE(DISCARD),         \
  CHOOSE(RECEIVE),         \
}

#include "stp_vector.h"

char* STP_receive_get_state_name (int state);

Bool stp_receive_mstpVendorCisco(char *mac);
        
Bool stp_receive_fromSameRegion(STATE_MACH_T* this );   /* 13.26.5 */

Bool stp_receive_rcvdAnyMsg(STATE_MACH_T* this);  /* 13.25.7 */

void stp_receive_clearAllRcvdMsgs(void);  /* 13.26.3 */

static void stp_receive_setRcvdMsgs(STATE_MACH_T* this);  /* 13.26.15 */

static Bool stp_receive_setTcFlags(STATE_MACH_T* this); /* 13.26.19 */

Bool stp_receive_updtBPDUVersion (STATE_MACH_T* this);

void MSTP_receive_enter_state (STATE_MACH_T* this);

Bool MSTP_receive_check_conditions (STATE_MACH_T* this);
#endif
