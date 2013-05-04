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
* stp_topoch.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Topolgy Change state machine in stp module
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

/* Topolgy Change state machine : 17.25 */
  
#include "stp_base.h"
#include "stp_stpm.h"
#include "stp_to.h" /* for stp_to_flush_lt */
  
#define STATES { \
  CHOOSE(INIT),             \
  CHOOSE(INACTIVE),         \
  CHOOSE(ACTIVE),         \
  CHOOSE(DETECTED),         \
  CHOOSE(NOTIFIED_TC),          \
  CHOOSE(PROPAGATING),          \
  CHOOSE(ACKNOWLEDGED),         \
  CHOOSE(NOTIFIED_TCN),         \
}

#define GET_STATE_NAME STP_topoch_get_state_name
#include "stp_choose.h"

#ifndef STRONGLY_SPEC_802_1W
/* 
 * In many kinds of hardware the function
 * stp_to_flush_lt is a) is very hard and b) cannot
 * delete learning emtries per port. The alternate
 * method may be used: we don't care operEdge flag here,
 * but clean learning table once for TopologyChange
 * for all ports, except the received port. I am ready to discuss :(
 * See below word STRONGLY_SPEC_802_1W
 */
#else
static Bool
stp_topoch_flush (STATE_MACH_T *this, char* reason) /* 17.19.9 */
{
	register PORT_T* port = this->owner.port;
	Bool bret;
	LT_FLASH_TYPE_T type = LT_FLASH_ONLY_THE_PORT;

	if (port->operEdge) return True;
	if (this->debug) {
	  stp_trace("%s (%s, %s, %s, '%s')",
	      "stp_topoch_flush", port->port_name, port->owner->name,
	      LT_FLASH_ONLY_THE_PORT == type ? "this port" : "other ports",
	      reason);
	}

	bret = stp_to_flush_lt (port->port_index, port->owner->vlan_id,
	                         LT_FLASH_ONLY_THE_PORT, reason);
}
#endif

static void
stp_topoch_setTcPropTree (STATE_MACH_T* this, char* reason) /* 17.19.14 */
{
  register PORT_T* port = this->owner.port;
  register PORT_T* tmp;

  for (tmp = port->owner->ports; tmp; tmp = tmp->next) {
    if (tmp->port_index != port->port_index)
      tmp->tcProp = True;
  }

#ifndef STRONGLY_SPEC_802_1W
#ifdef STP_DBG
  if (this->debug) {
    stp_trace("%s (%s, %s, %s, '%s')",
        "clearFDB", port->port_name, port->owner->name,
        "other ports", reason);
  }
#endif

  stp_to_flush_lt (port->port_index, port->owner->vlan_id,
                    LT_FLASH_ALL_PORTS_EXCLUDE_THIS, reason);
#endif
}

static unsigned int
stp_topoch_newTcWhile (STATE_MACH_T* this) /* 17.19.7 */
{
  register PORT_T* port = this->owner.port;
  register PORT_T*           cistport = stp_port_mst_findport(STP_CIST_ID, port->port_index) ;
#if 0
  if (port->sendRSTP && port->operPointToPointMac) {
    return 2 * port->owner->rootTimes.HelloTime;
  }
  return port->owner->rootTimes.MaxAge;
#endif
  if ( port->tcWhile == 0 && port->sendRSTP && port->operPointToPointMac) {
    return 2 * cistport->owner->rootTimes.HelloTime;
  }
  return cistport->owner->rootTimes.MaxAge + cistport->owner->rootTimes.ForwardDelay;
}

