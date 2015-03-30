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
* AsdCertAuth.c
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include "include/openssl/ec.h"
#include "include/openssl/obj_mac.h"

#include "common.h"

#include "asd.h"
#include "ASDStaInfo.h"
#include "ASDCallback.h"

#include "wcpss/asd/asd.h"
#include "include/proc.h"
#include "include/key_neg.h"
//#include "include/typedef.h"
#include "include/pack.h"
#include "include/debug.h"
#include "include/raw_socket.h"
#include "include/wai_sta.h"
#include "include/alg_comm.h"
#include "include/certupdate.h"
#include "include/structure.h"
#include "include/auth.h"
#include "include/cert_auth.h"
#include "include/ecc_crypt.h"
#include "include/cgi_server.h"
#include "wcpss/asd/asd.h"	/*xm add*/
#include "asd_bak.h"

#include "ASD8021XOp.h"
#include "ASD80211Op.h"
#include "ASDBeaconOp.h"
#include "ASDHWInfo.h"
#include "ASDAccounting.h"
#include "ASDEapolSM.h"
#include "circle.h"
#include "ASDDbus.h"

static const unsigned char	Input_text[100] = "multicast or station key expansion for station unicast and multicast and broadcast";

//	xm06301
//static int rekey_mcastkey_tlimit(apdata_info *pap,void *p);

static void display_wapi_sta_status(struct auth_sta_info_t *wapi_sta_info);

static int wai_fixdata_cert_by_certificate(const struct cert_obj_st_t *cert_obj, 
										wai_fixdata_cert *fixdata_cert,
										u16 index);
/*构造并发送鉴别激活分组*/
static int ap_activate_sta(struct auth_sta_info_t *wapi_sta_info);
/*使用证书鉴别与密钥管理套间是的BK导出方法*/
static int ap_certauthbk_derivation(struct auth_sta_info_t *wapi_sta_info);
/*使用PSK鉴别与密钥管理套间是的BK导出方法*/
static void ap_pskbkid_derivation(struct auth_sta_info_t *wapi_sta_info);

/*create wai fix data of cert by inputing certificate data */
static int wai_fixdata_cert_by_certificate(const struct cert_obj_st_t *cert_obj, 
										wai_fixdata_cert *fixdata_cert,
										u16 index)
{
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	if((fixdata_cert==NULL)||(cert_obj == NULL)) return -1;
	fixdata_cert->cert_flag =index; /*GBW*/
	fixdata_cert->cert_bin.data = cert_obj->cert_bin->data;
	fixdata_cert->cert_bin.length = cert_obj->cert_bin->length;
	return 0;

}

int wai_fixdata_id_by_ident(void *cert_st, 
								void *cert,wai_fixdata_id *fixdata_id,
								u16 index)
{
	u8 *temp ;
	byte_data		*subject_name = NULL;
	byte_data		*issure_name = NULL;
	byte_data		*serial_no = NULL;
	struct cert_obj_st_t *cert_obj = cert;

	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	if(fixdata_id == NULL) return -1;
	temp= fixdata_id->id_data;
	fixdata_id->id_flag = index;
	/*according to the index, can call external objs if want to process asue id at the 
	same time*/
	if(cert_st == NULL) 
	{
		asd_printf(ASD_WAPI,MSG_ERROR,"cert_st is NULL\n");
		return -1;
	}
//	cert_obj = get_cert_obj(index);
	if((cert_obj == NULL)
		||(cert_obj->obj_new == NULL)
		||(cert_obj->obj_free == NULL)
		||(cert_obj->decode == NULL)
		||(cert_obj->encode == NULL)
		||(cert_obj->cmp == NULL)
		||(cert_obj->get_public_key == NULL)
		||(cert_obj->get_subject_name == NULL)
		||(cert_obj->get_issuer_name == NULL)
		||(cert_obj->get_serial_number == NULL)
		||(cert_obj->verify_key == NULL)
		||(cert_obj->sign == NULL)
		||(cert_obj->verify == NULL))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"Not found cert_obj\n");
		return -4;
	}
	subject_name = (*cert_obj->get_subject_name)(cert_st);
	issure_name = (*cert_obj->get_issuer_name)(cert_st);
	serial_no = (*cert_obj->get_serial_number)(cert_st);

	if((subject_name == NULL) || (issure_name == NULL) || (serial_no == NULL) )
	{
		return -2;
	}
	memcpy(temp, subject_name->data, subject_name->length);
	temp += subject_name->length;

	memcpy(temp, issure_name->data, issure_name->length);
	temp += issure_name->length;

	memcpy(temp, serial_no->data, serial_no->length);
	temp +=serial_no->length;

	fixdata_id->id_len = temp - fixdata_id->id_data;
	free_buffer(subject_name,sizeof(byte_data));
	free_buffer(issure_name,sizeof(byte_data));
	free_buffer(serial_no,sizeof(byte_data));
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s success.\n",__func__);
	return 0;
}

/*构造并发送鉴别激活分组*/
static int ap_activate_sta(struct auth_sta_info_t *wapi_sta_info)
{
	u8 sendto_asue[FROM_MT_LEN];
   	int sendtoMT_len = 0;
	int sendlen = 0;
	struct ethhdr eh;
	auth_active  auth_active_info;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	apdata_info *pap;

	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	if(wapi_sta_info == NULL) return -1;
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s (wapi_sta_info != NULL)\n",__func__);
	
	pap= wapi_sta_info->pap;
	if(pap == NULL) return -1;
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s (pap != NULL)\n",__func__);
	
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s (wapid != NULL)\n",__func__);
	
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s (tmp_circle != NULL)\n",__func__);
	
	memset(&auth_active_info, 0, sizeof(auth_active));
	if((tmp_circle->has_cert != 1)) return -1;
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s (tmp_circle->has_cert == 1)\n",__func__);
	
	memcpy(eh.h_dest, wapi_sta_info->mac, ETH_ALEN);
	if(wapi_sta_info->bksa.bk_update)
	{
		auth_active_info.flag |= WAI_FLAG_BK_UPDATE;
		memcpy(auth_active_info.ae_auth_flag, wapi_sta_info->bksa.ae_auth_flag, 32);
	}else{
		get_random((u8 *)(&auth_active_info.ae_auth_flag ), 32);
	}

	/*从AE的证书中取的ASU的身份字段*/
	if(wai_fixdata_id_by_ident(tmp_circle->cert_info.ap_cert_obj->asu_cert_st, 
							tmp_circle->cert_info.ap_cert_obj,
							&(auth_active_info.asu_id),
							tmp_circle->cert_info.config.used_cert)!=0)
	{
		return -1;
	}

	wai_fixdata_cert_by_certificate(tmp_circle->cert_info.ap_cert_obj,
								&(auth_active_info.ae_cer),
								tmp_circle->cert_info.config.used_cert);
	wai_initialize_ecdh(&auth_active_info.ecdh);
	
	sendtoMT_len = pack_auth_active(&auth_active_info, sendto_asue,  FROM_MT_LEN);

	if((u16)sendtoMT_len == PACK_ERROR) 	
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"pack_auth_active PACK_ERROR");
		return -1;
	}

	/*设置flag和分组序号等*/
	*((u16 *)(sendto_asue + 8)) = htons(wapi_sta_info->ae_group_sc);
	wapi_sta_info->flag &= 0xFC;/*0xFC:11111100*//*先将0和1位清零*/
	wapi_sta_info->flag |= (auth_active_info.flag & 0x03);/*bit0:bk_update, bit1:pre_auth */
	memcpy(wapi_sta_info->bksa.ae_auth_flag , auth_active_info.ae_auth_flag, 32);
	wai_copy_ecdh(&wapi_sta_info->ecdh,&auth_active_info.ecdh);
	set_table_item(wapi_sta_info, SENDTO_STA, 0, sendto_asue, sendtoMT_len);
	circle_cancel_timeout(wapi_retry_timer, pap, wapi_sta_info);
	circle_register_timeout(1, 0, wapi_retry_timer, pap, wapi_sta_info);
	asd_printf(ASD_WAPI,MSG_DEBUG,"register wapi_retry_timer after send ap active sta\n");

	sendlen = send_rs_data(sendto_asue, sendtoMT_len, &eh, pap);

	if(sendlen != sendtoMT_len) 
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"(sendlen != sendtoMT_len)\n");
		//return -1;
	}
	wapi_sta_info->ae_group_sc += 1;
	if(wapi_sta_info->status == NO_AUTH)
	{
		wapi_sta_info->status = MT_WAITING_ACCESS_REQ;
	}
	asd_printf(ASD_WAPI,MSG_DEBUG,"ap_activate_sta\n");
	return 0;
}

/*使用证书鉴别与密钥管理套间是的BK导出方法*/
static int ap_certauthbk_derivation(struct auth_sta_info_t *wapi_sta_info)
{
	apdata_info *pap ;
	char  input_text[100] = "base key expansion for key and additional nonce";
	u8 text[200] = {0,};
	u8 temp_out[48] = {0,};
	u8  ecdhkey[24] = {0,};
	int  ecdhkeyl = 0;
	int ret = -1;
	
	EC_KEY *ae_eck = NULL;	
	EC_KEY  *asue_eck = NULL;

	if(wapi_sta_info== NULL) goto err;
	pap= wapi_sta_info->pap;
		
	ae_eck = geteckey_by_curve(NID_X9_62_prime192v4);

	if(ae_eck == NULL){
		goto err;
	} else if (!EC_KEY_generate_key(ae_eck)){
		goto err;
	}
	
	wapi_sta_info->ae_key_data.length = point2bin(ae_eck, (u8 *)wapi_sta_info->ae_key_data.data, MAX_BYTE_DATA_LEN);

	asue_eck = octkey2eckey((u8 *)wapi_sta_info->asue_key_data.data, wapi_sta_info->asue_key_data.length, NULL, 0, NID_X9_62_prime192v4);
	
	ecdhkeyl  = ECDH_compute_key(ecdhkey, 24,EC_KEY_get0_public_key(asue_eck), ae_eck, NULL);
		
	assert(ecdhkeyl >= 0);
		
	/*计算BK*/
	memset(text, 0, 200);
	memcpy(text, wapi_sta_info->ae_nonce, 32);
	memcpy(text + 32, wapi_sta_info->asue_nonce, 32);
	memcpy(text + 32 + 32, input_text, strlen(input_text));
	KD_hmac_sha256(text, 32+32+strlen(input_text), 
						ecdhkey, ecdhkeyl,
						temp_out, 16 + 32);
	memcpy(wapi_sta_info->bksa.bk, temp_out, 16);
	/*计算BKID*/
	memset(text, 0, 200);
	memcpy(text, pap->macaddr, WLAN_ADDR_LEN);
	memcpy(text + WLAN_ADDR_LEN, wapi_sta_info->mac, WLAN_ADDR_LEN);
	KD_hmac_sha256(text, 12,
						wapi_sta_info->bksa.bk, 16, 
						wapi_sta_info->bksa.bkid, 16);
	/*计算下一次鉴别标识*/
	mhash_sha256(temp_out + 16, 32, wapi_sta_info->bksa.ae_auth_flag);
	ret = 0;
err:	
	if(ae_eck)
		EC_KEY_free(ae_eck);
	if(asue_eck)
		EC_KEY_free(asue_eck);
	return ret;
}

