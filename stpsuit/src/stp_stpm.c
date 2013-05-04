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
* stp_stpm.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for stp instances in stp module
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

 /* STP machine instance : bridge per VLAN: 17.17 */
 
#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_to.h" /* for stp_to_flush_lt */
#include "stp_in.h"
#include "stp_log.h"
#include "stp_statmch.h"

static STPM_T *bridges = NULL;
extern Bool g_flag;
/*mstp*/
static STPM_T *g_CISTBRIDGE = NULL;

static void stp_stpm_mst_update_transmit( STPM_T* this );
static void stp_stpm_mst_update_receive( PORT_T *port);
static void stp_stpm_mst_init_transmit( STPM_T* this );
static void stp_stpm_mst_init_receive( STPM_T* this );
void stp_stpm_mst_update ( PORT_T *port) ;
/*mstp*/
static int
stp_stpm_init_machine (STATE_MACH_T* this)
{
  this->State = BEGIN;
  stp_syslog_dbg("RSTP>>%s state call concreteEnterState\n",this->name);
  (*(this->concreteEnterState)) (this);
  return 0;
}

static int
stp_stpm_iterate_machines (STPM_T* this,
                           int (*iter_callb) (STATE_MACH_T*),
                           Bool exit_on_non_zero_ret)
{
  register STATE_MACH_T* stater;
  register PORT_T*       port;
  int                    iret, mret = 0;

  /* state machines per bridge */
  for (stater = this->machines; stater; stater = stater->next) {
	/*stp_stpm_init_machine*/
	//printf("stpm 53: machine name %s\n",stater->name);
    iret = (*iter_callb) (stater);
    if (exit_on_non_zero_ret && iret)
      return iret;
    else
      mret += iret;
  }

	//printf("stpm 61:: start port machine \n");
  /* state machines per port */
  for (port = this->ports; port; port = port->next) {
    for (stater = port->machines; stater; stater = stater->next) {
		//printf("stpm 64:: port_index[%d],machine name %s\n",stater->name);
      iret = (*iter_callb) (stater);
      if (exit_on_non_zero_ret && iret)
        return iret;
      else
        mret += iret;
    }
  }
  
  return mret;
}

void
stp_stpm_init_data (STPM_T* this)
{

	stp_syslog_dbg("RSTP>>this->BrId.prio= %d ,MAC ADDR : %2x %2x %2x %2x %2x %2x\n",
		this->BrId.prio,this->BrId.addr[0],this->BrId.addr[1],this->BrId.addr[2],this->BrId.addr[3],
		this->BrId.addr[4],this->BrId.addr[5]);
	
  stp_vector_create (&this->rootPrio,
                   &this->BrId,   /*mstp*/
                   0,
                   &this->BrId,
                   0,
                   &this->BrId,
                   0, 0);

  this->BrTimes.MessageAge = 0;

  stp_times_copy (&this->rootTimes, &this->BrTimes);
}

static unsigned char
stp_stpm_check_topoch (STPM_T* this)
{
  register PORT_T*  port;
  
  for (port = this->ports; port; port = port->next) {
    if (port->tcWhile) {
      return 1;
    }
  }
  return 0;
}

#if 0
static int
_stp_stpm_new_iterate_machines(STPM_T * this, PORT_T *port,int (* iter_callb)(STATE_MACH_T *),Bool exit_on_non_zero_ret)
{
  register STATE_MACH_T* stater;
  int                    iret, mret = 0;

  /* state machines per bridge */
  for (stater = this->machines; stater; stater = stater->next) {
	/*stp_stpm_init_machine*/
    iret = (*iter_callb) (stater);
    if (exit_on_non_zero_ret && iret)
      return iret;
    else
      mret += iret;
  }

  /* state machines per port */
    for (stater = port->machines; stater; stater = stater->next) {
      iret = (*iter_callb) (stater);
      if (exit_on_non_zero_ret && iret)
        return iret;
      else
        mret += iret;
    }
  
  return mret;

}

