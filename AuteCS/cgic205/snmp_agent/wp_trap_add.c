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
* wp_trap_add.c
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


void ShowAddTrapPage(char *m,int add_flag,char *TrapID,char *is_bind_sour_ip,struct list *lpublic,struct list *lsnmpd);  
void AddorModifyTrap(int add_flag,int sid,DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;   
  char AddFlag[5] = { 0 };  
  char TrapID[10] = { 0 };
  char is_bind_sour_ip[10] = { 0 };  
  int add_flag = 1;	/*1表示添加Trap，0表示修改Trap*/
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsnmpd = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsnmpd=get_chain_head("../htdocs/text/snmpd.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(AddFlag,0,sizeof(AddFlag));
  memset(TrapID,0,sizeof(TrapID));
  memset(is_bind_sour_ip,0,sizeof(is_bind_sour_ip));
  
  cgiFormStringNoNewlines("AddFlag", AddFlag, BUF_LEN);
  if(strcmp(AddFlag,"0") == 0)
  	add_flag = 0;  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {  	
	cgiFormStringNoNewlines("TrapID", TrapID, 10);  	
	if(cgiFormStringNoNewlines("Is_bind_sour_ip",is_bind_sour_ip,10) == cgiFormNotFound)
	{
		strncpy(is_bind_sour_ip,"0",sizeof(is_bind_sour_ip)-1);
	}
  }
  else
  {
    cgiFormStringNoNewlines("encry_add_trap",encry,BUF_LEN);
	cgiFormStringNoNewlines("TrapID", TrapID, 10);  	
	cgiFormStringNoNewlines("is_bind_sour_ip",is_bind_sour_ip,10);	
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowAddTrapPage(encry,add_flag,TrapID,is_bind_sour_ip,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowAddTrapPage(char *m,int add_flag,char *TrapID,char *is_bind_sour_ip,struct list *lpublic,struct list *lsnmpd)
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
  STSNMPTrapReceiver *receiver_array = NULL;
  unsigned int receiver_num = 0;
  int trap_id = -1;
  char *tem = NULL;
  char ip1[5] = { 0 };
  char ip2[5] = { 0 };
  char ip3[5] = { 0 };
  char ip4[5] = { 0 };  

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>TRAP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  "</style>"\
  "</head>"\
  "<script type='text/javascript'>"\
	"function ChangeState(){"\
		"var no_bind_sour_ip = document.getElementsByName('is_bind_sour_ip')[0];"\
		"var bind_sour_ip = document.getElementsByName('is_bind_sour_ip')[1];"\
		"var sour_ipaddr = document.getElementById('sour_ipaddr');"\
		"var sour_ipaddr_tr = document.getElementById('sour_ipaddr_tr');"\
		"if( no_bind_sour_ip.checked == true)"\
		"{"\
			"sour_ipaddr.style.display = \"none\";"\
			"sour_ipaddr_tr.style.display = \"none\";"\
		"}"\
		"else if( bind_sour_ip.checked == true)"\
		"{"\
			"sour_ipaddr.style.display = \"block\";"\
			"sour_ipaddr_tr.style.display = \"block\";"\
		"}"\		 
		"var trap_version_index = document.all.trap_version.selectedIndex;"\
		"var trap_community = document.getElementById('trap_community');"\
		"if(trap_version_index == 3)"\
  		"{"\
  			"trap_community.style.display = \"none\";"\
  			"trap_community_tr.style.display = \"none\";"\
  		"}"\
  		"else"\
	    "{"\
		    "trap_community.style.display = \"block\";"\
		    "trap_community_tr.style.display = \"block\";"\
	    "}"\
	"}"\
  "</script>"\
  "<script src=/slotid_onchange.js>"\
  "</script>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<body onLoad=\"ChangeState();\">");
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
  if(0 == add_flag)/*修改共同体*/
  {
	  trap_id=strtoul(TrapID,&endptr,10);
  }

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("add_mod_trap_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	 AddorModifyTrap(add_flag,slot_id,select_connection,lpublic,lsnmpd);
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>TRAP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=add_mod_trap_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
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
					"<td align=left id=tdleft><a href=wp_trap_summary.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"info"));
				  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_config.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> TRAP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config"));
                  fprintf(cgiOut,"</tr>");				  
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_list.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=26>");
				  	if(1 == add_flag)
                      fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s </font><font id=yingwen_san>TRAP</font></td>",search(lpublic,"menu_san"),search(lpublic,"ntp_add"));   /*突出显示*/
					else
					  fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s </font><font id=yingwen_san>TRAP</font></td>",search(lpublic,"menu_san"),search(lpublic,"log_mod"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");	 
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_trap_detail.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>TRAP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"switch"));
                  fprintf(cgiOut,"</tr>");

			      ret = ac_manage_show_trap_receiver(select_connection, &receiver_array, &receiver_num);
				  limit = 6;

				  fprintf(cgiOut,"<tr height=25 id=sour_ipaddr_tr style=\"display:none\">"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
				  fprintf(cgiOut,"<tr height=25 id=trap_community_tr style=\"display:block\">"\
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
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
	"<table width=770 border=0 cellspacing=0 cellpadding=0>");
		fprintf(cgiOut,"<tr height=30>"\
		  "<td>SLOT ID:</td>"\
		  "<td colspan=2>"\
			  "<select name=slot_id id=slot_id style=width:100px onchange=slotid_change(this,\"wp_trap_add.cgi\",\"%s\")>",m);
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
		"<tr height=30>");
			if(strcmp(is_bind_sour_ip,"0") == 0)
			{
				fprintf(cgiOut,"<td colspan=3>"\
					"<input name=is_bind_sour_ip type=radio value=0 checked=checked onclick=\"ChangeState()\">%s"\
					"<input name=is_bind_sour_ip type=radio value=1 onclick=\"ChangeState()\">%s"
				"</td>",search(lsnmpd,"no_bind_sour_ip"),search(lsnmpd,"bind_sour_ip"));
			}
			else
			{
				fprintf(cgiOut,"<td colspan=3>"\
					"<input name=is_bind_sour_ip type=radio value=0 onclick=\"ChangeState()\">%s"\
					"<input name=is_bind_sour_ip type=radio value=1 checked=checked onclick=\"ChangeState()\">%s"
				"</td>",search(lsnmpd,"no_bind_sour_ip"),search(lsnmpd,"bind_sour_ip"));
			}
		fprintf(cgiOut,"</tr>"
		"<tr height=30>"\
			"<td width=70>%s:</td>",search(lsnmpd,"trap_name"));
			if(1 == add_flag)
			{
				fprintf(cgiOut,"<td width=100><input name=trap_name size=21 maxLength=30></td>");
			}
			else if(receiver_array)
			{
				fprintf(cgiOut,"<td width=100><input name=trap_name size=21 maxLength=30 value=%s></td>",receiver_array[trap_id].name);
			}
			fprintf(cgiOut,"<td width=600><font color=red>(%s)</font></td>",search(lsnmpd,"trap_name_range"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"hansi_type"));
			fprintf(cgiOut,"<td colspan=2><select name=hansi_type id=hansi_type style=width:100px>");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<option value=>"\
					"<option value=\"local-hansi\">local-hansi"\
					"<option value=\"remote-hansi\">remote-hansi");
				}
				else
				{
					if((AC_MANAGE_SUCCESS == ret)&&(receiver_array)&&(receiver_array[trap_id].local_id))
					{
						fprintf(cgiOut,"<option value=\"local-hansi\" selected=selected>local-hansi"\
						"<option value=\"remote-hansi\">remote-hansi");
					}
					else
					{
						fprintf(cgiOut,"<option value=\"local-hansi\">local-hansi"\
						"<option value=\"remote-hansi\" selected=selected>remote-hansi");
					}
				}
			fprintf(cgiOut,"</select></td>"\
		"</tr>"\
		"<tr height=30>"\
			"<td>%s ID:</td>",search(lpublic,"instance"));
			if(1 == add_flag)
			{
				fprintf(cgiOut,"<td><input name=instance_id size=21 maxLength=20></td>");
			}
			else if(receiver_array)
			{
				fprintf(cgiOut,"<td><input name=instance_id size=21 maxLength=20 value=%d></td>",receiver_array[trap_id].instance_id);
			}
			fprintf(cgiOut,"<td width=600><font color=red>(%s)</font></td>",search(lsnmpd,"instance_descript"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"trap_version"));
			fprintf(cgiOut,"<td colspan=2><select name=trap_version id=trap_version style=width:100px onchange=\"ChangeState()\">");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<option value=>"\
					"<option value=v1>v1"\
					"<option value=v2c>v2c"\
					"<option value=v3>v3");
				}
				else
				{
					if((AC_MANAGE_SUCCESS == ret)&&(receiver_array)&&(V2 == receiver_array[trap_id].version))
					{
						fprintf(cgiOut,"<option value=>"\
						"<option value=v1>v1"\
						"<option value=v2c selected=selected>v2c"\
						"<option value=v3>v3");
					}
					else if((AC_MANAGE_SUCCESS == ret)&&(receiver_array)&&(V3 == receiver_array[trap_id].version))
					{
						fprintf(cgiOut,"<option value=>"\
						"<option value=v1>v1"\
						"<option value=v2c>v2c"\
						"<option value=v3 selected=selected>v3");
					}
					else
					{
						fprintf(cgiOut,"<option value=>"\
						"<option value=v1 selected=selected>v1"\
						"<option value=v2c>v2c"\
						"<option value=v3>v3");
					}
				}
			fprintf(cgiOut,"</select></td>"\
		"</tr>"\
		"<tr height=30 id=sour_ipaddr style=\"display:none\">"\
			"<td>%s:</td>",search(lsnmpd,"sour_ipaddr"));
			fprintf(cgiOut,"<td colspan=2>"\
				"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<input type=text name='sour_ipaddr1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='sour_ipaddr2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='sour_ipaddr3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='sour_ipaddr4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
				}
				else
				{
					memset(ip1,0,sizeof(ip1));
					memset(ip2,0,sizeof(ip2));
					memset(ip3,0,sizeof(ip3));
					memset(ip4,0,sizeof(ip4));
					i=0;
					if((AC_MANAGE_SUCCESS == ret)&&(receiver_array))
					{
						if(receiver_array[trap_id].sour_ipAddr[0])
						{
							tem = strtok(receiver_array[trap_id].sour_ipAddr,".");
							while(tem != NULL)
							{
							  i++;
							  if(i==1)
								  strncpy(ip1,tem,sizeof(ip1)-1);
							  else if(i ==2 )
								  strncpy(ip2,tem,sizeof(ip2)-1);
							  else if(i==3)
								  strncpy(ip3,tem,sizeof(ip3)-1);
							  else if(i==4)
								  strncpy(ip4,tem,sizeof(ip4)-1);
							  tem = strtok(NULL,".");	  
							}				   
						}
						else
						{
							strncpy(ip1,"0",sizeof(ip1)-1);
							strncpy(ip2,"0",sizeof(ip2)-1);
							strncpy(ip3,"0",sizeof(ip3)-1);
							strncpy(ip4,"0",sizeof(ip4)-1);
						}
					}
					fprintf(cgiOut,"<input type=text name='sour_ipaddr1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip1);
					fprintf(cgiOut,"<input type=text name='sour_ipaddr2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip2);
					fprintf(cgiOut,"<input type=text name='sour_ipaddr3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip3);
					fprintf(cgiOut,"<input type=text name='sour_ipaddr4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>",search(lpublic,"ip_error"),ip4);
				}
				fprintf(cgiOut,"</div>"\
			"</td>"\
		"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"dest_ipaddr"));
			fprintf(cgiOut,"<td colspan=2>"\
				"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<input type=text name='dest_ipaddr1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='dest_ipaddr2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='dest_ipaddr3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='dest_ipaddr4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
				}
				else
				{
					memset(ip1,0,sizeof(ip1));
					memset(ip2,0,sizeof(ip2));
					memset(ip3,0,sizeof(ip3));
					memset(ip4,0,sizeof(ip4));
					i=0;
					if((AC_MANAGE_SUCCESS == ret)&&(receiver_array))
					{
						tem = strtok(receiver_array[trap_id].dest_ipAddr,".");
						while(tem != NULL)
						{
						  i++;
						  if(i==1)
							  strncpy(ip1,tem,sizeof(ip1)-1);
						  else if(i ==2 )
							  strncpy(ip2,tem,sizeof(ip2)-1);
						  else if(i==3)
							  strncpy(ip3,tem,sizeof(ip3)-1);
						  else if(i==4)
							  strncpy(ip4,tem,sizeof(ip4)-1);
						  tem = strtok(NULL,".");	  
						}				   
					}
					fprintf(cgiOut,"<input type=text name='dest_ipaddr1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip1);
					fprintf(cgiOut,"<input type=text name='dest_ipaddr2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip2);
					fprintf(cgiOut,"<input type=text name='dest_ipaddr3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip3);
					fprintf(cgiOut,"<input type=text name='dest_ipaddr4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>",search(lpublic,"ip_error"),ip4);
				}
				fprintf(cgiOut,"</div>"\
			"</td>"\
		"</tr>");
		fprintf(cgiOut,"<tr height=30 id=trap_community style=\"display:none\">");
			fprintf(cgiOut,"<td>%s:</td>",search(lsnmpd,"trap_community"));
			if((0 == add_flag)&&((AC_MANAGE_SUCCESS == ret)&&(receiver_array)&&(V3 != receiver_array[trap_id].version)))
			{
				fprintf(cgiOut,"<td><input name=trap_community size=21 maxLength=20 value=%s></td>",receiver_array[trap_id].trapcom);
			}
			else
			{
				fprintf(cgiOut,"<td><input name=trap_community size=21 maxLength=20></td>");					
			}
			fprintf(cgiOut,"<td><font color=red>(%s)</font></td>",search(lsnmpd,"community_name_range"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"trap_state"));
			fprintf(cgiOut,"<td colspan=2><select name=trap_state id=trap_state style=width:100px>");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<option value=>"\
					"<option value=enable>enable"\
					"<option value=disable>disable");
				}
				else
				{
					if((AC_MANAGE_SUCCESS == ret)&&(receiver_array)&&(receiver_array[trap_id].status))
					{
						fprintf(cgiOut,"<option value=enable selected=selected>enable"\
						"<option value=disable>disable");
					}
					else
					{
						fprintf(cgiOut,"<option value=enable>enable"\
						"<option value=disable selected=selected>disable");
					}
				}
			fprintf(cgiOut,"</select></td>"\
		"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"dest_port"));
			if(1 == add_flag)
			{
				fprintf(cgiOut,"<td><input name=dest_port size=21 maxLength=20></td>");
			}
			else if(receiver_array)
			{
				fprintf(cgiOut,"<td><input name=dest_port size=21 maxLength=20 value=%d></td>",receiver_array[trap_id].dest_port ? receiver_array[trap_id].dest_port : 162);
			}
			fprintf(cgiOut,"<td><font color=red>(1-65535,%s)</font></td>",search(lsnmpd,"dest_port_descript"));
		fprintf(cgiOut,"</tr>"\
		"<tr>"\
	 		"<td><input type=hidden name=encry_add_trap value=%s></td>",m);
			fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
			fprintf(cgiOut,"<td><input type=hidden name=AddFlag value=%d></td>",add_flag);
		fprintf(cgiOut,"</tr>"\
		"<tr>"\
	 		"<td><input type=hidden name=is_bind_sour_ip value=%s></td>",is_bind_sour_ip);
			fprintf(cgiOut,"<td colspan=2><input type=hidden name=SLOT_ID value=%s></td>",select_slotid);
		fprintf(cgiOut,"</tr>");
		if(0 == add_flag)
		{
			fprintf(cgiOut,"<tr>"\
				"<td colspan=3><input type=hidden name=TrapID value=%d></td>",trap_id);
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
if(AC_MANAGE_SUCCESS == ret && receiver_array)
{
	MANAGE_FREE(receiver_array);
}
free_instance_parameter_list(&paraHead);
}


void AddorModifyTrap(int add_flag,int sid,DBusConnection *select_connection,struct list *lpublic,struct list *lsnmpd)
{
	int ret = AC_MANAGE_DBUS_ERROR;
	char is_bind_sour_ip[5] = { 0 };
	char trap_name[40] = { 0 };
	char hansi_type[15] = { 0 };
	char ins_id[5] = { 0 };
	char instance_id[10] = { 0 };
	char trap_version[10] = { 0 };	
	char sour_ipaddr[20] = { 0 };
	char ip1[4] = { 0 };
	char ip2[4] = { 0 };
	char ip3[4] = { 0 };
	char ip4[4] = { 0 };
	char dest_ipaddr[20] = { 0 };
	char trap_community[20] = { 0 };
	char trap_state[10] = { 0 };
	char dest_port[10] = { 0 };	
	
	unsigned int slot_id = 0;	 
	STSNMPTrapReceiver receiver;
	memset(&receiver, 0, sizeof(STSNMPTrapReceiver));
	unsigned int type = 0;	
	int ret_c = 0;

	memset(is_bind_sour_ip,0,sizeof(is_bind_sour_ip));
	cgiFormStringNoNewlines("is_bind_sour_ip", is_bind_sour_ip, 5);
	memset(trap_name,0,sizeof(trap_name));
	cgiFormStringNoNewlines("trap_name", trap_name, 40);
	memset(hansi_type,0,sizeof(hansi_type));
	cgiFormStringNoNewlines("hansi_type", hansi_type, 15);
	memset(ins_id,0,sizeof(ins_id));
	cgiFormStringNoNewlines("instance_id", ins_id, 5);
	if(strcmp(ins_id,""))
	{
		memset(instance_id,0,sizeof(instance_id));
		snprintf(instance_id,sizeof(instance_id)-1,"%d-%s",sid,ins_id);
	}
	memset(trap_version,0,sizeof(trap_version));
	cgiFormStringNoNewlines("trap_version", trap_version, 10);	
	memset(trap_community,0,sizeof(trap_community));
	cgiFormStringNoNewlines("trap_community", trap_community, 20);
	memset(trap_state,0,sizeof(trap_state));
	cgiFormStringNoNewlines("trap_state", trap_state, 10);
	memset(dest_port,0,sizeof(dest_port));
	cgiFormStringNoNewlines("dest_port", dest_port, 10);


	if(strcmp(is_bind_sour_ip,"0") == 0)/*无绑定源IP*/
	{
		memset(dest_ipaddr,0,sizeof(dest_ipaddr));                                 
	    memset(ip1,0,sizeof(ip1));
	    cgiFormStringNoNewlines("dest_ipaddr1",ip1,4);	
	    strncat(dest_ipaddr,ip1,sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
	    strncat(dest_ipaddr,".",sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
	    memset(ip2,0,sizeof(ip2));
	    cgiFormStringNoNewlines("dest_ipaddr2",ip2,4); 
	    strncat(dest_ipaddr,ip2,sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);	
	    strncat(dest_ipaddr,".",sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
	    memset(ip3,0,sizeof(ip3));
	    cgiFormStringNoNewlines("dest_ipaddr3",ip3,4); 
	    strncat(dest_ipaddr,ip3,sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);	
	    strncat(dest_ipaddr,".",sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
	    memset(ip4,0,sizeof(ip4));
	    cgiFormStringNoNewlines("dest_ipaddr4",ip4,4);
	    strncat(dest_ipaddr,ip4,sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
		if(!((strcmp(ip1,""))&&(strcmp(ip2,""))&&(strcmp(ip3,""))&&(strcmp(ip4,""))))
	    {
			ShowAlert(search(lsnmpd,"dest_ipaddr_not_null"));
			return;
	    }
	
		if(strcmp(trap_version,"v3"))/*非V3版本*/
		{
			if((strcmp(trap_name,""))&&(strcmp(hansi_type,""))&&(strcmp(instance_id,""))&&(strcmp(trap_version,""))&&(strcmp(trap_community,""))&&(strcmp(trap_state,"")))
			{			    
				type = add_flag;
			    if(1 != trap_name_is_legal_input(trap_name)) 
				{
					if(strlen(trap_name) > (MAX_SNMP_NAME_LEN - 1))
					{
						ShowAlert(search(lsnmpd,"trap_lenth_illegal"));
						return;
					}
					else
					{
						ShowAlert(search(lsnmpd,"trap_illegal"));
						return;
					}					
				}
			    strncpy(receiver.name, trap_name, sizeof(receiver.name) - 1);
			    
			    if(0 == strcmp(hansi_type, "local-hansi")) {
			        receiver.local_id= 1;
			    }
			    else if(0 == strcmp(hansi_type, "remote-hansi")) {
			        receiver.local_id = 0;
			    }
			    else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    
			    if(0 == sscanf(instance_id, "%d-%d", &slot_id, &(receiver.instance_id))) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    
			    if(slot_id < 1|| slot_id > 16) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    else if(NULL == select_connection) 
				{
					ShowAlert(search(lsnmpd,"slot_not_connect"));
					return;
				}
			    
			    if(receiver.instance_id < 1 || receiver.instance_id > 16)
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    
		        if(0 == strcmp(trap_version, "v1")) {
		            receiver.version = 1;
		        }
		        else if(0 == strcmp(trap_version, "v2c")) {
		            receiver.version = 2;
		        }
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        
		        if(1 != snmp_ipaddr_is_legal_input(dest_ipaddr)) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        strncpy(receiver.dest_ipAddr, dest_ipaddr, sizeof(receiver.dest_ipAddr) - 1);
		        
			    ret_c = check_snmp_name(trap_community, 1);
			    if((ret_c == -2)||(ret_c == -1)) 
				{
					ShowAlert(search(lsnmpd,"community_lenth_illegal"));
					return;
				}
			    else if(ret_c == -3)
				{
					ShowAlert(search(lsnmpd,"community_illegal1"));
					return;
				}
			    else if(ret_c == -4) 
				{
					ShowAlert(search(lsnmpd,"community_illegal2"));
					return;
				}
			    else if(ret_c == -5)
				{
					ShowAlert(search(lsnmpd,"community_illegal3"));
					return;
				}
		        strncpy(receiver.trapcom, trap_community, sizeof(receiver.trapcom) - 1);
		        
		        if(0 == strcmp(trap_state, "enable")) {
		            receiver.status = 1;
		        }
		        else if( 0 == strcmp(trap_state, "disable")) {
		            receiver.status = 0;
		        }
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

				if(strcmp(dest_port,""))
		        {
		            receiver.dest_port = atoi(dest_port);
		        }

	            ret = ac_manage_config_trap_config_receiver(select_connection, &receiver, type);
	            if(AC_MANAGE_SUCCESS == ret) 
				{
					if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_succ"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_succ"));
					}
					return;
				}
	            else if(AC_MANAGE_DBUS_ERROR == ret) 
				{
					if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_fail"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_fail"));
					}
					return;
				}
	            else if(AC_MANAGE_SERVICE_ENABLE == ret) 
				{
					ShowAlert(search(lsnmpd,"disable_trap_service"));
					return;
				}
	            else if(AC_MANAGE_CONFIG_EXIST == ret)
				{
					ShowAlert(search(lsnmpd,"trap_exist"));
					return;
				}
	            else if(AC_MANAGE_CONFIG_NONEXIST == ret) 
				{
					ShowAlert(search(lsnmpd,"trap_not_exist"));
					return;
				}
	            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) 
				{
					ShowAlert(search(lsnmpd,"input_type_error"));
					return;
				}
	            else
				{
					if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_fail"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_fail"));
					}
					return;
				}                                                  
			}
			else if(!((strcmp(trap_name,""))&&(strcmp(hansi_type,""))&&(strcmp(instance_id,""))&&(strcmp(trap_version,""))&&(strcmp(trap_community,""))&&(strcmp(trap_state,""))))
			{
				ShowAlert(search(lpublic,"para_incom"));
				return;
		    }
		}
		else	/*V3版本*/
		{
			if((strcmp(trap_name,""))&&(strcmp(hansi_type,""))&&(strcmp(instance_id,""))&&(strcmp(trap_version,""))&&(strcmp(trap_state,"")))
			{
				type = add_flag;
			    if(1 != trap_name_is_legal_input(trap_name)) 
				{
					if(strlen(trap_name) > (MAX_SNMP_NAME_LEN - 1))
					{
						ShowAlert(search(lsnmpd,"trap_lenth_illegal"));
						return;
					}
					else
					{
						ShowAlert(search(lsnmpd,"trap_illegal"));
						return;
					}					
				}
			    strncpy(receiver.name, trap_name, sizeof(receiver.name) - 1);
			    
			    if(0 == strcmp(hansi_type, "local-hansi")) {
			        receiver.local_id= 1;
			    }
			    else if(0 == strcmp(hansi_type, "remote-hansi")) {
			        receiver.local_id = 0;
			    }
			    else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    
			    if(0 == sscanf(instance_id, "%d-%d", &slot_id, &(receiver.instance_id))) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    
			    if(slot_id < 1|| slot_id > 16) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    else if(NULL == select_connection) 
				{
					ShowAlert(search(lsnmpd,"slot_not_connect"));
					return;
				}
			    
			    if(receiver.instance_id < 1 || receiver.instance_id > 16)
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

		        receiver.version = 3;

		        if(1 != snmp_ipaddr_is_legal_input(dest_ipaddr))
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        strncpy(receiver.dest_ipAddr, dest_ipaddr, sizeof(receiver.dest_ipAddr) - 1);
		        
		        if(0 == strcmp(trap_state, "enable")) {
		            receiver.status = 1;
		        }
		        else if( 0 == strcmp(trap_state, "disable")) {
		            receiver.status = 0;
		        }
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

		        if(strcmp(dest_port,""))
				{
		            receiver.dest_port = atoi(dest_port);			        
			    }

				ret = ac_manage_config_trap_config_receiver(select_connection, &receiver, type);
	            if(AC_MANAGE_SUCCESS == ret) 
				{
					if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_succ"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_succ"));
					}
					return;
				}
	            else if(AC_MANAGE_DBUS_ERROR == ret) 
				{
					if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_fail"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_fail"));
					}
					return;
				}
	            else if(AC_MANAGE_SERVICE_ENABLE == ret) 
				{
					ShowAlert(search(lsnmpd,"disable_trap_service"));
					return;
				}
	            else if(AC_MANAGE_CONFIG_EXIST == ret)
				{
					ShowAlert(search(lsnmpd,"trap_exist"));
					return;
				}
	            else if(AC_MANAGE_CONFIG_NONEXIST == ret) 
				{
					ShowAlert(search(lsnmpd,"trap_not_exist"));
					return;
				}
	            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) 
				{
					ShowAlert(search(lsnmpd,"input_type_error"));
					return;
				}
	            else
				{
					if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_fail"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_fail"));
					}
					return;
				}      
			}
			else if(!((strcmp(trap_name,""))&&(strcmp(hansi_type,""))&&(strcmp(instance_id,""))&&(strcmp(trap_version,""))&&(strcmp(trap_state,""))))
			{
				ShowAlert(search(lpublic,"para_incom"));
				return;
		    }
		}
	}
	else if(strcmp(is_bind_sour_ip,"1") == 0)/*绑定源IP*/
	{
		
		memset(sour_ipaddr,0,sizeof(sour_ipaddr));								   
		memset(ip1,0,sizeof(ip1));
		cgiFormStringNoNewlines("sour_ipaddr1",ip1,4);	
		strncat(sour_ipaddr,ip1,sizeof(sour_ipaddr)-strlen(sour_ipaddr)-1);
		strncat(sour_ipaddr,".",sizeof(sour_ipaddr)-strlen(sour_ipaddr)-1);
		memset(ip2,0,sizeof(ip2));
		cgiFormStringNoNewlines("sour_ipaddr2",ip2,4); 
		strncat(sour_ipaddr,ip2,sizeof(sour_ipaddr)-strlen(sour_ipaddr)-1); 
		strncat(sour_ipaddr,".",sizeof(sour_ipaddr)-strlen(sour_ipaddr)-1);
		memset(ip3,0,sizeof(ip3));
		cgiFormStringNoNewlines("sour_ipaddr3",ip3,4); 
		strncat(sour_ipaddr,ip3,sizeof(sour_ipaddr)-strlen(sour_ipaddr)-1); 
		strncat(sour_ipaddr,".",sizeof(sour_ipaddr)-strlen(sour_ipaddr)-1);
		memset(ip4,0,sizeof(ip4));
		cgiFormStringNoNewlines("sour_ipaddr4",ip4,4);
		strncat(sour_ipaddr,ip4,sizeof(sour_ipaddr)-strlen(sour_ipaddr)-1);
		if(!((strcmp(ip1,""))&&(strcmp(ip2,""))&&(strcmp(ip3,""))&&(strcmp(ip4,""))))
		{
			ShowAlert(search(lsnmpd,"sour_ipaddr_not_null"));
			return;
		}

		memset(dest_ipaddr,0,sizeof(dest_ipaddr));                                 
	    memset(ip1,0,sizeof(ip1));
	    cgiFormStringNoNewlines("dest_ipaddr1",ip1,4);	
	    strncat(dest_ipaddr,ip1,sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
	    strncat(dest_ipaddr,".",sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
	    memset(ip2,0,sizeof(ip2));
	    cgiFormStringNoNewlines("dest_ipaddr2",ip2,4); 
	    strncat(dest_ipaddr,ip2,sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);	
	    strncat(dest_ipaddr,".",sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
	    memset(ip3,0,sizeof(ip3));
	    cgiFormStringNoNewlines("dest_ipaddr3",ip3,4); 
	    strncat(dest_ipaddr,ip3,sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);	
	    strncat(dest_ipaddr,".",sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
	    memset(ip4,0,sizeof(ip4));
	    cgiFormStringNoNewlines("dest_ipaddr4",ip4,4);
	    strncat(dest_ipaddr,ip4,sizeof(dest_ipaddr)-strlen(dest_ipaddr)-1);
		if(!((strcmp(ip1,""))&&(strcmp(ip2,""))&&(strcmp(ip3,""))&&(strcmp(ip4,""))))
	    {
			ShowAlert(search(lsnmpd,"dest_ipaddr_not_null"));
			return;
	    }

		if(strcmp(trap_version,"v3"))/*非V3版本*/
		{
			if((strcmp(trap_name,""))&&(strcmp(hansi_type,""))&&(strcmp(instance_id,""))&&(strcmp(trap_version,""))&&(strcmp(trap_community,""))&&(strcmp(trap_state,"")))
			{    
			   	type = add_flag;
			    if(1 != trap_name_is_legal_input(trap_name))
				{
					if(strlen(trap_name) > (MAX_SNMP_NAME_LEN - 1))
					{
						ShowAlert(search(lsnmpd,"trap_lenth_illegal"));
						return;
					}
					else
					{
						ShowAlert(search(lsnmpd,"trap_illegal"));
						return;
					}			
			    }
			    strncpy(receiver.name, trap_name, sizeof(receiver.name) - 1);
			    
			    if(0 == strcmp(hansi_type, "local-hansi")) {
			        receiver.local_id = 1;
			    }
			    else if(0 == strcmp(hansi_type, "remote-hansi")) {
			        receiver.local_id = 0;
			    }
			    else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

			    if(0 == sscanf(instance_id, "%d-%d", &slot_id, &(receiver.instance_id))) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

			    if(slot_id < 1 || slot_id > 16) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    else if(NULL == select_connection) 
				{
					ShowAlert(search(lsnmpd,"slot_not_connect"));
					return;
				}
			    
			    if(receiver.instance_id < 1 || receiver.instance_id > 16) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    
		        if(0 == strcmp(trap_version, "v1")) {
		            receiver.version = 1;
		        }
		        else if(0 == strcmp(trap_version, "v2c")) {
		            receiver.version = 2;
		        }
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

		        if(1 != snmp_ipaddr_is_legal_input(sour_ipaddr)) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        strncpy(receiver.sour_ipAddr, sour_ipaddr, sizeof(receiver.sour_ipAddr) - 1);
		        
		        if(1 != snmp_ipaddr_is_legal_input(dest_ipaddr)) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        strncpy(receiver.dest_ipAddr, dest_ipaddr, sizeof(receiver.dest_ipAddr) - 1);
		        
				ret_c = check_snmp_name(trap_community, 1);
			    if((ret_c == -2)||(ret_c == -1)) 
				{
					ShowAlert(search(lsnmpd,"community_lenth_illegal"));
					return;
				}
			    else if(ret_c == -3)
				{
					ShowAlert(search(lsnmpd,"community_illegal1"));
					return;
				}
			    else if(ret_c == -4) 
				{
					ShowAlert(search(lsnmpd,"community_illegal2"));
					return;
				}
			    else if(ret_c == -5)
				{
					ShowAlert(search(lsnmpd,"community_illegal3"));
					return;
				}
		        strncpy(receiver.trapcom, trap_community, sizeof(receiver.trapcom) - 1);
		        
		        if(0 == strcmp(trap_state, "enable")) {
		            receiver.status = 1;
		        }
		        else if( 0 == strcmp(trap_state, "disable")) {
		            receiver.status = 0;
		        }
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

				if(strcmp(dest_port,""))
		        {
		            receiver.dest_port = atoi(dest_port);
		        }

	            ret = ac_manage_config_trap_config_receiver(select_connection, &receiver,type);
	            if(AC_MANAGE_SUCCESS == ret) 
				{
	                if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_succ"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_succ"));
					}
					return;
	            }
	            else if(AC_MANAGE_DBUS_ERROR == ret) 
				{
	                if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_fail"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_fail"));
					}
					return;
	            }
	            else if(AC_MANAGE_SERVICE_ENABLE == ret) 
				{
					ShowAlert(search(lsnmpd,"disable_trap_service"));
					return;
				}
	            else if(AC_MANAGE_CONFIG_EXIST == ret)
				{
					ShowAlert(search(lsnmpd,"trap_exist"));
					return;
				}
	            else if(AC_MANAGE_CONFIG_NONEXIST == ret) 
				{
					ShowAlert(search(lsnmpd,"trap_not_exist"));
					return;
				}
	            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) 
				{
					ShowAlert(search(lsnmpd,"input_type_error"));
					return;
				}
	            else
				{
	                if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_fail"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_fail"));
					}
					return;
	            }                                                  
			}
			else if(!((strcmp(trap_name,""))&&(strcmp(hansi_type,""))&&(strcmp(instance_id,""))&&(strcmp(trap_version,""))&&(strcmp(trap_community,""))&&(strcmp(trap_state,""))))
			{
				ShowAlert(search(lpublic,"para_incom"));
				return;
		    }
		}
		else	/*V3版本*/
		{
			if((strcmp(trap_name,""))&&(strcmp(hansi_type,""))&&(strcmp(instance_id,""))&&(strcmp(trap_version,""))&&(strcmp(trap_state,"")))
			{
				type = add_flag;
			    if(1 != trap_name_is_legal_input(trap_name))
				{
					if(strlen(trap_name) > (MAX_SNMP_NAME_LEN - 1))
					{
						ShowAlert(search(lsnmpd,"trap_lenth_illegal"));
						return;
					}
					else
					{
						ShowAlert(search(lsnmpd,"trap_illegal"));
						return;
					}			
			    }
			    strncpy(receiver.name, trap_name, sizeof(receiver.name) - 1);
			    
			    if(0 == strcmp(hansi_type, "local-hansi")) {
			        receiver.local_id = 1;
			    }
			    else if(0 == strcmp(hansi_type, "remote-hansi")) {
			        receiver.local_id = 0;
			    }
			    else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

			    if(0 == sscanf(instance_id, "%d-%d", &slot_id, &(receiver.instance_id))) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

			    if(slot_id < 1 || slot_id > 16) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
			    else if(NULL == select_connection) 
				{
					ShowAlert(search(lsnmpd,"slot_not_connect"));
					return;
				}
			    
			    if(receiver.instance_id < 1 || receiver.instance_id > 16) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

		        receiver.version = 3;

		        if(1 != snmp_ipaddr_is_legal_input(sour_ipaddr))
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        strncpy(receiver.sour_ipAddr, sour_ipaddr, sizeof(receiver.sour_ipAddr) - 1);
		        
		        if(1 != snmp_ipaddr_is_legal_input(dest_ipaddr)) 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}
		        strncpy(receiver.dest_ipAddr, dest_ipaddr, sizeof(receiver.dest_ipAddr) - 1);
		        
		        if(0 == strcmp(trap_state, "enable")) {
		            receiver.status = 1;
		        }
		        else if( 0 == strcmp(trap_state, "disable")) {
		            receiver.status = 0;
		        }        
		        else 
				{
					ShowAlert(search(lpublic,"input_para_illegal"));
					return;
				}

		        if(strcmp(dest_port,""))
				{
		            receiver.dest_port = atoi(dest_port);
		        }
				
				ret = ac_manage_config_trap_config_receiver(select_connection, &receiver,type);
	            if(AC_MANAGE_SUCCESS == ret) 
				{
	                if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_succ"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_succ"));
					}
					return;
	            }
	            else if(AC_MANAGE_DBUS_ERROR == ret) 
				{
	                if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_fail"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_fail"));
					}
					return;
	            }
	            else if(AC_MANAGE_SERVICE_ENABLE == ret) 
				{
					ShowAlert(search(lsnmpd,"disable_trap_service"));
					return;
				}
	            else if(AC_MANAGE_CONFIG_EXIST == ret)
				{
					ShowAlert(search(lsnmpd,"trap_exist"));
					return;
				}
	            else if(AC_MANAGE_CONFIG_NONEXIST == ret) 
				{
					ShowAlert(search(lsnmpd,"trap_not_exist"));
					return;
				}
	            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) 
				{
					ShowAlert(search(lsnmpd,"input_type_error"));
					return;
				}
	            else
				{
	                if(type)
					{
						ShowAlert(search(lsnmpd,"add_trap_fail"));
					}
					else
					{
						ShowAlert(search(lsnmpd,"mod_trap_fail"));
					}
					return;
	            }      
			}
			else if(!((strcmp(trap_name,""))&&(strcmp(hansi_type,""))&&(strcmp(instance_id,""))&&(strcmp(trap_version,""))&&(strcmp(trap_state,""))))
			{
				ShowAlert(search(lpublic,"para_incom"));
				return;
		    }
		}
	}	
}


