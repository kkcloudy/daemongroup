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
*stp_mgmt.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for management in stp module
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

/* This file contains API from an operation system to the RSTP library */
#include <stdlib.h>
#include <string.h>
#include "assert.h"

#include "stp_base.h"
#include "stp_bitmap.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_in.h" /* for bridge defaults */
#include "stp_to.h"
#include "stp_log.h"

/*mstp*/
unsigned short g_VID2MSTID[4096] = {0};
BITMAP_T  g_VlanPBMP[4096] = {{0}};
UID_STP_MODE_T g_stp_state = STP_DISABLED;
extern BITMAP_T enabled_ports;
extern struct stp_admin_infos **stp_ports;

#define STP_STATE_NONE_STR "None"
#define STP_STATE_DISCARDING_STR "Discarding"
#define STP_STATE_LEARNING_STR "Learning"
#define STP_STATE_FORWARDING_STR "Forwarding"

static int stp_mgmt_calc_configid_digest(STPM_T * this)
{
   unsigned short  *vlan_mst_map_table = NULL;
   //unsigned short vlan_mst_map_table[4096] = {0};
    unsigned char  digest[16];
    int i;
    //static char* STP_UM_KEY = "13AC06A62E47FD51F95D2BA243CD0346" ;
	static char STP_UM_KEY[] = {	\
		0x13,0xAC,0x06,0xA6,	\
		0x2E,0x47,0xFD,0x51,	\
		0xF9,0x5D,0x2B,0xA2,	\
		0x43,0xCD,0x03,0x46};
	
    vlan_mst_map_table = (unsigned short  *)malloc(4096 * 2);
    memset(vlan_mst_map_table,0,4096*2);
    for(i=1;i<4095;i++)
    {
        vlan_mst_map_table[i] = htons(g_VID2MSTID[i]);
    }
    stp_md5_get_digest((unsigned char*)vlan_mst_map_table, 4096*2,STP_UM_KEY,16,digest);
    memcpy(this->MstConfigId.ConfigurationDigest, digest,16);
    free( vlan_mst_map_table );
    return STP_OK ;

}

int stp_mgmt_build_configid_digest()
{
	STPM_T* this = NULL;
	int err_code;
	
	this = stp_stpm_get_the_cist();
	if (this) { /* it had just been created :( */
	 	err_code = STP_Nothing_To_Do;
		goto Unlock;
	}
	
	 stp_mgmt_calc_configid_digest(this);

	err_code = STP_OK;
	Unlock:
	RSTP_CRITICAL_PATH_END;

	return err_code;
}

void stp_mgmt_init_vlanpbmp(  unsigned short VID, BITMAP_T ports)
{
	int i = 0;
	for(; i < 9 ; i++) {
		g_VlanPBMP[VID].part[i] = ports.part[i];
		stp_syslog_dbg("bmp %02x\n",g_VlanPBMP[VID].part[i]);
	}
}

unsigned short stp_mgmt_get_mstid(int vid)
{
	return g_VID2MSTID[vid];
}

int stp_mgmt_set_cist_vlan_map(unsigned short VID,Bool isAdd)
{
	int err_code;
	STPM_T * cist = stp_stpm_get_the_cist();
	if (!cist) { /* it had just been created :( */
	 	err_code = STP_BRIDGE_NOTFOUND;
		stp_syslog_dbg("set cist vlan map error(%s)\n",
		          stp_in_get_error_explanation (err_code));

		goto Unlock;
	}
	
	if(isAdd) {
		cist->vlan_map.bmp[VID/8] &=  ~(1u<< (VID % 8));
		cist->vlan_map.ulcount --;
	}
	else {
		cist->vlan_map.bmp[VID/8] |= (1u<< (VID % 8));
		cist->vlan_map.ulcount ++;
	}

	err_code = STP_OK;
	Unlock:
	return err_code;
}