void
stp_stpm_one_second (STPM_T* param)
{
  STPM_T*           this = (STPM_T*) param;
  register PORT_T*  port;
  register int      iii;

  if (STP_ENABLED != this->admin_state) return;

  for (port = this->ports; port; port = port->next) {
    for (iii = 0; iii < TIMERS_NUMBER; iii++) {
      if (*(port->timers[iii]) > 0) {
        (*port->timers[iii])--;
      }
    }    
    port->uptime++;
  }
	
  stp_stpm_update (this);
  STP_LOG(STP_LOG_DEBUG,"RSTP>>===================END=====================\n");
  /*check tcWhile ,Topo_Change = tcWhile?1:0*/
  this->Topo_Change = stp_stpm_check_topoch (this);
  if (this->Topo_Change) {
    this->Topo_Change_Count++;
    this->timeSince_Topo_Change = 0;
  } else {
    this->Topo_Change_Count = 0;
    this->timeSince_Topo_Change++;
  }
}
#endif
/*mstp stp_stpm_one_second (void)
*/
int
stp_stpm_one_second (void)
{
  register STPM_T* this;
  register int     dbg_cnt = 0;
  register PORT_T*  port;
  register int      iii;

  for (this = stp_stpm_get_the_list (); this; this = this->next) {
    if (STP_ENABLED != this->admin_state ) continue;
    dbg_cnt ++;
    for (port = this->ports; port; port = port->next) {
      for (iii = 0; iii < TIMERS_NUMBER; iii++) {
        if (*(port->timers[iii]) > 0) {
          (*port->timers[iii])--;
        }
      }    
      port->uptime++;
    }  
    this->Topo_Change = stp_stpm_check_topoch (this);
    if (this->Topo_Change) {
      this->Topo_Change_Count++;
      this->timeSince_Topo_Change = 0;
    } else {
      this->Topo_Change_Count = 0;
      this->timeSince_Topo_Change++;
    }
  }

  if(dbg_cnt)
    stp_stpm_mst_update (NULL);
  
  return dbg_cnt;
}

STPM_T*
stp_stpm_get_the_cist (void)
{
  return g_CISTBRIDGE;
}

STPM_T*
stp_stpm_get_instance (unsigned int mstid)
{
  register STPM_T * stpm = NULL;
  for(stpm = bridges; stpm; stpm = stpm->next)
  {
    if(stpm->vlan_id == mstid)
      return stpm;
  }
  return NULL;
}

#if 0
STPM_T*
STP_stpm_create (int vlan_id, char* name)
{
  STPM_T* this;

  STP_NEW_IN_LIST(this, STPM_T, bridges, "stp instance");

  this->admin_state = STP_DISABLED;
  
  this->vlan_id = vlan_id;
  if (name) {
    STP_STRDUP(this->name, name, "stp bridge name");
  }

  this->machines = NULL;
  this->ports = NULL;

  STP_STATE_MACH_IN_LIST(rolesel);

#ifdef STP_DBG
  /* this->rolesel->debug = 2;*/  
#endif

  return this;
}
#endif
STPM_T* stp_stpm_mst_create (unsigned int mstid, char* name)
{
	STPM_T * this;
	UID_STP_CFG_T cfg;

	if ( mstid > STP_MSTI_MAX)
	{
	    return NULL ;
	}

	if(mstid == STP_CIST_ID)
	{
	    STP_NEW_IN_LIST(this, STPM_T, bridges, "stp instance");
	    g_CISTBRIDGE = bridges;
	}
	else if(g_CISTBRIDGE)
	{
	    STP_NEW_IN_LIST(this, STPM_T, g_CISTBRIDGE->next, "stp instance");
	}
	else
	{
	    return NULL;
	}

	this->admin_state = STP_DISABLED;

	this->vlan_id = mstid;

	if (name) {
	  memcpy(this->name, name, 32);
	}

	this->machines = NULL;
	this->ports = NULL;
	memset(&this->vlan_map, 0, sizeof(VLAN_MAP_T));
	MSTP_STATE_MACH_IN_LIST( rolesel, True);
	if ( NULL == this->rolesel )
	{
	    stp_stpm_delete( this );
	    return NULL;
	}
	memset(&cfg, 0, sizeof(UID_STP_CFG_T));
	stp_to_get_init_stpm_cfg (mstid, &cfg);

	this->BrId.prio = DEF_BR_PRIO ;

	stp_syslog_dbg("stpm307:: instance id %d\n",this->vlan_id);
	if ( mstid == STP_CIST_ID)
	{
	  this->ForceVersion = NORMAL_MSTP ;
	  this->BrTimes.MessageAge = 0;
	  this->BrTimes.MaxAge = cfg.max_age;
	  this->BrTimes.ForwardDelay = cfg.forward_delay;
	  this->BrTimes.HelloTime = cfg.hello_time ;
	  this->BrTimes.RemainingHops = DEF_REMAINING_HOPS ;
	}
	else
	{
	  STPM_T * cist_bridge = NULL ;
	  this->BrTimes.MessageAge = 0;
	  this->BrTimes.MaxAge = 0 ;
	  this->BrTimes.ForwardDelay = 0 ;
	  this->BrTimes.HelloTime = 0 ;
	  this->BrTimes.RemainingHops = DEF_REMAINING_HOPS ;
	  
	  this->BrId.prio += ( 0x0fff & mstid ); /* 13.23.2 */

	  cist_bridge = stp_stpm_get_the_cist() ;
	  if ( cist_bridge != NULL )
	  {
	    this->ForceVersion = cist_bridge->ForceVersion;
	    this->BrTimes.RemainingHops = cist_bridge->BrTimes.RemainingHops;
	    memcpy( this->BrId.addr , cist_bridge->BrId.addr , 6 );
	  }
	  else
	  {
	    this->BrTimes.HelloTime = DEF_BR_HELLOT;
	    this->BrTimes.RemainingHops = DEF_REMAINING_HOPS ;
	  }
	}	
	return this;

}

