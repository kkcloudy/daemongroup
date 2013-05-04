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
* wai_sta.h
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

#ifndef __WAI_STA_H__
#define __WAI_STA_H__
#define DEL_DEBUG
#include "auth.h"

//struct auth_sta_info_t * ap_get_sta(unsigned char *mac, apdata_info *pap);
struct auth_sta_info_t * ap_get_sta_pos(u8 *mac, struct auth_sta_info_t *sta_info_table);
struct auth_sta_info_t * ap_add_sta(asso_mt *passo_mt_info, int packtype,  apdata_info *pap);
void sta_timeout_handle(u8 *mac, apdata_info *pap);
 int  wapid_ioctl(apdata_info  *pap, u16 cmd, void *buf, int buf_len);
int sta_deauth(u8 *addr, int reason_code, apdata_info *pap);
/*重新初始化STA信息*/
void reset_sta_info(struct auth_sta_info_t *wapi_sta_info, apdata_info*pap);
void  notify_driver_disauthenticate_sta(struct auth_sta_info_t *wapi_sta_info, const char *func, int line);
/*清除重发缓冲区*/
void reset_table_item(struct auth_sta_info_t *wapi_sta_info);
void set_table_item(struct auth_sta_info_t *wapi_sta_info, 
		u16 direction, u16 flag, 
		u8*sendto_asue, int sendto_len);
int set_wapi_stats(struct auth_sta_info_t *wapi_sta_info);
#endif

