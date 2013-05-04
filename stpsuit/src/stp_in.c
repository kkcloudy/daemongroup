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
*stp_in.c
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       APIs from OS to stp module
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
#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_in.h"
#include "stp_to.h"
#include "stp_log.h"
#include "stp_param.h"
 
#define INCR100(nev) { nev++; if (nev > 99) nev = 0;}

RSTP_EVENT_T tev = RSTP_EVENT_LAST_DUMMY;
int			 nev = 0;
int max_port = 40;

/*mstp*/
BPDU_RECORD_T  g_curBPDU = {0};
struct port_speed_node **port_speed_s = NULL;
struct stp_admin_infos **stp_ports = NULL;
extern struct port_duplex_mode **port_duplex_mode_nodes;
extern unsigned int sttrans_count;/*zhengcaisheng add used in sttrans*/

Bool stp_in_get_cur_bpdu( BPDU_T **bpdu, int *type, unsigned char *digestSnp);

void  stp_in_set_cur_bpdu( BPDU_T *bpdu, int type, unsigned char digestSnp );

/*init needed value space*/
void stp_in_port_global_init(void)
{
    unsigned int port_index = 0;
	unsigned short mstid = 0;
	/*init port_speed construct*/
	if(NULL == port_speed_s){
		port_speed_s = (void*)malloc(sizeof(void*)*PORT_INDEX_MAX);
		if(NULL == port_speed_s) {
		  /*if failed, the programe can run,so do not return error*/
		  printf("malloc memory for port_speed_s error\n");
		  port_speed_s = NULL;
		}
		else{
		  memset(port_speed_s,0,(sizeof(void*)*PORT_INDEX_MAX));
		} 
	}
	/*init duplex mode structure*/
	if(NULL == port_duplex_mode_nodes){
		port_duplex_mode_nodes = (void*)malloc(sizeof(void*)*PORT_INDEX_MAX);
		if(NULL == port_duplex_mode_nodes){
		  /*if failed, the programe can run,so do not return error*/
		   printf("malloc memory for port_duplex_mode_nodes error\n");
		   port_duplex_mode_nodes = NULL;
		}
		else{
		  memset(port_duplex_mode_nodes,0,(sizeof(void*)*PORT_INDEX_MAX));
		}
	}

	/*init stp ports infos structure*/
	if(NULL == stp_ports){
		stp_ports = (void*)malloc(sizeof(void*)*PORT_INDEX_MAX);
		if(NULL == stp_ports){
			printf("malloc memory for stp ports failed!\n");
			stp_ports = NULL;
		}
		else{
			 memset(stp_ports,0,(sizeof(void*)*PORT_INDEX_MAX));
			 for(port_index = 0;port_index < PORT_INDEX_MAX;port_index++){
	            if(NULL == stp_ports[port_index]){
			  	   printf("Malloc space for port_index %d\n",port_index);
			       stp_ports[port_index] = (struct stp_admin_infos*)malloc(sizeof(struct stp_admin_infos));
				   if(NULL == stp_ports[port_index]){
				  	  printf("Malloc for stp_ports[%d]failed!\n",port_index);
			          stp_ports[port_index] = NULL;
				   }
				   else{
				  	  struct stp_admin_infos *stp_info = stp_ports[port_index];
					   memset(stp_info,0,sizeof(struct stp_admin_infos));
					   stp_info->edge = DEF_ADMIN_EDGE;
					   stp_info->nonstp = DEF_ADMIN_NON_STP;
					   stp_info->p2p = DEF_P2P;
					   for(mstid = 0;mstid < MAX_MST_ID;mstid++){
						  stp_info->pathcost[mstid] = ADMIN_PORT_PATH_COST_AUTO;
						  stp_info->prio[mstid] = DEF_PORT_PRIO;
					   }
				  }
			  }
			} 
			 
		}
	}
}

/*the space need to release when disable the stp*/
void stp_in_port_global_destroy(void)
{
    unsigned int i = 0;
    /*free the port_speed_s*/
	  if(NULL != port_speed_s){
		  for(i = 0;i<PORT_INDEX_MAX;i++){
              if(NULL != port_speed_s[i]){
                 free(port_speed_s[i]);
				 port_speed_s[i] = NULL;
			  }
	  	  }
		  free(port_speed_s);
		  port_speed_s=NULL;
	  }
	  /*free the port_duplex_mode_nodes*/
	 if(NULL != port_duplex_mode_nodes){
		for(i = 0;i<PORT_INDEX_MAX;i++){
          if(NULL != port_duplex_mode_nodes[i]){
             free(port_duplex_mode_nodes[i]);
			 port_duplex_mode_nodes[i] = NULL;
		  }
	    }
		free(port_duplex_mode_nodes);
	    port_duplex_mode_nodes = NULL;
     }
	 /*free stp ports info structure*/
	  if(NULL != stp_ports){
		for(i = 0;i<PORT_INDEX_MAX;i++){
          if(NULL != stp_ports[i]){
             free(stp_ports[i]);
			 stp_ports[i] = NULL;
		  }
	   }
	   free(stp_ports);
	   stp_ports = NULL;
     }
}

int stp_in_global_enable()
{	
 	int ret = 0;
	UID_STP_CFG_T uid_cfg;
  	STPM_T *stpm;
		
	for(stpm = stp_stpm_get_the_list(); stpm; stpm = stpm->next)
	{
		  uid_cfg.field_mask = BR_CFG_STATE;
		  uid_cfg.stp_enabled = STP_ENABLED;

          /*init speed and duplex mode constructure*/
		  stp_in_port_global_init();

		  	
		  ret |= stp_in_stpm_set_cfg (stpm,&uid_cfg);
		  if (ret) {
		  	stp_syslog_dbg("can't enable: %s\n", stp_in_get_error_explanation (ret));
		  } 
	}

	return ret;
}

