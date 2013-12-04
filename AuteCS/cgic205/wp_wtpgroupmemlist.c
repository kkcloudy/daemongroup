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
* wp_wtplis.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* hupx@autelan.com
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
#include "ws_dcli_ap_group.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ws_dcli_wlans.h"


#define MAX_PAGE_NUM 25     /*每页显示的最多AP组成员个数*/

void ShowWtpGroupMemPage(char *m, char *t, struct list *lpublic, struct list *lwlan);    

int cgiMain()
{
	  char encry[BUF_LEN] = { 0 };  
	  char *str = NULL;      
	  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
	  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
	  lpublic=get_chain_head("../htdocs/text/public.txt");
	  lwlan=get_chain_head("../htdocs/text/wlan.txt");
	  
	  ccgi_dbus_init();
	  memset(encry,0,sizeof(encry));
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
	    	ShowErrorPage(search(lpublic,"ill_user"));
	  }/*用户非法*/
	  else
	  {   
		ShowWtpGroupMemPage(encry,str,lpublic,lwlan);	
	  }
	  release(lpublic);  
	  release(lwlan);
	  destroy_ccgi_dbus();
	  return 0;
}

void ShowWtpGroupMemPage(char *m,char *t, struct list *lpublic,struct list *lwlan)
{  
	char *endptr = NULL;	
	int i = 0,result = 1,retu = 1,cl = 1; 
	char select_insid[10] = { 0 };
	char groupID[5] = { 0 };
	char page_no[5] = { 0 }; 
	int n=0;
	int apgroupid=0;
	char groupname[WTP_AP_GROUP_NAME_MAX_LEN+5] = { 0 };
	char add_del[10] = { 0 };
	dbus_parameter ins_para;
	instance_parameter *paraHead1 = NULL;
	unsigned int *wtp_list = NULL;
	DCLI_WTP_API_GROUP_ONE *wtp_one = NULL;
	unsigned int apcount=0;
	int limit = 0,start_wtpno = 0,end_wtpno = 0,wtpno_page = 0,total_pnum = 0;    /*start_wtpno表示要显示的起始wtp id，end_wtpno表示要显示的结束wtp id，wtpno_page表示本页要显示的wtp数，total_pnum表示总页数*/

	memset(select_insid,0,sizeof(select_insid));
	memset(groupID,0,sizeof(groupID));
	memset(groupname,0,sizeof(groupname));
	cgiFormStringNoNewlines("INSTANCE_ID", select_insid, 10);
	cgiFormStringNoNewlines("groupID", groupID, 5);
	cgiFormStringNoNewlines("groupname", groupname, WTP_AP_GROUP_NAME_MAX_LEN+5);
	if(cgiFormStringNoNewlines("PN", page_no, 5)!=cgiFormNotFound )
	{
		n= strtoul(page_no,&endptr,10);    
	}
	else
		n=0;
	get_slotID_localID_instanceID(select_insid,&ins_para);	
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
	
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>Wtp</title>");
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
	fprintf(cgiOut,"<style>"\
		"#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
		"#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
		"#link{ text-decoration:none; font-size: 12px}"\
		"</style>"\
		"</head>"\
		  "<script type=\"text/javascript\">");
	fprintf(cgiOut,"function disable_s(str)"\
		"{"\
			"var memid=document.getElementsByName('mem_conte')[0];"\
			"if(str == \"all\")"\
			"{"\
				"memid.disabled=true;"\
			"}"\
			"else"\
			"{"\
				"memid.disabled=false;"\
			"}"\
		"}");
	
	fprintf(cgiOut,"function page_change(obj)"\
	     "{"\
	  	 "var page_num = obj.options[obj.selectedIndex].value;"\
	       "var url = 'wp_wtpgroupmemlist.cgi?UN=%s&groupID=%s&groupname=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
	       "window.location.href = url;"\
	      "}", m ,groupID,groupname, select_insid);
	  fprintf(cgiOut,"</script>"\
	  "<script src=/instanceid_onchange.js>"\
	  "</script>"\
	"<body>");	  

  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><a href=wp_wtpgrouplist.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,select_insid,search(lpublic,"img_ok"));
 //         			 "<td width=62 align=center><input id=but type=submit name=wtpgroup_mem style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtpgrouplist.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,select_insid,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");
				
      fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
      "<tr>"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"\
            "<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>"\
            "<tr>"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>"\
                   "<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");	
		      fprintf(cgiOut,"<tr height=25>"\
			    "<td align=left id=tdleft><a href=wp_wtplis.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));			
      			fprintf(cgiOut,"</tr>");

		  fprintf(cgiOut, "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                  fprintf(cgiOut,"</tr>");
		  retu=checkuser_group(t);
		  if(retu==0)  /*管理员*/
		  {
	                    fprintf(cgiOut,"<tr height=25>"\
	  					  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
	                    fprintf(cgiOut,"</tr>");
			    
			    fprintf(cgiOut,"<tr height=26>"\
	                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lwlan,"ap_group_mem_list"));   /*突出显示*/
	                  fprintf(cgiOut,"</tr>");
			    		    
	                    fprintf(cgiOut,"<tr height=25>"\
	  					  "<td align=left id=tdleft><a href=wp_wtpgroupnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"create_apgroup"));                       
	                    fprintf(cgiOut,"</tr>");
		}
		fprintf(cgiOut,"<tr height=25>"\
  				"<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
                fprintf(cgiOut,"</tr>");
		  if(retu==0)  /*管理员*/
		  {
			fprintf(cgiOut,"<tr height=25>"\
  				"<td align=left id=tdleft><a href=wp_wtpdown.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_upload"));                       
                    	fprintf(cgiOut,"</tr>");
		  }
		fprintf(cgiOut,"<tr height=25>"\
  			"<td align=left id=tdleft><a href=wp_verbind.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"version_bind"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
                  fprintf(cgiOut,"</tr>");
		  if(retu==0)  /*管理员*/
		  {
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                    fprintf(cgiOut,"</tr>");
		  }
				  fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                  fprintf(cgiOut,"</tr>");
		  if(retu==0) /*管理员*/
		  {
                    fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                    fprintf(cgiOut,"</tr>");
		  }

		  if(paraHead1)
		  {	  
			  apgroupid= strtoul(groupID,&endptr,10);
			  result=ccgi_show_group_member_cmd(paraHead1->parameter,paraHead1->connection,apgroupid,&wtp_list,&apcount);
		  } 
		total_pnum=((apcount%MAX_PAGE_NUM)==0)?(apcount/MAX_PAGE_NUM):((apcount/MAX_PAGE_NUM)+1);
		start_wtpno=n*MAX_PAGE_NUM;   
		end_wtpno=(((n+1)*MAX_PAGE_NUM)>apcount)?apcount:((n+1)*MAX_PAGE_NUM);
		wtpno_page=end_wtpno-start_wtpno;
		if((wtpno_page<(MAX_PAGE_NUM/2))||(apcount==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个wtp*/
		      	limit=2;
		else if((wtpno_page<MAX_PAGE_NUM)||(apcount==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个wtp*/
		  	limit=13;
		else	 /*大于30个翻页*/
		  	limit=14;
		if(retu==1)  /*普通用户*/
			limit+=4;
		for(i=0;i<limit;i++)
	        {
  			fprintf(cgiOut,"<tr height=25>"\
                      		"<td id=tdleft>&nbsp;</td>"\
                   		 "</tr>");
	        }
		  
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:10px; padding-top:10px\">"\
   "<table width=773 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>");

	
	fprintf(cgiOut,"<tr >"\
			   "<td width=140 align=left id=thead5>%s ID:%s</td>",search(lwlan,"ap_group"),groupID);
	fprintf(cgiOut,"<td width=50>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>");
	fprintf(cgiOut,"<td width=70>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>");
	fprintf(cgiOut,"<td width=70>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>");
	fprintf(cgiOut,"<td width=200  align=right id=thead5>%s ID:%s</td>",search(lpublic,"instance"),select_insid);
	fprintf(cgiOut,"</tr>");
	
	fprintf(cgiOut,"<tr height=20>");
	fprintf(cgiOut,"<td width=160>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>");
	fprintf(cgiOut,"</tr>");
		
	fprintf(cgiOut,"<tr align=left>"\
			   "<th width=140><font id=%s>AP%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
	fprintf(cgiOut,"<th width=50><font id=%s>AP ID</font></th>",search(lpublic,"menu_thead"));
	fprintf(cgiOut,"<th width=70><font id=%s>%s</font><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"run"),search(lpublic,"menu_thead"),search(lwlan,"state"));
	fprintf(cgiOut,"<th width=70><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"state"));
	fprintf(cgiOut,"<th width=200><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"location"));
	fprintf(cgiOut,"</tr>");

	if((result == 0)&&(apcount > 0)&&(NULL !=paraHead1))
	{
		char wtp_sta[WTP_ARRAY_NAME_LEN] = { 0 };  
		for(i=start_wtpno;i<end_wtpno;i++)
		{
			int apid=0;
			apid=wtp_list[i];
			result=show_wtp_one(paraHead1->parameter,paraHead1->connection,apid,&wtp_one);
			if((wtp_one)&&(wtp_one->WTP[0]))
			{
				
				memset(wtp_sta,0,sizeof(wtp_sta));
				CheckWTPState(wtp_sta,wtp_one->WTP[0]->WTPStat);

				
				fprintf(cgiOut,"<tr align=left>"\
						   "<td width=140>%s</td>",wtp_one->WTP[0]->WTPNAME);
				fprintf(cgiOut,"<td width=50>%d</td>",wtp_one->WTP[0]->WTPID);
				fprintf(cgiOut,"<td width=70>%s</td>",wtp_sta);
				if(wtp_one->WTP[0]->isused==1)
					fprintf(cgiOut,"<td width=70>%s</td>","used");
				else
					fprintf(cgiOut,"<td width=70>%s</td>","unused");
				fprintf(cgiOut,"<td width=200>%s</td>",wtp_one->WTP[0]->location);
				fprintf(cgiOut,"</tr>");
			}
		}
	}
	
	fprintf(cgiOut,"<tr>");
	  //"<td><input type=hidden name=UN value=%s></td>",m);
	  //fprintf(cgiOut,"<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
	   //fprintf(cgiOut,"<td ><input type=hidden name=groupID value=%s></td>",groupID);
	  // fprintf(cgiOut,"<td ><input type=hidden name=groupname value=%s></td>",groupname);
	fprintf(cgiOut,"</tr>");

	if(apcount>MAX_PAGE_NUM)		    /*大于30个wtp时，显示翻页的链接*/
	{
		fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(n!=0)
	        	fprintf(cgiOut,"<td align=left width=140><a href=wp_wtpgroupmemlist.cgi?UN=%s&groupID=%s&groupname=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,groupID,groupname,n-1,select_insid,search(lpublic,"up_page"));
		else
		  	fprintf(cgiOut,"<td width=140>&nbsp;</td>");
		
		fprintf(cgiOut,"<td width=50>&nbsp;</td>");
		fprintf(cgiOut,"<td align=center colspan=2>%s",search(lpublic,"jump_to_page1"));
			 fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this)>");
			 for(i=0;i<total_pnum;i++)
			 {
			   if(i==n)
				 fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
			   else
				 fprintf(cgiOut,"<option value=%d>%d",i,i+1);
			 }
			 fprintf(cgiOut,"</select>"\
			 "%s</td>",search(lpublic,"jump_to_page2"));
		if(n!=((apcount-1)/MAX_PAGE_NUM))
	      fprintf(cgiOut,"<td align=right width=100><a href=wp_wtpgroupmemlist.cgi?UN=%s&groupID=%s&groupname=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,groupID,groupname,n+1,select_insid,search(lpublic,"down_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"</tr>");
	  }

fprintf(cgiOut,"</table>"\
              "</td>"\
            "</tr>"\
            "<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>"\
          "</table>"\
        "</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
      "</tr>"\
    "</table></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>"\
"</html>");
	free_instance_parameter_list(&paraHead1);
	Free_ccgi_show_group_member_cmd(wtp_list);

}

