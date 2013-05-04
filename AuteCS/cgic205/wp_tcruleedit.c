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
* wp_tcruleedit.c
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

static int showInterfaceSelector( char *selectName, char *onchangestr );


int cgiMain()
{
//		char ruleType[10]="";
		char editType[10]="";
		char ruleNum[10]="";
		int	ruleNuma,ruleNumBak=0;
		int ruleTotalNum = 0;
		int i;
		char *encry=(char *)malloc(BUF_LEN);
		char *str;
		struct list *lpublic;
		struct list *lfirewall;
		char lan[3]; 
		FILE *fp1;
		PTCRule ptcRoot = NULL;
		PTCRule ptcCurRule=NULL;
		
		
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
	 	if((fp1=fopen("../htdocs/text/public.txt","r"))==NULL)		   /*以只读方式打开资源文件*/
	 	{
			ShowAlert(search(lpublic,"error_open"));
			return 0;
		}		
		fseek(fp1,4,0);						  /*将文件指针移到离文件首4个字节处，即lan=之后*/
		fgets(lan,3,fp1); 	 
		fclose(fp1);     
		
		ptcRoot = tcParseDoc( TCRULES_XML_FILE );     
		
//获得规则总数
			//计算总数
		ptcCurRule = ptcRoot;
		for( ruleTotalNum=0; ptcCurRule != NULL; ptcCurRule = ptcCurRule->next, ruleTotalNum++ );		          

//获得当前修改状态：编辑、添加，如果是编辑，需要将之前的信息读取进来
		cgiFormStringNoNewlines( "editType", editType, sizeof(editType) );
		if( strcmp(editType,"edit") == 0 )//得到当前编辑规则的序号
		{
			cgiFormStringNoNewlines( "ruleNum", ruleNum, sizeof(ruleNum) );
			sscanf( ruleNum, "%d", (int *)&ruleNuma );
			//找到这个规则，后面将其类容添加到控件中
			
			ptcCurRule = ptcRoot;
			ruleNumBak = ruleNuma;
			for( ; ((ruleNuma > 0)&&(ptcCurRule != NULL)); ptcCurRule = ptcCurRule->next, ruleNuma-- );
		}

		fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
		fprintf( cgiOut, "<head> \n" );
		fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
	  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
	  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
	  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");
		
		fprintf( cgiOut, "<title>Add firewall rule</title> \n" );
		fprintf( cgiOut, "<script language='javascript' type='text/javascript' src='/fw.js'></script> \n" );
		//fprintf( cgiOut, "<script language='javascript' type='text/javascript' src='./jquery-1.2.6.pack.js'></script> \n" );
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
		fprintf( cgiOut, "<body onload='on_body_resize();' onresize='on_body_resize();'> \n" );
		fprintf( cgiOut, "<form action='wp_tcrulemodify.cgi' method='post' onSubmit='return isAllLegal(this)'> \n" );
		
		fprintf( cgiOut, "<input type='hidden' name='UN' value='%s'>", encry );
		fprintf( cgiOut, "<input type='hidden' name='editType' value='%s' />", editType );

			
		fprintf( cgiOut, "<div align=center> \n" );
		fprintf( cgiOut, "<table width=976 border=0 cellpadding=0 cellspacing=0> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td> \n" );
		fprintf( cgiOut, "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td> \n" );
		fprintf( cgiOut, "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td> \n", search(lpublic,"title_style"), search(lfirewall,"title_tc"));
		fprintf( cgiOut, "<td width=690 align=right valign=bottom background=/images/di22.jpg><table width=130 border=0 cellspacing=0 cellpadding=0> \n" );
		fprintf( cgiOut, "<tr> \n" );
		
		{
			fprintf( cgiOut, "<td width=62 align=center><input id='but' type='submit' name='submit_addtcrule' style='background-image:url(/images/%s)' value=''></td>\n", search(lpublic,"img_ok" ) );
			fprintf( cgiOut, "<td width=62 align=left><a href=wp_tcruleview.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td> \n", encry, search(lpublic,"img_cancel" ) );
		}
		
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "</table></td> \n" );
		fprintf( cgiOut, "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td> \n" );
		fprintf( cgiOut, "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0> \n" );
		fprintf( cgiOut, "<tr height=4 valign=bottom> \n" );
		fprintf( cgiOut, "<td width=120>&nbsp;</td> \n" );
		fprintf( cgiOut, "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr> \n" );
		fprintf( cgiOut, "<td><table width=120 border=0 cellspacing=0 cellpadding=0> \n" );
		fprintf( cgiOut, "<tr height=25> \n" );
		fprintf( cgiOut, "<td id=tdleft>&nbsp;</td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr height=26> \n" );
		fprintf( cgiOut, "<td align=left id=tdleft><a href=wp_tcruleview.cgi?UN=%s><font id=%s>%s</font></a></td>\n", encry, search(lpublic,"menu_san"), search(lfirewall,"title_tc"));
		fprintf( cgiOut, "</tr> \n" );		
		fprintf( cgiOut, "<tr height=26> \n" );
		if( strcmp( editType, "add" ) == 0 ){
			fprintf( cgiOut, "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>\n", search(lpublic,"menu_san"), search(lfirewall,"tc_addrule"));
		}
		else{
			fprintf( cgiOut, "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>\n", search(lpublic,"menu_san"), search(lfirewall,"ruleedit_title"));	
		}
			
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<tr height=25> \n" );
		fprintf( cgiOut, "<td align=left id=tdleft>&nbsp;</td> \n" );
		fprintf( cgiOut, "</tr> \n" );

		fprintf( cgiOut, "<tr id=height_control> \n" );
		fprintf( cgiOut, "<td id=tdleft>&nbsp;</td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "</table></td> \n" );
		fprintf( cgiOut, "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\"><div id=\"rules\" align=\"left\" style=\"width:768px;overflow:auto\"> \n" );
		fprintf( cgiOut, "<div id='content_container' style=\"width:768px;overflow:auto\" > \n" );
		fprintf( cgiOut, "	<script type='text/javascript'>\n"\
						"	function on_body_resize()\n"\
						"	{\n"\
						"		var height_ctrl_obj = document.getElementById( 'height_control' );\n"\
						"		var content_container = document.getElementById( 'content_container' );\n"\
						"\n"\
						"		height_ctrl_obj.style.height = content_container.offsetHeight - 60;\n"\
						"	}\n"\
						"	</script>\n" );
		
		fprintf( cgiOut, "<table border='0' style='border-collapse:collapse; line-height: 20px;'> \n" );
		
{//shao jun wu   2008-11-17 14:37:22  //添加一个对规则顺序的选择
		fprintf( cgiOut, "<tr class=even><td width=100>%s</td><td>\n",search(lfirewall,"rule_position"));
		if( strcmp(editType,"edit") == 0 && NULL != ptcCurRule )//如果是选择的edit进入的界面就不能让其选择其他的编号。
		{
			fprintf( cgiOut, "<input type=hidden name='ruleNum' value='%d' />\n", ruleNumBak );
			fprintf( cgiOut, "<select name='ruleNum_selector' ><option value='%d'>%d</option> </select>", ruleNumBak, ruleNumBak+1 );
		}
		else
		{
			//得到当前的规则总数，因为selecter中要用，使用一个hidden的input来保存当前用户选择的顺序，
			fprintf( cgiOut, "	<input type=hidden name='ruleNum' value='%d' />\n", ruleTotalNum+1 );
			fprintf( cgiOut, "	<select name='ruleNum_selector' onchange='document.getElementsByName(\"ruleNum\")[0].value=this.options[this.selectedIndex].value;' > \n" );
			for( i=0; i<ruleTotalNum; i++ )
			{
				fprintf( cgiOut, "		<option value='%d'>%d</option>\n", i, i+1 );
			}
			fprintf( cgiOut, "		<option value='%d' selected>%d</option>\n", i, i+1 );
			fprintf( cgiOut, "	</selcet>\n");			
		}
		fprintf( cgiOut, "</td></tr>\n" );
		
}		
		fprintf( cgiOut, "<tr class=odd><td width=100 >%s</td><td width=400 ><input type=checkbox name=nouse /><label for=nouse>%s</label><br />%s</td></tr> \n", search(lfirewall,"tc_state"), search(lfirewall,"tc_nouse"), search(lfirewall,"tc_nouse_detail") );
		//
		fprintf( cgiOut, "<tr class=even><td>%s</td><td>\n", search(lfirewall,"tc_interface") );
		showInterfaceSelector( "interfaceSelector", "interfaceSelectorOnchange(\"interfaceSelector\")" );
		fprintf( cgiOut, "<br />%s</td></tr>\n", search(lfirewall, "tc_if_exp") );
		
		fprintf( cgiOut, "<tr class=odd><td>%s</td><td>\n", search(lfirewall,"tc_up_interface") );
		showInterfaceSelector( "up_interfaceSelector", "interfaceSelectorOnchange(\"up_interfaceSelector\")" );
		fprintf( cgiOut, "<br />%s%s</td></tr>\n", search(lfirewall,"tc_up_if_exp"), search(lfirewall,"tc_up_ip_exp2") );		
		
		fprintf( cgiOut, "<input type=hidden name=interfaceSelectorValue value=''>\n" );
		fprintf( cgiOut, "<input type=hidden name=up_interfaceSelectorValue value=''>\n" );
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "function interfaceSelectorOnchange( name ){\n" );
		fprintf( cgiOut, "		document.getElementsByName( name+'Value' )[0].value = document.getElementsByName( name )[0].value;\n" );
		fprintf( cgiOut, "}\n" );
		fprintf( cgiOut, "</script>\n" );
	
		fprintf( cgiOut, "<tr class=even><td>%s</td><td>%s: \n", search(lfirewall,"tc_ipaddress"), search(lfirewall,"tc_ipaddress_type") );
		fprintf( cgiOut, "<select name=addrtype_select onchange='addrtype_onchange(this);'> \n" );
		fprintf( cgiOut, "<option value=address selected>%s</option> \n", search(lfirewall,"tc_ipaddress_type_single") );
		fprintf( cgiOut, "<option value=addrrange>%s</option> \n", search(lfirewall,"tc_ipaddress_type_range") );
		fprintf( cgiOut, "</select><div name=addr_info_detail id=addr_info_detail ></div> \n" );
		fprintf( cgiOut, "</td> \n" );
		fprintf( cgiOut, "</tr> \n" );
		fprintf( cgiOut, "<script type='text/javascript'> \n" );
		fprintf( cgiOut, "function addrtype_onchange( obj ){ \n" );
		fprintf( cgiOut, "var addr_info = '%s '; \n", search(lfirewall,"tc_ipaddress_label") );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "if( 'address' == obj.value ) \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "addr_info += '<input type=text name=address_single size=16>'; \n" );
		fprintf( cgiOut, "addr_info += '<br />%s:192.168.1.123'; \n", search(lfirewall,"tc_ipaddress_eg") );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "else if( 'addrrange' == obj.value ) \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "addr_info += '<input type=text name=address_range_begin size=16 />'; \n" );
		fprintf( cgiOut, "addr_info += '%s'; \n", search( lfirewall, "tc_mask" ) );
		fprintf( cgiOut, "addr_info += '<input type=text name=address_range_end size=16 /><br />'; \n" );//address_range_end实际是掩码
		//添加了一个模式选择，子网应用时，是共享带宽，还是独立带宽
		fprintf( cgiOut, "addr_info += '%s:<input type=radio name=range_mode value=share checked/>%s  <input type=radio name=range_mode value=notshare />%s';",search( lfirewall, "tc_mode" ) ,search( lfirewall, "tc_mode_share" ),search( lfirewall, "tc_mode_notshare" ) );
		fprintf( cgiOut, "addr_info += '<br />%s 192.168.1.12/255.255.255.0<br />'; \n", search(lfirewall,"tc_ipaddress_eg") );
		fprintf( cgiOut, "addr_info += '%s'\n",search( lfirewall, "tc_mode_notify" ) );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "document.getElementById('addr_info_detail').innerHTML = addr_info; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "addrtype_onchange( document.getElementsByName('addrtype_select')[0] ); \n" );
		fprintf( cgiOut, "</script> \n" );

