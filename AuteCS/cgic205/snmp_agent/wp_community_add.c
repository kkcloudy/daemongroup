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
* wp_community_add.c
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


void ShowAddCommunityPage(char *m,int add_flag,char *old_name,char *ComID,struct list *lpublic,struct list *lsnmpd);  
int AddorModifyCommunity(int cllection_mode,int add_flag,DBusConnection *select_connection,char *old_name,struct list *lpublic,struct list *lsnmpd);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;   
  char AddFlag[5] = { 0 };  
  char old_name[25] = { 0 };
  char ComID[10] = { 0 };
  int add_flag = 1;	/*1表示添加共同体，0表示修改共同体*/
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsnmpd = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsnmpd=get_chain_head("../htdocs/text/snmpd.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(AddFlag,0,sizeof(AddFlag));
  memset(old_name,0,sizeof(old_name));
  memset(ComID,0,sizeof(ComID));
  
  cgiFormStringNoNewlines("AddFlag", AddFlag, BUF_LEN);
  if(strcmp(AddFlag,"0") == 0)
  	add_flag = 0;  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {  	
	cgiFormStringNoNewlines("OldCommunityName", old_name, 25);
	cgiFormStringNoNewlines("CommunityID", ComID, 10);  	
  }
  else
  {
    cgiFormStringNoNewlines("encry_add_community",encry,BUF_LEN);
	cgiFormStringNoNewlines("old_name", old_name, 25);
	cgiFormStringNoNewlines("ComID", ComID, 10);  	
  }  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
  else
	ShowAddCommunityPage(encry,add_flag,old_name,ComID,lpublic,lsnmpd);
  release(lpublic);  
  release(lsnmpd);
  destroy_ccgi_dbus();
  return 0;
}

