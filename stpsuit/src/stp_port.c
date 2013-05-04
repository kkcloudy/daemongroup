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
* stp_port.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for STP PORT instance in stp module
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

/* STP PORT instance : 17.18, 17.15 */
#include "sysdef/npd_sysdef.h"
 
#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_in.h"

/* #include "stp_rolesel.h" */
#include "stp_portinfo.h"
#include "stp_roletrns.h"
#include "stp_sttrans.h"
#include "stp_topoch.h"
#include "stp_migrate.h"
#include "stp_transmit.h"
#include "stp_p2p.h"
#include "stp_pcost.h"
#include "stp_edge.h"
#include "stp_receive.h"
#include "stp_log.h"
#include "stp_to.h" /* for stp_to_get_port_name & stp_to_get_port_link_status */
#include "stp_npd_cmd.h"
#include "stp_bitmap.h"
/*mstp*/
#if 0
#define PORT_MAX_INDEX  /*256*/327
#endif
#define PORT_MAX_INDEX 640 /*for distributed system*/
PORT_T * g_CISTPORT[NUMBER_OF_PORTS + 1] = {0};
extern unsigned int productId;
extern unsigned int stp_is_distributed;
PORT_T * stp_port_mst_findport( unsigned short MSTID, unsigned long port_index );
#if 0
PORT_T *
STP_port_create (STPM_T* stpm, int port_index)
{
  PORT_T*        this;
  UID_STP_PORT_CFG_T port_cfg;
  register int   iii;
  unsigned short port_prio;

  /* check, if the port has just been added */
  for (this = stpm->ports; this; this = this->next) {
    if (this->port_index == port_index) {
	  STP_LOG(STP_LOG_ERR,"RSTP>>%% STP_port_create exist\n");
      return NULL;
    }
  }
  STP_LOG(STP_LOG_DEBUG,"RSTP>>$$$ create new port  port_index = %d $$$\n",port_index);
  STP_NEW_IN_LIST(this, PORT_T, stpm->ports, "port create");

  this->owner = stpm;
  this->machines = NULL;
  this->port_index = port_index;
  this->port_name = strdup (stp_to_get_port_name (port_index));
  this->uptime = 0;

  /*init port info*/
  stp_to_get_init_port_cfg (stpm->vlan_id, port_index, &port_cfg);
  port_prio =                  port_cfg.port_priority;
  this->admin_non_stp =        port_cfg.admin_non_stp;
  this->adminEdge =            port_cfg.admin_edge;
  this->adminPCost =           port_cfg.admin_port_path_cost;
  this->adminPointToPointMac = port_cfg.admin_point2point;
  
  this->LinkDelay = DEF_LINK_DELAY;
  this->port_id = (port_prio << 8) + port_index;

  iii = 0;
  this->timers[iii++] = &this->fdWhile;
  this->timers[iii++] = &this->helloWhen;
  this->timers[iii++] = &this->mdelayWhile;
  this->timers[iii++] = &this->rbWhile;
  this->timers[iii++] = &this->rcvdInfoWhile;
  this->timers[iii++] = &this->rrWhile;
  this->timers[iii++] = &this->tcWhile;
  this->timers[iii++] = &this->txCount;
  this->timers[iii++] = &this->lnkWhile;
  /* create and bind port state machines */
  STP_STATE_MACH_IN_LIST(topoch);

  STP_STATE_MACH_IN_LIST(migrate);

  STP_STATE_MACH_IN_LIST(p2p);

  STP_STATE_MACH_IN_LIST(edge);
                  
  STP_STATE_MACH_IN_LIST(pcost)

  STP_STATE_MACH_IN_LIST(info);
                  
  STP_STATE_MACH_IN_LIST(roletrns);

  STP_STATE_MACH_IN_LIST(sttrans);

  STP_STATE_MACH_IN_LIST(transmit);

                  
#ifdef STP_DBG

#if 0
  this->roletrns->ignoreHop2State = 14; /* DESIGNATED_PORT; */
  this->info->ignoreHop2State =      3; /* CURRENT */
  this->transmit->ignoreHop2State =  3; /* IDLE */
  this->edge->ignoreHop2State =      0; /* DISABLED; */
#endif

#if 0
  this->info->debug = 1;
  this->pcost->debug = 1;
  this->p2p->debug = 1;
  this->edge->debug = 1;
  this->migrate->debug = 1;
  this->sttrans->debug = 1;
  this->topoch->debug = 1;
  this->roletrns->debug = 1;
  this->sttrans->debug = 1;
#endif

#endif
  return this;
}
#endif

