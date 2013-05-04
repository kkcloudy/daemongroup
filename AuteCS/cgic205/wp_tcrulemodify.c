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
* wp_tcrulemodify.c
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/

#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_tcrule.h"

#define IP_ADDR_STR_LEN		64
#define	TRAFFIC_STR_LEN		32

static PTCRule getRule(  );

int cgiMain()
{
	struct list *lpublic;
	PTCRule ptcRuleRoot=NULL;
	PTCRule	ptcRuleNew=NULL;
	char editType[32]="";
	char ruleNum[10]="";
	int	ruleIndex;

//	int ruleType = 0;
		//得到rule的类型， fileter dnat  snat
	char *encry=(char *)malloc(BUF_LEN);
	char *str;
		
	lpublic=get_chain_head("../htdocs/text/public.txt");
	
 	memset(encry,0,BUF_LEN);
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
  	str=dcryption(encry);
  	if(str==NULL)
  	{
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
  	}
	cgiHeaderContentType("text/html");
	  
	
  	//将规则读到链表中	 	
  	ptcRuleRoot = tcParseDoc( TCRULES_XML_FILE );
  	//如果是提交规则表单，就添加新的规则，注意要区分当前是修改还是添加!!!
	cgiFormStringNoNewlines( "editType", editType, sizeof(editType) );
	//printf( "editType = %s<br/>", editType );
	if( cgiFormSubmitClicked("submit_addtcrule") == cgiFormSuccess )
	{
		//printf("aaaaaaaaaaaaaaaa  editType=%s<br />", editType );
		if( strcmp( editType, "add" ) == 0 )
		{
			//printf("1111 ptcRuleRoot=%x<br />", (unsigned int)ptcRuleRoot);
			//读取规则，
			ptcRuleNew = getRule();
			//添加到链表
			if( NULL != ptcRuleNew )
				ptcRuleRoot = insertRule( ptcRuleRoot, ptcRuleNew, ptcRuleNew->ruleIndex );
			//保存文件
			//printf("2222 ptcRuleRoot=%x<br />", (unsigned int)ptcRuleRoot);
			tcSaveDoc( TCRULES_XML_FILE, ptcRuleRoot );
		}
		else if( strcmp( editType, "edit" ) == 0 )
		{
			//读取规则，
			//printf("cccccccccccccccccc<br />");
			ptcRuleNew = getRule();
			//添加到链表
			if( NULL != ptcRuleNew )
			{
				cgiFormStringNoNewlines( "ruleNum", ruleNum, sizeof(ruleNum) );
				sscanf( ruleNum, "%d", &ruleIndex );
				ptcRuleRoot = replaceRule( ptcRuleRoot, ptcRuleNew, ruleIndex );
			}
			//保存文件
			tcSaveDoc( TCRULES_XML_FILE, ptcRuleRoot );
		}
	}
	else if( cgiFormSubmitClicked("submit_doalltcrules") == cgiFormSuccess )
	{
		//执行所有规则	
		//printf("do all rules<br />\n");
		tc_doAllRules( ptcRuleRoot );
	}
	else if( strcmp( editType, "delete" ) == 0 )
	{
		if( cgiFormStringNoNewlines( "ruleNum", ruleNum, sizeof(ruleNum) ) == cgiFormSuccess )
		{
			
			sscanf( ruleNum, "%d", &ruleIndex );
			//printf( "ruleIndex = %d<br />", ruleIndex );
			//从链表中找到，并删除规则
			ptcRuleRoot = deleteRule( ptcRuleRoot, ruleIndex );
			//保存文件
			tcSaveDoc( TCRULES_XML_FILE, ptcRuleRoot );
		}
	}
	else if( strcmp( editType, "changeindex" ) == 0 )
	{
	    char oldIndex[16];
	    char newIndex[16];
	    int iOldIndex,iNewIndex;
	    
	    cgiFormStringNoNewlines( "oldIndex", oldIndex, sizeof(oldIndex) );
	    cgiFormStringNoNewlines( "newIndex", newIndex, sizeof(newIndex) );
	    
	    sscanf( oldIndex, "%d", &iOldIndex );
	    sscanf( newIndex, "%d", &iNewIndex );
	    
	    ptcRuleRoot = changeRuleIndex( ptcRuleRoot, iOldIndex, iNewIndex ); 
		//保存文件
	    tcSaveDoc( TCRULES_XML_FILE, ptcRuleRoot );	    
	}
	//printf( "xxxxxxxxxxxxxxxxxxxxx66666666666666666<br />" );
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
  	
	fprintf( cgiOut, "<title>modify</title> \n" );
	fprintf( cgiOut, "</head> \n" );
	fprintf( cgiOut, "<body> \n" );

#if 1
	fprintf( cgiOut, "<script type='text/javascript'>\n" );
	fprintf( cgiOut, "window.location.href='wp_tcruleview.cgi?UN=%s';\n", encry );
	fprintf( cgiOut, "</script>\n" );
#endif	
	fprintf( cgiOut, "</body>\n" );
	fprintf( cgiOut, "</html>\n" );	
	
	tcFreeList( ptcRuleRoot );
	release(lpublic);
	return 0;
}