int stp_mgmt_stpm_add_vlan(STPM_T * stpm,  unsigned short VID)
{
	int port_index = 0;
	int bytes = 0,bit = 0,i = 0;
	unsigned int rc = 0;

	if (VID < 0 || VID > STP_MAX_VID)
	{
	  	return STP_ERROR ;
	}

	if ( stpm == NULL )
	{
	  	return STP_ERROR ;
	}

	if(0 != VID) {		
		stpm->vlan_map.bmp[VID/8] |=  1<< (VID % 8);
		stpm->vlan_map.ulcount ++;
		stp_syslog_dbg("Map vid %d to mstid [%d],get vlan_map.bmp[%d]=[%02x]\n",	\
				VID,stpm->vlan_id,VID/8,stpm->vlan_map.bmp[VID/8]);

		/*把原来再CIST上的映射删除*/
		if(STP_CIST_ID != stpm->vlan_id) 
			stp_mgmt_set_cist_vlan_map(VID,True);
	}
	for(i = 0;i < 9 ; i++){
		stp_syslog_dbg("Get VID %d bitmap:",VID);
		stp_syslog_dbg(" bmp[%d] %02x\n",i,g_VlanPBMP[VID].part[i]);
		stp_syslog_dbg("\n");
	}
	for(bytes = 0; bytes < NUMBER_OF_BYTES; bytes++) {
		for(bit = 0; bit < BIT_OF_BYTE; bit++) {
			port_index = stp_bitmap_get_portindex_from_bmp(&g_VlanPBMP[VID], bytes, bit);
			stp_syslog_dbg("stp get port index %d from bmp byte %d,bit %d\n",port_index,bytes,bit);
			if(-2 == port_index) {	
				break;
			}
			else if(-1 == port_index) {
				continue;
			}
			else {
				rc = stp_port_mst_addport (stpm->vlan_id,port_index, VID, False ,0);
				if (STP_OK != rc) {
				    /* can't add port :( */
				    stp_trace ("Create port instance ()failed!\n",stpm->vlan_id,(int) port_index);
				    stp_stpm_delete (stpm);
				    return STP_Cannot_Create_Instance_For_Port;			
				}
				else {/*set mstid to stp port infos*/
				   if((NULL != stp_ports)&&(NULL != stp_ports[port_index])){
                      stp_ports[port_index]->mstid[stpm->vlan_id] = stpm->vlan_id;
				   }
				}
			}
		}
	}
	
	g_VID2MSTID[ VID ] = stpm->vlan_id;
	
	return STP_OK ;
}

int stp_mgmt_stpm_del_vlan(STPM_T * stpm,  unsigned short VID)
{
	int port_index = 0;
	int bytes,bit;
	PORT_T *port;
	int iRet = STP_OK;

	if ( VID < 0 || VID > STP_MAX_VID)
	{
	    return STP_ERROR ;
	}

	/* 1. 如果桥指针为空，则创建之 */
	if ( stpm == NULL )
	{
	    return STP_ERROR ;
	}
	
	stpm->vlan_map.bmp[VID/8] &=  ~(1u<< (VID % 8));
	stpm->vlan_map.ulcount --;

	/*把映射恢复到CIST上*/
	if(STP_CIST_ID != stpm->vlan_id) 
		stp_mgmt_set_cist_vlan_map(VID,False);
	
	for(bytes = 0; bytes < NUMBER_OF_BYTES; bytes++) {
		for(bit = 0; bit < BIT_OF_BYTE; bit++) {
			port_index = stp_bitmap_get_portindex_from_bmp(&g_VlanPBMP[VID], bytes, bit);
			if(-1 == port_index)
				continue;
			else {
			  port = stp_port_mst_findport(stpm->vlan_id,  port_index);
			  if(!port)
			     continue;
			  if ( port->adminEnable && !port->admin_non_stp) {
			    stp_to_flush_lt (port_index, VID, LT_FLASH_ONLY_THE_PORT, "del vlan");
			  }

			  iRet = stp_port_mst_delport(stpm->vlan_id, port_index, VID, False);
			  if(STP_OK != iRet)
			    return iRet;
			}
		}
	}
	
	if ( !stpm->vlan_map.ulcount  && stpm->vlan_id != STP_CIST_ID)
	{
	  if(stpm->ports){
	   ;// assert(0);
	  }

	  stp_stpm_delete( stpm ) ;
	}

	g_VID2MSTID[ VID ] = STP_CIST_ID;
	return iRet ;
}