PORT_T * stp_port_mst_port_create ( STPM_T * stpm, unsigned short MSTID, unsigned long port_index, unsigned int isWan, unsigned short VID)
{
	PORT_T        *this,  *cist_port;
	UID_STP_PORT_CFG_T port_cfg;
	register int   iii;
	unsigned short port_prio;
	PORT_T * port = NULL ;
	unsigned int   slot_no = 0;
	unsigned int   port_no = 0;
	
#define SYSFS_PATH_MAX 256
	FILE *f = NULL;
	char path[SYSFS_PATH_MAX] = {0};
	char ifname[21] = {0};
	char ifname2[21] = {0};
	unsigned int br_ifindex = 0;

	/* check, if the port has just been added */
	for (this = stpm->ports; this; this = this->next) {
		if (this->port_index == port_index) {
		  	return NULL;
		}
	}

	STP_NEW_IN_LIST( this, PORT_T, stpm->ports, "port create" );

	if (isWan) {
		if ((port = stp_port_mst_findport(0, port_index)) != NULL ){
			slot_no = port->slot_no;
			port_no = port->port_no;
			this->slot_no = slot_no;
			this->port_no = port_no;
			this->isWAN = isWan;
			
			stp_syslog_dbg("slot_no %d ,port_no %d \n",slot_no,port_no);
		}
		else {
			stp_syslog_warning("get intance 0 port failed!\n",MSTID);
		}
		snprintf(path, SYSFS_PATH_MAX, "/sys/class/net/eth%d-%d.%d/brport/bridge/ifindex", slot_no, port_no,VID);
		f = fopen(path, "r");
		if (f) {
			fscanf(f, "%d", &br_ifindex);
			sprintf(ifname, "eth%d-%d.%d", slot_no, port_no, VID);
			this->port_ifindex = if_nametoindex(ifname);
			this->br_ifindex = br_ifindex;
			
			stp_syslog_dbg("eth%d-%d.%d ifindex %d brindex %d\n",slot_no, port_no, VID,this->port_ifindex,this->br_ifindex);

			fclose(f);
			/*when add port state is discard first*/
			stp_npd_set_wan_state(this->br_ifindex, this->port_ifindex, 1);/*NAM_STP_PORT_STATE_DISCARD_E*/
		}
		else{
			stp_syslog_error("eth%d-%d.%d not in br \n",slot_no, port_no, VID);
		}
	}

	this->owner = stpm;
	this->machines = NULL;
	this->vlan_id = MSTID;/*mstid*/
	this->port_index = port_index;

	memcpy(this->port_name,strdup(stp_to_get_port_name (port_index)), 32);


	stp_to_get_init_port_cfg (stpm->vlan_id, port_index, &port_cfg);
	port_prio =                  port_cfg.port_priority;
	this->admin_non_stp =        port_cfg.admin_non_stp;
	this->adminEdge =            port_cfg.admin_edge;
	this->adminPCost =           port_cfg.admin_port_path_cost;
	this->adminPointToPointMac = port_cfg.admin_point2point;

	this->LinkDelay = DEF_LINK_DELAY;
	this->port_id = (port_prio << 8) + port_index;
	#if 0
	if(port_index > PORT_MAX_INDEX) {
		this->port_id = this->port_id & 0xfeff;
	}
    #else
	/* use the PORT_INDEX_MAX in STP */
	if(port_index > PORT_INDEX_MAX) {
		this->port_id = this->port_id & 0xfeff;
	}	
	#endif
	if ( MSTID != STP_CIST_ID)
	{
	    cist_port = (PORT_T *)stp_port_mst_findport( STP_CIST_ID, port_index );
	    if ( cist_port )
	    {
	        this->operEdge = cist_port->operEdge;
	        this->portEnabled = cist_port->portEnabled;
	        this->operPointToPointMac = cist_port->operPointToPointMac;
	    }
	}

	this->uptime = 0;
	this->rx_cfg_bpdu_cnt = 0;
	this->rx_rstp_bpdu_cnt = 0;
	this->rx_tcn_bpdu_cnt = 0;

	iii = 0;
	this->timers[ iii++ ] = &this->fdWhile;
	this->timers[ iii++ ] = &this->helloWhen;
	this->timers[ iii++ ] = &this->mdelayWhile;
	this->timers[ iii++ ] = &this->rbWhile;
	this->timers[ iii++ ] = &this->rcvdInfoWhile;
	this->timers[ iii++ ] = &this->rrWhile;
	this->timers[ iii++ ] = &this->tcWhile;
	this->timers[ iii++ ] = &this->txCount;
	this->timers[ iii++ ] = &this->lnkWhile;

	MSTP_STATE_MACH_IN_LIST( topoch, True );
	MSTP_STATE_MACH_IN_LIST( pcost, True );
	MSTP_STATE_MACH_IN_LIST( info, True);
	MSTP_STATE_MACH_IN_LIST( roletrns, True);
	MSTP_STATE_MACH_IN_LIST( sttrans, True );

	if ( NULL == this->topoch ||
	        NULL == this->info ||
	        NULL == this->roletrns ||
	        NULL == this->sttrans )
	{
	    stp_port_delete( this );
	    return NULL;
	}

	if ( stpm->vlan_id  == STP_CIST_ID )
	{
	    MSTP_STATE_MACH_IN_LIST( p2p, True);
	    MSTP_STATE_MACH_IN_LIST( edge, True);
	    MSTP_STATE_MACH_IN_LIST( migrate, True);
	    MSTP_STATE_MACH_IN_LIST(transmit, False);
	    MSTP_STATE_MACH_IN_LIST(receive, False);


	    if ( NULL == this->migrate || NULL == this->edge ||
	          NULL == this->transmit || NULL == this->p2p  ||
	           NULL == this->receive)
	    {
	        stp_port_delete( this );
	        return NULL;
	    }
	}
	this->nextMst = NULL;
	return this;

}

