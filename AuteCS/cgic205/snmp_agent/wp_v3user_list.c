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
* wp_v3user_list.c
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
#include <dbus/dbus.h>
#include "ws_snmpd_engine.h"
#include "ac_manage_interface.h"
#include "ws_dcli_wlans.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"


void ShowV3userListPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd);  
void DeleteV3user(DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;      
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsnmpd = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsnmpd=get_chain_head("../htdocs/text/snmpd.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN);
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowV3userListPage(encry,str,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowV3userListPage(char *m,char *n,struct list *lpublic,struct list *lsnmpd)
{    
  char IsDeleete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  int i = 0,retu = 1,limit = 0,cl = 1;   
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  char select_slotid[10] = { 0 };
  char temp[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  char *endptr = NULL;
  int ret = AC_MANAGE_DBUS_ERROR;
  STSNMPV3User *v3user_array = NULL;
  unsigned int v3user_num = 0;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>SNMP V3</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "<style>"\
    "#div1{ width:62px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:60px; height:15px; padding-left:5px; padding-top:3px}"\
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
  "</script>"\
  "<script src=/slotid_onchange.js>"\
  "</script>"\
  "<body>");
  memset(select_slotid,0,sizeof(select_slotid));
  cgiFormStringNoNewlines( "SLOT_ID", select_slotid, 10 );
  list_instance_parameter(&paraHead, SNMPD_SLOT_CONNECT);
  if(strcmp(select_slotid,"")==0)
  { 
	if(paraHead)
	{
		snprintf(select_slotid,sizeof(select_slotid)-1,"%d",paraHead->parameter.slot_id);
	} 
  }
  slot_id=strtoul(select_slotid,&endptr,10);
  get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);
  
  memset(IsDeleete,0,sizeof(IsDeleete));
  cgiFormStringNoNewlines("DeleteV3user", IsDeleete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDeleete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    DeleteV3user(select_connection,lpublic,lsnmpd);
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>SNMP V3</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
     	  "<td width=62 align=center><a href=wp_snmp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_snmp.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
				    "<td align=left id=tdleft><a href=wp_view_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"view_list"));
				  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);		  
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_view_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_view")); 					  
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_group_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"group_list"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_group_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_group"));                       
                    fprintf(cgiOut,"</tr>");
				  }	
				  fprintf(cgiOut,"<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsnmpd,"v3user_list"));   /*突出显示*/
				  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_v3user_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_v3user"));                       
                    fprintf(cgiOut,"</tr>");
				  }	
				  
				  limit = 0;
				  ret = ac_manage_show_snmp_v3user(select_connection, &v3user_array, &v3user_num, NULL);  
				  if(AC_MANAGE_SUCCESS == ret)
				  {
				  	if(v3user_num > 5)
				  	{
						limit = v3user_num - 5;
				  	}
				  }

				  if(retu == 1)  /*普通用户*/
				  	limit+=3;

				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
			    
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
 "<table width=613 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>");
  fprintf(cgiOut,"<tr style=\"padding-bottom:15px\">"\
	"<td width=70><b>SLOT ID:</b></td>"\
	  "<td width=543>"\
		  "<select name=slot_id id=slot_id style=width:45px onchange=slotid_change(this,\"wp_v3user_list.cgi\",\"%s\")>",m);
		  for(paraNode = paraHead; paraNode; paraNode = paraNode->next)
		  {
			 memset(temp,0,sizeof(temp));
			 snprintf(temp,sizeof(temp)-1,"%d",paraNode->parameter.slot_id);
  
			 if(strcmp(select_slotid,temp) == 0)
			   fprintf(cgiOut,"<option value='%s' selected=selected>%s",temp,temp);
			 else
			   fprintf(cgiOut,"<option value='%s'>%s",temp,temp);
		  } 		  
		  fprintf(cgiOut,"</select>"\
	  "</td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=2 valign=top align=center style=\"padding-bottom:5px\">"\
		"<table frame=below rules=rows width=613 border=1>"\
			"<tr align=left>"\
				"<th width=150><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsnmpd,"v3user_name"));
				fprintf(cgiOut,"<th width=150><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsnmpd,"auth_proto"));
				fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsnmpd,"privacy_proto"));
				fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsnmpd,"group_name"));
				fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsnmpd,"status"));
				fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
			"</tr>");				
			if((AC_MANAGE_SUCCESS == ret)&&(v3user_array))
			{
				for(i = 0; i < v3user_num; i++)
				{
					memset(menu,0,sizeof(menu));
					strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
					snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
					strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
					fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
					fprintf(cgiOut,"<td>%s</td>",v3user_array[i].name);
					if(AUTH_PRO_NONE == v3user_array[i].authentication.protocal)
					{
						fprintf(cgiOut,"<td>None</td>");
					}
					else if(AUTH_PRO_MD5 == v3user_array[i].authentication.protocal)
					{
						fprintf(cgiOut,"<td>MD5</td>");
					}
					else if(AUTH_PRO_SHA == v3user_array[i].authentication.protocal)
					{
						fprintf(cgiOut,"<td>SHA</td>");
					}
					
					if(PRIV_PRO_NONE == v3user_array[i].privacy.protocal)
					{
						fprintf(cgiOut,"<td>None</td>");
					}
					else if(PRIV_PRO_DES == v3user_array[i].privacy.protocal)
					{
						fprintf(cgiOut,"<td>DES</td>");
					}
					else if(PRIV_PRO_AES == v3user_array[i].privacy.protocal)
					{
						fprintf(cgiOut,"<td>AES</td>");
					}
					fprintf(cgiOut,"<td>%s</td>",v3user_array[i].group_name);
					fprintf(cgiOut,"<td>%s</td>",v3user_array[i].status ? "enable":"disable");
					fprintf(cgiOut,"<td>");
					if(retu == 0)  /*管理员*/
					{
						fprintf(cgiOut,"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(v3user_num-i),menu,menu);
						fprintf(cgiOut,"<img src=/images/detail.gif>"\
							"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
								fprintf(cgiOut,"<div id=div1>");
								if(retu==0)  /*管理员*/
								{
									fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_v3user_add.cgi?UN=%s target=mainFrame>%s</a></div>",m,search(lpublic,"ntp_add"));   
									fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_v3user_list.cgi?UN=%s&DeleteV3user=%s&V3userName=%s&SLOT_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,"true",v3user_array[i].name,select_slotid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));                             
								}
								fprintf(cgiOut,"</div>"\
							"</div>"\
						"</div>");
					}
					else
					{
						fprintf(cgiOut,"&nbsp;");
					}
					fprintf(cgiOut,"</td></tr>");
					cl=!cl;
				}
				MANAGE_FREE(v3user_array);
			}
		fprintf(cgiOut,"</table>"\
	"</td>"\
 "</tr>"\
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
free_instance_parameter_list(&paraHead);
}


