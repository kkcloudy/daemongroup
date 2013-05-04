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
* wp_wtpver.c
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
#include "ws_dcli_vrrp.h"
#include <libxml/xpathInternals.h>
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

#define MAX_PAGE_NUM 25     /*每页显示的最多AP模式个数*/   

void ShowWtpListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan);    


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };  
  char page_no[5] = { 0 };  
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
	if(cgiFormStringNoNewlines("PN", page_no, 5)!=cgiFormNotFound )  /*点击翻页进入该页面*/
	{
	  pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowWtpListPage(encry,pno,str,lpublic,lwlan);
	}
	else
	  ShowWtpListPage(encry,0,str,lpublic,lwlan);	
  }
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWtpListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan)
{  
  char wtp_state[20] = { 0 };
  int wnum = 0;      
  DCLI_WTP_API_GROUP_ONE *WTPINFO = NULL;
  char apmodel[255] = { 0 };
  char versionname[255] = { 0 };  	
  char versionpath[255] = { 0 };
  int i = 0,retu = 1,result = 0,cl = 1;                        /*颜色初值为#f9fafe*/  
  int limit = 0,start_wtpno = 0,end_wtpno = 0,wtpno_page = 0,total_pnum = 0;    /*start_wtpno表示要显示的起始wtp id，end_wtpno表示要显示的结束wtp id，wtpno_page表示本页要显示的wtp数，total_pnum表示总页数*/
  char model_id[20] = { 0 };  
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  char *tempt = NULL;
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  char temp[10] = { 0 };
  dbus_parameter ins_para;

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
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
    "#div1{ width:42px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:40px; height:15px; padding-left:5px; padding-top:3px}"\
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
	  "function page_change(obj)"\
	  "{"\
	     "var page_num = obj.options[obj.selectedIndex].value;"\
	   	 "var url = 'wp_wtpver.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
	   	 "window.location.href = url;"\
	   	"}", m , select_insid);
	  fprintf(cgiOut,"</script>"\
	  "<script src=/instanceid_onchange.js>"\
	  "</script>"\
  "<body>");
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lwlan,"ap"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><a href=wp_wlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
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
                  "</tr>");	              
				  retu=checkuser_group(t);
 			        fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtplis.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san> AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));                       
                    fprintf(cgiOut,"</tr>"\
					"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpsearch.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"Search"));                       
                    fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> AP</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                    fprintf(cgiOut,"</tr>");				 
				  }
				    fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>AP</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"mode"));   /*突出显示*/
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
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
  					"<td align=left id=tdleft><a href=wp_conAutoAp.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san>AP</font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"config_auto"),search(lpublic,"menu_san"),search(lpublic,"policy"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  if(paraHead1)
				  {
					  result=show_version(paraHead1->parameter,paraHead1->connection,&WTPINFO);
				  } 
				  if((result ==1)&&(WTPINFO))
				  {
					wnum = WTPINFO->num;
				  }
				  total_pnum=((wnum%MAX_PAGE_NUM)==0)?(wnum/MAX_PAGE_NUM):((wnum/MAX_PAGE_NUM)+1);
				  start_wtpno=n*MAX_PAGE_NUM;   
				  end_wtpno=(((n+1)*MAX_PAGE_NUM)>wnum)?wnum:((n+1)*MAX_PAGE_NUM);
				  wtpno_page=end_wtpno-start_wtpno;
				  if((wtpno_page<(MAX_PAGE_NUM/2))||(wnum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个wtp*/
				  	limit=2;
				  else if((wtpno_page<MAX_PAGE_NUM)||(wnum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个wtp*/
			  	    limit=13;
			      else         /*大于30个翻页*/
				   	limit=14;
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
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
   "<table width=773 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
	"<tr style=\"padding-bottom:15px\">"\
	   "<td width=70>%s ID:</td>",search(lpublic,"instance"));
	   fprintf(cgiOut,"<td width=703>"\
		   "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_wtpver.cgi\",\"%s\")>",m);
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
	"<tr>"\
    "<td colspan=2 align=center valign=top style=\"padding-top:5px; padding-bottom:10px\">");
			if(result == 1)    /*显示所有wtp的信息，wlan_head返回wtp信息链表的链表头*/
	        { 
	          fprintf(cgiOut,"<table width=773 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
              "<td align=left colspan=3>");
	          if(wnum>0)       /*如果WTP存在*/
	          {			          
				if(retu==0)  /*管理员*/
				  fprintf(cgiOut,"<table frame=below rules=rows width=773 border=1>");
				else
				  fprintf(cgiOut,"<table frame=below rules=rows width=760 border=1>");
				fprintf(cgiOut,"<tr align=left>"\
                "<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"mode"));
                fprintf(cgiOut,"<th width=130><font id=yingwen_thead>AP</font><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"ap_version"));
                fprintf(cgiOut,"<th width=330><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"path"));
                fprintf(cgiOut,"<th width=130><font id=yingwen_thead>Radio</font><font id=%s> %s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"count"));
                fprintf(cgiOut,"<th width=70><font id=yingwen_thead>Bss</font><font id=%s> %s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"count"));
				if(retu==0)  /*管理员*/
				  fprintf(cgiOut,"<th width=13>&nbsp;</th>");
	            fprintf(cgiOut,"</tr>");
				
				for (i = start_wtpno; i < end_wtpno; i++) 
				{
					if((WTPINFO)&&(WTPINFO->AP_VERSION[i]))
					{		          
					  memset(menu,0,sizeof(menu));
					  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
			          snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
			          strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
	                  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
					  if(WTPINFO->AP_VERSION[i]->apmodel)
					  {
						  fprintf(cgiOut,"<td>%s</td>",WTPINFO->AP_VERSION[i]->apmodel);
					  }
					  if(WTPINFO->AP_VERSION[i]->versionname)
					  {
						  fprintf(cgiOut,"<td style=\"word-break:break-all\">%s</td>",WTPINFO->AP_VERSION[i]->versionname);
					  }
					  if(WTPINFO->AP_VERSION[i]->versionpath)
					  {
						  fprintf(cgiOut,"<td style=\"word-break:break-all\">%s</td>",WTPINFO->AP_VERSION[i]->versionpath);
					  }
	                  fprintf(cgiOut,"<td>%d</td>",WTPINFO->AP_VERSION[i]->radionum);
	                  fprintf(cgiOut,"<td>%d</td>",WTPINFO->AP_VERSION[i]->bssnum);
					  memset(wtp_state,0,sizeof(wtp_state));	
					 if(retu==0)  /*管理员*/
					 {
				      fprintf(cgiOut,"<td>"\
					 	              "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(wnum-i),menu,menu);
	                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
	                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
	                                   fprintf(cgiOut,"<div id=div1>");
									   								   
									   tempt=(char *)malloc(64);	
									   if(tempt)
									   {
										   memset(tempt,0,64);							
										   if(WTPINFO->AP_VERSION[i]->apmodel)
										   {
											   tempt=replace_url(WTPINFO->AP_VERSION[i]->apmodel," ","%20");
										   }
									   }

									   memset(apmodel,0,sizeof(apmodel));
									   memset(versionname,0,sizeof(versionname));
									   memset(versionpath,0,sizeof(versionpath));
									   if(WTPINFO->AP_VERSION[i]->apmodel)
									   {
									   	 strncpy(apmodel,WTPINFO->AP_VERSION[i]->apmodel,sizeof(apmodel)-1);
									   }
									   if(WTPINFO->AP_VERSION[i]->versionname)
									   {
									   	 strncpy(versionname,WTPINFO->AP_VERSION[i]->versionname,sizeof(versionname)-1);
									   }
									   if(WTPINFO->AP_VERSION[i]->versionpath)
									   {
									   	 strncpy(versionpath,WTPINFO->AP_VERSION[i]->versionpath,sizeof(versionpath)-1);
									   }
									   
									   if(tempt==NULL)
									     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpmcon.cgi?UN=%s&WtpID=%s&INSTANCE_ID=%s&ver=%s&path=%s&rad=%d&bss=%d&PN=%d target=mainFrame>%s</a></div>",m,apmodel,select_insid,versionname,versionpath,WTPINFO->AP_VERSION[i]->radionum,WTPINFO->AP_VERSION[i]->bssnum,n,search(lpublic,"configure"));
									   else
									     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpmcon.cgi?UN=%s&WtpID=%s&INSTANCE_ID=%s&ver=%s&path=%s&rad=%d&bss=%d&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,tempt,select_insid,versionname,versionpath,WTPINFO->AP_VERSION[i]->radionum,WTPINFO->AP_VERSION[i]->bssnum,n,search(lpublic,"configure"));
									   
									   FREE_OBJECT(tempt);
									   
									   fprintf(cgiOut,"</div>"\
	                                   "</div>"\
					 	"</td>");
					 }
					 fprintf(cgiOut,"</tr>");
	  	             cl=!cl;
			        }
				}		        				
				  fprintf(cgiOut,"</table>");
	          }
	          else				 /*no wlan exist*/
		        fprintf(cgiOut,"%s",search(lwlan,"no_wtp"));
			  fprintf(cgiOut,"</td></tr>");
			  if(wnum>MAX_PAGE_NUM)               /*大于30个wtp时，显示翻页的链接*/
			  {
			    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
				if(n!=0)          /**/
			      fprintf(cgiOut,"<td align=left width=100><a href=wp_wtpver.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n-1,select_insid,search(lpublic,"up_page"));
				else
				  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
				fprintf(cgiOut,"<td align=center width=573>%s",search(lpublic,"jump_to_page1"));
												 fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this)>");
												 for(i=0;i<total_pnum;i++)
												 {
												   if(i==n)
													 fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
												   else
													 fprintf(cgiOut,"<option value=%d>%d",i,i+1);
												 }
												 fprintf(cgiOut,"</select>"\
												 "%s</td>",search(lpublic,"jump_to_page2"));
				if(n!=((wnum-1)/MAX_PAGE_NUM))
			      fprintf(cgiOut,"<td align=right width=100><a href=wp_wtpver.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n+1,select_insid,search(lpublic,"down_page"));
				else
				  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
			    fprintf(cgiOut,"</tr>");
			  }
              fprintf(cgiOut,"</table>");
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
if(result==1)
{
  Free_wtp_model(WTPINFO);
}
free_instance_parameter_list(&paraHead1);
}