/*使用PSK鉴别与密钥管理套间时间的BKID导出方法*/
static void ap_pskbkid_derivation(struct auth_sta_info_t *wapi_sta_info)
{
	apdata_info *pap ;
	u8 input_text[12] = {0,};
	if(wapi_sta_info == NULL) return;
	pap= wapi_sta_info->pap;
	
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	/*计算BKID*/
	memcpy(input_text,pap->macaddr, 6);
	memcpy(input_text + 6, wapi_sta_info->mac, 6);
	memcpy(wapi_sta_info->bksa.bk , pap->psk.bk, 16);

	KD_hmac_sha256(input_text, 12, 
					pap->psk.bk, 16, 
					wapi_sta_info->bksa.bkid, 16);
#if 1	
	DPrint_string_array("AP MAC:",pap->macaddr, 6);
	DPrint_string_array("STA MAC:",wapi_sta_info->mac, 6);
	DPrint_string_array("input_text", input_text, 12);
	DPrint_string_array("bk", wapi_sta_info->bksa.bk, 16);
	DPrint_string_array("bkid ", wapi_sta_info->bksa.bkid, 16);
#endif
}
/*使用PSK鉴别与密钥管理套间时间的BK导出方法*/
void ap_pskbk_derivation(apdata_info *pap)
{
	char input_text[200] = "preshared key expansion for authentication and key negotiation";
	struct wapid_interfaces *wapid ;
	if(pap==NULL) return;
	wapid = pap->user_data;
	/*计算BK*/

	KD_hmac_sha256((u8 *)input_text, strlen(input_text), wapid->password, wapid->password_len, 	pap->psk.bk, 16);

	DPrint_string_array("BK", pap->psk.bk, 16);
	DPrint_string_array("password",wapid->password, wapid->password_len);
}
/*单播密钥导出方法*/
void ae_usk_derivation(struct auth_sta_info_t *wapi_sta_info, session_key_neg_response *key_res_buff)
{
	const char  Input_text[100] = "pairwise key expansion for unicast and additional keys and nonce"; 
	u8 		 master_key[KD_HMAC_OUTKEY_LEN] = {0};
	u8			text[200] ;/*ADDID||N1||N2||"pairwise key expansion for unicast and additional keys and nonce"*/
	u8	 		uskid;
	int 			text_len;
	if((wapi_sta_info) == NULL || (key_res_buff == NULL)) return ;
	uskid = key_res_buff->uskid;//和AE比较??????
	memcpy(text, (u8 *)&(key_res_buff->addid), sizeof(wai_fixdata_addid));
	memcpy(text + sizeof(wai_fixdata_addid), wapi_sta_info->ae_nonce, 32);
	memcpy(text + sizeof(wai_fixdata_addid) + 32, wapi_sta_info->asue_nonce, 32);
	memcpy(text + sizeof(wai_fixdata_addid) + 32 + 32, Input_text, strlen(Input_text));
	text_len = sizeof(wai_fixdata_addid) + 32 + 32 + strlen(Input_text);

	KD_hmac_sha256(text, text_len, wapi_sta_info->bksa.bk, 16,
					master_key, KD_HMAC_OUTKEY_LEN);
	memcpy(wapi_sta_info->usksa.usk[uskid].uek, master_key, 16);
	memcpy(wapi_sta_info->usksa.usk[uskid].uck, master_key + 16, 16);
	memcpy(wapi_sta_info->usksa.usk[uskid].mck,master_key + 32, 16);
	memcpy(wapi_sta_info->usksa.usk[uskid].kek, master_key + 48, 16);
	mhash_sha256(master_key + 64, 32, wapi_sta_info->ae_nonce);
}
/*鉴别初始化*/
int auth_initiate(struct sta_info *sta, struct asd_data *wasd)
{
	int akm_no;
	int active = 0;
	int psk = 0;
	int i = 0;
	//BYTE mac[WLAN_ADDR_LEN]={0,};
	u8 akm_haha1[4] = {0x00, 0x14, 0x72, 0x01};/*WAI证书鉴别和密钥管理*/
	u8 akm_haha2[4] = {0x00, 0x14, 0x72, 0x02};/*WAI共享密钥鉴别和密钥管理*/
	u8 *wie = sta->wapi_sta_info.wie;
	struct auth_sta_info_t *wapi_sta_info = &(sta->wapi_sta_info);
	//memcpy(MT_mac, passo_mt_info->mac, WLAN_ADDR_LEN);

	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	if(wapi_sta_info == NULL)
	{
		return -1;
	}
	akm_no = ((wie[5]<<8)|wie[4]);
	for(i = 0; i < akm_no; i ++)
	{
		if(memcmp(akm_haha1, wie + 6 + i * 4, 4) == 0){
			/*ASUE使用证书鉴别与密钥管理套间*/
			active = 1;
		}else if(memcmp(akm_haha2, wie + 6 + i * 4, 4) == 0){
			/*ASUE使用PSK鉴别与密钥管理套间*/
			psk = 1;
		}	
	}
	/*记录ASUE的IE*/
	
	wapi_sta_info->bksa.akm_no = akm_no;
	memcpy(wapi_sta_info->bksa.akm, wie + 6 , akm_no * 4);
	if(active)
	{
		/*发送鉴别激活*/
		ap_activate_sta(wapi_sta_info);
	}
	else if (psk)
	{
		/*导出BK*/
		ap_pskbkid_derivation(wapi_sta_info);
		/*发送单播密钥协商请求*/
		usk_negotiation_req(wapi_sta_info);
	}
	return 0;
}


