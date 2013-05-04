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
* AsdKeyNeg.c
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

#include <assert.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include "asd.h"
#include "ASDStaInfo.h"
#include "wcpss/asd/asd.h"

#include "include/auth.h"
#include "include/debug.h"
#include "include/cert_auth.h"
#include "include/wai_sta.h"
#include "include/alg_comm.h"
#include "include/key_neg.h"
#include "include/sms4.h"
#include "include/cgi_server.h"
#include "include/raw_socket.h"
#include "ASDDbus.h"
#include "circle.h"

/*单播Rekey中的单播密钥协商请求*/
void usk_dynegotiation_req(asso_mt *passo_mt_info, apdata_info *pap)
{
	int sendtoMT_len = 0;
	int sendlen = 0;
	u16 offset= 0;
	u8 sendto_asue[FROM_MT_LEN];
	u8 keyid;
	session_key_neg_request  dynamic_sessionkey;
	struct auth_sta_info_t *wapi_sta_info = NULL;
	struct ethhdr eh;
	wai_fixdata_flag flag;
	packet_head head;

	memset(&dynamic_sessionkey, 0,sizeof(session_key_neg_request));
	memcpy(eh.h_dest, passo_mt_info->mac, ETH_ALEN);

	/*添加STA信息*/
	//wapi_sta_info = ap_add_sta(passo_mt_info, PACKET_SESSIONKEY_TYPE, pap);

	if(wapi_sta_info == NULL)
	{
		return;
    	}
	/*设置uskid*/
	wapi_sta_info->usksa.dynamic_key_used = wapi_sta_info->usksa.uskid;
	DPrintf("1 keyid:%d\n", wapi_sta_info->usksa.uskid);
	wapi_sta_info->usksa.uskid =wapi_sta_info->usksa.uskid^0x01;
	keyid = wapi_sta_info->usksa.uskid;	
	wapi_sta_info->usksa.usk[keyid].valid_flag = 0;/*invalid*/
	DPrintf("2 keyid:%d\n", wapi_sta_info->usksa.uskid);

	/*构造单播密钥协商请求包头*/
	memset((u8 *)&head, 0, sizeof(packet_head));
	head.version = VERSIONNOW;
	head.type = WAI;
	head.sub_type = SESSIONKEYNEGREQUEST;
	head.reserved = RESERVEDDEF;
	head.data_len     = 0x0000;
	head.frame_sc = 0x00;
	head.flag = 0x00;
	head.group_sc =  wapi_sta_info->ae_group_sc;
	
	/*确定使用哪个uskid，使用当前无效的那个，经过协商
	后使之有效，而另一个成为无效*/
	memset((u8 *)&flag, 0, 1);
	/*设置USK更新标识*/
	flag = WAI_FLAG_USK_UPDATE;
	/*打包包头*/
	offset = c_pack_packet_head(&head, sendto_asue, offset, FROM_MT_LEN);
	/*打包标志*/
	offset = c_pack_byte((u8 *)&flag, sendto_asue, offset, FROM_MT_LEN);
	/*打包bkid*/
	offset = c_pack_16bytes((u8 *)&wapi_sta_info->bksa.bkid, sendto_asue, offset, FROM_MT_LEN);
	/*打包uskid*/
	offset = c_pack_byte((u8 *)&wapi_sta_info->usksa.uskid, sendto_asue, offset, FROM_MT_LEN);
	/*打包addid*/
	offset = pack_mac(pap->macaddr, sendto_asue, offset);
	offset = pack_mac(wapi_sta_info->mac, sendto_asue, offset);
	/*打包AE的挑战*/
	offset = c_pack_32bytes(wapi_sta_info->ae_nonce, sendto_asue, offset, FROM_MT_LEN);
	/*设置包的长度*/
	set_packet_data_len(sendto_asue,offset);


	sendtoMT_len = offset;

	DPrint_string_array("usk_dynegotiation_req", sendto_asue, sendtoMT_len);

	/*设置重发缓存区*/
	set_table_item(wapi_sta_info, SENDTO_STA, 1, sendto_asue, sendtoMT_len);
	//circle_cancel_timeout(wapi_retry_timer, pap, wapi_sta_info);
	//circle_register_timeout(1, 0, wapi_retry_timer, pap, wapi_sta_info);
	
	if(wapi_sta_info->status == MT_GROUPNOTICEING)
		wapi_sta_info->status = MT_SESSIONGROUPING;

	/*set used key id to avoid collision,that is when we send dynamic session require, 
	   and send group notice before we receive dynamic session response, we firstly  
	   receive dynamic session response later, at the same time we change dynamic session key, 
	   and check key,  secondly receive group notice response in which we check data with raw check
	   key ,not the one we changed*/

	sendlen = send_rs_data(sendto_asue, sendtoMT_len, &eh, pap);
	if(sendlen != sendtoMT_len)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"(sendlen != sendtoMT_len)\n");

		DPrintf("send_rs_data error in send_ap_random_handle\n");
		//return;
	}
	wapi_sta_info->ae_group_sc+= 1;
	return;	
}/*==========================end============================*/


