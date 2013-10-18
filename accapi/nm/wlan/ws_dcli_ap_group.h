/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* ws_dcli_ap_group.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h> 
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"
//#include "ws_fdb.h"
//#include "ws_dcli_acl.h"
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
//#include "dbus/wcpss/dcli_wid_wtp.h"
//#include "dbus/wcpss/dcli_wid_wlan.h"
//#include "memory.h"
//#include "sysdef/npd_sysdef.h"
//#include "dbus/npd/npd_dbus_def.h"
//#include "wcpss/asd/asd.h"

struct ap_group_list
{
	unsigned int test_id;
	unsigned char *test_name;
	struct ap_group_list *next;
};

extern int ccgi_create_ap_group_cmd(dbus_parameter parameter, DBusConnection *connection, char *ap_g_id, char *ap_g_name);
								/*返回1表示失败，返回0表示成功，返回2表示unknown id format*/
								/*返回3表示ap_g_id should be 1 to WTP_GROUP_NUM-1，返回4表示name is too long,out of the limit of 64*/
								/*返回5表示id exist，返回6表示error*/
extern int ccgi_del_ap_group_cmd(dbus_parameter  parameter, DBusConnection *connection, char *ap_g_id);
								/*返回0表示成功，返回-1表示失败，返回-2表示unknown id format*/
								/*返回-3表示ap_g_id should be 1 to WTP_GROUP_NUM-1，返回-4表示(NULL == reply)*/
								/*返回-5表示WLAN_ID_NOT_EXIST，返回-6表示error*/

/*oper为"add"或"delete"*/
/*wtp_id_list为"all"或AP ID列表，格式为1,8,9-20,33*/
extern int ccgi_add_del_ap_group_member_cmd(dbus_parameter  parameter, DBusConnection *connection,char *ap_g_id,char *oper,char *wtp_id_list);
								/*返回0表示成功，返回1表示成功，返回-1表示指针为空*/
								/*返回-2表示操作只能是add和delete;返回-3表示set wtp list error,like 1,8,9-20,33，*/
								/*返回-3表示ap group id非法*/
								/*返回-4表示ap group id  不正确，返回-5表示ap group id	不存在，返回-6表示error*/

extern void Free_show_group_member_cmd(unsigned int *wtp_list);
/*只要调用函数，就调用Free_show_group_member_cmd()释放空间*/
extern int ccgi_show_group_member_cmd(dbus_parameter  parameter, DBusConnection *connection,unsigned int id,unsigned int **wtp_list,unsigned int *count);
								   /*返回0表示失败，返回1表示成功，返回-1表示ap group id非法*/
extern int ccgi_show_ap_group_cmd(dbus_parameter  parameter, DBusConnection *connection, struct ap_group_list *head);
									/*返回-2表示ap group id does not exist，返回-3表示error*/