/*处理STA发送的帧
	a 接入鉴别请求分组
	b 密钥协商响应分组
	c 广播密钥通告响应分组
*/
int wapi_process_from_sta(u8 *read_asue, int readlen, u8 *mac, apdata_info *pap)
{
	int sendtoAS_len = 0;
	int sendlen = 0;
	int ret = 0;
	u8 buf[FROM_MT_LEN];
	u8 packID;
	u8  asue_group_sc;
	struct auth_sta_info_t *wapi_sta_info = NULL;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	struct asd_data *wasd;
	struct sta_info *sta;
	asd_printf(ASD_WAPI,MSG_DEBUG,"wapi_process_from_sta\n");
	if((pap == NULL) || (read_asue == NULL)|| (mac == NULL)||(readlen < 12)) return -1;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;

	wasd = (struct asd_data *)tmp_circle->wasd_bk;

	sta = ap_get_sta(wasd,mac);
	if(sta == NULL)
		return -1;
	wapi_sta_info = &(sta->wapi_sta_info);
	if (wapi_sta_info == NULL)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR" doesn't exist in the auth_table\n",MAC2STR(mac));
		return -1;
	}

	/* 获取WAI分组序号*/
	asue_group_sc = get_packet_group_sc(read_asue);
	
	/*分组序号错误,分组序号是递增的*/
	if(asue_group_sc < wapi_sta_info->asue_group_sc)
	{
		return -1;
	}
	
	packID = get_packet_sub_type(read_asue);
	asd_printf(ASD_WAPI,MSG_DEBUG,"Receive data from STA "MACSTR", packID = %u.\n",MAC2STR(mac),packID);
	display_wapi_sta_status(wapi_sta_info);
	asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :%02X\n",MAC2STR(wapi_sta_info->mac),wapi_sta_info->status);

	if((packID < 1) || (packID >0x10))
	{
		pap->wapi_mib_stats.wai_format_errors++;
		wapi_sta_info->wapi_mib_stats.wai_format_errors++;
		asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_format_errors++\n\n",MAC2STR(pap->macaddr));
		return -1;		
	}	

	switch(packID)
	{
		case STAAUTHREQUEST:
			asd_printf(ASD_WAPI,MSG_DEBUG,"received ACCESS REQUEST from sta:"MACSTR, MAC2STR(wapi_sta_info->mac));
			/*接入鉴别请求分组*/
			apdata_info *pap1 = sta->wapi_sta_info.pap;
			circle_cancel_timeout(wapi_retry_timer, pap1, wapi_sta_info);
			asd_printf(ASD_WAPI,MSG_DEBUG,"cancle wapi_retry_timer after receive ACCESS REQUEST\n");
			if(access_auth_req(wapi_sta_info, read_asue, readlen, buf, &sendtoAS_len) != 0)
			{
				return -1;
			}
			/*设置重发缓冲区*/
			set_table_item(wapi_sta_info, SENDTO_AS, 0, (u8 *)buf, sendtoAS_len);
			wapi_sta_info->sendinfo.timeout = 10;
			//circle_cancel_timeout(wapi_retry_timer, pap, wapi_sta_info);
			//circle_register_timeout(10, 0, wapi_retry_timer, pap, wapi_sta_info);
			asd_printf(ASD_WAPI,MSG_DEBUG,"%s SendtoAS data len = %d\n",__func__,sendtoAS_len);
			/*发送证书鉴别请求*/
			sendlen = sendto(tmp_circle->as_udp_sk, buf, sendtoAS_len,
							0, (struct sockaddr *)&(tmp_circle->as_addr), 
							sizeof(struct sockaddr_in));
			asd_printf(ASD_WAPI,MSG_DEBUG,"%s SendtoAS sendlen = %d\n",__func__,sendlen);
			if (sendlen != sendtoAS_len)
			{
				asd_printf(ASD_WAPI,MSG_INFO,"Fail to send to ASU\n");
				return -2;
			}			
			asd_printf(ASD_WAPI,MSG_DEBUG,"%s SendtoAS success\n",__func__);
			wapi_sta_info->status = MT_WAITING_AUTH_FROM_AS;
			break;
		/*单播密钥响应分组*/	
		case SESSIONKEYNEGRESPONSE:
			asd_printf(ASD_WAPI,MSG_DEBUG,"receive SESSION KEY NEG RESPONSE from sta:" MACSTR ",sta status=%d\n", 
				MAC2STR(wapi_sta_info->mac),wapi_sta_info->status);
			
			/*密钥协商响应分组*/
			ret = usk_negotiation_res(read_asue, readlen, wapi_sta_info);
			if (ret != 0)
			{
				asd_printf(ASD_WAPI,MSG_DEBUG,"process_dynamic_sessionkey Error\n ");
				return -3;
			}
			apdata_info *pap2 = sta->wapi_sta_info.pap;
			circle_cancel_timeout(wapi_retry_timer, pap2, wapi_sta_info);
			asd_printf(ASD_WAPI,MSG_DEBUG,"cancle wapi_retry_timer after receive USK NEGOTIATION RES\n");

			if (wapi_sta_info->status == MT_SESSIONKEYING)
			{
				/*单播密钥Rekey的过程中发生了组播密钥Rekey*/
				wapi_sta_info->status = MT_AUTHENTICATED;
				wapi_sta_info->usksa.dynamic_key_used = wapi_sta_info->usksa.uskid;
				wapi_sta_info->usksa.usk[wapi_sta_info->usksa.uskid].valid_flag = 1;/*valid*/
				wapi_sta_info->usksa.usk[wapi_sta_info->usksa.uskid^0x01].valid_flag = 0;

				/*发送密钥协商确认分组*/	
				usk_negotiation_confirmation(wapi_sta_info);

				/*安装STA 的UEK UCK*/
				set_ucastkey(pap, wapi_sta_info);
				if(is_secondary == 0)
					bak_add_sta(wasd,sta);
				//AsdStaInfoToWID(wasd, wapi_sta_info->mac, WID_ADD);
				break;
			}
			else if(wapi_sta_info->status == MT_SESSIONGROUPING)
			{
				/*组播密钥Rekey的过程中发生了单播密钥Rekey*/

				asd_printf(ASD_WAPI,MSG_DEBUG,"running in MT_SESSIONGROUPING status and waiting for group notice response!\n");
				wapi_sta_info->status  = MT_WAITING_DYNAMIC_GROUPING;
				usk_negotiation_confirmation(wapi_sta_info);
				set_ucastkey(pap, wapi_sta_info);
				break;				
			}
			else if(wapi_sta_info->status == MT_WAITING_DYNAMIC_SESSION)
			{
				/*单播REKEY中的密钥协商响应分组*/
				/*change keyid?*/
				asd_printf(ASD_WAPI,MSG_DEBUG,"running in MT_WAITING_DYNAMIC_SESSION status and will succeed!\n");
				wapi_sta_info->status = MT_AUTHENTICATED;
				wapi_sta_info->usksa.dynamic_key_used = wapi_sta_info->usksa.uskid;
				wapi_sta_info->usksa.usk[wapi_sta_info->usksa.uskid].valid_flag = 0;
				wapi_sta_info->usksa.usk[wapi_sta_info->usksa.uskid^0x01].valid_flag = 1;
				usk_negotiation_confirmation(wapi_sta_info);
				set_ucastkey(pap, wapi_sta_info);
				break;
			}
			else if(wapi_sta_info->status == MT_WAITING_SESSION)
			{
				if(wapi_sta_info->auth_mode == AUTH_MODE)
				{
					/*鉴别过程中的密钥协商响应分组*/
					asd_printf(ASD_WAPI,MSG_DEBUG,"SESSIONKEYNEGRESPONSE AUTH_MODE\n");

					usk_negotiation_confirmation(wapi_sta_info);
					sleep(2);
					msk_announcement_tx(wapi_sta_info, (u8 *)buf);	
				}else
				{
					/*预鉴别过程中的密钥协商响应分组*/

					asd_printf(ASD_WAPI,MSG_DEBUG,"SESSIONKEYNEGRESPONSE MT_PRE_AUTH_OVER\n");
					wapi_sta_info->status = MT_PRE_AUTH_OVER;
					//record current time and after 30 minutes it will be deleted
					wapi_sta_info->pre_auth_save_time = time(0);
				}
			}
			
			break;
		case GROUPKEYNOTICERESPONSE:
			/*组播通告响应分组*/			
			asd_printf(ASD_WAPI,MSG_DEBUG,"received GROUP KEY NOTICE RESPONSE from sta:"MACSTR, MAC2STR(wapi_sta_info->mac));

			ret = msk_announcement_res(wapi_sta_info, read_asue, readlen);
			if (ret == 0)
			{
				apdata_info *pap3 = sta->wapi_sta_info.pap;
				circle_cancel_timeout(wapi_retry_timer, pap3, wapi_sta_info);
				asd_printf(ASD_WAPI,MSG_DEBUG,"cancle wapi_retry_timer after receive MSK ANNOUNCEMENT RES\n");

				wasd->acc_tms++;		//mahz add 2011.4.2
				wasd->assoc_auth_sta_num++;		//mahz add 2011.11.9 for GuangZhou Mobile
				wasd->assoc_auth_succ_num++;
				sta->sta_assoc_auth_flag = 1;
				sta->flags |= WLAN_STA_AUTHORIZED;
				if (wapi_sta_info->status == MT_GROUPNOTICEING)
				{
					/*组播Rekey 过程中的组播通告响应分组*/
					wapi_sta_info->status = MT_AUTHENTICATED;
					wapi_sta_info->usksa.dynamic_key_used = wapi_sta_info->usksa.uskid;
					pap->group_No--;//record group notice no.
					display_wapi_sta_status(wapi_sta_info);
					return(notify_groupnotice_to_apdriver(pap));
				} 
				else if(wapi_sta_info->status == MT_WAITING_GROUPING)
				{
					/*一次鉴别过程中的组播通告响应分组*/
					wapi_sta_info->status = MT_AUTHENTICATED;
					wapi_sta_info->usksa.dynamic_key_used = wapi_sta_info->usksa.uskid;
					//send_auth_result_to_driver(sta_info, TRUE);
					asd_printf(ASD_WAPI,MSG_DEBUG,"receive GroupKey RESPONSE.  status=%d\n", wapi_sta_info->status);
					/*安装单播密钥*/
					set_ucastkey(pap, wapi_sta_info);					
					if(is_secondary == 0)
						bak_add_sta(wasd,sta);
					//AsdStaInfoToWID(wasd, wapi_sta_info->mac, WID_ADD);
				}
				else if(wapi_sta_info->status == MT_SESSIONGROUPING)
				{
					/*组播Rekey 的过程中发生了单播Rekey*/
					asd_printf(ASD_WAPI,MSG_DEBUG,"running in MT_SESSIONGROUPING status and waiting for session response!\n");
					wapi_sta_info->status = MT_WAITING_DYNAMIC_SESSION;
					/*组播通告计数递减*/
					pap->group_No--;//record group notice no.
					display_wapi_sta_status(wapi_sta_info);
					return(notify_groupnotice_to_apdriver(pap));
				}
				else if(wapi_sta_info->status == MT_WAITING_DYNAMIC_GROUPING)
				{
					/*单播Rekey的过程发生了组播Rekey*/
					asd_printf(ASD_WAPI,MSG_DEBUG,"running in MT_WAITING_DYNAMIC_GROUPING status and will succeed!\n");
					wapi_sta_info->status = MT_AUTHENTICATED;
					wapi_sta_info->usksa.dynamic_key_used = wapi_sta_info->usksa.uskid;
					pap->group_No--;//record group notice no.
					display_wapi_sta_status(wapi_sta_info);
					return(notify_groupnotice_to_apdriver(pap));
				}

				//mahz add 2010.11.24
				if((wasd->conf) && (wasd->conf->wapi_radius_auth_enable)) {
					asd_printf(ASD_WAPI,MSG_DEBUG,"wapi radius auth enable\n");

					if(wasd->eapol_auth){
						if (!sta->eapol_sm) {
							sta->eapol_sm = eapol_auth_alloc(wasd->eapol_auth, sta->addr,
											 sta->flags & WLAN_STA_PREAUTH,sta);
							if (!sta->eapol_sm)
								return 0;
						}
						circle_register_timeout(10, 0, wapi_radius_auth_send, wasd, sta);
					}
				}
				//else
				//	asd_printf(ASD_WAPI,MSG_DEBUG,"wapi radius auth disable\n");
			}
			break;
		default:			
			break;
	}
	display_wapi_sta_status(wapi_sta_info);
	asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :%02X\n",MAC2STR(wapi_sta_info->mac),wapi_sta_info->status);

	return 0;
}

/*处理AS发送的证书鉴别响应分组*/
int wapi_process_1_of_1_from_as(u8 *read_as, int readlen, apdata_info *pap)
{
	struct auth_sta_info_t *wapi_sta_info = NULL; 
	u8		packID;
	int hlen = sizeof(packet_head);
	int idlen = sizeof(wai_fixdata_addid);
	int auth_result = -3;

	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	if((pap == NULL) || (read_as == NULL)||(readlen < 12)) return -1;

	packID = get_packet_sub_type(read_as);
	
	if (packID != APAUTHRESPONSE)
	{
		pap->wapi_mib_stats.wai_format_errors++;
		asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_format_errors++\n\n",MAC2STR(pap->macaddr));
		return -1;/*报文错误*/
	}
	auth_result = certificate_auth_res(&wapi_sta_info, read_as, readlen, pap);
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s auth_result = %d\n",__func__,auth_result);

	if(wapi_sta_info == NULL){	//mahz add 2011.6.1
		asd_printf(ASD_WAPI,MSG_DEBUG,"wapi_sta_info is NULL\n");
		return -1;
	}

	switch(auth_result)
	{
		case 0:
		case -1:
		case -2:
			access_auth_res(wapi_sta_info, read_as + hlen + idlen, readlen - hlen - idlen);
			break;
		case -3:
			pap->wapi_mib_stats.wai_format_errors++;
			wapi_sta_info->wapi_mib_stats.wai_format_errors++;
			asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_format_errors++\n\n",MAC2STR(pap->macaddr));
			break;
		case -4:
			pap->wapi_mib_stats.wai_discard++;
			wapi_sta_info->wapi_mib_stats.wai_discard++;
			asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_discard++\n\n",MAC2STR(pap->macaddr));
			break;
		default:
			break;
	}

	if((auth_result == -1) ||(auth_result == -2  ))
	{
		usleep(10);
		notify_driver_disauthenticate_sta(wapi_sta_info, __func__, __LINE__);
		asd_printf(ASD_WAPI,MSG_DEBUG,"notify_driver_disauthenticate_sta");
	}
	else if(auth_result == 0)
	{
		/*构造并发送单播密钥协商请求*/
		usk_negotiation_req(wapi_sta_info);

	}
	return 0;
}