int
stp_stpm_enable (STPM_T* this, UID_STP_MODE_T admin_state)
{
  int rc = 0;

  if (admin_state == this->admin_state) {
    return rc;
  }

	
  if (STP_ENABLED == admin_state) {
    rc = stp_stpm_start (this);
    this->admin_state = admin_state;
  } else {
    this->admin_state = admin_state;
    stp_stpm_stop (this);
  }
  
  return rc;
}

void
stp_stpm_delete (STPM_T* this)
{
	register STPM_T*       tmp;
	register STPM_T*       prev;
	register STATE_MACH_T* stater;
	register PORT_T*       port;
	register void*         pv;

	stp_stpm_enable (this, STP_DISABLED);

	for (stater = this->machines; stater; ) {
		pv = (void*) stater->next;
		stp_statmch_delete (stater);
		this->machines = stater = (STATE_MACH_T*) pv;
	}

	for (port = this->ports; port; port = port->next) {
		pv = (void*) port->next;
		stp_port_delete (port);
		this->ports = port = (PORT_T*) pv;
	}

	prev = NULL;
	for (tmp = bridges; tmp; tmp = tmp->next) {
		if (tmp->vlan_id == this->vlan_id) {
			if (prev) {
				prev->next = tmp->next;
			} else {
				bridges = tmp->next;
			}

			if(this->vlan_id == STP_CIST_ID)
				g_CISTBRIDGE = NULL;

			printf("free stpm mstid %d \n",this->vlan_id);
			STP_FREE(this, "stp instance");
			break;
		}
		prev = tmp;
	}

	return;
}

int
stp_stpm_start (STPM_T* this)
{
	register PORT_T* port;
#if 0
	if (! this->ports) { /* there are not any ports :( */
	  return STP_There_Are_No_Ports;
	}
#endif
	if( this->vlan_id == STP_CIST_ID)
	{
	  STPM_T *stpm = NULL;
	              
	  if (!stp_stpm_compute_bridge_id (this)) {/* can't compute bridge id ? :( */
			stp_syslog_dbg("file %s, lines %d enable fialed\n",__FILE__,__LINE__);
			return STP_Cannot_Compute_Bridge_Prio;
	  }

	  for(stpm = g_CISTBRIDGE->next; stpm; stpm = stpm->next)
	  {
	    memcpy( stpm->BrId.addr , this->BrId.addr , 6 );
	  }
	}

#if 0
	/* check, that the stpm has unique bridge Id */
	if (0 != stp_stpm_check_bridge_priority (this)) {
	  /* there is an enabled bridge with same ID :( */
	  return STP_Invalid_Bridge_Priority;
	}
#endif
	stp_stpm_init_data (this);

	for (port = this->ports; port; port = port->next) {
	  stp_port_init (port, this, True);
	}

#ifndef STRONGLY_SPEC_802_1W
	/* A. see comment near STRONGLY_SPEC_802_1W in topoch.c */
	/* B. port=0 here means: delete for all ports */
#ifdef STP_DBG
	stp_trace("%s (all, start stpm)",
	      "clearFDB");
#endif

	stp_to_flush_lt (0, this->vlan_id, LT_FLASH_ONLY_THE_PORT, "start stpm");
#endif
	if(STP_CIST_ID == this->vlan_id)
	{
	    stp_stpm_mst_init_receive( this);
	    stp_stpm_mst_init_transmit( this);
	  	stp_syslog_dbg("stpm.c::467 MSTID %d\n",this->vlan_id);
	}
	stp_stpm_iterate_machines (this, stp_stpm_init_machine, False);

	if(STP_CIST_ID == this->vlan_id)
	    stp_stpm_mst_update_receive( NULL);

	stp_stpm_update (this);

	if(STP_CIST_ID == this->vlan_id)
	    stp_stpm_mst_update_transmit( this);

	return STP_OK;
}

