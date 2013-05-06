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
* ws_firewall.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef ATCS_FIREWALL
#define ATCS_FIREWALL 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define DORULE_PREFIX_CMD	system("sudo /usr/bin/setlinkstate.sh down");
#define DORULE_POSTFIX_CMD	system("sudo /usr/bin/setlinkstate.sh up");
//网卡数据转发设置
#define PEND_CVM_FILE	"/sys/module/cavium_ethernet/parameters/pend_cvm"

#define DORULE_ETHERNET_ON 	\
	do { \
		char cmd[256]; \
		sprintf(cmd, "sudo chmod a+rw %s; echo 0 > %s", PEND_CVM_FILE, PEND_CVM_FILE); \
		system(cmd); \
	} while (0);
	
#define DORULE_ETHERNET_OFF \
	do { \
		char cmd[256]; \
		sprintf(cmd, "sudo chmod a+rw %s; echo 1 > %s", PEND_CVM_FILE, PEND_CVM_FILE); \
		system(cmd); \
	} while (0);


#define NAT_UDP_TIMEOUT_MIN		30
#define NAT_UDP_TIMEOUT_MAX		1800
#define NAT_UDP_TIMEOUT_DEFAULT	1200

//规则的编辑状态：完成、已修改、新添加
typedef enum{
FW_DONE,
FW_CHANGED,
FW_NEW,
}fwRuleStatus;

//规则匹配时动作：允许、丢弃、拒绝
typedef enum{
FW_ACCEPT,
FW_DROP,
FW_REJECT,
FW_TCPMSS
}fwRuleAct;

//协议类型
typedef enum{
FW_PTCP=1,
FW_PUDP,
FW_PTCPANDUDP,
FW_PICMP,
}fwProtocl;

//规则的用途类型
typedef enum{
FW_WALL,		//filter过滤
FW_DNAT,		//DNAT
FW_SNAT,		//SNAT
FW_INPUT	//input chain
}fwRuleType;

//ip地址的表现形式
typedef enum{
FW_MASQUERADE,
FW_IPSINGLE,
FW_IPHOST,
FW_IPMASK,
FW_IPNET,
FW_IPRANG,
}fwIPType;

//端口的表现形式
typedef enum{
FW_PTSINGLE=1,
FW_PTRANG,
FW_PTCOLLECT,
}fwPortType;


typedef struct fwRule{
fwRuleType	 type; 		//规则的类型
unsigned int id;		//规则编号(同一类型的 id唯一)
unsigned int ordernum;	//规则创建时的历史顺序号
char 		*name;		//规则名称
int		 enable;	//是否可用
fwRuleStatus status;	//规则状态
char		*comment;	//规则注释，标注体现规则作用的注释

char 		*ineth;		//数据进入接口
char		*outeth;	//数据流出接口
fwIPType	 srctype;
char		*srcadd;	//源IP地址
fwIPType	 dsttype;
char		*dstadd;	//目的IP地址

fwProtocl	 protocl;	//匹配指定的协议

fwPortType	 sptype;
char		*sport;		//源端口
fwPortType	 dptype;
char		*dport;		//目的端口

/* connect limit */
char 		*connlimit;

fwRuleAct 	 act;		//规则动作--允许、拒绝、丢弃
char		*tcpmss_var;
//当type为wall时使用
char		*pkg_state;//
char		*string_filter;

//当type非WALL时使用
fwIPType	 natiptype;
char 		*natipadd;	//地址转换的地址参数
fwPortType	 natpttype;
char 		*natport;	//地址转换的端口参数

//指向下一规则
struct fwRule		*next;
}fwRule,*fwRulePtr;


//全部规则分类存储在下列格式的一个结构中
typedef struct{
fwRule *wall;
int	iWallTotalNum;
fwRule *snat;
int	iSNATTotalNum;
fwRule *dnat;
int	iDNATTotalNum;
fwRule *input;
int iInputTotalNum;
}fwRuleList;


void	firewall_free_ruleDate(fwRule *rule);

void	firewall_free_array(fwRule **array, unsigned int count);

//开启iptables服务 返回值 0:成功	
int fwServiceStart();

//停止iptables服务 返回值 0:成功	
int fwServiceStop();

//保存配置返回值 0:成功	
int fwServiceSaveConf();

//清空所有正使用的规则 返回值 0:成功	
int fwServiceFlush();

#define firewall_chmod_conf_file() \
do { \
	system("sudo chmod a+rw /opt/services/conf/firewall_conf.conf >/dev/null 2>&1"); \
	system("sudo chmod a+rw /opt/services/status/iptables_status.status >/dev/null 2>&1"); \
	system("sudo chmod a+rw /opt/services/status/firewall_status.status >/dev/null 2>&1"); \
} while(0);

#ifdef __cplusplus
}
#endif
#endif


