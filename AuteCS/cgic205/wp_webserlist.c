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
* wp_webserlist.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* system function for syslog info config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"


#include "ws_dbus_list.h"
#include "ws_dbus_list_interface.h"
#include "ws_init_dbus.h"
#include "ws_webservice_conf.h"




int ShowWebservicePage(struct list *lpublic, struct list *lsystem);     /*m代表加密后的字符串*/

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lsystem;	  /*解析system.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lsystem=get_chain_head("../htdocs/text/system.txt");

	ShowWebservicePage(lpublic,lsystem);
  	release(lpublic);  
  	release(lsystem);

	return 0;
}

int ShowWebservicePage(struct list *lpublic, struct list *lsystem)
{ 
	 
	  char *encry=(char *)malloc(BUF_LEN);				
	  char *str;
	  char	 web_name[15]={0};
	  char	 web_type[10]={0};
	  char	 web_ip1[4]={0};
	  char	 web_ip2[4]={0};
	  char	 web_ip3[4]={0};
	  char	 web_ip4[4]={0};
	  char	 web_port[10]={0};
	   char web_ip[32]={0};
	   char   web_slot[]={0};
	  int ret = 0,cl=1;
      char addn[N]="";

	  	  	 
		  memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
		  ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
		  return 0;
		}
		strcpy(addn,str);
  	
	  cgiFormStringNoNewlines("UN", encry, BUF_LEN); 

	  
	  ccgi_dbus_init();
	  fprintf(stderr,"1111111111111111111\n");
	  /***********************2008.5.26*********************/
	  cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lpublic,"web_service"));
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		"<style type=text/css>"\
		  "#div1{ width:82px; height:18px; border:1px solid #666666; background-color:#f9f8f7;}"\
		  "#div2{ width:80px; height:15px; padding-left:5px; padding-top:3px}"\
		  "#link{ text-decoration:none; font-size: 12px}"\
		"</style>"\
	  "<script language=javascript src=/ip.js>"\
	  "</script>"\
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

	  char IsDeleete[10] = { 0 };
	  char IsSubmit[5] = { 0 };
		char web_del_name[15]={0};
		int ret_del=0;
		int retu=0,retu1=0,i=0;
		char command[256] = {0};

	  memset(IsDeleete,0,sizeof(IsDeleete));
	  cgiFormStringNoNewlines("DeletWlan", IsDeleete, 10);
	  memset(IsSubmit,0,sizeof(IsSubmit));	
	  cgiFormStringNoNewlines("SubmitFlag", IsSubmit, 5);
	  if((strcmp(IsDeleete,"true")==0)&&(strcmp(IsSubmit,"")))
	  {
		  cgiFormStringNoNewlines("web_del_name", web_del_name, 15);
		  ret_del=ccgi_delete_http_https_config_cmd(web_del_name);
		  fprintf(stderr,"ret_del=%d",ret_del);
		  if(ret_del!=0)
		  {
			  switch(ret_del)
			  {
				  case -1:
					  ShowAlert(search(lpublic,"hs_param"));
					  break;
				  case -2:
					  ShowAlert(search(lpublic,"web_run"));
					  break;
				  case -3:
					  ShowAlert(search(lpublic,"no_web"));
					  break;
				  default:
					 ShowAlert(search(lpublic,"oper_fail"));
					  break;
			  }
		  }
		  else
		  {
			  ShowAlert(search(lpublic,"oper_succ"));
		  }
	  }	  
	  if(cgiFormSubmitClicked("starts") == cgiFormSuccess)
	  {
	  	
		strcat(command, "sudo /etc/init.d/apache2 restart >/var/log/apache_tmp2.log 2>&1;");
		retu1=WEXITSTATUS(system(command));	
		if(retu1==0)
		{
			ShowAlert(search(lpublic,"oper_succ"));
		}
		else
		{
			ShowAlert(search(lpublic,"oper_fail"));
		}
/*		  retu=contrl_disable_webservice_cmd();
		  retu1=contrl_enable_webservice_cmd();
		  if((retu==0)&&(retu1==0))
		  {
			  ShowAlert(search(lpublic,"oper_succ"));
		  }
		  else
		  {
			  ShowAlert(search(lpublic,"oper_fail"));
		  }*/
	  }

	 
	  fprintf(cgiOut,"<form method=post encType=multipart/form-data>"\
	  "<div align=center>"\
	  "<table width=976 border=0 cellpadding=0 cellspacing=0>");  
	  fprintf(cgiOut,"<tr>"\
		"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
		"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	  "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lsystem,"sys_function"));
		fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	  
	    fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
	 
			  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>");  
		fprintf(cgiOut,"<tr>"\
		"<td width=62 align=center><input id=but type=submit name=log_config style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));	
     	fprintf(cgiOut,"<td width=62 align=left><a href=wp_webservice.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		fprintf(cgiOut,"</tr>"\
		"</table>"); 	
		fprintf(cgiOut,"</td>"\
		"<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
		"<tr>"); 
		
			fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
			"<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); 
			  fprintf(cgiOut,"<tr height=4 valign=bottom>"\
				  "<td width=120>&nbsp;</td>"\
				  "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
			  "</tr>"\
			  "<tr>"\
				  "<td>");
			  fprintf(cgiOut,"<table width=120 border=0 cellspacing=0 cellpadding=0>");

			  fprintf(cgiOut,"<tr height=25>"\
				  "<td id=tdleft>&nbsp;</td>"\
				"</tr>"); 
                     /*管理员*/
//                     if(checkuser_group(addn)==0)
//					{
							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_webservice.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"add_web_service"));
							fprintf(cgiOut,"</tr>");

							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"web_service_list"));	/*突出显示*/
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_webportal.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"add_web_portal"));
							fprintf(cgiOut,"</tr>");

							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_webportalist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_portal_list"));
							fprintf(cgiOut,"</tr>");
							for(i=0;i<12;i++)
							{
								fprintf(cgiOut,"<tr height=25>"\
								"<td id=tdleft>&nbsp;</td>"\
								"</tr>");
							}
						
	//				}
                  
					fprintf(cgiOut,"</table>"); 
					fprintf(cgiOut,"</td>"\
					"<td  align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px\">");

					fprintf(cgiOut,"<table width=600 border=0 cellspacing=0 cellpadding=0>"); 