/*单播密钥协商请求*/
void	usk_negotiation_req(struct auth_sta_info_t *wapi_sta_info)
{	
	int sendtoMT_len = 0;
	int sendlen = 0;
	int   i;

	big_data data_buff;
	u16 offset= 0;

	struct ethhdr eh;
	wai_fixdata_flag flag;
	packet_head head;
	
	u8 sendto_asue[FROM_MT_LEN];
	apdata_info *pap = wapi_sta_info->pap;
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);
	memcpy(eh.h_dest, wapi_sta_info->mac, ETH_ALEN);
	
	memset(&data_buff, 0, sizeof(big_data));

	DPrintf("\nfirst change session key starts\n");	
	/*创建单播密钥请求报文*/
	memset((u8 *)&head, 0, sizeof(packet_head));
	head.version = VERSIONNOW;
	head.type = WAI;
	head.sub_type= SESSIONKEYNEGREQUEST;
	head.reserved = RESERVEDDEF;
	head.data_len     = 0x0000;
	head.frame_sc = 0x00;
	head.flag = 0x00;
	head.group_sc = wapi_sta_info->ae_group_sc;
	/*确定使用哪个uskid，使用当前无效的那个，经过协商
	后使之有效，而另一个成为无效*/
	for(i = 0; i < 2; i++)
	{
		if(wapi_sta_info->usksa.usk[i].valid_flag == 0 /*invalid*/)
			wapi_sta_info->usksa.uskid = i;
	}
	memset((u8 *)&flag, 0, 1); /*bit4 = 0*/
	wapi_sta_info->flag &= 0xEF;/*0xEF:11101111*//*先将4位清零*/
	wapi_sta_info->flag |= flag & WAI_FLAG_USK_UPDATE;/*0x10:00010000*//*bit4:usk_update*/

	offset = c_pack_packet_head(&head, sendto_asue, offset, FROM_MT_LEN);
	offset = c_pack_byte((u8 *)&flag, sendto_asue, offset, FROM_MT_LEN);
	offset = c_pack_16bytes(wapi_sta_info->bksa.bkid, sendto_asue, offset, FROM_MT_LEN);
	offset = c_pack_byte((u8 *)&wapi_sta_info->usksa.uskid , sendto_asue, offset, FROM_MT_LEN);
	offset = pack_mac(pap->macaddr, sendto_asue, offset);
	offset = pack_mac(wapi_sta_info->mac, sendto_asue, offset);
	get_random(wapi_sta_info->ae_nonce, 32);
	offset = c_pack_32bytes(wapi_sta_info->ae_nonce, sendto_asue, offset, FROM_MT_LEN);
	set_packet_data_len(sendto_asue,offset);


	sendtoMT_len = offset;
	
	/*设置重发缓冲区*/
	set_table_item(wapi_sta_info, SENDTO_STA, 1, sendto_asue, sendtoMT_len);
	circle_cancel_timeout(wapi_retry_timer, (void *)pap, (void *)wapi_sta_info);
	circle_register_timeout(1, 0, wapi_retry_timer, (void *)pap, (void *)wapi_sta_info);
	asd_printf(ASD_WAPI,MSG_DEBUG,"register wapi_retry_timer after send USK NEGOTIATION REQ\n");
	
	sendlen = send_rs_data(sendto_asue, sendtoMT_len, &eh, pap);
	if(sendlen != sendtoMT_len)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"(sendlen != sendtoMT_len)\n");

		DPrintf("\nsessionkey send error!!!\n\n");
		//return; 
	}
	wapi_sta_info->ae_group_sc += 1;/*分组序号+1*/
	wapi_sta_info->status = MT_WAITING_SESSION;/*设置asue状态*/
	return;	
}



