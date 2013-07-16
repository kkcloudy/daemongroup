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
* wp_configvlan.c
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
#include "ws_ec.h"
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_dcli_wlans.h"
#include "ws_dbus_list_interface.h"
#include "ws_dcli_vlan.h"
#include "ws_returncode.h"

#define MAX_PAGE_NUM 20     /*每页显示的最多VLAN个数*/ 

void ShowVlanListPage(char *m,char *n,int p,struct list *lpublic,struct list *lcontrol);    
void DeleteVlan(char *ID,struct list *lpublic,struct list *lcontrol);

int cgiMain()
{
  char encry[BUF_LEN] = { 0 };
  char page_no[5] = { 0 };
  char *str = NULL;    
  char *endptr = NULL;  
  int pno = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lcontrol = NULL;     /*解析wlan.txt文件的链表头*/  
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcontrol=get_chain_head("../htdocs/text/control.txt");
  
  DcliWInit();
  ccgi_dbus_init();
  memset(encry,0,sizeof(encry));
  memset(page_no,0,sizeof(page_no));

  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页*/
  {
  	;
  }
  else
  {
  	cgiFormStringNoNewlines("encry_convlan", encry, BUF_LEN);
  }
  cgiFormStringNoNewlines("PN",page_no,10);
  pno= strtoul(page_no,&endptr,10); /*char转成int，10代表十进制*/ 

  str=dcryption(encry);
  if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
  else
  {
    ShowVlanListPage(encry,str,pno,lpublic,lcontrol);
  }
  release(lpublic);  
  release(lcontrol);
  destroy_ccgi_dbus();
  return 0;
}

