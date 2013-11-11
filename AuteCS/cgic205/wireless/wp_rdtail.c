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
* wp_rdtail.c
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
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sta.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"

void ShowRdtailPage(char *m,char *rid,char *n,char *wtp_id,char *flag,char * ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);  
void RadDeleteWlan(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);
void RadEnableWlan(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);
void RadDisableWlan(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan);



int cgiMain()
{
  char encry[BUF_LEN] = { 0 };
  char ID[10] = { 0 };
  char wtp_id[10] = { 0 };
  char instance_id[10] = { 0 };
  char flag[5] = { 0 }; /*fla=="1",表示上一页为wp_radiolis.cgi,否则上一页为wp_wtpdta.cgi*/ 
  char *str = NULL;             
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
  memset(wtp_id,0,sizeof(wtp_id));
  memset(flag,0,sizeof(flag));
  memset(instance_id,0,sizeof(instance_id));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  cgiFormStringNoNewlines("ID", ID, 10); 
	  cgiFormStringNoNewlines("WID", wtp_id, 10); 
	  cgiFormStringNoNewlines("FL", flag, 5);
	  cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {
      cgiFormStringNoNewlines("encry_raddta",encry,BUF_LEN);
	  cgiFormStringNoNewlines("radid",ID,10);
	  cgiFormStringNoNewlines("wtp_id", wtp_id, 10); 
	  cgiFormStringNoNewlines("FLAG",flag,5);
	  cgiFormStringNoNewlines("instance_id",instance_id,10);
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
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowRdtailPage(encry,ID,str,wtp_id,flag,instance_id,paraHead1,lpublic,lwlan);

  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

void ShowRdtailPage(char *m,char *rid,char *n,char *wtp_id,char *flag,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{   
  char pno[10] = { 0 };  
  char menu[15]="menuLists";
  char menu_id[10] = { 0 };
  DCLI_RADIO_API_GROUP_ONE *radio = NULL;
  char radio_rate[100] = { 0 };
  char rate[10] = { 0 };
  char whichinterface[RADIO_IF_NAME_LEN] = { 0 };
  char bwlanid[40] = { 0 };
  char tembwid[5] = { 0 };  
  memset(radio_rate,0,sizeof(radio_rate));
  memset(whichinterface,0,sizeof(whichinterface));
  char *endptr = NULL;  
  char RType[10] = { 0 };
  int i = 0,radio_id = 0,radio_local_id = 0,cl = 1,ret = 0,retu = 1,limit = 0,ret2 = 0;             /*颜色初值为#f9fafe*/
  int result = 0;
  int result1 = 0;
  DCLI_RADIO_API_GROUP_ONE *rad_qos_head = NULL;
  char alt[100] = { 0 };
  char max_radio_num[10] = { 0 };
  DCLI_RADIO_API_GROUP_ONE *radio_info = NULL;
  char txantenna_policy[3][5] = {"AUTO","MAIN","VICE"};
  struct dcli_bss_info *bss = NULL;
  char *maclist_name[3]={"none","black","white"};
  char Tx_chainmask[10] = { 0 };
  char Rx_chainmask[10] = { 0 };
  char wlan_id[5] = { 0 };
  int WlanID = 0,BSSIndex = 0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Radio Detail</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
  	"th{ font-family:Arial, Helvetica, sans-serif; font-weight:bold; font-size:12px; color:#0a4e70}"\
  	"#div1{ width:92px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:90px; height:15px; padding-left:5px; padding-top:3px}"\
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
				 "function checkAll(e, itemName)"\
				 "{"\
				   "var aa = document.getElementsByName(itemName);"\
				   "for (var x=0; x<aa.length; x++)"\
					 "aa[x].checked = e.checked;"\
				 "}"\
				 "</script>"\

  "<body>");
  radio_id= strtoul(rid,&endptr,10);   /*char转成int，10代表十进制*/		  
  retu=checkuser_group(n); 
  if(ins_para)
  {
	  ret=show_radio_one(ins_para->parameter,ins_para->connection,radio_id,&radio);
  }
  
  if(ret == 1)
  {
	  if((retu==0)&&(radio->wlan_num>0))     /*管理员且存在绑定的WLAN*/
	  {
	    if(cgiFormSubmitClicked("RadDeleteWlan_apply") == cgiFormSuccess)
	    {
	    	if(ins_para)
		    {
				RadDeleteWlan(ins_para,radio_id,lpublic,lwlan);
		    }
	    }
		if(cgiFormSubmitClicked("RadEnableWlan_apply") == cgiFormSuccess)
		{
	    	if(ins_para)
		    {
				RadEnableWlan(ins_para,radio_id,lpublic,lwlan);
		    }
	    }
		if(cgiFormSubmitClicked("RadDisableWlan_apply") == cgiFormSuccess)
		{
	    	if(ins_para)
		    {
				RadDisableWlan(ins_para,radio_id,lpublic,lwlan);
		    }
	    }
	  }
  }
  if(ret==1)
  {
    Free_radio_one_head(radio);
  }	
  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>");
    if(strcmp(flag,"0")==0)
      fprintf(cgiOut,"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
	else
	  fprintf(cgiOut,"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>RF</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
		memset(pno,0,sizeof(pno));
		cgiFormStringNoNewlines("PN",pno,10);
	    	
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
		  if(strcmp(flag,"0")==0)
		  {
            fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtpdta.cgi?UN=%s&ID=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,wtp_id,ins_id,search(lpublic,"img_ok"));
			fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtpdta.cgi?UN=%s&ID=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,wtp_id,ins_id,search(lpublic,"img_cancel"));
		  }
		  else
		  {
	        fprintf(cgiOut,"<td width=62 align=center><a href=wp_radiolis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,ins_id,search(lpublic,"img_ok"));	  	
			fprintf(cgiOut,"<td width=62 align=center><a href=wp_radiolis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,ins_id,search(lpublic,"img_cancel"));	  	
		  }
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>Radio</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"details"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
			      if(strcmp(flag,"0")==0)
			      {
                    fprintf(cgiOut,"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
	                fprintf(cgiOut,"</tr>");
					if(retu==0)  /*管理员*/
					{
	                  fprintf(cgiOut,"<tr height=25>"\
	  				    "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
	                  fprintf(cgiOut,"</tr>");
					}
		fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpgrouplist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"ap_group_list"));			  
		  fprintf(cgiOut,"</tr>");
		  if(retu==0)  /*管理员*/
		  {
		fprintf(cgiOut,"<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpgroupnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"create_apgroup"));			  
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
					fprintf(cgiOut,"<tr height=25>"\
	  				  "<td align=left id=tdleft><a href=wp_showAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"show_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
	                fprintf(cgiOut,"</tr>");
					if(retu==0) /*管理员*/
					{
	                  fprintf(cgiOut,"<tr height=25>"\
	  				    "<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
	                  fprintf(cgiOut,"</tr>");
					}				  
			      }
			      else
				  {
				  	if(retu==0) /*管理员*/
				  	{
					  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_bssbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter")); 					  
					  fprintf(cgiOut,"</tr>");
				  	}
			      }				  
				  
				  ret=0;
				  if(ins_para)
				  {
					  ret=show_radio_one(ins_para->parameter,ins_para->connection,radio_id,&radio);
				  }
				  if(ret == 1)
				  {
					  if(strcmp(flag,"0")==0)
					  	limit=9+radio->bss_num;
					  else
					  	limit=15+radio->bss_num;

					  if((radio->RADIO[0]->Radio_Type&IEEE80211_11N)>0)
					  {
					  	limit+=7;
					  }
				  }

				  if(ins_para)
				  {
					  result1=show_radio_qos_cmd_func(ins_para->parameter,ins_para->connection,&rad_qos_head);
					  ret2=show_radio_bss_cmd(ins_para->parameter,ins_para->connection,radio_id,&radio_info);				 
				  }
				 
				  if(result1==1)
				  {
					for(i=0;i<rad_qos_head->qos_num;i++)
					{
						if((rad_qos_head->RADIO[i])&&(rad_qos_head->RADIO[i]->Radio_G_ID==radio_id)&&(rad_qos_head->RADIO[i]->QOSID!=0))
						{
							limit+=1;
						}
					}
				  }
				  
				   
				  if(ret == 1)
				  {
					  if(radio->wlan_num>0)
					  {
					  	if(retu==0) /*管理员*/
					  	  limit+=radio->wlan_num+3;
						else
						  limit+=2;
					  }
				  }					
					
				  if(ret2==1)
				  	limit+=3;
				  
				  if(retu==1) /*普通用户*/
				  {
				  	if(strcmp(flag,"0")==0)
					  limit+=5;
					else
					  limit+=1;
				  }

				  if((radio)&&(radio->RADIO[0])&&((radio->RADIO[0]->Radio_Type&IEEE80211_11N)>0))
				  {
				  	limit+=7;
				  }
				  
 			  	  for(i=0;i<limit;i++)
				  {
					fprintf(cgiOut,"<tr height=25>"\
					  "<td id=tdleft>&nbsp;</td>"\
					"</tr>");
				  }
				 
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
 "<table width=763 height=240 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
 "<tr>"\
 "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
 fprintf(cgiOut,"</tr>"\
   "<tr>"\
    "<td align=center>");
			
			if(ret==1)	 /*显示所有wlan的信息，wlan_head返回wlan信息链表的链表头*/
			{			
			   if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->Support_Rate_Count != 0))
	           { 
	           	 for (i=0;i<(radio->RADIO[0]->Support_Rate_Count);i++)
             	 {
					memset(rate,0,sizeof(rate));
					snprintf(rate,sizeof(rate)-1,"%0.1f",(*(radio->RADIO[0]->RadioRate[i]))/10.0);
					strncat(radio_rate,rate,sizeof(radio_rate)-strlen(radio_rate)-1);
					strncat(radio_rate," ",sizeof(radio_rate)-strlen(radio_rate)-1);
			 	 }
			   }
			   
			  fprintf(cgiOut,"<table width=763 border=0 cellspacing=0 cellpadding=0>"\
	          "<tr align=left height=10>"\
	          "<td id=thead3>Radio %s</td>",search(lpublic,"details"));
	          fprintf(cgiOut,"</tr>"\
              "<tr>"\
              "<td align=left style=\"padding-left:20px\"><table frame=below rules=rows width=460 border=1>"\
              "<tr align=left>"\
                "<td id=td1 width=130>Radio ID</td>"\
                "<td id=td2 width=330>%d</td>",radio_id);
              fprintf(cgiOut,"</tr>"\
              "<tr align=left>"\
                "<td id=td1>AP ID</td>");
			    if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->WTPID);			  
			    }
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>Radio Local ID</td>");
			  	if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->Radio_L_ID);			  
			    }
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"channel_state"));
			  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->auto_channel_cont==0))
			  {
				fprintf(cgiOut,"<td id=td2>auto</td>");
			  }
			  else
			  {
                fprintf(cgiOut,"<td id=td2>manual</td>");
			  }
			  fprintf(cgiOut,"</tr>"\
              "<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"channel"));
			  if((radio)&&(radio->RADIO[0]))
			  {
				  if(radio->RADIO[0]->Radio_Chan==0)
				  {
					fprintf(cgiOut,"<td id=td2>%s</td>","auto");
				  }
				  else
				  {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->Radio_Chan);
				  }
			  }			  
			  fprintf(cgiOut,"</tr>"\
              "<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"tx_power"));
			    if((radio)&&(radio->RADIO[0]))
			    {
					if((radio->RADIO[0]->Radio_TXP == 0)||((radio->RADIO[0]->Radio_TXP == 100)))
					  fprintf(cgiOut,"<td id=td2>auto</td>");
					else	
					  fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->Radio_TXP);
			    }
              fprintf(cgiOut,"</tr>"\
			  /*"<tr align=left>"\
                "<td id=td1>uptime</td>");
                if((radio)&&(radio->RADIO[0]))
                {
               	 fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->upcount);			  
                }
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>downtime</td>");
                if((radio)&&(radio->RADIO[0]))
                {
	                fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->downcount);			  
                }
              fprintf(cgiOut,"</tr>"\*/
              "<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"ad_state"));
			    if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->AdStat== 2))
                  fprintf(cgiOut,"<td id=td2>disable</td>");
		        else
				  fprintf(cgiOut,"<td id=td2>enable</td>");
              fprintf(cgiOut,"</tr>"\
              "<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"op_state"));
			    if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->OpStat== 2))
			      fprintf(cgiOut,"<td id=td2>disable</td>");
		        else
				  fprintf(cgiOut,"<td id=td2>enable</td>");
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>Support Radio Count</td>");
			    if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->Support_Rate_Count);
			    }
			  fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>Radio %s</td>",search(lwlan,"rate"));
			  fprintf(cgiOut,"<td id=td2>%s %s</td>",radio_rate,"M/bps");
			  fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"fragment"));
			    if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->FragThreshold);
			    }
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>Beacon</td>");
			    if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->BeaconPeriod);
			    }
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"pream"));
			    if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->IsShortPreamble==1))
                  fprintf(cgiOut,"<td id=td2>short</td>");
				else
				  fprintf(cgiOut,"<td id=td2>long</td>");
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>DTIM</td>");
			    if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->DTIMPeriod);
			    }
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"rtsde"));
			    if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->rtsthreshold);
			    }
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>%s</td>","ShortRetry");
			    if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->ShortRetry);
			    }
              fprintf(cgiOut,"</tr>"\
			  "<tr align=left>"\
                "<td id=td1>%s</td>","LongRetry");
			    if((radio)&&(radio->RADIO[0]))
			    {
					fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->LongRetry);
			    }
              fprintf(cgiOut,"</tr>");  
			  memset(RType,0,sizeof(RType));
			  if((radio)&&(radio->RADIO[0]))
			  {
				  Radio_Type(radio->RADIO[0]->Radio_Type,RType);
			  }
              fprintf(cgiOut,"<tr align=left>"\
              "<td id=td1>Radio %s</td>",search(lwlan,"type"));
              fprintf(cgiOut,"<td id=td2>%s</td>",RType);
              fprintf(cgiOut,"</tr>");
			  
			    if(result1==1)
				{
					for(i=0;i<rad_qos_head->qos_num;i++)
					{
						if(rad_qos_head->RADIO[i]&&(rad_qos_head->RADIO[i]->Radio_G_ID==radio_id)&&(rad_qos_head->RADIO[i]->QOSID>0))
						{
							fprintf(cgiOut,"<tr align=left>"\
							  "<td id=td1>%s QOS</td>",search(lpublic,"bind"));
							  fprintf(cgiOut,"<td id=td2>%d</td>",rad_qos_head->RADIO[i]->QOSID);
							fprintf(cgiOut,"</tr>"); 
						}
					}
				}
			  /*fprintf(cgiOut,"<tr align=left>"\
                "<td id=td1>%s</td>",search(lwlan,"rad_max_throughout"));
                fprintf(cgiOut,"<td id=td2>%d</td>",rad_throught);
              fprintf(cgiOut,"</tr>");*/
			  if(ret2==1)
			  {
				  fprintf(cgiOut,"<tr align=left>");
				    if(strcmp(search(lwlan,"diversity"),"Diversity")==0)/*英文*/
				  		fprintf(cgiOut,"<td id=td1>%s %s</td>",search(lwlan,"auto_channel"),search(lpublic,"switch"));
					else
						fprintf(cgiOut,"<td id=td1>%s%s</td>",search(lwlan,"auto_channel"),search(lpublic,"switch"));
				  if((radio_info)&&(radio_info->RADIO[0])&&(radio_info->RADIO[0]->auto_channel==1))
					fprintf(cgiOut,"<td id=td2>enable</td>");
				  else
				  	fprintf(cgiOut,"<td id=td2>disable</td>");
				  fprintf(cgiOut,"</tr>"\
				  "<tr align=left>");
				    if(strcmp(search(lwlan,"diversity"),"Diversity")==0)/*英文*/
						fprintf(cgiOut,"<td id=td1>%s %s</td>",search(lwlan,"diversity"),search(lpublic,"switch"));
					else
						fprintf(cgiOut,"<td id=td1>%s%s</td>",search(lwlan,"diversity"),search(lpublic,"switch"));
				  if((radio_info)&&(radio_info->RADIO[0])&&(radio_info->RADIO[0]->diversity==1))
					fprintf(cgiOut,"<td id=td2>enable</td>");
				  else
				  	fprintf(cgiOut,"<td id=td2>disable</td>");
				  fprintf(cgiOut,"</tr>"\
				  "<tr align=left>"\
					"<td id=td1>%s</td>",search(lwlan,"txantenna"));
				    if((radio_info)&&(radio_info->RADIO[0]))
				    {
						fprintf(cgiOut,"<td id=td2>%s</td>",txantenna_policy[radio_info->RADIO[0]->txantenna]);
				    }
				  fprintf(cgiOut,"</tr>");
				  if((radio->RADIO[0]->Radio_Type&IEEE80211_11N)>0)
				  {
					  fprintf(cgiOut,"<tr align=left>"\
						"<td id=td1>ampdu %s</td>",search(lpublic,"switch"));
						if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->Ampdu.Able == 1))
						  fprintf(cgiOut,"<td id=td2>enable</td>");
						else
						  fprintf(cgiOut,"<td id=td2>disable</td>");
					  fprintf(cgiOut,"</tr>"\
					  "<tr align=left>"\
						"<td id=td1>ampdu %s</td>",search(lwlan,"limit"));
						if((radio)&&(radio->RADIO[0]))
						{
							fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->Ampdu.AmpduLimit);
						}
					  fprintf(cgiOut,"</tr>"\
					  "<tr align=left>"\
						"<td id=td1>ampdu subframe</td>");
						if((radio)&&(radio->RADIO[0]))
						{
							fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->Ampdu.subframe);
						}
					  fprintf(cgiOut,"</tr>"\
					  "<tr align=left>"\
						"<td id=td1>amsdu %s</td>",search(lpublic,"switch"));
						if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->Amsdu.Able == 1))
						  fprintf(cgiOut,"<td id=td2>enable</td>");
						else
						  fprintf(cgiOut,"<td id=td2>disable</td>");
					  fprintf(cgiOut,"</tr>"\
					  "<tr align=left>"\
						"<td id=td1>amsdu %s</td>",search(lwlan,"limit"));
						if((radio)&&(radio->RADIO[0]))
						{
							fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->Amsdu.AmsduLimit);
						}
					  fprintf(cgiOut,"</tr>"\
					  "<tr align=left>"\
						"<td id=td1>amsdu subframe</td>");
						if((radio)&&(radio->RADIO[0]))
						{
							fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->Amsdu.subframe);
						}
					  fprintf(cgiOut,"</tr>"\
					  "<tr align=left>"\
						"<td id=td1>11n %s</td>",search(lwlan,"work_mode"));
						if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->MixedGreenfield.Mixed_Greenfield == 1))
						  fprintf(cgiOut,"<td id=td2>pureN</td>");
						else
						  fprintf(cgiOut,"<td id=td2>Mixed</td>");
					  fprintf(cgiOut,"</tr>");
					  if((radio)&&(radio->RADIO[0]))
					  {
						  fprintf(cgiOut,"<tr align=left>"\
							"<td id=td1>%s</td>",search(lwlan,"channel_offset"));
							if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->channel_offset == 0))
							  fprintf(cgiOut,"<td id=td2>none</td>");
							if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->channel_offset == -1))
							  fprintf(cgiOut,"<td id=td2>down</td>");
							if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->channel_offset == 1))
							  fprintf(cgiOut,"<td id=td2>up</td>");
						  fprintf(cgiOut,"</tr>"\
						  "<tr align=left>"\
							"<td id=td1>11n %s</td>",search(lwlan,"chainmask_num"));
							if((radio)&&(radio->RADIO[0]))
							{
								fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->chainmask_num);
							}
						  fprintf(cgiOut,"</tr>"\
						  "<tr align=left>"\
							"<td id=td1>TX%s</td>",search(lwlan,"chainmask"));
							memset(Tx_chainmask,0,sizeof(Tx_chainmask));				
							if((radio)&&(radio->RADIO[0])&&((radio->RADIO[0]->tx_chainmask_state_value & 0x4)>0))
								strncat(Tx_chainmask,"1:",sizeof(Tx_chainmask)-strlen(Tx_chainmask)-1);
							else
								strncat(Tx_chainmask,"0:",sizeof(Tx_chainmask)-strlen(Tx_chainmask)-1);
							if((radio)&&(radio->RADIO[0])&&((radio->RADIO[0]->tx_chainmask_state_value & 0x2)>0))
								strncat(Tx_chainmask,"1:",sizeof(Tx_chainmask)-strlen(Tx_chainmask)-1);
							else
								strncat(Tx_chainmask,"0:",sizeof(Tx_chainmask)-strlen(Tx_chainmask)-1);
							if((radio)&&(radio->RADIO[0])&&((radio->RADIO[0]->tx_chainmask_state_value & 0x1)>0))
								strncat(Tx_chainmask,"1",sizeof(Tx_chainmask)-strlen(Tx_chainmask)-1);
							else
								strncat(Tx_chainmask,"0",sizeof(Tx_chainmask)-strlen(Tx_chainmask)-1);
							fprintf(cgiOut,"<td id=td2>%s</td>",Tx_chainmask);
						  fprintf(cgiOut,"</tr>"\
						  "<tr align=left>"\
							"<td id=td1>RX%s</td>",search(lwlan,"chainmask"));
							memset(Rx_chainmask,0,sizeof(Rx_chainmask));				
							if((radio)&&(radio->RADIO[0])&&((radio->RADIO[0]->rx_chainmask_state_value & 0x4)>0))
								strncat(Rx_chainmask,"1:",sizeof(Rx_chainmask)-strlen(Rx_chainmask)-1);
							else
								strncat(Rx_chainmask,"0:",sizeof(Rx_chainmask)-strlen(Rx_chainmask)-1);
							if((radio)&&(radio->RADIO[0])&&((radio->RADIO[0]->rx_chainmask_state_value & 0x2)>0))
								strncat(Rx_chainmask,"1:",sizeof(Rx_chainmask)-strlen(Rx_chainmask)-1);
							else
								strncat(Rx_chainmask,"0:",sizeof(Rx_chainmask)-strlen(Rx_chainmask)-1);
							if((radio)&&(radio->RADIO[0])&&((radio->RADIO[0]->rx_chainmask_state_value & 0x1)>0))
								strncat(Rx_chainmask,"1",sizeof(Rx_chainmask)-strlen(Rx_chainmask)-1);
							else
								strncat(Rx_chainmask,"0",sizeof(Rx_chainmask)-strlen(Rx_chainmask)-1);
							fprintf(cgiOut,"<td id=td2>%s</td>",Rx_chainmask);
						  fprintf(cgiOut,"</tr>"\
						  "<tr align=left>"\
							"<td id=td1>%s</td>",search(lwlan,"cwmode"));
							if((radio)&&(radio->RADIO[0]))
							{
								if(radio->RADIO[0]->cwmode == 0)
								  fprintf(cgiOut,"<td id=td2>ht20</td>");
								else if(radio->RADIO[0]->cwmode == 1)
								  fprintf(cgiOut,"<td id=td2>ht20/40</td>");
								else if(radio->RADIO[0]->cwmode == 2)
								  fprintf(cgiOut,"<td id=td2>ht40</td>");
							}
						  fprintf(cgiOut,"</tr>"\
						  "<tr align=left>"\
							"<td id=td1>ShortGI</td>");
							if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->guardinterval == 1))
							  fprintf(cgiOut,"<td id=td2>400ns</td>");
							else
							  fprintf(cgiOut,"<td id=td2>800ns</td>");
						  fprintf(cgiOut,"</tr>");
					  }
				  }
			  }
			  if((radio)&&(radio->RADIO[0]))
			  {
				  radio_local_id=radio->RADIO[0]->Radio_L_ID;
			  }
		     if(radio->wlan_num==0)
			  { 
			    fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"apply_wlan"));
				  fprintf(cgiOut,"<td id=td2>NONE</td>"\
				"</tr>");
			  }
			  else
			  {
			    if(retu==0)  /*管理员*/
			    {
				  fprintf(cgiOut,"<tr align=left>");
				  fprintf(cgiOut,"<td id=td1>%s</td>",search(lwlan,"apply_wlan"));
					fprintf(cgiOut,"<td id=td2><input type=submit style=\"width:40px\" border=0 name=RadDeleteWlan_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"><input type=submit style=\"width:40px; margin-left:5px\" border=0 name=RadEnableWlan_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"><input type=submit style=\"width:40px; margin-left:5px\" border=0 name=RadDisableWlan_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lpublic,"delete"),search(lpublic,"version_enable"),search(lwlan,"wtp_dis"));			
			      fprintf(cgiOut,"</tr>");
					
   				  for(i=0;i<radio->wlan_num;i++)				  
				  {
				    memset(tembwid,0,sizeof(tembwid));
					if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->WlanId != NULL))
						snprintf(tembwid,sizeof(tembwid)-1,"%d",radio->RADIO[0]->WlanId[i]); 	/*int转成char*/
					fprintf(cgiOut,"<tr align=left>"\
				    "<td colspan=2 id=td2 style=\"padding-left:170px\"><input type=checkbox name=Radbindwlan value=%s>wlan %s</td>",tembwid,tembwid);
				    fprintf(cgiOut,"</tr>");
			      }
				  fprintf(cgiOut,"<tr align=left>");
				      fprintf(cgiOut,"<td colspan=2 id=td1 style=\"padding-left:170px\"><input type=checkbox name=Raddelwlan_all value=all onclick=\"checkAll(this,'Radbindwlan')\">%s</td>",search(lwlan,"all"));
				  fprintf(cgiOut,"</tr>");
		      	}
				else
			    {
			      fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"apply_wlan"));
    			  memset(bwlanid,0,sizeof(bwlanid));
    			  for(i=0;i<radio->wlan_num;i++)			
    			  {
    			    memset(tembwid,0,sizeof(tembwid));
				    if(i==radio->wlan_num-1)
				    {
						if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->WlanId != NULL))
				  		  snprintf(tembwid,sizeof(tembwid)-1,"%d",radio->RADIO[0]->WlanId[i]); 	/*int转成char*/
				    }
				    else 
				    {
						if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->WlanId != NULL))
                      	  snprintf(tembwid,sizeof(tembwid)-1,"%d,",radio->RADIO[0]->WlanId[i]); 	/*int转成char*/
				    }
    			    strncat(bwlanid,tembwid,sizeof(bwlanid)-strlen(bwlanid)-1);
    			  }
			  	  fprintf(cgiOut,"<td id=td2>%s</td>",bwlanid);
				  fprintf(cgiOut,"</tr>");
			    }
			  }
             fprintf(cgiOut,"</table>"\
              "</td>"\
              "</tr>");
			 
			 
			  if( ret == 1)
			  {
                fprintf(cgiOut,"<tr align=left style=padding-top:20px;>");
                  fprintf(cgiOut,"<td id=thead4>Radio%d BSS%s</td>",radio_id,search(lpublic,"summary"));
                fprintf(cgiOut,"</tr>"\
                "<tr>");
				if(retu==0)/*管理员*/
                  fprintf(cgiOut,"<td align=left style=\"padding-left:20px\"><table align=left frame=below rules=rows width=763 border=1>");
				else
				  fprintf(cgiOut,"<td align=left style=\"padding-left:20px\"><table align=left frame=below rules=rows width=750 border=1>");
                fprintf(cgiOut,"<tr align=left height=20>"\
				"<th width=60>WLAN ID</th>"\
                "<th width=70>Keyindex</th>"\
                "<th width=70>BSSindex</th>"\
                "<th width=140>BSSID</th>"\
	            "<th width=50>%s</th>",search(lwlan,"state"));
				fprintf(cgiOut,"<th width=70>%s%s</th>",search(lpublic,"inter"),search(lpublic,"policy"));
				fprintf(cgiOut,"<th width=110>%s</th>",search(lwlan,"max_sta_num"));
				fprintf(cgiOut,"<th width=80>%s</th>",search(lwlan,"wds_mode"));
				fprintf(cgiOut,"<th width=100>%s</th>",search(lwlan,"mac_filter_type"));
				fprintf(cgiOut,"<th width=13>&nbsp;</th>");
                fprintf(cgiOut,"</tr>");				
			    for(i=0;i<radio->bss_num;i++)
		        {
		          snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
		          strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
                  fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
				  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
				  {
					  fprintf(cgiOut,"<td id=td3>%d</td>",radio->RADIO[0]->BSS[i]->WlanID);
					  fprintf(cgiOut,"<td id=td3>%d</td>",radio->RADIO[0]->BSS[i]->keyindex);
				  }
				  if((radio_info)&&(radio_info->RADIO[0])&&(radio_info->RADIO[0]->BSS[i]))
				  {
					  fprintf(cgiOut,"<td id=td3>%d</td>",radio_info->RADIO[0]->BSS[i]->BSSIndex);
				  }
				  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i])&&(radio->RADIO[0]->BSS[i]->BSSID))
				  {
					  fprintf(cgiOut,"<td id=td3>%02X:%02X:%02X:%02X:%02X:%02X</td>",radio->RADIO[0]->BSS[i]->BSSID[0],radio->RADIO[0]->BSS[i]->BSSID[1],radio->RADIO[0]->BSS[i]->BSSID[2],radio->RADIO[0]->BSS[i]->BSSID[3],radio->RADIO[0]->BSS[i]->BSSID[4],radio->RADIO[0]->BSS[i]->BSSID[5]);
				  }
				  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i])&&(radio->RADIO[0]->BSS[i]->State==0))
			        fprintf(cgiOut,"<td id=td3>disable</td>");
				  else				  	
				    fprintf(cgiOut,"<td id=td3>enable</td>");				  
				  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
				  {
					  CheckWIDIfPolicy(whichinterface,radio->RADIO[0]->BSS[i]->BSS_IF_POLICY);
				  }
				  fprintf(cgiOut,"<td id=td3>%s</td>",whichinterface);
				  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
				  {
					  fprintf(cgiOut,"<td id=td3>%d</td>",radio->RADIO[0]->BSS[i]->bss_max_allowed_sta_num);
					  if(radio->RADIO[0]->BSS[i]->WDSStat == 0)
						fprintf(cgiOut,"<td id=td3>disable</td>");
					  else if(radio->RADIO[0]->BSS[i]->WDSStat == 1)
						fprintf(cgiOut,"<td id=td3>ANY</td>");
					  else if(radio->RADIO[0]->BSS[i]->WDSStat == 2)
						fprintf(cgiOut,"<td id=td3>SOME</td>"); 			  
				  }
				  memset(wlan_id,0,sizeof(wlan_id));
				  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
				  {
					  snprintf(wlan_id,sizeof(wlan_id)-1,"%d",radio->RADIO[0]->BSS[i]->WlanID);
				  }
				  if(ins_para)
				  {
					  result=show_radio_bss_mac_list(ins_para->parameter,ins_para->connection,radio_id,wlan_id,&bss);
				  }
				  if(result==1)
				    fprintf(cgiOut,"<td id=td3>%s</td>",maclist_name[bss->acl_conf.macaddr_acl]);	
				  else
				    fprintf(cgiOut,"<td id=td3 align=center>%s</td>","-");

				 if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
				 {
				 	WlanID = radio->RADIO[0]->BSS[i]->WlanID;
				 }
				 else
				 {
				 	WlanID = 0;
				 }
				 if((radio_info)&&(radio_info->RADIO[0])&&(radio_info->RADIO[0]->BSS[i]))
				 {
				 	BSSIndex = radio_info->RADIO[0]->BSS[i]->BSSIndex;
				 }
				 else
				 {
				 	BSSIndex = 0;
				 }
			     fprintf(cgiOut,"<td align=left>"\
			  	               "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(radio->bss_num-i),menu,menu);
                               fprintf(cgiOut,"<img src=/images/detail.gif>"\
                               "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                               fprintf(cgiOut,"<div id=div1>");
							   if(retu==0)/*管理员*/
							   {
                                 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_bsscon.cgi?UN=%s&ID=%d&RID=%d&FL=%s&INSTANCE_ID=%s&WID=%s&PN=%s&WLANID=%d&BSSindex=%d target=mainFrame>%s</a></div>",m,i+1,radio_id,flag,ins_id,wtp_id,pno,WlanID,BSSIndex,search(lpublic,"configure"));
							   }
							   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_bssdta.cgi?UN=%s&RID=%d&WLANID=%d&FL=%s&INSTANCE_ID=%s&WID=%s&PN=%s&BSSindex=%d target=mainFrame>%s</a></div>",m,radio_id,WlanID,flag,ins_id,wtp_id,pno,BSSIndex,search(lpublic,"details"));
							   if(retu==0)/*管理员*/
							   {
								 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_rdmap.cgi?UN=%s&BSSID=%d&RID=%d&RLID=%d&FL=%s&INSTANCE_ID=%s&WLANID=%d&WID=%s&PN=%s target=mainFrame>L3%s</a></div>",m,i,radio_id,radio_local_id,flag,ins_id,WlanID,wtp_id,pno,search(lpublic,"interface"));							     
							   }
							   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wdsBssidList.cgi?UN=%s&RID=%d&FL=%s&WID=%s&PN=%s&INSTANCE_ID=%s&WLANID=%d target=mainFrame>WDS BSSID %s</a></div>",m,radio_id,flag,wtp_id,pno,ins_id,WlanID,search(lpublic,"list"));
                               fprintf(cgiOut,"</div>"\
                               "</div>"\
                               "</div>"\
			  	 "</td>");
  				 fprintf(cgiOut,"</tr>");	
		        }     
                fprintf(cgiOut,"</table></td>"\
                "</tr>");
			  }
			  fprintf(cgiOut,"<tr>"\
			    "<td><input type=hidden name=encry_raddta value=%s></td>",m);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=radid value=%s></td>",rid);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=wtp_id value=%s></td>",wtp_id);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=FLAG value=%s></td>",flag);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=instance_id value=%s></td>",ins_id);
			  fprintf(cgiOut,"</tr>"\
              "</table>");
			}
			else if(ret==-2)
			{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"radio_id_illegal1"),sizeof(alt)-1);
				memset(max_radio_num,0,sizeof(max_radio_num));
				snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM);
				strncat(alt,max_radio_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"radio_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				fprintf(cgiOut,"%s",alt); 
			}
            else if(ret==-1)
		      fprintf(cgiOut,"%s",search(lwlan,"radio_id_not_exist")); 
			else				
			  fprintf(cgiOut,"%s",search(lpublic,"contact_adm")); 	
  fprintf(cgiOut,"</td>"\
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
if(ret==1)
{
  Free_radio_one_head(radio);
}
if(result==1)
{
  Free_mac_head(bss);
}
if(result1==1)
{
  Free_show_radio_qos(rad_qos_head);
}
if(ret2==1)
{
  Free_radio_bss_head(radio_info);
}
}

