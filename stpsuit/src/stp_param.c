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
(Istp_NCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* stp_param.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs for stp protocol parameters in stp module
*
* DATE:
*       04/18/2008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.3 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include<string.h>
#include <stdlib.h>

#include <sysdef/returncode.h>
#include "stp_base.h"
#include "stp_bitmap.h"
#include "stp_statmch.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_uid.h"
#include "stp_in.h"
#include "stp_param.h"
#include "stp_to.h"
#include "stp_log.h"
#include "stp_dbus.h"

struct port_duplex_mode** port_duplex_mode_nodes = NULL;

extern int stp_log_level;
extern BITMAP_T  g_VlanPBMP[4096] ;
extern Bool g_flag;
extern UID_STP_MODE_T g_stp_state;
extern BITMAP_T enabled_ports;
extern struct stp_admin_infos **stp_ports;
RUNNING_MODE current_mode = NOT_MODE;



int stp_param_check_stp_state()
{   
	return g_flag;
}

int stp_param_set_debug_value(unsigned int val_mask)
{
	stp_log_level |= val_mask;
	return 0;
}

int stp_param_set_no_debug_value(unsigned int val_mask)
{
	stp_log_level &= ~val_mask;
	return 0;
}

int stp_param_set_stpm_enable()
{
	int ret = 0;
	
	if(False == g_flag) {
		if(0 == (ret = stp_in_global_enable())) {
			g_flag = True;
			g_stp_state = STP_ENABLED;
		}
         else {
			 stp_syslog_dbg("set stpm enable false ");
	    }
	}
	else 
		return STP_HAVE_ENABLED;
	
	return ret;
}

int stp_param_set_stpm_disable()
{
	int ret = 0;

	if(True == g_flag) {
		if(0 == (ret = stp_in_global_disable())) {
			g_flag = False;
			g_stp_state = STP_DISABLED;

			/*for next enable*/
			ret = stp_mgmt_stpm_create_cist();
			if (STP_OK != ret) {
			 stp_trace("FATAL: can't enable:%s\n",
			             stp_in_get_error_explanation (ret));
			  return (-1);
			}
		}
	}
	else
		return STP_NOT_ENABLED;

	return ret;
}

void stp_param_set_running_mode(RUNNING_MODE mode)
{
	current_mode = mode;
}

int stp_param_get_running_mode()
{   
    
	return current_mode;
}

int stp_param_set_stpm_port_enable(unsigned int port_index,int lkState,unsigned int speed, unsigned int isWAN, unsigned int  slot_no, unsigned int  port_no)
{
	int rc = 0;
    struct stp_admin_infos *stp_info = NULL;
	stp_syslog_dbg("RSTP>>npd_cmd108:: STP_enable port index %d link %s\n",	\
		port_index,lkState ? "DOWN":"UP");
	if(!g_flag){
		return STP_NOT_ENABLED;
	}
	if(NULL != stp_ports){
        stp_info = stp_ports[port_index];
	}
	else{
       return STP_NOT_ENABLED;
	}
    if(STP_LINK_STATE_UP_E == lkState)
	    stp_bitmap_set_bit(&enabled_ports, port_index);
	if((NULL != stp_info)&&(1 == stp_info->stpEnable)){
		return STP_PORT_HAVE_ENABLED;
	}
	rc = stp_in_enable_port_on_stpm (port_index,1,lkState,speed, isWAN,slot_no,port_no);
    /*
	if(1 == lkState) {
		stp_bitmap_set_bit(&enabled_ports, port_index);
		rc = stp_in_enable_port_on_stpm (port_index,1,lkState,speed);
	}
	else{
			rc = STP_PORT_NOT_EXIST;
		}
	*/
	if(!rc){
        rc = STP_RETURN_CODE_SUCCESS;
	}
    else{
        rc = STP_RETURN_CODE_MSTP_NOT_ENABLED;
    }
	return rc;
}

int stp_param_set_stpm_port_disable
(
	unsigned int port_index, 
	int lkState,
	unsigned int speed, 
	unsigned int isWAN,
	unsigned int slot_no,
	unsigned int port_no
)
{
	int rc = 0;
	struct stp_admin_infos *stp_info =NULL;
	
	stp_syslog_dbg("RSTP>>npd_cmd108:: STP_enable \n");
	if(!g_flag){
		return STP_NOT_ENABLED;
	}
	if((NULL != stp_ports)&&(NULL != stp_ports[port_index])){
	   stp_info = stp_ports[port_index];
	}
	else {
        return STP_NOT_ENABLED;
	}
	if((NULL != stp_info)&&(0 == stp_info->stpEnable)){
		return STP_PORT_NOT_ENABLE;
	}
		
	rc = stp_in_enable_port_on_stpm (port_index,0,lkState,speed, isWAN,slot_no,port_no);
	/*
	if(1 == lkState){
		rc = stp_in_enable_port_on_stpm (port_index,0,speed);
	}
	else {
		rc = STP_PORT_NOT_EXIST;
	}
      */
	if(!rc){
        rc = STP_RETURN_CODE_SUCCESS;
	}
    else{
        rc = STP_RETURN_CODE_MSTP_NOT_ENABLED;
    }
	return rc;
}

#if 0
int add_ports_to_mstp(unsigned short vid,unsigned int port_index)
{
	int rc = 0;
	
	if(vid < 1 || vid > STP_MAX_VID || port_index < 0)
		return -1;
	printf("stp_param89:: add_ports_to_mstp\n");
	rc = stp_in_add_port_to_mstp(vid,port_index);
	return rc;

}

int del_ports_from_mstp(unsigned short vid,unsigned int port_index)
{
	int rc = 0;
	
	if(vid < 1 || vid > STP_MAX_VID || port_index < 0)
		return -1;

	printf("stp_param102 :: del_ports_from_mstp\n");
	rc = stp_in_del_port_from_mstp(vid,port_index);
	return rc;
}

int del_vlan_from_mstp(unsigned short vid)
{
	int ret = 0;
	ret = stp_in_del_vlan_from_cist(vid);
	return ret;
}
#endif

int stp_param_add_vlan_to_mstp(unsigned short vid,BITMAP_T ports)
{
	int ret = 0;

	ret = stp_in_add_vlan_to_cist (vid, ports);

	return ret;
		
}

void stp_param_mstp_init_vlan_info(unsigned short vid,BITMAP_T ports)
{
	 stp_mgmt_init_vlanpbmp(vid,ports);
}

int stp_param_set_vlan_to_instance (unsigned short vid, int mstid)
{

	return stp_in_set_vlan_2_instance (vid, mstid);	
}

int stp_param_set_bridge_region_name(char* name)
{
	STPM_T *stpm = NULL;

	stpm = stp_stpm_get_the_cist();
	if(stpm)
	{
		memset(stpm->MstConfigId.ConfigurationName, 0, 32);
		strcpy(stpm->MstConfigId.ConfigurationName, name);
		return 0;
	}

	return -1;
	
}

int stp_param_set_port_duplex_mode(unsigned int port_index,unsigned int duplex_mode)
{
	struct port_duplex_mode* port_duplex_mode_node = NULL;
    
	if((NULL != port_duplex_mode_nodes)&&(NULL != port_duplex_mode_nodes[port_index])){
        port_duplex_mode_node = port_duplex_mode_nodes[port_index];
	}
	else{
		port_duplex_mode_node = (struct port_duplex_mode*)malloc(sizeof(struct port_duplex_mode));
		if(NULL == port_duplex_mode_node){
			/*if get failed ,use the default value*/
			return STP_ERR;
		}
		else {
           memset(port_duplex_mode_node,0,sizeof(struct port_duplex_mode));
		}
	}
   if(NULL != port_duplex_mode_nodes){
	   port_duplex_mode_node->port_index = port_index;
	   if(port_duplex_mode_node->port_duplex_mode != duplex_mode){
           port_duplex_mode_node->port_duplex_mode = duplex_mode;
	       port_duplex_mode_nodes[port_index] = port_duplex_mode_node;
		   stp_in_change_port_duplex(port_index);
	   }
       port_duplex_mode_nodes[port_index] = port_duplex_mode_node;
   }
   else{
      free(port_duplex_mode_node);
	  port_duplex_mode_node = NULL;
   }
  
	return STP_OK;
	
}

int stp_param_set_bridge_revision(unsigned short value)
{   
	STPM_T *stpm  = NULL;

	stpm = stp_stpm_get_the_cist();
	if(stpm)
	{
	    *((short *)stpm->MstConfigId.RevisionLevel) = value;
		return 0;
	}

	return -1;

}


static int stp_param_set_bridge_cfg_value 
(
	unsigned short mstid, 
	unsigned int value, 
	unsigned int val_mask
)
{
  UID_STP_CFG_T uid_cfg = {0};
  char*         val_name;
  int           rc;
  STPM_T *stpm = NULL;

  uid_cfg.field_mask = val_mask;
  switch (val_mask) {
    case BR_CFG_STATE:
      uid_cfg.stp_enabled = value;
      val_name = "state";
      break;
    case BR_CFG_PRIO:
      uid_cfg.bridge_priority = value;
      val_name = "priority";
      break;
    case BR_CFG_AGE:
      uid_cfg.max_age = value;
      val_name = "max_age";
      break;
    case BR_CFG_HELLO:
      uid_cfg.hello_time = value;
      val_name = "hello_time";
      break;
    case BR_CFG_DELAY:
      uid_cfg.forward_delay = value;
      val_name = "forward_delay";
      break;
    case BR_CFG_FORCE_VER:
		printf("forceversion %d\n",value);
      uid_cfg.force_version = value;
      val_name = "force_version";
      break;
    case BR_CFG_MAX_HOPS:
      uid_cfg.maxhops = value;
      val_name = "max_hops";
      break;		
    case BR_CFG_AGE_MODE:
    case BR_CFG_AGE_TIME:
    default: printf ("Invalid value mask 0X%x\n", val_mask);  return;
      break;
  }
  
  //printf("##set_value 256P	priority value %d, vla_mask: %02x\n", uid_cfg.bridge_priority, val_mask);

  if(BR_CFG_MAX_HOPS == val_mask)
  {
    for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next)
    {
        rc = stp_in_stpm_set_cfg (stpm,  &uid_cfg);
        if (0 != rc) {
          printf ("Can't change rstp bridge %s:%s", val_name, stp_in_get_error_explanation (rc));
		  return rc;
        }
    }
  }
  else
  {
  	//printf("uid_cfg.force_version %d mstid %d prio %d\n",uid_cfg.force_version,mstid,uid_cfg.bridge_priority);
     stpm = stp_stpm_get_instance(mstid);
     rc = STP_Vlan_Had_Not_Yet_Been_Created;
     if(NULL != stpm){
       rc = stp_in_stpm_set_cfg (stpm,&uid_cfg);
	   //printf("forceversion %d\n",stpm->ForceVersion);
     }

  }
  if (0 != rc) {
    printf ("Can't change rstp bridge %s:%s\n", val_name, stp_in_get_error_explanation (rc));
	return rc;
  } else {
    printf ("Changed rstp bridge %s\n", val_name);
  }
}