/*接入鉴别响应*/
int access_auth_res(struct auth_sta_info_t *wapi_sta_info,	
							u8 *temp,/*记录证书鉴别响应分组中的除了地址索引外的其他字段*/
							int temp_len )
{
	struct ethhdr eh;
	int sendlen = 0;
	u16 sign_len_pos = 0;
	u16 sign_val_len = 48;
	u16 offset = 0;
	big_data data_buff;
	tsign				sign;
	packet_head head;
	wai_fixdata_flag flag;
	u8 sendto_asue[FROM_AS_LEN];
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	apdata_info *pap ;

	if(wapi_sta_info == NULL) return -1;
	pap= wapi_sta_info->pap;

	if(pap == NULL) return -1;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
		
	memset((u8 *)&flag, 0, 1);
	memset((u8 *)&head, 0, sizeof(packet_head));
	head.version      = VERSIONNOW;
	head.type = WAI;
	head.sub_type= STAAUTHRESPONSE;
	head.reserved     = RESERVEDDEF;
	head.data_len     = 0x0000;
	head.group_sc = wapi_sta_info->ae_group_sc;
	
	flag =  wapi_sta_info->flag & 0x03;/*bit0:bk_update, bit1:pre_auth */

	if(wapi_sta_info->flag & WAI_FLAG_AUTH_CERT)/*bit2  ASUE 要求验证AE证书*/
		flag |= WAI_FLAG_OPTION_BYTE;/*bit3*/

	offset = c_pack_packet_head(&head,sendto_asue,offset,FROM_AS_LEN);   //xk debug:negative returns
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_byte((u8 *)&flag,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_32bytes((u8 *)&wapi_sta_info->asue_nonce,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_32bytes((u8 *)&wapi_sta_info->ae_nonce,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_byte(&wapi_sta_info->auth_result,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_byte_data(&wapi_sta_info->asue_key_data,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_byte_data(&wapi_sta_info->ae_key_data,sendto_asue,offset,FROM_AS_LEN);
    if( offset==PACK_ERROR ) return PACK_ERROR;
	/*AE身份*/
	offset = c_pack_word(&tmp_circle->ae_id.id_flag,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
    offset = c_pack_word(&tmp_circle->ae_id.id_len,sendto_asue,offset,FROM_AS_LEN);
    if( offset==PACK_ERROR ) return PACK_ERROR;
	memcpy(sendto_asue + offset, tmp_circle->ae_id.id_data, tmp_circle->ae_id.id_len);
	//c_pack_htons_id(sendto_asue + offset);/*转换身份字段中的长度*/
	offset += tmp_circle->ae_id.id_len;
	
	/*ASUE身份*/
	offset = c_pack_word(&wapi_sta_info->asue_id.id_flag,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_word(&wapi_sta_info->asue_id.id_len,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	memcpy(sendto_asue + offset, wapi_sta_info->asue_id.id_data, wapi_sta_info->asue_id.id_len);
	//c_pack_htons_id(sendto_asue + offset);
	offset +=  wapi_sta_info->asue_id.id_len;

	if(flag & WAI_FLAG_OPTION_BYTE)
	{
		/*copy复合得证书验证结果*/
		memcpy(sendto_asue + offset, temp, temp_len);
		offset += temp_len;
	}
	
	/*计算AE签名*/
	/*对上面的数据签名，如何确定鉴别载荷里的长度值*/
	data_buff.length = offset - sizeof(packet_head);
	memcpy(data_buff.data, sendto_asue + sizeof(packet_head), data_buff.length);/*????*/
	sign.length = (*tmp_circle->cert_info.ap_cert_obj->sign)(
		tmp_circle->cert_info.ap_cert_obj->private_key->data,
		tmp_circle->cert_info.ap_cert_obj->private_key->length,
		data_buff.data, 	data_buff.length, sign.data);

	assert(sign.length != 0);
	if(sign.length == 0)
	{
		asd_printf(ASD_WAPI,MSG_ERROR,"err in %s:%d\n", __func__, __LINE__);
		abort();
	}
	/*构造签名属性*********************************8*/
	*(sendto_asue + offset) = 1;	/*类型*/
	offset += 1;		
	sign_len_pos = offset;				
	offset += 2;/*length*/

	/*AE身份*/
	offset = c_pack_word(&tmp_circle->ae_id.id_flag,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_word(&tmp_circle->ae_id.id_len,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	memcpy(sendto_asue + offset, &tmp_circle->ae_id.id_data, tmp_circle->ae_id.id_len);
	offset += tmp_circle->ae_id.id_len;
	
	/*签名算法,没有参数*/
	offset = c_pack_sign_alg(&pap->sign_alg, sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_word((u16 *)&sign_val_len,sendto_asue,offset,FROM_AS_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	
	memcpy(sendto_asue + offset, sign.data, 48);
	offset += 48;

	
	//DPrint_string_array("sendto_asue", sendto_asue, offset);

	/*设置签名属性长度*/
	sendto_asue[sign_len_pos] = ((offset - sign_len_pos - 2)>>8)&0xff;
	sendto_asue[sign_len_pos+1] = ((offset - sign_len_pos - 2))&0xff;
/*构造签名属性*********************************8*/
	set_packet_data_len(sendto_asue,offset );
	//(*sendtoMT_len) = offset;
	
	/*设置重发缓冲区*/
	set_table_item(wapi_sta_info, SENDTO_STA, 0, sendto_asue, offset);
	//circle_cancel_timeout(wapi_retry_timer, pap, wapi_sta_info);
	//circle_register_timeout(1, 0, wapi_retry_timer, pap, wapi_sta_info);
	memcpy(eh.h_dest, wapi_sta_info->mac, ETH_ALEN);
	asd_printf(ASD_WAPI,MSG_DEBUG,"send_rs_data to sta\n");
	sendlen = send_rs_data(sendto_asue, offset, &eh, pap);
	wapi_sta_info->ae_group_sc += 1;
	return sendlen;
}

/*处理接入鉴别请求分组*/
int access_auth_req (struct auth_sta_info_t *wapi_sta_info, const u8 *read_asue, 
						int readlen, u8 *sendto_as, int *psendtoas_len)
{
	u16					no_signdata_len = 0;
	u16					offset_mt=0;
	u16					cert_type;
	big_data 					data_buff;
	sta_auth_request			sta_auth_requ;
	ap_auth_request 			ap_auth_req ;
	tsign						sign;
	tkey  					*pubkey = NULL;
	void 						*asue_cert = NULL;
	const struct cert_obj_st_t 	*cert_obj = NULL;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	int res =0;
	int ret =0;
	apdata_info *pap;

	byte_data *serial_number = NULL;		//mahz add 2010/11/17
	//int i=0,len=0,val=0;
	//unsigned char no[128]={0};

	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	if(wapi_sta_info== NULL) return -1;
	
	pap = wapi_sta_info->pap;
	if(pap == NULL) return -1;
	
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;

	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	
	memset(&sta_auth_requ, 0, sizeof(sta_auth_request));
	memset(&data_buff, 0, sizeof(big_data));
	memset(&sign, 0, sizeof(tsign));
	memset(&ap_auth_req, 0,sizeof(ap_auth_request) );
	
	display_wapi_sta_status(wapi_sta_info);
	asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :%02X\n",MAC2STR(wapi_sta_info->mac),wapi_sta_info->status);
	switch (wapi_sta_info->status )
	{	
		case MT_WAITING_AUTH_FROM_AS:/*重传帧*/
			/*AE接收到接入鉴别请求分组并向ASU发送证书鉴别请求分组后，
				在证书鉴别请求分组超时时间里不对ASUE发送得接入鉴别
				请求分组进行处理*/
			return -1;
		default :
			break;
	}
	/*对接入鉴别请求分组解包*/
	offset_mt = unpack_sta_auth_request(&sta_auth_requ, read_asue,  readlen,&no_signdata_len);
	if( offset_mt == PACK_ERROR )	
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"unpack_sta_auth_request error\n");
		pap->wapi_mib_stats.wai_format_errors++;
		wapi_sta_info->wapi_mib_stats.wai_format_errors++;
		asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_format_errors++\n\n",MAC2STR(pap->macaddr));
		return -1;		
	}

	switch (wapi_sta_info->status )
	{
		case MT_WAITING_ACCESS_REQ:
			if(memcmp(sta_auth_requ.ae_auth_flag , wapi_sta_info->bksa.ae_auth_flag, 32)!=0
				||(sta_auth_requ.flag & 0x03) != (wapi_sta_info->flag & 0x03))
			{
				/*比较鉴别标示,分组中的标示字段策略域是否相同*/
				asd_printf(ASD_WAPI,MSG_DEBUG,"ae_auth_flag ,bk_update,pre_auth not same\n");
				DPrint_string_array("current sta's ae_auth_flag ", sta_auth_requ.ae_auth_flag, 32);
				DPrint_string_array("saved sta's ae_auth_flag ",wapi_sta_info->bksa.ae_auth_flag, 32);
				asd_printf(ASD_WAPI,MSG_DEBUG,"current sta's flag.bk_update and pre auth= %d, saved =%d\n ",
					sta_auth_requ.flag & 0x03, wapi_sta_info->flag & 0x03);
				pap->wapi_mib_stats.wai_discard++;
				wapi_sta_info->wapi_mib_stats.wai_discard++;
				asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_discard++\n\n",MAC2STR(pap->macaddr));
				return -1;
			}
			break;
		
		default :
			if(memcmp(sta_auth_requ.ae_auth_flag, wapi_sta_info->bksa.ae_auth_flag,32) != 0)
			{
				/*AE没有发送鉴别激活分组*/
				DPrint_string_array("current sta's mac ",wapi_sta_info->mac, 6);
				asd_printf(ASD_WAPI,MSG_DEBUG,"\n");
				asd_printf(ASD_WAPI,MSG_DEBUG,"current status = %d\n", wapi_sta_info->status );
				pap->wapi_mib_stats.wai_discard++;
				wapi_sta_info->wapi_mib_stats.wai_discard++;
				asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_discard++\n\n",MAC2STR(pap->macaddr));
				return -1;
			}
			break;
	}
	/*比较AE身份*/
	if(memcmp((u8 *)&(sta_auth_requ.ae_id), (u8 *)&(tmp_circle->ae_id), tmp_circle->ae_id.id_len + 4) !=0)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"access_auth_req compare ae id\n");

		asd_printf(ASD_WAPI,MSG_DEBUG,"ae id is not same,id len =%d\n",tmp_circle->ae_id.id_len);
		DPrint_string_array("current ae id ",(u8 *)&(sta_auth_requ.ae_id), tmp_circle->ae_id.id_len + 4);
		DPrint_string_array("saved ae id ",(u8 *)&(tmp_circle->ae_id), tmp_circle->ae_id.id_len + 4);
		pap->wapi_mib_stats.wai_discard++;
		wapi_sta_info->wapi_mib_stats.wai_discard++;
		asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_discard++\n\n",MAC2STR(pap->macaddr));
		return -1;
	}
	/*比较ECDH*/
	if(wai_compare_ecdh(&sta_auth_requ.ecdh, &wapi_sta_info->ecdh)!=0)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"ecdh is not same\n");
		DPrint_string_array("current ecdh", (u8 *)&sta_auth_requ.ecdh, wapi_sta_info->ecdh.para_len + 3);
		DPrint_string_array("saved ae id", (u8 *)&(wapi_sta_info->ecdh), wapi_sta_info->ecdh.para_len + 3);
		pap->wapi_mib_stats.wai_discard++;
		wapi_sta_info->wapi_mib_stats.wai_discard++;
		asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_discard++\n\n",MAC2STR(pap->macaddr));
		return -1;
	}
	
	/*验证ASUE 签名*/
	data_buff.length = no_signdata_len - sizeof(packet_head);
	memcpy(data_buff.data, read_asue + sizeof(packet_head), data_buff.length);/*????*/

	/*取证书的类型 1:X509, 2:GBW*/
	cert_type = sta_auth_requ.asue_cert.cert_flag;
	if(cert_type != 1)
	{
		/*None X509证书*/
		asd_printf(ASD_WAPI,MSG_DEBUG,"none X509 certificate\n");
			return -2;
		}
	/*根据证书类型取证书中的公钥*/
	//cert_obj = get_cert_obj(cert_type);
	cert_obj = tmp_circle->cert_info.ap_cert_obj;
	if((cert_obj == NULL)
		||(cert_obj->obj_new == NULL)
		||(cert_obj->obj_free == NULL)
		||(cert_obj->decode == NULL)
		||(cert_obj->encode == NULL)
		||(cert_obj->cmp == NULL)
		||(cert_obj->get_public_key == NULL)
		||(cert_obj->get_subject_name == NULL)
		||(cert_obj->get_issuer_name == NULL)
		||(cert_obj->get_serial_number == NULL)
		||(cert_obj->verify_key == NULL)
		||(cert_obj->sign == NULL)
		||(cert_obj->verify == NULL))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"Not found cert_obj\n");
		return -4;
	}
	(*cert_obj->decode)(&asue_cert, sta_auth_requ.asue_cert.cert_bin.data, sta_auth_requ.asue_cert.cert_bin.length);
	pubkey = (*cert_obj->get_public_key)(asue_cert);
	if(pubkey == NULL) {
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s pubkey == NULL\n",__func__);
		return -4;
	}

	//mahz add for get serial number 2010/11/17
	serial_number = (*cert_obj->get_serial_number)(asue_cert);
	if(serial_number == NULL) {
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s serial_number == NULL\n",__func__);
		return -4;
	}
	if(convert_serial_no(&wapi_sta_info->serial_no,serial_number) != 0){
		asd_printf(ASD_WAPI,MSG_DEBUG," error in convert_serial_no\n");
	}
	//
	/*验证证书*/
	res = (*cert_obj->verify)(pubkey->data, pubkey->length,
							data_buff.data,data_buff.length,
							sta_auth_requ.asue_sign.sign_data.data, 
							sta_auth_requ.asue_sign.sign_data.length);
	if(!res)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"\nAE verify ASUE sign  error!!!,res = %d\n\n",res);
		ret = -1;
		pap->wapi_mib_stats.wai_sign_errors ++;
		wapi_sta_info->wapi_mib_stats.wai_sign_errors++;
		asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_sign_errors++\n\n",MAC2STR(pap->macaddr));
		pap->wapi_mib_stats.wai_discard++;
		wapi_sta_info->wapi_mib_stats.wai_discard++;
		asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_discard++\n\n",MAC2STR(pap->macaddr));
		goto errexit;
	}
	
	/*copy 密钥数据*/
	memcpy(wapi_sta_info->asue_nonce , sta_auth_requ.asue_challenge, 32);
	memcpy(wapi_sta_info->asue_key_data.data, sta_auth_requ.key_data.data, sta_auth_requ.key_data.length);
	wapi_sta_info->asue_key_data.length = sta_auth_requ.key_data.length;

	/*根据证书生成身份*/
	wai_fixdata_id_by_ident(asue_cert, tmp_circle->cert_info.ap_cert_obj,&(wapi_sta_info->asue_id),cert_type);

	/*记录ASUE 证书的hash值，用以和证书鉴别响应中的证书做对比*/
	mhash_sha256(sta_auth_requ.asue_cert.cert_bin.data, 
				sta_auth_requ.asue_cert.cert_bin.length,
				wapi_sta_info->asue_cert_hash);

	/*记录flag的比特2*/
	wapi_sta_info->flag &= 0xFB;/*0xFB:11111011*//*先将2位清零*/
	wapi_sta_info->flag |= sta_auth_requ.flag & 0x04;/*0x04:00000100*//*bit2:auth_cert*/

	/*根据本地鉴别策略构造证书鉴别请求发往ASU*/
	certificate_auth_req(&sta_auth_requ, wapi_sta_info, sendto_as, psendtoas_len);

	/*good negoration response and not nessary to resend negoration require */
	/*释放ASUE重发缓冲区*/
	reset_table_item(wapi_sta_info);
errexit:	
	/*释放存放公钥和证书内存*/
	if(pubkey != NULL)
	{
		pubkey = free_buffer(pubkey, sizeof(tkey));
	}
	if(asue_cert != NULL)
	{
		(*cert_obj->obj_free)(&asue_cert);
	}
	//mahz add 2010/11/17
	if(serial_number != NULL){
		serial_number = free_buffer(serial_number,sizeof(byte_data));
	}
	return ret;
}

int certificate_auth_req (const struct _sta_auth_request *sta_auth_requ, 
							struct auth_sta_info_t *wapi_sta_info,
							u8 *sendto_as, int *psendtoas_len) /*本地策略默认是通过ASU鉴别证书*/
{
	u16 offset=0;
	packet_head head;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	struct cert_obj_st_t *ap_cert_obj ;
	apdata_info *pap;
	int i;
	
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	if(wapi_sta_info== NULL) return -1;
	pap= wapi_sta_info->pap;
	if(pap == NULL) return -1;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	ap_cert_obj =tmp_circle 	->cert_info.ap_cert_obj;
	
	
	memset((u8 *)&head, 0, sizeof(packet_head));
	head.version      = VERSIONNOW;
	head.type =  WAI;
	head.sub_type= APAUTHREQUEST;
	head.reserved     = RESERVEDDEF;
	head.data_len     = 0x0000;
	head.group_sc	= 1;
	head.frame_sc = 0;

	/*构造分组头*/
	offset = c_pack_packet_head(&head,sendto_as,offset,FROM_MT_LEN);  //xk debug:negative returns
	if( offset==PACK_ERROR ) return PACK_ERROR;
	//DPrint_string_array("APAUTHREQUEST head", sendto_as, sizeof(packet_head));

	/*构造ADDID*/
	offset = pack_mac((u8 *)pap->macaddr, sendto_as, offset);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = pack_mac((u8 *)wapi_sta_info->mac, sendto_as, offset);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	
	/*构造AE和ASUE的挑战*/
	get_random(wapi_sta_info->ae_nonce , 32);
	smash_random(wapi_sta_info->ae_nonce, 32);
	offset = c_pack_32bytes(wapi_sta_info->ae_nonce, sendto_as, offset, FROM_MT_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_32bytes(wapi_sta_info->asue_nonce, sendto_as, offset, FROM_MT_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;

	/*ASUE certificate*/
	offset = c_pack_word(&sta_auth_requ->asue_cert.cert_flag,sendto_as ,offset,FROM_MT_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_word(&sta_auth_requ->asue_cert.cert_bin.length,sendto_as ,offset,FROM_MT_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	memcpy(sendto_as + offset, sta_auth_requ->asue_cert.cert_bin.data, sta_auth_requ->asue_cert.cert_bin.length);
	offset += sta_auth_requ->asue_cert.cert_bin.length;

	/*AE certificate*/
	asd_printf(ASD_WAPI,MSG_DEBUG,"process_access request:cert_index=%d\n",tmp_circle->cert_info.config.used_cert);
	//offset = c_pack_word(&APData.cert_info.config.used_cert,sendto_as ,offset,FROM_MT_LEN);
	offset = c_pack_word(&tmp_circle->cert_info.config.used_cert, sendto_as ,offset,FROM_MT_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	offset = c_pack_word(&ap_cert_obj->cert_bin->length,sendto_as,offset,FROM_MT_LEN);
	if( offset==PACK_ERROR ) return PACK_ERROR;
	memcpy(sendto_as + offset, ap_cert_obj->cert_bin->data, ap_cert_obj->cert_bin->length);
	offset += tmp_circle->cert_info.ap_cert_obj->cert_bin->length;
	//DPrint_string_array("ae cert", ap_cert_obj->cert_bin->data, ap_cert_obj->cert_bin->length);

	if(sta_auth_requ->asu_id_list.identifier == 3)
	{
		/*说明填充了ASUE信任的ASU列表*/
		
		offset = c_pack_byte((u8 *)&(sta_auth_requ->asu_id_list.identifier),
			               sendto_as, offset,FROM_MT_LEN);
		if( offset==PACK_ERROR ) return PACK_ERROR;
		
		offset = c_pack_word((u16 *)&(sta_auth_requ->asu_id_list.length),
		                                  sendto_as,offset,FROM_MT_LEN);
		if( offset==PACK_ERROR ) return PACK_ERROR;
			  
		offset = c_pack_byte((u8 *)&(sta_auth_requ->asu_id_list.rev),
		               sendto_as, offset,FROM_MT_LEN);
		if( offset==PACK_ERROR ) return PACK_ERROR;
		
       	offset = c_pack_word((u16 *)&(sta_auth_requ->asu_id_list.id_no),
	                                  sendto_as,offset,FROM_MT_LEN);
		if( offset==PACK_ERROR ) return PACK_ERROR;

	   	for(i=0;i<sta_auth_requ->asu_id_list.id_no;i++)
	   	{

	   	//身份标识
		 offset = c_pack_word((u16 *)&(sta_auth_requ->asu_id_list.id[i].id_flag),
                                                   sendto_as,offset,FROM_MT_LEN);
		 if( offset==PACK_ERROR ) return PACK_ERROR;
		//身份长度
		 offset = c_pack_word((u16 *)&(sta_auth_requ->asu_id_list.id[i].id_len),
	                                                sendto_as,offset,FROM_MT_LEN);
		 if( offset==PACK_ERROR ) return PACK_ERROR;

		//身份数据
		 offset = c_pack_nbytes((u8 *)&(sta_auth_requ->asu_id_list.id[i].id_data), 
		 	                                sta_auth_requ->asu_id_list.id[i].id_len, sendto_as,
		 	                                offset,FROM_MT_LEN);	
	    if( offset==PACK_ERROR ) return PACK_ERROR;
		}
		   
		//offset += sta_auth_requ->asu_id_list.id_no * 4 + 6;
	}
	
	assert(offset !=PACK_ERROR);
	/*设置包的长度*/
	set_packet_data_len(sendto_as, offset);
	*psendtoas_len = offset;
	return 0;
}	

/*处理证书鉴别响应*/
int certificate_auth_res(struct auth_sta_info_t **sta_info0, u8 *read_as, int readlen , apdata_info *pap)//???
{
	//char			*cert_invalid = "2";
	u8			errorcode_toASUE=0;
	u8			errorcode_toAE=0;
	u8 			hash_value[32]={0,};

	int 			auth_error = 0;
	u16 			no_signdata_len = 0;
	u16			no_signdata_len1 = 0;
	u16 			cert_type;
	u16 			offset = 0;	
	ap_auth_response	auth_res_buff;
	big_data			data_buff;
	tkey 				*pubkey = NULL;
	wai_fixdata_id 		as_id,ca_id;
	struct auth_sta_info_t *wapi_sta_info = NULL;
	const struct cert_obj_st_t  *cert_obj = NULL;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	int use_ca_cert=0;
	struct sta_info *sta = NULL;
	struct asd_data *wasd = NULL;
	
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s\n",__func__);
	if(pap == NULL) return -1;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	wasd = (struct asd_data *)tmp_circle->wasd_bk;
	memset(&auth_res_buff, 0, sizeof(ap_auth_response));
	memset(&data_buff, 0, sizeof(big_data));
	
	/*解包证书鉴别响应分组*/
	offset = unpack_ap_auth_response(&auth_res_buff,  read_as,  readlen, 
									&no_signdata_len, &no_signdata_len1);
	if(offset == PACK_ERROR) 
	{
		auth_error = -1;
		asd_printf(ASD_WAPI,MSG_DEBUG,"unpack_ap_auth_response :PACK_ERROR\n");
		return -3;
	}
	/*查找STA*/
	sta = ap_get_sta(wasd,auth_res_buff.addid.mac2);
	if(sta == NULL)
		return -4;
		
	wapi_sta_info = &sta->wapi_sta_info;
	if(wapi_sta_info == NULL) 
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"process_accessauthentificate_ans:not find STA");
		return -4;
	}
	*sta_info0 = wapi_sta_info;
	/*检查ASUE的状态是否为等待证书鉴别响应*/
	if(wapi_sta_info->status != MT_WAITING_AUTH_FROM_AS)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"process_accessauthentificate_ans: not MT_WAITING_AUTH_FROM_AS\n");
		return -4;
	}
	
	/*比较AE的挑战*/
	if(memcmp(wapi_sta_info->ae_nonce,auth_res_buff.cert_result.cert_result.ae_challenge,32)!=0)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"ae_nonce not same\n");
		DPrint_string_array("current:", auth_res_buff.cert_result.cert_result.ae_challenge,32);
		DPrint_string_array("saved:",wapi_sta_info->ae_nonce,32);
		return -4;
	}
	
	/*检查是不是来自AE信任的服务器*/
	memset(&as_id, 0, sizeof(wai_fixdata_id ));
	memset(&ca_id, 0, sizeof(wai_fixdata_id ));
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s Get ASU's identify.\n",__func__);
	if(wai_fixdata_id_by_ident(tmp_circle->cert_info.ap_cert_obj->asu_cert_st, 
							tmp_circle->cert_info.ap_cert_obj,
							&(as_id), tmp_circle->cert_info.config.used_cert)!=0)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s Get ASU's identify fail.\n",__func__);
		return -4;
	}
	//三证书 获取CA 身份
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s Get CA's identify.\n",__func__);
	if((tmp_circle->multi_cert) && (wai_fixdata_id_by_ident(tmp_circle->cert_info.ap_cert_obj->ca_cert_st, 
							tmp_circle->cert_info.ap_cert_obj,
							&(ca_id), tmp_circle->cert_info.config.used_cert)!=0))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s Get CA's identify fail.\n",__func__);
		return -4;
	}
	

	/*比较ASU的身份*/
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s Compare ASU's identify.\n",__func__);
	if((auth_res_buff.cert_result.ae_trust_asu_sign.id.id_len != as_id.id_len)||
		(memcmp(auth_res_buff.cert_result.ae_trust_asu_sign.id.id_data,
			as_id.id_data, as_id.id_len)!=0))
	{
     	//比较CA的身份－三证书
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s Compare CA's identify.\n",__func__);
		if((tmp_circle->multi_cert) && (auth_res_buff.cert_result.ae_trust_asu_sign.id.id_len == ca_id.id_len)&&
			(memcmp(auth_res_buff.cert_result.ae_trust_asu_sign.id.id_data,
				ca_id.id_data, ca_id.id_len)==0))
		{	       
			use_ca_cert=1;
		}
		else
      	{
			asd_printf(ASD_WAPI,MSG_DEBUG,"not trust ca\n");
			DPrint_string_array("ASU ",auth_res_buff.cert_result.ae_trust_asu_sign.id.id_data, 
				auth_res_buff.cert_result.ae_trust_asu_sign.id.id_len);
			DPrint_string_array("CA ",ca_id.id_data, ca_id.id_len);
			return -4;
	  	}
	}
	asd_printf(ASD_WAPI,MSG_DEBUG,"%s use_ca_cert = %d\n",__func__,use_ca_cert);
	
	/*HASH 鉴别响应分组中ASUE证书*/
	mhash_sha256(auth_res_buff.cert_result.cert_result.cert1.cert_bin.data, 
				auth_res_buff.cert_result.cert_result.cert1.cert_bin.length, hash_value);
	/*和当前ASUE 证书的hash做对比，检查是不是正确的ASUE证书*/
	if(memcmp(wapi_sta_info->asue_cert_hash, hash_value, 32)!=0)
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"receive auth response with mismatch asue cert\n");
		return -4;
	}
	
	/*检查签名算法OID*/
	if(cmp_oid(auth_res_buff.cert_result.ae_trust_asu_sign.alg.sign_para.para_data,
		auth_res_buff.cert_result.ae_trust_asu_sign.alg.sign_para.para_len)!=0){
		return -4;
	}
	/*验证AS签名*/
	data_buff.length = no_signdata_len1 - sizeof(packet_head) - sizeof(wai_fixdata_addid);
	memcpy(data_buff.data, read_as + sizeof(packet_head) + sizeof(wai_fixdata_addid), data_buff.length);

	cert_type = tmp_circle->cert_info.config.used_cert;
