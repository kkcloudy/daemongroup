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
* igmp_snoop_cli.c
*
*
* CREATOR:
* 		chenbin@autelan.com
*
* DESCRIPTION:
* 		igmp inter source to handle orders or configure file
*
* DATE:
*		6/19/2008
*
* FILE REVISION NUMBER:
*  		$Revision: 1.20 $
*
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "igmp_snoop_com.h"
#include "igmp_snoop_inter.h"
#include "igmp_snoop.h"
#include "igmp_snp_log.h"
#include "sysdef/returncode.h"

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

typedef enum {
	VLAN_LIFETIME = 1,
	GROUP_LIFETIME,
	ROBUST_VARIABLE,
	QUERY_INTERVAL,
	RESP_INTERVAL,
	RXMT_INTERVAL
}IGMP_SNP_TIMER_TYPE;

/***********************************declare functions**************************************/
INT igmp_add_routeport(LONG vlan_id,ULONG ifindex,ULONG group );
INT igmp_del_routeport(LONG vlan_id,ULONG ifindex,ULONG group );
static INT igmp_set_enable(LONG flag);
INT igmp_snp_set_enable_dbus();
INT igmp_snp_set_disable_dbus();
INT igmp_snp_debug_on_dbus();
INT igmp_snp_debug_off_dbus();
INT igmp_set_vlanlife_timerinterval_dbus(unsigned int timeout);
INT igmp_set_grouplife_timerinterval_dbus(unsigned int timeout);
INT igmp_set_query_timerinterval_dbus(unsigned int timeout);
INT igmp_set_robust_timerinterval_dbus(unsigned int  timeout);
INT igmp_show_query_timerinterval_dbus(unsigned int* queryinterval);
INT igmp_show_resp_timerinterval_dbus(unsigned int* respinterval);
INT igmp_snp_config_timer_dbus(	unsigned int type,	unsigned int timeout);
INT igmp_show_robust_timerinterval_dbus(unsigned int* robust);
INT igmp_show_host_timerinterval_dbus(unsigned int * hostime);
INT igmp_show_vlanlife_timerinterval_dbus(unsigned int *mcgvlanlife);
INT igmp_show_grouplife_timerinterval_dbus(unsigned int *grouplife);
INT igmp_show_igmpvlan_cnt_dbus(unsigned int *igmpvlanCnt);
INT igmp_show_group_cnt_dbus(unsigned int vlanId,unsigned int *groupcount);
INT igmp_show_group_cnt_all_dbus(unsigned int * groupcount);
INT igmp_del_spec_mcgroup_dbus(	LONG vlan_id,	ULONG groupaddr);
INT igmp_del_all_mcgvlan_dbus(VOID);
INT igmp_snp_npd_msg_proc(struct npd_mng_igmp *msg_skb);
void *create_npd_msg_thread(void);
void *create_npd_msg_thread_dgram(void);

/*****************************************global value****************************************/
INT		npdmng_fd = 0;/* for stream*/
INT		recv_fd = 0;  /* for dgram */
/*for dgram*/
struct sockaddr_un local_addr;
struct sockaddr_un remote_addr;
#if 0
/***********************************declare functions**************************************/
static void *igmp_set_snp_enable(struct cfg_element *cur,void *value);
static void *igmp_set_snp_debug(struct cfg_element *cur,void *value);
static void *igmp_cfg_add_vlan(struct cfg_element *cur,void *value);
static void *igmp_cfg_add_port(struct cfg_element *cur,void *value);
static void *igmp_cfg_add_trunkport(struct cfg_element *cur,void *value);
static void *igmp_cfg_add_routeport(struct cfg_element *cur,void *value);
static INT igmp_snp_msg_proc(cmd_msg *t_msg, INT *flag);
static INT igmp_del_spec_mcgroup(cmd_msg *t_msg);
static INT igmp_del_all_vlan(cmd_msg *t_msg );
static INT igmp_show_group_cnt(cmd_msg *t_msg);
static INT igmp_show_host_timerinterval(cmd_msg *t_msg);
static INT igmp_show_query_timerinterval(cmd_msg *t_msg);
static INT igmp_set_query_timerinterval(cmd_msg *t_msg);
static INT igmp_show_resp_timerinterval(cmd_msg *t_msg);
static INT igmp_set_robust_timerinterval(cmd_msg *t_msg);
static INT igmp_show_robust_timerinterval(cmd_msg *t_msg);
static INT igmp_set_grouplife_timerinterval(cmd_msg *t_msg);
static INT igmp_show_grouplife_timerinterval(cmd_msg *t_msg);
static INT igmp_set_vlanlife_timerinterval(cmd_msg *t_msg);
static INT igmp_show_vlanlife_timerinterval(cmd_msg *t_msg);
static INT igmp_show_mcroute_port(cmd_msg *t_msg);
/********************************************global value****************************************/
struct cfg_element igmp_config_element[] = {
		{"igmp_snoop_enable",0,1,1,(void *)igmp_set_snp_enable},
		{"igmp_snoop_debug",0,1,0,(void *)igmp_set_snp_debug},
		{"add vlan",0,0,0,(void *)igmp_cfg_add_vlan},
		{"add port",0,0,0,(void *)igmp_cfg_add_port},
		{"add trunk port",0,0,0,(void *)igmp_cfg_add_trunkport},
		{"test router port",0,0,0,(void *)igmp_cfg_add_routeport}
};
/************************************extern value****************************************/
#endif
extern igmp_routerport	*p_routerlist;
extern LONG	igmp_snoop_enable;	/*IGMP snoop enable or not*/
extern LONG	mld_snoop_enable;	/*MLD snoop enable or not*/
extern igmp_vlan_list *p_vlanlist;		/*system vlan information*/

extern UINT igmp_snp_daemon_log_open;
extern INT	first_mcgroupvlan_idx;	/*初始值为-1*/
extern ULONG igmp_groupcount;
extern ULONG mld_groupcount;
extern ULONG igmp_vlancount;/*vlan in system enabled for IGMP*/
extern MC_group_vlan	*mcgroup_vlan_queue[IGMP_GENERAL_GUERY_MAX];
extern USHORT igmp_robust_variable;		/*default IGMP_ROBUST_VARIABLE*/
extern ULONG igmp_query_interval;			/*default IGMP_V2_UNFORCED_QUERY_INTERVAL*/
extern ULONG igmp_resp_interval;			/*default IGMP_V2_QUERY_RESP_INTERVAL*/
extern ULONG igmp_grouplife;				/*IGMP_GROUP_LIFETIME*/
extern ULONG igmp_vlanlife;				/*IGMP_VLAN_LIFE_TIME*/
extern LONG igmp_snp_searchvlangroup( LONG lVid, ULONG ulGroup,
									MC_group **ppMcGroup,
									MC_group_vlan **ppPrevGroupVlan );
extern LONG igmp_vlanisused( LONG vlan_id, ULONG * ret );
/***************************************************************************************/

/***********************************functions ********************************************/
/*void *create_npd_msg_thread(void);*/
/***************************************************************************************/


