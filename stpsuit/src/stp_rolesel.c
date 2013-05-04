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
* stp_rolesel.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for Port Role Selection state machine in stp module
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

/* Port Role Selection state machine : 17.22 */

#include "stp_base.h"
#include "stp_stpm.h"
#include "stp_log.h"

#define STATES { \
  CHOOSE(INIT_BRIDGE),      \
  CHOOSE(RECEIVE),	\
}

#define GET_STATE_NAME STP_rolesel_get_state_name
#include "stp_choose.h"

#ifdef STP_DBG
void stp_rolesel_dbg_break_point (PORT_T * port, STPM_T* stpm)
{
}
#endif

static Bool
stp_rolesel_is_backup_port (PORT_T* port, STPM_T* this)
{
  if (!stp_vector_compare_bridge_id
      (&port->portPrio.design_bridge, &this->BrId)) {
#if 0 /* def STP_DBG */
    if (port->info->debug) {
      stp_vector_br_id_print ("portPrio.design_bridge",
                            &port->portPrio.design_bridge, True);
      stp_vector_br_id_print ("            this->BrId",
                            &this->BrId, True);
    }
    stp_rolesel_dbg_break_point (port, this);
#endif
    return True;
  } else {
    return False;
  }
}

static void
stp_rolesel_setRoleSelected (char* reason, STPM_T* stpm, PORT_T* port,
                PORT_ROLE_T newRole)
{
  char* new_role_name;

  port->selectedRole = newRole;

  if (newRole == port->role)
    return;

  switch (newRole) {
    case DisabledPort:
      new_role_name = "Disabled";
      break;
    case AlternatePort:
      new_role_name = "Alternate";
      break;
    case BackupPort:
      new_role_name = "Backup";
      break;
    case RootPort:
      new_role_name = "Root";
      break;
    case DesignatedPort:
      new_role_name = "Designated";
      break;
/*mstp*/
    case MasterPort :
      new_role_name = "Master";
      break;        
    case NonStpPort:
      new_role_name = "NonStp";
      port->role = newRole;
      break;
    default:
      stp_trace ("%s-%s:port %s => Unknown (%d ?)",
                 reason, stpm->name, port->port_name, (int) newRole);
      return;
  }
  /*端口角色发生变化后，其相应的参与状态变化的信息应该
  被清为初始状态*/
  port->proposing = port->proposed = False;
  port->agree = port->agreed = False;
  port->sync = port->synced = False;
#ifdef STP_DBG
  if (port->roletrns->debug)
    stp_trace ("%s(%s-%s) => %s",
               reason, stpm->name, port->port_name, new_role_name);
#endif
}

static void
stp_rolesel_updtRoleDisableTree (STPM_T* this)
{               /* 17.10.20 */
  register PORT_T *port;

  for (port = this->ports; port; port = port->next) {
    port->selectedRole = DisabledPort;
  }
}

static void
stp_rolesel_clearReselectTree (STPM_T* this)
{               /* 17.19.1 */
  register PORT_T *port;

  for (port = this->ports; port; port = port->next) {
    port->reselect = False;
  }
}

