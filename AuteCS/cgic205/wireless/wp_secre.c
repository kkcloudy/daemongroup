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
* wp_secre.c
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "wcpss/asd/asd.h"
#include "ws_security.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

int ShowSecurityCreatePage(char *m,struct list *lpublic,struct list *lsecu); 
void Create_Security(instance_parameter *ins_para,struct list *lpublic,struct list *lsecu); 

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };			  
  char *str = NULL;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsecu = NULL;     /*解析security.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsecu=get_chain_head("../htdocs/text/security.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }
  else
  {
    cgiFormStringNoNewlines("encry_secre",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
    ShowSecurityCreatePage(encry,lpublic,lsecu);
  release(lpublic);  
  release(lsecu);
  destroy_ccgi_dbus();
  return 0;
}

int ShowSecurityCreatePage(char *m,struct list *lpublic,struct list *lsecu)
{ 
  int i = 0;
  char sid_scope[10] = { 0 };
  int sid_len = 0;
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lsecu,"wlan_sec"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
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
  if(cgiFormSubmitClicked("submit_secre") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		Create_Security(paraHead1,lpublic,lsecu);
	}
  }
  fprintf(cgiOut,"<form method=post >"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsecu,"wlan_sec"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=submit_secre style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                  "</tr>"\
				    "<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_seculis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"secur_list"));                       
                    fprintf(cgiOut,"</tr>"\
				    "<tr height=26>"\
                      "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsecu,"create_sec"));   /*突出显示*/
                    fprintf(cgiOut,"</tr>"\
                    "<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"config_sec"));                       
                    fprintf(cgiOut,"</tr>");
						 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secscon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"config_secon_sec"));                       
                    fprintf(cgiOut,"</tr>");

					 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_cer_dload.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"cer_up"));                       
                    fprintf(cgiOut,"</tr>");
				  for(i=0;i<0;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
				  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
                "<table width=400 border=0 cellspacing=0 cellpadding=0>"\
				   "<tr height=30>"\
					"<td>%s ID:</td>",search(lpublic,"instance"));
					fprintf(cgiOut,"<td align=left colspan=2>"\
						"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_secre.cgi\",\"%s\")>",m); 	
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
				   "<tr height=30>"\
					  "<td width=90>%s ID:</td>",search(lpublic,"security"));
				  	  memset(sid_scope,0,sizeof(sid_scope));
					  snprintf(sid_scope,sizeof(sid_scope)-1,"%d",WLAN_NUM-1);
					  sid_len=strlen(sid_scope);
					  fprintf(cgiOut,"<td width=140><input type=text name=sec_id size=21 maxLength=%d onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>",sid_len);
					  fprintf(cgiOut,"<td width=170><font color=red>(1--%d)</font></td>",WLAN_NUM-1);
					fprintf(cgiOut,"</tr>"\
					"<tr height=30>"\
					  "<td>%s:</td>",search(lsecu,"secur_name"));
					  fprintf(cgiOut,"<td><input type=text name=sec_name size=21 maxLength=15 onkeypress=\"return event.keyCode!=32\"></td>");
                        fprintf(cgiOut,"<td align=left><font color=red>(%s)</font></td>",search(lsecu,"most"));
					fprintf(cgiOut,"</tr>"\
					"<tr>"\
					  "<td><input type=hidden name=encry_secre value=%s></td>",m);					  
					  fprintf(cgiOut,"<td colspan=2><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
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
return 0;
}


void Create_Security(instance_parameter *ins_para,struct list *lpublic,struct list *lsecu)
{
  int id = 0,ret = 0;  
  char *endptr = NULL;  
  char ID[20] = { 0 };
  char secu_name[20] = { 0 };
  char alt[100] = { 0 };
  char max_sec_num[10] = { 0 };
  memset(ID,0,sizeof(ID));
  cgiFormStringNoNewlines("sec_id",ID,20);   
  memset(secu_name,0,sizeof(secu_name));
  cgiFormStringNoNewlines("sec_name",secu_name,20);    
  if(strcmp(ID,"")!=0)     /*security id不能为空*/
  {
	id= strtoul(ID,&endptr,10);					   /*char转成int，10代表十进制*/		
	if((id>0)&&(id<WLAN_NUM))         
	{
      if(strcmp(secu_name,"")!=0)            /*security name不能为空*/
      {
      	if(strchr(secu_name,' ')==NULL)	   /*不包含空格*/
      	{
			if(strlen(secu_name)>15)
			  ShowAlert(search(lpublic,"input_too_long"));
			else
			{
				ret=create_security(ins_para->parameter,ins_para->connection,ID,secu_name);	  /*返回0表示失败，返回1表示成功，返回-1表示security id非法，返回-2表示security ID existed，返回-3表示error*/
				switch(ret)
				{
				  case SNMPD_CONNECTION_ERROR:
				  case 0:ShowAlert(search(lpublic,"create_fail"));
						 break;
				  case 1:ShowAlert(search(lpublic,"create_success"));
						 break;
				  case -1:{
							memset(alt,0,sizeof(alt));
							strncpy(alt,search(lpublic,"secur_id_illegal1"),sizeof(alt)-1);
							memset(max_sec_num,0,sizeof(max_sec_num));
							snprintf(max_sec_num,sizeof(max_sec_num)-1,"%d",WLAN_NUM-1);
							strncat(alt,max_sec_num,sizeof(alt)-strlen(alt)-1);
							strncat(alt,search(lpublic,"secur_id_illegal2"),sizeof(alt)-strlen(alt)-1);
							ShowAlert(alt);
							break;
						  }
				  case -2:ShowAlert(search(lsecu,"secur_exist"));
						  break;
				  case -3:ShowAlert(search(lpublic,"error"));
						  break;
				}	
			}
      	}
		else
		  ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
      }
      else
		ShowAlert(search(lsecu,"secur_name_not_null"));
	}
	else
	{
		memset(alt,0,sizeof(alt));
		strncpy(alt,search(lpublic,"secur_id_illegal1"),sizeof(alt)-1);
		memset(max_sec_num,0,sizeof(max_sec_num));
		snprintf(max_sec_num,sizeof(max_sec_num)-1,"%d",WLAN_NUM-1);
		strncat(alt,max_sec_num,sizeof(alt)-strlen(alt)-1);
		strncat(alt,search(lpublic,"secur_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		ShowAlert(alt);
	}
  }
  else
	ShowAlert(search(lsecu,"secur_id_not_null"));
}