void
MSTP_topoch_enter_state (STATE_MACH_T* this)
{
  register PORT_T*      port = this->owner.port;
  Bool           cist = (port->owner->vlan_id == STP_CIST_ID) ? 1 : 0;

  switch (this->State) {
    case BEGIN:
    case INIT:
#ifdef STRONGLY_SPEC_802_1W
      stp_topoch_flush (this, "topoch INIT");
#endif
      port->tcWhile = 0;
     /* port->tc =
      port->tcProp =*/
      if (cist) port->tcAck = False;
      /*port->tcAck = False;*/
      break;
    case INACTIVE:
      if (cist)
      {
         port->rcvdTc = port->rcvdTcn =  port->rcvdTcAck = False ;     
      }
      port->rcvdTc = port->tcProp = False;
      break;
    case ACTIVE:
      break;
    case DETECTED:
      port->tcWhile = stp_topoch_newTcWhile (this);
#ifdef STP_DBG
  if (this->debug) 
    stp_trace("DETECTED: tcWhile=%d on port %s", 
        port->tcWhile, port->port_name);
#endif
      stp_topoch_setTcPropTree (this, "DETECTED");
      /*setTcPropBridge (this, "DETECTED");*/
      stp_roletrns_setnewinfoXst(port);
      break;
    case NOTIFIED_TC:
      if (cist) port->rcvdTcn =  False;
      port->rcvdTc = False;
      if (cist && (port->role == DesignatedPort) ) {
        port->tcAck = True;
      }
      stp_topoch_setTcPropTree (this, "NOTIFIED_TC");
      break;
    case PROPAGATING:
      port->tcWhile = stp_topoch_newTcWhile (this);
#ifdef STP_DBG
  if (this->debug) 
    stp_trace("PROPAGATING: tcWhile=%d on port %s", 
        port->tcWhile, port->port_name);
#endif
#ifdef STRONGLY_SPEC_802_1W
      stp_topoch_flush (this, "topoch PROPAGATING");
#endif
      port->tcProp = False;
      break;
    case ACKNOWLEDGED:
      port->tcWhile = 0;
#ifdef STP_DBG
  if (this->debug) 
    stp_trace("ACKNOWLEDGED: tcWhile=%d on port %s", 
        port->tcWhile, port->port_name);
#endif
      port->rcvdTcAck = False;
      break;
    case NOTIFIED_TCN:
      port->tcWhile = stp_topoch_newTcWhile (this);
#ifdef STP_DBG
  if (this->debug) 
    stp_trace("NOTIFIED_TCN: tcWhile=%d on port %s", 
        port->tcWhile, port->port_name);
#endif
      break;
  };
}

Bool
MSTP_topoch_check_conditions (STATE_MACH_T* this)
{
  register PORT_T*      port = this->owner.port;

  if (BEGIN == this->State) {
    return stp_statmch_hop_2_state (this, INIT);
  }

  switch (this->State) {
    case INIT:
      return stp_statmch_hop_2_state (this, INACTIVE);
    case INACTIVE:
#if 0
      if (port->role == RootPort || port->role == DesignatedPort)
        return stp_statmch_hop_2_state (this, TCACTIVE);
      if (port->rcvdTc || port->rcvdTcn || port->rcvdTcAck ||
          port->tc || port->tcProp)
        return stp_statmch_hop_2_state (this, INACTIVE);
#endif
      if (port->rcvdTc || port->rcvdTcn || port->rcvdTcAck || port->tcProp)
        return stp_statmch_hop_2_state (this, INACTIVE);
      if ( ((port->role == RootPort) ||(port->role == DesignatedPort) || (port->role == MasterPort)) &&
         port->forward && !port->operEdge )
      {
          return stp_statmch_hop_2_state (this, DETECTED);
      }
      break;
    case DETECTED:
      return stp_statmch_hop_2_state (this, ACTIVE);      
    case ACTIVE:
      if (port->role != RootPort && (port->role != DesignatedPort) && (port->role != MasterPort))
        return stp_statmch_hop_2_state (this, INIT);
      /*if (port->tc)
        return stp_statmch_hop_2_state (this, DETECTED);*/
      if (port->rcvdTcn)
        return stp_statmch_hop_2_state (this, NOTIFIED_TCN);
      if (port->rcvdTc)
        return stp_statmch_hop_2_state (this, NOTIFIED_TC);
      if (port->tcProp )
     /* if (port->tcProp && !port->operEdge)*/
        return stp_statmch_hop_2_state (this, PROPAGATING);
      if (port->rcvdTcAck)
        return stp_statmch_hop_2_state (this, ACKNOWLEDGED);
      break;
    /*case DETECTED:
      return stp_statmch_hop_2_state (this, TCACTIVE);*/
    case NOTIFIED_TC:
      return stp_statmch_hop_2_state (this, ACTIVE);
    case PROPAGATING:
      return stp_statmch_hop_2_state (this, ACTIVE);
    case ACKNOWLEDGED:
      return stp_statmch_hop_2_state (this, ACTIVE);
    case NOTIFIED_TCN:
      return stp_statmch_hop_2_state (this, NOTIFIED_TC);
  };
  return False;
}
#ifdef __cplusplus
}
#endif

