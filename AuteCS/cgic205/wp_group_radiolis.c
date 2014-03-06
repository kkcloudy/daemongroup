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
* wp_wtplis.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* hupx@autelan.com
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
#include "ws_dcli_ap_group.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

#define MAX_PAGE_NUM 1     /*每页显示的最多Radio个数*/   


void ShowWtpGroupradioListPage(char *m, char *t, struct list *lpublic, struct list *lwlan);    

int cgiMain()
{
	  char encry[BUF_LEN] = { 0 };  
	  char *str = NULL;      
	  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
	  struct list *lwlan = NULL;     /*解析wlan.txt文件的链表头*/  
	  lpublic=get_chain_head("../htdocs/text/public.txt");
	  lwlan=get_chain_head("../htdocs/text/wlan.txt");
	  int pno = 0;
	  char page_no[5] = { 0 };   
	  
	  ccgi_dbus_init();
	  memset(encry,0,sizeof(encry));
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
	  str=dcryption(encry);
	  if(str==NULL)
	  {
	    	ShowErrorPage(search(lpublic,"ill_user"));
	  }/*用户非法*/
	  else
	  {        
		ShowWtpGroupradioListPage(encry,str,lpublic,lwlan);	
	  }
	  release(lpublic);  
	  release(lwlan);
	  destroy_ccgi_dbus();
	  return 0;
}