static void
stp_rolesel_updtRootPrio (STATE_MACH_T* this)
{
  PRIO_VECTOR_T rootPathPrio;   /* 17.4.2.2 */
  register PORT_T *port, *cist_port;
  register STPM_T *stpm;
  register unsigned int dm;

  stpm = this->owner.stpm;

  for (port = stpm->ports; port; port = port->next) {
    if (port->admin_non_stp) {
      continue;
    }
	//STP_LOG (STP_LOG_DEBUG,"port[%02x] port->info =%d\n",port->port_index,port->infoIs);
    if (Disabled == port->infoIs)
      continue;
    if (Aged == port->infoIs)
      continue;
    if (Mine == port->infoIs) {
/*mstp*/
#if 0 /* def STP_DBG */
      stp_rolesel_dbg_break_point (port); /* for debugger break point */
#endif
      continue;
    }
    #if 0
    /* exculde backup port from computing */
    if (!memcmp( &port->portPrio.design_bridge.addr, &stpm->BrId.addr, 6 ))
    {
        continue;
    }
    #else
	if (!stp_vector_compare_bridge_id (&port->portPrio.design_bridge, &stpm->BrId))
	{
        continue;
    }
	#endif
    /*msti:如果IST是boundary port ，不参与计算*/
    if ( stpm->vlan_id != STP_CIST_ID)
    {
        cist_port = stp_port_mst_findport( STP_CIST_ID , port->port_index ) ;
        if(cist_port == NULL || (cist_port->infoInternal == 0 && cist_port->infoIs == Received))
            continue;
    }

    #ifdef STP_DBG
      if ( port->roletrns->debug )
      {
          char title[64];
          
          sprintf(title,"stp_rolesel_UpdtRootPrio: Port %d portPrio: \r\n", 
               port->port_index);
          
         stp_vector_print(title,&port->portPrio);    
      }
    #endif      
/*mstp*/

    stp_vector_copy (&rootPathPrio, &port->portPrio);
    if (  stpm->vlan_id  == STP_CIST_ID ) /* CIST */
    {
        if ( port->infoInternal== True )
        {
            rootPathPrio.region_root_path_cost += port->operPCost;
        }
        else
        {
            rootPathPrio.root_path_cost += port->operPCost;
            memcpy(&rootPathPrio.region_root_bridge , &port->owner->BrId , sizeof(BRIDGE_ID));
            rootPathPrio.region_root_path_cost = 0 ;
        }
    }
    else
    {
        rootPathPrio.region_root_path_cost += port->operPCost;
    }

    if (stp_vector_compare (&rootPathPrio, &stpm->rootPrio) < 0) {
      stp_vector_copy (&stpm->rootPrio, &rootPathPrio);
      stp_times_copy (&stpm->rootTimes, &port->portTimes);
      /*dm = (8 +  stpm->rootTimes.MaxAge) / 16;
      if (!dm)
        dm = 1;
      stpm->rootTimes.MessageAge += dm;*/
#ifdef STP_DBG
      if (port->roletrns->debug)
          stp_trace ("stp_rolesel_updtRootPrio: dm=%d rootTimes.MessageAge=%d on port %s",
                 (int) dm, (int) stpm->rootTimes.MessageAge,
                 port->port_name);
#endif
    }
  }
}