void ShowAddCommunityPage(char *m,int add_flag,char *old_name,char *ComID,struct list *lpublic,struct list *lsnmpd)
{    
  char IsSubmit[5] = { 0 };
  int i = 0,limit = 0;     
  int cllection_mode = 0;/*0表示关闭分板采集，1表示开启分板采集*/  
  char select_slotid[10] = { 0 };
  char temp[10] = { 0 };
  unsigned int slot_id = 0;
  DBusConnection *select_connection = NULL;
  instance_parameter *paraHead = NULL,*paraNode = NULL;
  char *endptr = NULL;	
  int ret = AC_MANAGE_DBUS_ERROR;
  STCommunity *community_array = NULL;
  unsigned int community_num = 0;
  int community_id = 0;
  char *tem = NULL;
  char ip1[5] = { 0 };
  char ip2[5] = { 0 };
  char ip3[5] = { 0 };
  char ip4[5] = { 0 };
  char mask1[5] = { 0 };
  char mask2[5] = { 0 };
  char mask3[5] = { 0 };
  char mask4[5] = { 0 };
  char CommunityName[25] = { 0 };
  int AddMod_ret = 0;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>SNMP</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  "</style>"\
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

  if(snmp_cllection_mode(ccgi_dbus_connection))
  {
	cllection_mode = 1;
  }
  else
  {
	cllection_mode = 0;
  }	
  if(1 == cllection_mode)/*开启分板采集*/
  {
  	  slot_id=strtoul(select_slotid,&endptr,10);
	  get_slot_dbus_connection(slot_id,&select_connection,SNMPD_INSTANCE_MASTER_V3);
  }
  if(0 == add_flag)/*修改共同体*/
  {
	  community_id=strtoul(ComID,&endptr,10);
  }

  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("add_mod_community_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	 AddMod_ret = AddorModifyCommunity(cllection_mode,add_flag,select_connection,old_name,lpublic,lsnmpd);
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>SNMP</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");        
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=add_mod_community_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
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
					"<td align=left id=tdleft><a href=wp_snmp_summary.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>SNMP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"info")); 					  
				  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_snmp_config.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> SNMP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config"));                       
                  fprintf(cgiOut,"</tr>");				  
				  fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_community_list.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsnmpd,"community_list"));                       
                  fprintf(cgiOut,"</tr>");
				  fprintf(cgiOut,"<tr height=26>");
				  	if(1 == add_flag)
                      fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsnmpd,"add_community"));   /*突出显示*/
					else
					  fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsnmpd,"mod_community"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");	 

				  ret = AC_MANAGE_DBUS_ERROR;
				  if(1 == cllection_mode)
			      {
			      	ret = ac_manage_show_snmp_community(select_connection, &community_array, &community_num);
					limit = 3;
				  }
				  else
				  {
				  	ret = ac_manage_show_snmp_community(ccgi_dbus_connection, &community_array, &community_num);
				  	limit = 2;
				  }

				  if(0 == add_flag)
				  {
					memset(CommunityName,0,sizeof(CommunityName));
					cgiFormStringNoNewlines("community_name", CommunityName, 25);
				  }

				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }			    
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
	"<table width=760 border=0 cellspacing=0 cellpadding=0>");
		if(1 == cllection_mode)/*开启分板采集*/
	    {
		  fprintf(cgiOut,"<tr height=30>"\
			  "<td>SLOT ID:</td>"\
			  "<td colspan=2>"\
				  "<select name=slot_id id=slot_id style=width:55px onchange=slotid_change(this,\"wp_community_add.cgi\",\"%s\")>",m);
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
		  "</tr>");
	    }
		fprintf(cgiOut,"<tr height=30>"\
			"<td width=60>%s:</td>",search(lsnmpd,"community_name"));
			if(1 == add_flag)
			{
				fprintf(cgiOut,"<td width=100><input name=community_name size=21 maxLength=20></td>");
			}
			else
			{
				if(1 == AddMod_ret)
					fprintf(cgiOut,"<td width=100><input name=community_name size=21 maxLength=20 value=%s></td>",CommunityName);
				else
					fprintf(cgiOut,"<td width=100><input name=community_name size=21 maxLength=20 value=%s></td>",old_name);
			}
			fprintf(cgiOut,"<td width=600><font color=red>(%s)</font></td>",search(lsnmpd,"community_name_range"));
		fprintf(cgiOut,"</tr>"\
		"<tr height=30>"\
			"<td>IP %s:</td>",search(lpublic,"addr"));
			fprintf(cgiOut,"<td colspan=2>"\
				"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<input type=text name='comm_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='comm_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='comm_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='comm_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
				}
				else
				{
					memset(ip1,0,sizeof(ip1));
					memset(ip2,0,sizeof(ip2));
					memset(ip3,0,sizeof(ip3));
					memset(ip4,0,sizeof(ip4));
					i=0;
					if((AC_MANAGE_SUCCESS == ret)&&(community_array))
					{
						tem = strtok(community_array[community_id].ip_addr,".");
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
					fprintf(cgiOut,"<input type=text name='comm_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip1);
					fprintf(cgiOut,"<input type=text name='comm_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip2);
					fprintf(cgiOut,"<input type=text name='comm_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),ip3);
					fprintf(cgiOut,"<input type=text name='comm_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>",search(lpublic,"ip_error"),ip4);
				}
				fprintf(cgiOut,"</div>"\
			"</td>"\
		"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lpublic,"netmask"));
			fprintf(cgiOut,"<td colspan=2>"\
				"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<input type=text name='comm_mask1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='comm_mask2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='comm_mask3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
					fprintf(cgiOut,"<input type=text name='comm_mask4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
				}
				else
				{
					memset(mask1,0,sizeof(mask1));
					memset(mask2,0,sizeof(mask2));
					memset(mask3,0,sizeof(mask3));
					memset(mask4,0,sizeof(mask4));
					i=0;
					if((AC_MANAGE_SUCCESS == ret)&&(community_array))
					{
						tem = strtok(community_array[community_id].ip_mask,".");
						while(tem != NULL)
						{
						  i++;
						  if(i==1)
							  strncpy(mask1,tem,sizeof(mask1)-1);
						  else if(i ==2 )
							  strncpy(mask2,tem,sizeof(mask2)-1);
						  else if(i==3)
							  strncpy(mask3,tem,sizeof(mask3)-1);
						  else if(i==4)
							  strncpy(mask4,tem,sizeof(mask4)-1);
						  tem = strtok(NULL,".");	  
						}				
					}
					fprintf(cgiOut,"<input type=text name='comm_mask1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),mask1);
					fprintf(cgiOut,"<input type=text name='comm_mask2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),mask2);
					fprintf(cgiOut,"<input type=text name='comm_mask3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>.",search(lpublic,"ip_error"),mask3);
					fprintf(cgiOut,"<input type=text name='comm_mask4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() value=%s>",search(lpublic,"ip_error"),mask4);
				}
				fprintf(cgiOut,"</div>"\
			"</td>"\
		"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"access_mode"));
			fprintf(cgiOut,"<td colspan=2><select name=access_mode id=access_mode style=width:55px>");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<option value=>"\
					"<option value=ro>%s",search(lsnmpd,"read_only"));
					fprintf(cgiOut,"<option value=rw>%s",search(lsnmpd,"read_write"));
				}
				else
				{
					if((AC_MANAGE_SUCCESS == ret)&&(community_array)&&(community_array[community_id].access_mode))
					{
						fprintf(cgiOut,"<option value=ro>%s",search(lsnmpd,"read_only"));
						fprintf(cgiOut,"<option value=rw selected=selected>%s",search(lsnmpd,"read_write"));
					}
					else
					{
						fprintf(cgiOut,"<option value=ro selected=selected>%s",search(lsnmpd,"read_only"));
						fprintf(cgiOut,"<option value=rw>%s",search(lsnmpd,"read_write"));
					}
				}
			fprintf(cgiOut,"</select></td>"\
		"</tr>"\
		"<tr height=30>"\
			"<td>%s:</td>",search(lsnmpd,"status"));
			fprintf(cgiOut,"<td colspan=2><select name=status id=status style=width:55px>");
				if(1 == add_flag)
				{
					fprintf(cgiOut,"<option value=>"\
					"<option value=enable>%s",search(lpublic,"version_enable"));
					fprintf(cgiOut,"<option value=disable>%s",search(lpublic,"version_disable"));
				}
				else
				{
					if((AC_MANAGE_SUCCESS == ret)&&(community_array)&&(community_array[community_id].status))
					{
						fprintf(cgiOut,"<option value=enable selected=selected>%s",search(lpublic,"version_enable"));
						fprintf(cgiOut,"<option value=disable>%s",search(lpublic,"version_disable"));
					}
					else
					{
						fprintf(cgiOut,"<option value=enable>%s",search(lpublic,"version_enable"));
						fprintf(cgiOut,"<option value=disable selected=selected>%s",search(lpublic,"version_disable"));
					}
				}
			fprintf(cgiOut,"</select></td>"\
		"</tr>"\
		"<tr>"\
		 	"<td><input type=hidden name=encry_add_community value=%s></td>",m);
			fprintf(cgiOut,"<td><input type=hidden name=SubmitFlag value=%d></td>",1);
			fprintf(cgiOut,"<td><input type=hidden name=AddFlag value=%d></td>",add_flag);
		fprintf(cgiOut,"</tr>"\
		"<tr>"\
		 	"<td colspan=3><input type=hidden name=SLOT_ID value=%s></td>",select_slotid);
		fprintf(cgiOut,"</tr>");
		if(0 == add_flag)
		{
			fprintf(cgiOut,"<tr>");
			if(1 == AddMod_ret)
				fprintf(cgiOut,"<td><input type=hidden name=old_name value=%s></td>",CommunityName);
			else
				fprintf(cgiOut,"<td><input type=hidden name=old_name value=%s></td>",old_name);
			fprintf(cgiOut,"<td colspan=2><input type=hidden name=ComID value=%d></td>",community_id);
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
	FREE_OBJECT(community_array);
}
free_instance_parameter_list(&paraHead);
}


int AddorModifyCommunity(int cllection_mode,int add_flag,DBusConnection *select_connection,char *old_name,struct list *lpublic,struct list *lsnmpd)
{
	int ret = AC_MANAGE_DBUS_ERROR;
	char CommunityName[25] = { 0 };
	char CommunityIp[20] = { 0 };
	char ip1[4] = { 0 };
	char ip2[4] = { 0 };
	char ip3[4] = { 0 };
	char ip4[4] = { 0 };
	char CommunityMask[20] = { 0 };
	char mask1[4] = { 0 };
	char mask2[4] = { 0 };
	char mask3[4] = { 0 };
	char mask4[4] = { 0 };
	char access_mode[5] = { 0 };
	char status[10] = { 0 };	
	int ret_c = 0;	
    int ret1 = 0, ret2 = 0;	
    STCommunity communityNode = { 0 };

	memset(CommunityName,0,sizeof(CommunityName));
	cgiFormStringNoNewlines("community_name", CommunityName, 25);
	if(strcmp(CommunityName,"")==0)
	{
		ShowAlert(search(lsnmpd,"community_name_not_null"));
		return 0;
    }	
	
	memset(CommunityIp,0,sizeof(CommunityIp));                                 
    memset(ip1,0,sizeof(ip1));
    cgiFormStringNoNewlines("comm_ip1",ip1,4);	
    strncat(CommunityIp,ip1,sizeof(CommunityIp)-strlen(CommunityIp)-1);
    strncat(CommunityIp,".",sizeof(CommunityIp)-strlen(CommunityIp)-1);
    memset(ip2,0,sizeof(ip2));
    cgiFormStringNoNewlines("comm_ip2",ip2,4); 
    strncat(CommunityIp,ip2,sizeof(CommunityIp)-strlen(CommunityIp)-1);	
    strncat(CommunityIp,".",sizeof(CommunityIp)-strlen(CommunityIp)-1);
    memset(ip3,0,sizeof(ip3));
    cgiFormStringNoNewlines("comm_ip3",ip3,4); 
    strncat(CommunityIp,ip3,sizeof(CommunityIp)-strlen(CommunityIp)-1);	
    strncat(CommunityIp,".",sizeof(CommunityIp)-strlen(CommunityIp)-1);
    memset(ip4,0,sizeof(ip4));
    cgiFormStringNoNewlines("comm_ip4",ip4,4);
    strncat(CommunityIp,ip4,sizeof(CommunityIp)-strlen(CommunityIp)-1);

	if(!((strcmp(ip1,""))&&(strcmp(ip2,""))&&(strcmp(ip3,""))&&(strcmp(ip4,""))))
    {
		ShowAlert(search(lpublic,"ip_not_null"));
		return 0;
    }

	memset(CommunityMask,0,sizeof(CommunityMask));                                 
    memset(mask1,0,sizeof(mask1));
    cgiFormStringNoNewlines("comm_mask1",mask1,4);	
    strncat(CommunityMask,mask1,sizeof(CommunityMask)-strlen(CommunityMask)-1);
    strncat(CommunityMask,".",sizeof(CommunityMask)-strlen(CommunityMask)-1);
    memset(mask2,0,sizeof(mask2));
    cgiFormStringNoNewlines("comm_mask2",mask2,4); 
    strncat(CommunityMask,mask2,sizeof(CommunityMask)-strlen(CommunityMask)-1);	
    strncat(CommunityMask,".",sizeof(CommunityMask)-strlen(CommunityMask)-1);
    memset(mask3,0,sizeof(mask3));
    cgiFormStringNoNewlines("comm_mask3",mask3,4); 
    strncat(CommunityMask,mask3,sizeof(CommunityMask)-strlen(CommunityMask)-1);	
    strncat(CommunityMask,".",sizeof(CommunityMask)-strlen(CommunityMask)-1);
    memset(mask4,0,sizeof(mask4));
    cgiFormStringNoNewlines("comm_mask4",mask4,4);
    strncat(CommunityMask,mask4,sizeof(CommunityMask)-strlen(CommunityMask)-1);
	
	if(!((strcmp(mask1,""))&&(strcmp(mask2,""))&&(strcmp(mask3,""))&&(strcmp(mask4,""))))
    {
		ShowAlert(search(lpublic,"mask_not_null"));
		return 0;
    }

	memset(access_mode,0,sizeof(access_mode));
	cgiFormStringNoNewlines("access_mode", access_mode, 5);
	if(strcmp(access_mode,"")==0)
	{
		ShowAlert(search(lsnmpd,"access_mode_not_null"));
		return 0;
    }

	memset(status,0,sizeof(status));
	cgiFormStringNoNewlines("status", status, 10);
	if(strcmp(status,"")==0)
	{
		ShowAlert(search(lsnmpd,"status_not_null"));
		return 0;
    }

	ret_c = check_snmp_name(CommunityName, 1);
	if((ret_c == -2)||(ret_c == -1)) 
	{
		ShowAlert(search(lsnmpd,"community_lenth_illegal"));
		return 0;
	}
	else if(ret_c == -3)
	{
		ShowAlert(search(lsnmpd,"community_illegal1"));
		return 0;
	}
	else if(ret_c == -4) 
	{
		ShowAlert(search(lsnmpd,"community_illegal2"));
		return 0;
	}
	else if(ret_c == -5)
	{
		ShowAlert(search(lsnmpd,"community_illegal3"));
		return 0;
	}
	
	ret1 = checkIpFormatValid((char *)CommunityIp);
	ret2 = checkIpFormatValid((char *)CommunityMask);
	if(ret1 || ret2) 
	{
		ShowAlert(search(lsnmpd,"ip_mask_dont_meet_format"));
		return 0;
	}
	
	strncpy(communityNode.community, CommunityName, sizeof(communityNode.community) - 1);
	strncpy(communityNode.ip_addr, CommunityIp, sizeof(communityNode.ip_addr) - 1);
	strncpy(communityNode.ip_mask, CommunityMask, sizeof(communityNode.ip_mask) - 1);	
	
	if(0 == strcmp(access_mode, "ro")) {
		communityNode.access_mode = 0;
	}
	else if(0 == strcmp(access_mode, "rw")) {
		communityNode.access_mode = 1;
	}
	else 
	{
		ShowAlert(search(lpublic,"input_para_illegal"));
		return 0;
	}
	
	if(0 == strcmp(status, "disable")) {
		communityNode.status = 0;
	}
	else if(0 == strcmp(status, "enable")) {
		communityNode.status = 1;
	}
	else
	{
		ShowAlert(search(lpublic,"input_para_illegal"));
		return 0;
	}

	if(1 == add_flag)/*添加共同体*/
	{		
		if(1 == cllection_mode) /*开启分板采集*/
		{
			ret = ac_manage_config_snmp_add_community(select_connection, &communityNode);
		}
		else
		{
			ret = ac_manage_config_snmp_add_community(ccgi_dbus_connection, &communityNode);
		}
		
		if(AC_MANAGE_SUCCESS == ret) 
		{
			ShowAlert(search(lsnmpd,"add_community_succ"));
			return 1;
		}
		else if(AC_MANAGE_DBUS_ERROR == ret) 
		{
			ShowAlert(search(lsnmpd,"add_community_fail"));
			return 0;
		}
		else if(AC_MANAGE_SERVICE_ENABLE == ret) 
		{
			ShowAlert(search(lsnmpd,"disable_snmp_service"));
			return 0;
		}
		else if(AC_MANAGE_CONFIG_EXIST == ret) 
		{
			ShowAlert(search(lsnmpd,"community_exist"));
			return 0;
		}
		else
		{
			ShowAlert(search(lsnmpd,"add_community_fail"));
			return 0;
		}
	}
	else/*修改共同体*/
	{
		ret_c = 0;
		ret_c = check_snmp_name(old_name, 1);
		if((ret_c == -2)||(ret_c == -1)) 
		{
			ShowAlert(search(lsnmpd,"community_lenth_illegal"));
			return 0;
		}
		else if(ret_c == -3)
		{
			ShowAlert(search(lsnmpd,"community_illegal1"));
			return 0;
		}
		else if(ret_c == -4) 
		{
			ShowAlert(search(lsnmpd,"community_illegal2"));
			return 0;
		}
		else if(ret_c == -5)
		{
			ShowAlert(search(lsnmpd,"community_illegal3"));
			return 0;
		}

		if(1 == cllection_mode) /*开启分板采集*/
		{
			ret = ac_manage_config_snmp_set_community(select_connection, old_name, &communityNode);
		}
		else
		{
			ret = ac_manage_config_snmp_set_community(ccgi_dbus_connection, old_name, &communityNode);
		}
		
		if(AC_MANAGE_SUCCESS == ret) 
		{
			ShowAlert(search(lsnmpd,"mod_community_succ"));
			return 1;
		}
		else if(AC_MANAGE_DBUS_ERROR == ret) 
		{
			ShowAlert(search(lsnmpd,"mod_community_fail"));
			return 0;
		}
		else if(AC_MANAGE_SERVICE_ENABLE == ret) 
		{
			ShowAlert(search(lsnmpd,"disable_snmp_service"));
			return 0;
		}
		else if(AC_MANAGE_CONFIG_NONEXIST == ret) 
		{
			ShowAlert(search(lsnmpd,"community_not_exist"));
			return 0;
		}
		else
		{
			ShowAlert(search(lsnmpd,"add_community_fail"));
		}
	}
}

