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
* wp_facl_list.c
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
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_init_dbus.h"
#include "ws_usrinfo.h"
#include "facl/facl_db.h"
#include "facl/facl_errcode.h"

#define MAX_PAGE_NUM 20     /*每页显示的最多FACL个数*/ 

void ShowFaclListPage(char *m,char *n,int p,struct list *lpublic,struct list *lfirewall);    
void DeleteFacl(char *Index,struct list *lpublic);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };
  char page_no[5] = { 0 };
  char *str = NULL;    
  char *endptr = NULL;  
  int pno = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lfirewall = NULL; /*解析firewall.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lfirewall=get_chain_head("../htdocs/text/firewall.txt");
  
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
      ShowFaclListPage(encry,str,pno,lpublic,lfirewall);
	}
	else
	  ShowFaclListPage(encry,str,0,lpublic,lfirewall);	
  }
  release(lpublic);  
  release(lfirewall);
  destroy_ccgi_dbus();
  return 0;
}

void ShowFaclListPage(char *m,char *n,int p,struct list *lpublic,struct list *lfirewall)
{    
  char IsDelete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  struct list_head policy_buf_head = { 0 };
  policy_rule_buf_t *policy_buf = NULL;
  char facl_index[10] = { 0 };    
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  int fnum = 0;             /*存放facl的个数*/
  int i = 0,j = 0,result = 0,retu = 1,cl = 1,limit = 0;                        /*颜色初值为#f9fafe*/
  int start_faclno = 0,end_faclno = 0,faclno_page = 0,total_pnum = 0;    /*start_faclno表示要显示的起始facl index，end_faclno表示要显示的结束facl index，faclno_page表示本页要显示的facl数，total_pnum表示总页数*/

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>FACL</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
"</head>"\
	  "<script type=\"text/javascript\">"\
	  "function page_change(obj)"\
	  "{"\
	     "var page_num = obj.options[obj.selectedIndex].value;"\
	   	 "var url = 'wp_facl_list.cgi?UN=%s&PN='+page_num;"\
	   	 "window.location.href = url;"\
	   	"}", m );
	  fprintf(cgiOut,"</script>"\
  "<body>");
  memset(IsDelete,0,sizeof(IsDelete));
  cgiFormStringNoNewlines("DeletFacl", IsDelete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDelete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    memset(facl_index,0,sizeof(facl_index));
    cgiFormStringNoNewlines("FaclIndex", facl_index, 10);
	DeleteFacl(facl_index,lpublic);
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=titleen background=/images/di22.jpg>FACL</td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	 
    	  fprintf(cgiOut,"<table width=155 border=0 cellspacing=0 cellpadding=0>"\
          "<tr>"\
          "<td width=62 align=center><a href=wp_firewall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_firewall.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>FACL</font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"policy"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_add_facl.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lfirewall,"add_facl"));                       
                    fprintf(cgiOut,"</tr>");
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_add_faclrule.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lfirewall,"tc_addrule"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  memset(&policy_buf_head, 0, sizeof(policy_buf_head));
				  INIT_LIST_HEAD(&policy_buf_head);
				  result=facl_interface_show_running(ccgi_dbus_connection, &policy_buf_head);
				  if(result == 0)
				  {
				  	list_for_each_entry(policy_buf, &policy_buf_head, node)
					{
						fnum++;
					}
				  }

				  total_pnum=((fnum%MAX_PAGE_NUM)==0)?(fnum/MAX_PAGE_NUM):((fnum/MAX_PAGE_NUM)+1);
 				  start_faclno=p*MAX_PAGE_NUM;   
				  end_faclno=(((p+1)*MAX_PAGE_NUM)>fnum)?fnum:((p+1)*MAX_PAGE_NUM);
				  faclno_page=end_faclno-start_faclno;
				  if((faclno_page<(MAX_PAGE_NUM/2))||(fnum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个facl*/
				  	limit=6;
				  else if((faclno_page<MAX_PAGE_NUM)||(fnum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个facle*/
			  	    limit=14;
			      else         /*大于30个翻页*/
				   	limit=18;
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
   "<tr>"\
    "<td valign=top align=center style=\"padding-top:5px; padding-bottom:10px\">");
	if(result == 0)
	{ 
	  fprintf(cgiOut,"<table width=763 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=3>");
	  if(fnum>0)           /*如果FACL存在*/
	  {		
		fprintf(cgiOut,"<table frame=below rules=rows width=763 border=1>"\
		"<tr align=left>"\
        "<th width=100><font id=yingwen_thead>Index</font></th>"\
        "<th width=550><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
		fprintf(cgiOut,"<th width=113><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"version_opt"));
        fprintf(cgiOut,"</tr>");
		i = 0;
		j = 0;
		list_for_each_entry(policy_buf, &policy_buf_head, node)
		{
			i++;
			if((policy_buf)&&(i > start_faclno))
			{
			  memset(menu,0,sizeof(menu));
			  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
			  snprintf(menu_id,sizeof(menu_id)-1,"%d",j+1); 
			  strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
			  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
			  fprintf(cgiOut,"<td>%d</td>",policy_buf->facl_tag);
			  fprintf(cgiOut,"<td>%s</td>",policy_buf->facl_name);
			  fprintf(cgiOut,"<td>"\
			  	"<a href=wp_faclrule_list.cgi?UN=%s&FaclIndex=%d&PN=%d target=mainFrame>%s</a>",m,policy_buf->facl_tag,p,search(lfirewall,"add_rule"));
			    fprintf(cgiOut,"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
			  	if(retu==0)  /*管理员*/
			  	{
					if((p>0)&&(p==((fnum-1)/MAX_PAGE_NUM))&&(((fnum-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
						fprintf(cgiOut,"<a href=wp_facl_list.cgi?UN=%s&FaclIndex=%d&DeletFacl=%s&PN=%d&SubmitFlag=1 target=mainFrame>%s</a>",m,policy_buf->facl_tag,"true",p-1,search(lpublic,"delete"));
					else
						fprintf(cgiOut,"<a href=wp_facl_list.cgi?UN=%s&FaclIndex=%d&DeletFacl=%s&PN=%d&SubmitFlag=1 target=mainFrame>%s</a>",m,policy_buf->facl_tag,"true",p,search(lpublic,"delete"));
			  	}
	          fprintf(cgiOut,"</td></tr>");
			  cl=!cl;
			  j++;
			  if(j == (end_faclno-start_faclno))
			  {
			  	break;
			  }
			}
		}
		fprintf(cgiOut,"</table>");
	  }
	  else				 /*no wlan exist*/
		fprintf(cgiOut,"%s",search(lfirewall,"no_facl"));
	  fprintf(cgiOut,"</td></tr>");
	  if(fnum>MAX_PAGE_NUM)               /*大于30个facl时，显示翻页的链接*/
	  {
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(p!=0) 
	      fprintf(cgiOut,"<td align=left width=100><a href=wp_facl_list.cgi?UN=%s&PN=%d target=mainFrame>%s</a></td>",m,p-1,search(lpublic,"up_page"));
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
		if(p!=((fnum-1)/MAX_PAGE_NUM))
	      fprintf(cgiOut,"<td align=right width=100><a href=wp_facl_list.cgi?UN=%s&PN=%d target=mainFrame>%s</a></td>",m,p+1,search(lpublic,"down_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"</tr>");
	  }
      fprintf(cgiOut,"</table>");
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
facl_interface_free_policy_buf(&policy_buf_head);
}

void DeleteFacl(char *Index,struct list *lpublic)
{
	char *endptr = NULL; 
	int faclIndex = 0,ret = 0;

	faclIndex=strtoul(Index,&endptr,10);  /*char转成int*/ 
	ret=facl_interface_delete_policy(ccgi_dbus_connection,faclIndex);
	if (FACL_RETURN_OK == ret) 
	{
    	;
	}
	else if (FACL_TAG_VALUE_ERR == ret)
	{
    	//vty_out(vty, "facl's tag value limit:%d\n", FACL_TAG_MAX_NUM);
	}
	else if (FACL_POLICY_TAG_ALREADY_EXIST == ret) 
	{
    	//vty_out(vty, "facl's tag already exit\n");
	}
	else if (FACL_POLICY_TAG_NOT_EXIST == ret) 
	{
    	//vty_out(vty, "facl's tag not find\n");
	}
	else 
	{
    	ShowAlert(search(lpublic,"error"));
	}
}
