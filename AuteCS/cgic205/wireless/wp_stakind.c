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
* wp_stakind.c
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
#include "ws_sta.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"

void ShowStadionKindPage(char *m,char *n,int t,struct list *lpublic,struct list *lwlan);  

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };       
  char page_no[10] = { 0 }; 
  char *str = NULL; 
  char *endptr = NULL; 
  int pno = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(page_no,0,sizeof(page_no));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
  {
    if(cgiFormStringNoNewlines("SPN", page_no, 10)!=cgiFormNotFound )  /*点击翻页进入该页面*/
    {
      pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowStadionKindPage(encry,str,pno,lpublic,lwlan);
	}
	else
      ShowStadionKindPage(encry,str,0,lpublic,lwlan);
  }  
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowStadionKindPage(char *m,char *n,int t,struct list *lpublic,struct list *lwlan)
{  
  char pno[10] = { 0 }; 
  char ID[10] = { 0 };
  char *endptr = NULL;  
  char type[10] = { 0 };  
  char mac[30] = { 0 };
  int bss_num = 0,i = 0,wid = 0,cl = 1,result1 = 0,retu = 1,result2 = 0;                 /*颜色初值为#f9fafe*/  
  int limit = 0;
  char alt[100] = { 0 };
  char max_wtp_num[10] = { 0 };
  char max_wlan_num[10] = { 0 };
  char select_insid[10] = { 0 };
  struct dcli_wlan_info *wlan = NULL;
  struct dcli_wtp_info *wtp = NULL;
  struct dcli_bss_info *bss = NULL;  
  struct dcli_sta_info *sta = NULL;  
  int wfirstnum = 0; 
  unsigned char ieee80211_state[20] = { 0 };
  unsigned char PAE[20] = { 0 };
  unsigned char BACKEND[20] = { 0 };
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
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
	  "</script>"\
  "<script src=/instanceid_onchange.js>"\
  "</script>"\
  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>");
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

	memset(type,0,sizeof(type));
    cgiFormStringNoNewlines("ST", type, 10);
	if(strcmp(type,"wlan")==0)  
      fprintf(cgiOut,"<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>WLAN</td>");
	else
	  fprintf(cgiOut,"<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");	
		memset(pno,0,sizeof(pno));
		cgiFormStringNoNewlines("PN",pno,10);
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>");
		  if(strcmp(type,"wlan")==0)	
		  {
            fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlanlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,select_insid,search(lpublic,"img_ok"));
		    fprintf(cgiOut,"<td width=62 align=center><a href=wp_wlanlis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,select_insid,search(lpublic,"img_cancel"));
		  }
		  else
		  {
            fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtplis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,select_insid,search(lpublic,"img_ok"));
		    fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtplis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,select_insid,search(lpublic,"img_cancel"));
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
				  retu=checkuser_group(n);
				  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"station"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
  				  if(strcmp(type,"wlan")==0)
  				  {
  				    if(retu==0)  /*管理员*/
  				    {
					  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_wlannew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> WLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));					   
					  fprintf(cgiOut,"</tr>");
					  fprintf(cgiOut,"<tr height=25>"\
						"<td align=left id=tdleft><a href=wp_wlanbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));					   
					  fprintf(cgiOut,"</tr>");
  				    }
  				  }
  				  else
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
				  cgiFormStringNoNewlines("ID", ID, 10);
				  wid= strtoul(ID,&endptr,10);   /*char转成int，10代表十进制*/	
				  fprintf(cgiOut,"<script type=\"text/javascript\">"\
				  "function page_change(obj)"\
				  "{"\
				     "var page_num = obj.options[obj.selectedIndex].value;"\
				   	 "var url = 'wp_stakind.cgi?UN=%s&SPN='+page_num+'&ID=%s&ST=%s&PN=%s&INSTANCE_ID=%s';"\
				   	 "window.location.href = url;"\
				   	"}", m , ID, type ,pno,select_insid);
				  fprintf(cgiOut,"</script>");
				  if(paraHead1)
				  {
					  if(strcmp(type,"wlan")==0)	
						result1=show_sta_bywlanid(paraHead1->parameter,paraHead1->connection,wid,&wlan);/*返回0表示失败，返回1表示成功，返回-1表示wlan ID非法，返回-2表示WLAN NOT Provide services，返回-3表示WLAN NOT EXIST*/
					  else
						result2=show_sta_bywtpid(paraHead1->parameter,paraHead1->connection,wid,&wtp);
				  }

	              if(result1==1)
	              {
				  	bss_num=wlan->num_bss;
	              }
	              if(result2==1)
	              {
				  	bss_num=wtp->num_bss;
	              }

				  if(bss_num>0)
				  {
					 if(result1==1)
                        bss = wlan->bss_list;
	                 if(result2==1)							
						bss = wtp->bss_list;
					 
					 for(i=0;i<t;i++)
					 {
					 	if(bss)
					 	{
							bss=bss->next;	 /*移动到第i个bss信息节点处*/
					 	}
					 }

					if(bss)
				 	{
						if(bss_num>1)				  /*需要翻页*/
						{
						  if(retu==0)/*管理员*/
							limit=2+bss->num_sta;
						  else
							limit=7+bss->num_sta;
						}
						else
						{
						  if(retu==0)/*管理员*/
							limit=1+bss->num_sta;
						  else
							limit=6+bss->num_sta;
						}
				 	}
				  }
				  else  /*没有bss*/
					limit=1;
				  
				  if((strcmp(type,"wlan")==0)&&(bss_num>0))
				  {				  	  
					  if(retu==0)/*管理员*/
						limit+=7;
					  else
						limit+=4;				  	  
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
			 "<table width=720 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>");
			  fprintf(cgiOut,"<tr>"\
                     "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),select_insid);
                  fprintf(cgiOut,"</tr>");			
			  fprintf(cgiOut,"<tr valign=middle>"\
			    "<td>");		

			if(((result1==1)||(result2==1))&&(bss_num > 0))
			{	
              if(result1==1)
			  	wfirstnum=wlan->num_bss;
              if(result2==1)
			  	wfirstnum=wtp->num_bss;
			  
			  for(i=0; i<wfirstnum; i++)
		  	  {
				if(result1==1)
				{
					if(bss == NULL)
						break;
				}
                if(result2==1)
                {
					if(bss == NULL)
						break;
                }
			  fprintf(cgiOut,"<table width=720 border=0 cellspacing=0 cellpadding=0>"\
	          "<tr align=left height=10 valign=top>"\
	            "<td colspan=3 id=thead1>%s</td>",search(lwlan,"sta_list"));
	          fprintf(cgiOut,"</tr>"\
              "<tr>"\
                "<td colspan=3 align=left style=\"padding-left:20px\">"\
			    "<table frame=below rules=rows width=320 border=1>"\
			    "<tr align=left>"\
			     "<td id=td1 width=170>WLAN ID</td>"\
			     "<td id=td2 width=150>%d</td>",bss->WlanID);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>AP ID</td>"\
				  "<td id=td2>%d</td>",bss->WtpID);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>BSS %s</td>",search(lwlan,"sta_num"));			  
				  fprintf(cgiOut,"<td id=td2>%d</td>",bss->num_sta);
				fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>Radio_G_ID</td>"\
				  "<td id=td2>%d</td>",bss->Radio_G_ID);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>Radio_L_ID</td>"\
				  "<td id=td2>%d</td>",bss->Radio_L_ID);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>BSS%s</td>",search(lwlan,"index"));
			      fprintf(cgiOut,"<td id=td2>%d</td>",bss->BSSIndex);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%sID</td>",search(lwlan,"security"));			  
				  fprintf(cgiOut,"<td id=td2>%d</td>",bss->SecurityID);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"bss_assoc_num"));			  
				  fprintf(cgiOut,"<td id=td2>%d</td>",bss->num_assoc);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"bss_reassoc_num"));			  
				  fprintf(cgiOut,"<td id=td2>%d</td>",bss->num_reassoc);
			    fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"bss_assoc_failure_num"));			  
				  fprintf(cgiOut,"<td id=td2>%d</td>",bss->num_assoc_failure);
			    fprintf(cgiOut,"</tr>"\
			  "</table></td>"\
				"</tr>"\
				"<tr>"\
				"<td colspan=3 align=left style=\"padding-left:20px; padding-top:20px\"><table frame=below rules=rows width=720 border=1>"\
				"<tr height=20>"\
				  "<th align=left width=140 id=td1>MAC</th>"\
				  "<th align=left width=140 id=td1>IEEE80211 %s</th>",search(lwlan,"state"));
				  fprintf(cgiOut,"<th align=left width=90 id=td1>PAE_%s</th>",search(lwlan,"state"));
				  fprintf(cgiOut,"<th align=left width=100 id=td1>Backend_%s</th>",search(lwlan,"state"));
				  fprintf(cgiOut,"<th align=left width=200 id=td1>%s</th>",search(lwlan,"acc_time"));
				  fprintf(cgiOut,"<th align=left width=50 id=td1>&nbsp;</th>");
				fprintf(cgiOut,"</tr>");

 			    for(i=0,sta = bss->sta_list;((i<bss->num_sta)&&(NULL != sta));i++,sta = sta->next)
				{
					memset(ieee80211_state, 0, sizeof(ieee80211_state));
					memset(PAE, 0, sizeof(PAE));
					memset(BACKEND, 0, sizeof(BACKEND));

					asd_state_check(ieee80211_state,sta->sta_flags,PAE,sta->pae_state,BACKEND,sta->backend_state);	
					memset(mac,0,sizeof(mac));
					fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
					fprintf(cgiOut,"<td id=td2>%02X:%02X:%02X:%02X:%02X:%02X</td>",MAC2STRZ(sta->addr));
					snprintf(mac,sizeof(mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STRZ(sta->addr));
					fprintf(cgiOut,"<td id=td2>%s</td>",ieee80211_state);
					fprintf(cgiOut,"<td id=td2>%s</td>",PAE);		 
					fprintf(cgiOut,"<td id=td2>%s</td>",BACKEND);
					/*time_t 	now,online_time,now_sysrun,statime;
					time(&now);
					get_sysruntime(&now_sysrun);
					online_time = now_sysrun - sta->StaTime+sta->sta_online_time;
					statime = now - online_time;*/
					fprintf(cgiOut,"<td id=td2>%s</td>",ctime(&sta->sta_access_time));
					fprintf(cgiOut,"<td id=td2 align=left><a href=wp_stakick.cgi?UN=%s&Nm=%s&INSTANCE_ID=%s&Type=%s&PN=%s&ID=%s target=mainFrame><font color=black>%s</font></td>",m,mac,select_insid,type,pno,ID,search(lwlan,"kick"));
					fprintf(cgiOut,"</tr>");
					cl=!cl;
				} 
			  fprintf(cgiOut,"</table>"\
				  "</td>"\
				"</tr>");
                if(bss_num>1)               /*bss多余一个时，显示翻页的链接*/
			    {
			      fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
				  if(t!=0)          
			        fprintf(cgiOut,"<td align=left width=100><a href=wp_stakind.cgi?UN=%s&SPN=%d&ID=%s&ST=%s&PN=%s&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,t-1,ID,type,pno,select_insid,search(lpublic,"up_page"));
				  else
					fprintf(cgiOut,"<td width=100>&nbsp;</td>");
				  fprintf(cgiOut,"<td align=center width=520>%s",search(lpublic,"jump_to_page1"));
												   fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this)>");
												   for(i=0;i<bss_num;i++)
												   {
													 if(i==t)
													   fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
													 else
													   fprintf(cgiOut,"<option value=%d>%d",i,i+1);
												   }
												   fprintf(cgiOut,"</select>"\
												   "%s</td>",search(lpublic,"jump_to_page2"));
				  if(t!=(bss_num-1))
			        fprintf(cgiOut,"<td align=right width=100><a href=wp_stakind.cgi?UN=%s&SPN=%d&ID=%s&ST=%s&PN=%s&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,t+1,ID,type,pno,select_insid,search(lpublic,"down_page"));
				  else
					fprintf(cgiOut,"<td width=100>&nbsp;</td>");
			      fprintf(cgiOut,"</tr>");
			    }				
			  fprintf(cgiOut,"</table>");
				}
			}
			else if(bss_num==0)
			  fprintf(cgiOut,"%s",search(lwlan,"no_bss"));
			else if((result1==0)||(result1 == SNMPD_CONNECTION_ERROR)||(result2==0)||(result2 == SNMPD_CONNECTION_ERROR))
			  fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));	
			else if((result1==-1)||(result2==-1))
			{
			  if(strcmp(type,"wlan")==0)	
			  {
			  	  memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
				  memset(max_wlan_num,0,sizeof(max_wlan_num));
				  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
				  strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
		  	      fprintf(cgiOut,"%s",alt);	
			  }
			  else
			  {
			  	  memset(alt,0,sizeof(alt));
				  strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				  memset(max_wtp_num,0,sizeof(max_wtp_num));
				  snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				  strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				  strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				  fprintf(cgiOut,"%s",alt);		
			  }
		    }
			else if((result1==-2)||(result2==-2))
			{
			  if(strcmp(type,"wlan")==0)	
				fprintf(cgiOut,"%s",search(lwlan,"wlan_not_exist"));	  
			  else
				fprintf(cgiOut,"%s",search(lwlan,"wtp_not_exist"));	  
		    }	
			else
			{
				fprintf(cgiOut,"%s",search(lpublic,"error"));	  
			}
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
if(result1 == 1)
{
  Free_sta_bywlanid(wlan);
}
if(result2 == 1)
{
  Free_sta_bywtpid(wtp);
}
free_instance_parameter_list(&paraHead1);
}



