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
* wp_ebrlis.c
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
#include "wcpss/wid/WID.h"
#include "ws_dcli_ebr.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


#define MAX_PAGE_NUM 25     /*每页显示的最多EBR个数*/   

void ShowEbrListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan);    
void DeleteEbr(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan);



int cgiMain()
{
  char encry[BUF_LEN] = { 0 };  
  char page_no[5] = { 0 };  
  char *str = NULL;      
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
	  pno= strtoul(page_no,0,10);	/*char转成int，10代表十进制*/ 
      ShowEbrListPage(encry,pno,str,lpublic,lwlan);
	}
	else
	  ShowEbrListPage(encry,0,str,lpublic,lwlan);	

  }

  release(lpublic);  
  release(lwlan);
  destroy_ccgi_dbus();
  return 0;
}

void ShowEbrListPage(char *m,int n,char *t,struct list *lpublic,struct list *lwlan)
{  
	char select_insid[10] = { 0 };
	DCLI_EBR_API_GROUP  *ebrinfo = NULL;
	char IsDeleete[10] = { 0 };
	int ebr_num = 0;
	int i = 0,result = 0,retu = 1,cl = 1;                        /*颜色初值为#f9fafe*/  
	int limit = 0,start_ebrno = 0,end_ebrpno = 0,ebrno_page = 0,total_pnum = 0;    /*start_ebrno表示要显示的起始ebr id，end_ebrno表示要显示的结束ebr id，ebrno_page表示本页要显示的ebr数，total_pnum表示总页数*/
	char ebr_id[10] = { 0 };  
	char menu_id[10] = { 0 };
	char menu[15] = { 0 };
	instance_parameter *paraHead1 = NULL,*paraHead2 = NULL;
	instance_parameter *pq = NULL;
	char temp[10] = { 0 };
	dbus_parameter ins_para;
	
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>EBR</title>");
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
	"function page_change(obj,instanceid)"\
	"{"\
		"var page_num = obj.options[obj.selectedIndex].value;"\
		"var url = 'wp_ebrlis.cgi?UN=%s&INSTANCE_ID='+instanceid+'&PN='+page_num;"\
		"window.location.href = url;"\
	"}", m);	
    fprintf(cgiOut,"</script>\n");
	fprintf(cgiOut,"<script src=/instanceid_onchange.js>"\
  	  "</script>");
	fprintf(cgiOut,"<body>");
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
	
	memset(IsDeleete,0,sizeof(IsDeleete));
	cgiFormStringNoNewlines("DeletEbr", IsDeleete, 10);
	if(strcmp(IsDeleete,"true")==0)
	{
		memset(ebr_id,0,sizeof(ebr_id));
		cgiFormStringNoNewlines("EbrID", ebr_id, 10);
		if(paraHead1)
	    {
			DeleteEbr(paraHead1,ebr_id,lpublic,lwlan);
	    }
	}
	fprintf(cgiOut,"<form>"\
		"<div align=center>"\
		"<table width=976 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
		"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>EBR</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
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
	fprintf(cgiOut,"<tr height=26>"\
		"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>EBR</font><font id=%s> %s</font></td>",search(lpublic,"menu_san"),search(lpublic,"list"));   /*突出显示*/
	fprintf(cgiOut,"</tr>");
	retu=checkuser_group(t);
	if(retu==0)  /*管理员*/
	{
		fprintf(cgiOut,"<tr height=25>"\
		"<td align=left id=tdleft><a href=wp_ebrnew.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> EBR</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"create"));                       
		fprintf(cgiOut,"</tr>");
	}
	if(paraHead1)
    {
		result=show_ethereal_bridge_list(paraHead1->parameter,paraHead1->connection, &ebrinfo);
    }
	if(result==1)
	{
		ebr_num = ebrinfo->ebr_num;
	}

	total_pnum=((ebr_num%MAX_PAGE_NUM)==0)?(ebr_num/MAX_PAGE_NUM):((ebr_num/MAX_PAGE_NUM)+1);
	start_ebrno=n*MAX_PAGE_NUM;   
	end_ebrpno=(((n+1)*MAX_PAGE_NUM)>ebr_num)?ebr_num:((n+1)*MAX_PAGE_NUM);
	ebrno_page=end_ebrpno-start_ebrno;

	if((ebrno_page<(MAX_PAGE_NUM/2))||(ebr_num==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个ebr*/
		limit=10;
	else if((ebrno_page<MAX_PAGE_NUM)||(ebr_num==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个ebr*/
		limit=21;
	else         /*大于30个翻页*/
		limit=22;

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
		"<table width=403 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
	"<tr style=\"padding-bottom:15px\">"\
		"<td width=70>%s ID:</td>",search(lpublic,"instance"));
	    fprintf(cgiOut,"<td width=335>"\
		"<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_ebrlis.cgi\",\"%s\")>",m);
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
		"<td align=center valign=top colspan=2 style=\"padding-top:5px; padding-bottom:10px\">");
	if(result == 1)
	{ 
		fprintf(cgiOut,"<table width=403 border=0 cellspacing=0 cellpadding=0>"\
			"<tr>"\
			"<td align=left colspan=3>");
		if(ebr_num>0)		/*如果ebr存在*/
		{		
		fprintf(cgiOut,"<table frame=below rules=rows width=403 border=1>"\
			"<tr align=left>"\
			"<th width=40><font id=yingwen_thead>ID</font></th>"\
			"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
		fprintf(cgiOut,"<th width=70><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"state"));
		fprintf(cgiOut,"<th width=90><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"isolation"));
		fprintf(cgiOut,"<th width=90><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lwlan,"multicast"));
		fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
			"</tr>");

		for(i=start_ebrno;i<end_ebrpno;i++)
		{		 
			memset(menu,0,sizeof(menu));
			strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
			snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
			strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
			fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
			if((ebrinfo)&&(ebrinfo->EBR[i]))
			{
				fprintf(cgiOut,"<td>%d</td>",ebrinfo->EBR[i]->EBRID);
				if(ebrinfo->EBR[i]->name)
				{
					fprintf(cgiOut,"<td>%s</td>",ebrinfo->EBR[i]->name);
				}
			}
			if((ebrinfo)&&(ebrinfo->EBR[i])&&(ebrinfo->EBR[i]->state==1))
				fprintf(cgiOut,"<td>enable</td>");
			else
				fprintf(cgiOut,"<td>disable</td>");
			if((ebrinfo)&&(ebrinfo->EBR[i])&&(ebrinfo->EBR[i]->isolation_policy==1))
				fprintf(cgiOut,"<td>enable</td>");
			else
				fprintf(cgiOut,"<td>disable</td>");				  
			if((ebrinfo)&&(ebrinfo->EBR[i])&&(ebrinfo->EBR[i]->multicast_isolation_policy==1))
				fprintf(cgiOut,"<td>enable</td>");
			else
				fprintf(cgiOut,"<td>disable</td>");
			memset(ebr_id,0,sizeof(ebr_id));
			if((ebrinfo)&&(ebrinfo->EBR[i]))
			{
				snprintf(ebr_id,sizeof(ebr_id)-1,"%d",ebrinfo->EBR[i]->EBRID);	   /*int转成char*/
			}
			fprintf(cgiOut,"<td>"\
				"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(ebr_num-i),menu,menu);
			fprintf(cgiOut,"<img src=/images/detail.gif>"\
				"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
			fprintf(cgiOut,"<div id=div1>");
			if(retu==0)  /*管理员*/
			{
				fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_ebrnew.cgi?UN=%s target=mainFrame>%s</a></div>",m,search(lpublic,"create"));
				if((n>0)&&(n==((ebr_num-1)/MAX_PAGE_NUM))&&(((ebr_num-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
				  fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_ebrlis.cgi?UN=%s&EbrID=%s&PN=%d&DeletEbr=%s&INSTANCE_ID=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,ebr_id,n-1,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));
				else
				  fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_ebrlis.cgi?UN=%s&EbrID=%s&PN=%d&DeletEbr=%s&INSTANCE_ID=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,ebr_id,n,"true",select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));
				fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_ebrcon.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,ebr_id,n,select_insid,search(lpublic,"configure"));
				fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_ebrmapinter.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>L3%s</a></div>",m,ebr_id,n,select_insid,search(lpublic,"interface")); 
			}
			fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_ebrdta.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,ebr_id,n,select_insid,search(lpublic,"details"));
			fprintf(cgiOut,"</div>"\
				"</div>"\
				"</div>"\
				"</td></tr>");
			cl=!cl;
		}		
		fprintf(cgiOut,"</table>");
		
		}
		else			   /*no wlan exist*/
		  fprintf(cgiOut,"%s",search(lwlan,"no_ebr"));
		fprintf(cgiOut,"</td></tr>");
		if(ebr_num>MAX_PAGE_NUM)               /*大于30个wtp时，显示翻页的链接*/
		{

			fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
			if(n!=0)          /**/
			  fprintf(cgiOut,"<td align=left width=100><a href=wp_ebrlis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n-1,select_insid,search(lpublic,"up_page"));
			else
			  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
			fprintf(cgiOut,"<td align=center width=203>%s",search(lpublic,"jump_to_page1"));
			fprintf(cgiOut,"<select name=page_num id=page_num style=width:50px onchange=page_change(this,'%s')>",select_insid);
			for(i=0;i<total_pnum;i++)
			{
				if(i==n)
					fprintf(cgiOut,"<option value=%d selected=selected>%d",i,i+1);
				else
					fprintf(cgiOut,"<option value=%d>%d",i,i+1);
			}

			fprintf(cgiOut,"</select>"\
			"%s</td>",search(lpublic,"jump_to_page2"));
			if(n!=((ebr_num-1)/MAX_PAGE_NUM))
			  fprintf(cgiOut,"<td align=right width=100><a href=wp_ebrlis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,n+1,select_insid,search(lpublic,"down_page"));
			else
			  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
			fprintf(cgiOut,"</tr>");
		}	
		fprintf(cgiOut,"</table>");
	}	
	else if(result == -1) /*no ebr exist*/
		fprintf(cgiOut,"%s",search(lwlan,"no_ebr"));
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
		Free_ethereal_bridge_head(ebrinfo);
	}
	free_instance_parameter_list(&paraHead1);
}


void DeleteEbr(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lwlan)
{
  char temp[100] = { 0 };
  char ebr_id[10] = { 0 };
  int ret = 0;  

  ret=delete_ethereal_bridge_cmd(ins_para->parameter,ins_para->connection,ID);/*返回0表示失败，返回1表示成功，返回-1表示unknown id format，返回-2表示ebr id should be 1 to EBR_NUM-1*/
																			 /*返回-3表示ebr id does not exist，返回-4表示system cmd error，返回-5表示ebr is enable,please disable it first，返回-6表示error*/		
  switch(ret)
  {
    case SNMPD_CONNECTION_ERROR:
	case 0:ShowAlert(search(lwlan,"delete_ebr_fail"));
	       break;
	case 1:ShowAlert(search(lwlan,"delete_ebr_succ"));
		   break;
	case -1:ShowAlert(search(lpublic,"unknown_id_format"));
   		    break;		   
	case -2:memset(temp,0,sizeof(temp));
			strncpy(temp,search(lwlan,"ebr_id_1"),sizeof(temp)-1);
            memset(ebr_id,0,sizeof(ebr_id));
            snprintf(ebr_id,sizeof(ebr_id)-1,"%d",EBR_NUM-1);
            strncat(temp,ebr_id,sizeof(temp)-strlen(temp)-1);
            strncat(temp,search(lwlan,"ebr_id_2"),sizeof(temp)-strlen(temp)-1);
	  		ShowAlert(temp);
   		    break;			
	case -3:ShowAlert(search(lwlan,"ebr_not_exist"));
			break;			
	case -4:ShowAlert(search(lpublic,"sys_cmd_err"));
			break;
	case -5:ShowAlert(search(lwlan,"dis_ebr"));
			break;
	case -6:ShowAlert(search(lpublic,"error"));
			break;
  }
}


