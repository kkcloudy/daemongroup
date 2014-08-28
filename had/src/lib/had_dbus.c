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
* had_dbus.c
*
* CREATOR:
*		zhengcs@autelan.com
*
* DESCRIPTION:
*		APIs used in HAD module for dbus message process.
*
* DATE:
*		06/16/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.16 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <syslog.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "dbus/npd/npd_dbus_def.h"

#include "had_dbus.h"
#include "had_log.h"
#include "had_vrrpd.h"
#include "had_ipaddr.h"
#include "hmd/hmdpub.h"

/*
  *	VRRP DBUS connection 
  *	Mainly used as a connection to response CLI dbus message and
  *	also another purpose to send snmp trap message to SNMPD
  */
DBusConnection *vrrp_cli_dbus_connection = NULL;

/*
  *	VRRP DBUS connection
  *	Mainly used as a message channel to notify other process such as
  *	NPD, portal, wid and so on.
  *	This dbus connection has the purpose of announcing HA state & HA info, which
  *	will be useful to NPD,portal and wid.
  */
DBusConnection *vrrp_notify_dbus_connection = NULL;

unsigned int global_current_instance_no = 0;
char global_current_objpath[VRRP_OBJPATH_LEN] = {0};
char global_cli_dbusname[VRRP_DBUSNAME_LEN] = {0};
char global_notify_dbusname[VRRP_DBUSNAME_LEN] = {0};

extern unsigned int vrrp_log_close;
hansi_s     **g_hansi = NULL;
extern struct state_trace** TRACE_LOG;
extern pthread_mutex_t StateMutex;
extern int global_enable;
extern int service_enable[MAX_HANSI_PROFILE];
extern int global_protal;
#ifndef _VERSION_18SP7_
extern int global_pppoe;
#endif
extern int master_ipaddr_uplink[255];
extern int master_ipaddr_downlink[255];
extern char* global_ht_ifname;
extern int global_ht_ip;
extern int global_ht_state;
extern int global_ht_opposite_ip;
extern int wid_transfer_state[MAX_HANSI_PROFILE];
extern int portal_transfer_state[MAX_HANSI_PROFILE];
#ifndef _VERSION_18SP7_
extern int pppoe_transfer_state[MAX_HANSI_PROFILE];
#endif
extern int uidSock;
extern int global_ms_down_packet_count;
extern int global_multi_link_detect;
extern int sendsock;
/*
 *******************************************************************************
 *had_splice_objpath_string()
 *
 *  DESCRIPTION:
 *		use of common path and vrrp instance profile,
 *		splicing obj-path string of special vrrp instance.
 *  INPUTS:
 * 	 	char *common_objpath,	- vrrp obj common path
 *		unsigned int profile,	- profile of vrrp instance
 *
 *  OUTPUTS:
 * 	 	char *splice_objpath	- special obj-path string 
 *
 *  RETURN VALUE:
 *		void
 *
 *******************************************************************************
 */
void had_splice_objpath_string
(
	char *common_objpath,
	unsigned int profile,
	char *splice_objpath
)
{
	sprintf(splice_objpath,
			"%s%d",
			common_objpath, profile);
	return;
}

/**********************************************************************************
 *  had_profile_create
 *
 *	DESCRIPTION:
 * 		This method create HANSI profile with global HANSI profile id
 *
 *	INPUT:
 *		profileId - global HANSI profile id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *
 **********************************************************************************/
unsigned int had_profile_create
(
	unsigned int profileId,
	int  vrid
)
{

	hansi_s* hansiNode = NULL;

	if(profileId >= MAX_HANSI_PROFILE) {
		vrrp_syslog_error("create hansi profile %d error: profile id out of range\n",profileId);
		return VRRP_RETURN_CODE_PROFILE_OUT_OF_RANGE;
	}
	else if(NULL != (hansiNode = g_hansi[profileId])) {
		vrrp_syslog_error("hansi profile %d exists when new creation!\n",profileId);
		return VRRP_RETURN_CODE_PROFILE_EXIST;
	}
	
	hansiNode = (hansi_s*)malloc(sizeof(hansi_s));
	if(NULL == hansiNode) {
		vrrp_syslog_error("HANSI profile %d creation memory fail!",profileId);
		return VRRP_RETURN_CODE_MALLOC_FAILED ;
	}

	memset(hansiNode,0,sizeof(hansi_s));
	hansiNode->vrid = vrid;
    hansiNode->vlist = NULL;
	hansiNode->func = NULL;

	g_hansi[profileId] = hansiNode;
	return VRRP_RETURN_CODE_OK;
}



/**********************************************************************************
 *  had_profile_destroy
 *
 *	DESCRIPTION:
 * 		This method create mirror profile with global HANSI profile id
 *
 *	INPUT:
 *		profileId - global HANSI profile id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *
 **********************************************************************************/