//	cert_obj = get_cert_obj(cert_type);
	cert_obj = tmp_circle->cert_info.ap_cert_obj;

	if((cert_obj == NULL)
		||(cert_obj->obj_new == NULL)
		||(cert_obj->obj_free == NULL)
		||(cert_obj->decode == NULL)
		||(cert_obj->encode == NULL)
		||(cert_obj->cmp == NULL)
		||(cert_obj->get_public_key == NULL)
		||(cert_obj->get_subject_name == NULL)
		||(cert_obj->get_issuer_name == NULL)
		||(cert_obj->get_serial_number == NULL)
		||(cert_obj->verify_key == NULL)
		||(cert_obj->sign == NULL)
		||(cert_obj->verify == NULL))
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"Not found cert_obj\n");
		return -4;
	}

	if(use_ca_cert)    //用CA证书验证
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s Verify with CA cert\n",__func__);
		
		pubkey = (*cert_obj->get_public_key)(tmp_circle->cert_info.ap_cert_obj->ca_cert_st);

	    if(!(*cert_obj->verify)(pubkey->data, pubkey->length,
								data_buff.data,data_buff.length,
								auth_res_buff.cert_result.ae_trust_asu_sign.sign_data.data, 
								auth_res_buff.cert_result.ae_trust_asu_sign.sign_data.length))
		{
			free_buffer(pubkey, sizeof(tkey));
			asd_printf(ASD_WAPI,MSG_DEBUG,"CA sign error!!!\n");
			errorcode_toASUE = CERTIFSIGNERR;
			auth_error = -1;
			pap->wapi_mib_stats.wai_sign_errors ++;
			wapi_sta_info->wapi_mib_stats.wai_sign_errors ++;
			return -4;
		}
		free_buffer(pubkey, sizeof(tkey));

	}
	else          //用asu证书验证
	{
		asd_printf(ASD_WAPI,MSG_DEBUG,"%s Verify with ASU cert\n",__func__);
        pubkey = (*cert_obj->get_public_key)(tmp_circle->cert_info.ap_cert_obj->asu_cert_st);

		if(!(*cert_obj->verify)(pubkey->data, pubkey->length,
								data_buff.data,data_buff.length,
								auth_res_buff.cert_result.ae_trust_asu_sign.sign_data.data, 
								auth_res_buff.cert_result.ae_trust_asu_sign.sign_data.length))
		{
			free_buffer(pubkey, sizeof(tkey));
			asd_printf(ASD_WAPI,MSG_DEBUG,"ASUE sign error!!!\n");
			errorcode_toASUE = CERTIFSIGNERR;
			auth_error = -1;
			pap->wapi_mib_stats.wai_sign_errors ++;
			wapi_sta_info->wapi_mib_stats.wai_sign_errors ++;
			asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_sign_errors++\n\n",MAC2STR(pap->macaddr));
			return -4;
		}
		free_buffer(pubkey, sizeof(tkey));
	}
	errorcode_toASUE = auth_res_buff.cert_result.cert_result.auth_result1; 
	errorcode_toAE = auth_res_buff.cert_result.cert_result.auth_result2;

	wapi_sta_info->auth_result = errorcode_toASUE;
	if((errorcode_toASUE == 0) && (errorcode_toAE == 0))
	{
		asd_printf(ASD_WAPI,MSG_INFO,"STA & AP authenticated \n");
		ap_certauthbk_derivation(wapi_sta_info);
		//update_cert_status(tmp_circle->fileconfig) ;
	}
	else{
		auth_error = -2;
		asd_printf(ASD_WAPI,MSG_INFO,"STA  unauthenticated \n");
		pap->wapi_mib_stats.wai_auth_res_fail++;
		wapi_sta_info->wapi_mib_stats.wai_auth_res_fail++;
		asd_printf(ASD_WAPI,MSG_DEBUG,MACSTR" wapi_mib_stats.wai_auth_res_fail++\n\n",MAC2STR(pap->macaddr));
		
		/*ht add 090810*/
		/*SNMPD_ATTACK_INVALID_CERT*/
		if(tmp_circle != NULL && tmp_circle->wasd_bk != NULL){
			
				signal_wapi_trap(wapi_sta_info->mac,tmp_circle->wasd_bk->BSSIndex,ATTACK_INVALID_CERT);
		}
		if(errorcode_toAE !=0){
			asd_printf(ASD_WAPI,MSG_DEBUG,"save_cert_status\n");
			//save_cert_status(tmp_circle->fileconfig, cert_invalid);
		}
	}
	reset_table_item(wapi_sta_info);
	
	if(1)
	{
		switch(errorcode_toASUE) 
		{
		case CERTIFVALID:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate valid!\n");
			break;
		case CERTIFISSUERUNKNOWN:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate Issuer unknow!\n");
			break;
		case CERTIFUNKNOWNCA:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate is based on untrust root cert!\n");
			break;
		case CERTIFEXPIRED:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate Expired!\n");
			break;
		case CERTIFSIGNERR:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate sign error!\n");
			break;
		case CERTIFREVOKED:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate Revoked!\n");
			break;
		case CERTIFBADUSE:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate not used legally!\n");
			break;
		case CERTUNKNOWNREVOKESTATUS:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate unknown revoke-status!\n");
			break;
		case CERTUNKNOWNERROR:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA Certificate unknown error!\n");
			break;
		default:
			asd_printf(ASD_WAPI,MSG_DEBUG,"Other Error!\n");
			break;
		}
		switch(errorcode_toAE) 
		{
		case CERTIFVALID:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP Certificate valid!\n");
			break;
		case CERTIFISSUERUNKNOWN:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP Certificate Issuer unknow!\n");
			break;
		case CERTIFUNKNOWNCA:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP  Certificate is based on untrust root cert!\n");
			break;
		case CERTIFEXPIRED:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP Certificate Expired!\n");
			break;
		case CERTIFSIGNERR:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP Certificate sign error!\n");
			break;
		case CERTIFREVOKED:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP Certificate Revoked!\n");
			break;
		case CERTIFBADUSE:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP Certificate not used legally!\n");
			break;
		case CERTUNKNOWNREVOKESTATUS:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP Certificate unknown revoke-status!\n");
			break;
		case CERTUNKNOWNERROR:
			asd_printf(ASD_WAPI,MSG_DEBUG,"AP Certificate unknown error!\n");
			break;
		default:
			asd_printf(ASD_WAPI,MSG_DEBUG,"Other Error!\n");
			break;
		}		
	}
	return auth_error;
}