#if 0
static void
updtRolesBridge (STATE_MACH_T* this)
{               /* 17.19.21 */
  register PORT_T* port;
  register STPM_T* stpm;
  PORT_ID old_root_port; /* for tracing of root port changing */

  stpm = this->owner.stpm;
  old_root_port = stpm->rootPortId;    /*no initation*/

  /*calc STPM_T rootPrio*/
  stp_vector_create (&stpm->rootPrio, &stpm->BrId, 0, &stpm->BrId, 0, 0);
  stp_times_copy (&stpm->rootTimes, &stpm->BrTimes);
  stpm->rootPortId = 0;

  /*find best rootPrio*/
  STP_LOG(STP_LOG_DEBUG,"RSTP>>\n<<<<<< ROLESEL STATE >>>>>\n");
  stp_rolesel_updtRootPrio (this);

  /*created new every ports' designPrio */
  for (port = stpm->ports; port; port = port->next) {
    if (port->admin_non_stp) {
      continue;
    }
    stp_vector_create (&port->designPrio,
             &stpm->rootPrio.root_bridge,
             stpm->rootPrio.root_path_cost,
             &stpm->BrId, port->port_id, port->port_id);
    stp_times_copy (&port->designTimes, &stpm->rootTimes);


#ifdef STP_DBG
    
    if (port->roletrns->debug) {
	  stp_vector_br_id_print ("ch:designPrio.root_bridge",
                            &port->designPrio.root_bridge, True);
      stp_vector_br_id_print ("ch:designPrio.design_bridge",
                            &port->designPrio.design_bridge, True);
    }
	STP_LOG(STP_LOG_DEBUG,"RSTP>>port->designPrio.root_path_cost = %d \n \
		port->designPrio.design_port = %d \n \
		port->designPrio.design_port = %d \n",
		port->designPrio.root_path_cost,port->designPrio.design_port,port->designPrio.bridge_port);
#endif

  }

  stpm->rootPortId = stpm->rootPrio.bridge_port;

#ifdef STP_DBG
  if (old_root_port != stpm->rootPortId) {
    if (! stpm->rootPortId) {
      stp_trace ("\nbrige %s became root", stpm->name);
    } else {
      stp_trace ("\nbrige %s new root port: %s",
        stpm->name,
        stp_stpm_get_port_name_by_id (stpm, stpm->rootPortId));
    }
  }
#endif

  /*config ports <selectedRole>*/
  for (port = stpm->ports; port; port = port->next) {
    if (port->admin_non_stp) {
      stp_rolesel_setRoleSelected ("Non", stpm, port, NonStpPort);
      port->forward = port->learn = True;
      continue;
    }

    switch (port->infoIs) {
      case Disabled:
        stp_rolesel_setRoleSelected ("Dis", stpm, port, DisabledPort);
        break;
      case Aged:
        setRoleSelected ("Age", stpm, port, DesignatedPort);
        port->updtInfo = True;
        break;
      case Mine:
        stp_rolesel_setRoleSelected ("Mine", stpm, port, DesignatedPort);
        if (0 != stp_vector_compare (&port->portPrio,
                      &port->designPrio) ||
            0 != stp_times_compare (&port->portTimes,
                  &port->designTimes)) {
            port->updtInfo = True;
        }
        break;
      case Received:
        if (stpm->rootPortId == port->port_id) {
          stp_rolesel_setRoleSelected ("Rec", stpm, port, RootPort);
        } else if (stp_vector_compare (&port->designPrio, &port->portPrio) < 0) {
          /* Note: this important piece has been inserted after
           * discussion with Mick Sieman and reading 802.1y Z1 */
          stp_rolesel_setRoleSelected ("Rec", stpm, port, DesignatedPort);
          port->updtInfo = True;
          break;
        } else {
          if (stp_rolesel_is_backup_port (port, stpm)) {
            stp_rolesel_setRoleSelected ("rec", stpm, port, BackupPort);
          } else {
            stp_rolesel_setRoleSelected ("rec", stpm, port, AlternatePort);
          }
        }
        port->updtInfo = False;
        break;
      default:
        stp_trace ("undef infoIs=%d", (int) port->infoIs);
        break;
    }
  }

	STP_LOG(STP_LOG_DEBUG,"RSTP>><<<<<<<<<END>>>> >>>>>\n");

}
#endif

