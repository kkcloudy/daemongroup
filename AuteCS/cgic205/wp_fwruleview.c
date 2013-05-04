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
* wp_fwruleview.c
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

//#define _MANAGE_FIREWALL_ 1

//#ifdef _MANAGE_FIREWALL_ 

int fwServiceStatus();

#define CMD_STATUS_IS_START "read fwstatus < /opt/services/status/firewall_status.status && test $fwstatus = start"

#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)


#define TABLE_HEAD	"<table frame=below rules=rows border=1 style='border-collapse:collapse;align:left;overflow:visible'>\n"\
					"	<tr height=25 align=left>\n"\
					"		<th width=20></th>\n"\
					"		<th width=90px><font id=%s>%s</font></th>\n"\
					"		<th width=360px><font id=%s>%s</font></th>\n"\
					"		<th width=80px><font id=%s>%s</font></th>\n"\
					"		<th width=120px><font id=%s>%s</font></th>\n"\
					"		<th width=20px></th>\n"\
					"		<th width=80px></th>\n"\
					"	</tr>\n"\
					"	<tr height=25>\n"\
					"	</tr>\n"
					
#define RULEPOSTION_SELECTOR	"<script type=text/javascript>\n"\
					"	var code=ruleIndexSelector( %d, %d, '%s', '%s' );\n"\
					"	document.write( code );\n"\
					"	</script>"
					
#define POP_MENU	"<script type=text/javascript>\n"\
					"	var %s=new popMenu( '%s','%d');\n"\
					"	%s.addItem( new popMenuItem( '%s', '%s' ) );\n"\
					"	%s.addItem( new popMenuItem( '%s', '%s' ) );\n"\
					"	%s.show();\n"\
					"</script>"
					 
#define TABLE_LINE 	"<tr height=25 bgcolor=%s><td>\n"\
					"		<div style='cursor:hand;align:left;'"\
					" onclick='procDetail(this,\"rule%s%d\",fwrules%s[%d]);'>+</div></td>\n"\
					"		<td>%s</td>\n"\
					"		<td>%s</td>\n"\
					"		<td>%s</td>\n"\
					"		<td>%s</td>\n"\
					"		<td>%s</td>\n"\
					"		<td></td>\n"\
					"	</tr>\n"\
					"	<tr>\n"\
					"		<td colspan='7'><div id='rule%s%d'></div>"\
					"	</tr>\n"