void RadDeleteWlan(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)
{
  int result = cgiFormNotFound;   
  char **responses;
  int ret = 0,flag = 1;
  char alt[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  char max_radio_num[10] = { 0 };
  
  result = cgiFormStringMultiple("Radbindwlan", &responses);
  if(result == cgiFormNotFound)         /*如果没有选择任何用户*/
  {
    flag=0;
    ShowAlert(search(lwlan,"select_wlan"));
  }
  else                  
  {
    int i = 0;	
    while((responses[i])&&(flag))
	{
		ret=set_radio_delete_wlan_cmd(ins_para->parameter,ins_para->connection,id,responses[i]); /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																								/*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
																								/*返回-4表示wlan not exist，返回-5表示radio delete wlan fail，返回-6表示Radio ID非法*/
																								/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type，返回-11表示bss is enable*/
																								/*返回-12表示radio interface is in ebr,please delete it from ebr first*/
																								/*返回-13表示you want to delete wlan, please do not operate like this*/
																								/*返回-14表示radio interface is binded to this wlan used other ESSID*/
																								/*返回-15表示please disable wlan service first*/
																								/*返回SNMPD_CONNECTION_ERROR表示connection error*/
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
			        ShowAlert(search(lpublic,"input_para_error"));
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
			        ShowAlert(search(lwlan,"radio_not_exist"));
	                flag=0;
				    break;
			      }
		  case -4:{
			        ShowAlert(search(lwlan,"wlan_not_exist"));
	                flag=0;
				    break;
			      }
		  case -5:{
			        ShowAlert(search(lwlan,"delete_wlan_fail"));
	                flag=0;
				    break;
			      }
		  case -6:{
			        memset(alt,0,sizeof(alt));
				    strncpy(alt,search(lwlan,"radio_id_illegal1"),sizeof(alt)-1);
				    memset(max_radio_num,0,sizeof(max_radio_num));
				    snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				    strncat(alt,max_radio_num,sizeof(alt)-strlen(alt)-1);
				    strncat(alt,search(lwlan,"radio_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		  		    ShowAlert(alt);
	                flag=0;
				    break;
			      }
		  case -7:{
			        ShowAlert(search(lpublic,"input_exceed_max_value"));
	                flag=0;
				    break;
			      }
		  case -11:{
			         ShowAlert(search(lwlan,"unuse_bss"));
	                 flag=0;
				     break;
			       }
		  case -12:{
			         ShowAlert(search(lwlan,"delete_inter_from_ebr"));
	                 flag=0;
				     break;
			       }
		  case -13:{
			         ShowAlert(search(lwlan,"dont_del_wlan"));
	                 flag=0;
				     break;
			       }
		  case -14:{
			         ShowAlert(search(lwlan,"radio_has_bind_wlan"));
	                 flag=0;
				     break;
			       }
		  case -15:{
			         ShowAlert(search(lwlan,"dis_wlan"));
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

void RadEnableWlan(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)
{
  int result = cgiFormNotFound;   
  char **responses;  
  int ret = 0,flag = 1;
  char temp[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  char max_radio_num[10] = { 0 };
  
  result = cgiFormStringMultiple("Radbindwlan", &responses);
  if(result == cgiFormNotFound)           /*如果没有选择任何用户*/
  {
    flag=0;
    ShowAlert(search(lwlan,"select_wlan"));
  }
  else                  
  {
    int i = 0;	
    while((responses[i])&&(flag))
    {
	  ret=set_radio_enable_wlan_cmd(ins_para->parameter,ins_para->connection,id,responses[i]);   /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																								/*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
																								/*返回-4表示wlan not exist， 返回-5表示wtp over max wep wlan count 4*/
																								/*返回-6表示radio is already enable this wlan，返回-7表示wtp binding interface not match wlan binding interface*/
																								/*返回-8表示radio is not binding this wlan，返回-9表示wlan is disable ,you should enable it first*/
																								/*返回-10表示radio enable wlan fail，返回-11表示Radio ID非法*/
																								/*返回-12表示illegal input:Input exceeds the maximum value of the parameter type*/
	  switch(ret)
	  {
	    case SNMPD_CONNECTION_ERROR:
		case 0:{
		         ShowAlert(search(lwlan,"enable_wlan_fail"));
		         flag=0;
			     break;
			   }
	    case 1:break;
		case -1:{
		          ShowAlert(search(lpublic,"input_para_error"));
			      flag=0;
				  break;
			    }
		case -2:{
				  memset(temp,0,sizeof(temp));
				  strncpy(temp,search(lwlan,"wlan_id_illegal1"),sizeof(temp)-1);
				  memset(max_wlan_num,0,sizeof(max_wlan_num));
				  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				  strncat(temp,max_wlan_num,sizeof(temp)-strlen(temp)-1);
				  strncat(temp,search(lwlan,"wlan_id_illegal2"),sizeof(temp)-strlen(temp)-1);
		          ShowAlert(temp);
			      flag=0;
		          break;
			    }
   	    case -3:{
		          ShowAlert(search(lwlan,"radio_not_exist"));
                  flag=0;
			      break;
			    }
  		case -4:{
   		          ShowAlert(search(lwlan,"wlan_not_exist"));
                  flag=0;
   			      break;
   			    }
		case -5:{
				  memset(temp,0,sizeof(temp));
                  strncpy(temp,search(lwlan,"wtp_over_wep_wlan_count1"),sizeof(temp)-1);
             	  strncat(temp,"4",sizeof(temp)-strlen(temp)-1);
                  strncat(temp,search(lwlan,"wtp_over_wep_wlan_count2"),sizeof(temp)-strlen(temp)-1);
                  ShowAlert(temp);
                  flag=0;
   			      break;
   			    }
		case -6:{
   		          ShowAlert(search(lwlan,"rad_already_enable_wlan"));
                  flag=0;
   			      break;
   			    }
   		case -7:{
   		          ShowAlert(search(lwlan,"interface_not_match"));
                  flag=0;
   			      break;
   			    }
		case -8:{
   		          ShowAlert(search(lwlan,"radio_not_bind_wlan"));
                  flag=0;
   			      break;
   			    }
		case -9:{
   		          ShowAlert(search(lwlan,"enable_wlan"));
                  flag=0;
   			      break;
   			    }
   		case -10:{
   		           ShowAlert(search(lwlan,"enable_wlan_fail"));
                   flag=0;
   			       break;
   			     }
		case -11:{
		           memset(temp,0,sizeof(temp));
			       strncpy(temp,search(lwlan,"radio_id_illegal1"),sizeof(temp)-1);
			       memset(max_radio_num,0,sizeof(max_radio_num));
			       snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
			       strncat(temp,max_radio_num,sizeof(temp)-strlen(temp)-1);
			       strncat(temp,search(lwlan,"radio_id_illegal2"),sizeof(temp)-strlen(temp)-1);
	  		       ShowAlert(temp);
                   flag=0;
			       break;
		         }
	    case -12:{
		           ShowAlert(search(lpublic,"input_exceed_max_value"));
                   flag=0;
			       break;
		         }
	  }
	  i++;
	}
  }  
  cgiStringArrayFree(responses);
  if(flag==1)
  	ShowAlert(search(lwlan,"enable_wlan_succ"));
}

void RadDisableWlan(instance_parameter *ins_para,int id,struct list *lpublic,struct list *lwlan)
{
  int result = cgiFormNotFound;   
  char **responses;  
  int ret = 0,flag = 1;
  char alt[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  char max_radio_num[10] = { 0 };
  
  result = cgiFormStringMultiple("Radbindwlan", &responses);
  if(result == cgiFormNotFound)           /*如果没有选择任何用户*/
  {
    flag=0;
    ShowAlert(search(lwlan,"select_wlan"));
  }
  else                  
  {
    int i = 0;	
    while((responses[i])&&(flag))
    {
	  ret=set_radio_disable_wlan_cmd(ins_para->parameter,ins_para->connection,id,responses[i]);  /*返回0表示失败，返回1表示成功，返回-1表示input parameter error*/
																								/*返回-2表示input parameter should be 1 to WLAN_NUM-1，返回-3表示radio not exist*/
																								/*返回-4表示wlan not exist，返回-5表示radio disable wlan fail，返回-6表示Radio ID非法*/
																								/*返回-7表示illegal input:Input exceeds the maximum value of the parameter type*/
	  switch(ret)
	  {
	    case SNMPD_CONNECTION_ERROR:
		case 0:{
		         ShowAlert(search(lwlan,"disable_wlan_fail"));
		         flag=0;
			     break;
			   }
	    case 1:break;
		case -1:{
		          ShowAlert(search(lpublic,"input_para_error"));
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
		          ShowAlert(search(lwlan,"radio_not_exist"));
                  flag=0;
			      break;
			    }
   		case -4:{
   		          ShowAlert(search(lwlan,"wlan_not_exist"));
                  flag=0;
   			      break;
   			    }
        case -5:{
		          ShowAlert(search(lwlan,"disable_wlan_fail"));
                  flag=0;
			      break;
			    }
		case -6:{
		          memset(alt,0,sizeof(alt));
			      strncpy(alt,search(lwlan,"radio_id_illegal1"),sizeof(alt)-1);
			      memset(max_radio_num,0,sizeof(max_radio_num));
			      snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
			      strncat(alt,max_radio_num,sizeof(alt)-strlen(alt)-1);
			      strncat(alt,search(lwlan,"radio_id_illegal2"),sizeof(alt)-strlen(alt)-1);
	  		      ShowAlert(alt);
                  flag=0;
			      break;
		        }
	    case -7:{
		          ShowAlert(search(lpublic,"input_exceed_max_value"));
                  flag=0;
			      break;
		        }
	  }
	  i++;
	}
  }  
  cgiStringArrayFree(responses);
  if(flag==1)
  	ShowAlert(search(lwlan,"disable_wlan_succ"));
}


