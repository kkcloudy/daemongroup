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
* wp_wqosnew.c
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
#include "wcpss/wid/WID.h"
#include "ws_dcli_wqos.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


int ShowWqosnewPage(char *m,struct list *lpublic,struct list *lwlan);    
void NewWqos(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);

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
    cgiFormStringNoNewlines("encry_newwqos",encry,BUF_LEN);
  }
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
    ShowWqosnewPage(encry,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWqosnewPage(char *m,struct list *lpublic,struct list *lwlan)
{  
  int i = 0;
  char wid_scope[20] = { 0 };
  int wid_len = 0;
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WQOS</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
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
  
  if(cgiFormSubmitClicked("wqosnew_apply") == cgiFormSuccess)
  {
  	if(paraHead1)
	{
		NewWqos(paraHead1,lpublic,lwlan);
	}
  }
	fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=%s>%s</font><font id=titleen>QOS</font></td>",search(lpublic,"title_style"),search(lwlan,"wireless"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=wqosnew_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));			  
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
                  "</tr>"\
				  "<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_wqoslis.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>QOS</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"wireless"),search(lpublic,"menu_san"),search(lpublic,"list"));                       
                  fprintf(cgiOut,"</tr>"\
  				  "<tr height=26>");
				  if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
                    fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=%s> %s</font><font id=yingwen_san>QOS</font></td>",search(lpublic,"menu_san"),search(lpublic,"create"),search(lpublic,"menu_san"),search(lwlan,"wireless"));   /*突出显示*/
				  else
					fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=%s>%s</font><font id=yingwen_san>QOS</font></td>",search(lpublic,"menu_san"),search(lpublic,"create"),search(lpublic,"menu_san"),search(lwlan,"wireless"));   /*突出显示*/
				  fprintf(cgiOut,"</tr>");
                  for(i=0;i<1;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
              "<table width=500 border=0 cellspacing=0 cellpadding=0>"\
   "<tr height=30>"\
    "<td>%s ID:</td>",search(lpublic,"instance"));
    fprintf(cgiOut,"<td align=left colspan=2>"\
		"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_wqosnew.cgi\",\"%s\")>",m);	
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
    "<td width=70>WQOS ID:</td>");
	memset(wid_scope,0,sizeof(wid_scope));
    snprintf(wid_scope,sizeof(wid_scope)-1,"%d",QOS_NUM-1);
    wid_len=strlen(wid_scope);
    fprintf(cgiOut,"<td width=260 align=left><input type=text name=wqos_id size=40 maxLength=%d onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>",wid_len);
    fprintf(cgiOut,"<td width=170><font color=red>(1--%d)</font></td>",QOS_NUM-1);
  fprintf(cgiOut,"</tr>"\
  "<tr height=30>"\
    "<td>WQOS %s:</td>",search(lpublic,"name"));
    fprintf(cgiOut,"<td align=left><input type=text name=wqos_name size=40 maxLength=15 onkeypress=\"return event.keyCode!=32\"></td>");
      fprintf(cgiOut,"<td align=left><font color=red>(%s%d%s)</font></td>",search(lwlan,"most1"),15,search(lwlan,"most2"));
  fprintf(cgiOut,"</tr>"\
  "<tr>"\
    "<td><input type=hidden name=encry_newwqos value=%s></td>",m);
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


void NewWqos(instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{
  int id = 0,ret = 0;  
  char *endptr = NULL;  
  char ID[20] = { 0 };
  char wqos_name[20] = { 0 };
  char temp[100] = { 0 };
  char wqos_num[5] = { 0 };
  
  memset(ID,0,sizeof(ID));
  cgiFormStringNoNewlines("wqos_id",ID,20);   
  memset(wqos_name,0,sizeof(wqos_name));
  cgiFormStringNoNewlines("wqos_name",wqos_name,20);  
  if(strcmp(ID,"")!=0)                             /*wlan id不能为空*/
  {
	id= strtoul(ID,&endptr,10);					   /*char转成int，10代表十进制*/		
	if((id>0)&&(id<QOS_NUM))           /*最多WLAN_NUM个wlan*/   
	{
      if(strcmp(wqos_name,"")!=0)            /*wlan name不能为空*/
      {
      	if(strchr(wqos_name,' ')==NULL)     /*不包含空格*/
      	{
			if(strlen(wqos_name)>15)
			  ShowAlert(search(lpublic,"input_too_long"));
			else
			{
			  ret=create_qos(ins_para->parameter,ins_para->connection,ID,wqos_name);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示qos id should be 1 to QOS_NUM-1*/
																		  	  	   /*返回-3表示qos name is too long,it should be 1 to 15，返回-4表示qos id exist，返回-5表示error*/
			  switch(ret)
			  {
			    case SNMPD_CONNECTION_ERROR:
				case 0:ShowAlert(search(lpublic,"create_fail"));
					   break;
				case 1:ShowAlert(search(lpublic,"create_success"));
					   break;
				case -1:ShowAlert(search(lpublic,"unknown_id_format"));
						break;
				case -2:{
						  memset(temp,0,sizeof(temp));
						  strncpy(temp,"WQOS ID",sizeof(temp)-1);
						  strncat(temp,search(lwlan,"wqos_id_illegal"),sizeof(temp)-strlen(temp)-1);
						  memset(wqos_num,0,sizeof(wqos_num));
						  snprintf(wqos_num,sizeof(wqos_num)-1,"%d",QOS_NUM-1);   /*int转成char*/
						  strncat(temp,wqos_num,sizeof(temp)-strlen(temp)-1);
						  strncat(temp,search(lwlan,"bss_id_2"),sizeof(temp)-strlen(temp)-1);
						  ShowAlert(temp);
						  break;
						}
				case -3:ShowAlert(search(lwlan,"wqos_name_too_long"));
						break;			
				case -4:ShowAlert(search(lwlan,"wqos_exist"));
						break;
				case -5:ShowAlert(search(lpublic,"error"));
						break;
			  }   
			}
      	}
		else
		  ShowAlert(search(lpublic,"input_para_dont_contain_spaces"));
      }
      else
	    ShowAlert(search(lwlan,"wqos_name_not_null"));
	}
	else
	{
	  memset(temp,0,sizeof(temp));
	  strncpy(temp,"WQOS ID",sizeof(temp)-1);
	  strncat(temp,search(lwlan,"wqos_id_illegal"),sizeof(temp)-strlen(temp)-1);
	  memset(wqos_num,0,sizeof(wqos_num));
	  snprintf(wqos_num,sizeof(wqos_num)-1,"%d",QOS_NUM-1);   /*int转成char*/
	  strncat(temp,wqos_num,sizeof(temp)-strlen(temp)-1);
	  strncat(temp,search(lwlan,"bss_id_2"),sizeof(temp)-strlen(temp)-1);
	  ShowAlert(temp);
	}
  }
  else
	ShowAlert(search(lwlan,"wqos_id_not_null"));  
}


