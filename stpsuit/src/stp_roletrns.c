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
* stp_roletrns.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Port Role Transitions state machine in stp module
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

/* Port Role Transitions state machine : 17.24 */
 
#include "stp_base.h"

#include "stp_stpm.h"
#if 0
#define STATES { \
   CHOOSE(INIT_PORT),       \
   CHOOSE(BLOCK_PORT),      \
   CHOOSE(BLOCKED_PORT),    \
   CHOOSE(BACKUP_PORT),     \
   CHOOSE(ROOT_PROPOSED),   \
   CHOOSE(ROOT_AGREED),     \
   CHOOSE(REROOT),      \
   CHOOSE(ROOT_PORT),       \
   CHOOSE(REROOTED),        \
   CHOOSE(ROOT_LEARN),      \
   CHOOSE(ROOT_FORWARD),    \
   CHOOSE(DESIGNATED_PROPOSE),  \
   CHOOSE(DESIGNATED_SYNCED),   \
   CHOOSE(DESIGNATED_RETIRED),  \
   CHOOSE(DESIGNATED_PORT), \
   CHOOSE(DESIGNATED_LISTEN),   \
   CHOOSE(DESIGNATED_LEARN),    \
   CHOOSE(DESIGNATED_FORWARD),  \
}
#endif
/*mstp*/
#define STATES { \
   CHOOSE(INIT_PORT),       \
   CHOOSE(BLOCK_PORT),      \
   CHOOSE(BLOCKED_PORT),    \
   CHOOSE(BACKUP_PORT),     \
   CHOOSE(PROPOSED),   \
   CHOOSE(PROPOSING),     \
   CHOOSE(AGREES),      \
   CHOOSE(SYNCED),       \
   CHOOSE(REROOT),        \
   CHOOSE(ACTIVE_PORT),      \
   CHOOSE(FORWARD),    \
   CHOOSE(LEARN),  \
   CHOOSE(LISTEN),   \
   CHOOSE(REROOTED),  \
   CHOOSE(ROOT), \
   CHOOSE(NON_STP) , \
}
#define GET_STATE_NAME STP_roletrns_get_state_name
#include "stp_choose.h"

static void stp_roletrns_setSyncTree (STATE_MACH_T *this)  /* 13.26.18 */
{
  register PORT_T* port;

  for (port = this->owner.port->owner->ports; port; port = port->next) {
    port->sync = True; /* in ROOT_PROPOSED (setSyncBridge) */
  }
}

static void stp_roletrns_setReRootTree (STATE_MACH_T *this)  {  /* 13.26.16  */
  register PORT_T* port;

  for (port = this->owner.port->owner->ports; port; port = port->next) {
    port->reRoot = True; /* In setReRootBridge */
  }
}

static Bool stp_roletrns_compute_all_synced (PORT_T* this)  /*13.25.1 allSynced*/
{
  register PORT_T* port;

  for (port = this->owner->ports; port; port = port->next) {
    if (port->port_index == this->port_index || port->admin_non_stp ) continue;
      if ( !port->synced ) {
        return False;
    }
  }

  return True;
}

static Bool stp_roletrns_compute_re_rooted(PORT_T *this) /* 13.25.10 */
{
  register PORT_T* port;

  for (port = this->owner->ports; port; port = port->next) {
    if (port->port_index == this->port_index || port->admin_non_stp ) continue;
    if (port->rrWhile) {
      return False;
    }
  }
  return True;
}

inline void stp_roletrns_setnewinfoCist(PORT_T *this)
{
    if (NULL == this)
    {
        return;
    }
    this->newInfoCist = True;
	
    return;
}
inline void stp_roletrns_setnewinfoMsti(PORT_T *this)
{
    PORT_T *cistport = NULL;

    if (NULL == this)
    {
        return;
    }

    cistport = stp_port_mst_findport(STP_CIST_ID, this->port_index);
    if(NULL == cistport)
	return;
    cistport->newInfoMsti = True;
    return;
}

