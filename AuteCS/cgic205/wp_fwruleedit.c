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
* wp_fwruleedit.c
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

#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>


#include "cgic.h"
#include "ws_err.h"
#include "ws_public.h"
#include "ws_firewall.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"

#include "ac_manage_def.h"
#include "ac_manage_firewall_interface.h"


#define HOST_SET_ENGABLE	0
#define PORT_COLLECTION_ENABLE	0
#define DEFAULT_NETWORK_ENABLE	0

#define _DEBUG	1

#if _DEBUG
#define debug_printf(a...) fprintf(a)
#else
#define debug_printf(a...)
#endif

#define BUF_LEN					32
#define FIREWALL_MAX_RULE_NUM			256
#define FW_MAX_PKG_STATE_LEN			64
#define FW_MAX_STR_FILTER_LEN			BUF_LEN
#define MAX_SLOT				16


static int slot_id = 1;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};


static int showInterfaceSelector( char *selectName, char *onchangestr, char *disable );

static fwRule *
ccgi_firewall_get_rule_by_index(int index, int rule_type)
{
	u_long rule_num = 0;
	fwRule *rule_array = NULL;
	int ret = 0;
	fwRule *rule = NULL;

	ret = ac_manage_show_firewall_rule(ccgi_connection, NULL, NULL, &rule_array, &rule_num);

	if(AC_MANAGE_SUCCESS == ret && rule_array && rule_num) {
		int j = 0;
		for (; j < rule_num; j++) {
			if (rule_array[j].type == rule_type && rule_array[j].id == index) {
				rule = malloc(sizeof(fwRule));
				if (rule) {
					memcpy(rule, &(rule_array[j]), sizeof(fwRule));
				}
				break;
			}
		}
		firewall_free_array(&rule_array, rule_num);
	}
	return rule;
}

