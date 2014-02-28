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



void ShowWtpGroupMemPage(char *m, char *t, struct list *lpublic, struct list *lwlan);    
void DeleteWtpgroupMem(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan);
void AddWtpgroupMem(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan);


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
	int apgroupid=0;
	char groupname[WTP_AP_GROUP_NAME_MAX_LEN+5] = { 0 };
	char add_del[10] = { 0 };
	dbus_parameter ins_para;
	instance_parameter *paraHead1 = NULL;
	unsigned int *wtp_list = NULL;
	unsigned int apcount=0;

	memset(select_insid,0,sizeof(select_insid));
	memset(groupID,0,sizeof(groupID));
	memset(groupname,0,sizeof(groupname));
	cgiFormStringNoNewlines("INSTANCE_ID", select_insid, 10);
	cgiFormStringNoNewlines("groupID", groupID, 5);
	cgiFormStringNoNewlines("groupname", groupname, WTP_AP_GROUP_NAME_MAX_LEN+5);
	
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
	  fprintf(cgiOut,"</script>"\
	  "<script src=/instanceid_onchange.js>"\
	  "</script>"\
	"<body>");	  

	if(cgiFormSubmitClicked("wtpgroup_mem") == cgiFormSuccess)
	{ 
		if(paraHead1)
		{
			cgiFormStringNoNewlines("add_dele", add_del, sizeof(add_del));
			if(strcmp(add_del,"add")==0)
			{
				AddWtpgroupMem(paraHead1,groupID, lpublic,lwlan);
			}
			else if(strcmp(add_del,"dele")==0)
			{
				DeleteWtpgroupMem(paraHead1,groupID,lpublic,lwlan);
			}
		} 
	}
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
          "<td width=62 align=center><input id=but type=submit name=wtpgroup_mem style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
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
	                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lwlan,"ap_group_mem"));   /*突出显示*/
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
                  for(i=0;i<3;i++)
	          {
  			fprintf(cgiOut,"<tr height=25>"\
                      		"<td id=tdleft>&nbsp;</td>"\
                   		 "</tr>");
	          }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:10px; padding-top:10px\">"\
   "<table width=773 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>");
	fprintf(cgiOut,"<tr>"\
			"<td><table><tr>"
	   	 	"<td width=110 style=\"font-size:14px; font-weight:bold;\">%s ID:%s",search(lpublic,"instance"),select_insid);
	fprintf(cgiOut,"</td>"\
		"</tr></table></td></tr>");
	fprintf(cgiOut,"<tr><td><table>"\
			"<tr><td id=thead5 align=left>%s ID:%s</td>"\
			"<td id=thead5 align=left>group %s:%s</td></tr>"\
			"<td width=160>&nbsp;&nbsp;&nbsp;&nbsp;</td></tr>"\
			"</table></td></tr>",search(lwlan,"ap_group"),groupID,search(lpublic,"name"),groupname);
	
	if(retu==0)  /*管理员*/
	{
		fprintf(cgiOut,"<tr><td><table><tr>");
		fprintf(cgiOut,"<td width=160 align=center><input type=radio name=add_dele value=\"add\" checked>%s</td>\n",search(lwlan,"add_ap_mem"));
		fprintf(cgiOut,"<td width=110 align=left><input type=radio name=add_dele value=\"dele\" >%s</td>\n",search(lwlan,"delete_ap_mem"));
		fprintf(cgiOut,"<td>&nbsp;&nbsp;&nbsp;&nbsp;</tr>");
		fprintf(cgiOut,"</tr>");
		
		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td width=110><input type=radio name=all_not value=\"all\" onclick=disable_s('all') >%s</td>\n",search(lwlan,"by_all"));
		fprintf(cgiOut,"</tr>");
		
		fprintf(cgiOut,"<tr>");
		fprintf(cgiOut,"<td colspan=3><input type=radio name=all_not value=\"by_mem\" checked onclick=disable_s('bymem') >%s:\n",search(lwlan,"by_ap_mem"));
		fprintf(cgiOut,"<input type=text name=mem_conte id=mem_conte size=30 onkeypress=\"return ((event.keyCode>=48&&event.keyCode<=57)||(event.keyCode==44)||(event.keyCode==45))\">"\
			"<font color=red>(%s)</font></td>",search(lwlan,"format_input"));
		fprintf(cgiOut,"</tr></table></td></tr>");
	}
	fprintf(cgiOut,"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;</td></tr>");
	
	if(paraHead1)
	{	
		int j=0;
		apgroupid= strtoul(groupID,&endptr,10);
		result=ccgi_show_group_member_cmd(paraHead1->parameter,paraHead1->connection,apgroupid,&wtp_list,&apcount);
		if(result == 0)
		{	
			fprintf(cgiOut,"<tr><td><table>");
			if(apcount == 0)
			{
				fprintf(cgiOut,"<tr><td>%s</td></tr>",search(lwlan,"group_no_mem_list"));
			}
			for(i=0;i<apcount;i++)
			{
				if(j%30 == 0)
				{
					fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
					if(i==0)
					{
						j=j+3;
						fprintf(cgiOut,"<td colspan=5>%s:</td>",search(lwlan,"group_mem_list"));
					}
					else
					{
						fprintf(cgiOut,"<td width=13>%d,</td>",wtp_list[i]);
					}
				}
				fprintf(cgiOut,"<td width=13>%d,</td>",wtp_list[i]);
				if(j%10 == 29)
				{
					fprintf(cgiOut,"</tr>");
					cl=!cl;					
				}
				j=j+1;
			}
			fprintf(cgiOut,"</tr></td></table>");
		}
	} 
	
	fprintf(cgiOut,"<tr>"\
	  "<td><input type=hidden name=UN value=%s></td>",m);
	  fprintf(cgiOut,"<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
	   fprintf(cgiOut,"<td ><input type=hidden name=groupID value=%s></td>",groupID);
	   fprintf(cgiOut,"<td ><input type=hidden name=groupname value=%s></td>",groupname);
	fprintf(cgiOut,"</tr>"\
"</table>"\
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

void DeleteWtpgroupMem(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan)
{
	int ret = 0;  
	char type[10]={0};
	char mem_conte[200]={0};
	
	cgiFormStringNoNewlines("all_not", type, 10);
	if(strcmp(type,"all")==0)
	{
		fprintf(stderr,"dele_______by ___all");
		ret=ccgi_add_del_ap_group_member_cmd(ins_para->parameter,ins_para->connection,ID, "delete","all");
	}
	else if(strcmp(type,"by_mem")==0)
	{
		fprintf(stderr,"dele_______by __by_mem");
		cgiFormStringNoNewlines("mem_conte", mem_conte, 200);
		fprintf(stderr,"dele_______by __by_mem mem_conte=%s",mem_conte);
		ret=ccgi_add_del_ap_group_member_cmd(ins_para->parameter,ins_para->connection,ID,"delete",mem_conte);
	}
	switch(ret)
	{
		case 0:
		{
			ShowAlert(search(lwlan,"delete_group_succ"));
			break;
		}
		case 1:
		{
			ShowAlert(search(lwlan,"delete_group_part_fail"));
			break;
		}
		case -3:
		{
			ShowAlert(search(lwlan,"format_input"));
			break;
		}
		default:
		{
			ShowAlert(search(lwlan,"delete_group_fail"));
			break;
		}
	}
}

void AddWtpgroupMem(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan)
{
	int ret = 0;  
	char type[10]={0};
	char mem_conte[200]={0};
	
	cgiFormStringNoNewlines("all_not", type, 10);
	if(strcmp(type,"all")==0)
	{
		fprintf(stderr,"add_______by ___all");
		ret=ccgi_add_del_ap_group_member_cmd(ins_para->parameter,ins_para->connection,ID,"add", "all");
	}
	else if(strcmp(type,"by_mem")==0)
	{
		fprintf(stderr,"add_______by __by_mem");
		cgiFormStringNoNewlines("mem_conte", mem_conte, 200);
		fprintf(stderr,"add_______by __by_mem mem_conte=%s",mem_conte);
		ret=ccgi_add_del_ap_group_member_cmd(ins_para->parameter,ins_para->connection,ID,"add",mem_conte);
	}
	switch(ret)
	{
		case 0:
		{
			ShowAlert(search(lwlan,"add_group_succ"));
			break;
		}
		case 1:
		{
			ShowAlert(search(lwlan,"add_group_part_fail"));
			break;
		}
		case -3:
		{
			ShowAlert(search(lwlan,"format_input"));
			break;
		}
		default:
		{
			ShowAlert(search(lwlan,"add_group_fail"));
			break;
		}
	}
}