int stp_param_set_bridge_priority(int mstid,int prio)
{	
	int rc = 0;
	STPM_T *stpm = NULL;
	unsigned int flag = 0;/*flag that if the mstid exist*/
	
	/*first check the mstid exist or not*/
	for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next) {
		if(mstid == stpm->vlan_id){
           flag = 1;
		   break;
		}
	}
	if(1 == flag){
	   rc = stp_param_set_bridge_cfg_value (mstid, prio, BR_CFG_PRIO);
	}
	else {
       rc = STP_Vlan_Had_Not_Yet_Been_Created;
	}
	return rc;
}

int stp_param_reset_bridge_priority(int mstpid)
{
	int ret = 0;
	unsigned long value = DEF_BR_PRIO;
	/*UID_STP_CFG_T uid_cfg;
  	STPM_T *stpm;
	for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next)
	{
		  uid_cfg.field_mask = BR_CFG_PRIO;
		  uid_cfg.bridge_priority = DEF_BR_PRIO;
		  ret |= stp_in_stpm_set_cfg (stpm,  &uid_cfg);
		  if (ret) {
		  	STP_LOG(STP_LOG_DEBUG,"can't reset bridge priority: %s\n", stp_in_get_error_explanation (ret));
		  } 
	}*/

	ret = stp_param_set_bridge_cfg_value(mstpid,  value, BR_CFG_PRIO);
	return ret;
	
}