unsigned int had_profile_destroy
(
	unsigned int profileId
)
{

	hansi_s* hansiNode = NULL;
	vrrp_rt *vsrv = NULL;

	if(profileId >= MAX_HANSI_PROFILE) {
		vrrp_syslog_error("create hansi profile %d error: profile id out of range\n",profileId);
		return VRRP_RETURN_CODE_PROFILE_OUT_OF_RANGE;
	}
	else if(NULL == (hansiNode = g_hansi[profileId])) {
		vrrp_syslog_error("hansi profile %d exists when new creation!\n",profileId);
		return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	if(NULL != hansiNode->func){
        free(hansiNode->func);
		hansiNode->func= NULL;
	}
    if(NULL != hansiNode->vlist){
		vsrv = hansiNode->vlist;
        free(hansiNode->vlist);
		hansiNode->vlist = NULL;
	}

	/* clear real ip of uplink & downlink */
	memset(hansiNode->uplink_real_ip, 0, VRRP_LINK_MAX_CNT * sizeof(vrrp_if_real_ip));
	memset(hansiNode->downlink_real_ip, 0, VRRP_LINK_MAX_CNT * sizeof(vrrp_if_real_ip));

	#if 0
	/* delete by jinpc for buglist AUTELAN-1129 */
	free(hansiNode);
	g_hansi[profileId -1] = NULL;
	#endif
	vrrp_syslog_dbg("g hansi %d free to null!\n",profileId);
	return VRRP_RETURN_CODE_OK;
}

/**********************************************************************************
 * had_get_profile_node
 *
 *	DESCRIPTION:
 * 		this routine get profile node
 *
 *	INPUT:
 *		profile			-- hansi profile
 *		
 *	
 *	OUTPUT:
 *		
 *
 * 	RETURN:
 *		NULL
 *		
 *
 **********************************************************************************/
hansi_s* had_get_profile_node
(
	unsigned int profile
)
{
	hansi_s* hansiNode = NULL;

	if(profile >= MAX_HANSI_PROFILE)
		return NULL;
	
	return (hansiNode = g_hansi[profile]);
	
}

int had_check_global_enable()
{
    int i = 0;
	for (i = 0; i < MAX_HANSI_PROFILE; i++) {
		/* modify jinpc 20091015 
		 * for get global_enable status
		 */
		#if 0
		if(NULL == g_hansi[i]){
		#else
		if (VRRP_SERVICE_DISABLE == service_enable[i]) {
		#endif
           continue;
		}
		else{
           return 1;
		}
	}
	return 0;
}
int had_check_vrrp_configure
(
    int profile
)
{
    hansi_s* hansi = NULL;
	hansi = had_get_profile_node(profile);
	if(NULL == hansi){
       return VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else{
       if(NULL == hansi->vlist){
          return VRRP_RETURN_CODE_ERR;
	   }
	}
	return VRRP_RETURN_CODE_OK;
}
/**************************************************
 * DESCRIPTION:
 *       convert ifname wlanx-y,ebrx-y to wlany,ebry
 **************************************************/
int had_ifname_convert_back(char * ifname)
{
        int i = 0,j = 0;
        int id = 0;
        unsigned int ifnameLen = 0;
        unsigned int preLen = 0;
        char * needConvIfnamePre[NEED_CONVERT_IF_NUM] = NEED_CONVERT_IFNAMES;
        unsigned int convIfPreLens[NEED_CONVERT_IF_NUM] = {0};
        for(i = 0;i < NEED_CONVERT_IF_NUM;i++){
                convIfPreLens[i] = strlen(needConvIfnamePre[i]);
        }
        if(!ifname){
                return VRRP_RETURN_CODE_ERR;
        }
        ifnameLen = strlen(ifname);
        for(i = 0;i < NEED_CONVERT_IF_NUM;i++){
                if(!strncmp(ifname,needConvIfnamePre[i],convIfPreLens[i])){
                        preLen = convIfPreLens[i];
                        break;
                }
        }
        if(preLen){
                for(j = preLen;j < ifnameLen;j++){
                        if('-' == ifname[j]){
                                id = strtoul(ifname+j+1,NULL,NULL);
                                sprintf(ifname+preLen,"%d",id);
                                break;
                        }
                }
        }
        return VRRP_RETURN_CODE_OK;
}
void had_profile_config_save
(
	char* showStr
)
{
	char* pos = showStr;
	unsigned int totalLen = 0;
	int i = 0;
	char showBuf[2048] = {0},*curPos = NULL;
	unsigned int curLen = 0;
	char uplink_ip[17] = {0},downlink_ip[17] = {0},vgateway_ip[17] = {0};
	char uplink_realip[17] = {0},downlink_realip[17]={0};
	char ht_ip[17];
	unsigned int uplink_mask = 0,downlink_mask = 0;
	int j = 0;
	int first_uplink_index = 0;
	int first_downlink_index = 0;
	int ipv6_up_link_flag = 0;
	int ipv6_down_link_flag = 0;
	int ipv6_up_flag = 0;
	int ipv6_down_flag = 0;
	unsigned int uplink_prefix = 0;
	unsigned int downlink_prefix = 0;
	unsigned int uplink_linklocal_prefix = 0;
	unsigned int downlink_linklocal_prefix = 0;
	char ipv6_downlink_addr[INET6_ADDRSTRLEN] = {0};
	char ipv6_uplink_addr[INET6_ADDRSTRLEN] = {0};
	char ipv6_downlink_linklocal_addr[INET6_ADDRSTRLEN] = {0};
	char ipv6_uplink_linklocal_addr[INET6_ADDRSTRLEN] = {0};

		char tempHeartbeatlinkIfname[MAX_IFNAME_LEN] = {0};
        char tempUplinkRealipIfname[MAX_IFNAME_LEN] = {0};
        char tempDownlinkRealipIfname[MAX_IFNAME_LEN] = {0};
        char tempUplinkVifIfname[MAX_IFNAME_LEN] = {0};
        char templ2UplinkVifIfname[MAX_IFNAME_LEN] = {0};
        char tempDownlinkVifIfname[MAX_IFNAME_LEN] = {0};

	if(NULL == showStr)
		return ;

	for (i = 0 ; i < MAX_HANSI_PROFILE; i++ ) {
         hansi_s* hansiNode = g_hansi[i];
		 vrrp_rt* vrrp = NULL;
		 if((NULL == hansiNode)||(NULL ==(vrrp = hansiNode->vlist))){
            continue;
		 }
		 curPos = showBuf;
		 curLen = 0;
		 /*next time to enter,reset enter_node */
		 if(NULL != global_ht_ifname && (0 != global_ht_ip)){
  		 	sprintf(ht_ip,"%d.%d.%d.%d ",((global_ht_ip& 0xff000000) >> 24),((global_ht_ip & 0xff0000) >> 16),	\
						((global_ht_ip & 0xff00) >> 8),(global_ht_ip & 0xff));

			 curLen += sprintf(curPos," config heartbeatlink %s %s\n",global_ht_ifname,ht_ip);
			 curPos = showBuf + curLen;            
		 }

		 j = 0;
		 for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
		 {
		 	 memset(uplink_realip, 0, 17);
 		 	 memset(downlink_realip, 0, 17);
                         strcpy(tempUplinkRealipIfname,hansiNode->uplink_real_ip[j].ifname);
                         strcpy(tempDownlinkRealipIfname,hansiNode->downlink_real_ip[j].ifname);
 //                        had_ifname_convert_back(tempUplinkRealipIfname);
 //                        had_ifname_convert_back(tempDownlinkRealipIfname);
			 if ((VRRP_LINK_SETTED == hansiNode->uplink_real_ip[j].set_flg) &&
				 (VRRP_LINK_SETTED == hansiNode->downlink_real_ip[j].set_flg) &&
			 	 (0 != hansiNode->uplink_real_ip[j].real_ip) &&
				 (0 != hansiNode->downlink_real_ip[j].real_ip))
			 {
				sprintf(uplink_realip, "%d.%d.%d.%d ",
										((hansiNode->uplink_real_ip[j].real_ip & 0xff000000) >> 24),
										((hansiNode->uplink_real_ip[j].real_ip & 0xff0000) >> 16),
										((hansiNode->uplink_real_ip[j].real_ip & 0xff00) >> 8),
										(hansiNode->uplink_real_ip[j].real_ip & 0xff));
				sprintf(downlink_realip, "%d.%d.%d.%d ",
										((hansiNode->downlink_real_ip[j].real_ip & 0xff000000) >> 24),
										((hansiNode->downlink_real_ip[j].real_ip & 0xff0000) >> 16),
										((hansiNode->downlink_real_ip[j].real_ip & 0xff00) >> 8),
										(hansiNode->downlink_real_ip[j].real_ip & 0xff));
                                
                                curLen += sprintf(curPos, " appoint realip uplink %s %s downlink %s %s\n",
											tempUplinkRealipIfname,
											uplink_realip,
											tempDownlinkRealipIfname,
											downlink_realip);
											curPos = showBuf + curLen;

			 }
			 else if((VRRP_LINK_NO_SETTED == hansiNode->uplink_real_ip[j].set_flg) &&
					 (VRRP_LINK_SETTED == hansiNode->downlink_real_ip[j].set_flg) &&
					 (0 != hansiNode->downlink_real_ip[j].real_ip))
			 {
				sprintf(downlink_realip, "%d.%d.%d.%d ",
										((hansiNode->downlink_real_ip[j].real_ip & 0xff000000) >> 24),
										((hansiNode->downlink_real_ip[j].real_ip & 0xff0000) >> 16),
										((hansiNode->downlink_real_ip[j].real_ip & 0xff00) >> 8),
										(hansiNode->downlink_real_ip[j].real_ip & 0xff));
				 
				curLen += sprintf(curPos," appoint realip downlink %s %s\n",
										tempDownlinkRealipIfname,
										downlink_realip);
				curPos = showBuf + curLen;
			 }
			 else if((VRRP_LINK_NO_SETTED == hansiNode->downlink_real_ip[j].set_flg) &&
				 	 (VRRP_LINK_SETTED == hansiNode->uplink_real_ip[j].set_flg) &&
					 (0 != hansiNode->uplink_real_ip[j].real_ip))
			 {
				sprintf(uplink_realip, "%d.%d.%d.%d ",
										((hansiNode->uplink_real_ip[j].real_ip & 0xff000000) >> 24),
										((hansiNode->uplink_real_ip[j].real_ip & 0xff0000) >> 16),
										((hansiNode->uplink_real_ip[j].real_ip & 0xff00) >> 8),
										(hansiNode->uplink_real_ip[j].real_ip & 0xff));
				 
				curLen += sprintf(curPos," appoint realip uplink %s %s\n",
										tempUplinkRealipIfname,
										uplink_realip);
				curPos = showBuf + curLen;
			 }
		 }
 //Change reason: equipment launch configuration is not loaded
/*	 	 
		 if(vrrp->vrid != i){
			  curLen += sprintf(curPos," set vrid %d\n",vrrp->vrid);
			  curPos = showBuf + curLen;             
		 }
*/
		if (VRRP_LINK_NO_SETTED != vrrp->uplink_flag)
		{
			/* get first uplink virtual ip */
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
				if (VRRP_LINK_NO_SETTED != vrrp->uplink_vif[j].set_flg)
				{
					sprintf(uplink_ip,"%d.%d.%d.%d",
									((vrrp->uplink_vaddr[j].addr& 0xff000000) >> 24),
									((vrrp->uplink_vaddr[j].addr & 0xff0000) >> 16),
									((vrrp->uplink_vaddr[j].addr& 0xff00) >> 8),
									(vrrp->uplink_vaddr[j].addr & 0xff));
					uplink_mask = vrrp->uplink_vaddr[j].mask;
					first_uplink_index = j;
					break;
				}
			}
		}
		if (VRRP_LINK_NO_SETTED != vrrp->downlink_flag)
		{
			/* get first downlink virtual ip */
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
				if (VRRP_LINK_NO_SETTED != vrrp->downlink_vif[j].set_flg)
				{
					sprintf(downlink_ip,"%d.%d.%d.%d",
										((vrrp->downlink_vaddr[j].addr& 0xff000000) >> 24),
										((vrrp->downlink_vaddr[j].addr & 0xff0000) >> 16),
										((vrrp->downlink_vaddr[j].addr& 0xff00) >> 8),
										(vrrp->downlink_vaddr[j].addr & 0xff));
					downlink_mask = vrrp->downlink_vaddr[j].mask;
					first_downlink_index = j;
					break;
				}
			}
		}
                strcpy(tempUplinkVifIfname,vrrp->uplink_vif[first_uplink_index].ifname);
                strcpy(tempDownlinkVifIfname,vrrp->downlink_vif[first_downlink_index].ifname);
 //               had_ifname_convert_back(tempUplinkVifIfname);
 //               had_ifname_convert_back(tempDownlinkVifIfname);
                if (VRRP_LINK_NO_SETTED != vrrp->uplink_flag &&
			 VRRP_LINK_NO_SETTED != vrrp->downlink_flag){
		 	 if((32 == uplink_mask)&&(32 == downlink_mask)){
				 curLen += sprintf(curPos," config uplink %s %s downlink %s %s priority %d\n",
				 							tempUplinkVifIfname,
				 							uplink_ip,
				 							tempDownlinkVifIfname,
				 							downlink_ip,
				 							vrrp->priority);
				 curPos = showBuf + curLen;
		 	 }
			 else{
				 curLen += sprintf(curPos," config uplink %s %s/%d downlink %s %s/%d priority %d\n",
				 							tempUplinkVifIfname,
				 							uplink_ip,
				 							uplink_mask,
				 							tempDownlinkVifIfname,
				 							downlink_ip,
				 							downlink_mask,
				 							vrrp->priority);
				 curPos = showBuf + curLen;
		 	 }
                }
		 else if(VRRP_LINK_NO_SETTED == vrrp->uplink_flag){
		 	 if(32 == downlink_mask){
				 curLen += sprintf(curPos," config downlink %s %s priority %d\n",
				 						tempDownlinkVifIfname,
				 						downlink_ip,
				 						vrrp->priority);
				 curPos = showBuf + curLen;
		 	 }
			 else{
				 curLen += sprintf(curPos," config downlink %s %s/%d priority %d\n",
				 						tempDownlinkVifIfname,
				 						downlink_ip,
				 						downlink_mask,
				 						vrrp->priority);
				 curPos = showBuf + curLen;
		 	 }		 	
		 }
		 else if(VRRP_LINK_NO_SETTED == vrrp->downlink_flag){
		 	 if(32 == uplink_mask){
				 curLen += sprintf(curPos," config uplink %s %s priority %d\n",
				 						tempUplinkVifIfname,
				 						uplink_ip,
				 						vrrp->priority);
				 curPos = showBuf + curLen;
		 	 }
			 else{
				 curLen += sprintf(curPos," config uplink %s %s/%d priority %d\n",
				 						tempUplinkVifIfname,
				 						uplink_ip,
				 						uplink_mask,
				 						vrrp->priority);
				 curPos = showBuf + curLen;
		 	 }		 	
		 }

		/* multi-virtual ip for uplink|downlink */
		if ((VRRP_LINK_SETTED == vrrp->uplink_flag) &&
			(1 < vrrp->uplink_naddr))
		{
			/* j = first_uplink_index + 1
			 * because uplink_vaddr[first_uplink_index] is not processed here 
			 * if uplink_vaddr[first_uplink_index] has setted
			 */
			for (j = first_uplink_index + 1; j < VRRP_LINK_MAX_CNT; j++) {
				if (VRRP_LINK_SETTED == vrrp->uplink_vif[j].set_flg)
				{
                                        strcpy(tempUplinkVifIfname,vrrp->uplink_vif[j].ifname);
    //                                    had_ifname_convert_back(tempUplinkVifIfname);
					curLen += sprintf(curPos, " add uplink %s %d.%d.%d.%d/%d\n",
											tempUplinkVifIfname,
											(vrrp->uplink_vaddr[j].addr >> 24) & 0xFF,
											(vrrp->uplink_vaddr[j].addr >> 16) & 0xFF,
											(vrrp->uplink_vaddr[j].addr >> 8) & 0xFF,
											(vrrp->uplink_vaddr[j].addr) & 0xFF,
											vrrp->uplink_vaddr[j].mask);
					curPos = showBuf + curLen;
				}
			}
		}
		if (VRRP_LINK_NO_SETTED != vrrp->l2_uplink_flag)
		{
			/* get first uplink virtual ip */
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
				if (VRRP_LINK_NO_SETTED != vrrp->l2_uplink_vif[j].set_flg)
				{
					strcpy(templ2UplinkVifIfname, vrrp->l2_uplink_vif[j].ifname);
	         //       had_ifname_convert_back(templ2UplinkVifIfname);
					curLen += sprintf(curPos," add l2-uplink %s\n",
					 						templ2UplinkVifIfname);
					curPos = showBuf + curLen;
				}
			}
		}
		if ((VRRP_LINK_SETTED == vrrp->downlink_flag) &&
			(1 < vrrp->downlink_naddr))
		{
			/* j = first_downlink_index + 1
			 * because downlink_vaddr[first_downlink_index] is not processed here 
			 * if downlink_vaddr[first_downlink_index] has setted
			 */
			for (j = first_downlink_index + 1; j < VRRP_LINK_MAX_CNT; j++) {
				if (VRRP_LINK_SETTED == vrrp->downlink_vif[j].set_flg)
				{
                                        strcpy(tempDownlinkVifIfname,vrrp->downlink_vif[j].ifname);
  //                                      had_ifname_convert_back(tempDownlinkVifIfname);
					curLen += sprintf(curPos, " add downlink %s %d.%d.%d.%d/%d\n",
											tempDownlinkVifIfname,
											(vrrp->downlink_vaddr[j].addr >> 24) & 0xFF,
											(vrrp->downlink_vaddr[j].addr >> 16) & 0xFF,
											(vrrp->downlink_vaddr[j].addr >> 8) & 0xFF,
											(vrrp->downlink_vaddr[j].addr) & 0xFF,
											vrrp->downlink_vaddr[j].mask);
					curPos = showBuf + curLen;
				}
			}
		}
		 
         if(0 != vrrp->vgateway_flag){
		 	for(j = 0; j < VRRP_LINK_MAX_CNT; j++) {
                                 vrrp_if  *vgwIf   = &(vrrp->vgateway_vif[j]); 
				 vip_addr *vgwAddr = &(vrrp->vgateway_vaddr[j]);
				 unsigned int vgwIp = vgwAddr->addr;
				 memset(vgateway_ip, 0, sizeof(vgateway_ip));
				 if(VRRP_LINK_SETTED == vrrp->vgateway_vif[j].set_flg) {
                                        char tempVgatewayVifIfname[MAX_IFNAME_LEN] = {0};
                                        strcpy(tempVgatewayVifIfname,vrrp->vgateway_vif[j].ifname);
   //                                     had_ifname_convert_back(tempVgatewayVifIfname);
				        sprintf(vgateway_ip,"%d.%d.%d.%d",((vgwIp & 0xff000000) >> 24), \
				 			((vgwIp & 0xff0000) >> 16),	((vgwIp & 0xff00) >> 8),(vgwIp & 0xff));
					 if(vgwAddr->mask) {
						 curLen += sprintf(curPos," add vgateway %s %s/%d\n",tempVgatewayVifIfname,vgateway_ip, vgwAddr->mask);						
					 }
					 else {
						 curLen += sprintf(curPos," add vgateway %s %s\n",tempVgatewayVifIfname,vgateway_ip);
					 }
				        curPos = showBuf + curLen;   
                                 }
		 	}
	 		if (vrrp->vgateway_tf_flag != VRRP_VGATEWAY_TF_FLG_OFF) {
				curLen += sprintf(curPos, " config vgateway anomaly-check on\n");
				curPos = showBuf + curLen;
			}
		 }		 
#if 1
		/*get virtual ipv6 address*/
		if (VRRP_LINK_NO_SETTED != vrrp->uplink_flag)
		{
			/* get first uplink virtual link local ipv6 addr*/
			vrrp_syslog_dbg(
    			"*************had_profile_config_save: ipv6 link local address "NIP6QUAD_FMT" \n",
    			NIP6QUAD(vrrp->uplink_local_ipv6_vaddr[0].sin6_addr.s6_addr)
    			);
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
	    		if(!ipv6_addr_eq_null(vrrp->uplink_local_ipv6_vaddr[j].sin6_addr.s6_addr))  /*show virtual ipv6 link local address*/
	            {
	    			ipv6_up_link_flag = 1;
	    			inet_ntop(AF_INET6, vrrp->uplink_local_ipv6_vaddr[j].sin6_addr.s6_addr, ipv6_uplink_linklocal_addr, INET6_ADDRSTRLEN); 
	    			uplink_linklocal_prefix= vrrp->uplink_local_ipv6_vaddr[j].mask;
					first_uplink_index = j;
					break;
	    		}
			}
			vrrp_syslog_dbg(
    			"*************had_profile_config_save: ipv6 address  "NIP6QUAD_FMT" \n",
    			NIP6QUAD(vrrp->uplink_ipv6_vaddr[0].sin6_addr.s6_addr)
    			);
			/*check vitual ipv6 addr*/
    		if(!ipv6_addr_eq_null(vrrp->uplink_ipv6_vaddr[j].sin6_addr.s6_addr))  /*show virtual ipv6 link local address*/
            {
    			ipv6_up_flag = 1;
    			inet_ntop(AF_INET6, vrrp->uplink_ipv6_vaddr[j].sin6_addr.s6_addr, ipv6_uplink_addr, INET6_ADDRSTRLEN); 
    			uplink_prefix= vrrp->uplink_ipv6_vaddr[j].mask;
    		}
		}
		if (VRRP_LINK_NO_SETTED != vrrp->downlink_flag)
		{
			/* get first downlink virtual link local ipv6 addr */
			for (j = 0; j < VRRP_LINK_MAX_CNT; j++)
			{
				if(!ipv6_addr_eq_null(vrrp->downlink_local_ipv6_vaddr[j].sin6_addr.s6_addr))  /*show virtual ipv6 link local address*/
	            {
				    ipv6_down_link_flag = 1;
					inet_ntop(AF_INET6, vrrp->downlink_local_ipv6_vaddr[j].sin6_addr.s6_addr, ipv6_downlink_linklocal_addr, INET6_ADDRSTRLEN);
					downlink_linklocal_prefix= vrrp->downlink_local_ipv6_vaddr[j].mask;
					first_downlink_index = j;
					break;
				}
			}
			if(!ipv6_addr_eq_null(vrrp->downlink_ipv6_vaddr[j].sin6_addr.s6_addr))  /*show virtual ipv6 link local address*/
            {
				ipv6_down_flag = 1;
				inet_ntop(AF_INET6, vrrp->downlink_ipv6_vaddr[j].sin6_addr.s6_addr, ipv6_downlink_addr, INET6_ADDRSTRLEN);
			    downlink_prefix= vrrp->downlink_ipv6_vaddr[j].mask;
			}

		}

        strcpy(tempUplinkVifIfname,vrrp->uplink_vif[first_uplink_index].ifname);
        strcpy(tempDownlinkVifIfname,vrrp->downlink_vif[first_downlink_index].ifname);
 //               had_ifname_convert_back(tempUplinkVifIfname);
 //               had_ifname_convert_back(tempDownlinkVifIfname);
        if (VRRP_LINK_NO_SETTED != vrrp->uplink_flag &&
			 VRRP_LINK_NO_SETTED != vrrp->downlink_flag){
			 if(1 == ipv6_up_link_flag)
			 {
			 	 curLen += sprintf(curPos," config uplink link-local %s %s prelen %d\n",
				 						tempUplinkVifIfname,
				 						ipv6_uplink_linklocal_addr,
				 						uplink_linklocal_prefix);
				 curPos = showBuf + curLen;
			 }
			 if(1 == ipv6_down_link_flag)
			 {
			 	 curLen += sprintf(curPos," config downlink link-local %s %s prelen %d\n",
				 						tempDownlinkVifIfname,
				 						ipv6_downlink_linklocal_addr,
				 						downlink_linklocal_prefix);
				 curPos = showBuf + curLen;
			 }

			 if(1 == ipv6_up_flag && 1 == ipv6_down_flag)
			 {
				 curLen += sprintf(curPos," config uplink ipv6 %s %s prelen %d downlink %s %s prelen %d priority %d\n",
				 						tempUplinkVifIfname,
				 						ipv6_uplink_addr,
				 						uplink_prefix,
				 						tempDownlinkVifIfname,
				 						ipv6_downlink_addr,
				 						downlink_prefix,
				 						vrrp->priority);
				 curPos = showBuf + curLen;

			 }
         }
		 else if(VRRP_LINK_NO_SETTED == vrrp->uplink_flag){
			 if(1 == ipv6_down_link_flag)
			 {
			 	 curLen += sprintf(curPos," config downlink link-local %s %s prelen %d\n",
				 						tempDownlinkVifIfname,
				 						ipv6_downlink_linklocal_addr,
				 						downlink_linklocal_prefix);
				 curPos = showBuf + curLen;
			 }
			 if(1 == ipv6_down_flag)
			 {
			 	 curLen += sprintf(curPos," config downlink ipv6 %s %s prelen %d priority %d\n",
				 						tempDownlinkVifIfname,
				 						ipv6_downlink_addr,
				 						downlink_prefix,
				 						vrrp->priority);
				 curPos = showBuf + curLen;
			 }
		 }
		 else if(VRRP_LINK_NO_SETTED == vrrp->downlink_flag){
			 if(1 == ipv6_up_link_flag)
			 {
			 	 curLen += sprintf(curPos," config uplink link-local %s %s prelen %d\n",
				 						tempUplinkVifIfname,
				 						ipv6_uplink_linklocal_addr,
				 						uplink_linklocal_prefix);
				 curPos = showBuf + curLen;
			 }
			 if(1 == ipv6_up_flag)
			 {
			 	 curLen += sprintf(curPos," config uplink ipv6 %s %s prelen %d priority %d\n",
				 						tempUplinkVifIfname,
				 						ipv6_uplink_addr,
				 						uplink_prefix,
				 						vrrp->priority);
				 curPos = showBuf + curLen;
			 }
			 
		 }

		/* multi-virtual ip for uplink|downlink */
		if ((VRRP_LINK_SETTED == vrrp->uplink_flag) &&
			(1 < vrrp->uplink_ipv6_naddr))
		{
			/* j = first_uplink_index + 1
			 * because uplink_vaddr[first_uplink_index] is not processed here 
			 * if uplink_vaddr[first_uplink_index] has setted
			 */
			for (j = first_uplink_index + 1; j < VRRP_LINK_MAX_CNT; j++) {
				if(!ipv6_addr_eq_null(vrrp->uplink_local_ipv6_vaddr[j].sin6_addr.s6_addr))  /*show virtual ipv6 link local address*/
	            {
                    strcpy(tempUplinkVifIfname,vrrp->uplink_vif[j].ifname);
	    			inet_ntop(AF_INET6, vrrp->uplink_local_ipv6_vaddr[j].sin6_addr.s6_addr, 
						ipv6_uplink_linklocal_addr, INET6_ADDRSTRLEN); 
					uplink_linklocal_prefix= vrrp->uplink_local_ipv6_vaddr[j].mask;
					curLen += sprintf(curPos, " add uplink ipv6 link-local %s %s prelen %d\n",
											tempUplinkVifIfname,
											ipv6_uplink_linklocal_addr,
											uplink_linklocal_prefix);
					curPos = showBuf + curLen;
				}
				if(!ipv6_addr_eq_null(vrrp->uplink_ipv6_vaddr[j].sin6_addr.s6_addr))  /*show virtual ipv6 address*/
	            {
                    strcpy(tempUplinkVifIfname,vrrp->uplink_vif[j].ifname);
	    			inet_ntop(AF_INET6, vrrp->uplink_ipv6_vaddr[j].sin6_addr.s6_addr, 
						ipv6_uplink_addr, INET6_ADDRSTRLEN); 
					uplink_prefix= vrrp->uplink_ipv6_vaddr[j].mask;
					curLen += sprintf(curPos, " add uplink ipv6 %s %s prelen %d\n",
											tempUplinkVifIfname,
											ipv6_uplink_addr,
											uplink_prefix);
					curPos = showBuf + curLen;
				}
			}
		}
		if ((VRRP_LINK_SETTED == vrrp->downlink_flag) &&
			(1 < vrrp->downlink_ipv6_naddr))
		{
			/* j = first_downlink_index + 1
			 * because downlink_vaddr[first_downlink_index] is not processed here 
			 * if downlink_vaddr[first_downlink_index] has setted
			 */
			for (j = first_downlink_index + 1; j < VRRP_LINK_MAX_CNT; j++) {
				if(!ipv6_addr_eq_null(vrrp->downlink_local_ipv6_vaddr[j].sin6_addr.s6_addr))  /*show virtual ipv6 link local address*/
	            {
                    strcpy(tempDownlinkVifIfname,vrrp->downlink_vif[j].ifname);
	    			inet_ntop(AF_INET6, vrrp->downlink_local_ipv6_vaddr[j].sin6_addr.s6_addr, 
						ipv6_downlink_linklocal_addr, INET6_ADDRSTRLEN); 
					downlink_linklocal_prefix= vrrp->downlink_local_ipv6_vaddr[j].mask;
					curLen += sprintf(curPos, " add downlink ipv6 link-local %s %s prelen %d\n",
											tempDownlinkVifIfname,
											ipv6_downlink_linklocal_addr,
											downlink_linklocal_prefix);
					curPos = showBuf + curLen;
				}
				if(!ipv6_addr_eq_null(vrrp->downlink_ipv6_vaddr[j].sin6_addr.s6_addr))  /*show virtual ipv6 address*/
	            {
                    strcpy(tempDownlinkVifIfname,vrrp->downlink_vif[j].ifname);
	    			inet_ntop(AF_INET6, vrrp->downlink_ipv6_vaddr[j].sin6_addr.s6_addr, 
						ipv6_downlink_addr, INET6_ADDRSTRLEN); 
					downlink_prefix= vrrp->downlink_ipv6_vaddr[j].mask;
					curLen += sprintf(curPos, " add downlink ipv6 %s %s prelen %d\n",
											tempDownlinkVifIfname,
											ipv6_downlink_addr,
											downlink_prefix);
					curPos = showBuf + curLen;
				}
			}
		}

#endif		 
		 if(vrrp->adver_int != VRRP_ADVER_DFL*VRRP_TIMER_HZ){
			 curLen += sprintf(curPos," config hansi advertime %d\n",vrrp->adver_int/VRRP_TIMER_HZ);
			 curPos = showBuf + curLen;
		 }
		 if(VRRP_DEFAULT_DOWN_PACKET_COUNT != global_ms_down_packet_count){
			 curLen += sprintf(curPos,"  config hansi master down-count  %d\n",  global_ms_down_packet_count);
			 curPos = showBuf + curLen;
		 }
		 if(vrrp->preempt != VRRP_PREEMPT_DFL){
 			 curLen += sprintf(curPos," config hansi preempt %s\n",vrrp->preempt ? "yes" : "no");
			 curPos = showBuf + curLen;            
		 }
 //Change reason: equipment launch configuration is not loaded
		 if(vrrp->vrid != i){
			  curLen += sprintf(curPos," set vrid %d\n",vrrp->vrid);
			  curPos = showBuf + curLen;             
		 }

		 if(vrrp->no_vmac != 1){
 			 curLen += sprintf(curPos," config hansi virtual mac yes\n");
			 curPos = showBuf + curLen;
			 
			 if(vrrp->smart_vmac_enable != VRRP_SMART_VMAC_ENABLE){
				 curLen += sprintf(curPos," config hansi smart vmac disable\n");
				 curPos = showBuf + curLen; 		   
			 }
		 }	
		 if(VRRP_STATE_MAST == vrrp->wantstate){
 			 curLen += sprintf(curPos," config start-state master yes\n");
			 curPos = showBuf + curLen;
		 }	
		 if(global_multi_link_detect){
 			 curLen += sprintf(curPos," config hansi multi-link-detect on\n");
			 curPos = showBuf + curLen;
		 }	
		if (vrrp->notify_flg != VRRP_NOTIFY_OBJ_DFL) {
			/* bit[0] for WID */
			if (!(vrrp->notify_flg & VRRP_NOTIFY_BIT_WID)) {
				curLen += sprintf(curPos, " config hansi notify wireless-control off\n");
				curPos = showBuf + curLen;
			}
			/* bit[1] for PORTAL */
			if (!(vrrp->notify_flg & VRRP_NOTIFY_BIT_PORTAL)) {
				curLen += sprintf(curPos, " config hansi notify easy-access-gateway off\n");
				curPos = showBuf + curLen;
			}
			/* bit[2] for DHCP */
			if (!(vrrp->notify_flg & VRRP_NOTIFY_BIT_DHCP)) {
				curLen += sprintf(curPos, " config hansi notify dhcp-server off\n");
				curPos = showBuf + curLen;
			}
		#ifndef _VERSION_18SP7_
			if(!(vrrp->notify_flg&VRRP_NOTIFY_BIT_PPPOE)){
				curLen += sprintf(curPos, " config hansi notify pppoe off\n");
				curPos = showBuf + curLen;
			}
		#endif /* !_VERSION_18SP7_ */	
		}	
		/* dhcp-failover config */
		if((~0UI != vrrp->failover.localip)&&(~0UI != vrrp->failover.peerip)) {
			curLen += sprintf(curPos, " config hansi dhcp-failover peer %d.%d.%d.%d local %d.%d.%d.%d\n", \
						(vrrp->failover.peerip >> 24)&0xFF, (vrrp->failover.peerip >> 16)&0xFF, \
						(vrrp->failover.peerip >> 8)&0xFF, vrrp->failover.peerip & 0xFF, \
						(vrrp->failover.localip >> 24)&0xFF, (vrrp->failover.localip >> 16)&0xFF, \
						(vrrp->failover.localip >> 8)&0xFF, vrrp->failover.localip & 0xFF);
			curPos = showBuf + curLen;			
		}
		else if(~0UI != vrrp->failover.peerip) {
			curLen += sprintf(curPos, " config hansi dhcp-failover peer %d.%d.%d.%d\n", \
						(vrrp->failover.peerip >> 24)&0xFF, (vrrp->failover.peerip >> 16)&0xFF, \
						(vrrp->failover.peerip >> 8)&0xFF, vrrp->failover.peerip & 0xFF);
			curPos = showBuf + curLen;
		}
		if(VRRP_OFF == vrrp->vrrp_trap_sw){
			curLen += sprintf(curPos, " config vrrp trap switch disable\n");
			curPos = showBuf + curLen;
		}
		if(VRRP_ON == vrrp->uplink_back_down_flag){			
			curLen += sprintf(curPos, " config smart uplink enable\n");
			curPos = showBuf + curLen;
		}
		
		if(VRRP_ON == vrrp->downlink_back_down_flag){			
			curLen += sprintf(curPos, " config smart downlink enable\n");
			curPos = showBuf + curLen;
		}
		
		if(VRRP_ON == vrrp->vgateway_back_down_flag){			
			curLen += sprintf(curPos, " config smart vgateway enable\n");
			curPos = showBuf + curLen;
		}
		
		
		if(VRRP_ON == vrrp->l2_uplink_back_down_flag){
			curLen += sprintf(curPos, " config smart l2-uplink enable\n");
			curPos = showBuf + curLen;
		}
		
		 if(VRRP_SERVICE_DISABLE != service_enable[vrrp->vrid]){
 			 curLen += sprintf(curPos," config service enable\n");
			 curPos = showBuf + curLen;               
		 }
		#if 0
		/* delete by jinpc,
		 * for support mutli-process in vrrp,
		 * and add show running config string of wid
		 */
		totalLen += sprintf(pos,"config hansi-profile %d \n",i+1);
		pos = showStr + totalLen;
		#endif
		totalLen += sprintf(pos,"%s",showBuf);
		pos = showStr + totalLen;
		#if 0
		/* delete by jinpc,
		 * for support mutli-process in vrrp,
		 * and add show running config string of wid
		 */
		totalLen += sprintf(pos," exit\n");
		pos = showStr + totalLen;
		#endif
	}
	return ;
}

