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
* wp_wqoscon.c
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
#include "ws_dbus_list_interface.h"

char *wqos_service_type[] = {
	"besteffort",
	"background",
	"video",
	"voice"
};


int ShowWqosconPage(char *m,char *n,char *ins_id,char *radio_qos_type,char *client_qos_type,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);  /*m代表加密字符串，n代表wlan id，t代表wlan name ，r代表security id，intf代表interafce，Hessid代表Hideessid，s代表service state*/
void Config_Wqos(instance_parameter *ins_para,char *id,struct list *lpublic,struct list *lwlan); 

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;                
  char ID[5] = { 0 };
  char instance_id[10] = { 0 };
  char radio_qos_type[15] = { 0 };  
  char client_qos_type[15] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  memset(instance_id,0,sizeof(instance_id));
  memset(radio_qos_type,0,sizeof(radio_qos_type));
  memset(client_qos_type,0,sizeof(client_qos_type));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	cgiFormStringNoNewlines("ID", ID, 5);	
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }  
  else
  {  
    cgiFormStringNoNewlines("encry_conwqos",encry,BUF_LEN);
    cgiFormStringNoNewlines("wqos_id",ID,5);  
	cgiFormStringNoNewlines("instance_id",instance_id,10);  
	cgiFormStringNoNewlines("radio_qos_service",radio_qos_type,15);	
	cgiFormStringNoNewlines("client_qos_service",client_qos_type,15);	
  }  
  if(strcmp(radio_qos_type,"") == 0)
  {
  	strncpy(radio_qos_type,"besteffort",sizeof(radio_qos_type)-1);
  }
  if(strcmp(client_qos_type,"") == 0)
  {
  	strncpy(client_qos_type,"besteffort",sizeof(client_qos_type)-1);
  }
  
  if(strcmp(instance_id,"")==0)
  {	
	list_instance_parameter(&paraHead1, INSTANCE_STATE_WEB);	
	if(paraHead1)
	{
		snprintf(instance_id,sizeof(instance_id)-1,"%d-%d-%d",paraHead1->parameter.slot_id,paraHead1->parameter.local_id,paraHead1->parameter.instance_id); 
	}
  }
  else
  {
	get_slotID_localID_instanceID(instance_id,&ins_para);	
	get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);
  }

  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
	ShowWqosconPage(encry,ID,instance_id,radio_qos_type,client_qos_type,paraHead1,lpublic,lwlan);
  
  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWqosconPage(char *m,char *n,char *ins_id,char *radio_qos_type,char *client_qos_type,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  int i = 0;  
  int result = 0;
  DCLI_WQOS *wqos = NULL;	
  int radioQosChoice = 0;
  int clientQosChoice = 0;
  char IsSubmit[5] = { 0 }; 
  char *endptr = NULL;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>WQOS</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");	  
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((cgiFormSubmitClicked("wqoscon_apply") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
  	if(ins_para)
	{
		Config_Wqos(ins_para,n,lpublic,lwlan); 
	}
  }
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=%s>%s</font><font id=titleen>QOS</font></td>",search(lpublic,"title_style"),search(lwlan,"wireless"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
	   
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=wqoscon_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_wqoslis.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,ins_id,search(lpublic,"img_cancel"));
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
	              fprintf(cgiOut,"<tr height=26>");
				    if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
                      fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=%s> %s</font><font id=yingwen_san>QOS</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"),search(lpublic,"menu_san"),search(lwlan,"wireless"));   /*突出显示*/
					else
					  fprintf(cgiOut,"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font><font id=%s>%s</font><font id=yingwen_san>QOS</font></td>",search(lpublic,"menu_san"),search(lpublic,"config"),search(lpublic,"menu_san"),search(lwlan,"wireless"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>"\
                  "<tr height=25>");
					if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
					  fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_wqosnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=%s> %s</font><font id=yingwen_san>QOS</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"),search(lpublic,"menu_san"),search(lwlan,"wireless"));                       
					else
					  fprintf(cgiOut,"<td align=left id=tdleft><a href=wp_wqosnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=%s>%s</font><font id=yingwen_san>QOS</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"),search(lpublic,"menu_san"),search(lwlan,"wireless"));                       
                  fprintf(cgiOut,"</tr>");

				  if(ins_para)
				  {
					  result=show_qos_one(ins_para->parameter,ins_para->connection,n,&wqos);
				  }
				  
                  for(i=0;i<27;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
       "<table width=700 border=0 cellspacing=0 cellpadding=0>"\
        "<tr>"\
          "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
        fprintf(cgiOut,"</tr>"\
		"<tr>"\
		  "<td><table width=700 border=0 cellspacing=0 cellpadding=0>"\
                  "<tr height=30 align=left>"\
                    "<td id=thead5 align=left>RADIO%s</td>",search(lpublic,"detail_config"));
                  fprintf(cgiOut,"</tr>"\
               "</table>"\
          "</td>"\
        "</tr>"\
		"<tr><td align=left style=\"padding-left:20px\">");
			  	fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>"\
                 "<td>QOS%s:</td>",search(lpublic,"service"));
			     fprintf(cgiOut,"<td colspan=2 align=left><select name=radio_qos_service id=radio_qos_service style=width:100px onchange=\"javascript:this.form.submit();\">");
				 for(i=0;i<4;i++)
				 {
				 	if(strcmp(wqos_service_type[i],radio_qos_type)==0)              /*显示上次选中的radio qos service type*/
   	                  fprintf(cgiOut,"<option value=%s selected=selected>%s",wqos_service_type[i],wqos_service_type[i]);
		            else			  	
		              fprintf(cgiOut,"<option value=%s>%s",wqos_service_type[i],wqos_service_type[i]);
				 }
				 cgiFormSelectSingle("radio_qos_service", wqos_service_type, 4, &radioQosChoice, 0);
		         fprintf(cgiOut,"</select></td>"\
		        "</tr>"\
                "<tr height=30>"\
                 "<td width=100>CWMIN:</td>");
				 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[radioQosChoice]))
				 {
					 fprintf(cgiOut,"<td align=left width=100><input name=radio_cwmin size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",wqos->qos[0]->radio_qos[radioQosChoice]->CWMin);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td align=left width=100><input name=radio_cwmin size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				 }
				 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\" width=500><font color=red>(0--15)</font></td>"\
                "</tr>"\
                "<tr height=30>"\
                 "<td>CWMAX:</td>");
				 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[radioQosChoice]))
				 {
					 fprintf(cgiOut,"<td align=left><input name=radio_cwmax size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",wqos->qos[0]->radio_qos[radioQosChoice]->CWMax);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td align=left><input name=radio_cwmax size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				 }
				 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(0--15)</font></td>"\
                "</tr>"\
                "<tr height=30>"\
                 "<td>AIFS:</td>");
                 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[radioQosChoice]))
				 {
					 fprintf(cgiOut,"<td align=left><input name=radio_aifs size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",wqos->qos[0]->radio_qos[radioQosChoice]->AIFS);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td align=left><input name=radio_aifs size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				 }
				 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(0--15)</font></td>"\
                "</tr>"\
                "<tr height=30>"\
                 "<td>TXOPLIMIT:</td>");
				 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[radioQosChoice]))
				 {
					 fprintf(cgiOut,"<td align=left><input name=radio_txoplimit size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",wqos->qos[0]->radio_qos[radioQosChoice]->TXOPlimit);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td align=left><input name=radio_txoplimit size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				 }
				 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(0--8192)</font></td>"\
                "</tr>"\
                "<tr height=30>"\
                 "<td>ACK:</td>"\
                 "<td colspan=2 align=left><select name=radio_ack id=radio_ack style=width:100px>");
				 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->radio_qos[radioQosChoice])&&(wqos->qos[0]->radio_qos[radioQosChoice]->ACK==1))
				 {
					 fprintf(cgiOut,"<option value=ack selected=selected>ack"\
					 "<option value=noack>noack");
				 }
				 else
				 {
					 fprintf(cgiOut,"<option value=ack>ack"\
					 "<option value=noack selected=selected>noack");
				 }
		         fprintf(cgiOut,"</select></td>"\
                "</tr>"\
  "</table></td></tr>"\
  		"<tr>"\
		  "<td style=\"padding-top:20px\"><table width=700 border=0 cellspacing=0 cellpadding=0>"\
                  "<tr height=30 align=left>"\
                    "<td id=thead5 align=left>CLIENT%s</td>",search(lpublic,"detail_config"));
                  fprintf(cgiOut,"</tr>"\
               "</table>"\
          "</td>"\
        "</tr>"\
		"<tr><td align=left style=\"padding-left:20px\">");
			  	fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>"\
                 "<td>QOS%s:</td>",search(lpublic,"service"));
			     fprintf(cgiOut,"<td colspan=2 align=left><select name=client_qos_service id=client_qos_service style=width:100px onchange=\"javascript:this.form.submit();\">");
				 for(i=0;i<4;i++)
				 {
				 	if(strcmp(wqos_service_type[i],client_qos_type)==0)              /*显示上次选中的client qos service type*/
   	                  fprintf(cgiOut,"<option value=%s selected=selected>%s",wqos_service_type[i],wqos_service_type[i]);
		            else			  	
		              fprintf(cgiOut,"<option value=%s>%s",wqos_service_type[i],wqos_service_type[i]);
				 }
				 cgiFormSelectSingle("client_qos_service", wqos_service_type, 4, &clientQosChoice, 0);
		         fprintf(cgiOut,"</select></td>"\
		        "</tr>"\
                "<tr height=30>"\
                 "<td width=100>CWMIN:</td>");
				 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->client_qos[clientQosChoice]))
				 {
					 fprintf(cgiOut,"<td align=left width=100><input name=client_cwmin size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",wqos->qos[0]->client_qos[clientQosChoice]->CWMin);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td align=left width=100><input name=client_cwmin size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				 }
				 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\" width=500><font color=red>(0--15)</font></td>"\
                "</tr>"\
                "<tr height=30>"\
                 "<td>CWMAX:</td>");
				 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->client_qos[clientQosChoice]))
				 {
					 fprintf(cgiOut,"<td align=left><input name=client_cwmax size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",wqos->qos[0]->client_qos[clientQosChoice]->CWMax);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td align=left><input name=client_cwmax size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				 }
				 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(0--15)</font></td>"\
                "</tr>"\
                "<tr height=30>"\
                 "<td>AIFS:</td>");
				 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->client_qos[clientQosChoice]))
				 {
					 fprintf(cgiOut,"<td align=left><input name=client_aifs size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",wqos->qos[0]->client_qos[clientQosChoice]->AIFS);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td align=left><input name=client_aifs size=15 maxLength=2 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				 }
				 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(0--15)</font></td>"\
                "</tr>"\
                "<tr height=30>"\
                 "<td>TXOPLIMIT:</td>");
				 if((result == 1)&&(wqos)&&(wqos->qos[0])&&(wqos->qos[0]->client_qos[clientQosChoice]))
				 {
					 fprintf(cgiOut,"<td align=left><input name=client_txoplimit size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=%d></td>",wqos->qos[0]->client_qos[clientQosChoice]->TXOPlimit);
				 }
				 else
				 {
					 fprintf(cgiOut,"<td align=left><input name=client_txoplimit size=15 maxLength=4 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\"></td>");
				 }
				 fprintf(cgiOut,"<td align=left style=\"padding-left:30px\"><font color=red>(0--8192)</font></td>"\
                "</tr>"\
  "</table></td></tr>"\
        "<tr>"\
		  "<td style=\"padding-top:20px\"><table width=700 border=0 cellspacing=0 cellpadding=0>"\
                  "<tr height=30 align=left>");
				  if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
                    fprintf(cgiOut,"<td id=thead5 align=left>open|close WMM mapping</td>");
				  else
				  	fprintf(cgiOut,"<td id=thead5 align=left>开启|关闭WMM映射</td>");
                  fprintf(cgiOut,"</tr>"\
               "</table>"\
          "</td>"\
        "</tr>"\
		"<tr><td align=left style=\"padding-left:20px\">");
			  	fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>");
				 if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
                   fprintf(cgiOut,"<td width=100>WMM mapping:</td>");
				 else
				   fprintf(cgiOut,"<td width=100>WMM映射:</td>");
			     fprintf(cgiOut,"<td colspan=2 align=left width=600><select name=wmm_map id=wmm_map style=width:100px>"\
				 	"<option value=>"\
					"<option value=enable>enable"\
  				    "<option value=disable>disable"\
		         "</select></td>"\
		        "</tr>"\
  "</table></td></tr>"\
  		"<tr>"\
		  "<td style=\"padding-top:20px\"><table width=700 border=0 cellspacing=0 cellpadding=0>"\
                  "<tr height=30 align=left>");
				  if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
                    fprintf(cgiOut,"<td id=thead5 align=left>WMM map dotlp configuration</td>");
				  else
				 	fprintf(cgiOut,"<td id=thead5 align=left>配置WMM向dotlp的映射</td>");
                  fprintf(cgiOut,"</tr>"\
               "</table>"\
          "</td>"\
        "</tr>"\
		"<tr><td align=left style=\"padding-left:20px\">");
			  	fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>"\
                 "<td>QOS%s:</td>",search(lpublic,"service"));
			     fprintf(cgiOut,"<td colspan=2 align=left><select name=wmm_map_service id=wmm_map_service style=width:100px>"\
					"<option value=>"\
				    "<option value=besteffort>besteffort"\
  				    "<option value=background>background"\
  				    "<option value=video>video"\
  				    "<option value=voice>voice"\
		         "</select></td>"\
		        "</tr>"\
                "<tr height=30>"\
                 "<td width=100>dotlp %s:</td>",search(lpublic,"priority"));
                 fprintf(cgiOut,"<td align=left width=100><input name=dotlp_priority size=15 maxLength=1 onkeypress=\"return event.keyCode>=48&&event.keyCode<=57\" value=\"\"></td>"\
				 "<td align=left style=\"padding-left:30px\" width=500><font color=red>(0--7)</font></td>"\
                "</tr>"\
  "</table></td></tr>"\
  		"<tr>"\
		  "<td style=\"padding-top:20px\"><table width=700 border=0 cellspacing=0 cellpadding=0>"\
                  "<tr height=30 align=left>");
				  if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
                    fprintf(cgiOut,"<td id=thead5 align=left>dotlp map WMM configuration</td>");
				  else
				 	fprintf(cgiOut,"<td id=thead5 align=left>配置dotlp向WMM的映射</td>");
                  fprintf(cgiOut,"</tr>"\
               "</table>"\
          "</td>"\
        "</tr>"\
		"<tr><td align=left style=\"padding-left:20px\">");
			  	fprintf(cgiOut,"<table width=700 border=0 cellspacing=0 cellpadding=0>"\
				"<tr height=30>"\
                 "<td width=100>LIST:</td>"\
                 "<td align=left width=100><input name=dotlp_list size=15 maxLength=19 onkeypress=\"return ((event.keyCode>=49&&event.keyCode<=55)||(event.keyCode==44))\" value=\"\"></td>");
				 if(strcmp(search(lwlan,"wireless"),"Wireless ")==0)/*英文界面*/
				   fprintf(cgiOut,"<td align=left style=\"padding-left:30px\" width=500><font color=red>(list is composed by 1-7,and separate by the comma)</font></td>");
				 else
				   fprintf(cgiOut,"<td align=left style=\"padding-left:30px\" width=500><font color=red>(1-7组成的队列，以逗号隔开)</font></td>");
                fprintf(cgiOut,"</tr>"\
				"<tr height=30>"\
                 "<td>QOS%s:</td>",search(lpublic,"service"));
			     fprintf(cgiOut,"<td colspan=2 align=left><select name=dotlp_map_service id=dotlp_map_service style=width:100px>"\
					"<option value=>"\
				    "<option value=besteffort>besteffort"\
  				    "<option value=background>background"\
  				    "<option value=video>video"\
  				    "<option value=voice>voice"\
		         "</select></td>"\
		        "</tr>"\
		        "<tr>"\
                  "<td><input type=hidden name=encry_conwqos value=%s></td>",m);
                  fprintf(cgiOut,"<td><input type=hidden name=wqos_id value=%s></td>",n);	
				  fprintf(cgiOut,"<td><input type=hidden name=instance_id value=%s></td>",ins_id);	
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
				  "<td colspan=3><input type=hidden name=SubmitFlag value=%d></td>",1);
	            fprintf(cgiOut,"</tr>"\
  "</table></td></tr>"\
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
if(result==1)
{
	Free_qos_one(wqos);
}
return 0;
}

