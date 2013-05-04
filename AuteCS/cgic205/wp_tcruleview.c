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
* wp_tcruleview.c
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


static int ruleIndexSelector( int ruleNum, int ruleTotalNum, char *encry );

int cgiMain()
{ 
	struct list *lpublic;
	struct list *lfirewall; 
	int i=0;
//	int ruleType = 0;
		//得到rule的类型， fileter dnat  snat
	char ruleTypeStr[10]="";//FILTER   SNAT   DNAT	
	PTCRule ptcRuleRoot,ptcRuleTemp;

	char *encry=(char *)malloc(BUF_LEN);
	char *str;

	char lan[3]; 
	FILE *fp1;
	int manager = -1;
	int ruleNumTotal=0;
		
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lfirewall=get_chain_head("../htdocs/text/firewall.txt");
	
 	memset(encry,0,BUF_LEN);
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
  	str=dcryption(encry);
	tcrule_status_exist();
  	if(str==NULL)
  	{
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
  	}
  manager = checkuser_group(str);
	cgiHeaderContentType("text/html");
	
 	if((fp1=fopen("../htdocs/text/public.txt","r"))==NULL)		   /*以只读方式打开资源文件*/
 	{
		ShowAlert(search(lpublic,"error_open"));
		return 0;
	}
	fseek(fp1,4,0);						  /*将文件指针移到离文件首4个字节处，即lan=之后*/
	fgets(lan,3,fp1); 	 
	fclose(fp1);
		  	
	ptcRuleRoot = tcParseDoc( TCRULES_XML_FILE );

    for( ptcRuleTemp=ptcRuleRoot; ptcRuleTemp!=NULL; ptcRuleTemp=ptcRuleTemp->next,ruleNumTotal++ );//统计总数
	
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
  	
	fprintf( cgiOut, "<title>%s</title> \n", "xxxxxxx" );
	fprintf( cgiOut, "<link rel=stylesheet href=/style.css type=text/css> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, ".usrlis {overflow-x:hidden; overflow:auto; width: 416px; height: 270px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "</head> \n" );
	fprintf( cgiOut, "<script src=\"/fw.js\"></script> \n" );
	fprintf( cgiOut, "<script type='text/javascript'>\n" );
	fprintf( cgiOut, "function autoCheckHeight()\n" );
	fprintf( cgiOut, "{\n");
	fprintf( cgiOut, "	if( document.getElementById('list').offsetHeight < 50 )\n"\
					"		document.getElementById('FILL_OBJ').style.height = 1;\n"\
					"	else\n" );
	fprintf( cgiOut, "		document.getElementById('FILL_OBJ').style.height = document.getElementById('list').offsetHeight - 50;\n");
	fprintf( cgiOut, "}\n");
	fprintf( cgiOut, "</script>\n" );
	fprintf( cgiOut, "<body> \n" );

	fprintf( cgiOut, "<form action='wp_tcrulemodify.cgi' method='post'> \n" );
	fprintf( cgiOut, "<input type='hidden' name='UN' value='%s'>", encry );
	fprintf( cgiOut, "<input type='hidden' name='ruleType' value='%s'>", ruleTypeStr );
	fprintf( cgiOut, "<div align=center> \n" );
	fprintf( cgiOut, "<table width=976 border=0 cellpadding=0 cellspacing=0> \n" );
	fprintf( cgiOut, "<tr><td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td> \n" );
	fprintf( cgiOut, "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td> \n" );
	fprintf( cgiOut, "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s </td> \n", search(lpublic,"title_style"), search(lfirewall,"title_tc") );

	fprintf( cgiOut, "<td width=690 align=right valign=bottom background=/images/di22.jpg> \n" );
	
		  
	{   
		fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		"<tr>\n" );
		if( 0 == manager )
		{
			fprintf( cgiOut, "<td width=62 align=center><input id='but' type='submit' name='submit_doalltcrules' style='background-image:url(/images/%s)' value=''></td>\n", search(lpublic,"img_ok") );	
		}
		else
		{
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_firewall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry, search(lpublic,"img_ok"));
		}
		fprintf(cgiOut,"<td width=62 align=left><a href=wp_firewall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		fprintf(cgiOut,"</tr>"\
		"</table>");
	}
	
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


	fprintf( cgiOut, "<tr height=26><td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td></tr> \n", search(lpublic,"menu_san"), search(lfirewall,"title_tc"));
	if( 0 == manager )
		fprintf( cgiOut, "<tr height=26><td align=left id=tdleft><a href='wp_tcruleedit.cgi?UN=%s&editType=add'><font id=%s>%s</font></td></tr> \n", encry, search(lpublic,"menu_san"), search(lfirewall,"tc_addrule"));	
	else
		fprintf( cgiOut, "<tr height=26><td align=left id=tdleft>&nbsp;</td></tr>" );
	
	fprintf( cgiOut, "<tr><td align=left id='FILL_OBJ' style='height:100px;padding-left:10px;border-right:1px solid #707070;color:#000000;text-align:center'>&nbsp;</td></tr> \n" );
       	
	fprintf( cgiOut, "</table> \n" );
	fprintf( cgiOut, "</td> \n" );
	fprintf( cgiOut, "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">\n");
	fprintf( cgiOut, "<div id=list align=left style='width:768px;overflow:auto' onresize='autoCheckHeight();'>\n" );
	
	
	//这里完成对规则制表。
//	fprintf( cgiOut, "<a href='wp_tcruleedit.cgi?UN=%s&editType=add'><i>%s</i></a> %s", encry, search(lfirewall,"rule_add") , search(lfirewall,"rule_new")  );
	fprintf( cgiOut, "<table frame=below rules=rows border=1 style='border-collapse:collapse;align:left'>\n" );
	fprintf( cgiOut, "<tr align=left>\n" );
		fprintf( cgiOut, "<th width=16></th><th width=60>%s</th>", search(lfirewall,"rule_pos") );
		fprintf( cgiOut, "<th width=200>%s</th>\n", search(lfirewall,"tc_ipaddr") );
		fprintf( cgiOut, "<th width=200><font id=%s>%s</font>(<font id=%s>%s</font><font id=yingwen_thead>:kbyte</font>)</th>\n", search(lpublic,"menu_thead"),search(lfirewall,"tc_traffic"), search(lpublic,"menu_thead"),  search(lfirewall,"tc_unit") );
		fprintf( cgiOut, "<th width=150><font id=yingwen_thead>P2P</font></th>\n" );
#if USE_RULE_TIME_CTROL
		fprintf( cgiOut, "<th width=100><font id=yingwen_thead>Time</font></th>\n" );
#endif		
		fprintf( cgiOut, "<th width=60><font id=yingwen_thead>%s</font></th>\n",search(lfirewall,"rule_state") );		

		fprintf( cgiOut, "<th width=20> </th>\n" );
		fprintf( cgiOut, "<th width=60> </th>\n" );
	fprintf( cgiOut, "</tr>\n" );

	if( NULL == ptcRuleRoot )
	{
		fprintf( cgiOut, "</table>\n" );
		if( 0 == manager )
			fprintf( cgiOut, "%s <a href='wp_tcruleedit.cgi?UN=%s&editType=add'><i>%s</i></a> %s", search(lfirewall,"rule_no_pre"),encry, search(lfirewall,"rule_no_here"), search(lfirewall,"rule_no_post") );
	}
	else
	{
	    
		int tcRuleNum=0;
		
		for( ptcRuleTemp=ptcRuleRoot; ptcRuleTemp!=NULL; ptcRuleTemp=ptcRuleTemp->next,tcRuleNum++ )
		{
			fprintf( stderr, "ptcRuleTemp = %x\n", (unsigned int)ptcRuleTemp );
		}
		
		for( ptcRuleTemp=ptcRuleRoot,i=0; ptcRuleTemp!=NULL; ptcRuleTemp=ptcRuleTemp->next,i++ )
		{
				fprintf( stderr, "ptcRuleTemp = %x\n", (unsigned int)ptcRuleTemp );
				fprintf( cgiOut, "<tr align=left bgcolor=%s>\n", setclour((i+1)%2) );
				
				fprintf( cgiOut, "<td><div style='cursor:hand;background-color:#eeeeff;align:middle;' onclick='detailShowHide_rule_%d(this)'>+</div></td>", i );//展开详情
				
				{//位置参数
					fprintf( cgiOut, "<td>\n");
					ruleIndexSelector( i, ruleNumTotal, encry );
					fprintf( cgiOut, "</td>\n" );
				}
				if( strcmp( ptcRuleTemp->addrtype, "address" ) == 0 )
				{//单一地址
					fprintf( cgiOut, "<td>%s</td>\n", ptcRuleTemp->addr_begin );
				}
				else
				{
					fprintf( cgiOut, "<td>%s/%s</td>\n", ptcRuleTemp->addr_begin, ptcRuleTemp->addr_end );
				}
				
				fprintf( cgiOut, "<td>%s:%s;%s:%s;</td>\n", search(lfirewall,"tc_upload"), ptcRuleTemp->uplink_speed, search(lfirewall,"tc_download"), ptcRuleTemp->downlink_speed );

				if( ptcRuleTemp->useP2P == 1 )
				{
					fprintf( cgiOut, "<td>%s</td>\n", search(lfirewall,"tc_forbid_p2p") );
				}
				else
				{
					fprintf( cgiOut, "<td> </td>\n"  );
				}
				fprintf( cgiOut, "<td>%s</td>",ptcRuleTemp->enable?search(lfirewall,"rule_enabled"):search(lfirewall,"rule_disabled") );
				fprintf( cgiOut, "<td>");
				if( 0 == manager )
				{
					fprintf( cgiOut, "<script type=text/javascript>\n" );
					fprintf( cgiOut, "var popMenu%d = new popMenu('popMenu%d','%d');\n", i, i, tcRuleNum-i );
					fprintf( cgiOut, "popMenu%d.addItem( new popMenuItem( '%s', 'wp_tcruleedit.cgi?UN=%s&editType=edit&ruleNum=%d' ) );\n", i,search(lfirewall, "rule_edit" ), encry, i );
					fprintf( cgiOut, "popMenu%d.addItem( new popMenuItem( '%s', 'wp_tcrulemodify.cgi?UN=%s&editType=delete&ruleNum=%d' ) );\n", i, search(lfirewall, "rule_delete" ), encry, i );
					fprintf( cgiOut, "popMenu%d.show();\n", i );
					fprintf( cgiOut, "</script>" );
				}
        		fprintf( cgiOut, "</td><td></td>\n" );
        	fprintf( cgiOut, "</tr>\n" );
        	fprintf( cgiOut, "<tr height=1><td colspan=7><div id='div_rule_%d' style=\"position:relative; display:block\"></div></td></tr>", i );
            fprintf( cgiOut, "  <script type='text/javascript'>\n"
                             "      function detailShowHide_rule_%d( obj )\n"\
                             "      {\n"\
                             "          var divobj=document.getElementById('div_rule_%d');\n", i, i );
            fprintf( cgiOut, "          var content='<table><tr><td width=100></td><td>%s</td><td>%s</td></tr>'+\n", "Uplink Interface:",ptcRuleTemp->up_interface );
            fprintf( cgiOut, "                             '<tr><td></td><td>%s</td><td>%s</td></tr>'+\n", "Downlink Interface:", ptcRuleTemp->interface );
            
            fprintf( cgiOut, "                             '<tr><td></td><td>%s</td><td>%s%s%s'+\n", "IP:", 
                                                                                ptcRuleTemp->addr_begin,
                                                                                (strcmp( ptcRuleTemp->addrtype, "address" ) != 0)?"/":"",
                                                                                (strcmp( ptcRuleTemp->addrtype, "address" ) != 0)?ptcRuleTemp->addr_end:""  );
{			
			char *mode=NULL;
			if( NULL != ptcRuleTemp->mode )
			{
					mode = (strcmp(ptcRuleTemp->mode,"notshare")==0)?"mode:non-share":"mode:share";
			}
			fprintf( cgiOut, "								';  %s</td></tr>'+\n",
							(mode==NULL)?"":mode );
}
            fprintf( cgiOut, "                             '<tr><td></td><td>%s</td><td>%s</td></tr>'+\n", "Uplink Speed:", ptcRuleTemp->uplink_speed );
            fprintf( cgiOut, "                             '<tr><td></td><td>%s</td><td>%s</td></tr>'+\n", "Downlink Speed:", ptcRuleTemp->downlink_speed );
            fprintf( cgiOut, "                             '<tr><td></td><td>%s</td><td>%s</td></tr>'+\n", "P2P:", (0==ptcRuleTemp->useP2P)?"Permit":"Forbid" );
            fprintf( cgiOut, "                             '</table>';\n" );
            fprintf( cgiOut, "          if( '+' == obj.innerHTML )\n"\
                             "          {\n"\
                             "              divobj.innerHTML = content;\n"\
                             "              obj.innerHTML='-';\n"\
                             "              divobj.parentNode.parentNode.height=20*6;\n"\
                             "          }\n"\
                             "          else\n"\
                             "          {\n"\
                             "              divobj.innerHTML = '';\n"\
                             "              obj.innerHTML='+';\n"\
                             "              divobj.parentNode.parentNode.height=1;\n"\
                             "          }\n"\
                             "      }\n"\
                             "  </script>\n" );        	
		}

		
		fprintf( cgiOut, "</table>\n" );
		//加一行,否则弹出popmenu时会显示滚动条
		fprintf( cgiOut, "<table ><tr height=25><td></td></tr></table>\n");
	}
	
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
	fprintf( cgiOut, "  autoCheckHeight();\n" );
	fprintf( cgiOut, "</script> \n" );
    fprintf( cgiOut, " \n" );
	
	fprintf( cgiOut, "</html> \n" );
 

	free(encry);
	release(lpublic); 
	release(lfirewall); 
	
	tcFreeList( ptcRuleRoot );
	return 0;	
}

/***************************************************************
*Useage:    生成顺序选择部分的代码，包含了其中的页面调整
*Param:		ruleNum -> 当前的规则的序号
            ruleTotalNum -> 规则总数
            encry -> 加密字符串，生成跳转的url是要用。
*Return:	0 -> success
*			others -> failure
*Auther:    shao jun wu
*Date:      2008-11-18
*Modify:    (include modifyer,for what resease,date)
****************************************************************/
static int ruleIndexSelector( int ruleNum, int ruleTotalNum, char *encry )
{
    int i;

    fprintf( cgiOut, "<select onchange=\"var url='wp_tcrulemodify.cgi?UN='+'%s'"\
                                "+'&editType=changeindex'+'&oldIndex=%d'+'&newIndex='+this.options[this.selectedIndex].value;window.open(url,'mainFrame');\">", encry, ruleNum );

    for( i=0; i<ruleTotalNum; i++ )
    {
        if( i==ruleNum )
        { 
            fprintf( cgiOut, "<option value='%d' selected>%d</option>", i, i+1 );
        }
        else
        {
            fprintf( cgiOut, "<option value='%d'>%d</option>", i, i+1 );
        }
    }
    fprintf( cgiOut, "</select>" );
   
   return 0;
}