inline static void stp_rolesel_updtRolesCist (STATE_MACH_T* this)  /* 13.26.25  */
{
  register PORT_T* port;
  register STPM_T* stpm;
  PORT_ID old_root_port; /* for tracing of root port changing */
  int had_cist_pathcost =0;
  unsigned char old_region_root[6];
  
  Bool bUpdateMsti = False; /*force msti to reslect */

  stpm = this->owner.stpm;
  old_root_port = stpm->rootPortId;

  had_cist_pathcost = stpm->rootPrio.root_path_cost;
  memcpy( old_region_root, stpm->rootPrio.region_root_bridge.addr, 6);

  stp_vector_create (&stpm->rootPrio, &stpm->BrId, 0,  &stpm->BrId  , 0, &stpm->BrId, 0, 0);
  stp_times_copy (&stpm->rootTimes, &stpm->BrTimes);
  stpm->rootPortId = 0;

  stp_rolesel_updtRootPrio (this);

  for (port = stpm->ports; port; port = port->next) {
    if (port->admin_non_stp) {
      continue;
    }
    stp_vector_create (&port->designPrio,
             &stpm->rootPrio.root_bridge,
             stpm->rootPrio.root_path_cost,
             &stpm->rootPrio.region_root_bridge,
             stpm->rootPrio.region_root_path_cost,
             &stpm->BrId, port->port_id, port->port_id);
    if ( !port->sendRSTP)
    {
       memcpy(&port->designPrio.region_root_bridge , &port->owner->BrId , sizeof(BRIDGE_ID)) ;
    }
    stp_times_copy (&port->designTimes, &stpm->rootTimes);

  }

  stpm->rootPortId = stpm->rootPrio.bridge_port;

#ifdef STP_DBG
  if (old_root_port != stpm->rootPortId) {
    if (! stpm->rootPortId) {
      stp_trace ("\nbrige %s became root", stpm->name);
    } else {
      stp_trace ("\nbrige %s new root port: %s",
        stpm->name,
        stp_stpm_get_port_name_by_id (stpm, stpm->rootPortId));
    }
	
  }
#endif
  /*13.24.3*/
  if ( (stpm->rootPrio.root_path_cost || had_cist_pathcost) &&  /* has or had */ 
     0 != memcmp(old_region_root , stpm->rootPrio.region_root_bridge.addr , 6) 
  )
  {
     for ( port = stpm->ports; port; port = port->next)
     {
        register PORT_T *mstPort = port->nextMst;
        for ( ; mstPort; mstPort = mstPort->nextMst)
            mstPort->changedMaster = True ;
     }
  }

  for (port = stpm->ports; port; port = port->next) {
    if (port->admin_non_stp) {
      stp_rolesel_setRoleSelected ("Non", stpm, port, NonStpPort);
      port->forward = port->learn = True;
      continue;
    }

    switch (port->infoIs) {
      case Disabled:
        stp_rolesel_setRoleSelected ("Dis", stpm, port, DisabledPort);
        break;
      case Aged:
        stp_rolesel_setRoleSelected ("Age", stpm, port, DesignatedPort);
        port->updtInfo = True;
        break;
      case Mine:
        stp_rolesel_setRoleSelected ("Mine", stpm, port, DesignatedPort);
        if (0 != stp_vector_compare (&port->portPrio,
                      &port->designPrio) ||
            0 != stp_times_compare (&port->portTimes,
                  &port->designTimes)) 
            port->updtInfo = True;
            break;

      case Received:
        if (stpm->rootPortId == port->port_id) {
          stp_rolesel_setRoleSelected ("Rec", stpm, port, RootPort);
        } else if (stp_vector_compare (&port->designPrio, &port->portPrio) < 0) {
          /* Note: this important piece has been inserted after
           * discussion with Mick Sieman and reading 802.1y Z1 */
          stp_rolesel_setRoleSelected ("Rec", stpm, port, DesignatedPort);
          port->updtInfo = True;
          break;
        } else {
          if (stp_rolesel_is_backup_port (port, stpm)) {
            stp_rolesel_setRoleSelected ("rec", stpm, port, BackupPort);
          } else {
            stp_rolesel_setRoleSelected ("rec", stpm, port, AlternatePort);
          }
        }
        port->updtInfo = False;
        break;
      default:
        stp_trace ("undef infoIs=%d", (int) port->infoIs);
        break;
    }
    if(!port->infoInternal || !port->rcvdInternal)
   {/*对于boundary端口，因为边界端口的msti是不可能收到任何信息的，
       所以其msti上的端口角色的变化，不是由于报文触发，而是由相应
       的cist端口变化触发，因此在此处将该端口的所有msti端口设置为重新
       计算生成树状态*/
      register PORT_T *mstport =port->nextMst;
      for( ; mstport; mstport = mstport->nextMst)
      {
          mstport->reselect = True;
          mstport->selected = False;
      }
   }

  }
  
}

