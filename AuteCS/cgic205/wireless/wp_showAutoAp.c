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
* wp_showAutoAp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
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
#include "wcpss/wid/WID.h"
#include "ws_dcli_ac.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

void ShowAutoApPage(char *m,char *n,struct list *lpublic,struct list *lwlan);  
void AutoApDeleteWlan(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);



int cgiMain()
{
  char encry[BUF_LEN] = { 0 };     
  char *str = NULL;          
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	; 
  }
  else
  {
    cgiFormStringNoNewlines("encry_showautoap",encry,BUF_LEN);
  }  
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowAutoApPage(encry,str,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowAutoApPage(char *m,char *n,struct list *lpublic,struct list *lwlan)
{ 
  DCLI_AC_API_GROUP_FIVE *auto_ap_login = NULL;  
  wid_auto_ap_if *head = NULL;
  int i = 0,j = 0,retu = 1,result1 = 0,limit = 0,cl = 1,is_bind_wlan = 0;                 /*颜色初值为#f9fafe*/
  char bwlanid[40] = { 0 };
  char tembwid[5] = { 0 };
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  char select_insid[10] = { 0 };
  int flag1 = 0,flag2 = 0;
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Auto AP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "<style>"\
  	"th{ font-family:Arial, Helvetica, sans-serif; font-weight:bold; font-size:12px; color:#0a4e70}"\
  	"#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
    "#link{ text-decoration:none; font-size: 12px}"\
  "</style>"\
  "</head>"\
  "<script type=\"text/javascript\">"\
  "function popMenu(objId)"\
  "{"\
	  "var obj = document.getElementById(objId);"\
	  "if (obj.style.display == 'none')"\
	  "{"\
		"obj.style.display = 'block';"\
	  "}"\
	  "else"\
	  "{"\
		"obj.style.display = 'none';"\
	  "}"\
  "}"\
  "function configInter(inter_name)"\
  "{"\
     "myform.IF_Name.value=inter_name;"\
     "myform.submit();"\
  "}"\
  "</script>"\
  "<script src=/instanceid_onchange.js>"\
  "</script>"\
  "<body>");  
  memset(select_insid,0,sizeof(select_insid));
  cgiFormStringNoNewlines( "INSTANCE_ID", select_insid, 10 );
  if(strcmp(select_insid,"")==0)
  {	
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);	
	if(paraHead1)
	{
		snprintf(select_insid,sizeof(select_insid)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id);
	}
  }  
  else
  {
	get_slotID_localID_instanceID(select_insid,&ins_para);	
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
  }
  if(paraHead1)
  {
	  AutoApDeleteWlan(paraHead1,lpublic,lwlan);
  }
  fprintf(cgiOut,"<form id=myform method=post>"\
  "<input type=hidden name='IF_Name' id='IF_Name' value=''/>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>"\
    	"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
		  "<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
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
				  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  if(paraHead1)
				  {
					  result1=show_auto_ap_config(paraHead1->parameter,paraHead1->connection,&auto_ap_login);
				  }
				  if(result1 == 1)
				  {
					  if((auto_ap_login != NULL)&&(auto_ap_login->auto_login != NULL))
					  {
					  	flag1 = 1;
						if(auto_ap_login->auto_login->auto_ap_if)
						  flag2 = 1;
					  }
				  }

				  if((flag1 == 1)&&(auto_ap_login->auto_login->ifnum>3))
				  	limit+=auto_ap_login->auto_login->ifnum-3;
				  if(retu==1) /*普通用户*/
				  	limit+=4;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:20px\">"\
			 "<table width=763 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
			 "<tr style=\"padding-bottom:15px\">"\
			   "<td width=70>%s ID:</td>",search(lpublic,"instance"));
			   fprintf(cgiOut,"<td width=693>"\
				   "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_showAutoAp.cgi\",\"%s\")>",m);
				   list_instance_parameter(&paraHead2, INSTANCE_STATE_WEB);	  
				   for(pq=paraHead2;(NULL != pq);pq=pq->next)
				   {
					  memset(temp,0,sizeof(temp));
					  snprintf(temp,sizeof(temp)-1,"%d-%d-%d",pq->parameter.slot_id,pq->parameter.local_id,pq->parameter.instance_id);
				   
					  if(strcmp(select_insid,temp) == 0)
						fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp,temp);
					  else
						fprintf(cgiOut,"<option value='%s'>%s",temp,temp);
				   }		   
				   free_instance_parameter_list(&paraHead2);
				   fprintf(cgiOut,"</select>"\
			   "</td>"\
			  "</tr>"\
			  "<tr>"\
				"<td align=center colspan=2>");
				if(result1==1)	
				{	
				   fprintf(cgiOut,"<table width=763 border=0 cellspacing=0 cellpadding=0>"\
				   "<tr align=left height=10 valign=top>"\
				   "<td id=thead1>%sAP%s%s</td>",search(lwlan,"dynamic"),search(lpublic,"policy"),search(lpublic,"info"));
				   fprintf(cgiOut,"</tr>"\
				   "<tr>"\
				   "<td align=left style=\"padding-left:20px\">"\
				   "<table frame=below rules=rows width=320 border=1>"\
				     "<tr align=left>"\
				       "<td id=td1 width=160>%sAP%s</td>",search(lwlan,"dynamic"),search(lpublic,"policy"));
				       if((flag1 == 1)&&(auto_ap_login->auto_login->auto_ap_switch==1))
			             fprintf(cgiOut,"<td id=td2 width=160>enable</td>");
			           else
			         	 fprintf(cgiOut,"<td id=td2 width=160>disable</td>");
				  	 fprintf(cgiOut,"</tr>"\
					 "<tr align=left>"\
					   "<td id=td1>%s%sAP%s</td>",search(lpublic,"log_save"),search(lwlan,"dynamic"),search(lpublic,"policy"));
					   if((flag1 == 1)&&(auto_ap_login->auto_login->save_switch==1))
						 fprintf(cgiOut,"<td id=td2>enable</td>");
					   else
						 fprintf(cgiOut,"<td id=td2>disable</td>");
				  	 fprintf(cgiOut,"</tr>"\
					 "<tr align=left>"\
					   "<td id=td1>%s%s</td>",search(lwlan,"bind_interface"),search(lpublic,"num"));
					   if(flag1 == 1)
					     fprintf(cgiOut,"<td id=td2>%d</td>",auto_ap_login->auto_login->ifnum);
					   else
					   	 fprintf(cgiOut,"<td id=td2>%d</td>",0);
				    fprintf(cgiOut,"</tr>"\
				  "</table></td>"\
				  "</tr>");
				  if((flag1 == 1)&&(auto_ap_login->auto_login->ifnum>0))
				  {
					  fprintf(cgiOut,"<tr align=left style=\"padding-top:30px\">"); 		  
						fprintf(cgiOut,"<td id=thead2>%s%s</td>",search(lwlan,"bind_interface"),search(lpublic,"info"));
					  fprintf(cgiOut,"</tr>"\
					  "<tr>"\
						"<td align=center style=\"padding-left:20px\">");
						  if(retu==0)/*管理员*/
							fprintf(cgiOut,"<table align=left frame=below rules=rows width=763 border=1>");
						  else
							fprintf(cgiOut,"<table align=left frame=below rules=rows width=390 border=1>");
						  fprintf(cgiOut,"<tr align=left height=20>"\
							"<th width=130>%s</th>",search(lwlan,"bind_interface"));
							fprintf(cgiOut,"<th width=110>%s%s</th>",search(lwlan,"bind_wlan"),search(lpublic,"num"));
							if(retu==0)/*管理员*/
							{
							  fprintf(cgiOut,"<th width=510>%s</th>",search(lwlan,"bind_wlan"));
							  fprintf(cgiOut,"<th width=13>&nbsp;</th>");
							}
							else
							  fprintf(cgiOut,"<th width=150>%s</th>",search(lwlan,"bind_wlan"));
						  fprintf(cgiOut,"</tr>");
						  if(flag2 == 1)
						  {
						    i = 0;
						  	for(head = auto_ap_login->auto_login->auto_ap_if;(NULL != head);head = head->ifnext)
							{ 		
								is_bind_wlan=0;
								memset(menu,0,sizeof(menu));
								strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
								snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
								strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
								fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
								fprintf(cgiOut,"<td id=td3>%s</td>",head->ifname);
								fprintf(cgiOut,"<td id=td3>%d</td>",head->wlannum);
								fprintf(cgiOut,"<td id=td3>");
								if(head->wlannum>0)
								{
									if(retu==0)  /*管理员*/
									{
										for(j=0;j<L_BSS_NUM;j++)
										{
											if(head->wlanid[j] != 0)
											{
												fprintf(cgiOut,"<input type=checkbox name='%s' value='%d'>wlan %d",head->ifname,head->wlanid[j],head->wlanid[j]);
												is_bind_wlan=1;
											}
										}
									}
									else
									{
										memset(bwlanid,0,sizeof(bwlanid));
										for(j=0;j<L_BSS_NUM;j++)
										{
											if(head->wlanid[j] != 0)
											{
												memset(tembwid,0,sizeof(tembwid));
												if(j==0)
												  snprintf(tembwid,sizeof(tembwid)-1,"%d",head->wlanid[j]);	/*int转成char*/
												else
												  snprintf(tembwid,sizeof(tembwid)-1,",%d",head->wlanid[j]);	/*int转成char*/
												strncat(bwlanid,tembwid,sizeof(bwlanid)-strlen(bwlanid)-1);
											}
										}
										fprintf(cgiOut,"%s",bwlanid);
									}
								}
								fprintf(cgiOut,"</td>");
								if(retu==0)/*管理员*/
								{
									fprintf(cgiOut,"<td align=left>");
													 if(flag1 == 1)
													   fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(auto_ap_login->auto_login->ifnum-i),menu,menu);
													 else
													   fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(256-i),menu,menu);
													 fprintf(cgiOut,"<img src=/images/detail.gif>"\
													 "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
													 if(is_bind_wlan==1)/*存在绑定的WLAN*/
													 {
														 fprintf(cgiOut,"<div id=div1>"\
														   "<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=# target=mainFrame onclick=configInter(\"%s\");>%s</a></div>",head->ifname,search(lpublic,"delete"));						
														 fprintf(cgiOut,"</div>");
													 }
													 fprintf(cgiOut,"</div>"\
													 "</div>"\
									"</td>");
								}
								fprintf(cgiOut,"</tr>");	  
								cl=!cl;
								i++;
							}
						  }
						fprintf(cgiOut,"</table>"\
						"</td>"\
						"</tr>");
				  }
					fprintf(cgiOut,"<tr>"\
					  "<td><input type=hidden name=encry_showautoap value=%s></td>",m);
					fprintf(cgiOut,"</tr>"\
					"<tr>"\
					  "<td><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
					fprintf(cgiOut,"</tr>");					
				  fprintf(cgiOut,"</table>");
				}
				else
				  fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));	
				fprintf(cgiOut,"</td>"\
			 " </tr>"\
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
if(result1 == 1)
{
  Free_auto_ap_config(auto_ap_login);
}
free_instance_parameter_list(&paraHead1);
}