int stp_in_global_disable()
{
	int ret = 0;
	UID_STP_CFG_T uid_cfg;
  	STPM_T *stpm = NULL;
	unsigned int i = 0;

	stpm = stp_stpm_get_the_cist();
	if(NULL != stpm) {
	 	uid_cfg.field_mask = BR_CFG_STATE;
	 	uid_cfg.stp_enabled = STP_DISABLED;
	 	ret = stp_in_stpm_set_cfg (stpm,  &uid_cfg);
	 	if (ret) {
	 	 stp_syslog_dbg("can't enable: %s\n", stp_in_get_error_explanation (ret));
	 	}
		/*destroy the space used for speed and duplex mode */
		stp_in_port_global_destroy();  
	}
	
	stp_mgmt_stpm_delete_all();	
	
	return ret;
}

int 
stp_in_add_vlan_to_cist (int vid, BITMAP_T port_bmp)
{
	register STPM_T*  this = NULL;
	int               err_code;

	stp_syslog_dbg("MSTP add vlan %d to cist\n", vid);
	stp_mgmt_init_vlanpbmp(vid, port_bmp);
	
	RSTP_CRITICAL_PATH_START;  
	this = stp_stpm_get_the_cist();
	if (!this) { /* it had just been created :( */
	 	err_code = STP_BRIDGE_NOTFOUND;
		stp_syslog_dbg("MSTP add vlan %d to cist error(%s)\n",
		          vid,stp_in_get_error_explanation (err_code));

		goto Unlock;
	}
	stp_syslog_dbg("MSTP add vlan %d to cist stpm\n", vid);
	/*how to do, cann't add vlan?*/
	 err_code = stp_mgmt_stpm_add_vlan(this, vid);
	 if(err_code != STP_OK)
	    goto Unlock;

	stp_mgmt_build_configid_digest();
	err_code = STP_OK;
	Unlock:
	RSTP_CRITICAL_PATH_END;
	return err_code;  

}

int stp_in_del_vlan_from_cist(int vid)
{
	register STPM_T*  this = NULL;
	unsigned short mstid = 0; 
	int               err_code;

	stp_syslog_dbg("MSTP>>#@ into stp_in_del_vlan_to_cist\n");
	
	RSTP_CRITICAL_PATH_START;  

	mstid = stp_mgmt_get_mstid(vid);
	if(STP_CIST_ID != mstid) {
		this = stp_stpm_get_instance(mstid);
		if(!this) {
			err_code = STP_Cannot_Create_Instance_For_Vlan;
			goto Unlock;
		}
		
		/*how to do, cann't add vlan?*/
		 err_code = stp_mgmt_stpm_del_vlan(this, vid);
		 if(err_code != STP_OK)
		    goto Unlock;

		stp_mgmt_build_configid_digest();
	}
	
	err_code = STP_OK;
	Unlock:
	RSTP_CRITICAL_PATH_END;
	return err_code;  

}


int stp_in_add_port_to_mstp(int vid,unsigned int port_index,unsigned int isWan)
{
	unsigned short MSTID = 0;

	MSTID = stp_mgmt_get_mstid(vid);

	if(STP_CIST_ID != MSTID) {
		if (STP_OK != stp_port_mst_addport(MSTID,port_index, vid, True, isWan)) {
					    /* can't add port :( */
					    stp_trace ("stp_in114:: can't create port %d", (int) port_index);
					    return STP_Cannot_Create_Instance_For_Port;			
		}
	}
	
	return STP_OK;	
}

int stp_in_del_port_from_mstp(int vid,unsigned int port_index)
{
	unsigned short MSTID = stp_mgmt_get_mstid(vid);

	if(STP_CIST_ID != MSTID) {
		if( STP_OK != stp_port_mst_delport(MSTID, port_index, vid, True))
			 return -1;
	}

	return STP_OK;
}

STPM_T *
stp_in_stpm_find (int vlan_id)
{
  register STPM_T* this;

  for (this = stp_stpm_get_the_list (); this; this = this->next)
    if (vlan_id == this->vlan_id)
      return this;

  return NULL;
}

static PORT_T *
stp_in_port_find (STPM_T* this, int port_index)
{
  register PORT_T* port;

  for (port = this->ports; port; port = port->next)
    if (port_index == port->port_index) {
      return port;
    }

  return NULL;
}

static PORT_T *
stp_in_slot_port_find (STPM_T* this, int slot_no, int port_no)
{
  register PORT_T* port;

  for (port = this->ports; port; port = port->next)
    if ((slot_no == port->slot_no) &&
		(port_no == port->port_no)){
      return port;
    }

  return NULL;
}

#if 0
static void
_conv_br_id_2_uid (IN BRIDGE_ID* f, OUT UID_BRIDGE_ID_T* t)
{
  memcpy (t, f, sizeof (UID_BRIDGE_ID_T));
}
#endif