/*安装组播密钥*/
int set_mcastkey(apdata_info *pap, struct sta_msksa *msksa)
{
	struct ioctl_drv io_buff;
	u8 *pos = io_buff.iodata.pbData;
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	struct asd_data *wasd;
	if(pap == NULL) return -1;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	wasd = tmp_circle->wasd_bk;
	if(wasd == NULL) return -1;
	
	memset(&io_buff, 0, sizeof(io_buff));

	io_buff.io_packet = P80211_PACKET_SETKEY;
	memset(pos, 0xFF, WLAN_ADDR_LEN);
	pos += WLAN_ADDR_LEN;
	*pos =  0x1; pos += 1;

	asd_printf(ASD_WAPI,MSG_DEBUG,"current MSKID = %d\n",msksa->mskid);
	DPrint_string_array("MSK", msksa->msk, 16);

	time(&(msksa->setkeytime));	//	xm0623
	
	*pos =  msksa->mskid; pos += 1;
	KD_hmac_sha256((u8 *)Input_text, 
			strlen((const char *)Input_text), 
			msksa->msk, 16, pos, 32);
	io_buff.iodata.wDataLen = WLAN_ADDR_LEN + 2+32;
	//wapid_ioctl(pap, P80211_IOCTL_SETWAPI_INFO, &io_buff, sizeof(io_buff));	
	if (asd_set_encryption(wasd->conf->iface, wasd, "SMS4",
				   NULL, 0, (u8 *)(&io_buff), sizeof(io_buff),
				   1)) {
		asd_printf(ASD_WAPI,MSG_ERROR, "Could not set individual WEP "
			   "encryption.");
	}
	
#define PKEY "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "
#define BKEY(a)	a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],a[10],a[11],a[12],a[13],a[14],a[15]
	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : MSK:"PKEY".\n",__func__,BKEY(msksa->msk));
#undef PKEY 
#undef BKEY
	/*ht del rekey mcastkey*/	
	if(0 && ASD_SECURITY[wasd->SecurityID]!=NULL
		&&ASD_SECURITY[wasd->SecurityID]->wapi_mcast_rekey_method==1
		&&ASD_SECURITY[wasd->SecurityID]->wapi_mcast_rekey_para_t!=0){

		unsigned int time_out;
		time_out=ASD_SECURITY[wasd->SecurityID]->wapi_mcast_rekey_para_t;
		circle_cancel_timeout((void *)rekey_mcastkey_tlimit,(void *)pap,NULL);

		if(0!=circle_register_timeout(time_out, 0, (void *)rekey_mcastkey_tlimit,(void *)pap,(void *)ASD_SECURITY[wasd->SecurityID])){	//	xm06301
			asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : register rekey_mcastkey_tlimit() failed.\n",__func__);
		}

		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : register rekey_mcastkey_tlimit() to rekey mcastkey after %u s.\n"
			,__func__,time_out);
	}
	return 0;
}

