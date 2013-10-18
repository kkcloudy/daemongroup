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
* wp_bssdta.c
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
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "ws_public.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"

void ShowBSSdtailPage(char *m,char *n,char *rid,char *wlan_id,char *bss_index,char *flag,char *wtp_id,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan);  
void CleanVlan(instance_parameter *ins_para,int rid,char *wid,struct list *lpublic,struct list *lwlan);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };    
  char RID[10] = { 0 };
  char wlan_id[10] = { 0 };
  char bss_index[10] = { 0 };
  char flag[5] = { 0 }; /*fla=="1",表示上一页为wp_radiolis.cgi,否则上一页为wp_wtpdta.cgi*/  
  char wtp_id[10] = { 0 };
  char instance_id[10] = { 0 };
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
  memset(RID,0,sizeof(RID));
  memset(wlan_id,0,sizeof(wlan_id));
  memset(bss_index,0,sizeof(bss_index));
  memset(flag,0,sizeof(flag));
  memset(wtp_id,0,sizeof(wtp_id));
  memset(instance_id,0,sizeof(instance_id));
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
	  cgiFormStringNoNewlines("RID", RID, 10); 
	  cgiFormStringNoNewlines("WLANID", wlan_id, 10); 
	  cgiFormStringNoNewlines("BSSindex", bss_index, 5);
	  cgiFormStringNoNewlines("FL", flag, 5);
	  cgiFormStringNoNewlines("WID", wtp_id, 10); 
	  cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  }
  else
  {
	  cgiFormStringNoNewlines("encry_bssdta", encry, BUF_LEN);
	  cgiFormStringNoNewlines("radid", RID, 10); 
	  cgiFormStringNoNewlines("Wlan_id", wlan_id, 10); 
	  cgiFormStringNoNewlines("Bss_index", bss_index, 5);
	  cgiFormStringNoNewlines("FLAG", flag, 5);
	  cgiFormStringNoNewlines("wtp_id", wtp_id, 10);
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
    ShowBSSdtailPage(encry,str,RID,wlan_id,bss_index,flag,wtp_id,instance_id,paraHead1,lpublic,lwlan);

  release(lpublic);  
  release(lwlan);
  free_instance_parameter_list(&paraHead1);
  destroy_ccgi_dbus();
  return 0;
}