/*单播密钥协商响应分组*/
int usk_negotiation_res(void *read_asue, int readlen,  struct auth_sta_info_t *wapi_sta_info)
{
	u16					err_pack = 0;
	u8						HMAC[HMAC_LEN] = {0,};
	u8						*hmac_data = NULL;
	u8						hmac_data_len = 0;
	u8	 					uskid;
	session_key_neg_response   key_res_buff;
	struct wapid_interfaces *tmp_wapid;
	struct asd_wapi *tmp_circle;
	apdata_info *pap = wapi_sta_info->pap;
	
	assert(pap!=NULL);
	tmp_wapid = (struct wapid_interfaces *)pap->user_data;
	tmp_circle = (struct asd_wapi *)tmp_wapid->circle_save;

	/*判断STA的状态*/
	if( (wapi_sta_info->status != MT_WAITING_SESSION) &&
		(wapi_sta_info->status != MT_SESSIONKEYING) &&
		(wapi_sta_info->status != MT_SESSIONGROUPING) &&
		(wapi_sta_info->status != MT_WAITING_DYNAMIC_SESSION))
	{
		DPrintf("current sta(" MACSTR ") status is %d\n", MAC2STR(wapi_sta_info->mac), wapi_sta_info->status);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"error 11111111111111111\n");
		goto err;
	}
	memset(&key_res_buff, 0, sizeof(struct _session_key_neg_response));
	
	err_pack=unpack_ucastkey_neg_response(&key_res_buff, read_asue,readlen);

	if(err_pack == PACK_ERROR)  
	{
		pap->wapi_mib_stats.wai_format_errors++;
		wapi_sta_info->wapi_mib_stats.wai_format_errors++;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_format_errors++\n\n",MAC2STR(pap->macaddr));
		
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"error 22222222222222222222222 \n");
		goto err;
	}
	/*save the lase bkid for mib*/
	if(tmp_circle != NULL && tmp_circle->wasd_bk != NULL)
	{
		WID_BSS *BSS = ASD_BSS[tmp_circle->wasd_bk->BSSIndex];
		if((BSS) && (ASD_WLAN[BSS->WlanID])){
			os_memcpy(ASD_WLAN[BSS->WlanID]->bkid, &(key_res_buff.bkid),16);
		}
	}
	uskid = key_res_buff.uskid;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"\n\n\n\n\n\n uskid %d \n\n\n\n\n\n",uskid);

	if(key_res_buff.flag & WAI_FLAG_USK_UPDATE)
	{
		/*检查当前有有效的USKSA 并且USKID所指USKSA无效*/
		if(wapi_sta_info->usksa.usk[uskid].valid_flag)
		{
			DPrintf("当前有有效的USKSA 并且USKID所指USKSA有效\n");
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"error 333333333333333333333\n");
			goto err;
		}
	}
	/*比较AE的挑战*/
	if(memcmp(key_res_buff.ae_challenge , wapi_sta_info->ae_nonce, 32) !=0)
	{
		DPrintf("ae_challenge not same\n");
		DPrint_string_array("AE Challenge from STA", key_res_buff.ae_challenge, 32);
		DPrint_string_array("AE Challenge in AP", wapi_sta_info->ae_nonce, 32);
		
		/*SNMPD_ATTACK_CHALLENGE_REPLAY*/
		if(tmp_circle != NULL && tmp_circle->wasd_bk != NULL)
		{
			signal_wapi_trap(wapi_sta_info->mac,tmp_circle->wasd_bk->BSSIndex,ATTACK_MIC_JUGGLE);
		}
		//trap2snmp(sta_info->mac, SNMPD_ATTACK_CHALLENGE_REPLAY);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"error 4444444444444444444444444\n");
		goto err;
	}
	memcpy(wapi_sta_info->asue_nonce , key_res_buff.asue_challenge, 32);

	/*计算KD_HMAC_SHA256,导出usk*/
	ae_usk_derivation(wapi_sta_info, &key_res_buff);
		
	/*计算MIC,比较MIC*/
	hmac_data = (unsigned char *)read_asue + sizeof(packet_head);
	hmac_data_len = readlen - sizeof(packet_head) - HMAC_LEN;
	hmac_sha256(hmac_data, hmac_data_len, 
			wapi_sta_info->usksa.usk[uskid].mck, 16, 
			HMAC, HMAC_LEN);
	
	CWCaptrue(16,wapi_sta_info->usksa.usk[uskid].mck);
	if(memcmp(HMAC, key_res_buff.mic, HMAC_LEN) != 0)
	{
		DPrintf("different hmac with sta's\n");
		DPrint_string_array("HMAC from STA", key_res_buff.mic, HMAC_LEN);
		DPrint_string_array("computed hmac by AP", HMAC, HMAC_LEN);
		DPrint_string_array("dump negotiation_res", read_asue, readlen);
		pap->wapi_mib_stats.wai_hmac_errors++;
		wapi_sta_info->wapi_mib_stats.wai_hmac_errors++;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_hmac_errors++\n\n",MAC2STR(pap->macaddr));
		/*SNMPD_ATTACK_MIC_JUGGLE*/
		if(tmp_circle != NULL && tmp_circle->wasd_bk != NULL)
		{
			signal_wapi_trap(wapi_sta_info->mac,tmp_circle->wasd_bk->BSSIndex,ATTACK_MIC_JUGGLE);
		}
		//trap2snmp(sta_info->mac, SNMPD_ATTACK_MIC_JUGGLE);
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"error 55555555555555555555555555555\n");
		goto err;//?????
	}
	if((key_res_buff.flag & WAI_FLAG_USK_UPDATE )== 0)
	{
		u8 wie_len = *(key_res_buff.wie_asue+ 1);

		if(memcmp(key_res_buff.wie_asue,  wapi_sta_info->wie, wie_len + 2) != 0)
		{
			/*SNMPD_ATTACK_LOW_SAFE_LEVEL*/
			if(tmp_circle != NULL && tmp_circle->wasd_bk != NULL)
			{
				signal_wapi_trap(wapi_sta_info->mac,tmp_circle->wasd_bk->BSSIndex,ATTACK_LOW_SAFE_LEVEL);
			}
			/*解除链路验证*/
			DPrintf("different WIE\n");
			DPrint_string_array("WIE from STA", key_res_buff.wie_asue, wie_len + 2);
			DPrint_string_array("WIE from STA association frame",wapi_sta_info->wie, wie_len+ 2);
			DPrintf("calling notify_driver_disauthenticate_sta\n");
			notify_driver_disauthenticate_sta(wapi_sta_info, __func__, __LINE__);
		}
	}
	wapi_sta_info->flag &= 0xEF;/*0xEF:11101111*//*先将4位清零*/
	wapi_sta_info->flag |= key_res_buff.flag & WAI_FLAG_USK_UPDATE;/*0x10:00010000*//*bit4:usk_update*/
	/*清除重发缓冲区*/
	reset_table_item(wapi_sta_info);
	DPrintf("\n\rchange session key final...........\n\r");
	return 0;
	
err:	
	DPrintf("Received a undesirable negoration response and discard it \n");
	
	pap->wapi_mib_stats.wai_discard++;
	wapi_sta_info->wapi_mib_stats.wai_discard++;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_discard++\n\n",MAC2STR(pap->macaddr));
	return -1;
}