int cgiMain()
{
//#ifdef _MANAGE_FIREWALL_ 

	char ruleType[10]="";
	char editType[10]="";
	char ruleNum[10]="";
	fwRuleList *list=NULL;
	fwRule *rule=NULL;
	char *encry=(char *)malloc(BUF_LEN);
	char *str;
	struct list *lpublic;
	struct list *lfirewall;
	instance_parameter *p_q = NULL;

	DcliWInit();
	ccgi_dbus_init();
	
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

	memset(plotid,0,sizeof(plotid));
	cgiFormStringNoNewlines("plotid", plotid, sizeof(plotid)); 
	fprintf(stderr, "----------------------------------------------plotid=%s\n", plotid);

	list_instance_parameter(&paraHead1, SNMPD_SLOT_CONNECT);
	if (NULL == paraHead1) {
		return 0;
	}
	if(strcmp(plotid, "") == 0) {
		slot_id = paraHead1->parameter.slot_id;
	} else {
		slot_id = atoi(plotid);
	}
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	fprintf(stderr, "----------------------------------------------plotid=%s\n", plotid);
	

		
		cgiHeaderContentType("text/html");

		list=(fwRuleList *)malloc(sizeof(fwRuleList));
		memset( list, 0, sizeof(fwRuleList) );
		//fwParseDoc( list );
		
		
		fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
		fprintf( cgiOut, "<head> \n" );
		fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
	  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
	  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
	  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
		
		fprintf( cgiOut, "<title>Add firewall rule</title> \n" );
		fprintf( cgiOut, "<script language='javascript' type='text/javascript' src='/fw.js'></script> \n" );
		fprintf( cgiOut, "<link rel=stylesheet href=/style.css type=text/css> \n" );
		fprintf( cgiOut, "<style type=text/css> \n" );
		fprintf( cgiOut, ".usrlis {overflow-x:hidden; overflow:auto; width: 416px; height: 270px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} \n" );
		fprintf( cgiOut, "</style> \n" );
		fprintf( cgiOut, "<style type=text/css> \n" );
		fprintf( cgiOut, "tr.even td { \n" );
		fprintf( cgiOut, "background-color: #eee; \n" );
		fprintf( cgiOut, "border: 0; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "tr.odd td { \n" );
		fprintf( cgiOut, "background-color: #fff; \n" );
		fprintf( cgiOut, "border: 0; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, " \n" ); 
		fprintf( cgiOut, "tr.changed td { \n" );
		fprintf( cgiOut, "background-color: #00f; \n" );
		fprintf( cgiOut, "border: 0; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, " \n" ); 
		fprintf( cgiOut, "tr.new td { \n" );  
		fprintf( cgiOut, "background-color: #0f0; \n" );
		fprintf( cgiOut, "border: 0; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, " \n" ); 			
		fprintf( cgiOut, ".col1{ \n" );
		fprintf( cgiOut, "position: relative; \n" );
		fprintf( cgiOut, "left:0px; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, ".col2{ \n" );
		fprintf( cgiOut, "position: relative; \n" );
		fprintf( cgiOut, "left:0px; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, ".col3{ \n" );
		fprintf( cgiOut, "position: relative; \n" );
		fprintf( cgiOut, "left:0px; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "</style> \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "</head> \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		//fprintf( cgiOut, "//alert(\"ssss\"); \n" );
		//得到rule类型
		cgiFormStringNoNewlines( "ruleType", ruleType, sizeof(ruleType) );
		//strcpy( ruleType , "SNAT" );//"SNAT" "DNAT" or "FILTER" 
		fprintf( cgiOut, "var PAGE_TYPE = '%s';\n", ruleType );
		fprintf( cgiOut, "</script> \n" );
		fprintf( cgiOut, "<body> \n" );
		fprintf( cgiOut, "<form action='wp_fwrulemodify.cgi' method='post' onSubmit='return isAllLegal(this);'> \n" );
		
		fprintf( cgiOut, "<input type='hidden' name='ruleType' value='%s'>", ruleType );
		fprintf( cgiOut, "<input type='hidden' name='UN' value='%s'>", encry );
		cgiFormStringNoNewlines( "editType", editType, sizeof(editType) );
		fprintf( cgiOut, "<input type='hidden' name='editType' value='%s'>", editType );
		
		fprintf( cgiOut, "<div align=center> \n" );
		fprintf( cgiOut, "<table width=976 border=0 cellpadding=0 cellspacing=0> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td> \n" );
		fprintf( cgiOut, "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td> \n" );
		fprintf( cgiOut, "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>&nbsp;%s</td> \n", search(lpublic,"title_style"), search(lfirewall, "ruleedit_title" ) );
		fprintf( cgiOut, "<td width=690 align=right valign=bottom background=/images/di22.jpg><table width=130 border=0 cellspacing=0 cellpadding=0> \n" );
		fprintf( cgiOut, "<tr> \n" );
			fprintf( cgiOut, "<td width=62 align=center><input id='but' type='submit' name='submit_addrule' style='background-image:url(/images/%s)' value=''></td>\n", search(lpublic,"img_ok") );

			fprintf( cgiOut, "<td width=62 align=left><a href=wp_fwruleview.cgi?UN=%s&ruleType=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td> \n", encry, ruleType, search(lpublic,"img_cancel") );
	
		
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "</table></td> \n" );
		fprintf( cgiOut, "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0 border=1> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td> \n" );
		fprintf( cgiOut, "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0> \n" );
		fprintf( cgiOut, "<tr height=4 valign=bottom> \n" );
		fprintf( cgiOut, "<td width=120>&nbsp;</td> \n" );
		fprintf( cgiOut, "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td><table width=120 border=0 cellspacing=0 cellpadding=0 height=100%% style='tableLayout:'> \n" );
		fprintf( cgiOut, "<tr height=50> \n" );
		fprintf( cgiOut, "<td id=tdleft>&nbsp;</td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr height=26> \n" );
#if 0		
		fprintf( cgiOut, "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\">&nbsp;<font id=%s>%s</font></td> \n", search(lpublic,"menu_san"), search( lfirewall,"ruleedit_title") );
#else
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_WALL' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n",encry, search(lpublic,"menu_san"), search( lfirewall, "title_fw_rule") );	
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_SNAT' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n", encry, search(lpublic,"menu_san"), search( lfirewall, "title_nat") );
		fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleview.cgi?UN=%s&ruleType=FW_INPUT' target=mainFrame class=top><font id=%s>%s</font></a></td></tr> \n", encry, search(lpublic,"menu_san"), search( lfirewall, "title_input") );

		if( strcmp( ruleType, "FW_WALL" ) == 0 )
		{
			fprintf( cgiOut, "<tr height=26><td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></td></tr> \n", 
						search(lpublic,"menu_san"), (strcmp( editType, "add" ) == 0)?search( lfirewall, "rule_add"):search( lfirewall, "rule_edit"), 
						search(lpublic,"menu_san"),	search( lfirewall, "title_fw"),
						search(lpublic,"menu_san"),search( lfirewall, "add_rule") );

		}
		else
		{
			fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=FW_WALL&editType=add' target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></a></td></tr> \n",encry,list->iWallTotalNum+1, list->iWallTotalNum, search(lpublic,"menu_san"), search( lfirewall, "add_new"),search(lpublic,"menu_san"), search( lfirewall, "title_fw"),search(lpublic,"menu_san"), search( lfirewall, "add_rule") );			
		}
		
		if( strcmp( ruleType, "FW_SNAT" ) == 0 )
		{
			fprintf( cgiOut, "<tr height=26><td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></td></tr> \n", 
						search(lpublic,"menu_san"), (strcmp( editType, "add" ) == 0)?search( lfirewall, "rule_add"):search( lfirewall, "rule_edit"), 
						search(lpublic,"menu_san"),	"SNAT ",
						search(lpublic,"menu_san"),search( lfirewall, "add_rule") );	
		}
		else
		{
			fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=FW_SNAT&editType=add' target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></a></td></tr> \n",encry,list->iSNATTotalNum+1, list->iSNATTotalNum, search(lpublic,"menu_san"), search( lfirewall, "add_new"),search(lpublic,"menu_san"), "SNAT ",search(lpublic,"menu_san"), search( lfirewall, "add_rule") );		
		}

		if( strcmp( ruleType, "FW_DNAT" ) == 0 )
		{
			fprintf( cgiOut, "<tr height=26><td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></td></tr> \n", 
						search(lpublic,"menu_san"), (strcmp( editType, "add" ) == 0)?search( lfirewall, "rule_add"):search( lfirewall, "rule_edit"), 
						search(lpublic,"menu_san"),"DNAT ",
						search(lpublic,"menu_san"),search( lfirewall, "add_rule") );

		}
		else
		{
			fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=FW_DNAT&editType=add' target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></a></td></tr> \n",encry,list->iDNATTotalNum+1, list->iDNATTotalNum, search(lpublic,"menu_san"), search( lfirewall, "add_new"),search(lpublic,"menu_san"), "DNAT ",search(lpublic,"menu_san"), search( lfirewall, "add_rule") );			
		}

		if( strcmp( ruleType, "FW_INPUT" ) == 0 )
		{
			fprintf( cgiOut, "<tr height=26><td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></td></tr> \n", 
						search(lpublic,"menu_san"), (strcmp( editType, "add" ) == 0)?search( lfirewall, "rule_add"):search( lfirewall, "rule_edit"), 
						search(lpublic,"menu_san"),	"INPUT ",
						search(lpublic,"menu_san"),search( lfirewall, "add_rule") );

		}
		else
		{
			fprintf( cgiOut, "<tr height=25><td align=left id=tdleft><a href='wp_fwruleedit.cgi?UN=%s&ruleID=%d&ruleNum=%d&ruleType=FW_INPUT&editType=add' target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=%s>%s</font></a></td></tr> \n",encry,list->iInputTotalNum+1, list->iInputTotalNum, search(lpublic,"menu_san"), search( lfirewall, "add_new"),search(lpublic,"menu_san"), "INPUT ",search(lpublic,"menu_san"), search( lfirewall, "add_rule") );			
		}
		//添加各种规则的入口
#endif
		
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr height=350px> \n" );
		fprintf( cgiOut, "<td id='FILL_OBJ' style='height:100px;padding-left:10px;border-right:1px solid #707070;color:#000000;text-align:center'>&nbsp;</td> \n" );
		fprintf( cgiOut, "</tr> \n" );		
		fprintf( cgiOut, "</table>\n" );
		fprintf( cgiOut, "</td> \n" );
		fprintf( cgiOut, "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\"><div id=\"rules\" align=\"left\" style=\"width:768px;overflow:auto\"> \n" );
		fprintf( cgiOut, "<div> \n" );
		fprintf( cgiOut, " \n" );	
		
		fprintf( cgiOut, "<div class=\"col1\">\n" );
		fprintf( cgiOut, "<label class=\"col1\">%s</label> \n", search( lfirewall, "slot_num") );
		fprintf( cgiOut, "<span class=\"col2\"><select name=plotid id=plotid style=width:72px onchange=plotid_change(this)></span>\n");
		for(p_q=paraHead1;(NULL != p_q);p_q=p_q->next)
		{
			if(p_q->parameter.slot_id == slot_id)
			{
				fprintf(cgiOut,"<option value=\"%d\" selected>%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
			}
			else
			{
				fprintf(cgiOut,"<option value=\"%d\">%d</option>",p_q->parameter.slot_id,p_q->parameter.slot_id);
			}		
		}
		fprintf( cgiOut, "</select></span> </div> \n" );
		fprintf(cgiOut, "<script type=text/javascript>\n");
		fprintf(cgiOut, "function plotid_change( obj )\n"\
				"{\n"\
				"var plotid = obj.options[obj.selectedIndex].value;\n"\
				"var url = 'wp_fwruleview.cgi?UN=%s&ruleType=%s&plotid='+plotid;\n"\
				"window.location.href = url;\n"\
				"}\n", encry, ruleType);
		fprintf(cgiOut,"</script>\n");
		
		fprintf( cgiOut, "<div class=\"col1\"> \n" );
		fprintf( cgiOut, "<label class=\"col1\">%s</label> \n", search( lfirewall, "ruleedit_pos" ) );
		fprintf( cgiOut, "<span class=\"col2\" > \n" );
		//fprintf( cgiOut, "<input type='hidden' name='ruleIndexUserSelected' value='' />" );
		//fprintf( cgiOut, "<select name='ruleIndexSelect' onchange='indexSelectChange()'>\n" );
#if 1
		{
			int j;
			int ruleNuma = 0;
			char ruleIDStr[10]="";
			int ruleID = 0;
			int rule_type = 0;
			
			cgiFormStringNoNewlines( "ruleNum", ruleNum, sizeof(ruleNum) );
			sscanf( ruleNum, "%d", (int *)&ruleNuma );
			
			cgiFormStringNoNewlines( "ruleID", ruleIDStr, sizeof(ruleIDStr) );
			sscanf( ruleIDStr, "%d", (int *)&ruleID );
			
			if( strcmp(editType,"edit") == 0 )
			{
				fprintf( cgiOut, "<input type=text name='ruleIndexUserSelected' value=%s maxLength=3  onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" /><font color=red>(1-256)</font>", ruleIDStr);
				
				
				if (strcmp("FW_DNAT", ruleType) == 0) {
					rule_type = FW_DNAT;
				} else if(strcmp("FW_SNAT",ruleType) == 0) {
					rule_type = FW_SNAT;
				} else if(strcmp("FW_INPUT",ruleType) == 0) {
					rule_type = FW_INPUT;
				} else {
					rule_type = FW_WALL;
				}

				rule = ccgi_firewall_get_rule_by_index(ruleID, rule_type);
				
				if( NULL == rule )
				{
					fprintf( cgiOut, "<script type='text/javascript'>\n" );
					fprintf( cgiOut, "alert( 'err happen!' ); \n" );	
					fprintf( cgiOut, "</script>\n" );
				}
			}
			else
			{	
				fprintf( cgiOut, "<input type=text name='ruleIndexUserSelected' maxLength=3 value=''  onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"  /><font color=red>(1-256)</font>");
			}
		}
#endif	
		fprintf( cgiOut, "</span> </div> \n" );
		
		//fprintf( cgiOut, "</div> \n" );
		#if 0
		fprintf( cgiOut, "<script type=text/javascript>\n" );
		fprintf( cgiOut, "function indexSelectChange(){\n"\
		"	var sel=document.getElementsByName('ruleIndexSelect')[0];\n"\
		"	var inp=document.getElementsByName( 'ruleIndexUserSelected' )[0];\n"\
		"	inp.value = sel.options[sel.selectedIndex].value;\n"\
		"}" );
//		fprintf( cgiOut, "indexSelectChange();\n" );
		fprintf( cgiOut, "</script>\n" );

		fprintf( cgiOut, "<div class=\"col1\"> \n" );
		fprintf( cgiOut, "<label class=\"col1\">%s</label> \n", search( lfirewall, "ruleedit_description") );
		fprintf( cgiOut, "<span class=\"col2\"> \n" );
		fprintf( cgiOut, "<input type=\"text\" id=\"ruleDescription\" name=\"ruleDescription\" size=30 maxLength=30/> \n" );
		fprintf( cgiOut, "</span> <span><font color=red>(%s)</font></span></div> \n", search( lfirewall, "ruleedit_description_limite" ) );
#endif


#if 0//这两个选项还没有功能，先去掉		
		fprintf( cgiOut, "<div class=\"col1\"> &nbsp;&nbsp; \n" );
		fprintf( cgiOut, "<input id=\"loggingEnabledCHECKBOX\" name=\"loggingEnabledCHECKBOX\" type=\"checkbox\" class=\"checkbox\" /> \n" );
		fprintf( cgiOut, "<label for=\"loggingEnabledCHECKBOX\">Logging for this rule enabled</label> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "<div class=\"col1\"> &nbsp;&nbsp; \n" );
		fprintf( cgiOut, "<input id=\"countingEnabled\" name=\"countingEnabled\" type=\"checkbox\" class=\"checkbox\" /> \n" );
		fprintf( cgiOut, "<label for=\"countingEnabled\">Rule hit counter enabled</label> \n" );
		fprintf( cgiOut, "</div> \n" );
#endif		
		fprintf( cgiOut, "<div class=\"col1\"> &nbsp;&nbsp; \n" );
		fprintf( cgiOut, "<input id=\"ruleEnabledCHECKBOX\" name=\"ruleEnabledCHECKBOX\" type=\"checkbox\" class=\"checkbox\" checked=\"checked\"/> \n" );
		fprintf( cgiOut, "<label for=\"ruleEnabledCHECKBOX\">%s</label> \n", search( lfirewall, "ruleedit_enabled" ) );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "<hr width=100%% size=1 color=#fff align=center noshade /> \n" );
		fprintf( cgiOut, "<div> \n" );
		fprintf( cgiOut, "<div class=\"col1\"> \n" );
		fprintf( cgiOut, "<h3 class=\"col1\">%s</h3> \n", search( lfirewall, "ruleedit_package" ) );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "<div class=\"col1\"> \n" );
		fprintf( cgiOut, "<label class=\"col1\">%s</label> \n", search( lfirewall, "ruleedit_ineth" ) );
		fprintf( cgiOut, "<span class=\"col2\"> \n" );
#if 0		
{
		int ethNum = 4,i;
		fprintf( cgiOut, "<script type='text/javascript'> \n" );
		fprintf( cgiOut, "function ethselector( id ) { \n" );
		fprintf( cgiOut, "selobj = new selectObj(); \n");
		fprintf( cgiOut, "selobj.setName( id ); \n" );
		fprintf( cgiOut, "selobj.setId(id); \n");
		fprintf( cgiOut, "selobj.setClass(id); \n");
		fprintf( cgiOut, "selobj.setOnSelChangeFunc('selchange()'); \n");
		fprintf( cgiOut, "selobj.addItem('any','any'); \n");
		for( i = 0; i < ethNum; i++ ){
			fprintf( cgiOut, "selobj.addItem('eth%d','eth%d'); \n", i, i);
		}
		fprintf( cgiOut, "selobj.setSelected(0); \n");
		fprintf( cgiOut, "selobj.writeHtml(); \n");
		fprintf( cgiOut, "}\n" );
		fprintf( cgiOut, " ethselector( 'inEth' ); \n ");
		fprintf( cgiOut, "function selchange(){	} \n");
		fprintf( cgiOut, "</script> \n");
}
#endif
		fprintf( cgiOut, "<input type=hidden name=inEthValue  value='' />\n" );
		showInterfaceSelector( "inEth", "onInEthChange(this)", (strcmp( ruleType, "FW_SNAT" ) == 0 )?"disabled":"" );
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "function onInEthChange( obj ){\n " );
		fprintf( cgiOut, "	document.getElementsByName( 'inEthValue' )[0].value = obj.value;\n");
		fprintf( cgiOut, "}\n" );
//		fprintf( cgiOut, "onInEthChange( document.getElementsByName('inEth')[0] );\n" );
		fprintf( cgiOut, "</script>\n" );

	if( strcmp( ruleType, "FW_INPUT" ) != 0){
		fprintf( cgiOut, "</span> </div> \n" );
		fprintf( cgiOut, "<div class=\"col1\"> \n" );
		fprintf( cgiOut, "<label class=\"col1\">%s</label> \n", search( lfirewall, "ruleedit_outeth" ) );
		fprintf( cgiOut, "<span>\n" );
#if 0		
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "ethselector( 'outEth' ); \n" );
		fprintf( cgiOut, "</script>\n");
#endif		
		
		fprintf( cgiOut, "<input type=hidden name=outEthValue  value='' />\n" );
		showInterfaceSelector( "outEth", "onOutEthChange(this)", (strcmp( ruleType, "FW_DNAT" ) == 0)?"disabled":"");
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "function onOutEthChange( obj ){\n " );
		fprintf( cgiOut, "	document.getElementsByName( 'outEthValue' )[0].value = obj.value;\n");
		fprintf( cgiOut, "}\n" );
//		fprintf( cgiOut, "onOutEthChange( document.getElementsByName('outEth')[0] );\n" );
		fprintf( cgiOut, "</script>\n" );		
	}
	
		fprintf( cgiOut, "</span>\n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "<div> \n" );
		fprintf( cgiOut, "<div class=\"col1\"> \n" );
		fprintf( cgiOut, "<label class=\"col1\"> \n" );
		fprintf( cgiOut, "<h4>%s</h4> \n", search( lfirewall, "ruleedit_soucreaddr" ) );
		fprintf( cgiOut, "</label> \n" );
		fprintf( cgiOut, "</div> \n" );
		
		fprintf( cgiOut, "<table><tr><td>\n" );
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.SourceAddress.Any\" name='Package.SourceAddress' value=\"0\" onclick=\"packageSourceAddressRadioControl.setActive('Package.SourceAddress.Any')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.SourceAddress.Any\">%s</label> \n", search( lfirewall, "ruleedit_sourceaddr_any" ) );
		fprintf( cgiOut, "</td><td></td> \n" );
		fprintf( cgiOut, "</tr><tr><td>\n" );
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.SourceAddress.Single\" name='Package.SourceAddress'  value=\"1\" onclick=\"packageSourceAddressRadioControl.setActive('Package.SourceAddress.Single')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.SourceAddress.Single\">%s </label></td><td> \n", search( lfirewall, "ruleedit_addr_single") );
		fprintf( cgiOut, "<input class=\"col2\" type=\"text\" id=\"Package.SourceAddress.Single.IP\" name='Package.SourceAddress.Single.IP'/> \n" );	
		fprintf( cgiOut, "</td></tr><tr>\n" );
		
		
#if HOST_SET_ENGABLE //先注释掉 host 方式		
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.SourceAddress.DefinedHost\"  name='Package.SourceAddress'  value=\"2\" onclick=\"packageSourceAddressRadioControl.setActive('Package.SourceAddress.DefinedHost')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.SourceAddress.DefinedHost\">Defined host: </label> \n" );
		fprintf( cgiOut, "<input type=\"text\" class=\"col2\" id=\"Package.SourceAddress.DefinedHost.Select\" name=\"Package.SourceAddress.DefinedHost.Select\"/> \n" );
#endif
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "<td>\n" );
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.SourceAddress.AddrMask\" name='Package.SourceAddress'  value=\"3\" onclick=\"packageSourceAddressRadioControl.setActive('Package.SourceAddress.AddrMask')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.SourceAddress.AddrMask\">%s</label> </td><td>\n", search( lfirewall, "ruleedit_addr_ipmask" ) );
		fprintf( cgiOut, "<input type=\"text\" class=\"col2\" id=\"Package.SourceAddress.AddrMask.Addr\" name='Package.SourceAddress.AddrMask.Addr'/><label>--</label><input type=\"text\" id=\"Package.SourceAddress.AddrMask.Mask\" name='Package.SourceAddress.AddrMask.Mask'> \n" );
		fprintf( cgiOut, "</td> </tr> \n" );
#if DEFAULT_NETWORK_ENABLE		
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.SourceAddress.DefinedNetwork\" name='Package.SourceAddress'  value=\"4\" onclick=\"packageSourceAddressRadioControl.setActive('Package.SourceAddress.DefinedNetwork')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.SourceAddress.DefinedNetwork\">Defined network: </label> \n" );
		fprintf( cgiOut, "<input type=\"text\" class=\"col2\" id=\"Package.SourceAddress.DefinedNetwork.Select\" name='Package.SourceAddress.DefinedNetwork.Select'/> \n" );
#endif		
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.SourceAddress.AddressRange\" name='Package.SourceAddress'  value=\"5\" onclick=\"packageSourceAddressRadioControl.setActive('Package.SourceAddress.AddressRange')\"/> </td><td>\n" );
		
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.SourceAddress.AddressRange\">%s </label> </td><td>\n", search( lfirewall, "ruleedit_addr_iprange" ) );
		fprintf( cgiOut, "<input type=\"text\" class=\"col2\" id=\"Package.SourceAddress.AddressRange.Begin\" name='Package.SourceAddress.AddressRange.Begin'/><label>--</label><input type=\"text\" id=\"Package.SourceAddress.AddressRange.End\" name='Package.SourceAddress.AddressRange.End'> \n" );
		fprintf( cgiOut, "</td> </tr></table> \n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "//sources address setting control \n" );
		fprintf( cgiOut, "var packageSourceAddressRadioControl = new radioGroup(); \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.addItem( \"Package.SourceAddress.Any\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.addItem( \"Package.SourceAddress.Single\" ); \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.bindRelativeItemToRadio( \"Package.SourceAddress.Single\", \"Package.SourceAddress.Single.IP\" ); \n" );
		fprintf( cgiOut, " \n" );
#if HOST_SET_ENGABLE		
		fprintf( cgiOut, "packageSourceAddressRadioControl.addItem( \"Package.SourceAddress.DefinedHost\" ); \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.bindRelativeItemToRadio( \"Package.SourceAddress.DefinedHost\",\"Package.SourceAddress.DefinedHost.Select\" ); \n" );
#endif		
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.addItem( \"Package.SourceAddress.AddrMask\" ); \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.bindRelativeItemToRadio( \"Package.SourceAddress.AddrMask\",\"Package.SourceAddress.AddrMask.Addr\" ); \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.bindRelativeItemToRadio( \"Package.SourceAddress.AddrMask\",\"Package.SourceAddress.AddrMask.Mask\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, " \n" );
#if DEFAULT_NETWORK_ENABLE		
		fprintf( cgiOut, "packageSourceAddressRadioControl.addItem( \"Package.SourceAddress.DefinedNetwork\" ); \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.bindRelativeItemToRadio( \"Package.SourceAddress.DefinedNetwork\",\"Package.SourceAddress.DefinedNetwork.Select\" ); \n" );
		fprintf( cgiOut, " \n" );
#endif		
		fprintf( cgiOut, "packageSourceAddressRadioControl.addItem( \"Package.SourceAddress.AddressRange\" ); \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.bindRelativeItemToRadio( \"Package.SourceAddress.AddressRange\", \"Package.SourceAddress.AddressRange.Begin\" ); \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.bindRelativeItemToRadio( \"Package.SourceAddress.AddressRange\", \"Package.SourceAddress.AddressRange.End\" ); \n" );	
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageSourceAddressRadioControl.setActive( \"Package.SourceAddress.Any\" ); \n" );
		fprintf( cgiOut, "function isSourceAddrLegel() \n" );
		fprintf( cgiOut, "{\n" );   
		fprintf( cgiOut, "var sourceradios = document.getElementsByName( 'Package.SourceAddress' );\n");
		fprintf( cgiOut, "if( sourceradios['Package.SourceAddress.Any'].checked ) \n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "return true;\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( sourceradios['Package.SourceAddress.Single'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var sourcesingleIP = document.getElementById( 'Package.SourceAddress.Single.IP' ).value;\n");
		fprintf( cgiOut, "return isLegalIpAddr( sourcesingleIP );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( sourceradios['Package.SourceAddress.AddrMask'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var sourceipaddr = document.getElementById( 'Package.SourceAddress.AddrMask.Addr' ).value;\n");
		fprintf( cgiOut, "var sourceipmask = document.getElementById( 'Package.SourceAddress.AddrMask.Mask' ).value;\n");
		fprintf( cgiOut, "return ( isLegalIpAddr( sourceipaddr ) && isLegalMask( sourceipmask ) );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( sourceradios['Package.SourceAddress.AddressRange'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var sourceipbegin = document.getElementById( 'Package.SourceAddress.AddressRange.Begin' ).value;\n");
		fprintf( cgiOut, "var sourceipend = document.getElementById( 'Package.SourceAddress.AddressRange.End' ).value;\n");
		fprintf( cgiOut, "return isLegalIpRang( sourceipbegin, sourceipend );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "return false;\n");
		fprintf( cgiOut, "}\n");
		
		fprintf( cgiOut, "</script> \n" );


		fprintf( cgiOut, "<div class=\"col1\"> \n" );
		fprintf( cgiOut, "<label class=\"col1\"> \n" );
		fprintf( cgiOut, "<h4>%s</h4> \n", search( lfirewall,"ruleedit_desaddr" ) );
		fprintf( cgiOut, "</label> \n" );
		fprintf( cgiOut, "</div> \n" );
		
		
		fprintf( cgiOut, "<table><tr><td>\n" );
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.DestinationAddress.Any\" name='Package.DestinationAddress'  value=\"0\" onclick=\"packageDestinationAddressRadioControl.setActive('Package.DestinationAddress.Any')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.Any\">%s</label> \n", search( lfirewall, "ruleedit_desaddr_any" ) );
		fprintf( cgiOut, "</td><td></td> \n" );
		fprintf( cgiOut, "<tr><td>\n" );
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.DestinationAddress.Single\" name='Package.DestinationAddress'  value=\"1\" onclick=\"packageDestinationAddressRadioControl.setActive('Package.DestinationAddress.Single')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.Single\">%s</label></td><td> \n", search( lfirewall, "ruleedit_addr_single" ) );
		fprintf( cgiOut, "<input class=\"col2\" type=\"text\" id=\"Package.DestinationAddress.Single.IP\" name='Package.DestinationAddress.Single.IP' /> \n" );
		fprintf( cgiOut, "</td></tr> \n" );

#if HOST_SET_ENGABLE		//先注释到host 设置方式
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.DestinationAddress.DefinedHost\"  value=\"2\" name='Package.DestinationAddress' onclick=\"packageDestinationAddressRadioControl.setActive('Package.DestinationAddress.DefinedHost')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.DefinedHost\">Defined host: </label> \n" );
		fprintf( cgiOut, "<input type=\"text\" class=\"col2\" id=\"Package.DestinationAddress.DefinedHost.Select\" name='Package.DestinationAddress.DefinedHost.Select'/> \n" );

#endif
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.DestinationAddress.AddrMask\" name='Package.DestinationAddress'  value=\"3\" onclick=\"packageDestinationAddressRadioControl.setActive('Package.DestinationAddress.AddrMask')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.AddrMask\">%s</label> </td><td>\n", search( lfirewall, "ruleedit_addr_ipmask" ) );
		fprintf( cgiOut, "<input type=\"text\" class=\"col2\" id=\"Package.DestinationAddress.AddrMask.Addr\" name='Package.DestinationAddress.AddrMask.Addr' /><label>--</label><input type=\"text\" id=\"Package.DestinationAddress.AddrMask.Mask\" name='Package.DestinationAddress.AddrMask.Mask'/> \n" );
		fprintf( cgiOut, "</td></tr>\n" );
#if DEFAULT_NETWORK_ENABLE		
		
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.DestinationAddress.DefinedNetwork\" name='Package.DestinationAddress'  value=\"4\" onclick=\"packageDestinationAddressRadioControl.setActive('Package.DestinationAddress.DefinedNetwork')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.DefinedNetwork\">Defined network: </label> \n" );
		fprintf( cgiOut, "<input type=\"text\" class=\"col2\" id=\"Package.DestinationAddress.DefinedNetwork.Select\" name='Package.DestinationAddress.DefinedNetwork.Select'/> \n" );
#endif		
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input type=\"radio\" class=\"col1\" id=\"Package.DestinationAddress.AddressRange\" name='Package.DestinationAddress'  value=\"5\" onclick=\"packageDestinationAddressRadioControl.setActive('Package.DestinationAddress.AddressRange')\"/> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.AddressRange\">%s </label> </td><td>\n", search( lfirewall, "ruleedit_addr_iprange" ) );
		fprintf( cgiOut, "<input type=\"text\" class=\"col2\" id=\"Package.DestinationAddress.AddressRange.Begin\" name='Package.DestinationAddress.AddressRange.Begin'/><label>--</label><input type=\"text\" id=\"Package.DestinationAddress.AddressRange.End\" name='Package.DestinationAddress.AddressRange.End'> \n" );
		fprintf( cgiOut, "</td></tr>\n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "//sources address setting control \n" );
		fprintf( cgiOut, "var packageDestinationAddressRadioControl = new radioGroup(); \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.addItem( \"Package.DestinationAddress.Any\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.addItem( \"Package.DestinationAddress.Single\" ); \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.bindRelativeItemToRadio( \"Package.DestinationAddress.Single\", \"Package.DestinationAddress.Single.IP\" ); \n" );
		fprintf( cgiOut, " \n" );

#if HOST_SET_ENGABLE
		fprintf( cgiOut, "packageDestinationAddressRadioControl.addItem( \"Package.DestinationAddress.DefinedHost\" ); \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.bindRelativeItemToRadio( \"Package.DestinationAddress.DefinedHost\",\"Package.DestinationAddress.DefinedHost.Select\" ); \n" );
#endif		
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.addItem( \"Package.DestinationAddress.AddrMask\" ); \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.bindRelativeItemToRadio( \"Package.DestinationAddress.AddrMask\",\"Package.DestinationAddress.AddrMask.Addr\" ); \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.bindRelativeItemToRadio( \"Package.DestinationAddress.AddrMask\",\"Package.DestinationAddress.AddrMask.Mask\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, " \n" );
#if DEFAULT_NETWORK_ENABLE		
		fprintf( cgiOut, "packageDestinationAddressRadioControl.addItem( \"Package.DestinationAddress.DefinedNetwork\" ); \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.bindRelativeItemToRadio( \"Package.DestinationAddress.DefinedNetwork\",\"Package.DestinationAddress.DefinedNetwork.Select\" ); \n" );
		fprintf( cgiOut, " \n" );
#endif		
		fprintf( cgiOut, "packageDestinationAddressRadioControl.addItem( \"Package.DestinationAddress.AddressRange\" ); \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.bindRelativeItemToRadio( \"Package.DestinationAddress.AddressRange\", \"Package.DestinationAddress.AddressRange.Begin\" ); \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.bindRelativeItemToRadio( \"Package.DestinationAddress.AddressRange\", \"Package.DestinationAddress.AddressRange.End\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageDestinationAddressRadioControl.setActive( \"Package.DestinationAddress.Any\" ); \n" );

		fprintf( cgiOut, "function isDestinationAddrLegel() \n" );
		fprintf( cgiOut, "{\n" );
		fprintf( cgiOut, "var destradios = document.getElementsByName( 'Package.DestinationAddress' );\n");
		fprintf( cgiOut, "if( destradios['Package.DestinationAddress.Any'].checked ) \n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "return true;\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( destradios['Package.DestinationAddress.Single'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var destsingleIP = document.getElementById( 'Package.DestinationAddress.Single.IP' ).value;\n");
		fprintf( cgiOut, "return isLegalIpAddr( destsingleIP );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( destradios['Package.DestinationAddress.AddrMask'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var destipaddr = document.getElementById( 'Package.DestinationAddress.AddrMask.Addr' ).value;\n");
		fprintf( cgiOut, "var destipmask = document.getElementById( 'Package.DestinationAddress.AddrMask.Mask' ).value;\n");
		fprintf( cgiOut, "return ( isLegalIpAddr( destipaddr ) && isLegalMask( destipmask ) );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( destradios['Package.DestinationAddress.AddressRange'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var destipbegin = document.getElementById( 'Package.DestinationAddress.AddressRange.Begin' ).value;\n");
		fprintf( cgiOut, "var destipend = document.getElementById( 'Package.DestinationAddress.AddressRange.End' ).value;\n");
		fprintf( cgiOut, "return isLegalIpRang( destipbegin, destipend );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "return false;\n");
		fprintf( cgiOut, "}\n");		
		
		fprintf( cgiOut, "</script> <!-- protocol --> \n" );
		fprintf( cgiOut, "<tr><td></td><td><label> %s </label></td><td> \n",  search( lfirewall, "ruleedit_protocol" ) );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "selobj = new selectObj(); \n" );
		fprintf( cgiOut, "selobj.setName(\"Protocol\"); \n" );
		fprintf( cgiOut, "selobj.setId(\"Protocol\"); \n" );
		fprintf( cgiOut, "selobj.setClass(\"Protocol\"); \n" );
		fprintf( cgiOut, "selobj.setOnSelChangeFunc(\"PortTypeSelChange()\"); \n" );
		fprintf( cgiOut, "selobj.addItem(\"0\",\"any\"); \n" );
		fprintf( cgiOut, "selobj.addItem(\"1\",\"TCP\"); \n" );
		fprintf( cgiOut, "selobj.addItem(\"2\",\"UDP\"); \n" );
		fprintf( cgiOut, "selobj.addItem(\"3\",\"TCP and UDP\"); \n" );
		fprintf( cgiOut, "selobj.addItem(\"4\",\"ICMP\"); \n" );
		fprintf( cgiOut, "selobj.setSelected(0); \n" );
		fprintf( cgiOut, "selobj.writeHtml(); \n" );
		fprintf( cgiOut, "</script> \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "</td></tr></table>  \n" );
		
		fprintf( cgiOut, "<div id=\"PACKAGE_PORT\"> \n" );
		fprintf( cgiOut, "<!-- for port --> \n" );
		fprintf( cgiOut, "<label class=\"col1\"> \n" );
		fprintf( cgiOut, "<h4>%s</h4> \n", search( lfirewall, "ruleedit_source_port" ) );
		fprintf( cgiOut, "</label> \n" );
		
		fprintf( cgiOut, "<table><tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.SourcePort.Any\" type=\"radio\" name=\"Package.SourcePort\" value='0' onclick=\"packageSourcePortRadioControl.setActive('Package.SourcePort.Any')\" /> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for='Package.SourcePort.Any' >%s</label> </td>\n", search( lfirewall, "ruleedit_port_any" ) );
		fprintf( cgiOut, "<td></td></tr> \n" );
		fprintf( cgiOut, "<tr> <td>\n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.SourcePort.Single\" type=\"radio\" name=\"Package.SourcePort\" value='1' onclick=\"packageSourcePortRadioControl.setActive('Package.SourcePort.Single')\" /> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for='Package.SourcePort.Single'>%s</label> </td><td>\n", search( lfirewall, "ruleedit_port_single" ) );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.SourcePort.Single.Port\" type=\"text\" name='Package.SourcePort.Single.Port' /> \n" );
		fprintf( cgiOut, "</td></tr> \n" );
		fprintf( cgiOut, "<tr> <td>\n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.SourcePort.Range\" type=\"radio\" name=\"Package.SourcePort\" value='2' onclick=\"packageSourcePortRadioControl.setActive('Package.SourcePort.Range')\" /> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for='Package.SourcePort.Range'>%s</label> </td><td>\n", search( lfirewall, "ruleedit_port_rang" ) );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.SourcePort.Range.Begin\" type=\"text\" name='Package.SourcePort.Range.Begin'/> \n" );
		fprintf( cgiOut, "<label class=\"col1\">--</label> \n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.SourcePort.Range.End\" type=\"text\" name='Package.SourcePort.Range.End' /> \n" );
		fprintf( cgiOut, "</td></tr> </table>\n" );
#if PORT_COLLECTION_ENABLE  //先去掉 port collection		
		fprintf( cgiOut, "<div> \n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.SourcePort.Collection\" type=\"radio\" name=\"Package.SourcePort\" value='3' onclick=\"packageSourcePortRadioControl.setActive('Package.SourcePort.Collection')\" /> \n" );
		fprintf( cgiOut, "<label for='Package.SourcePort.Collection'>Service / port collection:</label> \n" );
		fprintf( cgiOut, "<div>\n" );
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		
		fprintf( cgiOut, "		sourcePortCollector = new PortCollector( 'sourcePortCollector', 'SourcePortCollector' );\n" );
		fprintf( cgiOut, "		sourcePortCollector.setOptionons(getPortSelectionOptions());\n" );
		fprintf( cgiOut, "		sourcePortCollector.writeHTML();\n" );
		fprintf( cgiOut, "</script>\n" ); 
		fprintf( cgiOut, "</div>\n" );
		fprintf( cgiOut, "</div> \n" );
#endif		
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "var packageSourcePortRadioControl = new radioGroup(); \n" );
		fprintf( cgiOut, "packageSourcePortRadioControl.addItem(\"Package.SourcePort.Any\"); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageSourcePortRadioControl.addItem(\"Package.SourcePort.Single\"); \n" );
		fprintf( cgiOut, "packageSourcePortRadioControl.bindRelativeItemToRadio( \"Package.SourcePort.Single\",\"Package.SourcePort.Single.Port\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageSourcePortRadioControl.addItem(\"Package.SourcePort.Range\"); \n" );
		fprintf( cgiOut, "packageSourcePortRadioControl.bindRelativeItemToRadio( \"Package.SourcePort.Range\",\"Package.SourcePort.Range.Begin\" ); \n" );
		fprintf( cgiOut, "packageSourcePortRadioControl.bindRelativeItemToRadio( \"Package.SourcePort.Range\",\"Package.SourcePort.Range.End\" ); \n" );
		fprintf( cgiOut, " \n" );
#if PORT_COLLECTION_ENABLE		
		fprintf( cgiOut, "packageSourcePortRadioControl.addItem(\"Package.SourcePort.Collection\"); \n" );
#endif		
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageSourcePortRadioControl.setActive( \"Package.SourcePort.Any\" ); \n" );
		fprintf( cgiOut, " \n" );
		
		fprintf( cgiOut, "function isSourcePortLegal()\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var sourcePortRadios = document.getElementsByName( 'Package.SourcePort' );\n" );
		fprintf( cgiOut, "if( sourcePortRadios['Package.SourcePort.Any'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "return true;\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( sourcePortRadios['Package.SourcePort.Single'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var portSingle = document.getElementById('Package.SourcePort.Single.Port').value;\n");
		fprintf( cgiOut, "return isLegalPort( portSingle );\n" );
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( sourcePortRadios['Package.SourcePort.Range'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var portBegin = document.getElementById( 'Package.SourcePort.Range.Begin' ).value;\n");
		fprintf( cgiOut, "var portEnd = document.getElementById( 'Package.SourcePort.Range.End' ).value;\n");	
		fprintf( cgiOut, "return ( isLegalPort( portBegin ) && isLegalPort( portEnd ) &&  (parseInt(portEnd) - parseInt(portBegin) > 0 ) );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "return false;\n");
		fprintf( cgiOut, "}\n");
		
		fprintf( cgiOut, "</script> \n" );
		

		fprintf( cgiOut, "<label class=\"col1\"> \n" );
		fprintf( cgiOut, "<h4>%s</h4> \n", search( lfirewall, "ruleedit_des_port" ) );
		fprintf( cgiOut, "</label> \n" );

		fprintf( cgiOut, "<table><tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.DestinationPort.Any\" type=\"radio\" name='Package.DestinationPort' value='0' onclick=\"packageDestinationPortRadioControl.setActive('Package.DestinationPort.Any')\" /> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for='Package.DestinationPort.Any'>%s</label> </td><td>\n", search( lfirewall, "ruleedit_port_any" ) );
		fprintf( cgiOut, "</td></tr> \n" );
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.DestinationPort.Single\" type=\"radio\" name='Package.DestinationPort' value='1' onclick=\"packageDestinationPortRadioControl.setActive('Package.DestinationPort.Single')\" /> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for='Package.DestinationPort.Single'>%s</label> </td><td>\n", search( lfirewall, "ruleedit_port_single" ) );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.DestinationPort.Single.Port\" type=\"text\" name='Package.DestinationPort.Single.Port'/> \n" );
		fprintf( cgiOut, "</td></tr> \n" );
		fprintf( cgiOut, "<tr> <td>\n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.DestinationPort.Range\" type=\"radio\" name='Package.DestinationPort' value='2'  onclick=\"packageDestinationPortRadioControl.setActive('Package.DestinationPort.Range')\" /> </td><td>\n" );
		fprintf( cgiOut, "<label class=\"col1\" for='Package.DestinationPort.Range'>%s</label> </td><td>\n", search( lfirewall, "ruleedit_port_rang" ) );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.DestinationPort.Range.Begin\" type=\"text\" name='Package.DestinationPort.Range.Begin' /> \n" );
		fprintf( cgiOut, "<label class=\"col1\">--</label> \n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.DestinationPort.Range.End\" type=\"text\" name='Package.DestinationPort.Range.End' /> \n" );
		fprintf( cgiOut, "</td></tr></table> \n" );
#if PORT_COLLECTION_ENABLE// 先注释掉端口收集器		
		fprintf( cgiOut, "<div> \n" );
		fprintf( cgiOut, "<input class=\"col1\" id=\"Package.DestinationPort.Collection\" type=\"radio\" name='Package.DestinationPort' value='3' onclick=\"packageDestinationPortRadioControl.setActive('Package.DestinationPort.Collection')\" /> \n" );
		fprintf( cgiOut, "<label for='Package.DestinationPort.Collection'>Service / port collection:</label> \n" );
		fprintf( cgiOut, "<div><!-- 端口收集器 --> \n" );
		fprintf( cgiOut, "<input type=\"text\" id=\"PortInput\" /><input type=\"button\" value=\"add\" onclick=\"addport()\"/> \n" );
		fprintf( cgiOut, "<select size=\"6\" id=\"PortSelect\" style=\"width:80px\"> \n" );
		fprintf( cgiOut, "</select> \n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "function addport(){ \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "var usrinput = document.getElementById( \"PortInput\" ).value; \n" );
		fprintf( cgiOut, "document.getElementById( \"PortInput\" ).value = \"\"; \n" );
		fprintf( cgiOut, "var opt = new Option(); \n" );
		fprintf( cgiOut, "opt.value=usrinput; \n" );
		fprintf( cgiOut, "opt.text=usrinput; \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "document.getElementById(\"PortSelect\").add(opt,null); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "</script> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "</div> \n" );
#endif
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "var packageDestinationPortRadioControl = new radioGroup(); \n" );
		fprintf( cgiOut, "packageDestinationPortRadioControl.addItem(\"Package.DestinationPort.Any\"); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageDestinationPortRadioControl.addItem(\"Package.DestinationPort.Single\"); \n" );
		fprintf( cgiOut, "packageDestinationPortRadioControl.bindRelativeItemToRadio( \"Package.DestinationPort.Single\",\"Package.DestinationPort.Single.Port\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageDestinationPortRadioControl.addItem(\"Package.DestinationPort.Range\"); \n" );
		fprintf( cgiOut, "packageDestinationPortRadioControl.bindRelativeItemToRadio( \"Package.DestinationPort.Range\",\"Package.DestinationPort.Range.Begin\" ); \n" );
		fprintf( cgiOut, "packageDestinationPortRadioControl.bindRelativeItemToRadio( \"Package.DestinationPort.Range\",\"Package.DestinationPort.Range.End\" ); \n" );
		fprintf( cgiOut, " \n" );
#if PORT_COLLECTION_ENABLE		
		fprintf( cgiOut, "packageDestinationPortRadioControl.addItem(\"Package.DestinationPort.Collection\"); \n" );
#endif		
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "packageDestinationPortRadioControl.setActive( \"Package.DestinationPort.Any\" ); \n" );
		fprintf( cgiOut, " \n" );
		
		fprintf( cgiOut, "function isDestinationPortLegal()\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var destinationPortRadios = document.getElementsByName( 'Package.DestinationPort' );\n" );
		fprintf( cgiOut, "if( destinationPortRadios['Package.DestinationPort.Any'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "return true;\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( destinationPortRadios['Package.DestinationPort.Single'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var portSingle = document.getElementById('Package.DestinationPort.Single.Port').value;\n");
		fprintf( cgiOut, "return isLegalPort( portSingle );\n" );
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( destinationPortRadios['Package.DestinationPort.Range'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var portBegin = document.getElementById( 'Package.DestinationPort.Range.Begin' ).value;\n");
		fprintf( cgiOut, "var portEnd = document.getElementById( 'Package.DestinationPort.Range.End' ).value;\n");	
		fprintf( cgiOut, "return ( isLegalPort( portBegin ) && isLegalPort( portEnd ) &&  (parseInt(portEnd) - parseInt(portBegin) > 0 ) );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "return false;\n");
		fprintf( cgiOut, "}\n");		
		
		fprintf( cgiOut, "</script> \n" );
		fprintf( cgiOut, "</div> \n" );
		
	//state
		if( strcmp( ruleType, "FW_WALL" )==0 || strcmp( ruleType, "FW_INPUT") == 0)
		{
			fprintf( cgiOut, "<div>" );
			fprintf( cgiOut, "<label><h4>%s</h4></label>\n", search( lfirewall, "ruleedit_state" ) );
			fprintf( cgiOut, "<table>" );
			fprintf( cgiOut, "<tr><td><input type=checkbox name=state id=state_NEW value='NEW'/></td><td><label for=state_NEW>NEW</label></td>" );
			fprintf( cgiOut, "<td><input type=checkbox name=state id=state_ESTABLISHED value='ESTABLISHED'/></td><td><label for=state_ESTABLISHED>ESTABLISHED</label></td>" );
			fprintf( cgiOut, "<td><input type=checkbox name=state id=state_RELATED value='RELATED'/></td><td><label for=state_RELATED>RELATED</label></td>" );
			fprintf( cgiOut, "<td><input type=checkbox name=state id=state_INVALID value='INVALID'/></td><td><label for=state_INVALID>INVALID</label></td></tr>" );
			fprintf( cgiOut, "</table>" );
			fprintf( cgiOut, "</div>" );
	
		}
	// string filter
		if( strcmp( ruleType, "FW_WALL" )==0 || strcmp( ruleType, "FW_INPUT" )==0)
		{
			fprintf( cgiOut, "<div>" );
			fprintf( cgiOut, "<label><h4>%s</h4></label>\n", search( lfirewall, "string_filter" ) );
			fprintf( cgiOut, "<table>" );
			fprintf( cgiOut, "<tr><td><input type=text name=string_filter id=string_filter maxLength=%d value='%s' /></td></tr>", BUF_LEN-1, (rule==NULL||rule->string_filter==NULL)?"":rule->string_filter );
			fprintf( cgiOut, "</table>" );
			fprintf( cgiOut, "</div>" );
		}
	//state  end
	
		fprintf( cgiOut, "<hr width=100%% size=1 color=#fff align=center noshade /> \n" );
		fprintf( cgiOut, "<div> \n" );
		
		fprintf( cgiOut, "<div id=\"NAT\"> \n" );
		fprintf( cgiOut, "<div id=\"SNAT\"> \n" );
		fprintf( cgiOut, "<label> \n" );
		fprintf( cgiOut, "<h4>%s</h4> \n", search( lfirewall, "ruleedit_trans_source_addr" ) );
		fprintf( cgiOut, "</label> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "<div id=\"DNAT\"> \n" );
		fprintf( cgiOut, "<label > \n" );
		fprintf( cgiOut, "<h4>%s</h4> \n", search( lfirewall, "ruleedit_trans_dest_addr" ) );
		fprintf( cgiOut, "</label> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "<label> \n" );
		fprintf( cgiOut, "<h5>%s</h5> \n", search( lfirewall, "ruleedit_trans_addr" ) );
		fprintf( cgiOut, "</label> \n" );
		fprintf( cgiOut, "<table><tr><td> \n" );
		
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatMassquerade\" name='NatTranslation.IP' value='0' onclick=\"NatTranslationIp.setActive( 'NatMassquerade' )\" %s/> </td><td>\n", (strcmp( ruleType, "FW_DNAT" ) == 0 )?"disabled":"" );
		fprintf( cgiOut, "<label for=\"NatMassquerade\">%s </label> </td>\n", search( lfirewall, "ruleedit_trans_addr_masquerade" ) );
		fprintf( cgiOut, "<td></td></tr> \n" );
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatSingleAddr\" name='NatTranslation.IP' value='1' onclick=\"NatTranslationIp.setActive( 'NatSingleAddr' )\" /> </td><td>\n" );
		fprintf( cgiOut, "<label for=\"NatSingleAddr\">%s</label> </td><td>\n", search( lfirewall, "ruleedit_addr_single" ) );
		fprintf( cgiOut, "<input type=\"text\" id=\"NatSingleIpAddress\" name='NatSingleIpAddress' /> \n" );
		fprintf( cgiOut, "</td><tr> \n" );
#if HOST_SET_ENGABLE //先注释到host设置方式		
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatDefinedHost\" name='NatTranslation.IP' value='2' onclick=\"NatTranslationIp.setActive( 'NatDefinedHost' )\"/> </td><td>\n" );
		fprintf( cgiOut, "<label for=\"NatDefinedHost\">Defined host:</label> </td>\n" );
		fprintf( cgiOut, "<td></td></tr> \n" );
#endif
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatAddrMask\" name='NatTranslation.IP' value='3' onclick=\"NatTranslationIp.setActive( 'NatAddrMask' )\"/> </td><td>\n" );
		fprintf( cgiOut, "<label for=\"NatAddrMask\">%s</label> </td><td>\n", search( lfirewall, "ruleedit_addr_ipmask" ) );
		fprintf( cgiOut, "<input type=\"text\" id=\"NatAddressNetmaskAddr\" name='NatAddressNetmaskAddr' /> \n" );
		fprintf( cgiOut, "<label>--</label> \n" );
		fprintf( cgiOut, "<input type=\"text\" id=\"NatAddressNetmaskMask\" name='NatAddressNetmaskMask' /> \n" );
		fprintf( cgiOut, "</td></tr> \n" );
#if DEFAULT_NETWORK_ENABLE		
		fprintf( cgiOut, "<div> \n" );
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatDefineNetwork\" name='NatTranslation.IP' value='4' onclick=\"NatTranslationIp.setActive( 'NatDefineNetwork' )\"/> \n" );
		fprintf( cgiOut, "<label for=\"NatDefineNetwork\">Defined network</label> \n" );
		fprintf( cgiOut, "<input type='text' name='NatDefineNetworkSelect' />" );
		fprintf( cgiOut, "</div> \n" );
#endif		
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatAddrRange\" name='NatTranslation.IP' value='5' onclick=\"NatTranslationIp.setActive( 'NatAddrRange' )\"/> </td><td>\n" );
		fprintf( cgiOut, "<label for=\"NatAddrRange\">%s</label> </td><td>\n", search( lfirewall, "ruleedit_addr_iprange" ) );
		fprintf( cgiOut, "<input type=\"text\" id=\"NatAddrRangeBegin\" name='NatAddrRangeBegin' /> \n" );
		fprintf( cgiOut, "<label>--</label> \n" );
		fprintf( cgiOut, "<input type=\"text\" id=\"NatAddrRangeEnd\" name='NatAddrRangeEnd' /> \n" );
		fprintf( cgiOut, "</td></tr></table> \n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "var NatTranslationIp = new radioGroup(); \n" );
		fprintf( cgiOut, "NatTranslationIp.addItem( \"NatMassquerade\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "NatTranslationIp.addItem( \"NatSingleAddr\" ); \n" );
		fprintf( cgiOut, "NatTranslationIp.bindRelativeItemToRadio(\"NatSingleAddr\", \"NatSingleIpAddress\"); \n" );
		fprintf( cgiOut, " \n" );
#if HOST_SET_ENGABLE		
		fprintf( cgiOut, "NatTranslationIp.addItem( \"NatDefinedHost\" ); \n" );
#endif		
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "NatTranslationIp.addItem( \"NatAddrMask\" ); \n" );
		fprintf( cgiOut, "NatTranslationIp.bindRelativeItemToRadio(\"NatAddrMask\", \"NatAddressNetmaskAddr\"); \n" );
		fprintf( cgiOut, "NatTranslationIp.bindRelativeItemToRadio(\"NatAddrMask\", \"NatAddressNetmaskMask\"); \n" );
		fprintf( cgiOut, " \n" );
#if DEFAULT_NETWORK_ENABLE		
		fprintf( cgiOut, "NatTranslationIp.addItem( \"NatDefineNetwork\" ); \n" );
		fprintf( cgiOut, "NatTranslationIp.bindRelativeItemToRadio( \"NatDefineNetwork\", \"NatDefineNetworkSelect\" );\n" );
		fprintf( cgiOut, " \n" );
#endif
		fprintf( cgiOut, "NatTranslationIp.addItem( \"NatAddrRange\" ); \n" );
		fprintf( cgiOut, "NatTranslationIp.bindRelativeItemToRadio(\"NatAddrRange\", \"NatAddrRangeBegin\"); \n" );
		fprintf( cgiOut, "NatTranslationIp.bindRelativeItemToRadio(\"NatAddrRange\", \"NatAddrRangeEnd\"); \n" );
		fprintf( cgiOut, "NatTranslationIp.setActive( \"%s\" ); \n", (strcmp( ruleType, "FW_DNAT" ) == 0 )?"NatSingleAddr":"NatMassquerade" );
		
		
		fprintf( cgiOut, "function isTranslationAddrLegal() \n" );
		fprintf( cgiOut, "{\n" );   
#if 1		
		fprintf( cgiOut, "	var transAddrRadios = document.getElementsByName( 'NatTranslation.IP' );\n");
		fprintf( cgiOut, "	if( transAddrRadios['NatMassquerade'].checked ) \n");
		fprintf( cgiOut, "	{\n");
		//fprintf( cgiOut, " alert('1111111111111');\n" );
		fprintf( cgiOut, "		return true;\n");
		fprintf( cgiOut, "	}\n");
		fprintf( cgiOut, "	else if( transAddrRadios['NatSingleAddr'].checked )\n");
		fprintf( cgiOut, "	{\n");
		fprintf( cgiOut, "		var singleIP = document.getElementById( 'NatSingleIpAddress' ).value;\n");
		//fprintf( cgiOut, " 		alert('22222' + singleIP);\n" );
		fprintf( cgiOut, "		return isLegalIpAddr( singleIP );\n");
		fprintf( cgiOut, "	}\n");
		fprintf( cgiOut, "	else if( transAddrRadios['NatAddrMask'].checked )\n");
		fprintf( cgiOut, "	{\n");
		fprintf( cgiOut, "		var ipaddr = document.getElementById( 'NatAddressNetmaskAddr' ).value;\n");
		fprintf( cgiOut, "		var ipmask = document.getElementById( 'NatAddressNetmaskMask' ).value;\n");
		//fprintf( cgiOut, " 		alert('mask' + ipaddr + "    " + ipmask);\n" );
		fprintf( cgiOut, "		return ( isLegalIpAddr( ipaddr ) && isLegalMask( ipmask ) );\n");
		fprintf( cgiOut, "	}\n");
		fprintf( cgiOut, "	else if( transAddrRadios['NatAddrRange'].checked )\n");
		fprintf( cgiOut, "	{\n");
		fprintf( cgiOut, "		var ipbegin = document.getElementById( 'NatAddrRangeBegin' ).value;\n");
		fprintf( cgiOut, "		var ipend = document.getElementById( 'NatAddrRangeEnd' ).value;\n");
		//fprintf( cgiOut, " 		alert('rang' + ipbegin + "    " + ipend);\n" );
		fprintf( cgiOut, "		return isLegalIpRang( ipbegin, ipend );\n");
		fprintf( cgiOut, "	}\n");
		fprintf( cgiOut, "	return false;\n");
#else
		fprintf( cgiOut, "	return true;\n");
#endif		
		fprintf( cgiOut, "}\n");
		
		
		
		fprintf( cgiOut, "</script> \n" );
		fprintf( cgiOut, "<div id=\"NatPorttrans\"> \n" ); 
		fprintf( cgiOut, "<label id=\"PORTTRANS\"> \n" );
		fprintf( cgiOut, "<h4>%s</h4> \n", search( lfirewall, "ruleedit_trans_port" ) );
		fprintf( cgiOut, "</label> \n" );
		fprintf( cgiOut, "<table><tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatPortDefault\" name='NatTranslation.Port' value='0' onclick=\"NatTranslationPort.setActive( 'NatPortDefault' )\"/> </td>\n" );
		fprintf( cgiOut, "<td><label for=\"NatPortDefault\">%s</label> </td>\n", search( lfirewall, "ruleedit_trans_port_default" ) );
		fprintf( cgiOut, "<td></td></tr> \n" );
		fprintf( cgiOut, "<tr> <td>\n" );
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatPortSinglePort\" name='NatTranslation.Port' value='1' onclick=\"NatTranslationPort.setActive( 'NatPortSinglePort' )\" /> </td><td>\n" );
		fprintf( cgiOut, "<label for=\"NatPortSinglePort\">%s</label> </td><td>\n", search( lfirewall, "ruleedit_port_single" ) );
		fprintf( cgiOut, "<input type=\"text\" id=\"NatSinglePort\" name='NatSinglePort' /> \n" );
		fprintf( cgiOut, "</td></tr> \n" );
		fprintf( cgiOut, "<tr><td> \n" );
		fprintf( cgiOut, "<input class=\"col1\" type=\"radio\" id=\"NatPortRangePort\" name='NatTranslation.Port' value='2' onclick=\"NatTranslationPort.setActive( 'NatPortRangePort' )\" /> </td><td>\n" );
		fprintf( cgiOut, "<label for=\"NatPortRangePort\">%s</label> </td>\n", search( lfirewall, "ruleedit_port_rang" ) );
		fprintf( cgiOut, "<td><input type=\"text\" id=\"NatPortRangBegin\" name='NatPortRangBegin'/> \n" );
		fprintf( cgiOut, "<label>--</label> \n" );
		fprintf( cgiOut, "<input type=\"text\" id=\"NatPortRangEnd\" name='NatPortRangEnd'/> \n" );
		fprintf( cgiOut, "</td></tr></table> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "var NatTranslationPort = new radioGroup(); \n" );
		fprintf( cgiOut, "NatTranslationPort.addItem( \"NatPortDefault\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "NatTranslationPort.addItem( \"NatPortSinglePort\" ); \n" );
		fprintf( cgiOut, "NatTranslationPort.bindRelativeItemToRadio( \"NatPortSinglePort\", \"NatSinglePort\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "NatTranslationPort.addItem( \"NatPortRangePort\" ); \n" );
		fprintf( cgiOut, "NatTranslationPort.bindRelativeItemToRadio( \"NatPortRangePort\", \"NatPortRangBegin\" ); \n" );
		fprintf( cgiOut, "NatTranslationPort.bindRelativeItemToRadio( \"NatPortRangePort\", \"NatPortRangEnd\" ); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "NatTranslationPort.setActive( \"NatPortDefault\" ); \n" );
//nat port 
		fprintf( cgiOut, "function isTranslationPortLegal()\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var translationPortRadios = document.getElementsByName( 'NatTranslation.Port' );\n" );
		fprintf( cgiOut, "if( translationPortRadios['NatPortDefault'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "return true;\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( translationPortRadios['NatPortSinglePort'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var portSingle = document.getElementById('NatSinglePort').value;\n");
		fprintf( cgiOut, "return isLegalPort( portSingle );\n" );
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "else if( translationPortRadios['NatPortRangePort'].checked )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var portBegin = document.getElementById( 'NatPortRangBegin' ).value;\n");
		fprintf( cgiOut, "var portEnd = document.getElementById( 'NatPortRangEnd' ).value;\n");	
		fprintf( cgiOut, "return ( isLegalPort( portBegin ) && isLegalPort( portEnd ) &&  (parseInt(portEnd) - parseInt(portBegin) > 0 ) );\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "return false;\n");
		fprintf( cgiOut, "}\n");		

		
		fprintf( cgiOut, "</script> \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "<div id=\"FILTER\"> \n" );
		fprintf( cgiOut, "<label><h4>%s</h4></label> \n", search(lfirewall, "rule_action") );
		fprintf( cgiOut, "<label>%s</label> \n", search(lfirewall, "rule_action") );
		fprintf( cgiOut, "<select name='filterAction' onchange='filterActionChange();'> \n" );
		fprintf( cgiOut, "<option value=\"allow\">%s</option> \n", search(lfirewall, "rule_action_allow") );
		fprintf( cgiOut, "<option value=\"deny\">%s</option> \n", search(lfirewall, "rule_action_deny") );
		fprintf( cgiOut, "<option value=\"reject\">%s</option> \n", search(lfirewall, "rule_action_reject") );
		fprintf( cgiOut, "</select> \n" );
//tcpmss		
		fprintf( cgiOut, "<div id='div_tcpmss_value'>\n"\
						"</div>\n");
		fprintf( cgiOut, "	<script type=text/javascript>\n"\
						"	function filterActionChange()\n"\
						"	{\n"
						"		var div_tcpmss_value = document.getElementById('div_tcpmss_value');\n"\
						"		var filterAction = document.getElementsByName('filterAction')[0];\n"\
						"		var innerhtml = \"MSS:<input type=text maxLength=4 name=tcpmss_val value='' />\";\n"\
						"		if( filterAction.value == 'tcpmss' )\n"\
						"		{\n"\
						"			div_tcpmss_value.innerHTML = innerhtml;\n"\
						"		}\n"\
						"		else\n"\
						"		{\n"\
						"			div_tcpmss_value.innerHTML = '';\n"\
						"		}\n"\
						"	}\n"\
						"</script>\n");
//tcpmss						
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "</div></td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr height=4 valign=top> \n" );
		fprintf( cgiOut, "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td> \n" );
		fprintf( cgiOut, "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "</table></td> \n" );
		fprintf( cgiOut, "<td width=15 background=/images/di999.jpg>&nbsp;</td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "</table></td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td> \n" );
		fprintf( cgiOut, "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td> \n" );
		fprintf( cgiOut, "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "</table> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "</form> \n" );
		fprintf( cgiOut, " \n" );

		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		fprintf( cgiOut, "var ProtocolInnerHtml = document.getElementById(\"PACKAGE_PORT\").innerHTML; \n" );
		fprintf( cgiOut, "var NatPortInnerHtml = document.getElementById(\"NatPorttrans\").innerHTML; \n " );
		fprintf( cgiOut, "function PortTypeSelChange() \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "if( \"1\" == document.getElementById(\"Protocol\").value || \"2\" == document.getElementById(\"Protocol\").value \n" );
		fprintf( cgiOut, "|| \"3\" == document.getElementById(\"Protocol\").value ) \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "	document.getElementById(\"PACKAGE_PORT\").innerHTML = ProtocolInnerHtml; \n" );
		fprintf( cgiOut, "	if(null != document.getElementById(\"NatPorttrans\") ) \n" );
		fprintf( cgiOut, "	{\n" );
		fprintf( cgiOut, "		document.getElementById(\"NatPorttrans\").innerHTML = NatPortInnerHtml; \n " );
		fprintf( cgiOut, "	}\n" );
		//将show出来的端口配置信息中的radio的第一个选上
		fprintf( cgiOut, "	packageSourcePortRadioControl.setActive( \"Package.SourcePort.Any\" ); \n" );
		fprintf( cgiOut, "	packageDestinationPortRadioControl.setActive( \"Package.DestinationPort.Any\" ); \n" );
		fprintf( cgiOut, "	NatTranslationPort.setActive( \"NatPortDefault\" ); \n" );		
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "else \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "	document.getElementById(\"PACKAGE_PORT\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "	if(null != document.getElementById(\"NatPorttrans\") ) \n" );
		fprintf( cgiOut, "	{\n" );
		fprintf( cgiOut, "		document.getElementById(\"NatPorttrans\").innerHTML = \"\"; \n " );
		fprintf( cgiOut, "	}\n" );
		fprintf( cgiOut, "} \n" );
//如果协议是tcp,在action中添加　TCPMSS的选项。
//否则去掉该选项，如果当前action选择的是该选项，将选择的选项改为accept同时，将tcpmss的val显示去掉。
		fprintf( cgiOut, "if( PAGE_TYPE == 'FW_WALL' ){\n" );
		fprintf( cgiOut, "	var actionSelector = document.getElementsByName('filterAction')[0];\n"\
						"	if( '1' == document.getElementById('Protocol').value )\n"
						"	{\n"\
						"		var option = document.createElement('option');\n"\
						"		option.text = 'tcpmss';\n"\
						"		option.value = 'tcpmss';\n"\
						"		try\n"\
    					"		{\n"\
    					"			actionSelector.add(option,null); \n"\
    					"		}\n"\
  						"		catch(ex)\n"\
    					"		{\n"\
    					"			actionSelector.add(option);\n"\
    					"		}\n"\
  						"	}\n"\
						"	else\n"\
						"	{\n"\
						"		if( actionSelector.length == 4 )\n"\
						"		{\n"\
						"			actionSelector.remove(3);\n"\
						"			actionSelector.selectedIndex == 0;\n"\
						"			filterActionChange();\n"\
						"		}\n"\
						"	}\n" );
		fprintf( cgiOut, "}\n" );
//END tcpmss
		fprintf( cgiOut, "autoCheckHeight();\n" );
		fprintf( cgiOut, "} \n" );
		//修改table高度，解决左边黑线断掉的问题。
		fprintf( cgiOut, "function autoCheckHeight()\n" );
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "	document.getElementById('FILL_OBJ').style.height = document.getElementById('rules').offsetHeight-80;\n");
		fprintf( cgiOut, "}\n");		
		fprintf( cgiOut, "PortTypeSelChange(); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "</script> \n" );
		fprintf( cgiOut, "<script type=\"text/javascript\"> \n" );
		//fprintf( cgiOut, "alert('eee');\n" );
		fprintf( cgiOut, "if( \"FW_SNAT\" == PAGE_TYPE ) \n" );
		fprintf( cgiOut, "{ \n" );
		//fprintf( cgiOut, "alert('fff');\n" );	
		fprintf( cgiOut, "document.getElementById(\"DNAT\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "document.getElementById(\"FILTER\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "else if( \"FW_DNAT\" == PAGE_TYPE ) \n" );
		fprintf( cgiOut, "{ \n" );
		//fprintf( cgiOut, "alert('ggg');\n" );	
		fprintf( cgiOut, "document.getElementById(\"SNAT\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "document.getElementById(\"FILTER\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "else if( \"FW_INPUT\" == PAGE_TYPE ) \n" );
		fprintf( cgiOut, "{ \n" );
		//fprintf( cgiOut, "alert('ggg');\n" );	
		fprintf( cgiOut, "document.getElementById(\"NAT\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "//document.getElementById(\"DNAT\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "else /*if( \"FW_WALL\" == PAGE_TYPE ) */\n" );
		fprintf( cgiOut, "{ \n" );
		//fprintf( cgiOut, "alert('hhh');\n" );
		fprintf( cgiOut, "document.getElementById(\"NAT\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "//document.getElementById(\"DNAT\").innerHTML = \"\"; \n" );
		fprintf( cgiOut, "} \n" );	
		fprintf( cgiOut, "</script> \n" );
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		if( NULL != rule )
		{
			fprintf( cgiOut, "var curRule = new fwrule( \"%d\", \"%d\",\"%d\",", rule->type, rule->id, rule->ordernum);
			fprintf( cgiOut, "\"%s\", \"%d\", \"%d\", \"%s\", \"%s\", \"%s\", ", rule->name, rule->enable, rule->status, rule->comment, rule->ineth, rule->outeth );
			fprintf( cgiOut, "\"%d\", \"%s\", \"%d\", \"%s\", \"%d\", \"%d\",\"%s\", \"%d\", \"%s\", ", rule->srctype, rule->srcadd, rule->dsttype, rule->dstadd, rule->protocl, rule->sptype, rule->sport, rule->dptype, rule->dport );
			fprintf( cgiOut, "\"%d\", \"%d\",\"%s\",\"%d\",\"%s\", '%s', '%s' );\n", rule->act, rule->natiptype, rule->natipadd, rule->natpttype, rule->natport,rule->pkg_state, rule->string_filter );
			fprintf( cgiOut, "document.getElementById( 'ruleDescription' ).value = curRule.rulename;\n");
			fprintf( cgiOut, "if( curRule.enable == '1' ){\n" );
			fprintf( cgiOut, "	document.getElementById( 'ruleEnabledCHECKBOX' ).checked = 'checked';\n" );
			fprintf( cgiOut, "}else{\n" );
			fprintf( cgiOut, "	document.getElementById( 'ruleEnabledCHECKBOX' ).checked = '';\n" );
			fprintf( cgiOut, "}\n" );
			//fprintf( cgiOut, "alert( curRule.ineth );\n");
			fprintf( cgiOut, "document.getElementById( 'inEth' ).value = curRule.ineth; \n" );
		if( strcmp( ruleType, "FW_INPUT" ) != 0)
			fprintf( cgiOut, "document.getElementById( 'outEth' ).value = curRule.outeth; \n" );
		
			//fprintf( cgiOut, "alert( curRule.srcIpType );\n");
			//fprintf( cgiOut, "document.getElementsByName( 'Package.SourceAddress' )[curRule.srcIpType].checked = 'checked';\n" );

			//fprintf( cgiOut, "alert( curRule.srcIpType );\n");
			//fprintf( cgiOut, "alert( typeof curRule.srcIpType );\n");
			fprintf( cgiOut, "switch( parseInt(curRule.srcIpType) ){\n");
			fprintf( cgiOut, "	case 1: \n" );
			fprintf( cgiOut, "		packageSourceAddressRadioControl.setActive( \"Package.SourceAddress.Single\" ); \n" );
			fprintf( cgiOut, "		document.getElementById( 'Package.SourceAddress.Single.IP' ).value = curRule.srcIpAddr;\n" );
			fprintf( cgiOut, "		break;\n" );
			fprintf( cgiOut, "	case 2: \n ");
			//第二个是 host  现在去掉了
		//	fprintf( cgiOut, "packageSourceAddressRadioControl.setActive( \"Package.SourceAddress.AddrMask\" ); \n" );
			fprintf( cgiOut, "		break;\n" );
			fprintf( cgiOut, "	case 3:\n");
			fprintf( cgiOut, "		packageSourceAddressRadioControl.setActive( \"Package.SourceAddress.AddrMask\" ); \n" );
//fprintf( cgiOut, "alert(curRule.srcIpAddr);\n" );
//fprintf( cgiOut, "alert( curRule.srcIpAddr.match( /.*\\// ).toString().replace( /.$/, '') );\n" );
			fprintf( cgiOut, "		document.getElementById( 'Package.SourceAddress.AddrMask.Addr' ).value = curRule.srcIpAddr.match( /.*\\// ).toString().replace( /.$/, '');\n" );		
			fprintf( cgiOut, "		document.getElementById( 'Package.SourceAddress.AddrMask.Mask' ).value = curRule.srcIpAddr.match( /\\/.*/ ).toString().replace( /^./, '');\n" );
			
			//fprintf( cgiOut, "	document.getElementById( 'Package.SourceAddress.AddrMask.Mask' ).value = curRule.srcIpAddr;\n" );
			fprintf( cgiOut, "		break;\n");
#if DEFAULT_NETWORK_ENABLE			
			fprintf( cgiOut, "	case 4:\n");
			fprintf( cgiOut, "		packageSourceAddressRadioControl.setActive( \"Package.SourceAddress.DefinedNetwork\" ); \n" );
			fprintf( cgiOut, "		document.getElementById( 'Package.SourceAddress.DefinedNetwork.Select' ).value = curRule.srcIpAddr;\n" );
			fprintf( cgiOut, "		break;\n");
#endif			
			fprintf( cgiOut, "	case 5:\n");
			fprintf( cgiOut, "		packageSourceAddressRadioControl.setActive( \"Package.SourceAddress.AddressRange\" ); \n" );
			fprintf( cgiOut, "		document.getElementById( 'Package.SourceAddress.AddressRange.Begin' ).value = curRule.srcIpAddr.match( /.*-/ ).toString().replace( /.$/, '');\n" );		
			fprintf( cgiOut, "		document.getElementById( 'Package.SourceAddress.AddressRange.End' ).value = curRule.srcIpAddr.match( /-.*/ ).toString().replace( /^./, '');\n" );	
			fprintf( cgiOut, "		break;\n");
			fprintf( cgiOut, "	case 0:\n");
			fprintf( cgiOut, "	default:\n");
			fprintf( cgiOut, "		packageSourceAddressRadioControl.setActive( \"Package.SourceAddress.Any\" ); \n" );
			fprintf( cgiOut, "		break;\n");
			fprintf( cgiOut, "}\n" );
#if 1			
			//目标地址
			fprintf( cgiOut, "switch( parseInt(curRule.dstIpType) ){\n");
			fprintf( cgiOut, "	case 1: \n" );
			fprintf( cgiOut, "		packageDestinationAddressRadioControl.setActive( \"Package.DestinationAddress.Single\" ); \n" );
			fprintf( cgiOut, "		document.getElementById( 'Package.DestinationAddress.Single.IP' ).value = curRule.dstIpAddr;\n" );
			fprintf( cgiOut, "		break;\n" );
			fprintf( cgiOut, "	case 2: \n ");
			//第二个是 host  现在去掉了
		//	fprintf( cgiOut, "		packageDestinationAddressRadioControl.setActive( \"Package.DestinationAddress.AddrMask\" ); \n" );
			fprintf( cgiOut, "		break;\n" );
			fprintf( cgiOut, "	case 3:\n");
			fprintf( cgiOut, "		packageDestinationAddressRadioControl.setActive( \"Package.DestinationAddress.AddrMask\" ); \n" );
			//fprintf( cgiOut, "	alert(curRule.srcIpAddr);\n" );
			//fprintf( cgiOut, "	alert( curRule.srcIpAddr.match( /\\/.*/ ).toString().replace( /^./, '') );\n" );
			fprintf( cgiOut, "		document.getElementById( 'Package.DestinationAddress.AddrMask.Addr' ).value = curRule.dstIpAddr.match( /.*\\// ).toString().replace( /.$/, '');\n" );		
			fprintf( cgiOut, "		document.getElementById( 'Package.DestinationAddress.AddrMask.Mask' ).value = curRule.dstIpAddr.match( /\\/.*/ ).toString().replace( /^./, '');\n" );
			
			//fprintf( cgiOut, "	document.getElementById( 'Package.DestinationAddress.AddrMask.Mask' ).value = curRule.srcIpAddr;\n" );
			fprintf( cgiOut, "		break;\n");
#if DEFAULT_NETWORK_ENABLE			
			fprintf( cgiOut, "	case 4:\n");
			fprintf( cgiOut, "		packageDestinationAddressRadioControl.setActive( \"Package.DestinationAddress.DefinedNetwork\" ); \n" );
			fprintf( cgiOut, "		document.getElementById( 'Package.DestinationAddress.DefinedNetwork.Select' ).value = curRule.dstIpAddr;\n" );
			fprintf( cgiOut, "		break;\n");
#endif			
			fprintf( cgiOut, "	case 5:\n");
			fprintf( cgiOut, "		packageDestinationAddressRadioControl.setActive( \"Package.DestinationAddress.AddressRange\" ); \n" );
			fprintf( cgiOut, "		document.getElementById( 'Package.DestinationAddress.AddressRange.Begin' ).value = curRule.dstIpAddr.match( /.*-/ ).toString().replace( /.$/, '');\n" );		
			fprintf( cgiOut, "		document.getElementById( 'Package.DestinationAddress.AddressRange.End' ).value = curRule.dstIpAddr.match( /-.*/ ).toString().replace( /^./, '');\n" );		
			fprintf( cgiOut, "		break;\n");
			fprintf( cgiOut, "	case 0:\n");
			fprintf( cgiOut, "	default:\n");
			fprintf( cgiOut, "		packageDestinationAddressRadioControl.setActive( \"Package.DestinationAddress.Any\" ); \n" );
			fprintf( cgiOut, "		break;\n");
			fprintf( cgiOut, "}\n" );
#endif
			//选择协议类型
			fprintf( cgiOut, "document.getElementById( 'Protocol' ).value = curRule.protocl;\n" );
			fprintf( cgiOut, "PortTypeSelChange();\n" );
			
			//端口信息
			fprintf( cgiOut, "if( '1' == curRule.protocl || '2' == curRule.protocl || '3' == curRule.protocl ) \n " );
			fprintf( cgiOut, "{\n" );		
			//source port	
			fprintf( cgiOut, "	switch( parseInt( curRule.srcPortType )	)\n" );
			fprintf( cgiOut, "	{\n" );
			fprintf( cgiOut, "		case 1:\n" );

			fprintf( cgiOut, "			packageSourcePortRadioControl.setActive( \"Package.SourcePort.Single\" ); \n" );
			fprintf( cgiOut, "			document.getElementById( 'Package.SourcePort.Single.Port' ).value = curRule.srcPort;\n"  			);
			fprintf( cgiOut, "			break;\n");
			fprintf( cgiOut, "		case 2:\n");
			//fprintf( cgiOut, "alert('2222');\n" );
			fprintf( cgiOut, "			packageSourcePortRadioControl.setActive( \"Package.SourcePort.Range\" ); \n" );
			//fprintf( cgiOut, "alert( curRule.srcPort );\n" );
			fprintf( cgiOut, "			document.getElementById( 'Package.SourcePort.Range.Begin' ).value = curRule.srcPort.toString().match( /.*:/ ).toString().replace( /.$/, '');\n" );		
			fprintf( cgiOut, "			document.getElementById( 'Package.SourcePort.Range.End' ).value = curRule.srcPort.toString().match( /:.*/ ).toString().replace( /^./, '');\n" );						
			fprintf( cgiOut, "			break;\n" );
#if PORT_COLLECTION_ENABLE
			fprintf( cgiOut, "		case 3:\n" );
			fprintf( cgiOut, "			break;\n" );
#endif						
			fprintf( cgiOut, "		default:\n" );
			fprintf( cgiOut, "		case 0:\n");
			//fprintf( cgiOut, "alert('3333');\n" );
			fprintf( cgiOut, "			packageSourcePortRadioControl.setActive( \"Package.SourcePort.Any\" ); \n" );
			fprintf( cgiOut, "			break;\n" );
			fprintf( cgiOut, "	}\n");
			
			//des port
			fprintf( cgiOut, "	switch( parseInt( curRule.desPortType )	)\n" );
			fprintf( cgiOut, "	{\n" );
			fprintf( cgiOut, "		case 1:\n" );
			fprintf( cgiOut, "			packageDestinationPortRadioControl.setActive( \"Package.DestinationPort.Single\" ); \n" );
			fprintf( cgiOut, "			document.getElementById( 'Package.DestinationPort.Single.Port' ).value = curRule.desPort;\n"  			);
			fprintf( cgiOut, "			break;\n");
			fprintf( cgiOut, "		case 2:\n");
			//fprintf( cgiOut, "alert('2222');\n" );
			fprintf( cgiOut, "			packageDestinationPortRadioControl.setActive( \"Package.DestinationPort.Range\" ); \n" );
			fprintf( cgiOut, "			document.getElementById( 'Package.DestinationPort.Range.Begin' ).value = curRule.desPort.toString().match( /.*:/ ).toString().replace( /.$/, '');\n" );		
			fprintf( cgiOut, "			document.getElementById( 'Package.DestinationPort.Range.End' ).value = curRule.desPort.toString().match( /:.*/ ).toString().replace( /^./, '');\n" );						
			fprintf( cgiOut, "			break;\n" );
#if PORT_COLLECTION_ENABLE
			fprintf( cgiOut, "		case 3:\n" );
			fprintf( cgiOut, "			break;\n" );
#endif						
			fprintf( cgiOut, "		default:\n" );
			fprintf( cgiOut, "		case 0:\n");
			//fprintf( cgiOut, "alert('3333');\n" );
			fprintf( cgiOut, "			packageDestinationPortRadioControl.setActive( \"Package.DestinationPort.Any\" ); \n" );		
			fprintf( cgiOut, "			break;\n" );
			fprintf( cgiOut, "	}\n\n\n\n");		
			

			
			//如果是nat，则nat部分也需要配置端口
			fprintf( cgiOut, "if( curRule.ruleType == '1' || curRule.ruleType == '2' ) \n  " );//如果当前规则类型nat
			fprintf( cgiOut, "{\n" );
				//port trans
			fprintf( cgiOut, "		switch( parseInt( curRule.natPortType )	)\n" );
			fprintf( cgiOut, "		{\n" );
			fprintf( cgiOut, "		case 1:\n" );
			fprintf( cgiOut, "			NatTranslationPort.setActive( \"NatPortSinglePort\" ); \n" );
			fprintf( cgiOut, "			document.getElementById( 'NatSinglePort' ).value = curRule.natPort;\n"  			);
			fprintf( cgiOut, "			break;\n");
			fprintf( cgiOut, "		case 2:\n");
					//fprintf( cgiOut, "alert('2222');\n" );
			fprintf( cgiOut, "			NatTranslationPort.setActive( \"NatPortRangePort\" ); \n" );
			fprintf( cgiOut, "			document.getElementById( 'NatPortRangBegin' ).value = curRule.natPort.match( /.*-/ ).toString().replace( /.$/, '');\n" );		
			fprintf( cgiOut, "			document.getElementById( 'NatPortRangEnd' ).value = curRule.natPort.match( /-.*/ ).toString().replace( /^./, '');\n" );						
			fprintf( cgiOut, "			break;\n" );
#if PORT_COLLECTION_ENABLE
			fprintf( cgiOut, "		case 3:\n" );
			fprintf( cgiOut, "			break;\n" );
#endif						
			fprintf( cgiOut, "		default:\n" );
			fprintf( cgiOut, "		case 0:\n");
			//fprintf( cgiOut, "alert('3333');\n" );
			fprintf( cgiOut, "			NatTranslationPort.setActive( \"NatPortDefault\" ); \n" );
			fprintf( cgiOut, "			break;\n" );
			fprintf( cgiOut, "		}\n");
			fprintf( cgiOut, "}\n");
			fprintf( cgiOut, "}\n");

			//fprintf( cgiOut, "alert( curRule.ruleType );\n" );
			fprintf( cgiOut, "if( curRule.ruleType == '0' ){\n" );
			fprintf( cgiOut, "	switch( parseInt(curRule.ruleaction) ){\n" );
			fprintf( cgiOut, "		case 0:\n" );
			fprintf( cgiOut, "			document.getElementById('filterAction').value = 'allow';\n" );
			fprintf( cgiOut, "			break;\n" );
			fprintf( cgiOut, "		case 1:\n");
			fprintf( cgiOut, "			document.getElementById('filterAction').value = 'deny';\n" );
			fprintf( cgiOut, "			break;\n" );
			fprintf( cgiOut, "		case 2:\n" );
			fprintf( cgiOut, "			document.getElementById('filterAction').value = 'reject';\n" );
			fprintf( cgiOut, "			break;\n" );	
//tcpmss
			fprintf( cgiOut, "		case 3:\n" );
			fprintf( cgiOut, "			document.getElementById('filterAction').value = 'tcpmss';\n"\
							"			filterActionChange();\n"\
							"			document.getElementsByName('tcpmss_val')[0].value=%s\n", rule->tcpmss_var );
			fprintf( cgiOut, "			break;\n" );
//end tcpmss			
			fprintf( cgiOut, "	}\n");
			fprintf( cgiOut, "}\n");
				//addr trans
			//目标地址
			fprintf( cgiOut, "	if( curRule.ruleType == '1' || curRule.ruleType == '2' ) {\n");
			fprintf( cgiOut, "switch( parseInt(curRule.natIpType) ){\n");
				fprintf( cgiOut, "case 1: \n" );
					fprintf( cgiOut, "NatTranslationIp.setActive( \"NatSingleAddr\" ); \n" );
					fprintf( cgiOut, "document.getElementById( 'NatSingleIpAddress' ).value = curRule.natIpAddr;\n" );
					fprintf( cgiOut, "	break;\n" );
				fprintf( cgiOut, "case 2: \n ");
				//第二个是 host  现在去掉了
			//	fprintf( cgiOut, "NatTranslationIp.setActive( \"Package.DestinationAddress.AddrMask\" ); \n" );
				fprintf( cgiOut, "break;\n" );
				fprintf( cgiOut, "case 3:\n");
					fprintf( cgiOut, "NatTranslationIp.setActive( \"NatAddrMask\" ); \n" );
					//fprintf( cgiOut, "alert(curRule.srcIpAddr);\n" );
					//fprintf( cgiOut, "alert( curRule.srcIpAddr.match( /\\/.*/ ).toString().replace( /^./, '') );\n" );
					fprintf( cgiOut, "document.getElementById( 'NatAddressNetmaskAddr' ).value = curRule.natIpAddr.match( /.*\\// ).toString().replace( /.$/, '');\n" );		
					fprintf( cgiOut, "document.getElementById( 'NatAddressNetmaskMask' ).value = curRule.natIpAddr.match( /\\/.*/ ).toString().replace( /^./, '');\n" );
					
					//fprintf( cgiOut, "document.getElementById( 'Package.DestinationAddress.AddrMask.Mask' ).value = curRule.srcIpAddr;\n" );
				fprintf( cgiOut, "break;\n");
#if DEFAULT_NETWORK_ENABLE				
				fprintf( cgiOut, "case 4:\n");
					fprintf( cgiOut, "NatTranslationIp.setActive( \"NatDefineNetwork\" ); \n" );
					fprintf( cgiOut, "document.getElementById( 'NatDefineNetworkSelect' ).value = curRule.natIpAddr;\n" );
				fprintf( cgiOut, "break;\n");
#endif				
				fprintf( cgiOut, "case 5:\n");
					fprintf( cgiOut, "NatTranslationIp.setActive( \"NatAddrRange\" ); \n" );
					fprintf( cgiOut, "document.getElementById( 'NatAddrRangeBegin' ).value = curRule.natIpAddr.match( /.*-/ ).toString().replace( /.$/, '');\n" );		
					fprintf( cgiOut, "document.getElementById( 'NatAddrRangeEnd' ).value = curRule.natIpAddr.match( /-.*/ ).toString().replace( /^./, '');\n" );		
					fprintf( cgiOut, "break;\n");
				fprintf( cgiOut, "case 0:\n");
				fprintf( cgiOut, "default:\n");
					fprintf( cgiOut, "NatTranslationIp.setActive( \"NatMassquerade\" ); \n" );
				fprintf( cgiOut, "break;\n");
			fprintf( cgiOut, "}\n" );				
				
			fprintf( cgiOut, "}\n");
			
			//state
			if( NULL !=  rule->pkg_state )
			{
				if( NULL !=  strstr( rule->pkg_state, "NEW" ) )
					fprintf( cgiOut, "document.getElementById( 'state_NEW' ).checked = true;\n");
				if( NULL !=  strstr( rule->pkg_state, "ESTABLISHED" ) )
					fprintf( cgiOut, "document.getElementById( 'state_ESTABLISHED' ).checked = true;\n");
				if( NULL !=  strstr( rule->pkg_state, "RELATED" ) )
					fprintf( cgiOut, "document.getElementById( 'state_RELATED' ).checked = true;\n");
				if( NULL !=  strstr( rule->pkg_state, "INVALID" ) )
					fprintf( cgiOut, "document.getElementById( 'state_INVALID' ).checked = true;\n");
			}
			//string
			
		}
		
{
		fprintf( cgiOut, "function isAllLegal( thisform )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "var ret = true;\n");
		//src
		fprintf( cgiOut, "if( isSourceAddrLegel() != true )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "alert( '%s' );\n", search( lfirewall, "ruleedit_err_srcaddr" ) );
		fprintf( cgiOut, "ret = false;\n" );
		fprintf( cgiOut, "}\n" );
#if 1	//dest
		fprintf( cgiOut, "if( isDestinationAddrLegel() != true )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "alert( '%s' );\n", search( lfirewall, "ruleedit_err_desaddr") );
		fprintf( cgiOut, "ret = false;\n" );
		fprintf( cgiOut, "}\n" );
#endif		
		//srcport
		fprintf( cgiOut, "var protocolSel = document.getElementById('Protocol').value;\n");
		fprintf( cgiOut, "if( '1' == protocolSel || '2' == protocolSel || '3' == protocolSel )\n");
		fprintf( cgiOut, "{\n");
		
		fprintf( cgiOut, "if( isSourcePortLegal() != true )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "alert( '%s' );\n", search( lfirewall, "ruleedit_err_srcport") );
		fprintf( cgiOut, "ret = false;\n");
		fprintf( cgiOut, "}\n");
		//desport
		fprintf( cgiOut, "if( isDestinationPortLegal() != true )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "alert( '%s' );\n", search( lfirewall, "ruleedit_err_desport") );
		fprintf( cgiOut, "ret = false;\n");
		fprintf( cgiOut, "}\n");
		
		fprintf( cgiOut, "}\n");
#if 1		
		//trans addr
		//fprintf( cgiOut,"alert( 'aaaa'+PAGE_TYPE );\n" );
		fprintf( cgiOut, "if( PAGE_TYPE == 'FW_SNAT' || PAGE_TYPE == 'FW_DNAT' )\n ");
		fprintf( cgiOut, "{\n");
		//fprintf( cgiOut,"alert( 'before isTranslationAddrLegal' );\n" );
		fprintf( cgiOut, "if( isTranslationAddrLegal() != true )\n");
		fprintf( cgiOut, "{\n");
				//if( PAGE_TYPE == 'SNAT' )
				//	alert( 'SNAT  ip address error!' );
				//else
		fprintf( cgiOut, "alert( '%s' );\n", search( lfirewall, "ruleedit_err_nataddr" ));
		fprintf( cgiOut, "ret=false;\n" );
		fprintf( cgiOut, "}\n");
		
		fprintf( cgiOut, "if( '1' == protocolSel || '2' == protocolSel || '3' == protocolSel )\n");
		fprintf( cgiOut, "{\n");
		//port  trans
		fprintf( cgiOut, "if( isTranslationPortLegal() != true )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "alert( '%s' );\n", search( lfirewall,"ruleedit_err_natport" ) );
		fprintf( cgiOut, "ret = false;\n");
		fprintf( cgiOut, "}\n");
		
		fprintf( cgiOut, "}\n");

		fprintf( cgiOut, "}\n");
		
		//判断rule name是否都为字母
		/*fprintf(cgiOut,"var re = /^[\w_.]*[\w]+[\w_.]*$/;"\
			"if( ! re.test(document.getElementById('ruleDescription').value) )"\
			"{"\
			"alert('%s');"\
			"return false;"\
			"}",search( lfirewall, "ruleedit_err_rulename" ));*/
		//fprintf( cgiOut, "if( isLegalRuleName( document.getElementById('ruleDescription').value) != true ){\n" );
		//fprintf( cgiOut, "alert( '%s' );\n", search( lfirewall, "ruleedit_err_rulename" ) );
		//fprintf( cgiOut, "ret = false;\n }\n" );
		
		//判断mssval是否都是数字
		fprintf( cgiOut, "if( PAGE_TYPE == 'FW_WALL' ){\n" );
		fprintf( cgiOut, "	var reg=/^\\s*(\\d+)\\s*$/;\n"\
						"	var action = document.getElementsByName('filterAction')[0].value;\n"\
						"	if( action == 'tcpmss' )\n"\
						"	{\n"\
						"		var tcmmms_value = document.getElementsByName( 'tcpmss_val' )[0].value;\n"\
						"		if(! reg.test(tcmmms_value))\n"
						"		{\n"
						"			alert('MSS  error!');\n"\
						"			ret = false;\n"\
						"		}\n"
						"	}\n" );
		fprintf( cgiOut, "}\n" );
		
#endif
		
		fprintf( cgiOut, "return ret;\n");
		fprintf( cgiOut, "}\n");			
}
		
		
		
		fprintf( cgiOut, "</script>\n" );		
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "</body> \n" );
		fprintf( cgiOut, "<script type=text/javascript>\n" );
		fprintf( cgiOut, "autoCheckHeight();\n" );
		//onInEthChange( document.getElementsByName( 'inEth' )[0] );
		fprintf( cgiOut, "onInEthChange( document.getElementsByName('inEth')[0] );\n" );
		if( strcmp( ruleType, "FW_INPUT" ) != 0)
		{
			fprintf( cgiOut, "onOutEthChange( document.getElementsByName('outEth')[0] );\n" );
		}
		fprintf( cgiOut, "indexSelectChange();\n");		
		fprintf( cgiOut, "</script>\n" );
		fprintf( cgiOut, "</html> \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, " \n" );

		if (NULL != rule) {
			free(rule);
		}
		
		free( encry );
		//fwFreeList(list);		
		release(lpublic); 
		release(lfirewall);
		
		free_instance_parameter_list(&paraHead1);
//#endif		
		return 0;	
}
		

//#ifdef _MANAGE_FIREWALL_ 
static int showInterfaceSelector( char *selectName, char *onchangestr, char *disable )
{
#if 0
	FILE *fp;
	char *cmd = "ip link | sed \"/.*link\\/.*brd.*/d\" | sed \"s/^[0-9]*:.\\(.*\\):.*$/\\1/g\" | sed \"/lo/d\" |sed \"/pimreg@NONE/d\" |sed \"/link\\/pimreg/d\" | sed \"s/^\\(.*\\)$/<option value='\\1'>\\1<\\/option>/g\"";
	char buff[256];
	char dupinf[20] = {0};
	
	fp = popen( cmd, "r" );
	if( NULL == fp )
	{
		return -1;	
	}
	
	fprintf( cgiOut, "<select name=%s onchange='%s' %s>", selectName, onchangestr, disable );
	//可以选择any
	fprintf( cgiOut, "<option value=any>any</option>\n");
	fgets( buff, sizeof(buff), fp );
	do//最后一行老是会多读一次，采用do  while方式可以把最后多读的一行去掉～
	{
		fprintf(stderr, "--------------------prev=#%s#,intf=#%s#\n", dupinf, buff);
		if (0 == strcmp(buff, dupinf)) {
			continue;
		}
		strncpy(dupinf, buff, sizeof(dupinf));
		fprintf( cgiOut, "%s", buff );
		fgets( buff, sizeof(buff), fp );
	}while( !feof(fp) );
	pclose(fp);
	fprintf( cgiOut, "</select>\n" );
	
	return 0;
#endif

	FILE * ft;
	char * syscommand=(char *)malloc(300);
	memset(syscommand,0,300);
	int i = 0, j = 0,is_repeat = 0;
	sprintf(syscommand,"ip addr | awk 'BEGIN{FS=\":\";RS=\"(\^|\\n)[0-9]+:[ ]\"}{print $1}' | awk 'NR==2,NR==0{print}' ");
	ft = popen(syscommand,"r"); 
	char  temp[20];
	memset(temp,0,20);
	char intfname[4095][20] = {{0}};
	i=0;

	fprintf( cgiOut, "<select name=%s onchange='%s' %s>", selectName, onchangestr, disable );
	//可以选择any
	fprintf( cgiOut, "<option value=any>any</option>\n");

	if(ft != NULL)
	{
		while((fgets(temp,18,ft)) != NULL)
		{
			 is_repeat = 0;
			 for(j=0;j<i;j++)
			 {
			 	if(0 == strncmp(temp,intfname[j],strlen(temp)-1))
			 	{
			 		is_repeat = 1;
			 	}
			 }

			 if(0 == is_repeat)
			 {			 	 
				 strncpy(intfname[i],temp,strlen(temp)-1);
				 i++;
				 if(0 == strncmp(temp, "ve", 2)) {
					char *temp_ve = NULL;

				 	temp_ve = strchr(temp,'@');
					if (temp_ve) {
						*temp_ve = '\0';
					}
				 }
				 fprintf(cgiOut, "<option value='%s'>%s</option>\n", temp, temp);
			 }
			 memset(temp,0,20);
		}
		pclose(ft);
	}
	
	fprintf( cgiOut, "</select>\n" );
	free(syscommand);
	return 1;
}
//#endif
