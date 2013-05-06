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
* ws_tcrule.h
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
#ifndef WS_TCTULE_H
#define WS_TCTULE_H


#ifdef __cplusplus
extern "C"
{
#endif// end #ifdef __cplusplus

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <sys/wait.h>
/*#include "ws_err.h"*/

#define TCRULES_XML_FILE "/opt/services/conf/traffic_conf.conf"
#define TCRULE_STATUS_FILE "/opt/services/status/traffic_status.status"
#define CMD_LEN		2048
#define TRANS_TO_CMD	0

//下面两个模块还有问题，先将其设置为0
#define USE_P2P_TRAFFIC_CTROL	0
#define USE_RULE_TIME_CTROL		0


struct tcrule_offset_s {
    unsigned int ruleIndex;

    int     uplink_offset;
    int     downlink_offset;
};

//添加一个新属性的流程
/*
1、在struct tcRule中添加该属性
2、在addTCRuleToNode中处理新添加的属性
3、在tcParseXmlNode中处理新添加的属性
4、在tcFreeRule中释放该指针
5、在测试程序中添加相应的代码
*/
typedef struct tcRule{
	char 	*name;		//规则名称，还未用到
	int		enable;	//是否可用
	int		ruleIndex;
	char	*comment;	//规则注释，标注体现规则作用的注释， 还未用到

	char	*interface;//new
	char	*up_interface;//2008-10-30 10:14:11
	char	*protocol;//new
	
	char	*p2p_detail;//还未用到
	
	//包地址
	char	*addrtype;//new
	char	*addr_begin;//new
	char	*addr_end;//new
	char	*mode;//new 当addrtype为一个range时，可用设置这个为share和noshare的。
	
	//流量控制
	char	*uplink_speed;//new
	char	*downlink_speed;//new
	//p2p流量
	int		useP2P;
	char	*p2p_uplink_speed;//new
	char	*p2p_downlink_speed;//new
	//时间控制
	int		time_begin;//new
	int 	time_end;//new
#if TRANS_TO_CMD	
	//规则转换为相应的命令
	char	*cmd_mark;
	char	*cmd_class;
	char	*cmd_filter;
#endif	
	char	*limit_speed;// 不用

    struct tcrule_offset_s offset;
	
	struct tcRule *next;
}TCRule,*PTCRule;


int if_tcrule_file(char * fpath);
//创建一条规则
PTCRule	tcNewRule();

//释放一条规则
void tcFreeRule(TCRule *rule);

//释放规则链表
void tcFreeList( TCRule *root );

void tcFreeArray(TCRule **array, unsigned int count);

//存储规则文件
int tcSaveDoc( char *path, TCRule *root );

//解析规则文件
PTCRule tcParseDoc( char *path );

//将规则转换为命令
int processRuleCmd( PTCRule ptcRule );

//得到当前规则总数

int getTcRuleNum( );

//替换一条规则
PTCRule replaceRule( PTCRule ptcRuleRoot, PTCRule ptcRuleNew, int index );

//插入、添加一条规则
PTCRule insertRule( PTCRule ptcRuleRoot, PTCRule ptcRuleNew, int index );

//删除一条规则
PTCRule deleteRule( PTCRule ptcRuleRoot, int index );

//修改一条规则的顺序
PTCRule changeRuleIndex( PTCRule ptcRuleRoot, int iOldIndex, int iNewIndex );

//执行所有规则
int tc_doAllRules( PTCRule ptcRuleRoot );

extern void tcrule_status_exist();


//根据index得到链表中的规制
PTCRule getTCRuleByIndex( PTCRule ptcRuleRoot, int iIndex );
#ifdef __cplusplus
}
#endif// end   #ifdef __cplusplus

#endif// end #ifndef WS_TCTULE_H

