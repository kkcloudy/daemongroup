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
* wp_v3user_add.c
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
#include "ws_public.h"


void ShowAddV3userPage(char *m,struct list *lpublic,struct list *lsnmpd);  
void AddV3user(DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd);


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
  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }
  else
  {
    cgiFormStringNoNewlines("encry_add_v3user",encry,BUF_LEN);
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowAddV3userPage(encry,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}
void ShowAddV3userPage(char *m,struct list *lpublic,struct list *lsnmpd)
{    
  char IsSubmit[5] = { 0 };
  int i = 0,limit = 0;     
  char select_slotid[10] = { 0 };
  char temp[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  char *endptr = NULL;	
  int ret = AC_MANAGE_DBUS_ERROR;
  STSNMPGroup *group_array = NULL;
  unsigned int group_num = 0;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>SNMP V3</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script type='text/javascript'>"\
	"function ChangeState(){"\
		"var auth_proto_index = document.all.auth_proto.selectedIndex;"\
		"var auth_key_tr = document.getElementById('auth_key_tr');"\
		"var privacy_proto_tr = document.getElementById('privacy_proto_tr');"\
		"var privacy_key_tr = document.getElementById('privacy_key_tr');"\
		"if(auth_proto_index == 1)"\
  		"{"\
  			"auth_key_tr.style.display = \"none\";"\
  			"auth_key_limit.style.display = \"none\";"\
  			"privacy_proto_tr.style.display = \"none\";"\
  			"privacy_proto_limit.style.display = \"none\";"\
  			"privacy_key_tr.style.display = \"none\";"\
  			"privacy_key_limit.style.display = \"none\";"\
  		"}"\
  		"else"\
	    "{"\
		    "auth_key_tr.style.display = \"block\";"\
		    "auth_key_limit.style.display = \"block\";"\
		    "privacy_proto_tr.style.display = \"block\";"\
  			"privacy_proto_limit.style.display = \"block\";"\
  			"privacy_key_tr.style.display = \"block\";"\
  			"privacy_key_limit.style.display = \"block\";"\
	    "}"\
  		
  		"var privacy_proto_index = document.all.privacy_proto.selectedIndex;"\
  		"if(((auth_proto_index == 2)||(auth_proto_index == 3))&&(privacy_proto_index == 1))"\
  		"{"\
  			"privacy_key_tr.style.display = \"none\";"\
  			"privacy_key_limit.style.display = \"none\";"\
  		"}"\
  		"else if(privacy_proto_index == 2)"\
	    "{"\
  			"privacy_key_tr.style.display = \"block\";"\
  			"privacy_key_limit.style.display = \"block\";"\
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

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("add_v3user_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	 AddV3user(select_connection,lpublic,lsnmpd);
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
          "<td width=62 align=center><input id=but type=submit name=add_v3user_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
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
				  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
				    "<td align=left id=tdleft><a href=wp_view_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_view"));
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_group_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"group_list"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_group_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_group"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_v3user_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"v3user_list"));                       
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=26>"\
					"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsnmpd,"add_v3user"));   /*突出显示*/
				  fprintf(cgiOut,"</tr>");

				  limit = 0;				  

				  fprintf(cgiOut,"<tr height=25 id=auth_key_limit style=\"display:block\">"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
				  fprintf(cgiOut,"<tr height=25 id=privacy_proto_limit style=\"display:block\">"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
				  fprintf(cgiOut,"<tr height=25 id=privacy_key_limit style=\"display:block\">"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }			    
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:20px; padding-top:10px\">"\
	"<table width=790 border=0 cellspacing=0 cellpadding=0>");
		fprintf(cgiOut,"<tr height=30>"\
		  "<td>SLOT ID:</td>"\
		  "<td colspan=2>"\
			  "<select name=slot_id id=slot_id style=width:100px onchange=slotid_change(this,\"wp_v3user_add.cgi\",\"%s\")>",m);
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
		"</tr>"
		"<tr height=30>"\
			"<td width=70>%s:</td>",search(lsnmpd,"v3user_name"));
			fprintf(cgiOut,"<td width=100><input name=v3user_name size=23 maxLength=30></td>"\
			"<td width=620><font color=red>(%s)</font></td>",search(lsnmpd,"v3user_name_range"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"auth_proto"));
			fprintf(cgiOut,"<td colspan=2><select name=auth_proto id=auth_proto style=width:100px onchange=\"ChangeState()\">"\
				"<option value=>"\
				"<option value=none>none"\
				"<option value=md5>md5"\
				"<option value=sha>sha"\
			"</select></td>"\
		"</tr>"\
		"<tr height=30 id=auth_key_tr style=\"display:block\">"\
			"<td>%s:</td>",search(lsnmpd,"auth_key"));
			fprintf(cgiOut,"<td><input name=auth_key size=23 maxLength=20></td>"\
			"<td><font color=red>(%s)</font></td>",search(lsnmpd,"key_lenth"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30 id=privacy_proto_tr style=\"display:block\">"\
			"<td>%s:</td>",search(lsnmpd,"privacy_proto"));
			fprintf(cgiOut,"<td colspan=2><select name=privacy_proto id=privacy_proto style=width:100px onchange=\"ChangeState()\">"\
				"<option value=>"\
				"<option value=none>none"\
				"<option value=des>des"\
			"</select></td>"\
		"</tr>"\
		"<tr height=30 id=privacy_key_tr style=\"display:block\">"\
			"<td>%s:</td>",search(lsnmpd,"privacy_key"));
			fprintf(cgiOut,"<td><input name=privacy_key size=23 maxLength=20></td>"\
			"<td><font color=red>(%s)</font></td>",search(lsnmpd,"key_lenth"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"group_name"));
			fprintf(cgiOut,"<td colspan=2><select name=group_name id=group_name style=width:150px>"\
				"<option value=>");
				ret = ac_manage_show_snmp_group(select_connection, &group_array, &group_num, NULL); 
				if((AC_MANAGE_SUCCESS == ret)&&(group_array))
				{
					for(i = 0; i < group_num; i++)
					{
						fprintf(cgiOut,"<option value='%s'>%s",group_array[i].group_name,group_array[i].group_name);
					}	
					MANAGE_FREE(group_array);
				}
			fprintf(cgiOut,"</select></td>"\
		"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"status"));
			fprintf(cgiOut,"<td colspan=2><select name=status id=status style=width:100px>"\
				"<option value=>"\
				"<option value=enable>enable"\
				"<option value=disable>disable"\
			"</select></td>"\
		"</tr>"\
		"<tr>"\
	 		"<td><input type=hidden name=encry_add_v3user value=%s></td>",m);
			fprintf(cgiOut,"<td><input type=hidden name=SLOT_ID value=%s></td>",select_slotid);
			fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
		fprintf(cgiOut,"</tr>");
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
free_instance_parameter_list(&paraHead);
}

void AddV3user(DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd)
{
	int ret = AC_MANAGE_DBUS_ERROR;
	char v3user_name[30] = { 0 };
	char auth_proto[10] = { 0 };
	char group_name[30] = { 0 };
	char status[10] = { 0 };
	char auth_key[30] = { 0 };
	char privacy_proto[10] = { 0 };
	char privacy_key[30] = { 0 };
	int ret_c = 0;		
	STSNMPV3User v3user_node = { 0 };
	
	memset(v3user_name,0,sizeof(v3user_name));
	cgiFormStringNoNewlines("v3user_name", v3user_name, 30);
	memset(auth_proto,0,sizeof(auth_proto));
	cgiFormStringNoNewlines("auth_proto", auth_proto, 10);
	memset(group_name,0,sizeof(group_name));
	cgiFormStringNoNewlines("group_name", group_name, 30);
	memset(status,0,sizeof(status));
	cgiFormStringNoNewlines("status", status, 10);
	memset(auth_key,0,sizeof(auth_key));
	cgiFormStringNoNewlines("auth_key", auth_key, 30);
	memset(privacy_proto,0,sizeof(privacy_proto));
	cgiFormStringNoNewlines("privacy_proto", privacy_proto, 10);
	memset(privacy_key,0,sizeof(privacy_key));
	cgiFormStringNoNewlines("privacy_key", privacy_key, 30);
	
	if((strcmp(v3user_name,""))&&(strcmp(auth_proto,""))&&(strcmp(group_name,""))&&(strcmp(status,"")))
	{
		ret_c = check_snmp_name(v3user_name, 0);
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
		
		ret_c = 0;
		ret_c = check_snmp_name(group_name, 0);
		if((ret_c == -2)||(ret_c == -1)) 
		{
			ShowAlert(search(lsnmpd,"group_lenth_illegal"));
			return;
		}
		else if(ret_c == -3)
		{
			ShowAlert(search(lsnmpd,"group_illegal1"));
			return;
		}
		else if(ret_c == -4) 
		{
			ShowAlert(search(lsnmpd,"group_illegal2"));
			return;
		}
		else if(ret_c == -5)
		{
			ShowAlert(search(lsnmpd,"group_illegal3"));
			return;
		}

		if(strcmp(auth_proto,"none") == 0)/*添加无认证协议V3用户*/
		{		    
	        strncpy(v3user_node.name, v3user_name, sizeof(v3user_node.name) - 1);
	        strncpy(v3user_node.group_name, group_name, sizeof(v3user_node.group_name) - 1);
	        v3user_node.authentication.protocal = AUTH_PRO_NONE;
			
	        if(0 == strcmp(status, "enable")) 
			{
	            v3user_node.status = RULE_ENABLE;
	        }
	        else if(0 == strcmp(status, "disable"))
			{
	            v3user_node.status = RULE_DISABLE;
	        }
	        else 
			{
				ShowAlert(search(lpublic,"input_para_illegal"));
				return;
			}

			ret = ac_manage_config_snmp_add_v3user(select_connection, &v3user_node);


		    if(AC_MANAGE_SUCCESS == ret) 
			{
				ShowAlert(search(lsnmpd,"add_v3user_succ"));
				return;
			}
		    else if(AC_MANAGE_SERVICE_ENABLE == ret) 
			{
				ShowAlert(search(lsnmpd,"disable_snmp_service"));
				return;
			}
		    else if(AC_MANAGE_CONFIG_EXIST == ret) 
			{
				ShowAlert(search(lsnmpd,"v3user_exist"));
				return;
			}
		    else if(AC_MANAGE_CONFIG_NONEXIST == ret)
			{
				ShowAlert(search(lsnmpd,"group_not_exist"));
				return;
			}
		    else if(AC_MANAGE_DBUS_ERROR == ret) 
			{
				ShowAlert(search(lsnmpd,"add_v3user_fail"));
				return;
			}
		    else 
			{
				ShowAlert(search(lsnmpd,"add_v3user_fail"));
				return;
			}
		}
		else if((strcmp(auth_key,""))&&(strcmp(privacy_proto,"")))
		{
			if(strcmp(privacy_proto,"none") == 0)/*添加无私有协议V3用户*/
			{
		        if(strlen(auth_key) < 8 || strlen(auth_key) > 20) 
				{
					ShowAlert(search(lsnmpd,"auth_key_lenth_wrong"));
					return;
				}

		        strncpy(v3user_node.name, v3user_name, sizeof(v3user_node.name) - 1);
		        strncpy(v3user_node.authentication.passwd, auth_key, sizeof(v3user_node.authentication.passwd) - 1);
		        strncpy(v3user_node.group_name, group_name, sizeof(v3user_node.group_name) - 1);

		        if(0 == strcmp(auth_proto, "md5")) {
		            v3user_node.authentication.protocal = AUTH_PRO_MD5;
		        }
		        else if(0 == strcmp(auth_proto, "sha")) {
		            v3user_node.authentication.protocal = AUTH_PRO_SHA;
		        }
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        
		        v3user_node.privacy.protocal = PRIV_PRO_NONE;

		        if(0 == strcmp(status, "enable")) {
		            v3user_node.status = RULE_ENABLE;
		        }
		        else if(0 == strcmp(status, "disable")){
		            v3user_node.status = RULE_DISABLE;
		        }        
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

				ret = AC_MANAGE_DBUS_ERROR;
				ret = ac_manage_config_snmp_add_v3user(select_connection, &v3user_node);

			    if(AC_MANAGE_SUCCESS == ret)
				{
					ShowAlert(search(lsnmpd,"add_v3user_succ"));
					return;
				}
			    else if(AC_MANAGE_SERVICE_ENABLE == ret) 
				{
					ShowAlert(search(lsnmpd,"disable_snmp_service"));
					return;
				}
			    else if(AC_MANAGE_CONFIG_EXIST == ret)
				{
					ShowAlert(search(lsnmpd,"v3user_exist"));
					return;
				}
			    else if(AC_MANAGE_CONFIG_NONEXIST == ret) 
				{
					ShowAlert(search(lsnmpd,"group_not_exist"));
					return;
				}
			    else if(AC_MANAGE_DBUS_ERROR == ret)
				{
					ShowAlert(search(lsnmpd,"add_v3user_fail"));
					return;
				}
			    else
				{
					ShowAlert(search(lsnmpd,"add_v3user_fail"));
					return;
				}
			}
			else if(strcmp(privacy_key,""))/*添加DES私有协议V3用户*/
			{
		        if(strlen(auth_key) < 8 || strlen(auth_key) > 20) 
				{
					ShowAlert(search(lsnmpd,"auth_key_lenth_wrong"));
					return;
				}
		        if(strlen(privacy_key) < 8 || strlen(privacy_key) > 20) 
				{
		            ShowAlert(search(lsnmpd,"privacy_key_lenth_wrong"));
		            return;
		        }			    

		        strncpy(v3user_node.name, v3user_name, sizeof(v3user_node.name) - 1);
		        strncpy(v3user_node.authentication.passwd, auth_key, sizeof(v3user_node.authentication.passwd) - 1);
		        strncpy(v3user_node.privacy.passwd, privacy_key, sizeof(v3user_node.privacy.passwd) - 1);
		        strncpy(v3user_node.group_name, group_name, sizeof(v3user_node.group_name) - 1);

		        if(0 == strcmp(auth_proto, "md5")) {
		            v3user_node.authentication.protocal = AUTH_PRO_MD5;
		        }
		        else if(0 == strcmp(auth_proto, "sha")) {
		            v3user_node.authentication.protocal = AUTH_PRO_SHA;
		        }
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        
		        v3user_node.privacy.protocal = PRIV_PRO_DES;

		        if(0 == strcmp(status, "enable")) {
		             v3user_node.status = RULE_ENABLE;
		        }
		        else if(0 == strcmp(status, "disable")){
		             v3user_node.status = RULE_DISABLE;
		        }
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

			   	ret = AC_MANAGE_DBUS_ERROR;
				ret = ac_manage_config_snmp_add_v3user(select_connection, &v3user_node);

			    if(AC_MANAGE_SUCCESS == ret) 
				{
					ShowAlert(search(lsnmpd,"add_v3user_succ"));
					return;
				}
			    else if(AC_MANAGE_SERVICE_ENABLE == ret)
				{
					ShowAlert(search(lsnmpd,"disable_snmp_service"));
					return;
				}
			    else if(AC_MANAGE_CONFIG_EXIST == ret)
				{
					ShowAlert(search(lsnmpd,"v3user_exist"));
					return;
				}
			    else if(AC_MANAGE_CONFIG_NONEXIST == ret) 
				{
					ShowAlert(search(lsnmpd,"group_not_exist"));
					return;
				}
			    else if(AC_MANAGE_DBUS_ERROR == ret) 
				{
					ShowAlert(search(lsnmpd,"add_v3user_fail"));
					return;
				}
			    else 
				{
					ShowAlert(search(lsnmpd,"add_v3user_fail"));
					return;
				}
			}
			else
			{
				ShowAlert(search(lpublic,"para_incom"));
				return;
			}
		}		
		else
		{
			ShowAlert(search(lpublic,"para_incom"));
			return;
		}
	}	
	else
	{
		ShowAlert(search(lpublic,"para_incom"));
		return;
	}
}

