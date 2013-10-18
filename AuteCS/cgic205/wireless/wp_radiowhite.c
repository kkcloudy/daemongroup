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
* wp_radiowhite.c
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
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_sta.h"
#include "ws_dbus_list_interface.h"

void ShowRadioWhitePage(char *m,char *n,char *t,char *b,char *flag,char *ins_id,struct list *lpublic,struct list *lwlan);  


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };  
  char instance_id[10] = { 0 };
  char flag[5] = { 0 }; /*fla=="1",表示上一页为wp_radiolis.cgi,否则上一页为wp_wtpdta.cgi*/ 
  char *str = NULL;      
  char ID[5] = { 0 };
  char WlanID[5] = { 0 };
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lwlan=get_chain_head("../htdocs/text/wlan.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(flag,0,sizeof(flag));
  memset(ID,0,sizeof(ID));
  memset(WlanID,0,sizeof(WlanID)); 
  cgiFormStringNoNewlines("FL", flag, 5);
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
    cgiFormStringNoNewlines("ID", ID, 5);
	cgiFormStringNoNewlines("INSTANCE_ID",instance_id,10);  
	str=dcryption(encry);
    if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
    else if(cgiFormStringNoNewlines("WLAN_ID", WlanID, 5)!=cgiFormNotFound)    /*从wp_wlanbla_del.cgi进入该页*/
  	  ShowRadioWhitePage(encry,str,ID,WlanID,flag,instance_id,lpublic,lwlan);
    else
	  ShowRadioWhitePage(encry,str,ID,"1",flag,instance_id,lpublic,lwlan);
  }
  else
  {	  
    cgiFormStringNoNewlines("encry_radiowhite",encry,BUF_LEN);	
	cgiFormStringNoNewlines("radio_id", ID, 5);
	cgiFormStringNoNewlines("wlan_id", WlanID, 5);
	cgiFormStringNoNewlines("instance_id",instance_id,10);  
	str=dcryption(encry);
    if(str==NULL)
	  ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
    else
  	  ShowRadioWhitePage(encry,str,ID,WlanID,flag,instance_id,lpublic,lwlan);
  }
  if(strcmp(instance_id,"")==0)
  {
    memset(instance_id,0,sizeof(instance_id));
    strncpy(instance_id,"0",sizeof(instance_id)-1);
  }  

  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowRadioWhitePage(char *m,char *n,char *t,char *b,char *flag,char *ins_id,struct list *lpublic,struct list *lwlan)
{  
  int i = 0,j = 0,retu = 1,result = 0,limit = 0;  
  int radio_id = 0,wlan_id = 0;
  int white_num = 0;
  char wtp_id[10] = { 0 };
  char max_wlan_num[5] = { 0 };
  char pno[10] = { 0 };
  char alt[100] = { 0 };
  char max_radio_num[10] = { 0 };
  memset(wtp_id,0,sizeof(wtp_id));
  memset(pno,0,sizeof(pno));
  cgiFormStringNoNewlines("WID", wtp_id, 10); 
  cgiFormStringNoNewlines("PN",pno,10);
  struct dcli_bss_info *bss = NULL;
  char tem_mac[30] = { 0 };
  struct maclist *tmp = NULL;
  int ret = 0;
  DCLI_RADIO_API_GROUP_ONE *radio = NULL;
  instance_parameter *paraHead1 = NULL;
  dbus_parameter ins_para;
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>"\
  "<form method=post>"\
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lwlan,"white"));   /*突出显示*/
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
				  radio_id= atoi(t);	/*char转成int，10代表十进制*/	  
				  wlan_id= atoi(b);	/*char转成int，10代表十进制*/
				  
				  get_slotID_localID_instanceID(ins_id,&ins_para);	
				  get_instance_dbus_connection(ins_para, &paraHead1, INSTANCE_STATE_WEB);

				  if(paraHead1)
				  {
					  result=show_radio_bss_mac_list(paraHead1->parameter,paraHead1->connection,radio_id,b,&bss);
				  }
				  if(result==1)
				  	white_num = bss->acl_conf.num_accept_mac;
				  if(strcmp(flag,"0")==0)
				  {
					  limit=white_num-6;
					  if(retu==1)  /*普通用户*/
						limit+=7;
				  }
				  else
				  {
					  limit=white_num+1;
					  if(retu==1)  /*普通用户*/
						limit+=3;
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
 "<table width=200 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
  "<tr valign=middle>"\
    "<td align=left>"\
	  "<table width=300 border=0 cellspacing=0 cellpadding=0>"\
	  "<tr>"\
	  "<td id=ins_style colspan=3>%s:%s</td>",search(lpublic,"instance"),ins_id);
	  fprintf(cgiOut,"</tr>"\
	  "<tr style=\"padding-bottom:20px\">"
	  "<td width=50>BSS ID:</td>"
	  "<td width=120><select name=wlan_id id=wlan_id style=width:40px onchange=\"javascript:this.form.submit();\">");
	  if(paraHead1)
	  {
		  ret = show_radio_one(paraHead1->parameter,paraHead1->connection,radio_id,&radio);
	  }
	  if((ret==1)&&(radio->wlan_num>0))
	  {
	  	for(i=0;i<radio->wlan_num;i++)
	  	{
	  		if((radio->RADIO[0])&&(radio->RADIO[0]->WlanId))
	  		{
				if(radio->RADIO[0]->WlanId[i]==wlan_id)
				  fprintf(cgiOut,"<option value=%d selected=selected>%d",radio->RADIO[0]->WlanId[i],radio->RADIO[0]->WlanId[i]);
				else			  
				  fprintf(cgiOut,"<option value=%d>%d",radio->RADIO[0]->WlanId[i],radio->RADIO[0]->WlanId[i]);
	  		}
	  	}
	  }
	  if(ret==1)
      {
        Free_radio_one_head(radio);
      }	
	  fprintf(cgiOut,"</select></td>");
	  if(strcmp(flag,"0")==0)
	    fprintf(cgiOut,"<td width=130><a href=wp_bssbw.cgi?UN=%s&INSTANCE_ID=%s target=mainFrame><font id=yingwen_san style=\"text-decoration: underline; color=blue; font-size:11px\">BSS MAC </font><font id=%s style=\"text-decoration: underline; color=blue; font-size:11px\">%s</font></a></td>",m,ins_id,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));
	  else
	  	fprintf(cgiOut,"<td width=130>&nbsp;</td>");
	  fprintf(cgiOut,"</tr>");
		if(result==1)
		{   
		  if(white_num>0)
		  {
			 fprintf(cgiOut,"<tr>"
			 "<td colspan=3>"
			 "<table width=300 border=0 cellspacing=0 cellpadding=0>"\
	         "<tr align=left height=10 valign=top>"\
	            "<td id=thead1>Radio%s BSS%s %s</td>",t,b,search(lwlan,"white"));
	         fprintf(cgiOut,"</tr>"\
             "<tr>"\
             "<td style=\"padding-left:20px\">");
			 if(retu==0)  /*管理员*/
			   fprintf(cgiOut,"<table frame=below rules=rows width=200 border=1>");
			 else
			   fprintf(cgiOut,"<table frame=below rules=rows width=130 border=1>");		

			for(j=0,tmp = bss->acl_conf.accept_mac; ((j<bss->acl_conf.num_accept_mac)&&(NULL != tmp)); j++,tmp = tmp->next)
			{
				memset(tem_mac,0,sizeof(tem_mac));
				snprintf(tem_mac,sizeof(tem_mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",MAC2STRZ(tmp->addr));

				fprintf(cgiOut,"<tr align=left>"\
					"<td id=td1>%s</td>",tem_mac);
				if(retu==0)  /*管理员*/
					fprintf(cgiOut,"<td id=td2><a href=wp_wlanbla_del.cgi?UN=%s&ID=%s&WLAN_ID=%s&stat=%s&mac=%s&Type=%s&FL=%s&WID=%s&INSTANCE_ID=%s target=mainFrame class=top>%s</td>",m,t,b,"white",tem_mac,"radio",flag,wtp_id,ins_id,search(lpublic,"delete"));
				fprintf(cgiOut,"</tr>");
			}
			  fprintf(cgiOut,"</table></td>"\
			  "</tr>"\
			  "</table></td></tr>");
		  }
		  else
		    fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",search(lwlan,"white_empty"));
		}
		else if(result==-1)
		{
			memset(alt,0,sizeof(alt));
			strncpy(alt,search(lwlan,"radio_id_illegal1"),sizeof(alt)-1);
			memset(max_radio_num,0,sizeof(max_radio_num));
			snprintf(max_radio_num,sizeof(max_radio_num)-1,"%d",G_RADIO_NUM-1);
			strncat(alt,max_radio_num,sizeof(alt)-strlen(alt)-1);
			strncat(alt,search(lwlan,"radio_id_illegal2"),sizeof(alt)-strlen(alt)-1);
			fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",alt);
		}
		else if(result==-2)
		{
			memset(alt,0,sizeof(alt));
			strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
			memset(max_wlan_num,0,sizeof(max_wlan_num));
			snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
			strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
			strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
			fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",alt);
		}
		else if(result==-3)
			fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",search(lwlan,"bss_not_exist"));
		else if(result==-4)
			fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",search(lpublic,"error"));
		else if(result==-5)
			fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",search(lpublic,"unknown_id_format"));
		else
          fprintf(cgiOut,"<tr><td colspan=3>%s</td></tr>",search(lpublic,"contact_adm"));
		fprintf(cgiOut,"<tr>"\
		  "<td><input type=hidden name=encry_radiowhite value=%s></td>",m);		
		  fprintf(cgiOut,"<td><input type=hidden name=radio_id value=%s></td>",t);
		  fprintf(cgiOut,"<td><input type=hidden name=FL value=%s></td>",flag);	
		fprintf(cgiOut,"</tr>"\
		"<tr>"\
		  "<td><input type=hidden name=WID value=%s></td>",wtp_id);		
		  fprintf(cgiOut,"<td><input type=hidden name=PN value=%s></td>",pno);	
		  fprintf(cgiOut,"<td><input type=hidden name=instance_id value=%s></td>",ins_id);	
		fprintf(cgiOut,"</tr>"\
		"</table>"
	"</td>"\
 " </tr>"\
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
  Free_mac_head(bss);
}
free_instance_parameter_list(&paraHead1);
}