/////////////
				fprintf(cgiOut,"<tr height=30><td></td></tr>");
				fprintf(cgiOut,"<tr align=left>");
				fprintf(cgiOut,"<td>\n");
				fprintf(cgiOut,"<input type=submit name=starts value=\"%s\">",search(lpublic,"re_web_service"));
				fprintf(cgiOut,"</td>\n");
				fprintf(cgiOut,"</tr>\n");
				fprintf(cgiOut,"<tr height=15><td></td></tr>");


				fprintf(cgiOut,"<tr align=left bgcolor=%s>",setclour(cl));
					fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"name"));
				fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"l_type"));
				fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"webportal_ip"));
				fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"port"));
				fprintf(cgiOut,"<th width=100><font id=%s>%s</font></th>",search(lpublic,"menu_thead"),search(lpublic,"inter"));
				fprintf(cgiOut,"</tr>");


				fprintf(stderr,"111111111111111111111");
				
				struct web_info WtpIfHead;
				char menu[15]={0};
				char menu_id[10] = {0};
				struct web_info *q = NULL;
				int ret_web=0,num=0,master_slot_id=0;
				fprintf(stderr,"22222222222222222");
				ret_web=ccgi_show_webservice_info_cmd(&WtpIfHead,&num,&master_slot_id);
				
				fprintf(stderr,"uuuuuuuuuuuuuuuu33333333333333333");
				fprintf(stderr,"num=%d",num);
				for(q = WtpIfHead.next; q!=NULL; q = q->next)
				{
					i=i+1;
					memset(menu,0,15);
					strcat(menu,"menuLists");
					sprintf(menu_id,"%d",i); 
					strcat(menu,menu_id);
					fprintf(cgiOut,"<tr align=left>"\
						"<td width=100>%s</td>",q->name!=NULL?q->name:"");
					fprintf(cgiOut,"<td width=100>%s</td>",q->type==HTTP_SERVICE?"http":"https");
					fprintf(cgiOut,"<td width=100>%s</td>",q->address!=NULL?q->address:"");
					fprintf(cgiOut,"<td width=100>%d</td>",q->port);
					fprintf(cgiOut,"<td width=100>%s</td>",q->infname!=NULL?q->infname:"");
					fprintf(cgiOut,"<td>"\
					"<div style=\"position:relative; z-index:%d\" onmouseover=\"popMenu('%s');\" onmouseout=\"popMenu('%s');\">",(128-i),menu,menu);
					fprintf(cgiOut,"<img src=/images/detail.gif>"\
					"<div id=%s style=\"display:none; position:absolute; top:5px; left:0;\">",menu);
					fprintf(cgiOut,"<div id=div1>");
					fprintf(cgiOut,"<div id=div2 onmouseover=\"this.style.backgroundColor='#b6bdd2'\" onmouseout=\"this.style.backgroundColor='#f9f8f7'\"><a id=link href=wp_webserlist.cgi?UN=%s&DeletWlan=%s&SubmitFlag=1&web_del_name=%s target=mainFrame onclick=\"return confirm('%s')\">%s</a></div>",encry,"true",q->name,search(lpublic,"confirm_delete"),search(lpublic,"delete"));							  
					fprintf(cgiOut,"</div>"\
					"</div>"\
					"</div>"\
					"</td>");
					fprintf(cgiOut,"</tr>");
				}
				fprintf(stderr,"4444444444444444");


            fprintf(cgiOut,"<tr height=15><td></td></tr>");
			 
/////////		   
					
   fprintf(cgiOut,"</table>"); 
			  fprintf(cgiOut,"</td>"\
			  "</tr>"\
			  "<tr height=4 valign=top>"\
				  "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
				  "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
			  "</tr>"\
			"</table>");
		  fprintf(cgiOut,"</td>"\
		  "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
		"</tr>"\
	  "</table></td>");
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
	  "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
		"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
	"</tr>"\
	"</table>");
	fprintf(cgiOut,"</div>"\
	"</form>"\
	"</body>"\
	"</html>");
	free(encry);
	return 0;
}