static int
stp_in_check_stpm_config (IN UID_STP_CFG_T* uid_cfg, IN unsigned short mstid)
{
  if (uid_cfg->bridge_priority < MIN_BR_PRIO) {
    stp_trace ("%d bridge_priority small", (int) uid_cfg->bridge_priority);
    return STP_Small_Bridge_Priority;
  }

  if (uid_cfg->bridge_priority > MAX_BR_PRIO) {
    stp_trace ("%d bridge_priority large", (int) uid_cfg->bridge_priority);
    return STP_Large_Bridge_Priority;
  }

/*mstp*/
  if (uid_cfg->maxhops < MIN_REMAINING_HOPS) {
    stp_trace ("%d maxhops small", (int) uid_cfg->maxhops);
    return STP_Small_Max_Hops;
  }

  if (uid_cfg->maxhops> MAX_REMAINING_HOPS) {
    stp_trace ("%d maxhops large", (int) uid_cfg->maxhops);
    return STP_Large_Max_Hops;
  }

  if(STP_CIST_ID != mstid)
  	return STP_OK;
  
  if (uid_cfg->hello_time < MIN_BR_HELLOT) {
    stp_trace ("%d hello_time small", (int) uid_cfg->hello_time);
    return STP_Small_Hello_Time;
  }

  if (uid_cfg->hello_time > MAX_BR_HELLOT) {
    stp_trace ("%d hello_time large", (int) uid_cfg->hello_time);
    return STP_Large_Hello_Time;
  }

  if (uid_cfg->max_age < MIN_BR_MAXAGE) {
    stp_trace ("%d max_age small", (int) uid_cfg->max_age);
    return STP_Small_Max_Age;
  }

  if (uid_cfg->max_age > MAX_BR_MAXAGE) {
    stp_trace ("%d max_age large", (int) uid_cfg->max_age);
    return STP_Large_Max_Age;
  }

  if (uid_cfg->forward_delay < MIN_BR_FWDELAY) {
    stp_trace ("%d forward_delay small", (int) uid_cfg->forward_delay);
    return STP_Small_Forward_Delay;
  }

  if (uid_cfg->forward_delay > MAX_BR_FWDELAY) {
    stp_trace ("%d forward_delay large", (int) uid_cfg->forward_delay);
    return STP_Large_Forward_Delay;
  }

  if (2 * (uid_cfg->forward_delay - 1) < uid_cfg->max_age) {
    return STP_Forward_Delay_And_Max_Age_Are_Inconsistent;
  }

  if (uid_cfg->max_age < 2 * (uid_cfg->hello_time + 1)) {
	stp_trace ("%d max_age < 2*hello_time %d", (int) uid_cfg->max_age,(int) uid_cfg->hello_time);
    return STP_Hello_Time_And_Max_Age_Are_Inconsistent;
  }
  if(2*(uid_cfg->hello_time + 1)>2 * (uid_cfg->forward_delay - 1)){
    stp_trace ("%d 2*(uid_cfg->hello_time + 1) < 2 * (uid_cfg->forward_delay - 1) %d", (int) uid_cfg->hello_time,(int) uid_cfg->forward_delay);
	return STP_Hello_Time_And_Forward_Delay_Are_Inconsistent;
  }
  return STP_OK;
}

static int
_stp_in_enable_port_on_stpm 
(
	int port_index,
	unsigned int link_state,
	unsigned int isWAN,
	Bool enable,  
	unsigned int slot_no,
	unsigned int port_no
)
{
	int rc = 0;
	register PORT_T* port = NULL, *cist_port = NULL;
	struct stp_admin_infos *stp_info = stp_ports[port_index];
#define SYSFS_PATH_MAX 256
	FILE *f = NULL;
	char path[SYSFS_PATH_MAX] = {0};
	char ifname[21] = {0};
	unsigned int br_ifindex = 0;
	 
  cist_port = stp_port_mst_findport(STP_CIST_ID, port_index);
  port = cist_port;
 
  if (! port){
    stp_syslog_error("find port failed!port_index %d\n",port_index);
	return 1; 
  }
  for( ; port; port = port->nextMst)
  {
  		/* modified by zhengcs@autelan.com in 2008.11.14
		not used variable portEnabled to just the STP port enable or not since that the portEnabled was set in p2p.c if adminEnable set
		so when enable port in link down state ,the portEnabled not setted though the port has already set enable and when link up it start to enter the 
		p2p.c to set portEnabled and transmit bpdu..
            if (port->portEnabled == enable) {
        		if(1 == enable)
        		{
              		return STP_PORT_HAVE_ENABLED;
        		}
        		else
        		{
        			return STP_PORT_NOT_ENABLE;
        		}
        
            }           
    
   */ 
    port->uptime = 0;
    if (enable) { 
      port->rx_cfg_bpdu_cnt =
      port->rx_rstp_bpdu_cnt =
      port->rx_tcn_bpdu_cnt = 0;
    }  
  
  #ifdef STP_DBG
   /* if (port->rolesel && port->rolesel->debug) */{
      stp_trace ("Port %s (instance %d)became '%s' adminEdge=%c",
          port->port_name, port->owner->vlan_id, enable ? "enable" : "disable",
          port->adminEdge ? 'Y' : 'N');
    }
  #endif
    stp_syslog_dbg("port %d,link state %d,enable %d\n",port_index,link_state,enable);
    //port->adminEnable = enable; added by zhengcs@autelan.com 20081108
    /*link_state 1 means port link down,0 means port link up*/
    if(enable){//if enable,just the adminEnable by the link state
		port->adminEnable = link_state ? 0:1 ;
	}
    else {
        port->adminEnable = enable;
	}
	/*add for master control port*/
	port->isWAN = isWAN;
	port->slot_no = slot_no;
	port->port_no = port_no;
	if (isWAN) {
		snprintf(path, SYSFS_PATH_MAX, "/sys/class/net/eth%d-%d/brport/bridge/ifindex", slot_no, port_no);
		f = fopen(path, "r");
		if (f) {
			fscanf(f, "%d", &br_ifindex);
			sprintf(ifname, "eth%d-%d", slot_no, port_no);
			port->port_ifindex = if_nametoindex(ifname);
			port->br_ifindex = br_ifindex;
			fclose(f);
			/*when add port state is discard first*/
			stp_npd_set_wan_state(port->br_ifindex, port->port_ifindex, 1);/*NAM_STP_PORT_STATE_DISCARD_E*/
		}
		else{
			stp_syslog_error("eth%d-%d not in br \n",slot_no, port_no);
			return 0;
		}
	}
	stp_syslog_dbg("port adminEnable %d, isMaster %d, slot_no %d, port_no %d\n",port->adminEnable,isWAN,slot_no,port_no);	
    stp_port_init (port, port->owner, False);	
    port->reselect = True;
    port->selected = False;  
  }
  
  	/*set the stp stpenable variable*/
	if(NULL != stp_info){
      stp_info->stpEnable = enable;
	  stp_trace("stp_info->stpEnable %d,port_index %d\n",stp_info->stpEnable,port_index);
   }
  
  stp_stpm_mst_update(cist_port);


  if(!enable){//added by zhangcs@autelan.com at 2008.when topol changed ,reset the value to insure that only one time to configure the hw in the station.
    sttrans_count = 0;
  }

	return rc;

}


