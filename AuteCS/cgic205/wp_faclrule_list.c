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
* wp_faclrule_list.c
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

#define MAX_PAGE_NUM 20     /*每页显示的最多FACL Rule个数*/ 

void ShowFaclRuleListPage(char *m,char *n,int facl_index,int p,struct list *lpublic,struct list *lfirewall);    
void DeleteFaclRule(int facl_index,char *rule_index,struct list *lpublic,struct list *lfirewall);
char *facl_proto_to_str(int proto);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };
  char page_no[5] = { 0 };
  char ID[5] = { 0 };
  char *str = NULL;    
  char *endptr = NULL;  
  int pno = 0;
  int facl_index = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lfirewall = NULL; /*解析firewall.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lfirewall=get_chain_head("../htdocs/text/firewall.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(page_no,0,sizeof(page_no));
  memset(ID,0,sizeof(ID));
  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
  cgiFormStringNoNewlines("FaclIndex", ID, 5);
  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
  {
    facl_index = strtoul(ID,&endptr,10);	  /*char转成int，10代表十进制*/ 
    if(cgiFormStringNoNewlines("PN", page_no, 5)!=cgiFormNotFound )  /*点击翻页进入该页面*/
	{
	  pno= strtoul(page_no,&endptr,10);	/*char转成int，10代表十进制*/ 
      ShowFaclRuleListPage(encry,str,facl_index,pno,lpublic,lfirewall);
	}
	else
	  ShowFaclRuleListPage(encry,str,facl_index,0,lpublic,lfirewall);
  }  
  release(lpublic);  
  release(lfirewall);
  destroy_ccgi_dbus();
  return 0;
}

