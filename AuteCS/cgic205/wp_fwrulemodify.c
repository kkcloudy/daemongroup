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
* wp_fwrulemodify.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
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
#include "ws_firewall.h"


//#ifdef _MANAGE_FIREWALL_ 

int getRuleUserEdit(struct list *lfirewall, fwRuleList *list, int type );
int saveRule( fwRuleList **list );
int doAllRules( fwRuleList *list );
int delRule( fwRuleList *list, int type, int index );
int changRuleIndex( fwRuleList *list, int oldIndex, int newIndex, int ruleType );

int cgi_firewall_desc_is_legal_input(const char *str)
{
	const char *p;

	if (NULL == str || '\0' == str[0])
		return 0;
	
	for (p = str; *p; p++)
		if (!isalnum(*p) && *p != '_' && *p != '.')
			return 0;

	return 1;
}
//#endif

int cgiMain()
{
//#ifdef _MANAGE_FIREWALL_ 
	fwRuleList *list;

	struct list *lpublic;
	struct list *lfirewall;

	int ruleType = 0;
		//得到rule的类型， fileter dnat  snat input

	char ruleTypeStr[10];//FILTER   SNAT   DNAT	INPUT
	char delRuleIndex[10];
	char doFireWall[10];
	char *encry=(char *)malloc(BUF_LEN);
	char *str;
	char changeIndexOldRuleIndex[10];
	char changeIndexNewRuleIndex[10];

	lpublic=get_chain_head("../htdocs/text/public.txt");
	lfirewall=get_chain_head("../htdocs/text/firewall.txt");

 	memset(encry,0,BUF_LEN);
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
  	
  	str=dcryption(encry);

  	if(str==NULL)
  	{
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
  	}
	

	cgiHeaderContentType("text/html");
	
	list=(fwRuleList *)malloc(sizeof(fwRuleList));
	memset( list, 0, sizeof(fwRuleList) );
	//fwParseDoc( list );

	cgiFormStringNoNewlines( "ruleType", ruleTypeStr, sizeof(ruleTypeStr) );
	if( strcmp("FW_DNAT", ruleTypeStr) == 0 )
	{
		ruleType = FW_DNAT;
	}
	else if( strcmp("FW_SNAT",ruleTypeStr) == 0)
	{
		ruleType = FW_SNAT;
	}
	else if( strcmp("FW_INPUT",ruleTypeStr) == 0)
	{
		ruleType = FW_INPUT;
	}
	else
	{
		ruleType = FW_WALL;
	}	

	if(cgiFormSubmitClicked("submit_addrule") == cgiFormSuccess)
	{
		//getRuleUserEdit( lfirewall,list, ruleType );
		//saveRule( &list );
	}
	else if(cgiFormSubmitClicked("submit_doallrules") == cgiFormSuccess)
	{
		//firewall_chmod_conf_file();
		//fprintf( cgiOut, "do all rules!!!!" );
		//doAllRules( list );
		//saveRule( &list );
		//fprintf( cgiOut, "do all ok" );
		//调用上主界面
	}
	else if( cgiFormStringNoNewlines( "delRuleIndex", delRuleIndex, sizeof(delRuleIndex) ) == cgiFormSuccess )
	{
		int index;
		
		sscanf( delRuleIndex, "%d", (int *)&index );
		//delRule( list, ruleType, index-1 );
		//saveRule( &list );
	}
	else if( cgiFormStringNoNewlines( "doFireWall", doFireWall, sizeof(doFireWall) ) == cgiFormSuccess )
	{
		
		//firewall_chmod_conf_file();
		if( strcmp( doFireWall, "stop" ) == 0 )	
		{
		//stop fire wall
			if( 0 !=  fwServiceStop() )
			{
				fprintf( cgiOut, "<script type='text/javascript'>\n" );	
				fprintf( cgiOut, "alert('stop failed!');\n" );
				fprintf( cgiOut, "</script>\n" );
			}

		}
		else
		{
		//start fire wall
			//如果iptables没有start  先start   iptables
			system( "[ -e /opt/services/status/iptables_status.status ] || /opt/services/init/iptables_init start >/dev/null 2>&1" );
			//必须先将状态改为 start，因为doAllRules中会去检查状态，如果状态为stop就不能将规则加载。
			system( "echo \"start\" > /opt/services/status/firewall_status.status" );
			if( 0 != doAllRules( list ) )
			{
				system( "echo \"stop\" > /opt/services/status/firewall_status.status" );
				fprintf( cgiOut, "<script type='text/javascript'>\n" );
				fprintf( cgiOut, "alert('start failed!')\n" );
				fprintf( cgiOut, "</script>\n" );
			}
		}
	}
	else if( cgiFormStringNoNewlines( "oldIndex", changeIndexOldRuleIndex, sizeof(changeIndexOldRuleIndex) ) == cgiFormSuccess &&
		cgiFormStringNoNewlines( "newIndex", changeIndexNewRuleIndex, sizeof(changeIndexNewRuleIndex) ) == cgiFormSuccess )
	{
		//执行规则顺序调整的动作。
		int oldIndex;
		int newIndex;
		
		sscanf( changeIndexOldRuleIndex, "%d", (int *)&oldIndex );
		sscanf( changeIndexNewRuleIndex, "%d", (int *)&newIndex );
		
		//changRuleIndex( list, oldIndex-1, newIndex-1, ruleType );
		
		//saveRule( &list );
	}

//	else if(  )
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
  	
	fprintf( cgiOut, "<title>%s</title> \n", search( lfirewall, "title_fw" ) );
	fprintf( cgiOut, "<link rel=stylesheet href=/style.css type=text/css> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, ".usrlis {overflow-x:hidden; overflow:auto; width: 416px; height: 270px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, "tr.even td { \n" );
	fprintf( cgiOut, "background-color: #eee; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.odd td { \n" );
	fprintf( cgiOut, "background-color: #fff; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.changed td { \n" );
	fprintf( cgiOut, "background-color: #ffd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, " \n" ); 
	fprintf( cgiOut, "tr.new td { \n" );  
	fprintf( cgiOut, "background-color: #dfd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "</head> \n" );
	fprintf( cgiOut, "<script src=\"/fw.js\"></script> \n" );
		
	fprintf( cgiOut, "<body> \n" );
	
{
	//将当前页面转到ruleview!
	/*
	char changeURL[20]="abc";
	
	cgiFormStringNoNewlines( "changeURL", changeURL, sizeof(changeURL) );

	if( strcmp(changeURL,"no") != 0 )
	*/
	{
#if 1
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "window.location.href='wp_fwruleview.cgi?UN=%s&ruleType=%s';\n", encry, ruleTypeStr );
		fprintf( cgiOut, "</script>\n" );
#endif		
		fprintf( cgiOut, "</body>\n" );
		fprintf( cgiOut, "</html>\n" );
	}

}

	//fwFreeList(list);
	free(encry);
	release(lpublic); 
//#endif
	
	return 0;	
}

//#ifdef _MANAGE_FIREWALL_ 

int procesID( fwRulePtr head )
{
	fwRulePtr temp = head;	
	int i = 1;
	
	while( NULL != temp ) 
	{
		temp->id = i;
		//printf( "temp->name=%s<br />", temp->name );
		//printf( "temp->ineth=%s<br />", temp->ineth );
		i++;
		temp = temp->next;
	}
	return 0;
}

int saveRule( fwRuleList **list )
{
	procesID( (*list)->wall );
	procesID( (*list)->snat );
	procesID( (*list)->dnat );
	procesID( (*list)->input);
	
	//fwSaveDoc(*list);
	//fwFreeList(*list);


	*list = malloc(sizeof(fwRuleList));
	memset(*list,0,sizeof(fwRuleList));
	//fwParseDoc(*list);
//	procesID( (*list)->wall );
//	procesID( (*list)->snat );
//	procesID( (*list)->dnat );
	
	return 0;
}

//处理从ruleedit页面save后的流程：
int getRuleUserEdit (struct list *lfirewall, fwRuleList *list, int type )
{
	fwRulePtr rule,ruleLast;
	fwRulePtr ruletemp;
//	char *ruleIndex[] = {"1","2","3","4"};
//	char *ethIndex[] = {"any","eth0","eth1","eth2","eth3"};
	char *ipAddrTypeIndex[] = {"0","1","2","3","4","5"};
	char *ipProtocl[] = {"0","1","2","3","4"};
	char *portType[] = {"0","1","2","3"};
	char *filterAction[] = {"allow","deny","reject","tcpmss"};//
	int resoult,i;
	char editType[10];
	char **ruleEnableed;  
	
	//rule = fwNewRule(type,1);
		
	
	rule->type = type;
{
	char ruleIndex_str[32];
	cgiFormStringNoNewlines( "ruleIndexUserSelected", ruleIndex_str, sizeof(ruleIndex_str) );
	sscanf( ruleIndex_str, "%d", &(rule->id) );
}
	cgiFormStringNoNewlines( "editType", editType, sizeof(editType) );
	
	
	rule->ordernum = 65;//no use current
	
	//得到规则的名字
	rule->name = (char *)malloc(BUF_LEN);
	//strcpy(rule->name,"test new");、
	cgiFormStringNoNewlines( "ruleDescription", rule->name, BUF_LEN );

	if (!cgi_firewall_desc_is_legal_input(rule->name)){
		//vty_out(vty, "error desc format : %s\n", strDesc);
		ShowAlert(search( lfirewall, "ruleedit_err_rulename" ));
		return 0;
	}
	
	if( cgiFormNotFound != cgiFormStringMultiple("ruleEnabledCHECKBOX", &ruleEnableed) )
	{
		rule->enable = 1;	
	}
	else
	{
		rule->enable = 0;	
	}
	
	
	rule->comment = (char *)malloc(BUF_LEN); 
	strcpy(rule->comment,"FW_WALL_FWR:1:001");
	//cgiFormStringNoNewlines( "ruleDescription", rule->comment, BUF_LEN );
	
	rule->ineth = (char *)malloc(BUF_LEN);
	//strcpy(rule->ineth,"eth0");
	memset( rule->ineth, 0, BUF_LEN );
//	cgiFormSelectSingle( "inEth", ethIndex, 5, &resoult, 0);
//	strncpy( rule->ineth, ethIndex[resoult], BUF_LEN-1 );
	cgiFormStringNoNewlines( "inEthValue", rule->ineth, BUF_LEN );
//	printf( "inEthValue = %s<br />\n", rule->ineth );

	if (rule->type != FW_INPUT)
	{
		rule->outeth = (char *)malloc(BUF_LEN);
		//	strcpy(rule->outeth,"eth0");
		memset( rule->outeth, 0, BUF_LEN );
		//	cgiFormSelectSingle( "outEth", ethIndex, 5, &resoult, 0);
		//	strncpy( rule->outeth, ethIndex[resoult], BUF_LEN-1 );
		cgiFormStringNoNewlines( "outEthValue", rule->outeth, BUF_LEN );	
		//	printf( "outEthValue = %s<br />\n", rule->outeth );
	}
	//得到包匹配的原地址类型
	cgiFormRadio( "Package.SourceAddress", ipAddrTypeIndex, 6, &resoult, 0 ) ;
	rule->srctype = ipAddrTypeIndex[resoult][0]-'0';
	
	//根据ip地址类型得到ip地址
	rule->srcadd = (char *)malloc(BUF_LEN);
	memset( rule->srcadd, 0, BUF_LEN );
	switch(rule->srctype)
	{
		case FW_IPSINGLE:
			cgiFormStringNoNewlines( "Package.SourceAddress.Single.IP", rule->srcadd, BUF_LEN );
			break;
		case FW_IPHOST:
			cgiFormStringNoNewlines( "Package.SourceAddress.DefinedHost.Select", rule->srcadd, BUF_LEN );
			break;
		case FW_IPMASK:
			cgiFormStringNoNewlines( "Package.SourceAddress.AddrMask.Addr", rule->srcadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->srcadd ) > 2 )
			{
				strcat( rule->srcadd, "/" );
				cgiFormStringNoNewlines( "Package.SourceAddress.AddrMask.Mask", rule->srcadd+strlen(rule->srcadd), BUF_LEN-strlen(rule->srcadd) );
			}			
			break;
		case FW_IPNET:
			cgiFormStringNoNewlines( "Package.SourceAddress.DefinedNetwork", rule->srcadd, BUF_LEN );
			break;
		case FW_IPRANG:
			cgiFormStringNoNewlines( "Package.SourceAddress.AddressRange.Begin", rule->srcadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->srcadd ) > 2 )
			{
				strcat( rule->srcadd, "-" );
				cgiFormStringNoNewlines( "Package.SourceAddress.AddressRange.End", rule->srcadd+strlen(rule->srcadd), BUF_LEN-strlen(rule->srcadd) );
			}
			break;
		case 0:
		default:
			strcpy( rule->srcadd, "any" );
			break;	
	}
	
	//得到包匹配的目的地址类型
	cgiFormRadio( "Package.DestinationAddress", ipAddrTypeIndex, 6, &resoult, 0 ) ;
	rule->dsttype = ipAddrTypeIndex[resoult][0]-'0';
	
	//根据ip地址类型得到ip地址
	rule->dstadd = (char *)malloc(BUF_LEN);
	memset( rule->dstadd, 0, BUF_LEN );
	switch(rule->dsttype)
	{
		case FW_IPSINGLE:
			cgiFormStringNoNewlines( "Package.DestinationAddress.Single.IP", rule->dstadd, BUF_LEN );
			break;
		case FW_IPHOST:
			cgiFormStringNoNewlines( "Package.DestinationAddress.DefinedHost.Select", rule->dstadd, BUF_LEN );
			break;
		case FW_IPMASK:
			cgiFormStringNoNewlines( "Package.DestinationAddress.AddrMask.Addr", rule->dstadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->dstadd ) > 2 )
			{
				strcat( rule->dstadd, "/" );
				cgiFormStringNoNewlines( "Package.DestinationAddress.AddrMask.Mask", rule->dstadd+strlen(rule->dstadd), BUF_LEN-strlen(rule->dstadd) );
			}			
			break;
		case FW_IPNET:
			cgiFormStringNoNewlines( "Package.DestinationAddress.DefinedNetwork", rule->dstadd, BUF_LEN );
			break;
		case FW_IPRANG:
			cgiFormStringNoNewlines( "Package.DestinationAddress.AddressRange.Begin", rule->dstadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->dstadd ) > 2 )
			{
				strcat( rule->dstadd, "-" );
				cgiFormStringNoNewlines( "Package.DestinationAddress.AddressRange.End", rule->dstadd+strlen(rule->dstadd), BUF_LEN-strlen(rule->dstadd) );
			}
			break;
		case 0:
		default:
			strcpy( rule->dstadd, "any" );
			break;	
	}

	
	cgiFormSelectSingle( "Protocol", ipProtocl, 5, &resoult, 0 );
	rule->protocl = resoult;//FW_PTCP;
	if( 1 == rule->protocl || 2 == rule->protocl || 3 == rule->protocl )
	{
		//收集端口信息
		cgiFormRadio( "Package.SourcePort", portType, 4, &resoult, 0 ) ;
		rule->sptype = resoult;
		rule->sport = (char *)malloc(BUF_LEN);
		memset( rule->sport, 0, BUF_LEN );
		switch( rule->sptype )
		{
			case 1:
				cgiFormStringNoNewlines( "Package.SourcePort.Single.Port", rule->sport, BUF_LEN );
				break;
			case 2:
				cgiFormStringNoNewlines( "Package.SourcePort.Range.Begin", rule->sport, BUF_LEN );
				strcat( rule->sport, ":" );
				cgiFormStringNoNewlines( "Package.SourcePort.Range.End", rule->sport+strlen(rule->sport), BUF_LEN-strlen(rule->sport) );
				break;
			case 3:
				strcpy( rule->sport, "port collection" );
				break;
			case 0:
			default:
				strcpy( rule->sport, "any" );
				break;
		}

		cgiFormRadio( "Package.DestinationPort", portType, 4, &resoult, 0 ) ;
		rule->dptype = resoult;
		rule->dport = (char *)malloc(BUF_LEN);
		memset( rule->dport, 0, BUF_LEN );
		switch( rule->dptype )
		{
			case 1:
				cgiFormStringNoNewlines( "Package.DestinationPort.Single.Port", rule->dport, BUF_LEN );
				break;
			case 2:
				cgiFormStringNoNewlines( "Package.DestinationPort.Range.Begin", rule->dport, BUF_LEN );
				strcat( rule->dport, ":" );
				cgiFormStringNoNewlines( "Package.DestinationPort.Range.End", rule->dport+strlen(rule->dport), BUF_LEN-strlen(rule->dport) );
				break;
			case 3:
				strcpy( rule->dport, "port collection" );
				break;
			case 0:
			default:
				strcpy( rule->dport, "any" );
				break;
		}
	}
	//state
	{
		char *states[]={"NEW","ESTABLISHED","RELATED","INVALID"};
		int invalid[]={0,0,0,0};
		int result[4];
		int i;
		
		cgiFormCheckboxMultiple( "state", states, 4, result, invalid );
		if( result[0] != 0 || result[1] != 0 || result[2] != 0 || result[3] != 0  )
		{
			if( rule->pkg_state == NULL )
			{
				rule->pkg_state = (char *)malloc( 64 );
				if( NULL == rule->pkg_state )
				{
					return -1;	
				}
				memset( rule->pkg_state, 0, 64 );
				for( i=0;i<4;i++ )
				{
					if( 1 == result[i] )
					{
						if( 0 == strlen(rule->pkg_state) )
						{
							strcat( rule->pkg_state, states[i] );	
						}
						else
						{
							strcat( rule->pkg_state, "," );
							strcat( rule->pkg_state, states[i] );
						}
					}		
				}
			}
		}
	//	fprintf( cgiOut,"<script type=text/javascript>\n" );
	//	fprintf( cgiOut,"alert( '%s' );\n", rule->pkg_state );
	//	fprintf( cgiOut,"</script>\n" );
	}
	//end state
	
	//string_filter
	{
		if( NULL == rule->string_filter )
		{
			rule->string_filter	= (char*)malloc(BUF_LEN);
			if( NULL == rule->string_filter )
			{
				return -1;	
			}
		}
		memset( rule->string_filter, 0, BUF_LEN );
		cgiFormStringNoNewlines( "string_filter", rule->string_filter, BUF_LEN );
	}
	//end string_filter
	
	switch( type )
	{
		
		case FW_SNAT://SNAT
		case FW_DNAT://DNAT
			
			cgiFormRadio( "NatTranslation.IP", ipAddrTypeIndex, 6, &resoult, 0 ) ;
			rule->natiptype = resoult;
			rule->natipadd = (char *)malloc(BUF_LEN);
			memset( rule->natipadd, 0, BUF_LEN );
			switch(rule->natiptype)
			{
				case FW_IPSINGLE:
					cgiFormStringNoNewlines( "NatSingleIpAddress", rule->natipadd, BUF_LEN );
					break;
				case FW_IPHOST:
					//cgiFormStringNoNewlines( "Package.DestinationAddress.DefinedHost.Select", rule->natipadd, BUF_LEN );
					break;
				case FW_IPMASK:
					cgiFormStringNoNewlines( "NatAddressNetmaskAddr", rule->natipadd, BUF_LEN );
					if( BUF_LEN - strlen( rule->natipadd ) > 2 )
					{
						strcat( rule->natipadd, "/" );
						cgiFormStringNoNewlines( "NatAddressNetmaskMask", rule->natipadd+strlen(rule->natipadd), BUF_LEN-strlen(rule->natipadd) );
					}			
					break;
				case FW_IPNET:
					//cgiFormStringNoNewlines( "Package.DestinationAddress.DefinedNetwork", rule->natipadd, BUF_LEN );
					break;
				case FW_IPRANG:
					cgiFormStringNoNewlines( "NatAddrRangeBegin", rule->natipadd, BUF_LEN );
					if( BUF_LEN - strlen( rule->natipadd ) > 2 )
					{
						strcat( rule->natipadd, "-" );
						cgiFormStringNoNewlines( "NatAddrRangeEnd", rule->natipadd+strlen(rule->natipadd), BUF_LEN-strlen(rule->natipadd) );
					}
					break;
				case 0:
				default:
					strcpy( rule->natipadd, "any" );
					break;	
			}
						
			if( 1 == rule->protocl || 2 == rule->protocl || 3 == rule->protocl )//如果设置了协议  有端口设置
			{
				//rule->natpttype = FW_PTSINGLE;
				cgiFormRadio( "NatTranslation.Port", ipAddrTypeIndex, 6, &resoult, 0 ) ;
				rule->natpttype = resoult;
				
				rule->natport = (char *)malloc(BUF_LEN);
				memset( rule->natport, 0, BUF_LEN );
				switch( resoult )
				{
					case 1:
						cgiFormStringNoNewlines( "NatSinglePort", rule->natport, BUF_LEN );
						break;
					case 2:
						cgiFormStringNoNewlines( "NatPortRangBegin", rule->natport, BUF_LEN );
						if( BUF_LEN - strlen( rule->natport ) > 2 )
						{
							strcat( rule->natport, "-" );
							cgiFormStringNoNewlines( "NatPortRangEnd", rule->natport+strlen(rule->natport), BUF_LEN-strlen(rule->natport) );
						}						
						break;
					case 0:
					default:
						strcat( rule->natport, "any" );
						break;
				}
			}
			break;
		case FW_WALL:// fire wall
		case FW_INPUT:
		default:
			cgiFormSelectSingle( "filterAction", filterAction, 4, &resoult, 0);
			rule->act = resoult;
			//tcpmss
			if( 3 == rule->act && rule->type != FW_INPUT)
			{
				rule->tcpmss_var = (char*)malloc(BUF_LEN);
				memset( rule->tcpmss_var, 0, BUF_LEN );
				cgiFormStringNoNewlines( "tcpmss_val", rule->tcpmss_var, BUF_LEN );
			}
			break;
	}

	i = rule->id;
	switch( type )
	{		
		case FW_DNAT:
			
			ruleLast = list->dnat;
			if( NULL == ruleLast )
			{
				rule->status = FW_NEW;
				list->dnat = rule;
			}
			else if( (1 == i) && (strcmp(editType,"add") == 0 ) )
			{
				rule->status = FW_NEW;
				rule->next = list->dnat;
				list->dnat = rule;
			}
			else if( (1==i) && (strcmp(editType,"edit") == 0 ) )
			{
				rule->status = FW_CHANGED;
				rule->next = list->dnat->next;
				list->dnat = rule;
			}	
			else
			{
				for( ; (ruleLast->next != 0) && (i>2); ruleLast=ruleLast->next, i-- ){;}
				
				if( strcmp(editType,"add") == 0 )
				{
					rule->status = FW_NEW;
					ruletemp = ruleLast->next;	
					ruleLast->next = rule;
					rule->next = ruletemp;					
				}
				else
				{
					rule->status = FW_CHANGED;
					ruletemp = ruleLast->next->next;
					free( ruleLast->next );
					ruleLast->next = rule;
					rule->next = ruletemp;
				}
			}
			break;
		case FW_SNAT:
			
			ruleLast = list->snat;
			if( NULL == ruleLast )
			{
				rule->status = FW_NEW;
				list->snat = rule;
			}
			else if( (1 == i) && (strcmp(editType,"add") == 0 ) )
			{
				rule->status = FW_NEW;
				rule->next = list->snat;
				list->snat = rule;
			}
			else if( (1==i) && (strcmp(editType,"edit") == 0 ) )
			{
				rule->status = FW_CHANGED;
				rule->next = list->snat->next;
				list->snat = rule;
			}		
			else
			{
				for( ; (ruleLast->next != 0) && (i>2); ruleLast=ruleLast->next, i-- ){;}
				
				if( strcmp(editType,"add") == 0 )
				{
					rule->status = FW_NEW;
					ruletemp = ruleLast->next;	
					ruleLast->next = rule;
					rule->next = ruletemp;					
				}
				else
				{
					rule->status = FW_CHANGED;
					ruletemp = ruleLast->next->next;
					free( ruleLast->next );
					ruleLast->next = rule;
					rule->next = ruletemp;
				}
			}
			break;
		case FW_INPUT:
			
			ruleLast = list->input;
			if( NULL == ruleLast )
			{
				rule->status = FW_NEW;
				list->input = rule;
			}
			else if( (1 == i) && (strcmp(editType,"add") == 0 ) )
			{
				rule->status = FW_NEW;
				rule->next = list->input;
				list->input = rule;
			}
			else if( (1==i) && (strcmp(editType,"edit") == 0 ) )
			{
				rule->status = FW_CHANGED;
				rule->next = list->input->next;
				list->input = rule;
			}		
			else
			{
				for( ; (ruleLast->next != 0) && (i>2); ruleLast=ruleLast->next, i-- ){;}
				
				if( strcmp(editType,"add") == 0 )
				{
					rule->status = FW_NEW;
					ruletemp = ruleLast->next;	
					ruleLast->next = rule;
					rule->next = ruletemp;					
				}
				else
				{
					rule->status = FW_CHANGED;
					ruletemp = ruleLast->next->next;
					free( ruleLast->next );
					ruleLast->next = rule;
					rule->next = ruletemp;
				}
			}
			break;
		case FW_WALL:
		default:			
			
			ruleLast = list->wall;
			if( NULL == ruleLast )
			{
				rule->status = FW_NEW;
				list->wall = rule;
			}
			else if( (1 == i) && (strcmp(editType,"add") == 0 ) )
			{
				rule->status = FW_NEW;
				rule->next = list->wall;
				list->wall = rule;
			}
			else if( (1==i) && (strcmp(editType,"edit") == 0 ) )
			{
				rule->status = FW_CHANGED;
				rule->next = list->wall->next;
				list->wall = rule;
			}
			else
			{
				for( ; (ruleLast->next != 0) && (i>2); ruleLast=ruleLast->next, i-- ){;}
				
				if( strcmp(editType,"add") == 0 )
				{
					rule->status = FW_NEW;
					ruletemp = ruleLast->next;	
					ruleLast->next = rule;
					rule->next = ruletemp;					
				}
				else
				{
					rule->status = FW_CHANGED;
					ruletemp = ruleLast->next->next;
					free( ruleLast->next );
					ruleLast->next = rule;
					rule->next = ruletemp;
				}
			}
			break;
	}