void ShowVlanListPage(char *m,char *n,int p,struct list *lpublic,struct list *lcontrol)
{    
  char IsDeleete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  struct vlan_info_detail head,*q = NULL;
  char vlan_id[10] = { 0 };    
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  int vlan_num = 0;
  int i = 0,result = 0,retu = 1,cl = 1,limit = 0;                        /*颜色初值为#f9fafe*/
  int start_vlanno = 0,end_vlanno = 0,vlanno_page = 0,total_pnum = 0;    /*start_vlanno表示要显示的起始vlan id，end_vlanno表示要显示的结束vlan id，vlanno_page表示本页要显示的vlan数，total_pnum表示总页数*/
  int egress_status = 0;
  int ret = 0;
  char sub_text[10] = { 0 };
  char search_text[30] = { 0 };
  char *endptr = NULL; 
  int temp_vid = 0;
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>Vlan</title>");
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
	   	 "var url = 'wp_configvlan.cgi?UN=%s&PN='+page_num;"\
	   	 "window.location.href = url;"\
	   	"}", m);
	  fprintf(cgiOut,"</script>"\
  "<body>");
  memset(IsDeleete,0,sizeof(IsDeleete));
  cgiFormStringNoNewlines("DeletVlan", IsDeleete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDeleete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    memset(vlan_id,0,sizeof(vlan_id));
    cgiFormStringNoNewlines("VlanID", vlan_id, 10);
	DeleteVlan(vlan_id,lpublic,lcontrol);
  }

  egress_status = show_vlan_egress_filter();/*return 1:enable; return 2:disable*/
  if((cgiFormSubmitClicked("submit_egress_filter") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
  {
	if(egress_status == 1)
	{
		ret = config_vlan_egress_filter("disable");
		switch(ret)
		{
			case 0:ShowAlert(search(lcontrol,"opt_fail"));
				   break;
			case 1:ShowAlert(search(lcontrol,"opt_succ"));
				   break;
			case -1:ShowAlert(search(lpublic,"input_para_illegal"));
					break;
			default:ShowAlert(search(lpublic,"error"));
					break;
		}
	}
	else if(egress_status == 2)
	{
		ret = config_vlan_egress_filter("enable");
		switch(ret)
		{
			case 0:ShowAlert(search(lcontrol,"opt_fail"));
				   break;
			case 1:ShowAlert(search(lcontrol,"opt_succ"));
				   break;
			case -1:ShowAlert(search(lpublic,"input_para_illegal"));
					break;
			default:ShowAlert(search(lpublic,"error"));
					break;
		}
	}
  }

  memset(sub_text,0,sizeof(sub_text));
  cgiFormStringNoNewlines("search_vlan",sub_text,10);
  if(0 == strcmp(sub_text,""))
  {
  	strcpy(sub_text, "VID");
  }  
  memset(search_text,0,sizeof(search_text));
  cgiFormStringNoNewlines("search_text",search_text,30);
  
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>VLAN </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcontrol,"config"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
				  if(retu==0)  /*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_addvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> VLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lcontrol,"add"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  	fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_show_pvlan.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>PVLAN </font><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lcontrol,"list"));		  
					fprintf(cgiOut,"</tr>");
				  if(retu==0)  /*管理员*/
				  {
					fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_config_pvlan.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> PVLAN</font></a></td>",m,search(lpublic,"menu_san"),search(lcontrol,"pvlan_add"));                       
                    fprintf(cgiOut,"</tr>");
				  }
				  result=show_vlan_list_slot(&head, &vlan_num);

				  total_pnum=((vlan_num%MAX_PAGE_NUM)==0)?(vlan_num/MAX_PAGE_NUM):((vlan_num/MAX_PAGE_NUM)+1);
 				  start_vlanno=p*MAX_PAGE_NUM;   
				  end_vlanno=(((p+1)*MAX_PAGE_NUM)>vlan_num)?vlan_num:((p+1)*MAX_PAGE_NUM);
				  vlanno_page=end_vlanno-start_vlanno;

				  if((vlanno_page<(MAX_PAGE_NUM/2))||(vlan_num==(MAX_PAGE_NUM/2)))   /*该页显示1--9个或者一共有10个wtp*/
				  	limit=8;
				  else if((vlanno_page<MAX_PAGE_NUM)||(vlan_num==MAX_PAGE_NUM))  /*该页显示10--19个或者一共有20个wtp*/
		  	     	limit=16;
		       	  else         /*大于20个翻页*/
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
              "<table width=500 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
    "<tr>"\
		"<td id=sec1 colspan=4 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcontrol,"Vlan_info"));
	fprintf(cgiOut,"</tr>"\
	"<tr height=25 padding-top:10px  align=left>"\
		"<td colspan=2>%s: </td>", search(lcontrol,"cur_filter_status"));
		egress_status = show_vlan_egress_filter();
		if(egress_status == 2)
		{
			fprintf(cgiOut,"<td>disabled</td>");
		}
		else if(egress_status == 1)
		{
			fprintf(cgiOut,"<td>enabled</td>");
		}
		
		if(egress_status == 2)
		{
			fprintf(cgiOut,"<td><input type=submit name=submit_egress_filter style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>", search(lcontrol,"enable"));
		}
		else if(egress_status == 1)
		{
			fprintf(cgiOut,"<td><input type=submit name=submit_egress_filter style=width:60px; height:36px border=0 style=background-image:url(/images/SubBackGif.gif)  value=%s></td>", search(lcontrol,"disable"));
		}
	fprintf(cgiOut,"</tr>"\
	"<tr height=25 padding-top:10px  align=left>");
		if(strcmp(sub_text,"VNAME")==0)
		{
			fprintf(cgiOut,"<td width=40 align=right><select name=search_vlan>"\
				"<option value=VNAME>Vlan Name</option>"\
				"<option value=VID>VlanID</option></select>"\
				"</td>");
		}
		else
		{
			fprintf(cgiOut,"<td width=40 align=right><select name=search_vlan>"\
				"<option value=VID>VlanID</option>"\
				"<option value=VNAME>Vlan Name</option></select>"\
				"</td>");
		}
		if((cgiFormSubmitClicked("submit_search") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
		{
			fprintf(cgiOut,"<td width=60><input type=text name=search_text size=12 value=%s></td>",search_text);
		}
		else
		{
			fprintf(cgiOut,"<td width=60><input type=text name=search_text size=12></td>");
		}	
		fprintf(cgiOut,"<td width=60 style=padding-left:5px><input type=submit name=submit_search style=width:60px; height:36px border=0 name=addIP style=background-image:url(/images/SubBackGif.gif) value=%s></td>",search(lcontrol,"search_vlan"));
		fprintf(cgiOut,"<td width=340 style=padding-left:5px><input type=submit name=submit_ret style=width:60px; height:36px border=0 name=addIP style=background-image:url(/images/SubBackGif.gif)  value=%s></td>",search(lpublic,"return"));
	fprintf(cgiOut,"</tr>"\
    "<tr>"\
    "<td colspan=4 valign=top align=left style=\"padding-top:10px; padding-bottom:10px;\">");
	if(result == 1)
	{ 
	  fprintf(cgiOut,"<table width=363 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=3>");
	  if(vlan_num>0)           /*如果VLAN存在*/
	  {		
		fprintf(cgiOut,"<table frame=below rules=rows width=363 border=1>"\
		"<tr align=left>"\
        "<th width=70><font id=yingwen_thead>VLAN ID</font></th>"\
        "<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"vlan_name"));
	    fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"vlan_stat"));
		fprintf(cgiOut,"<th width=80><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lcontrol,"port_num"));
        fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
        "</tr>");
		if((cgiFormSubmitClicked("submit_search") == cgiFormSuccess)&&(strcmp(IsSubmit,"")))
		{	
			if(strcmp(search_text,""))
			{				
				for(q=head.next;q;q=q->next)
				{
					if(strcmp(sub_text,"VNAME")==0)/*search by vlan name*/
					{
						if(0 == strcmp(q->vlanName,search_text))
						{
							memset(menu,0,sizeof(menu));
							strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
							snprintf(menu_id,sizeof(menu_id)-1,"%d",q->vlanId); 
							strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
							
							fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(1));
							  fprintf(cgiOut,"<td>%d</td>",q->vlanId);
							  fprintf(cgiOut,"<td>%s</td>",q->vlanName);
							  fprintf(cgiOut,"<td>%s</td>",q->updown ? "UP":"DOWN");
							  fprintf(cgiOut,"<td>%d</td>",(q->untagnum+q->tagnum));	
							  fprintf(cgiOut,"<td>"\
													   "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(vlan_num-q->vlanId),menu,menu);
													   fprintf(cgiOut,"<img src=/images/detail.gif>"\
													   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
													   fprintf(cgiOut,"<div id=div1>");
													   if(retu==0)	/*管理员*/
													   {
														 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlandetail.cgi?UN=%s&VID=%d&SetVlan=%s target=mainFrame>%s</a></div>",m,q->vlanId,"NoSet",search(lpublic,"configure"));	
														 if(1 == q->vlanId)
															fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_portconfig.cgi?UN=%s&VID=%d target=mainFrame>%s</a></div>",m,q->vlanId,search(lcontrol,"port_show"));
														 else
															fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_portconfig.cgi?UN=%s&VID=%d target=mainFrame>%s</a></div>",m,q->vlanId,search(lcontrol,"port_configure"));
														 if(1 != q->vlanId)
														 {
															 if((p>0)&&(p==((vlan_num-1)/MAX_PAGE_NUM))&&(((vlan_num-1)%MAX_PAGE_NUM)==0))	/*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
															   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VlanID=%d&DeletVlan=%s&PN=%d&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,q->vlanId,"true",p-1,search(lpublic,"confirm_delete"),search(lpublic,"delete"));							  
															 else
															   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VlanID=%d&DeletVlan=%s&PN=%d&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,q->vlanId,"true",p,search(lpublic,"confirm_delete"),search(lpublic,"delete")); 							
														 }
													   }
													   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%d&VNAME=%s target=mainFrame>%s</a></div>",m,q->vlanId,q->vlanName,search(lpublic,"details"));
													   fprintf(cgiOut,"</div>"\
													   "</div>"\
													   "</div>"\
							"</td></tr>");
							break;
						}
					}
					else if(strcmp(sub_text,"VID")==0)/*search by vlan id*/
					{
						temp_vid=strtoul(search_text,&endptr,10);
						if(temp_vid == q->vlanId)
						{
							memset(menu,0,sizeof(menu));
							strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
							snprintf(menu_id,sizeof(menu_id)-1,"%d",q->vlanId); 
							strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);

							fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(1));
							  fprintf(cgiOut,"<td>%d</td>",q->vlanId);
							  fprintf(cgiOut,"<td>%s</td>",q->vlanName);
							  fprintf(cgiOut,"<td>%s</td>",q->updown ? "UP":"DOWN");
							  fprintf(cgiOut,"<td>%d</td>",(q->untagnum+q->tagnum));	
							  fprintf(cgiOut,"<td>"\
													   "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(vlan_num-q->vlanId),menu,menu);
													   fprintf(cgiOut,"<img src=/images/detail.gif>"\
													   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
													   fprintf(cgiOut,"<div id=div1>");
													   if(retu==0)	/*管理员*/
													   {
														 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlandetail.cgi?UN=%s&VID=%d&SetVlan=%s target=mainFrame>%s</a></div>",m,q->vlanId,"NoSet",search(lpublic,"configure"));	
														 if(1 == q->vlanId)
															fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_portconfig.cgi?UN=%s&VID=%d target=mainFrame>%s</a></div>",m,q->vlanId,search(lcontrol,"port_show"));
														 else
															fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_portconfig.cgi?UN=%s&VID=%d target=mainFrame>%s</a></div>",m,q->vlanId,search(lcontrol,"port_configure"));
														 if(1 != q->vlanId)
														 {
															 if((p>0)&&(p==((vlan_num-1)/MAX_PAGE_NUM))&&(((vlan_num-1)%MAX_PAGE_NUM)==0))	/*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
															   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VlanID=%d&DeletVlan=%s&PN=%d&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,q->vlanId,"true",p-1,search(lpublic,"confirm_delete"),search(lpublic,"delete"));							  
															 else
															   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VlanID=%d&DeletVlan=%s&PN=%d&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,q->vlanId,"true",p,search(lpublic,"confirm_delete"),search(lpublic,"delete")); 							
														 }
													   }
													   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%d&VNAME=%s target=mainFrame>%s</a></div>",m,q->vlanId,q->vlanName,search(lpublic,"details"));
													   fprintf(cgiOut,"</div>"\
													   "</div>"\
													   "</div>"\
							"</td></tr>");
							break;
						}
					}
				}
			}
		}
		else
		{
			q=head.next;
			for(i=0;i<start_vlanno;i++)
			{
				if(q)
				{
					q=q->next;
				}
			}
			for(i=start_vlanno;((i<end_vlanno)&&q);i++)
			{
			  memset(menu,0,sizeof(menu));
			  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
			  snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
			  strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
			  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
			  fprintf(cgiOut,"<td>%d</td>",q->vlanId);
			  fprintf(cgiOut,"<td>%s</td>",q->vlanName);
			  fprintf(cgiOut,"<td>%s</td>",q->updown ? "UP":"DOWN");
			  fprintf(cgiOut,"<td>%d</td>",(q->untagnum+q->tagnum));	
			  fprintf(cgiOut,"<td>"\
									   "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(vlan_num-i),menu,menu);
									   fprintf(cgiOut,"<img src=/images/detail.gif>"\
									   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
									   fprintf(cgiOut,"<div id=div1>");
									   if(retu==0)	/*管理员*/
									   {
										 fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlandetail.cgi?UN=%s&VID=%d&SetVlan=%s target=mainFrame>%s</a></div>",m,q->vlanId,"NoSet",search(lpublic,"configure"));	
										 if(1 == q->vlanId)
											fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_portconfig.cgi?UN=%s&VID=%d target=mainFrame>%s</a></div>",m,q->vlanId,search(lcontrol,"port_show"));
										 else
											fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_portconfig.cgi?UN=%s&VID=%d target=mainFrame>%s</a></div>",m,q->vlanId,search(lcontrol,"port_configure"));
										 if(1 != q->vlanId)
										 {
											 if((p>0)&&(p==((vlan_num-1)/MAX_PAGE_NUM))&&(((vlan_num-1)%MAX_PAGE_NUM)==0))	/*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
											   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VlanID=%d&DeletVlan=%s&PN=%d&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,q->vlanId,"true",p-1,search(lpublic,"confirm_delete"),search(lpublic,"delete"));							  
											 else
											   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_configvlan.cgi?UN=%s&VlanID=%d&DeletVlan=%s&PN=%d&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,q->vlanId,"true",p,search(lpublic,"confirm_delete"),search(lpublic,"delete")); 							
										 }
									   }
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_vlanInfo.cgi?UN=%s&VID=%d&VNAME=%s target=mainFrame>%s</a></div>",m,q->vlanId,q->vlanName,search(lpublic,"details"));
									   fprintf(cgiOut,"</div>"\
									   "</div>"\
									   "</div>"\
			  "</td></tr>");
			  cl=!cl;
			  if(q)
			  {
				 q=q->next;
			  }
			}	
		}
		fprintf(cgiOut,"</table>");
	  }
	  else				 /*no vlan exist*/
		fprintf(cgiOut,"%s",search(lcontrol,"no_vlan"));
	  fprintf(cgiOut,"</td></tr>");
	  if(vlan_num>MAX_PAGE_NUM)               /*大于10个vlan时，显示翻页的链接*/
	  {
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(p!=0) 
	      fprintf(cgiOut,"<td align=left width=100><a href=wp_configvlan.cgi?UN=%s&PN=%d target=mainFrame>%s</a></td>",m,p-1,search(lpublic,"up_page"));
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
		if(p!=((vlan_num-1)/MAX_PAGE_NUM))
	      fprintf(cgiOut,"<td align=right width=100><a href=wp_configvlan.cgi?UN=%s&PN=%d target=mainFrame>%s</a></td>",m,p+1,search(lpublic,"down_page"));
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
  "<tr>"\
    "<td><input type=hidden name=encry_convlan value=%s></td>",m);
	fprintf(cgiOut,"<td><input type=hidden name=page_no value=%d></td>",p);
	fprintf(cgiOut,"<td colspan=2><input type=hidden name=SubmitFlag value=%d></td>",1);
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
Free_show_vlan_list_slot(&head);
}

void DeleteVlan(char *ID,struct list *lpublic,struct list *lcontrol)
{
	char *endptr = NULL; 
	int vlanid = 0,ret = 0;

	vlanid=strtoul(ID,&endptr,10);  /*char转成int*/ 
	deleteIntfForVlanNoShow(vlanid);
	instance_parameter *paraHead2 = NULL;
	instance_parameter *pq = NULL;
	list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	for(pq=paraHead2;(NULL != pq);pq=pq->next)
	{
		ret = delete_vlan(pq->connection,vlanid);
	}
	free_instance_parameter_list(&paraHead2);

	switch(ret)
	{
		case -2:ShowAlert(search(lcontrol,"illegal_vID"));
		        break;
		case 0:ShowAlert(search(lcontrol,"opt_fail"));
		       break;
		case -4:ShowAlert(search(lcontrol,"vID_NotExist"));
				break;
		case -5:ShowAlert(search(lcontrol,"Default_delete_error"));
				break;
		case -6:ShowAlert(search(lcontrol,"HW_error"));
				break;
		case 1:ShowAlert(search(lcontrol,"delete_vlan_success"));
			   break;
		case -8:ShowAlert(search(lcontrol,"s_arp_unknow_err"));
				break;
		case -9:ShowAlert(search(lcontrol,"unbond_vlan_first"));
				break;
		default:ShowAlert(search(lcontrol,"opt_fail"));
				break;
	}
}