int 
stp_mgmt_stpm_create_cist ()
{
	register STPM_T*  this;
	int               err_code;

	stp_trace ("STP_IN_stpm_create(%s)", "stp cist");
    
	RSTP_CRITICAL_PATH_START;  
	this = stp_stpm_get_the_cist();
	if (this) { /* it had just been created :( */
	 	err_code = STP_Nothing_To_Do;
		goto Unlock;
	}

	this = (STPM_T *)stp_stpm_mst_create (STP_CIST_ID, "CIST");
	if (! this) { /* can't create stpm :( */
	  err_code = STP_Cannot_Create_Instance_For_Vlan;
	  goto Unlock;
	}

	memset(this->vlan_map.bmp,0xff,512);
	this->vlan_map.ulcount = STP_MAX_VID;
	 
	this->MstConfigId.FormatSelector = 0;
	this->MstConfigId.RevisionLevel[ 0 ] = 0;
	this->MstConfigId.RevisionLevel[ 1 ] = 0;
	stp_mgmt_calc_configid_digest(this);
	sprintf( this->MstConfigId.ConfigurationName, "%.2x%.2x%.2x%.2x%.2x%.2x",
	              this->BrId.addr[ 0 ], this->BrId.addr[ 1 ], this->BrId.addr[ 2 ] ,
	              this->BrId.addr[ 3 ], this->BrId.addr[ 4 ], this->BrId.addr[ 5 ] ) ;

	err_code = STP_OK;
	Unlock:
	RSTP_CRITICAL_PATH_END;

	return err_code;
	
}

int
stp_mgmt_stpm_update_instance ( unsigned short MSTID, unsigned short VID)
{
	register STPM_T*  this = NULL ;
	STPM_T * old_stpm = NULL, *cist_stpm = NULL;
	int               err_code;
	char name[32] ={0};

	stp_trace ("stp_mgmt_stpm_update_instance(%s-%d)", "stp instance",MSTID);

	RSTP_CRITICAL_PATH_START;  
	if(MSTID == g_VID2MSTID[VID])
	{
		stp_syslog_dbg("stpmgmt280 :: vlan %d is already in instance %d.\n",VID, MSTID);
		return STP_OK;
	}

	/*firstly find instance by MSTID,check this instance exist or not*/
	this = stp_stpm_get_instance(MSTID);
	sprintf(name, "%s-%d","MSTI",MSTID);
	if (!this) { /* it had just been created :( */
	  	this = (STPM_T *)stp_stpm_mst_create (MSTID, name);
	  	stp_syslog_dbg("stpmgmt290::vlan %d , instance %d created\n",VID, MSTID);
	 	if (! this) { /* can't create stpm :( */
	    	err_code = STP_Cannot_Create_Instance_For_Vlan;
	    goto Unlock;
	  }
	}
	/*check this vlan if */
	old_stpm = stp_stpm_get_instance (g_VID2MSTID[VID]);
	
	if (!old_stpm) { /* it had just been created :( */
	  err_code = STP_Cannot_Create_Instance_For_Vlan;
	  goto Unlock;
	}
	
	 stp_syslog_dbg("stpmgmt304::finded instance %d \n",g_VID2MSTID[VID]);
	err_code = stp_mgmt_stpm_del_vlan(old_stpm, VID);
	
	if(err_code != STP_OK){
	  	goto Unlock;
		}
	stp_syslog_dbg("stpmgmt309::add instance %d \n",this->vlan_id);
	err_code = stp_mgmt_stpm_add_vlan(this, VID);

	
	if(err_code != STP_OK)
	  goto Unlock;

	
	cist_stpm = (STPM_T *)stp_stpm_get_the_cist();
	
	stp_mgmt_calc_configid_digest(cist_stpm);

	if(g_stp_state == STP_ENABLED)
	{

	  	err_code = stp_stpm_enable (this, g_stp_state);
	  	if(err_code != STP_OK)
	    goto Unlock;
	}
	err_code = STP_OK;

	Unlock:
	RSTP_CRITICAL_PATH_END;
	return err_code;  
}


void
stp_mgmt_stpm_delete_all ()
{
	register STPM_T*  stpm;
	register PORT_T*  port;
	stp_trace ("stp_mgmt_stpm_delete_all(%s)", "all");

	RSTP_CRITICAL_PATH_START;  

	while(stpm = stp_stpm_get_the_list()){
		while(port = stpm->ports) {	  
		    if ( port->adminEnable && !port->admin_non_stp)
		    {
		      #if 0
		      stp_to_flush_lt (port->port_index, VID, LT_FLASH_ONLY_THE_PORT, "del vlan");
		      #endif
		    }
		    stp_port_delete(port);
		}
		stp_stpm_delete(stpm);
	}

	stp_port_clear_gloabl_portarray();
	memset(g_VlanPBMP,0,4096);
	memset(g_VID2MSTID,0,4096);
	stp_bitmap_clear(&enabled_ports);
	RSTP_CRITICAL_PATH_END;
	return ;  
}


