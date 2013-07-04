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
* ws_dcli_license.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#include <dbus/dbus.h>


#ifndef _WS_DCLI_LICENSE_H
#define _WS_DCLI_LICENSE_H

#define MAX_INSTANCE (16+1)
#define MAX_SLOT_NUM (16)

struct LicenseData{
	int total_num;
	int free_num;
	int slot_id[MAX_SLOT_NUM];
	int l_assign_num[MAX_SLOT_NUM][MAX_INSTANCE];
	int r_assign_num[MAX_SLOT_NUM][MAX_INSTANCE];
};

typedef enum{
	c_first_id=0,
	c_sub,
	c_fail,
	c_end,
	c_success,
	c_second_id
}id_state;
#define PARSE_ID_IFNAME_SUB '-'



extern int license_text_cmd_func();/*show multi licence texts */
extern int license_request_cmd_func();/*获取机器码*/

extern int license_install_cmd_func(char *SN);/*返回0表示失败，返回1表示成功，返回-1表示License fail,please contact vendor*/
extern int license_assign_cmd(char *type_p,char *num_p,char *hansi,int slotid,char *hansi_id,DBusConnection *connection);
extern int license_assign_show_cmd(DBusConnection *connection,int *license_count,struct LicenseData **LicenseInfo);

#endif