DBusMessage *had_dbus_set_hansi_profile(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;		
	unsigned int		ret = VRRP_RETURN_CODE_BASE;
	unsigned int	profile = DEFAULT_HANSI_PROFILE;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID))) 
	{
	    vrrp_syslog_error("set hansi profile:Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("set hansi profile %s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}	

	if(NULL != had_get_profile_node(profile)){//if the node has already exist
	   ret = VRRP_RETURN_CODE_OK;
	}
	else {//else if not exist ,create first!
		ret = had_profile_create(profile,profile);
	}

	dbus_error_init(&err);

	reply = dbus_message_new_method_return(msg);	
	if(NULL == reply){		
		vrrp_syslog_error("vrrp set hansi profile dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&profile);
	return reply;
}


DBusMessage * had_dbus_appoint_real_ip(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	char *uplink_ifname = NULL,*downlink_ifname = NULL,*uplink_ip = NULL,*downlink_ip = NULL;
	char *u_ifname = NULL,*d_ifname = NULL;
	unsigned int	addr[1024] = {0};
	int naddr = 0;
	int i = 0;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
	hansi_s* hansi = NULL;
	int uplink_index = -1;
	int downlink_index = -1;
        char temUplinkIfname[MAX_IFNAME_LEN] = {0},temDownlinkIfname[MAX_IFNAME_LEN] = {0};
	
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_STRING,&u_ifname,
					DBUS_TYPE_STRING,&uplink_ip,
					DBUS_TYPE_STRING,&d_ifname,
					DBUS_TYPE_STRING,&downlink_ip,					
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp appoint real ip %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
	 vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
	   return NULL;
	} 
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,u_ifname,strlen(u_ifname));
	
	downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == downlink_ifname){
	 vrrp_syslog_error("%s %d,downlink_ifname malloc fail.\n",__func__,__LINE__);
	   if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
	   }
	   return NULL;
	} 
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,d_ifname,strlen(d_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
	 vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
	  if(uplink_ifname != NULL){
		  free(uplink_ifname);
		  uplink_ifname = NULL;
	  }
	  if(downlink_ifname != NULL){
		  free(downlink_ifname);
		  downlink_ifname = NULL;
	  }
	   return NULL;
	} 
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(uplink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
		memcpy(uplink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,uplink_ifname %s not exsit.\n",__func__,__LINE__,uplink_ifname);
	}

	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(downlink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
		memcpy(downlink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,downlink_ifname %s not exsit.\n",__func__,__LINE__,downlink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if(NULL == (hansi = had_get_profile_node(profile))){
        ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
    else if(NULL != had_check_if_exist(hansi->vrid)){
        ret = VRRP_RETURN_CODE_PROFILE_EXIST;
	}
	else{
                /*strcpy(temUplinkIfname,uplink_ifname);
                strcpy(temDownlinkIfname,downlink_ifname);
                if((VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temUplinkIfname))||\
                        (VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temDownlinkIfname))){
                        ret = VRRP_RETURN_CODE_BAD_PARAM;
                        goto out;
                }
                uplink_ifname = temUplinkIfname;
                downlink_ifname = temDownlinkIfname;*/
		naddr = had_ipaddr_list( had_ifname_to_idx(uplink_ifname),addr,sizeof(addr)/sizeof(addr[0]) );
        for( i = 0; i < naddr; i++ ){
           if(inet_addr(uplink_ip)== addr[i]){
			  break;
           	}
		}
		if(i == naddr){
			vrrp_syslog_error("not find real ip %d.%d.%d.%d in interface %s\n",
								(inet_addr(uplink_ip) >> 24) & 0xFF,
								(inet_addr(uplink_ip) >> 16) & 0xFF,
								(inet_addr(uplink_ip) >> 8) & 0xFF,
								(inet_addr(uplink_ip)) & 0xFF,
								uplink_ifname);
           ret = VRRP_RETURN_CODE_BAD_PARAM;
		}
		else{
			naddr = had_ipaddr_list( had_ifname_to_idx(downlink_ifname),addr,sizeof(addr)/sizeof(addr[0]) );
	        for( i = 0; i < naddr; i++ ){
	           if(inet_addr(downlink_ip)== addr[i]){
				  break;
	           	}
			} 
			if(i == naddr){
				vrrp_syslog_error("not find real ip %d.%d.%d.%d in interface %s\n",
					(inet_addr(downlink_ip) >> 24) & 0xFF,
					(inet_addr(downlink_ip) >> 16) & 0xFF,
					(inet_addr(downlink_ip) >> 8) & 0xFF,
					(inet_addr(downlink_ip)) & 0xFF,
					downlink_ifname);
                ret = VRRP_RETURN_CODE_BAD_PARAM;
		    }
			else{
				/* [1]step: add uplink real ip */
				ret = had_get_link_realip_index_by_ifname(hansi,
															uplink_ifname,
															VRRP_LINK_TYPE_UPLINK,
															&uplink_index);
				if ((VRRP_RETURN_CODE_IF_EXIST == ret) &&
					(0 <= uplink_index) &&
					(uplink_index < VRRP_LINK_MAX_CNT))
				{	/* exist, set real ip address */
					hansi->uplink_real_ip[uplink_index].real_ip = inet_addr(uplink_ip);
					ret = VRRP_RETURN_CODE_OK;
				}
				else if ((VRRP_RETURN_CODE_IF_NOT_EXIST == ret) &&
						 (0 <= uplink_index) &&
						 (uplink_index < VRRP_LINK_MAX_CNT))
				{	/* not exist, add ifname & real ip to first empty position */
					strncpy(hansi->uplink_real_ip[uplink_index].ifname, uplink_ifname, strlen(uplink_ifname));
					hansi->uplink_real_ip[uplink_index].real_ip = inet_addr(uplink_ip);
					hansi->uplink_real_ip[uplink_index].set_flg = VRRP_LINK_SETTED;
					ret = VRRP_RETURN_CODE_OK;
				}
				else {
					vrrp_syslog_error("get interface %s in %s_real_ip index %d error!\n",
										uplink_ifname,
										VRRP_LINK_TYPE_DESCANT(VRRP_LINK_TYPE_UPLINK),
										uplink_index);
					ret = VRRP_RETURN_CODE_BAD_PARAM;
				}

				/* error, skip to config downlink real ip */
				if (VRRP_RETURN_CODE_BAD_PARAM != ret)
				{	/* [2]step: add downlink real ip */
					ret = had_get_link_realip_index_by_ifname(hansi,
																downlink_ifname,
																VRRP_LINK_TYPE_DOWNLINK,
																&downlink_index);
					if ((VRRP_RETURN_CODE_IF_EXIST == ret) &&
						(0 <= downlink_index) &&
						(downlink_index < VRRP_LINK_MAX_CNT))
					{	/* exist, set real ip address */
						hansi->downlink_real_ip[downlink_index].real_ip = inet_addr(downlink_ip);
						ret = VRRP_RETURN_CODE_OK;
					}
					else if ((VRRP_RETURN_CODE_IF_NOT_EXIST == ret) &&
							 (0 <= downlink_index) &&
							 (downlink_index < VRRP_LINK_MAX_CNT))
					{	/* not exist, add ifname & real ip to first empty position */
						strncpy(hansi->downlink_real_ip[downlink_index].ifname,
																downlink_ifname, strlen(downlink_ifname));
						hansi->downlink_real_ip[downlink_index].real_ip = inet_addr(downlink_ip);
						hansi->downlink_real_ip[downlink_index].set_flg = VRRP_LINK_SETTED;
						ret = VRRP_RETURN_CODE_OK;
					}
					else {
						/* clear set value of uplink in [1]step */
						memset(&(hansi->uplink_real_ip[uplink_index]), 0, sizeof(vrrp_if_real_ip));
						
						vrrp_syslog_error("get interface %s in %s_real_ip index %d error!\n",
											downlink_ifname,
											VRRP_LINK_TYPE_DESCANT(VRRP_LINK_TYPE_DOWNLINK),
											downlink_index);
						ret = VRRP_RETURN_CODE_BAD_PARAM;
					}
				}
			}
		}
	}
out:	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp appoint real ip dbus reply null!\n");
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
		if(downlink_ifname != NULL){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(uplink_ifname != NULL){
		free(uplink_ifname);
		uplink_ifname = NULL;
	}
	if(downlink_ifname != NULL){
		free(downlink_ifname);
		downlink_ifname = NULL;
	}
	return reply;	
}



DBusMessage * had_dbus_appoint_real_ip_downlink(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	char *downlink_ifname = NULL,*downlink_ip = NULL;
	char *d_ifname = NULL;
	unsigned int	addr[1024] = {0};
	int naddr = 0;
	int i = 0;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
	hansi_s *hansi = NULL;
	int downlink_index = -1;
        char temDownlinkIfname[MAX_IFNAME_LEN] = {0};
	
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_STRING,&d_ifname,
					DBUS_TYPE_STRING,&downlink_ip,					
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp appoint downlink real ip %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,d_ifname,strlen(d_ifname));

	
	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
	   return NULL;
	} 
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(downlink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
		memcpy(downlink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,downlink_ifname %s not exsit.\n",__func__,__LINE__,downlink_ifname);
	}
	if(tmp_ifname != NULL){
		free(tmp_ifname);
		tmp_ifname = NULL;
	}
	if(NULL == (hansi = had_get_profile_node(profile))){
        ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if(NULL != had_check_if_exist(profile)){
        ret = VRRP_RETURN_CODE_PROFILE_EXIST;
	}
	else{
		naddr = had_ipaddr_list( had_ifname_to_idx(downlink_ifname),addr,sizeof(addr)/sizeof(addr[0]) );
        for( i = 0; i < naddr; i++ ){
           if(inet_addr(downlink_ip)== addr[i]){
			  break;
           	}
		} 
		if(i == naddr){
			vrrp_syslog_error("not find real ip %d.%d.%d.%d in interface %s\n",
								(inet_addr(downlink_ip) >> 24) & 0xFF,
								(inet_addr(downlink_ip) >> 16) & 0xFF,
								(inet_addr(downlink_ip) >> 8) & 0xFF,
								(inet_addr(downlink_ip)) & 0xFF,
								downlink_ifname);
            ret = VRRP_RETURN_CODE_BAD_PARAM;
	    }
		else{
			/* add downlink real ip */
                        /*strcpy(temDownlinkIfname,downlink_ifname);
                        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temDownlinkIfname)){
                                ret = VRRP_RETURN_CODE_BAD_PARAM;
                                goto out;
                        }
                        downlink_ifname = temDownlinkIfname;*/
			ret = had_get_link_realip_index_by_ifname(hansi,
														downlink_ifname,
														VRRP_LINK_TYPE_DOWNLINK,
														&downlink_index);
			if ((VRRP_RETURN_CODE_IF_EXIST == ret) &&
				(0 <= downlink_index) &&
				(downlink_index < VRRP_LINK_MAX_CNT))
			{	/* exist, set real ip address */
				hansi->downlink_real_ip[downlink_index].real_ip = inet_addr(downlink_ip);
				ret = VRRP_RETURN_CODE_OK;
			}
			else if ((VRRP_RETURN_CODE_IF_NOT_EXIST == ret) &&
					 (0 <= downlink_index) &&
					 (downlink_index < VRRP_LINK_MAX_CNT))
			{	/* not exist, add ifname & real ip to first empty position */
				strncpy(hansi->downlink_real_ip[downlink_index].ifname,
														downlink_ifname, strlen(downlink_ifname));
				hansi->downlink_real_ip[downlink_index].real_ip = inet_addr(downlink_ip);
				hansi->downlink_real_ip[downlink_index].set_flg = VRRP_LINK_SETTED;
				ret = VRRP_RETURN_CODE_OK;
			}
			else {
				vrrp_syslog_error("get interface %s in %s_real_ip index %d error!\n",
									downlink_ifname,
									VRRP_LINK_TYPE_DESCANT(VRRP_LINK_TYPE_DOWNLINK),
									downlink_index);
				ret = VRRP_RETURN_CODE_BAD_PARAM;
			}			
		}
	}
out:
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp appoint downlink real ip dbus reply null!\n");
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(downlink_ifname){
		free(downlink_ifname);
		downlink_ifname = NULL;
	}
	return reply;
}

DBusMessage *had_dbus_appoint_real_ip_uplink
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err;

	unsigned int ret = VRRP_RETURN_CODE_OK;
    unsigned int profile = 0;
	char *uplink_ifname = NULL;
	char *u_ifname = NULL;
	char *uplink_ip = NULL;
	unsigned int addr[1024] = {0};
	int naddr = 0;
	int i = 0;
	hansi_s *hansi = NULL;
	int uplink_index = -1;
        char temUplinkIfname[MAX_IFNAME_LEN] = {0};

    vrrp_syslog_dbg("start appoint real ip to uplink.\n");

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_STRING, &u_ifname,
								DBUS_TYPE_STRING, &uplink_ip,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
		vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	}
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,u_ifname,strlen(u_ifname));
	
	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(uplink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
		memcpy(uplink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,uplink_ifname %s not exsit.\n",__func__,__LINE__,uplink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if (NULL == (hansi = had_get_profile_node(profile)))
	{
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if (NULL != had_check_if_exist(profile))
	{
		ret = VRRP_RETURN_CODE_PROFILE_EXIST;
	}
	else
	{
		naddr = had_ipaddr_list(had_ifname_to_idx(uplink_ifname), addr, sizeof(addr) / sizeof(addr[0]));
		for (i = 0; i < naddr; i++)
		{
			if (inet_addr(uplink_ip) == addr[i])
			{
				break;
			}
		} 
		if (i == naddr) {
			vrrp_syslog_error("not find real ip %d.%d.%d.%d in interface %s\n",
								(inet_addr(uplink_ip) >> 24) & 0xFF,
								(inet_addr(uplink_ip) >> 16) & 0xFF,
								(inet_addr(uplink_ip) >> 8) & 0xFF,
								(inet_addr(uplink_ip)) & 0xFF,
								uplink_ifname);
			ret = VRRP_RETURN_CODE_BAD_PARAM;
		}
		else
		{
			/* add uplink real ip */
                        /*strcpy(temUplinkIfname,uplink_ifname);
                        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temUplinkIfname)){
                                ret = VRRP_RETURN_CODE_BAD_PARAM;
                                goto out;
                        }
                        uplink_ifname = temUplinkIfname;*/
			ret = had_get_link_realip_index_by_ifname(hansi,
														uplink_ifname,
														VRRP_LINK_TYPE_UPLINK,
														&uplink_index);
			if ((VRRP_RETURN_CODE_IF_EXIST == ret) &&
				(0 <= uplink_index) &&
				(uplink_index < VRRP_LINK_MAX_CNT))
			{	/* exist, set real ip address */
				hansi->uplink_real_ip[uplink_index].real_ip = inet_addr(uplink_ip);
				ret = VRRP_RETURN_CODE_OK;
			}
			else if ((VRRP_RETURN_CODE_IF_NOT_EXIST == ret) &&
					 (0 <= uplink_index) &&
					 (uplink_index < VRRP_LINK_MAX_CNT))
			{	/* not exist, add ifname & real ip to first empty position */
				strncpy(hansi->uplink_real_ip[uplink_index].ifname,
														uplink_ifname, strlen(uplink_ifname));
				hansi->uplink_real_ip[uplink_index].real_ip = inet_addr(uplink_ip);
				hansi->uplink_real_ip[uplink_index].set_flg = VRRP_LINK_SETTED;
				ret = VRRP_RETURN_CODE_OK;
			}
			else {
				vrrp_syslog_error("get interface %s in %s_real_ip index %d error!\n",
									uplink_ifname,
									VRRP_LINK_TYPE_DESCANT(VRRP_LINK_TYPE_UPLINK),
									uplink_index);
				ret = VRRP_RETURN_CODE_BAD_PARAM;
			}			
		}
	}
out:
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, &ret);

	if(uplink_ifname != NULL){
		free(uplink_ifname);
		uplink_ifname = NULL;
	}
	return reply;
}

#if 1
DBusMessage *had_dbus_appoint_no_real_ip_downlink
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err;

	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int profile = 0;
	char *downlink_ifname = NULL;
	char *d_ifname = NULL;
	char *downlink_ip = NULL;
	hansi_s *hansi = NULL;
	int downlink_index = -1;
        char temDownlinkIfname[MAX_IFNAME_LEN] = {0};
	
	vrrp_syslog_dbg("start no appoint downlink real ip\n");

	dbus_error_init( &err );
	if (!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_STRING, &d_ifname,
								DBUS_TYPE_STRING, &downlink_ip,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",
								err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == downlink_ifname){
		vrrp_syslog_error("%s %d,downlink_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	}
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,d_ifname,strlen(d_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(downlink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
		memcpy(downlink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,downlink_ifname %s not exsit.\n",__func__,__LINE__,downlink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if (NULL == (hansi = had_get_profile_node(profile)))
	{
        ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else
	{	/* no appoint downlink real ip */
                /*strcpy(temDownlinkIfname,downlink_ifname);
                if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temDownlinkIfname)){
                        ret = VRRP_RETURN_CODE_BAD_PARAM;
                        goto out;
                }
                downlink_ifname = temDownlinkIfname;*/
		ret = had_get_link_realip_index_by_ifname(hansi,
													downlink_ifname,
													VRRP_LINK_TYPE_DOWNLINK,
													&downlink_index);
		if ((VRRP_RETURN_CODE_IF_EXIST == ret) &&
			(0 <= downlink_index) &&
			(downlink_index < VRRP_LINK_MAX_CNT))
		{	/* exist, clear real ip address */
			if (hansi->downlink_real_ip[downlink_index].real_ip == inet_addr(downlink_ip))
			{
				memset(&(hansi->downlink_real_ip[downlink_index]), 0, sizeof(vrrp_if_real_ip));
				ret = VRRP_RETURN_CODE_OK;
			}
		}
		else {
			vrrp_syslog_error("not found interface %s in %s_real_ip index %d error!\n",
								downlink_ifname,
								VRRP_LINK_TYPE_DESCANT(VRRP_LINK_TYPE_DOWNLINK),
								downlink_index);
			ret = VRRP_RETURN_CODE_BAD_PARAM;
		}
	}
out:
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	if(downlink_ifname){
		free(downlink_ifname);
		downlink_ifname = NULL;
	}
	return reply;
}

DBusMessage *had_dbus_appoint_no_real_ip_uplink
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err;

	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int profile = 0;
	char *uplink_ifname = NULL;
	char *u_ifname = NULL;
	char *uplink_ip = NULL;
	hansi_s *hansi = NULL;
	int uplink_index = -1;
        char temUplinkIfname[MAX_IFNAME_LEN] = {0};
	
	vrrp_syslog_dbg("start no appoint uplink real ip\n");

	dbus_error_init( &err );
	if (!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_STRING, &u_ifname,
								DBUS_TYPE_STRING, &uplink_ip,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",
								err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
		vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	}
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,u_ifname,strlen(u_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(uplink_ifname){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(uplink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
		memcpy(uplink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,uplink_ifname %s malloc fail.\n",__func__,__LINE__,uplink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if (NULL == (hansi = had_get_profile_node(profile)))
	{
                ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else
	{	/* no appoint downlink real ip */
                /*strcpy(temUplinkIfname,uplink_ifname);
                if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temUplinkIfname)){
                        ret = VRRP_RETURN_CODE_BAD_PARAM;
                        goto out;
                }
                uplink_ifname = temUplinkIfname;*/
		ret = had_get_link_realip_index_by_ifname(hansi,
							uplink_ifname,
							VRRP_LINK_TYPE_UPLINK,
							&uplink_index);
		if ((VRRP_RETURN_CODE_IF_EXIST == ret) &&
			(0 <= uplink_index) &&
			(uplink_index < VRRP_LINK_MAX_CNT))
		{	/* exist, clear real ip address */
			if (hansi->uplink_real_ip[uplink_index].real_ip == inet_addr(uplink_ip))
			{
				memset(&(hansi->uplink_real_ip[uplink_index]), 0, sizeof(vrrp_if_real_ip));
				ret = VRRP_RETURN_CODE_OK;
			}
		}
		else {
			vrrp_syslog_error("not found interface %s in %s_real_ip index %d error!\n",
								uplink_ifname,
								VRRP_LINK_TYPE_DESCANT(VRRP_LINK_TYPE_DOWNLINK),
								uplink_index);
			ret = VRRP_RETURN_CODE_BAD_PARAM;
		}
	}
out:
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(uplink_ifname){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	if(uplink_ifname){
		free(uplink_ifname);
		uplink_ifname = NULL;
	}
	return reply;
}
#endif

DBusMessage * had_dbus_config_heartbeat_link(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	char *heartbeat_ifname = NULL,*heartbeat_ip = NULL;
	char *h_ifname = NULL;
	unsigned int	addr[1024] = {0};
	int naddr = 0;
	int i = 0;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
        char temHeartbIfname[MAX_IFNAME_LEN] = {0};
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_STRING,&h_ifname,
					DBUS_TYPE_STRING,&heartbeat_ip,					
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp config heartbeat %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	heartbeat_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == heartbeat_ifname){
		vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	}
	memset(heartbeat_ifname,0,MAX_IFNAME_LEN);
	memcpy(heartbeat_ifname,h_ifname,strlen(h_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
	 vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
	 	if(heartbeat_ifname != NULL){
			free(heartbeat_ifname);
			heartbeat_ifname = NULL;
		}
	   return NULL;
	} 
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(heartbeat_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,heartbeat_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,heartbeat_ifname,tmp_ifname);
		memcpy(heartbeat_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,heartbeat_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,heartbeat_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,heartbeat_ifname %s not exsit.\n",__func__,__LINE__,heartbeat_ifname);
	}
	if(tmp_ifname != NULL){
		free(tmp_ifname);
		tmp_ifname = NULL;
	}

    if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
                /*strcpy(temHeartbIfname,heartbeat_ifname);
                if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temHeartbIfname)){
                        ret = VRRP_RETURN_CODE_BAD_PARAM;
                        goto out;
                }
                heartbeat_ifname = temHeartbIfname;*/
				if((-1 == had_ifname_to_idx(heartbeat_ifname))){
						ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
						goto out;
				}
		naddr = had_ipaddr_list( had_ifname_to_idx(heartbeat_ifname),addr,sizeof(addr)/sizeof(addr[0]) );
		for( i = 0; i < naddr; i++ ){
		   if(inet_addr(heartbeat_ip)== addr[i]){
			  break;
			}
		} 
		if(i == naddr){
			ret = VRRP_RETURN_CODE_BAD_PARAM;
		}
		else{
			if(NULL == global_ht_ifname){
				global_ht_ifname = (char *)malloc(sizeof(char)*20);
				if(NULL == global_ht_ifname){
				   ret = VRRP_RETURN_CODE_MALLOC_FAILED;
				}
			}
			if(VRRP_RETURN_CODE_OK == ret){
				memset(global_ht_ifname,0,20);
				memcpy(global_ht_ifname,heartbeat_ifname,strlen(heartbeat_ifname));
				global_ht_ip = inet_addr(heartbeat_ip);
				global_ht_state = vrrp_get_ifname_linkstate(global_ht_ifname);
				vrrp_syslog_event("config heart beat link ifname %s,ip %d.%d.%d.%d\n",global_ht_ifname,
					(global_ht_ip & 0xff000000)>>24,(global_ht_ip & 0xff0000)>>16,(global_ht_ip & 0xff00)>>8,global_ht_ip & 0xff);
				/*update vrrp sock*/
				had_set_heartbeatlink_sock();
			}
		}
	}
out:
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp config heartbeat dbus reply null!\n");
	 	if(heartbeat_ifname != NULL){
			free(heartbeat_ifname);
			heartbeat_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(heartbeat_ifname != NULL){
		free(heartbeat_ifname);
		heartbeat_ifname = NULL;
	}
	return reply;
	
}


DBusMessage * had_dbus_set_vrid(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0,vrid = 0;
    hansi_s* hansi = NULL;
	vrrp_rt* vrrp = NULL;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
    

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
		            DBUS_TYPE_UINT32,&profile,
					DBUS_TYPE_UINT32,&vrid, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp set vrid %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	vrrp_syslog_dbg("change profile %d vrid to %d... \n",profile,vrid);
	
	pthread_mutex_lock(&StateMutex);				
    hansi = had_get_profile_node(profile);
	if(NULL == hansi){
       ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else if(vrid != hansi->vrid){
       hansi->vrid = vrid;
	   if(NULL != (vrrp = hansi->vlist)){	   	
	      had_LIST_UPDATE(vrrp,vrid);
          vrrp->vrid = vrid;
	   }
	}	
	pthread_mutex_unlock(&StateMutex);				
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp set vrid dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;
	
}

DBusMessage * had_dbus_start_ipv6(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0,priority = 0;
	unsigned int link_local = 0;
	char * uplink_ifname = NULL,*downlink_ifname = NULL /*,*uplink_ip = NULL,*downlink_ip = NULL*/;
	char * u_ifname = NULL,*d_ifname = NULL;
	unsigned int  prefix_length_up = 0;
	unsigned int  prefix_length_down = 0;
	struct iaddr ipv6_addr_up;
	struct iaddr ipv6_addr_down;

	unsigned int	ret = VRRP_RETURN_CODE_OK;
	char temUplinkIfname[MAX_IFNAME_LEN] = {0},temDownlinkIfname[MAX_IFNAME_LEN] = {0};
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&priority,  \
					DBUS_TYPE_STRING,&u_ifname,
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[0]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[1]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[2]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[3]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[4]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[5]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[6]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[7]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[8]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[9]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[10]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[11]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[12]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[13]),
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[14]),							
					DBUS_TYPE_BYTE, &(ipv6_addr_up.iabuf[15]),
					DBUS_TYPE_UINT32, &prefix_length_up,
					DBUS_TYPE_STRING,&d_ifname,
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[0]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[1]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[2]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[3]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[4]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[5]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[6]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[7]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[8]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[9]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[10]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[11]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[12]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[13]),
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[14]),							
					DBUS_TYPE_BYTE, &(ipv6_addr_down.iabuf[15]),
					DBUS_TYPE_UINT32, &prefix_length_down,					
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp start %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
/*
	vrrp_syslog_info(
		"get arguments profile = %d priority = %d uplink ifname = %s \
		uplink ipv6 addr = "NIP6QUAD_FMT" \
		prefix_length_up = %d downlink ifname = %s \
		downlink ipv6 addr = "NIP6QUAD_FMT" \
        prefix_length_down = %d \n",
		profile,
		priority,
		u_ifname,
		NIP6QUAD(ipv6_addr_up.iabuf),
		prefix_length_up,
		d_ifname,
		NIP6QUAD(ipv6_addr_down.iabuf),
		prefix_length_down
	);
*/	
    if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
                /*strcpy(temUplinkIfname,uplink_ifname);
                strcpy(temDownlinkIfname,downlink_ifname);
                if((VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temUplinkIfname))||\
                        (VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temDownlinkIfname))){
                        ret = VRRP_RETURN_CODE_BAD_PARAM;
                        goto out;
                }
                uplink_ifname = temUplinkIfname;
                downlink_ifname = temDownlinkIfname;*/
		uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
		if(NULL == uplink_ifname){
		 vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
		   return NULL;
		} 
		memset(uplink_ifname,0,MAX_IFNAME_LEN);
		memcpy(uplink_ifname,u_ifname,strlen(u_ifname));
		
		downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
		if(NULL == downlink_ifname){
		 vrrp_syslog_error("%s %d,downlink_ifname malloc fail.\n",__func__,__LINE__);
		   if(uplink_ifname != NULL){
				free(uplink_ifname);
				uplink_ifname = NULL;
		   }
		   return NULL;
		} 
		memset(downlink_ifname,0,MAX_IFNAME_LEN);
		memcpy(downlink_ifname,d_ifname,strlen(d_ifname));

		char *tmp_ifname = NULL;
		tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
		if(NULL == tmp_ifname){
			vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
			if(uplink_ifname != NULL){
				 free(uplink_ifname);
				 uplink_ifname = NULL;
			}
			if(downlink_ifname != NULL){
				 free(downlink_ifname);
				 downlink_ifname = NULL;
			}
		   return NULL;
		}	
		memset(tmp_ifname,0,MAX_IFNAME_LEN);
		if(!check_ve_interface(uplink_ifname,tmp_ifname)){
			vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
			memcpy(uplink_ifname,tmp_ifname,strlen(tmp_ifname));
			vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
		}else{
			vrrp_syslog_error("%s %d,uplink_ifname %s not exsit.\n",__func__,__LINE__,uplink_ifname);
		}

		memset(tmp_ifname,0,MAX_IFNAME_LEN);
		if(!check_ve_interface(downlink_ifname,tmp_ifname)){
			vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
			memcpy(downlink_ifname,tmp_ifname,strlen(tmp_ifname));
			vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
		}else{
			vrrp_syslog_error("%s %d,downlink_ifname %s not exsit.\n",__func__,__LINE__,downlink_ifname);
		}
		free(tmp_ifname);
		tmp_ifname = NULL;
		
		if((-1 == had_ifname_to_idx(uplink_ifname)) || (-1 == had_ifname_to_idx(downlink_ifname))){
				ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
				goto out;
		}

		pthread_mutex_lock(&StateMutex); 
		ret = had_ipv6_start(profile,priority,link_local,uplink_ifname,&ipv6_addr_up,prefix_length_up,downlink_ifname,&ipv6_addr_down,prefix_length_down);
		pthread_mutex_unlock(&StateMutex); 
	}
out:
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp start dbus reply null!\n");
		if(uplink_ifname != NULL){
			 free(uplink_ifname);
			 uplink_ifname = NULL;
		}
		if(downlink_ifname != NULL){
			 free(downlink_ifname);
			 downlink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(uplink_ifname != NULL){
		 free(uplink_ifname);
		 uplink_ifname = NULL;
	}
	if(downlink_ifname != NULL){
		 free(downlink_ifname);
		 downlink_ifname = NULL;
	}
	return reply;
	
}

DBusMessage * had_dbus_start(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0,priority = 0;
	char * uplink_ifname = NULL,*downlink_ifname = NULL /*,*uplink_ip = NULL,*downlink_ip = NULL*/;
	char * u_ifname = NULL,*d_ifname = NULL;
	unsigned long uplink_ip = 0,downlink_ip  =0;
	unsigned int uplink_mask = 0,downlink_mask = 0;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
        char temUplinkIfname[MAX_IFNAME_LEN] = {0},temDownlinkIfname[MAX_IFNAME_LEN] = {0};
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&priority,  \
					DBUS_TYPE_STRING,&u_ifname,
					DBUS_TYPE_UINT32,&uplink_ip,
					DBUS_TYPE_UINT32,&uplink_mask,
					DBUS_TYPE_STRING,&d_ifname,
					DBUS_TYPE_UINT32,&downlink_ip,
					DBUS_TYPE_UINT32,&downlink_mask,					
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp start %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
    if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
                /*strcpy(temUplinkIfname,uplink_ifname);
                strcpy(temDownlinkIfname,downlink_ifname);
                if((VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temUplinkIfname))||\
                        (VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temDownlinkIfname))){
                        ret = VRRP_RETURN_CODE_BAD_PARAM;
                        goto out;
                }
                uplink_ifname = temUplinkIfname;
                downlink_ifname = temDownlinkIfname;*/
		uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
		if(NULL == uplink_ifname){
		 vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
		   return NULL;
		} 
		memset(uplink_ifname,0,MAX_IFNAME_LEN);
		memcpy(uplink_ifname,u_ifname,strlen(u_ifname));
		
		downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
		if(NULL == downlink_ifname){
		 vrrp_syslog_error("%s %d,downlink_ifname malloc fail.\n",__func__,__LINE__);
		   if(uplink_ifname != NULL){
				free(uplink_ifname);
				uplink_ifname = NULL;
		   }
		   return NULL;
		} 
		memset(downlink_ifname,0,MAX_IFNAME_LEN);
		memcpy(downlink_ifname,d_ifname,strlen(d_ifname));

		char *tmp_ifname = NULL;
		tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
		if(NULL == tmp_ifname){
			vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
			if(uplink_ifname != NULL){
				 free(uplink_ifname);
				 uplink_ifname = NULL;
			}
			if(downlink_ifname != NULL){
				 free(downlink_ifname);
				 downlink_ifname = NULL;
			}
		   return NULL;
		}	
		memset(tmp_ifname,0,MAX_IFNAME_LEN);
		if(!check_ve_interface(uplink_ifname,tmp_ifname)){
			vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
			memcpy(uplink_ifname,tmp_ifname,strlen(tmp_ifname));
			vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
		}else{
			vrrp_syslog_error("%s %d,uplink_ifname %s not exsit.\n",__func__,__LINE__,uplink_ifname);
		}

		memset(tmp_ifname,0,MAX_IFNAME_LEN);
		if(!check_ve_interface(downlink_ifname,tmp_ifname)){
			vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
			memcpy(downlink_ifname,tmp_ifname,strlen(tmp_ifname));
			vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
		}else{
			vrrp_syslog_error("%s %d,downlink_ifname %s not exsit.\n",__func__,__LINE__,downlink_ifname);
		}
		free(tmp_ifname);
		tmp_ifname = NULL;
		
		if((-1 == had_ifname_to_idx(uplink_ifname)) || (-1 == had_ifname_to_idx(downlink_ifname))){
				ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
				goto out;
		}

		pthread_mutex_lock(&StateMutex); 
		ret = had_start(profile,priority,uplink_ifname,uplink_ip,uplink_mask,downlink_ifname,downlink_ip,downlink_mask);
		pthread_mutex_unlock(&StateMutex); 
	}
out:
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp start dbus reply null!\n");
		if(uplink_ifname != NULL){
			 free(uplink_ifname);
			 uplink_ifname = NULL;
		}
		if(downlink_ifname != NULL){
			 free(downlink_ifname);
			 downlink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(uplink_ifname != NULL){
		 free(uplink_ifname);
		 uplink_ifname = NULL;
	}
	if(downlink_ifname != NULL){
		 free(downlink_ifname);
		 downlink_ifname = NULL;
	}
	return reply;
	
}

DBusMessage *had_dbus_start_downlink_link_local
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};

	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int profile = 0;
	unsigned int link_local = 0;
	char *downlink_ifname = NULL;
	char *d_ifname = NULL;
	struct iaddr ipv6_downlink;
	unsigned int prefix_length = 0;

	vrrp_syslog_dbg("start set downlink link local address:\n");

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
					DBUS_TYPE_UINT32, &profile,
					DBUS_TYPE_STRING, &d_ifname,
					DBUS_TYPE_UINT32, &prefix_length,
					DBUS_TYPE_UINT32, &link_local,
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[0]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[1]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[2]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[3]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[4]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[5]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[6]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[7]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[8]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[9]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[10]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[11]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[12]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[13]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[14]),							
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[15]),
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	/*
	vrrp_syslog_info(
		"get arguments profile = %d downlink ifname = %s \
		ipv6 addr = "NIP6QUAD_FMT" \
		prefix_length = %d  link_local = %d\n",
		profile,
		d_ifname,
		NIP6QUAD(ipv6_downlink.iabuf),
		prefix_length,
		link_local
	);
	*/
	downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == downlink_ifname){
	 vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
	   return NULL;
	} 
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,d_ifname,strlen(d_ifname));
	
	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(downlink_ifname != NULL){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(downlink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
		memcpy(downlink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,downlink_ifname %s not exsit.\n",__func__,__LINE__,downlink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
        /*strcpy(temIfname,uplink_ifname);
        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temIfname)){
                ret = VRRP_RETURN_CODE_BAD_PARAM;
                goto out;
        }
        uplink_ifname = temIfname;*/
	if(-1 == had_ifname_to_idx(downlink_ifname)){
			ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
			goto out;
	}

    if( ipv6_downlink.iabuf[0] != 0xfe || ipv6_downlink.iabuf[1] != 0x80 ){
		ret = VRRP_RETURN_CODE_LINKLOCAL_ERROR;
		goto out;
	}
	pthread_mutex_lock(&StateMutex); 
	ret = had_ipv6_start(profile,0,link_local,NULL,0,0,downlink_ifname,&ipv6_downlink,prefix_length);
	pthread_mutex_unlock(&StateMutex); 
	}
out:	
        vrrp_syslog_dbg("config vrrp start ret %x.\n", ret);
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(downlink_ifname != NULL){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	if(downlink_ifname != NULL){
		free(downlink_ifname);
		downlink_ifname = NULL;
	}
	return reply;
}

DBusMessage *had_dbus_start_uplink_link_local
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};

	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int profile = 0;
	unsigned int link_local = 0;
	char *uplink_ifname = NULL;
	char *u_ifname = NULL;
	struct iaddr ipv6_uplink;
	unsigned int prefix_length = 0;

	vrrp_syslog_dbg("start set uplink link local address:\n");

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
					DBUS_TYPE_UINT32, &profile,
					DBUS_TYPE_STRING, &u_ifname,
					DBUS_TYPE_UINT32, &prefix_length,
					DBUS_TYPE_UINT32, &link_local,
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[0]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[1]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[2]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[3]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[4]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[5]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[6]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[7]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[8]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[9]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[10]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[11]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[12]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[13]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[14]),							
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[15]),
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
/*	
	vrrp_syslog_info(
		"get arguments profile = %d downlink ifname = %s \
		ipv6 addr = "NIP6QUAD_FMT" \
		prefix_length = %d  link_local = %d\n",
		profile,
		u_ifname,
		NIP6QUAD(ipv6_uplink.iabuf),
		prefix_length,
		link_local
	);
	*/
	uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
	 vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
	   return NULL;
	} 
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,u_ifname,strlen(u_ifname));
	
	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(uplink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
		memcpy(uplink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,uplink_ifname %s not exsit.\n",__func__,__LINE__,uplink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
        /*strcpy(temIfname,uplink_ifname);
        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temIfname)){
                ret = VRRP_RETURN_CODE_BAD_PARAM;
                goto out;
        }
        uplink_ifname = temIfname;*/
	if(-1 == had_ifname_to_idx(uplink_ifname)){
			ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
			goto out;
	}

	if( ipv6_uplink.iabuf[0] != 0xfe || ipv6_uplink.iabuf[1] != 0x80 ){
		ret = VRRP_RETURN_CODE_LINKLOCAL_ERROR;
		goto out;
	}
	pthread_mutex_lock(&StateMutex); 
	ret = had_ipv6_start(profile,0,link_local,uplink_ifname,&ipv6_uplink,prefix_length,NULL,0,0);
	pthread_mutex_unlock(&StateMutex); 
	}
out:	
        vrrp_syslog_dbg("config vrrp start ret %x.\n", ret);
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	if(uplink_ifname != NULL){
		free(uplink_ifname);
		uplink_ifname = NULL;
	}
	return reply;
}

DBusMessage *had_dbus_start_uplink_ipv6
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};

	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int profile = 0;
	unsigned int priority = 0;
	unsigned int link_local = 0;
	char *uplink_ifname = NULL;
	char *u_ifname = NULL;
	struct iaddr ipv6_uplink;
	unsigned int prefix_length = 0;

	vrrp_syslog_dbg("start vrrp uplink:\n");

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
					DBUS_TYPE_UINT32, &profile,
					DBUS_TYPE_UINT32, &priority,
					DBUS_TYPE_STRING, &u_ifname,
					DBUS_TYPE_UINT32, &prefix_length,
					DBUS_TYPE_UINT32, &link_local,
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[0]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[1]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[2]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[3]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[4]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[5]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[6]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[7]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[8]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[9]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[10]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[11]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[12]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[13]),
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[14]),							
            		DBUS_TYPE_BYTE, &(ipv6_uplink.iabuf[15]),
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

