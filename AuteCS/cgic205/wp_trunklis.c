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
* wp_trunklis.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for trunk list
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "npd/nam/npd_amapi.h"
#include "ws_ec.h"
#include "ws_public.h"
#include "ws_trunk.h"
#include "ws_usrinfo.h"
#include "ws_err.h"

#define MAX_PAGE_NUM 30     /*每页显示的最多Tunk个数*/   

void ShowTrunkListPage(char *m,int n,char *t,struct list *lpublic,struct list *lcon);    
void DeleteTrunk(char *ID,struct list *lpublic,struct list *lcon);


int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN);  
  char *page_no=(char *)malloc(5);  
  char *str;      
  char *endptr = NULL;  
  int pno;
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  struct list *lcon;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcon=get_chain_head("../htdocs/text/control.txt");
  memset(encry,0,BUF_LEN);
  memset(page_no,0,5);
  cgiFormStringNoNewlines("UN", encry, BUF_LEN);   
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
  {        
	if(cgiFormStringNoNewlines("PN", page_no, 5)!=cgiFormNotFound )  /*点击翻页进入该页面*/
	{
	  pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowTrunkListPage(encry,pno,str,lpublic,lcon);
	}
	else
	  ShowTrunkListPage(encry,0,str,lpublic,lcon);	
  }
  free(encry);
  free(page_no);
  release(lpublic);  
  release(lcon);
  return 0;
}