//	fwReplaceIptablesRule(list);
	return 0;
}



//处理从当前页面点击save后的流程：
int doAllRules( fwRuleList *list )
{
	int err;
	fwRulePtr rule;
	//err = fwReplaceIptablesRule(list);

	if( 0 != err )
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
	switch( err )
	{
		#if 0
		case 0:
			break;
		case FWE_START_FAILE:
			fprintf( cgiOut, "alert('启动服务失败!');\n" );
			break;
		case FWE_FLUSH_FAILE:
			fprintf( cgiOut, "alert('清空规则失败!');\n" );
			break;
		case FWE_RULE_FAILE:
			fprintf( cgiOut, "alert('规则执行出错!');\n" );
			break;
		case FWE_SAVE_FAILE:
			fprintf( cgiOut, "alert('保存配置失败!');\n" );
			break;
		case FWE_STOP_FAILE:
			fprintf( cgiOut, "alert('关闭服务失败!');\n" );
			break;
		default:
			fprintf( cgiOut, "alert('未知错误!');\n" );
			break;
	#endif
	}
	if( 0 != err )
		fprintf( cgiOut, "</script>\n" );


	if( 0 == err )//所有规则执行成功
	{
		rule = list->wall;
		while( NULL != rule )
		{
			rule->status = FW_DONE;	
			rule = rule->next;
		}
		rule = list->snat;
		while( NULL != rule )
		{
			rule->status = FW_DONE;	
			rule = rule->next;
		}
		rule = list->dnat;
		while( NULL != rule )
		{
			rule->status = FW_DONE;	
			rule = rule->next;
		}
		rule = list->input;
		while( NULL != rule )
		{
			rule->status = FW_DONE;	
			rule = rule->next;
		}
	}
	
	return err;
}


