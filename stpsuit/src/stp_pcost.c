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
* stp_pcost.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Path Cost monitoring state machine in stp module
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

/* Path Cost monitoring state machine */
#include "stp_base.h"
#include "stp_stpm.h"
#include "stp_to.h" /* for stp_to_get_port_oper_speed */

#define STATES {        \
  CHOOSE(AUTO),         \
  CHOOSE(FORSE),        \
  CHOOSE(STABLE),       \
}

#define GET_STATE_NAME STP_pcost_get_state_name
#include "stp_choose.h"

static long
stp_pcost_compute_auto_pcost (STATE_MACH_T *this)
{
    long lret;
    register PORT_T*  port = this->owner.port;

    if (port->usedSpeed        < 10L) {         /* < 10Mb/s */
        lret = 20000000;
    } else if (port->usedSpeed <= 10L) {        /* 10 Mb/s  */
        lret = 2000000;        
    } else if (port->usedSpeed <= 100L) {       /* 100 Mb/s */
        lret = 200000;     
    } else if (port->usedSpeed <= 1000L) {      /* 1 Gb/s */
        lret = 20000;      
    } else if (port->usedSpeed <= 10000L) {     /* 10 Gb/s */
        lret = 2000;       
    } else if (port->usedSpeed <= 100000L) {    /* 100 Gb/s */
        lret = 200;        
    } else if (port->usedSpeed <= 1000000L) {   /* 1 GTb/s */
        lret = 20;     
    } else if (port->usedSpeed <= 10000000L) {  /* 10 Tb/s */
        lret = 2;      
    } else   /* ??? */                        { /* > Tb/s */
        lret = 1;       
    }
#ifdef STP_DBG
    if (port->pcost->debug) {
      stp_trace ("usedSpeed=%lu lret=%ld", port->usedSpeed, lret);
    }
#endif

    return lret;
}

static void
stp_pcost_updport_pcost (STATE_MACH_T *this)
{
	/*register PORT_T*  port = this->owner.port;
	port->operPCost  = 20000;*/
/*mstp*/
  register PORT_T*  port = this->owner.port;

  port->reselect = 1;
  port->selected = 0;
}

void
MSTP_pcost_enter_state (STATE_MACH_T *this)
{
  register PORT_T*  port = this->owner.port;

  switch (this->State) {
    case BEGIN:
      break;
    case AUTO:
      port->operSpeed = stp_to_get_port_oper_speed (port);
#ifdef STP_DBG
      if (port->pcost->debug) {
        stp_trace ("AUTO:operSpeed=%lu", port->operSpeed);
      }
#endif
      port->usedSpeed = port->operSpeed;
      port->operPCost = stp_pcost_compute_auto_pcost (this);
      break;
    case FORSE:
      port->operPCost = port->adminPCost;
      port->usedSpeed = -1;
      break;
    case STABLE:
	  /*????????*/
	#ifdef STP_DBG
      if (port->pcost->debug) {
        stp_trace ("STABLE:operSpeed=%lu", port->operSpeed);
      }
#endif
      stp_pcost_updport_pcost (this);
      break;
  }
}

Bool
MSTP_pcost_check_conditions (STATE_MACH_T* this)
{
  register PORT_T*  port = this->owner.port;

  switch (this->State) {
    case BEGIN:
      return stp_statmch_hop_2_state (this, AUTO);
    case AUTO:
      return stp_statmch_hop_2_state (this, STABLE);
    case FORSE:
      return stp_statmch_hop_2_state (this, STABLE);
    case STABLE:
      if (ADMIN_PORT_PATH_COST_AUTO == port->adminPCost && 
          port->operSpeed != port->usedSpeed) {
          return stp_statmch_hop_2_state (this, AUTO);
      }

      if (ADMIN_PORT_PATH_COST_AUTO != port->adminPCost &&
          port->operPCost != port->adminPCost) {
          return stp_statmch_hop_2_state (this, FORSE);
      }
      break;
  }
  return False;
}
#ifdef __cplusplus
}
#endif