/*
	vrrp_syslog_info(
		"get arguments profile = %d priority = %d downlink ifname = %s \
		ipv6 addr = "NIP6QUAD_FMT" \
		prefix_length = %d  link_local = %d\n",
		profile,
		priority,
		u_ifname,
		NIP6QUAD(ipv6_uplink.iabuf),
		prefix_length,
		link_local
	);
*/	
	uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
	 vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
	   return NULL;
	} 
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,u_ifname,strlen(u_ifname));
	
	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(uplink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
		memcpy(uplink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,uplink_ifname %s not exsit.\n",__func__,__LINE__,uplink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
        /*strcpy(temIfname,uplink_ifname);
        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temIfname)){
                ret = VRRP_RETURN_CODE_BAD_PARAM;
                goto out;
        }
        uplink_ifname = temIfname;*/
	if(-1 == had_ifname_to_idx(uplink_ifname)){
			ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
			goto out;
	}
	pthread_mutex_lock(&StateMutex); 
	ret = had_ipv6_start(profile,priority,link_local,uplink_ifname,&ipv6_uplink,prefix_length,NULL,0,0);
	pthread_mutex_unlock(&StateMutex); 
	}
out:	
        vrrp_syslog_dbg("config vrrp start ret %x.\n", ret);
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	if(uplink_ifname != NULL){
		free(uplink_ifname);
		uplink_ifname = NULL;
	}
	return reply;
}