void 
stp_in_init (int max_port_index)
{
  max_port = max_port_index;
  RSTP_INIT_CRITICAL_PATH_PROTECTIO;
}

int stp_in_enable_port_on_stpm 
(
    int port_index, 
    Bool enable,
    unsigned int link_state,
    unsigned int port_speed,
    unsigned int isWAN,
    unsigned int slot_no,
    unsigned int port_no
)
{
	int rc = 0;
	struct port_speed_node *port_speed_nodes = NULL;
	stp_syslog_dbg("port_speed is %d\n",port_speed);
	RSTP_CRITICAL_PATH_START;
	tev = enable ? RSTP_PORT_EN_T : RSTP_PORT_DIS_T; INCR100(nev);
	if (! enable) {
#ifdef STP_DBG
	  stp_trace("%s (p%02d, all, %s, '%s')",
	      "clearFDB", (int) port_index, "this port", "disable port");
#endif
	  stp_to_flush_lt (port_index, 0, LT_FLASH_ONLY_THE_PORT, "disable port");
	}
	else if (NULL
!= port_speed_s){
		if(NULL == port_speed_s[port_index]){
			port_speed_nodes = (struct port_speed_node*)malloc(sizeof(struct port_speed_node));
		    if( NULL == port_speed_nodes){
				/*malloc error but do not return , use the default value*/
				stp_syslog_dbg("stp_in_enable_port_on_stpm malloc error!\n");
			}
			else{
			   memset(port_speed_nodes,0,sizeof(struct port_speed_node));
               port_speed_s[port_index]= port_speed_nodes;
			}
		}
		/*init port speed value space added by zhengcs@autelan.com 20081108*/
		/*init port speed node*/
		else {
		    port_speed_nodes = port_speed_s[port_index];
		}
        port_speed_nodes->port_index = port_index;
	    port_speed_nodes->port_speed = port_speed;
	    port_speed_s[port_index]=port_speed_nodes;
		stp_syslog_dbg(" stp_in_enable_port_on_stpm ::port_speed_s[%d] %d\n",port_index,port_speed_nodes->port_speed );
	}
	 rc = _stp_in_enable_port_on_stpm (port_index, link_state,isWAN, enable, slot_no,port_no);
	RSTP_CRITICAL_PATH_END;
	return rc;
}

int /* call it, when port speed has been changed, speed in Kb/s  */
stp_in_change_port_speed (int port_index, long speed)
{
  register STPM_T* stpm;
  register PORT_T* port;

  RSTP_CRITICAL_PATH_START;
  tev = RSTP_PORT_SPEED_T; INCR100(nev);
  for (stpm = stp_stpm_get_the_list (); stpm; stpm = stpm->next) {
    if (STP_ENABLED != stpm->admin_state) continue;
    
    port = stp_in_port_find (stpm, port_index);
    if (! port) continue; 
    port->operSpeed = speed;
#ifdef STP_DBG
    if (port->pcost->debug) {
      stp_trace ("changed operSpeed=%lu", port->operSpeed);
    }
#endif

    port->reselect = True;
    port->selected = False;
  }
  RSTP_CRITICAL_PATH_END;
  return 0;
}

int /* call it, when port duplex mode has been changed  */
stp_in_change_port_duplex (int port_index)
{
  register STPM_T* stpm;
  register PORT_T* port;

  RSTP_CRITICAL_PATH_START;
  tev = RSTP_PORT_DPLEX_T; INCR100(nev);
  for (stpm = stp_stpm_get_the_list (); stpm; stpm = stpm->next) {
    if (STP_ENABLED != stpm->admin_state) continue;
    
    port = stp_in_port_find (stpm, port_index);
    if (! port) continue; 
#ifdef STP_DBG
    if (port->p2p && port->p2p->debug) {
      stp_trace ("stp_in_change_port_duplex(%s)", port->port_name);
    }
#endif
    stp_syslog_dbg("stp_in_changed_port_duplex-port p2p recompute %d\n",port->p2p_recompute);
    port->p2p_recompute = True;
    port->reselect = True;
    port->selected = False;
  }
  RSTP_CRITICAL_PATH_END;
  return 0;
}