#if USE_PROTOCOL_CONTROL		
		fprintf( cgiOut, "<tr class=even><td>%s</td><td>", search(lfirewall,"tc_protocol")  );
		fprintf( cgiOut, "<input type=checkbox name=protocol value=tcp /><label>TCP</label>\n" );
		fprintf( cgiOut, "<input type=checkbox name=protocol value=udp /><label>UDP</label>\n" );
		fprintf( cgiOut, "<input type=checkbox name=protocol value=icmp /><label>ICMP</label>\n" );
		fprintf( cgiOut, "</td></tr>\n" );
#endif		
		fprintf( cgiOut, "<tr class=ood><td>%s</td><td> \n", search(lfirewall,"tc_traffic") );
		fprintf( cgiOut, "%s: <input type=text size=6 maxLength=4 name=uplink_bandwidth /> Kbyte<br /> \n", search(lfirewall,"tc_upload") );
		fprintf( cgiOut, "%s: <input type=text size=6 maxLength=4 name=downlink_bandwidth /> Kbyte<br /> \n", search(lfirewall,"tc_download") );
		fprintf( cgiOut, "%s \n", search(lfirewall,"tc_traffic_exp") );
		fprintf( cgiOut, "</td> \n" );
		fprintf( cgiOut, "</tr> \n" );

		fprintf( cgiOut, "<tr class=even>\n" );
		
