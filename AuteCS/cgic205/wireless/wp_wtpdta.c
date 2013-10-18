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
* wp_wtpdta.c
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

#include <config/auteware_config.h>
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sta.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dbus_list_interface.h"

void ShowWtpdtaPage(char *m,char *n,char *t,char *ins_id,struct list *lpublic,struct list *lwlan);  


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };
  char ID[10] = { 0 };
  char instance_id[10] = { 0 };
  char *str = NULL;          
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(ID,0,sizeof(ID));
  memset(instance_id,0,sizeof(instance_id));
  cgiFormStringNoNewlines("INSTANCE_ID", instance_id, 10);
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {    
	cgiFormStringNoNewlines("ID", ID, 10); 
  }
  else
  {
    cgiFormStringNoNewlines("encry_wtpdta",encry,BUF_LEN);
	cgiFormStringNoNewlines("wtpid",ID,10);
  }  
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
    ShowWtpdtaPage(encry,str,ID,instance_id,lpublic,lwlan);
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWtpdtaPage(char *m,char *n,char *t,char *ins_id,struct list *lpublic,struct list *lwlan)
{   
  char pno[10] = { 0 };
  DCLI_WTP_API_GROUP_ONE *wtp = NULL;
  char wtp_sta[WTP_ARRAY_NAME_LEN] = { 0 };  
  char quitreason[WTP_ARRAY_NAME_LEN] = { 0 };
  char radio_id[10] = { 0 };
  char RType[10] = { 0 };
  char *endptr = NULL;  
  char menu_id[10] = { 0 };
  char menu[15]="menuLists";
  char bwlanid[40] = { 0 };
  char tembwid[5] = { 0 };
  int rnum = 0,i = 0,j = 0,wtp_id = 0,retu = 1,cl = 1,bwlannum = 0,result = 0,limit = 0;                 /*颜色初值为#f9fafe*/
  char alt[100] = { 0 };
  char max_wtp_num[10] = { 0 };
  int result1 = 0,result2 = 0,black_num = 0,white_num = 0;
  struct dcli_wtp_info *wtpz = NULL;
  DCLI_WTP_API_GROUP_THREE *WTPINFO = NULL;   
  char *maclist_name[3]={"none","black","white"};
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  int ret1 = 0,ret2 = 0;
  DCLI_WTP_API_GROUP_TWO *wtpinfo1 = NULL,*wtpinfo2 = NULL;

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
  	"th{ font-family:Arial, Helvetica, sans-serif; font-weight:bold; font-size:12px; color:#0a4e70}"\
  	"#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
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
		   "</script>"\
  "<body>"\
  "<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
		memset(pno,0,sizeof(pno));
		cgiFormStringNoNewlines("PN",pno,10);
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
		  "<td width=62 align=center><a href=wp_wtplis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,ins_id,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_wtplis.cgi?UN=%s&PN=%s&INSTANCE_ID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,pno,ins_id,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>AP</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"details"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>"\
				  "<tr height=25>"\
					"<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));					   
				  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create")); 					  
					fprintf(cgiOut,"</tr>");
				  }		fprintf(cgiOut,"<tr height=25>"\
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
				  wtp_id= strtoul(t,&endptr,10);	/*char转成int，10代表十进制*/	  
				  
				  get_slotID_localID_instanceID(ins_id,&ins_para);	
				  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

				  if(paraHead1)
				  {
					  result=show_wtp_one(paraHead1->parameter,paraHead1->connection,wtp_id,&wtp);
				  } 
				  if((result == 1)&&(wtp)&&(wtp->WTP[0]))
				  {
				  	bwlannum = wtp->WTP[0]->apply_wlan_num;
				  	rnum = wtp->WTP[0]->radio_num;
				  }

				  if(paraHead1)
				  {
					  result1=show_wtp_mac_list(paraHead1->parameter,paraHead1->connection,wtp_id,&wtpz);
					  result2=show_wtp_runtime(paraHead1->parameter,paraHead1->connection,wtp_id,&WTPINFO);
					  ret1 = show_wtp_extension_information_v3(paraHead1->parameter,paraHead1->connection,wtp_id,&wtpinfo1);
					  ret2 = show_ap_if_info_func(paraHead1->parameter,paraHead1->connection,wtp_id,&wtpinfo2);
				  }
				  if(result1 ==1)
				  {
				    black_num = wtpz->acl_conf.num_deny_mac ;
					white_num = wtpz->acl_conf.num_accept_mac;
				  }
				  
				  limit=12+rnum;
				  if((result == 1)&&(wtp)&&(wtp->WTP[0])&&(wtp->WTP[0]->WTPStat == 7))
				  	limit+=1;
				  if(result1==1)
				  	limit+=1;
				  if(result2==1)
				  	limit+=2;
				  if((1 == ret2)&&(wtpinfo2)&&(wtpinfo2->WTP[0]))
				  {
				  	limit+=wtpinfo2->WTP[0]->apifinfo.eth_num;
				  }
				  if(retu==1)  /*普通用户*/
				  	limit+=4;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">"\
 "<table width=500 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
  "<tr valign=middle>"\
    "<td align=center>");
			if((result==1)&&(wtp)&&(wtp->WTP[0]))
			{	
			   fprintf(cgiOut,"<table width=500 border=0 cellspacing=0 cellpadding=0>"\
			   "<tr>"\
		         "<td id=ins_style>%s:%s</td>",search(lpublic,"instance"),ins_id);
		       fprintf(cgiOut,"</tr>"\
	           "<tr align=left height=10 valign=top>"\
	           "<td id=thead1>AP %s</td>",search(lpublic,"details"));
	           fprintf(cgiOut,"</tr>"\
               "<tr>"\
               "<td align=left style=\"padding-left:20px\">"\
			   "<table frame=below rules=rows width=500 border=1>"\
			   "<tr align=left>"\
			   "<td id=td1 width=160>%s</td>",search(lpublic,"name"));
			   if(wtp->WTP[0]->WTPNAME)
			   {
				   fprintf(cgiOut,"<td id=td2 width=340 style=\"word-break:break-all\">%s</td>",wtp->WTP[0]->WTPNAME);
			   }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>ID</td>"\
				  "<td id=td2>%d</td>",wtp->WTP[0]->WTPID);
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"sn"));
			  	  if(wtp->WTP[0]->WTPSN)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wtp->WTP[0]->WTPSN);
			  	  }
			  fprintf(cgiOut,"</tr>");
			  #ifdef __WITH_AP_CPU_FREQ
			  	FILE *fp = NULL;
			    char cpu_freq[256] = { 0 };
				memset(cpu_freq,0,256);
				
			  	fp = fopen("/mnt/WtpCpuFreq","r");
				if(fp)
				{					
					fgets(cpu_freq,256,fp);
					fclose(fp);
				}	
				else
				{
					strncpy(cpu_freq, "500MHz", sizeof(cpu_freq)-1);
				}
		
			  	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"cpu_freq"));
				  fprintf(cgiOut,"<td id=td2>%s</td>",cpu_freq);
			    fprintf(cgiOut,"</tr>");
			  #endif
				fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"model"));
			  	  if(wtp->WTP[0]->WTPModel)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wtp->WTP[0]->WTPModel);
			  	  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>");
				  fprintf(cgiOut,"<td id=td1>%s %s</td>",search(lwlan,"run"),search(lwlan,"state"));
			  memset(wtp_sta,0,sizeof(wtp_sta));
			  CheckWTPState(wtp_sta,wtp->WTP[0]->WTPStat);
			  fprintf(cgiOut,"<td id=td2>%s</td>",wtp_sta);			  
			  fprintf(cgiOut,"</tr>");
			  if(wtp->WTP[0]->WTPStat == 7)
			  {
				memset(quitreason,0,sizeof(quitreason));
				CheckWTPQuitReason(quitreason,wtp->WTP[0]->quitreason);
			  	fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"quit_reason"));
			    fprintf(cgiOut,"<td id=td2>%s</td>",quitreason);		
			  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				"<td id=td1>%s</td>",search(lwlan,"state"));			  
			  if(wtp->WTP[0]->isused==1)
			    fprintf(cgiOut,"<td id=td2>used</td>");
			  else
			  	fprintf(cgiOut,"<td id=td2>unused</td>");
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>IP</td>");
			  	  if(wtp->WTP[0]->WTPIP)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wtp->WTP[0]->WTPIP);
			  	  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>MAC</td>");
			  	  if(wtp->WTP[0]->WTPMAC)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%02X:%02X:%02X:%02X:%02X:%02X</td>",wtp->WTP[0]->WTPMAC[0],wtp->WTP[0]->WTPMAC[1],wtp->WTP[0]->WTPMAC[2],wtp->WTP[0]->WTPMAC[3],wtp->WTP[0]->WTPMAC[4],wtp->WTP[0]->WTPMAC[5]);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"wtp_sta_max"));
				  fprintf(cgiOut,"<td id=td2>%d</td>",wtp->WTP[0]->wtp_allowed_max_sta_num);
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"wtp_triger_num_by_sta"));
				  fprintf(cgiOut,"<td id=td2>%d</td>",wtp->WTP[0]->wtp_triger_num);
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"wtp_triger_num_by_flow"));
				  fprintf(cgiOut,"<td id=td2>%d</td>",wtp->WTP[0]->wtp_flow_triger);
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>%s ID</td>",search(lwlan,"control_tunnel"));
				  fprintf(cgiOut,"<td id=td2>%d</td>",wtp->WTP[0]->CTR_ID);
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s ID</td>",search(lwlan,"data_tunnel"));
				  fprintf(cgiOut,"<td id=td2>%d</td>",wtp->WTP[0]->DAT_ID);
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>WTP%s</td>",search(lwlan,"hard_version"));
			  	  if(wtp->WTP[0]->sysver)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wtp->WTP[0]->sysver);
			  	  }
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>WTP%s</td>",search(lwlan,"soft_version"));
			  	  if(wtp->WTP[0]->ver)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wtp->WTP[0]->ver);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>WTP%s</td>",search(lwlan,"update_filename"));
			  	  if(wtp->WTP[0]->updatepath)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wtp->WTP[0]->updatepath);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			  	"<tr align=left>"\
				  "<td id=td1>WTP%s</td>",search(lwlan,"update_version"));
			  	  if(wtp->WTP[0]->updateversion)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wtp->WTP[0]->updateversion);
			  	  }
			  fprintf(cgiOut,"</tr>"\
			    "<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"apply_interface"));
			  	  if(wtp->WTP[0]->apply_interface_name)
			  	  {
					  fprintf(cgiOut,"<td id=td2>%s</td>",wtp->WTP[0]->apply_interface_name);
			  	  }
			  fprintf(cgiOut,"</tr>");			  	
			  if(bwlannum==0)
			  { 
			    fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"apply_wlan"));
				  fprintf(cgiOut,"<td id=td2>NONE</td>"\
				"</tr>");
			  }
			  else
			  {
			      fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"apply_wlan"));
    			  memset(bwlanid,0,sizeof(bwlanid));
				  for (j = 0; j < wtp->WTP[0]->apply_wlan_num; j++)
				  {	
				  	memset(tembwid,0,sizeof(tembwid));
				    if(j==bwlannum-1)
				  	  snprintf(tembwid,sizeof(tembwid)-1,"%d",wtp->WTP[0]->apply_wlanid[j]); 	/*int转成char*/
				    else
                      snprintf(tembwid,sizeof(tembwid)-1,"%d,",wtp->WTP[0]->apply_wlanid[j]); 	/*int转成char*/
    			    strncat(bwlanid,tembwid,sizeof(bwlanid)-strlen(bwlanid)-1);
				  }				  
			  	  fprintf(cgiOut,"<td id=td2>%s</td>",bwlanid);
				  fprintf(cgiOut,"</tr>");
			  }
				fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>Radio %s</td>",search(lpublic,"count"));
				  fprintf(cgiOut,"<td id=td2>%d</td>",wtp->WTP[0]->RadioCount);
			  fprintf(cgiOut,"</tr>"\
				"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"first_g_radio_id"));
				  fprintf(cgiOut,"<td id=td2>%d</td>",wtp->WTP[0]->WFR_Index);
			  fprintf(cgiOut,"</tr>");
			  if(result1==1)
			  {
			    fprintf(cgiOut,"<tr align=left>"\
				  "<td id=td1>%s</td>",search(lwlan,"mac_filter_type"));			      
			      fprintf(cgiOut,"<td id=td2>%s</td>",maclist_name[wtpz->acl_conf.macaddr_acl]);
			    fprintf(cgiOut,"</tr>");
			  }
			  if(result2==1)
			  {
			    if(WTPINFO->addtime == 0)
			    {
				  fprintf(cgiOut,"<tr align=left>"\
				    "<td id=td1>%s</td>",search(lwlan,"acc_time"));			      
			        fprintf(cgiOut,"<td id=td2>Not Accessed</td>");
			      fprintf(cgiOut,"</tr>");
			      fprintf(cgiOut,"<tr align=left>"\
				    "<td id=td1>%s</td>",search(lwlan,"run_time"));			      
			        fprintf(cgiOut,"<td id=td2>NONE</td>");
			      fprintf(cgiOut,"</tr>");
			    }
				else
				{
		  			time_t now,online_time;
					time(&now);
					online_time = now - (WTPINFO->addtime);
					int hour,min,sec;
					hour=online_time/3600;
					min=(online_time-hour*3600)/60;
					sec=(online_time-hour*3600)%60;

				  fprintf(cgiOut,"<tr align=left>"\
				    "<td id=td1>%s</td>",search(lwlan,"acc_time"));			      
			        fprintf(cgiOut,"<td id=td2>%s</td>",ctime(&WTPINFO->addtime));
			      fprintf(cgiOut,"</tr>");
			      fprintf(cgiOut,"<tr align=left>"\
				    "<td id=td1>%s</td>",search(lwlan,"run_time"));			      
			        fprintf(cgiOut,"<td id=td2>%02d:%02d:%02d</td>",hour,min,sec);
			      fprintf(cgiOut,"</tr>");
			    }
			  }
			  fprintf(cgiOut,"<tr align=left>"\
				"<td id=td1>%s</td>",search(lwlan,"ap_extension_info_switch"));			  
				fprintf(cgiOut,"<td id=td2>%s</td>",((1 == ret1)?"enable":"disable"));
			  fprintf(cgiOut,"</tr>");
			  if((1 == ret2)&&(wtpinfo2)&&(wtpinfo2->WTP[0]))
			  {
			    for(i=0;i<wtpinfo2->WTP[0]->apifinfo.eth_num;i++)
			    {
					fprintf(cgiOut,"<tr align=left>"\
					  "<td id=td1>eth%d%s</td>",i,search(lpublic,"rate"));				
					  fprintf(cgiOut,"<td id=td2>%d M</td>",wtpinfo2->WTP[0]->apifinfo.eth[i].eth_rate);
					fprintf(cgiOut,"</tr>");
			    }
			  }
			  fprintf(cgiOut,"</table></td>"\
				"</tr>"\
				"<tr align=left style=\"padding-top:10px\">");			  
				  fprintf(cgiOut,"<td id=thead2>AP%d %s</td>",wtp->WTP[0]->WTPID,search(lpublic,"summary"));
				fprintf(cgiOut,"</tr>"\
				"<tr>"\
				"<td align=center style=\"padding-left:20px\"><table align=left frame=below rules=rows width=320 border=1>"\
				"<tr align=left height=20>"\
				  "<th>Radio ID</th>"\
				  "<th>%s</th>",search(lwlan,"channel"));
				  fprintf(cgiOut,"<th>%s</th>",search(lwlan,"tx_power"));
				  fprintf(cgiOut,"<th>Radio %s</th>",search(lwlan,"type"));
				  fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
				"</tr>");
				for(i=0;i<rnum;i++)
				{				  
				  snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
				  strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
				  if(wtp->WTP[0]->WTP_Radio[i])
			  	  {
					  snprintf(radio_id,sizeof(radio_id)-1,"%d",wtp->WTP[0]->WTP_Radio[i]->Radio_G_ID); 	/*int转成char*/
			  	  }
				  memset(RType,0,sizeof(RType));
				  if(wtp->WTP[0]->WTP_Radio[i])
			  	  {
					  Radio_Type(wtp->WTP[0]->WTP_Radio[i]->Radio_Type,RType);
			  	  }
				  fprintf(cgiOut,"<tr bgcolor=%s>",setclour(cl));
				  if(wtp->WTP[0]->WTP_Radio[i])
			  	  {
					  fprintf(cgiOut,"<td id=td3>%d</td>",wtp->WTP[0]->WTP_Radio[i]->Radio_G_ID);
					  if(wtp->WTP[0]->WTP_Radio[i]->Radio_Chan==0)
					  {
						fprintf(cgiOut,"<td id=td3>%s</td>","auto");
					  }
					  else
					  {
						fprintf(cgiOut,"<td id=td3>%d</td>",wtp->WTP[0]->WTP_Radio[i]->Radio_Chan);
					  }
					  if((wtp->WTP[0]->WTP_Radio[i]->Radio_TXP == 0)||(wtp->WTP[0]->WTP_Radio[i]->Radio_TXP == 100))
					  {
						fprintf(cgiOut,"<td id=td3>%s</td>","auto");
					  }
					  else
					  {
						fprintf(cgiOut,"<td id=td3>%d</td>",wtp->WTP[0]->WTP_Radio[i]->Radio_TXP);
					  }
			  	  }
				  fprintf(cgiOut,"<td id=td3>%s</td>",RType);
				  fprintf(cgiOut,"<td align=left>"\
				  	               "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(rnum-i),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=div1>");
								   if(retu==0)/*管理员*/
	                                 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_radcon.cgi?UN=%s&ID=%s&WID=%s&FL=%s&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,t,"0",ins_id,search(lpublic,"configure"));
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_rdtail.cgi?UN=%s&ID=%s&WID=%s&FL=%s&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,t,"0",ins_id,search(lpublic,"details"));
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_radioblack.cgi?UN=%s&ID=%s&WID=%s&FL=%s&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,t,"0",ins_id,search(lwlan,"black")); 
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_radiowhite.cgi?UN=%s&ID=%s&WID=%s&FL=%s&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,radio_id,t,"0",ins_id,search(lwlan,"white")); 
                                   fprintf(cgiOut,"</div>"\
                                   "</div>"\
                                   "</div>"\
				  	"</td></tr>");	  
				  cl=!cl;
				} 
			  fprintf(cgiOut,"</table>"\
				  "</td>"\
				"</tr>"\
				"<tr>"\
			      "<td><input type=hidden name=encry_wtpdta value=%s></td>",m);
			    fprintf(cgiOut,"</tr>"\
				"<tr>"\
			      "<td><input type=hidden name=wtpid value=%s></td>",t);
			    fprintf(cgiOut,"</tr>"\
			  "</table>");
			}
			else if(result==-1)
			{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lwlan,"wtp_id_illegal1"),sizeof(alt)-1);
				memset(max_wtp_num,0,sizeof(max_wtp_num));
				snprintf(max_wtp_num,sizeof(max_wtp_num)-1,"%d",WTP_NUM-1);
				strncat(alt,max_wtp_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lwlan,"wtp_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				fprintf(cgiOut,"%s",alt);  
			}
            else if(result==-2)
		      fprintf(cgiOut,"%s",search(lwlan,"no_wtp")); 	 
			else if(result==-3)
			  fprintf(cgiOut,"%s",search(lpublic,"error")); 	 
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
if(result==1)
{
  Free_one_wtp_head(wtp);
}
if(result1==1)
{
  Free_sta_bywtpid(wtpz);  
}
if(result2==1)
{
  free_show_wtp_runtime(WTPINFO);
}
if(ret1 == 1)
{
  free_how_wtp_extension_information_v3(wtpinfo1);
}
if(ret2 == 1)
{
  free_show_ap_if_info(wtpinfo2);
}
free_instance_parameter_list(&paraHead1);
}


