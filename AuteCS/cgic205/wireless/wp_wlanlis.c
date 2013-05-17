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
* wp_wlanlis.c
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
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

#define MAX_PAGE_NUM 25     /*每页显示的最多WLAN个数*/ 

void ShowWlanListPage(char *m,char *n,int p,struct list *lpublic,struct list *lwlan);    
void DeleteWlan(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan);



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
      ShowWlanListPage(encry,str,pno,lpublic,lwlan);
	}
	else
	  ShowWlanListPage(encry,str,0,lpublic,lwlan);	
  }
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowWlanListPage(char *m,char *n,int p,struct list *lpublic,struct list *lwlan)
{    
  char IsDeleete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  DCLI_WLAN_API_GROUP *WLANINFO = NULL;
  char wlan_id[10] = { 0 };    
  char whichinterface[WLAN_IF_NAME_LEN] = { 0 };
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  int wnum = 0;             /*存放wlan的个数*/
  int i = 0,result = 0,retu = 1,cl = 1,limit = 0;                        /*颜色初值为#f9fafe*/
  int start_wlanno = 0,end_wlanno = 0,wlanno_page = 0,total_pnum = 0;    /*start_wtpno表示要显示的起始wtp id，end_wtpno表示要显示的结束wtp id，wtpno_page表示本页要显示的wtp数，total_pnum表示总页数*/
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
  fprintf(cgiOut,"<title>Wlan</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
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
	  "function page_change(obj)"\
	  "{"\
	     "var page_num = obj.options[obj.selectedIndex].value;"\
	   	 "var url = 'wp_wlanlis.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
	   	 "window.location.href = url;"\
	   	"}", m , select_insid);
	  fprintf(cgiOut,"</script>"\
	  "<script src=/instanceid_onchange.js>"\
	  "</script>"\
  "<body>");
  memset(IsDeleete,0,sizeof(IsDeleete));
  cgiFormStringNoNewlines("DeletWlan", IsDeleete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDeleete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    memset(wlan_id,0,sizeof(wlan_id));
    cgiFormStringNoNewlines("WlanID", wlan_id, 10);
	if(paraHead1)
	{
		DeleteWlan(paraHead1,wlan_id,lpublic,lwlan);
	} 
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>WLAN</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	 
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
				  fprintf(cgiOut,"<tr height=26>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>WLAN</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wlannew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> WLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
                    fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_wlanbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  if(paraHead1)
				  {
					  result=show_wlan_list(paraHead1->parameter,paraHead1->connection,&WLANINFO);
				  } 
				  if(result == 1)
				  {
				  	wnum = WLANINFO->wlan_num;
				  }
				  
				  total_pnum=((wnum%MAX_PAGE_NUM)==0)?(wnum/MAX_PAGE_NUM):((wnum/MAX_PAGE_NUM)+1);
 				  start_wlanno=p*MAX_PAGE_NUM;   
				  end_wlanno=(((p+1)*MAX_PAGE_NUM)>wnum)?wnum:((p+1)*MAX_PAGE_NUM);
				  wlanno_page=end_wlanno-start_wlanno;
				  if((wlanno_page<(MAX_PAGE_NUM/2))||(wnum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个wtp*/
				  	limit=9;
				  else if((wlanno_page<MAX_PAGE_NUM)||(wnum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个wtp*/
				  	     limit=20;
				       else         /*大于30个翻页*/
					   	 limit=21;
				  if(retu==1)  /*普通用户*/
				  	limit+=2;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
              "<table width=763 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
   "<tr style=\"padding-bottom:15px\">"\
	  "<td width=70>%s ID:</td>",search(lpublic,"instance"));
	  fprintf(cgiOut,"<td width=693>"\
		  "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_wlanlis.cgi\",\"%s\")>",m);
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
    "<td colspan=2 valign=top align=center style=\"padding-top:5px; padding-bottom:10px\">");
	if(result == 1)
	{ 
	  fprintf(cgiOut,"<table width=763 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=3>");
	  if(wnum>0)           /*如果WLAN存在*/
	  {		
		fprintf(cgiOut,"<table frame=below rules=rows width=763 border=1>"\
		"<tr align=left>"\
        "<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
        fprintf(cgiOut,"<th width=50><font id=yingwen_thead>ID</font></th>");
		if(1 == get_product_info("/var/run/mesh_flag"))
		  fprintf(cgiOut,"<th width=100><font id=yingwen_thead>Mesh</font></th>");
		else
		  fprintf(cgiOut,"<th width=100><font id=yingwen_thead>WDS</font></th>");
        fprintf(cgiOut,"<th width=200><font id=yingwen_thead>ESSID</font></th>"\
        "<th width=70><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"status"));
		fprintf(cgiOut,"<th width=120><font id=%s>%s</font><font id=yingwen_thead> ID</font></th>",search(lpublic,"menu_thead"),search(lpublic,"security"));
		fprintf(cgiOut,"<th width=110><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"if_policy"));
        fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
        "</tr>");
        for(i=start_wlanno;i<end_wlanno;i++)
		{
		  memset(menu,0,sizeof(menu));
		  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
		  snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
		  strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
		  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
		  if((WLANINFO)&&(WLANINFO->WLAN[i])&&(WLANINFO->WLAN[i]->WlanName))
		  {
			  fprintf(cgiOut,"<td>%s</td>",WLANINFO->WLAN[i]->WlanName);
		  }
		  if((WLANINFO)&&(WLANINFO->WLAN[i]))
		  {
			  fprintf(cgiOut,"<td>%d</td>",WLANINFO->WLAN[i]->WlanID);
		  }
 		  if((WLANINFO)&&(WLANINFO->WLAN[i])&&(WLANINFO->WLAN[i]->WDSStat==1))
			fprintf(cgiOut,"<td>enable</td>");
		  else
			fprintf(cgiOut,"<td>disable</td>");
		  if((WLANINFO)&&(WLANINFO->WLAN[i])&&(WLANINFO->WLAN[i]->ESSID))
		  {
			int len = 0;
			int j = 0;			
			len = strlen(WLANINFO->WLAN[i]->ESSID);
			fprintf(cgiOut,"<td>");
			for(j=0;((j<len)&&(j<ESSID_DEFAULT_LEN));j++)
			{
				if(WLANINFO->WLAN[i]->ESSID[j] == 32)/*html会将连续的多个空格显示为1个，只能通过&nbsp;代替空格*/
					fprintf(cgiOut,"&nbsp;");
				else
					fprintf(cgiOut,"%c",WLANINFO->WLAN[i]->ESSID[j]);
			}
			fprintf(cgiOut,"</td>");
		  }
		  if((WLANINFO)&&(WLANINFO->WLAN[i])&&(WLANINFO->WLAN[i]->Status==1))
			fprintf(cgiOut,"<td>disable</td>");
		  else
			fprintf(cgiOut,"<td>enable</td>");
		  if((WLANINFO)&&(WLANINFO->WLAN[i]))
		  {
			  if(WLANINFO->WLAN[i]->SecurityID==0)
			  {
				fprintf(cgiOut,"<td>%s</td>","NONE");
			  }
			  else
			  {
				fprintf(cgiOut,"<td>%d</td>",WLANINFO->WLAN[i]->SecurityID);
			  }
		  }
		  memset(whichinterface,0,sizeof(whichinterface));
		  if((WLANINFO)&&(WLANINFO->WLAN[i]))
		  {
			  CheckWIDIfPolicy(whichinterface,WLANINFO->WLAN[i]->wlan_if_policy);
		  }
		  fprintf(cgiOut,"<td>%s</td>",whichinterface);	
		  if((WLANINFO)&&(WLANINFO->WLAN[i]))
		  {
			  snprintf(wlan_id,sizeof(wlan_id)-1,"%d",WLANINFO->WLAN[i]->WlanID);	 /*int转成char*/
		  }
		  fprintf(cgiOut,"<td>"\
		                           "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(wnum-i),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=div1>");
								   if(retu==0)  /*管理员*/
								   {
								     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlannew.cgi?UN=%s target=mainFrame>%s</a></div>",m,search(lpublic,"create"));   
									 if((p>0)&&(p==((wnum-1)/MAX_PAGE_NUM))&&(((wnum-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlanlis.cgi?UN=%s&WlanID=%s&DeletWlan=%s&PN=%d&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,wlan_id,"true",p-1,select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));                             
									 else
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlanlis.cgi?UN=%s&WlanID=%s&DeletWlan=%s&PN=%d&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,wlan_id,"true",p,select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));                             
									 if((WLANINFO)&&(WLANINFO->WLAN[i])&&(WLANINFO->WLAN[i]->WlanName))
								     {
										 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlancon.cgi?UN=%s&Na=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,WLANINFO->WLAN[i]->WlanName,wlan_id,p,select_insid,search(lpublic,"configure"));							   
								     }
									 else
									 {
										 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlancon.cgi?UN=%s&Na=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,"",wlan_id,p,select_insid,search(lpublic,"configure"));							   
								     }
	                                 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlanmapinter.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>L3%s</a></div>",m,wlan_id,p,select_insid,search(lpublic,"interface"));                             
								   }
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlandta.cgi?UN=%s&ID=%s&FL=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wlan_id,whichinterface,p,select_insid,search(lpublic,"details"));
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_stakind.cgi?UN=%s&ID=%s&ST=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wlan_id,"wlan",p,select_insid,search(lwlan,"station"));                             
								   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlanblack.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wlan_id,p,select_insid,search(lwlan,"black"));                             
								   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlanwhite.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,wlan_id,p,select_insid,search(lwlan,"white"));        
								   if(retu==0)  /*管理员*/
								     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wlanmapvlan.cgi?UN=%s&ID=%s&FL=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>WLAN_VLAN%s</a></div>",m,wlan_id,whichinterface,p,select_insid,search(lpublic,"map"));                             	
								   fprintf(cgiOut,"</div>"\
                                   "</div>"\
                                   "</div>"\
          "</td></tr>");
		  cl=!cl;
		}	
		fprintf(cgiOut,"</table>");
	  }
	  else				 /*no wlan exist*/
		fprintf(cgiOut,"%s",search(lwlan,"no_wlan"));
	  fprintf(cgiOut,"</td></tr>");
	  if(wnum>MAX_PAGE_NUM)               /*大于30个wtp时，显示翻页的链接*/
	  {
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(p!=0) 
	      fprintf(cgiOut,"<td align=left width=100><a href=wp_wlanlis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,p-1,select_insid,search(lpublic,"up_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"<td align=center width=463>%s",search(lpublic,"jump_to_page1"));
			                             fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this)>");
										 for(i=0;i<total_pnum;i++)
										 {
										   if(i==p)
			                                 fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
										   else
										     fprintf(cgiOut,"<option value=%d>%d",i,i+1);
										 }
			                             fprintf(cgiOut,"</select>"\
			                             "%s</td>",search(lpublic,"jump_to_page2"));
		if(p!=((wnum-1)/MAX_PAGE_NUM))
	      fprintf(cgiOut,"<td align=right width=100><a href=wp_wlanlis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,p+1,select_insid,search(lpublic,"down_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"</tr>");
	  }
      fprintf(cgiOut,"</table>");
	}
	else if(result == -1)
	  fprintf(cgiOut,"%s",search(lwlan,"no_wlan"));
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
if(result == 1)
{
  Free_wlan_head(WLANINFO);
}
free_instance_parameter_list(&paraHead1);
}

