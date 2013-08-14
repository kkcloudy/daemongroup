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
* AsdWaiSta.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "asd.h"
#include "ASDStaInfo.h"
#include "wireless_copy.h" 
#include "include/debug.h"
#include "include/alg_comm.h"
#include "include/wai_sta.h"
#include "include/pack.h"
#include "include/raw_socket.h"
#include "include/cert_auth.h"
#include "wcpss/asd/asd.h"


extern int errno;

static int set80211priv( apdata_info *pap, int op, void *data, int len);

void dump_sta(int i, struct auth_sta_info_t *sta)
{
	DPrintf("dump sta in table(%d):\n", i);
	DPrintf("status: %d\n", sta->status);
#if 0	
	DPrintf("mac=" MACSTR"\n",  MAC2STR(sta->mac));
#endif
}
/*在STA记录表中查找STA*/
/*
struct auth_sta_info_t * ap_get_sta(unsigned char *mac, apdata_info *pap)
{
	int i;
	struct auth_sta_info_t *sta_info = NULL;
	
	if(pap == NULL || mac == NULL) return NULL;
	sta_info= pap->sta_info;
	if(sta_info == NULL) return NULL;
	for (i=0; i<MAX_AUTH_MT_SIMU; i++)
	{
		if (sta_info[i].status != NO_AUTH)
		{
			dump_sta(i, &sta_info[i]);
			if((memcmp(sta_info[i].mac, mac, WLAN_ADDR_LEN)) == 0)
				break;
		}
	}
	return (i==MAX_AUTH_MT_SIMU ? NULL : &sta_info[i]);
}
*/
/*在STA记录表中为找一个记录STA信息的位置*/
struct auth_sta_info_t *ap_get_sta_pos(u8 *mac, struct auth_sta_info_t *sta_info_table)
{
	int i, k;
	k = -1;
	for (i=0; i<MAX_AUTH_MT_SIMU; i++)
	{
		if (sta_info_table[i].status != NO_AUTH)
		{
			if(memcmp((sta_info_table[i].mac), mac, WLAN_ADDR_LEN) == 0)
			{
				k = i;
				break;
			}
		}
		else
		{
			if (k == -1)
				k = i;
		}
	}
	
	return (k==-1? NULL:&sta_info_table[k]);
}

/*向STA记录表添加STA的信息*/
/*
struct auth_sta_info_t * ap_add_sta(asso_mt *passo_mt_info, int packtype,  apdata_info *pap)
{
	struct auth_sta_info_t *sta_info = NULL;
	
	DPrint_string_array("adding mac is :",  passo_mt_info->mac, 6);
	sta_info = ap_get_sta_pos(passo_mt_info->mac, pap->sta_info);	

	if (sta_info  == NULL)
	{
		DPrint_string_array("no position for", passo_mt_info->mac, 6);
		goto exit_error;
	}

	if(packtype == PACKET_AUTH_TYPE)
	{
		u8 ie_len = passo_mt_info->wie[1];
		reset_sta_info(sta_info, pap);
		
		memcpy(sta_info->mac, passo_mt_info->mac, WLAN_ADDR_LEN);           
		sta_info->packet_type = packtype;		
		htonl_buffer(passo_mt_info->gsn, 16);
		memcpy(sta_info->gsn, passo_mt_info->gsn, 16);
		DPrint_string_array("auth_table[i].gsn:", sta_info->gsn, 16);
		memcpy(sta_info->wie, passo_mt_info->wie, ie_len + 2);
		DPrint_string_array("auth_table[i].wie:",sta_info->wie, ie_len+ 2);
	}
	else if(packtype == PACKET_SESSIONKEY_TYPE )
	{
		sta_info->packet_type = packtype;		
		memcpy(sta_info->mac, passo_mt_info->mac, WLAN_ADDR_LEN);
		if(sta_info->status == MT_AUTHENTICATED)
			sta_info->status = MT_SESSIONKEYING;
	}
	else
	{
		printf("UNKNOWN packet from drv ,app abort();\n");
		sta_info = NULL;
	}

exit_error:
	return sta_info;
}
*/
/*STA超时处理*/
void sta_timeout_handle(u8 *mac, apdata_info *pap)
{
	struct auth_sta_info_t *sta_info = NULL;
	
	//sta_info = ap_get_sta(mac, pap);
	
	if(sta_info != NULL)	
	{
		if(sta_info->auth_mode == AUTH_MODE
			||(sta_info->auth_mode ==PRE_AUTH_MODE
			&&sta_info->status == MT_AUTHENTICATED))
		{
			/*清除重发缓冲区*/
			DPrintf("wapid:sta_timeout_handle\n");
			reset_table_item( sta_info);
			memset(sta_info, 0, sizeof(struct auth_sta_info_t));
			sta_info->status = NO_AUTH;
		}
	}
	return ;
}