int delRule( fwRuleList *list, int type, int index )
{
	fwRulePtr ruleDel,ruleDelPrev;
	if( NULL == list )
	{
		return -1;
	}
	
	switch( type )
	{
		case FW_WALL:
			ruleDel = list->wall;
			break;
		case FW_SNAT:
			ruleDel = list->snat;
			break;
		case FW_DNAT:
			ruleDel = list->dnat;
			break;
		case FW_INPUT:
			ruleDel = list->input;
			break;
		default:
			return -1;
			break;		
	}
	ruleDelPrev = NULL;
	
	if( 0 == index && NULL != ruleDel )
	{
		switch( type )
		{
			case FW_WALL:
				list->wall = ruleDel->next;
				break;	
			case FW_SNAT:
				list->snat = ruleDel->next;
				break;
			case FW_DNAT:
				list->dnat = ruleDel->next;
				break;
			case FW_INPUT:
				list->input = ruleDel->next;
				break;
			default:
				return -1;
				break;
		}
	}
	else
	{
		
		for( ; ((index > 0)&(ruleDel != NULL)); index-- )
		{
			ruleDelPrev = ruleDel;
			ruleDel = ruleDel->next;
		}
		if( 0 == index && NULL != ruleDel )
		{
			ruleDelPrev->next = ruleDel->next;
		}
	}
	
	
	//fwFreeRule(ruleDel);
	//fwSaveDoc(list);
	return 0;
}



