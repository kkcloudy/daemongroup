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
* ws_dcli_mirror.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_init_dbus.h"
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "ws_dcli_vlan.h"
#include "ws_fdb.h"





#ifndef __DCLI_MIRROR_H__
#define __DCLI_MIRROR_H__

#define MIRROR_STR "Mirror Configuration\n"

#define MIRROR_ERROR_NONE	0
#define MIRROR_SUCCESS			            	MIRROR_ERROR_NONE
#define MIRROR_FAIL                         	(MIRROR_ERROR_NONE + 1)
#define MIRROR_BAD_PARAM                    	(MIRROR_ERROR_NONE + 2)
#define MIRROR_MALLOC_FAIL                  	(MIRROR_ERROR_NONE + 3)
#define MIRROR_ACTION_NOT_SUPPORT           	(MIRROR_ERROR_NONE + 4)
#define MIRROR_SRC_PORT_EXIST			    	(MIRROR_ERROR_NONE + 5)
#define MIRROR_SRC_PORT_NOTEXIST 		    (MIRROR_ERROR_NONE + 6)
#define MIRROR_PROFILE_ID_OUTOFRANGE			(MIRROR_ERROR_NONE + 7)
#define MIRROR_SRC_VLAN_EXIST			    	(MIRROR_ERROR_NONE + 8)
#define MIRROR_SRC_VLAN_NOTEXIST		    	(MIRROR_ERROR_NONE + 9)
#define MIRROR_VLAN_NOT_EXIST 			    (MIRROR_ERROR_NONE + 10)
#define MIRROR_DESTINATION_NODE_NOTEXIST    (MIRROR_ERROR_NONE + 11)
#define MIRROR_DESTINATION_NODE_EXIST 	    (MIRROR_ERROR_NONE + 12)
#define MIRROR_DESTINATION_NODE_CREATE_FAIL (MIRROR_ERROR_NONE + 13)
#define MIRROR_SRC_FDB_NOTEXIST   		    (MIRROR_ERROR_NONE+14)
#define MIRROR_SRC_FDB_EXIST  			    (MIRROR_ERROR_NONE+15)
#define MIRROR_DONT_SUPPORT   			    (MIRROR_ERROR_NONE+16)
#define MIRROR_ACL_GLOBAL_NOT_EXISTED       	(MIRROR_ERROR_NONE+17)
#define MIRROR_SRC_ACL_EXIST   			    (MIRROR_ERROR_NONE+18)
#define MIRROR_SRC_ACL_NOTEXIST 		    	(MIRROR_ERROR_NONE+19)
#define MIRROR_DESTINATION_NODE_MEMBER_EXIST 	(MIRROR_ERROR_NONE+20)
#define MIRROR_FDB_MAC_BE_SYSMAC             (MIRROR_ERROR_NONE+21)
#define MIRROR_SRC_PORT_CONFLICT             (MIRROR_ERROR_NONE + 22)
#define MIRROR_DESTINATION_NODE_MEMBER_NOEXIST (MIRROR_ERROR_NONE+23)
#define MIRROR_PROFILE_NODE_CREATED			(MIRROR_ERROR_NONE + 24)
#define MIRROR_PROFILE_NOT_CREATED			(MIRROR_ERROR_NONE + 25)

// mirror error message corresponding to error code 
extern char * dcli_mirror_err_msg[];


#endif
#define mirror_number 100
struct mirror_info
{
	char * destPort_in;
	unsigned int in_flag;
	char * destPort_eg;
	unsigned int eg_flag;
	char * destPort_bid;
	unsigned int bid_flag;
	char * PortNo[mirror_number];
	char * Port_mode[mirror_number];
	unsigned short  VlanID[mirror_number];
	char * Mac[mirror_number];
	unsigned short fdb_vlan[mirror_number];
	char * fdb_port[mirror_number];
	unsigned int  policyindex[mirror_number];
	int PortNum;
	int VlanNum;
	int fdbNum;
	int policyindexNum;
};

extern int config_mirror_profile();/*创建镜像文件,返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示其他类型错误*/
//extern int Create_destPort(char * PortNO,char * port_mode);/*返回0表示失败，返回1表示成功，返回-2表示Illegal format with slot/port，返回-3表示其他类型错误*/
extern int Create_destPort(char * PortNO,char * port_mode,unsigned int profilez);

//extern int del_destPort(char * PortNO,char * port_mode);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示Illegal format with slot/port，返回-3表示其他错误*/
extern int del_destPort(char * PortNO,char * port_mode,unsigned int profilez);/*retu=0:fail; retu=-1:error; retu = 1 :succ*/

//extern int mirror_policy(char * policy_index);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示Illegal rule index，返回-3表示其他类型错误*/
extern int mirror_policy(char * policy_index,unsigned int profilez);/*retu=0:error, retu=-1:fail, retu = 1 :succ*/
extern int no_mirror_policy(char * policy_index,unsigned int profilez);/*retu=0:fail; retu=-1:error; retu=1:succ*/

//extern int no_mirror_policy(char * policy_index);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示Illegal rule index，返回-3表示其他类型错误*/
//extern int mirror_port(char * PortNO,char * port_mode);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示Illegal format with slot/port，返回-3表示Illegal slot no*/
extern int mirror_port(char * PortNO,char * port_mode,unsigned int profilez);/*retu=0:fail, retu=-1:error, retu=1:succ*/
extern int no_mirror_port(char * PortNO,char * port_mode,unsigned int profilez);/*retu=1:fail; retu=-1:error; retu=1:succ*/

//extern int no_mirror_port(char * PortNO,char * port_mode);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示Illegal format with slot/port，返回-3表示其他类型错误*/
//extern int mirror_vlan(char * VlanID);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示Illegal rule index，返回-3表示其他类型错误*/
extern int mirror_vlan(char * VlanID,unsigned int profilez);/*retu=0:error,retu=-1:fail;retu=1:succ*/
extern int no_mirror_vlan(char * VlanID,unsigned int profilez);/*retu=0:fail; retu=-1:error; retu=1:succ*/

//extern int no_mirror_vlan(char * VlanID);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示Illegal rule index，返回-3表示其他类型错误*/
//extern int mirror_fdb(char * mac,char * vlanid,char * PortNo);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示erro:input broadcast or multicast mac，返回-3表示Unknow vlan id format，返回-4表示FDB vlan outrange，返回-5表示Illegal format with slot/port!，返回2表示error mac format，返回-6表示其他错误*/
extern int mirror_fdb(char * mac,char * vlanid,char * PortNo,unsigned int profilez);/*retu=0:fail; retu=-1:error; retu=1:succ*/
extern int no_mirror_fdb(char * mac,char * vlanid,char * PortNo,unsigned int profilez);/*retu=0:fail; retu=-1:error; retu=1:succ*/

//extern int no_mirror_fdb(char * mac,char * vlanid,char * PortNo);/*返回0表示失败，返回1表示成功，返回-1表示error，返回-2表示erro:input broadcast or multicast mac，返回-3表示Unknow vlan id format，返回-4表示FDB vlan outrange，返回-5表示Illegal format with slot/port!，返回2表示error mac format，返回-6表示其他错误*/
extern int show_mirror_configure(struct mirror_info * rev_mirror_info);

