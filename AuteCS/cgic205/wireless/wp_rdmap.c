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
* wp_rdmap.c
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
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include <sys/wait.h>
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"


int ShowWlanMapInterPage(char *m,char *n,char *bssid,char *radio_g_id,char *radio_local_id,char *f,char *wlanid,char *wtp_id,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);               /*m代表加密字符串，n代表wlan id，t代表接口类型*/
int config_bss_interface(instance_parameter *ins_para,int type,int WLAN_IF,int rid,char *bss_id,char *wtp_id,char *radio_local_id,char *wlan_id,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };      
  char BSSID[10] = { 0 };
  char rid[10] = { 0 };  
  char r_l_id[10] = { 0 };  
  char flag[5] = { 0 }; /*fla=="1",表示上一页为wp_radiolis.cgi,否则上一页为wp_wtpdta.cgi*/  
  char WLANID[10] = { 0 };  
  char wtp_id[10] = { 0 };
  char l3Choice[5] = { 0 };
  char instance_id[10] = { 0 };
  char *str = NULL;                
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  DCLI_WLAN_API_GROUP *wlan = NULL;
  DCLI_RADIO_API_GROUP_ONE *radio = NULL;
  int result1 = 0,result2 = 0,wlan_id = 0,id = 0,bss_id = 0;
  char *endptr = NULL; 
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(BSSID,0,sizeof(BSSID));
  memset(rid,0,sizeof(rid));
  memset(r_l_id,0,sizeof(r_l_id));
  memset(flag,0,sizeof(flag));
  memset(WLANID,0,sizeof(WLANID));
  memset(wtp_id,0,sizeof(wtp_id));
  memset(l3Choice,0,sizeof(l3Choice));
  memset(instance_id,0,sizeof(instance_id));
  
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
  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	cgiFormStringNoNewlines("BSSID", BSSID, 10);		
	cgiFormStringNoNewlines("RID", rid, 10);
	cgiFormStringNoNewlines("RLID", r_l_id, 10);
	cgiFormStringNoNewlines("FL", flag, 5);
	cgiFormStringNoNewlines("WLANID", WLANID, 10);	
	cgiFormStringNoNewlines("WID", wtp_id, 10); 
	cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
	wlan_id=strtoul(WLANID,&endptr,10); 
	if(paraHead1)
	{
		result1=show_wlan_one(paraHead1->parameter,paraHead1->connection,wlan_id,&wlan);
	}
	id=strtoul(rid,&endptr,10); 
	bss_id=strtoul(BSSID,&endptr,10);
	if(paraHead1)
	{
		result2=show_radio_one(paraHead1->parameter,paraHead1->connection,id,&radio);
	}
	if(result2 == 1)
	{
		if((result1 == 1)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->wlan_if_policy==0)) /*当WLAN的三层接口策略为NO_IF时*/
		{
			if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[bss_id]))
			{
				switch(radio->RADIO[0]->BSS[bss_id]->BSS_IF_POLICY)
				{
					case 0:{	   /*BSS_IF为NO_IF*/
							 strncpy(l3Choice,"0",sizeof(l3Choice)-1);
							 break;
						   }
					case 2:{	   /*BSS_IF为BSS_IF*/
							 strncpy(l3Choice,"1",sizeof(l3Choice)-1);
							 break;
						   }
				}
			}
		}
		else
		{
			if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[bss_id]))
			{
				switch(radio->RADIO[0]->BSS[bss_id]->BSS_IF_POLICY)
				{
					case 0:{	   /*BSS_IF为NO_IF*/
							 strncpy(l3Choice,"0",sizeof(l3Choice)-1);
							 break;
						   }
					case 1:{	   /*BSS_IF为WLAN_IF*/
							 strncpy(l3Choice,"3",sizeof(l3Choice)-1);
							 break;
						   }
					case 2:{	   /*BSS_IF为BSS_IF*/
							 strncpy(l3Choice,"1",sizeof(l3Choice)-1);
							 break;
						   }
				}
			}
		}
  	}
  }  
  else
  {  
    cgiFormStringNoNewlines("encry_mapwlaninter",encry,BUF_LEN);
    cgiFormStringNoNewlines("bss_id",BSSID,10);	  
	cgiFormStringNoNewlines("radio_g_id",rid,10); 
	cgiFormStringNoNewlines("radio_local_id",r_l_id,10);  
	cgiFormStringNoNewlines("flag",flag,5);
	cgiFormStringNoNewlines("wlan_id",WLANID,10);  
	cgiFormStringNoNewlines("wtp_id",wtp_id,10); 
	cgiFormStringNoNewlines("l3_type",l3Choice,5);	
	cgiFormStringNoNewlines("instance_id",instance_id,10);  
  }      
  
  str=dcryption(encry);
  if(str==NULL)
	ShowErrorPage(search(lpublic,"ill_user"));			  /*用户非法*/
  else
	ShowWlanMapInterPage(encry,l3Choice,BSSID,rid,r_l_id,flag,WLANID,wtp_id,instance_id,paraHead1,lpublic,lwlan);

  if(result1==1)
  {
    Free_one_wlan_head(wlan);
  }
  if(result2==1)
  {
  	Free_radio_one_head(radio);
  }
  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