int stp_param_set_bridge_maxage(int maxage)
{
	int rc = 0;
	//why is not the value of the rc changed?
	rc = stp_param_set_bridge_cfg_value (0,maxage, BR_CFG_AGE);
	return rc;

}

int stp_param_reset_bridge_maxage(int mstid)
{
	int ret = 0;
	long  maxage = DEF_BR_MAXAGE;
//Why is not the value of ret changed
	ret = stp_param_set_bridge_cfg_value(mstid,maxage,BR_CFG_AGE);
	return ret;
}

int stp_param_set_bridge_fdelay(int value)
{
	int rc = 0;

	rc = stp_param_set_bridge_cfg_value(0,value,BR_CFG_DELAY);
	return rc;
}


int stp_param_reset_bridge_fdelay(int mstid)
{
	int rc = 0;
	long  value = DEF_BR_FWDELAY;

	rc = stp_param_set_bridge_cfg_value(mstid,value,BR_CFG_DELAY);
	return rc;
}

int stp_param_set_bridge_fvers(int fvers)
{
	int rc  = 0;

	if(FORCE_STP_COMPAT == fvers || NORMAL_RSTP == fvers) {
		current_mode = STP_MODE;
	}
	else if(NORMAL_MSTP == fvers) {
		current_mode = MST_MODE;
	}
	
	rc = stp_param_set_bridge_cfg_value (0,fvers, BR_CFG_FORCE_VER);
	return rc;
}

int stp_param_reset_bridge_fvers()
{
	int rc = 0;
	long  value = DEF_FORCE_VERS;

	rc = stp_param_set_bridge_cfg_value(0,value,BR_CFG_FORCE_VER);
	return rc;

}

int stp_param_set_bridge_max_hops(unsigned int value)
{
    unsigned int ret = 0;
	
	ret = stp_param_set_bridge_cfg_value(0,value,BR_CFG_MAX_HOPS);
	return ret;
}

int stp_param_reset_bridge_max_hops()
{
	long value = DEF_REMAINING_HOPS;
	unsigned int ret = 0;
	ret = stp_param_set_bridge_cfg_value(0,value,BR_CFG_MAX_HOPS);

	return ret;
}