void ShowFaclRuleListPage(char *m,char *n,int facl_index,int p,struct list *lpublic,struct list *lfirewall)
{    
  char IsDelete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  struct list_head rule_head = { 0 };
  facl_rule_t *rule = NULL;
  char rule_index[10] = { 0 };    
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  int rnum = 0;             /*存放facl rule的个数*/
  int i = 0,j = 0,result = 0,retu = 1,cl = 1,limit = 0;                        /*颜色初值为#f9fafe*/
  int start_ruleno = 0,end_ruleno = 0,ruleno_page = 0,total_pnum = 0;    /*start_ruleno表示要显示的起始facl rule index，end_ruleno表示要显示的结束facl rule index，ruleno_page表示本页要显示的facl rule数，total_pnum表示总页数*/

  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>FACL Rule</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
"</head>"\
	  "<script type=\"text/javascript\">"\
	  "function page_change(obj)"\
	  "{"\
	     "var page_num = obj.options[obj.selectedIndex].value;"\
	   	 "var url = 'wp_faclrule_list.cgi?UN=%s&PN='+page_num+'&FaclIndex=%d';"\
	   	 "window.location.href = url;"\
	   	"}", m , facl_index);
	  fprintf(cgiOut,"</script>"\
  "<body>");
  memset(IsDelete,0,sizeof(IsDelete));
  cgiFormStringNoNewlines("DeletRule", IsDelete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDelete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    memset(rule_index,0,sizeof(rule_index));
    cgiFormStringNoNewlines("RuleIndex", rule_index, 10);
	DeleteFaclRule(facl_index,rule_index,lpublic,lfirewall);
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
          "<td width=62 align=center><a href=wp_facl_list.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_ok"));
		  fprintf(cgiOut,"<td width=62 align=center><a href=wp_facl_list.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",m,search(lpublic,"img_cancel"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lfirewall,"add_rule"));   /*突出显示*/
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
				  memset(&rule_head, 0, sizeof(rule_head));
				  INIT_LIST_HEAD(&rule_head);
				  result=facl_interface_show_rule(ccgi_dbus_connection, facl_index, &rule_head);
				  if(result == FACL_RETURN_OK)
				  {
				  	list_for_each_entry(rule, &rule_head, node)
					{
						rnum++;
					}
				  }
				  
				  total_pnum=((rnum%MAX_PAGE_NUM)==0)?(rnum/MAX_PAGE_NUM):((rnum/MAX_PAGE_NUM)+1);
 				  start_ruleno=p*MAX_PAGE_NUM;   
				  end_ruleno=(((p+1)*MAX_PAGE_NUM)>rnum)?rnum:((p+1)*MAX_PAGE_NUM);
				  ruleno_page=end_ruleno-start_ruleno;
				  if((ruleno_page<(MAX_PAGE_NUM/2))||(rnum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个facl*/
				  	limit=6;
				  else if((ruleno_page<MAX_PAGE_NUM)||(rnum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个facle*/
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
              "<table width=780 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
   "<tr>"\
    "<td valign=top align=center style=\"padding-top:5px; padding-bottom:10px\">");
	if(result == FACL_RETURN_OK)
	{ 
	  fprintf(cgiOut,"<table width=780 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=3>");
	  if(rnum>0)           /*如果FACL存在*/
	  {		
		fprintf(cgiOut,"<table frame=below rules=rows width=780 border=1>"\
		"<tr align=left>"\
        "<th width=45><font id=yingwen_thead>Index</font></th>"\
        "<th width=45><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"action_type"));
		fprintf(cgiOut,"<th width=70><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"input_intf"));
		fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"output_intf"));
		fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"source_ip"));
		fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"dest_ip"));
		fprintf(cgiOut,"<th width=40><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"tc_protocol"));
		fprintf(cgiOut,"<th width=55><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"source_port"));
		fprintf(cgiOut,"<th width=75><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"dest_port"));
		fprintf(cgiOut,"<th width=130><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lfirewall,"domain"));
		fprintf(cgiOut,"<th width=40><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"version_opt"));
        fprintf(cgiOut,"</tr>");
		i = 0;
		j = 0;
		list_for_each_entry(rule, &rule_head, node)
		{
			i++;
			if((rule)&&(i > start_ruleno))
			{				
				memset(menu,0,sizeof(menu));
				strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
				snprintf(menu_id,sizeof(menu_id)-1,"%d",j+1); 
				strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
				fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
				fprintf(cgiOut,"<td>%u</td>",rule->index);
				fprintf(cgiOut,"<td>%s</td>",FACL_RULE_TYPE_PERMIT==rule->data.type?"permit":"deny");
				fprintf(cgiOut,"<td>%s</td>",rule->data.inif);
				fprintf(cgiOut,"<td>%s</td>",rule->data.outif);
				fprintf(cgiOut,"<td>%s</td>",rule->data.srcip);
				fprintf(cgiOut,"<td>%s</td>",rule->data.dstip);
				fprintf(cgiOut,"<td>%s</td>",facl_proto_to_str(rule->data.proto));
				fprintf(cgiOut,"<td>%s</td>",rule->data.srcport);
				fprintf(cgiOut,"<td>%s</td>",rule->data.dstport);
				fprintf(cgiOut,"<td>%s</td>",rule->data.domain);
				fprintf(cgiOut,"<td>");
					if(retu==0)  /*管理员*/
					{
						if((p>0)&&(p==((rnum-1)/MAX_PAGE_NUM))&&(((rnum-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
							fprintf(cgiOut,"<a href=wp_faclrule_list.cgi?UN=%s&FaclIndex=%d&RuleIndex=%d&DeletRule=%s&PN=%d&SubmitFlag=1 target=mainFrame>%s</a>",m,facl_index,rule->index,"true",p-1,search(lpublic,"delete"));
						else
							fprintf(cgiOut,"<a href=wp_faclrule_list.cgi?UN=%s&FaclIndex=%d&RuleIndex=%d&DeletRule=%s&PN=%d&SubmitFlag=1 target=mainFrame>%s</a>",m,facl_index,rule->index,"true",p,search(lpublic,"delete"));
					}
				fprintf(cgiOut,"</td></tr>");
				cl=!cl;
				j++;
				if(j == (end_ruleno-start_ruleno))
				{
					break;
				}
			}
		}
		fprintf(cgiOut,"</table>");
	  }
	  else				 /*no wlan exist*/
		fprintf(cgiOut,"%s",search(lfirewall,"no_rule"));
	  fprintf(cgiOut,"</td></tr>");
	  if(rnum>MAX_PAGE_NUM)               /*大于30个facl时，显示翻页的链接*/
	  {
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(p!=0) 
	      fprintf(cgiOut,"<td align=left width=100><a href=wp_faclrule_list.cgi?UN=%s&PN=%d&FaclIndex=%d target=mainFrame>%s</a></td>",m,p-1,facl_index,search(lpublic,"up_page"));
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
		if(p!=((rnum-1)/MAX_PAGE_NUM))
	      fprintf(cgiOut,"<td align=right width=100><a href=wp_faclrule_list.cgi?UN=%s&PN=%d&FaclIndex=%d target=mainFrame>%s</a></td>",m,p+1,facl_index,search(lpublic,"down_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"</tr>");
	  }
      fprintf(cgiOut,"</table>");
	}
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
facl_interface_free_rule(&rule_head);
}

void DeleteFaclRule(int facl_index,char *rule_index,struct list *lpublic,struct list *lfirewall)
{
	char *endptr = NULL; 
	int index = 0,ret = 0;

	index=strtoul(rule_index,&endptr,10);  /*char转成int*/ 
	ret=facl_interface_del_rule(ccgi_dbus_connection, facl_index, index);
	if(0 != ret)
	{
    	ShowAlert(search(lpublic,"error"));
	}
}

char *facl_proto_to_str(int proto)
{
	switch (proto) {
	case FACL_ICMP:
		return "icmp";
	case FACL_TCP:
		return "tcp";
	case FACL_UDP:
		return "udp";
	case FACL_PRANY:
	default:
		return "any";
	}
}