void ShowTrunkListPage(char *m,int n,char *t,struct list *lpublic,struct list *lcon)
{  
  FILE *fp;
  char lan[3];
  char *IsDeleete=(char *)malloc(10);
  struct trunk_profile head,*q;   
  int tnum = 0;            
  int i,result,retu,cl=1;                              /*颜色初值为#f9fafe*/  
  int limit,start_trunkno,end_trunkno,trunkno_page;    /*start_trunkno表示要显示的起始trunk id，end_trunkno表示要显示的结束trunk id，trunkno_page表示本页要显示的trunk数*/
  char *trunk_id=(char *)malloc(10);  
  char *menu_id=(char *)malloc(10);
  char *menu=(char *)malloc(15);
  head.trunkId=0;
  head.mSlotNo=0;
  head.mPortNo=0;
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Wtp</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>");
  fprintf(cgiOut,"<style>"\
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
  "<body>");
  memset(IsDeleete,0,10);
  cgiFormStringNoNewlines("DeletTrunk", IsDeleete, 10);
  if(strcmp(IsDeleete,"true")==0)
  {
    memset(trunk_id,0,10);
    cgiFormStringNoNewlines("TrunkID", trunk_id, 10);
    DeleteTrunk(trunk_id,lpublic,lcon);
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lcon,"trunk_manage"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	    if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)		 /*以只读方式打开资源文件*/
		{
			ShowAlert(search(lpublic,"error_open"));
	    }
		else
		{
			fseek(fp,4,0);						/*将文件指针移到离文件首4个字节处，即lan=之后*/
			fgets(lan,3,fp);	   
			fclose(fp);
		}

	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td width=62 align=center><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));		
	  fprintf(cgiOut,"<td width=62 align=center><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcon,"trunk_list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(t);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
  					  "<td align=left id=tdleft><a href=wp_trunknew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s<font></a></td>",m,search(lpublic,"menu_san"),search(lcon,"create_trunk"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  ccgi_dbus_init();
				  result=show_trunk_list(&head,&tnum);
				  start_trunkno=n*MAX_PAGE_NUM;   
				  end_trunkno=(((n+1)*MAX_PAGE_NUM)>tnum)?tnum:((n+1)*MAX_PAGE_NUM);
				  trunkno_page=end_trunkno-start_trunkno;
				  if((trunkno_page<(MAX_PAGE_NUM/2))||(tnum==(MAX_PAGE_NUM/2)))   /*该页显示1--(MAX_PAGE_NUM/2-1)个或者一共有(MAX_PAGE_NUM/2)个trunk*/
				  	limit=11;
				  else if((trunkno_page<MAX_PAGE_NUM)||(tnum==MAX_PAGE_NUM))  /*该页显示(MAX_PAGE_NUM/2)--(MAX_PAGE_NUM-1)个或者一共有MAX_PAGE_NUM个trunk*/
				  	     limit=23;
				       else         /*大于30个翻页*/
					   	 limit=25;
				  if(retu==1)  /*普通用户*/
				  	limit+=1;
				  for(i=0;i<limit;i++)
	              {
  				    fprintf(cgiOut,"<tr height=25>"\
                      "<td id=tdleft>&nbsp;</td>"\
                    "</tr>");
	              }
                fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
   "<table width=463 height=230 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
	"<tr>"\
    "<td align=center valign=top style=\"padding-top:5px; padding-bottom:10px\">");
			if(result == 1)    /*显示所有trunk的信息，head返回trunk信息链表的链表头*/
	        { 
	          fprintf(cgiOut,"<table width=463 border=0 cellspacing=0 cellpadding=0>"\
              "<tr>"\
              "<td align=left colspan=2>");
	          if(tnum>0)       /*如果trunk存在*/
	          { 
	            if(strcmp(lan,"ch")==0)
				  fprintf(cgiOut,"<table frame=below rules=rows width=393 border=1>");
				else
				  fprintf(cgiOut,"<table frame=below rules=rows width=463 border=1>");
				fprintf(cgiOut,"<tr align=left>"\
				"<th width=90><font id=yingwen_thead>Trunk ID</font></th>"\
                "<th width=110><font id=yingwen_thead>Trunk</font><font id=%s> %s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
				if(strcmp(lan,"ch")==0)
				{
				  fprintf(cgiOut,"<th width=90><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcon,"load_balance"));
				  fprintf(cgiOut,"<th width=90><font id=%s>%s</font><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"master"),search(lpublic,"menu_thead"),search(lpublic,"port"));
				}
				else
				{
				  fprintf(cgiOut,"<th width=130><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcon,"load_balance"));
				  fprintf(cgiOut,"<th width=120><font id=%s>%s</font><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"master"),search(lpublic,"menu_thead"),search(lpublic,"port"));
				}
	            fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
                "</tr>");
				q=head.next;
				for(i=0;i<start_trunkno;i++)
				  q=q->next;
		        for(i=start_trunkno;i<end_trunkno;i++)
		        {		          
				  memset(menu,0,15);
				  strcat(menu,"menuLists");
		          sprintf(menu_id,"%d",i+1); 
		          strcat(menu,menu_id);
                  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
                  fprintf(cgiOut,"<td>%d</td>",q->trunkId);
                  fprintf(cgiOut,"<td>%s</td>",q->trunkName);
				  fprintf(cgiOut,"<td>%s</td>",q->loadBalanc);
				  if(q->masterFlag!=0)
				  	fprintf(cgiOut,"<td>%d/%d</td>",q->mSlotNo,q->mPortNo);
				  else
                    fprintf(cgiOut,"<td>No masterPort</td>");
				  memset(trunk_id,0,10);
				  sprintf(trunk_id,"%d",q->trunkId);     /*int转成char*/
			      fprintf(cgiOut,"<td>"\
				 	              "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(tnum-i),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=div1>");
								   if(retu==0)  /*管理员*/
								   {
								     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_trunknew.cgi?UN=%s target=mainFrame>%s</a></div>",m,search(lpublic,"create"));
									 if((n==((tnum-1)/MAX_PAGE_NUM))&&(((tnum-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_trunklis.cgi?UN=%s&TrunkID=%s&PN=%d&DeletTrunk=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,trunk_id,n-1,"true",search(lpublic,"confirm_delete"),search(lpublic,"delete"));
									 else
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_trunklis.cgi?UN=%s&TrunkID=%s&PN=%d&DeletTrunk=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,trunk_id,n,"true",search(lpublic,"confirm_delete"),search(lpublic,"delete"));
									 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_trunkcon.cgi?UN=%s&ID=%s&PN=%d target=mainFrame>%s</a></div>",m,trunk_id,n,search(lpublic,"configure"));
								   }
	                               fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_trunkdta.cgi?UN=%s&ID=%s&PN=%d target=mainFrame>%s</a></div>",m,trunk_id,n,search(lpublic,"details"));
                                   fprintf(cgiOut,"</div>"\
                                   "</div>"\
                                   "</div>"\
				 	"</td></tr>");
  	             cl=!cl;
		         q=q->next;
		        }				
				fprintf(cgiOut,"</table>");
	          }
	          else				 /*no wlan exist*/
		        fprintf(cgiOut,"%s",search(lcon,"no_trunk"));
			  fprintf(cgiOut,"</td></tr>");
			  if(tnum>MAX_PAGE_NUM)               /*大于MAX_PAGE_NUM个trunk时，显示翻页的链接*/
			  {
			    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
				if(n!=0)          /*如果不是首页*/
			      fprintf(cgiOut,"<td align=left><a href=wp_trunklis.cgi?UN=%s&PN=%d target=mainFrame>%s</a></td>",m,n-1,search(lpublic,"up_page"));
				if(n!=((tnum-1)/MAX_PAGE_NUM))      /*如果不是最后一页*/
			      fprintf(cgiOut,"<td align=right><a href=wp_trunklis.cgi?UN=%s&PN=%d target=mainFrame>%s</a></td>",m,n+1,search(lpublic,"down_page"));
			    fprintf(cgiOut,"</tr>");
			  }
              fprintf(cgiOut,"</table>");
	        }
	        else if(result == 0)
              fprintf(cgiOut,"%s",search(lpublic,"contact_adm"));	   
			else if(result == -1)
			  fprintf(cgiOut,"%s",search(lcon,"no_trunk"));	
			else
			  fprintf(cgiOut,"%s",search(lpublic,"error"));	
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
free(IsDeleete);
free(trunk_id);
free(menu_id);
free(menu);
if((result==1)&&(tnum>0))
  Free_trunk_head(&head);
}


void DeleteTrunk(char *ID,struct list *lpublic,struct list *lcon)
{
  char *endptr = NULL;  
  int trunkid,ret;  
  ccgi_dbus_init();
  trunkid=strtoul(ID,&endptr,10);  /*char转成int*/ 
  ret=delete_trunk_byid(trunkid);/*返回0表示 删除失败，返回1表示删除成功，返回-1表示Trunk id Illegal，返回-2表示trunk not exists，返回-3表示出错*/
  switch(ret)
  {
	case 0:ShowAlert(search(lcon,"delete_trunk_fail"));
		   break;
	case 1:ShowAlert(search(lcon,"delete_trunk_succ"));
		   break;
	case -1:ShowAlert(search(lcon,"trunk_id_illegal"));
   		    break;
	case -2:ShowAlert(search(lcon,"trunk_not_exist"));
			break;
	case -3:ShowAlert(search(lpublic,"error"));
			break;
  }  	
}