fwRulePtr getRuleByIndex( fwRulePtr head, int index )
{
	fwRulePtr ret;
	
	ret = head;
	if( index <= 0 )
		return ret;

	for( ;(index>0)&&(ret->next!=NULL); index-- )
	{
		ret = ret->next;
	}
	
	return ret;
}


int changRuleIndex( fwRuleList *list, int oldIndex, int newIndex, int ruleType )
{
	fwRulePtr curRule,*root;
	fwRulePtr insertPos;
	
	if( oldIndex < 0 || newIndex < 0 || oldIndex == newIndex )
		return -1;
	
	if( FW_WALL == ruleType )
	{
		root = &(list->wall);
	}
	else if( FW_SNAT == ruleType )
	{
		root = &(list->snat);
	}
	else if( FW_DNAT == ruleType )
	{
		root = &(list->dnat);
	}
	else if( FW_INPUT == ruleType )
	{
		root = &(list->input);
	}
	
	if( 0 == oldIndex )
	{
		//先将curRule断开
		curRule = *root;
		*root = curRule->next;
		curRule->next = NULL;
		
		//将curRule插入到需要的位置
		insertPos = getRuleByIndex( *root, newIndex-1 );
		curRule->next = insertPos->next;
		insertPos->next = curRule;
	}
	else if( 0 == newIndex )
	{
		//先将curRule断开
		insertPos = getRuleByIndex( *root, oldIndex-1 );
		curRule = insertPos->next;
		insertPos->next = curRule->next;
		//将curRule插入到开头
		curRule->next = *root;
		*root = curRule;
	}
	else
	{
		//先将curRule断开
		insertPos = getRuleByIndex( *root, oldIndex-1 );
		curRule = insertPos->next;
		insertPos->next = curRule->next;

		//找到要插入的位置
		insertPos = getRuleByIndex( *root, newIndex-1 );
		curRule->next = insertPos->next ;
		insertPos->next = curRule;

	}
	
	//fwSaveDoc(list);
	return 0;
}

//#endif