/*安装单播密钥*/
int set_ucastkey(apdata_info *pap,  struct auth_sta_info_t *wapi_sta_info)
{
	struct ioctl_drv io_buff;
	struct sta_usksa *usksa;
	u8 *pos = io_buff.iodata.pbData;
	
	struct wapid_interfaces *wapid ;
	struct asd_wapi *tmp_circle ;
	struct asd_data *wasd;
	if(pap == NULL) return -1;
	wapid = (struct wapid_interfaces *)pap->user_data;
	if(wapid == NULL) return -1;
	tmp_circle = (struct asd_wapi *)wapid->circle_save;
	if(tmp_circle == NULL) return -1;
	wasd = tmp_circle->wasd_bk;
	if(wasd == NULL) return -1;
	if((pap == NULL) ||(wapi_sta_info == NULL)) return -1;
	
	usksa = &wapi_sta_info->usksa;

	time(&(usksa->setkeytime));	/*	xm0623*/
	
	memset(&io_buff, 0, sizeof(io_buff));
	
	io_buff.io_packet = P80211_PACKET_SETKEY;

	memcpy( pos, wapi_sta_info->mac, WLAN_ADDR_LEN);
	pos += WLAN_ADDR_LEN;
	*pos = 1;	pos+=1;

	*pos = usksa->uskid; pos+=1;
	memcpy(pos, usksa->usk[usksa->uskid].uek , KEY_LEN); pos += KEY_LEN;
	memcpy(pos, usksa->usk[usksa->uskid].uck , KEY_LEN); pos += KEY_LEN;
	io_buff.iodata.wDataLen = WLAN_ADDR_LEN+2+2* KEY_LEN;
#if 1	
	asd_printf(ASD_WAPI,MSG_DEBUG,"current USKID = %d\n",usksa->uskid);
	DPrint_string_array("UEK", usksa->usk[usksa->uskid].uek , KEY_LEN);
	DPrint_string_array("UCK", usksa->usk[usksa->uskid].uck , KEY_LEN);
#endif
	
	if (asd_set_encryption(wasd->conf->iface, wasd, "SMS4",
				   wapi_sta_info->mac, 0, (u8 *)(&io_buff), sizeof(io_buff),
				   1)) {
		asd_printf(ASD_WAPI,MSG_ERROR, "Could not set individual WEP "
			   "encryption.");
	}

#define PKEY "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "
#define BKEY(a)	a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],a[10],a[11],a[12],a[13],a[14],a[15]
	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : UEK:"PKEY".\n",__func__,BKEY(usksa->usk[usksa->uskid].uek));
#undef PKEY 
#undef BKEY
	/*ht del rekey ucastkey*/	
	if(0 && ASD_SECURITY[wasd->SecurityID]!=NULL
		&&ASD_SECURITY[wasd->SecurityID]->wapi_ucast_rekey_method==1
		&&ASD_SECURITY[wasd->SecurityID]->wapi_ucast_rekey_para_t!=0){
		unsigned int time_out;
		time_out=ASD_SECURITY[wasd->SecurityID]->wapi_ucast_rekey_para_t;
		/*set rekey timer.*/
		circle_cancel_timeout((void *)rekey_ucastkey_tlimit,(void *)wapi_sta_info,(void *)pap);

		if(0!=circle_register_timeout(time_out, 0,  (void *)rekey_ucastkey_tlimit,(void *)wapi_sta_info,(void *)pap)){	//	xm06301
			asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : register rekey_ucastkey_tlimit() failed.\n",__func__);
		}

		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : register rekey_ucastkey_tlimit() to rekey ucastkey after %u s.\n",__func__,time_out);
	}
	return 0;
}

