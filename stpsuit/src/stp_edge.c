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
* stp_edge.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for mstp edge state machine in stp module
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

/* Note: this state mashine distinkts from described in P802.1t Clause 18. */
/* I am ready to discuss it                                                */
 
#include "stp_base.h"
#include "stp_stpm.h"

#define STATES {        \
  CHOOSE(DISABLED),         \
  CHOOSE(DETECTED),     \
  CHOOSE(DELEAYED),     \
  CHOOSE(RESOLVED),     \
}

#define GET_STATE_NAME STP_edge_get_state_name
#include "stp_choose.h"

#define DEFAULT_LINK_DELAY  3

void
MSTP_edge_enter_state (STATE_MACH_T *s)
{
  register PORT_T *port = s->owner.port;
  register PORT_T *mstport = port->nextMst;

  switch (s->State) {
    case BEGIN:
      break;
    case DISABLED:
      port->operEdge = port->adminEdge;
      port->wasInitBpdu = False;
      port->lnkWhile = 0;
      port->portEnabled = False;
/*mstp*/
      for( ; mstport; mstport = mstport->nextMst)
      {
          mstport->operEdge =mstport->adminEdge ;
          mstport->portEnabled =  False;
      }
      break;
    case DETECTED:
      port->portEnabled = True;
      port->lnkWhile = port->LinkDelay;
      port->operEdge = False;
/*mstp*/
      for( ; mstport; mstport = mstport->nextMst)
      {
          mstport->operEdge = False;
          mstport->portEnabled = True;
      }
      break;
    case DELEAYED:
      break;
    case RESOLVED:
      if (! port->wasInitBpdu) {
          port->operEdge = port->adminEdge;
/*mstp*/
           for( ; mstport; mstport = mstport->nextMst)
          {
            mstport->operEdge = port->operEdge;
          }
     }
      break;
  }
}

Bool
MSTP_edge_check_conditions (STATE_MACH_T *s)
{
  register PORT_T *port = s->owner.port;

  switch (s->State) {
    case BEGIN:
      return stp_statmch_hop_2_state (s, DISABLED);
    case DISABLED:
      if (port->adminEnable) {
        return stp_statmch_hop_2_state (s, DETECTED);
      }
      break;
    case DETECTED:
      return stp_statmch_hop_2_state (s, DELEAYED);
    case DELEAYED:
      if (port->wasInitBpdu) {
#ifdef STP_DBG
        if (s->debug)
            stp_trace ("port %s 'edge' resolved by BPDU", port->port_name);
#endif        
        return stp_statmch_hop_2_state (s, RESOLVED);
      }

      if (! port->lnkWhile)  {
#ifdef STP_DBG
        if (s->debug)
          stp_trace ("port %s 'edge' resolved by timer", port->port_name);
#endif        
        return stp_statmch_hop_2_state (s, RESOLVED);
      }

      if (! port->adminEnable) {
        return stp_statmch_hop_2_state (s, DISABLED);
      }
      break;
    case RESOLVED:
      if (! port->adminEnable) {
        return stp_statmch_hop_2_state (s, DISABLED);
      }
      break;
  }
  return False;
}
#ifdef __cplusplus
}
#endif