inline static void stp_rolesel_updtRolesMsti (STATE_MACH_T* this)  /* 13.26.26  */
{
  register PORT_T* port;
  register STPM_T* stpm;
  PORT_ID old_root_port; /* for tracing of root port changing */
  BRIDGE_ID null_br;
  Bool bUpdateMsti = False; /*force msti to reslect */

  stpm = this->owner.stpm;
  old_root_port = stpm->rootPortId;

  memset(&null_br, 0, sizeof(BRIDGE_ID));
  stp_vector_create (&stpm->rootPrio, &null_br, 0, &stpm->BrId , 0, &stpm->BrId, 0, 0);
  stp_times_copy (&stpm->rootTimes, &stpm->BrTimes);
  stpm->rootPortId = 0;

  stp_rolesel_updtRootPrio (this);

  for (port = stpm->ports; port; port = port->next) {
    if (port->admin_non_stp) {
      continue;
    }
    stp_vector_create (&port->designPrio,
             &stpm->rootPrio.root_bridge,
             stpm->rootPrio.root_path_cost,
             &stpm->rootPrio.region_root_bridge,
             stpm->rootPrio.region_root_path_cost,
             &stpm->BrId, port->port_id, port->port_id);
    stp_times_copy (&port->designTimes, &stpm->rootTimes);

  }

  stpm->rootPortId = stpm->rootPrio.bridge_port;

#ifdef STP_DBG
  if (old_root_port != stpm->rootPortId) {
    if (! stpm->rootPortId) {
      stp_trace ("\nbrige %s became root", stpm->name);
    } else {
      stp_trace ("\nbrige %s new root port: %s",
        stpm->name,
        stp_stpm_get_port_name_by_id (stpm, stpm->rootPortId));
    }
	
  }
#endif

  for (port = stpm->ports; port; port = port->next) {

    PORT_T *cistport =stp_port_mst_findport(STP_CIST_ID, port->port_index);
  
    if (port->admin_non_stp) {
      stp_rolesel_setRoleSelected ("Non", stpm, port, NonStpPort);
      port->forward = port->learn = True;
      continue;
    }
    
    if ( cistport == NULL )
    {
      return ;
    }
    if ( cistport->portEnabled && cistport->infoIs == Received && 
      cistport->infoInternal == False )  
    {
      if (cistport->selectedRole == RootPort )
      {
        stp_rolesel_setRoleSelected ("Master", stpm, port, MasterPort);
      }
      else if (cistport->selectedRole == AlternatePort )
      {
        stp_rolesel_setRoleSelected ("Alt", stpm, port, AlternatePort);                
      }
      if (0 != stp_vector_compare (&port->portPrio, &port->designPrio) ||
          0 != stp_times_compare (&port->portTimes, &port->designTimes))
      {
        port->updtInfo = True;
      }
	  
    } else{
      switch (port->infoIs) {
        case Disabled:
          stp_rolesel_setRoleSelected ("Dis", stpm, port, DisabledPort);
        break;
        case Aged:
          stp_rolesel_setRoleSelected ("Age", stpm, port, DesignatedPort);
          port->updtInfo = True;
        break;
        case Mine:
          stp_rolesel_setRoleSelected ("Mine", stpm, port, DesignatedPort);
          if (0 != stp_vector_compare (&port->portPrio, &port->designPrio) ||
              0 != stp_times_compare (&port->portTimes, &port->designTimes))
          {
            port->updtInfo = True;
          }
        break;
        case Received:
          if (stpm->rootPortId == port->port_id) {
            stp_rolesel_setRoleSelected ("Rec", stpm, port, RootPort);
          } else if (stp_vector_compare (&port->designPrio, &port->portPrio) < 0) {
          /* Note: this important piece has been inserted after
          * discussion with Mick Sieman and reading 802.1y Z1 */
            stp_rolesel_setRoleSelected ("Rec", stpm, port, DesignatedPort);
            port->updtInfo = True;
          break;
          } else {
            if (stp_rolesel_is_backup_port (port, stpm)) {
              stp_rolesel_setRoleSelected ("rec", stpm, port, BackupPort);
            } else {
              stp_rolesel_setRoleSelected ("rec", stpm, port, AlternatePort);
            }
          }
          port->updtInfo = False;
        break;
        default:
        if(this->debug)
        stp_trace ("Undef infoIs=%d", (int) port->infoIs);
        break;
      }
    }
  }   
}