/*单播密钥协商确认分组*/
void usk_negotiation_confirmation(struct auth_sta_info_t *wapi_sta_info)
{
	struct ethhdr eh;
	apdata_info *pap = wapi_sta_info->pap;
	int 		sendtoMT_len = 0;
	int 		sendlen = 0;
	u16 	offset= 0;
	u8 	sendto_asue[FROM_MT_LEN];
	u8 	dynamic_key_used = wapi_sta_info->usksa.dynamic_key_used;
	u8  mic[20],wie_len = 0;
	wai_fixdata_flag flag;
	packet_head head;

	memset((u8 *)&head, 0, sizeof(packet_head));
	memcpy(eh.h_dest, wapi_sta_info->mac, ETH_ALEN);
	/*构造包头*/
	head.version = VERSIONNOW;
	head.type = WAI;
	head.sub_type= SESSIONKEYNEGCONFIRM;
	head.reserved = RESERVEDDEF;
	head.data_len     = 0x0000;
	head.frame_sc = 0x00;
	head.flag = 0x00;
	head.group_sc = wapi_sta_info->ae_group_sc;
	
	memset((u8 *)&flag, 0, 1);
	flag  = wapi_sta_info->flag & WAI_FLAG_USK_UPDATE;
	/*打包包头*/
	offset = c_pack_packet_head(&head, sendto_asue, offset, FROM_MT_LEN);	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"offset 1 %d\n",offset);
	/*打包标志*/
	offset = c_pack_byte((u8 *)&flag, sendto_asue, offset, FROM_MT_LEN);	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"offset 2 %d\n",offset);
	/*打包bkid*/
	offset = c_pack_16bytes((u8 *)&wapi_sta_info->bksa.bkid, sendto_asue, offset, FROM_MT_LEN);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"offset 3 %d\n",offset);

	/*打包有效的uskid*/
	offset = c_pack_byte(&dynamic_key_used, sendto_asue, offset, FROM_MT_LEN);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"offset 4 %d\n",offset);
	/*打包addid*/
	offset = pack_mac((u8 *)&pap->macaddr, sendto_asue, offset);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"offset 5 %d\n",offset);
	offset = pack_mac(wapi_sta_info->mac, sendto_asue, offset);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"offset 6 %d\n",offset);
	/*打包ASUE的挑战*/
	offset = c_pack_32bytes(wapi_sta_info->asue_nonce, sendto_asue, offset, FROM_MT_LEN);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"offset 7 %d\n",offset);
	/*打包AE的WIE*/
	wie_len =*( pap->wie_ae + 1);
	memcpy(sendto_asue + offset, pap->wie_ae, wie_len + 2);
	offset += wie_len + 2;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"offset %d\n",offset);
	CWCaptrue(wie_len+2, pap->wie_ae);

	/*计算MIC*/
	hmac_sha256(sendto_asue + sizeof(packet_head), offset - sizeof(packet_head), 
		wapi_sta_info->usksa.usk[dynamic_key_used].mck, 16, mic, HMAC_LEN);
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"\n\n\n\n\n\n dynamic_key_used %d \n\n\n\n\n\n",dynamic_key_used);
	CWCaptrue(16,wapi_sta_info->usksa.usk[dynamic_key_used].mck);
	memcpy(sendto_asue + offset, mic, 20);
	offset += 20;
	/*设置包长度*/
	set_packet_data_len(sendto_asue,offset);
	assert(offset != PACK_ERROR);
	sendtoMT_len = offset;

	/*清除重发缓冲区*/
	set_table_item(wapi_sta_info, SENDTO_STA, 1, sendto_asue, sendtoMT_len);
	//circle_cancel_timeout(wapi_retry_timer, pap, wapi_sta_info);
	//circle_register_timeout(1, 0, wapi_retry_timer, pap, wapi_sta_info);

	//auth_table[asue_no].status = MT_SESSIONGROUPING;
	/*set used key id to avoid collision,that is when we send dynamic session require, 
	   and send group notice before we receive dynamic session response, we firstly  
	   receive dynamic session response later, at the same time we change dynamic session key, 
	   and check key,  secondly receive group notice response in which we check data with raw check
	   key ,not the one we changed*/
	/*发数据*/
	CWCaptrue(sendtoMT_len,sendto_asue);
	sendlen = send_rs_data(sendto_asue, sendtoMT_len, &eh, pap);
	if(sendlen != sendtoMT_len)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"(sendlen != sendtoMT_len)\n");

		DPrintf("send_rs_data error in send_ap_confirm_handle\n");
		//return;
	}
	wapi_sta_info->ae_group_sc += 1;
	return;	
}/*==========================end============================*/