void ShowBSSdtailPage(char *m,char *n,char *rid,char *wlan_id,char *bss_index,char *flag,char *wtp_id,char *ins_id,instance_parameter *ins_para,struct list *lpublic,struct list *lwlan)
{   
  char pno[10] = { 0 };
  char *endptr = NULL;
  int i = 0,j = 0,k = 0,radio_id = 0,bssindex = 0,ret1 = 0, ret2 = 0,ret3 = 0,limit = 0,retu = 1;
  DCLI_RADIO_API_GROUP_ONE *radio = NULL;
  char whichinterface[RADIO_IF_NAME_LEN] = { 0 };
  DCLI_RADIO_API_GROUP_ONE *radio_info = NULL;
  char forward_policy[3][8] = {"UNKNOWN","BRIDGE","ROUTE"};
  memset(pno,0,sizeof(pno));
  cgiFormStringNoNewlines("PN",pno,10);
  struct dcli_bss_info *bss = NULL;
  char *maclist_name[3]={"none","black","white"};
  char if_name[20] = { 0 };
  int ret = -1;
  if_list_p interf;
  if3 *q = NULL;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>BSS Detail</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");
  radio_id= strtoul(rid,&endptr,10);   /*char转成int，10代表十进制*/		  
  if(cgiFormSubmitClicked("clean_vlan_apply") == cgiFormSuccess)
  {
  	if(ins_para)
	{
		CleanVlan(ins_para,radio_id,wlan_id,lpublic,lwlan);
	} 
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
	    	
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
          fprintf(cgiOut,"<td width=62 align=center><a href=wp_rdtail.cgi?UN=%s&FL=%s&ID=%s&WID=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,flag,rid,wtp_id,pno,ins_id,search(lpublic,"img_ok"));	  	
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_rdtail.cgi?UN=%s&FL=%s&ID=%s&WID=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,flag,rid,wtp_id,pno,ins_id,search(lpublic,"img_cancel"));	  	
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>BSS</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"details"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
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

				  if(ins_para)
				  {
					  ret1=show_radio_one(ins_para->parameter,ins_para->connection,radio_id,&radio);
					  ret2=show_radio_bss_cmd(ins_para->parameter,ins_para->connection,radio_id,&radio_info);
				  } 

				  if(strcmp(flag,"0")==0)
				  {
					  limit=12;
					  if(retu==1) /*普通用户*/
						limit+=5;
				  }
				  else
				  {
					  limit=20;
					  if(retu==1) /*普通用户*/
						limit+=1;
				  }

				  /*获取radio三层接口信息*/
				  if((ret1 == 1)&&(radio != NULL)&&(radio->RADIO[0] != NULL))
				  {
					  memset(if_name,0,sizeof(if_name));
					  if(ins_para)
					  {
						  if(ins_para->parameter.local_id == SNMPD_LOCAL_INSTANCE)
							snprintf(if_name,sizeof(if_name)-1,"r%d-%d-%d.%s",ins_para->parameter.instance_id,radio->RADIO[0]->WTPID,radio->RADIO[0]->Radio_L_ID,wlan_id);
						  else
							snprintf(if_name,sizeof(if_name)-1,"r%d-%d-%d-%d.%s",ins_para->parameter.slot_id,ins_para->parameter.instance_id,radio->RADIO[0]->WTPID,radio->RADIO[0]->Radio_L_ID,wlan_id);
					  } 
				  }
				  ret = get_all_if_info(&interf);
				  if((ret == 0)&&(interf.if_head))
				  {
				  	for(i = 0,q = interf.if_head->next;
						((i<interf.if_num)&&(NULL != q));
						i++,q = q->next)
				  	{
						if((strlen(if_name)==strlen(q->ifname))&&(strcmp(if_name,q->ifname) == 0))
						{
							limit++;
						}
				  	}
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
    "<td align=center>");
			if(ret2==1)
			{
			  fprintf(cgiOut,"<table width=763 border=0 cellspacing=0 cellpadding=0>"\
			  "<tr>"\
		         "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
		       fprintf(cgiOut,"</tr>"\
	          "<tr align=left height=10>"\
	          "<td id=thead3>BSS %s</td>",search(lpublic,"details"));
	          fprintf(cgiOut,"</tr>"\
              "<tr>");
			  if(strcmp(search(lpublic,"details"),"Details")==0)
                fprintf(cgiOut,"<td align=left style=\"padding-left:20px\"><table frame=below rules=rows width=450 border=1>");
			  else
			  	fprintf(cgiOut,"<td align=left style=\"padding-left:20px\"><table frame=below rules=rows width=350 border=1>");
			  if(ret1 == 1)
				{
				  for(i=0;i<radio->bss_num;i++)
				  {
				    bssindex= strtoul(bss_index,&endptr,10);   /*char转成int，10代表十进制*/		  
					if((ret2 == 1)&&(radio_info)&&(radio_info->RADIO[0])&&(radio_info->RADIO[0]->BSS[i])&&(radio_info->RADIO[0]->BSS[i]->BSSIndex==bssindex))
				    {
						fprintf(cgiOut,"<tr align=left>");
						  if(strcmp(search(lpublic,"details"),"Details")==0)
						    fprintf(cgiOut,"<td id=td1 width=250>WLAN ID</td>");
						  else
						    fprintf(cgiOut,"<td id=td1 width=150>WLAN ID</td>");
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
						  {
							  fprintf(cgiOut,"<td id=td2 width=200>%d</td>",radio->RADIO[0]->BSS[i]->WlanID);
						  }
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>Keyindex</td>");
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
						  {
							  fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->BSS[i]->keyindex);			
						  }
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>BSSindex</td>"\
						  "<td id=td2>%d</td>",radio_info->RADIO[0]->BSS[i]->BSSIndex);			
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>BSSID</td>");
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i])&&(radio->RADIO[0]->BSS[i]->BSSID))
						  {
							  fprintf(cgiOut,"<td id=td2>%02X:%02X:%02X:%02X:%02X:%02X</td>",radio->RADIO[0]->BSS[i]->BSSID[0],radio->RADIO[0]->BSS[i]->BSSID[1],radio->RADIO[0]->BSS[i]->BSSID[2],radio->RADIO[0]->BSS[i]->BSSID[3],radio->RADIO[0]->BSS[i]->BSSID[4],radio->RADIO[0]->BSS[i]->BSSID[5]);
						  }
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>%s</td>",search(lwlan,"state"));
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i])&&(radio->RADIO[0]->BSS[i]->State==0))
							fprintf(cgiOut,"<td id=td2>disable</td>");
						  else
							fprintf(cgiOut,"<td id=td2>enable</td>");
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>%s%s</td>",search(lpublic,"inter"),search(lpublic,"policy"));
						  memset(whichinterface,0,sizeof(whichinterface));
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
						  {
							  CheckWIDIfPolicy(whichinterface,radio->RADIO[0]->BSS[i]->BSS_IF_POLICY);
						  }
						  fprintf(cgiOut,"<td id=td2>%s</td>",whichinterface);
						fprintf(cgiOut,"</tr>");
						if(strcmp(if_name,""))
					    {
						  j=0;
						  if((ret == 0)&&(interf.if_head))
						  {
							for(k = 0,q = interf.if_head->next;
								((k<interf.if_num)&&(NULL != q));
								k++,q = q->next)
							{
								if((strlen(if_name)==strlen(q->ifname))&&(strcmp(if_name,q->ifname) == 0))
								{
									if(j == 0)
									{
										fprintf(cgiOut,"<tr align=left>"\
										  "<td id=td1>%s</td>",search(lpublic,"l3_interface_name_ip_mask"));
										fprintf(cgiOut,"<td id=td2>%s: %s/%d</td>",if_name,q->ipaddr,q->mask);
										fprintf(cgiOut,"</tr>");
									}
									else
									{
										fprintf(cgiOut,"<tr align=left>"\
										  "<td id=td1>&nbsp;</td>"\
										  "<td id=td2>%s: %s/%d</td>",if_name,q->ipaddr,q->mask);
										fprintf(cgiOut,"</tr>");
									}
									j++;
								}
							}
						  }
					    }
						fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>%s</td>",search(lwlan,"max_sta_num"));
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
						  {
							  fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->BSS[i]->bss_max_allowed_sta_num);
						  }
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>%s</td>",search(lwlan,"wds_mode"));
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i])&&(radio->RADIO[0]->BSS[i]->WDSStat == 0))
							fprintf(cgiOut,"<td id=td2>disable</td>");
						  else if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i])&&(radio->RADIO[0]->BSS[i]->WDSStat == 1))
							fprintf(cgiOut,"<td id=td2>ANY</td>");
						  else if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i])&&(radio->RADIO[0]->BSS[i]->WDSStat == 2))
							fprintf(cgiOut,"<td id=td2>SOME</td>");
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>upcnt</td>");
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
						  {
							  fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->BSS[i]->upcount);
						  }
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>dwncnt</td>");
						  if((radio)&&(radio->RADIO[0])&&(radio->RADIO[0]->BSS[i]))
						  {
							  fprintf(cgiOut,"<td id=td2>%d</td>",radio->RADIO[0]->BSS[i]->downcount);
						  }
						fprintf(cgiOut,"</tr>");
						
						if(ins_para)
						{
							ret3=show_radio_bss_mac_list(ins_para->parameter,ins_para->connection,radio_id,wlan_id,&bss);					
						} 
						fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>Radio %s</td>",search(lwlan,"mac_filter_type"));
						  if((ret3==1)&&(bss))
							fprintf(cgiOut,"<td id=td2>%s</td>",maclist_name[bss->acl_conf.macaddr_acl]);
						  else
							fprintf(cgiOut,"<td id=td2>%s</td>","-");
						fprintf(cgiOut,"</tr>");

						fprintf(cgiOut,"<tr align=left>"\
						  "<td id=td1>BSS VLAN ID</td>");
					
						  if(retu==0)  /*管理员*/
						    fprintf(cgiOut,"<td id=td2>%d"\
						    			     "<input type=submit style=\"width:140px; margin-left:20px\" border=0 name=clean_vlan_apply style=background-image:url(/images/SubBackGif.gif) value=\"%s BSS VLAN ID\">"\
						    			     "</td>",radio_info->RADIO[0]->BSS[i]->vlanid,search(lpublic,"delete"));
						  else
						  	fprintf(cgiOut,"<td id=td2>%d</td>",radio_info->RADIO[0]->BSS[i]->vlanid);
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>WLAN VLAN ID</td>"\
						  "<td id=td2>%d</td>",radio_info->RADIO[0]->BSS[i]->wlan_vlanid);	
						
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>%s</td>",search(lwlan,"forward_policy"));
						  fprintf(cgiOut,"<td id=td2>%s</td>",forward_policy[(radio_info->RADIO[0]->BSS[i]->BSS_IF_POLICY)]);			
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>%s</td>",search(lwlan,"tunnel_policy"));
						  if(radio_info->RADIO[0]->BSS[i]->BSS_TUNNEL_POLICY==8)
						    fprintf(cgiOut,"<td id=td2>IPIP</td>");
						  else if(radio_info->RADIO[0]->BSS[i]->BSS_TUNNEL_POLICY==2)
						  	fprintf(cgiOut,"<td id=td2>CAPWAP802DOT3</td>");
						  else
						  	fprintf(cgiOut,"<td id=td2>CAPWAP802DOT11</td>");
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>%s</td>",search(lpublic,"l2_isolation"));
						  if(radio_info->RADIO[0]->BSS[i]->ath_l2_isolation==1)
						    fprintf(cgiOut,"<td id=td2>enable</td>");
						  else
						  	fprintf(cgiOut,"<td id=td2>disable</td>");
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>CWM %s</td>",search(lpublic,"mode"));
						  fprintf(cgiOut,"<td id=td2>%d</td>",radio_info->RADIO[0]->BSS[i]->cwmmode);			
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>BSS%s</td>",search(lpublic,"traffic_limit_switch"));
						  if(radio_info->RADIO[0]->BSS[i]->traffic_limit_able==1)
						    fprintf(cgiOut,"<td id=td2>enable</td>");
						  else
						  	fprintf(cgiOut,"<td id=td2>disable</td>");		
						  
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>BSS%s</td>",search(lpublic,"uplink_traffic_limit_threshold"));
						  fprintf(cgiOut,"<td id=td2>%d kbps</td>",radio_info->RADIO[0]->BSS[i]->traffic_limit);			
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>STA%s</td>",search(lpublic,"average_uplink_traffic_limit_threshold"));
						  fprintf(cgiOut,"<td id=td2>%d kbps</td>",radio_info->RADIO[0]->BSS[i]->average_rate);			
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>BSS%s</td>",search(lpublic,"downlink_traffic_limit_threshold"));
						  fprintf(cgiOut,"<td id=td2>%d kbps</td>",radio_info->RADIO[0]->BSS[i]->send_traffic_limit);			
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>STA%s</td>",search(lpublic,"average_downlink_traffic_limit_threshold"));
						  fprintf(cgiOut,"<td id=td2>%d kbps</td>",radio_info->RADIO[0]->BSS[i]->send_average_rate);	
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>IP MAC %s</td>",search(lpublic,"bind"));
						  if(radio_info->RADIO[0]->BSS[i]->ip_mac_binding==1)
						    fprintf(cgiOut,"<td id=td2>enable</td>");
						  else
						    fprintf(cgiOut,"<td id=td2>disable</td>");	
						fprintf(cgiOut,"</tr>"\
						"<tr align=left>"\
						  "<td id=td1>%snas_id</td>",search(lpublic,"inter"));
						  fprintf(cgiOut,"<td id=td2>%s</td>",radio_info->RADIO[0]->BSS[i]->nas_id);			
						fprintf(cgiOut,"</tr>");
							
						break;  /*跳出for(i=0;i<bssnum;i++)循环*/
				    }
					//bq=bq->next;
				 }
			 }
             fprintf(cgiOut,"</table>"\
                "</td>"\
              "</tr>"\
              "<tr>"\
			    "<td><input type=hidden name=encry_bssdta value=%s></td>",m);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=radid value=%s></td>",rid);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=Wlan_id value=%s></td>",wlan_id);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=Bss_index value=%s></td>",bss_index);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=FLAG value=%s></td>",flag);
			  fprintf(cgiOut,"</tr>"\
			  "<tr>"\
			    "<td><input type=hidden name=wtp_id value=%s></td>",wtp_id);
			  fprintf(cgiOut,"</tr>"\
			  	"<tr>"\
			    "<td><input type=hidden name=instance_id value=%s></td>",ins_id);
			  fprintf(cgiOut,"</tr>"\
              "</table>");
			}
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
if(ret == 0)
{
	Free_get_all_if_info(&interf);
}
if(ret1==1)
  	Free_radio_one_head(radio);
