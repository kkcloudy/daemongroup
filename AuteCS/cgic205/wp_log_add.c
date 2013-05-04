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
* wp_log_add.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for syslog config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include <fcntl.h>
#include <sys/wait.h>
#include "ws_log_conf.h"
#include "ws_dhcp_conf.h"

int ShowLogconfPage(struct list *lcontrol,struct list *lpublic);
int modify_systemip(struct list *lpublic);

int cgiMain()
{
	
	
	struct list *lcontrol;
	struct list *lpublic;
	lcontrol = get_chain_head("../htdocs/text/control.txt");
	lpublic= get_chain_head("../htdocs/text/public.txt");
	
	ShowLogconfPage(lcontrol,lpublic);
	release(lcontrol);
	release(lpublic); 
	return 0;
}



int ShowLogconfPage(struct list *lcontrol,struct list *lpublic)
{ 
  char *encry=(char *)malloc(BUF_LEN);
 
  char *str;
 //  FILE *pp;
 // char buff[128]; //读取popen中的块大小

  char file_name[128];  //读取文件
  memset(file_name,0,128);
  
  char version_encry[BUF_LEN]; 
  char addn[N]; 
  
  ST_IP_CONTENT ip;
  memset(&ip,0,sizeof(ip));  

   /*下拉框内容1*/
  char * select=(char*)malloc(256);
  memset(select,0,256); 

  struct substringz s_head,*sq;
  int subnum=0,retflag=-1;

  ST_LOG_KEY logkey;
  memset(&logkey,0,sizeof(logkey));       

   int i = 0;   
   cgiFormStringNoNewlines("Nb", file_name, 128);

  	
  if(cgiFormSubmitClicked("version") != cgiFormSuccess)
  {
		memset(encry,0,BUF_LEN);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		str=dcryption(encry);
		if(str==NULL)
		{
			ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
		}
		strcpy(addn,str);
		memset(version_encry,0,BUF_LEN);                   /*清空临时变量*/
  }
  else
  {
	cgiFormStringNoNewlines("encry_version", version_encry, BUF_LEN); 
	str=dcryption(version_encry);
	if(str==NULL)
	{
		ShowErrorPage(search(lpublic,"ill_user")); 	 /*用户非法*/
	}
	strcpy(addn,str);
	memset(version_encry,0,BUF_LEN);                   /*清空临时变量*/

  }
 
  cgiFormStringNoNewlines("encry_version",version_encry,BUF_LEN);
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
 
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  		"<style type=text/css>"\
  	".a3{width:30;border:0; text-align:center}"\
  	"</style>"\
    "<style type=text/css>"\
	".usrlis {overflow-x:hidden;overflow:auto; width: 350px; height: 100px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} "\
	"</style>"\
    "</head>"\
    "<script language=javascript src=/ip.js>"\
    "</script>"\
    "<script type=\"text/javascript\">"\
    "function log_ip()"\
    "{"\
  		"var bind_list = new Array();"\
  		"var ip1 = document.all.gate_ip1.value;"\
		"var ip2 = document.all.gate_ip2.value;"\
		"var ip3 = document.all.gate_ip3.value;"\
		"var ip4 = document.all.gate_ip4.value;"\
		"var pocol = document.all.tcp.value;"\
		"var port = document.all.portid.value;"\
		"var ip=ip1+\".\"+ip2+\".\"+ip3+\".\"+ip4;"\
		"if(ip1==\"\"||ip2==\"\"||ip3==\"\"||ip4==\"\") {"\
			"alert(\"%s\");"\
			"return;"\
			"}"\
	  	"var value = pocol + \"(\" + '\"' +ip + '\"' + \" port(\" + port + \"))\" ;"\
	  	"var number=document.all.select.options.length;"\
		"if(number > 7)"\
		"{"\
			"alert(\"%s\");"\
			"return ;"\
		"}"\
		"else{"\
		"var lenz=document.all.select.options.length;"\
		"if(lenz==0)"\
		"{"\
			"document.all.select.options.add("\
			    "new Option(value,value,false,false)"\
			");"\
		"}"\
		"else{"\
		    "var tflag=0;"\
		  	"for(var i=0; i<document.all.select.options.length; i++)"\
			"{"\
				"if( value == document.all.select.options[i].value)"\
				"{"\
					"alert(\"%s\");"\
					"tflag = 1;"\
					"document.all.gate_ip1.value=\"\";"\
					"document.all.gate_ip2.value=\"\";"\
					"document.all.gate_ip3.value=\"\";"\
					"document.all.gate_ip4.value=\"\";"\
					"return;"\
				"}"\
			"}"\
		    "if(tflag==0)"\
		    "{"\
				"document.all.select.options.add("\
			    "new Option(value,value,false,false)"\
			    ");"\
			    "document.all.gate_ip1.value=\"\";"\
				"document.all.gate_ip2.value=\"\";"\
				"document.all.gate_ip3.value=\"\";"\
				"document.all.gate_ip4.value=\"\";"\
			"}"\
		"}"\
	    "}"\
		"document.all.gate_ip1.value=\"\";"\
		"document.all.gate_ip2.value=\"\";"\
		"document.all.gate_ip3.value=\"\";"\
		"document.all.gate_ip4.value=\"\";"\
  "}",search(lcontrol,"ip_null"),search(lpublic,"log_max"),search(lpublic,"log_design"));
  fprintf(cgiOut,"function rm_ip()"\
  "{"\
  	"for(var i=0; i<document.all.select.options.length; i++){"\
			"if(document.all.select.options[i].selected==true){"\
			"var optionIndex ;"\
			"optionIndex = i;"\
			"if(optionIndex!=null){"\
			"document.all.select.remove(optionIndex);"\
			"}"\
			"}"\
			"}"\
	"}");
  fprintf(cgiOut,"function mysubmit()"\
	"{"\
		"var bind_ip=new Array();"\
		"var len=document.all.select.options.length;"\
		"if(len==0)"\
		"{"\
			"document.all.ip_s1.value=\"\";"\
		"}"\
		"else{"\
			"for(var i=0; i<len; i++)"\
							"{"\
	   							"if(i==0)"\
					   				"{"\
					   					"document.all.ip_s1.value = document.all.select.options[i].value ;"\
					   				"}"\
					   				"else{"\
					   					"document.all.ip_s1.value = document.all.ip_s1.value + \";\" + document.all.select.options[i].value ;"\
					   				"}"\
					   "}"\
					   "document.all.ip_s1.value = document.all.ip_s1.value + \";\" "\
				"}"\
	"}");
  fprintf(cgiOut,"</script>"\
  	"<body>");   
  
 
    if(cgiFormSubmitClicked("version") == cgiFormSuccess)
    {
       modify_systemip(lpublic);
    } 
	fprintf(cgiOut,"<form method=post onsubmit=\"return mysubmit()\">"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>");  //111111111111111111111
  fprintf(cgiOut,"<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom id=%s background=/images/di22.jpg>%s</td>",search(lpublic,"title_style"),search(lpublic,"log_info"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");
	   
        // 鉴权
        fprintf(cgiOut,"<input type=hidden name=UN value=%s />",encry);
		fprintf(cgiOut,"<input type=hidden name=Nb value=%s />",file_name);  //取到传送的值 

	   	
    	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"); //22222222222222222222
          fprintf(cgiOut,"<tr>");
          if(checkuser_group(addn)==0)  /*管理员*/
          	{
				 fprintf(cgiOut,"<td width=62 align=center><input id=but type=submit name=version style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));
		  	}
		   
		  if(cgiFormSubmitClicked("version") != cgiFormSuccess)
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_log_info.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else                                         
     		fprintf(cgiOut,"<td width=62 align=left><a href=wp_log_info.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",version_encry,search(lpublic,"img_cancel"));
		  fprintf(cgiOut,"</tr>"\
          "</table>");  //22222222222222222222222222
			
		
	fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>");
    fprintf(cgiOut,"<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>");
                            //333333333333333333333
	  fprintf(cgiOut,"<tr>");
        fprintf(cgiOut,"<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"); //44444444444444444444		
            fprintf(cgiOut,"<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>");
	fprintf(cgiOut,"<tr>");  //次内
              fprintf(cgiOut,"<td><table width=120 border=0 cellspacing=0 cellpadding=0>"); //555555555555555
                   fprintf(cgiOut,"<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");

			         if(cgiFormSubmitClicked("version") != cgiFormSuccess)
			         	{
                       fprintf(cgiOut,"<tr height=25>"\
					   "<td align=left id=tdleft><a href=wp_log_mod.cgi?UN=%s&Nb=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,file_name,search(lpublic,"menu_san"),search(lpublic,"log_modrule"));
					   fprintf(cgiOut,"</tr>");
			         	}
					 else
					 	{
                       fprintf(cgiOut,"<tr height=25>"\
					   "<td align=left id=tdleft><a href=wp_log_mod.cgi?UN=%s&Nb=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",version_encry,file_name,search(lpublic,"menu_san"),search(lpublic,"log_modrule"));
					   fprintf(cgiOut,"</tr>");

					 	}
					 
						 fprintf(cgiOut,"<tr height=26>"\
												 "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lpublic,"log_addip"));	/*突出显示*/
					 fprintf(cgiOut,"</tr>");
					
					


	for(i=0;i<7;i++)
	{
		fprintf(cgiOut,"<tr height=25>"\
			"<td id=tdleft>&nbsp;</td>"\
			"</tr>");
	}

	fprintf(cgiOut,"</table>"); 
	fprintf(cgiOut,"</td>"\
		"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">");

	fprintf(cgiOut,"<table width=600 border=0 cellspacing=0 cellpadding=0>");	 
	//js add and delete syslog info            
	fprintf(cgiOut,"<tr >"\
		"<td>");			
	fprintf(cgiOut,"<select name=select multiple=\"multiple\" size=\"6\" style=\"width:250px\">");
    /////deal with
	memset(&logkey,0,sizeof(logkey));                    
	find_log_node(XML_FPATH, NODE_DES, NODE_ATT, L_IP, NODE_CONTENT,&logkey);	
	string_linksep_list(&s_head, &subnum,logkey.key,";"); 
	sq=s_head.next;					
	while(sq != NULL)
	{
	    fprintf(cgiOut,"<option value='%s'>%s</option>",sq->substr,sq->substr);		
		sq=sq->next;				
	}
	if((retflag==0 )&& (subnum > 0))
		Free_substringz_all(&s_head);	
    /////////end
	fprintf(cgiOut,"</select></td>");
	fprintf(cgiOut,"<td colspan=2>\n");
	fprintf(cgiOut,"<table>\n");
	fprintf(cgiOut,"<tr>\n");
	fprintf(cgiOut,"<td>\n");
	fprintf(cgiOut,"<input type=button name=bind_add id=bind_add onclick=log_ip() style=\"width:50px\" value=\"%s\"><font color=red>(0-8)</font>",search(lcontrol, "dhcp_add"));
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>");
	fprintf(cgiOut,"<tr height=10><td colspan=2></td></tr>\n");
	fprintf(cgiOut,"<tr>\n");
	fprintf(cgiOut,"<td>\n");
	fprintf(cgiOut,"<input type=button name=ranges_rm id=ranges_rm onclick=rm_ip() style=\"width:50px\" value=\"%s\">",search(lcontrol, "dhcp_rm"));
	fprintf(cgiOut,"</td></tr>\n");
	fprintf(cgiOut,"</table>\n");
	fprintf(cgiOut,"</td>\n");
	fprintf(cgiOut,"</tr>\n");		

	/*分割线*/
	fprintf(cgiOut, "<tr><td colspan=3><hr width=100%% size=1 color=#fff align=center noshade />"\
		"</td></tr>" );

	fprintf(cgiOut,"<tr height=7><td colspan=3></td></tr>");

	fprintf(cgiOut,"<tr ><td bgcolor=#FFFFFF width=250>");
					 

	fprintf(cgiOut, "<label class=\"col1\" for=\"Package.DestinationAddress.Single\">%s</label></td><td colspan=2> \n", search(lpublic,"log_port") );
	fprintf(cgiOut, "<select style=width:140px  name=tcp>\n" );  
	fprintf(cgiOut,"<option value=udp>UDP</option>");
	fprintf(cgiOut,"<option value=tcp>TCP</option>");                     
	fprintf(cgiOut, "</select>\n" );					
	fprintf(cgiOut,"</td></tr>");   


	fprintf(cgiOut,"<tr height=7><td></td></tr>");

	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td width=250>%s</td>",search(lpublic,"log_ip"));
	fprintf(cgiOut,"<td colspan=4 width=140>"\
		"<div style=\"border-width:1;border-color:#a5acb2;border-style:solid;width:140;font-size:9pt\">");
	fprintf(cgiOut,"<input type=text	name=gate_ip1 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error")); 
	fprintf(cgiOut,"<input type=text	name=gate_ip2 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text	name=gate_ip3 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>.",search(lpublic,"ip_error"));
	fprintf(cgiOut,"<input type=text	name=gate_ip4 value=\"\" maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c()>",search(lpublic,"ip_error"));
	fprintf(cgiOut,"</div></td>");					 
	fprintf(cgiOut,"</tr>");

	fprintf(cgiOut,"<tr height=7><td></td></tr>");

	fprintf(cgiOut,"<tr>");
	fprintf(cgiOut,"<td width=250>%s:(1--65535 || 514)</td>",search(lpublic,"log_desport"));
	fprintf(cgiOut,"<td><input type=text name=portid style=width:140px value=514></td>");					 
	fprintf(cgiOut,"</tr>");

	fprintf(cgiOut,"<tr>");										
	if(cgiFormSubmitClicked("version") != cgiFormSuccess)
	{
		fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_version value=%s></td>",encry);
	}
	else if(cgiFormSubmitClicked("version") == cgiFormSuccess)
	{
		fprintf(cgiOut,"<td colspan=2><input type=hidden name=encry_version value=%s></td>",version_encry);

	}		
	fprintf(cgiOut,"<td><input type=hidden name=ip_s1 value=\"%s\"></td>",select);
	fprintf(cgiOut,"</tr>"\
		"</table>"); 
	fprintf(cgiOut,"</td>");
	fprintf(cgiOut,"</tr>");  
	fprintf(cgiOut,"<tr height=4 valign=top>"\
		"<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
		"<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
		"</tr>"\
		"</table>"); 
	fprintf(cgiOut,"</td>"\
		"<td width=15 background=/images/di999.jpg>&nbsp;</td>");
	fprintf(cgiOut,"</tr>"); 
	fprintf(cgiOut,"</table></td>");
	fprintf(cgiOut,"</tr>"\
		"<tr>"\
		"<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
		"<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
		"<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
		"</tr>"\
		"</table>"); 
	fprintf(cgiOut,"</div>"\
		"</form>"\
		"</body>");
	fprintf(cgiOut,"</html>");  

	free(encry);
	free(select);
	return 0;
}
//deal 
int modify_systemip(struct list *lpublic)
{
  char *select=(char*)malloc(256);
  memset(select,0,256);

  int ret=-1;
  ST_SYS_ALL sysall;  
  memset(&sysall,0,sizeof(sysall));
  
  cgiFormStringNoNewlines("ip_s1",select,256);
  mod_log_node(XML_FPATH,NODE_DES,NODE_ATT,L_IP,NODE_CONTENT,select); 
  read_filter(XML_FPATH, NODE_LOG, &sysall);
  ret=write_config(&sysall,CONF_FPATH);

  if(ret==0)
  	ShowAlert(search(lpublic,"oper_succ"));
  else
  	ShowAlert(search(lpublic,"oper_fail"));
  free(select);
  return 0;
}