/*向driver发送消息,set key ,set wapi */
int  wapid_ioctl(apdata_info  *pap, u16 cmd, void *buf, int buf_len)
{
	int		result = 0;
	struct	iwreq  wreq;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	
	if(pap == NULL) return -1;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	memset(&wreq, 0, sizeof(struct iwreq));

	DPrintf("wapid_ioctl:wapid->identity:%s\n",wapid->identity);

	strcpy(wreq.ifr_ifrn.ifrn_name, wapid->identity);
	wreq.u.data.pointer = (caddr_t)buf;
	wreq.u.data.length =  buf_len;
//	result = ioctl(tmp_circle->ioctl_fd, cmd, &wreq);
	
	if ( result < 0 ) 
	{
		fprintf(stderr, "in %s:%d return(%d) :  \"%s\"........... \n", __func__, __LINE__, (result),strerror(errno));
	}
	return result;
}

static int
set80211priv( apdata_info *pap, int op, void *data, int len)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	struct iwreq iwr;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	
	if(pap == NULL) return -1;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;

	memset(&iwr, 0, sizeof(iwr));
	strcpy(iwr.ifr_name, wapid->identity);
	if (len < IFNAMSIZ) {
		/*
		 * Argument data fits inline; put it there.
		 */
		memcpy(iwr.u.name, data, len);
	} else {
		/*
		 * Argument data too big for inline transfer; setup a
		 * parameter block instead; the kernel will transfer
		 * the data for the driver.
		 */
		iwr.u.data.pointer = data;
		iwr.u.data.length = len;
	}

//	if (ioctl(tmp_circle->ioctl_fd, op, &iwr) < 0) 
		{
		static const char *opnames[] = {
			"ioctl[IEEE80211_IOCTL_SETPARAM]",
			"ioctl[IEEE80211_IOCTL_GETPARAM]",
			"ioctl[IEEE80211_IOCTL_SETKEY]",
			"ioctl[SIOCIWFIRSTPRIV+3]",
			"ioctl[IEEE80211_IOCTL_DELKEY]",
			"ioctl[SIOCIWFIRSTPRIV+5]",
			"ioctl[IEEE80211_IOCTL_SETMLME]",
			"ioctl[SIOCIWFIRSTPRIV+7]",
			"ioctl[IEEE80211_IOCTL_SETOPTIE]",
			"ioctl[IEEE80211_IOCTL_GETOPTIE]",
			"ioctl[IEEE80211_IOCTL_ADDMAC]",
			"ioctl[SIOCIWFIRSTPRIV+11]",
			"ioctl[IEEE80211_IOCTL_DELMAC]",
			"ioctl[SIOCIWFIRSTPRIV+13]",
			"ioctl[IEEE80211_IOCTL_CHANLIST]",
			"ioctl[SIOCIWFIRSTPRIV+15]",
			"ioctl[IEEE80211_IOCTL_GETRSN]",
			"ioctl[SIOCIWFIRSTPRIV+17]",
			"ioctl[IEEE80211_IOCTL_GETKEY]",
		};
		op -= SIOCIWFIRSTPRIV;
		if ((0 <= op )&& ((unsigned)op < N(opnames)))
			perror(opnames[op]);
		else
			perror("ioctl[unknown???]");
		return -1;
	}
	return 0;
#undef N
}

/*解除链路验证*/
int sta_deauth(u8 *addr, int reason_code, apdata_info *pap)
{
	struct ieee80211req_mlme mlme;

	if(addr == NULL)
	{
		asd_printf(ASD_DEFAULT,MSG_INFO,"sta_deauth:addr == NULL\n");
		return -1;//qiuchen
	}
	if(pap == NULL)
	{
		asd_printf(ASD_DEFAULT,MSG_INFO,"sta_deauth:pap == NULL\n");
		return -1;//qiuchen
	}
	mlme.im_op = IEEE80211_MLME_DEAUTH;
	mlme.im_reason = reason_code;
	memcpy(mlme.im_macaddr, addr, WLAN_ADDR_LEN);
	return set80211priv(pap, IEEE80211_IOCTL_SETMLME, &mlme, sizeof(mlme));
}

