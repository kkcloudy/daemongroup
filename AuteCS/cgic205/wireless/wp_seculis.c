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
* wp_seculis.c
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
#include "ws_security.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"

#define MAX_PAGE_NUM 25     /*每页显示的最多security个数*/   

void ShowSecurityListPage(char *m,char *n,int p,struct list *lpublic,struct list *lsecu);    
void DeleteSecurity(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lsecu);


int cgiMain()
{
  char encry[BUF_LEN] = { 0 };              
  char *str = NULL;           
  char *endptr = NULL;
  char page_no[5] = { 0 };  
  int pno = 0;
  struct list *lpublic = NULL;   /*解析public.txt文件的链表头*/
  struct list *lsecu = NULL;     /*解析security.txt文件的链表头*/  
  
  DcliWInit();
  ccgi_dbus_init();
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lsecu=get_chain_head("../htdocs/text/security.txt");
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
	  ShowSecurityListPage(encry,str,pno,lpublic,lsecu);
	}
	else
	  ShowSecurityListPage(encry,str,0,lpublic,lsecu);
  }
  release(lpublic);  
  release(lsecu);
  destroy_ccgi_dbus();
  return 0;
}

void ShowSecurityListPage(char *m,char *n,int p,struct list *lpublic,struct list *lsecu)
{   
  char IsDeleete[10] = { 0 };
  char IsSubmit[5] = { 0 };
  struct dcli_security *head = NULL,*q = NULL;          /*存放security信息的链表头*/       
  char security_id[10] = { 0 };  
  char SecurityType[20] = { 0 }; 
  char EncryptionType[20] = { 0 }; 
  char menu_id[10] = { 0 };
  char menu[15] = { 0 };
  int snum = 0;             /*存放security的个数*/
  int i = 0,result = 0,limit = 0,retu = 1,cl = 1;                        /*颜色初值为#f9fafe*/
  int start_secno = 0,end_secno = 0,secno_page = 0,total_pnum = 0;    /*start_secno表示要显示的起始security id，end_secno表示要显示的结束security id，secno_page表示本页要显示的security数，total_pnum表示总页数*/
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
  fprintf(cgiOut,"<title>%s</title>",search(lsecu,"wlan_sec"));
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
	  "function page_change(obj)"\
	  "{"\
	     "var page_num = obj.options[obj.selectedIndex].value;"\
	   	 "var url = 'wp_seculis.cgi?UN=%s&PN='+page_num+'&INSTANCE_ID=%s';"\
	   	 "window.location.href = url;"\
	   	"}", m , select_insid);
	  fprintf(cgiOut,"</script>"\
	  "<script src=/instanceid_onchange.js>"\
	  "</script>"\
  "<body>");
  memset(IsDeleete,0,sizeof(IsDeleete));
  cgiFormStringNoNewlines("DeletSecurity", IsDeleete, 10);
  memset(IsSubmit,0,sizeof(IsSubmit));  
  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
  if((strcmp(IsDeleete,"true")==0)&&(strcmp(IsSubmit,"")))
  {
    memset(security_id,0,sizeof(security_id));
    cgiFormStringNoNewlines("SecuID", security_id, 10);
	if(paraHead1)
	{
		DeleteSecurity(paraHead1,security_id,lpublic,lsecu);
	}
  }
  fprintf(cgiOut,"<form>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsecu,"wlan_sec"));
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
                    "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lsecu,"secur_list"));   /*突出显示*/
                  fprintf(cgiOut,"</tr>");
				  retu=checkuser_group(n);
				  if(retu==0)/*管理员*/
				  {
                    fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secre.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"create_sec"));                       
                    fprintf(cgiOut,"</tr>"\
				    "<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"config_sec"));                       
                    fprintf(cgiOut,"</tr>");
					 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_secscon.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lsecu,"config_secon_sec"));                       
                    fprintf(cgiOut,"</tr>");

					 fprintf(cgiOut,"<tr height=25>"\
					  "<td align=left id=tdleft><a href=wp_cer_dload.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",m,search(lpublic,"menu_san"),search(lpublic,"cer_up"));                       
                    fprintf(cgiOut,"</tr>");
					
				  }
				  if(paraHead1)
				  {
					  result=show_security_list(paraHead1->parameter,paraHead1->connection,&head,&snum);
				  }

				  total_pnum=((snum%MAX_PAGE_NUM)==0)?(snum/MAX_PAGE_NUM):((snum/MAX_PAGE_NUM)+1);
				  start_secno=p*MAX_PAGE_NUM;   
				  end_secno=(((p+1)*MAX_PAGE_NUM)>snum)?snum:((p+1)*MAX_PAGE_NUM);
				  secno_page=end_secno-start_secno;
				  if((secno_page<(MAX_PAGE_NUM/2))||(snum==(MAX_PAGE_NUM/2)))   /*该页显示1--14个或者一共有15个wtp*/
				  	limit=7;
				  else if((secno_page<MAX_PAGE_NUM)||(snum==MAX_PAGE_NUM))  /*该页显示15--29个或者一共有30个wtp*/
			  	    limit=17;
			      else         /*大于30个翻页*/
				    limit=19;
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
              "<table width=713 border=0 bgcolor=#ffffff cellspacing=0 cellpadding=0>"\
   "<tr style=\"padding-bottom:15px\">"\
	 "<td width=70>%s ID:</td>",search(lpublic,"instance"));
	 fprintf(cgiOut,"<td width=643>"\
		 "<select name=instance_id id=instance_id style=width:72px onchange=instanceid_change(this,\"wp_seculis.cgi\",\"%s\")>",m);
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
	  fprintf(cgiOut,"<table width=713 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>"\
      "<td align=left colspan=3>");
	  if(snum>0)           /*如果WLAN存在*/
	  {		
		fprintf(cgiOut,"<table frame=below rules=rows width=713 border=1>"\
		"<tr align=left>"\
		"<th width=70><font id=yingwen_thead>ID</font></th>");
        fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
		fprintf(cgiOut,"<th width=100><font id=%s>RadiusID</font></th>",search(lpublic,"menu_thead"));
        fprintf(cgiOut,"<th width=130><font id=%s>%s</font><font id=yingwen_thead> IP</font></th>",search(lpublic,"menu_thead"),search(lpublic,"host"));
        fprintf(cgiOut,"<th width=150><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsecu,"secur_type"));
		fprintf(cgiOut,"<th width=150><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lsecu,"encry_type"));
        fprintf(cgiOut,"<th width=13>&nbsp;</th>"\
        "</tr>");
		q=head;
		for(i=0;i<start_secno;i++)
		{
			if(q)
			{
				q=q->next;
			}
		}
		for(i=start_secno;i<end_secno;i++)
		{
		  memset(menu,0,sizeof(menu));
		  strncat(menu,"menuLists",sizeof(menu)-strlen(menu)-1);
		  snprintf(menu_id,sizeof(menu_id)-1,"%d",i+1); 
		  strncat(menu,menu_id,sizeof(menu)-strlen(menu)-1);
		  fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
		  if(q)
		  {
			  fprintf(cgiOut,"<td>%d</td>",q->SecurityID);
			  if(q->name)
			  {
				  fprintf(cgiOut,"<td>%s</td>",q->name);
			  }
			  fprintf(cgiOut,"<td>%d</td>",q->RadiusID);
			  if(q->host_ip)
			  {
				  fprintf(cgiOut,"<td>%s</td>",q->host_ip);
			  }
		  }
		  memset(SecurityType, 0, sizeof(SecurityType));
		  memset(EncryptionType, 0, sizeof(EncryptionType));
		  if(q)
		  {
			  CheckSecurityType(SecurityType, q->SecurityType);
			  CheckEncryptionType(EncryptionType, q->EncryptionType);
		  }
		  fprintf(cgiOut,"<td>%s</td>",SecurityType);
		  fprintf(cgiOut,"<td>%s</td>",EncryptionType);
		  memset(security_id,0,sizeof(security_id));
		  if(q)
		  {
			  snprintf(security_id,sizeof(security_id)-1,"%d",q->SecurityID);	 /*int转成char*/
		  }
		  fprintf(cgiOut,"<td>"\
		                           "<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(snum-i),menu,menu);
                                   fprintf(cgiOut,"<img src=/images/detail.gif>"\
                                   "<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
                                   fprintf(cgiOut,"<div id=div1>");
								   if(retu==0)/*管理员*/
								   {
								     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_secre.cgi?UN=%s target=mainFrame>%s</a></div>",m,search(lpublic,"create"));                             
									 if((p>0)&&(p==((snum-1)/MAX_PAGE_NUM))&&(((snum-1)%MAX_PAGE_NUM)==0))  /*如果是最后一页且删除该页的最后一项数据，跳转至上一页*/
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_seculis.cgi?UN=%s&SecuID=%s&DeletSecurity=%s&PN=%d&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,security_id,"true",p-1,select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));                             
									 else
								       fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_seculis.cgi?UN=%s&SecuID=%s&DeletSecurity=%s&PN=%d&INSTANCE_ID=%s&SubmitFlag=1 target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",m,security_id,"true",p,select_insid,search(lpublic,"confirm_delete"),search(lpublic,"delete"));                             
								     fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_secon.cgi?UN=%s&SecuID=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></div>",m,security_id,p,select_insid,search(lpublic,"configure"));
								   }
								   if((q)&&(q->name))
								   {
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_secdta.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s&Na=%s target=mainFrame>%s</a></div>",m,security_id,p,select_insid,q->name,search(lpublic,"details"));							  
								   }
								   else
								   {
									   fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_secdta.cgi?UN=%s&ID=%s&PN=%d&INSTANCE_ID=%s&Na=%s target=mainFrame>%s</a></div>",m,security_id,p,select_insid,"",search(lpublic,"details"));							  
								   }
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
		fprintf(cgiOut,"</table>");
	  }
	  else
		fprintf(cgiOut,"%s",search(lpublic,"no_security"));
	  fprintf(cgiOut,"</td></tr>");
	  if(snum>MAX_PAGE_NUM)               /*大于30个wtp时，显示翻页的链接*/
	  {
	    fprintf(cgiOut,"<tr style=\"padding-top:20px\">");
		if(p!=0)
	      fprintf(cgiOut,"<td align=left width=100><a href=wp_seculis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,p-1,select_insid,search(lpublic,"up_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"<td align=center width=413>%s",search(lpublic,"jump_to_page1"));
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
		if(p!=((snum-1)/MAX_PAGE_NUM))
	      fprintf(cgiOut,"<td align=right width=100><a href=wp_seculis.cgi?UN=%s&PN=%d&INSTANCE_ID=%s target=mainFrame>%s</a></td>",m,p+1,select_insid,search(lpublic,"down_page"));
		else
		  fprintf(cgiOut,"<td width=100>&nbsp;</td>");
	    fprintf(cgiOut,"</tr>");
	  }
      fprintf(cgiOut,"</table>");
	}
	else if(result == 0)
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
  Free_security_head(head);
}
free_instance_parameter_list(&paraHead1);
}