int
stp_in_check_bpdu_header (PORT_T* port, MSTP_BPDU_T* bpdu, size_t len, unsigned char  *bpdu_type)
{
  unsigned short len8023 = 0;
  unsigned short version_3_length = 0;
  short MstCnt = 0;
  MSTP_Cisco_BPDU_T *ciscoBpdu = NULL;

  if ( len < MIN_BPDU )
  {
      return -1 ;
  }
  
  len8023 = ntohs (*(unsigned short*) bpdu->eth.len8023);
  if (len8023 > 1500) {/* big len8023 format :( */
    return STP_Big_len8023_Format;
  }

  if (len8023 < MIN_BPDU) { /* small len8023 format :( */
    return STP_Small_len8023_Format;
  }

  if (len8023 + 14 > len) { /* len8023 format gt len :( */
    return STP_len8023_Format_Gt_Len;
  }

  if (bpdu->eth.dsap != BPDU_L_SAP                 ||
      bpdu->eth.ssap != BPDU_L_SAP                 ||
      bpdu->eth.llc != LLC_UI) {
    /* this is not a proper 802.3 pkt! :( */
    return STP_Not_Proper_802_3_Packet;
  }

  if (bpdu->hdr.protocol[0] || bpdu->hdr.protocol[1]) {
    return STP_Invalid_Protocol;
  }

/*mstp*/
  if ( bpdu->hdr.protocol[ 0 ] != 0 || bpdu->hdr.protocol[ 1 ] != 0 )
  {
      return -1 ;
  }

  switch( bpdu->hdr.bpdu_type ){
      case BPDU_TOPO_CHANGE_TYPE:
	  	if(len >=4)
	  	{
		  *bpdu_type = BPDU_TOPO_CHANGE_TYPE;
		  return 0;
	  	}
      break;
      case BPDU_CONFIG_TYPE:
	  	if(len >=35)
	  	{
		  *bpdu_type = BPDU_CONFIG_TYPE;
		  return 0;
	  	}
      break;
      case BPDU_RSTP:
	  	if ( bpdu->hdr.version == 2 && len >=36)
	  	{
		  *bpdu_type = BPDU_RSTP;
		  return 0;
	  	}
		else if(bpdu->hdr.version >= 3 )
		{
		  if(port && (True == port->configDigestSnp)) {
		  	ciscoBpdu = (MSTP_Cisco_BPDU_T*)bpdu;
			version_3_length = ntohs(*(unsigned short*)ciscoBpdu->version_3_length);
		  }
		  else {
		   	version_3_length = ntohs(*( unsigned short* ) bpdu->version_3_length);
		  }
           MstCnt = (version_3_length - 64 ) / 16;
		  if((len >= 35 && len <103) || ( bpdu->ver_1_len != 0 ) || 
		  	( MstCnt > 64 || MstCnt < 0))
          {
                  *bpdu_type = BPDU_RSTP;
                  return 0;
          }
		  else if((len >=102) && ( bpdu->ver_1_len == 0 ) && ( MstCnt <=64 && MstCnt >=0))
          {
                  *bpdu_type = BPDU_MSTP;
                  return 0;
          }
		}
      break;
      default :  
	  	;
  }
  
  return -1;
#if 0
  if (bpdu->hdr.version != BPDU_VERSION_ID) {
    return STP_Invalid_Version;  
  }

  /* see also 9.3.4: think & TBD :( */
  return 0;
#endif
}

#ifdef STP_DBG
int dbg_rstp_deny = 0;
#endif


int
stp_in_rx_bpdu 
(
	unsigned int vlan_id, 
	unsigned int port_index, 
	unsigned int slot_no, 
	unsigned int port_no, 
	BPDU_T* bpdu, 
	size_t len
)
{
  register PORT_T* port;
  register STPM_T* this;
  int              iret;

#ifdef STP_DBG
  if (1 == dbg_rstp_deny) {
    return 0;
  }
#endif

  RSTP_CRITICAL_PATH_START;
  tev = RSTP_PORT_RX_T; INCR100(nev);
  this = stp_stpm_get_the_cist();
  /*this = stp_in_stpm_find (vlan_id);*/
  if (! this) { /*  the stpm had not yet been created :( */
    RSTP_CRITICAL_PATH_END;
    return STP_Vlan_Had_Not_Yet_Been_Created;
  }

  if (STP_DISABLED == this->admin_state) {/* the stpm had not yet been enabled :( */
    RSTP_CRITICAL_PATH_END;
    return STP_Had_Not_Yet_Been_Enabled_On_The_Vlan;
  }
	if (port_index) {
		port = stp_in_port_find (this, port_index);
		if (! port) {/* port is absent in the stpm :( */
			stp_trace ("RX bpdu vlan_id=%d port=%d port is absent in the stpm :(", (int) vlan_id, (int) port_index);
			RSTP_CRITICAL_PATH_END;
			return STP_Port_Is_Absent_In_The_Vlan;
		}
	}
	else {
/*printf ("RX bpdu vlan_id=%d slot_no=%d port_no=%d port is absent in the stpm :(\n", (int) vlan_id, (int) slot_no, port_no);		*/
		port = stp_in_slot_port_find (this, slot_no, port_no);
		stp_trace("port is 0x%x\n",port);
		if (! port) {/* port is absent in the stpm :( */			
			stp_trace ("RX bpdu vlan_id=%d slot_no=%d port_no %d port is absent in the stpm :(\n", (int) vlan_id, (int) slot_no, port_no, port_index);
			RSTP_CRITICAL_PATH_END;
			return STP_Port_Is_Absent_In_The_Vlan;
		}
	}
	
#ifdef STP_DBG
  if (port->skip_rx > 0) {
    if (1 == port->skip_rx)
      stp_trace ("port %s stop rx skipping",
                 port->port_name);
    else
      stp_trace ("port %s skip rx %d",
                 port->port_name, port->skip_rx);
    port->skip_rx--;
    RSTP_CRITICAL_PATH_END;
    return STP_Nothing_To_Do;
  }
#endif

  if (port->operEdge && ! port->lnkWhile && port->portEnabled) {
#ifdef STP_DBG
	  if (port->topoch->debug) {
    	stp_trace ("port %s tc=TRUE by operEdge", port->port_name);
	  }
#endif
    port->tc = True; /* IEEE 802.1y, 17.30 */
  }

  #if 0
  if (! port->portEnabled) {/* port link change indication will come later :( */
    _stp_in_enable_port_on_stpm (this, port->port_index, True);
  }
  #endif
#ifdef STP_DBG
  if (port->edge && port->edge->debug && port->operEdge) {
    stp_trace ("port %s not operEdge !", port->port_name);
  }
#endif

  if(port->operEdge )
  {
      PORT_T *mstport = port;
      for( ; mstport; mstport = mstport->nextMst)
      {
          mstport->operEdge = False;
      }
  }
  port->wasInitBpdu = True;
  
  iret = stp_port_rx_bpdu (port, bpdu, len);
  if(0 == iret)
    stp_stpm_mst_update (port);
  
  stp_in_set_cur_bpdu(NULL, 0, 0);
  /*stp_stpm_update (this);*/
  RSTP_CRITICAL_PATH_END;

  return iret;
}

