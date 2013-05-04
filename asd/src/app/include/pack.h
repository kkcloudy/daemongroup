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
* pack.h
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

#ifndef _PACK_H
#define _PACK_H

#include "structure.h"
#include "common.h"
//#include "typedef.h"

void htonl_buffer(void *buffer, int len_in_byte);
void ntohl_buffer(void *buffer, int len_in_byte);

void htons_buffer(void *buffer, int len_in_byte);
void ntohs_buffer(void *buffer, int len_in_byte);

short  c_pack_byte(const u8* content,void* buffer,u16 offset,unsigned short bufflen);
short  c_pack_word(const u16* content,void* buffer,u16 offset,unsigned short bufflen);
short  c_pack_dword(const u32* content,void* buffer,u16 offset,unsigned short bufflen);
short  c_pack_16bytes(const u8* content,void* buffer,u16 offset,unsigned short bufflen);
short  c_pack_32bytes(const u8* content,void* buffer,u16 offset,unsigned short bufflen);
short pack_mac(const u8* pData, void* buffer, short offset);
short  c_pack_sign_alg(const wai_fixdata_alg *pSign_alg,void* buffer,u16 offset,unsigned short bufflen);
short  c_unpack_sign_alg(wai_fixdata_alg* p_sign_alg,const void* buffer,u16 offset,u16 bufflen);
short  c_unpack_pubkey(pubkey_data* p_pubkey,const void* buffer,u16 offset,u16 bufflen);

short  c_pack_byte_data(const byte_data* pData,void* buffer,u16 offset,unsigned short bufflen);
short  c_pack_packet_head( packet_head *pHead,void* buffer,u16 offset,unsigned short bufflen);
short c_pack_packed_head(void *buffer, unsigned short bufflen);

short  c_unpack_word(u16* content, const void* buffer,u16 offset,
		u16 bufflen);
void  set_packet_type(void* buffer,const u16 the_type);
void  set_packet_reserved(void* buffer,const u16 the_reserved);
u16  get_packet_reserved(const void* buffer);

u16  get_packet_type(const void* buffer);
u8  get_packet_sub_type(const void* buffer);

void  set_packet_data_len(void* buffer,const u16 the_len);
u16  get_packet_data_len(const void* buffer);


u16  get_packet_type(const void* buffer);
void  set_packet_type(void* buffer,const u16 the_type);
u16  get_packet_group_sc(const void* buffer);


	
/* 认证激活报文的打包模块，供AP调用 */
short  pack_auth_active(const auth_active* pContent,void* buffer,unsigned short bufflen);

short  unpack_head(packet_head* p_head,const void* buffer,unsigned short bufflen);
 
/* 接入认证请求报文解包函数，供sta调用 */
short  unpack_sta_auth_request(sta_auth_request *p_sta_auth_request,const void* buffer,u16 bufflen, u16 *no_signdata_len);


/* 证书认证响应报文解包函数，供AS调用 */
short  unpack_ap_auth_response(ap_auth_response *p_auth_response,
						const void* buffer,u16 bufflen,
						u16 *no_signdata_len,u16 *no_signdata_len1);

/* 会话密钥协商响应报文解包函数，供响应方AP调用 */
short  unpack_ucastkey_neg_response(session_key_neg_response *p_session_key_neg_response,const void* buffer,u16 bufflen);


 /* 组播密钥响应报文的解包函数，供AP调用 */
short unpack_msk_announcement_res(groupkey_notice_response *p_groupkey_notice_response, const void* buffer, u16 bufflen);
short c_pack_nbytes(const u8* content, int n, void* buffer,u16 offset,unsigned short bufflen);
#endif
