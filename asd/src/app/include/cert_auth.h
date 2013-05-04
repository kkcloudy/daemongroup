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
* cert_auth.h
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

#ifndef __CERT_AUTH_H__
#define __CERT_AUTH_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <linux/if_packet.h>
//#include <linux/if_ether.h>

#include "structure.h"
#include "auth.h"
#include "cert_info.h"
#include "key_neg.h"
#include "common.h"
//#include "typedef.h"
//#include "prf.h"
#include "pack.h"
#include "debug.h"
//#include "raw_socket.h"

/*xm add */
//int rekey_ucastkey_tlimit(struct auth_sta_info_t *wapi_sta_info,apdata_info *pap);
int rekey_ucastkey_tlimit(void *circle_data, void *user_data);
int rekey_mcastkey_tlimit(void *circle_data, void *user_data);

void ap_pskbk_derivation(apdata_info *pap);

/*取证书中的颁发者名称,持有者名称和序列号*/
int wai_fixdata_id_by_ident(void *cert_st, 
								void *cert,wai_fixdata_id *fixdata_id,
								u16 index);
/*单播密钥导出方法*/
void ae_usk_derivation(struct auth_sta_info_t *wapi_sta_info, session_key_neg_response *key_res_buff);
/*鉴别初始化*/
int auth_initiate(struct sta_info *sta, struct asd_data *wasd);

/*处理STA发送的帧
	a 接入鉴别请求分组
	b 密钥协商响应分组
	c 广播密钥通告响应分组
*/
int wapi_process_from_sta(u8 *read_asue, int readlen, u8 *mac, apdata_info *pap);

/*处理AS发送的证书鉴别响应分组*/
int wapi_process_1_of_1_from_as(u8 *read_as, int readlen, apdata_info *pap);

/*接入鉴别响应*/
int access_auth_res(struct auth_sta_info_t *wapi_sta_info,	u8 *temp, int temp_len );

/*处理接入鉴别请求分组*/
int access_auth_req (struct auth_sta_info_t *wapi_sta_info, const u8 *read_asue, 
						int readlen, u8 *sendto_as, int *psendtoas_len);

/*构造证书鉴别请求"本地策略默认是通过ASU鉴别证书"*/
int certificate_auth_req (const sta_auth_request	*sta_auth_requ, 
							struct auth_sta_info_t *wapi_sta_info,
							u8 *sendto_as, int *psendtoas_len); 
	
/*处理证书鉴别响应*/
int certificate_auth_res(struct auth_sta_info_t **wapi_sta_info, u8 *read_as, int readlen , apdata_info *pap);

/*安装组播密钥*/
int set_mcastkey(apdata_info *pap, struct sta_msksa *msksa);
/*安装单播密钥*/
int set_ucastkey(apdata_info *pap,  struct auth_sta_info_t *wapi_sta_info);
/*处理Driver中消息*/
void wapi_process_for_driver(unsigned char *readapdrv, int readlen, apdata_info *pap);

//mahz add 2010.12.9   用于将证书序列号转换为字符串
int convert_serial_no(byte_data *dst, byte_data *src);

#endif