int
stp_in_one_second (void)
{
  /*register STPM_T* stpm;*/
  register int     dbg_cnt = 0;

  RSTP_CRITICAL_PATH_START;
  tev = RSTP_PORT_TIME_T; INCR100(nev);
#if 0
  for (stpm = stp_stpm_get_the_list (); stpm; stpm = stpm->next) {
    if (STP_ENABLED == stpm->admin_state) {
      /*stp_trace ("stp_in_one_second vlan_id=%d", (int) stpm->vlan_id);*/ 
      stp_stpm_one_second (stpm);
      dbg_cnt++;
    }
  }
#endif  
  dbg_cnt = stp_stpm_one_second ();
  dbg_cnt++;
  RSTP_CRITICAL_PATH_END;

  return dbg_cnt;
}

int
stp_in_stpm_get_cfg (IN STPM_T *this, OUT UID_STP_CFG_T* uid_cfg)
{
  uid_cfg->field_mask = 0;

  if (this->admin_state != STP_DISABLED) {
    uid_cfg->field_mask |= BR_CFG_STATE;
  }
  uid_cfg->stp_enabled = this->admin_state;

  if (this->BrId.prio != DEF_BR_PRIO) {
    uid_cfg->field_mask |= BR_CFG_PRIO;
  }
  uid_cfg->bridge_priority = this->BrId.prio;

  if ( this->BrTimes.RemainingHops != DEF_REMAINING_HOPS )
  {
    uid_cfg->field_mask |= BR_CFG_MAX_HOPS;
  }
  uid_cfg->maxhops = this->BrTimes.RemainingHops;  
  #if 0
  if(STP_CIST_ID != this->vlan_id)
  	return 0;
  #endif
  if (this->ForceVersion != 2) {
    uid_cfg->field_mask |= BR_CFG_FORCE_VER;
  }
  uid_cfg->force_version = this->ForceVersion;

  if (this->BrTimes.MaxAge != DEF_BR_MAXAGE) {
    uid_cfg->field_mask |= BR_CFG_AGE;
  }
  uid_cfg->max_age = this->BrTimes.MaxAge;

  if (this->BrTimes.HelloTime != DEF_BR_HELLOT) {
    uid_cfg->field_mask |= BR_CFG_HELLO;
  }
  uid_cfg->hello_time = this->BrTimes.HelloTime;

  if (this->BrTimes.ForwardDelay != DEF_BR_FWDELAY) {
    uid_cfg->field_mask |= BR_CFG_DELAY;
  }
  uid_cfg->forward_delay = this->BrTimes.ForwardDelay;
  

  uid_cfg->hold_time = TxHoldCount;

  return 0;
}