void Config_Wqos(instance_parameter *ins_para,char *id,struct list *lpublic,struct list *lwlan)
{	
	int ret = 0,wqos_id = 0,flag = 1;	
	char qos_cwmin_num[10] = { 0 };
	char qos_cwmax_num[10] = { 0 };
	char qos_aifs_num[10] = { 0 };
	char qos_txoplimit_num[10] = { 0 };
    char temp[100] = { 0 };
	char *endptr = NULL;
	char qos_service[15] = { 0 }; 
	char cwmin[5] = { 0 }; 
	char cwmax[5] = { 0 }; 
	char aifs[5] = { 0 }; 
	char txoplimit[10] = { 0 }; 
	char ack[10] = { 0 }; 
	char wmm_map[10] = { 0 }; 
	char dotlp_priority[5] = { 0 }; 
	char dotlp_list[20] = { 0 }; 
	char wqos_num[5] = { 0 };
	
	wqos_id=strtoul(id,&endptr,10);	  /*char转成int，10代表十进制*/   

	/*config radio qos service*/
	memset(qos_service,0,sizeof(qos_service));
	cgiFormStringNoNewlines("radio_qos_service",qos_service,15);
	memset(cwmin,0,sizeof(cwmin));
	cgiFormStringNoNewlines("radio_cwmin",cwmin,5);
	memset(cwmax,0,sizeof(cwmax));
	cgiFormStringNoNewlines("radio_cwmax",cwmax,5);
	memset(aifs,0,sizeof(aifs));
	cgiFormStringNoNewlines("radio_aifs",aifs,5);
	memset(txoplimit,0,sizeof(txoplimit));
	cgiFormStringNoNewlines("radio_txoplimit",txoplimit,10);
	memset(ack,0,sizeof(ack));
	cgiFormStringNoNewlines("radio_ack",ack,10);

	if((strcmp(qos_service,"")!=0)&&(strcmp(cwmin,"")!=0)&&(strcmp(cwmax,"")!=0)&&(strcmp(aifs,"")!=0)&&(strcmp(txoplimit,"")!=0)&&(strcmp(ack,"")!=0))
	{
		ret=config_radio_qos_service(ins_para->parameter,ins_para->connection,wqos_id,qos_service,cwmin,cwmax,aifs,txoplimit,ack);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:ShowAlert(search(lwlan,"con_radio_qos_fail"));
				   flag=0;
				   break;
			case 1:break;
			case -1:ShowAlert(search(lwlan,"unknown_qos_type"));
				    flag=0;
				    break;
			case -2:ShowAlert(search(lpublic,"unknown_id_format"));
				    flag=0;
				    break;
			case -3:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos cwmin",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_cwmin_num,0,sizeof(qos_cwmin_num));
			  		snprintf(qos_cwmin_num,sizeof(qos_cwmin_num)-1,"%d",QOS_CWMIN_NUM-1);
			  		strncat(temp,qos_cwmin_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -4:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos cwmax",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_cwmax_num,0,sizeof(qos_cwmax_num));
			  		snprintf(qos_cwmax_num,sizeof(qos_cwmax_num)-1,"%d",QOS_CWMAX_NUM-1); 
			  		strncat(temp,qos_cwmax_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -5:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos aifs",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_aifs_num,0,sizeof(qos_aifs_num));
			  		snprintf(qos_aifs_num,sizeof(qos_aifs_num)-1,"%d",QOS_AIFS_NUM-1);
			  		strncat(temp,qos_aifs_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -6:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos txoplimit",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_txoplimit_num,0,sizeof(qos_txoplimit_num));
			  		snprintf(qos_txoplimit_num,sizeof(qos_txoplimit_num)-1,"%d",QOS_TXOPLIMIT_NUM);
			  		strncat(temp,qos_txoplimit_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -7:ShowAlert(search(lwlan,"wqos_not_exist"));
				    flag=0;
				    break;
			case -8:ShowAlert(search(lwlan,"unuse_wqos"));
				    flag=0;
				    break;
		    case -9:ShowAlert(search(lpublic,"error"));
				    flag=0;
				    break;			
			case -10:memset(temp,0,sizeof(temp));
					 strncpy(temp,"WQOS ID",sizeof(temp)-1);
					 strncat(temp,search(lwlan,"wqos_id_illegal"),sizeof(temp)-strlen(temp)-1);
					 memset(wqos_num,0,sizeof(wqos_num));
					 snprintf(wqos_num,sizeof(wqos_num)-1,"%d",QOS_NUM-1);
					 strncat(temp,wqos_num,sizeof(temp)-strlen(temp)-1);
					 strncat(temp,search(lwlan,"bss_id_2"),sizeof(temp)-strlen(temp)-1);
					 ShowAlert(temp);
					 flag=0;
					 break;
			case -11:ShowAlert(search(lwlan,"wqos_minmax"));
				     flag = 0;
				     break;
		}
	}
	else   /*参数不全*/
	{
		flag=0;
		ShowAlert(search(lpublic,"para_incom"));
	}


    /*config client qos service*/
	memset(qos_service,0,sizeof(qos_service));
	cgiFormStringNoNewlines("client_qos_service",qos_service,15);
	memset(cwmin,0,sizeof(cwmin));
	cgiFormStringNoNewlines("client_cwmin",cwmin,5);
	memset(cwmax,0,sizeof(cwmax));
	cgiFormStringNoNewlines("client_cwmax",cwmax,5);
	memset(aifs,0,sizeof(aifs));
	cgiFormStringNoNewlines("client_aifs",aifs,5);
	memset(txoplimit,0,sizeof(txoplimit));
	cgiFormStringNoNewlines("client_txoplimit",txoplimit,10);

	if((strcmp(qos_service,"")!=0)&&(strcmp(cwmin,"")!=0)&&(strcmp(cwmax,"")!=0)&&(strcmp(aifs,"")!=0)&&(strcmp(txoplimit,"")!=0))
	{
		ret=config_client_qos_service(ins_para->parameter,ins_para->connection,wqos_id,qos_service,cwmin,cwmax,aifs,txoplimit);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:ShowAlert(search(lwlan,"con_client_qos_fail"));
				   flag=0;
				   break;
			case 1:break;
			case -1:ShowAlert(search(lwlan,"unknown_qos_type"));
				    flag=0;
				    break;
			case -2:ShowAlert(search(lpublic,"unknown_id_format"));
				    flag=0;
				    break;
			case -3:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos cwmin",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_cwmin_num,0,sizeof(qos_cwmin_num));
			  		snprintf(qos_cwmin_num,sizeof(qos_cwmin_num)-1,"%d",QOS_CWMIN_NUM-1);
			  		strncat(temp,qos_cwmin_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -4:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos cwmax",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_cwmax_num,0,sizeof(qos_cwmax_num));
			  		snprintf(qos_cwmax_num,sizeof(qos_cwmax_num)-1,"%d",QOS_CWMAX_NUM-1); 
			  		strncat(temp,qos_cwmax_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -5:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos aifs",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_aifs_num,0,sizeof(qos_aifs_num));
			  		snprintf(qos_aifs_num,sizeof(qos_aifs_num)-1,"%d",QOS_AIFS_NUM-1);
			  		strncat(temp,qos_aifs_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -6:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos txoplimit",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_txoplimit_num,0,sizeof(qos_txoplimit_num));
			  		snprintf(qos_txoplimit_num,sizeof(qos_txoplimit_num)-1,"%d",QOS_TXOPLIMIT_NUM);
			  		strncat(temp,qos_txoplimit_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -7:ShowAlert(search(lwlan,"wqos_not_exist"));
				    flag=0;
				    break;
			case -8:ShowAlert(search(lwlan,"unuse_wqos"));
				    flag=0;
				    break;
		    case -9:ShowAlert(search(lpublic,"error"));
				    flag=0;
				    break;
			case -10:memset(temp,0,sizeof(temp));
					 strncpy(temp,"WQOS ID",sizeof(temp)-1);
					 strncat(temp,search(lwlan,"wqos_id_illegal"),sizeof(temp)-strlen(temp)-1);
					 memset(wqos_num,0,sizeof(wqos_num));
					 snprintf(wqos_num,sizeof(wqos_num)-1,"%d",QOS_NUM-1);
					 strncat(temp,wqos_num,sizeof(temp)-strlen(temp)-1);
					 strncat(temp,search(lwlan,"bss_id_2"),sizeof(temp)-strlen(temp)-1);
					 ShowAlert(temp);
					 flag=0;
					 break;
			case -11:ShowAlert(search(lwlan,"wqos_minmax"));
				     flag = 0;
				     break;
		}
	}
	else   /*参数不全*/
	{
		flag=0;
		ShowAlert(search(lpublic,"para_incom"));
	}

	/*open or close wmm mapping*/
	memset(wmm_map,0,sizeof(wmm_map));
	cgiFormStringNoNewlines("wmm_map",wmm_map,10);

	if(strcmp(wmm_map,"")!=0)
	{
		ret=config_wmm_service(ins_para->parameter,ins_para->connection,wqos_id,wmm_map);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:ShowAlert(search(lwlan,"con_wmm_map_fail"));
				   flag=0;
				   break;
			case 1:break;
			case -1:flag=0;                   /*input parameter should be only 'enable' or 'disable'*/
				    break;
			case -2:ShowAlert(search(lwlan,"wqos_not_exist"));
				    flag=0;
				    break;
			case -3:ShowAlert(search(lwlan,"unuse_wqos"));
				    flag=0;
				    break;
		    case -4:ShowAlert(search(lpublic,"error"));
				    flag=0;
				    break;
		}
	}

	/*config wmm map dotlp*/
	memset(qos_service,0,sizeof(qos_service));
	cgiFormStringNoNewlines("wmm_map_service",qos_service,15);
	memset(dotlp_priority,0,sizeof(dotlp_priority));
	cgiFormStringNoNewlines("dotlp_priority",dotlp_priority,5);

	if((strcmp(qos_service,"")!=0)&&(strcmp(dotlp_priority,"")!=0))
	{
		ret=config_wmm_map_dotlp(ins_para->parameter,ins_para->connection,wqos_id,qos_service,dotlp_priority);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:ShowAlert(search(lwlan,"wmm_map_dotlp_fail"));
				   flag=0;
				   break;
			case 1:break;
			case -1:flag=0;              /*input parameter should be only 'voice' 'video' 'besteffort' or 'background'*/
				    break;
			case -2:ShowAlert(search(lpublic,"unknown_id_format"));
				    flag=0;
				    break;
			case -3:memset(temp,0,sizeof(temp));
					strncpy(temp,"qos dotlp",sizeof(temp)-1);
			  		strncat(temp,search(lwlan,"qos_range"),sizeof(temp)-strlen(temp)-1);
			  		memset(qos_cwmin_num,0,sizeof(qos_cwmin_num));
			  		snprintf(qos_cwmin_num,sizeof(qos_cwmin_num)-1,"%d",7);
			  		strncat(temp,qos_cwmin_num,sizeof(temp)-strlen(temp)-1);
			  		strncat(temp,search(lwlan,"qos_id_2"),sizeof(temp)-strlen(temp)-1);
			  		ShowAlert(temp);
				    flag=0;
				    break;
			case -4:ShowAlert(search(lwlan,"wqos_not_exist"));
				    flag=0;
				    break;
			case -5:ShowAlert(search(lwlan,"unuse_wqos"));
				    flag=0;
				    break;
			case -6:ShowAlert(search(lwlan,"used_wqos"));
				    flag=0;
				    break;
		    case -7:ShowAlert(search(lpublic,"error"));
				    flag=0;
				    break;
		}
	}
	else if(!((strcmp(qos_service,"")==0)&&(strcmp(dotlp_priority,"")==0)))  /*参数不全*/
	{
		flag=0;
		ShowAlert(search(lpublic,"para_incom"));
	}


	/*config dotlp map wmm*/
	memset(dotlp_list,0,sizeof(dotlp_list));
	cgiFormStringNoNewlines("dotlp_list",dotlp_list,20);
	memset(qos_service,0,sizeof(qos_service));
	cgiFormStringNoNewlines("dotlp_map_service",qos_service,15);

	if((strcmp(dotlp_list,"")!=0)&&(strcmp(qos_service,"")!=0))
	{
		ret=config_dotlp_map_wmm(ins_para->parameter,ins_para->connection,wqos_id,dotlp_list,qos_service);
		switch(ret)
		{
			case SNMPD_CONNECTION_ERROR:
			case 0:ShowAlert(search(lwlan,"dotlp_map_wmm_fail"));
				   flag=0;
				   break;
			case 1:break;
			case -1:ShowAlert(search(lpublic,"input_para_illegal"));
				    flag=0;
				    break;
			case -2:flag=0;              /*input parameter should be only 'voice' 'video' 'besteffort' or 'background'*/
				    break;
			case -3:ShowAlert(search(lwlan,"wqos_not_exist"));
				    flag=0;
				    break;
			case -4:ShowAlert(search(lwlan,"unuse_wqos"));
				    flag=0;
				    break;
			case -5:ShowAlert(search(lwlan,"used_wqos"));
				    flag=0;
				    break;
		    case -6:ShowAlert(search(lpublic,"error"));
				    flag=0;
				    break;
		}
	}
	else if(!((strcmp(dotlp_list,"")==0)&&(strcmp(qos_service,"")==0)))  /*参数不全*/
	{
		flag=0;
		ShowAlert(search(lpublic,"para_incom"));
	}

	

    if(flag==1)
    {
		ShowAlert(search(lpublic,"oper_succ"));
    }
}


