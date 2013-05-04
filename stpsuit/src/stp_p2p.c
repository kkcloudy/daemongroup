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
* stp_p2p.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Point To Point MAC mode selection machine in stp module
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

/* Point To Point MAC mode selection machine : 6.4.3, 6.5.1 */
#include "stp_base.h"
#include "stp_stpm.h"
#include "stp_to.h" /* for stp_to_get_duplex */

#define STATES { \
  CHOOSE(INIT),     \
  CHOOSE(RECOMPUTE),    \
  CHOOSE(STABLE),    \
}

#define GET_STATE_NAME STP_p2p_get_state_name
#include "stp_choose.h"

/*char*
STP_p2p_get_state_name (int state)
{
	return GET_STATE_NAME (state);
}*/


static Bool
stp_p2p_port_check(PORT_T *port)
{
    switch (port->adminPointToPointMac) {
      case P2P_FORCE_TRUE_E:
        return True;
      case P2P_FORCE_FALSE_E:
        return False;
      default:
      case P2P_AUTO_E:
        return stp_to_get_duplex (port);
    }
}

void
MSTP_p2p_enter_state (STATE_MACH_T* s)
{
  register PORT_T* port = s->owner.port;
  register PORT_T* mstport;

  switch (s->State) {
    case BEGIN:
    case INIT:
      port->p2p_recompute = True;
      break;
    case RECOMPUTE:
      port->operPointToPointMac = stp_p2p_port_check (port);
      port->p2p_recompute = False;
/*mstp*/
      for( mstport = port->nextMst; mstport; mstport = mstport->nextMst)     
        mstport->operPointToPointMac = port->operPointToPointMac;
      break;
    case STABLE:
      break;
  }
}

Bool
MSTP_p2p_check_conditions (STATE_MACH_T* s)
{
  register PORT_T* port = s->owner.port;

  switch (s->State) {
    case BEGIN:
    case INIT:
      return stp_statmch_hop_2_state (s, STABLE);
    case RECOMPUTE:
      return stp_statmch_hop_2_state (s, STABLE);
    case STABLE:
      if (port->p2p_recompute) {
        return stp_statmch_hop_2_state (s, RECOMPUTE);
      }
      break;
  }
  return False;
}
#ifdef __cplusplus
}
#endif