static PTCRule getRule(  )
{
#if USE_RULE_TIME_CTROL
	char *timeSelectStr[]={"0","1","2","3","4","5","6","7","8","9","10","11",
							"12","13","14","15","16","17","18","19","20","21","22","23",
							"24","25","26","27","28","29","30","31","32","33","34","35",
							"36","37","38","39","40","41","42","43","44","45","46","47"};
#endif							
	char *addrTypes[] = {"address","addrrange"};
	PTCRule ptcRuleNew;
	char **ruleEnableed;
	int resoult;
	char szRuleNum[16]="";
	
	
	ptcRuleNew = tcNewRule();
	if( NULL == ptcRuleNew )
	{
		return NULL;	
	}

	//向ptcRuleNew中添加数据
	//得到当前规则的序号
	cgiFormStringNoNewlines( "ruleNum", szRuleNum, sizeof(szRuleNum) );
	sscanf( szRuleNum, "%d", &(ptcRuleNew->ruleIndex) );
	
	//得到当前规则是否有效
	if( cgiFormNotFound == cgiFormStringMultiple("nouse", &ruleEnableed) )
	{
		ptcRuleNew->enable = 1;	
	}
	else
	{
		ptcRuleNew->enable = 0;	
	}

	//得到当前规则的地址类型
	cgiFormSelectSingle( "addrtype_select", addrTypes, sizeof(addrTypes)/sizeof(char *), &resoult, 0 );
	if( NULL != ptcRuleNew->addrtype )
		free( ptcRuleNew->addrtype );
	ptcRuleNew->addrtype = (char *)malloc( strlen( (char*)addrTypes[resoult] )+2 );
	if( NULL == ptcRuleNew->addrtype )
	{
		goto error;
	}
	strcpy( ptcRuleNew->addrtype, (char *)addrTypes[resoult] );
	
	//得到地址，如果是单ip，就放在begin中
	if( 0 == resoult )//单ip
	{
		if( NULL == ptcRuleNew->addr_begin )
			ptcRuleNew->addr_begin = (char*)malloc( IP_ADDR_STR_LEN );
		cgiFormStringNoNewlines( "address_single", ptcRuleNew->addr_begin, IP_ADDR_STR_LEN );
	}
	else if( 1 == resoult )//ip范围
	{
		if( NULL == ptcRuleNew->addr_begin )
			ptcRuleNew->addr_begin = (char*)malloc( IP_ADDR_STR_LEN );
		cgiFormStringNoNewlines( "address_range_begin", ptcRuleNew->addr_begin, IP_ADDR_STR_LEN );
		
		if( NULL == ptcRuleNew->addr_end )
			ptcRuleNew->addr_end = (char*)malloc( IP_ADDR_STR_LEN );
		cgiFormStringNoNewlines( "address_range_end", ptcRuleNew->addr_end, IP_ADDR_STR_LEN );
		
		if( NULL == ptcRuleNew->mode )
			ptcRuleNew->mode = (char*)malloc( 32 );
		cgiFormStringNoNewlines( "range_mode", ptcRuleNew->mode, 32 );	
	}

	//流量
	if( NULL == ptcRuleNew->uplink_speed )
		ptcRuleNew->uplink_speed = (char*)malloc( TRAFFIC_STR_LEN );	
	cgiFormStringNoNewlines( "uplink_bandwidth", ptcRuleNew->uplink_speed, TRAFFIC_STR_LEN );
	
	if( NULL == ptcRuleNew->downlink_speed )
		ptcRuleNew->downlink_speed = (char*)malloc( TRAFFIC_STR_LEN );	
	cgiFormStringNoNewlines( "downlink_bandwidth", ptcRuleNew->downlink_speed, TRAFFIC_STR_LEN );
		
	//p2p流量
	//得到当前p2p是否有效
	if( cgiFormSuccess == cgiFormCheckboxSingle("usep2p") )
	{
		ptcRuleNew->useP2P = 1;	
	}
	else
	{
		ptcRuleNew->useP2P = 0;
	}	
	//根据useP2P状态,读取p2p流量限制
	if( 1 == ptcRuleNew->useP2P )
	{
		if( NULL == ptcRuleNew->p2p_uplink_speed )
			ptcRuleNew->p2p_uplink_speed = (char*)malloc( TRAFFIC_STR_LEN );	
		cgiFormStringNoNewlines( "p2p_uplink_bandwidth", ptcRuleNew->p2p_uplink_speed, TRAFFIC_STR_LEN );
		
		if( NULL == ptcRuleNew->p2p_downlink_speed )
			ptcRuleNew->p2p_downlink_speed = (char*)malloc( TRAFFIC_STR_LEN );	
		cgiFormStringNoNewlines( "p2p_downlink_bandwidth", ptcRuleNew->p2p_downlink_speed, TRAFFIC_STR_LEN );		
	}

#if USE_RULE_TIME_CTROL	
	//时间段控制
	cgiFormSelectSingle( "time_from", timeSelectStr, sizeof(timeSelectStr)/sizeof(char *), &resoult, 0 );
	ptcRuleNew->time_begin = resoult;
	cgiFormSelectSingle( "time_to", timeSelectStr, sizeof(timeSelectStr)/sizeof(char *), &resoult, 0 );
	ptcRuleNew->time_end = resoult;
#else
	ptcRuleNew->time_begin = 0;
	ptcRuleNew->time_end = 0;
#endif	
#if 1	
	//协议
{
	char *protocolValues[] = {"tcp","udp","icmp"};
	int result[sizeof(protocolValues)/sizeof(char*)];
	int invalid;
	//printf("<br /><br /><br />111ptcRuleNew->protocol = %x<br />", (unsigned int)ptcRuleNew->protocol );
	if( cgiFormNotFound != cgiFormCheckboxMultiple( "protocol", protocolValues, sizeof(protocolValues)/sizeof(char*), result, &invalid) )
	{
		if( NULL == ptcRuleNew->protocol )
			ptcRuleNew->protocol = (char*)malloc(64);
		memset( ptcRuleNew->protocol, 0, 64 );
		
		if( 1 == result[0] )
		{
			//printf( "tcp<br />" );
			strcat( ptcRuleNew->protocol, "tcp" );
		}
		if( 1 == result[1] )
		{
			//printf( "udp<br />" );
			strcat( ptcRuleNew->protocol, " udp" );
		}
		if( 1 == result[2] )
		{
			//printf( "icmp<br />" );
			strcat( ptcRuleNew->protocol, " icmp" );
		}
		
		//printf("<br /><br /><br />222ptcRuleNew->protocol = %s<br />", ptcRuleNew->protocol );
	} 
}	
	//interface
{
#define MAX_INTERFACE_STR_LEN	16
	if( NULL == ptcRuleNew->interface )
	{
		ptcRuleNew->interface = (char *)malloc( MAX_INTERFACE_STR_LEN );
	}
	memset( ptcRuleNew->interface, 0, MAX_INTERFACE_STR_LEN );
	cgiFormStringNoNewlines( "interfaceSelectorValue", ptcRuleNew->interface, MAX_INTERFACE_STR_LEN );

	if( NULL == ptcRuleNew->up_interface )
	{
		ptcRuleNew->up_interface = (char *)malloc( MAX_INTERFACE_STR_LEN );
	}
	memset( ptcRuleNew->up_interface, 0, MAX_INTERFACE_STR_LEN );
	cgiFormStringNoNewlines( "up_interfaceSelectorValue", ptcRuleNew->up_interface, MAX_INTERFACE_STR_LEN );	
}	
#endif

	//将设置好的数据转换成命令 
	//processRuleCmd( ptcRuleNew );
	
	return ptcRuleNew;
	
error:
	tcFreeRule( ptcRuleNew );
	return NULL;	
}