int stp_param_set_bridge_hello_time(unsigned int value)
{
    unsigned int ret = 0;
	ret = stp_param_set_bridge_cfg_value(0,value,BR_CFG_HELLO);
	return ret;
}

int stp_param_reset_bridge_hello_time(int mstid)
{
	long value = DEF_BR_HELLOT;
	unsigned int ret = 0;
	
	ret = stp_param_set_bridge_cfg_value(mstid,value,BR_CFG_HELLO);

	return ret;
}


static int stp_param_set_port_cfg_value 
(
	unsigned short mstid,
	unsigned int 	port_index,
	unsigned int  value,
	unsigned int  val_mask
)
{
  UID_STP_PORT_CFG_T uid_cfg;
  int           rc = 0;
  char          *val_name = NULL;
  PORT_T *port = NULL;
  char * exprion = NULL;
  struct stp_admin_infos *stp_info = NULL;

  uid_cfg.field_mask = val_mask;
  switch (val_mask) {
    case PT_CFG_MCHECK:
      val_name = "mcheck";
      break;
    case PT_CFG_COST:
      uid_cfg.admin_port_path_cost = value;
      val_name = "path cost";
      break;
    case PT_CFG_PRIO:
      uid_cfg.port_priority = value;
      val_name = "priority";
      break;
    case PT_CFG_P2P:
      uid_cfg.admin_point2point = (ADMIN_P2P_T) value;
      val_name = "p2p flag";
      break;
    case PT_CFG_EDGE:
      uid_cfg.admin_edge = value;
      val_name = "adminEdge";
      break;
    case PT_CFG_NON_STP:
      uid_cfg.admin_non_stp = value;
      val_name = "adminNonStp";
      break;
#ifdef STP_DBG
    case PT_CFG_DBG_SKIP_TX:
      uid_cfg.skip_tx = value;
      val_name = "skip tx";
      break;
    case PT_CFG_DBG_SKIP_RX:
      uid_cfg.skip_rx = value;
      val_name = "skip rx";
      break;
#endif
    case PT_CFG_STATE:
    default:
      printf ("Invalid value mask 0X%x\n", val_mask);
      return;
  }
  
  
  if (!(port_index < 0)) {

    port = stp_port_mst_findport(mstid, port_index);
	STP_DBUS_DEBUG(("##port index %d, port %p\n", port_index, port));
    rc = STP_OK;
    if(NULL != port)
    {
      
	  /*config the port protocol*/
      rc = stp_in_set_port_cfg (port, &uid_cfg);
      if(PT_CFG_EDGE == val_mask || PT_CFG_NON_STP == val_mask || PT_CFG_P2P == val_mask)
      {
        PORT_T *mstport = NULL;
        for(mstport = port->nextMst; mstport; mstport = mstport->nextMst)
	       rc = stp_in_set_port_cfg (mstport, &uid_cfg);	
      }
	  
      /*for save stp infos*/  
	  if((NULL != stp_ports)&&(NULL != stp_ports[port->port_index])){
	  	 stp_info = stp_ports[port->port_index];
		 stp_info->mstid[mstid] = mstid;
	  	 switch (val_mask) {
		    case PT_CFG_COST:
			  stp_info->pathcost[mstid]= uid_cfg.admin_port_path_cost;
		      break;
		    case PT_CFG_PRIO:
		      stp_info->prio[mstid]= uid_cfg.port_priority;				
		      break;
		    case PT_CFG_P2P:
			  stp_info->p2p = uid_cfg.admin_point2point;	
		      break;
		    case PT_CFG_EDGE:
			  stp_info->edge = uid_cfg.admin_edge;
		      break;
		    case PT_CFG_NON_STP:
			  stp_info->nonstp =  uid_cfg.admin_non_stp;
		      break;
		    case PT_CFG_STATE:
		    default:
		      printf ("Invalid value mask 0X%x\n", val_mask);
		      return;
	  	 } 		  
	  }  
    }
	else {
		rc = STP_PORT_NOT_ENABLE;
	}
  }
	
  if (0 != rc) {
		if(NULL != port) {
		exprion = stp_in_get_error_explanation (rc);
	    printf ("can't change rstp port[%s] %s: %s\n",
	          port->port_name,val_name, (exprion) ? "NULL" : exprion);
		}
		return rc;
  } else {
    printf ("changed rstp port[%s] \n", val_name);
	return rc;
  }

}

int stp_param_set_port_past_cost(int mstpid,int port_index,int past_cost)
{
	int rc = 0;
	rc = stp_param_set_port_cfg_value(mstpid,port_index,past_cost,PT_CFG_COST);
	return rc;
}

int stp_param_reset_port_past_cost(int mstpid,int port_index)
{
	int rc = 0;
	
	stp_param_set_port_cfg_value(mstpid,port_index,ADMIN_PORT_PATH_COST_AUTO,PT_CFG_COST);
	return rc;
}

int stp_param_set_port_priority(int mstpid,int port_index,int prio)
{
	int rc = 0;
	rc = stp_param_set_port_cfg_value(mstpid,port_index,prio,PT_CFG_PRIO);
	return rc;
}