void stp_mgmt_del_all_mst()
{
	register STPM_T*  stpm =NULL,*prev = NULL;
	register PORT_T*  port = NULL;
	stp_trace ("stp_mgmt_stpm_delete_all(%s)", "all");

	RSTP_CRITICAL_PATH_START; 
	stpm = stp_stpm_get_the_list(); 
	while(stpm){
		//printf("del all 380 stpm %p\n",stpm);
		while(port = stpm->ports) {
			if ( port->adminEnable && !port->admin_non_stp)
			{
#if 0
			stp_to_flush_lt (port->port_index, VID, LT_FLASH_ONLY_THE_PORT, "del vlan");
#endif
			}
			//printf("del port port_index[%d]\n",port->port_index);
			stp_port_delete(port);
		}
		//printf("del stpm  vlanid[%d]\n",stpm->vlan_id);
		if(stpm->vlan_id != STP_CIST_ID) {
			prev = stpm->next;
			stp_stpm_delete(stpm);
			stpm = prev;
		}
		if(stpm) {
			stpm = stpm->next;
			//printf("del all 396 stpm ->next%p\n",stpm->next);
		}
	}
	
	memset(g_VlanPBMP,0,4096);
	RSTP_CRITICAL_PATH_END;
	return 0;  
}


/********************************************************************************
**                         STP 显示相关
*********************************************************************************/
static void stp_mgmt_vlan_show(STPM_T *this)
{
  unsigned short vid =0, cnt = 0;

  for(vid = 1; vid <=STP_MAX_VID; vid++)
  {
      if(this->vlan_map.bmp[vid /8] & (1u <<(vid % 8)))
      {
        printf("%d  ",vid);
        cnt ++;
      }
      if(cnt > 10)
      {
        printf("\n    ");
        cnt = 0;
      }
  }
  printf("\n    ");

}

static void
stp_mgmt_print_bridge_id (UID_BRIDGE_ID_T *bridge_id, unsigned char cr)
{
  printf("%04lX-%02x%02x%02x%02x%02x%02x",
                  (unsigned long) bridge_id->prio,
                  (unsigned char) bridge_id->addr[0],
                  (unsigned char) bridge_id->addr[1],
                  (unsigned char) bridge_id->addr[2],
                  (unsigned char) bridge_id->addr[3],
                  (unsigned char) bridge_id->addr[4],
                  (unsigned char) bridge_id->addr[5]);
  if (cr)
        printf("\n");
}

static char *
stp_state2str (RSTP_PORT_STATE stp_port_state, int detail)
{
  if (detail) {
    switch (stp_port_state) {
      case UID_PORT_DISABLED:   return "Disabled";
      case UID_PORT_DISCARDING: return "Discarding";
      case UID_PORT_LEARNING:   return "Learning";
      case UID_PORT_FORWARDING: return "Forwarding";
      case UID_PORT_NON_STP:    return "NoStp";
      default:                  return "Unknown";
    }
  }

  switch (stp_port_state) {
    case UID_PORT_DISABLED:     return "Dis";
    case UID_PORT_DISCARDING:   return "Blk";
    case UID_PORT_LEARNING:     return "Lrn";
    case UID_PORT_FORWARDING:   return "Fwd";
    case UID_PORT_NON_STP:      return "Non";
    default:                    return "Unk";
  }
}

static void stp_mgmt_out_port_id (int port, unsigned char cr)
{
  printf ("%s", stp_to_get_port_name (port));
  if (cr)
        printf("\n");
}

static void
stp_mgmt_convert_brgid2uid (IN BRIDGE_ID* f, OUT UID_BRIDGE_ID_T* t)
{
  memcpy (t, f, sizeof (UID_BRIDGE_ID_T));
}

