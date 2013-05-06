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
* ws_dcli_ac_ip_list.h
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
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include <time.h>     //xm add
#include <sys/time.h> //xm add


extern void Free_show_ac_ip_list_one(DCLI_AC_IP_LIST_API_GROUP *IPLIST);
/*返回1时，调用Free_show_ac_ip_list_one()释放空间*/
/*id的范围是1至ACIPLIST_NUM-1*/
extern int show_ac_ip_list_one_cmd(int instance_id,char *id, DCLI_AC_IP_LIST_API_GROUP **IPLIST);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																									   /*返回-2表示AC IP LIST id should be 1 to ACIPLIST_NUM-1*/
											                                             			   /*返回-3表示id does not exist，返回-4表示error*/

extern void Free_show_ac_ip_list(DCLI_AC_IP_LIST_API_GROUP *IPLIST);
/*返回1时，调用Free_show_ac_ip_list()释放空间*/
extern int show_ac_ip_list_cmd(int instance_id,DCLI_AC_IP_LIST_API_GROUP **IPLIST);/*返回0表示失败，返回1表示成功*/

/*ID的范围是1至ACIPLIST_NUM-1*/
/*ifname的长度要小于16 */
extern int create_ac_ip_list_cmd(int instance_id,char *ID,char *ifname);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																		   /*返回-2表示id should be 1 to ACIPLIST_NUM-1，返回-3表示ifname is too long,out of the limit of 16*/
																		   /*返回-4表示id exist，返回-5表示error，返回-6表示interface has be binded in other hansi*/

/*ID的范围是1至ACIPLIST_NUM-1*/
extern int del_ac_ip_list_cmd(int instance_id,char *ID);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
														   /*返回-2表示id should be 1 to ACIPLIST_NUM-1，返回-3表示wlan id does not exist*/
														   /*返回-4表示wlan is enable,please disable it first，返回-5表示error*/

/*Priority的范围是1-100*/
extern int add_ac_ip_cmd(int instance_id,char *id,char *IP,char *Priority);/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
																			   /*返回-2表示unknown id format，返回-3表示wlan id does not exist*/
																			   /*返回-4表示interface does not exist，返回-5表示wlan is enable,please disable it first*/
																			   /*返回-6表示error*/

extern int del_ac_ip_cmd(int instance_id,char *id,char *IP);/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
															   /*返回-2表示wlan id does not exist，返回-3表示interface does not exist*/
															   /*返回-4表示wlan is enable,please disable it first，返回-5表示error*/

extern int modify_ac_ip_priority_cmd(int instance_id,char *id,char *IP,char *Priority);/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
																						   /*返回-2表示unknown id format，返回-3表示wlan id does not exist*/
																						   /*返回-4表示interface does not exist，返回-5表示wlan is enable,please disable it first*/
																						   /*返回-6表示error*/

extern int set_ac_ip_list_banlance_cmd(int instance_id,char *id,char *state);/*返回0表示失败，返回1表示成功，返回-1表示wlan id does not exist*/
																				  /*返回-2表示interface does not exist，返回-3表示wlan is enable,please disable it first*/
																				  /*返回-4表示error*/

extern int set_ac_ip_threshold_cmd(int instance_id,char *id,char *IP,char *value);/*返回0表示失败，返回1表示成功，返回-1表示unknown ip format*/
																					   /*返回-2表示unknown id format，返回-3表示wlan id does not exist*/
																					   /*返回-4表示interface does not exist，返回-5表示wlan is enable,please disable it first*/
																					   /*返回-6表示ip addr no exit*/
																						   
extern int set_ac_ip_diffnum_banlance_cmd(int instance_id,char *id,char *value);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																					   /*返回-2表示wlan id does not exist，返回-3表示interface does not exist*/
																					   /*返回-4表示wlan is enable,please disable it first，返回-5表示error*/