int stp_param_reset_port_priority(int mstpid,int port_index)
{
	int rc= 0;
	rc = stp_param_set_port_cfg_value(mstpid,port_index,DEF_PORT_PRIO,PT_CFG_PRIO);
	return rc;
}

int stp_param_set_port_p2p(int mstpid,int port_index,unsigned int value)
{
	int rc = 0;
	stp_param_set_port_cfg_value(mstpid,port_index,value,PT_CFG_P2P);
	return rc;
}

int stp_param_reset_port_p2p(int mstpid,int port_index)
{

	int rc = 0;
	stp_param_set_port_cfg_value(mstpid,port_index,DEF_P2P,PT_CFG_P2P);
	return rc;
}

int stp_param_set_port_edge(int mstpid,int port_index,unsigned char value)
{
	int rc = 0;
	stp_param_set_port_cfg_value(mstpid,port_index,value,PT_CFG_EDGE);
	return rc;
}

int stp_param_reset_port_edge(int mstpid,int port_index)
{
	int rc = 0;
	stp_param_set_port_cfg_value(mstpid,port_index,DEF_ADMIN_EDGE,PT_CFG_EDGE);
	return rc;
}

int stp_param_set_port_mcheck(int mstpid,int port_index,unsigned char value)
{
	int rc = 0;
	stp_param_set_port_cfg_value(mstpid,port_index,value,PT_CFG_MCHECK);
	return rc;
}

int stp_param_reset_port_mcheck(int mstpid,int port_index)
{
	//stp_param_set_port_cfg_value(port_index,value,PT_CFG_MCHECK);
	return 0;
}

 int stp_param_set_port_non_stp(int mstpid,unsigned int port_index,unsigned int isEn)
 {
	stp_param_set_port_cfg_value (mstpid, port_index, isEn, PT_CFG_NON_STP);
	return 0;
 }

int stp_param_reset_port_non_stp(int mstpid,unsigned int port_index)
{
	stp_param_set_port_cfg_value (mstpid, port_index,DEF_ADMIN_NON_STP, PT_CFG_NON_STP);
	return 0;
}

 int stp_param_set_bridge_nocfg (int mstpid)
 {
 	int ret = 0;

	stp_param_reset_bridge_priority(mstpid);
	
	stp_param_reset_bridge_fdelay(mstpid);
	stp_param_reset_bridge_hello_time(mstpid);
	stp_param_reset_bridge_maxage(mstpid);
	stp_param_reset_bridge_max_hops();
	if(MST_MODE == current_mode)
		stp_param_set_bridge_cfg_value(mstpid,NORMAL_MSTP,BR_CFG_FORCE_VER);
	else if(STP_MODE == current_mode) 
		stp_param_reset_bridge_fvers();

	
	return ret;
 }

 int stp_param_set_port_cfg_defaultvalue (int mstpid, int port_index)
 {
 	int ret = 0;

	ret = stp_param_reset_port_priority(mstpid,port_index);
	ret += stp_param_reset_port_past_cost(mstpid,port_index);
	ret += stp_param_reset_port_edge(mstpid,port_index);
	ret += stp_param_reset_port_p2p(mstpid,port_index);
	ret += stp_param_reset_port_non_stp(mstpid,port_index);
	//ret += stp_param_reset_port_mcheck(port_index);

	return ret;
	
 }


void
stp_param_get_port_state (int mstid,unsigned int port_index,UID_STP_PORT_STATE_T* portInfo)
{
	PORT_T *port = NULL;

	port = stp_port_mst_findport(mstid, port_index);
	if(!port) {
		*((unsigned char *)(&portInfo->port_id)) = 128;
		*((unsigned char *)(&portInfo->port_id) + 1) = 128;
		portInfo->linkState = False;
		portInfo->path_cost = 2000000;
		portInfo->oper_point2point = P2P_AUTO_E;
		portInfo->oper_edge = False;
		portInfo->role = DisabledPort;
		portInfo->state = 0;
		stp_syslog_dbg("can not get instance %d port %d.\n", mstid, port_index);
		return ;
	}
	else {
		stp_syslog_dbg("-_- && -_- mstid %d,port_index %d\n",mstid,port->port_index);
		stp_mgmt_port_info(port,portInfo);
		return ;
	}
}

int stp_param_get_vid_by_mstid(int mstid,unsigned short* vid)
{
	STPM_T* stpm = NULL;
	for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next)
	{
		
	}
	return 0;
}

int stp_param_compare(const void *elem1, const void *elem2) 
{ 
    return *((int *)(elem1)) - *((int *)(elem2));
} 

