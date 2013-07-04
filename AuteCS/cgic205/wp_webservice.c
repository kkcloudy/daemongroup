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
* wp_webservice.c
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

      char addn[N]="";

	  char	 web_name[15]={0};
	  char	 web_type[10]={0};
	  char	 web_ip1[4]={0};
	  char	 web_ip2[4]={0};
	  char	 web_ip3[4]={0};
	  char	 web_ip4[4]={0};
	  char	 web_port[10]={0};
	   char web_ip[32]={0};
	   char web_inf[15]={0};
	  int ret = 0,ret1=0,i=0;
	  	  	 
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
	  instance_parameter *paraHead2 = NULL;
	  instance_parameter *p_q = NULL;
	  fprintf(stderr,"tttttttttttttt\n");
	  list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	  for(p_q=paraHead2;NULL!=p_q;p_q=p_q->next)
	  	{

	  }
	  
	  fprintf(stderr,"22222222222222222222\n");
	  /***********************2008.5.26*********************/
	  cgiHeaderContentType("text/html");
	  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	  fprintf(cgiOut,"<title>%s</title>",search(lpublic,"web_service"));
	  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
		"<style type=text/css>"\
		".a3{width:30;border:0; text-align:center}"\
		"</style>"\
	  "<script language=javascript src=/ip.js>"\
	  "</script>"\
		"</head>"\
		"<body>");

	  if(cgiFormSubmitClicked("log_config") == cgiFormSuccess)
	  {
		   memset(web_name,0,15);
		   memset(web_type,0,10);
		   memset(web_ip1,0,4);
		   memset(web_ip2,0,4);
		   memset(web_ip3,0,4);
		   memset(web_ip4,0,4);
		   memset(web_port,0,10);
		   memset(web_ip,0,32);
			memset(web_inf,0,15);
		   for(p_q=paraHead2;NULL!=p_q;p_q=p_q->next)
			{
		  		fprintf(stderr,"111111111p_q->connection=%p",p_q->connection);
		   }
		   cgiFormStringNoNewlines("web_name",web_name,15);
		  cgiFormStringNoNewlines("web_type",web_type,10);
		  cgiFormStringNoNewlines("web_ip1",web_ip1,4);
		  cgiFormStringNoNewlines("web_ip2",web_ip2,4);
		  cgiFormStringNoNewlines("web_ip3",web_ip3,4);
		  cgiFormStringNoNewlines("web_ip4",web_ip4,4);
		  cgiFormStringNoNewlines("web_port",web_port,4);
		  cgiFormStringNoNewlines("web_inf",web_inf,15);
		  sprintf(web_ip,"%ld.%ld.%ld.%ld",strtoul(web_ip1,0,10),strtoul(web_ip2,0,10),strtoul(web_ip3,0,10),strtoul(web_ip4,0,10));
		  fprintf(stderr,"web_name=%s",web_name);
		  fprintf(stderr,"web_type=%s",web_type);
		  fprintf(stderr,"web_port=%s",web_port);
		  fprintf(stderr,"web_ip=%s",web_ip);
		  fprintf(stderr,"&&&&&&&&&&&&web_inf=%s",web_inf);
		  if((strcmp(web_name,"")!=0)&&(strcmp(web_type,"")!=0)&&(strcmp(web_ip,"")!=0)&&(strcmp(web_port,"")!=0))
		  {
		  	ret=0;
			  ret=ccgi_add_http_https_ip_port_cmd(web_name,web_type,web_ip,web_port);
			  if(ret!=0)
			  {
				  switch(ret)
				  {
					  case -1:
						  ShowAlert(search(lpublic,"hs_param"));
						  break;
					  case -2:
						  ShowAlert(search(lpublic,"max_num_http"));
						  break;
					  case -3:
						  ShowAlert(search(lpublic,"web_run"));
						  break;
					  case -4:
						  ShowAlert(search(lpublic,"web_na_ex"));
						  break;
					  case -5:
						  ShowAlert(search(lpublic,"web_ex"));
						  break;
					  default:
					 	 ShowAlert(search(lpublic,"oper_fail"));
						  break;
				  }

			  }
		  }
		  if((ret==0)&&(strcmp(web_inf,"")!=0))
		  {
			  fprintf(stderr,"fffffffffffffffffffffffffff");
		  		ret1=0;
			  ret1=ccgi_add_web_forword_cmd(web_name,web_inf);
			  if(ret1!=0)
			  {
				  switch(ret)
				  {
					  case -1:
						  ShowAlert(search(lpublic,"name_error"));
						  break;
					  case -2:
						  ShowAlert(search(lpublic,"inf_error"));
						  break;
					  case -4:
						  ShowAlert(search(lpublic,"max_num_http"));
						  break;
					  case -5:
						  ShowAlert(search(lpublic,"no_web"));
						  break;
					  case -6:
						  ShowAlert(search(lpublic,"web_ex"));
						  break;
					  case -7:
						  ShowAlert(search(lpublic,"web_run"));
						  break;
					  default:
						 ShowAlert(search(lpublic,"oper_fail_inf"));
						  break;
				  }
			  }
		  }
		  if((ret==0) && (ret1==0))
		  {
			  ShowAlert(search(lpublic,"oper_succ"));
		  }
		fprintf(stderr,"ret=%d",ret);
		fprintf(stderr,"ret1=%d",ret1);
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
     	fprintf(cgiOut,"<td width=62 align=left><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
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
							 fprintf(cgiOut,"<tr height=25>"\
							 "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"add_web_service"));  /*突出显示*/
							 fprintf(cgiOut,"</tr>");
							//新增条目
							fprintf(cgiOut,"<tr height=25>"\
							"<td align=left id=tdleft><a href=wp_webserlist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lpublic,"web_service_list"));
							fprintf(cgiOut,"</tr>");

							//boot upgrade 
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

		  //web service		  
		  fprintf(cgiOut,"<tr height=15><td></td></tr>");
		  

		fprintf(cgiOut,"<tr height=30>"\
		  "<td>%s:</td>",search(lpublic,"name"));
		  fprintf(cgiOut,"<td align=left><input type=text name=web_name size=22 maxLength=10 onkeypress=\"return (event.keyCode!=32)&&(event.keyCode!=35)&&(event.keyCode!=38)\"><font color=red>(%s)</font></td>",search(lpublic,"legal_input"));
	   
	   
	   fprintf(cgiOut,"<tr height=30>");
	   fprintf(cgiOut,"<td>\n");
	   fprintf(cgiOut,"%s:",search(lpublic,"type"));
	   fprintf(cgiOut,"</td>\n");
	   fprintf(cgiOut,"<td>\n");
	   fprintf(cgiOut,"<select name=web_type>");
		   fprintf(cgiOut,"<option value='http'>http</option>");
		   fprintf(cgiOut,"<option value='https'>https</option>");
	   fprintf(cgiOut,"</select>\n");
	   fprintf(cgiOut,"</td>\n");
	   fprintf(cgiOut,"</tr>\n");
      
	   fprintf(cgiOut,"<tr	height=30>"\
	   "<td width=140>%s:</td>",search(lpublic,"webportal_ip"));
	   fprintf(cgiOut,"<td colspan=2>"\
	   "<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:150;font-size:9pt\">");
	   fprintf(cgiOut,"<input type=text  name=web_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	   fprintf(cgiOut,"<input type=text  name=web_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	   fprintf(cgiOut,"<input type=text  name=web_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	   fprintf(cgiOut,"<input type=text  name=web_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	   fprintf(cgiOut,"</div></td>");
	   fprintf(cgiOut,"</tr>\n");
	   
	   fprintf(cgiOut,"<tr height=30>"\
		 "<td>%s:</td>",search(lpublic,"port"));
		 fprintf(cgiOut,"<td align=left><input type=text name=web_port size=22 maxLength=5><font color=red>(1-65535)</font></td>");
		 
		 fprintf(cgiOut,"<tr height=30>"\
		   "<td>%s:</td>",search(lpublic,"s_inf_name"));
		   fprintf(cgiOut,"<td align=left><input type=text name=web_inf size=22 maxLength=14></td>");
////////////////		  					
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
	"</table>");//111111111111111
	fprintf(cgiOut,"</div>"\
	"</form>"\
	"</body>"\
	"</html>");
	free(encry); 
	return 0;
}  