static void
stp_rolesel_updtRolesXst (STATE_MACH_T* this)
{               /* 17.19.21 */
   if(STP_CIST_ID == this->owner.stpm->vlan_id)
     stp_rolesel_updtRolesCist( this);
   else
     stp_rolesel_updtRolesMsti( this);
}

static Bool
stp_rolesel_setSelectedTree (STPM_T* this)
{
  register PORT_T* port;

  for (port = this->ports; port; port = port->next) {
    if (port->reselect) {
#ifdef STP_DBG
      stp_trace ("setSelectedBridge: TRUE=reselect on port %s", port->port_name);
#endif
      return False;
    }
  }

  for (port = this->ports; port; port = port->next) {
    port->selected = True;
  }

  return True;
}

void
MSTP_rolesel_enter_state (STATE_MACH_T* this)
{
  STPM_T* stpm;

  stpm = this->owner.stpm;

  switch (this->State) {
    case BEGIN:
    case INIT_BRIDGE:
      stp_rolesel_updtRoleDisableTree (stpm);
      break;
#if 0
    case ROLE_SELECTION:
      clearReselectBridge (stpm);
      updtRolesBridge (this);
	  /*config ports <selected = True>*/
      setSelectedBridge (stpm);
      break;
#endif
    case RECEIVE:
      stp_rolesel_clearReselectTree (stpm);
      stp_rolesel_updtRolesXst (this);
      stp_rolesel_setSelectedTree (stpm);
      break;
  }
}

Bool
MSTP_rolesel_check_conditions (STATE_MACH_T* s)
{
  STPM_T* stpm;
  register PORT_T* port;

  /*if (BEGIN == s->State) {
    stp_statmch_hop_2_state (s, INIT_BRIDGE);
  }*/

  switch (s->State) {
    case BEGIN:
      return stp_statmch_hop_2_state (s, INIT_BRIDGE);
    case INIT_BRIDGE:
      return stp_statmch_hop_2_state (s, RECEIVE);
    case RECEIVE:
      stpm = s->owner.stpm;
      for (port = stpm->ports; port; port = port->next) {
        if (port->reselect) {
          /* stp_trace ("reselect on port %s", port->port_name); */
          return stp_statmch_hop_2_state (s, RECEIVE);
        }
      }
      break;
  }

  return False;
}

void
stp_rolesel_update_stpm (STPM_T* this)
{
  register PORT_T* port;
  PRIO_VECTOR_T rootPathPrio;   /* 17.4.2.2 */

  stp_trace ("%s", " stp_rolesel_update_stpm ");
  stp_vector_create (&rootPathPrio, &this->BrId, 0,  &this->BrId, 0, &this->BrId, 0, 0);

  if (!this->rootPortId ||
      stp_vector_compare (&rootPathPrio, &this->rootPrio) < 0) {
    stp_vector_copy (&this->rootPrio, &rootPathPrio);
  }

  for (port = this->ports; port; port = port->next) {
    stp_vector_create (&port->designPrio,
             &this->rootPrio.root_bridge,
             this->rootPrio.root_path_cost,
/*mstp*/
             &this->rootPrio.region_root_bridge,
             this->rootPrio.region_root_path_cost,
            &this->BrId, port->port_id, port->port_id);
/*mstp*/
    if (Received != port->infoIs || this->rootPortId == port->port_id) {
      stp_vector_copy (&port->portPrio, &port->designPrio);
    }
    port->reselect = True;
    port->selected = False;
  }
}
#ifdef __cplusplus
}
#endif