int stp_param_get_mstp_vlan_map_info(int mstid,unsigned short* map,int* count)
{
	STPM_T *stpm = NULL;
	int i,cnt = 0,j = 0;
	
	stpm = stp_stpm_get_instance(mstid);
	if(stpm) {
		for(i = 1; i < 4095; i++) {//zhengcaisheng change i = 0 => i = 1
			if(0 == mstid) {
				if((stpm->vlan_map.bmp[i/8] & (1u <<(i % 8)))) {
	        		cnt ++;
					if( 0 != i){
					   map[j] = i;
					}
					j++;
				}
			}
			else {
				if(stpm->vlan_map.bmp[i/8] & (1u <<(i % 8))) {
	        		cnt ++;
					map[j] = i;
					j++;
				}
			}
		}
		/* order the vid */
		if(0 != cnt){
             qsort(map, cnt, sizeof(unsigned short), stp_param_compare); 
		}
	}

	*count = cnt;
}

int stp_param_check_port_if_in_mst
(
    int mstid,
    unsigned int port_index
)
{
    unsigned short vid_array[4095] = {0},vid = 0;
	int count = 0,i = 0,j = 0;
	unsigned int index = 0;
	BITMAP_T bmp ;

	memset(&bmp,0,sizeof(BITMAP_T));

	/*parse port_index to index 0-23*/
	index = stp_port_global_index_parse(port_index);
	/*get vlan map in mst*/
	stp_param_get_mstp_vlan_map_info(mstid,vid_array,&count);
    /*get port bmp in each vlan*/
	for(;i<count;i++){
	   vid = vid_array[i];
       if (! stp_bitmap_get_bit(&g_VlanPBMP[vid],index)) {
	   	continue;
       }
	   else{
          return STP_OK;
	   }
	}
	return STP_Cannot_Find_Vlan;//PORT NOT IN MSTID

}
int stp_param_save_stp_mode_cfg
(
	unsigned char* buf,
	unsigned int 	buflen
)
{
	STPM_T* stpm = NULL;
	PORT_T* port = NULL;
	UID_STP_CFG_T cfg = {0};
	char* tmpPtr = buf;
	int length = 0;
	unsigned int port_index = 0;
	unsigned char slotno = 0,portno = 0;
	unsigned int ret = 0,cfgflag = 0;

	stp_to_get_init_stpm_cfg (1,&cfg);
	
	stpm = stp_stpm_get_the_cist();
	if(stpm) {

	    if(0 == cfgflag && ((length + 28) < STP_RUNNING_CFG_MEM)) {
			length += sprintf(tmpPtr," config spanning-tree stp\n");
			tmpPtr = buf + length;
			length += sprintf(tmpPtr," config spanning-tree enable\n");
		    tmpPtr = buf + length;
			cfgflag = 1;
		}
		else {
            return 0;
		}
	
		if(cfg.bridge_priority != stpm->BrId.prio) {
			length += sprintf(tmpPtr," config spanning-tree priority %d\n",stpm->BrId.prio);
			tmpPtr = buf + length;
		}

		if(cfg.max_age != stpm->BrTimes.MaxAge) {
			length += sprintf(tmpPtr," config spanning-tree max-age %d\n",stpm->BrTimes.MaxAge);
			tmpPtr = buf + length;
		}

		if(cfg.forward_delay != stpm->BrTimes.ForwardDelay) {
			length += sprintf(tmpPtr," config spanning-tree forward-delay %d\n", stpm->BrTimes.ForwardDelay);
			tmpPtr = buf + length;
		}

		if(cfg.hello_time != stpm->BrTimes.HelloTime) {
			length += sprintf(tmpPtr," config spanning-tree hello-time %d\n", stpm->BrTimes.HelloTime);
			tmpPtr = buf + length;
		}

		if(cfg.force_version != stpm->ForceVersion) {
			length += sprintf(tmpPtr," config spanning-tree force-version %d\n", stpm->ForceVersion);
			tmpPtr = buf + length;
		}
		
        for(port_index = 0;port_index < PORT_INDEX_MAX;port_index++){
           if(NULL != stp_ports){
			  if(NULL != stp_ports[port_index]){
                 struct stp_admin_infos *stp_info = stp_ports[port_index];
				 if(NULL != stp_info){
                    printf("port_index %d\n",port_index);
				 }
				 ret = stp_bitmap_get_panelport_from_portindex(port_index,&slotno,&portno);
				 if(STP_OK == ret){
					 if( stp_info->stpEnable){
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d enable\n",slotno,portno);
						 //printf("stpInfo->stpEnable %d\n", stpInfo->stpEnable);
						 tmpPtr = buf+length;
					 }
					 if(DEF_PORT_PRIO != stp_info->prio[0]) {
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d priority %d\n",slotno,portno,stp_info->prio[0]);
							//printf("stpInfo->priority %ld\n", stpInfo->prio[MIN_MST_ID]);
						 tmpPtr = buf+length;
					 }
					 if(ADMIN_PORT_PATH_COST_AUTO != stp_info->pathcost[0]){
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d path-cost %d\n",slotno,portno,stp_info->pathcost[0]);
						 tmpPtr = buf+length;
					 }

					 if(DEF_ADMIN_EDGE != stp_info->edge) {
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d edge no\n",slotno,portno);
						 tmpPtr = buf+length;
					 }

					 if(DEF_P2P !=stp_info->p2p) {
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d p2p %s\n",slotno,portno,stp_info->p2p ? "yes" : "no");
						 tmpPtr = buf+length;
					 }		
					 if(DEF_ADMIN_NON_STP != stp_info->nonstp) {
						 length += sprintf(tmpPtr,"config spanning-tree eth-port %d/%d none-stp yes\n",slotno,portno);
						 tmpPtr = buf+length;
					 }
				 }
			}
		  }
		}
	}
	if(cfgflag){
		length += sprintf(tmpPtr," exit\n");
		tmpPtr = buf+length;
		cfgflag = 0;
	}	
	return 0;
}