void DeleteWlan(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan)
{
  char *endptr = NULL; 
  char alt[100] = { 0 };
  char max_wlan_num[10] = { 0 };
  int wlanid = 0,ret = 0;

  wlanid=strtoul(ID,&endptr,10);  /*char转成int*/ 
  ret=delete_wlan(ins_para->parameter,ins_para->connection,wlanid);
  switch(ret)
  {
    case SNMPD_CONNECTION_ERROR:
	case 0:ShowAlert(search(lwlan,"delete_wlan_fail"));
		   break;
	case 1:ShowAlert(search(lwlan,"delete_wlan_succ"));
		   break;
	case -1:{
			  memset(alt,0,sizeof(alt));
			  strncpy(alt,search(lwlan,"wlan_id_illegal1"),sizeof(alt)-1);
			  memset(max_wlan_num,0,sizeof(max_wlan_num));
			  snprintf(max_wlan_num,sizeof(max_wlan_num)-1,"%d",WLAN_NUM-1);
			  strncat(alt,max_wlan_num,sizeof(alt)-strlen(alt)-1);
			  strncat(alt,search(lwlan,"wlan_id_illegal2"),sizeof(alt)-strlen(alt)-1);
	  	      ShowAlert(alt);
		  	  break;
			}
	case -2:ShowAlert(search(lwlan,"wlan_not_exist"));
			break;
	case -3:ShowAlert(search(lwlan,"dis_wlan"));
	        break;
	case -4:ShowAlert(search(lpublic,"error"));
			break;
	case -5:ShowAlert(search(lwlan,"delete_inter_from_ebr"));
			break;
  }
}


