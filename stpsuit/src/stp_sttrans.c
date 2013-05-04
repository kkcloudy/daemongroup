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
* stp_sttrans.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for state transfering in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.2 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* Port State Transition state machine : 17.24 */
    
#include "stp_base.h"
#include "stp_stpm.h"
#include "stp_to.h"
#include "stp_log.h"
#include "stp_npd_cmd.h"

#define STATES { \
  CHOOSE(DISCARDING),   \
  CHOOSE(LEARNING), \
  CHOOSE(FORWARDING),   \
}

#define GET_STATE_NAME STP_sttrans_get_state_name
#include "stp_choose.h"

unsigned int sttrans_count = 0;

#ifdef STRONGLY_SPEC_802_1W
static Bool
stp_sttrans_disableLearning (STATE_MACH_T *this)
{
  register PORT_T *port = this->owner.port;

  return stp_to_set_learning (port->port_index, port->owner->vlan_id, False);
}

static Bool
stp_sttrans_enableLearning (STATE_MACH_T *this)
{
  register PORT_T *port = this->owner.port;

  return stp_to_set_learning (port->port_index, port->owner->vlan_id, True);
}

static Bool
stp_sttrans_disableForwarding (STATE_MACH_T *this)
{
  register PORT_T *port = this->owner.port;

  return stp_to_set_forwarding (port->port_index, port->owner->vlan_id, False);
}

static Bool
stp_sttrans_enableForwarding (STATE_MACH_T *this)
{
  register PORT_T *port = this->owner.port;

  return stp_to_set_forwarding (port->port_index, port->owner->vlan_id, True);
}
#endif

static void stp_sttrans_sendStpStatetoNpd(STATE_MACH_T *this)
{
	register PORT_T  *port = this->owner.port;
	register STPM_T  *stpm = port->owner;
	unsigned int mstid = port->vlan_id;
	unsigned int port_index = port->port_index;
	unsigned short vid = 0;
	NAM_RSTP_PORT_STATE_E state = NAM_STP_PORT_STATE_DISABLE_E;
	
	if(DISCARDING == this->State)
		state = NAM_STP_PORT_STATE_DISCARD_E;
	else if(LEARNING == this->State)
		state = NAM_STP_PORT_STATE_LEARN_E;
	else if(FORWARDING == this->State)
		state = NAM_STP_PORT_STATE_FORWARD_E;

	/*change for add master mstp suport*/
	if (port->isWAN) {;

		stp_npd_set_wan_state(port->br_ifindex, port->port_ifindex, state);
		stp_syslog_dbg("set wan port state br %d, port %d, state %d \n", port->br_ifindex, port->port_ifindex, state);
	}
	else {
		stp_npd_cmd_send_stp_info(mstid,vid,port_index,state);
	}
	/*stp_npd_cmd_send_stp_info(mstid,vid,port_index,state);*/

	return ;
}

void
MSTP_sttrans_enter_state (STATE_MACH_T *this)
{
  register PORT_T    *port = this->owner.port;

  switch (this->State) {
    case BEGIN:
    case DISCARDING:
      port->learning = False;
      port->forwarding = False;
	  if(port->portEnabled){
	  	stp_sttrans_sendStpStatetoNpd(this);
	  }

#ifdef STRONGLY_SPEC_802_1W
      stp_sttrans_disableLearning (this);
      stp_sttrans_disableForwarding (this);
#else
      stp_to_set_port_state (port->port_index, port->owner->vlan_id, UID_PORT_DISCARDING);
	  stp_syslog_protocol("STP port[%#0x] vlan[%d] state change %s =>%s\n", \
								port->port_index,port->owner->vlan_id,STR(discarding),STR(discarding));
#endif
      break;
    case LEARNING:
      port->learning = True;
	  stp_sttrans_sendStpStatetoNpd(this);
#ifdef STRONGLY_SPEC_802_1W
      stp_sttrans_enableLearning (this);
#else	
      stp_to_set_port_state (port->port_index, port->owner->vlan_id, UID_PORT_LEARNING);
#endif
	  stp_syslog_protocol("STP port[%#0x] vlan[%d] state change %s =>%s\n", \
								port->port_index,port->owner->vlan_id,STR(learning),STR(learning));
      break;
    case FORWARDING:
      port->tc = !port->operEdge;
      port->forwarding = True;
/*mstp*/
      port->synced = False;
      port->proposing = False;
      port->agreed    = False; 
	  stp_sttrans_sendStpStatetoNpd(this);
#ifdef STRONGLY_SPEC_802_1W
      stp_sttrans_enableForwarding (this);

#else
      stp_to_set_port_state (port->port_index, port->owner->vlan_id, UID_PORT_FORWARDING);
#endif
	  stp_syslog_protocol("STP port[%#0x] vlan[%d] state change %s =>%s\n", \
								port->port_index,port->owner->vlan_id,STR(forwarding),STR(forwarding));
      break;
  }

}

Bool
MSTP_sttrans_check_conditions (STATE_MACH_T *this)
{
  register PORT_T       *port = this->owner.port;
  unsigned int ret = 0;

  
  if (BEGIN == this->State) {
       return stp_statmch_hop_2_state (this, DISCARDING);
  	}
  switch (this->State) {
    case DISCARDING:
	  if((port->portEnabled)&&(sttrans_count < 1)&&(!port->learn)){
	  	  stp_sttrans_sendStpStatetoNpd(this);
		  sttrans_count++;
	  }
      if (port->learn) {
        return stp_statmch_hop_2_state (this, LEARNING);
      }
      break;
    case LEARNING:
      if (port->forward) {
        return stp_statmch_hop_2_state (this, FORWARDING);
      }
      if (!port->learn) {
        return stp_statmch_hop_2_state (this, DISCARDING);
      }
      break;
    case FORWARDING:
      if (!port->forward) {
        return stp_statmch_hop_2_state (this, DISCARDING);
      }
      break;
  }

  return False;
}
#ifdef __cplusplus
}
#endif