int stp_param_save_mstp_mode_cfg 
(
	unsigned char* buf,
	unsigned int 	buflen
)
{
	STPM_T* stpm = NULL;
	PORT_T* port = NULL;
	UID_STP_CFG_T cfg = {0};
	char* tmpPtr = buf;
	int length = 0;
	int i=0,j=0;
	unsigned int mstid = 0,count = 0;
	unsigned short vid[4095] = {0};
	unsigned int port_index = 0;
	unsigned char slotno = 0,portno = 0;
	unsigned int ret = 0,cfgflag = 0;

	stp_to_get_init_stpm_cfg (1,&cfg);
	
	stpm = stp_stpm_get_the_cist();
	if(stpm) {
	    if(0 == cfgflag && ((length + 28) < STP_RUNNING_CFG_MEM)) {
			length += sprintf(tmpPtr," config spanning-tree mst\n");
			tmpPtr = buf + length;
			length += sprintf(tmpPtr," config spanning-tree enable\n");
		    tmpPtr = buf + length;
			cfgflag = 1;
		}
		else {
            return 0;
		}
	//	for(i = 0; i < 32;i++) {
		if((stpm->MstConfigId.ConfigurationName[0] != 0)&& \
			(strcmp(stpm->MstConfigId.ConfigurationName,"000000000000"))){
			length += sprintf(tmpPtr," config spanning-tree region-name %s\n",	\
				stpm->MstConfigId.ConfigurationName);
			tmpPtr = buf + length;
		}
	//	}

		if(0 != *((short *)stpm->MstConfigId.RevisionLevel)){
				length += sprintf(tmpPtr," config spanning-tree revision %d\n",*((short *)stpm->MstConfigId.RevisionLevel));
				tmpPtr = buf + length;
		}

		if(cfg.max_age != stpm->BrTimes.MaxAge) {
			length += sprintf(tmpPtr," config spanning-tree max-age %d\n",stpm->BrTimes.MaxAge);
			tmpPtr = buf + length;
		}

		if(cfg.forward_delay != stpm->BrTimes.ForwardDelay) {
			length += sprintf(tmpPtr," config spanning-tree forward-delay %d\n", stpm->BrTimes.ForwardDelay);
			tmpPtr = buf + length;
		}

		if(cfg.hello_time != stpm->BrTimes.HelloTime) {
			length += sprintf(tmpPtr," config spanning-tree hello-time %d\n", stpm->BrTimes.HelloTime);
			tmpPtr = buf + length;
		}

		if(DEF_REMAINING_HOPS != stpm->BrTimes.RemainingHops) {
			length += sprintf(tmpPtr," config spanning-tree max-hops %d\n", stpm->BrTimes.RemainingHops);
			tmpPtr = buf + length;
		}			

		if((length + 30) > buflen) {
			printf("%s %d :: buf too small\n",__FILE__,__LINE__);
			return 0;
		}	
		
		for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next) {
			mstid = stpm->vlan_id;
		    if( 0 != mstid){
				printf("The mstid is %d\n",mstid);
				stp_param_get_mstp_vlan_map_info(mstid,vid,&count);
				for(j = 0;j<count;j++){
					length += sprintf(tmpPtr," config spanning-tree map %d instance %d\n",vid[j],mstid);
					tmpPtr = buf + length;
					if((length + 30) > buflen) {
						printf("%s %d :: buf too small\n",__FILE__,__LINE__);
						return 0;
					}
				}
		    }
			if(cfg.bridge_priority != (stpm->BrId.prio-(0xfff&mstid))) {
				//printf("The stpm->BrId.prio is %d\n",stpm->BrId.prio);
				length += sprintf(tmpPtr," config spanning-tree %d priority %d\n",mstid,(stpm->BrId.prio-(0xfff&mstid)));
				tmpPtr = buf + length;

				if((length + 30) > buflen) {
					printf("%s %d :: buf too small\n",__FILE__,__LINE__);
					return 0;
				}				
			}
		}
		/*for port enable*/
		for(port_index = 0;port_index < PORT_INDEX_MAX;port_index++){
           if(NULL != stp_ports){
			  if(NULL != stp_ports[port_index]){
                 struct stp_admin_infos *stp_info = stp_ports[port_index];
				 ret =  stp_bitmap_get_panelport_from_portindex(port_index,&slotno,&portno);
				 if(STP_OK == ret){
					 if(1 == stp_info->stpEnable){
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d enable\n",slotno,portno);
						 //printf("stpInfo->stpEnable %d\n", stpInfo->stpEnable);
						 tmpPtr = buf+length;
						 if((length + 30) > buflen) {
							printf("%s %d :: buf too small\n",__FILE__,__LINE__);
							return 0;
						 }	
					 }
					 for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next) {
				         mstid = stpm->vlan_id;
						 if((DEF_PORT_PRIO != stp_info->prio[mstid])&&(mstid < 64)) {
							length += sprintf(tmpPtr," config spanning-tree %d eth-port %d/%d priority %d\n",stp_info->mstid[mstid],slotno,portno,stp_info->prio[mstid]);
							  tmpPtr = buf+length;
							  if((length + 53) > buflen) {
								 printf("%s %d :: buf too small\n",__FILE__,__LINE__);
								 return 0;
							  }								  
						  }
						  if((ADMIN_PORT_PATH_COST_AUTO != stp_info->pathcost[mstid])&&(mstid < 64)){
							  length += sprintf(tmpPtr," config spanning-tree %d eth-port %d/%d path-cost %d\n",stp_info->mstid[mstid],slotno,portno,stp_info->pathcost[mstid]);
							  tmpPtr = buf+length;
							  if((length + 53) > buflen) {
								 printf("%s %d :: buf too small\n",__FILE__,__LINE__);
								 return 0;
							  }								  
						  }

					 }
		
					 if(DEF_ADMIN_EDGE != stp_info->edge) {
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d edge no\n",slotno,portno);
						 tmpPtr = buf+length;
						 if((length + 44) > buflen) {
							 printf("%s %d :: buf too small\n",__FILE__,__LINE__);
							 return 0;
						 }								  
					 }
			 
					 if(DEF_P2P != stp_info->p2p) {
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d p2p %s\n",slotno,portno,stp_info->p2p ? "yes" : "no");
						 tmpPtr = buf+length;
						 if((length + 30) > buflen) {
							 printf("%s %d :: buf too small\n",__FILE__,__LINE__);
							 return 0;
						}								  
					 }		 
					 if(DEF_ADMIN_NON_STP != stp_info->nonstp) {
						 length += sprintf(tmpPtr," config spanning-tree eth-port %d/%d none-stp yes\n",slotno,portno);
						 tmpPtr = buf+length;
						 if((length + 30) > buflen) {
							 printf("%s %d :: buf too small\n",__FILE__,__LINE__);
							 return 0;
						 }								  
					 }
				}
			 }
		}
	}

	STPM_T* stpm_digest = NULL;
	stpm_digest = stp_stpm_get_instance(STP_CIST_ID);
	if (NULL != stpm_digest)
	{
		printf("%s %d :: user config digest£\n",__FILE__,__LINE__);
		if (((length + 48) < buflen) &&
			(strlen(stpm_digest->digest) != 0))
		{
			length += sprintf(tmpPtr, " config spanning-tree digest %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
						stpm_digest->digest[0],  stpm_digest->digest[1],  stpm_digest->digest[2],  stpm_digest->digest[3],
						stpm_digest->digest[4],  stpm_digest->digest[5],  stpm_digest->digest[6],  stpm_digest->digest[7],
						stpm_digest->digest[8],  stpm_digest->digest[9],  stpm_digest->digest[10], stpm_digest->digest[11],
						stpm_digest->digest[12], stpm_digest->digest[13], stpm_digest->digest[14], stpm_digest->digest[15]);
			tmpPtr = buf + length;
		}
	}

	stpm_digest = NULL;
	stpm_digest = stp_stpm_get_instance(STP_CIST_ID);
	if (NULL != stpm_digest)
	{
		for (port = stpm_digest->ports; port; port = port->next)
		{
			if ((1 == (port->configDigestSnp & 0x1)) &&
				((length + 60) < buflen))
			{
				ret =  stp_bitmap_get_panelport_from_portindex(port->port_index, &slotno, &portno);
				length += sprintf(tmpPtr, " config spanning-tree eth-port %d/%d digest-snooping enable\n",
											slotno, portno);
				tmpPtr = buf + length;
			}
		}
	}
  }
	if(cfgflag){
		length += sprintf(tmpPtr," exit\n");
		tmpPtr = buf+length;
		cfgflag = 0;
	}	
	return 0;

}

int stp_param_save_running_cfg(char* buf,unsigned int bufLen)
{
	
	if(False == g_flag)
		return STP_NOT_ENABLED;
	else {
		switch (current_mode) {
			case STP_MODE:
				stp_param_save_stp_mode_cfg(buf, bufLen);
				break;
			case MST_MODE:
				stp_param_save_mstp_mode_cfg(buf, bufLen);
				break;
			default:
				printf("non stp mode %d\n",current_mode);
				break;
		}
	}

	return 0;
}

#ifdef __cplusplus
}
#endif