DBusMessage * had_dbus_start_downlink_ipv6(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0,priority = 0,link_local = 0;
	char *downlink_ifname = NULL;
	char *d_ifname = NULL;
	struct iaddr ipv6_downlink;
    unsigned int prefix_length= 0;
	unsigned int	ret = 0;
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err,
					DBUS_TYPE_UINT32,&profile,
					DBUS_TYPE_UINT32,&priority,
					DBUS_TYPE_STRING,&d_ifname,
					DBUS_TYPE_UINT32,&prefix_length,	
					DBUS_TYPE_UINT32,&link_local,			
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[0]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[1]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[2]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[3]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[4]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[5]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[6]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[7]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[8]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[9]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[10]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[11]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[12]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[13]),
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[14]),							
            		DBUS_TYPE_BYTE, &(ipv6_downlink.iabuf[15]),
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp start downlink %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
/*
	vrrp_syslog_info(
		"get arguments profile = %d priority = %d downlink ifname = %s \
		ipv6 addr = "NIP6QUAD_FMT" \
		prefix_length = %d  link_local = %d\n",
		profile,
		priority,
		d_ifname,
		NIP6QUAD(ipv6_downlink.iabuf),
		prefix_length,
		link_local
	);
*/
	downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == downlink_ifname){
	 vrrp_syslog_error("%s %d,downlink_ifname malloc fail.\n",__func__,__LINE__);
	   return NULL;
	} 
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,d_ifname,strlen(d_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(downlink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
		memcpy(downlink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,downlink_ifname %s not exsit.\n",__func__,__LINE__,downlink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
       /* strcpy(temIfname,downlink_ifname);
        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temIfname)){
                ret = VRRP_RETURN_CODE_BAD_PARAM;
                goto out;
        }
        downlink_ifname = temIfname;*/
		if(-1 == had_ifname_to_idx(downlink_ifname )){
				ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
				goto out;
		}

	pthread_mutex_lock(&StateMutex); 
	ret = had_ipv6_start(profile,priority,link_local,NULL,0,0,downlink_ifname,&ipv6_downlink,prefix_length);
	pthread_mutex_unlock(&StateMutex); 
	}
out:
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp start downlink dbus reply null!\n");
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(downlink_ifname){
		free(downlink_ifname);
		downlink_ifname = NULL;
	}
	return reply;
	
}

DBusMessage *had_dbus_start_uplink
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};

	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int profile = 0;
	unsigned int priority = 0;
	char *uplink_ifname = NULL;
	char *u_ifname = NULL;
	unsigned long uplink_ip = 0;
	unsigned int uplink_mask = 0;
        char temIfname[MAX_IFNAME_LEN] = {0};

	vrrp_syslog_dbg("start vrrp uplink:\n");

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
					DBUS_TYPE_UINT32, &profile,
					DBUS_TYPE_UINT32, &priority,
					DBUS_TYPE_STRING, &u_ifname,
					DBUS_TYPE_UINT32, &uplink_ip,
					DBUS_TYPE_UINT32, &uplink_mask,
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	uplink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == uplink_ifname){
	 vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
	   return NULL;
	} 
	memset(uplink_ifname,0,MAX_IFNAME_LEN);
	memcpy(uplink_ifname,u_ifname,strlen(u_ifname));
	
	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(uplink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
		memcpy(uplink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,uplink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,uplink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,uplink_ifname %s not exsit.\n",__func__,__LINE__,uplink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
        /*strcpy(temIfname,uplink_ifname);
        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temIfname)){
                ret = VRRP_RETURN_CODE_BAD_PARAM;
                goto out;
        }
        uplink_ifname = temIfname;*/
	if(-1 == had_ifname_to_idx(uplink_ifname)){
			ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
			goto out;
	}
	pthread_mutex_lock(&StateMutex); 
	ret = had_start(profile,
					priority,
					uplink_ifname,
					uplink_ip,
					uplink_mask,
					NULL,
					0,
					0);
	pthread_mutex_unlock(&StateMutex); 
	}
out:	
        vrrp_syslog_dbg("config vrrp start ret %x.\n", ret);
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(uplink_ifname != NULL){
			free(uplink_ifname);
			uplink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	if(uplink_ifname != NULL){
		free(uplink_ifname);
		uplink_ifname = NULL;
	}
	return reply;
}


DBusMessage * had_dbus_start_downlink(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0,priority = 0;
	char *downlink_ifname = NULL;
	char *d_ifname = NULL;
	unsigned long downlink_ip = 0;
    unsigned int downlink_mask = 0;
	unsigned int	ret = 0;
        char temIfname[MAX_IFNAME_LEN] = {0};
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&priority,  \
					DBUS_TYPE_STRING,&d_ifname,
					DBUS_TYPE_UINT32,&downlink_ip,	
					DBUS_TYPE_UINT32,&downlink_mask,	
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp start downlink %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	downlink_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == downlink_ifname){
	 vrrp_syslog_error("%s %d,downlink_ifname malloc fail.\n",__func__,__LINE__);
	   return NULL;
	} 
	memset(downlink_ifname,0,MAX_IFNAME_LEN);
	memcpy(downlink_ifname,d_ifname,strlen(d_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(downlink_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
		memcpy(downlink_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,downlink_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,downlink_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,downlink_ifname %s not exsit.\n",__func__,__LINE__,downlink_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
       /* strcpy(temIfname,downlink_ifname);
        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temIfname)){
                ret = VRRP_RETURN_CODE_BAD_PARAM;
                goto out;
        }
        downlink_ifname = temIfname;*/
		if(-1 == had_ifname_to_idx(downlink_ifname )){
				ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
				goto out;
		}

	pthread_mutex_lock(&StateMutex); 
	ret = had_start(profile,priority,NULL,0,0,downlink_ifname,downlink_ip,downlink_mask);
	pthread_mutex_unlock(&StateMutex); 
	}
out:
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp start downlink dbus reply null!\n");
		if(downlink_ifname){
			free(downlink_ifname);
			downlink_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(downlink_ifname){
		free(downlink_ifname);
		downlink_ifname = NULL;
	}
	return reply;
	
}

/*
 *******************************************************************************
 *had_dbus_add_del_link_vip()
 *
 *  DESCRIPTION:
 *		add|delete uplink|downlink virtual ip
 *  INPUTS:
 *		DBUS_TYPE_UINT32 profile,			- vrid
 *		DBUS_TYPE_UINT32 opttype,			- operate type
 *		DBUS_TYPE_UINT32 linktype,			- link type
 *		DBUS_TYPE_STRING ifname,			- uplink|downlink interface name
 *		DBUS_TYPE_UINT32 virtual_ip,		- virtual ip
 *		DBUS_TYPE_UINT32 mask,				- mask
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *
 *******************************************************************************
 */
DBusMessage *had_dbus_add_del_link_vip
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};

	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int profile = 0;
	unsigned int opttype = VRRP_VIP_OPT_TYPE_INVALID;
	unsigned int linktype = VRRP_LINK_TYPE_INVALID;
	char *ifname = NULL;
	char *t_ifname = NULL;
	unsigned long virtual_ip = 0;
	unsigned int mask = 0;
        char vg_ifname[MAX_IFNAME_LEN] = {0};

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
					DBUS_TYPE_UINT32, &profile,
					DBUS_TYPE_UINT32, &opttype,
					DBUS_TYPE_UINT32, &linktype,
					DBUS_TYPE_STRING, &t_ifname,
					DBUS_TYPE_UINT32, &virtual_ip,
					DBUS_TYPE_UINT32, &mask,
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

      /*  strcpy(vg_ifname,ifname);
        if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,vg_ifname)){
                vrrp_syslog_error("bad parameter vgateway ifname %s\n",vg_ifname?vg_ifname:"null");
                ret = VRRP_RETURN_CODE_BAD_PARAM;
                goto out;
        }
        else{
                ifname = vg_ifname;
        }*/
	ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == ifname){
		vrrp_syslog_error("%s %d,uplink_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	} 
	memset(ifname,0,MAX_IFNAME_LEN);
	memcpy(ifname,t_ifname,strlen(t_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(ifname != NULL){
			free(ifname);
			ifname = NULL;
		}
		return NULL;
	}   
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,ifname,tmp_ifname);
		memcpy(ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,ifname %s not exsit.\n",__func__,__LINE__,ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	vrrp_syslog_dbg("%s vrrp %d %s %s vip %d.%d.%d.%d/%d\n",
					VRRP_VIP_OPT_TYPE_DESCANT(opttype),
					profile,
					VRRP_LINK_TYPE_DESCANT(linktype),
					ifname,
					(virtual_ip >> 24) & 0xFF,
					(virtual_ip >> 16) & 0xFF,
					(virtual_ip >> 8) &0xFF,
					(virtual_ip) &0xFF,
					mask);

	if (VRRP_VIP_OPT_TYPE_ADD == opttype) {
		ret = had_add_vip( profile,
							ifname,
							virtual_ip,
							mask,
							linktype);
	}
	else if (VRRP_VIP_OPT_TYPE_DEL == opttype)
	{
		ret = had_delete_vip(  profile,
								ifname,
								virtual_ip,
								mask,
								linktype);
	}
out:
	vrrp_syslog_dbg("%s vrrp %s vip ret %x.\n",
					VRRP_VIP_OPT_TYPE_DESCANT(opttype),
					VRRP_LINK_TYPE_DESCANT(linktype),
					ret);

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(ifname != NULL){
			free(ifname);
			ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	if(ifname != NULL){
		free(ifname);
		ifname = NULL;
	}
	return reply;
}

/*
 *******************************************************************************
 *had_dbus_del_link_vip()
 *
 *  DESCRIPTION:
 *		add|delete uplink|downlink virtual ip
 *  INPUTS:
 *		DBUS_TYPE_UINT32 profile,			- vrid
 *		DBUS_TYPE_UINT32 opttype,			- operate type
 *		DBUS_TYPE_UINT32 linktype,			- link type
 *		DBUS_TYPE_STRING ifname,			- uplink|downlink interface name
 *		DBUS_TYPE_UINT32 virtual_ip,		- virtual ip
 *		DBUS_TYPE_UINT32 mask,				- mask
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *
 *******************************************************************************
 */
DBusMessage *had_dbus_del_link_vip
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError err = {0};

	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int profile = 0;
	unsigned int opttype = VRRP_VIP_OPT_TYPE_INVALID;
	unsigned int linktype = VRRP_LINK_TYPE_INVALID;
	char *ifname = NULL;
	unsigned long virtual_ip = 0;
	unsigned int mask = 0;
        char vg_ifname[MAX_IFNAME_LEN] = {0};

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg, &err,
					DBUS_TYPE_UINT32, &profile,
					DBUS_TYPE_UINT32, &opttype,
					DBUS_TYPE_UINT32, &linktype,
					DBUS_TYPE_STRING, &ifname,
					DBUS_TYPE_UINT32, &virtual_ip,
					DBUS_TYPE_UINT32, &mask,
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	vrrp_syslog_dbg("%s vrrp %d %s %s vip %d.%d.%d.%d/%d\n",
					VRRP_VIP_OPT_TYPE_DESCANT(opttype),
					profile,
					VRRP_LINK_TYPE_DESCANT(linktype),
					ifname,
					(virtual_ip >> 24) & 0xFF,
					(virtual_ip >> 16) & 0xFF,
					(virtual_ip >> 8) &0xFF,
					(virtual_ip) &0xFF,
					mask);

	if (VRRP_VIP_OPT_TYPE_DEL == opttype)
	{
		ret = had_delete_vip(  profile,
								ifname,
								virtual_ip,
								mask,
								linktype);
	}
	vrrp_syslog_dbg("%s vrrp %s vip ret %x.\n",
					VRRP_VIP_OPT_TYPE_DESCANT(opttype),
					VRRP_LINK_TYPE_DESCANT(linktype),
					ret);

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}

DBusMessage * had_dbus_want_master(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	int enable = -1;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
    vrrp_rt* vrrp = NULL;
	hansi_s* hansi = NULL;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&enable,
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg(0,"vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error(0,"vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	pthread_mutex_lock(&StateMutex);
    hansi = had_get_profile_node(profile);
    if(NULL != hansi){
       vrrp = hansi->vlist;	
       if(NULL == vrrp){
          ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	   }
	   else{
	   	  if(enable){
             if(VRRP_SERVICE_ENABLE == service_enable[profile]){
                ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
			 }
			 else{
                vrrp->wantstate = VRRP_STATE_MAST;
			 }
		  }
		  else{
             vrrp->wantstate = VRRP_STATE_INIT;
		  }
	   }
    }
	pthread_mutex_unlock(&StateMutex);
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error(0,"vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;	
}

DBusMessage * had_dbus_service_enable(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	int enable = -1;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
	unsigned int	sendAdv = 0;
    vrrp_rt* vrrp = NULL;
	hansi_s* hansi = NULL;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&enable,  \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp service %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	pthread_mutex_lock(&StateMutex);
    hansi = had_get_profile_node(profile);
    if(NULL != hansi){
       vrrp = hansi->vlist;	
       if(NULL == vrrp){
          ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	   }
	   else{
	   		had_vrrp_vif_rcv_pck_ON(vrrp);
		   if(1 == enable){
			  if(/*(NULL == vrrp->vgateway_vif.ifname) || \*/
			   (NULL == global_ht_ifname)||(0 == global_ht_ip)|| \
				(VRRP_RETURN_CODE_OK != (ret = had_check_vrrp_configure(profile)))){
				  ret = VRRP_RETURN_CODE_PROFILE_NOT_PREPARE;
			  }
			  else{
				  if(!global_enable)
					  global_enable =1;
				  service_enable[profile]=VRRP_SERVICE_ENABLE;
				  hansi_write_heartbeat_ifname_to_file(vrrp->profile, TRUE);/* TRUE : write global_ht_ifname to heartbeat file*/

				  /* open socket fd about heartbeat|uplink|downlink */
				  if (had_open_sock(vrrp))
				  {/*set socket to add to multicast ip group*/
					  ret = VRRP_RETURN_CODE_ERR;
				  }
			  }
		   }
		   else{
		   	  if(VRRP_STATE_INIT != vrrp->state){
			  	  vrrp_syslog_info("vrrp %d state %d service disable, goto disable!\n", vrrp->vrid, vrrp->state);
				  if(VRRP_STATE_TRANSFER == vrrp->state){
					  wid_transfer_state[profile] =0;
					  portal_transfer_state[profile] = 0;
				  }
				  /*if in MASTER state, send advertisement immediately*/				  
				  if(VRRP_STATE_MAST == vrrp->state){
				  	  sendAdv = 1;
				  }
				  else{
				  	  sendAdv = 0;
				  }
			      had_state_goto_disable(vrrp,sendAdv,NULL,NULL);
				  vrrp->state = VRRP_STATE_INIT;	
				  hansi_write_hansi_state_to_file(vrrp->profile, VRRP_STATE_DESCRIPTION(vrrp->state));	

			  }

			  /* here must add close socket code */
			  /* delete sock_fd from sock_list, and close these socket fd. */
			  had_LIST_DEL(vrrp);
			  vrrp_clear_link_fd(vrrp);

			  /* close fd of send vrrp packet*/
			  close(sendsock);
			  
			  service_enable[profile]= VRRP_SERVICE_DISABLE;
			  hansi_write_heartbeat_ifname_to_file(vrrp->profile, FALSE);/* FALSE : write "" to heartbeat file*/

			  #if 1
			  /* add by jinpc 20091015
			   * for modify global_enable status after disable an instance of hansi
			   */
			  global_enable = had_check_global_enable();
			  #endif
		   }
	   }
    }
	pthread_mutex_unlock(&StateMutex);
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp service dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;	
}

DBusMessage * had_dbus_state_change(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	int op = -1;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
	unsigned int	sendAdv = 0;
    vrrp_rt* vrrp = NULL;
	hansi_s* hansi = NULL;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&op,  \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp service %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	pthread_mutex_lock(&StateMutex);
    hansi = had_get_profile_node(profile);
    if(NULL != hansi)
	{
		vrrp = hansi->vlist;	
		if(NULL == vrrp)
		{
        	ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	  	}
	   	else
	   	{
		   	if(HMD_NOTICE_VRRP_STATE_CHANGE_MASTER == op)
		   	{
		   		if(vrrp->state == VRRP_STATE_MAST)
		   		{
					ret = VRRP_RETURN_CODE_OK;
					vrrp_syslog_info("global switch -->vrrp %d state %d already MASTER\n",vrrp->vrid, vrrp->state);
		   		}
				else
				{
					vrrp_syslog_info("global switch -->vrrp %d change state to MASTER\n",vrrp->vrid);
					if(VRRP_LINK_SETTED ==  vrrp->uplink_flag)
					{
					    VRRP_TIMER_SET(vrrp->uplink_ms_down_timer,vrrp->adver_int);
					}
					if(VRRP_LINK_SETTED == vrrp->downlink_flag)
					{
						VRRP_TIMER_SET(vrrp->downlink_ms_down_timer,vrrp->adver_int);
					}
					vrrp_syslog_dbg("set auth type to fail!\n");
					if(VRRP_LINK_SETTED == vrrp->uplink_flag) 
					{
						had_set_link_auth_type(vrrp, VRRP_LINK_TYPE_UPLINK, VRRP_AUTH_FAIL);
					}
					if(VRRP_LINK_SETTED == vrrp->downlink_flag) 
					{
						had_set_link_auth_type(vrrp, VRRP_LINK_TYPE_DOWNLINK, VRRP_AUTH_FAIL);
					}
					 /*log trace*/
					vrrp_state_trace(vrrp->profile,VRRP_STATE_BACK,
					 				"BACK CHANG","send fail-to-receive packet to notify old master,then goto master");
					 
					had_vrrp_vif_rcv_pck_ON(vrrp);
					 /*first notify to old master to force to fail!*/
					vrrp_send_adv( vrrp, vrrp->priority );	
					 /*recover the type value*/
					if(VRRP_LINK_SETTED == vrrp->uplink_flag) 
					{
						had_set_link_auth_type(vrrp, VRRP_LINK_TYPE_UPLINK, VRRP_AUTH_PASS);;
					}
					if(VRRP_LINK_SETTED == vrrp->downlink_flag) 
					{
						had_set_link_auth_type(vrrp, VRRP_LINK_TYPE_DOWNLINK, VRRP_AUTH_PASS);
					}

					had_state_goto_master( vrrp,VRRP_STATE_NONE);
				}
		   	}
		   	else
			{
				vrrp_syslog_info("global switch -->vrrp %d change state to BACK\n",vrrp->vrid);
		   		if((vrrp->state == VRRP_STATE_BACK) || (vrrp->state == VRRP_STATE_DISABLE))
	   			{
					ret = VRRP_RETURN_CODE_OK;
					vrrp_syslog_info("vrrp %d state %d already BACK or DISABLE\n",vrrp->vrid, vrrp->state);
	   			}
				else
				{
					had_state_leave_master(vrrp,1,NULL,NULL);
				}

			}
		}
    }
	pthread_mutex_unlock(&StateMutex);
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp service dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;	
}


/*********************************************
 * DESCRIPTION:
 *   this function drop ifname like wlanx-y,
 *           and convert ifname like wlany,ebry ... 
 *            to wlanx-y,ebrx-y ... , x is had profile num.
 *
 *********************************************/

int had_check_and_convert_ifname(unsigned int profile,char * ifname)
{
        int i = 0;
        unsigned int ifnameLen = 0;
        unsigned int id = 0;
        unsigned int preLen = 0;
        char * needConvIfnamePre[NEED_CONVERT_IF_NUM] = NEED_CONVERT_IFNAMES;
        unsigned int convIfPreLens[NEED_CONVERT_IF_NUM] = {0};
        for(i = 0;i < NEED_CONVERT_IF_NUM;i++){
                convIfPreLens[i] = strlen(needConvIfnamePre[i]);
        }
        if(!ifname){
                return VRRP_RETURN_CODE_ERR;
        }
        ifnameLen = strlen(ifname);     
        for(i = 0;i < NEED_CONVERT_IF_NUM;i++){
                if(!strncmp(ifname,needConvIfnamePre[i],convIfPreLens[i])){
                        preLen = convIfPreLens[i];
                        break;
                }
        }
        if(preLen){
                for(i = preLen;i < ifnameLen;i++){
                        if(('0' > ifname[i])||('9' < ifname[i])){
                                return VRRP_RETURN_CODE_ERR;
                        }
                }
                id = strtoul(ifname+preLen,NULL,NULL);
                sprintf(ifname+preLen,"%d-%d",profile,id);
        }
        return VRRP_RETURN_CODE_OK;
}

DBusMessage * had_dbus_v_gateway(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	char * vgateway_ifname = NULL;
	char * v_ifname = NULL;
        char vg_ifname[MAX_IFNAME_LEN] = {0};
	unsigned long vgateway_ip = 0;
	int vgateway_mask = 0;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_STRING,&v_ifname,
					DBUS_TYPE_UINT32,&vgateway_ip,
					DBUS_TYPE_UINT32,&vgateway_mask,
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	vgateway_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == vgateway_ifname){
		vrrp_syslog_error("%s %d,vgateway_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	} 
	memset(vgateway_ifname,0,MAX_IFNAME_LEN);
	memcpy(vgateway_ifname,v_ifname,strlen(v_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(vgateway_ifname != NULL){
			free(vgateway_ifname);
			vgateway_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(vgateway_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,vgateway_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,vgateway_ifname,tmp_ifname);
		memcpy(vgateway_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,vgateway_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,vgateway_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,vgateway_ifname %s not exsit.\n",__func__,__LINE__,vgateway_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

    if(VRRP_SERVICE_ENABLE == service_enable[profile]){
       ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
               /*  strcpy(vg_ifname,vgateway_ifname);
               if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,vg_ifname)){
                        vrrp_syslog_error("bad parameter vgateway ifname %s\n",vg_ifname?vg_ifname:"null");
                        ret = VRRP_RETURN_CODE_BAD_PARAM;
                }
                else*/{
                        //vgateway_ifname = vg_ifname;     
                        //vrrp_syslog_event("config vgateway ifname %s\n",vg_ifname?vg_ifname:"null");
	                ret = had_v_gateway(profile,vgateway_ifname,vgateway_ip,vgateway_mask);
                }
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(vgateway_ifname != NULL){
			free(vgateway_ifname);
			vgateway_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(vgateway_ifname != NULL){
		free(vgateway_ifname);
		vgateway_ifname = NULL;
	}
	return reply;
	
}


DBusMessage * had_dbus_no_v_gateway(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	char * vgateway_ifname = NULL;
	char * v_ifname = NULL;
	unsigned long vgateway_ip = 0;
	int vgateway_mask = 0;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_STRING,&v_ifname,
					DBUS_TYPE_UINT32,&vgateway_ip,
					DBUS_TYPE_UINT32,&vgateway_mask,
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp vgateway %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	vgateway_ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == vgateway_ifname){
		vrrp_syslog_error("%s %d,vgateway_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	} 
	memset(vgateway_ifname,0,MAX_IFNAME_LEN);
	memcpy(vgateway_ifname,v_ifname,strlen(v_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(vgateway_ifname != NULL){
			free(vgateway_ifname);
			vgateway_ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(vgateway_ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,vgateway_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,vgateway_ifname,tmp_ifname);
		memcpy(vgateway_ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,vgateway_ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,vgateway_ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,vgateway_ifname %s not exsit.\n",__func__,__LINE__,vgateway_ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

    if(VRRP_SERVICE_ENABLE == service_enable[profile]){
        ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
	}
	else{
           /* char temIfname[MAX_IFNAME_LEN] = {0};
            strcpy(temIfname,vgateway_ifname);
            if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temIfname)){
                ret = VRRP_RETURN_CODE_BAD_PARAM;
                goto out;
            }
            vgateway_ifname = temIfname;*/
	    ret = had_no_v_gateway(profile,vgateway_ifname,vgateway_ip,vgateway_mask);
	}
out:
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp vgateway dbus null!\n");
		if(vgateway_ifname != NULL){
			free(vgateway_ifname);
			vgateway_ifname = NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	if(vgateway_ifname != NULL){
		free(vgateway_ifname);
		vgateway_ifname = NULL;
	}
	return reply;
	
}


DBusMessage * had_dbus_no_transfer(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
    vrrp_syslog_dbg("start vrrp... \n");

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp no transfer %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

    wid_transfer_state[profile] =0;
	portal_transfer_state[profile] = 0;
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp no transfer dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);

	return reply;
	
}

DBusMessage * had_dbus_end(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = 0;
	hansi_s* hansiNode = NULL;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err,	\
					DBUS_TYPE_UINT32,&profile,	\
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp end %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	hansiNode = had_get_profile_node(profile);
    if(NULL == hansiNode){
        ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	else{
		pthread_mutex_lock(&StateMutex); 	
		vrrp_syslog_dbg("start to end vrrp!\n");
		ret = had_end(profile);	
	    if(VRRP_RETURN_CODE_OK == ret){
			hansi_write_heartbeat_ifname_to_file(profile, FALSE);/* FALSE : write "" to heartbeat file*/
			hansi_write_hansi_state_to_file(profile,"");	/*write "" to state file*/
			if(NULL == hansiNode->vlist){
			   vrrp_syslog_dbg("free global hansi %d node\n",profile);
			   had_profile_destroy(profile);
			}
			if(VRRP_SERVICE_DISABLE != service_enable[profile]){
	            service_enable[profile] = VRRP_SERVICE_DISABLE;
			}
			global_enable = had_check_global_enable();
			if(!global_enable){
				if(NULL != global_ht_ifname){
		            free(global_ht_ifname);
					global_ht_ifname = NULL;
				}
				global_ht_ip = 0;
				global_ht_opposite_ip = 0;
			}
		}
		pthread_mutex_unlock(&StateMutex);
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp end dbus reply null!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}



DBusMessage * had_dbus_config_profile(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = 0;
	unsigned int priority = 0;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&priority, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

    vrrp_syslog_dbg("start to change vrrp %d priority to %d\n",profile,priority);
	ret = had_priority_cfg(profile,priority);
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}

DBusMessage * had_dbus_config_gvmac(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
        unsigned int profile = 0;
	unsigned int	ret = 0;
	unsigned int g_vmac= 0;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&g_vmac,
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	vrrp_syslog_dbg("config vrrp %d %s mode\n",
					profile,
					(0 == g_vmac)? "no global virtual mac enable" : "global virtual mac enable");

	ret = had_global_vmac_cfg(profile, g_vmac);
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}

DBusMessage * had_dbus_config_state(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = 0;
	unsigned int preempt = 0;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&preempt,
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	vrrp_syslog_dbg("config vrrp %d %s mode\n",
					profile,
					(0 == preempt)? "no preempt" : "preempt");

	ret = had_preempt_cfg(profile,preempt);
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}



DBusMessage * had_dbus_config_advert(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = 0;
	unsigned int advert = 0;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&advert, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	ret = had_advert_cfg(profile,advert);
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}

DBusMessage * had_dbus_config_virtual_mac(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = 0;
	unsigned int mac = 0;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_UINT32,&mac, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	ret = had_mac_cfg(profile,mac);
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}
	
DBusMessage * had_dbus_config_ms_down_count(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;

	unsigned int	ret = VRRP_RETURN_CODE_OK;
	unsigned int count = 0;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&count, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}

	global_ms_down_packet_count = count;
	
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}
DBusMessage * had_dbus_config_multi_link_detect(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
	unsigned int profile = 0;
	unsigned int	ret = VRRP_RETURN_CODE_OK;
	unsigned int detect = 0;
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
		            DBUS_TYPE_UINT32,&profile,
					DBUS_TYPE_UINT32,&detect, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}	
	global_multi_link_detect = detect;
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
	return reply;

	
}

DBusMessage * had_dbus_show(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = 0;
	unsigned int advert = 0;
    vrrp_rt* vrrp = NULL;
	unsigned int uplink_set_flg = VRRP_LINK_NO_SETTED, downlink_set_flg = VRRP_LINK_NO_SETTED;
	unsigned int heartbeat_set_flg = VRRP_LINK_NO_SETTED, vgateway_set_flg = VRRP_LINK_NO_SETTED;
	unsigned int l2_uplink_set_flg = VRRP_LINK_NO_SETTED;
	int i = 0;
	char *ifname = NULL;
	hansi_s* hansiNode = NULL;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
    hansiNode = had_get_profile_node(profile);
	if(!hansiNode){
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		return reply;
	}
	vrrp = had_show_cfg(profile);
	if(NULL == vrrp){
        ret = VRRP_RETURN_CODE_NO_CONFIG;
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		return reply;		   
	}
	else{
		advert = vrrp->adver_int/VRRP_TIMER_HZ;
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->state);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->priority);
	    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&advert);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->preempt);

		if(VRRP_LINK_NO_SETTED == vrrp->uplink_flag){
			uplink_set_flg = VRRP_LINK_NO_SETTED;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&uplink_set_flg);
			#if 0
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&vrrp->downlink_vif.ifname);          
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->downlink_vaddr[0].addr);
			#endif
		}
		else {
			uplink_set_flg = vrrp->uplink_naddr;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&uplink_set_flg);
			/* show first config of uplink */
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vrrp->uplink_vif[i].set_flg)
				{
					ifname = vrrp->uplink_vif[i].ifname;

					dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&ifname);
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(vrrp->uplink_vaddr[i].addr));
					ifname = NULL;
				}
			}
		}

		if(VRRP_LINK_NO_SETTED == vrrp->l2_uplink_flag){
			l2_uplink_set_flg = VRRP_LINK_NO_SETTED;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&l2_uplink_set_flg);
		}
		else {
			l2_uplink_set_flg = vrrp->l2_uplink_naddr;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&l2_uplink_set_flg);
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vrrp->l2_uplink_vif[i].set_flg)
				{
					ifname = vrrp->l2_uplink_vif[i].ifname;

					dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&ifname);
					ifname = NULL;
				}
			}
		}

		if (VRRP_LINK_SETTED == vrrp->downlink_flag) {
			downlink_set_flg = vrrp->downlink_naddr;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&downlink_set_flg);
			/* show first config of downlink */
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if (VRRP_LINK_SETTED == vrrp->downlink_vif[i].set_flg)
				{
					ifname = vrrp->downlink_vif[i].ifname;

					dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&ifname);
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(vrrp->downlink_vaddr[i].addr));    		
					ifname = NULL;
				}
			}
		}
		else {
			downlink_set_flg = VRRP_LINK_NO_SETTED;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&downlink_set_flg);
			#if 0
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&vrrp->uplink_vif.ifname);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->uplink_vaddr[0].addr);
			#endif
		}
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wid_transfer_state[profile]);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&global_protal);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&portal_transfer_state[profile]);

		if(NULL != global_ht_ifname){
			heartbeat_set_flg = VRRP_LINK_SETTED;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&heartbeat_set_flg);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&global_ht_ip);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&global_ht_ifname);
			vrrp_syslog_dbg("transfer global ht ifname %s ip %#x\n",global_ht_ifname, global_ht_ip);

		} 
		else{
			heartbeat_set_flg = VRRP_LINK_NO_SETTED;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&heartbeat_set_flg);
			#if 0
			if (VRRP_LINK_SETTED == vrrp->downlink_flag) {
				dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&vrrp->downlink_vif.ifname);
			}else if (VRRP_LINK_SETTED == vrrp->uplink_flag) {
				dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&vrrp->uplink_vif.ifname);
			}else {
				dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING," ");
			}
			#endif
		}


		if(VRRP_LINK_NO_SETTED == vrrp->vgateway_flag) {
			vgateway_set_flg = VRRP_LINK_NO_SETTED;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vgateway_set_flg);
		}
		else {
			vgateway_set_flg = vrrp->vgateway_naddr;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vgateway_set_flg);
			for(i = 0; i < VRRP_LINK_MAX_CNT; i++) {
				if(VRRP_LINK_SETTED == vrrp->vgateway_vif[i].set_flg) {
					ifname = vrrp->vgateway_vif[i].ifname;

					dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&ifname);
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->vgateway_vaddr[i].addr);
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->vgateway_vaddr[i].mask);				
				}
			}
		}

		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &vrrp->failover.peerip);
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &vrrp->failover.localip);
				
		return reply;	
	}
	
}