void 
stp_port_init (PORT_T* this, STPM_T* stpm, Bool check_link)
{
  BRIDGE_ID null_br;  /*mstp*/
  if (check_link) {
  	stp_syslog_dbg("while initial port,port_index %d ifport_index %d slot_no %d port_no %d\n", this->port_index, this->port_ifindex, this->slot_no, this->port_no);
	/*port up or down*/	
    this->adminEnable = stp_to_get_port_link_status (this->port_index);
	stp_syslog_dbg("get admin state %d\n", this->adminEnable);
    memset(&null_br, 0, sizeof(BRIDGE_ID));   /*mstp*/
    stp_vector_create (&this->designPrio,
                   &stpm->BrId,
                   0,  /*mstp*/
                   &null_br,
                   0,
                   &stpm->BrId,
                   this->port_id,
                   this->port_id);
    stp_times_copy (&this->designTimes, &stpm->rootTimes);
  }

  /* reset timers */
  this->fdWhile =
  this->helloWhen =
  this->mdelayWhile =
  this->rbWhile =
  this->rcvdInfoWhile =
  this->rrWhile =
  this->tcWhile =
  this->txCount = 0;

  this->msgPortRole = RSTP_PORT_ROLE_UNKN;
  this->selectedRole = DisabledPort;
  this->sendRSTP = True;
  this->operSpeed = stp_to_get_port_oper_speed (this);
  this->p2p_recompute = True;
}