static int
stp_mgmt_stpm_get_state (IN STPM_T *this, OUT UID_STP_STATE_T* entry)
{

  strncpy (entry->vlan_name, this->name, NAME_LEN);
  entry->vlan_id = this->vlan_id;
  stp_mgmt_convert_brgid2uid (&this->rootPrio.root_bridge, &entry->designated_root);
  entry->root_path_cost = this->rootPrio.root_path_cost;
  entry->root_port = this->rootPortId;
  entry->max_age =       this->rootTimes.MaxAge;
  entry->forward_delay = this->rootTimes.ForwardDelay;
  entry->hello_time =    this->rootTimes.HelloTime;

  stp_mgmt_convert_brgid2uid (&this->BrId, &entry->bridge_id);

  entry->stp_enabled = this->admin_state;

  entry->timeSince_Topo_Change = this->timeSince_Topo_Change;
  entry->Topo_Change_Count = this->Topo_Change_Count;
  entry->Topo_Change = this->Topo_Change;
  
  return 0;
}

static int
stp_mgmt_get_port_state (IN PORT_T *port, INOUT UID_STP_PORT_STATE_T* entry)
{

  entry->port_id = port->port_id;
  if (DisabledPort == port->role) {
    entry->state = UID_PORT_DISABLED;
  } else if (! port->forward && ! port->learn) {
    entry->state = UID_PORT_DISCARDING;
  } else if (! port->forward && port->learn) {
    entry->state = UID_PORT_LEARNING;
  } else {
    entry->state = UID_PORT_FORWARDING;
  }

  //entry->uptime = port->uptime;
  entry->linkState = port->adminEnable;
  entry->path_cost = port->adminPCost;
  stp_mgmt_convert_brgid2uid (&port->portPrio.root_bridge, &entry->designated_root);
  if ( STP_CIST_ID == port->owner->vlan_id)  /*CIST */
  {
    if ( memcmp( &port->owner->rootPrio.region_root_bridge , &port->owner->BrId , sizeof( BRIDGE_ID ) ) == 0 ) /* IST Root */
    {
      entry->designated_cost = port->portPrio.root_path_cost ; 
    }
    else
    {
      if(port->infoInternal == 0)
      {
        entry->designated_cost = port->portPrio.root_path_cost ; 
      }
      else
      {
        entry->designated_cost = port->portPrio.region_root_path_cost ; 
      }
    }
    if ( port->role == AlternatePort || port->role == RootPort )
    {
      entry->designated_cost += port->operPCost ;
    }
  }
  else /* MSTI */
  {
    PORT_T *cist_port = NULL ;
    cist_port = stp_port_mst_findport(STP_CIST_ID, port->port_index ) ;
    if ( cist_port == NULL )
    {
      return -1;
    }
	//entry->linkState = port->adminEnable;
    entry->designated_cost = port->portPrio.region_root_path_cost ;
    if( port->role == RootPort )
    {
      entry->designated_cost += port->operPCost ;
    }
    else if( port->role == AlternatePort )
    {
      if(!(cist_port->infoInternal == 0 && cist_port->role == AlternatePort) )
      {
        entry->designated_cost += port->operPCost ;
      }
    }
  } 
  stp_mgmt_convert_brgid2uid (&port->portPrio.design_bridge, &entry->designated_bridge);
  entry->designated_port = port->portPrio.design_port;
  entry->role = port->role;
#if 0
  switch (port->role) {
    case DisabledPort:   entry->role = '  '; break;
    case AlternatePort:  entry->role = 'A'; break;
    case BackupPort:     entry->role = 'B'; break;
    case RootPort:       entry->role = 'R'; break;
    case DesignatedPort: entry->role = 'D'; break;
    case NonStpPort:     entry->role = '-'; break;
    case MasterPort:     entry->role = 'M'; break;
    default:             entry->role = '?'; break;
  }
 
  if (DisabledPort == port->role || NonStpPort == port->role) {
    memset (&entry->designated_root, 0, sizeof (UID_BRIDGE_ID_T));
    memset (&entry->designated_bridge, 0, sizeof (UID_BRIDGE_ID_T));
    entry->designated_cost = 0;
    entry->designated_port = port->port_id;
  }
  entry->rx_cfg_bpdu_cnt = port->rx_cfg_bpdu_cnt;
  entry->rx_rstp_bpdu_cnt = port->rx_rstp_bpdu_cnt;
  entry->rx_tcn_bpdu_cnt = port->rx_tcn_bpdu_cnt;

  entry->fdWhile =       port->fdWhile;      /* 17.15.1 */
  entry->helloWhen =     port->helloWhen;    /* 17.15.2 */
  entry->mdelayWhile =   port->mdelayWhile;  /* 17.15.3 */
  entry->rbWhile =       port->rbWhile;      /* 17.15.4 */
  entry->rcvdInfoWhile = port->rcvdInfoWhile;/* 17.15.5 */
  entry->rrWhile =       port->rrWhile;      /* 17.15.6 */
  entry->tcWhile =       port->tcWhile;      /* 17.15.7 */
  entry->txCount =       port->txCount;      /* 17.18.40 */
  entry->lnkWhile =      port->lnkWhile;

  entry->rcvdInfoWhile = port->rcvdInfoWhile;
  entry->top_change_ack = port->tcAck;
  entry->tc = port->tc;
 #endif
  if (DisabledPort == port->role) {
    entry->oper_point2point = port->adminPointToPointMac;
			/*(P2P_FORCE_FALSE == port->adminPointToPointMac) ? 0 : 1;*/
    entry->oper_edge = port->adminEdge;
    entry->oper_stp_neigb = 0;
  } else {
    entry->oper_point2point = port->operPointToPointMac ? 1 : 0;
    entry->oper_edge = port->operEdge                   ? 1 : 0;
    entry->oper_stp_neigb = port->sendRSTP              ? 0 : 1;
  }
  entry->oper_port_path_cost = port->operPCost;


  
  return 0; 
}
    