/*构造组播通告分组*/
static short  pack_msk_announcement(struct auth_sta_info_t *wapi_sta_info, 
									u8 *sendto_asue, u8 stakey_flag, 
									BOOL if_multi_rekey)
{
	u8 *hmac_data = NULL;
	u8 *hmac_tar_data =NULL;
	u8 used_key = wapi_sta_info->usksa.dynamic_key_used;;
	u8 iv[16] = {0,};
	wai_fixdata_flag flag ;
	packet_head head;
	u16 offset = 0;
	u16	hmac_data_len = 0;
	apdata_info *pap = wapi_sta_info->pap;

	memset((u8 *)&flag, 0, 1);
	memset((u8 *)&head, 0, sizeof(packet_head));
	head.version      = VERSIONNOW;
	head.type = WAI;
	head.sub_type		= GROUPKEYNOTICE;
	head.reserved     = RESERVEDDEF;
	head.data_len     = 0x0000;
	head.frame_sc = 0x00;
	head.flag = 0x00;
	head.group_sc = wapi_sta_info->ae_group_sc;
	
	switch(stakey_flag) {
		case 0:/*bit5-0,bit6-0*/
			flag &= 0x9F ;/*10011111*/
			break;
		case 1:/*bit5-0,bit6-1*/
			flag &= 0xDF ;/*11011111*/
			break;
		case 2:/*bit5-1,bit6-0*/
			flag &= 0xBF ;/*10111111*/
			break;
		case 3:/*bit5-1,bit6-1*/
			flag &= 0xFF ;/*11111111*/
			break;
		default:
			break;
	}
	wapi_sta_info->flag &= 0x9F;/*0x9F:10011111*//*先将5和6位清零*/
	wapi_sta_info->flag |= flag & 0x60;/*0x60:01100000*//*bit5:stakey-session bit6:stakey-del*/
	
	offset = c_pack_packet_head(&head,sendto_asue,offset,FROM_MT_LEN);
	offset = c_pack_byte((u8 *)&flag, sendto_asue, offset, FROM_MT_LEN);
	offset = c_pack_byte((u8 *)&(pap->msksa.mskid), sendto_asue, offset, FROM_MT_LEN);
	offset = c_pack_byte((u8 *)&(wapi_sta_info->usksa.dynamic_key_used), sendto_asue, offset, FROM_MT_LEN);
	offset = pack_mac(pap->macaddr, sendto_asue, offset);
	offset = pack_mac(wapi_sta_info->mac, sendto_asue, offset);
	if(if_multi_rekey == FALSE)
	{
		offset = c_pack_16bytes(wapi_sta_info->gsn, sendto_asue, offset, FROM_MT_LEN);
		DPrint_string_array("auth_table[i].gsn:",wapi_sta_info->gsn, 16);
	}
	else{
		u8 gsn[16] = {0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x36,
					   0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x36};		/*组播数据序号*/
		offset = c_pack_16bytes(gsn, sendto_asue, offset, FROM_MT_LEN);
		DPrint_string_array("multi rekey:gsn:",gsn, 16);
	}
	offset = c_pack_dword(&pap->gnonce[0], sendto_asue, offset, FROM_MT_LEN);
	offset = c_pack_dword(&pap->gnonce[1], sendto_asue, offset, FROM_MT_LEN);
	offset = c_pack_dword(&pap->gnonce[2], sendto_asue, offset, FROM_MT_LEN);
	offset = c_pack_dword(&pap->gnonce[3], sendto_asue, offset, FROM_MT_LEN);
	
	*(sendto_asue + offset) = MULTI_KEY_LEN;/*密钥通告数据长度*/
	offset += 1;

	/*加密组播密钥*/
	memcpy(iv, (u8 *)pap->gnonce, MULTI_KEY_LEN);
	htonl_buffer(iv, MULTI_KEY_LEN);

	wpi_encrypt(iv, pap->msksa.msk,	MULTI_KEY_LEN,
			wapi_sta_info->usksa.usk[used_key].kek,
			sendto_asue+offset);

	offset += MULTI_KEY_LEN;

	/*计算MIC*/
	hmac_data = sendto_asue+sizeof(packet_head);
	hmac_data_len = offset - sizeof(packet_head);
	hmac_tar_data = (unsigned char *)sendto_asue + offset;
	hmac_sha256(hmac_data, hmac_data_len, 
			wapi_sta_info->usksa.usk[used_key].mck, 16, 
			hmac_tar_data, HMAC_LEN);
	offset += HMAC_LEN;
	
	set_packet_data_len(sendto_asue,offset);
	return offset;
}
/*组播通告响应分组*/
int  msk_announcement_res(struct auth_sta_info_t *wapi_sta_info, u8 *read_asue, int readlen)
{
	apdata_info *pap = wapi_sta_info->pap;
	groupkey_notice_response  group_res_buff;
	u16	err_pack;
	u16 	hmac_data_len = 0;
	u8 	HMAC[20];
	u8 	*hmac_data = NULL;
	u8 	dynamic_key_used = wapi_sta_info->usksa.dynamic_key_used;
	
	DPrintf("\nprocess_groupkey_notice_response\n");
	/*检查ASUE的状态*/
	if((	wapi_sta_info->status !=MT_WAITING_GROUPING) && 
		(wapi_sta_info->status !=MT_GROUPNOTICEING)&& 
		(wapi_sta_info->status !=MT_SESSIONGROUPING) && 
		(wapi_sta_info->status !=MT_WAITING_DYNAMIC_GROUPING))
	{
		DPrintf("AP is not in a waiting group notice response\n");
		DPrintf("current sta's mac " MACSTR,MAC2STR(wapi_sta_info->mac));
		DPrintf("\n");
		DPrintf("current status = %d\n", wapi_sta_info->status );
		return -1;
	}
	
	memset(&group_res_buff, 0, sizeof(groupkey_notice_response));

	/*组播通告响应解包*/
	err_pack = unpack_msk_announcement_res(&group_res_buff, read_asue, readlen);

	if(err_pack == PACK_ERROR)  
	{
		DPrintf("unpack_groupkey_notice_response Error\n");
		pap->wapi_mib_stats.wai_format_errors++;
		wapi_sta_info->wapi_mib_stats.wai_format_errors++;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_format_errors++\n\n",MAC2STR(pap->macaddr));
		goto err;
	}

	/*计算MIC校验*/
	hmac_data = (unsigned char *)read_asue + sizeof(packet_head);
	hmac_data_len = readlen - sizeof(packet_head) - HMAC_LEN;
	hmac_sha256(hmac_data, hmac_data_len, 
			wapi_sta_info->usksa.usk[dynamic_key_used].mck, 16, HMAC, HMAC_LEN);

	/*与该分组中的MIC相比较*/	
	if(memcmp(HMAC, group_res_buff.mic, HMAC_LEN) !=0) 
	{
		DPrintf("Error Group Notice's hmac\n");
		DPrintf("USKID= %d\n ",dynamic_key_used);
		DPrint_string_array("MCK", wapi_sta_info->usksa.usk[dynamic_key_used].mck, 16);
		DPrint_string_array("receive sta's Group hmac",group_res_buff.mic, 20);
		DPrint_string_array("computed sta's Group hmac:",HMAC, 20);
		DPrint_string_array("DUMP group response", read_asue, readlen);
		pap->wapi_mib_stats.wai_hmac_errors++;
		wapi_sta_info->wapi_mib_stats.wai_hmac_errors++;
		asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_hmac_errors++\n\n",MAC2STR(pap->macaddr));
		goto err;
	}

	/*比较标识,MSKID, USKID, ADDID, 组播通告标识*/
	if (	(group_res_buff.flag & 0x60)!= (wapi_sta_info->flag & 0x60)/*bit5 and bit 6*/||
		group_res_buff.notice_keyid!= pap->msksa.mskid ||
		group_res_buff.uskid != wapi_sta_info->usksa.dynamic_key_used||
		memcmp(group_res_buff.addid.mac1, pap->macaddr, 6) !=0 ||
		memcmp(group_res_buff.addid.mac2, wapi_sta_info->mac, 6) !=0 ||
		group_res_buff.g_nonce[0]!= pap->gnonce[0] ||
		group_res_buff.g_nonce[1]!= pap->gnonce[1] ||
		group_res_buff.g_nonce[2]!= pap->gnonce[2] ||
		group_res_buff.g_nonce[3]!= pap->gnonce[3]){
		DPrintf("Error msksa.mskid or Gnonce\n");
		goto err;
	}

	/*清除重发缓冲区*/
	reset_table_item(wapi_sta_info);
	DPrintf("Auth Succeed\n");
	asd_printf(ASD_DEFAULT,MSG_INFO,MACSTR" WAPI Authorized.\n",MAC2STR(wapi_sta_info->mac));
	return 0;

err:	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"Received a undesirable group notice response and discard it!!\n\n");
	pap->wapi_mib_stats.wai_discard++;
	wapi_sta_info->wapi_mib_stats.wai_discard++;
	asd_printf(ASD_DEFAULT,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_discard++\n\n",MAC2STR(pap->macaddr));
	return -1;
}