#if USE_P2P_TRAFFIC_CTROL
		fprintf( cgiOut, "<td>P2P%s</td> \n", search(lfirewall,"tc_traffic") );
		fprintf( cgiOut, "<td><input type=checkbox name=usep2p onclick='checkp2pstate(this)' /><label for='usep2p'>%s</label><br /> \n", search(lfirewall,"tc_p2p_nouse") );
		fprintf( cgiOut, "%s: <input type=text size=8 name=p2p_uplink_bandwidth /> Kbyte<br /> \n", search(lfirewall,"tc_upload") );
		fprintf( cgiOut, "%s: <input type=text size=8 name=p2p_downlink_bandwidth /> Kbyte<br /> \n", search(lfirewall,"tc_download") );
		fprintf( cgiOut, "%s\n", search(lfirewall,"tc_traffic_exp") );
		fprintf( cgiOut, "</td> \n" );
		fprintf( cgiOut, "<script type='text/javascript'> \n" );
		fprintf( cgiOut, "function checkp2pstate( obj ){ \n" );
		fprintf( cgiOut, "var a=document.getElementsByName( 'p2p_uplink_bandwidth' )[0]; \n" );
		fprintf( cgiOut, "var b=document.getElementsByName( 'p2p_downlink_bandwidth' )[0]; \n" );
		fprintf( cgiOut, "if( obj.checked == true ){ \n" );
		fprintf( cgiOut, "a.disabled = true; \n" );
		fprintf( cgiOut, "b.disabled = true; \n" );
		fprintf( cgiOut, "a.style.backgroundColor = '#ccc'; \n" );
		fprintf( cgiOut, "b.style.backgroundColor = '#ccc'; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "else \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "a.disabled = false; \n" );
		fprintf( cgiOut, "b.disabled = false; \n" );
		fprintf( cgiOut, "a.style.backgroundColor = '#fff'; \n" );
		fprintf( cgiOut, "b.style.backgroundColor = '#fff'; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "checkp2pstate( document.getElementsByName('usep2p')[0] ); \n" );
		fprintf( cgiOut, "</script> \n" );
#else//p2p流量控制底层模块还有问题，现在不让设置上下行带宽，只能直接屏蔽p2p连接
		fprintf( cgiOut, "<td>%s</td> \n", search(lfirewall, "tc_p2p_control" ) );
		
		fprintf( cgiOut, "<td><input type=checkbox name=usep2p><label for='usep2p'>%s</label>\n", search(lfirewall,"tc_p2p_forbid") );
		fprintf( cgiOut, "<input type=hidden size=8 name=p2p_uplink_bandwidth value=0/>\n" );
		fprintf( cgiOut, "<input type=hidden size=8 name=p2p_downlink_bandwidth value=0/></td>\n" );
#endif
		fprintf( cgiOut, "</tr> \n" );

#if USE_RULE_TIME_CTROL		
		fprintf( cgiOut, "<tr class=even><td>%s</td><td>%s<br />%s<select name=time_from></select>%s<select name=time_to></select><br />%s<br />%s</td></tr> \n", search(lfirewall,"tc_time_control"), search(lfirewall,"tc_time_use"), search(lfirewall,"tc_time_from"), search(lfirewall,"tc_time_to"),  search(lfirewall,"tc_time_overday", search(lfirewall,"tc_time_same") );
		fprintf( cgiOut, "<script type=text/javascript> \n" );
		fprintf( cgiOut, "function setTimeSelectOption( obj ){ \n" );
		fprintf( cgiOut, "for( var i=0; i < 48; i++ ){ \n" );
		fprintf( cgiOut, "var opt=new Option(); \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, "opt.value = i; \n" );
		fprintf( cgiOut, "if( i%%2 == 0 ) \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "opt.text = ''+i/2+':00'; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "else \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "opt.text = '' + Math.floor(i/2) +':30'; \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "try \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "obj.add( opt, null ); \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "catch(ex) \n" );
		fprintf( cgiOut, "{ \n" );
		fprintf( cgiOut, "obj.add( opt );// IE only \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "} \n" );
		fprintf( cgiOut, "setTimeSelectOption( document.getElementsByName('time_from')[0] ); \n" );
		fprintf( cgiOut, "setTimeSelectOption( document.getElementsByName('time_to')[0] ); \n" );
		fprintf( cgiOut, "</script> \n" );
#endif		
		fprintf( cgiOut, "</table> \n" );
		fprintf( cgiOut, "</div> \n" );
		fprintf( cgiOut, "</td> \n" );
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
		
		fprintf( cgiOut, "</body> \n" );
		if( NULL != ptcCurRule )
		{
			fprintf( cgiOut, "<script type='text/javascript'>\n");
			//有效？
			fprintf( cgiOut, "//ptcCurRule->enable = %d\n", ptcCurRule->enable );
			if( ptcCurRule->enable == 0 ){
				fprintf( cgiOut, "document.getElementsByName( 'nouse' )[0].checked = true;\n");
			}
			//if( strcmp( ptcCurRule->addrtype, a )
			//地址
			fprintf( cgiOut, "//ptcCurRule->addrtype = %s\n", ptcCurRule->addrtype );
			fprintf( cgiOut, "document.getElementsByName('addrtype_select')[0].value = '%s';\n", ptcCurRule->addrtype );
			
			if( strcmp( ptcCurRule->addrtype, "address" ) == 0  )//单个地址
			{
				if( NULL !=  ptcCurRule->addr_begin )
				{
					fprintf( cgiOut, "//ptcCurRule->addr_begin = %s\n", ptcCurRule->addr_begin );
					fprintf( cgiOut, "document.getElementsByName('address_single')[0].value = '%s';\n", ptcCurRule->addr_begin );
				}
			}
			else if( strcmp( ptcCurRule->addrtype, "addrrange" ) == 0 )
			{
				fprintf( cgiOut, "addrtype_onchange( document.getElementsByName('addrtype_select')[0] ); \n" );
				if( NULL !=  ptcCurRule->addr_begin )
				{
					fprintf( cgiOut, "//ptcCurRule->addr_begin = %s\n", ptcCurRule->addr_begin );
					fprintf( cgiOut, "document.getElementsByName('address_range_begin')[0].value='%s';\n", ptcCurRule->addr_begin );
				}
				if( NULL !=  ptcCurRule->addr_end )
				{
					fprintf( cgiOut, "//ptcCurRule->addr_end = %s\n", ptcCurRule->addr_end );
					fprintf( cgiOut, "document.getElementsByName('address_range_end')[0].value='%s';\n", ptcCurRule->addr_end );
					fprintf( cgiOut, "//ptcCurRule->mode = %s\n", ptcCurRule->mode );
					fprintf( cgiOut, "if( '%s' == 'notshare' ){\n",ptcCurRule->mode );
					//fprintf( cgiOut, "alert('111111');\n" );
					fprintf( cgiOut, "	document.getElementsByName('range_mode')[1].checked=true;\n" );
					fprintf( cgiOut, "}else{\n" );
					fprintf( cgiOut, "	document.getElementsByName('range_mode')[0].checked=true;\n" );
					fprintf( cgiOut, "}\n" );
				}
			}
			//流量
			if( NULL != ptcCurRule->uplink_speed )
			{
				fprintf( cgiOut, "//ptcCurRule->uplink_speed = %s\n", ptcCurRule->uplink_speed );
				fprintf( cgiOut, "document.getElementsByName('uplink_bandwidth')[0].value='%s';\n", ptcCurRule->uplink_speed );
			}
			if( NULL != ptcCurRule->downlink_speed )
			{
				fprintf( cgiOut, "//ptcCurRule->downlink_speed = %s\n", ptcCurRule->downlink_speed );
				fprintf( cgiOut, "document.getElementsByName('downlink_bandwidth')[0].value='%s';\n", ptcCurRule->downlink_speed );
			}
			
			//p2p流量
			fprintf( cgiOut, "//ptcCurRule->useP2P = %d\n", ptcCurRule->useP2P );
			if( ptcCurRule->useP2P == 1 )
			{
				fprintf( cgiOut, "document.getElementsByName('usep2p')[0].checked=true;\n" );	
			}
			else
			{
				fprintf( cgiOut, "document.getElementsByName('usep2p')[0].checked=false;\n" );
#if USE_P2P_TRAFFIC_CTROL				
				if( NULL != ptcCurRule->p2p_uplink_speed )
				{
					fprintf( cgiOut, "//ptcCurRule->p2p_uplink_speed = %s\n", ptcCurRule->p2p_uplink_speed );
					fprintf( cgiOut, "document.getElementsByName('p2p_uplink_bandwidth')[0].value = '%s';\n", ptcCurRule->p2p_uplink_speed );
				}
				if( NULL != ptcCurRule->p2p_downlink_speed )
				{
					fprintf( cgiOut, "//ptcCurRule->p2p_downlink_speed = %s\n", ptcCurRule->p2p_downlink_speed );
					fprintf( cgiOut, "document.getElementsByName('p2p_downlink_bandwidth')[0].value = '%s';\n", ptcCurRule->p2p_downlink_speed );	
				}	
#endif				
			}
#if USE_P2P_TRAFFIC_CTROL			
			//这里修改了状态，需要确定是否使能p2p上的输入框
			fprintf( cgiOut, "checkp2pstate( document.getElementsByName('usep2p')[0] ); \n" );
#endif			

#if USE_RULE_TIME_CTROL			
			//时间设置
			fprintf( cgiOut, "//ptcCurRule->time_begin = %d\n", ptcCurRule->time_begin );
			fprintf( cgiOut, "document.getElementsByName('time_from')[0].value = '%d';\n", ptcCurRule->time_begin );
			fprintf( cgiOut, "//ptcCurRule->time_end = %d\n", ptcCurRule->time_end );
			fprintf( cgiOut, "document.getElementsByName('time_to')[0].value = '%d';\n", ptcCurRule->time_end );
#endif			
			//interface
			fprintf( cgiOut, "//ptcCurRule->interface = %s\n", ptcCurRule->interface );
			fprintf( cgiOut, "document.getElementsByName('interfaceSelector')[0].value = '%s';\n", ptcCurRule->interface );
			//up_interface
			fprintf( cgiOut, "//ptcCurRule->up_interface = %s\n", ptcCurRule->up_interface );
			fprintf( cgiOut, "document.getElementsByName('up_interfaceSelector')[0].value = '%s';\n", ptcCurRule->up_interface );			
			
			//protocol
			fprintf( cgiOut, "//ptcCurRule->protocol = %s\n", ptcCurRule->protocol );
#if 1
			if( (NULL != ptcCurRule->protocol) && (strstr( ptcCurRule->protocol, "tcp" ) != NULL) )
			{
				fprintf( cgiOut, "document.getElementsByName('protocol')[0].checked = true;\n" );
			}
			if( (NULL != ptcCurRule->protocol) && (strstr( ptcCurRule->protocol, "udp" ) != NULL) )
			{
				fprintf( cgiOut, "document.getElementsByName('protocol')[1].checked = true;\n" );
			}
			if( (NULL != ptcCurRule->protocol) && (strstr( ptcCurRule->protocol, "icmp" ) != NULL) )
			{
				fprintf( cgiOut, "document.getElementsByName('protocol')[2].checked = true;\n" );
			}			
#endif			
			fprintf( cgiOut, "</script>\n");
		}
		
		fprintf( cgiOut, "<script type='text/javascript'>\n" );
		fprintf( cgiOut, "function isAllLegal( thisform )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "	var ret = true;\n");
		//地址设置的监测
		fprintf( cgiOut, "var addrtype = document.getElementsByName('addrtype_select')[0].value;\n" );
		fprintf( cgiOut, "if( addrtype == 'address' )\n" );
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "	if( isLegalIpAddr( document.getElementsByName( 'address_single' )[0].value ) == false )\n" );
		fprintf( cgiOut, "	{\n" );
		fprintf( cgiOut, "		alert( '%s' );\n", search(lfirewall,"tc_err_ip") );
		fprintf( cgiOut, "		ret = false;\n" );
		fprintf( cgiOut, "	}\n");
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut ," else if( addrtype == 'addrrange' )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "  if( isLegalIpAddr( document.getElementsByName( 'address_range_begin' )[0].value ) == false || \n" );
		fprintf( cgiOut, "		isLegalMask( document.getElementsByName( 'address_range_end' )[0].value ) == false )\n" );
		fprintf( cgiOut, "	{\n" );
		fprintf( cgiOut, "		alert( '%s' );\n", search(lfirewall, "tc_err_ip"));
		fprintf( cgiOut, "		ret = false;\n");
		fprintf( cgiOut, "	}\n");
		fprintf( cgiOut, "	else\n"\
						 "	{\n"\
						 "		notshare_checked=document.getElementsByName('range_mode')[1].checked;"\
						 "		mask=document.getElementsByName( 'address_range_end' )[0].value;\n"\
						 "		re=/^\\s*(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\s*$/;\n"\
						 "		if( notshare_checked  && re.test(mask))\n"\
						 "		{\n"\
						 "			if( RegExp.$1 != 255 || RegExp.$2 != 255 || RegExp.$3 != 255  )\n"\
						 "			{\n"\
						 "				alert( '%s' );\n"\
						 "				ret = false;\n"\
						 "			}\n"\
						 "		}\n"\
						 "	}\n", search(lfirewall,"tc_mode_mask_spec") );
		fprintf( cgiOut, "}\n");

		//流量设置的监测
		fprintf( cgiOut, " re = /^\\s*(\\d{1,4})\\s*$/;\n");
		fprintf( cgiOut, "if( re.test( document.getElementsByName( 'uplink_bandwidth' )[0].value ) == false ||\n");
		fprintf( cgiOut, "	re.test( document.getElementsByName( 'downlink_bandwidth' )[0].value ) == false )\n");
		fprintf( cgiOut, "{\n");
		fprintf( cgiOut, "	alert( '%s' );\n", search(lfirewall,"tc_err_flow"));
		fprintf( cgiOut, "	ret = false;\n");
		fprintf( cgiOut, "}\n");
#if USE_P2P_TRAFFIC_CTROL		
		//p2p流量设置的监测
		fprintf( cgiOut, "if( document.getElementsByName( 'usep2p' )[0].checked == false )\n");
		fprintf( cgiOut, "{\n" );
		fprintf( cgiOut, "if( re.test( document.getElementsByName( 'p2p_uplink_bandwidth' )[0].value ) == false ||\n");
		fprintf( cgiOut, "	re.test( document.getElementsByName( 'p2p_downlink_bandwidth' )[0].value ) == false )\n");
		fprintf( cgiOut, "{\n" );
		fprintf( cgiOut, "	alert( '%s' );\n", search(lfirewall,"tc_err_p2p_flow") );
		fprintf( cgiOut, "	ret = false;\n");
		fprintf( cgiOut, "}	\n" );
		fprintf( cgiOut, "}\n");
#endif		
		//
		fprintf( cgiOut, "return ret;\n" );
		fprintf( cgiOut, "}\n");
		fprintf( cgiOut, "interfaceSelectorOnchange( 'interfaceSelector' );\n" );
		fprintf( cgiOut, "interfaceSelectorOnchange( 'up_interfaceSelector' );\n" );
		fprintf( cgiOut, "</script>\n" ); 
		
		fprintf( cgiOut, "</html> \n" );
		fprintf( cgiOut, " \n" );
		fprintf( cgiOut, " \n" );
		
		tcFreeList( ptcRoot );
		
		return 0;
}
		

static int showInterfaceSelector( char *selectName, char *onchangestr )
{
	FILE *fp;
	char *cmd = "ip link | sed \"/.*link\\/.*brd.*/d\" | sed \"s/^[0-9]*:.\\(.*\\):.*$/\\1/g\" | sed \"/lo/d\" | sed \"s/^\\(.*\\)$/<option value='\\1'>\\1<\\/option>/g\"";
	char buff[256];
	
	fp = popen( cmd, "r" );
	if( NULL == fp )
	{
		return -1;	
	}
	
	fprintf( cgiOut, "<select name=%s onchange='%s'>", selectName, onchangestr );
	
	fgets( buff, sizeof(buff), fp );
	do//最后一行老是会多读一次，采用do  while方式可以把最后多读的一行去掉～
	{
		fprintf( cgiOut, "%s", buff );
		fgets( buff, sizeof(buff), fp );
	}while( !feof(fp) );
	pclose(fp);
	fprintf( cgiOut, "</select>" );
	
	return 0;	
}