//又打补丁~~
char *int2str( unsigned int a )
{
	static char abc[128];
	
	memset( abc, 0, sizeof(abc) );
	sprintf( abc, "%u", a );
	
	return abc;
}
//ndif
int cgiMain()
{
//#ifdef _MANAGE_FIREWALL_ 
	fwRuleList *list;
	fwRule *rule;
	struct list *lpublic;
	struct list *lfirewall;
	int i=0;
	int ruleType = 0;
		//得到rule的类型， fileter dnat  snat input
	char ruleTypeStr[10];//FILTER   SNAT   DNAT	INPUT
	char *encry=(char *)malloc(BUF_LEN);
	char *str;
	int manager=-1;
		
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
	
  	manager = checkuser_group(str);
	cgiHeaderContentType("text/html");
	  	
	list=(fwRuleList *)malloc(sizeof(fwRuleList));
	memset( list, 0, sizeof(fwRuleList) );
	//ParseDoc( list );

	cgiFormStringNoNewlines( "ruleType", ruleTypeStr, sizeof(ruleTypeStr) );
	if( strcmp("FW_DNAT", ruleTypeStr) == 0 )
	{
		ruleType = FW_DNAT;
	}
	else if( strcmp("FW_SNAT",ruleTypeStr) == 0)
	{
		
		ruleType = FW_SNAT;
	}
	// add by chensheng, on 2010-04-15, for input chain 
	else if (strcmp("FW_INPUT", ruleTypeStr) == 0)
	{
		ruleType = FW_INPUT;
	}
	else
	{
		ruleType = FW_WALL;
		strcpy( ruleTypeStr, "FW_WALL" );
	}


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
	fprintf( cgiOut, "</head> \n" );
	fprintf( cgiOut, "<script src=\"/fw.js\"></script> \n" );
	fprintf( cgiOut, "<script type='text/javascript'>\n" );
	fprintf( cgiOut, "function autoCheckHeight()\n" );
	fprintf( cgiOut, "{\n");//-75是因为又加了连接左边的高度增加了
	fprintf( cgiOut, "	if( document.getElementById('list').offsetHeight > 150 )\n" );
	fprintf( cgiOut, "		document.getElementById('FILL_OBJ').style.height = document.getElementById('list').offsetHeight - 100;\n");
	fprintf( cgiOut, "	else\n"\
					"		document.getElementById('FILL_OBJ').style.height = 1;" );
	fprintf( cgiOut, "}\n");

	switch( ruleType )
	{
		case FW_DNAT:
		case FW_SNAT:
//添加snat
			rule = list->snat;
			fprintf(  cgiOut, "var fwrulesFW_SNAT = new Array(); \n" );
			if( rule != NULL )
			{
				while( (void*)0 != rule )
				{	
	// type  id   ordernum	 *name	 enable 	status	*comment   *ineth	*outeth
	// srctype	*srcadd    dsttype	 *dstadd	protocl    sptype	*sport	 dptype   *dport
	// act	  natiptype 	*natipadd	  natpttype    *natport
					fprintf( cgiOut, "fwrulesFW_SNAT[%d] = new fwrule( \"%d\", \"%d\",\"%d\",", i, rule->type, rule->id, rule->ordernum );
					fprintf( cgiOut, "\"%s\", \"%d\", \"%d\", \"%s\", \"%s\", \"%s\", ", rule->name, rule->enable, rule->status, rule->comment, rule->ineth, rule->outeth );
					fprintf( cgiOut, "\"%d\", \"%s\", \"%d\", \"%s\", \"%d\", \"%d\",\"%s\", \"%d\", \"%s\", ", rule->srctype, rule->srcadd, rule->dsttype, rule->dstadd, rule->protocl, rule->sptype, rule->sport, rule->dptype, rule->dport );
					fprintf( cgiOut, "\"%d\", \"%d\",\"%s\",\"%d\",\"%s\", '%s', '%s' );\n", rule->act, rule->natiptype, rule->natipadd, rule->natpttype, rule->natport, rule->pkg_state, rule->string_filter );			
					
					//fprintf( cgiOut, "alert(fwrulesFW_SNAT[%d]->type);\n", i );
					rule = rule->next;
					i++;
				}
			}

			rule = list->dnat;
			i = 0;
			fprintf(  cgiOut, "var fwrulesFW_DNAT = new Array(); \n" );
			if( rule != NULL )
			{
				while( (void*)0 != rule ){
	//		ddd sdd sss  dsd  sdd sds dds ds
	
	// type  id   ordernum   *name   enable     status  *comment   *ineth   *outeth
	// srctype  *srcadd    dsttype   *dstadd    protocl    sptype   *sport   dptype   *dport
	// act    natiptype     *natipadd     natpttype    *natport
					fprintf( cgiOut, "fwrulesFW_DNAT[%d] = new fwrule( \"%d\", \"%d\",\"%d\",", i, rule->type, rule->id, rule->ordernum );
					fprintf( cgiOut, "\"%s\", \"%d\", \"%d\", \"%s\", \"%s\", \"%s\", ", rule->name, rule->enable, rule->status, rule->comment, rule->ineth, rule->outeth );
					fprintf( cgiOut, "\"%d\", \"%s\", \"%d\", \"%s\", \"%d\", \"%d\",\"%s\", \"%d\", \"%s\", ", rule->srctype, rule->srcadd, rule->dsttype, rule->dstadd, rule->protocl, rule->sptype, rule->sport, rule->dptype, rule->dport );
					fprintf( cgiOut, "\"%d\", \"%d\",\"%s\",\"%d\",\"%s\", '%s', '%s' );\n", rule->act, rule->natiptype, rule->natipadd, rule->natpttype, rule->natport, rule->pkg_state, rule->string_filter );			
					
					//fprintf( cgiOut, "alert(fwrulesFW_DNAT[%d]->type);\n", i );
					rule = rule->next;
					i++;
				}
			}
			break;
		// add by chensheng, on 2010-04-15, for input chain 
		case FW_INPUT:
			rule = list->input;
			fprintf(  cgiOut, "var fwrulesFW_INPUT = new Array(); \n" );
			if( rule != NULL )
			{
				while( (void*)0 != rule )
				{	
	// type  id   ordernum   *name   enable     status  *comment   *ineth   *outeth
	// srctype  *srcadd    dsttype   *dstadd    protocl    sptype   *sport   dptype   *dport
	// act    natiptype     *natipadd     natpttype    *natport
					fprintf( cgiOut, "fwrulesFW_INPUT[%d] = new fwrule( \"%d\", \"%d\",\"%d\",", i, rule->type, rule->id, rule->ordernum );
					fprintf( cgiOut, "\"%s\", \"%d\", \"%d\", \"%s\", \"%s\", \"%s\", ", rule->name, rule->enable, rule->status, rule->comment, rule->ineth, rule->outeth );
					fprintf( cgiOut, "\"%d\", \"%s\", \"%d\", \"%s\", \"%d\", \"%d\",\"%s\", \"%d\", \"%s\", ", rule->srctype, rule->srcadd, rule->dsttype, rule->dstadd, rule->protocl, rule->sptype, rule->sport, rule->dptype, rule->dport );
					fprintf( cgiOut, "\"%d\", \"%d\",\"%s\",\"%d\",\"%s\", '%s', '%s' );\n", rule->act, rule->natiptype, rule->natipadd, rule->natpttype, rule->natport, rule->pkg_state, rule->string_filter );			
					
					//fprintf( cgiOut, "alert('%s');\n", rule->string_filter );
					rule = rule->next;
					i++;
				}
			}
			break;
		case FW_WALL:
		default:
			rule = list->wall;
			fprintf(  cgiOut, "var fwrulesFW_WALL = new Array(); \n" );
			if( rule != NULL )
			{
				while( (void*)0 != rule )
				{	
	// type  id   ordernum   *name   enable     status  *comment   *ineth   *outeth
	// srctype  *srcadd    dsttype   *dstadd    protocl    sptype   *sport   dptype   *dport
	// act    natiptype     *natipadd     natpttype    *natport
					fprintf( cgiOut, "fwrulesFW_WALL[%d] = new fwrule( \"%d\", \"%d\",\"%d\",", i, rule->type, rule->id, rule->ordernum );
					fprintf( cgiOut, "\"%s\", \"%d\", \"%d\", \"%s\", \"%s\", \"%s\", ", rule->name, rule->enable, rule->status, rule->comment, rule->ineth, rule->outeth );
					fprintf( cgiOut, "\"%d\", \"%s\", \"%d\", \"%s\", \"%d\", \"%d\",\"%s\", \"%d\", \"%s\", ", rule->srctype, rule->srcadd, rule->dsttype, rule->dstadd, rule->protocl, rule->sptype, rule->sport, rule->dptype, rule->dport );
					fprintf( cgiOut, "\"%d\", \"%d\",\"%s\",\"%d\",\"%s\", '%s', '%s' );\n", rule->act, rule->natiptype, rule->natipadd, rule->natpttype, rule->natport, rule->pkg_state, rule->string_filter );			
					
					//fprintf( cgiOut, "alert('%s');\n", rule->string_filter );
					rule = rule->next;
					i++;
				}
			}
			break;
		}
	fprintf( cgiOut, "</script>\n" );
	
	fprintf( cgiOut, "<body> \n" );

	fprintf( cgiOut, "<form action='wp_fwrulemodify.cgi' method='post'> \n" );
	fprintf( cgiOut, "<input type='hidden' name='UN' value='%s'>", encry );
	fprintf( cgiOut, "<input type='hidden' name='ruleType' value='%s'>", ruleTypeStr );
	fprintf( cgiOut, "<div align=center> \n" );
	fprintf( cgiOut, "<table width=976 border=0 cellpadding=0 cellspacing=0> \n" );
	fprintf( cgiOut, "<tr><td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td> \n" );
	fprintf( cgiOut, "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td> \n" );
	if( FW_WALL == ruleType )
	{
		fprintf( cgiOut, "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s </td> \n", search(lpublic,"title_style"), search( lfirewall, "title_fw_rule") );
	}
	else if( FW_INPUT == ruleType )
	{
		fprintf( cgiOut, "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s </td> \n", search(lpublic,"title_style"), search( lfirewall, "title_input") );
	}
	else
	{
		fprintf( cgiOut, "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s </td> \n", search(lpublic,"title_style"), search( lfirewall, "title_nat") );
	}
	fprintf( cgiOut, "<td width=690 align=right valign=bottom background=/images/di22.jpg> \n" );
	

	fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
	"<tr>\n" );
	if( 0 == manager )
		fprintf( cgiOut, "<td width=62 align=center><input id='but' type='submit' name='submit_doallrules' style='background-image:url(/images/%s)' value=''></td>\n", search(lpublic,"img_ok") );			
	else
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_firewall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
	fprintf(cgiOut,"<td width=62 align=left><a href=wp_firewall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
	fprintf(cgiOut,"</tr>"\
	"</table>");

	fprintf( cgiOut, "</td> \n" );
	fprintf( cgiOut, "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td> \n" );
	fprintf( cgiOut, "</tr> \n" );
	fprintf( cgiOut, "<tr><td colspan=5 align=center valign=middle> \n" );
	fprintf( cgiOut, "<table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0> \n" );
	fprintf( cgiOut, "<tr><td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td> \n" );
	fprintf( cgiOut, "<td width=948> \n" );
	fprintf( cgiOut, "<table width=947 border=0 cellspacing=0 cellpadding=0> \n" );
	fprintf( cgiOut, "<tr height=4 valign=bottom><td width=120>&nbsp;</td> \n" );
	fprintf( cgiOut, "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td> \n" );
	fprintf( cgiOut, "</tr> \n" );
	fprintf( cgiOut, "<tr><td> \n" );
	fprintf( cgiOut, "<table width=120 border=0 cellspacing=0 cellpadding=0> \n" );
	fprintf( cgiOut, "<tr height=25><td id=tdleft>&nbsp;</td></tr> \n" );
	//if( ruleType != FW_DNAT && ruleType != FW_SNAT )
	if (FW_WALL == ruleType )
	{
		fprintf( cgiOut, "<tr height=26><td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td></tr> \n", search(lpublic,"menu_san"), search( lfirewall, "title_fw_rule") );
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_SNAT' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n", encry, search(lpublic,"menu_san"), search( lfirewall, "title_nat") );
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_INPUT' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n", encry, search(lpublic,"menu_san"), search( lfirewall, "title_input") );
	}
	else if (FW_SNAT == ruleType || FW_DNAT == ruleType)
	{
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_WALL' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n",encry, search(lpublic,"menu_san"), search( lfirewall, "title_fw_rule") );	
		fprintf( cgiOut, "<tr height=26><td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td></tr> \n", search(lpublic,"menu_san"), search( lfirewall, "title_nat") );
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_INPUT' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n",encry, search(lpublic,"menu_san"), search( lfirewall, "title_input") );
	}
	else 
	{
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_WALL' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n",encry, search(lpublic,"menu_san"), search( lfirewall, "title_fw_rule") );
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_SNAT' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n",encry, search(lpublic,"menu_san"), search( lfirewall, "title_nat") );
		fprintf( cgiOut, "<tr height=26><td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td></tr> \n", search(lpublic,"menu_san"), search( lfirewall, "title_input") );
	}
	//添加各种规则的入口
	if( 0 == manager )
	{
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=FW_WALL&editType=add' target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></a></td></tr> \n",encry,list->iWallTotalNum+1, list->iWallTotalNum, search(lpublic,"menu_san"), search( lfirewall, "add_new"),search(lpublic,"menu_san"), search( lfirewall, "title_fw"),search(lpublic,"menu_san"), search( lfirewall, "add_rule") );
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=FW_SNAT&editType=add' target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></a></td></tr> \n",encry,list->iSNATTotalNum+1, list->iSNATTotalNum, search(lpublic,"menu_san"), search( lfirewall, "add_new"),search(lpublic,"menu_san"), "SNAT ",search(lpublic,"menu_san"), search( lfirewall, "add_rule") );
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=FW_DNAT&editType=add' target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></a></td></tr> \n",encry,list->iDNATTotalNum+1, list->iDNATTotalNum, search(lpublic,"menu_san"), search( lfirewall, "add_new"),search(lpublic,"menu_san"), "DNAT ",search(lpublic,"menu_san"), search( lfirewall, "add_rule") );
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=FW_INPUT&editType=add' target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></a></td></tr> \n",encry,list->iInputTotalNum+1, list->iInputTotalNum, search(lpublic,"menu_san"), search( lfirewall, "add_new"),search(lpublic,"menu_san"), "INPUT ",search(lpublic,"menu_san"), search( lfirewall, "add_rule") );
	}
	else
	{
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft>&nbsp;</td></tr><tr height=25><td align=left id=tdleft>&nbsp;</td></tr><tr height=25><td align=left id=tdleft>&nbsp;</td></tr>\n" );
	}	
	/*
	fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_WALL' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n",encry, search(lpublic,"menu_san"), search( lfirewall, "title_fw_rule") );	
	fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_WALL' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n",encry, search(lpublic,"menu_san"), search( lfirewall, "title_fw_rule") );	 */
	//三种规则入口结束
	
	fprintf( cgiOut, "<tr><td align=left id='FILL_OBJ' style='height:100px;padding-left:10px;border-right:1px solid #707070;color:#000000;text-align:center'>&nbsp;</td></tr> \n" );
       	
	fprintf( cgiOut, "</table> \n" );
	fprintf( cgiOut, "</td> \n" );
	fprintf( cgiOut, "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
					"<div id=\"list\" align=\"left\" style=\"width:768px;overflow:auto\">\n");
	//输出表格。
	//当前类型
{
	char str_RULEPOSTION_SELECTOR[512];
	char str_POP_MENU[512]; 
	char popMenuName[32];
	char str_url_del[128];
	char str_url_edit[128];

	if( FW_WALL == ruleType )
	{
		//启动关闭防火墙的按钮～～～
		if( 0 == manager )
		{
			if( 0 == fwServiceStatus() )//running
			{
				fprintf( cgiOut, "<span>%s:%s ! <a href='wp_fwrulemodify.cgi?UN=%s&doFireWall=stop'><font color=blue>%s</font></a></span><hr color=#163871 />\n", search(lfirewall,"rule_fwstate"),search(lfirewall,"rule_fwstate_running"), encry,search(lfirewall,"rule_fw_stop") );
			}
			else
			{
				fprintf( cgiOut, "<span>%s:%s ! <a href='wp_fwrulemodify.cgi?UN=%s&doFireWall=start'><font color=blue>%s</font></a></span><hr color=#163871 />\n", search(lfirewall,"rule_fwstate"),search(lfirewall,"rule_fwstate_stoped"), encry,search(lfirewall,"rule_fw_start") );
			}
		}
		//firewall
		//表头
		fprintf( cgiOut, TABLE_HEAD, 
					search(lpublic,"menu_thead"), search( lfirewall, "rule_position" ),
					search(lpublic,"menu_thead"), search( lfirewall, "rule_name_detail" ),
					search(lpublic,"menu_thead"), search( lfirewall, "rule_action" ),
					search(lpublic,"menu_thead"), search( lfirewall, "rule_state" ) );
		//内容
		if( 0 == list->iWallTotalNum || NULL == list->wall )
		{
			if(0 == manager)
				fprintf( cgiOut, "<tr><td colspan=6><div>%s <a href='wp_fwruleedit.cgi?UN=%s&ruleID=0&ruleNum=0&ruleType=FW_WALL&editType=add' target=mainFrame class=top><i>%s</i></a> %s </td></tr>\n", search(lfirewall,"rule_no_pre"), encry, search(lfirewall,"rule_no_here"), search(lfirewall,"rule_no_post") );
		}
		else
		{
			rule = list->wall;
			for( i=0; (i < list->iWallTotalNum) && (rule != NULL); i++ )
			{

				
				sprintf( str_RULEPOSTION_SELECTOR, RULEPOSTION_SELECTOR,
								list->iWallTotalNum, i+1, ruleTypeStr, encry );

				sprintf( popMenuName,"popMenuName%s%d",ruleTypeStr,i);
				sprintf( str_url_edit, "wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=%s&editType=edit",
								encry,i+1,list->iWallTotalNum,ruleTypeStr );
				sprintf( str_url_del, "wp_fwrulemodify.cgi?UN=%s&delRuleIndex=%d&ruleNum=%d&ruleType=%s",
								encry,i+1,list->iWallTotalNum,ruleTypeStr );				
				sprintf( str_POP_MENU, POP_MENU, popMenuName,popMenuName,list->iWallTotalNum-i,
								popMenuName,search( lfirewall, "rule_edit" ),str_url_edit ,
								popMenuName,search( lfirewall, "rule_delete" ),str_url_del,
								popMenuName );
							
				char *action=NULL;
				switch( rule->act )
				{
					case FW_ACCEPT:
						action = search(lfirewall,"rule_action_allow");
						break;
					case FW_DROP:
						action = search(lfirewall,"rule_action_deny");
						break;
					case FW_REJECT:
						action = search(lfirewall,"rule_action_reject");
						break;
					case FW_TCPMSS:
						action = "TCPMSS";
						break;
					default:
						break;
				}
				
				fprintf( cgiOut, TABLE_LINE, setclour((i+1)%2)/*class*/,
								ruleTypeStr/*ruletype*/,i,ruleTypeStr/*ruletype*/,i,
								(0 == manager)?str_RULEPOSTION_SELECTOR:int2str(i+1),/*pos*/
								rule->name,
								action,
								(rule->enable)?search(lfirewall,"rule_enabled"):search(lfirewall,"rule_disabled"),
								(0 == manager)?str_POP_MENU:"",
								ruleTypeStr,i);

				rule = rule->next;
											
			}
			//加入一个空行，解决popmenu显示不下的问题。
			//fprintf( cgiOut, "<tr height=25><td colspan=7 /></tr>\n");
		}
		//结束
		fprintf( cgiOut, "</table>\n" );
		//加一行,否则弹出popmenu时会显示滚动条
		fprintf( cgiOut, "<table ><tr height=25><td></td></tr></table>\n");
		
	}
	else if (FW_SNAT == ruleType || FW_DNAT == ruleType)
	{
		//nat
		// snat
		//表头
		strcpy( ruleTypeStr, "FW_SNAT" );
		fprintf( cgiOut, "<b>%s:</b><hr color=#163871 />", search( lfirewall, "ruleedit_trans_source_addr" ) );
//		fprintf( cgiOut, TABLE_HEAD, search( lfirewall, "rule_position" ), search( lfirewall, "rule_name_detail" ),  search( lfirewall, "rule_action" ), search( lfirewall, "rule_state" ) );
		fprintf( cgiOut, TABLE_HEAD, 
					search(lpublic,"menu_thead"), search( lfirewall, "rule_position" ),
					search(lpublic,"menu_thead"), search( lfirewall, "rule_name_detail" ),
					search(lpublic,"menu_thead"), "",
					search(lpublic,"menu_thead"), search( lfirewall, "rule_state" ) );
		//内容
		if( 0 == list->iSNATTotalNum || NULL == list->snat )
		{
			if(0 == manager)
				fprintf( cgiOut, "<tr><td colspan=6><div>%s <a href='wp_fwruleedit.cgi?UN=%s&ruleID=0&ruleNum=0&ruleType=FW_SNAT&editType=add' target=mainFrame class=top><i>%s</i></a> %s </td></tr>\n", search(lfirewall,"rule_no_pre"), encry, search(lfirewall,"rule_no_here"), search(lfirewall,"rule_no_post") );
		}
		else
		{
			rule = list->snat;
			for( i=0; ((i < list->iSNATTotalNum)&&(rule!=NULL)); i++ )
			{
				sprintf( str_RULEPOSTION_SELECTOR, RULEPOSTION_SELECTOR,
								list->iSNATTotalNum, i+1, ruleTypeStr, encry );

				sprintf( popMenuName,"popMenuName%s%d",ruleTypeStr,i);
				sprintf( str_url_edit, "wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=%s&editType=edit",
								encry,i+1,list->iSNATTotalNum,ruleTypeStr );
				sprintf( str_url_del, "wp_fwrulemodify.cgi?UN=%s&delRuleIndex=%d&ruleNum=%d&ruleType=%s",
								encry,i+1,list->iSNATTotalNum,ruleTypeStr );				
				sprintf( str_POP_MENU, POP_MENU, popMenuName,popMenuName,list->iSNATTotalNum-i,
								popMenuName,search( lfirewall, "rule_edit" ),str_url_edit ,
								popMenuName,search( lfirewall, "rule_delete" ),str_url_del,
								popMenuName );
				fprintf( cgiOut, TABLE_LINE, setclour((i+1)%2)/*class*/,
								ruleTypeStr/*ruletype*/,i,ruleTypeStr/*ruletype*/,i,
								(0 == manager)?str_RULEPOSTION_SELECTOR:int2str(i+1),/*pos*/
								rule->name,
								"",
								(rule->enable)?search(lfirewall,"rule_enabled"):search(lfirewall,"rule_disabled"),
								(0 == manager)?str_POP_MENU:"",
								ruleTypeStr,i);
				rule = rule->next;
			}
			
		}
		//结束
		fprintf( cgiOut, "</table>\n" );
		//加一行,否则弹出popmenu时会显示滚动条
		fprintf( cgiOut, "<table ><tr height=25><td></td></tr></table>\n");

		// dnat
		//表头
		strcpy( ruleTypeStr, "FW_DNAT" );
		fprintf( cgiOut, "<br /><b>%s:</b><hr color=#163871 />", search( lfirewall, "ruleedit_trans_dest_addr" ) );
		fprintf( cgiOut, TABLE_HEAD, 
					search(lpublic,"menu_thead"), search( lfirewall, "rule_position" ),
					search(lpublic,"menu_thead"), search( lfirewall, "rule_name_detail" ),
					search(lpublic,"menu_thead"), "",
					search(lpublic,"menu_thead"), search( lfirewall, "rule_state" ) );
		//内容
		if( 0 == list->iDNATTotalNum || NULL == list->dnat )
		{
			if(0 == manager)
				fprintf( cgiOut, "<tr><td colspan=6><div>%s <a href='wp_fwruleedit.cgi?UN=%s&ruleID=0&ruleNum=0&ruleType=FW_DNAT&editType=add' target=mainFrame class=top><i>%s</i></a> %s </td></tr>\n", search(lfirewall,"rule_no_pre"), encry, search(lfirewall,"rule_no_here"), search(lfirewall,"rule_no_post") );
		}
		else
		{
			rule = list->dnat;
			for( i=0; (i < list->iDNATTotalNum)&&(rule!=NULL); i++ )
			{
			
				sprintf( str_RULEPOSTION_SELECTOR, RULEPOSTION_SELECTOR,
								list->iDNATTotalNum, i+1, ruleTypeStr, encry );

				sprintf( popMenuName,"popMenuName%s%d",ruleTypeStr,i);
				sprintf( str_url_edit, "wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=%s&editType=edit",
								encry,i+1,list->iDNATTotalNum,ruleTypeStr );
				sprintf( str_url_del, "wp_fwrulemodify.cgi?UN=%s&delRuleIndex=%d&ruleNum=%d&ruleType=%s",
								encry,i+1,list->iDNATTotalNum,ruleTypeStr );				
				sprintf( str_POP_MENU, POP_MENU, popMenuName,popMenuName,list->iDNATTotalNum-i,
								popMenuName,search( lfirewall, "rule_edit" ),str_url_edit ,
								popMenuName,search( lfirewall, "rule_delete" ),str_url_del,
								popMenuName );
				fprintf( cgiOut, TABLE_LINE, setclour((i+1)%2)/*class*/,
								ruleTypeStr/*ruletype*/,i,ruleTypeStr/*ruletype*/,i,
								(0 == manager)?str_RULEPOSTION_SELECTOR:int2str(i+1),/*pos*/
								rule->name,
								"",
								(rule->enable)?search(lfirewall,"rule_enabled"):search(lfirewall,"rule_disabled"),
								(0 == manager)?str_POP_MENU:"",
								ruleTypeStr,i);
				rule = rule->next;
			}

		}
		//结束
		fprintf( cgiOut, "</table>\n" );
		//加一行,否则弹出popmenu时会显示滚动条
		fprintf( cgiOut, "<table ><tr height=25><td></td></tr></table>\n");
		//对于nat，添加一个需要开启防火墙的提示
		if( FW_WALL != ruleType )
		{
			fprintf(cgiOut,"<table width=766px>\n" );
			fprintf(cgiOut,"<tr>"\
			"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			fprintf(cgiOut,"</tr>");
			
    			fprintf(cgiOut,"<tr height=25 style=padding-top:2px>"\
    			  "<td style=font-size:14px;color:#FF0000>%s</td>"\
    			"</tr>",search(lfirewall,"nat_need"));
			fprintf(cgiOut,"</table>" );
		}

	}
	else
	{
		//firewall
		//表头
		fprintf( cgiOut, "<br /><b>%s:</b><hr color=#163871 />", search( lfirewall, "rule_input_description" ) );
		fprintf( cgiOut, TABLE_HEAD, 
					search(lpublic,"menu_thead"), search( lfirewall, "rule_position" ),
					search(lpublic,"menu_thead"), search( lfirewall, "rule_name_detail" ),
					search(lpublic,"menu_thead"), search( lfirewall, "rule_action" ),
					search(lpublic,"menu_thead"), search( lfirewall, "rule_state" ) );
		//内容
		if( 0 == list->iInputTotalNum || NULL == list->input)
		{
			if(0 == manager)
				fprintf( cgiOut, "<tr><td colspan=6><div>%s <a href='wp_fwruleedit.cgi?UN=%s&ruleID=0&ruleNum=0&ruleType=FW_INPUT&editType=add' target=mainFrame class=top><i>%s</i></a> %s </td></tr>\n", search(lfirewall,"rule_no_pre"), encry, search(lfirewall,"rule_no_here"), search(lfirewall,"rule_no_post") );
		}
		else
		{
			rule = list->input;
			for( i=0; (i < list->iInputTotalNum) && (rule != NULL); i++ )
			{

				
				sprintf( str_RULEPOSTION_SELECTOR, RULEPOSTION_SELECTOR,
								list->iInputTotalNum, i+1, ruleTypeStr, encry );

				sprintf( popMenuName,"popMenuName%s%d",ruleTypeStr,i);
				sprintf( str_url_edit, "wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=%s&editType=edit",
								encry,i+1,list->iInputTotalNum,ruleTypeStr );
				sprintf( str_url_del, "wp_fwrulemodify.cgi?UN=%s&delRuleIndex=%d&ruleNum=%d&ruleType=%s",
								encry,i+1,list->iInputTotalNum,ruleTypeStr );				
				sprintf( str_POP_MENU, POP_MENU, popMenuName,popMenuName,list->iInputTotalNum-i,
								popMenuName,search( lfirewall, "rule_edit" ),str_url_edit ,
								popMenuName,search( lfirewall, "rule_delete" ),str_url_del,
								popMenuName );
							
				char *action=NULL;
				switch( rule->act )
				{
					case FW_ACCEPT:
						action = search(lfirewall,"rule_action_allow");
						break;
					case FW_DROP:
						action = search(lfirewall,"rule_action_deny");
						break;
					case FW_REJECT:
						action = search(lfirewall,"rule_action_reject");
						break;
					//case FW_TCPMSS:
					//	action = "TCPMSS";
					//	break;
					default:
						break;
				}
				
				fprintf( cgiOut, TABLE_LINE, setclour((i+1)%2)/*class*/,
								ruleTypeStr/*ruletype*/,i,ruleTypeStr/*ruletype*/,i,
								(0 == manager)?str_RULEPOSTION_SELECTOR:int2str(i+1),/*pos*/
								rule->name,
								action,
								(rule->enable)?search(lfirewall,"rule_enabled"):search(lfirewall,"rule_disabled"),
								(0 == manager)?str_POP_MENU:"",
								ruleTypeStr,i);

				rule = rule->next;
											
			}
			//加入一个空行，解决popmenu显示不下的问题。
			//fprintf( cgiOut, "<tr height=25><td colspan=7 /></tr>\n");
		}
		//结束
		fprintf( cgiOut, "</table>\n" );
		//加一行,否则弹出popmenu时会显示滚动条
		fprintf( cgiOut, "<table ><tr height=25><td></td></tr></table>\n");
		//对于nat，添加一个需要开启防火墙的提示
		if( FW_WALL != ruleType )
		{
			fprintf(cgiOut,"<table width=766px>\n" );
			fprintf(cgiOut,"<tr>"\
			"<td id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px\">%s</td>",search(lpublic,"description"));
			fprintf(cgiOut,"</tr>");
			
    			fprintf(cgiOut,"<tr height=25 style=padding-top:2px>"\
    			  "<td style=font-size:14px;color:#FF0000>%s</td>"\
    			"</tr>",search(lfirewall,"input_need"));
			fprintf(cgiOut,"</table>" );
		}

	}
}//表格输出完成
	fprintf( cgiOut, "</div></td> \n" );
	fprintf( cgiOut, "</tr> \n" );
	fprintf( cgiOut, "<tr height=4 valign=top><td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td> \n" );
	fprintf( cgiOut, "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td> \n" );
	fprintf( cgiOut, "</tr> \n" );
	fprintf( cgiOut, "</table> \n" );
	fprintf( cgiOut, "</td> \n" );
	fprintf( cgiOut, "<td width=15 background=/images/di999.jpg>&nbsp;</td> \n" );
	fprintf( cgiOut, "</tr> \n" );
	fprintf( cgiOut, "</table> \n" );
	fprintf( cgiOut, "</td> \n" );
	fprintf( cgiOut, "</tr> \n" );
	fprintf( cgiOut, "<tr><td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td> \n" );
	fprintf( cgiOut, "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td> \n" );
	fprintf( cgiOut, "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td> \n" );
	fprintf( cgiOut, "</tr> \n" );
	fprintf( cgiOut, "</table> \n" );
	fprintf( cgiOut, "</div> \n" );
	fprintf( cgiOut, "</form> \n" );	
	fprintf( cgiOut, "</body> \n" );
	fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
	fprintf( cgiOut, "autoCheckHeight();\n" );
	fprintf( cgiOut, "</script> \n" );
	fprintf( cgiOut, " \n" );
	
	fprintf( cgiOut, "</html> \n" );

	//FreeList(list);
	free(encry);
	release(lpublic); 
	release(lfirewall);
//endif
	
	return 0;
}

int fwServiceStatus()
{
	int ret = -1;
	int status;

	status = system(CMD_STATUS_IS_START);
	ret = WEXITSTATUS(status);
	
	return ret;	
}