void ShowWtpGroupradioListPage(char *m, char *t, struct list *lpublic,struct list *lwlan)
{  

  int i = 0,result = 1,retu = 1,cl = 1;                        //颜色初值为#f9fafe  
  unsigned int rnum= 0;
  char select_insid[10] = { 0 };
  instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
  instance_parameter *pq = NULL;
  dbus_parameter ins_para;
  char *endptr = NULL;	
  struct ap_group_list head,*q = NULL, *q_tail = NULL;
  char temp[10] = { 0 };
  char menu[15] = { 0 };
  char page_no[5] = { 0 };   
  char menu_id[10] = { 0 };
  char wtpgroup_id[5] = { 0 };  
  unsigned int  ap_count_ptr=0;
  char for_i = 0;
  int index_i=0;
  int page=0;
  int limit,start_radiono,end_radiono,radiono_page,total_pnum;    /*start_radiono表示要显示的起始radio global id，end_radiono表示要显示的结束radio global id，radiono_page表示本页要显示的radio数，total_pnum表示总页数*/

  memset(select_insid,0,sizeof(select_insid));
  cgiFormStringNoNewlines( "INSTANCE_ID", select_insid, 10 );
  if(cgiFormStringNoNewlines("PN", page_no, 5)!=cgiFormNotFound ) 
  {
       page= strtoul(page_no,&endptr,10);
  }
  else
      page = 0;  


  
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
    "#div1{ width:86px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
    "#div2{ width:84px; height:15px; padding-left:5px; padding-top:3px}"\
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
	  "}");
  
  fprintf(cgiOut,"function page_change(obj)"\
		  "{"\
		     "var page_num = obj.options[obj.selectedIndex].value;"\
			 "var url = 'wp_group_radiolis.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
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

      
		      fprintf(cgiOut,"<tr height=25>"\
			      "<td align=left id=tdleft><a href=wp_radiolis.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>Radio</font><font id=%s> %s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"list"));		       
      			fprintf(cgiOut,"</tr>");
		  retu=checkuser_group(t);
		  if(retu==0)  /*管理员*/
		  {
			  fprintf(cgiOut,"<tr height=25>"\
				  "<td align=left id=tdleft><a href=wp_bssbw.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>MAC </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lwlan,"mac_filter"));			  
			  fprintf(cgiOut,"</tr>");
		}
		fprintf(cgiOut,"<tr height=25>"\
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lwlan,"ap_group_radio_list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");

		  
		  if(paraHead1)
		  {
			  result=ccgi_show_ap_group_cmd(paraHead1->parameter,paraHead1->connection,&head, &ap_count_ptr);
		  }
		  if(result == 0)
		  {
		    	rnum = ap_count_ptr;
		  }
		  total_pnum=((rnum%MAX_PAGE_NUM)==0)?(rnum/MAX_PAGE_NUM):((rnum/MAX_PAGE_NUM)+1);
		  start_radiono=page*MAX_PAGE_NUM;   
		  end_radiono=(((page+1)*MAX_PAGE_NUM)>rnum)?rnum:((page+1)*MAX_PAGE_NUM);
		  radiono_page=end_radiono-start_radiono;
		  
		  if((radiono_page<(MAX_PAGE_NUM/2))||(rnum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个radio*/
			limit=9;
		  else if((radiono_page<MAX_PAGE_NUM)||(rnum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个radio*/
			     limit=19;
		       else	    /*大于30个翻页*/
				 limit=21;
		  
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
	   "<td width=160>%s ID:",search(lpublic,"instance"));
	   fprintf(cgiOut,"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_group_radiolis.cgi\",\"%s\")>",m);
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
	"</tr>");

	fprintf(cgiOut,"<tr align=left>"\
			      "<th width=160><font id=%s>Ap-group-radio ID</font></th>",search(lpublic,"menu_thead"));
		fprintf(cgiOut,"<th width=160><font id=%s>Ap-group %s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
		fprintf(cgiOut,"<th width=160><font id=%s>AP %s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"count"));
		fprintf(cgiOut,"<th  width=160>&nbsp;&nbsp;</th>");

	if((result == 0)&&(paraHead1))
	{
		  q=head.next;
		  for(i=0;i<start_radiono;i++)
		  {
			  if(q)
			  {
				  q=q->next;
			  }
		  }
		  
		  index_i=0;
		  for(i=start_radiono; i<end_radiono; i++)
		  {
		  	 int r_num=0;
		  	 int apgroupid=q->test_id;
			 unsigned int apcount=0;
			 unsigned int *wtp_list = NULL;
			 r_num=ccgi_show_group_member_cmd(paraHead1->parameter,paraHead1->connection,apgroupid,&wtp_list,&apcount);
			 if(r_num == 0)
			 {
				 for(for_i=0;for_i<4;for_i++)
				 {
					 index_i=index_i+1;
					 memset(menu,0,sizeof(menu));
					 strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
					 snprintf(menu_id,sizeof(menu_id)-1,"%d",index_i+1); 
					 strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
					 fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
					 fprintf(cgiOut,"<td  width=160>%d%s%d</td>",q->test_id,"-",for_i);
					 if(q->test_name)
					       fprintf(cgiOut,"<td>%s</td>",q->test_name);
					 fprintf(cgiOut,"<td>%d</td>",apcount);
					 fprintf(cgiOut,"<td  width=160>"\
							     "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(25-index_i),menu,menu);
					      fprintf(cgiOut,"<img src=/images/detail.gif>"\
					      "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
					      fprintf(cgiOut,"<div id=div1>");
					      if(retu==0)  
					      {
						       fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_wtpgroupradiocon.cgi?UN=%s&groupID=%d&groupname=%s&INSTANCE_ID=%s&RADIO_ID=%d&PN=%d target=mainFrame>%s</a></div>",m,q->test_id,q->test_name,select_insid,for_i,page,search(lpublic,"configure"));
					      }
					      fprintf(cgiOut,"</div>"\
					      "</div>"\
					      "</div>"\
						   "</td></tr>");
					       cl=!cl;
				 }
			 }
			 q=q->next;
		  }
	}
	if(rnum>MAX_PAGE_NUM)
	{
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
	    
		if(page!=0)
	      fprintf(cgiOut,"<td align=left width=160><a href=wp_group_radiolis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,page-1,select_insid,search(lpublic,"up_page"));
		else
		  fprintf(cgiOut,"<td width=160>&nbsp;&nbsp;&nbsp;</td>");
		fprintf(cgiOut,"<td align=right width=160>%s",search(lpublic,"jump_to_page1"));
		 fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this)>");
		 for(i=0;i<total_pnum;i++)
		 {
		   if(i==page)
			 fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
		   else
			 fprintf(cgiOut,"<option value=%d>%d",i,i+1);
		 }
		 fprintf(cgiOut,"</select>"\
		 "%s</td>",search(lpublic,"jump_to_page2"));
		 
		 fprintf(cgiOut,"<td width=160>&nbsp;&nbsp;&nbsp;</td>");
				if(page!=((rnum-1)/MAX_PAGE_NUM))
			      fprintf(cgiOut,"<td align=left width=160><a href=wp_group_radiolis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,page+1,select_insid,search(lpublic,"down_page"));
				else
				  fprintf(cgiOut,"<td width=160>&nbsp;</td>");
			    fprintf(cgiOut,"</tr>");
	}


	fprintf(cgiOut,"<tr>"\
	  "<td><input type=hidden name=UN value=%s></td>",m);
	  fprintf(cgiOut,"<td colspan=2><input type=hidden name=INSTANCE_ID value=%s></td>",select_insid);
	   
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
if(result==0)
{
  Free_ccgi_show_ap_group_cmd(head);
}
free_instance_parameter_list(&paraHead1);
}

