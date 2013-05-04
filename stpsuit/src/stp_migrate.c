/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* stp_migrate.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Port Protocol Migration state machine in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* Port Protocol Migration state machine : 17.26 */
#include "stp_base.h"
#include "stp_stpm.h"
#include "stp_log.h"

#define STATES { \
  CHOOSE(INIT),     \
  CHOOSE(SEND_RSTP),    \
  CHOOSE(SENDING_RSTP), \
  CHOOSE(SEND_STP), \
  CHOOSE(SENDING_STP),  \
}

#define GET_STATE_NAME STP_migrate_get_state_name
#include "stp_choose.h"

#define MigrateTime 3 /* 17,16.4 */

void
MSTP_migrate_enter_state (STATE_MACH_T* this)
{
  register PORT_T*       port = this->owner.port;

  switch (this->State) {
    case BEGIN:
    case INIT:
	
      port->initPm = True;
      port->mcheck = False;
      break;
    case SEND_RSTP:
      port->mdelayWhile = MigrateTime;
      port->mcheck = port->initPm = False;
      port->sendRSTP = True;
      break;
    case SENDING_RSTP:
      port->rcvdRSTP = port->rcvdSTP = False;
      break;
    case SEND_STP:
      port->mdelayWhile = MigrateTime;
      port->sendRSTP = False;
      port->initPm = False;
      break;
    case SENDING_STP:
      port->rcvdRSTP = port->rcvdSTP = False;
      break;
  }
}

Bool
MSTP_migrate_check_conditions (STATE_MACH_T* this)
{
  register PORT_T*    port = this->owner.port;

  if ((!port->portEnabled && !port->initPm) || BEGIN == this->State) {
    return stp_statmch_hop_2_state (this, INIT);
  }

  switch (this->State) {
    case INIT:
	  /*when received a packet form this port ,the portEnable became true*/
      if (port->portEnabled) {
        return stp_statmch_hop_2_state (this, (port->owner->ForceVersion >= 2) ?
                                   SEND_RSTP : SEND_STP);
      }
      break;
    case SEND_RSTP:
      return stp_statmch_hop_2_state (this, SENDING_RSTP);
    case SENDING_RSTP:
      if (port->mcheck)
        return stp_statmch_hop_2_state (this, SEND_RSTP);
      if (port->mdelayWhile &&
          (port->rcvdSTP || port->rcvdRSTP)) {
        return stp_statmch_hop_2_state (this, SENDING_RSTP);
      }
        
      if (!port->mdelayWhile && port->rcvdSTP) {
        return stp_statmch_hop_2_state (this, SEND_STP);       
      }
        
      if (port->owner->ForceVersion < 2) {
        return stp_statmch_hop_2_state (this, SEND_STP);
      }
        
      break;
    case SEND_STP:
      return stp_statmch_hop_2_state (this, SENDING_STP);
    case SENDING_STP:
      if (port->mcheck)
        return stp_statmch_hop_2_state (this, SEND_RSTP);
      if (port->mdelayWhile &&
          (port->rcvdSTP || port->rcvdRSTP))
        return stp_statmch_hop_2_state (this, SENDING_STP);
      if (!port->mdelayWhile && port->rcvdRSTP)
        return stp_statmch_hop_2_state (this, SEND_RSTP);
      break;
  }
  return False;
}
#ifdef __cplusplus
}
#endif