int
stp_in_stpm_set_cfg (IN  STPM_T *this,
                     IN UID_STP_CFG_T* uid_cfg)
{
  int rc = 0, prev_prio;
  UID_STP_CFG_T old;
           
  /* stp_trace ("stp_in_stpm_set_cfg"); */
  if(NULL == this)
  	return STP_Vlan_Had_Not_Yet_Been_Created;
  
  stp_in_stpm_get_cfg (this, &old) ;
  
  RSTP_CRITICAL_PATH_START;
  tev = RSTP_PORT_MNGR_T; INCR100(nev);
  if (BR_CFG_PRIO & uid_cfg->field_mask) {
    old.bridge_priority = uid_cfg->bridge_priority;
  }

  if (BR_CFG_AGE & uid_cfg->field_mask) {
    old.max_age = uid_cfg->max_age;
  }

  if (BR_CFG_HELLO & uid_cfg->field_mask) {
    old.hello_time = uid_cfg->hello_time;
  }

  if (BR_CFG_DELAY & uid_cfg->field_mask) {
    old.forward_delay = uid_cfg->forward_delay;
  }

  if (BR_CFG_FORCE_VER & uid_cfg->field_mask) {
    if ( uid_cfg->force_version >= DEF_FORCE_VERS )
    {
        PORT_T * port;
        for ( port = this->ports; port; port = port->next )
        {
            port->mcheck = True;
        }
    }
    old.force_version = uid_cfg->force_version;
	printf("uid_cfg.force_version %d\n",uid_cfg->force_version);
  }
  if ( BR_CFG_MAX_HOPS & uid_cfg->field_mask )
  {
      old.maxhops= uid_cfg->maxhops;
  }

  rc = stp_in_check_stpm_config (&old, this->vlan_id);
  if (0 != rc) {
    stp_trace ("stp_in_check_stpm_config failed %d", (int) rc);
    RSTP_CRITICAL_PATH_END;
    return rc;
  }
  if ((BR_CFG_STATE & uid_cfg->field_mask) &&
      (STP_DISABLED == uid_cfg->stp_enabled)) {
    stp_to_set_hardware_mode (this->vlan_id, STP_DISABLED);
	stp_syslog_dbg("stp_in698:: stp_stpm_enable STP_DISABLED \n"); 
     rc = stp_stpm_enable (this, STP_DISABLED);    
    if (0 != rc) {
      stp_trace ("can't disable rc=%d", (int) rc);
      RSTP_CRITICAL_PATH_END;
      return rc;
    }
    uid_cfg->field_mask &= ! BR_CFG_STATE;
    if (! uid_cfg->field_mask)  {
      RSTP_CRITICAL_PATH_END;
      return 0;
    }
  }

  prev_prio = this->BrId.prio;
  this->BrId.prio = (this->BrId.prio & 0xfff) + (old.bridge_priority & 0xf000);
  #if 0
  if (STP_ENABLED == this->admin_state) {
    if (0 != stp_stpm_check_bridge_priority (this)) {
      this->BrId.prio = prev_prio;
      stp_trace ("%s", "stp_stpm_check_bridge_priority failed");
      RSTP_CRITICAL_PATH_END;
      return STP_Invalid_Bridge_Priority;
    }
  }
  #endif
  
  this->BrTimes.MaxAge = old.max_age;
  this->BrTimes.HelloTime = old.hello_time;
  this->BrTimes.ForwardDelay = old.forward_delay;
  this->ForceVersion = (PROTOCOL_VERSION_T) old.force_version;
  this->BrTimes.RemainingHops = old.maxhops;
  if ((BR_CFG_STATE & uid_cfg->field_mask) &&
      STP_DISABLED != uid_cfg->stp_enabled &&
      STP_DISABLED == this->admin_state) {
    stp_syslog_dbg("stp_in698:: stp_stpm_enable\n");  
    rc = stp_stpm_enable (this, uid_cfg->stp_enabled);
    if (! rc) {
      stp_to_set_hardware_mode (this->vlan_id, uid_cfg->stp_enabled );
    }
    else {
      stp_trace ("file %s, line %d ,%s",__FILE__,__LINE__, "cannot enable");
      RSTP_CRITICAL_PATH_END;
      return rc;
    }
  }
  if ( STP_DISABLED != this->admin_state) {
    stp_stpm_update_after_bridge_management (this);
    stp_stpm_mst_update(NULL);
  }
  RSTP_CRITICAL_PATH_END;
  return 0;
}

int
stp_in_set_port_cfg (IN PORT_T *port, IN UID_STP_PORT_CFG_T* uid_cfg)
{
  register STPM_T* this = port->owner;
 
  RSTP_CRITICAL_PATH_START;
  tev = RSTP_PORT_MNGR_T; INCR100(nev);

  if (PT_CFG_MCHECK & uid_cfg->field_mask) {
    if (this->ForceVersion >= NORMAL_RSTP)
      port->mcheck = True;
	
  }
  
  if (PT_CFG_COST & uid_cfg->field_mask) {
    port->adminPCost = uid_cfg->admin_port_path_cost;
  }
  
  if (PT_CFG_PRIO & uid_cfg->field_mask) {
    port->port_id = ((uid_cfg->port_priority & 0xf0)<< 8) + port->port_index;
  }
  
  if (PT_CFG_P2P & uid_cfg->field_mask) {
    port->adminPointToPointMac = uid_cfg->admin_point2point;
    port->p2p_recompute = True;
  }
  
  if (PT_CFG_EDGE & uid_cfg->field_mask) {
    port->adminEdge = uid_cfg->admin_edge;
    port->operEdge = port->adminEdge;
  #ifdef STP_DBG
    if (port->edge && port->edge->debug) {
      stp_trace ("port %s is operEdge=%c in stp_in_set_port_cfg",
          port->port_name,
          port->operEdge ? 'Y' : 'n');
    }
  #endif
  }
  
  if (PT_CFG_NON_STP & uid_cfg->field_mask) {
  #ifdef STP_DBG
    if (port->roletrns->debug && port->admin_non_stp != uid_cfg->admin_non_stp) {
      stp_trace ("port %s is adminNonStp=%c in stp_in_set_port_cfg",
          port->port_name,
          uid_cfg->admin_non_stp ? 'Y' : 'n');
    }
  #endif
    port->admin_non_stp = uid_cfg->admin_non_stp;
  }
  
  #ifdef STP_DBG
  if (PT_CFG_DBG_SKIP_RX & uid_cfg->field_mask) {
    port->skip_rx = uid_cfg->skip_rx;
  }
  
  if (PT_CFG_DBG_SKIP_TX & uid_cfg->field_mask) {
    port->skip_tx = uid_cfg->skip_tx;
  }
  
  #endif
  
  port->reselect = True;
  port->selected = False;
  
  stp_stpm_mst_update (stp_port_mst_findport(STP_CIST_ID, port->port_index));
  
  RSTP_CRITICAL_PATH_END;

  return 0;
}

int
stp_in_set_vlan_2_instance (unsigned short vlan_id, short mstid)
{
  int iret = 0;
  
  	iret = stp_mgmt_stpm_update_instance(mstid, vlan_id);

  if(iret)
  	stp_in_get_error_explanation(iret);
  return iret;
}