void stp_roletrns_setnewinfoXst(PORT_T *this)
{
   if(this->vlan_id == STP_CIST_ID)
     return stp_roletrns_setnewinfoCist(this);
   return stp_roletrns_setnewinfoMsti( this);
}

void
MSTP_roletrns_enter_state (STATE_MACH_T* this)
{
  register PORT_T*           port = this->owner.port;
  register STPM_T*           stpm;

  stpm = stp_stpm_get_the_cist();
  if ( stpm == NULL )
  {
     return  ;
  }

  switch (this->State) {
    case BEGIN:
    case INIT_PORT:
#if 0 /* due 802.1y Z.4 */
      port->role = DisabledPort;
#else
      port->role = port->selectedRole = DisabledPort;
      port->reselect = True;
#endif
      port->synced = False; /* in INIT */
      port->sync = True; /* in INIT */
      port->reRoot = True; /* in INIT_PORT */
      port->rrWhile = stpm->rootTimes.ForwardDelay;
      port->fdWhile = stpm->rootTimes.ForwardDelay;
      port->rbWhile = 0; 
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("after init", port);
#endif
      break;
    case BLOCK_PORT:
      port->role = port->selectedRole;
      port->learn =
      port->forward = False;
      break;
    case BLOCKED_PORT:
      port->fdWhile = stpm->rootTimes.ForwardDelay;
      port->synced = True; /* In BLOCKED_PORT */
      port->rrWhile = 0;
      port->sync = port->reRoot = False; /* BLOCKED_PORT */
      break;
    case BACKUP_PORT:
      port->rbWhile = 2 * stpm->rootTimes.HelloTime;
      break;

    /* 17.23.2 */
    case PROPOSED:  /*mstp*/
      stp_roletrns_setSyncTree (this);
      port->proposed = False;
#ifdef STP_DBG
      if (this->debug) 
        stp_port_trace_flags ("PROPOSED", port);
#endif
      break;
    case PROPOSING:  /*mstp*/
      port->proposing = True ;
      stp_roletrns_setnewinfoXst(port);
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("PROPOSING", port);
#endif
      break;
    case AGREES:
      port->proposed = False ;
      port->agree = True ;
      stp_roletrns_setnewinfoXst(port);
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("AGREES", port);
#endif
      break;
    case SYNCED:
      if (port->role != RootPort)
      {
         port->rrWhile = 0;
      }
      port->synced = True;
      port->sync = False ;
     break;
    case REROOT:
      stp_roletrns_setReRootTree(this);
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("REROOT", port);
#endif
      break;
    case FORWARD:
      port->fdWhile = 0;
      port->forward = True;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("FORWARD", port);
#endif
      break;
    case LISTEN:
      port->learn = port->forward = False;
      port->fdWhile = stpm->rootTimes.ForwardDelay;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("ISTEN", port);
#endif
      break;
    case LEARN:
      port->learn = True;
      port->fdWhile = stpm->rootTimes.ForwardDelay;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("LEARN", port);
#endif
      break;
    case REROOTED:
      port->reRoot = False; /* In REROOTED */
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("REROOTED", port);
#endif
      break;
    case ROOT:
      port->rrWhile = stpm->rootTimes.ForwardDelay ;
      /*port->learn = True;*/
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("ROOT", port);
#endif
      break;
#if 0
    case ROOT_FORWARD:
      port->fdWhile = 0;
      port->forward = True;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("ROOT_FORWARD", port);
#endif
      break;

    /* 17.23.3 */
    case DESIGNATED_PROPOSE:
      port->proposing = True; /* in DESIGNATED_PROPOSE */
      port->newInfo = True;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("DESIGNATED_PROPOSE", port);
#endif
      break;
    case DESIGNATED_SYNCED:
      port->rrWhile = 0;
      port->synced = True; /* DESIGNATED_SYNCED */
      port->sync = False; /* DESIGNATED_SYNCED */
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("DESIGNATED_SYNCED", port);
#endif
      break;
    case DESIGNATED_RETIRED:
      port->reRoot = False; /* DESIGNATED_RETIRED */
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("DESIGNATED_RETIRED", port);
#endif
      break;
    case DESIGNATED_PORT:
      port->role = DesignatedPort;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("DESIGNATED_PORT", port);
#endif
      break;
    case DESIGNATED_LISTEN:
      port->learn = port->forward = False;
      port->fdWhile = stpm->rootTimes.ForwardDelay;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("DESIGNATED_LISTEN", port);
#endif
      break;
    case DESIGNATED_LEARN:
      port->learn = True;
      port->fdWhile = stpm->rootTimes.ForwardDelay;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("DESIGNATED_LEARN", port);
#endif
      break;
    case DESIGNATED_FORWARD:
      port->forward = True;
      port->fdWhile = 0;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("DESIGNATED_FORWARD", port);
#endif
      break;
#endif
/*mstp*/
    case NON_STP:
        port->role = NonStpPort ;
        break; 
    case ACTIVE_PORT :
        port->role = port->selectedRole ;
#ifdef STP_DBG
      if (this->debug)
        stp_port_trace_flags ("ACTIVE_PORT", port);
#endif
        break; 
    default :
        break; 
  };
}
    