DBusMessage * had_dbus_show_detail(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = 0;
	unsigned int advert = 0;
	unsigned int iflog = 0;
    vrrp_rt* vrrp = NULL;
	struct state_trace* log = NULL;
	char *ifname = NULL;
	int i = 0;
	int uplink_cnt = 0, downlink_cnt = 0, vgateway_cnt = 0;
	int l2_uplink_cnt = 0;
	unsigned heartbeat_set_flg = VRRP_LINK_NO_SETTED;
	
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	vrrp_syslog_dbg("show hansi %d detail\n",profile);
	vrrp = had_show_cfg(profile);
	if(NULL == vrrp){
        ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		return reply;		   
	}
	else{
	    if((NULL != TRACE_LOG)&&(NULL !=(log = TRACE_LOG[vrrp->profile]))){
	        iflog = 1;
		}		
		advert = vrrp->adver_int/VRRP_TIMER_HZ;
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->state);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&iflog);
		if(iflog){
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&log->action);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&log->step);
		}
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->priority);
	    dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&advert);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->preempt);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->no_vmac);
		vrrp_syslog_dbg("state %d log %d pri %d adv %d(s) preempt %d vmac %d\n", vrrp->state, iflog, \
						vrrp->priority, advert, vrrp->preempt, vrrp->no_vmac);

		/* uplink interface */
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->uplink_flag);  
		if (VRRP_LINK_NO_SETTED != vrrp->uplink_flag) {
			vrrp_syslog_dbg("%d uplink interfaces\n", vrrp->uplink_naddr);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->uplink_naddr);
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if ((VRRP_LINK_SETTED == vrrp->uplink_vif[i].set_flg) &&
					(uplink_cnt < vrrp->uplink_naddr))
				{
					ifname = vrrp->uplink_vif[i].ifname;
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&ifname);  
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&vrrp->uplink_vif[i].linkstate);  
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->uplink_vif[i].ipaddr);   			
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->uplink_vaddr[i].addr);
					vrrp_syslog_dbg("  %d:%-10s %-4s ip real %#x virtual %#x\n", uplink_cnt, ifname ? ifname:"nil", \
								vrrp->uplink_vif[i].linkstate ? "Up":"Down", vrrp->uplink_vif[i].ipaddr, \
								vrrp->uplink_vaddr[i].addr);
					ifname = NULL;
					uplink_cnt++;
				}
			}
		}

			/* l2-uplink interface */
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->l2_uplink_flag);  
		if (VRRP_LINK_NO_SETTED != vrrp->l2_uplink_flag) {
			vrrp_syslog_dbg("%d l2-uplink interfaces\n", vrrp->l2_uplink_naddr);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->l2_uplink_naddr);
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if ((VRRP_LINK_SETTED == vrrp->l2_uplink_vif[i].set_flg) &&
					(l2_uplink_cnt < vrrp->l2_uplink_naddr))
				{
					ifname = vrrp->l2_uplink_vif[i].ifname;
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&ifname);  
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&vrrp->l2_uplink_vif[i].linkstate); 
					vrrp_syslog_dbg("  %d:%-10s %-4s\n", l2_uplink_cnt, ifname ? ifname:"nil", \
								vrrp->l2_uplink_vif[i].linkstate ? "Up":"Down");
					ifname = NULL;
					l2_uplink_cnt++;
				}
			}
		}

		/* downlink interface */
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->downlink_flag);  
		if (VRRP_LINK_NO_SETTED != vrrp->downlink_flag) {
			vrrp_syslog_dbg("%d downlink interfaces\n", vrrp->downlink_naddr);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->downlink_naddr);
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if ((VRRP_LINK_SETTED == vrrp->downlink_vif[i].set_flg) &&
					(downlink_cnt < vrrp->downlink_naddr))
				{
					ifname = vrrp->downlink_vif[i].ifname;
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&ifname);
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&vrrp->downlink_vif[i].linkstate);  
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->downlink_vif[i].ipaddr); 			
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->downlink_vaddr[i].addr);
					vrrp_syslog_dbg("  %d:%-10s %-4s ip real %#x virtual %#x\n", downlink_cnt, ifname ? ifname :"nil", \
								vrrp->downlink_vif[i].linkstate ? "Up":"Down", vrrp->downlink_vif[i].ipaddr, \
								vrrp->downlink_vaddr[i].addr);
					ifname = NULL;
					downlink_cnt++;
				}
			}
		}
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wid_transfer_state[profile]);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&global_protal);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&portal_transfer_state[profile]);

		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&global_ht_ip);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&global_ht_state);
		if(NULL != global_ht_ifname){
			heartbeat_set_flg = VRRP_LINK_SETTED;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&heartbeat_set_flg);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&global_ht_ifname);
			vrrp_syslog_dbg("wireless state %d portal %-3s state %d heartbeat %s %#x state %d\n", \
						wid_transfer_state[profile], global_protal ? "on":"off",  \
						portal_transfer_state[profile], global_ht_ifname, global_ht_ip, global_ht_state);
		} 
		else{
			heartbeat_set_flg = VRRP_LINK_NO_SETTED;
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&heartbeat_set_flg);
			#if 0
			if (VRRP_LINK_NO_SETTED != vrrp->downlink_flag) {
				dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&vrrp->downlink_vif.ifname);
			}else if (VRRP_LINK_NO_SETTED != vrrp->uplink_flag) {
				dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&vrrp->uplink_vif.ifname);
			}else {
				dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING," ");
			}
			#endif
			vrrp_syslog_dbg("wireless state %d portal %-3s state %d heartbeat not configured %#x state %d\n", \
						wid_transfer_state[profile], global_protal ? "on":"off",  \
						portal_transfer_state[profile], global_ht_ip, global_ht_state);			
		}
		
		/* vgateway interface */
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->vgateway_flag);
		if(VRRP_LINK_NO_SETTED != vrrp->vgateway_flag){
			vrrp_syslog_dbg("%d vgateway interfaces\n", vrrp->vgateway_naddr);
			dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->vgateway_naddr);
			for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
			{
				if ((VRRP_LINK_SETTED == vrrp->vgateway_vif[i].set_flg) &&
					(vgateway_cnt < vrrp->vgateway_naddr))
				{
					ifname = vrrp->vgateway_vif[i].ifname;
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_STRING,&ifname);
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->vgateway_vif[i].ipaddr);
					/*dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->vgateway_vif[i].mask);*/
					
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->vgateway_vaddr[i].addr);	
					
					dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->vgateway_vif[i].linkstate);
					vrrp_syslog_dbg("  %d:%-10s %-4s ip %#x mask %#x\n", vgateway_cnt, ifname ? ifname:"nil", \
								vrrp->vgateway_vif[i].linkstate ? "Up":"Down", vrrp->vgateway_vif[i].ipaddr, \
								vrrp->vgateway_vif[i].mask);
					ifname = NULL;
					vgateway_cnt++;					
				}
			}
		}
		/* dhcp failover config */
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &vrrp->failover.peerip);
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &vrrp->failover.localip);

		return reply;	
	}
	
}


DBusMessage * had_dbus_show_switch_times(DBusConnection *conn, DBusMessage *msg, void *user_data){

	DBusMessage* reply;
	DBusMessageIter iter = {0};
	DBusError		err;
    unsigned int profile = 0;
	unsigned int	ret = 0;
    vrrp_rt* vrrp = NULL;
	hansi_s* hansiNode = NULL;
	
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err, \
					DBUS_TYPE_UINT32,&profile, \
					DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if(dbus_error_is_set( &err ))
		{
			vrrp_syslog_error("vrrp %s raised:%s\n",err.name ,err.message);
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");   
		return reply;
	}
	
	hansiNode = had_get_profile_node(profile);
	if(!hansiNode){
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		return reply;
	}
	vrrp = had_show_cfg(profile);
	if(NULL == vrrp){
        ret = VRRP_RETURN_CODE_NO_CONFIG;
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		return reply;		   
	}
	else{
		
		dbus_message_iter_init_append (reply, &iter);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ret);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&vrrp->backup_switch_times);	
		vrrp_syslog_dbg("show hansi backup_switch_times %d\n", vrrp->backup_switch_times);
		
	}
	
	return reply;	
}
DBusMessage * had_dbus_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	char *strShow = NULL;
	strShow = (char*)malloc(VRRP_SAVE_CFG_MEM);
	if(NULL == strShow) {
		vrrp_syslog_dbg("alloc memory fail when mirror show running-config\n");
		return NULL;
	}
	memset(strShow,0,VRRP_SAVE_CFG_MEM);

	dbus_error_init(&err);

	had_profile_config_save(strShow);

	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &strShow);

	free(strShow);
	strShow = NULL;
	return reply;
}

DBusMessage * had_dbus_get_ifname(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =0 ;
	unsigned int profile = 0;
	hansi_s* hansiNode = NULL;
	vrrp_rt* vrrp = NULL;
	char* ifname1 = NULL,*ifname2 = NULL;
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&profile,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		return NULL;	
	}
	/* here call rstp api */
	if((NULL == (hansiNode = had_get_profile_node(profile)))||(NULL == (vrrp = hansiNode->vlist))){
        ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
        ifname1 = "over";
		ifname2 = "over";
	}
	else{
        ret = VRRP_RETURN_CODE_OK;
		#if 0
		if(VRRP_LINK_NO_SETTED == vrrp->uplink_flag){
			ifname1 = vrrp->downlink_vif.ifname;
		}
		else{
			ifname1 = vrrp->uplink_vif.ifname;
		}

		if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
			ifname2 = vrrp->uplink_vif.ifname;
		}else {
			ifname2 = vrrp->downlink_vif.ifname;
		}
		#endif
	}
	reply = dbus_message_new_method_return(msg);	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_STRING,&ifname1,
							 DBUS_TYPE_STRING,&ifname2,
						     DBUS_TYPE_INVALID);
	vrrp_syslog_dbg("get vrrp ifname: uplink %s,downlink %s\n",ifname1,ifname2);	
	return reply;

}

DBusMessage * had_dbus_set_wid_transfer_state(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;

	int transfer = -1,vrid = 0;
	int profile = 0;

	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
		                        DBUS_TYPE_UINT32,&vrid,
								DBUS_TYPE_UINT32,&transfer,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		
		return NULL;	
	}

	profile = had_get_profile_by_vrid(vrid);
    wid_transfer_state[profile] = transfer;
	vrrp_syslog_dbg( "get dbus message from wid,set transfer state vrrp %d to %d\n",vrid,wid_transfer_state[profile]);
	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,&wid_transfer_state[profile]);
	return reply;

}


DBusMessage * had_dbus_set_portal_transfer_state(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;

	int transfer = -1,vrid = 0;

	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
		                        DBUS_TYPE_UINT32,&vrid,
								DBUS_TYPE_UINT32,&transfer,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		
		return NULL;	
	}
	int profile = 0;
	profile = had_get_profile_by_vrid(vrid);
    portal_transfer_state[profile] = transfer;
	vrrp_syslog_event( "get dbus message from portal,set transfer state to %d\n",portal_transfer_state[profile]);
	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,&portal_transfer_state[profile]);
	return reply;

}
/*add for pppoe*/
#ifndef _VERSION_18SP7_
DBusMessage * had_dbus_set_pppoe_transfer_state(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;

	int pppoe = -1,vrid = 0;

	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
		                        DBUS_TYPE_UINT32,&vrid,
								DBUS_TYPE_UINT32,&pppoe,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		
		return NULL;	
	}
	int profile = 0;
	profile = had_get_profile_by_vrid(vrid);
    pppoe_transfer_state[profile] = pppoe;
	vrrp_syslog_event( "get dbus message from pppoe,set transfer state to %d\n",pppoe_transfer_state[profile]);
	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,&pppoe_transfer_state[profile]);
	return reply;

}
#endif

DBusMessage * vrrp_dbus_snmp_get_vrrp_state(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	int profile = 0;
	hansi_s * hanode = NULL;
	vrrp_rt* vrrp = NULL;
	int state = 0;
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
		                        DBUS_TYPE_UINT32,&profile,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		return NULL;	
	}
	vrrp_syslog_event( "get dbus message from snmp,send had state to it\n");
	
	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	hanode = had_get_profile_node(profile);
	if((NULL != hanode)&&(NULL != ( vrrp = hanode->vlist))){
		state = vrrp->state;
		dbus_message_iter_init_append (reply, &iter);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,&profile);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,&state);
	}
	else {
		profile = 0;
		state = 0;
		dbus_message_iter_init_append (reply, &iter);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,&profile);
		
		dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,&state);
	}
	
	return reply;

}


DBusMessage *had_dbus_set_protal
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1, iter_array_vgw;
	
	int protal = 0;
	vrrp_rt* vrrp = NULL;
	int vrrp_count = 0,i = 0,j = 1;	
	int master_uplinkip = 0;
	int master_downlinkip = 0;
	int back_uplinkip = 0;
	int back_downlinkip = 0;
	int virtual_uplink_ip = 0;
	int virtual_downlink_ip = 0;
	char *uplink_ifname = NULL;
	char *downlink_ifname = NULL;
	char *heartbeatlink_ifname = NULL;
	char *vgateway_ifname = NULL;
	int uplink_cnt = 0, downlink_cnt = 0, vgateway_cnt = 0;
	int vgateway_ip = 0;
	int ret_uplink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int ret_downlink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int uplink_first_index = -1;
	int downlink_first_index = -1;