static int
stp_mgmt_get_port_cfg (PORT_T* port, UID_STP_PORT_CFG_T* uid_cfg)
{
  
  RSTP_CRITICAL_PATH_START;

  uid_cfg->field_mask = 0;

  uid_cfg->port_priority = port->port_id >> 8;
  if (uid_cfg->port_priority != DEF_PORT_PRIO)
    uid_cfg->field_mask |= PT_CFG_PRIO;

  uid_cfg->admin_port_path_cost = port->adminPCost;
  if (uid_cfg->admin_port_path_cost != ADMIN_PORT_PATH_COST_AUTO)
    uid_cfg->field_mask |= PT_CFG_COST;

  uid_cfg->admin_point2point = port->adminPointToPointMac;
  if (uid_cfg->admin_point2point != DEF_P2P)
    uid_cfg->field_mask |= PT_CFG_P2P;

  uid_cfg->admin_edge = port->adminEdge;
  if (uid_cfg->admin_edge != DEF_ADMIN_EDGE)
    uid_cfg->field_mask |= PT_CFG_EDGE;
    
  RSTP_CRITICAL_PATH_END;
  return 0;
}

int stp_mgmt_port_info(IN PORT_T* port,OUT UID_STP_PORT_STATE_T* portInfo)
{
	int rc;

	RSTP_CRITICAL_PATH_START;
	rc = stp_mgmt_get_port_state (port, portInfo);
	if (rc) {
	  printf ("can't get port state: %s\n", stp_in_get_error_explanation (rc));
	  return -1;
	}

	RSTP_CRITICAL_PATH_END;
	return 0;
}