Bool
MSTP_roletrns_check_conditions (STATE_MACH_T* this)
{
  register PORT_T           *port = this->owner.port;
  register STPM_T           *stpm;
  Bool                      allSynced;
  stpm = stp_stpm_get_the_cist() ;
  if ( stpm == NULL )
  {
     return  False;
  }

  if (BEGIN == this->State) {
    return stp_statmch_hop_2_state (this, INIT_PORT);
  }

  if (port->role != port->selectedRole &&
      port->selected &&
      ! port->updtInfo) {
    switch (port->selectedRole) {
      case DisabledPort:
      case AlternatePort:
      case BackupPort:
#if 0 /* def STP_DBG */
        if (this->debug) {
          stp_trace ("hop to BLOCK_PORT role=%d selectedRole=%d",
                                (int) port->role, (int) port->selectedRole);
        }
#endif
        return stp_statmch_hop_2_state (this, BLOCK_PORT);
      case RootPort:
      case DesignatedPort:
      case MasterPort :  /*mstp*/
        return stp_statmch_hop_2_state (this, ACTIVE_PORT);
        
      case NonStpPort: /*mstp*/
        return stp_statmch_hop_2_state (this, NON_STP);        
      default:
        return False;
    }
  }

  switch (this->State) {
    /* 17.23.1 */
    case INIT_PORT:
      return stp_statmch_hop_2_state (this, BLOCK_PORT);
    case BLOCK_PORT:
      if (!port->selected || port->updtInfo) break;
      if (!port->learning && !port->forwarding) {
        return stp_statmch_hop_2_state (this, BLOCKED_PORT);
      }
      break;
    case BLOCKED_PORT:
      if (!port->selected || port->updtInfo) break;
      if (port->fdWhile != stpm->rootTimes.ForwardDelay ||
          port->sync                ||
          port->reRoot              ||
          !port->synced) {
        return stp_statmch_hop_2_state (this, BLOCKED_PORT);
      }
      if (port->rbWhile != 2 * stpm->rootTimes.HelloTime &&
          port->role == BackupPort) {
        return stp_statmch_hop_2_state (this, BACKUP_PORT);
      }
      break;
    case BACKUP_PORT:
      return stp_statmch_hop_2_state (this, BLOCKED_PORT);
#if 0
    /* 17.23.2 */
    case ROOT_PROPOSED:
      return stp_statmch_hop_2_state (this, ROOT_PORT);
    case ROOT_AGREED:
      return stp_statmch_hop_2_state (this, ROOT_PORT);
    case REROOT:
      return stp_statmch_hop_2_state (this, ROOT_PORT);
    case ROOT_PORT:
      if (!port->selected || port->updtInfo) break;
      if (!port->forward && !port->reRoot) {
        return stp_statmch_hop_2_state (this, REROOT);
      }
      allSynced = stp_roletrns_compute_all_synced (port);
      if ((port->proposed && allSynced) ||
          (!port->synced && allSynced)) {
        return stp_statmch_hop_2_state (this, ROOT_AGREED);
      }
      if (port->proposed && !port->synced) {
        return stp_statmch_hop_2_state (this, ROOT_PROPOSED);
      }

      allReRooted = stp_roletrns_compute_re_rooted (port);
      if ((!port->fdWhile || 
           ((allReRooted && !port->rbWhile) && stpm->ForceVersion >=2)) &&
          port->learn && !port->forward) {
        return stp_statmch_hop_2_state (this, ROOT_FORWARD);
      }
      if ((!port->fdWhile || 
           ((allReRooted && !port->rbWhile) && stpm->ForceVersion >=2)) &&
          !port->learn) {
        return stp_statmch_hop_2_state (this, ROOT_LEARN);
      }

      if (port->reRoot && port->forward) {
        return stp_statmch_hop_2_state (this, REROOTED);
      }
      if (port->rrWhile != stpm->rootTimes.ForwardDelay) {
        return stp_statmch_hop_2_state (this, ROOT_PORT);
      }
      break;
    case REROOTED:
      return stp_statmch_hop_2_state (this, ROOT_PORT);
    case ROOT_LEARN:
      return stp_statmch_hop_2_state (this, ROOT_PORT);
    case ROOT_FORWARD:
      return stp_statmch_hop_2_state (this, ROOT_PORT);

    /* 17.23.3 */
    case DESIGNATED_PROPOSE:
      return stp_statmch_hop_2_state (this, DESIGNATED_PORT);
    case DESIGNATED_SYNCED:
      return stp_statmch_hop_2_state (this, DESIGNATED_PORT);
    case DESIGNATED_RETIRED:
      return stp_statmch_hop_2_state (this, DESIGNATED_PORT);
    case DESIGNATED_PORT:
      if (!port->selected || port->updtInfo) break;

      if (!port->forward && !port->agreed && !port->proposing && !port->operEdge) {
        return stp_statmch_hop_2_state (this, DESIGNATED_PROPOSE);
      }

      if (!port->rrWhile && port->reRoot) {
        return stp_statmch_hop_2_state (this, DESIGNATED_RETIRED);
      }
      
      if (!port->learning && !port->forwarding && !port->synced) {
        return stp_statmch_hop_2_state (this, DESIGNATED_SYNCED);
      }

      if (port->agreed && !port->synced) {
        return stp_statmch_hop_2_state (this, DESIGNATED_SYNCED);
      }
      if (port->operEdge && !port->synced) {
        return stp_statmch_hop_2_state (this, DESIGNATED_SYNCED);
      }
      if (port->sync && port->synced) {
        return stp_statmch_hop_2_state (this, DESIGNATED_SYNCED);
      }

      if ((!port->fdWhile || port->agreed || port->operEdge) &&
          (!port->rrWhile  || !port->reRoot) &&
          !port->sync &&
          (port->learn && !port->forward)) {
        return stp_statmch_hop_2_state (this, DESIGNATED_FORWARD);
      }
      if ((!port->fdWhile || port->agreed || port->operEdge) &&
          (!port->rrWhile  || !port->reRoot) &&
          !port->sync && !port->learn) {
        return stp_statmch_hop_2_state (this, DESIGNATED_LEARN);
      }
      if (((port->sync && !port->synced) ||
           (port->reRoot && port->rrWhile)) &&
          !port->operEdge && (port->learn || port->forward)) {
        return stp_statmch_hop_2_state (this, DESIGNATED_LISTEN);
      }
      break;
    case DESIGNATED_LISTEN:
      return stp_statmch_hop_2_state (this, DESIGNATED_PORT);
    case DESIGNATED_LEARN:
      return stp_statmch_hop_2_state (this, DESIGNATED_PORT);
    case DESIGNATED_FORWARD:
      return stp_statmch_hop_2_state (this, DESIGNATED_PORT);
#endif
    /* 17.23.2 */
    case PROPOSED:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case PROPOSING:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case AGREES:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case SYNCED:
        return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case REROOT:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case FORWARD:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case LISTEN:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case LEARN:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case REROOTED:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
    case ROOT:
      return stp_statmch_hop_2_state (this, ACTIVE_PORT);
      
    case ACTIVE_PORT:
        if ( !port->selected ||port->updtInfo ) break;

        if ((!port->learning && !port->forwarding && !port->synced &&
            (port->role != RootPort)) || (port->agreed && !port->synced) ||
            (port->operEdge && !port->synced) || (port->sync && port->synced)
           )
        {
            return stp_statmch_hop_2_state (this, SYNCED);
        }

        allSynced = stp_roletrns_compute_all_synced(port );
        if ( allSynced && ( port->proposed || !port->agree) )
        {
            return stp_statmch_hop_2_state (this, AGREES);
        }

        if ( !port->forward && !port->agreed && !port->proposing && 
             !port->operEdge &&  (port->role == DesignatedPort ) )
        {
            return stp_statmch_hop_2_state (this, PROPOSING);
        }

        if ( port->proposed && !port->agree )
        {
            return stp_statmch_hop_2_state (this, PROPOSED);
        }

        if (port->learn && !port->forward && ((port->fdWhile == 0) ||
           ((port->role == RootPort) && (stp_roletrns_compute_re_rooted(port) && (port->rbWhile == 0)) && (port->owner->ForceVersion >= 2)) ||
          ((port->role == DesignatedPort) && (port->agreed || port->operEdge) && ((port->rrWhile ==0) || !port->reRoot) && !port->sync) ||
          ((port->role == MasterPort) && allSynced)) )
         {
              return stp_statmch_hop_2_state (this, FORWARD);
          }

        if ( (port->rrWhile != stpm->rootTimes.ForwardDelay) && (port->role == RootPort ))
        {
            return stp_statmch_hop_2_state (this, ROOT);
        }

        if ( !port->forward && !port->reRoot && (port->role == RootPort) )
        {
            return stp_statmch_hop_2_state (this, REROOT);
        }

        #if 0
        if ( port->reRoot && (((port->role == RootPort) && port->forward) /* || (port->rrWhile == 0)*/)  )
        #endif
        if ( port->reRoot && (((port->role == RootPort) && port->forward)  || (port->rrWhile == 0))  )
        {
            return stp_statmch_hop_2_state (this, REROOTED);
        }
      
        if (!port->learn && ((port->fdWhile == 0) ||
            ((port->role == RootPort) && (stp_roletrns_compute_re_rooted(port) && (port->rbWhile == 0)) && (port->owner->ForceVersion >= 2)) ||
            ((port->role == DesignatedPort) && (port->agreed || port->operEdge) && ((port->rrWhile ==0) || !port->reRoot) && !port->sync) ||
            ((port->role == MasterPort) && allSynced)) )
        {
            return stp_statmch_hop_2_state (this, LEARN);
        }
        
      if ( (port->learn || port->forward) && !port->operEdge && (port->role != RootPort && port->role != MasterPort) && 
           ((port->sync && !port->synced) || (port->reRoot && (port->rrWhile != 0))) )
      {
          return stp_statmch_hop_2_state (this, LISTEN);
      }

      break;
      
    default :
        break;
  };

  return False;
}

#ifdef __cplusplus
}
#endif