/*组播通告分组*/
void msk_announcement_tx(struct auth_sta_info_t *wapi_sta_info, u8 *sendto_asue)
{
	int sendtoMT_len = 0;
	int sendlen = 0;
	struct ethhdr eh;
	apdata_info *pap = wapi_sta_info->pap;
	
	memcpy(eh.h_dest, wapi_sta_info->mac, ETH_ALEN);

	/*构造组播通告分组*/
	sendtoMT_len = pack_msk_announcement(wapi_sta_info, sendto_asue, 0,FALSE);
	/*设置重发缓冲区*/
	set_table_item(wapi_sta_info, SENDTO_STA, 0, sendto_asue, sendtoMT_len);
	circle_cancel_timeout(wapi_retry_timer, pap, wapi_sta_info);
	circle_register_timeout(1, 0, wapi_retry_timer, pap, wapi_sta_info);
	asd_printf(ASD_WAPI,MSG_DEBUG,"register wapi_retry_timer after send MSK ANNOUNCEMENT\n");
	/*发送数据*/
	sendlen = send_rs_data(sendto_asue, sendtoMT_len, &eh, pap);
	if(sendlen != sendtoMT_len)
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"(sendlen != sendtoMT_len)\n");

		DPrintf("------------fail to send groupkey_notice!!!\n");
		//return; 
	}
	wapi_sta_info->ae_group_sc += 1;
	DPrintf("send group notice to STA("MACSTR")\n", MAC2STR(wapi_sta_info->mac));
	wapi_sta_info->status = MT_WAITING_GROUPING;
	return;	
}


#if 0
/*给所有鉴别成功的STA发送组播通告*/
void send_msk_announcement_to_all(apdata_info *pap)
{
	u8 sendto_asue[FROM_MT_LEN];
	int sendtoMT_len =0;
	int sendlen = 0;
	int asue_no = 0;
	struct ethhdr eh;
	struct auth_sta_info_t *sta_info_table = pap->wapi_sta_info;
	
	pap->msksa.mskid = pap->msksa.mskid^0x01;
	pap->group_No = 0;
	for (asue_no=0; asue_no<MAX_AUTH_MT_SIMU; asue_no++)
	{
		if (sta_info_table[asue_no].status == MT_AUTHENTICATED
			||sta_info_table[asue_no].status == MT_SESSIONKEYING)
		{
			/*构造组播通告分组*/
			sendtoMT_len = pack_msk_announcement(&sta_info_table[asue_no], sendto_asue, 0, TRUE);

			memcpy(eh.h_dest, sta_info_table[asue_no].mac, ETH_ALEN);
			/*发送*/
			sendlen = send_rs_data(sendto_asue, sendtoMT_len, &eh, pap);
			if(sendlen != sendtoMT_len)
			{
				printf("(sendlen != sendtoMT_len)\n");

				DPrintf("------------fail to send groupkey_notice to all!!!\n");
				//return; 
			}
			DPrintf("+++++++++++sending multikey notice to all sta!++++++++++\n");
			if(sta_info_table[asue_no].status == MT_AUTHENTICATED){
				sta_info_table[asue_no].status = MT_GROUPNOTICEING;
			}else if(sta_info_table[asue_no].status == MT_SESSIONKEYING){
				sta_info_table[asue_no].status = MT_SESSIONGROUPING;
			}
			
			set_table_item(&sta_info_table[asue_no], SENDTO_STA, 0, sendto_asue, sendtoMT_len);
			circle_cancel_timeout(wapi_retry_timer, pap, &sta_info_table[asue_no]);
			circle_register_timeout(1, 0, wapi_retry_timer, pap, &sta_info_table[asue_no]);
			pap->group_No++;
			sta_info_table[asue_no].ae_group_sc += 1;
		}
	}
	/*如果没有STA鉴别成功*/
	if((pap->group_status ==1) && (pap->group_No==0))
	{
		DPrintf("tell driver the gkeyid although now there is no any STA associated with AP\n");
		/*安装组播密钥*/
		set_mcastkey(pap, &pap->msksa);
	}
	return;
}
#endif