void
stp_port_delete (PORT_T* this)
{
  STPM_T*                   stpm;
  register PORT_T*          prev;
  register PORT_T*          tmp;
  register STATE_MACH_T*    stater;
  register void*            pv;

  stpm = this->owner;

  /*free (this->port_name);*/
  for (stater = this->machines; stater; ) {
    pv = (void*) stater->next;
    stp_statmch_delete (stater);
    stater = (STATE_MACH_T*) pv;
  }
                 
  prev = NULL;
  for (tmp = stpm->ports; tmp; tmp = tmp->next) {
    if (tmp->port_index == this->port_index) {
		stp_syslog_dbg("delete port_index %d\n",tmp->port_index);
      if (prev) {
        prev->next = this->next;
      } else {
        stpm->ports = this->next;
      }
	  stp_npd_cmd_send_stp_info(this->vlan_id, 0, this->port_index, NAM_STP_PORT_STATE_DISABLE_E);
      STP_FREE(this, "stp instance");
      break;
    }
    prev = tmp;
  }
}

int
stp_port_rx_bpdu (PORT_T* this, BPDU_T* bpdu, size_t len)
{
  return stp_portinfo_rx_bpdu (this, bpdu, len);
}

unsigned int stp_port_global_index_parse(unsigned int port_index)
{
	unsigned int bit = 0,byte = 0;
	unsigned int base = 6;
	#if 0
	if(port_index < 0 && port_index > 327){
		return -1;
	}	
	#else
	if(port_index < 0 && port_index > PORT_INDEX_MAX)/*for distributed system */
	{
		return -1;
	}
	#endif
	else{
		if(STP_IS_DISTRIBUTED == stp_is_distributed)
		{
			return port_index % 64;     /* 2-1 return 1 */			
		}
		else if(PRODUCT_ID_AX7K == productId){
			byte = (port_index-64)/64;/*0-3*/
			bit = port_index % 64;/*0-5*/
			return byte*base + bit;/*0-23*/
		}
		else if((PRODUCT_ID_AU3K == productId) || 
			(PRODUCT_ID_AU3K_BCM == productId) ||
			(PRODUCT_ID_AU3K_BCAT == productId )||
			(PRODUCT_ID_AU2K_TCAT == productId )||
			(PRODUCT_ID_AX5K_E == productId) ||
			(PRODUCT_ID_AX5608== productId)){
			/*stp_syslog_error("port344::don't find ports productId %d,port_index %d\n",productId,port_index);*/
			return port_index;
		}
		/*
		else if((PRODUCT_ID_AX5K == productId) || (PRODUCT_ID_AU4K == productId) || (PRODUCT_ID_AX5K_I == productId)) {
			if(port_index >= 64) {
				return port_index - 64;//0-23
			}
			return port_index;//0-23
		}*/
		else if(PRODUCT_ID_AX5K_I == productId) {
			if(port_index >= 56) {
				return port_index - 56;//0-31
			}
			return port_index;//0-31
		}	
		else if((PRODUCT_ID_AX5K == productId) || (PRODUCT_ID_AU4K == productId)) {
			if(port_index >= 64) {
				return port_index - 64;/*0-23*/
			}
			return port_index;/*0-23*/
		}	
		else{
			return -1;
		}
	}
}

void stp_port_clear_gloabl_portarray()
{
	//printf("%d \n",NUMBER_OF_PORTS);
	memset(g_CISTPORT,NULL,(NUMBER_OF_PORTS + 1));
	int i;
	for(i = 0; i<(NUMBER_OF_PORTS + 1); i++){
		g_CISTPORT[i] = NULL;
		//printf("g_CISTPORT[i] %p\n",g_CISTPORT[i]);
	}
}

PORT_T * stp_port_mst_findport( unsigned short MSTID, unsigned long port_index )
{
    register PORT_T * port = NULL ;
    register int index = stp_port_global_index_parse(port_index);

	//printf("port_index %d, index %d\n",port_index,index);
    if((index < 0) || index > MAX_PORT_NUM)
        return NULL;
	
    if(!(port = g_CISTPORT[index])) {
		stp_syslog_error("port344::don't find ports mstid %d,port_index %d,index %d\n",MSTID,port_index,index);
		return NULL;
    }

    if(STP_CIST_ID == MSTID)
        return g_CISTPORT[index];

    for(port = port->nextMst; port; port = port->nextMst)
    {
		stp_syslog_dbg("%s :: port sec %p, port->owner->vlan_id %d \n",__FILE__,port,port->owner->vlan_id);
    	    if(port->owner->vlan_id == MSTID ) {
				stp_syslog_dbg("%s :: port sec %p\n",__FILE__,port);
            return port;
    	    }
    }
	stp_syslog_dbg("%s :: port sec %p\n",__FILE__,port);

	stp_syslog_dbg("port356:: don't find ports mstid %d,port_index %d,index %d\n",MSTID,port_index,index);
    return NULL;
}