void DeleteV3user(DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd)
{
	int ret = AC_MANAGE_DBUS_ERROR;
	char V3userName[25] = { 0 };
	int ret_c = 0;
	
	memset(V3userName,0,sizeof(V3userName));
    cgiFormStringNoNewlines("V3userName", V3userName, 25);
	if(strcmp(V3userName,""))
	{
	    ret_c = check_snmp_name(V3userName, 0);
		if((ret_c == -2)||(ret_c == -1)) 
		{
			ShowAlert(search(lsnmpd,"v3user_lenth_illegal"));
			return;
		}
		else if(ret_c == -3)
		{
			ShowAlert(search(lsnmpd,"v3user_illegal1"));
			return;
		}
		else if(ret_c == -4) 
		{
			ShowAlert(search(lsnmpd,"v3user_illegal2"));
			return;
		}
		else if(ret_c == -5)
		{
			ShowAlert(search(lsnmpd,"v3user_illegal3"));
			return;
		}		
	
		ret = ac_manage_config_snmp_del_v3user(select_connection, V3userName);
	    if(AC_MANAGE_SUCCESS == ret) 
		{
			ShowAlert(search(lsnmpd,"delete_v3user_succ"));
	    }
	    else if(AC_MANAGE_SERVICE_ENABLE == ret) 
		{
			ShowAlert(search(lsnmpd,"disable_snmp_service"));
	    }
	    else if(AC_MANAGE_CONFIG_NONEXIST == ret)
		{
			ShowAlert(search(lsnmpd,"v3user_not_exist"));
	    }
	    else if(AC_MANAGE_DBUS_ERROR == ret)
		{
			ShowAlert(search(lsnmpd,"delete_v3user_fail"));
	    }
	    else 
		{
			ShowAlert(search(lsnmpd,"delete_v3user_fail"));
	    }
	}	
}