#if 0
//给所有鉴别成功的STA发送组播通告
//	xm06301
void send_msk_announcement_to_all(apdata_info *pap)
{
	u8 sendto_asue[FROM_MT_LEN];
	int sendtoMT_len =0;
	int sendlen = 0;
	int asue_no = 0;
	struct ethhdr eh;
	struct auth_sta_info_t *sta_info_table = pap->wapi_sta_info;
	
	pap->msksa.mskid = pap->msksa.mskid^0x01;
	pap->group_No = 0;
	for (asue_no=0; asue_no<MAX_AUTH_MT_SIMU; asue_no++)
	{
		if (sta_info_table[asue_no].status == MT_AUTHENTICATED
			||sta_info_table[asue_no].status == MT_SESSIONKEYING)
		{
			//构造组播通告分组
			sendtoMT_len = pack_msk_announcement(&sta_info_table[asue_no], sendto_asue, 0, TRUE);

			memcpy(eh.h_dest, sta_info_table[asue_no].mac, ETH_ALEN);
			//发送
			sendlen = send_rs_data(sendto_asue, sendtoMT_len, &eh, pap);
			if(sendlen != sendtoMT_len)
			{
				printf("(sendlen != sendtoMT_len)\n");

				DPrintf("------------fail to send groupkey_notice to all!!!\n");
				//return; 
			}
			DPrintf("+++++++++++sending multikey notice to all sta!++++++++++\n");
			if(sta_info_table[asue_no].status == MT_AUTHENTICATED){
				sta_info_table[asue_no].status = MT_GROUPNOTICEING;
			}else if(sta_info_table[asue_no].status == MT_SESSIONKEYING){
				sta_info_table[asue_no].status = MT_SESSIONGROUPING;
			}
			
			set_table_item(&sta_info_table[asue_no], SENDTO_STA, 0, sendto_asue, sendtoMT_len);
			pap->group_No++;
			sta_info_table[asue_no].ae_group_sc += 1;
		}
	}
	//如果没有STA鉴别成功
	if((pap->group_status ==1) && (pap->group_No==0))
	{
		DPrintf("tell driver the gkeyid although now there is no any STA associated with AP\n");
		//安装组播密钥
		set_mcastkey(pap, &pap->msksa);
	}
	return;
}

/*给所有鉴别成功的STA发送组播通告*/
#endif

//	xm06301
void send_msk_announcement_to_all(apdata_info *pap)
{
	u8 sendto_asue[FROM_MT_LEN];
	int sendtoMT_len =0;
	int sendlen = 0;
	struct ethhdr eh;
	

	struct wapid_interfaces *user=(struct wapid_interfaces *)pap->user_data;
	struct asd_wapi *wapi_wasd=NULL;
	struct asd_data * wasd=NULL;
	struct sta_info *sta=NULL;

	//struct auth_sta_info_t *sta_info_table = pap->wapi_sta_info;
	
	if(user==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s:	user==NULL\n",__func__);
		return;
	}

	wapi_wasd=(struct asd_wapi *)user->circle_save;

	if(wapi_wasd==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s:	wapi_wasd==NULL\n",__func__);
		return;
	}

	wasd=(struct asd_data *)wapi_wasd->wasd_bk;

	if(wasd==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s:	wasd==NULL\n",__func__);
		return;
	}
	
	pap->msksa.mskid = pap->msksa.mskid^0x01;
	pap->group_No = 0;

	if(wasd->num_sta<0||wasd->num_sta>STA_HASH_SIZE){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s:	wasd->num_sta=%d\n",__func__,wasd->num_sta);
		return;
	}
	
	for(sta=wasd->sta_list;sta!=NULL;sta=sta->next){
		if (sta->wapi_sta_info.status == MT_AUTHENTICATED
			||sta->wapi_sta_info.status == MT_SESSIONKEYING)
		{
			/*构造组播通告分组*/
			sendtoMT_len = pack_msk_announcement(&sta->wapi_sta_info, sendto_asue, 0, TRUE);

			memcpy(eh.h_dest, sta->wapi_sta_info.mac, ETH_ALEN);
			/*发送*/
			sendlen = send_rs_data(sendto_asue, sendtoMT_len, &eh, pap);
			if(sendlen != sendtoMT_len)
			{
				asd_printf(ASD_DEFAULT,MSG_DEBUG,"(sendlen != sendtoMT_len)\n");

				DPrintf("------------fail to send groupkey_notice to all!!!\n");
				//return; 
			}
			DPrintf("+++++++++++sending multikey notice to all sta!++++++++++\n");
			if(sta->wapi_sta_info.status == MT_AUTHENTICATED){
				sta->wapi_sta_info.status = MT_GROUPNOTICEING;
			}else if(sta->wapi_sta_info.status == MT_SESSIONKEYING){
				sta->wapi_sta_info.status = MT_SESSIONGROUPING;
			}
			
			set_table_item(&sta->wapi_sta_info, SENDTO_STA, 0, sendto_asue, sendtoMT_len);
			//circle_cancel_timeout(wapi_retry_timer, pap, &sta->wapi_sta_info);
			//circle_register_timeout(1, 0, wapi_retry_timer, pap, &sta->wapi_sta_info);
			pap->group_No++;
			sta->wapi_sta_info.ae_group_sc += 1;
		}
	}

	/*如果没有STA鉴别成功*/
	if((pap->group_status ==1) && (pap->group_No==0))
	{
		DPrintf("tell driver the gkeyid although now there is no any STA associated with AP\n");
		/*安装组播密钥*/
		set_mcastkey(pap, &pap->msksa);
	}
	return;
}