/*
DBUS_TYPE_UINT32, &vrrp_count); 		// count of vrrp instance
	DBUS_TYPE_UINT32					// vrid
	DBUS_TYPE_UINT32					// state
	DBUS_TYPE_UINT32					// count of uplink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master uplnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back uplink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual uplink ip address
		DBUS_TYPE_STRING_AS_STRING		// uplink interface name
	DBUS_TYPE_UINT32					// count of downlink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master downlnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back downlink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual downlink ip address
		DBUS_TYPE_STRING_AS_STRING		// downlink interface name
	DBUS_TYPE_STRING					// heartbeat interface name
	DBUS_TYPE_UINT32					// heartbeat ip address
	DBUS_TYPE_UINT32					// opposite heartbeat ip address
*/

	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &protal,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		return NULL;
	}
	
    global_protal = protal;
	vrrp_syslog_dbg("portal = %d\n", protal);

    vrrp_count = had_get_runing_hansi();
	vrrp_syslog_dbg("%d vrrp is running!\n", vrrp_count);

	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &vrrp_count);
	if (1 == global_protal)
	{
		for (j = 0; j < MAX_HANSI_PROFILE; j++)
		{
			vrrp = had_check_if_exist(j);
			if (NULL == vrrp) {
				continue;
			}
			vrrp_syslog_dbg("start transfer vrrp %d message!\n", j);

			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &(vrrp->vrid));
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &(vrrp->state));
			vrrp_syslog_event("vrid %d,state %d\n",
								vrrp->vrid, vrrp->state);
			
			uplink_cnt = vrrp->uplink_naddr;
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &uplink_cnt);
			vrrp_syslog_event("uplink count %d\n",
								uplink_cnt);
		
			dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array);

			/* get uplink first setted index */
			ret_uplink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_UPLINK, &uplink_first_index);
			/* get downlink first setted index */
			ret_downlink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_DOWNLINK, &downlink_first_index);

			if (VRRP_LINK_SETTED == vrrp->uplink_flag)
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_NO_SETTED == vrrp->uplink_vif[i].set_flg)
					{
						continue;
					}
					
					/* init */
					uplink_ifname = NULL;
					virtual_uplink_ip = 0;
					master_uplinkip = 0;
					back_uplinkip = 0;
					
					uplink_ifname = vrrp->uplink_vif[i].ifname;
					virtual_uplink_ip = vrrp->uplink_vaddr[i].addr;
					
					switch (vrrp->state)
					{
						case VRRP_STATE_MAST:
						{
							master_uplinkip = vrrp->uplink_vif[i].ipaddr;
							back_uplinkip = 0;
							break;
						}
						case VRRP_STATE_TRANSFER :
						case VRRP_STATE_BACK :
						{
							master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
							back_uplinkip = vrrp->uplink_vif[i].ipaddr;
							break;
						}
						case VRRP_STATE_DISABLE :
						{
							master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
							back_uplinkip = 0;
							break;
						}
					}
		
					vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
										uplink_ifname,
										master_uplinkip,
										back_uplinkip,
										virtual_uplink_ip);
					DBusMessageIter iter_struct;
					dbus_message_iter_open_container(&iter_array,
													DBUS_TYPE_STRUCT,
													NULL,
													&iter_struct);
					dbus_message_iter_append_basic(&iter_struct,
								  DBUS_TYPE_UINT32, &master_uplinkip);
					dbus_message_iter_append_basic(&iter_struct,
								  DBUS_TYPE_UINT32, &back_uplinkip);
					dbus_message_iter_append_basic(&iter_struct,
								  DBUS_TYPE_UINT32, &virtual_uplink_ip);
					dbus_message_iter_append_basic(&iter_struct,
								  DBUS_TYPE_STRING, &uplink_ifname);
		
					dbus_message_iter_close_container (&iter_array, &iter_struct);
				}
			}
			dbus_message_iter_close_container(&iter, &iter_array);
			
			downlink_cnt = vrrp->downlink_naddr;
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &downlink_cnt);
			vrrp_syslog_event("downlink count %d\n",
								downlink_cnt);
			
			dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array1);
			
			/* downlink */
			if (VRRP_LINK_SETTED == vrrp->downlink_flag)
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_NO_SETTED == vrrp->downlink_vif[i].set_flg)
					{
						continue;
					}
					
					/* init */
					downlink_ifname = NULL;
					virtual_downlink_ip = 0;
					master_downlinkip = 0;
					back_downlinkip = 0;
				
					downlink_ifname = vrrp->downlink_vif[i].ifname;
					virtual_downlink_ip = vrrp->downlink_vaddr[i].addr;
					switch (vrrp->state)
					{
						case VRRP_STATE_MAST:
						{
							master_downlinkip = vrrp->downlink_vif[i].ipaddr;
							back_downlinkip = 0;
							break;
						}
						case VRRP_STATE_TRANSFER :
						case VRRP_STATE_BACK :
						{
							master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
							back_downlinkip = vrrp->downlink_vif[i].ipaddr; 
							break;
						}
						case VRRP_STATE_DISABLE :
						{
							master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
							back_downlinkip = 0;
							break;
						}
					}
					vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
										downlink_ifname,
										master_downlinkip,
										back_downlinkip,
										virtual_downlink_ip);
					
					DBusMessageIter iter_struct1;
					dbus_message_iter_open_container(&iter_array1,
													DBUS_TYPE_STRUCT,
													NULL,
													&iter_struct1);
					dbus_message_iter_append_basic(&iter_struct1,
								  DBUS_TYPE_UINT32, &master_downlinkip);
					dbus_message_iter_append_basic(&iter_struct1,
								  DBUS_TYPE_UINT32, &back_downlinkip);
					dbus_message_iter_append_basic(&iter_struct1,
								  DBUS_TYPE_UINT32, &virtual_downlink_ip);
					dbus_message_iter_append_basic(&iter_struct1,
								  DBUS_TYPE_STRING, &downlink_ifname);
					dbus_message_iter_close_container (&iter_array1, &iter_struct1);
				}
			}
			dbus_message_iter_close_container(&iter, &iter_array1);
			
			if(NULL != global_ht_ifname){
				heartbeatlink_ifname = global_ht_ifname;
			}
			else{
				if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
					if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
					{
						heartbeatlink_ifname = vrrp->uplink_vif[uplink_first_index].ifname;
					}
				}else {
					if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
					{
						heartbeatlink_ifname = vrrp->downlink_vif[downlink_first_index].ifname;
					}
				}
			}
			
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_STRING, &heartbeatlink_ifname);
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &global_ht_ip);
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &global_ht_opposite_ip);
			vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x\n",
								heartbeatlink_ifname, global_ht_ip, global_ht_opposite_ip);
			/* vgateway interface */
			#if 0
			if(NULL != vrrp->vgateway_vif.ifname){
				vgateway_ifname = vrrp->vgateway_vif.ifname;
				vgateway_ip = vrrp->vgateway_vif.ipaddr;
			}
			else {
				if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
					if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
					{
						vgateway_ifname = vrrp->uplink_vif[uplink_first_index].ifname;
						vgateway_ip = vrrp->uplink_vif[uplink_first_index].ipaddr;
					}
				}else {
					if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
					{
						vgateway_ifname = vrrp->downlink_vif[downlink_first_index].ifname;
						vgateway_ip = vrrp->downlink_vif[downlink_first_index].ipaddr;
					}
				}
			}
			
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_STRING, &vgateway_ifname);
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &vgateway_ip);
			
			vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x, vgateway %s ip %#x\n",
								heartbeatlink_ifname, global_ht_ip, global_ht_opposite_ip, vgateway_ifname, vgateway_ip);
			#else
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &vrrp->vgateway_naddr);
			dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array_vgw);
			if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
			{
				vgateway_cnt = 0;
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_NO_SETTED != vrrp->vgateway_vif[i].set_flg && 
							vgateway_cnt < vrrp->vgateway_naddr)
					{
						vgateway_ifname = vrrp->vgateway_vif[i].ifname;
						vgateway_ip = vrrp->vgateway_vaddr[i].addr; 					
						
						DBusMessageIter iter_struct_vgw;
						dbus_message_iter_open_container(&iter_array_vgw,
														DBUS_TYPE_STRUCT,
														NULL,
														&iter_struct_vgw);
						dbus_message_iter_append_basic(&iter_struct_vgw,
									  DBUS_TYPE_STRING, &vgateway_ifname);
						dbus_message_iter_append_basic(&iter_struct_vgw,
									  DBUS_TYPE_UINT32, &vgateway_ip);
						dbus_message_iter_close_container (&iter_array_vgw, &iter_struct_vgw);
						vrrp_syslog_event("%-10s %d %s ip %#x\n", vgateway_cnt ? "":"vgateway",
											vgateway_cnt, vgateway_ifname, vgateway_ip);
						vgateway_cnt++;
						vgateway_ifname = NULL;
					}
				}
			}
			dbus_message_iter_close_container(&iter, &iter_array_vgw);
		#endif
		}
	}

	return reply;
}
//for had notice to pppoe
#ifndef _VERSION_18SP7_
DBusMessage *had_dbus_set_pppoe
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1, iter_array_vgw;
	
	//int protal = 0;
	int pppoe = 0;
	vrrp_rt* vrrp = NULL;
	int vrrp_count = 0,i = 0,j = 1;	
	int master_uplinkip = 0;
	int master_downlinkip = 0;
	int back_uplinkip = 0;
	int back_downlinkip = 0;
	int virtual_uplink_ip = 0;
	int virtual_downlink_ip = 0;
	char *uplink_ifname = NULL;
	char *downlink_ifname = NULL;
	char *heartbeatlink_ifname = NULL;
	char *vgateway_ifname = NULL;
	int uplink_cnt = 0, downlink_cnt = 0, vgateway_cnt = 0;
	int vgateway_ip = 0;
	int ret_uplink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int ret_downlink = VRRP_RETURN_CODE_IF_NOT_EXIST;
	int uplink_first_index = -1;
	int downlink_first_index = -1;

/*
DBUS_TYPE_UINT32, &vrrp_count); 		// count of vrrp instance
	DBUS_TYPE_UINT32					// vrid
	DBUS_TYPE_UINT32					// state
	DBUS_TYPE_UINT32					// count of uplink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master uplnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back uplink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual uplink ip address
		DBUS_TYPE_STRING_AS_STRING		// uplink interface name
	DBUS_TYPE_UINT32					// count of downlink interfaces
	Array of uplink
		DBUS_TYPE_UINT32_AS_STRING		// master downlnik ip address
		DBUS_TYPE_UINT32_AS_STRING		// back downlink ip address
		DBUS_TYPE_UINT32_AS_STRING		// virtual downlink ip address
		DBUS_TYPE_STRING_AS_STRING		// downlink interface name
	DBUS_TYPE_STRING					// heartbeat interface name
	DBUS_TYPE_UINT32					// heartbeat ip address
	DBUS_TYPE_UINT32					// opposite heartbeat ip address
*/

	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &pppoe,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		return NULL;
	}
	
    global_pppoe = pppoe;
	vrrp_syslog_dbg("pppoe = %d\n", pppoe);

    vrrp_count = had_get_runing_hansi();
	vrrp_syslog_dbg("%d vrrp is running!\n", vrrp_count);

	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &vrrp_count);
	if (1 == global_pppoe)
	{
		for (j = 0; j < MAX_HANSI_PROFILE; j++)
		{
			vrrp = had_check_if_exist(j);
			if (NULL == vrrp) {
				continue;
			}
			vrrp_syslog_dbg("start transfer vrrp %d message!\n", j);

			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &(vrrp->vrid));
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &(vrrp->state));
			vrrp_syslog_event("vrid %d,state %d\n",
								vrrp->vrid, vrrp->state);
			
			uplink_cnt = vrrp->uplink_naddr;
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &uplink_cnt);
			vrrp_syslog_event("uplink count %d\n",
								uplink_cnt);
		
			dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array);

			/* get uplink first setted index */
			ret_uplink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_UPLINK, &uplink_first_index);
			/* get downlink first setted index */
			ret_downlink = had_get_link_first_setted_index(vrrp, VRRP_LINK_TYPE_DOWNLINK, &downlink_first_index);

			if (VRRP_LINK_SETTED == vrrp->uplink_flag)
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_NO_SETTED == vrrp->uplink_vif[i].set_flg)
					{
						continue;
					}
					
					/* init */
					uplink_ifname = NULL;
					virtual_uplink_ip = 0;
					master_uplinkip = 0;
					back_uplinkip = 0;
					
					uplink_ifname = vrrp->uplink_vif[i].ifname;
					virtual_uplink_ip = vrrp->uplink_vaddr[i].addr;
					
					switch (vrrp->state)
					{
						case VRRP_STATE_MAST:
						{
							master_uplinkip = vrrp->uplink_vif[i].ipaddr;
							back_uplinkip = 0;
							break;
						}
						case VRRP_STATE_TRANSFER :
						case VRRP_STATE_BACK :
						{
							master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
							back_uplinkip = vrrp->uplink_vif[i].ipaddr;
							break;
						}
						case VRRP_STATE_DISABLE :
						{
							master_uplinkip = master_ipaddr_uplink[vrrp->vrid];
							back_uplinkip = 0;
							break;
						}
					}
		
					vrrp_syslog_event("uplink %s master ip %x, back ip %x, virtual ip %x\n",
										uplink_ifname,
										master_uplinkip,
										back_uplinkip,
										virtual_uplink_ip);
					DBusMessageIter iter_struct;
					dbus_message_iter_open_container(&iter_array,
													DBUS_TYPE_STRUCT,
													NULL,
													&iter_struct);
					dbus_message_iter_append_basic(&iter_struct,
								  DBUS_TYPE_UINT32, &master_uplinkip);
					dbus_message_iter_append_basic(&iter_struct,
								  DBUS_TYPE_UINT32, &back_uplinkip);
					dbus_message_iter_append_basic(&iter_struct,
								  DBUS_TYPE_UINT32, &virtual_uplink_ip);
					dbus_message_iter_append_basic(&iter_struct,
								  DBUS_TYPE_STRING, &uplink_ifname);
		
					dbus_message_iter_close_container (&iter_array, &iter_struct);
				}
			}
			dbus_message_iter_close_container(&iter, &iter_array);
			
			downlink_cnt = vrrp->downlink_naddr;
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &downlink_cnt);
			vrrp_syslog_event("downlink count %d\n",
								downlink_cnt);
			
			dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array1);
			
			/* downlink */
			if (VRRP_LINK_SETTED == vrrp->downlink_flag)
			{
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_NO_SETTED == vrrp->downlink_vif[i].set_flg)
					{
						continue;
					}
					
					/* init */
					downlink_ifname = NULL;
					virtual_downlink_ip = 0;
					master_downlinkip = 0;
					back_downlinkip = 0;
				
					downlink_ifname = vrrp->downlink_vif[i].ifname;
					virtual_downlink_ip = vrrp->downlink_vaddr[i].addr;
					switch (vrrp->state)
					{
						case VRRP_STATE_MAST:
						{
							master_downlinkip = vrrp->downlink_vif[i].ipaddr;
							back_downlinkip = 0;
							break;
						}
						case VRRP_STATE_TRANSFER :
						case VRRP_STATE_BACK :
						{
							master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
							back_downlinkip = vrrp->downlink_vif[i].ipaddr; 
							break;
						}
						case VRRP_STATE_DISABLE :
						{
							master_downlinkip = master_ipaddr_downlink[vrrp->vrid];
							back_downlinkip = 0;
							break;
						}
					}
					vrrp_syslog_event("downlink %s master ip %x, back ip %x, virtual ip %x\n",
										downlink_ifname,
										master_downlinkip,
										back_downlinkip,
										virtual_downlink_ip);
					
					DBusMessageIter iter_struct1;
					dbus_message_iter_open_container(&iter_array1,
													DBUS_TYPE_STRUCT,
													NULL,
													&iter_struct1);
					dbus_message_iter_append_basic(&iter_struct1,
								  DBUS_TYPE_UINT32, &master_downlinkip);
					dbus_message_iter_append_basic(&iter_struct1,
								  DBUS_TYPE_UINT32, &back_downlinkip);
					dbus_message_iter_append_basic(&iter_struct1,
								  DBUS_TYPE_UINT32, &virtual_downlink_ip);
					dbus_message_iter_append_basic(&iter_struct1,
								  DBUS_TYPE_STRING, &downlink_ifname);
					dbus_message_iter_close_container (&iter_array1, &iter_struct1);
				}
			}
			dbus_message_iter_close_container(&iter, &iter_array1);
			
			if(NULL != global_ht_ifname){
				heartbeatlink_ifname = global_ht_ifname;
			}
			else{
				if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
					if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
					{
						heartbeatlink_ifname = vrrp->uplink_vif[uplink_first_index].ifname;
					}
				}else {
					if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
					{
						heartbeatlink_ifname = vrrp->downlink_vif[downlink_first_index].ifname;
					}
				}
			}
			
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_STRING, &heartbeatlink_ifname);
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &global_ht_ip);
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &global_ht_opposite_ip);
			vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x\n",
								heartbeatlink_ifname, global_ht_ip, global_ht_opposite_ip);
			/* vgateway interface */
			#if 0
			if(NULL != vrrp->vgateway_vif.ifname){
				vgateway_ifname = vrrp->vgateway_vif.ifname;
				vgateway_ip = vrrp->vgateway_vif.ipaddr;
			}
			else {
				if (VRRP_LINK_NO_SETTED == vrrp->downlink_flag) {
					if (VRRP_RETURN_CODE_IF_EXIST == ret_uplink)
					{
						vgateway_ifname = vrrp->uplink_vif[uplink_first_index].ifname;
						vgateway_ip = vrrp->uplink_vif[uplink_first_index].ipaddr;
					}
				}else {
					if (VRRP_RETURN_CODE_IF_EXIST == ret_downlink)
					{
						vgateway_ifname = vrrp->downlink_vif[downlink_first_index].ifname;
						vgateway_ip = vrrp->downlink_vif[downlink_first_index].ipaddr;
					}
				}
			}
			
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_STRING, &vgateway_ifname);
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &vgateway_ip);
			
			vrrp_syslog_event("heartbeat %s ip %#x opposite ip %#x, vgateway %s ip %#x\n",
								heartbeatlink_ifname, global_ht_ip, global_ht_opposite_ip, vgateway_ifname, vgateway_ip);
			#else
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_UINT32, &vrrp->vgateway_naddr);
			dbus_message_iter_open_container (&iter,
											DBUS_TYPE_ARRAY,
											DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_STRING_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
											DBUS_STRUCT_END_CHAR_AS_STRING,
											&iter_array_vgw);
			if (VRRP_LINK_SETTED == vrrp->vgateway_flag)
			{
				vgateway_cnt = 0;
				for (i = 0; i < VRRP_LINK_MAX_CNT; i++)
				{
					if (VRRP_LINK_NO_SETTED != vrrp->vgateway_vif[i].set_flg && 
							vgateway_cnt < vrrp->vgateway_naddr)
					{
						vgateway_ifname = vrrp->vgateway_vif[i].ifname;
						vgateway_ip = vrrp->vgateway_vaddr[i].addr; 					
						
						DBusMessageIter iter_struct_vgw;
						dbus_message_iter_open_container(&iter_array_vgw,
														DBUS_TYPE_STRUCT,
														NULL,
														&iter_struct_vgw);
						dbus_message_iter_append_basic(&iter_struct_vgw,
									  DBUS_TYPE_STRING, &vgateway_ifname);
						dbus_message_iter_append_basic(&iter_struct_vgw,
									  DBUS_TYPE_UINT32, &vgateway_ip);
						dbus_message_iter_close_container (&iter_array_vgw, &iter_struct_vgw);
						vrrp_syslog_event("%-10s %d %s ip %#x\n", vgateway_cnt ? "":"vgateway",
											vgateway_cnt, vgateway_ifname, vgateway_ip);
						vgateway_cnt++;
						vgateway_ifname = NULL;
					}
				}
			}
			dbus_message_iter_close_container(&iter, &iter_array_vgw);
		#endif
		}
	}

	return reply;
}
#endif

DBusMessage * had_dbus_set_debug_value(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =0 ;
	unsigned int flag = 0;
	char *ident = "VRRP";
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		return NULL;	
	}
	/* here call rstp api */
	if( !(set_debug_value(flag))){
		openlog(ident, 0, LOG_DAEMON); 
		vrrp_log_close = 0;
		ret = 1;
	} 
	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	
	return reply;
}

DBusMessage * had_dbus_no_debug_value(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =0 ;
	unsigned int flag;
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		return NULL;	
	}
	/* here call rstp api */
	if( !(set_no_debug_value(flag))){
		vrrp_log_close = 1;
		closelog();
		ret = 1;
	}

	reply = dbus_message_new_method_return(msg);
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 
							 DBUS_TYPE_INVALID);
	
	return reply;

}

DBusMessage * had_dbus_set_had_trap_switch(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =0 ;
	unsigned int flag = 0;
	char *ident = "VRRP";
	unsigned int profile = 0;
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&profile,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		return NULL;	
	}
	ret = had_set_vrrp_trap_on_off(profile,flag);
	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}
	
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);

	vrrp_syslog_dbg("%s %d,profile:%d,ret:%d,set had trap %s.\n",
					__func__\
					,__LINE__\
					,global_current_instance_no\
					,ret,(flag == 0)?"disable":"enable"
					);
	return reply;
}


DBusMessage * had_dbus_send_arp(DBusConnection *conn, DBusMessage *msg, void *user_data){
	DBusMessage* reply;
	DBusError err;
	DBusMessageIter	 iter;
	
	unsigned int ret =0 ;
	unsigned profile = 0;
    char* ifname = NULL,*ip = NULL;
	char *t_ifname = NULL;
	char mac[6] = {0};
	int ipaddr = 0;
        char temIfname[MAX_IFNAME_LEN] = {0};
   
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&profile,
								DBUS_TYPE_STRING,&t_ifname,
								DBUS_TYPE_STRING,&ip,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		
		return NULL;	
	}
    /*get uplink if ip addr*/
  /*  strcpy(temIfname,ifname);
    if(VRRP_RETURN_CODE_OK != had_check_and_convert_ifname(profile,temIfname)){
        ret = VRRP_RETURN_CODE_BAD_PARAM;
        goto out;
    }
    ifname = temIfname;
    vrrp_syslog_dbg("get ifname %s ip %s,mac %02x:%02x:%02x:%02x:%02x:%02x",ifname,ip,mac[0],\
                    mac[1],mac[2],mac[3],mac[4],mac[5]);*/
    /*uplink vritual ip*/
    ipaddr = inet_addr(ip);

	ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == ifname){
		vrrp_syslog_error("%s %d,vgateway_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	} 
	memset(ifname,0,MAX_IFNAME_LEN);
	memcpy(ifname,t_ifname,strlen(t_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(ifname != NULL){
			free(ifname);
			ifname = NULL;
		}
	   return NULL;
	}	
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,ifname,tmp_ifname);
		memcpy(ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,ifname %s not exsit.\n",__func__,__LINE__,ifname);
	}
	free(tmp_ifname);
	tmp_ifname = NULL;

	/*send gratiouas arp*/
	had_send_gratuitous_arp(ifname,mac,ipaddr);
out:
	reply = dbus_message_new_method_return(msg);
	
	if(NULL==reply)
	{
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(ifname != NULL){
			free(ifname);
			ifname = NULL;
		}
		return reply;
	}

	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,&ret);
	if(ifname != NULL){
		free(ifname);
		ifname = NULL;
	}
	return reply;

}


