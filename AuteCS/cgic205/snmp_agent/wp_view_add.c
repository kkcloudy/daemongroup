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
* wp_view_add.c
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


void ShowAddViewPage(char *m,int add_flag,char *ViewName,struct list *lpublic,struct list *lsnmpd);  
void AddorConfigView(int add_flag,DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;   
  char AddFlag[5] = { 0 };  
  char ViewName[25] = { 0 };
  int add_flag = 1;	/*1表示添加视图，0表示修改视图*/
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsnmpd = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsnmpd=get_chain_head("../htdocs/text/snmpd.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(AddFlag,0,sizeof(AddFlag));
  memset(ViewName,0,sizeof(ViewName));
  
  cgiFormStringNoNewlines("AddFlag", AddFlag, BUF_LEN);
  if(strcmp(AddFlag,"0") == 0)
  	add_flag = 0;  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {  	
  	if(0 == add_flag)
  	{
		cgiFormStringNoNewlines("ViewName", ViewName, 25);		
  	}
  }
  else
  {
    cgiFormStringNoNewlines("encry_add_view",encry,BUF_LEN);
	if(0 == add_flag)
	{
		cgiFormStringNoNewlines("ViewName", ViewName, 25);		
	}
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowAddViewPage(encry,add_flag,ViewName,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}
void ShowAddViewPage(char *m,int add_flag,char *ViewName,struct list *lpublic,struct list *lsnmpd)
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
  STSNMPView *view_array = NULL;
  unsigned int view_num = 0;
  struct oid_list *oid_node = NULL;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>SNMP V3</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<script src=/slotid_onchange.js>"\
  "</script>"\
  "<script src=/ip.js>"\
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
  if((cgiFormSubmitClicked("add_config_view_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	 AddorConfigView(add_flag,select_connection,lpublic,lsnmpd);
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
          "<td width=62 align=center><input id=but type=submit name=add_config_view_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
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
				  "<tr height=26>");
				  	if(1 == add_flag)
					  fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsnmpd,"add_view"));   /*突出显示*/
					else
					  fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsnmpd,"config_view"));   /*突出显示*/
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
                  "<tr height=25>"\
				    "<td align=left id=tdleft><a href=wp_v3user_add.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"add_v3user"));                       
                  fprintf(cgiOut,"</tr>");

				  if((0 == add_flag)&&(ViewName))
				  {
					  ret = ac_manage_show_snmp_view(select_connection, &view_array, &view_num, ViewName);	
				  }
				  
				  if(1 == add_flag)
				  {
				  	limit = 0;
				  }
				  else
				  {
				  	limit = 5;
				  }
				  
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
			  "<select name=slot_id id=slot_id style=width:100px onchange=slotid_change(this,\"wp_view_add.cgi\",\"%s\")>",m);
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
			"<td width=70>%s:</td>",search(lsnmpd,"view_name"));
			if(1 == add_flag)
			{
				fprintf(cgiOut,"<td width=100><input name=view_name size=21 maxLength=30></td>");
			}
			else
			{
				fprintf(cgiOut,"<td width=100><input name=view_name size=21 maxLength=30 value=%s></td>",ViewName);
			}
			fprintf(cgiOut,"<td width=620><font color=red>(%s)</font></td>",search(lsnmpd,"view_name_range"));
		fprintf(cgiOut,"</tr>");
		if(0 == add_flag)
		{
			fprintf(cgiOut,"<tr height=30 style=\"padding-top:5px\">"\
			  "<td>%s:</td>",search(lsnmpd,"view_included"));
			  fprintf(cgiOut,"<td><textarea name=included ROWS=7 COLS=35>");
			    if((AC_MANAGE_SUCCESS == ret)&&(view_array))
			    {
					for(i = 0, oid_node = view_array[0].view_included.oidHead;
						i < view_array[0].view_included.oid_num && NULL != oid_node;
						i++, oid_node = oid_node->next)
					{
						fprintf(cgiOut, "%s;\n",oid_node->oid);
					}
			    }
			  	fprintf(cgiOut,"</textarea>"\
			  "</td>"\
			  "<td><font color=red>(%s%d%s)</font></td>",search(lsnmpd,"input_oid1"),MAX_VIEW_INEX_NUM,search(lsnmpd,"input_oid2"));
			fprintf(cgiOut,"</tr>"\
			"<tr height=30 style=\"padding-top:10px\">"\
			  "<td>%s:</td>",search(lsnmpd,"view_excluded"));
			  fprintf(cgiOut,"<td><textarea name=excluded ROWS=7 COLS=35>");
			    if(AC_MANAGE_SUCCESS == ret)
			    {
					for(i = 0, oid_node = view_array[0].view_excluded.oidHead;
						i < view_array[0].view_excluded.oid_num && NULL != oid_node;
						i++, oid_node = oid_node->next)
					{
						fprintf(cgiOut, "%s;\n",oid_node->oid);
					}
			    }
			    fprintf(cgiOut,"</textarea>"\
			  "</td>"\
			  "<td><font color=red>(%s%d%s)</font></td>",search(lsnmpd,"input_oid1"),MAX_VIEW_INEX_NUM,search(lsnmpd,"input_oid2"));
			fprintf(cgiOut,"</tr>");
		}
		fprintf(cgiOut,"<tr>"\
	 		"<td><input type=hidden name=encry_add_view value=%s></td>",m);
			fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
			fprintf(cgiOut,"<td><input type=hidden name=AddFlag value=%d></td>",add_flag);
		fprintf(cgiOut,"</tr>"\
		"<tr>"\
	 		"<td colspan=3><input type=hidden name=SLOT_ID value=%s></td>",select_slotid);
		fprintf(cgiOut,"</tr>");
		if(0 == add_flag)
		{
			fprintf(cgiOut,"<tr>"\
				"<td colspan=3><input type=hidden name=ViewName value=%s></td>",ViewName);
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
if(AC_MANAGE_SUCCESS == ret)
{
	free_ac_manage_show_snmp_view(&view_array, view_num);
}
free_instance_parameter_list(&paraHead);
}