void DeleteSecurity(instance_parameter *ins_para,char *ID,struct list *lpublic,struct list *lsecu)
{
  char *endptr = NULL;  
  int secuid = 0,ret = 0;
  char alt[100] = { 0 };
  char max_sec_num[10] = { 0 };

  secuid=strtoul(ID,&endptr,10);  /*char转成int*/ 
  ret=delete_security(ins_para->parameter,ins_para->connection,secuid);  /*返回0表示 删除失败，返回1表示删除成功*/
																		/*返回-1表示security ID非法，返回-2表示security ID not existed*/
																		/*返回-3表示This Security Profile be used by some Wlans,please disable these Wlans first*/
																		/*返回-4表示出错，返回-5表示The radius heart test is on,turn it off first!*/
																		/*返回SNMPD_CONNECTION_ERROR表示connection error*/
  switch(ret)
  {
    case SNMPD_CONNECTION_ERROR:
	case 0:ShowAlert(search(lsecu,"delete_secur_fail"));
		   break;
	case 1:ShowAlert(search(lsecu,"delete_secur_succ"));
		   break;
	case -1:{
				memset(alt,0,sizeof(alt));
				strncpy(alt,search(lpublic,"secur_id_illegal1"),sizeof(alt)-1);
				memset(max_sec_num,0,sizeof(max_sec_num));
				snprintf(max_sec_num,sizeof(max_sec_num)-1,"%d",WLAN_NUM-1);
				strncat(alt,max_sec_num,sizeof(alt)-strlen(alt)-1);
				strncat(alt,search(lpublic,"secur_id_illegal2"),sizeof(alt)-strlen(alt)-1);
				ShowAlert(alt);
				break;
			}
	case -2:ShowAlert(search(lsecu,"secur_not_exist"));
			break;
	case -3:ShowAlert(search(lsecu,"secur_used_by_wlan"));
		    break;
	case -4:ShowAlert(search(lpublic,"error"));
			break;
	case -5:ShowAlert(search(lsecu,"radius_heart_is_on"));
            break;
  }
}