/*******************************************************************************
 * create_igmpsnp_clientsock_dgram
 *
 * DESCRIPTION:
 * 		create igmp client socket,socket type is dgram.    		
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	cltsock - the socket value
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK   - create success
 * 		IGMPSNP_RETURN_CODE_ERROR	- get socket or bind error
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
unsigned int	create_igmpsnp_clientsock_dgram(int *cltsock)
{
	memset(&local_addr,0,sizeof(local_addr));
	memset(&remote_addr,0,sizeof(remote_addr));

	if((*cltsock = socket(AF_LOCAL,SOCK_DGRAM,0)) == -1)
	{
		igmp_snp_syslog_err("igmpsnp_clientSock_dgram Create Fail.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	
	local_addr.sun_family = AF_LOCAL;
	strcpy(local_addr.sun_path, "/tmp/npd2igmpSnp_client");
	
	remote_addr.sun_family = AF_LOCAL;
	strcpy(remote_addr.sun_path, "/tmp/npd2igmpSnp_server");
	
    unlink(local_addr.sun_path);

	if(bind(*cltsock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1) 
	{
		igmp_snp_syslog_err("igmpsnp_clientSock_dgram Create Fail.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

	chmod(local_addr.sun_path, 0777);
	return IGMPSNP_RETURN_CODE_OK;	
	
}
/*******************************************************************************
 * igmp_add_routeport
 *
 * DESCRIPTION:
 * 		add route port to vlan ,if the route list is null,create one.  	
 *
 * INPUTS:
 * 		vlan_id - vlan id
 *		ifindex - port inerface index
 *		group - the group id
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - add route port success
 *		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL	- malloc memery fail
 *		IGMPSNP_RETURN_CODE_ERROR - search router port list fail
 *		
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_add_routeport(LONG vlan_id, ULONG ifindex, ULONG group)
{
	MC_group * t_group = NULL;
	MC_group_vlan *t_gpvlan = NULL;
	igmp_routerport *new_routeport = NULL;
	igmp_router_entry *t_router = NULL;
	igmp_router_entry *pre_router = NULL;
	INT ret = IGMPSNP_RETURN_CODE_OK;

	igmp_snp_syslog_dbg("add route port,vlan_id:%d ifindex:0x%x group:%u.%u.%u.%u\n", vlan_id, ifindex, NIPQUAD(group));
	/*serch: not found it'll create*/
	ret = igmp_snp_searchrouterportlist(vlan_id, ifindex, IGMP_PORT_ADD, &new_routeport);
	if (IGMPSNP_RETURN_CODE_OK != ret)
	{
		return ret;
	}

	/* set 0 to group, because there just want to check the vlan_id valid, maybe we can get t_group,
	because the mld multcast group's MC_ipadd =0,they did not take any role in the following.*/
	ret = igmp_snp_searchvlangroup(vlan_id, group, &t_group, &t_gpvlan);
	if (IGMPSNP_RETURN_CODE_OK != ret || t_gpvlan == NULL)
	{
		igmp_snp_syslog_err("search multicast-groupVlan failed.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

 	/***************************************/
 	/* add the route port to routerlist of the groupvlan */
 	/***************************************/
	t_router = t_gpvlan->routerlist;
	pre_router = t_router;

	/* find the last position, if routerlist not null */
	if(NULL != t_gpvlan->routerlist)
	{
		while(t_router)
		{
			/* pre_router is the last router ,when loop stop*/
			pre_router = t_router;		
			t_router = t_router->next;
		}
	}
	
	t_router = NULL;
	t_router = (igmp_router_entry *)malloc(sizeof(igmp_router_entry));
	if(NULL == t_router)
	{
		igmp_snp_syslog_err("alloc memory for router failed.\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset(t_router, 0, sizeof(igmp_router_entry));

	t_router->mroute_ifindex = ifindex;

	if (t_gpvlan->routerlist == NULL)
	{
		t_gpvlan->routerlist = t_router;
	}
	else
	{
		pre_router->next = t_router;
	}

	t_router->next = NULL;

	igmp_snp_syslog_dbg("add route port success. vlan_id:%d ifindex:0x%x\n",
						t_gpvlan->vlan_id, ifindex);

	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_del_routeport
 *
 * DESCRIPTION:
 * 		delete route port from vlan .  	
 *
 * INPUTS:
 * 		vlan_id - vlan id
 *		ifindex - port inerface index
 *		group - the group id
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - delete route port success
 *		IGMPSNP_RETURN_CODE_ROUTE_PORT_NOTEXIST	- can't find the route port
 *		IGMPSNP_RETURN_CODE_ERROR - search router port list fail
 *		
 * COMMENTS:
 *      
 **
 ********************************************************************************/

INT igmp_del_routeport(LONG vlan_id, ULONG ifindex, ULONG group )
{
	MC_group * t_group = NULL;
	igmp_routerport *new_routeport = NULL;
	MC_group_vlan *t_gpvlan = NULL;
	igmp_router_entry *t_router = NULL;
	igmp_router_entry *pre_router = NULL;
	INT ret = IGMPSNP_RETURN_CODE_OK;
	/* flg for delete route port from groupVlan's routelist
	 *  del_flg = 0,  is the first position
	 *  del_flg > 0,  isn't the first position
	 */
	INT del_flg = 0;

	igmp_snp_syslog_dbg("delete route port,vlan_id:%d ifindex:0x%x group:%u.%u.%u.%u\n",
						vlan_id, ifindex, NIPQUAD(group));

	ret = igmp_snp_searchrouterportlist(vlan_id, ifindex, IGMP_PORT_DEL, &new_routeport);
	if (IGMPSNP_RETURN_CODE_OK != ret)
	{
		igmp_snp_syslog_err("search route port %d in p_routerlist failed.\n", ifindex);
		return ret;
	}		
	igmp_snp_syslog_dbg("delete route port %d in p_routerlist ok, next delete the groupVlan %d.\n",
						ifindex, vlan_id);

	/* set 0 to group, because there just want to check the vlan_id valid, maybe we can get t_group,
	because the mld multcast group's MC_ipadd =0,they did not take any role in the following.*/
	ret= igmp_snp_searchvlangroup(vlan_id, group, &t_group, &t_gpvlan);
	if (IGMPSNP_RETURN_CODE_OK != ret || t_gpvlan == NULL)
	{
		igmp_snp_syslog_err("search multicast-groupVlan failed.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}

 	/******************************************/
 	/* delete the route port from routerlist of the groupvlan */
 	/******************************************/
	t_router = t_gpvlan->routerlist;
	pre_router = t_gpvlan->routerlist;
	del_flg = 0;
	igmp_snp_syslog_dbg("delete route port %d in groupVlan %d START.\n", ifindex, t_gpvlan->vlan_id);
	if( NULL != t_gpvlan->routerlist )
	{
		while(t_router)
		{
			/* find the route port, delete it */
			if (t_router->mroute_ifindex == ifindex)
			{
				if (del_flg == 0) {
					t_gpvlan->routerlist = t_router->next;
					pre_router = t_gpvlan->routerlist;					
				}
				else {
					pre_router->next = t_router->next;
				}
				igmp_snp_syslog_dbg("delete before\n");
				igmp_snp_syslog_dbg("delete route port %d in groupVlan %d ok.\n",
									ifindex, t_gpvlan->vlan_id);

				free(t_router);
				t_router = NULL;				igmp_snp_syslog_dbg("delete after\n");
				return IGMPSNP_RETURN_CODE_OK;
			}

			del_flg++;		
			pre_router = t_router;
			t_router = pre_router->next;
			igmp_snp_syslog_dbg("next route port.\n");
		}
	}

	igmp_snp_syslog_err("delete route port failed, there is not router port %d in Vlan %d.\n",
						ifindex, t_gpvlan->vlan_id);

	return IGMPSNP_RETURN_CODE_ROUTE_PORT_NOTEXIST;
}

/*Dbus CLI : global enable igmp snoop*/
/*******************************************************************************
 * igmp_set_enable
 *
 * DESCRIPTION:
 *   		begin to set the igmp enable or disable.
 *
 * INPUTS:
 * 		flag - a flag to define enable or disable igmp snooping
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set endis igmp snoop done
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_set_enable(LONG flag)
{
	int ret = IGMPSNP_RETURN_CODE_OK;

	if(ENABLE == mld_snoop_enable)
	{
   		igmp_snp_syslog_err(" mld snoop is enable, can not start igmp snoop!\n");	
    	return IGMPSNP_RETURN_CODE_ERROR_SW;		
	}
	else
	{
   		igmp_snp_syslog_dbg("igmp snoop is set to flag: %d.\n",flag);		
    	if( flag )
    	{
       		igmp_snp_syslog_dbg("igmp snoop enable start init:\n");			
   			ret = igmp_enable_init();
    	}
    	else
    	{
       		igmp_snp_syslog_dbg("igmp snoop enable stop: \n");						
   			ret = igmp_snp_stop();
    	}

   		igmp_snp_syslog_dbg(" igmp snoop is set flag %d ret %d.\n",flag,ret);				
		if(IGMPSNP_RETURN_CODE_OK == ret)
		{
    	    igmp_snoop_enable = flag;		
		}		
	}
	return ret;
}

/**********************************/
/*******************************************************************************
 * igmp_snp_set_enable_dbus
 *
 * DESCRIPTION:
 *   		set the enable igmp snooping flag .
 *
 * INPUTS:
 * 		null	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set success
 *		IGMPSNP_RETURN_CODE_ALREADY_SET - the enable already set
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_snp_set_enable_dbus()
{
	LONG flag = 0;
    int ret = IGMPSNP_RETURN_CODE_OK;
	
	if(!igmp_snoop_enable){
		flag = IGMP_SNOOP_YES;
		ret = igmp_set_enable(flag);
	}
	else{
		//IGMP_SNP_DEBUG("igmp snp has been Enable!\r\n");
		return IGMPSNP_RETURN_CODE_ALREADY_SET;/*error*/
	}
	return ret;/*success*/
}
/*******************************************************************************
 * igmp_snp_set_disable_dbus
 *
 * DESCRIPTION:
 *   		set the disable igmp snooping flag .
 *
 * INPUTS:
 * 		null	
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set success
 *		IGMPSNP_RETURN_CODE_ALREADY_SET - the disable already set
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_snp_set_disable_dbus()
{
	LONG flag = 0;
	int ret = IGMPSNP_RETURN_CODE_OK;
	
	if(igmp_snoop_enable){
		flag = IGMP_SNOOP_NO;
		ret = igmp_set_enable(flag);
	}
	else{
		igmp_snp_syslog_err("igmp snp has been Disable!\n");
		return IGMPSNP_RETURN_CODE_ALREADY_SET;
	}
	return ret;
}

/*******************************************************************************
 * igmp_set_vlanlife_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		set vlan life time interval.
 *
 * INPUTS:
 * 		timeout - the vlan time interval value
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set time seccess
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp snoop not global enable 
 *		IGMPSNP_RETURN_CODE_SAME_VALUE  - the time is the same with current value
 *		IGMPSNP_RETURN_CODE_OUT_RANGE - the value is out of the range
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_set_vlanlife_timerinterval_dbus(unsigned int timeout)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_set_vlanlife_timerinterval:igmp snoop or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if((timeout <= 100000) &&
	   (timeout >= 10000))/*default value 42,000*/
	{
		if (igmp_vlanlife == timeout)
		{
			return IGMPSNP_RETURN_CODE_SAME_VALUE;
		}
		else
		{
			igmp_vlanlife = timeout;
		}
	}
	else {
		//igmp_vlanlife = IGMP_VLAN_LIFE_TIME;/*stay at the value last config*/
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * igmp_set_grouplife_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		set group life time interval.
 *
 * INPUTS:
 * 		timeout - the group time interval value
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set time seccess
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp snoop not global enable 
 *		IGMPSNP_RETURN_CODE_SAME_VALUE  - the time is the same with current value
 *		IGMPSNP_RETURN_CODE_OUT_RANGE - the value is out of the range
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_set_grouplife_timerinterval_dbus(unsigned int timeout)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_set_grouplife_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if((timeout <= 50000) &&
	   (timeout >= 1000))/*default value 21,000*/
	{
		if (igmp_grouplife == timeout)
		{
			return IGMPSNP_RETURN_CODE_SAME_VALUE;
		}
		else
		{
			igmp_grouplife = timeout;
		}
	}	
	else {
		//igmp_grouplife = IGMP_GROUP_LIFETIME;/*stay at the value last config*/
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
		
	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * igmp_set_query_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		set query  time interval.
 *
 * INPUTS:
 * 		timeout - the query interval value
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set time seccess
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp snoop not global enable 
 *		IGMPSNP_RETURN_CODE_SAME_VALUE  - the time is the same with current value
 *		IGMPSNP_RETURN_CODE_OUT_RANGE - the value is out of the range
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_set_query_timerinterval_dbus(unsigned int timeout)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_set_query_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if((timeout <= 10000) &&
	   (timeout >= 1000))/*default value 5000*/
	{
		if (igmp_query_interval == timeout)
		{
			return IGMPSNP_RETURN_CODE_SAME_VALUE;
		}
		else
		{
			igmp_query_interval = timeout;
		}
	}		
	else {
		//igmp_query_interval = IGMP_V2_UNFORCED_QUERY_INTERVAL;/*stay at the value last config*/
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_set_resp_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		set response  time interval.
 *
 * INPUTS:
 * 		timeout - the rsponse interval value
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set time seccess
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp snoop not global enable 
 *		IGMPSNP_RETURN_CODE_SAME_VALUE  - the time is the same with current value
 *		IGMPSNP_RETURN_CODE_OUT_RANGE - the value is out of the range
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_set_resp_timerinterval_dbus(unsigned int  timeout)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_show_resp_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	if((100 <= timeout)&&(1000 >=timeout)) /*default value 500*/
	{
		if (igmp_resp_interval == timeout)
		{
			return IGMPSNP_RETURN_CODE_SAME_VALUE;
		}
		else
		{
			igmp_resp_interval = timeout;
		}
	}		
	else {
		//igmp_resp_interval = IGMP_V2_QUERY_RESP_INTERVAL;/*stay at the value last config*/
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_set_robust_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		set robust  value.
 *
 * INPUTS:
 * 		timeout - the robust value
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set time seccess
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp snoop not global enable 
 *		IGMPSNP_RETURN_CODE_SAME_VALUE  - the time is the same with current value
 *		IGMPSNP_RETURN_CODE_OUT_RANGE - the value is out of the range
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_set_robust_timerinterval_dbus(unsigned int  timeout)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_set_robust_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if((1 <= timeout)&&(100>=timeout))
	{
		if (igmp_robust_variable == timeout)
		{
			return IGMPSNP_RETURN_CODE_SAME_VALUE;
		}
		else
		{
			igmp_robust_variable = timeout;
		}
	}	
	else {
		//igmp_robust_variable = IGMP_ROBUST_VARIABLE;
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	return IGMPSNP_RETURN_CODE_OK;
}

/*******************************************************************************
 * igmp_snp_config_timer_dbus
 *
 * DESCRIPTION:
 *   		the dbus to config timer include vlanlife timer \grouplife timer\ querytimer\response timer\robust.
 *
 * INPUTS:
 * 		type - indicate config one of vlanlife timer \grouplife timer\ querytimer\response timer\robust.
 *		timeout - the time value
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - set success
 *
 * COMMENTS:
 *      	when return ret ,it include the return value from the five dbus
 **
 ********************************************************************************/
INT igmp_snp_config_timer_dbus
(
	unsigned int type,
	unsigned int timeout
)
{
	unsigned int ret = IGMPSNP_RETURN_CODE_OK;

	switch(type){
		case VLAN_LIFETIME: /*vlan lifetime*/
			ret = igmp_set_vlanlife_timerinterval_dbus(timeout);
		 	break;
		case GROUP_LIFETIME: /*group lifetime*/
			ret = igmp_set_grouplife_timerinterval_dbus(timeout);
			break;
		case ROBUST_VARIABLE: /*robust variable*/
			ret = igmp_set_robust_timerinterval_dbus(timeout);
			break;
		case QUERY_INTERVAL: /*query interval*/
			ret = igmp_set_query_timerinterval_dbus(timeout);
			break;
		case RESP_INTERVAL: /*resp interval*/
			ret = igmp_set_resp_timerinterval_dbus(timeout);
			break;
		case RXMT_INTERVAL: /*rmxt interval*/
		 	break;
		default :
			igmp_snp_syslog_err("IGMP snoop timer type Error!\n");
			break;

	}
	return ret;
}
/*******************************************************************************
 * igmp_show_vlanlife_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		get current vlan life timer interval value
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	mcgvlanlife - timer interval value
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the timer success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not  global enable 
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the pointer is null
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
INT igmp_show_vlanlife_timerinterval_dbus(unsigned int *mcgvlanlife)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()))
	{
		igmp_snp_syslog_dbg("igmp_show_vlanlife_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if( NULL == mcgvlanlife )
	{
		igmp_snp_syslog_err("igmp_show_vlanlife_timerinterval:parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	*mcgvlanlife = igmp_vlanlife;
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_show_grouplife_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		get current group life timer interval value
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	grouplife - timer interval value
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the timer success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not  global enable 
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the pointer is null
 *
 * COMMENTS:
 *    
 **
 ********************************************************************************/
INT igmp_show_grouplife_timerinterval_dbus(unsigned int *grouplife)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_show_grouplife_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if( NULL == grouplife)
	{
		igmp_snp_syslog_err("igmp_show_grouplife_timerinterval:parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	*grouplife= igmp_grouplife;
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_show_query_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		get current query timer interval value
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	queryinterval - timer interval value
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the timer success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not  global enable 
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the pointer is null
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
INT igmp_show_query_timerinterval_dbus(unsigned int* queryinterval)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_show_query_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if( NULL == queryinterval )
	{
		igmp_snp_syslog_err("igmp_show_query_timerinterval:parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	*queryinterval = igmp_query_interval;
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_show_resp_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		get current resp timer interval value
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	respinterval - timer interval value
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the timer success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not  global enable 
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the pointer is null
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
INT igmp_show_resp_timerinterval_dbus(unsigned int* respinterval)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_show_resp_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if( NULL == respinterval )
	{
		igmp_snp_syslog_err("igmp_show_resp_timerinterval:parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	*respinterval = igmp_resp_interval;
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_show_robust_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		get current robust interval value
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	robust -  interval value
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the timer success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not  global enable 
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the pointer is null
 *
 * COMMENTS:
 *   
 **
 ********************************************************************************/
INT igmp_show_robust_timerinterval_dbus(unsigned int* robust)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_show_robust_timerinterval:igmp mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if( NULL == robust )
	{
		igmp_snp_syslog_err("igmp_show_robust_timerinterval:parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	*robust = igmp_robust_variable;
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_show_host_timerinterval_dbus
 *
 * DESCRIPTION:
 *   		get current host timer interval value,
 *		host_timerinterval = igmp_robust_variable * igmp_query_interval + igmp_resp_interval
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	hosttime - timer interval value
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the timer success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not  global enable 
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the pointer is null
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_show_host_timerinterval_dbus(unsigned int * hostime)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_show_host_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if( NULL == hostime)
	{
		igmp_snp_syslog_err("igmp_show_host_timerinterval:parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	*hostime = igmp_robust_variable * igmp_query_interval + igmp_resp_interval;
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_show_igmpvlan_cnt_dbus
 *
 * DESCRIPTION:
 *   		get igmp vlan count.
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	igmpvlanCnt - output the count number
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the count success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not  global enable 
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_show_igmpvlan_cnt_dbus(unsigned int *igmpvlanCnt)
{

	LONG	lRet;
	ULONG 	i,ulRet;
	//igmp_vlan_list *t_vlan = NULL;
	INT vCnt = 0;
	if( !IGMP_SNP_ISENABLE())
	{
		igmp_snp_syslog_dbg("igmp_show_host_timerinterval:igmp snoop is disable.\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	
	for(i=0;i<IGMP_GENERAL_GUERY_MAX;i++){
		lRet = igmp_vlanisused(i,&ulRet);
		if(IGMPSNP_RETURN_CODE_OK == lRet && 1 == ulRet) {
			igmp_snp_syslog_dbg("find vlan %ld usedIgmp.\n",i);
			vCnt++;
		}
	}

	*igmpvlanCnt = vCnt;
	return IGMPSNP_RETURN_CODE_OK;
}

#if 0
INT igmp_show_group_cnt_dbus
(
	unsigned int vlanId,
	unsigned int *groupcount
)
{
	LONG lRet = IGMP_SNOOP_OK;
	ULONG ulGroup,ulIfIndex = 0;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	ULONG ulRet;
	INT	groupCnt = 0;
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_show_group_cnt:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == groupcount)
	{
		IGMP_SNP_DEBUG("igmp_show_group_cnt:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	/*find in mcgroupVlan queue, and find out there exists any group*/
	/*TODO*/
	lRet = igmp_vlanisused(vlanId,&ulRet);
	if(IGMP_SNOOP_OK == lRet && 1 == ulRet) {
		IGMP_SNP_DEBUG("find vlan %ld usedIgmp.\r\n",vlanId);
		lRet = igmp_snp_searchvlangroup(vlanId, ulGroup, &pMcGroup, &pPrevGroupVlan);
		if ( IGMP_SNOOP_OK != lRet)	{
			IGMP_SNP_DEBUG( "igmp_recv_report: search failed\r\n" );
			return IGMP_SNOOP_ERR;
		}
		if ( NULL != pMcGroup )	{
			IGMP_SNP_DEBUG("find mcGroup one:vid=%d,mcgroupip = %u.%u.%u.%u\r\n",\
										pMcGroup->vid,NIPQUAD(pMcGroup->MC_ipadd));
			groupCnt++;/*what's different with igmp_vlancount*/
		}
	}

	*groupcount = groupCnt;
	return IGMP_SNOOP_OK;
}
#endif
/*******************************************************************************
 * igmp_show_group_total_cnt_dbus
 *
 * DESCRIPTION:
 *   		get the igmp group count.
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	groupcount - output the group count number
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - get the count success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not  global enable 
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
INT igmp_show_group_total_cnt_dbus
(
	unsigned int *groupcount
)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()) )
	{
		igmp_snp_syslog_dbg("igmp_show_host_timerinterval:igmp or mld snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	if(IGMP_SNP_ISENABLE()){
		*groupcount = igmp_groupcount;
	}
	else if(MLD_SNP_ISENABLE()){
		*groupcount = mld_groupcount;
	}
	
	return IGMPSNP_RETURN_CODE_OK;
}

INT igmp_show_group_cnt_all_dbus
(
	unsigned int *groupcount
)
{
	LONG lRet = IGMP_SNOOP_OK;
	ULONG lVid,ulGroup,ulIfIndex = 0;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	ULONG i,ulRet;	
	if( !IGMP_SNP_ISENABLE())
	{
		igmp_snp_syslog_dbg("igmp_show_group_cnt:igmp snoop is disable.\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == groupcount)
	{
		igmp_snp_syslog_err("igmp_show_group_cnt:parameter error.\n");
		return IGMPSNP_RETURN_CODE_ERROR;
	}
	/*find in mcgroupVlan queue, and find out there exists any group*/
	/*TODO*/
	for(i=0;i<IGMP_GENERAL_GUERY_MAX;i++) {
		lRet = igmp_vlanisused(i,&ulRet);
		if(IGMPSNP_RETURN_CODE_OK == lRet && 1 == ulRet) {
			igmp_snp_syslog_dbg("find vlan %ld usedIgmp.\n",i);
			lRet = igmp_snp_searchvlangroup(i, ulGroup, &pMcGroup, &pPrevGroupVlan);
			if ( IGMP_SNOOP_OK != lRet)	{
				igmp_snp_syslog_err( "igmp_recv_report: search failed\n" );
				return IGMPSNP_RETURN_CODE_ERROR;
			}
			if ( NULL != pMcGroup )	{
				igmp_snp_syslog_dbg("find mcGroup one:vid=%d,mcgroupip = %u.%u.%u.%u\n",\
											pMcGroup->vid,NIPQUAD(pMcGroup->MC_ipadd));
				igmp_groupcount++;/*what's different with igmp_vlancount*/
			}
		}
	}
	igmp_snp_syslog_dbg("check loop complete!\n");
	*groupcount = igmp_groupcount;
	return IGMPSNP_RETURN_CODE_ERROR;
}
/*******************************************************************************
 * igmp_del_spec_mcgroup_dbus
 *
 * DESCRIPTION:
 *   		delete the igmp multicast group by group address
 *
 * INPUTS:
 * 		vlan_id - vlan  id
 *		groupaddr - the group address which will be delete
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - delete group success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not global enable
 *		IGMPSNP_RETURN_CODE_GROUP_NOTEXIST - the group not exist
 *		IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST - the vlan which include group not exist
 *
 * COMMENTS:
 *      group is the first group,in fact the last one,delete the mcgroup vlanself .
 **
 ********************************************************************************/
INT igmp_del_spec_mcgroup_dbus
(
	LONG vlan_id,
	ULONG groupaddr
)
{
	MC_group *t_group = NULL;
	MC_group *t_nextgroup = NULL;
	MC_group_vlan *t_gpvlan = NULL;

	if( !IGMP_SNP_ISENABLE())
	{
		igmp_snp_syslog_dbg("igmp_del_spec_mcgroup:igmp snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
	
	if( 0 == groupaddr)	/*delete all group in this vlan and mcgroup vlanself*/
	{
		igmp_snp_del_mcgroupvlan(vlan_id);
	}
	else
	{
		igmp_snp_syslog_dbg("igmp_del_spec_mcgroup:vlan_id:0x%x\tgroup:%u.%u.%u.%u\n",
				vlan_id,NIPQUAD(groupaddr));
		if( IGMPSNP_RETURN_CODE_OK !=igmp_snp_searchvlangroup(vlan_id,groupaddr,&t_group,&t_gpvlan))
		{
			igmp_snp_syslog_err("igmp_del_spec_mcgroup:searchgroupvlan failed.\n");
			return IGMPSNP_RETURN_CODE_GROUP_NOTEXIST;
		}

		if( NULL == t_gpvlan )
		{
			igmp_snp_syslog_err("igmp_del_spec_mcgroup:this vlan %d is not exist.\n",
							vlan_id);
			return IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST;
		}
		
		if( NULL == t_group )
		{
			igmp_snp_syslog_dbg("igmp_del_spec_mcgroup:group:%u.%u.%u.%u. is not exist in vlan %d.\n",
							NIPQUAD(groupaddr),vlan_id);
			return IGMPSNP_RETURN_CODE_GROUP_NOTEXIST;
		}
		igmp_snp_delgroup(t_gpvlan,t_group,&t_nextgroup);
		/*if group is the firstgroup of the mcgroup vlan*/
		if( NULL == t_gpvlan->firstgroup )
		{	
			/*group is the first group,in fact the last one,\
			delete the mcgroup vlanself */
			igmp_snp_del_mcgroupvlan(t_gpvlan->vlan_id);
		}
		return IGMPSNP_RETURN_CODE_OK;
	}
}
/*******************************************************************************
 * igmp_del_all_mcgvlan_dbus
 *
 * DESCRIPTION:
 *   		delete all the igmp multicast vlan 
 *
 * INPUTS:
 * 		vlan_id - vlan  id
 *		groupaddr - the group address which will be delete
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - delete multicast vlan success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - igmp not global enable
 *
 * COMMENTS:
 *      group is the first group,in fact the last one,delete the mcgroup vlanself .
 **
 ********************************************************************************/
INT igmp_del_all_mcgvlan_dbus(VOID)
{
	MC_group_vlan *t_gpvlan = NULL;
	INT next = 0;
	if( !IGMP_SNP_ISENABLE())
	{
		igmp_snp_syslog_dbg("igmp_del_all_vlan:igmp snoop is disable.\n");
		return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	t_gpvlan = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);
	next = first_mcgroupvlan_idx;
	while(t_gpvlan)
	{
		igmp_snp_syslog_dbg("igmp_del_all_vlan:del vlan_id:%d",t_gpvlan->vlan_id);
		next = t_gpvlan->next;
		igmp_snp_del_mcgroupvlan(t_gpvlan->vlan_id);
		t_gpvlan = GET_VLAN_POINT_BY_INDEX(next);
	}
	first_mcgroupvlan_idx = -1;
	return IGMPSNP_RETURN_CODE_OK;
}
/*******************************************************************************
 * igmp_snp_npd_msg_proc
 *
 * DESCRIPTION:
 *		init igmp_snoop receive messsage distribute function
 *		call in 'create_npd_msg_thread',get message from NPD.  	
 *
 * INPUTS:
 * 		struct igmp_skb *msg_skb -	receive message pointer
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - init success
 *		IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL - not igmp global enable
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the input pointer is null
 *		IGMPSNP_RETURN_CODE_ERROR - the receive message length is not enough
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
INT igmp_snp_npd_msg_proc(struct npd_mng_igmp *msg_skb)
{
	if( (!IGMP_SNP_ISENABLE()) && (!MLD_SNP_ISENABLE()))
	{
		igmp_snp_syslog_err("IGMP Snooping is switch-off.\n");
		/*igmp_snp_device_event();*/
		if (msg_skb->npd_dev_mng.event != IGMPSNP_EVENT_DEV_UNREGISTER) {
			return IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL;
		}
	}

	if( NULL == msg_skb)
	{
		igmp_snp_syslog_err("igmp message process,parameter is NULL.\n");
 		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	
	switch(msg_skb->nlh.nlmsg_type)
	{
		case IGMP_SNP_TYPE_DEVICE_EVENT:/*3*/
			{
				ULONG event;
				dev_notify_msg *dev_info = NULL;

				if(msg_skb->nlh.nlmsg_len < (sizeof(dev_notify_msg)+sizeof(struct nlmsghdr)))
				{
					igmp_snp_syslog_dbg("igmp message process, msg_skb error:msg_skb->nlh.nlmsg_len = %d\n",
										msg_skb->nlh.nlmsg_len);
					igmp_snp_syslog_dbg("sizeof(dev_notify_msg) = %d,sizeof(struct nlmsghdr) = %d.\n",
										sizeof(dev_notify_msg), sizeof(struct nlmsghdr));
					return IGMPSNP_RETURN_CODE_ERROR;
				}
				dev_info = &(msg_skb->npd_dev_mng);
				event = dev_info->event;
				igmp_snp_device_event(event,dev_info);
			}
			break;
		default:
			igmp_snp_syslog_err("igmp message process,unknown msg type.\n");
			break;
	}
	return IGMPSNP_RETURN_CODE_OK;
}
#if 0
/**********************************************************************************
*create_npd_msg_thread()
*INPUTS:
*
*OUTPUTS:
*RETURN VALUE:
*	
*DESCRIPTION:
*	IGMP SNOOP command message handle thread
*
***********************************************************************************/
void *create_npd_msg_thread(void)
{
	int ret,write_flag;
	int sock,accept_sock =0,len;
	int recv_len = 0;
	struct igmp_skb* msg =  NULL;
	struct sockaddr_un client;
	char buf[IGMP_MSG_MAX_SIZE];
	fd_set rfds;

	/*variable 'sock' ----> 'npdmng_fd' :can NOT work*/
	if(0 >(sock = creatclientsock_stream(IGMP_SNOOP_NPD_MSG_SOCK)))
	{
		IGMP_SNP_DEBUG("create_msg_thread::Create msg socket failed.\r\n");
		return;
	}
	npdmng_fd = sock;
	IGMP_SNP_DEBUG("create_msg_thread::Create msg socket ok,MsgSock fd=%d.\r\n",sock);
	memset(&client,0,sizeof(struct sockaddr_un));
	len = sizeof(struct sockaddr_un);

	msg = (struct igmp_skb*)buf;
	while(1)
	{
		IGMP_SNP_DEBUG("create_npd_msg_thread::enter forever while loop.\r\n");
		#if 0
		if((accept_sock = accept(sock,(struct sockaddr *)&client,&len))<0)
		{
			IGMP_SNP_DEBUG("Accept failed:errno %d [%s].\r\n",errno,strerror(errno));
			close(sock);
			/*需要增加对其他线程的退出信号*/
			return;
		}
		DEBUG_OUT("create_npd_msg_thread:accept command.\r\n");
		while(1)
		#endif 
		{
			memset(buf,0,sizeof(char)*IGMP_MSG_MAX_SIZE);
			FD_ZERO(&rfds);
			FD_SET(sock,&rfds);
			switch(select(sock+1,&rfds,NULL,NULL,NULL))
			{
				case -1:
					printf("select return -1\n");
					break;
				case 0:
					printf("select return 0.\n");
					break;
				default:
					if(FD_ISSET(sock,&rfds))
					{
						recv_len = read(sock,buf,IGMP_MSG_MAX_SIZE);
						if( 0 == recv_len )
							break;
						ret = igmp_snp_npd_msg_proc(msg);
						break;
					}
			}
			if(0 == recv_len )
				break;
		}
		/*close(accept_sock);
		  accept_sock = 0;	*/
	}
	close(sock);
	//accept_sock = 0;	
}
#endif


/**********************************************************************************
* create_npd_msg_thread_dgram()
*
* DESCRIPTION:
*	IGMP SNOOP command message handle thread
*
* INPUTS:
*		null
*
* OUTPUTS:
* 		null
*
*RETURN VALUE:
*	
*
***********************************************************************************/
void *create_npd_msg_thread_dgram(void)
{
	int ret,write_flag;
	int sock=-1,recv_len = 0;

	struct npd_mng_igmp* msg =  NULL;
	char buf[NPD_MSG_MAX_SIZE];
	socklen_t addrlen = sizeof(struct sockaddr);
	fd_set rfds;

	if(IGMPSNP_RETURN_CODE_OK != create_igmpsnp_clientsock_dgram(&sock))
	{
		igmp_snp_syslog_dbg("Create msg socket dgram failed.\r\n");
		return;
	}
	recv_fd = sock;
	igmp_snp_syslog_dbg("Create msg socket ok,MsgSock fd=%d.\r\n",sock);

	msg = (struct npd_mng_igmp*)buf;
	while(1)
	{
		igmp_snp_syslog_dbg("enter forever while loop.\r\n");
		memset(buf,0,sizeof(char)*NPD_MSG_MAX_SIZE);
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		switch(select(sock+1,&rfds,NULL,NULL,NULL))
		{
			case -1:
				igmp_snp_syslog_dbg("select return -1\n");
				break;
			case 0:
				igmp_snp_syslog_dbg("select return 0.\n");
				break;
			default:
				if(FD_ISSET(sock,&rfds))
				{
					do{
						recv_len = recvfrom(sock,buf,NPD_MSG_MAX_SIZE,0,(struct sockaddr *)&remote_addr, &addrlen);
						/*recv_len = read(sock,buf,IGMP_MSG_MAX_SIZE);*/ 
						if( 0 == recv_len || -1 == recv_len){
							break;
						}
						ret = igmp_snp_npd_msg_proc(msg);
					}while(recv_len > 0);	
				}
				break;
		}
	}
	close(sock);
	return;
}

#if 0
/*read config file for init igmp snooping*/
void read_config(char *line)
{	
	int i;	
	int tmp;		
	IGMP_SNP_DEBUG("read_config...\r\n");
	if(!line)		
		return;			
	for(i=0;i<6;++i)	
	{		
		tmp = strlen(igmp_config_element[i].str);		
		if(!strncmp(line,igmp_config_element[i].str,tmp))		
		{			
			IGMP_SNP_DEBUG("##Match igmp_config:: %s\r\n",igmp_config_element[i].str);
			igmp_config_element[i].func(igmp_config_element+i,(void *)line);			
			break;		
		}	
	}
}

/*igmp_snp_del_mcgroupvlan(LONG vlan_id);*/

/***********************************
 * !1.diable vlan join igmp snp
 * !2.delete vlan from npd-dcli
 * !3.check vlan status DOWN
 ***********************************/
int igmp_snp_vlan_del_dbus(unsigned short vid)
{
	IGMP_SNP_DEBUG("delete vlan %d\r\n",vid);
	igmp_vlan_list *t_igmpvlan = NULL;
	MC_group_vlan	*t_mcgroupvlan = NULL;
	INT	next = 0;
	if( vid > 4094 )	/*max value*/
	{
		IGMP_SNP_DEBUG("igmp_cfg_add_vlan:parameter error.\r\n");
		return 0;
	}

	/*delete igmp vlan(if exist),mcgroup_vlan(if exist),on SW.*/
	if( !p_vlanlist )	/*first*/
	{
		t_igmpvlan = p_vlanlist;
		while((t_igmpvlan->vlan_id != vid)&&(NULL != t_igmpvlan->next ))/*==, !=*/
		{
			t_igmpvlan = t_igmpvlan->next;
		}
		if( t_igmpvlan->vlan_id == vid)
		{
			IGMP_SNP_DEBUG("igmp_cfg_add_vlan:vlan_id alread existence.\r\n");
			igmp_vlancount--;
			free(t_igmpvlan);/*release igmp_vlan*/

			/*call igmp_snp_event(EVENT_DEV_UNREGISTER,\
								dev_notify_msg *dev_info) \
			 TO DO:
			 	1).igmp_snp_del_mcgroupvlan 
			 	2).Igmp_Snoop_Del_Reporter_ByVlan
			*/
			/*
			ULONG event = EVENT_DEV_UNREGISTER;
			dev_notify_msg *dev_info = NULL;
			dev_info = (dev_notify_msg *)malloc(sizeof(dev_notify_msg));

			dev_info->event = EVENT_DEV_UNREGISTER;
			dev_info->ifindex = ifindex;/*ifindex can not determins*/
			dev_info->vlan_id = vid;
			igmp_snp_device_event(event,dev_info);
			*/
			/*find mcgroup vlan list by index vlan Id*/
			t_mcgroupvlan = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);
			while(t_mcgroupvlan)
			{
				if(t_mcgroupvlan->vlan_id == vid){
					DEBUG_OUT("igmp_del_igmpgroup_vlan:vlan:%d",t_mcgroupvlan->vlan_id);
					igmp_snp_del_mcgroupvlan(t_mcgroupvlan->vlan_id);
				}
				next = t_mcgroupvlan->next;
				t_mcgroupvlan = GET_VLAN_POINT_BY_INDEX(next);
			}
			Igmp_Snoop_Del_Reporter_ByVlan(vid);
		}
	}
	/*notify msg to Hw in order to delete l2Mc_entry(if exist)*/
	/*igmp_snp_del_mcgroupvlan --> igmp_snp_delgroup --> igmp_snp_mod_addr*/	
	return IGMP_SNOOP_OK;
}

int igmp_snp_del_port_dbus
(
	unsigned short vid,
	unsigned int port_index
)
{
	IGMP_SNP_DEBUG("Enter delete vlan port...\r\n");
	int i,vlan_id,ifindex;
	igmp_vlan_list *t_vlan = NULL;

	vlan_id = vid;
	ifindex = port_index;

	if( !p_vlanlist )
	{
		IGMP_SNP_DEBUG("igmp_cfg_delete_port: can not find any vlan.\r\n");
		return;
	}
	else
	{
		t_vlan = p_vlanlist;
		/*find igmp_vlan by vlan_id*/
		while((t_vlan->vlan_id != vlan_id )&&(NULL != t_vlan))/*==,!= wujh*/
			t_vlan = t_vlan->next;
		if( NULL == t_vlan )
		{
			IGMP_SNP_DEBUG("igmp_cfg_delete_port:can not find vlan.\r\n");
			return;
		}
		igmp_vlan_port_list *t_port = t_vlan->first_port;
		if( NULL == t_port )		/*first*/
		{
			IGMP_SNP_DEBUG("igmp_cfg_delete_port:can not find port.\r\n");
			return;
		}
		else
		{
			/**/
			igmp_vlan_port_list *new_port = NULL;
			while( t_port->ifindex != ifindex )
			{
				if( NULL == t_port->next)
					break;
				t_port = t_port->next;
			}
			if( t_port->ifindex == ifindex)
			{
				/*find the port,delete it from port_list.*/
				free(t_port);
				
				/*check the port is any mcgroup member,specially,find in MC_port_state list*/
				/*if yes,delete it,and send leave packet(here not to this)*/
				ULONG event = IGMPSNP_EVENT_DEV_DOWN;
				dev_notify_msg *dev_info = NULL;
				dev_info = (dev_notify_msg *)malloc(sizeof(dev_notify_msg));

				dev_info->event = IGMPSNP_EVENT_DEV_DOWN;
				dev_info->ifindex = ifindex;
				dev_info->vlan_id = vid;
				igmp_snp_device_event(event,dev_info);
				
				IGMP_SNP_DEBUG("igmp_cfg_add_port:ifindex has alread existence.\r\n");
				return;
			}
			IGMP_SNP_DEBUG("239vlan %ld port's ifindex %ld,trunkflag %ld,next port pointer %p\r\n",\
				t_vlan->vlan_id,t_port->ifindex,t_port->trunkflag,t_port->next);
		}

	}
}

#endif

#if 0
void *igmp_set_snp_enable(struct cfg_element *cur,void *value)
{
	char *str = (char *)value + 17 + 2;
	LONG p = atoi(str);		
	if( (p>= cur->min)&&(p<= cur->max))	{	
		//igmp_snoop_enable = p;
		IGMP_SNP_DEBUG("igmp_set_enable:p = %ld\r\n",p);
		igmp_set_enable(p);}
	else {		
		//igmp_snoop_enable = cur->def_value;
		IGMP_SNP_DEBUG("igmp_set_enable:cfg_element cur->def_value = %d\r\n",cur->def_value);
		igmp_set_enable(cur->def_value);}
}

void *igmp_set_snp_debug(struct cfg_element *cur,void *value)
{
	char *str = (char *)value + 16 +2;
	int p = atoi(str);		
	if( (p>= cur->min)	&&(p<= cur->max))		
		igmp_snoop_debug = p;	
	else		
		igmp_snoop_debug = cur->def_value;
}

void *igmp_cfg_add_vlan(struct cfg_element *cur,void *value)
{
	char *str = (char *)value + 9;
	int p = atoi(str);
	IGMP_SNP_DEBUG("add vlan %d\r\n",p);
	igmp_vlan_list *t_vlan = NULL;

	if( p > 4094 )	/*max value*/
	{
		IGMP_SNP_DEBUG("igmp_cfg_add_vlan:parameter error.\r\n");
		return;
	}
	else
	{
		if( !p_vlanlist )	/*first*/
		{
			IGMP_SNP_DEBUG("The first vlan in the system.\r\n");
			t_vlan = (igmp_vlan_list *)malloc(sizeof(igmp_vlan_list));
			if(NULL == t_vlan )
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_vlan:malloc memory failed.\r\n");
				return;
			}
			memset(t_vlan,0,sizeof(igmp_vlan_list));
			t_vlan->next = NULL;
			t_vlan->vlan_id = p;
			t_vlan->first_port = NULL;
			p_vlanlist = t_vlan;
		}
		else
		{
			igmp_vlan_list *new_vlan =NULL;
			t_vlan = p_vlanlist;
			while((t_vlan->vlan_id != p)&&(NULL != t_vlan->next ))/*==, !=*/
			{
				t_vlan = t_vlan->next;
			}
			if( t_vlan->vlan_id == p)
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_vlan:vlan_id alread existence.\r\n");
				return;
			}
			
			new_vlan= (igmp_vlan_list *)malloc(sizeof(igmp_vlan_list));
			if(NULL == new_vlan )
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_vlan:malloc memory failed.\r\n");
				return;
			}
			memset(new_vlan,0,sizeof(igmp_vlan_list));
			new_vlan->next = NULL;
			new_vlan->vlan_id = p;
			new_vlan->first_port = NULL;
			t_vlan->next = new_vlan;
		}
	}
	igmp_vlancount++;/**/
}

void *igmp_cfg_add_port(struct cfg_element *cur,void *value)
{
	IGMP_SNP_DEBUG("Eter add vlan port...\r\n");
	char *str = (char *)value + 9;
	char buf[12];
	int i,vlan_id,ifindex;
	igmp_vlan_list *t_vlan = NULL;
	
	i=0;
	while(*(str+i) !='\t')
		++i;
	memset(buf,0,sizeof(char)*12);
	memcpy(buf,str,sizeof(char)*i);
	vlan_id = atoi(buf);

	str +=i+1;
	ifindex = atoi(str);

	if( !p_vlanlist )
	{
		IGMP_SNP_DEBUG("igmp_cfg_add_port: can not find any vlan.\r\n");
		return;
	}
	else
	{
		t_vlan = p_vlanlist;
		while((t_vlan->vlan_id != vlan_id )&&(NULL != t_vlan))/*==,!= wujh*/
			t_vlan = t_vlan->next;

		if( NULL == t_vlan )
		{
			IGMP_SNP_DEBUG("igmp_cfg_add_port:can not find vlan.\r\n");
			return;
		}
		{
		igmp_vlan_port_list *t_port = t_vlan->first_port;
		if( NULL == t_port )		/*first*/
		{
			IGMP_SNP_DEBUG("add first port...\r\n");
			t_port = (igmp_vlan_port_list *)malloc(sizeof(igmp_vlan_port_list));
			if( NULL == t_port)
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_port:malloc memory failed.\r\n");
				return;
			}
			t_port->next = NULL;
			t_port->ifindex = ifindex;/*t_port->ifindex is just portNum.*/
			t_port->trunkflag = 0;		
			t_vlan->first_port = t_port;
			IGMP_SNP_DEBUG("210vlan %ld port's ifindex %ld,trunkflag %ld,next port pointer %p\r\n",\
				t_vlan->vlan_id,t_port->ifindex,t_port->trunkflag,t_port->next);
		}
		else
		{
			igmp_vlan_port_list *new_port = NULL;
			while( t_port->ifindex != ifindex )
			{
				if( NULL == t_port->next)
					break;
				t_port = t_port->next;
			}
			if( t_port->ifindex == ifindex)
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_port:ifindex has alread existence.\r\n");
				return;
			}
			new_port = (igmp_vlan_port_list *)malloc(sizeof(igmp_vlan_port_list));
			if(NULL == new_port)
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_port:malloc memory failed.\r\n");
				return;
			}
			new_port->next = NULL;
			new_port->ifindex = ifindex;
			new_port->trunkflag = 0;	
			t_port->next = new_port;
			IGMP_SNP_DEBUG("239vlan %ld port's ifindex %ld,trunkflag %ld,next port pointer %p\r\n",\
				t_vlan->vlan_id,t_port->ifindex,t_port->trunkflag,t_port->next);
		}
		}
	}
}

void *igmp_cfg_add_trunkport(struct cfg_element *cur,void *value)
{
	IGMP_SNP_DEBUG("Eter add vlan trunk_port...\r\n");
	char *str = (char *)value + 15;
	char buf[12];
	int i,vlan_id,ifindex,trunkifindex;
	igmp_vlan_list *t_vlan = NULL;
	
	i=0;
	while(*(str+i) !='\t')
		++i;
	memset(buf,0,sizeof(char)*12);
	memcpy(buf,str,sizeof(char)*i);
	vlan_id = atoi(buf);
	str +=i +1;
	
	i = 0;
	while(*(str+i)!='\t')
		++i;
	memset(buf,0,sizeof(char)*12);
	memcpy(buf,str,sizeof(char)*i);
	ifindex = atoi(buf);
	
	str +=i+1;
	trunkifindex = atoi(str);

	if( !p_vlanlist )
	{
		IGMP_SNP_DEBUG("igmp_cfg_add_port: can not find any vlan.\r\n");
		return;
	}
	else
	{
		t_vlan = p_vlanlist;
		while((t_vlan->vlan_id != vlan_id )&&(NULL == t_vlan))
			t_vlan = t_vlan->next;

		if( NULL == t_vlan )
		{
			IGMP_SNP_DEBUG("igmp_cfg_add_port:can not find vlan.\r\n");
			return;
		}
		{
		igmp_vlan_port_list *t_port = t_vlan->first_port;
		if( NULL == t_port )		/*first*/
		{
			t_port = (igmp_vlan_port_list *)malloc(sizeof(igmp_vlan_port_list));
			if( NULL == t_port)
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_port:malloc memory failed.\r\n");
				return;
			}
			t_port->next = NULL;
			t_port->ifindex = ifindex;
			t_port->trunkflag = trunkifindex;		
			t_vlan->first_port = t_port;
			IGMP_SNP_DEBUG("300vlan %ld trunk_port's ifindex %ld,trunkflag(trunkindex) %ld,next port pointer %p",\
				t_vlan->vlan_id,t_port->ifindex,t_port->trunkflag,t_port->next);
		}
		else
		{
			igmp_vlan_port_list *new_port = NULL;
			while( t_port->ifindex != ifindex )
			{
				if( NULL == t_port->next)
					break;
				t_port = t_port->next;
			}
			if( t_port->ifindex == ifindex)
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_port:ifindex has alread existence.\r\n");
				return;
			}
			new_port = (igmp_vlan_port_list *)malloc(sizeof(igmp_vlan_port_list));
			if(NULL == new_port)
			{
				IGMP_SNP_DEBUG("igmp_cfg_add_port:malloc memory failed.\r\n");
				return;
			}
			new_port->next = NULL;
			new_port->ifindex = ifindex;
			new_port->trunkflag = trunkifindex;	
			t_port->next = new_port;
			IGMP_SNP_DEBUG("327vlan %ld trunk_port's ifindex %ld,trunkflag(trunkindex) %ld,next port pointer %p\r\n",\
				t_vlan->vlan_id,t_port->ifindex,t_port->trunkflag,t_port->next);
		}
		}
	}
}


void *igmp_cfg_add_routeport(struct cfg_element *cur,void *value)
{	/*test, create pim packet*/
	char *str = (char *)value + 17;
	char buf[12];
	int i,vlan_id,ifindex;
	igmp_vlan_list *t_vlan = NULL;
	igmp_routerport *new_routeport = NULL;
	
	i=0;
	while(*(str+i) !='\t')
		++i;
	memset(buf,0,sizeof(char)*12);
	memcpy(buf,str,sizeof(char)*i);
	vlan_id = atoi(buf);

	str +=i+1;
	ifindex = atoi(str);

	igmp_snp_searchrouterportlist(vlan_id,ifindex,IGMP_PORT_ADD,&new_routeport);
}

/**********************************************************************************
*igmp_snp_msg_proc()
*INPUTS:
*command message handle function
*OUTPUTS:
*RETURN VALUE:
*	0 - success
*	!=0 - error
*DESCRIPTION:
*	IGMP SNOOP 
*
***********************************************************************************/
INT igmp_snp_msg_proc(cmd_msg *t_msg, INT *flag)
{
	INT ret;

	if( !t_msg)
	{
		IGMP_SNP_DEBUG("igmp_snp_msg_proc:Msg error.\r\n");
		return IGMP_SNOOP_ERR;
	}

	if( (CMD_MSG_TYPE_BEGIN>= t_msg->msgtype)||
		(CMD_MSG_TYPE_END<= t_msg->msgtype))
	{
		IGMP_SNP_DEBUG("igmp_snp_msg_proc:Msg type error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	IGMP_SNP_DEBUG("print command msg as below:\n");
	IGMP_SNP_DEBUG("msg_type:%d;\r\nmsg_code:%d;\r\nmsg_ext:%d;\r\nmsg_len:%d;\r\n",\
																		t_msg->msgtype,\
																		t_msg->msgcode,\
																		t_msg->msgext, \
																		t_msg->msglen);
	switch(t_msg->msgtype)
	{
		case CMD_MSG_TYPE_KERNEL:
			{
			}
			break;
		case CMD_MSG_TYPE_SHOW:
			{
				switch(t_msg->msgcode)
				{
					case CMD_SHOW_IGMPVLANCNT:
						igmp_show_igmpvlan_cnt(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					case CMD_SHOW_GROUPCNT:
						igmp_show_group_cnt(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					case CMD_SHOW_HOSTTIMEOUT:
						igmp_show_host_timerinterval(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					case CMD_SHOW_QUERYTIMEOUT:
						igmp_show_query_timerinterval(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					case CMD_SHOW_RESPTIMEOUT:
						igmp_show_resp_timerinterval(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					case CMD_SHOW_ROBUSTTIMEOUT:
						igmp_show_robust_timerinterval(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					case CMD_SHOW_GROUPLIFETIME:
						igmp_show_grouplife_timerinterval(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					case CMD_SHOW_VLANLIFETIME:
						igmp_show_vlanlife_timerinterval(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					case CMP_SHOW_MCROUTE:
						igmp_show_mcroute_port(t_msg);
						ret = IGMP_SNOOP_DUMP;
						break;
					default:
						IGMP_SNP_DEBUG("igmp_snp_msg_proc:error msg_code.\r\n");
						break;
				}
			}
			break;
		case CMD_MSG_TYPE_NOTIFY:
			{
			}
			break;
		case CMD_MSG_TYPE_CONF:
			{
				switch( t_msg->msgcode )
				{
					case CMD_CONF_ENABLE:
						if(IGMP_SNOOP_OK == igmp_set_enable(IGMP_SNOOP_YES))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_DISABLE:
						if(IGMP_SNOOP_OK == igmp_set_enable(IGMP_SNOOP_NO))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_DEBUG_ON:
						igmp_snoop_debug = 1;
						break;
					case CMD_CONF_DEBUG_OFF:
						igmp_snoop_debug = 0;
						break;
					case CMD_CONF_DEL_SPEC:
						if( IGMP_SNOOP_OK == igmp_del_spec_mcgroup(t_msg))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_DEL_ALL:
						if( IGMP_SNOOP_OK == igmp_del_all_vlan(t_msg))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_SET_QUERYTIME:
						if( IGMP_SNOOP_OK == igmp_set_query_timerinterval(t_msg))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_SET_ROBUSTTIME:
						if( IGMP_SNOOP_OK == igmp_set_robust_timerinterval(t_msg))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_SET_GROUPLIFE:
						if( IGMP_SNOOP_OK == igmp_set_grouplife_timerinterval(t_msg))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_SET_VLANLIFE:
						if( IGMP_SNOOP_OK == igmp_set_vlanlife_timerinterval(t_msg))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_ADD_MCROUTE_PORT:
						if( IGMP_SNOOP_OK == igmp_add_mcroute_port(t_msg))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					case CMD_CONF_DEL_MCROUTE_PORT:
						if( IGMP_SNOOP_OK == igmp_del_mcroute_port(t_msg))
							t_msg->msgext = CMD_EXT_RETURN_SUCCESS;
						else
							t_msg->msgext = CMD_EXT_RETURN_FAILED;
						ret = IGMP_SNOOP_RETURN;
						break;
					default:
						IGMP_SNP_DEBUG("igmp_snp_msg_proc:error msg_code.\r\n");
						break;
				}
			}
			break;
		case CMD_MSG_TYPE_REQUEST:
			{
			}
			break;
		case CMD_MSG_TYPE_ACK:
			{
			}
			break;
		default:
			IGMP_SNP_DEBUG("igmp_snp_msg_proc:Unknown msg type.\r\n");
			break;
	}
	return ret;
}

/**********************************************************************************
*create_msg_thread()
*INPUTS:
*
*OUTPUTS:
*RETURN VALUE:
*	
*DESCRIPTION:
*	IGMP SNOOP command message handle thread
*
***********************************************************************************/
void *create_msg_thread(void)
{
	int ret,write_flag;
	int sock,accept_sock,len;
	int recv_len = 0;
	cmd_msg *msg;
	struct sockaddr_un client;
	char buf[IGMP_MSG_MAX_SIZE];
	fd_set rfds;

	if(0 >(sock = creatservsock_stream(IGMP_SNOOP_MSG_SOCK)))
	{
		IGMP_SNP_DEBUG("create_msg_thread::Create msg socket failed.\r\n");
		return;
	}
	IGMP_SNP_DEBUG("create_msg_thread::Create msg socket ok,MsgSock fd=%d.\r\n",sock);
	memset(&client,0,sizeof(struct sockaddr_un));
	len = sizeof(struct sockaddr_un);

	msg = (cmd_msg *)buf;
	while(1)
	{
		IGMP_SNP_DEBUG("create_msg_thread::enter forever while loop.\r\n");
		if((accept_sock = accept(sock,(struct sockaddr *)&client,&len))<0)
		{
			IGMP_SNP_DEBUG("Accept failed:errno %d [%s].\r\n",errno,strerror(errno));
			close(sock);
			/*需要增加对其他线程的退出信号*/
			return;
		}
		DEBUG_OUT("create_msg_thread:accept command.\r\n");
		while(1)
		{
			memset(buf,0,sizeof(char)*IGMP_MSG_MAX_SIZE);
			FD_ZERO(&rfds);
			FD_SET(accept_sock,&rfds);
			switch(select(accept_sock+1,&rfds,NULL,NULL,NULL))
			{
				case -1:
					break;
				case 0:
					break;
				default:
					if(FD_ISSET(accept_sock,&rfds))
					{
						recv_len = read(accept_sock,buf,IGMP_MSG_MAX_SIZE);
						if( 0 == recv_len )
							break;
						ret = igmp_snp_msg_proc(msg,&write_flag);
						if( (IGMP_SNOOP_DUMP == ret )
							||(IGMP_SNOOP_RETURN == ret) )	/*need send message*/
						{
							write(accept_sock,buf,IGMP_MSG_MAX_SIZE);
						}
						break;
					}
			}
			if(0 == recv_len )
				break;
		}
		close(accept_sock);
		accept_sock = 0;
	}
}



/*read config file for init igmp snooping*/
void read_config(char *line)
{	
	int i;	
	int tmp;		
	IGMP_SNP_DEBUG("read_config...\r\n");
	if(!line)		
		return;			
	for(i=0;i<6;++i)	
	{		
		tmp = strlen(igmp_config_element[i].str);		
		if(!strncmp(line,igmp_config_element[i].str,tmp))		
		{			
			IGMP_SNP_DEBUG("##Match igmp_config:: %s\r\n",igmp_config_element[i].str);
			igmp_config_element[i].func(igmp_config_element+i,(void *)line);			
			break;		
		}	
	}
}

void write_handle(FILE *fd,FILE *tmp_fd)
{
	int tmp = 0,i;
	char buf[CONF_FILE_MAX_ROW];
	
	memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
	while(fgets(buf,CONF_FILE_MAX_ROW-1,fd))
	{
		if('#' == buf[0])
		{
			fputs(buf,tmp_fd);
			memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
			continue;
		}
		for(i=0;i<6;++i)
		{
			if(!strncmp(buf,igmp_config_element[i].str,strlen(igmp_config_element[i].str)))
			{
				break;
			}
		}
		
		switch(i)
		{
			case 0:
				memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
				sprintf(buf,"igmp_snoop_enable	=%d\n",igmp_snoop_enable);
				fputs(buf,tmp_fd);
				break;
			case 1:
				memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
				sprintf(buf,"igmp_snoop_debug	=%d\n",igmp_snoop_debug);
				fputs(buf,tmp_fd);
				break;
			case 2:
				memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
				//sprintf(buf,"cfgdatalen	=%d\n",test_conf.datalen);
				fputs(buf,tmp_fd);
				break;
			case 3:
				memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
				//sprintf(buf,"cfgtime	=%d\n",test_conf.time);
				fputs(buf,tmp_fd);
				break;
			default:
				fputs(buf,tmp_fd);
				memset(buf,0,sizeof(char)*CONF_FILE_MAX_ROW);
				break;
		}
	}
}

void write_config(char *path,void (*handle)(FILE *,FILE *))
{	
	FILE *fd = NULL;	
	FILE *tmp_fd = NULL;	
	char tmp_path[120];	
	struct stat file_stat;		

	if( !path )		
		return;		
	stat(path,&file_stat);	
	if((fd=fopen(path,"rt"))==NULL)	{		
		fprintf(stderr,"Can not open file.errno:%d [%s]\r\n",errno,strerror(errno));		
		return;	
	}		
	memset(tmp_path,0,120);	
	strncpy(tmp_path,path,strlen(path));	
	strcat(tmp_path,"_1");	
	if( (tmp_fd = fopen(tmp_path,"ab+")) == NULL )	{		
		fprintf(stderr,"Open file failed. errno %d [%s].\r\n",errno,strerror(errno));		
		return;	
		}
	if( handle)		
		handle(fd,tmp_fd);			
	fclose(fd);
	remove(path);
	fclose(tmp_fd);
	rename(tmp_path,path);
	chmod(path,file_stat.st_mode);	
	return;
}

/*Blank Func when get it*/
int creat_config_file(char *path)
{
	/*Call Func:void write_config()*/
}

INT igmp_del_spec_mcgroup(cmd_msg *t_msg)
{
	cmd_opt_info *opt = NULL;
	MC_group * t_group = NULL;
	MC_group *t_nextgroup = NULL;
	MC_group_vlan *t_gpvlan = NULL;

	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_del_spec_mcgroup:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	
	if( (NULL == t_msg )
		||((sizeof(cmd_opt_info)+sizeof(cmd_msg) )> t_msg->msglen ))
	{
		IGMP_SNP_DEBUG("igmp_del_spec_mcgroup:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	opt = (cmd_opt_info *)(t_msg + 1);

	if( 0 == opt->groupadd )	/*delete all group in this vlan*/
	{
		igmp_snp_del_mcgroupvlan(opt->vlan_id);
	}
	else
	{
		DEBUG_OUT("igmp_del_spec_mcgroup:vlan_id:0x%x\tgroup:%u.%u.%u.%u\r\n",
				opt->vlan_id,NIPQUAD(opt->groupadd));
		if( IGMP_SNOOP_OK !=igmp_snp_searchvlangroup(opt->vlan_id,opt->groupadd,&t_group,&t_gpvlan))
		{
			IGMP_SNP_DEBUG("igmp_del_spec_mcgroup:searchgroupvlan failed.\r\n");
			return IGMP_SNOOP_ERR;
		}

		if( NULL == t_gpvlan )
		{
			IGMP_SNP_DEBUG("igmp_del_spec_mcgroup:this vlan %d is not exist.\r\n",
							opt->vlan_id);
			return IGMP_SNOOP_ERR;
		}
		
		if( NULL == t_group )
		{
			IGMP_SNP_DEBUG("igmp_del_spec_mcgroup:group:%u.%u.%u.%u. is not exist in vlan %d.\r\n",
							NIPQUAD(opt->groupadd),opt->vlan_id);
			return IGMP_SNOOP_ERR;
		}
		igmp_snp_delgroup(t_gpvlan,t_group,&t_nextgroup);
		if( NULL == t_gpvlan->firstgroup )
		{
			igmp_snp_del_mcgroupvlan(t_gpvlan->vlan_id);
		}
		return IGMP_SNOOP_OK;
	}
}

INT igmp_del_all_vlan(cmd_msg *t_msg )
{
	MC_group_vlan *t_gpvlan = NULL;
	INT next = 0;
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_del_all_vlan:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}

	t_gpvlan = GET_VLAN_POINT_BY_INDEX(first_mcgroupvlan_idx);
	next = first_mcgroupvlan_idx;
	while(t_gpvlan)
	{
		DEBUG_OUT("igmp_del_all_vlan:del vlan_id:%d",t_gpvlan->vlan_id);
		next = t_gpvlan->next;
		igmp_snp_del_mcgroupvlan(t_gpvlan->vlan_id);
		t_gpvlan = GET_VLAN_POINT_BY_INDEX(next);
	}
	first_mcgroupvlan_idx = -1;
	return IGMP_SNOOP_OK;
}
INT igmp_show_igmpvlan_cnt(cmd_msg *t_msg)
{
	LONG	lRet;
	ULONG 	i,ulRet;
	igmp_vlan_list *t_vlan = NULL;

	for(i=0;i<IGMP_GENERAL_GUERY_MAX;i++){
		lRet = igmp_vlanisused(i,&ulRet);
		if(IGMP_SNOOP_OK == lRet && 1 == ulRet) {
			IGMP_SNP_DEBUG("find vlan %ld usedIgmp.\r\n",i);
			igmp_vlancount++;
		}
	}
	t_msg->msgext = igmp_vlancount;
	return IGMP_SNOOP_OK;
}
INT igmp_show_group_cnt(cmd_msg *t_msg)
{
	LONG lRet = IGMP_SNOOP_OK;
	ULONG lVid,ulGroup,ulIfIndex = 0;
	MC_group * pMcGroup = NULL;
	MC_group_vlan *pPrevGroupVlan = NULL;
	ULONG i,ulRet;	
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_show_group_cnt:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_show_group_cnt:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	/*find in mcgroupVlan queue, and find out there exists any group*/
	/*TODO*/
	for(i=0;i<IGMP_GENERAL_GUERY_MAX;i++) {
		lRet = igmp_vlanisused(i,&ulRet);
		if(IGMP_SNOOP_OK == lRet && 1 == ulRet) {
			IGMP_SNP_DEBUG("find vlan %ld usedIgmp.\r\n",i);
			lRet = igmp_snp_searchvlangroup(i, ulGroup, &pMcGroup, &pPrevGroupVlan);
			if ( IGMP_SNOOP_OK != lRet)	{
				IGMP_SNP_DEBUG( "igmp_recv_report: search failed\r\n" );
				return IGMP_SNOOP_ERR;
			}
			if ( NULL != pMcGroup )	{
				IGMP_SNP_DEBUG("find mcGroup one.\r\n");
				igmp_groupcount++;
			}
		}
		else {
			if(0 == i%500) {
			IGMP_SNP_DEBUG("vlan %ld NOT igmpused \r\n",i);}
			continue;
		}
	}
	t_msg->msgext = igmp_groupcount;
	return IGMP_SNOOP_OK;
}

INT igmp_set_vlanlife_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_set_vlanlife_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_set_vlanlife_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	if((10 <= t_msg->msgext )&&(100000>=t_msg->msgext))
		igmp_vlanlife = t_msg->msgext;
	else
		igmp_vlanlife = IGMP_VLAN_LIFE_TIME;
	return IGMP_SNOOP_OK;
}

INT igmp_set_grouplife_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_set_grouplife_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_set_grouplife_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	if((10 <= t_msg->msgext )&&(1000>=t_msg->msgext))
		igmp_grouplife = t_msg->msgext;
	else
		igmp_grouplife = IGMP_GROUP_LIFETIME;
	return IGMP_SNOOP_OK;
}

INT igmp_set_query_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_set_query_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_set_query_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	if((10 <= t_msg->msgext )&&(300>=t_msg->msgext))
		igmp_query_interval = t_msg->msgext;
	else
		igmp_query_interval = IGMP_V2_UNFORCED_QUERY_INTERVAL;
	return IGMP_SNOOP_OK;
}

INT igmp_set_robust_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_set_robust_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_set_robust_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	if((1 <= t_msg->msgext )&&(100>=t_msg->msgext))
		igmp_robust_variable = t_msg->msgext;
	else
		igmp_robust_variable = IGMP_ROBUST_VARIABLE;
	return IGMP_SNOOP_OK;
}

INT igmp_show_host_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_show_host_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_show_host_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	t_msg->msgext = igmp_robust_variable * igmp_query_interval + igmp_resp_interval;
	return IGMP_SNOOP_OK;
}

INT igmp_show_query_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_show_query_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_show_query_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	t_msg->msgext = igmp_query_interval;
	return IGMP_SNOOP_OK;
}

INT igmp_show_resp_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_show_resp_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_show_resp_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	t_msg->msgext = igmp_resp_interval;
	return IGMP_SNOOP_OK;
}

INT igmp_show_robust_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_show_robust_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_show_robust_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	t_msg->msgext = igmp_robust_variable;
	return IGMP_SNOOP_OK;
}

INT igmp_show_grouplife_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_show_grouplife_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_show_grouplife_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	t_msg->msgext = igmp_grouplife;
	return IGMP_SNOOP_OK;
}

INT igmp_show_vlanlife_timerinterval(cmd_msg *t_msg)
{
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_show_vlanlife_timerinterval:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( NULL == t_msg )
	{
		IGMP_SNP_DEBUG("igmp_show_vlanlife_timerinterval:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	t_msg->msgext = igmp_vlanlife;
	return IGMP_SNOOP_OK;
}

INT igmp_add_mcroute_port(cmd_msg *t_msg)
{
	INT	existed = 0;
	INT	ret =IGMP_SNOOP_OK;
	cmd_opt_info *opt = NULL;
	igmp_vlan_list *t_vlanlist = NULL;
	igmp_vlan_list *find_vlanlist = NULL;
	igmp_vlan_port_list	*t_port = NULL;
	
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_add_mcroute_port:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( (NULL == t_msg )
		||((sizeof(cmd_opt_info)+sizeof(cmd_msg) )> t_msg->msglen ))
	{
		IGMP_SNP_DEBUG("igmp_add_mcroute_port:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	opt = (cmd_opt_info *)(t_msg + 1);

	if( ( 0 == opt->vlan_id )||(0 == opt->ifindex ) )
	{
		IGMP_SNP_DEBUG("igmp_add_mcroute_port:data error.vlan_id:%d\tifindex:0x%x\r\n",
				opt->vlan_id,opt->ifindex);
		return IGMP_SNOOP_ERR;
	}
	t_vlanlist = p_vlanlist;
	while(t_vlanlist )
	{
		if( t_vlanlist->vlan_id == opt->vlan_id )
		{
			find_vlanlist = t_vlanlist;
			break;
		}
		t_vlanlist = t_vlanlist->next;
	}
	if( find_vlanlist )	/*find vlan*/
	{
		if( find_vlanlist->first_port )
		{
			t_port = find_vlanlist->first_port;
			while(t_port)
			{
				if(t_port->ifindex == opt->ifindex )
				{
					existed = 1;
					break;
				}
				t_port = t_port->next;
			}
		}
		else
		{
			IGMP_SNP_DEBUG("igmp_add_mcroute_port:no any ports in this vlan %d\r\n",
				opt->vlan_id);
			return IGMP_SNOOP_ERR;
		}
	}
	else
	{
		IGMP_SNP_DEBUG("igmp_add_mcroute_port:vlan %d is not exist.\r\n",
				opt->vlan_id);
		return IGMP_SNOOP_ERR;
	}

	if( existed )/*vlan port both exist.*/
	{
		ret = igmp_add_routeport(opt->vlan_id,opt->ifindex,0);
	}
	return ret;
}

INT igmp_del_mcroute_port(cmd_msg *t_msg)
{
	INT	existed = 0;
	INT	ret =IGMP_SNOOP_OK;
	cmd_opt_info *opt = NULL;
	igmp_vlan_list *t_vlanlist = NULL;
	igmp_vlan_list *find_vlanlist = NULL;
	igmp_vlan_port_list	*t_port = NULL;
	
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_del_mcroute_port:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
	if( (NULL == t_msg )
		||((sizeof(cmd_opt_info)+sizeof(cmd_msg) )> t_msg->msglen ))
	{
		IGMP_SNP_DEBUG("igmp_del_mcroute_port:parameter error.\r\n");
		return IGMP_SNOOP_ERR;
	}
	opt = (cmd_opt_info *)(t_msg + 1);

	if( ( 0 == opt->vlan_id )||(0 == opt->ifindex ) )
	{
		IGMP_SNP_DEBUG("igmp_del_mcroute_port:data error.vlan_id:%d\tifindex:0x%x\r\n",
				opt->vlan_id,opt->ifindex);
		return IGMP_SNOOP_ERR;
	}
	t_vlanlist = p_vlanlist;
	while(t_vlanlist )
	{
		if( t_vlanlist->vlan_id == opt->vlan_id )
		{
			find_vlanlist = t_vlanlist;
			break;
		}
		t_vlanlist = t_vlanlist->next;
	}
	if( find_vlanlist )	/*find vlan*/
	{
		if( find_vlanlist->first_port )
		{
			t_port = find_vlanlist->first_port;
			while(t_port)
			{
				if(t_port->ifindex == opt->ifindex )
				{
					existed = 1;
					break;
				}
				t_port = t_port->next;
			}
		}
		else
		{
			IGMP_SNP_DEBUG("igmp_del_mcroute_port:no any ports in this vlan %d\r\n",
				opt->vlan_id);
			return IGMP_SNOOP_ERR;
		}
	}
	else
	{
		IGMP_SNP_DEBUG("igmp_del_mcroute_port:vlan %d is not exist.\r\n",
				opt->vlan_id);
		return IGMP_SNOOP_ERR;
	}

	if( existed )
	{
		ret = igmp_del_routeport(opt->vlan_id,opt->ifindex,0);
	}
	return ret;
}


INT igmp_show_mcroute_port(cmd_msg *t_msg)
{
	igmp_routerport *t_routerport = NULL;
	cmd_opt_info *opt = NULL;
	
	if( !IGMP_SNP_ISENABLE())
	{
		IGMP_SNP_DEBUG("igmp_del_mcroute_port:igmp snoop is disable.\r\n");
		return IGMP_SNOOP_ERR_NOT_ENABLE_GLB;
	}
}
#endif
#ifdef __cplusplus
}
#endif