void AddorConfigView(int add_flag,DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd)
{
	int ret1 = AC_MANAGE_DBUS_ERROR;
	int ret2 = AC_MANAGE_DBUS_ERROR;
	int ret3 = AC_MANAGE_DBUS_ERROR;
	char view_name[40] = { 0 };
	int ret_c = 0;
	int included_length = 0;
	int excluded_length = 0;
	STSNMPView *view_array = NULL;
    unsigned int view_num = 0;
    struct oid_list *oid_node = NULL;
	
	memset(view_name,0,sizeof(view_name));
	cgiFormStringNoNewlines("view_name", view_name, 40);

	if(1 == add_flag)
	{		
		if(strcmp(view_name,"")==0)
		{
			ShowAlert(search(lsnmpd,"view_name_not_null"));
			return;
		}
		
		ret_c = check_snmp_name(view_name, 0);
		if((ret_c == -2)||(ret_c == -1)) 
		{
			ShowAlert(search(lsnmpd,"view_lenth_illegal"));
			return;
		}
		else if(ret_c == -3)
		{
			ShowAlert(search(lsnmpd,"view_illegal1"));
			return;
		}
		else if(ret_c == -4) 
		{
			ShowAlert(search(lsnmpd,"view_illegal2"));
			return;
		}
		else if(ret_c == -5)
		{
			ShowAlert(search(lsnmpd,"view_illegal3"));
			return;
		}			
		
		ret1 = ac_manage_config_snmp_view(select_connection, view_name, 1);		
		if(AC_MANAGE_SUCCESS == ret1) 
		{
			ShowAlert(search(lsnmpd,"add_view_succ"));
			return;
		}
		else if(AC_MANAGE_DBUS_ERROR == ret1) 
		{
			ShowAlert(search(lsnmpd,"add_view_fail"));
			return;
		}
		else if(AC_MANAGE_SERVICE_ENABLE == ret1) 
		{
			ShowAlert(search(lsnmpd,"disable_snmp_service"));
			return;
		}
		else if(AC_MANAGE_CONFIG_EXIST == ret1)
		{
			ShowAlert(search(lsnmpd,"view_exist"));
			return;
		}
		else 
		{
			ShowAlert(search(lsnmpd,"add_view_fail"));
			return;
		}
	}
	else if((0 == add_flag)&&(view_name))
	{
		int i = 0;
		unsigned int oid_type = 0;
		unsigned int mode = 0;
		char *included = NULL;
		char *excluded = NULL;
		char *start = NULL;
		char *end = NULL; 
		char oid[128] = { 0 };
			
		ret2 = ac_manage_show_snmp_view(select_connection, &view_array, &view_num, view_name);	
		if((AC_MANAGE_SUCCESS == ret2)&&(view_array))
		{
	        mode = 0;/*delete*/
	        oid_type = 1;/*view-included*/
			
			for(i = 0, oid_node = view_array[0].view_included.oidHead;
				i < view_array[0].view_included.oid_num && NULL != oid_node;
				i++, oid_node = oid_node->next)
			{
				ret3 = AC_MANAGE_DBUS_ERROR;
				ret3 = ac_manage_config_snmp_view_oid(select_connection, view_name, oid_node->oid, oid_type, mode);
				if(AC_MANAGE_SERVICE_ENABLE == ret3)
				{
					ShowAlert(search(lsnmpd,"disable_snmp_service"));
					return;
				}
			}	

			oid_type = 0;/*view-excluded*/
			for(i = 0, oid_node = view_array[0].view_excluded.oidHead;
			    i < view_array[0].view_excluded.oid_num && NULL != oid_node;
			    i++, oid_node = oid_node->next)
			{
				ret3 = AC_MANAGE_DBUS_ERROR;
				ret3 = ac_manage_config_snmp_view_oid(select_connection, view_name, oid_node->oid, oid_type, mode);
				if(AC_MANAGE_SERVICE_ENABLE == ret3)
				{
					ShowAlert(search(lsnmpd,"disable_snmp_service"));
					return;
				}
			}	
				
			free_ac_manage_show_snmp_view(&view_array, view_num);
		}
		
		cgiFormStringSpaceNeeded( "included", &included_length);/*获取的值为included输入框内输入字节数+1*/
		included = (char *)malloc(included_length);
		if(NULL == included)
			return;
		memset(included, 0, included_length);
		cgiFormStringNoNewlines("included", included, included_length);	

        mode = 1;/*add*/
        oid_type = 1;/*view-included*/
		start = included;
		end = included;
		for(i = 0; i< included_length && (end!=NULL); i++, end++)
		{
			if(';' == *end)
			{
				memset(oid,0,sizeof(oid)-1);
				strncpy(oid, start, end-start);
				ret3 = AC_MANAGE_DBUS_ERROR;
				ret3 = ac_manage_config_snmp_view_oid(select_connection, view_name, oid, oid_type, mode);
				if(AC_MANAGE_CONFIG_EXIST == ret3) 
				{
			        ShowAlert(search(lsnmpd,"oid_exist"));
			    }
				else if(AC_MANAGE_CONFIG_REACH_MAX_NUM == ret3) 
				{
					ShowAlert(search(lsnmpd,"oid_over_max_num"));
					return;
				}
				else if(AC_MANAGE_SERVICE_ENABLE == ret3)
				{
					ShowAlert(search(lsnmpd,"disable_snmp_service"));
					return;
				}
				start = end +1;
			}
		}

		
		cgiFormStringSpaceNeeded( "excluded", &excluded_length);/*获取的值为excluded输入框内输入字节数+1*/
		excluded = (char *)malloc(excluded_length);
		if(NULL == excluded)
			return;
		memset(excluded, 0, excluded_length);
		cgiFormStringNoNewlines("excluded", excluded, excluded_length);
		
		oid_type = 0;/*view-excluded*/
		start = excluded;
		end = excluded;
		for(i = 0; i< excluded_length && (end!=NULL); i++, end++)
		{
			if(';' == *end)
			{
				memset(oid,0,sizeof(oid)-1);
				strncpy(oid, start, end-start);
				ret3 = AC_MANAGE_DBUS_ERROR;
				ret3 = ac_manage_config_snmp_view_oid(select_connection, view_name, oid, oid_type, mode);
				if(AC_MANAGE_CONFIG_EXIST == ret3) 
				{
			        ShowAlert(search(lsnmpd,"oid_exist"));
			    }
				else if(AC_MANAGE_CONFIG_REACH_MAX_NUM == ret3) 
				{
					ShowAlert(search(lsnmpd,"oid_over_max_num"));
					return;
				}
				else if(AC_MANAGE_SERVICE_ENABLE == ret3)
				{
					ShowAlert(search(lsnmpd,"disable_snmp_service"));
					return;
				}
				start = end +1;
			}
		}
		
		FREE_OBJECT(included);
		FREE_OBJECT(excluded);
	}
}