void AutoApDeleteWlan(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{
    char inter_name[30] = { 0 };
	int result = cgiFormNotFound;   
    char **responses;
	int ret = 0,flag = 1;
	char alt[100] = { 0 };
	char max_wlan_num[10] = { 0 };
	
	memset(inter_name,0,sizeof(inter_name));
	cgiFormStringNoNewlines("IF_Name",inter_name,30);
	if(strcmp(inter_name,""))
	{
		result = cgiFormStringMultiple(inter_name, &responses);
		if(result == cgiFormNotFound)
		{
			flag=0;
   			ShowAlert(search(lwlan,"select_wlan"));
		}
		else
		{
			int i = 0;	
		    while((responses[i])&&(flag))
		    {
		    	ret=del_wirelesscontrol_auto_ap_binding_wlan_func(ins_para->parameter,ins_para->connection,responses[i],inter_name);
				switch(ret)
				{
					case SNMPD_CONNECTION_ERROR:
					case 0:{ 
						     ShowAlert(search(lwlan,"delete_wlan_fail"));
							 flag=0;
						     break;
						   }
				    case 1:break;
					case -1:{ 
						      ShowAlert(search(lpublic,"interface_too_long"));
							  flag=0;
						      break;
						    }
					case -2:{ 
							  memset(alt,0,sizeof(alt));
							  strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
							  memset(max_wlan_num,0,sizeof(max_wlan_num));
							  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
							  strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
							  strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
						      ShowAlert(alt);
							  flag=0;
						      break;
						    }
					case -3:{ 
						      ShowAlert(search(lwlan,"wlan_not_exist"));
							  flag=0;
						      break;
						    }
					case -4:{ 
						      ShowAlert(search(lwlan,"wlan_dont_bind_inter"));
							  flag=0;
						      break;
						    }
					case -5:{ 
						      ShowAlert(search(lwlan,"inter_not_auto_login_inter"));
							  flag=0;
						      break;
						    }
					case -6:{ 
						      ShowAlert(search(lwlan,"auto_ap_inter_not_set"));
							  flag=0;
						      break;
						    }
					case -7:{
							  memset(alt,0,sizeof(alt));
							  strncpy(alt,search(lwlan,"auto_ap_inter_wlan_num1"),sizeof(alt)-1);
							  strncat(alt,inter_name,sizeof(alt)-strlen(alt)-1);
							  strncat(alt,search(lwlan,"auto_ap_inter_wlan_num2"),sizeof(alt)-strlen(alt)-1);
							  strncat(alt,"0",sizeof(alt)-strlen(alt)-1);
							  strncat(alt,search(lwlan,"auto_ap_inter_wlan_num3"),sizeof(alt)-strlen(alt)-1);
							  ShowAlert(alt);
							  flag= 0;
							  break;
							}
					case -8:{ 
						      ShowAlert(search(lwlan,"disable_auto_ap_switch"));
							  flag=0;
						      break;
						    }
					case -9:{ 
						      ShowAlert(search(lpublic,"interface_not_exist"));
							  flag=0;
						      break;
						    }
					case -10:{ 
						      ShowAlert(search(lpublic,"error"));
							  flag=0;
						      break;
						    }
				}
		    	i++;
		    }
		}
		cgiStringArrayFree(responses);
		
		if(flag==1)
			ShowAlert(search(lwlan,"delete_wlan_succ"));
	}
}