/*检查组播更新结果*/
static int check_groupkey_response_result(apdata_info *pap)
{
	if(pap==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s:	pap==NULL\n",__func__);
		return -1;
	}
	
	struct wapid_interfaces *user=(struct wapid_interfaces *)pap->user_data;
	struct asd_wapi *wapi_wasd=NULL;
	struct asd_data * wasd=NULL;
	struct sta_info *sta=NULL;
	int k=0;

	if(user==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s:	user==NULL\n",__func__);
		return -1;
	}

	wapi_wasd=(struct asd_wapi *)user->circle_save;

	if(wapi_wasd==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s:	wapi_wasd==NULL\n",__func__);
		return -1;
	}

	wasd=(struct asd_data *)wapi_wasd->wasd_bk;

	if(wasd==NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s:	wasd==NULL\n",__func__);
		return -1;
	}

	for(sta=wasd->sta_list;sta!=NULL;sta=sta->next){
		if(sta->wapi_sta_info.status== MT_GROUPNOTICEING
			||sta->wapi_sta_info.status== MT_SESSIONGROUPING
			||sta->wapi_sta_info.status == MT_WAITING_DYNAMIC_GROUPING){
			k = 1;
			break;
		}
	}
	/*
	int i, k;
	for (i=k=0; i<MAX_AUTH_MT_SIMU; i++)
	{
		if (wapi_sta_info[i].status == MT_GROUPNOTICEING
			||wapi_sta_info[i].status == MT_SESSIONGROUPING
			||wapi_sta_info[i].status == MT_WAITING_DYNAMIC_GROUPING)
		{
			k = 1;	// some STA has noticed but not response
			break;
		}

	}
	return k;*/
	asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: k=%d\n",__func__,k);
	return k;/*xumin add*/
}
/*向Driver通知组播更新结果*/
int notify_groupnotice_to_apdriver(apdata_info *pap)
{
	/*检查是否所有asue组播通告都已经完成*/
	if (check_groupkey_response_result(pap) == 0)
	{
		/*安装组播密钥*/
		set_mcastkey(pap, &pap->msksa);
	}
	return 0;
}
int send_bk_to_sta(struct auth_sta_info_t *wapi_sta_info)
{
	struct ethhdr eh;
	int sendlen = 0, offset = 0;
	packet_head head;
	int packetlen = 0;
	u8 sendto_asue[FROM_MT_LEN];
	
	memset((u8 *)&head, 0, sizeof(packet_head));
	head.version      = VERSIONNOW;
	head.type = WAI;
	head.sub_type= SENDBKTOSTA;
	head.reserved     = RESERVEDDEF;
	head.data_len     = 0x0000;
	head.group_sc =0x0000;
	head.flag = 0x00;
	
	offset = c_pack_packet_head(&head,sendto_asue,offset,FROM_AS_LEN);
	offset = c_pack_16bytes( wapi_sta_info->bksa.bk,sendto_asue,offset,FROM_AS_LEN);


	set_packet_data_len(sendto_asue,offset);
	packetlen = offset;

	memcpy(eh.h_dest, wapi_sta_info->mac, ETH_ALEN);

	sendlen = send_rs_data(sendto_asue, packetlen, &eh, wapi_sta_info->pap);

	if(sendlen != packetlen )
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"(sendlen != sendtoMT_len)\n");

		DPrintf("fail to send_bk_to_sta,sendtoMT_len=%d,sendlen=%d \n",packetlen,sendlen);
		//return -3;
	}
	return 0;
}
/*初始化ECDH算法参数*/
int wai_initialize_ecdh(para_alg *ecdh)
{
	char alg_para_oid_der[16] = {0x06, 0x09,0x2a,0x81,0x1c, 0xd7,0x63,0x01,0x01,0x02,0x01};
	
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s\n",__func__);
	memset((u8 *)ecdh, 0, sizeof(para_alg));
	ecdh->para_flag= 1;
	ecdh->para_len = 11;
	memcpy(ecdh->para_data, alg_para_oid_der, 11);
	return 0;
}

int wai_copy_ecdh(para_alg *ecdh_a, para_alg *ecdh_b)
{
	memset((u8 *)ecdh_a, 0, sizeof(para_alg));
	
	ecdh_a->para_flag = ecdh_b->para_flag;
	ecdh_a->para_len = ecdh_b->para_len;
	memcpy(ecdh_a->para_data, ecdh_b->para_data, ecdh_b->para_len);
	return 0;
}

int wai_compare_ecdh(para_alg *ecdh_a, para_alg *ecdh_b)
{
	if(ecdh_a == NULL || ecdh_b == NULL) {
		return -1;
	}else if((ecdh_a->para_flag!= ecdh_b->para_flag) || 
			(ecdh_a->para_len !=  ecdh_b->para_len)|| 
			(memcmp(ecdh_a->para_data, ecdh_b->para_data,  ecdh_b->para_len)!=0)){
		return -1;
	}
	return 0;
}