/*重新初始化STA信息*/
void reset_sta_info(struct auth_sta_info_t *wapi_sta_info, apdata_info *pap)
{
	reset_table_item(wapi_sta_info);
	memset(wapi_sta_info, 0, sizeof(struct auth_sta_info_t));
	wapi_sta_info->status = NO_AUTH;
	wapi_sta_info->usksa.usk[0].valid_flag = 0;/*无效的key*/
	wapi_sta_info->usksa.usk[1].valid_flag = 1;/*有效的key*/
	wapi_sta_info->ae_group_sc = 1;
	wapi_sta_info->pap = pap;
}

void
 notify_driver_disauthenticate_sta(struct auth_sta_info_t *wapi_sta_info, const char *func, int line)
 {
	
	func = func; line = line;
	apdata_info *pap = wapi_sta_info->pap;
	/*解除链路认证*/
	DPrintf("notify_driver_disauthenticate_sta\n");
	sta_deauth(wapi_sta_info->mac, IEEE80211_MLME_DEAUTH, wapi_sta_info->pap);
	/*clear information about this STA in the auth table*/
	/*清除sta的信息*/
	reset_sta_info(wapi_sta_info, pap);
}

/*清除重发缓冲区*/
void reset_table_item(struct auth_sta_info_t *wapi_sta_info)
{
	if(wapi_sta_info == NULL)
	{
        asd_printf(ASD_DEFAULT,MSG_DEBUG," *wapi_sta_info is NULL\n");
		return;
	}
    asd_printf(ASD_DEFAULT,MSG_DEBUG," *wapi_sta_info is not NULL\n");
    wapi_sta_info->buf0.len = 0;
	wapi_sta_info->buf1.len = 0;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"in %s : wapi_sta_info->buf0.data=%p,wapi_sta_info->buf1.data=%p\n",__func__,wapi_sta_info->buf0.data,wapi_sta_info->buf1.data);
	if(wapi_sta_info->buf0.data)
	{
		free(wapi_sta_info->buf0.data);
		wapi_sta_info->buf0.data = NULL;
	}
	if(wapi_sta_info->buf1.data)
	{
		free(wapi_sta_info->buf1.data);
		wapi_sta_info->buf1.data = NULL;
	}
/*	memset(sta_info->buf0.data, 0 ,MAX_RESEND_BUF);
	memset(sta_info->buf1.data, 0 ,MAX_RESEND_BUF);*/	
	wapi_sta_info->sendinfo.cur_count = 0;
	wapi_sta_info->sendinfo.direction = SENDTO_STA;
}
/*设置重发缓冲区*/
void set_table_item(struct auth_sta_info_t *wapi_sta_info, u16 direction, u16 flag, u8*sendto_asue, int sendtoMT_len)
{
	// fill sendinfo. used to check timeout
	wapi_sta_info->sendinfo.send_time = time(0);
	wapi_sta_info->sendinfo.cur_count = 1;
	wapi_sta_info->sendinfo.timeout = TIMEOUT;
	wapi_sta_info->sendinfo.direction = direction;

	if(flag == 0)
	{
		if(wapi_sta_info->buf0.data !=NULL)
		{
			free((void *)(wapi_sta_info->buf0.data));
			wapi_sta_info->buf0.data = NULL;
		}
		wapi_sta_info->buf0.data = os_zalloc(sendtoMT_len);
		if(wapi_sta_info->buf0.data == NULL) return ;
		memset(wapi_sta_info->buf0.data, 0, sendtoMT_len);
		memcpy(wapi_sta_info->buf0.data, sendto_asue, sendtoMT_len);
		wapi_sta_info->buf0.len = sendtoMT_len;
	}
	if(flag == 1)
	{
		if(wapi_sta_info->buf1.data !=NULL)
		{
			free((void *)(wapi_sta_info->buf1.data));
			wapi_sta_info->buf1.data = NULL;
		}
		wapi_sta_info->buf1.data = os_zalloc(sendtoMT_len);
		if(wapi_sta_info->buf1.data == NULL) return;
		memset(wapi_sta_info->buf1.data, 0, sendtoMT_len);
		memcpy(wapi_sta_info->buf1.data, sendto_asue, sendtoMT_len);
		wapi_sta_info->buf1.len = sendtoMT_len;
	}
}
									 