int stp_port_mst_addport( unsigned short MSTID, unsigned long port_index, unsigned short VID, Bool bIfLock, unsigned int isWan )
{
    PORT_T * port = NULL ;
    int  iRet = 0 ;
    STPM_T *stpm =NULL;
	register int index = stp_port_global_index_parse(port_index);


    if ( MSTID > STP_MSTI_MAX || index < 0 || index > MAX_PORT_NUM )
    {
    	stp_syslog_error("port_index %d,index %d is error \n",port_index,index);
        return STP_ERROR ;
    }

    if ( bIfLock )
    {
        RSTP_CRITICAL_PATH_START;
    }
    /* 端口是否已经在mstid中了，如果不在，添加之*/
	//STP_LOG(STP_LOG_DEBUG,"port378 :: mstid %d,port_index %d,index %d\n",MSTID,port_index,index);
    if ((port = stp_port_mst_findport( MSTID, port_index ) )== NULL )
    { 
    	stp_syslog_dbg("no port(port index %d),create it!\n",port_index);
        stpm = stp_stpm_get_instance( MSTID );
        if ( NULL == stpm )
        {
            stp_syslog_warning("get stpm %d failed!\n",MSTID);
            iRet = STP_BRIDGE_NOTFOUND;
            goto Unlock;
        }    
        port = stp_port_mst_port_create( stpm , MSTID, port_index, isWan, VID );
        if ( NULL == port )
        {
            stp_syslog_warning("create port instance (MSTID %d,port index %d) failed!\n",MSTID,port_index);
            iRet = STP_CREATE_PORT_FAIL;
            goto Unlock;
        }
        stp_port_init( port, stpm, True );

    
        /*5 调用表操作，添加入端口映射表。*/
        if(STP_CIST_ID == MSTID)
        {	
             stp_syslog_dbg("Add port instance to CIST!\n");
			 g_CISTPORT[index] = port;
        }
        else
        {
        	stp_syslog_dbg("Add port instance( mstid %d,port_index %d)!\n",MSTID,port_index);
            port->nextMst = g_CISTPORT[index]->nextMst;
            g_CISTPORT[index]->nextMst = port;
        }
		
        if ( stpm->admin_state == STP_ENABLED )
        {
            if ( port->portEnabled  && !port->admin_non_stp ) 
            {
                stp_syslog_dbg("Enable the port %d instance!\n",port->port_index);
                stp_stpm_enable( stpm, STP_ENABLED );
            }
        }
    }

	
    port->vlan_map.bmp[VID/8] |=  1<< (VID % 8);
	if(isWan){
    	port->vlan_map.ulcount = 1;
	}
	else{
		port->vlan_map.ulcount ++;
	}

    /*6 如果bIfLock，解锁。*/
    iRet = STP_OK;
    goto Unlock;

Unlock:
    if ( bIfLock )
    {
        RSTP_CRITICAL_PATH_END;
    }

    return iRet ;
}