int ShowWlanMapInterPage(char *m,char *n,char *bssid,char *radio_g_id,char *radio_local_id,char *f,char *wlanid,char *wtp_id,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{  
  int i = 0,limit = 0,l3_type = 0,result = 0,result1 = 0,result2 = 0,id = 0,wlan_id = 0,bss_id = 0; 
  int is_display_ip = 0;
  char *endptr = NULL;  
  char pno[10] = { 0 };
  DCLI_WLAN_API_GROUP *wlan = NULL;
  DCLI_RADIO_API_GROUP_ONE *radio = NULL;
  memset(pno,0,sizeof(pno));
  cgiFormStringNoNewlines("PN",pno,10);
  l3_type=strtoul(n,&endptr,10); 
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wlan</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  	"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
  "</head>"\
  "<script src=/ip.js>"\
  "</script>"\
  "<body>");
  wlan_id=strtoul(wlanid,&endptr,10);
  if(ins_para)
  {
	  result1=show_wlan_one(ins_para->parameter,ins_para->connection,wlan_id,&wlan);
  }
  id=strtoul(radio_g_id,&endptr,10);
  if(ins_para)
  {
	  result2=show_radio_one(ins_para->parameter,ins_para->connection,id,&radio);
  }
  if(cgiFormSubmitClicked("rdmap_apply") == cgiFormSuccess)
  {
  	if((result1 == 1)&&(wlan)&&(wlan->WLAN[0]))
  	{
  		if(ins_para)
	    {
			result=config_bss_interface(ins_para,l3_type,wlan->WLAN[0]->wlan_if_policy,id,bssid,wtp_id,radio_local_id,wlanid,lpublic,lwlan);
	    }
		if(result==1)/*成功映射radio三层接口*/
		{
		  fprintf(cgiOut, "<script type=text/javascript>"\
							"window.location.href = 'wp_rdtail.cgi?UN=%s&FL=%s&ID=%s&WID=%s&PN=%s&INSTANCE_ID=%s';\n", m,f,radio_g_id,wtp_id,pno,ins_id);
						  fprintf(cgiOut, "</script>");
		}
  	}
  }  
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>");
    if(strcmp(f,"0")==0)
      fprintf(cgiOut,"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
	else
	  fprintf(cgiOut,"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RF</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	      	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><input id=but type=submit name=rdmap_apply style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_rdtail.cgi?UN=%s&FL=%s&ID=%s&WID=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,f,radio_g_id,wtp_id,pno,ins_id,search(lpublic,"img_cancel"));
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
	              fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>L3</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"interface"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  if(strcmp(f,"0")==0)
				  {
                    fprintf(cgiOut,"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
	                fprintf(cgiOut,"</tr>"\
	                "<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
	                fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpgrouplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"ap_group_list"));			  
		  fprintf(cgiOut,"</tr>");
		fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpgroupnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"create_apgroup"));			  
		  fprintf(cgiOut,"</tr>");
			fprintf(cgiOut,"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
	                fprintf(cgiOut,"</tr>"\
				    "<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpdown.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_upload"));                       
	                fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_verbind.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"version_bind"));                       
                    fprintf(cgiOut,"</tr>"\
				    "<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpupgrade.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"batch_update"));                       
                    fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
	                fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
	                fprintf(cgiOut,"</tr>"\
	                "<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
	                fprintf(cgiOut,"</tr>");
				  }
				  else
				  {
				    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_bssbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  if(strcmp(f,"0")==0)
				  	limit=0;
				  else
				  	limit=2;
                  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:20px\">");
			    fprintf(cgiOut,"<table width=240 border=0 cellspacing=0 cellpadding=0>"\
				"<tr>"\
				  "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
				fprintf(cgiOut,"</tr>"\
                "<tr height=30>");
				  	fprintf(cgiOut,"<td width=140>%s:</td>",search(lwlan,"bss_l3_convert"));
   				  fprintf(cgiOut,"<td width=100 align=left><select name=l3_type id=l3_type style=width:140px onchange=\"javascript:this.form.submit();\">");
				  bss_id=strtoul(bssid,&endptr,10);
				  if((result2==1)&&(radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[bss_id]))
	  				{
  							  if((result1 == 1)&&(wlan)&&(wlan->WLAN[0])&&(wlan->WLAN[0]->wlan_if_policy==0)) /*当WLAN的三层接口策略为NO_IF时*/
  							  {
  								  switch(radio->RADIO[0]->BSS[bss_id]->BSS_IF_POLICY)
  								  {
  								  	case 0:{       /*BSS_IF为NO_IF*/
  										     fprintf(cgiOut,"<option value=0 selected=selected>NO_IF --> BSS_IF");
  											 is_display_ip=1;
  											 break;
  										   }
  									case 2:{       /*BSS_IF为BSS_IF*/
  										     fprintf(cgiOut,"<option value=1 selected=selected>BSS_IF --> NO_IF");
  											 break;
  										   }
  								  }
  							  }
  							  else
  							  {
  							  	  switch(radio->RADIO[0]->BSS[bss_id]->BSS_IF_POLICY)
  								  {
  								  	case 0:{      /*BSS_IF为NO_IF*/
  										     if(l3_type==0)
  										     {
  											     fprintf(cgiOut,"<option value=0 selected=selected>NO_IF --> BSS_IF"\
  												 "<option value=2>NO_IF --> WLAN_IF");
  												 is_display_ip=1;
  												 break;
  										     }
  											 else
  											 {
  											     fprintf(cgiOut,"<option value=0>NO_IF --> BSS_IF"\
  												 "<option value=2 selected=selected>NO_IF --> WLAN_IF");
  												 break;
  										     }
  										   }
  									case 1:{      /*BSS_IF为WLAN_IF*/
  										     if(l3_type==3)
  										     {
  				 							     fprintf(cgiOut,"<option value=3 selected=selected>WLAN_IF --> NO_IF"\
  				 								 "<option value=5>WLAN_IF --> BSS_IF");
  												 break;
  										     }
  											 else
  											 {
  				 							     fprintf(cgiOut,"<option value=3>WLAN_IF --> NO_IF"\
  				 								 "<option value=5 selected=selected>WLAN_IF --> BSS_IF");
  												 is_display_ip=1;
  												 break;
  										     }
  										   }
  								    case 2:{      /*BSS_IF为BSS_IF*/
  											 if(l3_type==1)
  											 {
  											     fprintf(cgiOut,"<option value=1 selected=selected>BSS_IF --> NO_IF"\
  												 "<option value=4>BSS_IF --> WLAN_IF");
  												 break;
  											 }
  											 else
  											 {
  											     fprintf(cgiOut,"<option value=1>BSS_IF --> NO_IF"\
  												 "<option value=4 selected=selected>BSS_IF --> WLAN_IF");
  												 break;
  											 }
  										   }
  								  }
  							  }
	  				}
	              fprintf(cgiOut,"</select>"\
				  "</td>");
                fprintf(cgiOut,"</tr>");
				if(is_display_ip==1)
				{
	 			     fprintf(cgiOut,"<tr height=30>"\
	                  "<td>IP:</td>"\
	                  "<td align=left>"\
	 				   "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	 				   fprintf(cgiOut,"<input type=text name='port_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	 				   fprintf(cgiOut,"<input type=text name='port_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	 				   fprintf(cgiOut,"<input type=text name='port_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	 				   fprintf(cgiOut,"<input type=text name='port_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	 				   fprintf(cgiOut,"</div>"\
	 				 "</td>"\
	                 "</tr>"\
	 				"<tr height=30>"\
	                  "<td>%s:</td>",search(lpublic,"netmask"));
	                  fprintf(cgiOut,"<td align=left>"\
	 				   "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	               	   fprintf(cgiOut,"<input type=text name='port_mask1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	               	   fprintf(cgiOut,"<input type=text name='port_mask2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	               	   fprintf(cgiOut,"<input type=text name='port_mask3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	               	   fprintf(cgiOut,"<input type=text name='port_mask4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	 				   fprintf( cgiOut,"</div>"\
	 				 "</td>");
	                 fprintf(cgiOut,"</tr>");
                }
                fprintf(cgiOut,"<tr>"\
                "<td><input type=hidden name=encry_mapwlaninter value=%s></td>",m);
                fprintf(cgiOut,"<td><input type=hidden name=bss_id value=%s></td>",bssid);	
	          fprintf(cgiOut,"</tr>"\
			  	"<tr>"\
                "<td><input type=hidden name=radio_g_id value=%s></td>",radio_g_id);
                fprintf(cgiOut,"<td><input type=hidden name=radio_local_id value=%s></td>",radio_local_id);	
	          fprintf(cgiOut,"</tr>"\
			  	"<tr>"\
                "<td><input type=hidden name=flag value=%s></td>",f);
			  fprintf(cgiOut,"<td><input type=hidden name=wlan_id value=%s></td>",wlanid);
	          fprintf(cgiOut,"</tr>"\
			  	"<tr>"\
                "<td><input type=hidden name=wtp_id value=%s></td>",wtp_id);
			    fprintf(cgiOut,"<td><input type=hidden name=instance_id value=%s></td>",ins_id);
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
if(result1==1)
{
  Free_one_wlan_head(wlan);
}
if(result2==1)
{
  Free_radio_one_head(radio);
}
return 0;
}


int maskstr2int( char *mask )
{
	unsigned int iMask, m0, m1, m2, m3;
	char binarystr[64]="";
	int i, iRet;
	
	sscanf( mask, "%u.%u.%u.%u", &m3,&m2,&m1,&m0 );
	iMask = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	
	iRet = 0;
	for( i=0; i < 32 ;i++ )
	{
		if( ( iMask & 1 ) == 1 )
		{
			binarystr[31-i] = '1';
			iRet ++;
		}
		else
		{
			binarystr[31-i] = '0';	
		}
		iMask = iMask >> 1;
	}
	
	if( strstr( binarystr, "01" ) )
	{
		return -1;	
	}
	
	return iRet;
}

int config_bss_interface(instance_parameter *ins_para,int type,int WLAN_IF,int rid,char *bss_id,char *wtp_id,char *radio_local_id,char *wlan_id,struct list *lpublic,struct list *lwlan)
{
    int ret = 0,status = 0,retu = 0,ret1 = 0,flag = 1,status1 = 0,retu1 = 0;  
    char ip1[4] = { 0 };
	char ip2[4] = { 0 };
	char ip3[4] = { 0 };
	char ip4[4] = { 0 };
    char mask1[4] = { 0 };
	char mask2[4] = { 0 };
	char mask3[4] = { 0 };
	char mask4[4] = { 0 };
    char inter_ip[20] = { 0 };  
    char inter_mask[20] = { 0 };
    char mask[5] = { 0 };  
    char command[80] = { 0 };
    char inter[20] = { 0 };
	char temp[100] = { 0 };
	char max_radio_num[10] = { 0 };
	char max_wlan_num[10] = { 0 };
	char tmp[10] = { 0 };
  	
	if((type!=2)&&(type!=3))
	{
		memset(inter,0,sizeof(inter));
		strncpy(inter,wtp_id,sizeof(inter)-1);
		strncat(inter,"-",sizeof(inter)-strlen(inter)-1);
		strncat(inter,radio_local_id,sizeof(inter)-strlen(inter)-1);
		strncat(inter,".",sizeof(inter)-strlen(inter)-1);
		strncat(inter,wlan_id,sizeof(inter)-strlen(inter)-1);
	}

	if((type==0)||(type==5))
	{
	    memset(inter_ip,0,sizeof(inter_ip));                                 
	    memset(ip1,0,sizeof(ip1));
	    cgiFormStringNoNewlines("port_ip1",ip1,4);	
	    strncat(inter_ip,ip1,sizeof(inter_ip)-strlen(inter_ip)-1);
	    strncat(inter_ip,".",sizeof(inter_ip)-strlen(inter_ip)-1);
	    memset(ip2,0,sizeof(ip2));
	    cgiFormStringNoNewlines("port_ip2",ip2,4); 
	    strncat(inter_ip,ip2,sizeof(inter_ip)-strlen(inter_ip)-1);	
	    strncat(inter_ip,".",sizeof(inter_ip)-strlen(inter_ip)-1);
	    memset(ip3,0,sizeof(ip3));
	    cgiFormStringNoNewlines("port_ip3",ip3,4); 
	    strncat(inter_ip,ip3,sizeof(inter_ip)-strlen(inter_ip)-1);	
	    strncat(inter_ip,".",sizeof(inter_ip)-strlen(inter_ip)-1);
	    memset(ip4,0,sizeof(ip4));
	    cgiFormStringNoNewlines("port_ip4",ip4,4);
	    strncat(inter_ip,ip4,sizeof(inter_ip)-strlen(inter_ip)-1);
		
	    memset(inter_mask,0,sizeof(inter_mask)); 
	    memset(mask1,0,sizeof(mask1));
	    cgiFormStringNoNewlines("port_mask1",mask1,4);	
	    strncat(inter_mask,mask1,sizeof(inter_mask)-strlen(inter_mask)-1);
	    strncat(inter_mask,".",sizeof(inter_mask)-strlen(inter_mask)-1);
	    memset(mask2,0,sizeof(mask2));
	    cgiFormStringNoNewlines("port_mask2",mask2,4); 
	    strncat(inter_mask,mask2,sizeof(inter_mask)-strlen(inter_mask)-1);	
	    strncat(inter_mask,".",sizeof(inter_mask)-strlen(inter_mask)-1);
	    memset(mask3,0,sizeof(mask3));
	    cgiFormStringNoNewlines("port_mask3",mask3,4); 
	    strncat(inter_mask,mask3,sizeof(inter_mask)-strlen(inter_mask)-1);	
	    strncat(inter_mask,".",sizeof(inter_mask)-strlen(inter_mask)-1);
	    memset(mask4,0,sizeof(mask4));
	    cgiFormStringNoNewlines("port_mask4",mask4,4);	   
	    strncat(inter_mask,mask4,sizeof(inter_mask)-strlen(inter_mask)-1);

		if(!(((strcmp(ip1,"")==0)&&(strcmp(ip2,"")==0)&&(strcmp(ip3,"")==0)&&(strcmp(ip4,"")==0))||
			((strcmp(ip1,""))&&(strcmp(ip2,""))&&(strcmp(ip3,""))&&(strcmp(ip4,"")))))
	    {
	      ShowAlert(search(lpublic,"ip_not_null"));
		  return 0;
	    }

		if(strcmp(inter_ip,"...")!=0)/*输入IP*/
		{
		  if((strcmp(mask1,"")==0)&&(strcmp(mask2,"")==0)&&(strcmp(mask3,"")==0)&&(strcmp(mask4,"")==0))/*没有输入掩码时*/
		  {
		    ShowAlert(search(lpublic,"mask_not_null"));
		    return 0;
		  }
		}
		
		if(!(((strcmp(mask1,"")==0)&&(strcmp(mask2,"")==0)&&(strcmp(mask3,"")==0)&&(strcmp(mask4,"")==0))||
			((strcmp(mask1,""))&&(strcmp(mask2,""))&&(strcmp(mask3,""))&&(strcmp(mask4,"")))))
	    {
	      ShowAlert(search(lpublic,"mask_not_null"));
		  return 0;
	    }

        if(strcmp(inter_mask,"...")!=0)/*输入掩码时，将掩码由xxx.xxx.xxx.xxx的格式变成整数*/
        {
			ret = maskstr2int( inter_mask );
			snprintf( mask, sizeof(mask)-1, "%d", ret );
			
			if((ret<=0)||(ret>32))			 
			{
			  ShowAlert(search(lpublic,"mask_illegal"));
			  return 0;
			}
        }
	}

	if((type==0)||(type==5))
	{
		memset(command,0,sizeof(command));
	    strncat(command,"set_intf_ins.sh ",sizeof(command)-strlen(command)-1);

		memset(tmp,0,sizeof(tmp));
		snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.slot_id);
		strncat(command,tmp,sizeof(command)-strlen(command)-1);
		
		strncat(command," ",sizeof(command)-strlen(command)-1);
		memset(tmp,0,sizeof(tmp));
		snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.local_id);
		strncat(command,tmp,sizeof(command)-strlen(command)-1);
		
		strncat(command," ",sizeof(command)-strlen(command)-1);
		memset(tmp,0,sizeof(tmp));
		snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.instance_id);
		strncat(command,tmp,sizeof(command)-strlen(command)-1);
		
	    strncat(command," ",sizeof(command)-strlen(command)-1);
	    strncat(command,"radio",sizeof(command)-strlen(command)-1);
	    strncat(command,inter,sizeof(command)-strlen(command)-1);
		if((strcmp(inter_ip,""))&&(strcmp(inter_ip,"..."))&&(strcmp(mask,""))&&(strcmp(inter_mask,"...")))
		{
			strncat(command," ",sizeof(command)-strlen(command)-1);
			strncat(command,inter_ip,sizeof(command)-strlen(command)-1);
			strncat(command,"/",sizeof(command)-strlen(command)-1);
			strncat(command,mask,sizeof(command)-strlen(command)-1);
		}
	    strncat(command," >/var/run/apache2/radmapinter.txt",sizeof(command)-strlen(command)-1);

	    status = system(command); 	 
	    retu = WEXITSTATUS(status);
	    if(0!=retu)    /*command fail*/
	    {
			flag=0;
			status1 = system("radmapinter.sh"); 	 
			retu1 = WEXITSTATUS(status1);
			switch(retu1)
			{
			 	case 1: ShowAlert(search(lpublic,"malloc_error"));
						break;
				case 2: ShowAlert(search(lpublic,"input_id_invvalid"));
						break;
				case 3: ShowAlert(search(lwlan,"wtp_not_exist"));
						break;
				case 4: ShowAlert(search(lwlan,"wlan_not_exist"));
						break;
				case 5: ShowAlert(search(lwlan,"radio_not_exist"));
						break;						
				case 6: ShowAlert(search(lwlan,"bss_not_exist"));
						break;						
				case 7: ShowAlert(search(lwlan,"wtp_not_bind_wlan"));
						break;
				case 8: ShowAlert(search(lwlan,"wtp_bind_wlan_not_match"));
						break;						
				case 9: ShowAlert(search(lwlan,"unuse_bss"));
						break;						
				case 10: ShowAlert(search(lwlan,"wlan_is_noif"));
						 break;						 
				case 11: ShowAlert(search(lwlan,"bss_creat_l3_fail"));
						 break;
				case 12: ShowAlert(search(lwlan,"bss_del_l3_fail"));
						 break;
				case 13: ShowAlert(search(lwlan,"wlan_creat_l3_fail"));
						 break;
				case 14: ShowAlert(search(lwlan,"wlan_del_l3_fail"));
						 break;
				case 15: ShowAlert(search(lwlan,"radio_is_in_ebr"));
						 break;
				case 16: ShowAlert(search(lpublic,"l3_if_error"));
						 break;
				case 17: ShowAlert(search(lpublic,"unknown_error_occur"));
						 break;
				case 18: ShowAlert(search(lpublic,"error"));
						 break;
			}
			return 0;
	    }
	}
	else if((type==1)||(type==4))
	{
		memset(command,0,sizeof(command));
		strncpy(command,"no_inter_ins.sh ",sizeof(command)-1);
		
		memset(tmp,0,sizeof(tmp));
		snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.slot_id);
		strncat(command,tmp,sizeof(command)-strlen(command)-1);
		
		strncat(command," ",sizeof(command)-strlen(command)-1);
		memset(tmp,0,sizeof(tmp));
		snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.local_id);
		strncat(command,tmp,sizeof(command)-strlen(command)-1);
		
		strncat(command," ",sizeof(command)-strlen(command)-1);
		memset(tmp,0,sizeof(tmp));
		snprintf(tmp,sizeof(tmp)-1,"%d",ins_para->parameter.instance_id);
		strncat(command,tmp,sizeof(command)-strlen(command)-1);

	    strncat(command," ",sizeof(command)-strlen(command)-1);
		strncat(command,"r",sizeof(command)-strlen(command)-1);
   		strncat(command,inter,sizeof(command)-strlen(command)-1);
		strncat(command," >/var/run/apache2/radnomapinter.txt",sizeof(command)-strlen(command)-1);

        status = system(command); 	 
	    ret = WEXITSTATUS(status);
		if(0!=ret)    /*command fail*/
		{
			flag = 0;
			retu1 = 0;
			status1 = 0;
			status1 = system("radnomapinter.sh"); 	 
			retu1 = WEXITSTATUS(status1);
			switch(retu1)
			{
			 	case 1: ShowAlert(search(lpublic,"malloc_error"));
						break;
				case 2: ShowAlert(search(lpublic,"input_id_invvalid"));
						break;
				case 3: ShowAlert(search(lwlan,"wtp_not_exist"));
						break;
				case 4: ShowAlert(search(lwlan,"wlan_not_exist"));
						break;
				case 5: ShowAlert(search(lwlan,"radio_not_exist"));
						break;						
				case 6: ShowAlert(search(lwlan,"wtp_not_bind_wlan"));
						break;
				case 7: ShowAlert(search(lwlan,"bss_not_exist"));
						break;						
				case 8: ShowAlert(search(lwlan,"wtp_bind_wlan_not_match"));
						break;						
				case 9: ShowAlert(search(lwlan,"unuse_bss"));
						break;						
				case 10: ShowAlert(search(lwlan,"bss_creat_l3_fail"));
						 break;
				case 11: ShowAlert(search(lwlan,"bss_del_l3_fail"));
						 break;
				case 12: ShowAlert(search(lwlan,"remove_bss_inter_from_wlan_br_fail"));
						 break;						 	
				case 13: ShowAlert(search(lwlan,"delete_inter_from_ebr"));
					     break; 
				case 14: ShowAlert(search(lpublic,"error"));
						 break;
			}
			return 0;
	    }
		if((type==1)&&(WLAN_IF==1))
			ret1=set_radio_bss_l3_policy(ins_para->parameter,ins_para->connection,rid,wlan_id,"no");
	}
	else if(type==2)
	{
		ret1=set_radio_bss_l3_policy(ins_para->parameter,ins_para->connection,rid,wlan_id,"wlan");
	}
	else   /*type==3*/
	{
		ret1=set_radio_bss_l3_policy(ins_para->parameter,ins_para->connection,rid,wlan_id,"no");
	}

	if(((type==1)&&(WLAN_IF==1))||(type==2)||(type==3))
	{
		switch(ret1)				
		{
		  case SNMPD_CONNECTION_ERROR:
		  case 0:{
		  	       flag=0;
				   ShowAlert(search(lwlan,"con_radio_bss_l3_fail"));
				   break;
				 }
		  case 1:break;
		  case -1:{
		 		    flag=0;
				    ShowAlert(search(lpublic,"input_para_error"));
				    break; 
			      }
		  case -2:{
		  	        flag=0;
				    ShowAlert(search(lwlan,"unuse_bss"));
				    break;
				  }
		  case -3:{
		  	        flag=0;
				    ShowAlert(search(lwlan,"bss_l3_inter_exist"));
				    break;
				  }
		  case -4:{
		  	        flag=0;
				    ShowAlert(search(lwlan,"bss_creat_l3_fail"));
				    break;
				  }
		  case -5:{
		  	        flag=0;
				    ShowAlert(search(lwlan,"bss_del_l3_fail"));
				    break;
				  }
		  case -6:{
		  	        flag=0;
				    ShowAlert(search(lwlan,"wlan_policy_is_nointer"));
				    break;
				  }
		  case -7:{
		  	        flag=0;
				    ShowAlert(search(lwlan,"bss_not_exist"));
				    break;
				  }
		  case -8:{
		  	        flag=0;
				    ShowAlert(search(lpublic,"can_not_use_com"));
				    break;
				  }
		  case -9:{
		  	        flag=0;
				    ShowAlert(search(lwlan,"radio_not_exist"));
				    break;
				  }
		  case -10:{
		  	        flag=0;
				    ShowAlert(search(lwlan,"wtp_not_exist"));
				    break;
				  }
		  case -11:{
		  	         flag=0;
				     ShowAlert(search(lwlan,"wtp_not_bind_wlan"));
				     break;
				   }
		  case -12:{
		  	         flag=0;
				     ShowAlert(search(lwlan,"wlan_br_not_exist"));
				     break;
				   }
		  case -13:{
		  	         flag=0;
				     ShowAlert(search(lwlan,"add_bss_inter_to_wlanbr_fail"));
				     break;
				   }
		  case -14:{
		  	         flag=0;
				     ShowAlert(search(lwlan,"remove_bss_inter_from_wlan_br_fail"));
				     break;
				   }
		  case -15:{
	  		         flag=0;
				     memset(temp,0,sizeof(temp));
				     strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
				     memset(max_radio_num,0,sizeof(max_radio_num));
				     snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				     strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
				     strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		  		     ShowAlert(temp);
			         break; 
		           }
		  case -19:{
	  		         flag=0;
				     memset(temp,0,sizeof(temp));
				     strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
				     memset(max_wlan_num,0,sizeof(max_wlan_num));
				     snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				     strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
				     strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
				     ShowAlert(temp);
			         break; 
		           }
		  case -20:{
			         flag=0;
		             ShowAlert(search(lwlan,"wlan_not_exist"));
	                 break; 
		           }
		  case -21:{
	  		         flag=0;
				     ShowAlert(search(lwlan,"radio_not_bind_wlan"));
			         break; 
		           }
		}
	}

	if(flag==1)
  		ShowAlert(search(lpublic,"oper_succ"));
	
	return flag;
}