if(ret2==1)
{
  Free_radio_bss_head(radio_info);
}
if(ret3==1)
{
  Free_mac_head(bss);
}
}

void CleanVlan(instance_parameter *ins_para,int rid,char *wid,struct list *lpublic,struct list *lwlan)
{
	int ret = 0;
	char alt[100] = { 0 };
  	char max_wlan_num[10] = { 0 };
	char max_radio_num[10] = { 0 };

	ret=radio_apply_wlan_clean_vlan_cmd_func(ins_para->parameter,ins_para->connection,rid,wid);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format*/
																							  /*返回-2表示wlan id should be 1 to WLAN_NUM-1，返回-3表示wtp id does not exist*/
																							  /*返回-4表示radio id does not exist，返回-5表示binding wlan does not exist*/
																							  /*返回-6表示radio is not binding this wlan，返回-7表示bss is enable, you should disable it first*/
																							  /*返回-8表示error，返回-9表示Radio ID非法*/
																							  /*返回-10表示illegal input:Input exceeds the maximum value of the parameter type*/
	switch(ret)
	{
		case SNMPD_CONNECTION_ERROR:
		case 0:ShowAlert(search(lwlan,"clean_vlan_fail"));
			   break;
		case 1:ShowAlert(search(lwlan,"clean_vlan_succ"));
			   break;  
		case -1:ShowAlert(search(lpublic,"unknown_id_format"));
			    break;  
		case -2:{
			      memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
				  memset(max_wlan_num,0,sizeof(max_wlan_num));
				  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				  strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		  	      ShowAlert(alt);
			      break;  
			    }
		case -3:ShowAlert(search(lwlan,"wtp_not_exist"));
			    break;
	    case -4:ShowAlert(search(lwlan,"radio_not_exist"));
			    break;
		case -5:ShowAlert(search(lwlan,"bind_wlan_not_exist"));
			    break;
		case -6:ShowAlert(search(lwlan,"radio_not_bind_wlan"));
			    break;
		case -7:ShowAlert(search(lwlan,"unuse_bss"));
			    break;
		case -8:ShowAlert(search(lpublic,"error"));
    	        break;
		case -9:{
			      memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"radio_id_illegal1"),sizeof(alt)-1);
				  memset(max_radio_num,0,sizeof(max_radio_num));
				  snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
				  strncat(alt,max_radio_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"radio_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		  	      ShowAlert(alt);
			      break;  
			    }
		case -10:ShowAlert(search(lpublic,"input_exceed_max_value"));
    	         break;
	}
}