int stp_port_mst_delport( unsigned short MSTID, unsigned long port_index, unsigned short VID, Bool bIfLock )
{
    PORT_T * port = NULL;
    register PORT_T * mstport = NULL;
    STPM_T * stpm = NULL;
    int iRet;
	register int index = stp_port_global_index_parse(port_index);

    if ( MSTID > STP_MSTI_MAX || index < 0 || index > MAX_PORT_NUM)
    {
        return STP_ERROR ;
    }
	
    if ( MSTID == STP_CIST_ID)
    {
        return STP_OK ;
    }

    if ( bIfLock )
    {
        RSTP_CRITICAL_PATH_START;
    }
    stp_syslog_dbg("MSTID %d port_index %d\n",MSTID, port_index);
    port = stp_port_mst_findport( MSTID, port_index ) ;
    if (! port )
    {
        iRet = STP_PORT_NOTFOUND;
        goto Unlock;
    }
    
    port->vlan_map.bmp[VID/8] &= ~( 1<< (VID % 8));
    port->vlan_map.ulcount --;
	
    if(MSTID == STP_CIST_ID || port->vlan_map.ulcount )	
        goto Unlock;


    for( mstport = g_CISTPORT[index]; mstport->nextMst; mstport = mstport ->nextMst)
    {
        if(mstport->nextMst->owner->vlan_id == MSTID)
        {
            mstport->nextMst = mstport->nextMst->nextMst;
            break;
        }
    }
	
    stpm = port->owner;
    stp_port_delete( port );

	

    if ( stpm->admin_state == STP_ENABLED  && port->portEnabled && !port->admin_non_stp )
    {
       stp_stpm_enable( stpm, STP_ENABLED );
    }

    /*6 如果bIfLock，解锁。*/
    iRet = STP_OK;

Unlock:
    if ( bIfLock )
    {
        RSTP_CRITICAL_PATH_END;
    }
    return iRet;

}
#ifdef STP_DBG
int stp_port_trace_state_machine (PORT_T* this, char* mach_name, int enadis, int vlan_id)
{
    register struct state_mach_t* stater;
	printf("DEBUG::::::port\n");
    for (stater = this->machines; stater; stater = stater->next) {
        if (! strcmp (mach_name, "all") || ! strcmp (mach_name, stater->name)) {
            /* if (stater->debug != enadis) */
            {
                stp_trace ("port %s on %s trace %-8s (was %s) now %s,state now is %d",
                    this->port_name, this->owner->name,
                    stater->name,
                    stater->debug ? " enabled" :"disabled",
                    enadis        ? " enabled" :"disabled",
                    stater->State);
            }
            stater->debug = enadis;
        }
    }
/*mstp*/
    if((stater = this->receive))
    {
      if (! strcmp (mach_name, "all") || ! strcmp (mach_name, stater->name))
      {
         stp_trace ("port %s on %s trace %-8s (was %s) now %s",
             this->port_name, this->owner->name,
             stater->name,
             stater->debug ? " enabled" :"disabled",
             enadis        ? " enabled" :"disabled");
      stater->debug = enadis;   
      }
    }
    if((stater = this->transmit))
    {
      if (! strcmp (mach_name, "all") || ! strcmp (mach_name, stater->name))
      {
         stp_trace ("port %s on %s trace %-8s (was %s) now %s",
             this->port_name, this->owner->name,
             stater->name,
             stater->debug ? " enabled" :"disabled",
             enadis        ? " enabled" :"disabled");
      stater->debug = enadis;   
      }
    }

    return 0;
}

int stp_port_bridge_trace_state_machine(struct stpm_t* this,char* mach_name,int enadis,int vlan_id)
{
    register struct state_mach_t* stater;
	printf("DEBUG::::::bridge\n");
    for (stater = this->machines; stater; stater = stater->next) {
        if (! strcmp (mach_name, "all") || ! strcmp (mach_name, stater->name)) {
            /* if (stater->debug != enadis) */
            {
                stp_trace ("bridge %s  trace %-8s (was %s) now %s,state now is %d",
                    this->name, 
                    stater->name,
                    stater->debug ? " enabled" :"disabled",
                    enadis        ? " enabled" :"disabled",
                    stater->State);
            }
            stater->debug = enadis;
        }
    }

    return 0;
}

void stp_port_trace_flags (char* title, PORT_T* this)
{
#if 0 /* it may be opened for more deep debugging */
    unsigned long flag = 0L;
    
    if (this->reRoot)   flag |= 0x000001L;
    if (this->sync)     flag |= 0x000002L;
    if (this->synced)   flag |= 0x000004L;

    if (this->proposed)  flag |= 0x000010L;
    if (this->proposing) flag |= 0x000020L;
    if (this->agreed)    flag |= 0x000040L;
    if (this->updtInfo)  flag |= 0x000080L;

    if (this->operEdge)   flag |= 0x000100L;
    stp_trace ("         %-12s: flags=0X%04lx port=%s", title, flag, this->port_name);
#endif
}

#endif
#ifdef __cplusplus
}
#endif