int
stp_mgmt_port_state_show (IN PORT_T *port, IN Bool detail)
{
	register STPM_T* this = port->owner;
	int rc;
	UID_STP_STATE_T      uid_state;
	UID_STP_PORT_STATE_T uid_port;
	UID_STP_PORT_CFG_T   uid_cfg;

	rc = stp_mgmt_stpm_get_state (this, &uid_state);
	if (rc) {
	  printf ("can't get instance state: %s\n", stp_in_get_error_explanation (rc));
	  return -1;
	} 
	rc = stp_mgmt_get_port_state (port, &uid_port);
	if (rc) {
	  printf ("can't get port state: %s\n", stp_in_get_error_explanation (rc));
	  return -1;
	}
	memset (&uid_cfg, 0, sizeof (UID_STP_PORT_CFG_T));
	rc = stp_mgmt_get_port_cfg (port, &uid_cfg);
	if (rc) {
	  printf ("can't get  port config: %s\n", stp_in_get_error_explanation (rc));
	  return -1;
	}

	if (detail) {
	  printf("Stp Port "); stp_mgmt_out_port_id (port->port_index, False);
	  printf(": PortId: %04lx in Bridge '%s':\n",
	    (unsigned long) uid_port.port_id, uid_state.vlan_name);

	  printf ("Priority:          %-d\n", (int) (uid_port.port_id >> 8));
	  printf ("State:             %-16s", stp_state2str (uid_port.state, 1));
	  //printf ("       Uptime: %-9lu\n", uid_port.uptime);
	  printf ("PortPathCost:      admin: ");
	  if (ADMIN_PORT_PATH_COST_AUTO == uid_cfg.admin_port_path_cost)
	    printf ("%-9s", "Auto");
	  else
	    printf ("%-9lu", uid_cfg.admin_port_path_cost);
	  printf ("       oper: %-9lu\n", uid_port.oper_port_path_cost);

	  printf ("Point2Point:       admin: ");
	  switch (uid_cfg.admin_point2point) {
	    case P2P_FORCE_TRUE_E:
	      printf ("%-9s", "ForceYes");
	      break;
	    case P2P_FORCE_FALSE_E:
	      printf ("%-9s", "ForceNo");
	      break;
	    case P2P_AUTO_E:
	      printf ("%-9s", "Auto");
	      break;
	  }
	  printf ("       oper: %-9s\n", uid_port.oper_point2point ? "Yes" : "No");
	  printf ("Edge:              admin: %-9s       oper: %-9s\n",
	          uid_cfg.admin_edge ? "Y" : "N",
	          uid_port.oper_edge ? "Y" : "N");
	  printf ("Partner:                                  oper: %-9s\n",
	          uid_port.oper_stp_neigb ? "Slow" : "Rapid");
	    
	  if (0 != uid_port.role) {
	    if (6 != uid_port.role) {
	      printf("PathCost:          %-lu\n", (unsigned long) (uid_port.path_cost));
	      printf("Designated Root:   "); stp_mgmt_print_bridge_id (&uid_port.designated_root, 1);
	      printf("Designated Cost:   %-ld\n", (unsigned long) uid_port.designated_cost);
	      printf("Designated Bridge: "); stp_mgmt_print_bridge_id (&uid_port.designated_bridge, 1);
	      printf("Designated Port:   %-4lx\n\r", (unsigned long) uid_port.designated_port);
	    }
	    printf("Role:              ");
	    switch (uid_port.role) {
	      case 1: printf("Alternate\n"); break;
	      case 2: printf("Backup\n"); break;
	      case 3: printf("Root\n"); break;
	      case 4: printf("Designated\n"); break;
	 case 5: printf("Master\n"); break;
	      case 6: printf("NonStp\n"); break;
	      default:  printf("Unknown(%d)\n", uid_port.role); break;
	    }

	    if (3 == uid_port.role || 4 == uid_port.role) {
	      /* printf("Tc:                %c  ", uid_port.tc ? 'Y' : 'n'); */
	      printf("TcAck:             %c  ",
	           uid_port.top_change_ack ?  'Y' : 'N');
	      //printf("TcWhile:       %3d\n", (int) uid_port.tcWhile);
	    }
	  }

	  if (UID_PORT_DISABLED == uid_port.state || 6 == uid_port.role) {
#if 0
	    printf("helloWhen:       %3d  ", (int) uid_port.helloWhen);
	    printf("lnkWhile:      %3d\n", (int) uid_port.lnkWhile);
	    printf("fdWhile:         %3d\n", (int) uid_port.fdWhile);
#endif
	  } else if (6 != uid_port.role) {
#if 0	   
		printf("fdWhile:         %3d  ", (int) uid_port.fdWhile);
	    printf("rcvdInfoWhile: %3d\n", (int) uid_port.rcvdInfoWhile);
	    printf("rbWhile:         %3d  ", (int) uid_port.rbWhile);
	    printf("rrWhile:       %3d\n", (int) uid_port.rrWhile);

	    printf("mdelayWhile:     %3d  ", (int) uid_port.mdelayWhile);
	    printf("lnkWhile:      %3d\n", (int) uid_port.lnkWhile);
	    printf("helloWhen:       %3d  ", (int) uid_port.helloWhen);
	    printf("txCount:       %3d\n", (int) uid_port.txCount);
#endif
	  }
#if 0
	  printf("RSTP BPDU rx:      %lu\n", (unsigned long) uid_port.rx_rstp_bpdu_cnt);
	  printf("CONFIG BPDU rx:    %lu\n", (unsigned long) uid_port.rx_cfg_bpdu_cnt);
	  printf("TCN BPDU rx:       %lu\n", (unsigned long) uid_port.rx_tcn_bpdu_cnt);
#endif
	} else {
	  printf("%-6lx %-9lu %-4c %-5s ",uid_port.port_id,uid_port.path_cost,\
		uid_port.role, stp_state2str (uid_port.state, 0));
	  stp_mgmt_print_bridge_id (&uid_port.designated_bridge, 0);
	  printf("  %-7lx  %-9lu", (unsigned long) uid_port.designated_port, uid_port.designated_cost);
	  printf("%-3c  %-4c %-5c ",
	    (uid_port.oper_point2point) ? 'Y' : 'N',
	    (uid_port.oper_edge) ?        'Y' : ' N', 
	    (uid_port.oper_stp_neigb ) ? 'S' : 'R');
	 
	  printf ("\n");
	}
	RSTP_CRITICAL_PATH_END;
	return 0;
}