/*处理Driver中消息*/
void wapi_process_for_driver(unsigned char *readapdrv, int readlen, apdata_info *pap)
{
#define MIN_AUTH_MSG_LEN  (4 + 6+2+16+24)
#define MIN_REKEY_MSG_LEN  (4 + 6+2)
#define MIN_AGING_MSG_LEN  (4 + 6+2)

	const u16 AUTH = 0x00F1;
	const u16 UCAST_REKEY = 0x00F2;
	const u16 AGING = 0x00F3;
	const u16 MCAST_REKEY = 0x00F4;
	
	asso_mt *asso_mt_info = (asso_mt *)readapdrv;

	assert(asso_mt_info != NULL);

	if(asso_mt_info->type == AUTH)
	{
		/*STA关联成功消息*/
		asd_printf(ASD_WAPI,MSG_DEBUG,"receive auth new message(len =%d):\n", readlen);
		if(readlen < MIN_AUTH_MSG_LEN)
		{
			asd_printf(ASD_WAPI,MSG_DEBUG,"message is too short\n");
			return;
		}
		/*鉴别初始化*/
		//auth_initiate(asso_mt_info, pap);
	}
	else if(asso_mt_info->type == UCAST_REKEY)
	{
		/*单播密钥Rekey消息*/
		asd_printf(ASD_WAPI,MSG_DEBUG,"Receive message(len= %d) from driver and go dynamic session negoration\n ",readlen);
		if(readlen < MIN_REKEY_MSG_LEN)
		{
			asd_printf(ASD_WAPI,MSG_DEBUG,"message is too short\n");
			return;
		}
		/*发送单播密钥协商请求*/
		usk_dynegotiation_req(asso_mt_info,pap);
	}
	else if(asso_mt_info->type == MCAST_REKEY)
	{
		/*组播密钥Rekey消息*/
		asd_printf(ASD_WAPI,MSG_DEBUG,"Receive Multi-key change  message(len=%d) from driver!\n", readlen);
		if(readlen < MIN_REKEY_MSG_LEN)
		{
			asd_printf(ASD_WAPI,MSG_DEBUG,"message is too short\n");
			return;
		}
		/*组播密钥通告标识加1*/
		update_gnonce((unsigned long*)pap->gnonce,0);
		DPrint_string_array("current gnonce",(u8*)pap->gnonce, 16);
		
		/*判断组播密钥通告标识是否溢出*/
		if(overflow((unsigned long*)pap->gnonce) == 0)
		{
			/*should not happen */
			u8 mac[WLAN_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} ;
			/*与所有STA解除链路验证*/
			sta_deauth(mac, IEEE80211_MLME_DEAUTH, pap);
		}
		else
		{
			/*取随机数作为组播密钥*/
			get_random(pap->msksa.msk, MULTI_KEY_LEN);
			
			/*将组播通告状态置为有效,表示正在进行组播通告*/
			pap->group_status = 1;
			/*给所有鉴别成功的STA发送组播通告*/
			send_msk_announcement_to_all(pap);
		}
	}
	else if(asso_mt_info->type == AGING)
	{
		/*STA 超时消息*/
		asd_printf(ASD_WAPI,MSG_DEBUG,"Receive sta aging message(len= %d) from driver \n ",readlen);
		if(readlen < MIN_AGING_MSG_LEN)
		{
			asd_printf(ASD_WAPI,MSG_DEBUG,"message is too short\n");
			return;
		}
		asd_printf(ASD_WAPI,MSG_DEBUG,"sta(" MACSTR ") timeout \n", MAC2STR(asso_mt_info->mac));
		/*STA超时处理*/
		sta_timeout_handle(asso_mt_info->mac, pap);
	}
	
#undef MIN_AUTH_MSG_LEN
#undef MIN_REKEY_MSG_LEN
#undef MIN_AGING_MSG_LEN
}

/*	xm06301*/

int rekey_ucastkey_tlimit(void *circle_data, void *user_data){
//int rekey_ucastkey_tlimit(struct auth_sta_info_t *wapi_sta_info,apdata_info *pap){
	struct auth_sta_info_t *wapi_sta_info = (struct auth_sta_info_t *)circle_data;
	apdata_info *pap = (apdata_info *)user_data;
	int sendtoMT_len = 0;
	int sendlen = 0;
	u16 offset= 0;
	u8 sendto_asue[FROM_MT_LEN];
	u8 keyid;
	session_key_neg_request  dynamic_sessionkey;
	struct ethhdr eh;
	wai_fixdata_flag flag;
	packet_head head;
	
	if(wapi_sta_info==NULL||pap==NULL){
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : parameter invalid\n",__func__);
		return -1;
	}

	//单播密钥Rekey消息
	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : rekey ucastkey begin du to  rekey timeout expire.\n",__func__);

	
	
	//发送单播密钥协商请求
	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : staMAC %02X:%02X:%02X:%02X:%02X:%02X.\n",__func__,
		wapi_sta_info->mac[0],wapi_sta_info->mac[1],wapi_sta_info->mac[2],
		wapi_sta_info->mac[3],wapi_sta_info->mac[4],wapi_sta_info->mac[5]);
	
	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : apMAC %02X:%02X:%02X:%02X:%02X:%02X.\n",__func__,
		pap->macaddr[0],pap->macaddr[1],pap->macaddr[2],
		pap->macaddr[3],pap->macaddr[4],pap->macaddr[5]);

	if(wapi_sta_info->status != MT_AUTHENTICATED){
		
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : rekey ucastkey before autenticated.\n",__func__);
		return 0;
	}
	
	memset(&dynamic_sessionkey, 0,sizeof(session_key_neg_request));
	memcpy(eh.h_dest, wapi_sta_info->mac, ETH_ALEN);

	/*设置uskid*/
	wapi_sta_info->usksa.dynamic_key_used = wapi_sta_info->usksa.uskid;
	asd_printf(ASD_WAPI,MSG_DEBUG,"1 keyid:%d\n", wapi_sta_info->usksa.uskid);
	wapi_sta_info->usksa.uskid =wapi_sta_info->usksa.uskid^0x01;
	keyid = wapi_sta_info->usksa.uskid;	
	wapi_sta_info->usksa.usk[keyid].valid_flag = 0;/*invalid*/
	asd_printf(ASD_WAPI,MSG_DEBUG,"2 keyid:%d\n", wapi_sta_info->usksa.uskid);

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

	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : wapi_sta_info->status=%u.\n",__func__,wapi_sta_info->status);

	
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
		asd_printf(ASD_WAPI,MSG_DEBUG,"(sendlen != sendtoMT_len)\n");

		asd_printf(ASD_WAPI,MSG_DEBUG,"send_rs_data error in send_ap_random_handle\n");
		//return;
	}

	wapi_sta_info->status=MT_WAITING_DYNAMIC_SESSION;	//	xm06301
	wapi_sta_info->ae_group_sc+= 1;

	asd_printf(ASD_WAPI,MSG_DEBUG,"asd send MT_WAITING_DYNAMIC_SESSION to rekey.----------------xm\n");
	//set_ucastkey(pap, wapi_sta_info);	//	xm06301

	return 0;
}


//	xm06301
int rekey_mcastkey_tlimit(void *circle_data, void *user_data){
	apdata_info *pap = (apdata_info *)circle_data;
	security_profile *p = (security_profile *)user_data;
	security_profile * security=NULL;
	
	if(pap==NULL){
		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : parameter invalid\n",__func__);
		return -1;
	}
	
	security=(security_profile *)p;
	
	/*组播密钥Rekey消息*/
	asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : rekey mcastkey begin du to  rekey timeout expire.\n",__func__);
			
	/*组播密钥通告标识加1*/
	update_gnonce((unsigned long*)pap->gnonce,0);
	DPrint_string_array("current gnonce",(u8*)pap->gnonce, 16);
			
	/*判断组播密钥通告标识是否溢出*/
	if(overflow((unsigned long*)pap->gnonce) == 0)
	{
		/*should not happen */
		u8 mac[WLAN_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} ;
		/*与所有STA解除链路验证*/
		sta_deauth(mac, IEEE80211_MLME_DEAUTH, pap);
	}else{
		/*取随机数作为组播密钥*/
		get_random(pap->msksa.msk, MULTI_KEY_LEN);
				
		/*将组播通告状态置为有效,表示正在进行组播通告*/
		pap->group_status = 1;
		/*给所有鉴别成功的STA发送组播通告*/
		send_msk_announcement_to_all(pap);
	}

	if(security!=NULL
		&&security->wapi_mcast_rekey_method==1
		&&security->wapi_mcast_rekey_para_t!=0){

		unsigned int time_out;
		time_out=security->wapi_mcast_rekey_para_t;
		circle_cancel_timeout((void *)rekey_mcastkey_tlimit,(void *)pap,(void *)security);

		if(0!=circle_register_timeout(time_out, 0, (void *)rekey_mcastkey_tlimit,(void *)pap,(void *)security)){	//	xm06301
			asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : register rekey_mcastkey_tlimit() failed.\n",__func__);
		}

		asd_printf(ASD_WAPI,MSG_DEBUG,"in %s : register rekey_mcastkey_tlimit() to rekey ucastkey after %u s.\n"
			,__func__,time_out);
	}
	return 0;
}

static void display_wapi_sta_status(struct auth_sta_info_t *wapi_sta_info)
{
	return ;
	switch(wapi_sta_info->status) 
	{
		case NO_AUTH:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :NO_AUTH\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_WAITING_ACCESS_REQ:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_WAITING_ACCESS_REQ\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_WAITING_AUTH_FROM_AS:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_WAITING_AUTH_FROM_AS\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_WAITING_SESSION:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_WAITING_SESSION\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_WAITING_GROUPING:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_WAITING_GROUPING\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_SESSIONKEYING:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_SESSIONKEYING\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_GROUPNOTICEING:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_GROUPNOTICEING\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_SESSIONGROUPING:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_SESSIONGROUPING\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_WAITING_DYNAMIC_SESSION:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_WAITING_DYNAMIC_SESSION\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_WAITING_DYNAMIC_GROUPING:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_WAITING_DYNAMIC_GROUPING\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_AUTHENTICATED:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_AUTHENTICATED\n",MAC2STR(wapi_sta_info->mac));
			break;
		case MT_PRE_AUTH_OVER:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :MT_PRE_AUTH_OVER\n",MAC2STR(wapi_sta_info->mac));
			break;
		default:
			asd_printf(ASD_WAPI,MSG_DEBUG,"STA "MACSTR"'s status :Other Status!\n",MAC2STR(wapi_sta_info->mac));
			break;
	}

	return ;
}

//mahz add 2010.12.9
int convert_serial_no(byte_data *dst, byte_data *src)
{
	int val=0,len=0;
	unsigned int *q = NULL;
	unsigned char mid[128];
	//unsigned char *p = dst->data;
	
	len = src->data[1];
	memset(mid,0,sizeof(mid));
	memcpy(mid,src->data+2,len);
	mid[sizeof(mid)-1] = '\0';
	
	q = (unsigned int *)mid;

	switch(len){
		case 1:
			val = (*q)>>(8*3);
			break;
		case 2:
			val = (*q)>>(8*2);
			break;
		case 3:
			val = (*q)>>(8*1);
			break;
		case 4:
			val = *q;
			break;
		default:
			return -1;
	}
	memset(dst,0,sizeof(byte_data));
	sprintf(dst->data,"%d",val);
	dst->length = os_strlen(dst->data);
	
	asd_printf(ASD_WAPI,MSG_DEBUG," serial_data: dst %s\n",dst->data);
	asd_printf(ASD_WAPI,MSG_DEBUG," serial_length: dst %d\n",dst->length);

	return 0;
}