#ifdef STP_DBG
int
stp_in_dbg_set_port_trace (char* mach_name, int enadis,
                           int vlan_id, BITMAP_T* ports, short mstid,
                           int is_print_err)
{
  register STPM_T* this;
  register PORT_T* port = NULL;
  register int     port_index;
	int bytes,bit;

  RSTP_CRITICAL_PATH_START;
  this = stp_stpm_get_instance(mstid);
  /*this = stp_in_stpm_find (vlan_id);*/
  if (! this) { /* it had not yet been created :( */
    RSTP_CRITICAL_PATH_END;
    if (is_print_err) {
        Print ("RSTP instance with tag %d hasn't been created\n", (int) vlan_id);
    }
    return STP_Vlan_Had_Not_Yet_Been_Created;
  }
  if(! strcmp (mach_name, "all") || ! strcmp (mach_name, this->rolesel->name))
      this->rolesel->debug = 2;

	for(bytes = 0; bytes < NUMBER_OF_BYTES; bytes++) {		
		for(bit = 0; bit < BIT_OF_BYTE; bit++) {
			port_index = stp_bitmap_get_portindex_from_bmp(ports, bytes, bit);
			if(-1 == port_index) {	
				continue;
			}
			else {
				if ((port = stp_port_mst_findport (0,port_index)) == NULL) {
				    /* can't add port :( */
					continue;	
				}
				else
				 	stp_port_trace_state_machine (port, mach_name, enadis, vlan_id);
			}
		}
	}
  
  RSTP_CRITICAL_PATH_END;

  return 0;
}

#endif

const char*
stp_in_get_error_explanation (int rstp_err_no)
{
#define CHOOSE(a) #a
static char* rstp_error_names[] = RSTP_ERRORS;
#undef CHOOSE
  if (rstp_err_no < STP_OK) {
    return "Too small error code :(";
  }
  if (rstp_err_no >= STP_LAST_DUMMY) {
    return "Too big error code :(";
  }
  
  return rstp_error_names[rstp_err_no];
}

Bool
stp_in_get_cur_bpdu( BPDU_T **bpdu, int *type, unsigned char *digestSnp)
{
    if(NULL == g_curBPDU.bpdu )
        return False;

    *bpdu = g_curBPDU.bpdu;
    *type = g_curBPDU.bpdu_type;
	*digestSnp = g_curBPDU.digestSnp;
    
    return  True;
}

void 
stp_in_set_cur_bpdu( BPDU_T *bpdu, int type, unsigned char digestSnp)
{
	stp_trace("set budp %p, type %d", bpdu, type);
    g_curBPDU.bpdu = bpdu;
    g_curBPDU.bpdu_type = type;
	g_curBPDU.digestSnp = digestSnp;
}

int stp_in_reinit_port_cfg (IN int vlan_id, IN int port_index,IN UID_STP_PORT_CFG_T* uid_cfg)
{
  register STPM_T* this;
  register PORT_T* port;
  register int     port_no;

  RSTP_CRITICAL_PATH_START;
  tev = RSTP_PORT_MNGR_T; INCR100(nev);
  this = stp_in_stpm_find (vlan_id);
  if (! this) { /* it had not yet been created :( */
    RSTP_CRITICAL_PATH_END;
    Print ("RSTP instance with tag %d hasn't been created\n", (int) vlan_id);
    return STP_Vlan_Had_Not_Yet_Been_Created;
  }

  /*for (port_no = 1; port_no <= max_port; port_no++) {
    if (! stp_bitmap_get_bit(&uid_cfg->port_bmp, port_no - 1)) continue;*/
  if(this->ports)
  {
   	 	port = stp_in_port_find (this, port_index);
     	if (! port) {/* port is absent in the stpm :( */
      	//continue;
      		stp_syslog_dbg("RSTP>>the port not exist\n");
			return -1;
    	}


    if (PT_CFG_MCHECK & uid_cfg->field_mask) {
      if (this->ForceVersion >= NORMAL_RSTP)
        port->mcheck = True;
    }

    if (PT_CFG_COST & uid_cfg->field_mask) {
      port->adminPCost = uid_cfg->admin_port_path_cost;
    }
  
    if (PT_CFG_PRIO & uid_cfg->field_mask) {
      port->port_id = (uid_cfg->port_priority << 8) + port_no;
    }
  
    if (PT_CFG_P2P & uid_cfg->field_mask) {
      port->adminPointToPointMac = uid_cfg->admin_point2point;
      port->p2p_recompute = True;
    }
  
    if (PT_CFG_EDGE & uid_cfg->field_mask) {
      port->adminEdge = uid_cfg->admin_edge;
      port->operEdge = port->adminEdge;
#ifdef STP_DBG
      if (port->edge->debug) {
       		stp_trace ("port %s is operEdge=%c in stp_in_set_port_cfg",
            port->port_name,
            port->operEdge ? 'Y' : 'n');
      }
#endif
    }

    if (PT_CFG_NON_STP & uid_cfg->field_mask) {
#ifdef STP_DBG
      if (port->roletrns->debug && port->admin_non_stp != uid_cfg->admin_non_stp) {
        	stp_trace ("port %s is adminNonStp=%c in stp_in_set_port_cfg",
            port->port_name,
            uid_cfg->admin_non_stp ? 'Y' : 'n');
      }
#endif
      port->admin_non_stp = uid_cfg->admin_non_stp;
    }

#ifdef STP_DBG
    if (PT_CFG_DBG_SKIP_RX & uid_cfg->field_mask) {
      port->skip_rx = uid_cfg->skip_rx;
    }

    if (PT_CFG_DBG_SKIP_TX & uid_cfg->field_mask) {
      port->skip_tx = uid_cfg->skip_tx;
    }

#endif

    port->reselect = True;
    port->selected = False;
  }
  else
  {
  	printf("Don't create ports\n");
  	return -1;
  }

  return 0;
	
} 
#ifdef __cplusplus
}
#endif

