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
* wp_verbind.c
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

#define MAX_PAGE_NUM 25     /*每页显示的最多AP模式个数*/   

void ShowVerbindPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan);    
void DeleteBindInfo(instance_parameter *ins_para,char *ap_model,struct list *lpublic,struct list *lwlan);


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
      ShowVerbindPage(encry,pno,str,lpublic,lwlan);
	}
	else
	  ShowVerbindPage(encry,0,str,lpublic,lwlan);	
  }
  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowVerbindPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan)
{  
  char IsDeleete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  int model_tar_file_num = 0;      
  struct model_tar_file head,*q = NULL;
  int i = 0,retu = 1,result = 0,cl = 1;                        /*颜色初值为#f9fafe*/  
  int limit = 0,start_verbindno = 0,end_verbindno = 0,verbindno_page = 0,total_pnum = 0;    /*start_verbindno表示要显示的起始verbind，end_verbindno表示要显示的结束verbind，verbindno_page表示本页要显示的verbind数，total_pnum表示总页数*/
  char ap_model[256] = { 0 };  
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  char *tempt = NULL;
  char *endptr = NULL; 
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
	   	 "var url = 'wp_verbind.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
	   	 "window.location.href = url;"\
	   	"}", m , select_insid);
	  fprintf(cgiOut,"</script>"\
	  "<script src=/instanceid_onchange.js>"\
	  "</script>"\
  "<body>");
  memset(IsDeleete,0,sizeof(IsDeleete));
  cgiFormStringNoNewlines("DeletBindinfo", IsDeleete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDeleete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    memset(ap_model,0,sizeof(ap_model));
    cgiFormStringNoNewlines("Model", ap_model, 256);
	if(paraHead1)
	{
		DeleteBindInfo(paraHead1,ap_model,lpublic,lwlan);
	}
  }
  fprintf(cgiOut,"<form method=post>"\
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
				  fprintf(cgiOut,"<tr height=25>"\
				    "<td align=left id=tdleft><a href=wp_wtpver.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>AP</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"mode"));                       
                  fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_wtpdown.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"version_upload"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  fprintf(cgiOut,"<tr height=26>"\
				    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lwlan,"version_bind"));   /*突出显示*/
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
					  result=show_model_tar_file_bind_info(paraHead1->parameter,paraHead1->connection,&head,&model_tar_file_num);
				  }

				  total_pnum=((model_tar_file_num%MAX_PAGE_NUM)==0)?(model_tar_file_num/MAX_PAGE_NUM):((model_tar_file_num/MAX_PAGE_NUM)+1);
				  start_verbindno=n*MAX_PAGE_NUM;   
				  end_verbindno=(((n+1)*MAX_PAGE_NUM)>model_tar_file_num)?model_tar_file_num:((n+1)*MAX_PAGE_NUM);
				  verbindno_page=end_verbindno-start_verbindno;
				  if((verbindno_page<(MAX_PAGE_NUM/2))||(model_tar_file_num==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个wtp*/
				  	limit=2;
				  else if((verbindno_page<MAX_PAGE_NUM)||(model_tar_file_num==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个wtp*/
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
   "<table width=473 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
	"<tr style=\"padding-bottom:15px\">"\
	   "<td width=70>%s ID:</td>",search(lpublic,"instance"));
	   fprintf(cgiOut,"<td width=90>"\
		   "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_verbind.cgi\",\"%s\")>",m);
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
	   "<td width=313><a href=wp_bind_file.cgi?UN=%s&PN=%d&INSTANCE_ID=%s><font color=blue size=2>%s</font></a></td>",m,n,select_insid,search(lwlan,"version_bind"));
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
    "<td colspan=3 align=center valign=top style=\"padding-top:5px; padding-bottom:10px\">");		    
			if(result == 1)
	        { 
	          fprintf(cgiOut,"<table width=473 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
              "<td align=left colspan=3>");
	          if(model_tar_file_num>0)
	          {			          
				if(retu==0)  /*管理员*/
				  fprintf(cgiOut,"<table frame=below rules=rows width=473 border=1>");
				else
				  fprintf(cgiOut,"<table frame=below rules=rows width=460 border=1>");
				fprintf(cgiOut,"<tr align=left>"\
                "<th width=230><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"model"));
                fprintf(cgiOut,"<th width=230><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"bind_file"));
				if(retu==0)  /*管理员*/
				  fprintf(cgiOut,"<th width=13>&nbsp;</th>");
	            fprintf(cgiOut,"</tr>");
				q=head.next;
				for(i=0;i<start_verbindno;i++)
				{
					if(q)
					{
						q=q->next;
					}
				}
				
				for(i = start_verbindno; ((i < end_verbindno)&&(i < model_tar_file_num)&&(q)); i++) 
				{		          
			 	  memset(menu,0,sizeof(menu));
				  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
		          snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
		          strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
                  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
				  if(q->apmodel)
                    fprintf(cgiOut,"<td>%s</td>",q->apmodel);
				  if(q->filename)
                    fprintf(cgiOut,"<td style=\"word-break:break-all\">%s</td>",q->filename);
				  if(retu==0)  /*管理员*/
				  {
			        fprintf(cgiOut,"<td>"\
				 	              "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(model_tar_file_num-i),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=div1>");

								   tempt=(char *)malloc(64);
								   if(tempt)
								   {
									   memset(tempt,0,sizeof(tempt));								   
									   tempt=replace_url(q->apmodel," ","%20");
								   }				
								   
								   if(tempt==NULL)
								   {
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_bind_file.cgi?UN=%s&Model=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,q->apmodel,n,select_insid,search(lpublic,"bind"));
									   if((n>0)&&(n==((model_tar_file_num-1)/MAX_PAGE_NUM))&&(((model_tar_file_num-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
										 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_verbind.cgi?UN=%s&Model=%s&PN=%d&DeletBindinfo=%s&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,q->apmodel,n-1,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));
									   else
										 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_verbind.cgi?UN=%s&Model=%s&PN=%d&DeletBindinfo=%s&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,q->apmodel,n,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));
								   }
								   else
								   {
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_bind_file.cgi?UN=%s&Model=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,tempt,n,select_insid,search(lpublic,"bind"));
									   if((n>0)&&(n==((model_tar_file_num-1)/MAX_PAGE_NUM))&&(((model_tar_file_num-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
										 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_verbind.cgi?UN=%s&Model=%s&PN=%d&DeletBindinfo=%s&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,tempt,n-1,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));
									   else
										 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_verbind.cgi?UN=%s&Model=%s&PN=%d&DeletBindinfo=%s&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,tempt,n,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));
								   }
								   
								   FREE_OBJECT(tempt);								   
								   fprintf(cgiOut,"</div>"\
                                   "</div>"\
				 	"</td>");
				 }
				 fprintf(cgiOut,"</tr>");
  	             cl=!cl;
				 q=q->next;
			    }
				fprintf(cgiOut,"</table>");
	          }
	          else
		        fprintf(cgiOut,"%s",search(lwlan,"no_bind_info"));
			  fprintf(cgiOut,"</td></tr>");
			  if(model_tar_file_num>MAX_PAGE_NUM)               /*大于30个时，显示翻页的链接*/
			  {
			    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
				if(n!=0)          /**/
			      fprintf(cgiOut,"<td align=left width=100><a href=wp_verbind.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n-1,select_insid,search(lpublic,"up_page"));
				else
				  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
				fprintf(cgiOut,"<td align=center width=273>%s",search(lpublic,"jump_to_page1"));
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
				if(n!=((model_tar_file_num-1)/MAX_PAGE_NUM))
			      fprintf(cgiOut,"<td align=right width=100><a href=wp_verbind.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n+1,select_insid,search(lpublic,"down_page"));
				else
				  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
			    fprintf(cgiOut,"</tr>");
			  }
              fprintf(cgiOut,"</table>");
	        }
			else if(result == -1)
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
if((result==1)&&(model_tar_file_num>0))
{
  Free_show_model_tar_file_bind_info(&head);
}
free_instance_parameter_list(&paraHead1);
}

void DeleteBindInfo(instance_parameter *ins_para,char *ap_model,struct list *lpublic,struct list *lwlan)
{
  int ret = 0;  
  ret=delete_model_bind_info_config(ins_para->parameter,ins_para->connection,ap_model);
  switch(ret)
  {
    case SNMPD_CONNECTION_ERROR:
	case 0:ShowAlert(search(lwlan,"delete_bindinfo_fail"));
	       break;
	case 1:ShowAlert(search(lwlan,"delete_bindinfo_succ"));
		   break;
	case -1:ShowAlert(search(lpublic,"malloc_error"));
   		    break;
	case -2:ShowAlert(search(lwlan,"does_not_surport_model"));
			break;
	case -3:ShowAlert(search(lpublic,"update_is_process"));
			break;
	case -4:ShowAlert(search(lwlan,"model_has_not_bound"));
			break;
	case -5:ShowAlert(search(lpublic,"error"));
			break;
  }
}