void
stp_stpm_stop (STPM_T* this)
{
	this->admin_state = STP_DISABLED;
}

int
stp_stpm_do_update (STPM_T* this) /* returns number of loops */
{
  register Bool     need_state_change;
  register int      number_of_loops = 0;

  need_state_change = False; 
  
  for (;;) {/* loop until not need changes */
    need_state_change = stp_stpm_iterate_machines (this,
                                                   stp_statmch_check_condition,
                                                   True);
    if (! need_state_change) return number_of_loops;

    number_of_loops++;
    /* here we know, that at least one stater must be
       updated (it has changed state) */
    number_of_loops += stp_stpm_iterate_machines (this,
                                                  stp_statmch_change_state,
                                                  False);

  }

  return number_of_loops;
}

int 
stp_stpm_update(STPM_T* this)
{
	if(True == g_flag )
	{
		return stp_stpm_do_update(this);
	}
	else
		return 0;
}

BRIDGE_ID *
stp_stpm_compute_bridge_id (STPM_T* this)
{
  register PORT_T* port = NULL;
  register PORT_T* min_num_port = NULL;
  int              port_index = 0;

  for (port = this->ports; port; port = port->next) {
    if (! port_index || port->port_index < port_index) {
      min_num_port = port;
      port_index = port->port_index;
    }
  }

  if (!min_num_port) return NULL; /* IMHO, it may not be */

  stp_to_get_port_mac (min_num_port->port_index, this->BrId.addr);

  return &this->BrId;
}

STPM_T*
stp_stpm_get_the_list (void)
{
  return bridges;
}

void
stp_stpm_update_after_bridge_management (STPM_T* this)
{
  register PORT_T* port;

  for (port = this->ports; port; port = port->next) {
    port->reselect = True;
    port->selected = False;
  }
}

int
stp_stpm_check_bridge_priority (STPM_T* this)
{
  register STPM_T* oth;

  for (oth = bridges; oth; oth = oth->next) {
    if (STP_ENABLED == oth->admin_state && oth != this &&
        ! stp_vector_compare_bridge_id (&this->BrId, &oth->BrId)) {
      return STP_Invalid_Bridge_Priority;
    }
  }

  return 0;
}

const char*
stp_stpm_get_port_name_by_id (STPM_T* this, PORT_ID port_id)
{
  register PORT_T* port;

  for (port = this->ports; port; port = port->next) {
    if (port_id == port->port_id) {
        return port->port_name;
    }
  }

  return "Undef?";
}
/*mstp transmit*/
static void stp_stpm_mst_update_transmit( STPM_T* this )
{
    PORT_T* port;    
    int iret = 0;
	
    for ( port = this->ports; port; port = port->next )
    {
       iret = stp_statmch_check_condition ( port->transmit);
    
       if (iret )
       {
           stp_statmch_change_state( port->transmit);
       }
    }
    return ;
    
}

static void stp_stpm_mst_update_receive( PORT_T *port)
{
    int iret = 0;

    if(port)
    {
        iret = stp_statmch_check_condition ( port->receive);
        
        if (iret )
        {
           stp_statmch_change_state( port->receive);
        }
    }
    else
    {
        STPM_T* this = stp_stpm_get_the_cist();
        register PORT_T* mstport;    
		
        for ( mstport = this->ports; mstport; mstport = mstport->next )
        {
           iret = stp_statmch_check_condition ( mstport->receive);
        
           if (iret )
           {
               stp_statmch_change_state( mstport->receive);
           }
        }
    }
    return ;
    
}

static void stp_stpm_mst_init_transmit( STPM_T* this )
{
    PORT_T* port;    
	
    for ( port = this->ports; port; port = port->next )
    {
       stp_stpm_init_machine ( port->transmit);
    }
    return ;
    
}

static void stp_stpm_mst_init_receive( STPM_T* this )
{
    PORT_T* port;    
	
    for ( port = this->ports; port; port = port->next )
    {
       stp_stpm_init_machine ( port->receive);
    }
    
    return ;
    
}

void
stp_stpm_mst_update ( PORT_T *port) /* returns number of loops */
{
  register int      number_of_loops = 0;
  STPM_T * stpm = NULL ;

  stp_stpm_mst_update_receive(port);
  
  do    
  {
    number_of_loops = 0;
    for (stpm = stp_stpm_get_the_list (); stpm; stpm = stpm->next)
    {
      number_of_loops += stp_stpm_update( stpm ) ;
    }
  }
  while( number_of_loops );

  stp_stpm_mst_update_transmit(stp_stpm_get_the_cist());

  return ;

}

#ifdef __cplusplus
}
#endif