/*
 *******************************************************************************
 *vrrp_dbus_config_notify_obj_and_flg()
 *
 *  DESCRIPTION:
 *		config hansi notify obj and flag of on/off
 *			
 *  INPUTS:
 *		DBusConnection *conn,
 *		DBusMessage *msg,
 *		void *user_data
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
DBusMessage *had_dbus_config_notify_obj_and_flg
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
    unsigned int profile = 0;
	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned char notify_obj = VRRP_NOTIFY_OBJ_TPYE_INVALID;
	unsigned char notify_flg = VRRP_NOTIFY_OFF;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg ,&err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_BYTE, &notify_obj,
								DBUS_TYPE_BYTE, &notify_flg,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	vrrp_syslog_dbg("config vrrp %d notify obj %s %s.\n",
					profile,
					(VRRP_NOTIFY_OBJ_TPYE_WID == notify_obj) ? "WID" :
					((VRRP_NOTIFY_OBJ_TPYE_PORTAL == notify_obj) ? "PORTAL" : "ERR_OBJ"),
					(VRRP_NOTIFY_ON == notify_flg) ? "on" :
					((VRRP_NOTIFY_OFF == notify_flg) ? "off" : "ERR_FLG"));

	ret = had_set_notify_obj_on_off(profile, notify_obj, notify_flg);
	if (VRRP_RETURN_CODE_OK != ret) {
		vrrp_syslog_error("config vrrp %d notify obj error, ret %x.", ret);
	}

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}

/*
 *******************************************************************************
 *had_dbus_config_vgateway_transform_flg()
 *
 *  DESCRIPTION:
 *		config hansi instance vgateway transform flag of on/off
 *			
 *  INPUTS:
 *		DBusConnection *conn,
 *		DBusMessage *msg,
 *		void *user_data
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
DBusMessage *had_dbus_config_vgateway_transform_flg
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
    unsigned int profile = 0;
	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int vgateway_tf_flg = VRRP_VGATEWAY_TF_FLG_OFF;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg ,&err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_UINT32, &vgateway_tf_flg,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	vrrp_syslog_dbg("config vrrp %d vgateway transform flag %s.\n",
					profile,
					(VRRP_VGATEWAY_TF_FLG_ON == vgateway_tf_flg) ? "on" :
					((VRRP_VGATEWAY_TF_FLG_OFF == vgateway_tf_flg) ? "off" : "ERR_FLG"));

	ret = had_set_vgateway_tf_flag_on_off(profile, vgateway_tf_flg);
	if (VRRP_RETURN_CODE_OK != ret) {
		vrrp_syslog_error("config vrrp %d notify obj error, ret %x.", ret);
	}

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}

/*
 *******************************************************************************
 *had_dbus_config_back_down_flg()
 *
 *  DESCRIPTION:
 *		config hansi instance uplink/downlink/vgateway/l2-uplink back down flag on/off
 *			
 *  INPUTS:
 *		DBusConnection *conn,
 *		DBusMessage *msg,
 *		void *user_data
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
DBusMessage *had_dbus_config_back_down_flg
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter ;
	DBusError		err ;
    unsigned int profile = 0;
	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned int set_flg = VRRP_VGATEWAY_TF_FLG_OFF;
	unsigned int vrrp_link_type = VRRP_LINK_TYPE_INVALID;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg ,&err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_UINT32, &vrrp_link_type,
								DBUS_TYPE_UINT32, &set_flg,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	vrrp_syslog_dbg("config vrrp %d back down flag %s for type %d.\n",
					profile,
					(VRRP_ON == set_flg) ? "on" :
					((VRRP_OFF == set_flg) ? "off" : "ERR_FLG"),
						vrrp_link_type);

	ret = had_set_back_down_flag_on_off(profile, set_flg, vrrp_link_type);
	if (VRRP_RETURN_CODE_OK != ret) {
		vrrp_syslog_error("set back down flag %s for type %d error, ret %x.", 
			(VRRP_ON == set_flg) ? "on" :
			(VRRP_OFF == set_flg) ? "off" : "ERR_FLG",
				vrrp_link_type, ret);
	}

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}
/*
 *******************************************************************************
 * had_dbus_config_dhcp_failover()
 *
 *  DESCRIPTION:
 *		config hansi instance dhcp failover ip addresses of both sides
 *			
 *  INPUTS:
 *		DBusConnection *conn,
 *		DBusMessage *msg,
 *		void *user_data
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
DBusMessage *had_dbus_config_dhcp_failover
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
    unsigned int profile = 0, peerip = 0, localip = 0;
	unsigned int ret = VRRP_RETURN_CODE_OK;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg ,&err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_UINT32, &peerip,
								DBUS_TYPE_UINT32, &localip,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = had_cfg_dhcp_failover(profile, peerip, localip);
	if(VRRP_RETURN_CODE_OK != ret) {
		vrrp_syslog_dbg("config vrrp %d dhcp failover peer %#x local %#x failed %d!\n",
							profile, peerip, localip, ret);
	}
	else {
		ret = 0;
	}
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}

/*
 *******************************************************************************
 * had_dbus_clear_dhcp_failover()
 *
 *  DESCRIPTION:
 *		Clear hansi instance dhcp failover ip addresses of both sides
 *			
 *  INPUTS:
 *		DBusConnection *conn,
 *		DBusMessage *msg,
 *		void *user_data
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
DBusMessage *had_dbus_clear_dhcp_failover
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
    unsigned int profile = 0, peerip = 0, localip = 0;
	unsigned int ret = VRRP_RETURN_CODE_OK;

	dbus_error_init(&err);

	if (!(dbus_message_get_args(msg ,&err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = had_cfg_dhcp_failover(profile, ~0UI, ~0UI);
	if(VRRP_RETURN_CODE_OK != ret) {
		vrrp_syslog_dbg("config vrrp %d dhcp failover peer %#x local %#x failed %d!\n",
							profile, peerip, localip, ret);
	}
	else {
		ret = 0;
	}
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		return reply;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}

/*
 *******************************************************************************
 * had_dbus_l2_uplink_ifname_add
 *
 *  DESCRIPTION:
 *		Clear hansi instance dhcp failover ip addresses of both sides
 *			
 *  INPUTS:
 *		DBusConnection *conn,
 *		DBusMessage *msg,
 *		void *user_data
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_VIP_LAST_ONE                                  - full when add
 *		VRRP_RETURN_CODE_IF_NOT_EXIST						- ifname not found when delete
 *
 *******************************************************************************
 */
DBusMessage *had_dbus_l2_uplink_ifname_add
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
    unsigned int profile = 0;
	unsigned int ret = VRRP_RETURN_CODE_OK;
	unsigned char *ifname = NULL;
	unsigned char *t_ifname = NULL;
	unsigned int isAdd = 0;

	dbus_error_init(&err);
    vrrp_syslog_error("func %s line %d\n",__func__,__LINE__);
	if (!(dbus_message_get_args(msg ,&err,
								DBUS_TYPE_UINT32, &profile,
								DBUS_TYPE_STRING, &t_ifname,
								DBUS_TYPE_UINT32, &isAdd,
								DBUS_TYPE_INVALID)))
	{
		vrrp_syslog_dbg("vrrp unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			vrrp_syslog_error("vrrp %s raised:%s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    hansi_s* hansi = NULL;
    vrrp_rt* vrrp = NULL;

	ifname = (char*)malloc(MAX_IFNAME_LEN);
	if(NULL == ifname){
		vrrp_syslog_error("%s %d,vgateway_ifname malloc fail.\n",__func__,__LINE__);
		return NULL;
	} 
	memset(ifname,0,MAX_IFNAME_LEN);
	memcpy(ifname,t_ifname,strlen(t_ifname));

	char *tmp_ifname = NULL;
	tmp_ifname = (char *)malloc(MAX_IFNAME_LEN);
	if(NULL == tmp_ifname){
		vrrp_syslog_error("%s %d,tmp_ifname malloc fail.\n",__func__,__LINE__);
		if(ifname){
			free(ifname);
			ifname = NULL;
		}
	   return NULL;
	} 
	memset(tmp_ifname,0,MAX_IFNAME_LEN);
	if(!check_ve_interface(ifname,tmp_ifname)){
		vrrp_syslog_dbg("%s,%d,ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,ifname,tmp_ifname);
		memcpy(ifname,tmp_ifname,strlen(tmp_ifname));
		vrrp_syslog_dbg("%s,%d,ifname=%s,tmp_ifname=%s.\n",__func__,__LINE__,ifname,tmp_ifname);
	}else{
		vrrp_syslog_error("%s %d,ifname %s not exsit.\n",__func__,__LINE__,ifname);
	}
	if(tmp_ifname != NULL){
		free(tmp_ifname);
		tmp_ifname = NULL;
	}
	hansi = had_get_profile_node(profile);
	if(NULL != hansi){
		if(NULL != (vrrp= hansi->vlist)){
			if(VRRP_SERVICE_ENABLE == service_enable[profile]){
		        ret = VRRP_RETURN_CODE_SERVICE_NOT_PREPARE;
			}
			else{
				ret = VRRP_RETURN_CODE_OK;
				//ret = had_check_and_convert_ifname(profile, ifname);
				if(!if_nametoindex(ifname)){
					ret = VRRP_RETURN_CODE_IF_NOT_EXIST;
				}
				if(VRRP_RETURN_CODE_OK == ret){
					if(isAdd){
						ret = had_l2_uplink_ifname_add(vrrp,ifname);
						if(VRRP_RETURN_CODE_SUCCESS == ret){
							vrrp->l2_uplink_flag = VRRP_LINK_SETTED;
						}
					}
					else{
						ret = had_l2_uplink_ifname_delete(vrrp,ifname);
						if(VRRP_RETURN_CODE_SUCCESS == ret){
							if(!(vrrp->l2_uplink_naddr)){
								vrrp->l2_uplink_flag = VRRP_LINK_NO_SETTED;
							}
						}
					}
					if(0 != ret) {
						vrrp_syslog_error("had l2 uplink ifname %s %s failed %#x!\n",
											ifname, isAdd ? "add":"delete", ret);
					}	
				}
				else{
					vrrp_syslog_error("had l2 uplink interface %s not exists,ret %#x\n",ifname,ret);					
				}
			}
		}
		else{
			ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
		}
	}
	else{
		ret = VRRP_RETURN_CODE_PROFILE_NOTEXIST;
	}
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		vrrp_syslog_error("vrrp dbus set error!\n");
		if(ifname){
			free(ifname);
			ifname = NULL;
		}
		return reply;
	}

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	if(ifname){
		free(ifname);
		ifname = NULL;
	}
	return reply;
}

static DBusHandlerResult had_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
		DBusMessage 	*reply = NULL;
		
		if (strcmp(dbus_message_get_path(message), global_current_objpath) == 0) 
		{
		    vrrp_syslog_dbg("vrrp dbus message handler, path[%s]!\n", global_current_objpath);
			if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_CONFIG_HANSI_PROFILE))
			{
				reply = had_dbus_set_hansi_profile(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_HEARTBEAT_LINK))
			{
				reply = had_dbus_config_heartbeat_link(connection,message,user_data);
			}			
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_VRRP))
			{
				reply = had_dbus_start(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_REAL_IP))
			{
				reply = had_dbus_appoint_real_ip(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_REAL_IP_DOWNLINK))
			{
				reply = had_dbus_appoint_real_ip_downlink(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_REAL_IP_UPLINK))
			{
				reply = had_dbus_appoint_real_ip_uplink(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_NO_REAL_IP_DOWNLINK))
			{
				reply = had_dbus_appoint_no_real_ip_downlink(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_NO_REAL_IP_UPLINK))
			{
				reply = had_dbus_appoint_no_real_ip_uplink(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SET_VRRPID))
			{
				reply = had_dbus_set_vrid(connection,message,user_data);
			}			
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_VRRP_UPLINK))
			{
				reply = had_dbus_start_uplink(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_VRRP_DOWNLINK))
			{
				reply = had_dbus_start_downlink(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE))
			{
				reply = had_dbus_service_enable(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_STATE_CHANGE))
			{
				reply = had_dbus_state_change(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VRRP_WANT_STATE))
			{
				reply = had_dbus_want_master(connection,message,user_data);
			}				
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_V_GATEWAY))
			{
				reply = had_dbus_v_gateway(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_NO_V_GATEWAY))
			{
				reply = had_dbus_no_v_gateway(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_NO_TRANSFER))
			{
				reply = had_dbus_no_transfer(connection,message,user_data);
			}				
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_END_VRRP))
			{
				reply = had_dbus_end(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_PROFILE_VALUE))
			{
				reply = had_dbus_config_profile(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_GLOBAL_VMAC_ENABLE))
			{
				reply = had_dbus_config_gvmac(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_PREEMPT_VALUE))
			{
				reply = had_dbus_config_state(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_ADVERT_VALUE))
			{
				reply = had_dbus_config_advert(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VIRTUAL_MAC_VALUE))
			{
				reply = had_dbus_config_virtual_mac(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_MS_DOWN_PACKT_COUNT))
			{
				reply = had_dbus_config_ms_down_count(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_MULTI_LINK_DETECT))
			{
				reply = had_dbus_config_multi_link_detect(connection,message,user_data);
			}				
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SHOW))
			{
				reply = had_dbus_show(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SHOW_DETAIL))
			{
				reply = had_dbus_show_detail(connection,message,user_data);
			}			
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SHOW_RUNNING))
			{
				reply = had_dbus_show_running(connection,message,user_data);
			}			
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_BRG_DBUG_VRRP))
			{
				reply = had_dbus_set_debug_value(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_BRG_NO_DBUG_VRRP))
			{
				reply = had_dbus_no_debug_value(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_HAD_TRAP_SWITCH))
			{
				reply = had_dbus_set_had_trap_switch(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_GET_IFNAME))
			{
				reply = had_dbus_get_ifname(connection,message,user_data);
			}	
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SET_PROTAL))
			{
				reply = had_dbus_set_protal(connection,message,user_data);
			}
		#ifndef _VERSION_18SP7_	
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SET_PPPOE))
			{
				reply = had_dbus_set_pppoe(connection,message,user_data);
			}
		#endif	
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SET_TRANSFER_STATE))
			{
				reply = had_dbus_set_wid_transfer_state(connection,message,user_data);
			}	
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SET_PORTAL_TRANSFER_STATE))
			{
				reply = had_dbus_set_portal_transfer_state(connection,message,user_data);
			}
		#ifndef _VERSION_18SP7_	
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SET_PPPOE_TRANSFER_STATE))
			{
				reply = had_dbus_set_pppoe_transfer_state(connection,message,user_data);
			}
		#endif
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_SEND_ARP))
			{
				reply = had_dbus_send_arp(connection,message,user_data);
			}			
			else if (dbus_message_is_method_call(message, VRRP_DBUS_INTERFACE, VRRP_DBUS_METHOD_SET_NOTIFY_OBJ_AND_FLG))
			{
				reply = had_dbus_config_notify_obj_and_flg(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message, VRRP_DBUS_INTERFACE, VRRP_DBUS_METHOD_SET_VGATEWAY_TRANSFORM_FLG))
			{
				reply = had_dbus_config_vgateway_transform_flg(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message, VRRP_DBUS_INTERFACE, VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_VIP))
			{
				reply = had_dbus_add_del_link_vip(connection, message, user_data);
			}
			else if (dbus_message_is_method_call(message, VRRP_DBUS_INTERFACE, VRRP_DBUS_METHOD_VRRP_LINK_DEL_VIP))
			{
				reply = had_dbus_del_link_vip(connection, message, user_data);
			}
			else if(dbus_message_is_method_call(message, VRRP_DBUS_INTERFACE, VRRP_DBUS_METHOD_SET_DHCP_FAILOVER_IPADDR)) {
				reply = had_dbus_config_dhcp_failover(connection, message, user_data);
			}
			else if(dbus_message_is_method_call(message, VRRP_DBUS_INTERFACE, VRRP_DBUS_METHOD_CLEAR_DHCP_FAILOVER_IPADDR)) {
				reply = had_dbus_clear_dhcp_failover(connection, message, user_data);
			}			
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SNMP_GET_VRRP_STATE))
			{
				reply = vrrp_dbus_snmp_get_vrrp_state(connection,message,user_data);
			}
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_L2_UPLINK_ADD_DELETE))
			{
				reply = had_dbus_l2_uplink_ifname_add(connection,message,user_data);
			}			
			else if (dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_VIP_BACK_DOWN_FLG_SET))
			{
				reply = had_dbus_config_back_down_flg(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_SHOW_SWITCH_TIMES))
			{
				reply = had_dbus_show_switch_times(connection,message,user_data);
			}	
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_VRRP_UPLINK_IPV6))
			{
				reply = had_dbus_start_uplink_ipv6(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_VRRP_DOWNLINK_IPV6))
			{
				reply = had_dbus_start_downlink_ipv6(connection,message,user_data);
			}			
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_VRRP_UPLINK_LINK_LOCAL))
			{
				reply = had_dbus_start_uplink_link_local(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_VRRP_DOWNLINK_LINK_LOCAL))
			{
				reply = had_dbus_start_downlink_link_local(connection,message,user_data);
			}
			else if(dbus_message_is_method_call(message,VRRP_DBUS_INTERFACE,VRRP_DBUS_METHOD_START_VRRP_IPV6))
			{
				reply = had_dbus_start_ipv6(connection,message,user_data);
			}	
			else
			{
				vrrp_syslog_dbg("vrrp dbus message handler::no method match!\n");
			}
		} 
		if (reply)
		{
			dbus_connection_send (connection, reply, NULL);
			dbus_connection_flush(connection); // TODO	  Maybe we should let main loop process the flush
			dbus_message_unref (reply);
		}
	
	//	dbus_message_unref(message); //TODO who should unref the incoming message? 
	return DBUS_HANDLER_RESULT_HANDLED ;
}


/** Message handler for Signals
 *  or normally there should be no signals except dbus-daemon related.
 *
 *  @param  connection          D-BUS connection
 *  @param  message             Message
 *  @param  user_data           User data
 *  @return                     What to do with the message
 */
DBusHandlerResult
had_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0)
	{
		/* this is a local message; e.g. from libdbus in this process */
		vrrp_syslog_dbg(" vrrp get disconnected from the system message bus; "
							"vrrp retrying to reconnect every 3000 ms");
		
		dbus_connection_unref (vrrp_cli_dbus_connection);
		vrrp_cli_dbus_connection = NULL;
	} 
	else if (dbus_message_is_signal (message,DBUS_INTERFACE_DBUS,"vrrp nameOwnerChanged")) 
	{

		//if (services_with_locks != NULL)  service_deleted (message);
	} else
	{
		vrrp_syslog_error("vrrp no matched message filtered!\n");
		return TRUE;
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

int had_dbus_init2(void) {
	DBusError dbus_error;

	dbus_error_init (&dbus_error);

	vrrp_notify_dbus_connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &dbus_error);
	if (vrrp_notify_dbus_connection == NULL) {
		vrrp_syslog_error("vrrp get dbus for notify error:%s\n", dbus_error.message);
		return FALSE;
	}
	
	had_splice_objpath_string(VRRP_NOTIFY_DBUS_BUSNAME,
								global_current_instance_no,
								global_notify_dbusname);	

	dbus_bus_request_name(vrrp_notify_dbus_connection,global_notify_dbusname,0,&dbus_error);
	
	if (dbus_error_is_set(&dbus_error)) {
		vrrp_syslog_error("vrrp dbus for notify request name failed:%s\n", dbus_error.message);
		return FALSE;
	}
	vrrp_syslog_dbg("vrrp dbus(%p) for notify connected.\n");

	return 0;
}

int had_dbus_init(void)
{
	DBusError 	dbus_error;
	int			ret = 0;
	DBusObjectPathVTable	vrrp_vtable = {NULL, &had_dbus_message_handler, NULL, NULL, NULL, NULL};

	vrrp_syslog_dbg("start vrrp dbus init...\n");

	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	vrrp_cli_dbus_connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &dbus_error);
	if (vrrp_cli_dbus_connection == NULL) {
		vrrp_syslog_error("vrrp get dbus for cli failed,error %s\n", dbus_error.message);
		return 1;
	}

	if (!dbus_connection_register_fallback (vrrp_cli_dbus_connection, global_current_objpath, &vrrp_vtable, NULL)) {
		vrrp_syslog_dbg("vrrp dbus for cli register vtable fallback failed!\n");
		return 0;		
	}
		
	ret = dbus_bus_request_name (vrrp_cli_dbus_connection, global_cli_dbusname,0, &dbus_error);
	if(-1 == ret)
	{
		vrrp_syslog_error("vrrp dbus for cli request name error %d\n",ret);		
		ret = 1;
	}
	else
		vrrp_syslog_dbg("vrrp dbus request name %s ok\n", global_cli_dbusname);
	
	if (dbus_error_is_set (&dbus_error)) {
		vrrp_syslog_error("vrrp dbus for cli request name message:%s", dbus_error.message);
		return 1;
	}

	vrrp_syslog_dbg("vrrp dbus(%p) for cli dispatcher connected.\n",vrrp_cli_dbus_connection);
	return 0;
}

void * had_dbus_thread_main(void *arg)
{
	vrrp_debug_tell_whoami("dbusHdlr", 0);
	/*
	  * For all OAM method call, synchronous is necessary.
	  * Only signal/event could be asynchronous, it could be sent in other thread.
	  */	
	while (dbus_connection_read_write_dispatch(vrrp_cli_dbus_connection, -1)) {
             ;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif
