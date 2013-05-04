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
* key_neg.h
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

#ifndef __KEY_NEG_H__
#define __KEY_NEG_H__


//#include "cert_auth.h"
//#include "prf.h"
//#include "ecc192_x962.h"
//#include "sha256_api.h"
#include "auth.h"
#include "structure.h"

/*单播Rekey中的单播密钥协商请求*/
void usk_dynegotiation_req(asso_mt *passo_mt_info, apdata_info *pap);
/*单播密钥协商请求*/
void	usk_negotiation_req(struct auth_sta_info_t *wapi_sta_info);
/*单播密钥协商响应分组*/
int usk_negotiation_res(void *read_asue, int readlen,  struct auth_sta_info_t *sta_info);
/*单播密钥协商确认分组*/
void usk_negotiation_confirmation(struct auth_sta_info_t *wapi_sta_info);
/*组播通告*/
void msk_announcement_tx(struct auth_sta_info_t *wapi_sta_info, u8 *sendto_asue);
/*组播通告响应*/
int  msk_announcement_res(struct auth_sta_info_t *wapi_sta_info, u8 *read_asue, int readlen);
/*向所有STA发送组播通告*/
void send_msk_announcement_to_all(apdata_info *pap);
/*向Driver 发送组播通告结果*/
int notify_groupnotice_to_apdriver(apdata_info *pap);
/*向STA发送BK*/
int send_bk_to_sta(struct auth_sta_info_t *wapi_sta_info);
/*ECDH算法初始化*/
int wai_initialize_ecdh(para_alg *ecdh);
int wai_copy_ecdh(para_alg *ecdh_a, para_alg *ecdh_b);
int wai_compare_ecdh(para_alg *ecdh_a, para_alg *ecdh_b);


#endif