int
stp_mgmt_stpm_state_show (IN unsigned short MSTID)
{
  register STPM_T* this;
  register PORT_T* port;
  register STPM_T*  cist_stpm = (STPM_T *)stp_stpm_get_the_cist();

  this = stp_stpm_get_instance(MSTID);

  if (!this || !cist_stpm) { /* it had not yet been created :( */
    return STP_Vlan_Had_Not_Yet_Been_Created;
  }
  printf("Name %s,  Revision %d\n", cist_stpm->MstConfigId.ConfigurationName, *((short *)cist_stpm->MstConfigId.RevisionLevel));
  printf("Instance: %-7u State: ",
         MSTID);
  switch (this->admin_state) {
    case STP_ENABLED:  printf("enabled\n"); break;
    case STP_DISABLED: printf("disabled\n");break;
    default:           printf("unknown\n"); return 0;
  }
  if(STP_ENABLED != this->admin_state)
    return STP_OK;
  
  printf("Vlan Map:      ");
  stp_mgmt_vlan_show(this);
  printf("\n");
  printf("BridgeId:   "); stp_mgmt_print_bridge_id (&this->BrId, 0);
  printf("\n");
  printf ("Bridge Max Age:  %-2d\n", (int) this->BrTimes.MaxAge);
  printf ("Bridge Hello Time: %-2d\n", (int) this->BrTimes.HelloTime);
  printf ("Bridge Forward Delay: %-2d\n", (int) this->BrTimes.ForwardDelay);
  printf ("Bridge Remaining Hops: %-2d\n", (int) this->BrTimes.RemainingHops);
  printf ("Hold Time: %2d\n", (int) TxHoldCount); 
  printf("Force Version: %u\n",this->ForceVersion);

  if(STP_CIST_ID == this->vlan_id)
  {
    printf("Root Bridge: "); stp_mgmt_print_bridge_id (&this->rootPrio.root_bridge, 1);
    if (this->rootPortId) {
      printf("Root Port:  %04lx (", (unsigned long) this->rootPortId);
	  stp_mgmt_out_port_id (this->rootPortId & 0xfff, False);
      printf("), Root Cost: %-lu\n", (unsigned long) this->rootPrio.root_path_cost);
    } else {
      printf("Root Port:  none\n");
    }
    printf("Region Root Bridge: "); stp_mgmt_print_bridge_id (&this->rootPrio.region_root_bridge, 1);
    printf("Region Root Cost: %-lu\n", (unsigned long) this->rootPrio.region_root_path_cost);
    printf ("Max Age:  %2d\n", (int) this->rootTimes.MaxAge);
    printf ("Hello Time:  %2d\n",(int) this->rootTimes.ForwardDelay);
    printf ("Forward Delay:   %2d\n",(int) this->rootTimes.HelloTime);
  }
  else
  {
    printf("Region Root Bridge: "); stp_mgmt_print_bridge_id (&this->rootPrio.region_root_bridge, 1);
    if (this->rootPortId) {
      printf("Root Port:  %04lx (", (unsigned long) this->rootPortId);
	  stp_mgmt_out_port_id (this->rootPortId & 0xfff, False);
      printf("),Region  Root Cost:  %-lu\n", (unsigned long) this->rootPrio.region_root_path_cost);
    } else {
      printf("Root Port:  none\n");
    }
  }
  printf ("\n");
  printf("Port  oper_cost role state Desi_BridgeId      D_Port root_cost  p2p  edge neigb \n");
  printf ("\n");
  for(port = this->ports; port; port = port->next)
  	stp_mgmt_port_state_show(port, 0);
  return 0;
}
#ifdef __cplusplus
}
#endif

